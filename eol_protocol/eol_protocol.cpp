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
#include <QDateTime>
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
/*
上位机写入
           /------------/------------/------------/------------/------------/------------/-----------/
         /     帧头    /    功能码   /  寄存器地址 /    数据长度  /   变长数据  /   CRC16_L   /  CRC16_H  /
       /------------/------------/------------/------------/------------/-------------/-----------/
     /   2Bytes   /    bit0    /   bit1-7   /   2Bytes   /     ...    /    1Byte    /    1Byte   /
   /------------/------------/------------/------------/------------/-------------/------------/
*//***********C*************R***************C**********************/
/*
PS：
默认小端模式：低8位在前
帧长最小为：8Bytes
总帧长为：7Bytes + 数据长度
 */

/*
回复
           /------------/------------/------------/------------/------------/------------/------------/
         /     帧头    /   功能码    /  寄存器地址  / ACK RESULT /   MESSAGE  /   CRC16_L  /   CRC16_H  /
       /------------/------------/------------/------------/------------/------------/------------/
     /   2Bytes   /     bit0   /   bit1-7   /   1Bytes   /   1Bytes   /   1Byte    /    1Byte   /
   /------------/------------/------------/------------/------------/------------/------------/
*//***********C*************R***************C**********************/
/*
PS：
帧长固定为：7Bytes
*/

/*
上位机读取
           /------------/------------/------------/------------/------------/
         /     帧头    /    功能码   /  寄存器地址  /  CRC16_L   /   CRC16_H  /
       /------------/------------/-------------/------------/------------/
     /   2Bytes   /    bit0    /    bit1-7   /   1Byte    /    1Byte   /
   /------------/------------/-------------/------------/------------/
*//******C********R********C*************/
/*
PS：
默认小端模式：低8位在前
帧长固定为：5Bytes
*/

/*
回复
           /------------/------------/------------/------------/------------/------------/------------/
         /     帧头    /    功能码   /   寄存器地址 /    数据长度  /   变长数据  /  CRC16_L   /   CRC16_H  /
       /------------/------------/-------------/------------/------------/------------/------------/
     /   2Bytes   /    bit0    /    bit1-7   /   2Bytes   /     ...    /    1Byte   /    1Byte   /
   /------------/------------/-------------/------------/------------/------------/------------/
*//*******************C******************R*************C************/
/*
PS：
默认小端模式：低8位在前
帧长最短为：8Bytes
总帧长为：7Bytes + 数据长度
*/
#define ENABLE_SEND_DELAY             1       /**< 为1开启分包发送 */
#define ENABLE_SEND_DELAY_MS          0U      /**< 分包发送间隔ms >1ms */
#define ENABLE_SEND_DELAY_LIMIT_SIZE  64U     /**< >64Bytes时开启发送 */
#define SEND_ONE_PACKET_SIZE_MAX      64U     /**< 每包发送大小 */

#define MASTER_EOL_FRAME_HEADER       0x7AU   /**< 上位机帧头 */
#define SLAVE_EOL_FRAME_HEADER        0x75U   /**< 下位机帧头 */

#define EOL_FRAME_MIN_SIZE            7U      /**< 最小帧长 */

#define EOL_DEVICE_COM_ADDR           dev_addr/**< 下位机通讯地址 */
#define EOL_DEVICE_BROADCAST_COM_ADDR 0x55U   /**< 下位机通讯广播地址 */

#define EOL_PROTOCOL_MASTER_CAN_ID    0x157U  /**< 上位机CAN ID */
#define EOL_PROTOCOL_REPLY_CAN_ID     0x257U  /**< 回复上位机CAN ID */

/* 包重复检测 */
#define PACKAGE_REPEAT_CHECK_SIZE     10U     /**< 10包循环检测 */

/* 最小帧长 7Bytes DATA */
#define FRAME_MIN_SIZE                (EOL_FRAME_MIN_SIZE)

#define FRAME_TEMP_BUF_SIZE           (2048U) /**< 临时缓冲区大小 */

#define WAIT_LIST_LEN                 100U
#define FRAME_TIME_OUT                3U      /**< 1s超时检测 */
#define NO_RESPONSE_TIMES             0U      /**< 允许超时无响应次数，发出无响应信号 */

#define RETRY_NUM_MAX                 3U      /**< 失败重试次数，超时重试次数（包含首次发送） */
#define NOT_FULL_TIMEOUT_SEC_MAX      15U     /**< 允许帧不全超时时间 */
#define WAIT_SEND_HW_ERR_TIMES        3U      /**< 硬件发送失败3次 */

/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/

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
  /* 创建访问资源锁1个 */
  sem.release(1);

  /* 创建接收缓冲 */
  cq_obj = new CircularQueue(CircularQueue::UINT8_DATA_BUF, CircularQueue::CQ_BUF_4K, this);
  if(nullptr == cq_obj)
  {
    qDebug() << "eol create cq faild";
  }

  /* 初始化分包发送句柄 */
  send_task_handle.data_total_size = 0;
  send_task_handle.wait_send_size = 0;

  /* 定时器初始化 */
  timer_init();
}

void eol_protocol::set_can_driver_obj(can_driver_model *can_driver_)
{
  if(nullptr == can_driver_)
  {
    return;
  }
  can_driver_obj = can_driver_;
}

