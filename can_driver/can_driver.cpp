/**
  *  @file can_driver.cpp
  *
  *  @date 2023年02月21日 14:05:52 星期一
  *
  *  @author aron566
  *
  *  @copyright Copyright (c) 2022 aron566 <aron566@163.com>.
  *
  *  @brief None.
  *
  *  @details None.
  *
  *  @version v1.0.0 aron566 2023.02.21 14:05 初始版本.
  */
/** Includes -----------------------------------------------------------------*/
#include <QDateTime>
/** Private includes ---------------------------------------------------------*/
#include "can_driver.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define TMP_BUFFER_LEN 1000U
/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/

typedef struct _DeviceInfo
{
  const char *device_type_str;
  quint16 device_type;  //设备类型
  quint8 channel_count;//设备的通道个数
}DeviceInfo;
/*
  列表数据需要和对话框中设备列表数据一一对应
*/
static const DeviceInfo kDeviceType[] = {
  {"ZCAN_USBCAN1",            ZCAN_USBCAN1,            1},
  {"ZCAN_USBCAN2",            ZCAN_USBCAN2,            2},
  {"ZCAN_PCI9820I",           ZCAN_PCI9820I,           2},
  {"ZCAN_USBCAN_E_U",         ZCAN_USBCAN_E_U,         1},
  {"ZCAN_USBCAN_2E_U",        ZCAN_USBCAN_2E_U,        2},
  {"ZCAN_USBCAN_4E_U",        ZCAN_USBCAN_4E_U,        4},
  {"ZCAN_PCIE_CANFD_100U",    ZCAN_PCIE_CANFD_100U,    1},
  {"ZCAN_PCIE_CANFD_200U",    ZCAN_PCIE_CANFD_200U,    2},
  {"ZCAN_PCIE_CANFD_400U_EX", ZCAN_PCIE_CANFD_400U_EX, 4},
  {"ZCAN_USBCANFD_200U",      ZCAN_USBCANFD_200U,      2},
  {"ZCAN_USBCANFD_100U",      ZCAN_USBCANFD_100U,      1},
  {"ZCAN_USBCANFD_MINI",      ZCAN_USBCANFD_MINI,      1},
  {"ZCAN_CANETTCP",           ZCAN_CANETTCP,           1},
  {"ZCAN_CANETUDP",           ZCAN_CANETUDP,           1},
  {"ZCAN_WIFICAN_TCP",        ZCAN_WIFICAN_TCP,        1},
  {"ZCAN_WIFICAN_UDP",        ZCAN_WIFICAN_UDP,        1},
  {"ZCAN_CLOUD",              ZCAN_CLOUD,              1},
  {"ZCAN_CANFDWIFI_TCP",      ZCAN_CANFDWIFI_TCP,      1},
  {"ZCAN_CANFDWIFI_UDP",      ZCAN_CANFDWIFI_UDP,      1},
  {"ZCAN_CANFDNET_TCP",       ZCAN_CANFDNET_TCP,       2},
  {"ZCAN_CANFDNET_UDP",       ZCAN_CANFDNET_UDP,       2},
  {"ZCAN_CANFDNET_400U_TCP",  ZCAN_CANFDNET_400U_TCP,  4},
  {"ZCAN_CANFDNET_400U_UDP",  ZCAN_CANFDNET_400U_UDP,  4},
};

//USBCANFD
static const quint32 kAbitTimingUSB[] = {
  1000000,//1Mbps
  800000,//800kbps
  500000,//500kbps
  250000,//250kbps
  125000,//125kbps
  100000,//100kbps
  50000 //50kbps
};
static const quint32 kDbitTimingUSB[] = {
  5000000,//5Mbps
  4000000,//4Mbps
  2000000,//2Mbps
  1000000 //1Mbps
};

//PCIECANFD brp=1
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
  1000000,
  800000,
  500000,
  250000,
  125000,
  100000,
  50000,
  20000,
  10000,
  5000
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

/** Public application code --------------------------------------------------*/
/*******************************************************************************
 *
 *       Public code
 *
 ********************************************************************************
 */

can_driver::can_driver(QObject *parent)
    : QObject{parent}
{
  cq_obj = new CircularQueue(CircularQueue::UINT8_DATA_BUF, CircularQueue::CQ_BUF_512, this);
  if(nullptr == cq_obj)
  {
    qDebug() << "create cq faild";
  }
}

quint8 can_driver::set_device_type(const QString &device_type_str)
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

bool can_driver::open()
{
//  if(kDeviceType[device_type_index_].device_type == ZCAN_CLOUD)
//  {
//    zcloudDlg dlg;
//    if(IDCANCEL == dlg.DoModal())
//    {
//      return;
//    }
//    device_index_ = dlg.GetDeviceIndex();
//  }
  device_handle_ = ZCAN_OpenDevice(kDeviceType[device_type_index_].device_type, device_index_, 0);
  if(INVALID_DEVICE_HANDLE == device_handle_)
  {
    show_message(tr("open device faild"));
    return false;
  }
  device_opened_ = true;

  /* 发送can打开状态 */
  emit signal_can_is_opened();
  show_message(tr("open device ok"));
  return true;
}

