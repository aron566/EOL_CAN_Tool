/**
 *  @file eol_protocol.cpp
 *
 *  @date 2023年02月20日 17:05:52 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @details None.
 *
 *  @version v1.0.0 aron566 2023.02.20 17:05 初始版本.
 */
/** Includes -----------------------------------------------------------------*/
/** Private includes ---------------------------------------------------------*/
#include "eol_protocol.h"
#include <QDebug>
#include <QFile>
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
/*
上位机写入
           /------------/------------/------------/------------/------------/------------/-----------/
         /     帧头    /    功能码   /  寄存器地址  /   数据长度  /  变长数据   /    CRC16_L  /  CRC16_H  /
       /------------/------------/------------/------------/------------/-------------/-----------/
     /   2Bytes   /    1Byte   /   2Bytes   /   2Bytes   /     ...    /    1Byte    /    1Byte   /
   /------------/------------/------------/------------/------------/-------------/------------/
 *//***********C*************R***************C**********************/
/*
PS：
默认小端模式：低8位在前
帧长最小为：10Bytes
总帧长为：9Bytes + 数据长度
 */

/*
回复
           /------------/------------/------------/------------/------------/
         /    帧头     /    功能码   /  寄存器地址  / ACK RESULT /   MESSAGE  /
       /------------/------------/------------/------------/------------/
     /   2Bytes   /    1Byte   /   2Bytes   /   1Bytes   /   1Bytes   /
   /------------/------------/------------/------------/------------/
PS：
帧长固定为：7Bytes
*/

/*
上位机读取
           /------------/------------/------------/------------/------------/
         /    帧头    /    功能码   /   寄存器地址  /  CRC16_L   /   CRC16_H  /
       /------------/------------/-------------/------------/------------/
     /   2Bytes   / 1Byte 0x04 /    2Bytes   /   1Byte    /    1Byte   /
   /------------/------------/-------------/------------/------------/
 *//******C********R********C*************/
/*
PS：
默认小端模式：低8位在前
帧长固定为：7Bytes
*/

/*
回复
           /------------/------------/------------/------------/------------/------------/------------/
         /    帧头     /    功能码   /  寄存器地址  /   数据长度   /   变长数据  /  CRC16_L   /   CRC16_H  /
       /------------/------------/-------------/------------/------------/------------/------------/
     /   2Bytes   /    1Byte   /    2Bytes   /   2Bytes   /     ...    /    1Byte   /    1Byte   /
   /------------/------------/-------------/------------/------------/------------/------------/
 *//*******************C******************R*************C************/
/*
PS：
默认小端模式：低8位在前
帧长最短为：10Bytes
总帧长为：9Bytes + 数据长度
*/
#define ENABLE_SEND_DELAY             1       /**< 为1开启分包发送 */
#define ENABLE_SEND_DELAY_MS          5U      /**< 分包发送间隔ms >5ms */
#define ENABLE_SEND_DELAY_LIMIT_SIZE  8U      /**< >8Bytes时开启发送 */
#define SEND_ONE_PACKET_SIZE_MAX      8U      /**< 每包发送大小 */

#define MASTER_EOL_FRAME_HEADER       0x7AU   /**< 上位机帧头 */
#define SLAVE_EOL_FRAME_HEADER        0x75U   /**< 下位机帧头 */

#define EOL_FRAME_MIN_SIZE            7U      /**< 最小帧长 */

#define EOL_DEVICE_COM_ADDR           0x55U   /**< 下位机通讯地址 */
#define EOL_PROTOCOL_MASTER_CAN_ID    0x157U  /**< 上位机CAN ID */
#define EOL_PROTOCOL_REPLY_CAN_ID     0x257U  /**< 回复上位机CAN ID */
#define EOL_TEMP_BUF_SIZE_MAX         64U     /**< 临时缓冲区大小 */
/* 包重复检测 */
#define PACKAGE_REPEAT_CHECK_SIZE     10U     /**< 10包循环检测 */

/* 最小帧长 7Bytes DATA */
#define FRAME_MIN_SIZE                (EOL_FRAME_MIN_SIZE)

#define FRAME_TEMP_BUF_SIZE           (64U)

#define WAIT_LIST_LEN                 100U
#define FRAME_TIME_OUT                3U      /**< 3s超时检测 */
#define NO_RESPONSE_TIMES             0U      /**< 允许超时无响应次数，发出无响应信号 */

#define RETRY_NUM_MAX                 3U      /**< 无法发出数据，超时重试次数 */
#define NOT_FULL_TIMEOUT_SEC_MAX      15U     /**< 允许帧不全超时时间 */
#define WAIT_SEND_HW_ERR_TIMES        3U      /**< 硬件发送失败3次 */
/* 寄存器表 */
#define EOL_META_DATA_CHECK_REG        0x0000U/**< 安全认证请求 */
#define EOL_META_DATA_CHECK_OK_REG     0x0001U/**< 安全认证确认 */
#define EOL_META_DATA_SET_MODE_REG     0x0002U/**< 模式选择 */
#define EOL_RW_TABLE_DATA_REG          0x0007U
#define EOL_W_TABLE_SEL_REG            0x0008U
#define EOL_W_TABLE_DATA_SEL_REG       0x0009U