void eol_protocol::eol_protocol_clear()
{
  // qDebug() << "---eol_protocol_clear---";
  wait_response_list.clear();
  CircularQueue::CQ_emptyData(cq_obj->CQ_getCQHandle());
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
  while(run_state)
  {
    /* 取任务 */
    if(eol_task_list.isEmpty() == false)
    {
      sem.tryAcquire();
      task_ = eol_task_list.takeFirst();
      sem.release();

     /* 执行任务 */
     (this->*(task_.task))(&task_);
    }

    /* 监听任务 */
    if(true == listen_run_state)
    {
      protocol_stack_wait_reply_start(true);
    }

    /* 非监听下，任务执行完自动退出线程 */
    if(eol_task_list.isEmpty() == true && false == listen_run_state)
    {
      break;
    }

    /* 执行主线程 */
    QThread::usleep(0);
  }
  qDebug() << "eol_task end";
}

bool eol_protocol::eol_send_data_port(const uint8_t *data, uint16_t data_len, \
  EOL_SEND_HW_Typedef_t com_hw, QString &channel_num)
{
  switch(com_hw)
  {
    case EOL_CAN_HW:            /**< can发送数据 */
      {
        /* 发送 */
        bool ret = can_driver_obj->send(data, (quint8)data_len,
                                        EOL_PROTOCOL_MASTER_CAN_ID,
                                        can_driver_model::STD_FRAME_TYPE,
                                        data_len > 8U ?
                                        can_driver_model::CANFD_PROTOCOL_TYPE :
                                        can_driver_model::CAN_PROTOCOL_TYPE,
                                        (quint8)channel_num.toUInt());
        return ret;
      }
      break;

    case EOL_SERIAL_HW:        /**< 串口发送数据 */
      break;

    case EOL_ETH_HW:           /**< 网络发送数据 */
      break;

    case EOL_LIN_HW:
      break;

    default:
      break;
  }

  return false;
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
  bool ret = eol_send_data_port(reinterpret_cast<const quint8 *>(send_task_handle.buf_ptr + send_task_handle.current_send_index), \
                       (quint8)can_send_size, send_task_handle.com_hw, send_task_handle.channel_num);
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

/* 响应超时检测 */
bool eol_protocol::response_is_timeout(WAIT_RESPONSE_LIST_Typedef_t &wait)
{
  if((static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch()) - wait.start_time) >= FRAME_TIME_OUT)
  {
    return true;
  }
  return false;
}

/* 检测缓冲区是否有数据 */
uint32_t eol_protocol::check_can_read(CircularQueue::CQ_handleTypeDef *cq)
{
  /* 判断CAN ID 跳过无效头 */
  quint32 len = CircularQueue::CQ_getLength(cq);
  if(2U > len)
  {
    return len;
  }

  /* 广播帧不关心地址 */
  if(EOL_DEVICE_COM_ADDR == EOL_DEVICE_BROADCAST_COM_ADDR)
  {
    if(CircularQueue::CQ_manualGetOffsetData(cq, 0) != SLAVE_EOL_FRAME_HEADER)
    {
      return CircularQueue::CQ_skipInvaildU8Header(cq, (quint8)SLAVE_EOL_FRAME_HEADER);
    }
    /* 返回缓冲区长度 */
    return CircularQueue::CQ_getLength(cq);
  }
  else
  {
    /* 匹配地址 */
    if(CircularQueue::CQ_manualGetOffsetData(cq, 0) != SLAVE_EOL_FRAME_HEADER || \
        CircularQueue::CQ_manualGetOffsetData(cq, 1) != EOL_DEVICE_COM_ADDR)
    {
      /* 剔除首个 */
      CircularQueue::CQ_manualOffsetInc(cq, 1);
      if(CircularQueue::CQ_skipInvaildU8Header(cq, (quint8)SLAVE_EOL_FRAME_HEADER) == 0)
      {
        return 0;
      }

      /* 二次检测不过直接返回0 */
      len = CircularQueue::CQ_getLength(cq);
      if(2U > len)
      {
        return len;
      }

      if(CircularQueue::CQ_manualGetOffsetData(cq, 0) != SLAVE_EOL_FRAME_HEADER || \
          CircularQueue::CQ_manualGetOffsetData(cq, 1) != EOL_DEVICE_COM_ADDR)
      {
        return 0;
      }
    }
    /* 匹配上 */
    else
    {
      /* 返回缓冲区长度 */
      return CircularQueue::CQ_getLength(cq);
    }
  }
  /* 二次检测匹配上返回缓冲区长度 */
  return CircularQueue::CQ_getLength(cq);
}

/* 定时器初始化 */
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