bool can_driver::init(CHANNEL_STATE_Typedef_t &channel_state)
{
  ZCAN_CHANNEL_INIT_CONFIG config;
  memset(&config, 0, sizeof(config));

  quint32 type = kDeviceType[device_type_index_].device_type;
  const bool cloudDevice = type == ZCAN_CLOUD;
  const bool netcanfd = is_net_can_fd_type(type);
  const bool netcan = is_net_can_type(type);
  const bool netDevice = (netcan || netcanfd);
  const bool tcpDevice = is_net_tcp_type(type);
  const bool server = net_mode_index_ == 0;
  const bool usbcanfd = type == ZCAN_USBCANFD_100U ||
                        type == ZCAN_USBCANFD_200U ||
                        type == ZCAN_USBCANFD_MINI;
  const bool pciecanfd = type == ZCAN_PCIE_CANFD_100U ||
                         type == ZCAN_PCIE_CANFD_200U ||
                         type == ZCAN_PCIE_CANFD_400U_EX;
  const bool canfdDevice = usbcanfd || pciecanfd;

  if(cloudDevice)
  {
    qDebug() << "cloudDevice";
  }
  else if(netDevice)
  {
    qDebug() << "netDevice/";
    char path[50] = {0};
    char value[100] = {0};
    if(tcpDevice)
    {
      qDebug() << "tcpDevice/";
      sprintf(path, "%d/work_mode", channel_state.channel_num);
      sprintf(value, "%d", server ? 1 : 0);
      ZCAN_SetValue(device_handle_, path, value);
      if(server)
      {
        qDebug() << "server";
        sprintf(path, "%d/local_port", channel_state.channel_num);
        ZCAN_SetValue(device_handle_, path, local_port_str.toStdString().data());
      } // server
      else
      {
        qDebug() << "client";
        sprintf(path, "%d/ip", channel_state.channel_num);
        ZCAN_SetValue(device_handle_, path, service_ip_str.toStdString().data());

        sprintf(path, "%d/work_port", channel_state.channel_num);
        ZCAN_SetValue(device_handle_, path, service_port_str.toStdString().data());
      }
    } // tcp
    else
    {
      qDebug() << "udpDevice";
      sprintf(path, "%d/local_port", channel_state.channel_num);
      ZCAN_SetValue(device_handle_, path, local_port_str.toStdString().data());
      sprintf(path, "%d/ip", channel_state.channel_num);

      ZCAN_SetValue(device_handle_, path, service_ip_str.toStdString().data());
      sprintf(path, "%d/work_port", channel_state.channel_num);
      ZCAN_SetValue(device_handle_, path, service_port_str.toStdString().data());
    }
  }
  else
  {
    //设置波特率
    if(custom_baud_enable_)
    {
      qDebug() << "set diy bps";
      if(!custom_baud_rate_config(channel_state))
      {
        show_message(tr("set diy baudrate faild "), channel_state.channel_num);
        return false;
      }
    }
    else
    {
      qDebug() << "set bps";
      if(!canfdDevice && !baud_rate_config(channel_state))
      {
        show_message(tr("set baudrate faild "), channel_state.channel_num);
        return false;
      }
    }

    if(usbcanfd)
    {
      qDebug() << "set usbcanfd channel";
      char path[50] = {0};
      char value[100] = {0};
      sprintf(path, "%d/canfd_standard", channel_state.channel_num);
      sprintf(value, "%d", 0);
      ZCAN_SetValue(device_handle_, path, value);
    }
    if(usbcanfd)
    {
      qDebug() << "usbcanfd diy bps";
      if(custom_baud_enable_)
      {
        qDebug() << "usbcanfd diy bps set ...";
        if(!custom_baud_rate_config(channel_state))
        {
          show_message(tr("set diy baudrate faild "), channel_state.channel_num);
          return false;
        }
      }
      else
      {
        qDebug() << "usbcanfd bps set ...";
        if(!cand_fd_bps_config(channel_state))
        {
          show_message(tr("set baudrate faild "), channel_state.channel_num);
          return false;
        }
      }
      config.can_type = TYPE_CANFD;
      config.canfd.mode = work_mode_index_;
      config.canfd.filter = filter_mode_;
      bool ok;
      config.canfd.acc_code = acc_code_.toUInt(&ok, 16);
      config.canfd.acc_mask = acc_mask_.toUInt(&ok, 16);
    }
    else if(pciecanfd)
    {

      char path[50] = { 0 };
      char value[100] = { 0 };
      if(!cand_fd_bps_config(channel_state))
      {
        show_message(tr("set baudrate faild "), channel_state.channel_num);
        return false;
      }

      if(type == ZCAN_PCIE_CANFD_400U_EX )
      {
        sprintf(path, "0/set_device_recv_merge");
        sprintf(value, "0");
        ZCAN_SetValue(device_handle_, path, value);
      }

      config.can_type = TYPE_CANFD;
      config.canfd.mode = work_mode_index_;
      config.canfd.filter = filter_mode_;
      bool ok;
      config.canfd.acc_code = acc_code_.toUInt(&ok, 16);
      config.canfd.acc_mask = acc_mask_.toUInt(&ok, 16);

    }
    else
    {
        config.can_type = TYPE_CAN;
        config.can.mode = work_mode_index_;
        config.can.filter = filter_mode_;
        bool ok;
        config.can.acc_code = acc_code_.toUInt(&ok, 16);
        config.can.acc_mask = acc_mask_.toUInt(&ok, 16);
    }
  }

  channel_state.channel_hadle = ZCAN_InitCAN(device_handle_, channel_state.channel_num, &config);
  if(INVALID_CHANNEL_HANDLE == channel_state.channel_hadle)
  {
    show_message(tr("can ch %1 init faild").arg(channel_state.channel_num), channel_state.channel_num);
    return false;
  }
  if(usbcanfd)
  {
    if(resistance_enable_ && !resistance_config(channel_state))
    {
      show_message(tr("set resistance faild"), channel_state.channel_num);
      return false;
    }
  }
  show_message(tr("can ch %1 intit ok").arg(channel_state.channel_num), channel_state.channel_num);
  return true;
}

bool can_driver::init()
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
  return ret;
}

bool can_driver::start(const CHANNEL_STATE_Typedef_t &channel_state)
{
  if(ZCAN_StartCAN(channel_state.channel_hadle) != STATUS_OK)
  {
    show_message(tr("start can ch %1 faild").arg(channel_state.channel_num), channel_state.channel_num);
    return false;
  }

  show_message(tr("start can ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
  return true;
}

bool can_driver::start()
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

bool can_driver::reset(const CHANNEL_STATE_Typedef_t &channel_state)
{
  if(ZCAN_ResetCAN(channel_state.channel_hadle) != STATUS_OK)
  {
    show_message(tr("reset can ch %1 faild ").arg(channel_state.channel_num), channel_state.channel_num);
    return false;
  }

  show_message(tr("reset can ch %1 ok ").arg(channel_state.channel_num), channel_state.channel_num);
  return true;
}

bool can_driver::reset()
{
  /* 复位对应通道号 */
  bool ret = false;

  start_ = false;

  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }
    ret = reset(channel_state_list.value(i));
  }

  emit signal_can_driver_reset();
  return ret;
}