/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/*
		  	/-------/-------/-------/-------/-------/
			/   FF  /  RTR  /  DLC  /   ID  /  DATA /
		/-------/-------/-------/-------/-------/
	/ 1 bit / 1 bit / 4 bit / 11 bit/8 Bytes/
/-------/-------/-------/-------/-------/
标准帧 数据帧（8 Bytes）
*/

/** Private function prototypes ----------------------------------------------*/

/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

eol_protocol::eol_protocol(QObject *parent)
    : QObject{parent}
{
  cq_obj = new CircularQueue(CircularQueue::UINT8_DATA_BUF, CircularQueue::CQ_BUF_512, this);
  if(nullptr == cq_obj)
  {
    qDebug() << "eol create cq faild";
  }

  /* 初始化分包发送句柄 */
  memset(&send_task_handle, 0, sizeof(SEND_TASK_LIST_Typedef_t));

  /* 定时器初始化 */
  timer_init();
}

void eol_protocol::set_can_driver_obj(can_driver *can_driver_)
{
  can_driver_obj = can_driver_;
  if(nullptr == can_driver_)
  {
    return;
  }
}

void eol_protocol::eol_protocol_clear()
{
  wait_response_list.clear();
  CircularQueue::CQ_emptyData(cq_obj->get_cq_handle());
}

/**
 * @brief eol协议栈线程任务
 */
void eol_protocol::run_eol_task()
{
  /* 执行EOL协议解析 */
  qDebug() << "start eol protocal stack current thread:" << QThread::currentThreadId();

  /* 检查是否存在任务 */
  EOL_TASK_LIST_Typedef_t task_;
  while(eol_task_list.isEmpty() == false && run_state)
  {
    /* 取任务 */
    task_ = eol_task_list.takeFirst();

    /* 执行任务 */
//    while(task_.run_state && run_state)
//    {
      if(nullptr == task_.param)
      {
        (this->*(task_.task))(&task_);
      }
      else
      {
        (this->*(task_.task))(task_.param);
      }
//    }
  }
  qDebug() << "eol protocol stack thread end";
}


/**
  * @brief   待回复任务检测
  * @param   [in]force true强制发送.
  * @return  true 存在待回复任务.
  */
eol_protocol::SNED_CHECK_STATUS_Typedef_t eol_protocol::check_wait_send_task(bool force)
{
  if(send_task_handle.wait_send_size == 0)
  {
    return WAIT_NOTHING;
  }

  /* 检测经过时间 */
  quint32 elapsed_time_ms = (quint32)current_time_ms - send_task_handle.last_send_time_ms;
  if(elapsed_time_ms < ENABLE_SEND_DELAY_MS && force == false)
  {
    return WAIT_SEND;
  }

  send_task_handle.last_send_time_ms = current_time_ms;

  quint32 can_send_size = (send_task_handle.data_total_size - send_task_handle.current_send_index) > SEND_ONE_PACKET_SIZE_MAX ? \
        SEND_ONE_PACKET_SIZE_MAX : (send_task_handle.data_total_size - send_task_handle.current_send_index);

  /* 发送 */
  bool ret = can_driver_obj->send(reinterpret_cast<const quint8 *>(send_task_handle.buf_ptr + send_task_handle.current_send_index), \
                       (quint8)can_send_size, current_send_can_id, can_driver::STD_FRAME_TYPE, \
                                  can_driver::CANFD_PROTOCOL_TYPE);
  if(false == ret)
  {
    qDebug() << "hw send error " << send_task_handle.hw_send_err_times;
    /* 关闭发送 */
    send_task_handle.hw_send_err_times++;
    if(send_task_handle.hw_send_err_times > WAIT_SEND_HW_ERR_TIMES)
    {
      qDebug() << "hw send close";
      send_task_handle.wait_send_size = 0;
    }
    return SEND_ERR;
  }
  send_task_handle.hw_send_err_times = 0;

  /* 更新发送指针 */
  send_task_handle.current_send_index = (send_task_handle.current_send_index + SEND_ONE_PACKET_SIZE_MAX) > send_task_handle.data_total_size ? send_task_handle.current_send_index : (send_task_handle.current_send_index + SEND_ONE_PACKET_SIZE_MAX);
  send_task_handle.wait_send_size -= can_send_size;
  return WAIT_SEND;
}


bool eol_protocol::response_is_timeout(WAIT_RESPONSE_LIST_Typedef_t &wait)
{
  if((current_time_sec - wait.start_time) >= FRAME_TIME_OUT)
  {
    return true;
  }
  return false;
}


uint32_t eol_protocol::check_can_read(CircularQueue::CQ_handleTypeDef *cq)
{
  /* 判断CAN ID 跳过无效头 */
  if(CircularQueue::CQ_ManualGet_Offset_Data(cq, 0) != SLAVE_EOL_FRAME_HEADER || CircularQueue::CQ_ManualGet_Offset_Data(cq, 1) != EOL_DEVICE_COM_ADDR)
  {
    if(CircularQueue::CQ_skipInvaildU8Header(cq, (quint8)EOL_PROTOCOL_MASTER_CAN_ID) == 0)
    {
      return 0;
    }
  }
  /* 返回缓冲区长度 */
  return CircularQueue::CQ_getLength(cq);
}