eol_protocol::RETURN_TYPE_Typedef_t eol_protocol::protocol_stack_create_task( \
      EOL_CMD_Typedef_t command, uint8_t reg_addr,
      const uint8_t *data, uint16_t data_len, \
      EOL_SEND_HW_Typedef_t com_hw, QString channel_num)
{
  static uint8_t send_buf[FRAME_TEMP_BUF_SIZE];
  /* 帧头 */
  send_buf[0] = MASTER_EOL_FRAME_HEADER;
  send_buf[1] = EOL_DEVICE_COM_ADDR;

  if(wait_response_list.size() > (qint32)WAIT_LIST_LEN)
  {
    return RETURN_ERROR;
  }
  eol_protocol::WAIT_RESPONSE_LIST_Typedef_t wait;
  wait.reg_addr = reg_addr;
  wait.start_time = static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch());
  wait.command = (quint8)command;
  wait.channel_num = channel_num;
  wait.com_hw = com_hw;
  wait_response_list.append(wait);

  uint16_t index = 2;

  switch(command)
  {
    /* 写入 */
    case EOL_WRITE_CMD:
    {
      send_buf[index++] = static_cast<uint8_t>(reg_addr << 1 | (quint8)command);
      send_buf[index++] = static_cast<uint8_t>(data_len&0x00FF);
      send_buf[index++] = static_cast<uint8_t>((data_len>>8)&0xFF);
      for(uint16_t data_index = 0; data_index < data_len; data_index++)
      {
        send_buf[index++] = data[data_index];
      }
      uint16_t crc_val = utility::get_modbus_crc16_with_tab(send_buf, index);
      send_buf[index++] = static_cast<uint8_t>((crc_val&0x00FF));
      send_buf[index++] = static_cast<uint8_t>((crc_val>>8)&0xFF);
      break;
    }

    /* 读取 */
    case EOL_READ_CMD:
    {
      send_buf[index++] = static_cast<uint8_t>(reg_addr << 1 | (quint8)command);

      uint16_t crc_val = utility::get_modbus_crc16_with_tab(send_buf, index);
      send_buf[index++] = static_cast<uint8_t>((crc_val&0x00FF));
      send_buf[index++] = static_cast<uint8_t>((crc_val>>8)&0xFF);
      break;
    }

    /* 数据裸露发送 */
    case EOL_META_CMD:
      {
        index = data_len;
        memcpy_s(send_buf, FRAME_TEMP_BUF_SIZE, data, data_len);
        break;
      }

    default:
      return RETURN_ERROR;
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
    send_task_handle.com_hw = com_hw;
    send_task_handle.channel_num = channel_num;
    SNED_CHECK_STATUS_Typedef_t state = check_wait_send_task(false);

    if(EOL_WRITE_CMD == command
       || EOL_READ_CMD == command
       || EOL_META_CMD == command)
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
    RETURN_TYPE_Typedef_t ret = RETURN_ERROR;
    if(EOL_WRITE_CMD == command
       || EOL_READ_CMD == command
       || EOL_META_CMD == command)
    {
      while((ret = protocol_stack_wait_reply_start()) == RETURN_WAITTING && true == run_state);
    }

    /* 移除等待队列 */
    if(false == wait_response_list.isEmpty())
    {
      wait_response_list.removeFirst();
    }

    if((RETURN_OK == ret || RETURN_ACK_ERROR == ret) && (EOL_READ_CMD == command || EOL_META_CMD == command))
    {
      /* 接收一帧 */
      emit signal_send_rec_one_frame(false);
    }
    return ret;
  }
#endif
  // qDebug() << "------direct send-------" << QDateTime::currentMSecsSinceEpoch();

  /* 发送 */
  bool ok = eol_send_data_port(reinterpret_cast<const quint8 *>(send_buf), \
                         (quint8)index, com_hw, channel_num);

  if(EOL_WRITE_CMD == command
     || EOL_READ_CMD == command
     || EOL_META_CMD == command)
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
  RETURN_TYPE_Typedef_t ret = RETURN_ERROR;
  if(EOL_WRITE_CMD == command
     || EOL_READ_CMD == command
     || EOL_META_CMD == command)
  {
    while((ret = protocol_stack_wait_reply_start()) == RETURN_WAITTING && true == run_state);
  }

  /* 移除等待队列 */
  if(false == wait_response_list.isEmpty())
  {
    wait_response_list.removeFirst();
  }

  if((RETURN_OK == ret || RETURN_ACK_ERROR == ret) && (EOL_READ_CMD == command || EOL_META_CMD == command))
  {
    /* 接收一帧 */
    emit signal_send_rec_one_frame(false);
  }
  return ret;
}

/* 从机ack应答 */
eol_protocol::EOL_OPT_STATUS_Typedef_t eol_protocol::decode_ack_frame(quint8 reg_addr, const quint8 *data)
{
  quint8 ack_rsl = data[3];
  QString msg_str;
  if(0 == ack_rsl)
  {
    msg_str = "no error";
  }
  else
  {
    msg_str = "error";
  }

  EOL_OPT_STATUS_Typedef_t msg = (EOL_OPT_STATUS_Typedef_t)data[4];
  switch(reg_addr)
  {
    case EOL_RW_TABLE_DATA_REG:
    default:
      // qDebug() << msg_str << ":" << msg;
      break;
  }

  switch(reg_addr)
  {
    case EOL_W_RUN_MODE_REG:
    {
      EOL_TASK_LIST_Typedef_t task;
      task.param = nullptr;

      /* 读配置 */
      task.reg = EOL_R_PROFILE_NUM_CH_NUM;
      task.command = EOL_READ_CMD;
      task.buf[0] = 0;
      task.len = 0;
      eol_master_common_rw_device(task);
      break;
    }
    case EOL_W_DEVICE_REBOOT_REG:
    case EOL_W_VCAN_TEST_REG:
    case EOL_RW_SN_REG:
    default:
      /* 发送完成信号 */
      emit signal_rw_device_ok(reg_addr, nullptr, 0);
      break;
  }
  return msg;
}

