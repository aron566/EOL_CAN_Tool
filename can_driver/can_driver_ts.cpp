/**
 *  @file can_driver_ts.cpp
 *
 *  @date 2023年10月31日 10:58:54 星期二
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief 同星can驱动.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.10.31 10:59 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-10-31 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
/** Private includes ---------------------------------------------------------*/
#include "can_driver_ts.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define USE_HW_CAN_SEND_64_DATA_EN  1/**< 是否开启非CANFD设备发送大于64Bytes的数据，采用分包发送 */
#define CAN_MSG_NUM_MAX             64U/**< can接收一次消息最大允许帧数 */
/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/

/* 列表数据需要和对话框中设备列表数据一一对应 */
static const can_driver_model::DEVICE_INFO_Typedef_t kDeviceType[] = {
  /* 同星can */
  {"TS_USBCANFD_1014",        can_driver_model::TS_CAN_BRAND,  6U,                      4},
};

/* USBCANFD */
static const quint32 kAbitTimingUSB[] = {
    1000000U,//1Mbps
    800000U,//800kbps
    500000U,//500kbps
    250000U,//250kbps
    125000U,//125kbps
    100000U,//100kbps
    50000U, //50kbps
};
static const quint32 kDbitTimingUSB[] = {
    5000000U,//5Mbps
    4000000U,//4Mbps
    2000000U,//2Mbps
    1000000U, //1Mbps
};

/* PCIECANFD brp=1 */
//static const quint32 kAbitTimingPCIE[] = {
//  1000000, //1M(80%)
//  800000, //800K(80%)
//  500000, //500K(80%)
//  250000, //250K(80%)
//  125000  //125K(80%)
//};

//static const quint32 kDbitTimingPCIE[] = {
//  8000000, //8Mbps(80%)
//  5000000, //5Mbps(75%)
//  5000000, //5Mbps(87.5%)
//  4000000, //4Mbps(80%)
//  2000000, //2Mbps(80%)
//  1000000  //1Mbps(80%)
//};

static const quint32 kBaudrate[] = {
  1000000U,
  800000U,
  500000U,
  250000U,
  125000U,
  100000U,
  50000U,
  20000U,
  10000U,
  5000U,
};

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


can_driver_ts::can_driver_ts(QObject *parent)
    : can_driver_model{parent}
{

}

can_driver_model::SET_FUNCTION_CAN_USE_Typedef_t can_driver_ts::function_can_use_update_for_choose(const QString &device_type_str)
{
  quint16 index = 0;
  SET_FUNCTION_CAN_USE_Typedef_t function_can_use;

  QString device_name;

  for(quint16 i = 0; i < sizeof(kDeviceType) / sizeof(kDeviceType[0]); i++)
  {
    device_name = QString(kDeviceType[i].device_type_str);
    function_can_use.device_list.append(device_name);
  }

  for(quint16 i = 0; i < sizeof(kDeviceType) / sizeof(kDeviceType[0]); i++)
  {
    if(0 == QString::compare(device_type_str, QString(kDeviceType[i].device_type_str)))
    {
      index = i;

      /* 获取当前通道数 */
      function_can_use.channel_num = kDeviceType[i].channel_count;
    }
  }

  quint32 type = kDeviceType[index].device_type;
  const bool cloudDevice = false;
  const bool netcanfd = false;
  const bool netcan = false;
  const bool netDevice = (netcan || netcanfd);
  const bool tcpDevice = false;
  const bool udpDevice = false;
  const bool usbcanfd = type == 6U;
  const bool pciecanfd = false;

  const bool canfdDevice = usbcanfd || pciecanfd || netcanfd;
  const bool accFilter = false;

  /* 工作模式是否可选 */
  function_can_use.work_mode_can_use = ((false == cloudDevice && false == netDevice));

  /* 终端电阻使能是否可选 */
  function_can_use.resistance_cs_use = ((true == usbcanfd));

  /* 波特率选择是否可选 */
  function_can_use.bauds_can_use = ((false == canfdDevice && false == netDevice && false == cloudDevice));

  /* 仲裁域，数据域波特率是否可选 */
  function_can_use.arbitration_data_bauds_can_use = ((true == canfdDevice && false == netDevice && false == cloudDevice));

  /* 自定义波特率选择是否可选 */
  function_can_use.diy_bauds_can_use = ((false == cloudDevice && false == netDevice));

  /* 过滤模式是否可选（验收码，屏蔽码） */
  function_can_use.filter_can_use = ((true == accFilter && false == cloudDevice && false == netDevice));

  /* 网络相关可选设置 */
  if(tcpDevice)
  {
    const bool server = (net_mode_index_ == 0);/* 0服务器 1客户端 */
    /* 本地端口是否可选 */
    function_can_use.local_port_can_use = (server);

    /* 远程端口是否可选 */
    function_can_use.remote_port_can_use = ((false == server));

    /* 远程地址是否可选 */
    function_can_use.remote_addr_can_use = ((false == server));
  }else if(udpDevice)
  {
    /* 本地端口是否可选 */
    function_can_use.local_port_can_use = true;

    /* 远程端口是否可选 */
    function_can_use.remote_port_can_use = true;

    /* 远程地址是否可选 */
    function_can_use.remote_addr_can_use = true;
  }
  else
  {
    /* 本地端口是否可选 */
    function_can_use.local_port_can_use = false;

    /* 远程端口是否可选 */
    function_can_use.remote_port_can_use = false;

    /* 远程地址是否可选 */
    function_can_use.remote_addr_can_use = false;
  }

  for(quint16 i = 0; i < sizeof(kAbitTimingUSB) / sizeof(kAbitTimingUSB[0]); i++)
  {
    function_can_use.abitrate_list.append(bps_number2str(kAbitTimingUSB[i]));
  }
  for(quint16 i = 0; i < sizeof(kDbitTimingUSB) / sizeof(kDbitTimingUSB[0]); i++)
  {
    function_can_use.datarate_list.append(bps_number2str(kDbitTimingUSB[i]));
  }
  for(quint16 i = 0; i < sizeof(kBaudrate) / sizeof(kBaudrate[0]); i++)
  {
    function_can_use.baudrate_list.append(bps_number2str(kBaudrate[i]));
  }

  return function_can_use;
}