void eol_protocol::timer_init()
{
  protocol_Timer = new QTimer(this);
  connect(protocol_Timer, &QTimer::timeout, this, &eol_protocol::slot_timer_timeout);
  protocol_Timer->start(1);
}

/**
 * @brief eol_protocol::slot_timer_timeout
 */
void eol_protocol::slot_timer_timeout()
{
  current_time_ms++;
  if(current_time_ms == 1000)
  {
    current_time_ms = 0;
    current_time_sec++;
  }

  /* 检测发送任务 */
  check_wait_send_task();
}

/* 打包报文进行发送并等待回复 */
eol_protocol::RETURN_TYPE_Typedef_t eol_protocol::protocol_stack_create_task(EOL_CMD_Typedef_t command, uint16_t reg_addr,
                                          const uint8_t *data, uint16_t data_len)
{
  static uint8_t send_buf[FRAME_TEMP_BUF_SIZE];
  /* 帧头 */
  send_buf[0] = MASTER_EOL_FRAME_HEADER;
  send_buf[1] = EOL_DEVICE_COM_ADDR;

  if(wait_response_list.size() > WAIT_LIST_LEN)
  {
    return RETURN_ERROR;
  }
  eol_protocol::WAIT_RESPONSE_LIST_Typedef_t wait;
  wait.reg_addr = reg_addr;
  wait.start_time = static_cast<uint32_t>(current_time_sec);
  wait.command = (quint8)command;
  wait_response_list.append(wait);

  uint16_t index = 2;

  /* 写入 */
  if(EOL_WRITE_CMD == command)
  {
    send_buf[index++] = (quint8)command;
    send_buf[index++] = static_cast<uint8_t>(reg_addr&0x00FF);
    send_buf[index++] = static_cast<uint8_t>((reg_addr>>8)&0xFF);
    send_buf[index++] = static_cast<uint8_t>(data_len&0x00FF);
    send_buf[index++] = static_cast<uint8_t>((data_len>>8)&0xFF);
    for(uint16_t data_index = 0; data_index < data_len; data_index++)
    {
      send_buf[index++] = data[data_index];
    }
    uint16_t crc_val = utility::get_modbus_crc16_with_tab(send_buf, index);
    send_buf[index++] = static_cast<uint8_t>((crc_val&0x00FF));
    send_buf[index++] = static_cast<uint8_t>((crc_val>>8)&0xFF);
  }

  /* 读取 */
  else if(EOL_READ_CMD == command)
  {
    send_buf[index++] = (quint8)command;
    send_buf[index++] = static_cast<uint8_t>(reg_addr&0x00FF);
    send_buf[index++] = static_cast<uint8_t>((reg_addr>>8)&0xFF);

    uint16_t crc_val = utility::get_modbus_crc16_with_tab(send_buf, index);
    send_buf[index++] = static_cast<uint8_t>((crc_val&0x00FF));
    send_buf[index++] = static_cast<uint8_t>((crc_val>>8)&0xFF);
  }

  /* 数据裸露发送 */
  else if(EOL_META_CMD == command)
  {
    index = data_len;
    memcpy_s(send_buf, FRAME_TEMP_BUF_SIZE, data, data_len);
  }

#if ENABLE_SEND_DELAY
  /* 检测发送大小是否需要分包发送 */
  if(index > ENABLE_SEND_DELAY_LIMIT_SIZE)
  {
    send_task_handle.current_send_index = 0;
    send_task_handle.data_total_size = index;
    send_task_handle.wait_send_size = index;
    send_task_handle.buf_ptr = send_buf;
    send_task_handle.last_send_time_ms = current_time_ms;
    SNED_CHECK_STATUS_Typedef_t state = check_wait_send_task(false);//不立即发

    if(EOL_WRITE_CMD == command || EOL_META_CMD == command)
    {
      /* 发送一帧 */
      emit signal_send_rec_one_frame(true);
    }

    if(SEND_ERR == state)
    {
      eol_protocol_clear();
      return RETURN_ERROR;
    }

    /* 解析 */
    RETURN_TYPE_Typedef_t ret = RETURN_OK;
    if(EOL_WRITE_CMD == command || EOL_READ_CMD == command ||
       EOL_META_CMD == command)
    {
      while((ret = protocol_stack_wait_reply_start()) == RETURN_WAITTING);
    }

    if(ret == RETURN_OK && (EOL_READ_CMD == command || EOL_META_CMD == command))
    {
      /* 接收一帧 */
      emit signal_send_rec_one_frame(false);
    }

    eol_protocol_clear();
    return ret;
  }
#endif
  qDebug() << "------direct send-------";

  /* 发送 */
  bool ok = can_driver_obj->send(reinterpret_cast<const quint8 *>(send_buf), \
                         (quint8)index, current_send_can_id, can_driver::STD_FRAME_TYPE, \
                                 can_driver::CANFD_PROTOCOL_TYPE);

  if(EOL_WRITE_CMD == command || EOL_META_CMD == command)
  {
    /* 发送一帧 */
    emit signal_send_rec_one_frame(true);
  }

  if(false == ok)
  {
    qDebug() << "send error";
    eol_protocol_clear();
    return RETURN_ERROR;
  }

  /* 解析 */
  RETURN_TYPE_Typedef_t ret = RETURN_OK;
  if(EOL_WRITE_CMD == command || EOL_READ_CMD == command ||
     EOL_META_CMD == command)
  {
    while((ret = protocol_stack_wait_reply_start()) == RETURN_WAITTING);
  }

  if(ret == RETURN_OK && (EOL_READ_CMD == command || EOL_META_CMD == command))
  {
    /* 接收一帧 */
    emit signal_send_rec_one_frame(false);
  }

  eol_protocol_clear();
  return ret;
}

