/**
 *  @file can_driver_model.cpp
 *
 *  @date 2023年10月24日 15:17:54 星期二
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief can驱动接口.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.10.24 15:17 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-10-24 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
/** Private includes ---------------------------------------------------------*/
#include "can_driver_model.h"
#include <QDateTime>
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/

/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
CircularQueue *can_driver_model::cq_obj = nullptr;
bool can_driver_model::device_opened_;
can_driver_model::CAN_BRAND_Typedef_t can_driver_model::brand_ = ZLG_CAN_BRAND;/**< 品牌 */
quint8 can_driver_model::device_index_ = 0;     /**< 设备索引 */
qint16 can_driver_model::device_type_index_ = 0;/**< 设备名索引 */
bool can_driver_model::start_ = false;
quint32 can_driver_model::channel_index_ = 0;
quint32 can_driver_model::work_mode_index_ = 0;
quint32 can_driver_model::filter_mode_ = 0;
quint32 can_driver_model::net_mode_index_;
QString can_driver_model::acc_code_ = "00000000";
QString can_driver_model::acc_mask_ = "FFFFFFFF";
QString can_driver_model::service_ip_str;
QString can_driver_model::service_port_str;
QString can_driver_model::local_port_str;
QString can_driver_model::custom_baudrate_;
bool can_driver_model::custom_baud_enable_ = false;
bool can_driver_model::send_queue_mode = false;
quint8 can_driver_model::send_type_index_ = 0;
bool can_driver_model::resistance_enable_ = false;
quint32 can_driver_model::baud_index_ = 0;
quint32 can_driver_model::abit_baud_index_ = 0;
quint32 can_driver_model::dbit_baud_index_ = 0;
quint32 can_driver_model::auto_send_index_ = 0;
quint32 can_driver_model::auto_send_period_ = 1000;//ms
QString can_driver_model::id_;
QString can_driver_model::datas_;
quint32 can_driver_model::frame_type_index_ = 0;
quint32 can_driver_model::protocol_index_ = 0;
quint32 can_driver_model::canfd_exp_index_ = 0;

void *can_driver_model::device_handle_ = nullptr;
void *can_driver_model::channel_handle_ = nullptr;
quint32 can_driver_model::frm_delay_time_;//队列帧延时时间ms
bool can_driver_model::frm_delay_flag_ = false;//队列帧延时标记
bool can_driver_model::support_delay_send_ = false;            //设备是否支持队列发送
bool can_driver_model::support_delay_send_mode_ = false;       //设备队列发送是否需要设置队列发送模式,USBCANFD系列，PCIECANFD系列设备需要设置发送模式才可以进行队列发送
bool can_driver_model::support_get_send_mode_ = false;         //设备是否支持查询当前模式
bool can_driver_model::thread_run_state = false;
quint32 can_driver_model::can_id_mask_ = 0xFFFF;
bool can_driver_model::can_id_mask_en_ = false;
QList<can_driver_model::CHANNEL_STATE_Typedef_t>can_driver_model::channel_state_list;

/** Private function prototypes ----------------------------------------------*/

/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

can_driver_model::can_driver_model(QObject *parent)
    : QObject{parent}
{
  /* 创建访问资源锁3个 */
  cq_sem.release(1);
  tx_sem.release(1);
  tx_msg_sem.release(1);
  cq_obj = new CircularQueue(CircularQueue::UINT8_DATA_BUF, CircularQueue::CQ_BUF_1M, this);
  if(nullptr == cq_obj)
  {
    qDebug() << "create cq failed";
  }
}

void can_driver_model::show_message(const QString &str, quint32 channel_num, \
                                  CAN_DIRECT_Typedef_t direct, const quint8 *data, quint32 data_len, quint32 can_id, bool thread_mode)
{
  /* 输出到显示框 */
  if(false == thread_mode)
  {
    emit signal_show_message(str, channel_num, (quint8)direct, data, data_len, can_id);
  }
  else
  {
    emit signal_show_thread_message(str, channel_num, (quint8)direct, data, data_len, can_id);
    return;
  }
}

void can_driver_model::show_message_bytes(quint8 bytes, quint32 channel_num, CAN_DIRECT_Typedef_t direct)
{
  emit signal_show_message_bytes(bytes, channel_num, (quint8)direct);
}