void can_driver_ts::show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const TLibCAN *data, quint32 len)
{
  quint16 can_id = 0;
  for(quint32 i = 0; i < len; ++i)
  {
    const TLibCAN& can = data[i];
    const quint32& id = can.FIdentifier;
    const bool is_eff = can.FProperties.bits.extframe;
    const bool is_rtr = can.FProperties.bits.remoteframe;
    can_id = (id & 0x1FFFFFFFU);

    /* 消息分发到UI显示cq */
    msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_RX_DIRECT, \
                     CAN_PROTOCOL_TYPE, is_eff ? EXT_FRAME_TYPE : STD_FRAME_TYPE, \
                     is_rtr ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE, \
                     can.FData.d, can.FDLC);

    /* 消息过滤分发 */
    msg_to_cq_buf(can_id, (quint8)channel_state.channel_num, can.FData.d, can.FDLC);
  }
}

void can_driver_ts::show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const TLibCANFD *data, quint32 len)
{
  quint16 can_id = 0;
  for(quint32 i = 0; i < len; ++i)
  {
    const TLibCANFD& can = data[i];
    const quint32& id = can.FIdentifier;
    const bool is_eff = can.FProperties.bits.extframe;
    const bool is_rtr = can.FProperties.bits.remoteframe;
    can_id = (id & 0x1FFFFFFFU);

    /* 消息分发到UI显示cq */
    msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_RX_DIRECT, \
                     (PROTOCOL_TYPE_Typedef_t)can.FFDProperties.bits.EDL, \
                     is_eff ? EXT_FRAME_TYPE : STD_FRAME_TYPE, \
                     is_rtr ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE, \
                     can.FData.d, can.FDLC);

    /* 消息过滤分发 */
    msg_to_cq_buf(can_id, (quint8)channel_state.channel_num, can.FData.d, can.FDLC);
  }
}
/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

QStringList can_driver_ts::set_device_brand(CAN_BRAND_Typedef_t brand)
{
  QStringList device_list;
  QString device_name;

  brand_ = brand;
  for(quint16 i = 0; i < sizeof(kDeviceType) / sizeof(kDeviceType[0]); i++)
  {
    device_name = QString(kDeviceType[i].device_type_str);
    if(brand_ == kDeviceType[i].brand)
    {
      device_list.append(device_name);
    }
  }
  return device_list;
}

quint8 can_driver_ts::set_device_type(const QString &device_type_str)
{
  for(quint16 i = 0; i < sizeof(kDeviceType) / sizeof(kDeviceType[0]); i++)
  {
    if(0 == QString::compare(device_type_str, QString(kDeviceType[i].device_type_str)))
    {
      device_type_index_ = i;

      /* 更新功能列表 */
      function_can_use_update();
      return kDeviceType[i].channel_count;
    }
  }
  return 0;
}