/* 读取数据解析 */
eol_protocol::RETURN_TYPE_Typedef_t eol_protocol::decode_data_frame(quint8 reg_addr, const quint8 *data, quint16 data_len)
{
  switch(reg_addr)
  {
    /* 访问码 */
    case EOL_R_ACCESS_CODE_REG:
      {
        memcpy(&access_code, data, sizeof(access_code));
        qDebug("acc code 0x%08X", access_code);
        EOL_TASK_LIST_Typedef_t task;
        task.param = nullptr;

        /* 设置工作模式 */
        task.reg = EOL_W_RUN_MODE_REG;
        task.command = EOL_WRITE_CMD;
        memcpy(&task.buf, &access_code, sizeof(access_code));
        task.buf[sizeof(access_code)] = (quint8)current_run_mode;
        utility::debug_print(task.buf, 5);
        task.len = 5;
        eol_master_common_rw_device(task);
        break;
      }

    /* 读运行模式，配置数，通道数 */
    case EOL_R_PROFILE_NUM_CH_NUM:
      {
        emit signal_device_mode(data);
        break;
      }

    case EOL_RW_TABLE_DATA_REG:
      {
        quint16 frame_num;
        memcpy(&frame_num, data, 2);

        /* 表信息帧 */
        if(0 == frame_num)
        {
          memset(&data_record, 0, sizeof(data_record));
          emit signal_recv_eol_table_data(frame_num, data + 2, data_len - 2);
        }

        /* 表数据帧 */
        else if(0 < frame_num && 0xFFFF > frame_num)
        {
          /* 是否否和预期帧号，否则属于丢帧 */
          if((data_record.frame_num + 1) == frame_num)
          {
            /* 只记录数据 帧号[0..1] 数据[2..257] 数据长度 */
            data_record.frame_num++;
            data_record.data_size += (data_len - 2);
            emit signal_recv_eol_table_data(frame_num, data + 2, data_len - 2);
          }
          /* 丢帧要求重发 */
          else
          {
            return RETURN_LOST_FRAME;
          }
        }

        /* 结束帧 */
        else
        {
          data_record.frame_num = frame_num;
          emit signal_recv_eol_data_complete();
        }
      }
      break;

    case EOL_RW_VERSION_REG:
    case EOL_RW_SN_REG:
    case EOL_R_MOUNTID_REG:
    case EOL_R_OBJ_LIST_REG:
    case EOL_RW_PROFILE_ID_REG:
    case EOL_R_2DFFT_DATA_REG:
    case EOL_RW_RCS_OFFSET_REG:
    case EOL_RW_CALI_MODE_REG:
    case EOL_R_BG_NOISE_REG:
    case EOL_RW_VOLTAGE_REG:
    case EOL_R_CHIP_SN_REG:
    case EOL_R_PMIC_SN_REG:
    case EOL_R_CLIENT_DID_VER_REG:
    case EOL_RW_RDM_DATA_REG:
    default:
      {
        /* 发送完成信号 */
        emit signal_rw_device_ok(reg_addr, data, data_len);
      }
      break;
  }
  return RETURN_OK;
}