void can_driver::close(const CHANNEL_STATE_Typedef_t &channel_state)
{
  // TODO: Add your control notification handler code here

  if(ZCLOUD_IsConnected())
  {
    ZCLOUD_DisconnectServer();
  }
  ZCAN_ResetCAN(channel_state.channel_hadle);
  ZCAN_CloseDevice(device_handle_);

  show_message(tr("device can ch %1 closed").arg(channel_state.channel_num), channel_state.channel_num);
}

void can_driver::close()
{
  start_ = false;

  /* 关闭对应通道号 */
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }

    close(channel_state_list.value(i));
  }

  /* 发送can关闭状态 */
  emit signal_can_is_closed();

  device_opened_ = false;
}

bool can_driver::send(const CHANNEL_STATE_Typedef_t &channel_state, const quint8 *data, quint8 size, quint32 id, FRAME_TYPE_Typedef_t frame_type, PROTOCOL_TYPE_Typedef_t protocol)
{
  quint32 nSendCount = 1;
  quint32 result = 0;//发送的帧数
  if(0 == (quint32)protocol)//can
  {
    ZCAN_Transmit_Data can_data;
    can_frame_packed(can_data, id, (quint32)frame_type, data, size);

    if(nSendCount > 0)
    {
      ZCAN_Transmit_Data* pData = new ZCAN_Transmit_Data[nSendCount];
      for(quint32 i = 0; i < nSendCount; ++i)
      {
        memcpy_s(&pData[i], sizeof(ZCAN_Transmit_Data), &can_data, sizeof(can_data));
      }

      show_message(channel_state, pData, 1);
      result = ZCAN_Transmit(channel_state.channel_hadle, pData, nSendCount);
      delete [] pData;
    }
  }
  else //canfd
  {
    ZCAN_TransmitFD_Data canfd_data;
    can_frame_packed(canfd_data, id, frame_type, data, size);

    if (nSendCount > 0)
    {
      ZCAN_TransmitFD_Data* pData = new ZCAN_TransmitFD_Data[nSendCount];
      for(quint32 i = 0; i < nSendCount; ++i)
      {
        memcpy_s(&pData[i], sizeof(ZCAN_TransmitFD_Data), &canfd_data, sizeof(canfd_data));
      }

      show_message(channel_state, pData, 1);
      result = ZCAN_TransmitFD(channel_state.channel_hadle, pData, nSendCount);
      delete [] pData;
    }
  }
  QString csText;
  csText = QString::asprintf(tr("send num:%d, sucess num:%d").toUtf8().data(), nSendCount, result);
  if(result != nSendCount)
  {
    show_message(tr("[%1]send data faild! ").arg(channel_state.channel_num) + csText, channel_state.channel_num);
    return false;
  }
  else
  {
    show_message(tr("[%1]send data sucessful! ").arg(channel_state.channel_num) + csText, channel_state.channel_num);
    return true;
  }
}

bool can_driver::send(const quint8 *data, quint8 size, quint32 id, FRAME_TYPE_Typedef_t frame_type, PROTOCOL_TYPE_Typedef_t protocol, quint8 channel_num)
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
    if((channel_num == channel_state.channel_num || 0xFF == channel_num) \
       && true == channel_state.channel_en)
    {
      ret = send(channel_state, data, size, id, frame_type, protocol);
      break;
    }
  }

  return ret;
}

void can_driver::send(const CHANNEL_STATE_Typedef_t &channel_state)
{
  if(datas_.isEmpty())
  {
    show_message(tr("data is empty"), channel_state.channel_num);
    return;
  }

  quint32 nSendCount = send_count_once_;
  quint32 result = 0;//发送的帧数
  if(0 == protocol_index_)//can
  {
    ZCAN_Transmit_Data can_data;
    can_frame_packed(can_data, true);

    if(nSendCount > 0)
    {
      ZCAN_Transmit_Data* pData = new ZCAN_Transmit_Data[nSendCount];
      for(quint32 i = 0; i < nSendCount; ++i)
      {
        memcpy_s(&pData[i], sizeof(ZCAN_Transmit_Data), &can_data, sizeof(can_data));
      }

      show_message(channel_state, pData, 1);
      result = ZCAN_Transmit(channel_state.channel_hadle, pData, nSendCount);
      delete [] pData;
    }
  }
  else //canfd
  {
    ZCAN_TransmitFD_Data canfd_data;
    can_frame_packed(canfd_data, true);

    if (nSendCount > 0)
    {
      ZCAN_TransmitFD_Data* pData = new ZCAN_TransmitFD_Data[nSendCount];
      for(quint32 i = 0; i < nSendCount; ++i)
      {
        memcpy_s(&pData[i], sizeof(ZCAN_TransmitFD_Data), &canfd_data, sizeof(canfd_data));
      }

      show_message(channel_state, pData, 1);
      result = ZCAN_TransmitFD(channel_state.channel_hadle, pData, nSendCount);
      delete [] pData;
    }
  }
  QString csText;
  csText = QString::asprintf(tr("send num:%d, sucess num:%d").toUtf8().data(), nSendCount, result);
  if(result != nSendCount)
  {
    show_message(tr("[%1]send data faild! ").arg(channel_state.channel_num) + csText, channel_state.channel_num);
  }
  else
  {
    show_message(tr("[%1]send data sucessful! ").arg(channel_state.channel_num) + csText, channel_state.channel_num);
  }
}

void can_driver::send(quint8 channel_index)
{
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en && (channel_index == channel_state.channel_num \
       || channel_index == channel_state_list.size()))
    {
      send(channel_state);
    }
  }
}