/* 从机ack应答 */
eol_protocol::DOA_TABLE_OPT_STATUS_Typedef_t eol_protocol::decode_ack_frame(const quint8 *data)
{
  quint16 reg_addr = 0;
  reg_addr = data[4];
  reg_addr <<= 8;
  reg_addr |= data[3];

  quint8 ack_rsl = data[5];
  QString msg_str;
  if(0 == ack_rsl)
  {
    msg_str = "no error";
  }
  else
  {
    msg_str = "error";
  }

  DOA_TABLE_OPT_STATUS_Typedef_t msg = (DOA_TABLE_OPT_STATUS_Typedef_t)data[6];
  switch(reg_addr)
  {
    case EOL_RW_TABLE_DATA_REG:
    default:
      qDebug() << msg_str << ":" << msg;
      return msg;
  }
  return DOA_TABLE_OPT_OK;
}

eol_protocol::RETURN_TYPE_Typedef_t eol_protocol::decode_data_frame(WAIT_RESPONSE_LIST_Typedef_t &wait, const quint8 *data, quint16 data_len)
{
  switch(wait.reg_addr)
  {
    case EOL_RW_TABLE_DATA_REG:
    {
      quint16 frame_num;
      memcpy(&frame_num, data, 2);
      qDebug() << "signal_recv_eol_table_data";
      emit signal_recv_eol_table_data(frame_num, data, data_len);

      /* 表信息帧 */
      if(0 == frame_num)
      {
        memset(&data_record, 0, sizeof(data_record));
      }

      /* 表数据帧 */
      else if(0 < frame_num && 0xFFFF > frame_num)
      {
        /* 是否否和预期帧号，否则属于丢帧 */
        if((data_record.frame_num + 1) == frame_num)
        {
          /* 只拷贝数据 帧号[0..1] 数据类型[2] 数据[3..6] */
          memcpy(&data_record.data_buf[data_record.frame_num * 4], data + 3, 4);
          data_record.frame_num++;
          data_record.data_size += 4;
        }
        /* 丢帧要求重发 */
        else
        {
          return RETURN_LOST_FRAME;
        }
      }
      else
      {
        qDebug() << "signal_recv_eol_data_complete";
        emit signal_recv_eol_data_complete();
      }
      break;
    }

    default:
      break;
  }
  return RETURN_OK;
}