bool can_driver_ts::open()
{
  ts_can_obj = QSharedPointer<TSCANLINApi>(new TSCANLINApi);
  if(ts_can_obj.isNull())
  {
    return false;
  }
  /* 扫描存在的设备：不是必须调用的 */
  quint32 device_cnt;
  if (ts_can_obj->tscan_scan_devices(&device_cnt) == 0)
  {
    if(0U == device_cnt)
    {
      return false;
    }
  }
  else
  {
    return false;
  }

  /* 发送can打开状态 */
  device_opened_ = true;
  emit signal_can_is_opened();
  show_message(tr("open device ok"));
  return true;
}

bool can_driver_ts::read_info()
{
//  QString show_info;

  if(nullptr == device_handle_)
  {
    return false;
  }

  return true;
}

bool can_driver_ts::init(CHANNEL_STATE_Typedef_t &channel_state)
{
  /* 连接设备：使用设备前必须调用 */
  ts_can_obj->InitTSCANAPI(true, false, false);
  quint32 connect_state = ts_can_obj->tscan_connect(0, &channel_state.device_handle);
  if((connect_state == 0U) || (connect_state == 5U))
  {
    int ret = ts_can_obj->tscan_config_canfd_by_baudrate(channel_state.device_handle,
                                                         (TS_APP_CHANNEL)channel_state.channel_num,
                                                         (double)kAbitTimingUSB[abit_baud_index_] / 1000,
                                                         (double)kDbitTimingUSB[abit_baud_index_] / 1000,
                                                         lfdtISOCAN,
                                                         lfdmNormal,
                                                         resistance_enable_);
    if(0 != ret)
    {
      show_message(tr("ts canfd ch %1 init failed").arg(channel_state.channel_num), channel_state.channel_num);
      return false;
    }
    show_message(tr("ts canfd ch %1 intit ok").arg(channel_state.channel_num), channel_state.channel_num);
    return true;
  }
  return false;
}

bool can_driver_ts::init()
{
  if(false == device_opened_)
  {
    show_message(tr("device is not open "));
    return false;
  }

  /* 初始化对应通道号 */
  bool ret = false;
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }
    channel_state = channel_state_list.takeAt(i);

    ret = init(channel_state);

    channel_state_list.insert(i, channel_state);
  }
  ts_init_flag = true;
  return ret;
}

bool can_driver_ts::start(const CHANNEL_STATE_Typedef_t &channel_state)
{
  quint32 connect_state;

  /* 避免二次初始化 */
  if(false == ts_init_flag)
  {
    init();
  }
  connect_state = ts_can_obj->tscan_connect(0, (quint64 *)&channel_state.device_handle);
  if((connect_state == 0U) || (connect_state == 5U))
  {
    show_message(tr("ts start canfd ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
    return true;
  }
  show_message(tr("ts start canfd ch %1 failed").arg(channel_state.channel_num), channel_state.channel_num);
  return false;
}

bool can_driver_ts::start()
{
  if(false == device_opened_)
  {
    show_message(tr("device is not open "));
    return false;
  }

  /* 启动对应通道号 */
  bool ret = false;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }
    ret = start(channel_state_list.value(i));
  }

  start_ = true;

  return ret;
}

bool can_driver_ts::reset(const CHANNEL_STATE_Typedef_t &channel_state)
{
  if (0U != ts_can_obj->tscan_disconnect_by_handle(channel_state.device_handle))
  {
    show_message(tr("ts reset canfd ch %1 failed").arg(channel_state.channel_num), channel_state.channel_num);
    return false;
  }
  show_message(tr("ts reset canfd ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
  return true;
}

bool can_driver_ts::reset()
{
  /* 复位对应通道号 */
  bool ret = false;
  ts_init_flag = false;
  start_ = false;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }
    ret = reset(channel_state_list.value(i));
  }
  return ret;
}

void can_driver_ts::close_channel(const CHANNEL_STATE_Typedef_t &channel_state)
{
  ts_can_obj->tscan_disconnect_by_handle(channel_state.device_handle);
  show_message(tr("ts device canfd ch %1 closed").arg(channel_state.channel_num), channel_state.channel_num);
}

bool can_driver_ts::close()
{
  /* 保护 */
  if(false == device_opened_)
  {
    show_message(tr("device is not open "));
    return false;
  }
  ts_init_flag = false;
  start_ = false;

  /* 关闭对应通道号 */
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }

    close_channel(channel_state_list.value(i));
  }

  /* 关闭设备 */
  /* todo nothing */

  device_opened_ = false;
  return true;
}