/**
 * @brief 设置通讯通道号
 *
 * @param index
 */
void can_driver::set_channel_index(quint8 index)
{
  /* 判断通道号是否全部使能 */
  CHANNEL_STATE_Typedef_t channel_state;
  if(kDeviceType[device_type_index_].channel_count <= index)
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

void can_driver::function_can_use_update()
{
  quint32 type = kDeviceType[device_type_index_].device_type;
  const bool cloudDevice = type == ZCAN_CLOUD;
  const bool netcanfd = is_net_can_fd_type(type);
  const bool netcan = is_net_can_type(type);
  const bool netDevice = (netcan || netcanfd);
  const bool tcpDevice = is_net_tcp_type(type);
  const bool udpDevice = is_net_udp_type(type);
  const bool usbcanfd = type == ZCAN_USBCANFD_100U ||
                        type == ZCAN_USBCANFD_200U ||
                        type == ZCAN_USBCANFD_MINI;
  const bool pciecanfd = type == ZCAN_PCIE_CANFD_100U ||
                         type == ZCAN_PCIE_CANFD_200U ||
                         type == ZCAN_PCIE_CANFD_400U ||
                         type == ZCAN_PCIE_CANFD_400U_EX;

  const bool canfdDevice = usbcanfd || pciecanfd || netcanfd;
  const bool accFilter = pciecanfd || type == ZCAN_USBCAN1 || type == ZCAN_USBCAN2;

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

  //队列发送支持
  support_delay_send_ = usbcanfd || pciecanfd || netcanfd;
  support_delay_send_mode_ = usbcanfd || pciecanfd;
  support_get_send_mode_ = usbcanfd || pciecanfd;
  delay_send_can_use_update(support_delay_send_, support_delay_send_mode_, support_get_send_mode_);

  /* 队列发送模式是否启用 */
  send_queue_mode = false;
  emit signal_send_queue_mode_can_use(send_queue_mode);

  //定时发送支持
  const bool support_autosend_canfd = canfdDevice;    // CANFD 设备
  const bool support_autosend_can = canfdDevice ;     // CANFD 设备和其他CAN设备
  const bool support_autosend_index = (support_autosend_can && !pciecanfd);   // PCIECANFD 不支持使用索引控制定时，PCIECANFD添加一条即立即发送
  const bool support_stop_single_autosend = usbcanfd;
  const bool support_get_autosend_list = netcanfd;
  auto_send_can_use_update(support_autosend_can, support_autosend_canfd, \
                           support_autosend_index, support_stop_single_autosend, \
                           support_get_autosend_list);

  /* 设置仲裁域波特率及数据域波特率 */
  if(usbcanfd)
  {
    arbitration_bps_list.clear();
    arbitration_bps_list.append(QString("1Mbps"));
    arbitration_bps_list.append(QString("800kbps"));
    arbitration_bps_list.append(QString("500kbps"));
    arbitration_bps_list.append(QString("250kbps"));
    arbitration_bps_list.append(QString("125kbps"));
    arbitration_bps_list.append(QString("100kbps"));
    arbitration_bps_list.append(QString("50kbps"));

    data_bps_list.clear();
    data_bps_list.append(QString("5Mbps"));
    data_bps_list.append(QString("4Mbps"));
    data_bps_list.append(QString("2Mbps"));
    data_bps_list.append(QString("1Mbps"));
  }
  else if(pciecanfd)
  {
    arbitration_bps_list.clear();
    arbitration_bps_list.append(QString("1Mbps(80%)"));
    arbitration_bps_list.append(QString("800kbps(80%)"));
    arbitration_bps_list.append(QString("500kbps(80%)"));
    arbitration_bps_list.append(QString("250kbps(80%)"));
    arbitration_bps_list.append(QString("125kbps(80%)"));
    arbitration_bps_list.append(QString("100kbps"));
    arbitration_bps_list.append(QString("50kbps"));

    data_bps_list.clear();
    data_bps_list.append(QString("5Mbps(80%)"));
    data_bps_list.append(QString("4Mbps(80%)"));
    data_bps_list.append(QString("2Mbps(80%)"));
    data_bps_list.append(QString("1Mbps(80%)"));
  }

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

//设置自定义波特率, 需要从CANMaster目录下的baudcal生成字符串
bool can_driver::custom_baud_rate_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  char path[50] = {0};
  sprintf(path, "%d/baud_rate_custom", channel_state.channel_num);
  return 1 == ZCAN_SetValue(device_handle_, path, custom_baudrate_.toUtf8().data());
}


void can_driver::receice_data(const CHANNEL_STATE_Typedef_t &channel_state)
{
  ZCAN_Receive_Data can_data[100];
  ZCAN_ReceiveFD_Data canfd_data[100];
  quint32 len;
  /* 获取can数据长度 */
  len = ZCAN_GetReceiveNum(channel_state.channel_hadle, TYPE_CAN);
  if(0 < len)
  {
    len = ZCAN_Receive(channel_state.channel_hadle, can_data, 100, 50);
    show_message(channel_state, can_data, len);
  }

  /* 获取canfd数据长度 */
  len = ZCAN_GetReceiveNum(channel_state.channel_hadle, TYPE_CANFD);
  if(0 < len)
  {
    len = ZCAN_ReceiveFD(channel_state.channel_hadle, canfd_data, 100, 50);
    show_message(channel_state, canfd_data, len);
  }
}

void can_driver::receice_data()
{
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en)
    {
      receice_data(channel_state);
    }
  }
}