/* 等待回复 */
eol_protocol::RETURN_TYPE_Typedef_t eol_protocol::protocol_stack_wait_reply_start(bool listen_mode)
{
  static quint8 temp_buf[FRAME_TEMP_BUF_SIZE];
  uint16_t data_len = 0;
  quint32 package_len = 0;
  uint32_t len = 0;
  uint8_t reg = 0;
  uint8_t cmd = 0;
  RETURN_TYPE_Typedef_t ret = RETURN_OK;

  /* 非线程模式，事件处理循环 */
//  QCoreApplication::processEvents();

  /* 获取句柄是否为空 */
  if(cq_obj == nullptr)
  {
    return RETURN_ERROR;
  }

  CircularQueue::CQ_handleTypeDef *cq = cq_obj->CQ_getCQHandle();

  /* 检查响应队列 */
  if(true == wait_response_list.isEmpty() && false == listen_mode)
  {
    CircularQueue::CQ_emptyData(cq);
    return RETURN_OK;
  }

  /* 获取等待数据信息 */
  WAIT_RESPONSE_LIST_Typedef_t wait;
  if(false == wait_response_list.isEmpty() && false == listen_mode)
  {
    wait = wait_response_list.first();
  }

  while(run_state)
  {
    /* 检测响应帧超时 */
    if(response_is_timeout(wait) == true && false == listen_mode)
    {
      /* 清空缓冲区 */
      CircularQueue::CQ_emptyData(cq);
      qDebug() << "clear cq buf";
      acc_error_cnt++;
      if(acc_error_cnt > NO_RESPONSE_TIMES)
      {
        qDebug() << "response_is_timeout > " << \
          (NO_RESPONSE_TIMES + 1U) * FRAME_TIME_OUT << "s -> signal_protocol_timeout";
        emit signal_protocol_timeout(acc_error_cnt * FRAME_TIME_OUT);
        acc_error_cnt = 0;
      }
      return RETURN_TIMEOUT;
    }

    /* 元数据报文 */
    uint32_t meta_len = 0;
    if((quint8)EOL_META_CMD == wait.command && false == listen_mode)
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

    len = CircularQueue::CQ_getLength(cq);
    if(EOL_FRAME_MIN_SIZE > len)
    {
      return RETURN_WAITTING;
    }

    /* 判断帧类型 */
    reg = CircularQueue::CQ_manualGetOffsetData(cq, 2U);
    cmd = reg & 0x01U;
    reg >>= 1U;

    switch((EOL_CMD_Typedef_t)cmd)
    {
      /* 主机写入，从机回复报文 */
      case EOL_WRITE_CMD:
      {
        package_len = EOL_FRAME_MIN_SIZE;

        /* 写入应答完成 */
        CircularQueue::CQ_manualGetDataTemp(cq, temp_buf, package_len);

        /* 校验CRC */
        if(utility::get_modbus_crc16_rsl_with_tab(temp_buf, static_cast<uint16_t>(package_len - 2U)) == false)
        {
          qDebug() << "crc err package_len " << package_len;
          utility::debug_print(temp_buf, package_len);
          CircularQueue::CQ_manualOffsetInc(cq, 1U);
          ret = RETURN_CRC_ERROR;
          break;
        }

        /* 移除报文 */
        CircularQueue::CQ_manualOffsetInc(cq, package_len);

        /* 处理ack */
        EOL_OPT_STATUS_Typedef_t ack_ret = decode_ack_frame(reg, temp_buf);
        if(EOL_OPT_OK != ack_ret)
        {
          qDebug() << "signal_protocol_error_occur";
          emit signal_protocol_error_occur((quint8)ack_ret);
          ret = RETURN_ACK_ERROR;
          break;
        }

        acc_error_cnt = 0;
        ret = RETURN_OK;
        break;
      }

      /* 主机读取，从机回复报文 */
      case EOL_READ_CMD:
      {
        data_len = static_cast<uint16_t>(CircularQueue::CQ_manualGetOffsetData(cq, 4U) << 8U);
        data_len += CircularQueue::CQ_manualGetOffsetData(cq, 3U);
        package_len = 7U + data_len;
        package_len = (package_len > FRAME_TEMP_BUF_SIZE) ? FRAME_TEMP_BUF_SIZE : package_len;

        if(len < package_len)
        {
          ret = RETURN_WAITTING;
          break;
        }
        CircularQueue::CQ_manualGetDataTemp(cq, temp_buf, package_len);
        if(utility::get_modbus_crc16_rsl_with_tab(temp_buf, static_cast<uint16_t>(package_len - 2U)) == false)
        {
          qDebug() << "crc err package_len " << package_len << "data len " << data_len;
          utility::debug_print(temp_buf, package_len);

          /* 清空一次 */
          CircularQueue::CQ_manualOffsetInc(cq, len);
          ret = RETURN_CRC_ERROR;
          break;
        }

        /* 处理数据 */
        ret = decode_data_frame(reg, temp_buf + 5U, data_len);

        /* 移除报文 */
        CircularQueue::CQ_manualOffsetInc(cq, package_len);

        acc_error_cnt = 0;
        break;
      }

      default:
        /* TODO */
        CircularQueue::CQ_manualOffsetInc(cq, 1U);
        ret = RETURN_WAITTING;
        break;
    }
    return ret;

__META_CMD_DECDE:
    /* 取出数据 */
    CircularQueue::CQ_manualGetDataTemp(cq, temp_buf, meta_len);
    QString str;
    for(quint8 i = 0; i < meta_len; i++)
    {
      str += QString::asprintf("%02X ", temp_buf[i]);
    }

    switch(wait.reg_addr)
    {
      default:
        CircularQueue::CQ_manualOffsetInc(cq, meta_len);

        acc_error_cnt = 0;
        return RETURN_OK;
    }
    CircularQueue::CQ_manualOffsetInc(cq, 1U);
    return RETURN_ERROR;
  }
  return RETURN_ERROR;
}