quint32 can_driver_ts::ts_can_send(const CHANNEL_STATE_Typedef_t &channel_state, \
                                   const quint8 *data, quint8 size, quint32 id, \
                                   FRAME_TYPE_Typedef_t frame_type, \
                                   PROTOCOL_TYPE_Typedef_t protocol)
{
  /* 实际发送的帧数 */
  quint32 result = 0;

  switch(protocol)
  {
    /* can */
    case CAN_PROTOCOL_TYPE:
      {
        TLibCAN canMsg;
        canMsg.FIdentifier                  = (qint32)id;
        canMsg.FProperties.value            = 0x00; // 清除原始属性
        canMsg.FProperties.bits.extframe    = (quint8)frame_type;
        canMsg.FProperties.bits.remoteframe = (quint8)DATA_FRAME_TYPE;// not remote frame，standard frame
        canMsg.FProperties.bits.istx        = 1;    // 设置属性为发送报文
        canMsg.FIdxChn                      = channel_state.channel_num;
        canMsg.FDLC                         = size > 8U ? 8U : size;
        memcpy_s(canMsg.FData.d, sizeof(canMsg.FData.d), data, canMsg.FDLC);
        /* 推送到fifo */
        if(0U != ts_can_obj->tscan_transmit_can_async(channel_state.device_handle, &canMsg))
        {
          break;
        }
        result = 1U;
        break;
      }

    /* canfd */
    case CANFD_PROTOCOL_TYPE:
      {
        TLibCANFD CANFDMsg;
        CANFDMsg.FIdentifier                  = (qint32)id;
        CANFDMsg.FProperties.value            = 0x00; // 清除原始属性
        CANFDMsg.FProperties.bits.extframe    = (quint8)frame_type;
        CANFDMsg.FProperties.bits.remoteframe = (quint8)DATA_FRAME_TYPE;// not remote frame，standard frame
        CANFDMsg.FProperties.bits.istx        = 1;    // 设置属性为发送报文
        CANFDMsg.FFDProperties.value          = 0;    /* 清除原始属性 */
        CANFDMsg.FFDProperties.bits.EDL       = 1;    /* canfd报文 */
        CANFDMsg.FIdxChn                      = channel_state.channel_num;
        CANFDMsg.FDLC                         = size > 64U ? 64U : size;
        memcpy_s(CANFDMsg.FData.d, sizeof(CANFDMsg.FData.d), data, CANFDMsg.FDLC);
        if(0U != ts_can_obj->tscan_transmit_canfd_async(channel_state.device_handle, &CANFDMsg))
        {
          break;
        }
        result = 1U;
        break;
      }

    default:
      break;
  }
  return result;
}

/* 发送固定协议帧长 */
bool can_driver_ts::send(const CHANNEL_STATE_Typedef_t &channel_state, \
                         const quint8 *data, quint8 size, quint32 id, \
                         FRAME_TYPE_Typedef_t frame_type, \
                         PROTOCOL_TYPE_Typedef_t protocol)
{
  /* 需要发送的帧数 */
  quint32 nSendCount = 1;

  /* 实际发送的帧数 */
  quint32 result = 0;

  bool ret = false;

  result = ts_can_send(channel_state, data, size, id, frame_type, protocol);

  /* 消息分发到UI显示cq */
  msg_to_ui_cq_buf(id, (quint8)channel_state.channel_num, CAN_TX_DIRECT,
                   protocol,
                   frame_type,
                   DATA_FRAME_TYPE,
                   data, size);

  QString csText;
  csText = QString("send num:%1, sucess num:%2").arg(nSendCount).arg(result);
  QString result_info_str;
  if(result != nSendCount)
  {
    ret = false;
    result_info_str = tr("[%1]send data failed! ").arg(channel_state.channel_num) + csText;
  }
  else
  {
    ret = true;
    result_info_str = tr("[%1]send data sucessful! ").arg(channel_state.channel_num) + csText;
  }
  msg_to_ui_cq_buf(id, (quint8)channel_state.channel_num, UNKNOW_DIRECT,
                   protocol,
                   frame_type,
                   DATA_FRAME_TYPE,
                   (const quint8 *)result_info_str.toUtf8().data(), result_info_str.size());
  return ret;
}