void can_driver::add_msg_filter(quint32 can_id, CircularQueue *cq_obj_)
{
  if(nullptr == cq_obj_)
  {
    return;
  }

  MSG_FILTER_Typedef_t msg_filter;
  msg_filter.can_id = can_id;
  msg_filter.cq_obj = cq_obj_;

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

void can_driver::remove_msg_filter(quint32 can_id)
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

void can_driver::show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_Receive_Data *data, quint32 len)
{
  uint8_t temp_buf[64 + 2] = {0};

  QString item;
  quint32 id_temp;
  quint16 can_id = 0;
  for(quint32 i = 0; i < len; ++i)
  {
    const ZCAN_Receive_Data& can = data[i];
    const canid_t& id = can.frame.can_id;
    const bool is_eff = IS_EFF(id);
    const bool is_rtr = IS_RTR(id);
    item = QString::asprintf(tr("[%u]Rx CAN ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
                             channel_state.channel_num, \
                             GET_ID(id), is_eff ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data(), \
                             is_rtr ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), \
                             can.frame.can_dlc);
    for(quint32 i = 0; i < can.frame.can_dlc; ++i)
    {
      item += QString::asprintf("%02X ", can.frame.data[i]);
    }

    can_id = GET_ID(id);

    /* 消息过滤 */
    id_temp = (can_id & can_id_mask_);
    if(can_id == id_temp || false == can_id_mask_en_)
    {
      /* 显示接收到的字节数 */
      show_message_bytes(can.frame.can_dlc, channel_state.channel_num, CAN_RX_DIRECT);
      show_message(item, channel_state.channel_num, CAN_RX_DIRECT, can.frame.data, can.frame.can_dlc, true);
    }

    /* 加入数据到cq */
    if(nullptr != cq_obj)
    {
      memcpy(temp_buf, &can_id, sizeof(quint16));
      memcpy(temp_buf + sizeof(quint16), can.frame.data, can.frame.can_dlc);
      CircularQueue::CQ_putData(cq_obj->get_cq_handle(), temp_buf, can.frame.can_dlc + sizeof(quint16));
    }

    /* 消息过滤分发 */
    if(msg_filter_list.isEmpty())
    {
      continue;
    }
    for(qint32 index = 0; index < msg_filter_list.size(); index++)
    {
      if(msg_filter_list.value(index).can_id == can_id)
      {
        CircularQueue::CQ_putData(msg_filter_list.value(index).cq_obj->get_cq_handle(), can.frame.data, can.frame.can_dlc);
        break;
      }
    }
  }
}

void can_driver::show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_ReceiveFD_Data *data, quint32 len)
{
  uint8_t temp_buf[64 + 2] = {0};

  QString item;
  quint32 id_temp;
  quint16 can_id = 0;
  for(quint32 i = 0; i < len; ++i)
  {
    const ZCAN_ReceiveFD_Data& canfd = data[i];
    const canid_t& id = canfd.frame.can_id;
    item = QString::asprintf(tr("[%u]Rx CANFD ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
            channel_state.channel_num, GET_ID(id), IS_EFF(id) ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data()
        , IS_RTR(id) ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), canfd.frame.len);
    for (quint32 i = 0; i < canfd.frame.len; ++i)
    {
      item += QString::asprintf("%02X ", canfd.frame.data[i]);
    }

    can_id = GET_ID(id);

    /* 消息过滤 */
    id_temp = (can_id & can_id_mask_);
    if(can_id == id_temp || false == can_id_mask_en_)
    {
      /* 显示接收到的字节数 */
      show_message_bytes(canfd.frame.len, channel_state.channel_num, CAN_RX_DIRECT);
      show_message(item, channel_state.channel_num, CAN_RX_DIRECT, canfd.frame.data, canfd.frame.len, true);
    }

    /* 加入数据到cq */
    if(nullptr != cq_obj)
    {  
      memcpy(temp_buf, &can_id, sizeof(quint16));
      memcpy(temp_buf + sizeof(quint16), canfd.frame.data, canfd.frame.len);
      CircularQueue::CQ_putData(cq_obj->get_cq_handle(), temp_buf, canfd.frame.len + sizeof(quint16));
    }

    /* 消息过滤分发 */
    if(msg_filter_list.isEmpty())
    {
      continue;
    }
    for(qint32 index = 0; index < msg_filter_list.size(); index++)
    {
      if(msg_filter_list.value(index).can_id == can_id)
      {
        CircularQueue::CQ_putData(msg_filter_list.value(index).cq_obj->get_cq_handle(), canfd.frame.data, canfd.frame.len);
//        utility::debug_print(canfd.frame.data, canfd.frame.len, "can driver:");
        break;
      }
    }
  }
}

void can_driver::show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_Transmit_Data *data, quint32 len)
{
  QString item;
  for(quint32 i = 0; i < len; ++i)
  {
    const ZCAN_Transmit_Data& can = data[i];
    const canid_t &id = can.frame.can_id;
    item = QString::asprintf(tr("[%u]Tx CAN ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
                             channel_state.channel_num, \
                             GET_ID(id), IS_EFF(id) ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data(), \
          IS_RTR(id) ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), can.frame.can_dlc);
    for(quint32 i = 0; i < can.frame.can_dlc; ++i)
    {
      item += QString::asprintf("%02X ", can.frame.data[i]);
    }
    show_message_bytes(can.frame.can_dlc, channel_state.channel_num, CAN_TX_DIRECT);
    show_message(item, channel_state.channel_num, CAN_TX_DIRECT, can.frame.data, can.frame.can_dlc);
  }
}

void can_driver::show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_TransmitFD_Data *data, quint32 len)
{
  QString item;
  for(quint32 i = 0; i < len; ++i)
  {
    const ZCAN_TransmitFD_Data& can = data[i];
    const canid_t &id = can.frame.can_id;
    item = QString::asprintf(tr("[%u]Tx CANFD ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
                             channel_state.channel_num, \
                             GET_ID(id), IS_EFF(id) ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data(), \
          IS_RTR(id) ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), can.frame.len);
    for(quint32 i = 0; i < can.frame.len; ++i)
    {
      item += QString::asprintf("%02X ", can.frame.data[i]);
    }
    show_message_bytes(can.frame.len, channel_state.channel_num, CAN_TX_DIRECT);
    show_message(item, channel_state.channel_num, CAN_TX_DIRECT, can.frame.data, can.frame.len);
  }
}