eol_protocol::RETURN_TYPE_Typedef_t eol_protocol::protocol_stack_wait_reply_start()
{
  static quint8 temp_buf[FRAME_TEMP_BUF_SIZE];
  uint16_t data_len = 0;
  quint32 package_len = 0;

//  QCoreApplication::processEvents();
  /* 获取句柄是否为空 */
  if(cq_obj == nullptr)
  {
    return RETURN_ERROR;
  }

  CircularQueue::CQ_handleTypeDef *cq = cq_obj->get_cq_handle();

  /* 检查响应队列 */
  if(wait_response_list.isEmpty() == true)
  {
    CircularQueue::CQ_emptyData(cq);
    return RETURN_OK;
  }

  /* 获取等待数据信息 */
  WAIT_RESPONSE_LIST_Typedef_t wait = wait_response_list.first();

  while(1)
  {
    /* 检测响应帧超时 */
    if(response_is_timeout(wait) == true)
    {
      /* 移除等待队列-清空缓冲区 */
      wait_response_list.removeFirst();
      CircularQueue::CQ_emptyData(cq);
      qDebug() << "clear wait one";
      acc_error_cnt++;
      if(acc_error_cnt > NO_RESPONSE_TIMES)
      {
        qDebug() << "response_is_timeout > " << \
          (NO_RESPONSE_TIMES + 1) * FRAME_TIME_OUT << "s -> signal_protocol_timeout";
        emit signal_protocol_timeout(acc_error_cnt * FRAME_TIME_OUT);
        acc_error_cnt = 0;
      }
      return RETURN_TIMEOUT;
    }

    /* 元数据报文 */
    uint32_t meta_len = 0;
    if((quint8)EOL_META_CMD == wait.command)
    {
      meta_len = CircularQueue::CQ_getLength(cq);
      if(meta_len > EOL_FRAME_MIN_SIZE)
      {
        goto __META_CMD_DECDE;
      }
      continue;
    }

    /* 检测缓冲区可读长度 */
    if(check_can_read(cq) == 0)
    {
      return RETURN_WAITTING;
    }

    uint32_t len = CircularQueue::CQ_getLength(cq);
    if(EOL_FRAME_MIN_SIZE > len)
    {
      return RETURN_WAITTING;
    }

    /* 判断帧类型 */
    uint8_t cmd = CircularQueue::CQ_ManualGet_Offset_Data(cq, 2);

    switch((EOL_CMD_Typedef_t)cmd)
    {
      /* 主机写入，从机回复报文 */
      case EOL_WRITE_CMD:
      {
        package_len = EOL_FRAME_MIN_SIZE;

        /* 写入应答完成 */
        CircularQueue::CQ_ManualGetData(cq, temp_buf, package_len);

        /* 正常写入-->移除等待队列 */
        wait_response_list.removeFirst();

        /* 缓冲区更新 */
        CircularQueue::CQ_ManualOffsetInc(cq, package_len);

        DOA_TABLE_OPT_STATUS_Typedef_t ret = decode_ack_frame(temp_buf);
        if(DOA_TABLE_OPT_OK != ret)
        {
          qDebug() << "signal_protocol_error_occur";
          emit signal_protocol_error_occur((quint8)ret);
          return RETURN_ERROR;
        }

        acc_error_cnt = 0;
        return RETURN_OK;
      }

      /* 主机读取，从机回复报文 */
      case EOL_READ_CMD:
      {
        data_len = static_cast<uint16_t>(CircularQueue::CQ_ManualGet_Offset_Data(cq, 6) << 8);
        data_len += CircularQueue::CQ_ManualGet_Offset_Data(cq, 5);
        package_len = 9 + data_len;
        package_len = (package_len > FRAME_TEMP_BUF_SIZE) ? FRAME_TEMP_BUF_SIZE : package_len;

        if(len < package_len)
        {
          return RETURN_WAITTING;
        }
        CircularQueue::CQ_ManualGetData(cq, temp_buf, package_len);
        if(utility::get_modbus_crc16_rsl_with_tab(temp_buf, static_cast<uint16_t>(package_len - 2)) == false)
        {
          break;
        }

        /* 处理数据 */
        RETURN_TYPE_Typedef_t ret = decode_data_frame(wait, temp_buf + 8, data_len);

        /* 正常写入-->移除等待队列 */
        wait_response_list.removeFirst();
        CircularQueue::CQ_ManualOffsetInc(cq, package_len);

        acc_error_cnt = 0;
        return ret;
      }

      default:
        /* TODO */
        CircularQueue::CQ_ManualOffsetInc(cq, 1);
        return RETURN_WAITTING;
    }
    CircularQueue::CQ_ManualOffsetInc(cq, 1);
    return RETURN_ERROR;

__META_CMD_DECDE:
    /* 取出数据 */
    CircularQueue::CQ_ManualGetData(cq, temp_buf, meta_len);
    QString str;
    for(quint8 i = 0; i < meta_len; i++)
    {
      str += QString::asprintf("%02X ", temp_buf[i]);
    }
    qDebug() << str;
    switch(wait.reg_addr)
    {
      case EOL_META_DATA_CHECK_REG:
        if(0 == memcmp(send_table_data.buf, temp_buf, 4))
        {
          /* 校验sum */
          if(true == utility::get_sum_rsl(temp_buf, 7))
          {
            /* 拷贝随机数 */
            memcpy(send_table_data.buf + 4, temp_buf + 4, 3);

           /* 正常移除等待队列 */
            wait_response_list.removeFirst();
            CircularQueue::CQ_ManualOffsetInc(cq, meta_len);

            acc_error_cnt = 0;
            return RETURN_OK;
          }
        }
        break;
      case EOL_META_DATA_CHECK_OK_REG:
        if(0 == memcmp(send_table_data.buf, temp_buf, 4))
        {
          /* 正常移除等待队列 */
           wait_response_list.removeFirst();
           CircularQueue::CQ_ManualOffsetInc(cq, meta_len);

           acc_error_cnt = 0;
          return RETURN_OK;
        }
        break;
      case EOL_META_DATA_SET_MODE_REG:
        /* 生产普通 - 01[ok] 31 58 AF 02[配置数量] 08[通道数量] 00 00 */
        if(0 == memcmp(send_table_data.buf + 1, temp_buf + 1, 3))
        {
          if(1 == temp_buf[0])
          {
            /* profile */
            send_table_data.buf[1] = temp_buf[4];
            /* channel */
            send_table_data.buf[2] = temp_buf[5];
          }
          /* 正常移除等待队列 */
           wait_response_list.removeFirst();
           CircularQueue::CQ_ManualOffsetInc(cq, meta_len);

           acc_error_cnt = 0;
          return RETURN_OK;
        }

        /* 正常模式 返回全0 */
        else if(NORMAL_MODE_RUN == (DEVICE_MODE_Typedef_t)send_table_data.buf[8])
        {
          memset(send_table_data.buf, 0, 8);
          if(0 == memcmp(send_table_data.buf, temp_buf, 8))
          {
            /* 正常移除等待队列 */
             wait_response_list.removeFirst();
             CircularQueue::CQ_ManualOffsetInc(cq, meta_len);

             acc_error_cnt = 0;
            return RETURN_OK;
          }
        }
        break;
      default:
        break;
    }
    CircularQueue::CQ_ManualOffsetInc(cq, 1);
    return RETURN_ERROR;
  }
}