void can_driver_ts::function_can_use_update()
{
  quint32 type = kDeviceType[device_type_index_].device_type;
  const bool cloudDevice = false;
  const bool netcanfd = false;
  const bool netcan = false;
  const bool netDevice = (netcan || netcanfd);
  const bool tcpDevice = false;
  const bool udpDevice = false;
  const bool usbcanfd = type == 6U;
  const bool pciecanfd = false;

  const bool canfdDevice = usbcanfd || pciecanfd || netcanfd;
  const bool accFilter = false;

  /* 更新设备通道列表 */
  channel_state_list.clear();
  CHANNEL_STATE_Typedef_t channel_state;
  for(quint8 i = 0; i < kDeviceType[device_type_index_].channel_count; i++)
  {
    /* 默认通道1开启 */
    if(0 == i)
    {
      channel_state.channel_en = true;
    }
    else
    {
      channel_state.channel_en = false;
    }
    channel_state.channel_num = i;
    channel_state_list.append(channel_state);
  }

  /* 队列发送支持 */
  support_delay_send_ = usbcanfd || pciecanfd || netcanfd;
  support_delay_send_mode_ = usbcanfd || pciecanfd;
  support_get_send_mode_ = usbcanfd || pciecanfd;
  delay_send_can_use_update(support_delay_send_, support_delay_send_mode_, support_get_send_mode_);

  /* 队列发送模式是否启用 */
  send_queue_mode = false;
  emit signal_send_queue_mode_can_use(send_queue_mode);

  /* 定时发送支持 */
  const bool support_autosend_canfd = canfdDevice;    // CANFD 设备
  const bool support_autosend_can = canfdDevice ;     // CANFD 设备和其他CAN设备
  const bool support_autosend_index = (support_autosend_can && !pciecanfd);   // PCIECANFD 不支持使用索引控制定时，PCIECANFD添加一条即立即发送
  const bool support_stop_single_autosend = usbcanfd;
  const bool support_get_autosend_list = netcanfd;
  auto_send_can_use_update(support_autosend_can, support_autosend_canfd, \
                           support_autosend_index, support_stop_single_autosend, \
                           support_get_autosend_list);

  /* 工作模式是否可选 */
  emit signal_work_mode_can_use((false == cloudDevice && false == netDevice));

  /* 终端电阻使能是否可选 */
  emit signal_resistance_cs_use((true == usbcanfd));

  /* 波特率选择是否可选 */
  emit signal_bauds_can_use((false == canfdDevice && false == netDevice && false == cloudDevice));

  /* 仲裁域，数据域波特率是否可选 */
  emit signal_arbitration_data_bauds_can_use((true == canfdDevice && false == netDevice && false == cloudDevice));

  /* 自定义波特率选择是否可选 */
  emit signal_diy_bauds_can_use((false == cloudDevice && false == netDevice));

  /* 过滤模式是否可选（验收码，屏蔽码） */
  emit signal_filter_can_use((true == accFilter && false == cloudDevice && false == netDevice));

  /* 网络相关可选设置 */
  if(tcpDevice)
  {
    const bool server = (net_mode_index_ == 0);
    /* 本地端口是否可选 */
    emit signal_local_port_can_use(server);

    /* 远程端口是否可选 */
    emit signal_remote_port_can_use((false == server));

    /* 远程地址是否可选 */
    emit signal_remote_addr_can_use((false == server));
  }else if(udpDevice)
  {
    /* 本地端口是否可选 */
    emit signal_local_port_can_use(true);

    /* 远程端口是否可选 */
    emit signal_remote_port_can_use(true);

    /* 远程地址是否可选 */
    emit signal_remote_addr_can_use(true);
  }
  else
  {
    /* 本地端口是否可选 */
    emit signal_local_port_can_use(false);

    /* 远程端口是否可选 */
    emit signal_remote_port_can_use(false);

    /* 远程地址是否可选 */
    emit signal_remote_addr_can_use(false);
  }
}

void can_driver_ts::receive_data(const CHANNEL_STATE_Typedef_t &channel_state)
{
  //        TLibCAN can_data[CAN_MSG_NUM_MAX];
  //        qint32 can_frame_size = CAN_MSG_NUM_MAX;
  //        quint32 ret = 0;
  //        ret = ts_can_obj->tsfifo_receive_can_msgs(channel_state.device_handle, can_data, &can_frame_size, channel_state.channel_num, ONLY_RX_MESSAGES);
  //        if(0 < can_frame_size && ret == 0U)
  //        {
  //          show_message(channel_state, can_data, (quint32)can_frame_size);
  //        }

  TLibCANFD canfd_data[CAN_MSG_NUM_MAX];
  qint32 can_frame_size = CAN_MSG_NUM_MAX;
  quint32 ret = 0;
  can_frame_size = CAN_MSG_NUM_MAX;
  ret = ts_can_obj->tsfifo_receive_canfd_msgs(channel_state.device_handle, canfd_data, &can_frame_size, channel_state.channel_num, ONLY_RX_MESSAGES);
  if(0 < can_frame_size && ret == 0U)
  {
    show_rec_message(channel_state, canfd_data, (quint32)can_frame_size);
  }
}

/******************************** End of file *********************************/

