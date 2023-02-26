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
#define EOL_PROTOCOL_MASTER_CAN_ID    0x566U  /**< 上位机CAN ID */
#define EOL_PROTOCOL_REPLY_CAN_ID     0x666U  /**< 回复上位机CAN ID */
#define EOL_TEMP_BUF_SIZE_MAX         64U     /**< 临时缓冲区大小 */
/* 包重复检测 */
#define PACKAGE_REPEAT_CHECK_SIZE     10U     /**< 10包循环检测 */

/* 最小帧长 7Bytes DATA */
#define FRAME_MIN_SIZE                (EOL_FRAME_MIN_SIZE)

#define FRAME_TEMP_BUF_SIZE           (64U)

#define WAIT_LIST_LEN                 100U
#define FRAME_TIME_OUT                3U      /**< 3s超时检测 */
#define NO_RESPONSE_TIMES             5U      /**< 允许无响应次数，发出无响应信号 */
#define ACK_FRAME_LEN                 5       /**< ACK帧长 */
#define WAIT_DATA_ACK_TIMEOUT         8       /**< 等待ACK超时时间s */
#define RETRY_NUM_MAX                 3U      /**< 超时重试次数 */
#define NOT_FULL_TIMEOUT_SEC_MAX      15U     /**< 允许帧不全超时时间 */
#define WAIT_SEND_HW_ERR_TIME         1U      /**< 60s */
/* 寄存器表 */
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

void eol_protocol::eol_protocol_clear()
{
  wait_response_list.clear();
  CircularQueue::CQ_emptyData(cq_obj->get_cq_handle());
}

void eol_protocol::run_eol_task()
{
  /* 执行EOL协议解析 */
  qDebug() << "start protocal stack current thread:" << QThread::currentThreadId();
  quint8 frame_buf[FRAME_TEMP_BUF_SIZE];
  quint32 len = 0;
  quint32 id = 0;
  while(run_state)
  {
    //QCoreApplication::processEvents();
    can_driver_obj->receice_data();
    QThread::msleep(1);
    len = cq_obj->CQ_getLength(cq_obj->get_cq_handle());

    /* 检测是否满足最小帧长 */
    if(len < FRAME_MIN_SIZE)
    {
      continue;
    }

    /* 检测ID */
    cq_obj->CQ_ManualGetData(cq_obj->get_cq_handle(), frame_buf, 4);
    memcpy(&id, frame_buf, 4);

    switch(id)
    {
      case 0x0566:
        break;
      default:
        break;
    }

    /* 发出数据 */
//  emit signal_post_data(frame_buf+FRAME_DATA_OFFSET, data_len);//<! 由于线程原因需加上连接参数Qt::BlockingQueuedConnection进行同步
  }
  qDebug() << "eol protocol stack end";
}

/**
  * @brief   待回复任务检测
  * @param   [in]force true强制发送.
  * @return  true 存在待回复任务.
  */