void can_driver_model::send(const CHANNEL_STATE_Typedef_t &channel_state)
{
  if(datas_.isEmpty())
  {
    show_message(tr("data is empty"), channel_state.channel_num);
    return;
  }

  /* 需要发送的帧数 */
  quint32 nSendCount = 1;

  quint32 pack_len = 0;
  switch(protocol_index_)
  {
    /* can */
    case CAN_PROTOCOL_TYPE:
      {
        pack_len = 8U;
        break;
      }

    /* canfd */
    case CANFD_PROTOCOL_TYPE:
      {
        pack_len = 64U;
        break;
      }

    default:
      return;
  }

  bool ok;
  quint32 id = id_.toUInt(&ok, 16);
  QStringList data_list = datas_.split(' ');
  quint32 data_size = (quint32)data_list.length();

  /* 计算分包数 */
  nSendCount = (data_size + pack_len - 1U) / pack_len;

  quint8 send_size = 0;
  if(nSendCount > 0)
  {
    for(quint32 i = 0; i < nSendCount; ++i)
    {
      quint8 data_buf[64] = {0};
      if((i * pack_len + pack_len) > data_size)
      {
        send_size = (quint8)(data_size - i * pack_len);
      }
      else
      {
        send_size = (quint8)pack_len;
      }
      /* 拷贝数据 */
      for(quint8 index = 0; index < send_size; index++)
      {
        data_buf[index] = (quint8)data_list[index + i * pack_len].toUShort(&ok, 16);
      }
      send(channel_state, data_buf, send_size, id, (FRAME_TYPE_Typedef_t)frame_type_index_, (PROTOCOL_TYPE_Typedef_t)protocol_index_);
    }
  }
}

/**
   * @brief send_data 发送数据（查询待发列表）
   * @return true 成功
   */
bool can_driver_model::send_data()
{
  // QReadLocker locker(&tx_msg_rw_lock);

  /* 检查是否有发送任务 */
  if(true == send_msg_list.isEmpty())
  {
    return false;
  }
  tx_msg_sem.acquire();
  SEND_MSG_Typedef_t msg = send_msg_list.dequeue();
  tx_msg_sem.release();
  qint32 channel_index = msg.channel_num;
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en && channel_index == channel_state.channel_num)
    {
      datas_ = msg.data;
      frame_type_index_ = (quint32)msg.frame_type;
      protocol_index_ = (quint32)msg.protocol;
      id_ = QString::number(msg.id, 16);
      tx_sem.acquire();
      send(channel_state);
      tx_sem.release();
      return true;
    }
  }
  return false;
}

/**
   * @brief receive_data 接收数据
   * @return true 成功
   */
bool can_driver_model::receive_data()
{
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en)
    {
      receive_data(channel_state);
    }
  }
  return true;
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

bool can_driver_model::send(const quint8 *data, quint8 size, quint32 id, FRAME_TYPE_Typedef_t frame_type, PROTOCOL_TYPE_Typedef_t protocol, quint8 channel_num)
{
  if(NULL == data)
  {
    return false;
  }

  bool ret = false;
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if((channel_num == channel_state.channel_num || 0xFFU == channel_num) \
        && true == channel_state.channel_en)
    {
      tx_sem.acquire();
      ret = send(channel_state, data, size, id, frame_type, protocol);
      tx_sem.release();
      break;
    }
  }

  return ret;
}

/**
 * @brief 设置通讯通道号
 *
 * @param index
 */
void can_driver_model::set_channel_index(quint8 index)
{
  /* 判断通道号是否全部使能 */
  CHANNEL_STATE_Typedef_t channel_state;
  if((quint8)channel_state_list.size() <= index)
  {
    for(qint32 i = 0; i < channel_state_list.size(); i++)
    {
      channel_state = channel_state_list.takeAt(i);
      channel_state.channel_en = true;
      channel_state_list.insert(i, channel_state);
    }
  }
  /* 启动特定的通道，其他关闭 */
  else
  {
    for(qint32 i = 0; i < channel_state_list.size(); i++)
    {
      if(index == channel_state_list.value(i).channel_num)
      {
        channel_state = channel_state_list.takeAt(i);
        channel_state.channel_en = true;
        channel_state_list.insert(i, channel_state);
      }
      else
      {
        channel_state = channel_state_list.takeAt(i);
        channel_state.channel_en = false;
        channel_state_list.insert(i, channel_state);
      }
    }
  }
}

void can_driver_model::delay_send_can_use_update(bool delay_send, bool send_queue_mode, bool get_send_mode)
{
  /* 队列帧发送延时是否可选 */
  emit signal_send_queue_delay_can_use(delay_send && send_queue_mode);

  /* 获取队列可用空间按钮是否可用 */
  emit signal_get_tx_available_can_use(delay_send);

  /* 清除队列按钮是否可用 */
  emit signal_clear_tx_queue_can_use(delay_send);

  /* 队列帧延时标记是否可选 */
  emit signal_queue_delay_flag_can_use(delay_send);

  /* 获取发送模式按钮是否可用 */
  emit signal_get_sen_mode_can_use(delay_send && get_send_mode);
}

void can_driver_model::auto_send_can_use_update(bool support_can, bool support_canfd, bool support_index, bool support_single_cancel, bool support_get_autosend_list)
{
  bool support_autosend = support_can || support_canfd;
  /* 设置自动发送设备索引是否可用 */
  emit signal_auto_send_dev_index_can_use(support_index);

  /* 设置自动发送周期是否可用 */
  emit signal_auto_send_period_can_use(support_autosend);

  /* 添加自动发送按钮是否可用 */
  emit signal_auto_send_add_can_use(support_autosend);

  /* 自动发送启动是否可用 */
  emit signal_auto_send_start_can_use(support_autosend);

  /* 自动发送停止是否可用 */
  emit signal_auto_send_stop_can_use(support_autosend);

  /* 取消单条自动发送按钮是否可用 */
  emit signal_auto_send_cancel_once_can_use(support_single_cancel);

  /* 获取设备自动发送列表 */
  emit signal_get_dev_auto_send_list_can_use(support_get_autosend_list);
}

