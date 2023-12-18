/**
 *  @file rts_protocol.cpp
 *
 *  @date 2023年12月01日 15:13:54 星期五
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.12.01 15:13 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-12-01 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <QDateTime>
/** Private includes ---------------------------------------------------------*/
#include "rts_protocol.h"

/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define FRAME_TEMP_BUF_SIZE           (64U)   /**< 临时缓冲区大小 */

#define RTS_FRAME_MIN_SIZE            1U      /**< 最小帧长 */

#define WAIT_LIST_LEN                 2U      /**< 等待列表 */
#define FRAME_TIME_OUT                3U      /**< 1s超时检测 */
#define NO_RESPONSE_TIMES             0U      /**< 允许超时无响应次数，发出无响应信号 */

#define RETRY_NUM_MAX                 3U      /**< 无法发出数据，超时重试次数 */
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

rts_protocol::rts_protocol(QObject *parent)
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
    qDebug() << "rts create cq faild";
  }

  /* 启动计时器 */
  timer_obj.start();
}

/**
 * @brief rts协议栈线程任务
 */
void rts_protocol::run_rts_task()
{
  /* 执行RTS协议解析 */
  qDebug() << "start RTS protocal stack current thread:" << QThread::currentThreadId();

  /* 检查是否存在任务 */
  RTS_TASK_LIST_Typedef_t task_;
  while(run_state)
  {
    /* 取任务 */
    if(rts_task_list.isEmpty() == false)
    {
      sem.tryAcquire();
      task_ = rts_task_list.takeFirst();
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
    if(rts_task_list.isEmpty() == true && false == listen_run_state)
    {
      break;
    }

    /* 执行主线程 */
    QThread::usleep(0);
  }
  sem.tryAcquire();
  rts_task_list.clear();
  sem.release();
  qDebug() << "rts_task end";
}

rts_protocol::RETURN_TYPE_Typedef_t rts_protocol::protocol_stack_create_task(const QString &cmd)
{
  if(wait_response_list.size() > (qint32)WAIT_LIST_LEN)
  {
    return RETURN_ERROR;
  }
  rts_protocol::WAIT_RESPONSE_LIST_Typedef_t wait;
  wait.start_time = static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch());
  wait.cmd = cmd;
  wait_response_list.append(wait);

  /* 发送 */
  bool ok = rts_send_data_port(reinterpret_cast<const quint8 *>(cmd.toUtf8().data()), \
                               cmd.toUtf8().size(), peer_addr);

  if(false == ok)
  {
    qDebug() << "send error";
    rts_protocol_clear();
    return RETURN_ERROR;
  }

  /* 解析 */
  RETURN_TYPE_Typedef_t ret = RETURN_ERROR;
  while((ret = protocol_stack_wait_reply_start()) == RETURN_WAITTING && true == run_state)
  {
    /* todo nothing */
  }
  return ret;
}

/* 等待回复 */
rts_protocol::RETURN_TYPE_Typedef_t rts_protocol::protocol_stack_wait_reply_start(bool listen_mode)
{
  static quint8 temp_buf[FRAME_TEMP_BUF_SIZE];
  uint16_t data_len = 0;
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
      /* 移除等待队列-清空缓冲区 */
      wait_response_list.removeFirst();
      CircularQueue::CQ_emptyData(cq);
      qDebug() << "clear wait one";
      acc_error_cnt++;
      if(acc_error_cnt > NO_RESPONSE_TIMES)
      {
        qDebug() << "response_is_timeout > " <<
            (NO_RESPONSE_TIMES + 1) * FRAME_TIME_OUT << "s -> signal_protocol_timeout";
        emit signal_protocol_timeout(acc_error_cnt * FRAME_TIME_OUT);
        acc_error_cnt = 0;
      }
      return RETURN_TIMEOUT;
    }

    len = CircularQueue::CQ_getLength(cq);
    if(RTS_FRAME_MIN_SIZE > len)
    {
      return RETURN_WAITTING;
    }

    /* 判断请求类型 */
    if(wait.cmd != RTS_GET_STATUS)
    /* 主机写入，从机回复报文 */
    {
      package_len = RTS_FRAME_MIN_SIZE;

      /* 写入应答完成 */
      CircularQueue::CQ_manualGetDataTemp(cq, temp_buf, package_len);


      /* 移除等待队列 */
      if(false == wait_response_list.isEmpty() && false == listen_mode)
      {
        wait_response_list.removeFirst();
      }

      /* 清空缓冲区 */
      CircularQueue::CQ_manualOffsetInc(cq, len);

      /* 解析状态 */
      RTS_OPT_STATUS_Typedef_t ret = decode_ack_frame(temp_buf);
      if(RTS_OPT_OK != ret)
      {
        qDebug() << "signal_protocol_error_occur";
        emit signal_protocol_error_occur((quint8)ret);
        return RETURN_ERROR;
      }

      acc_error_cnt = 0;
      return RETURN_OK;
    }
    /* 主机读取，从机回复报文 */
    else
    {
      package_len = 5U;

      if(len < package_len)
      {
        return RETURN_WAITTING;
      }
      CircularQueue::CQ_manualGetDataTemp(cq, temp_buf, len);

      /* 处理数据 */
      RETURN_TYPE_Typedef_t ret = decode_data_frame(temp_buf, data_len);

      /* 移除等待队列 */
      if(false == wait_response_list.isEmpty() && false == listen_mode)
      {
        wait_response_list.removeFirst();
      }

      CircularQueue::CQ_manualOffsetInc(cq, len);
      acc_error_cnt = 0;
      return ret;
    }
  }
  return RETURN_ERROR;
}