bool eol_protocol::check_wait_send_task(bool force)
{
  if(send_task_handle.wait_send_size == 0)
  {
    return false;
  }
  uint16_t can_send_size = 0;

  /* 检测经过时间 */
  quint32 elapsed_time_ms = (quint32)current_time_ms - send_task_handle.last_send_time_ms;
  if(elapsed_time_ms < ENABLE_SEND_DELAY_MS && force == false)
  {
    return true;
  }

  send_task_handle.last_send_time_ms = current_time_ms;

  can_send_size = (send_task_handle.data_total_size - send_task_handle.current_send_index) > SEND_ONE_PACKET_SIZE_MAX ? \
        SEND_ONE_PACKET_SIZE_MAX : (send_task_handle.data_total_size - send_task_handle.current_send_index);

  /* 发送 */
  bool ret = can_driver_obj->send(reinterpret_cast<const quint8 *>(send_task_handle.buf_ptr + send_task_handle.current_send_index), \
                       (quint8)can_send_size, EOL_PROTOCOL_MASTER_CAN_ID, can_driver::STD_FRAME_TYPE, \
                                  can_driver::CANFD_PROTOCOL_TYPE);
  if(false == ret)
  {
    qDebug() << "send error";
    /* 关闭发送 */
    send_task_handle.hw_send_err_times++;
    if((send_task_handle.hw_send_err_times / 1000) > WAIT_SEND_HW_ERR_TIME)
    {
      send_task_handle.hw_send_err_times = 0;
      send_task_handle.wait_send_size = 0;
    }
    return false;
  }
  send_task_handle.hw_send_err_times = 0;
  send_task_handle.current_send_index = (send_task_handle.current_send_index + SEND_ONE_PACKET_SIZE_MAX) > send_task_handle.data_total_size?send_task_handle.current_send_index:(send_task_handle.current_send_index + SEND_ONE_PACKET_SIZE_MAX);
  send_task_handle.wait_send_size -= can_send_size;
  return true;
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
  if(command == EOL_WRITE_CMD)
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
  else if(command == EOL_READ_CMD)
  {
    send_buf[index++] = (quint8)command;
    send_buf[index++] = static_cast<uint8_t>(reg_addr&0x00FF);
    send_buf[index++] = static_cast<uint8_t>((reg_addr>>8)&0xFF);

    uint16_t crc_val = utility::get_modbus_crc16_with_tab(send_buf, index);
    send_buf[index++] = static_cast<uint8_t>((crc_val&0x00FF));
    send_buf[index++] = static_cast<uint8_t>((crc_val>>8)&0xFF);
  }

#if ENABLE_SEND_DELAY
  /* 检测发送大小是否需要分包发送 */
  if(index > ENABLE_SEND_DELAY_LIMIT_SIZE)
  {
    send_task_handle.current_send_index = 0;
    send_task_handle.data_total_size = index;
    send_task_handle.wait_send_size = index;
    send_task_handle.buf_ptr = send_buf;
    check_wait_send_task(true);

    /* 解析 */
    RETURN_TYPE_Typedef_t ret = RETURN_OK;
    if(command == EOL_WRITE_CMD || command == EOL_READ_CMD)
    {
      while((ret = protocol_stack_wait_reply_start()) == RETURN_WAITTING);
    }

    eol_protocol_clear();
    return ret;
  }
#endif
  qDebug() << "-------------";

  /* 发送 */
  bool ok = can_driver_obj->send(reinterpret_cast<const quint8 *>(send_buf), \
                         (quint8)index, EOL_PROTOCOL_MASTER_CAN_ID, can_driver::STD_FRAME_TYPE, \
                                 can_driver::CANFD_PROTOCOL_TYPE);
  if(false == ok)
  {
    qDebug() << "send error";
//    protocol_stack_stop();
    return RETURN_ERROR;
  }

  /* 解析 */
  RETURN_TYPE_Typedef_t ret = RETURN_OK;
  if(command == EOL_WRITE_CMD || command == EOL_READ_CMD)
  {
    while((ret = protocol_stack_wait_reply_start()) == RETURN_WAITTING);
  }

//  protocol_stack_stop();

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
      qDebug() << "clear";
      acc_error_cnt++;
      if(acc_error_cnt > NO_RESPONSE_TIMES)
      {
        acc_error_cnt = 0;
        qDebug() << "signal_protocol_no_response";
        emit signal_protocol_no_response();
      }
      return RETURN_TIMEOUT;
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

        acc_error_cnt = 0;

        /* 缓冲区更新 */
        CircularQueue::CQ_ManualOffsetInc(cq, package_len);

        DOA_TABLE_OPT_STATUS_Typedef_t ret = decode_ack_frame(temp_buf);
        if(DOA_TABLE_OPT_OK != ret)
        {
          emit signal_protocol_error_occur((quint8)ret);
          return RETURN_ERROR;
        }
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
        if(utility::get_modbus_crc16_rsl_with_tab(temp_buf, static_cast<uint16_t>(package_len - 2)) == true)
        {
          /* 处理数据 */
          RETURN_TYPE_Typedef_t ret = decode_data_frame(wait, temp_buf + 8, data_len);

          /* 正常写入-->移除等待队列 */
          wait_response_list.removeFirst();
          CircularQueue::CQ_ManualOffsetInc(cq, package_len);
          acc_error_cnt = 0;

          return ret;
        }
        break;
      }

      default:
        /* TODO */
        CircularQueue::CQ_ManualOffsetInc(cq, 1);
        return RETURN_WAITTING;
    }
    CircularQueue::CQ_ManualOffsetInc(cq, 1);
    return RETURN_ERROR;
  }
}

