/**
 *  @file updatefw_protocol.cpp
 *
 *  @date 2024年02月22日 09:13:54 星期五
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 *
 *  @brief 更新固件协议.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2024.02.22 09:13 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-02-22 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <QDateTime>
#include <QFileInfo>
/** Private includes ---------------------------------------------------------*/
#include "updatefw_protocol.h"

/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define FRAME_TEMP_BUF_SIZE           (64U)   /**< 临时缓冲区大小 */

#define GET_FRAME_MIN_SIZE            8U      /**< 最小帧长 */

#define WAIT_LIST_LEN                 2U      /**< 等待列表 */
#define FRAME_TIME_OUT                100U    /**< 100ms超时检测 */
#define NO_RESPONSE_TIMES             0U      /**< 允许超时无响应次数，发出无响应信号 */

#define RETRY_NUM_MAX                 retry_num_max//(15U)      /**< 无法发出数据，超时重试次数 */
#define NOT_FULL_TIMEOUT_SEC_MAX      15U     /**< 允许帧不全超时时间 */
#define WAIT_SEND_HW_ERR_TIMES        3U      /**< 硬件发送失败3次 */

#ifndef __HAL_GET_ALIGN
/**
   * @brief 对齐size到align的整数倍
   * @param size 原大小
   * @param align 目标对齐倍数2n
   */
#define __HAL_GET_ALIGN(size, align)        (((size) + (align) - 1U) & (~((align) - 1U)))
#endif

#define UPDATE_FW_TO_BOOT_CMD         0x01U
#define UPDATE_FW_TO_EARSE_CMD        0x0DU
#define UPDATE_FW_TO_GET_LEN_CMD      0x07U
#define UPDATE_FW_TO_SAVE_CMD         0x0CU
#define UPDATE_FW_TO_GET_ALL_LEN_CMD  0x03U
#define UPDATE_FW_TO_GET_CRC_CMD      0x04U
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

updatefw_protocol::updatefw_protocol(QObject *parent)
    : QObject{parent}
{
  /* 线程池 */
  g_thread_pool = QThreadPool::globalInstance();

  /* 创建访问资源锁1个 */
  sem.release(1);

  /* 创建接收缓冲 */
  cq_obj = new CircularQueue(CircularQueue::UINT8_DATA_BUF, CircularQueue::CQ_BUF_128, this);
  if(nullptr == cq_obj)
  {
    qDebug() << "updatefw create cq faild";
  }
}

/**
 * @brief updatefw协议栈线程任务
 */
void updatefw_protocol::run_rts_task()
{
  /* 执行RTS协议解析 */
  qDebug() << "start updatefw protocal stack current thread:" << QThread::currentThreadId();

  /* 检查是否存在任务 */
  UPDATEFW_TASK_LIST_Typedef_t task_;
  while(run_state)
  {
    /* 取任务 */
    if(updatefw_task_list.isEmpty() == false)
    {
      sem.tryAcquire();
      task_ = updatefw_task_list.takeFirst();
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
    if(updatefw_task_list.isEmpty() == true && false == listen_run_state)
    {
      break;
    }

    /* 执行主线程 */
    QThread::usleep(0);
  }
  qDebug() << "updatefw_task end";
}

updatefw_protocol::RETURN_TYPE_Typedef_t updatefw_protocol::protocol_stack_create_task(uint32_t can_id, const uint8_t *data, uint16_t data_len)
{
  if(wait_response_list.size() > (qint32)WAIT_LIST_LEN)
  {
    return RETURN_ERROR;
  }
  updatefw_protocol::WAIT_RESPONSE_LIST_Typedef_t wait;
  wait.start_time = static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch());
  wait.id = can_id;
  memcpy(wait.send_data, data, data_len);
  wait_response_list.append(wait);

  /* 发送 */
  bool ok = send_data_port(can_id, data, data_len, channel_num);

  if(false == ok)
  {
    updatefw_protocol_clear();
    return RETURN_ERROR;
  }

  /* 解析 */
  RETURN_TYPE_Typedef_t ret = RETURN_ERROR;
  while((ret = protocol_stack_wait_reply_start()) == RETURN_WAITTING && true == run_state)
  {
    /* todo nothing */
  }

  /* 移除等待队列 */
  if(false == wait_response_list.isEmpty())
  {
    wait_response_list.removeFirst();
  }

  return ret;
}