void can_driver_model::add_msg_filter(quint32 can_id, CircularQueue *cq_obj_, quint8 channel_)
{
  if(nullptr == cq_obj_)
  {
    return;
  }

  MSG_FILTER_Typedef_t msg_filter;
  msg_filter.can_id = can_id;
  msg_filter.cq_obj = cq_obj_;
  msg_filter.channel = channel_;

  /* 查到id重复的，则替换 */
  for(qint32 i = 0; i < msg_filter_list.size(); i++)
  {
    if(can_id == msg_filter_list.value(i).can_id || msg_filter_list.value(i).cq_obj == cq_obj_)
    {
      msg_filter_list.replace(i, msg_filter);
      return;
    }
  }

  msg_filter_list.append(msg_filter);
}

void can_driver_model::remove_msg_filter(quint32 can_id)
{
  for(qint32 i = 0; i < msg_filter_list.size(); i++)
  {
    if(can_id == msg_filter_list.value(i).can_id)
    {
      msg_filter_list.removeAt(i);
      return;
    }
  }
}

void can_driver_model::msg_to_cq_buf(quint32 can_id, quint8 channel_num, const quint8 *data, quint32 data_len)
{
  if(msg_filter_list.isEmpty())
  {
    return;
  }
  for(qint32 index = 0; index < msg_filter_list.size(); index++)
  {
    if(msg_filter_list.value(index).can_id != can_id && msg_filter_list.value(index).can_id != 0xFFU)
    {
      continue;
    }
    if(msg_filter_list.value(index).channel != 0xFFU && msg_filter_list.value(index).channel != channel_num)
    {
      break;
    }
    CircularQueue::CQ_putData(msg_filter_list.value(index).cq_obj->CQ_getCQHandle(), data, data_len);
  }
}

void can_driver_model::msg_to_ui_cq_buf(quint32 can_id, quint8 channel_num, CAN_DIRECT_Typedef_t direction, \
                                      PROTOCOL_TYPE_Typedef_t protocol, FRAME_TYPE_Typedef_t frame_type, \
                                      FRAME_DATA_TYPE_Typedef_t frame_data_type, \
                                      const quint8 *data, quint8 data_len)
{
  quint32 id_temp;
  id_temp = (can_id & can_id_mask_);

  /* 掩码只针对接收 */
  if(CAN_RX_DIRECT == direction)
  {
    if(nullptr == cq_obj || (can_id != id_temp && false != can_id_mask_en_))
    {
      return;
    }
  }

  CAN_MSG_DISPLAY_Typedef_t ui_msg;
  ui_msg.can_id = can_id;
  ui_msg.can_protocol = protocol;
  ui_msg.frame_type = frame_type;
  ui_msg.frame_data_type = frame_data_type;
  ui_msg.direction = direction;
  ui_msg.channel_num = channel_num;
  ui_msg.data_len = data_len;
  memcpy(ui_msg.msg_data, data, data_len);

  cq_sem.acquire();
  if(true == CircularQueue::CQ_canSaveLength(cq_obj->CQ_getCQHandle(), sizeof(ui_msg)))
  {
    CircularQueue::CQ_putData(cq_obj->CQ_getCQHandle(), (quint8 *)&ui_msg, sizeof(ui_msg));
  }
  cq_sem.release();

  /* 其他报文不统计 */
  if(UNKNOW_DIRECT == direction)
  {
    return;
  }

  QDateTime dt = QDateTime::currentDateTime();
  emit signal_can_driver_msg(can_id, data, data_len, (quint8)direction, channel_num, (quint8)protocol, dt.toMSecsSinceEpoch());
}

void can_driver_model::send(quint8 channel_index)
{
  if(datas_.isEmpty())
  {
    show_message(tr("data is empty"));
    return;
  }

  SEND_MSG_Typedef_t msg;

  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en && (channel_index == channel_state.channel_num \
                                             || channel_index == channel_state_list.size()))
    {
      msg.channel_num = (quint8)channel_state.channel_num;
      msg.data = datas_;
      msg.frame_type = (FRAME_TYPE_Typedef_t)frame_type_index_;
      msg.protocol = (PROTOCOL_TYPE_Typedef_t)protocol_index_;
      msg.id = id_.toUInt(nullptr, 16);
      // QWriteLocker locker(&tx_msg_rw_lock);
      tx_msg_sem.acquire();
      send_msg_list.enqueue(msg);
      tx_msg_sem.release();
    }
  }
}

/******************************** End of file *********************************/