/* 获取表数据任务 */
bool eol_protocol::get_eol_table_data_task(void *param_)
{
  /* 暂停其他数据接收显示 */
  quint32 last_canid_mask = 0;
  bool last_canid_mask_en = false;
  can_driver_obj->set_msg_canid_mask(EOL_PROTOCOL_REPLY_CAN_ID + 100, true, &last_canid_mask, &last_canid_mask_en);

  qDebug() << "get_eol_table_data_task start";

  DOA_TABLE_HEADER_Typedef_t *param = (DOA_TABLE_HEADER_Typedef_t *)param_;
  /* 清空 */
  eol_protocol_clear();

  /* 启动接收任务 */
  /* 读取表信息 */
  /* 设置需要读取的表 */
  quint8 data_buf[FRAME_TEMP_BUF_SIZE];
  quint8 index = 0;
  quint8 error_cnt = 0;
  RETURN_TYPE_Typedef_t ret;

  /* 表选择 */
  index = 0;
  data_buf[index++] = (quint8)param->Table_Type;
  /* 发送 */
  do
  {
    qDebug() << "get_eol_table_data_task step 1";
    ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_W_TABLE_SEL_REG, data_buf, index);
    if(RETURN_OK != ret)
    {
      error_cnt++;
      if(RETRY_NUM_MAX > error_cnt)
      {
        continue;
      }
      goto __get_eol_table_data_err;
    }
    break;
  }while(run_state);

  /* 再次判断线程运行状态 */
  if(false == run_state)
  {
    goto __get_eol_table_data_err;
  }

  /* 清空错误统计 */
  error_cnt = 0;
  ret = RETURN_OK;
  /* 读取表数据 */
  do
  {
    qDebug() << "get_eol_table_data_task step 2";
    /* 丢帧检测 */
    if(RETURN_LOST_FRAME == ret)
    {
      data_buf[0] = (quint8)data_record.frame_num;
      data_buf[1] = (quint8)(data_record.frame_num >> 8);
      ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_W_TABLE_DATA_SEL_REG, data_buf, 2);
    }
    else
    {
      /* 读数据帧 */
      ret = protocol_stack_create_task(EOL_READ_CMD, EOL_RW_TABLE_DATA_REG, nullptr, 0);
    }

    if(RETURN_OK != ret)
    {
      error_cnt++;
      if(RETRY_NUM_MAX > error_cnt)
      {
        continue;
      }
      goto __get_eol_table_data_err;
    }

    /* 清空错误统计 */
    error_cnt = 0;

    /* 数据帧未结束 */
    if(0xFFFF > data_record.frame_num)
    {
      continue;
    }
    break;
  }while(run_state);

  /* 未到结束帧推出认为失败 */
  if(0xFFFF > data_record.frame_num)
  {
    goto __get_eol_table_data_err;
  }

  qDebug() << "get_eol_table_data_task exit ok";

  /* 恢复数据接收显示 */
  can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);

  return true;

__get_eol_table_data_err:
  /* 恢复数据接收显示 */
  can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);

  qDebug() << "signal_protocol_no_response";
  emit signal_protocol_no_response();
  return false;
}