/* 获取表数据任务 */
bool eol_protocol::get_eol_table_data_task(void *param_)
{
  /* 暂停其他数据接收显示 */
  quint32 last_canid_mask = 0;
  bool last_canid_mask_en = false;
  can_driver_obj->set_msg_canid_mask(EOL_PROTOCOL_REPLY_CAN_ID + 0x100, true, &last_canid_mask, &last_canid_mask_en);

  qDebug() << "get_eol_table_data_task start";
  EOL_TASK_LIST_Typedef_t *paramx = (EOL_TASK_LIST_Typedef_t *)param_;
  DOA_TABLE_HEADER_Typedef_t *param = (DOA_TABLE_HEADER_Typedef_t *)paramx->param;
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
  data_buf[index++] = (quint8)param->Common_Info.Table_Type;
  /* 发送 */
  do
  {
    qDebug() << "get_eol_table_data_task step 1";
    ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_W_TABLE_SEL_REG, data_buf, index, paramx->com_hw, paramx->channel_num);
    if(RETURN_OK != ret)
    {
      /* ack回复错误，不重发 */
      if(RETURN_ACK_ERROR == ret)
      {
        goto __get_eol_table_data_err;
      }
      
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
    if(RETURN_LOST_FRAME == ret || RETURN_CRC_ERROR == ret)
    {
      data_buf[0] = (quint8)(data_record.frame_num + 1U);
      data_buf[1] = (quint8)((data_record.frame_num + 1U) >> 8U);
      qDebug() << "get_eol_table_data_task step 2 lost" << (data_record.frame_num + 1U);
      ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_W_TABLE_DATA_SEL_REG, data_buf, 2U, paramx->com_hw, paramx->channel_num);
    }
    else
    {
      /* 读数据帧 */
      ret = protocol_stack_create_task(EOL_READ_CMD, EOL_RW_TABLE_DATA_REG, nullptr, 0U, paramx->com_hw, paramx->channel_num);
    }

    if(RETURN_OK != ret)
    {
      /* ack回复错误，不重发 */
      if(RETURN_ACK_ERROR == ret)
      {
        goto __get_eol_table_data_err;
      }

      error_cnt++;
      if(RETRY_NUM_MAX > error_cnt)
      {
        continue;
      }
      goto __get_eol_table_data_err;
    }
    else
    {
      /* 清空错误统计 */
      error_cnt = 0;
    }

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

/* 读写任务 */
bool eol_protocol::common_rw_device_task(void *param_)
{
  /* 暂停其他数据接收显示 */
  quint32 last_canid_mask = 0;
  bool last_canid_mask_en = false;
  can_driver_obj->set_msg_canid_mask(EOL_PROTOCOL_REPLY_CAN_ID + 0x100, true, &last_canid_mask, &last_canid_mask_en);

  eol_protocol::RETURN_TYPE_Typedef_t ret;
  quint8 error_cnt = 0;
  EOL_TASK_LIST_Typedef_t *param = (EOL_TASK_LIST_Typedef_t *)param_;

  /* 清空 */
  eol_protocol_clear();

  /* 设置消息过滤器 */
  can_driver_obj->add_msg_filter(EOL_PROTOCOL_REPLY_CAN_ID, cq_obj, (quint8)param->channel_num.toUShort());

  // qDebug("step1 rw device cmd %u reg 0x%02X len %u", param->command, param->reg, param->len);

  if(nullptr != param->param)
  {
    switch(param->reg)
    {
      case EOL_W_2DFFT_CONDITION_REG:
        {
          /* 记录起始时间 */
          QDateTime dt = QDateTime::currentDateTime();
          qint64 *p_time_ms_s = (qint64 *)param->param;
          *p_time_ms_s = dt.toMSecsSinceEpoch();
        }
        break;
      default:
        break;
    }
  }

  do
  {
    ret = protocol_stack_create_task(param->command, param->reg, param->buf, param->len, param->com_hw, param->channel_num);
    if(RETURN_OK != ret)
    {
      error_cnt++;
      if(RETRY_NUM_MAX > error_cnt)
      {
        continue;
      }
      goto __set_device_err;
    }
    break;
  }while(run_state);

  // qDebug() << "rw_device_task exit ok";

  /* 恢复数据接收显示 */
  can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);

  return true;
__set_device_err:
  /* 恢复数据接收显示 */
  can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);

  qDebug() << "signal_protocol_rw_err";
  emit signal_protocol_rw_err(param->reg, (quint8)param->command);
  return false;
}