bool eol_protocol::eol_master_send_table_data(DOA_TABLE_HEADER_Typedef_t &table_info, const quint8 *data)
{
  quint8 data_buf[FRAME_TEMP_BUF_SIZE];
  quint8 index = 0;
  quint16 frame_num = 0;
  quint8 error_cnt = 0;
  RETURN_TYPE_Typedef_t ret;

  /* 表头数据 */
  index = 0;
  frame_num = 0;
  data_buf[index++] = frame_num;// 第0包表头信息
  data_buf[index++] = (quint8)(frame_num >> 8);
  data_buf[index++] = (quint8)table_info.Table_Type;
  data_buf[index] = table_info.Version_MINOR;
  data_buf[index] <<= 4;
  data_buf[index++] |= table_info.Version_MAJOR;
  data_buf[index++] = table_info.Version_REVISION;
  data_buf[index++] = table_info.Data_Type;
  data_buf[index++] = table_info.Data_Size;
  data_buf[index++] = (quint8)(table_info.Data_Size >> 8);
  quint32 crc = utility::get_crc32_with_tab(data, table_info.Data_Size);
  data_buf[index++] = (quint8)crc;
  data_buf[index++] = (quint8)(crc >> 8);
  data_buf[index++] = (quint8)(crc >> 16);
  data_buf[index++] = (quint8)(crc >> 24);
  /* 发送表头信息 */
  do
  {
    ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_RW_TABLE_DATA_REG, data_buf, index);
    if(RETURN_OK != ret)
    {
      if(RETRY_NUM_MAX <= error_cnt)
      {
        error_cnt++;
        continue;
      }
      return false;
    }
  }while(0);

  /* 清空错误统计 */
  error_cnt = 0;

  /* 发送表数据，每包4Bytes，不足0填充 */
  for(quint32 i = 0; i < table_info.Data_Size + 3;)
  {
    frame_num++;
    index = 0;
    data_buf[index++] = frame_num;
    data_buf[index++] = (quint8)(frame_num >> 8);
    data_buf[index++] = (quint8)table_info.Data_Type;

    memcpy(&data_buf[index], data + i, 4);
    index += 4;

    /* 发送表数据 */
    do
    {
      ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_RW_TABLE_DATA_REG, data_buf, index);
      if(RETURN_OK != ret)
      {
        if(RETRY_NUM_MAX <= error_cnt)
        {
          error_cnt++;
          continue;
        }
        return false;
      }
    }while(0);

    /* 清空错误统计 */
    error_cnt = 0;

    i += 4;
    emit signal_send_progress(i, table_info.Data_Size);
  }

  error_cnt = 0;

  /* 发送结束帧 */
  index = 0;
  data_buf[index++] = 0xFF;
  data_buf[index++] = 0xFF;
  do
  {
    ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_RW_TABLE_DATA_REG, data_buf, index);
    if(RETURN_OK != ret)
    {
      if(RETRY_NUM_MAX <= error_cnt)
      {
        error_cnt++;
        continue;
      }
      return false;
    }
  }while(0);
  return true;
}

bool eol_protocol::eol_master_get_table_data(DOA_TABLE_Typedef_t table_type, quint8 *data)
{
  /* 读取表信息 */
  /* 设置需要读取的表 */
  quint8 data_buf[FRAME_TEMP_BUF_SIZE];
  quint8 index = 0;
  quint8 error_cnt = 0;
  RETURN_TYPE_Typedef_t ret;

  /* 表选择 */
  index = 0;
  data_buf[index++] = (quint8)table_type;;
  /* 发送 */
  do
  {
    ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_W_TABLE_SEL_REG, data_buf, index);
    if(RETURN_OK != ret)
    {
      if(RETRY_NUM_MAX <= error_cnt)
      {
        error_cnt++;
        continue;
      }
      return false;
    }
  }while(0);

  /* 清空错误统计 */
  error_cnt = 0;
  ret = RETURN_OK;
  /* 读取表数据 */
  do
  {
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
      if(RETRY_NUM_MAX <= error_cnt)
      {
        error_cnt++;
        continue;
      }
      return false;
    }

    /* 清空错误统计 */
    error_cnt = 0;

    /* 数据帧未结束 */
    if(0xFFFF > data_record.frame_num)
    {
      continue;
    }
  }while(0);

  /* 拷贝数据 */
  memcpy(data, data_record.data_buf, data_record.data_size);
  return true;
}
/******************************** End of file *********************************/