bool eol_protocol::set_device_mode_task(void *param_)
{
  /* 暂停其他数据接收显示 */
  quint32 last_canid_mask = 0;
  bool last_canid_mask_en = false;
  can_driver_obj->set_msg_canid_mask(0x200 + 100, true, &last_canid_mask, &last_canid_mask_en);

  qDebug() << "set_device_mode_task start";

  /* 清空 */
  eol_protocol_clear();

  quint8 error_cnt = 0;
  RETURN_TYPE_Typedef_t ret;
  EOL_SEND_DATA_Typedef_t *param = (EOL_SEND_DATA_Typedef_t *)param_;
  DEVICE_MODE_Typedef_t mode = (DEVICE_MODE_Typedef_t)param->buf[8];

  quint32 index = 0;
  /* 设置安全认证报文 */
  send_table_data.buf[index++] = 0x0E;
  send_table_data.buf[index++] = 0x31;
  send_table_data.buf[index++] = 0x58;
  send_table_data.buf[index++] = 0xAF;
  send_table_data.buf[index++] = 0x00;
  send_table_data.buf[index++] = 0x01;
  send_table_data.buf[index++] = 0x02;
  send_table_data.buf[index++] = (quint8)utility::get_data_sum(send_table_data.buf, 7);

  /* 发送安全请求 */
  qDebug() << "step1 device safe check ";
  do
  {
    ret = protocol_stack_create_task(EOL_META_CMD, EOL_META_DATA_CHECK_REG, param->buf, 8);
    if(RETURN_OK != ret)
    {
      error_cnt++;
      if(RETRY_NUM_MAX > error_cnt)
      {
        continue;
      }
      goto __set_device_mode_err;
    }
    break;
  }while(run_state);

  /* 再次判断线程运行状态 */
  if(false == run_state)
  {
    goto __set_device_mode_err;
  }

  /* 设置安全确认报文 */
  index = 0;
  param->buf[index++] = 0x0F;
  param->buf[index++] = 0x31;
  param->buf[index++] = 0x58;
  param->buf[index++] = 0xAF;
  param->buf[7] = (quint8)utility::get_data_sum(param->buf, 7);
  /* 发送安全确认 */
  qDebug() << "step2 device safe ok ";
  do
  {
    ret = protocol_stack_create_task(EOL_META_CMD, EOL_META_DATA_CHECK_OK_REG, param->buf, 8);
    if(RETURN_OK != ret)
    {
      error_cnt++;
      if(RETRY_NUM_MAX > error_cnt)
      {
        continue;
      }
      goto __set_device_mode_err;
    }
    break;
  }while(run_state);

  /* 再次判断线程运行状态 */
  if(false == run_state)
  {
    goto __set_device_mode_err;
  }

  /* 进入指定模式 */
  param->buf[0] = (quint8)mode;

  memset(param->buf + 4, 0, 4);

  qDebug() << "step3 device mode set ";
  do
  {
    ret = protocol_stack_create_task(EOL_META_CMD, EOL_META_DATA_SET_MODE_REG, param->buf, 8);
    if(RETURN_OK != ret)
    {
      error_cnt++;
      if(RETRY_NUM_MAX > error_cnt)
      {
        continue;
      }
      goto __set_device_mode_err;
    }
    break;
  }while(run_state);

  /* 再次判断线程运行状态 */
  if(false == run_state)
  {
    goto __set_device_mode_err;
  }

  /* 发送切换后的设备模式 */
  param->buf[0] = (quint8)mode;
  emit signal_device_mode(param->buf);

  /* 恢复数据接收显示 */
  can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);

  return true;

__set_device_mode_err:
  /* 恢复数据接收显示 */
  can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);

  qDebug() << "signal_protocol_no_response";
  emit signal_protocol_no_response();
  return false;
}

bool eol_protocol::send_eol_table_data_task(void *param_)
{
  /* 暂停其他数据接收显示 */
  quint32 last_canid_mask = 0;
  bool last_canid_mask_en = false;
  can_driver_obj->set_msg_canid_mask(EOL_PROTOCOL_REPLY_CAN_ID + 0x100, true, &last_canid_mask, &last_canid_mask_en);

  qDebug() << "send_eol_table_data_task start";
  quint8 data_buf[FRAME_TEMP_BUF_SIZE];
  quint8 index = 0;
  quint16 frame_num = 0;
  quint8 error_cnt = 0;
  RETURN_TYPE_Typedef_t ret;

  EOL_SEND_DATA_Typedef_t *param = (EOL_SEND_DATA_Typedef_t *)param_;

  /* 清空 */
  eol_protocol_clear();

  /* 表头数据 */
  index = 0;
  frame_num = 0;
  data_buf[index++] = frame_num;// 第0包表头信息
  data_buf[index++] = (quint8)(frame_num >> 8);
  data_buf[index++] = (quint8)param->head_data.Table_Type;
  data_buf[index] = param->head_data.Version_MINOR;
  data_buf[index] <<= 4;
  data_buf[index++] |= param->head_data.Version_MAJOR;
  data_buf[index++] = param->head_data.Version_REVISION;
  data_buf[index++] = param->head_data.Data_Type;
  data_buf[index++] = param->head_data.Data_Size;
  data_buf[index++] = (quint8)(param->head_data.Data_Size >> 8);
  quint32 crc = utility::get_crc32_with_tab(param->data, param->head_data.Data_Size);
  data_buf[index++] = (quint8)crc;
  data_buf[index++] = (quint8)(crc >> 8);
  data_buf[index++] = (quint8)(crc >> 16);
  data_buf[index++] = (quint8)(crc >> 24);
//  qDebug("crc 0x%08X size %u", crc, param->head_data.Data_Size);
  /* 发送表头信息 */
  qDebug() << "step1 send table head ";
  do
  {
    ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_RW_TABLE_DATA_REG, data_buf, index);
    if(RETURN_OK != ret)
    {
      error_cnt++;
      if(RETRY_NUM_MAX > error_cnt)
      {
        continue;
      }
      goto __send_eol_table_data_err;
    }
    break;
  }while(run_state);

  /* 再次判断线程运行状态 */
  if(false == run_state)
  {
    goto __send_eol_table_data_err;
  }