/* 等待回复 */
updatefw_protocol::RETURN_TYPE_Typedef_t updatefw_protocol::protocol_stack_wait_reply_start(bool listen_mode)
{
  static quint8 temp_buf[FRAME_TEMP_BUF_SIZE];
  quint32 package_len = 0;
  uint32_t len = 0;

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
      acc_error_cnt++;
      if(acc_error_cnt > NO_RESPONSE_TIMES)
      {
        // qDebug() << "response_is_timeout > " <<
        //     (NO_RESPONSE_TIMES + 1) * FRAME_TIME_OUT << "ms -> signal_protocol_timeout";
        emit signal_protocol_timeout(acc_error_cnt * FRAME_TIME_OUT);
        acc_error_cnt = 0;
      }
      return RETURN_TIMEOUT;
    }

    len = CircularQueue::CQ_getLength(cq);
    if(GET_FRAME_MIN_SIZE > len)
    {
      return RETURN_WAITTING;
    }

    /* 主机写入，从机回复报文 */
    package_len = GET_FRAME_MIN_SIZE;

    /* 写入应答完成 */
    CircularQueue::CQ_manualGetDataTemp(cq, temp_buf, package_len);

    switch(temp_buf[1])
    {
      case UPDATE_FW_TO_BOOT_CMD         :
      case UPDATE_FW_TO_EARSE_CMD        :
      case UPDATE_FW_TO_GET_LEN_CMD      :
      case UPDATE_FW_TO_SAVE_CMD         :
      case UPDATE_FW_TO_GET_ALL_LEN_CMD  :
      case UPDATE_FW_TO_GET_CRC_CMD      :
        {
          /* 清空缓冲区 */
          CircularQueue::CQ_manualOffsetInc(cq, len);

          /* 解析状态 */
          RTS_OPT_STATUS_Typedef_t ret = decode_ack_frame(temp_buf, wait.send_data);
          if(RTS_OPT_OK != ret)
          {
            qDebug() << "signal_protocol_error_occur";
            emit signal_protocol_error_occur((quint8)ret);
            return RETURN_ERROR;
          }
          acc_error_cnt = 0;
          return RETURN_OK;
        }

      default:
        CircularQueue::CQ_manualOffsetInc(cq, 1U);
        break;
    }
  }
  return RETURN_ERROR;
}

void updatefw_protocol::updatefw_protocol_clear()
{
  wait_response_list.clear();
  CircularQueue::CQ_emptyData(cq_obj->CQ_getCQHandle());
}

void updatefw_protocol::set_timeout(quint32 sec)
{
  retry_num_max = (sec * 1000U) / FRAME_TIME_OUT;
}

/* 响应超时检测 */
bool updatefw_protocol::response_is_timeout(WAIT_RESPONSE_LIST_Typedef_t &wait)
{
  if((QDateTime::currentMSecsSinceEpoch() - wait.start_time) >= FRAME_TIME_OUT)
  {
    return true;
  }
  return false;
}

updatefw_protocol::RTS_OPT_STATUS_Typedef_t updatefw_protocol::decode_ack_frame(const quint8 *temp_buf, const quint8 *send_data)
{
  quint8 reply_data[8] = {0};
  memcpy(reply_data, send_data, 8U);
  switch(temp_buf[1])
  {
    case UPDATE_FW_TO_BOOT_CMD         :
      {
        reply_data[0] = 0U;
        memset(&reply_data[3], 0, 5U);
        if(0 != memcmp(reply_data, temp_buf, 8U))
        {
          return RTS_OPT_ERR;
        }
        return RTS_OPT_OK;
      }
      break;

    case UPDATE_FW_TO_EARSE_CMD        :
      {
        reply_data[0] = 0U;
        memset(&reply_data[2], 0, 6U);
        if(0 != memcmp(reply_data, temp_buf, 8U))
        {
          return RTS_OPT_ERR;
        }
        return RTS_OPT_OK;
      }
      break;

    case UPDATE_FW_TO_GET_LEN_CMD      :
      {
        reply_data[0] = 0U;
        if(0 != memcmp(reply_data, temp_buf, 6U))
        {
          return RTS_OPT_ERR;
        }
        return RTS_OPT_OK;
      }
      break;

    case UPDATE_FW_TO_SAVE_CMD         :
      {
        reply_data[0] = 0U;
        memset(&reply_data[2], 0, 6U);
        if(0 != memcmp(reply_data, temp_buf, 8U))
        {
          return RTS_OPT_ERR;
        }
        return RTS_OPT_OK;
      }
      break;

    case UPDATE_FW_TO_GET_ALL_LEN_CMD  :
      {
        reply_data[0] = 0U;
        if(0 != memcmp(reply_data, temp_buf, 5U))
        {
          return RTS_OPT_ERR;
        }
        return RTS_OPT_OK;
      }
      break;

    case UPDATE_FW_TO_GET_CRC_CMD      :
      {
        reply_data[0] = 0U;
        if(0 != memcmp(reply_data, temp_buf, 8U))
        {
          return RTS_OPT_ERR;
        }
        return RTS_OPT_OK;
      }
      break;

    default:
      break;
  }

  return RTS_OPT_OK;
}