void rts_protocol::rts_protocol_clear()
{
  // qDebug() << "---rts_protocol_clear---";
  wait_response_list.clear();
  CircularQueue::CQ_emptyData(cq_obj->CQ_getCQHandle());
}

/* 响应超时检测 */
bool rts_protocol::response_is_timeout(WAIT_RESPONSE_LIST_Typedef_t &wait)
{
  if((QDateTime::currentSecsSinceEpoch() - wait.start_time) >= FRAME_TIME_OUT)
  {
    return true;
  }
  return false;
}

rts_protocol::RTS_OPT_STATUS_Typedef_t rts_protocol::decode_ack_frame(const quint8 *temp_buf)
{
  if(temp_buf[0] != '0')
  {
    return RTS_OPT_ERR;
  }
  return RTS_OPT_OK;
}

rts_protocol::RETURN_TYPE_Typedef_t rts_protocol::decode_data_frame(const quint8 *temp_buf, quint32 data_len)
{
  char str[64] = {0};
  memcpy(str, temp_buf, data_len >= 64U ? 63 : data_len);

  /* A(空格)B(空格)C B为VRTS连接状态，1为连接其他字符为未连接 当B不为1且C不为1时，则为VRTS报错 */
  QString status = QString::asprintf("%s", str);
  QStringList rts_status_list = status.split(" ");
  if(3 > rts_status_list.size())
  {
    return RETURN_ERROR;
  }
  /* 检查状态 */
  if(1 != rts_status_list.value(1).toUInt() && 1 != rts_status_list.value(2).toUInt())
  {
    return RETURN_ERROR;
  }
  return RETURN_OK;
}

bool rts_protocol::rts_send_data_port(const uint8_t *data, uint16_t data_len,
                                      QString &peer_addr)
{
  if(nullptr == network_device_obj)
  {
    return false;
  }

  /* 地址查询 */
  QStringList info = peer_addr.split(":");
  if(2 > info.size())
  {
    return false;
  }
  return network_device_obj->network_send_data(data, data_len, info.value(0),
                                               info.value(1),
                                               network_driver_model::NETWORK_CLIENT_ROLE,
                                               network_driver_model::NETWORK_UDP_TYPE);
}

bool rts_protocol::common_rw_device_task(void *param_)
{
  rts_protocol::RETURN_TYPE_Typedef_t ret;
  quint8 error_cnt = 0;
  RTS_TASK_LIST_Typedef_t *param = (RTS_TASK_LIST_Typedef_t *)param_;

  /* 清空 */
  rts_protocol_clear();

  /* 设置消息过滤器 */
  QStringList info = peer_addr.split(":");
  if(2 > info.size())
  {
    return false;
  }
  QString last_mask = network_rec_device_obj->set_rec_ip_mask(info.value(0));
  network_rec_device_obj->network_register_rec_msg(info.value(0), cq_obj);
  qDebug() << "step1 rw device cmd" << param->cmd;

  do
  {
    ret = protocol_stack_create_task(param->cmd);
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

  qDebug() << "rw_device_task exit ok";

  /* 恢复数据接收显示 */
  network_rec_device_obj->set_rec_ip_mask(last_mask);

  return true;
__set_device_err:
  /* 恢复数据接收显示 */
  network_rec_device_obj->set_rec_ip_mask(last_mask);

  qDebug() << "signal_protocol_rw_err";
  emit signal_protocol_rw_err(param->cmd);
  return false;
}
/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

/* 添加读写任务 */
bool rts_protocol::rts_master_common_rw_device(RTS_TASK_LIST_Typedef_t &task, bool check_repeat)
{
  /* 查重 */
  if(true == check_repeat)
  {
    for(qint32 i = 0; i < rts_task_list.size(); i++)
    {
      if(rts_task_list.value(i).cmd == task.cmd)
      {
        return true;
      }
    }
  }

  task.param = nullptr;
  task.task = &rts_protocol::common_rw_device_task;
  sem.tryAcquire();
  rts_task_list.append(task);
  sem.release();
  return true;
}
/******************************** End of file *********************************/