void can_driver::show_message(const QString &str, quint32 channel_num, CAN_DIRECT_Typedef_t direct, const quint8 *data, quint32 data_len, bool thread_mode)
{
  message = str;
  /* 输出到显示框 */
  if(false == thread_mode)
  {
    emit signal_show_message(message, channel_num, (quint8)direct, data, data_len);
  }
  else
  {
    emit signal_show_thread_message(message, channel_num, (quint8)direct, data, data_len);
    return;
  }

  qDebug() << message;
}

void can_driver::show_message_bytes(quint8 bytes, quint32 channel_num, CAN_DIRECT_Typedef_t direct)
{
  emit signal_show_message_bytes(bytes, channel_num, (quint8)direct);
}

bool can_driver::transmit_type_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  char path[50] = {0};
  char value[100] = {0};
  sprintf(path, "%d/send_type", channel_state.channel_num);
  sprintf(value, "%d", send_type_index_);
  return 1 == ZCAN_SetValue(device_handle_, path, value);
}

//设置终端电阻使能
bool can_driver::resistance_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  char path[50] = {0};
  sprintf(path, "%d/initenal_resistance", channel_state.channel_num);
  char value[10] = {0};
  sprintf(value, "%d", resistance_enable_);
  return 1 == ZCAN_SetValue(device_handle_, path, value);
}

//设置CAN卡波特率
bool can_driver::baud_rate_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  qDebug() << "/baud_rate_config";
  char path[50] = {0};
  sprintf(path, "%d/baud_rate", channel_state.channel_num);
  char value[10] = {0};
  sprintf(value, "%d", kBaudrate[baud_index_]);
  return 1 == ZCAN_SetValue(device_handle_, path, value);
}

//设置USBCANFD卡波特率
bool can_driver::cand_fd_bps_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  qDebug() << channel_state.channel_num << "/canfd_abit_baud_rate " << kAbitTimingUSB[abit_baud_index_];
  char path[50] = { 0 };
  sprintf(path, "%d/canfd_abit_baud_rate", channel_state.channel_num);
  char value[10] = { 0 };
  sprintf(value, "%d", kAbitTimingUSB[abit_baud_index_]);
  int ret_a = ZCAN_SetValue(device_handle_, path, value);

  qDebug() << channel_state.channel_num << "/canfd_dbit_baud_rate " << kDbitTimingUSB[dbit_baud_index_];
  sprintf(path, "%d/canfd_dbit_baud_rate", channel_state.channel_num);
  sprintf(value, "%d", kDbitTimingUSB[dbit_baud_index_]);
  int ret_d = ZCAN_SetValue(device_handle_, path, value);
  return 1 == (ret_a && ret_d);
}

void can_driver::show_tx_queue_available(const CHANNEL_STATE_Typedef_t &channel_state)
{
  // TODO: Add your control notification handler code here

  char path[50] = {0};
//  char value[100] = {0};
  QString csText;
  sprintf(path, "%d/get_device_available_tx_count/1", channel_state.channel_num);
  const char* pRet = (const char *)ZCAN_GetValue(device_handle_, path);
  if (pRet)
  {
    quint32 nSpace = *(int *)pRet;
    csText = QString((tr("[%1]queue can use space %2")).arg(channel_state.channel_num).arg(nSpace));
  }
  else
  {
    csText = tr("get queue can use space faild");
  }
  show_message(csText);
}

void can_driver::show_tx_queue_available()
{
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en)
    {
      show_tx_queue_available(channel_state);
    }
  }
}

void can_driver::clear_tx_queue(const CHANNEL_STATE_Typedef_t &channel_state)
{
  // TODO: Add your control notification handler code here

  char path[50] = {0};
  char value[100] = {0};
  sprintf(path, "%d/clear_delay_send_queue", channel_state.channel_num);
  int nRet = ZCAN_SetValue(device_handle_, path, value);
  QString csText;
  csText = QString(tr("[%1]clear tx queue [%2] ").arg(channel_state.channel_num).arg(nRet > 0 ? tr(" ok ") : tr(" faild ")));
  show_message(csText);
}
void can_driver::clear_tx_queue()
{
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en)
    {
      clear_tx_queue(channel_state);
    }
  }
}

bool can_driver::set_send_queue_mode(const CHANNEL_STATE_Typedef_t &channel_state, bool en)
{
  // TODO: Add your control notification handler code here

  char path[50] = {0};
  char value[100] = {0};
  int nDelaySendQueueMode = (send_queue_mode == true && en == true);
  sprintf(path, "%d/set_send_mode", channel_state.channel_num);
  sprintf(value, "%d", nDelaySendQueueMode);
  int nRet = ZCAN_SetValue(device_handle_, path, value);
  QString csText, csRet;
  csText = (nDelaySendQueueMode ? tr("[%1]open tx queue mode ").arg(channel_state.channel_num) : tr("[%1]close tx queue mode").arg(channel_state.channel_num));
  csRet = QString::asprintf((tr("[%s]").toUtf8().data(), \
          nRet > 0 ? tr(" ok ").toUtf8().data() : tr(" faild ").toUtf8().data()));
  show_message(csText + csRet);
  return nDelaySendQueueMode;
}
bool can_driver::set_send_queue_mode(bool en)
{
  bool ret = false;
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en)
    {
      ret =set_send_queue_mode(channel_state, en);
    }
  }
  return ret;
}

void can_driver::delay_send_can_use_update(bool delay_send, bool send_queue_mode, bool get_send_mode)
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

bool can_driver::is_net_can_type(quint32 type)
{
  return (type == ZCAN_CANETUDP || type == ZCAN_CANETTCP ||
          type == ZCAN_WIFICAN_TCP || type == ZCAN_WIFICAN_UDP ||
          type == ZCAN_CANDTU_NET || type == ZCAN_CANDTU_NET_400);
}

bool can_driver::is_net_can_fd_type(quint32 type)
{
  return (type == ZCAN_CANFDNET_TCP || type == ZCAN_CANFDNET_UDP ||
          type == ZCAN_CANFDNET_400U_TCP || type == ZCAN_CANFDNET_400U_UDP ||
          type == ZCAN_CANFDWIFI_TCP || type == ZCAN_CANFDWIFI_UDP);
}