updatefw_protocol::RETURN_TYPE_Typedef_t updatefw_protocol::decode_data_frame(const quint8 *temp_buf, quint32 data_len)
{
  char str[64] = {0};
  memcpy(str, temp_buf, data_len >= 64U ? 63 : data_len);

  return RETURN_OK;
}

bool updatefw_protocol::send_data_port(uint32_t can_id, const uint8_t *data, uint16_t data_len,
                                       QString &channel_num)
{
  if(nullptr == can_driver_obj)
  {
    return false;
  }

  /* 发送 */
  bool ret = can_driver_obj->send(data, (quint8)data_len,
                                  can_id,
                                  can_driver_model::STD_FRAME_TYPE,
                                  data_len > 8U ?
                                      can_driver_model::CANFD_PROTOCOL_TYPE :
                                      can_driver_model::CAN_PROTOCOL_TYPE,
                                  (quint8)channel_num.toUInt());
  return ret;
}

bool updatefw_protocol::update_device_app_task(void *param_)
{
  updatefw_protocol::RETURN_TYPE_Typedef_t ret = RETURN_OK;
  quint8 error_cnt = 0;
  UPDATEFW_TASK_LIST_Typedef_t *param = (UPDATEFW_TASK_LIST_Typedef_t *)param_;

  /* 设置消息过滤器 */
  /* 暂停其他数据接收显示 */
  quint32 last_canid_mask = 0;
  bool last_canid_mask_en = false;
  can_driver_obj->set_msg_canid_mask(0x200U, true, &last_canid_mask, &last_canid_mask_en);

  /* 清空 */
  updatefw_protocol_clear();

  /* 设置消息过滤器 */
  can_driver_obj->add_msg_filter(0x200U, cq_obj, (quint8)channel_num.toUShort());


  QFile file(param->fw_file_name);
  quint8 sequence_num = 0;
  quint8 sequence_num_get_len = 0;
  quint8 sequence_num_write_flash = 0;

  quint8 send_data[8] = {0};
  quint32 check_fw_len = 7U * 128U;
  quint32 write_fw_len = 7U * 512U;
  quint32 write_fw_len_cnt = 0;
  quint32 crc_val = 0xFFFFFFFFU;

  qDebug() << "step1 disable device all";
  send_data[0] = (1 << 7U) | sequence_num;
  send_data[1] = UPDATE_FW_TO_BOOT_CMD;   /**< BOOT2_CMD_toBoot_01 */
  send_data[2] = 1;   /**< 2boot 1app 3force */
  /* password */
  send_data[3] = 0x52;
  send_data[4] = 0xAE;
  send_data[5] = 0x87;
  send_data[6] = 0x93;
  send_data[7] = 0xEB;

  do
  {
    ret = protocol_stack_create_task(0x200U, send_data, 8U);
    if(RETURN_OK != ret)
    {
      continue;
    }
    break;
  }while(run_state);

  qDebug() << "step2 erase device flash";
  send_data[1] = UPDATE_FW_TO_EARSE_CMD; /**< BOOT2_CMD_code_erase */
  /* 启动地址 */
  quint32 start_addr = param->start_addr;
  send_data[2] = (start_addr >> 24U) & 0xFFU;
  send_data[3] = (start_addr >> 16U) & 0xFFU;
  send_data[4] = (start_addr >> 8U) & 0xFFU;
  send_data[5] = start_addr & 0xFFU;
  /* 擦除大小KB */
  /* 获取文件信息 */
  QFileInfo info(param->fw_file_name);
  quint32 fw_size_kb = (quint32)__HAL_GET_ALIGN(info.size(), 4096U) / 1024U;
  fw_size_kb = fw_size_kb < 68U ? 68U : fw_size_kb;
  send_data[6] = (fw_size_kb >> 8U) & 0xFFU;
  send_data[7] = fw_size_kb & 0xFFU;

  /* 限制 */
  check_fw_len = check_fw_len > (quint32)info.size() ? (quint32)info.size() : check_fw_len;
  write_fw_len = write_fw_len > (quint32)info.size() ? (quint32)info.size() : write_fw_len;

  do
  {
    set_timeout(10U);
    ret = protocol_stack_create_task(0x200U, send_data, 8U);
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

  /* 清空错误统计 */
  error_cnt = 0;

  /* 再次判断线程运行状态 */
  if(false == run_state)
  {
    goto __set_device_err;
  }

  qDebug() << "step3 send fw data";

  /* 打开文件句柄 */
  if(file.fileName().isEmpty() == true)
  {
    goto __set_device_err;
  }
  if(file.isOpen() == true)
  {
    file.close();
  }
  if(false == file.open(QIODevice::ReadOnly))
  {
    qDebug() << "open file failed";
    goto __set_device_err;
  }
  /* 发送数据，每包7Bytes，不足只发送剩余部分 */
  for(quint32 i = 0U; i < (quint32)info.size();)
  {
    sequence_num = sequence_num > 7U ? 0U : sequence_num;
    send_data[0] = (0 << 7U) | sequence_num;
    if((i + 7U) > (quint32)info.size())
    {
      memset(&send_data[1], 0, 7U);
      file.read((char *)&send_data[1], (quint32)info.size() - i);
      /* 计算crc */
      crc_val = utility::get_crc32_with_tab2_for_upfw(&send_data[1], (quint32)info.size() - i, crc_val, true);
      send_data[0] = (0 << 7U) | (sequence_num << 4U) | (((quint32)info.size() - i) << 1U) | 1U;
      write_fw_len_cnt += ((quint32)info.size() - i);
      i += ((quint32)info.size() - i);
    }
    else
    {
      file.read((char *)&send_data[1], 7U);
      /* 计算crc */
      crc_val = utility::get_crc32_with_tab2_for_upfw(&send_data[1], 7U, crc_val);
      send_data[0] = (0 << 7U) | (sequence_num << 4U) | (7U << 1U) | 1U;
      write_fw_len_cnt += 7U;
      i += 7U;
    }

    /* 翻转发送字节序 */
    // utility::bytes_invert(&send_data[1], 7U);

    /* 发送数据 */
    do
    {
      /* 直接发送 */
      if(true != send_data_port(0x200U, send_data, 8U, channel_num))
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

    /* 再次判断线程运行状态 */
    if(false == run_state)
    {
      goto __set_device_err;
    }

    /* 清空错误统计 */
    error_cnt = 0;

    /* 已发数据长度检验 */
    if(i >= check_fw_len)
    {
      qDebug() << "step3.1 check fw data len";
      sequence_num_get_len = sequence_num_get_len > 0x7FU ? 0U : sequence_num_get_len;
      send_data[0] = (1 << 7U) | sequence_num_get_len;
      send_data[1] = UPDATE_FW_TO_GET_LEN_CMD;/**< BOOT2_CMD_getLen_07 */
      send_data[2] = (check_fw_len >> 24U) & 0xFFU;
      send_data[3] = (check_fw_len >> 16U) & 0xFFU;
      send_data[4] = (check_fw_len >> 8U) & 0xFFU;
      send_data[5] = check_fw_len & 0xFFU;
      send_data[6] = 0x00;
      send_data[7] = 0x00;
      do
      {
        set_timeout(1U);
        ret = protocol_stack_create_task(0x200U, send_data, 8U);
        if(RETURN_OK != ret)
        {
          /* ack回复错误，不重发 */
          if(RETURN_ERROR == ret)
          {
            goto __set_device_err;
          }

          error_cnt++;
          if(RETRY_NUM_MAX > error_cnt)
          {
            continue;
          }
          goto __set_device_err;
        }
        break;
      }while(run_state);

      /* 再次判断线程运行状态 */
      if(false == run_state)
      {
        goto __set_device_err;
      }
      sequence_num_get_len++;
      check_fw_len += (7U * 128U);
      check_fw_len = check_fw_len > (quint32)info.size() ? (quint32)info.size() : check_fw_len;
    }

    /* 清空错误统计 */
    error_cnt = 0;

    /* 已发数据写入 */
    if(i >= write_fw_len)
    {
      qDebug() << "step3.2 write fw data len";
      sequence_num_write_flash = sequence_num_write_flash > 0x7FU ? 0U : sequence_num_write_flash;
      send_data[0] = (1 << 7U) | sequence_num_write_flash;
      send_data[1] = UPDATE_FW_TO_SAVE_CMD;/**< BOOT2_CMD_code_pro_flash */
      quint32 write_addr = start_addr + (write_fw_len - write_fw_len_cnt);
      send_data[2] = (write_addr >> 24U) & 0xFFU;
      send_data[3] = (write_addr >> 16U) & 0xFFU;
      send_data[4] = (write_addr >> 8U) & 0xFFU;
      send_data[5] = write_addr & 0xFFU;
      send_data[6] = (write_fw_len_cnt >> 8U) & 0xFFU;
      send_data[7] = write_fw_len_cnt & 0xFFU;
      do
      {
        set_timeout(1U);
        ret = protocol_stack_create_task(0x200U, send_data, 8U);
        if(RETURN_OK != ret)
        {
          /* ack回复错误，不重发 */
          if(RETURN_ERROR == ret)
          {
            goto __set_device_err;
          }

          error_cnt++;
          if(RETRY_NUM_MAX > error_cnt)
          {
            continue;
          }
          goto __set_device_err;
        }
        break;
      }while(run_state);

      /* 再次判断线程运行状态 */
      if(false == run_state)
      {
        goto __set_device_err;
      }
      sequence_num_write_flash++;
      write_fw_len_cnt = 0;
      write_fw_len += (7U * 512U);
      write_fw_len = write_fw_len > (quint32)info.size() ? (quint32)info.size() : write_fw_len;
    }

    /* 清空错误统计 */
    error_cnt = 0;
    sequence_num++;
    emit signal_send_progress(i - 1U, (quint32)info.size());
  }

  /* 再次判断线程运行状态 */
  if(false == run_state)
  {
    goto __set_device_err;
  }

  /* 恢复数据接收显示 */
  can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);

  qDebug() << "step4 check fw all data len";
  send_data[0] = (1 << 7U) | sequence_num;
  send_data[1] = UPDATE_FW_TO_GET_ALL_LEN_CMD;/**< BOOT2_CMD_len_03 */
  // send_data[2] = (write_fw_len >> 24U) & 0xFFU;
  send_data[2] = (write_fw_len >> 16U) & 0xFFU;
  send_data[3] = (write_fw_len >> 8U) & 0xFFU;
  send_data[4] = write_fw_len & 0xFFU;
  send_data[5] = 0x00;
  send_data[6] = 0x00;
  send_data[7] = 0x00;
  do
  {
    set_timeout(1U);
    ret = protocol_stack_create_task(0x200U, send_data, 8U);
    if(RETURN_OK != ret)
    {
      /* ack回复错误，不重发 */
      if(RETURN_ERROR == ret)
      {
        goto __set_device_err;
      }

      error_cnt++;
      if(RETRY_NUM_MAX > error_cnt)
      {
        continue;
      }
      goto __set_device_err;
    }
    break;
  }while(run_state);

  /* 清空错误统计 */
  error_cnt = 0;

  /* 再次判断线程运行状态 */
  if(false == run_state)
  {
    goto __set_device_err;
  }

  qDebug() << "step5 check fw data crc";
  send_data[0] = (1 << 7U) | sequence_num;
  send_data[1] = UPDATE_FW_TO_GET_CRC_CMD;/**< BOOT2_CMD_crc_04 */
  send_data[2] = (crc_val >> 24U) & 0xFFU;
  send_data[3] = (crc_val >> 16U) & 0xFFU;
  send_data[4] = (crc_val >> 8U) & 0xFFU;
  send_data[5] = crc_val & 0xFFU;
  send_data[6] = 0x00;
  send_data[7] = 0x00;
  do
  {
    set_timeout(20U);
    ret = protocol_stack_create_task(0x200U, send_data, 8U);
    if(RETURN_OK != ret)
    {
      /* ack回复错误，不重发 */
      if(RETURN_ERROR == ret)
      {
        goto __set_device_err;
      }

      error_cnt++;
      if(RETRY_NUM_MAX > error_cnt)
      {
        continue;
      }
      goto __set_device_err;
    }
    break;
  }while(run_state);

  /* 再次判断线程运行状态 */
  if(false == run_state)
  {
    goto __set_device_err;
  }

  qDebug() << "step6 wait update fw ok";

  if(file.isOpen() == true)
  {
    file.close();
  }

  emit signal_send_progress((quint32)info.size(), (quint32)info.size());
  return true;
__set_device_err:
  /* 恢复数据接收显示 */
  can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);

  if(file.isOpen() == true)
  {
    file.close();
  }

  emit signal_protocol_rw_err(param->fw_file_name);
  return false;
}
/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

bool updatefw_protocol::updatefw_device_app(UPDATEFW_TASK_LIST_Typedef_t &task, bool check_repeat)
{
  /* 查重 */
  if(true == check_repeat)
  {
    for(qint32 i = 0; i < updatefw_task_list.size(); i++)
    {
      if(updatefw_task_list.value(i).fw_file_name == task.fw_file_name)
      {
        return true;
      }
    }
  }

  task.param = nullptr;
  task.task = &updatefw_protocol::update_device_app_task;
  sem.tryAcquire();
  updatefw_task_list.append(task);
  sem.release();
  return true;
}
/******************************** End of file *********************************/