/* 发送表数据任务 */
bool eol_protocol::send_eol_table_data_task(void *param_)
{
  /* 暂停其他数据接收显示 */
  quint32 last_canid_mask = 0;
  bool last_canid_mask_en = false;
  can_driver_obj->set_msg_canid_mask(EOL_PROTOCOL_REPLY_CAN_ID + 0x100, true, &last_canid_mask, &last_canid_mask_en);

  qDebug() << "send_eol_table_data_task start";
  quint8 data_buf[FRAME_TEMP_BUF_SIZE];
  quint16 index = 0;
  quint16 frame_num = 0;
  quint8 error_cnt = 0;
  RETURN_TYPE_Typedef_t ret;
  quint8 check_sum = 0;

  EOL_TASK_LIST_Typedef_t *paramx = (EOL_TASK_LIST_Typedef_t *)param_;
  EOL_SEND_DATA_Typedef_t *param = (EOL_SEND_DATA_Typedef_t *)paramx->param;

  /* 清空 */
  eol_protocol_clear();

  /* 帧计数0，发送表头数据 */
  index = 0;
  frame_num = 0;
  data_buf[index++] = frame_num;
  data_buf[index++] = (quint8)(frame_num >> 8);

  quint32 crc = utility::get_crc32_with_tab(param->data, param->head_data.Common_Info.Data_Size);
  if(param->head_data.Common_Info.Crc_Val != crc)
  {
    qDebug() << "err crc expectance " << crc << "?" << param->head_data.Common_Info.Crc_Val;

    /* 恢复数据接收显示 */
    can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);

    emit signal_protocol_crc_check_failed();
    return false;
  }

  param->head_data.Common_Info.Crc_Val = crc;
  check_sum = utility::get_data_sum((const quint8 *)param->head_data.private_header, param->head_data.Common_Info.Header_Size);
  param->head_data.Common_Info.Headr_Check_Sum = check_sum;

  memcpy(&data_buf[index], &param->head_data, sizeof(param->head_data.Common_Info) + param->head_data.Common_Info.Header_Size);
  index += sizeof(param->head_data.Common_Info) + param->head_data.Common_Info.Header_Size;

  qDebug("check pass crc 0x%08X size %u", crc, param->head_data.Common_Info.Data_Size);
  /* 发送表头信息 */
  qDebug() << "step1 send table head ";
  do
  {
    ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_RW_TABLE_DATA_REG, data_buf, index, paramx->com_hw, paramx->channel_num);
    if(RETURN_OK != ret)
    {
      /* ack回复错误，不重发 */
      if(RETURN_ACK_ERROR == ret)
      {
        goto __send_eol_table_data_err;
      }

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
  error_cnt = 0U;
  qDebug() << "step2 send table data ";
  /* 发送表数据，每包256Bytes，不足只发送剩余部分 */
  for(quint32 i = 0U; i < param->head_data.Common_Info.Data_Size;)
  {
    frame_num++;
    index = 0;
    data_buf[index++] = frame_num;
    data_buf[index++] = (quint8)(frame_num >> 8U);

    if((i + 256U) > param->head_data.Common_Info.Data_Size)
    {
      memset(&data_buf[index], 0, 256U);
      memcpy(&data_buf[index], param->data + i, param->head_data.Common_Info.Data_Size - i);
      index += (param->head_data.Common_Info.Data_Size - i);
    }
    else
    {
      memcpy(&data_buf[index], param->data + i, 256U);
      index += 256U;
    }

    /* 发送表数据 */
    do
    {
      ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_RW_TABLE_DATA_REG, data_buf, index, paramx->com_hw, paramx->channel_num);
      if(RETURN_OK != ret)
      {
        /* ack回复错误，不重发 */
        if(RETURN_ACK_ERROR == ret)
        {
          goto __send_eol_table_data_err;
        }

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

    /* 累积已发送数据 */
    i += (index - 2);
    qDebug() << "signal_send_progress";
    emit signal_send_progress(i, param->head_data.Common_Info.Data_Size);
  }

  error_cnt = 0;

  /* 发送结束帧 */
  qDebug() << "step3 send end frame";
  index = 0;
  data_buf[index++] = 0xFF;
  data_buf[index++] = 0xFF;
  do
  {
    ret = protocol_stack_create_task(EOL_WRITE_CMD, EOL_RW_TABLE_DATA_REG, data_buf, index, paramx->com_hw, paramx->channel_num);
    if(RETURN_OK != ret)
    {
      /* ack回复错误，不重发 */
      if(RETURN_ACK_ERROR == ret)
      {
        goto __send_eol_table_data_err;
      }

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

/* 设置设备模式 */
bool eol_protocol::eol_master_set_device_mode(DEVICE_MODE_Typedef_t mode)
{
  /* 当前运行模式 */
  current_run_mode = (quint8)mode;

  EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 读安全码 */
  task.reg = EOL_R_ACCESS_CODE_REG;
  task.command = EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_master_common_rw_device(task);
  return true;
}

/* 添加发送表数据任务 */
bool eol_protocol::eol_master_send_table_data(COMMON_TABLE_HEADER_Typedef_t &table_info, const quint8 *data)
{
  /* 查重 */
  for(qint32 i = 0; i < eol_task_list.size(); i++)
  {
    if(eol_task_list.value(i).task == &eol_protocol::send_eol_table_data_task && \
       send_table_data.head_data.Common_Info.Table_Type == table_info.Common_Info.Table_Type)
    {
      return true;
    }
  }

  /* 设置消息过滤器 */
  can_driver_obj->add_msg_filter(EOL_PROTOCOL_REPLY_CAN_ID, cq_obj, (quint8)channel_num.toUShort());

  /* 公共 + 私有表头 */
  memcpy(&send_table_data.head_data, &table_info, sizeof(table_info.Common_Info) + table_info.Common_Info.Header_Size);
  send_table_data.data = data;

  EOL_TASK_LIST_Typedef_t task;
  task.com_hw = com_hw;
  task.channel_num = channel_num;
  task.param = &send_table_data;
  task.task = &eol_protocol::send_eol_table_data_task;
  sem.tryAcquire();
  eol_task_list.append(task);
  sem.release();
  return true;
}

/* 添加获取表数据任务 */
bool eol_protocol::eol_master_get_table_data(TABLE_Typedef_t table_type)
{
  /* 查重 */
  for(qint32 i = 0; i < eol_task_list.size(); i++)
  {
    if(eol_task_list.value(i).task == &eol_protocol::get_eol_table_data_task && \
       send_table_data.head_data.Common_Info.Table_Type == table_type)
    {
      return true;
    }
  }

  /* 设置消息过滤器 */
  can_driver_obj->add_msg_filter(EOL_PROTOCOL_REPLY_CAN_ID, cq_obj, (quint8)channel_num.toUShort());

  send_table_data.head_data.Common_Info.Table_Type = table_type;

  EOL_TASK_LIST_Typedef_t task;
  task.com_hw = com_hw;
  task.channel_num = channel_num;
  task.param = &send_table_data.head_data;
  task.task = &eol_protocol::get_eol_table_data_task;
  sem.tryAcquire();
  eol_task_list.append(task);
  sem.release();

  /* 数据记录清空 */
  memset(&data_record, 0, sizeof(data_record));
  return true;
}

/* 添加读写任务 */
bool eol_protocol::eol_master_common_rw_device(EOL_TASK_LIST_Typedef_t &task, bool check_repeat)
{
  /* 查重 */
  if(true == check_repeat)
  {
    for(qint32 i = 0; i < eol_task_list.size(); i++)
    {
      if(eol_task_list.value(i).reg == task.reg && eol_task_list.value(i).command == task.command)
      {
        return true;
      }
    }
  }

  /* VCAN通讯，改为V通道设置 */
  if(EOL_W_VCAN_TEST_REG == task.reg)
  {
    task.channel_num = vchannel_num;
  }
  else
  {
    task.channel_num = channel_num;
  }

  task.com_hw = com_hw;
  task.task = &eol_protocol::common_rw_device_task;
  sem.tryAcquire();
  eol_task_list.append(task);
  sem.release();
  return true;
}

/* 获取表类型 */
eol_protocol::TABLE_Typedef_t eol_protocol::get_table_type(quint8 profile_id, TABLE_CLASS_Typedef_t class_type)
{
  quint8 Table_Type = 0;
  switch(class_type)
  {
    case SV_AZIMUTH_TABLE:
      Table_Type = (quint8)PROFILE_0_SV_AZIMUTH_TABLE;
      break;
    case SV_ELEVATION_TABLE:
      Table_Type = (quint8)PROFILE_0_SV_ELEVATION_TABLE;
      break;
    case ANT_BOTH_TABLE:
      Table_Type = (quint8)PROFILE_0_ANT_BOTH_TABLE;
      break;
    case PATTERN_TABLE:
      Table_Type = (quint8)PROFILE_0_PATTERN_TABLE;
      break;
    case SV_ELEVATION_AZI_N45_TABLE:
      Table_Type = (quint8)PROFILE_0_SV_ELEVATION_AZI_N45_TABLE;
      break;
    case SV_ELEVATION_AZI_P45_TABLE:
      Table_Type = (quint8)PROFILE_0_SV_ELEVATION_AZI_P45_TABLE;
      break;
    case BACKGROUND_NOISE_TABLE:
      Table_Type = (quint8)PROFILE_ALL_BACKGROUND_NOISE_TABLE;
      return (TABLE_Typedef_t)(Table_Type);
    default:
      return UNKNOW_TABLE;
  }
  return (TABLE_Typedef_t)(Table_Type + profile_id);
}

eol_protocol::TABLE_CLASS_Typedef_t eol_protocol::get_table_class(TABLE_Typedef_t table_type)
{
  switch(table_type)
  {
    case PROFILE_0_SV_AZIMUTH_TABLE:
    case PROFILE_1_SV_AZIMUTH_TABLE:           /**< 方位导向矢量表@ELE+0deg */
    case PROFILE_2_SV_AZIMUTH_TABLE:           /**< 方位导向矢量表@ELE+0deg */
    case PROFILE_3_SV_AZIMUTH_TABLE:           /**< 方位导向矢量表@ELE+0deg */
      return SV_AZIMUTH_TABLE;
    case PROFILE_0_SV_ELEVATION_TABLE:         /**< 俯仰导向矢量表@AZI+0deg */
    case PROFILE_1_SV_ELEVATION_TABLE:         /**< 俯仰导向矢量表@AZI+0deg */
    case PROFILE_2_SV_ELEVATION_TABLE:         /**< 俯仰导向矢量表@AZI+0deg */
    case PROFILE_3_SV_ELEVATION_TABLE:         /**< 俯仰导向矢量表@AZI+0deg */
      return SV_ELEVATION_TABLE;
    case PROFILE_0_ANT_BOTH_TABLE:             /**< 天线间距坐标与初相信息表，双表合并 */
    case PROFILE_1_ANT_BOTH_TABLE:             /**< 天线间距坐标与初相信息表，双表合并 */
    case PROFILE_2_ANT_BOTH_TABLE:             /**< 天线间距坐标与初相信息表，双表合并 */
    case PROFILE_3_ANT_BOTH_TABLE:             /**< 天线间距坐标与初相信息表，双表合并 */
      return ANT_BOTH_TABLE;
    case PROFILE_0_PATTERN_TABLE:              /**< 方向图表 */
    case PROFILE_1_PATTERN_TABLE:              /**< 方向图表 */
    case PROFILE_2_PATTERN_TABLE:              /**< 方向图表 */
    case PROFILE_3_PATTERN_TABLE:              /**< 方向图表 */
      return PATTERN_TABLE;
    case PROFILE_0_SV_ELEVATION_AZI_N45_TABLE: /**< 俯仰导向矢量表@AZI-45deg */
    case PROFILE_1_SV_ELEVATION_AZI_N45_TABLE: /**< 俯仰导向矢量表@AZI-45deg */
    case PROFILE_2_SV_ELEVATION_AZI_N45_TABLE: /**< 俯仰导向矢量表@AZI-45deg */
    case PROFILE_3_SV_ELEVATION_AZI_N45_TABLE: /**< 俯仰导向矢量表@AZI-45deg */
      return SV_ELEVATION_AZI_N45_TABLE;
    case PROFILE_0_SV_ELEVATION_AZI_P45_TABLE: /**< 俯仰导向矢量表@AZI+45deg */
    case PROFILE_1_SV_ELEVATION_AZI_P45_TABLE: /**< 俯仰导向矢量表@AZI+45deg */
    case PROFILE_2_SV_ELEVATION_AZI_P45_TABLE: /**< 俯仰导向矢量表@AZI+45deg */
    case PROFILE_3_SV_ELEVATION_AZI_P45_TABLE: /**< 俯仰导向矢量表@AZI+45deg */
      return SV_ELEVATION_AZI_P45_TABLE;
    case PROFILE_ALL_BACKGROUND_NOISE_TABLE: /**< 配置下通道底噪表 */
      return BACKGROUND_NOISE_TABLE;
    default:
      return eol_protocol::UNKNOW_CLASS_TABLE;
  }
}
/******************************** End of file *********************************/