bool can_driver::is_net_tcp_type(quint32 type)
{
  return (type == ZCAN_CANETTCP || type == ZCAN_WIFICAN_TCP ||
          type == ZCAN_CANDTU_NET || type == ZCAN_CANDTU_NET_400 ||
          type == ZCAN_CANFDNET_TCP || type == ZCAN_CANFDNET_400U_TCP ||
          type == ZCAN_CANFDWIFI_TCP );
}

bool can_driver::is_net_udp_type(quint32 type)
{
  return (type == ZCAN_CANETUDP || type == ZCAN_WIFICAN_UDP ||
          type == ZCAN_CANFDNET_UDP || type == ZCAN_CANFDNET_400U_UDP ||
          type == ZCAN_CANFDWIFI_UDP);
}

void can_driver::show_send_mode(const CHANNEL_STATE_Typedef_t &channel_state)
{
  // TODO: Add your control notification handler code here

  QString csText;
  char path[50] = {0};
  sprintf(path, "%d/get_send_mode/1", channel_state.channel_num);
  const char* pRet = (const char*)ZCAN_GetValue(device_handle_, path);
  if(pRet)
  {
    quint32 nMode = *(int*)pRet;
    csText = QString(tr("[%1] device current mode: [%2] ").arg(channel_state.channel_num).arg(nMode ? tr("tx queue mode") : tr("tx normal mode")));
  }
  else
  {
    csText = tr("[%1]get channel modefaild").arg(channel_state.channel_num);
  }
  show_message(csText);
}
void can_driver::show_send_mode()
{
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en)
    {
      show_send_mode(channel_state);
    }
  }
}

void can_driver::auto_send_can_use_update(bool support_can, bool support_canfd, bool support_index, bool support_single_cancel, bool support_get_autosend_list)
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

void can_driver::can_frame_packed(ZCAN_Transmit_Data &can_data, quint32 id, quint32 frame_type, const quint8 *data, quint8 size)
{
  memset(&can_data, 0, sizeof(can_data));
  can_data.frame.can_id = MAKE_CAN_ID(id, frame_type, 0, 0);

  quint8 len = size > CAN_MAX_DLEN ? CAN_MAX_DLEN : (BYTE)size;
  for(quint32 i = 0; i < len; i++)
  {
    can_data.frame.data[i] = (BYTE)data[i];
  }

  can_data.frame.can_dlc = len;
  can_data.transmit_type = send_type_index_;
}

void can_driver::can_frame_packed(ZCAN_TransmitFD_Data &canfd_data, quint32 id, quint32 frame_type, const quint8 *data, quint8 size)
{
  memset(&canfd_data, 0, sizeof(canfd_data));
  canfd_data.frame.can_id = MAKE_CAN_ID(id, frame_type, 0, 0);

  quint8 len = size > CANFD_MAX_DLEN ? CANFD_MAX_DLEN : (BYTE)size;
  for(quint32 i = 0; i < len; i++)
  {
    canfd_data.frame.data[i] = (BYTE)data[i];
  }

  canfd_data.frame.len = len;
  canfd_data.transmit_type = send_type_index_;
  canfd_data.frame.flags |= canfd_exp_index_ ? CANFD_BRS : 0;
}

void can_driver::can_frame_packed(ZCAN_Transmit_Data &can_data, bool add_delay_flag)
{
  bool ok;
  quint32 id = id_.toUInt(&ok, 16);
  bool bDelay = frm_delay_flag_;
  quint32 nDelayTime = frm_delay_time_;

  memset(&can_data, 0, sizeof(can_data));
  can_data.frame.can_id = MAKE_CAN_ID(id, frame_type_index_, 0, 0);

  QStringList data_list = datas_.split(' ');
  quint8 len = data_list.length() > CAN_MAX_DLEN ? CAN_MAX_DLEN : (BYTE)data_list.length();
  for(quint32 i = 0; i < len; i++)
  {
    can_data.frame.data[i] = (BYTE)data_list[i].toUShort(&ok, 16);
  }

  can_data.frame.can_dlc = len;
  can_data.transmit_type = send_type_index_;
  if(add_delay_flag && bDelay)
  {
    can_data.frame.__pad |= TX_DELAY_SEND_FLAG;
    can_data.frame.__res0 = (BYTE)(nDelayTime & 0xFF);
    can_data.frame.__res1 = (BYTE)((nDelayTime >> 8) & 0xFF);
  }
}

void can_driver::can_frame_packed(ZCAN_TransmitFD_Data &canfd_data, bool add_delay_flag)
{
  bool ok;
  quint32 id = id_.toUInt(&ok, 16);
  bool bDelay = frm_delay_flag_;
  quint32 nDelayTime = frm_delay_time_;

  memset(&canfd_data, 0, sizeof(canfd_data));
  canfd_data.frame.can_id = MAKE_CAN_ID(id, frame_type_index_, 0, 0);

  QStringList data_list = datas_.split(' ');
  quint8 len = data_list.length() > CANFD_MAX_DLEN ? CANFD_MAX_DLEN : (BYTE)data_list.length();
  for(quint32 i = 0; i < len; i++)
  {
    canfd_data.frame.data[i] = (BYTE)data_list[i].toUShort(&ok, 16);
  }

  canfd_data.frame.len = len;
  canfd_data.transmit_type = send_type_index_;
  canfd_data.frame.flags |= canfd_exp_index_ ? CANFD_BRS : 0;
  if(add_delay_flag && bDelay)
  {
    canfd_data.frame.flags |= TX_DELAY_SEND_FLAG;
    canfd_data.frame.__res0 = (BYTE)(nDelayTime & 0xFF);
    canfd_data.frame.__res1 = (BYTE)((nDelayTime >> 8) & 0xFF);
  }
}