//  utility::debug_print(param->data, param->head_data.Data_Size);
  /* 清空错误统计 */
  error_cnt = 0;
  qDebug() << "step2 send table data ";
  /* 发送表数据，每包4Bytes，不足0填充 */
  for(quint32 i = 0; i < param->head_data.Data_Size;)
  {
    frame_num++;
    index = 0;
    data_buf[index++] = frame_num;
    data_buf[index++] = (quint8)(frame_num >> 8);
    data_buf[index++] = (quint8)param->head_data.Data_Type;

    if(index > param->head_data.Data_Size)
    {
      memset(&data_buf[index], 0, 4);
      memcpy(&data_buf[index], param->data + i, param->head_data.Data_Size - index);
    }
    else
    {
      memcpy(&data_buf[index], param->data + i, 4);
    }

    index += 4;

    /* 发送表数据 */
    do
    {
      ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_RW_TABLE_DATA_REG, data_buf, index);
      if(RETURN_OK != ret)
      {
        error_cnt++;
        if(RETRY_NUM_MAX > error_cnt)
        {
          continue;
        }
        goto __send_eol_table_data_err;
      }
      break;
    }while(run_state);

    /* 再次判断线程运行状态 */
    if(false == run_state)
    {
      goto __send_eol_table_data_err;
    }

    /* 清空错误统计 */
    error_cnt = 0;

    i += 4;
    qDebug() << "signal_send_progress";
    emit signal_send_progress(i, param->head_data.Data_Size);
  }

  error_cnt = 0;

  /* 发送结束帧 */
  qDebug() << "step3 send end frame";
  index = 0;
  data_buf[index++] = 0xFF;
  data_buf[index++] = 0xFF;
  do
  {
    ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_RW_TABLE_DATA_REG, data_buf, index);
    if(RETURN_OK != ret)
    {
      error_cnt++;
      if(RETRY_NUM_MAX > error_cnt)
      {
        continue;
      }
      goto __send_eol_table_data_err;
    }
    break;
  }while(run_state);

  /* 再次判断线程运行状态 */
  if(false == run_state)
  {
    goto __send_eol_table_data_err;
  }

  /* 发送完成信号 */
  emit signal_send_eol_data_complete();

  /* 恢复数据接收显示 */
  can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);

  return true;

__send_eol_table_data_err:
  /* 恢复数据接收显示 */
  can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);

  qDebug() << "signal_protocol_no_response";
  emit signal_protocol_no_response();
  return false;
}

bool eol_protocol::eol_master_set_device_mode(DEVICE_MODE_Typedef_t mode)
{
  /* 查重 */
  for(qint32 i = 0; i < eol_task_list.size(); i++)
  {
    if(eol_task_list.value(i).task == &eol_protocol::set_device_mode_task && \
       send_table_data.buf[8] == (quint8)mode)
    {
      return true;
    }
  }

  /* 设置消息过滤器 */
  can_driver_obj->add_msg_filter(0x200, cq_obj);
  current_send_can_id = 0x100;

  /* 目标设置模式 */
  send_table_data.buf[8] = (quint8)mode;

  EOL_TASK_LIST_Typedef_t task;
  task.run_state = true;
  task.param = &send_table_data;
  task.task = &eol_protocol::set_device_mode_task;
  eol_task_list.append(task);
  return true;
}

bool eol_protocol::eol_master_send_table_data(DOA_TABLE_HEADER_Typedef_t &table_info, const quint8 *data)
{
  /* 查重 */
  for(qint32 i = 0; i < eol_task_list.size(); i++)
  {
    if(eol_task_list.value(i).task == &eol_protocol::send_eol_table_data_task && \
       send_table_data.head_data.Table_Type == table_info.Table_Type)
    {
      return true;
    }
  }

  /* 设置消息过滤器 */
  can_driver_obj->add_msg_filter(EOL_PROTOCOL_REPLY_CAN_ID, cq_obj);
  current_send_can_id = EOL_PROTOCOL_MASTER_CAN_ID;

  memcpy(&send_table_data.head_data, &table_info, sizeof(DOA_TABLE_HEADER_Typedef_t));
  send_table_data.data = data;

  EOL_TASK_LIST_Typedef_t task;
  task.run_state = true;
  task.param = &send_table_data;
  task.task = &eol_protocol::send_eol_table_data_task;
  eol_task_list.append(task);
  return true;
}

bool eol_protocol::eol_master_get_table_data(DOA_TABLE_Typedef_t table_type)
{
  /* 查重 */
  for(qint32 i = 0; i < eol_task_list.size(); i++)
  {
    if(eol_task_list.value(i).task == &eol_protocol::get_eol_table_data_task && \
       send_table_data.head_data.Table_Type == table_type)
    {
      return true;
    }
  }

  /* 设置消息过滤器 */
  can_driver_obj->add_msg_filter(EOL_PROTOCOL_REPLY_CAN_ID, cq_obj);
  current_send_can_id = EOL_PROTOCOL_MASTER_CAN_ID;

  send_table_data.head_data.Table_Type = table_type;

  EOL_TASK_LIST_Typedef_t task;
  task.run_state = true;
  task.param = &send_table_data.head_data;
  task.task = &eol_protocol::get_eol_table_data_task;
  eol_task_list.append(task);

  /* 数据记录清空 */
  memset(&data_record, 0, sizeof(data_record));
  return true;
}
/******************************** End of file *********************************/