void can_driver::add_auto_can(quint32 nEnable)
{
  // TODO: Add your control notification handler code here

  ZCAN_AUTO_TRANSMIT_OBJ autoObj;
  memset(&autoObj, 0, sizeof(autoObj));
  autoObj.enable = nEnable;
  autoObj.interval = auto_send_period_;
  autoObj.index = auto_send_index_;
  can_frame_packed(autoObj.obj, false);

  char path[50] = {0};
  sprintf(path, "%d/auto_send", channel_index_);
  int nRet = ZCAN_SetValue(device_handle_, path, (const char *)&autoObj);
  QString csText;
  csText = QString::asprintf(tr("add CAN timed transmission index:%d enable:%d period:%d ms ID:0x%X [%s] ").toUtf8().data(), \
                             autoObj.index, autoObj.enable, autoObj.interval, autoObj.obj.frame.can_id, \
                             (nRet ? tr(" ok ").toUtf8().data() : tr(" faild ").toUtf8().data()));
  show_message(csText);
}

void can_driver::add_auto_can_fd(quint32 nEnable)
{
  // TODO: Add your control notification handler code here

  ZCANFD_AUTO_TRANSMIT_OBJ autoObj;
  memset(&autoObj, 0, sizeof(autoObj));
  autoObj.enable = nEnable;
  autoObj.interval = auto_send_period_;
  autoObj.index = auto_send_index_;
  can_frame_packed(autoObj.obj, false);

  char path[50] = {0};
  sprintf(path, "%d/auto_send_canfd", channel_index_);
  int nRet =  ZCAN_SetValue(device_handle_, path, (const char*)&autoObj);
  QString csText;
  csText = QString::asprintf(tr("add CANFD timed transmission index:%d enable:%d period:%d ms ID:0x%X [%s] ").toUtf8().data(), \
                             autoObj.index, autoObj.enable, autoObj.interval, autoObj.obj.frame.can_id, \
                             (nRet ? tr(" ok ").toUtf8().data() : tr(" faild ").toUtf8().data()));
  show_message(csText);
}

void can_driver::auto_send_start()
{
  // TODO: Add your control notification handler code here

  char path[50] = {0};
  sprintf(path, "%d/apply_auto_send", channel_index_);
  int nRet = ZCAN_SetValue(device_handle_, path,"0");
  QString csText = tr("start timed transmission");
  QString ret_str = nRet ? tr("[ok]") : tr("[faild]");
  csText += ret_str;
  show_message(csText);
}

void can_driver::auto_send_stop()
{
  // TODO: Add your control notification handler code here

  char path[50] = {0};
  sprintf(path, "%d/clear_auto_send", channel_index_);
  int nRet = ZCAN_SetValue(device_handle_, path, "0");
  QString csText = tr("stop timed transmission");
  QString ret_str = nRet ? tr("[ok]") : tr("[faild]");
  csText += ret_str;
  show_message(csText);
}

void can_driver::auto_send_stop_single()
{
  // TODO: Add your control notification handler code here

  if (0 == protocol_index_)
  {
    //CAN
    add_auto_can(0);
  }
  else
  {
    // CANFD
    add_auto_can_fd(0);
  }
}


void can_driver::add_auto_send()
{
  // TODO: Add your control notification handler code here

  if(0 == protocol_index_)
  {
    //CAN
    add_auto_can(1);
  }
  else
  {
    // CANFD
    add_auto_can_fd(1);
  }
}

void can_driver::show_dev_auto_send()
{
  // TODO: Add your control notification handler code here

  QString csText;
  quint32 nCount = 0;
  char path[50] = {0};

  //CAN
  sprintf(path, "%d/get_auto_send_can_count/1", channel_index_);
  const char* pRet = (const char*)ZCAN_GetValue(device_handle_,path);
  if (pRet)
  {
    nCount = *(int *)pRet;
    csText = QString(tr("CAN timed transmission num:%1").arg(nCount));
    show_message(csText);
    if (nCount > 0)
    {
      sprintf(path, "%d/get_auto_send_can_data/1", channel_index_);
      pRet = (const char*) ZCAN_GetValue(device_handle_,path);
      if (pRet)
      {
        const ZCAN_AUTO_TRANSMIT_OBJ* pData = (ZCAN_AUTO_TRANSMIT_OBJ*)pRet;
        for (quint32 i = 0; i < nCount; i++)
        {
          csText = QString::asprintf(tr("CAN timer index:%d period:%d ms ID:0x%08X").toUtf8().data(), \
                                     pData[i].index, pData[i].interval, pData[i].obj.frame.can_id);
          show_message(csText);
        }
      }
      else
      {
        csText = tr("get CAN timed transmission data faild");
        show_message(csText);
        return;
      }
    }
  }
  else
  {
    csText = tr("get CAN timed transmission num faild");
    show_message(csText);
    return;
  }


  //CANFD
  sprintf(path, "%d/get_auto_send_canfd_count/1", channel_index_);
  pRet = (const char *)ZCAN_GetValue(device_handle_,path);
  if (pRet)
  {
    nCount = *(int*)pRet;
    csText = QString(tr("CANFD timed transmission num:%1").arg(nCount));
    show_message(csText);
    if (nCount > 0)
    {
      sprintf(path, "%d/get_auto_send_canfd_data/1", channel_index_);
      pRet = (const char *)ZCAN_GetValue(device_handle_, path);
      if (pRet)
      {
        const ZCANFD_AUTO_TRANSMIT_OBJ *pData = (ZCANFD_AUTO_TRANSMIT_OBJ *)pRet;
        for (quint32 i = 0; i < nCount; i++)
        {
          csText = QString::asprintf(tr("CANFD timer index:%d period:%d ms ID:0x%08X").toUtf8().data(), \
                                     pData[i].index, pData[i].interval, pData[i].obj.frame.can_id);
          show_message(csText);
        }
      }
      else
      {
        csText = tr("get CANFD timed transmission data faild");
        show_message(csText);
        return;
      }
    }
  }
  else
  {
    csText = tr("get CANFD timed transmission num faild");
    show_message(csText);
    return;
  }
}

/******************************** End of file *********************************/
