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
  *           v1.0.1 aron566 2023.06.25 17:38 增加多品牌驱动兼容.
  *           v1.0.2 aron566 2023.06.27 19:27 增加GCcanfd驱动.
  *           v1.0.3 aron566 2023.06.29 10:28 GCcanfd驱动关闭优化避免二次关闭导致异常.
  *           v1.0.4 aron566 2023.06.29 17:16 修复GCcanfd发送帧诊断数据协议类型不对问题.
  *           v1.0.5 aron566 2023.09.01 17:55 优化can接收避免卡顿
  *           v1.0.6 aron566 2023.09.06 20:04 重构发送函数，重构ui显示的消息信号发送机制
  */
/** Includes -----------------------------------------------------------------*/
#include <QDateTime>
#include <QMessageBox>
/** Private includes ---------------------------------------------------------*/
#include "can_driver.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define USE_HW_CAN_SEND_64_DATA_EN 0/**< 是否开启非CANFD设备发送大于64Bytes的数据，采用分包发送 */
/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/

/* 设备信息 */
typedef struct _DeviceInfo
{
  const char *device_type_str;            /**< 品牌设备名 */
  can_driver::CAN_BRAND_Typedef_t brand;  /**< 品牌 */
  quint16 device_type;                    /**< 设备类型 */
  quint8 channel_count;                   /**< 设备的通道个数 */
}DEVICE_INFO_Typedef_t;

/* 列表数据需要和对话框中设备列表数据一一对应 */
static const DEVICE_INFO_Typedef_t kDeviceType[] = {
  /* 周立功can */
  {"ZCAN_USBCAN1",            can_driver::ZLG_CAN_BRAND, ZCAN_USBCAN1,            1},
  {"ZCAN_USBCAN2",            can_driver::ZLG_CAN_BRAND, ZCAN_USBCAN2,            2},
  {"ZCAN_PCI9820I",           can_driver::ZLG_CAN_BRAND, ZCAN_PCI9820I,           2},
  {"ZCAN_USBCAN_E_U",         can_driver::ZLG_CAN_BRAND, ZCAN_USBCAN_E_U,         1},
  {"ZCAN_USBCAN_2E_U",        can_driver::ZLG_CAN_BRAND, ZCAN_USBCAN_2E_U,        2},
  {"ZCAN_USBCAN_4E_U",        can_driver::ZLG_CAN_BRAND, ZCAN_USBCAN_4E_U,        4},
  {"ZCAN_PCIE_CANFD_100U",    can_driver::ZLG_CAN_BRAND, ZCAN_PCIE_CANFD_100U,    1},
  {"ZCAN_PCIE_CANFD_200U",    can_driver::ZLG_CAN_BRAND, ZCAN_PCIE_CANFD_200U,    2},
  {"ZCAN_PCIE_CANFD_400U_EX", can_driver::ZLG_CAN_BRAND, ZCAN_PCIE_CANFD_400U_EX, 4},
  {"ZCAN_USBCANFD_200U",      can_driver::ZLG_CAN_BRAND, ZCAN_USBCANFD_200U,      2},
  {"ZCAN_USBCANFD_100U",      can_driver::ZLG_CAN_BRAND, ZCAN_USBCANFD_100U,      1},
  {"ZCAN_USBCANFD_MINI",      can_driver::ZLG_CAN_BRAND, ZCAN_USBCANFD_MINI,      1},
  {"ZCAN_CANETTCP",           can_driver::ZLG_CAN_BRAND, ZCAN_CANETTCP,           1},
  {"ZCAN_CANETUDP",           can_driver::ZLG_CAN_BRAND, ZCAN_CANETUDP,           1},
  {"ZCAN_WIFICAN_TCP",        can_driver::ZLG_CAN_BRAND, ZCAN_WIFICAN_TCP,        1},
  {"ZCAN_WIFICAN_UDP",        can_driver::ZLG_CAN_BRAND, ZCAN_WIFICAN_UDP,        1},
  {"ZCAN_CLOUD",              can_driver::ZLG_CAN_BRAND, ZCAN_CLOUD,              1},
  {"ZCAN_CANFDWIFI_TCP",      can_driver::ZLG_CAN_BRAND, ZCAN_CANFDWIFI_TCP,      1},
  {"ZCAN_CANFDWIFI_UDP",      can_driver::ZLG_CAN_BRAND, ZCAN_CANFDWIFI_UDP,      1},
  {"ZCAN_CANFDNET_TCP",       can_driver::ZLG_CAN_BRAND, ZCAN_CANFDNET_TCP,       2},
  {"ZCAN_CANFDNET_UDP",       can_driver::ZLG_CAN_BRAND, ZCAN_CANFDNET_UDP,       2},
  {"ZCAN_CANFDNET_400U_TCP",  can_driver::ZLG_CAN_BRAND, ZCAN_CANFDNET_400U_TCP,  4},
  {"ZCAN_CANFDNET_400U_UDP",  can_driver::ZLG_CAN_BRAND, ZCAN_CANFDNET_400U_UDP,  4},

  /* 广成can */
  {"GC_USBCAN_II",            can_driver::GC_CAN_BRAND,  GC_USBCAN2,              2},
  {"GC_USBCANFD_II",          can_driver::GC_CAN_BRAND,  GC_USBCANFD,             2},
};

/* USBCANFD */
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
 *******************************************************************************
 */

/** Public application code --------------------------------------------------*/
/*******************************************************************************
 *
 *       Public code
 *
 *******************************************************************************
 */

can_driver::can_driver(QObject *parent)
    : QObject{parent}
{
  /* 创建访问资源锁1个 */
  sem.release(1);

  cq_obj = new CircularQueue(CircularQueue::UINT8_DATA_BUF, CircularQueue::CQ_BUF_1M, this);
  if(nullptr == cq_obj)
  {
    qDebug() << "create cq faild";
  }
}

QStringList can_driver::set_device_brand(CAN_BRAND_Typedef_t brand)
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
  /* 品牌 */
  switch(brand_)
  {
    case ZLG_CAN_BRAND:
      {
        device_handle_ = ZCAN_OpenDevice(kDeviceType[device_type_index_].device_type, device_index_, 0);
        if(INVALID_DEVICE_HANDLE == device_handle_)
        {
          show_message(tr("open zlg device faild"));
          return false;
        }
        break;
      }

    case GC_CAN_BRAND:
      {
        /* 确定型号 */
        switch(kDeviceType[device_type_index_].device_type)
        {
          case GC_USBCAN2:
            {
              if(GC_STATUS_OK != OpenDevice(kDeviceType[device_type_index_].device_type, device_index_, 0))
              {
                show_message(tr("open gc device faild"));
                return false;
              }
              break;
            }

          case GC_USBCANFD:
            {
              if(GC_CANFD_STATUS_OK != OpenDeviceFD(kDeviceType[device_type_index_].device_type, device_index_))
              {
                show_message(tr("open gc canfd device faild"));
                return false;
              }
              break;
            }

          default:
              return false;
        }
        break;
      }

    default:
      return false;
  }

  /* 发送can打开状态 */
  device_opened_ = true;
  emit signal_can_is_opened();
  show_message(tr("open device ok"));
  return true;
}

void can_driver::read_info()
{
  QString show_info;
  switch(brand_)
  {
    case ZLG_CAN_BRAND:
      {
        if(nullptr == device_handle_)
        {
          return;
        }
        ZCAN_DEVICE_INFO info;
        ZCAN_GetDeviceInf(device_handle_, &info);

        show_info += QString("<font size='5' color='green'><div align='legt'>hw_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.hw_Version, 0, 16);
        show_info += QString("<font size='5' color='green'><div align='legt'>fw_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.fw_Version, 0, 16);
        show_info += QString("<font size='5' color='green'><div align='legt'>dr_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.dr_Version, 0, 16);
        show_info += QString("<font size='5' color='green'><div align='legt'>str_Serial_Num:</div> <div align='right'>");
        char str[sizeof(info.str_Serial_Num) + 1] = {0};
        memcpy(str, info.str_Serial_Num, sizeof(info.str_Serial_Num));
        show_info += QString::asprintf("%s", str);
        show_info += QString("</div> </font>\r\n");

        show_info += QString("<font size='5' color='green'><div align='legt'>str_hw_Type:</div> <div align='right'>");
        char hwstr[sizeof(info.str_hw_Type) + 1] = {0};
        memcpy(hwstr, info.str_hw_Type, sizeof(info.str_hw_Type));
        show_info += QString::asprintf("%s", hwstr);
        show_info += QString("</div> </font>\r\n");

        show_info += QString("<font size='5' color='green'><div align='legt'>can_Num:</div> <div align='right'>%1</div> </font>\r\n").arg(info.can_Num);
        break;
      }

    case GC_CAN_BRAND:
      {
        /* 确定型号 */
        switch(kDeviceType[device_type_index_].device_type)
        {
          case GC_USBCAN2:
            {
              GC_CAN_BOARD_INFO info;
              ReadBoardInfo(kDeviceType[device_type_index_].device_type, device_index_, &info);
              show_info += QString("<font size='5' color='green'><div align='legt'>hw_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.hw_Version, 0, 16);
              show_info += QString("<font size='5' color='green'><div align='legt'>fw_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.fw_Version, 0, 16);
              show_info += QString("<font size='5' color='green'><div align='legt'>dr_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.dr_Version, 0, 16);
              show_info += QString("<font size='5' color='green'><div align='legt'>str_Serial_Num:</div> <div align='right'>");
              char str[sizeof(info.str_Serial_Num) + 1] = {0};
              memcpy(str, info.str_Serial_Num, sizeof(info.str_Serial_Num));
              show_info += QString::asprintf("%s", str);
              show_info += QString("</div> </font>\r\n");

              show_info += QString("<font size='5' color='green'><div align='legt'>str_hw_Type:</div> <div align='right'>");
              char hwstr[sizeof(info.str_hw_Type) + 1] = {0};
              memcpy(hwstr, info.str_hw_Type, sizeof(info.str_hw_Type));
              show_info += QString::asprintf("%s", hwstr);
              show_info += QString("</div> </font>\r\n");

              show_info += QString("<font size='5' color='green'><div align='legt'>can_Num:</div> <div align='right'>%1</div> </font>\r\n").arg(info.can_Num);
              break;
            }

          case GC_USBCANFD:
            {
              GC_CANFD_BOARD_INFO info;
              GetReference(kDeviceType[device_type_index_].device_type, device_index_, 0, 1, &info);
              show_info += QString("<font size='5' color='green'><div align='legt'>hw_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.hw_Version, 0, 16);
              show_info += QString("<font size='5' color='green'><div align='legt'>fw_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.fw_Version, 0, 16);
              show_info += QString("<font size='5' color='green'><div align='legt'>dr_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.dr_Version, 0, 16);
              show_info += QString("<font size='5' color='green'><div align='legt'>str_Serial_Num:</div> <div align='right'>");
              char str[sizeof(info.str_Serial_Num) + 1] = {0};
              memcpy(str, info.str_Serial_Num, sizeof(info.str_Serial_Num));
              show_info += QString::asprintf("%s", str);
              show_info += QString("</div> </font>\r\n");

              show_info += QString("<font size='5' color='green'><div align='legt'>str_hw_Type:</div> <div align='right'>");
              char hwstr[sizeof(info.str_hw_Type) + 1] = {0};
              memcpy(hwstr, info.str_hw_Type, sizeof(info.str_hw_Type));
              show_info += QString::asprintf("%s", hwstr);
              show_info += QString("</div> </font>\r\n");

              show_info += QString("<font size='5' color='green'><div align='legt'>can_Num:</div> <div align='right'>%1</div> </font>\r\n").arg(info.can_Num);
              break;
            }

          default:
              return;
        }
        break;
      }

    default:
      return;
  }

  QMessageBox message(QMessageBox::Information, tr("Info"), show_info, QMessageBox::Yes, nullptr);
  message.exec();
}

bool can_driver::init(CHANNEL_STATE_Typedef_t &channel_state)
{
  switch(brand_)
  {
    case ZLG_CAN_BRAND:
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
          /* 设置波特率 */
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
          show_message(tr("zlg can ch %1 init faild").arg(channel_state.channel_num), channel_state.channel_num);
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
        show_message(tr("zlg can ch %1 intit ok").arg(channel_state.channel_num), channel_state.channel_num);
        return true;
      }

    case GC_CAN_BRAND:
      {
        /* 确定型号 */
        switch(kDeviceType[device_type_index_].device_type)
        {
          case GC_USBCAN2:
            {
              GC_INIT_CONFIG config;
              /* 0正常模式 1为只听模式 2为自发自收模式（回环模式） */
              config.Mode = work_mode_index_;
              /* 滤波方式，0 单滤波 */
              config.Filter = 0;
              gc_can_lib_tool::get_bauds(kBaudrate[baud_index_] / 1000, &config.Timing0, &config.Timing1);
              bool ok;
              /* 验收码 */
              config.AccCode = acc_code_.toUInt(&ok, 16);
              /* 屏蔽码 */
              config.AccMask = acc_mask_.toUInt(&ok, 16);
              /* 初始化 */
              if(GC_STATUS_OK != InitCAN(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, &config))
              {
                show_message(tr("gc can ch %1 init faild").arg(channel_state.channel_num), channel_state.channel_num);
                return false;
              }
              show_message(tr("gc can ch %1 intit ok").arg(channel_state.channel_num), channel_state.channel_num);
              return true;
            }

          case GC_USBCANFD:
            {
              GC_INIT_CONFIG_FD config;
              config.CanReceMode = GLOBAL_STANDARD_AND_EXTENDED_RECEIVE;
              config.CanSendMode = PASSIVE_SEND;
              gc_canfd_lib_tool::get_bauds(kAbitTimingUSB[abit_baud_index_] / 1000,
                                           kDbitTimingUSB[abit_baud_index_] / 1000,
                                           &config.NominalBitRateSelect,
                                           &config.DataBitRateSelect);
              /* 初始化 */
              if(GC_CANFD_STATUS_OK != InitCANFD(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, &config))
              {
                show_message(tr("gc canfd ch %1 init faild").arg(channel_state.channel_num), channel_state.channel_num);
                return false;
              }
              show_message(tr("gc canfd ch %1 intit ok").arg(channel_state.channel_num), channel_state.channel_num);
              return true;
            }

          default:
              return false;
        }
        return true;
      }

    default:
      return false;
  }
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
  switch(brand_)
  {
    case ZLG_CAN_BRAND:
      {
        if(ZCAN_StartCAN(channel_state.channel_hadle) != STATUS_OK)
        {
          show_message(tr("zlg start can ch %1 faild").arg(channel_state.channel_num), channel_state.channel_num);
          return false;
        }
        show_message(tr("zlg start can ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
        return true;
      }

    case GC_CAN_BRAND:
      {
        /* 确定型号 */
        switch(kDeviceType[device_type_index_].device_type)
        {
          case GC_USBCAN2:
            {
              if(GC_STATUS_OK != StartCAN(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num))
              {
                show_message(tr("gc start can ch %1 faild").arg(channel_state.channel_num), channel_state.channel_num);
                return false;
              }
              show_message(tr("gc start can ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
              return true;
            }

          case GC_USBCANFD:
            {
              if(GC_CANFD_STATUS_OK != StartCANFD(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num))
              {
                show_message(tr("gc start canfd ch %1 faild").arg(channel_state.channel_num), channel_state.channel_num);
                return false;
              }
              show_message(tr("gc start canfd ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
              return true;
            }

          default:
              return false;
        }
        return true;
      }

    default:
      return false;
  }
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
  switch(brand_)
  {
    case ZLG_CAN_BRAND:
      {
        if(ZCAN_ResetCAN(channel_state.channel_hadle) != STATUS_OK)
        {
          show_message(tr("zlg reset can ch %1 faild ").arg(channel_state.channel_num), channel_state.channel_num);
          return false;
        }
        show_message(tr("zlg reset can ch %1 ok ").arg(channel_state.channel_num), channel_state.channel_num);
        return true;
      }

    case GC_CAN_BRAND:
      {
        /* 确定型号 */
        switch(kDeviceType[device_type_index_].device_type)
        {
          case GC_USBCAN2:
            {
              if(GC_STATUS_OK != ResetCAN(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num))
              {
                show_message(tr("gc reset can ch %1 faild").arg(channel_state.channel_num), channel_state.channel_num);
                return false;
              }
              show_message(tr("gc reset can ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
              return true;
            }

          case GC_USBCANFD:
            {
              if(GC_CANFD_STATUS_OK != ResetCANFD(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num))
              {
                show_message(tr("gc reset canfd ch %1 faild").arg(channel_state.channel_num), channel_state.channel_num);
                return false;
              }
              show_message(tr("gc reset canfd ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
              return true;
            }

          default:
              return false;
        }
        return true;
      }

    default:
      return false;
  }
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

void can_driver::close_channel(const CHANNEL_STATE_Typedef_t &channel_state)
{
  switch(brand_)
  {
    case ZLG_CAN_BRAND:
      {
        ZCAN_ResetCAN(channel_state.channel_hadle);
        show_message(tr("zlg device can ch %1 closed").arg(channel_state.channel_num), channel_state.channel_num);
        break;
      }

    case GC_CAN_BRAND:
      {
        /* 确定型号 */
        switch(kDeviceType[device_type_index_].device_type)
        {
          case GC_USBCAN2:
            {
              ClearBuffer(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num);
              ResetCAN(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num);
              show_message(tr("gc device can ch %1 closed").arg(channel_state.channel_num), channel_state.channel_num);
              break;
            }

          case GC_USBCANFD:
            {
              ResetCANFD(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num);
              StopCANFD(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num);
              show_message(tr("gc device canfd ch %1 closed").arg(channel_state.channel_num), channel_state.channel_num);
              break;
            }

          default:
              break;
        }
        break;
      }

    default:
      break;
  }
}

void can_driver::close(can_driver::CAN_BRAND_Typedef_t brand)
{
  switch(brand)
  {
    case ZLG_CAN_BRAND:
      {
        if(ZCLOUD_IsConnected())
        {
          ZCLOUD_DisconnectServer();
        }
        ZCAN_CloseDevice(device_handle_);
        break;
      }

    case GC_CAN_BRAND:
      {
        /* 确定型号 */
        switch(kDeviceType[device_type_index_].device_type)
        {
          case GC_USBCAN2:
            {
              CloseDevice(kDeviceType[device_type_index_].device_type, device_index_);
              break;
            }

          case GC_USBCANFD:
            {
              CloseDeviceFD(kDeviceType[device_type_index_].device_type, device_index_);
              break;
            }

          default:
              break;
        }
        break;
      }

    default:
      break;
  }
}

void can_driver::close()
{
  /* 保护 */
  if(false == device_opened_)
  {
    show_message(tr("device is not open "));
    return;
  }

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
  close(brand_);

  /* 发送can关闭状态 */
  emit signal_can_is_closed();

  device_opened_ = false;
}

quint32 can_driver::gc_can_send(const CHANNEL_STATE_Typedef_t &channel_state, \
                             const quint8 *data, quint8 size, quint32 id, \
                             FRAME_TYPE_Typedef_t frame_type, \
                             PROTOCOL_TYPE_Typedef_t protocol)
{
  /* 需要发送的帧数 */
  quint32 nSendCount = 1;

  /* 实际发送的帧数 */
  quint32 result = 0;

  switch(protocol)
  {
    /* can */
    case CAN_PROTOCOL_TYPE:
      {
        /* 确定型号 */
        switch(kDeviceType[device_type_index_].device_type)
        {
          case GC_USBCAN2:
            {
              GC_CAN_OBJ can_data;
              can_data.ID = id;
              can_data.SendType = send_type_index_;
              can_data.RemoteFlag = 0;
              can_data.ExternFlag = (quint8)frame_type;
              can_data.DataLen = size;
              memcpy(can_data.Data, data, size);

              if(nSendCount > 0)
              {
                GC_CAN_OBJ* pData = new GC_CAN_OBJ[nSendCount];
                for(quint32 i = 0; i < nSendCount; ++i)
                {
                  memcpy_s(&pData[i], sizeof(GC_CAN_OBJ), &can_data, sizeof(can_data));
                }

                result = Transmit(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, pData, nSendCount);
                delete [] pData;
              }
              break;
            }

          case GC_USBCANFD:
            {
              GC_CANFD_OBJ can_data;
              can_data.ID = id;
              can_data.CanORCanfdType.proto = (quint8)protocol;
              can_data.CanORCanfdType.type = GC_DATA_TYPE;
              can_data.CanORCanfdType.format = (quint8)frame_type;
              can_data.CanORCanfdType.bitratemode = BITRATESITCH_OFF;

              can_data.TimeStamp.mday = 0;
              can_data.TimeStamp.hour = 0;
              can_data.TimeStamp.minute = 0;
              can_data.TimeStamp.second = 0;
              can_data.TimeStamp.millisecond = 0;
              can_data.TimeStamp.microsecond = 0;

              can_data.DataLen = size;
              memcpy(can_data.Data, data, size);

              if(nSendCount > 0)
              {
                GC_CANFD_OBJ* pData = new GC_CANFD_OBJ[nSendCount];
                for(quint32 i = 0; i < nSendCount; ++i)
                {
                  memcpy_s(&pData[i], sizeof(GC_CANFD_OBJ), &can_data, sizeof(can_data));
                }

                if(GC_CANFD_STATUS_OK == TransmitFD(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, pData, nSendCount))
                {
                  result = nSendCount;
                }
                delete [] pData;
              }
              break;
            }

          default:
            return 0;
        }
        break;
      }

    /* canfd */
    case CANFD_PROTOCOL_TYPE:
      {
        /* 确定型号 */
        switch(kDeviceType[device_type_index_].device_type)
        {
          case GC_USBCAN2:
            {
#if USE_HW_CAN_SEND_64_DATA_EN
              /* 对64字节数据拆分8包 */
              GC_CAN_OBJ can_data;
              can_data.ID = id;
              can_data.SendType = send_type_index_;
              can_data.RemoteFlag = 0;
              can_data.ExternFlag = (quint8)frame_type;

              /* 计算分包数 */
              nSendCount = (size + 7) / 8;
              if(nSendCount > 0)
              {
                GC_CAN_OBJ *pData = new GC_CAN_OBJ[nSendCount];
                for(quint32 i = 0; i < nSendCount; ++i)
                {
                  if((i * 8 + 8) > size)
                  {
                    can_data.DataLen = size - i * 8;
                  }
                  else
                  {
                    can_data.DataLen = 8;
                  }
                  memcpy(can_data.Data, data + i * 8, can_data.DataLen);
                  memcpy_s(&pData[i], sizeof(GC_CAN_OBJ), &can_data, sizeof(can_data));
                }

                result = Transmit(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, pData, nSendCount);
                delete [] pData;
              }
              break;
#else
              return 0;
#endif
            }

          case GC_USBCANFD:
            {
              GC_CANFD_OBJ can_data;
              can_data.ID = id;
              can_data.CanORCanfdType.proto = (quint8)protocol;
              can_data.CanORCanfdType.type = GC_DATA_TYPE;
              can_data.CanORCanfdType.format = (quint8)frame_type;
              can_data.CanORCanfdType.bitratemode = BITRATESITCH_OFF;

              can_data.TimeStamp.mday = 0;
              can_data.TimeStamp.hour = 0;
              can_data.TimeStamp.minute = 0;
              can_data.TimeStamp.second = 0;
              can_data.TimeStamp.millisecond = 0;
              can_data.TimeStamp.microsecond = 0;

              can_data.DataLen = size;
              memset(can_data.Data, 0, sizeof(can_data.Data));
              memcpy(can_data.Data, data, size);
              can_data.DataLen = gc_canfd_lib_tool::get_send_len(can_data.DataLen);

              if(nSendCount > 0)
              {
                GC_CANFD_OBJ* pData = new GC_CANFD_OBJ[nSendCount];
                for(quint32 i = 0; i < nSendCount; ++i)
                {
                  memcpy_s(&pData[i], sizeof(GC_CANFD_OBJ), &can_data, sizeof(can_data));
                }

                if(GC_CANFD_STATUS_OK == TransmitFD(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, pData, nSendCount))
                {
                  result = nSendCount;
                }
                delete [] pData;
              }
              break;
            }

          default:
            return 0;
        }
        break;
      }

    default:
      return 0;
  }

  return result;
}

quint32 can_driver::zlg_can_send(const CHANNEL_STATE_Typedef_t &channel_state, \
                                 const quint8 *data, quint8 size, quint32 id, \
                                 FRAME_TYPE_Typedef_t frame_type, \
                                 PROTOCOL_TYPE_Typedef_t protocol)
{
  /* 需要发送的帧数 */
  quint32 nSendCount = 1;

  /* 实际发送的帧数 */
  quint32 result = 0;

  switch(protocol)
  {
    /* can */
    case CAN_PROTOCOL_TYPE:
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

          result = ZCAN_Transmit(channel_state.channel_hadle, pData, nSendCount);
          delete [] pData;
        }
        break;
      }

    /* canfd */
    case CANFD_PROTOCOL_TYPE:
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

          result = ZCAN_TransmitFD(channel_state.channel_hadle, pData, nSendCount);
          delete [] pData;
        }
        break;
      }

    default:
      return 0;
  }
  return result;
}

/* 发送固定协议帧长 */
bool can_driver::send(const CHANNEL_STATE_Typedef_t &channel_state, \
                      const quint8 *data, quint8 size, quint32 id, \
                      FRAME_TYPE_Typedef_t frame_type, \
                      PROTOCOL_TYPE_Typedef_t protocol)
{
  /* 需要发送的帧数 */
  quint32 nSendCount = 1;

  /* 实际发送的帧数 */
  quint32 result = 0;

  switch(brand_)
  {
    case ZLG_CAN_BRAND:
      {
        result = zlg_can_send(channel_state, data, size, id, frame_type, protocol);
        break;
      }

    case GC_CAN_BRAND:
      {
        result = gc_can_send(channel_state, data, size, id, frame_type, protocol);
        break;
      }

    default:
      return 0;
  }

  /* 消息分发到UI显示cq */
  msg_to_ui_cq_buf(id, (quint8)channel_state.channel_num, CAN_TX_DIRECT, \
                  protocol, \
                  frame_type, \
                  DATA_FRAME_TYPE, \
                  data, size);

  QString csText;
  csText = QString::asprintf(tr("send num:%d, sucess num:%d").toUtf8().data(), nSendCount, result);
  QString result_info_str;
  if(result != nSendCount)
  {
    result_info_str = tr("[%1]send data faild! ").arg(channel_state.channel_num) + csText;
  }
  else
  {
    result_info_str = tr("[%1]send data sucessful! ").arg(channel_state.channel_num) + csText;
  }
  msg_to_ui_cq_buf(id, (quint8)channel_state.channel_num, UNKNOW_DIRECT, \
                   protocol, \
                   frame_type, \
                   DATA_FRAME_TYPE, \
                   (const quint8 *)result_info_str.toUtf8().data(), result_info_str.size());

  emit signal_show_can_msg();
  return result;
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
    if((channel_num == channel_state.channel_num || 0xFFU == channel_num) \
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
  quint8 data_buf[64] = {0};
  if(nSendCount > 0)
  {
    for(quint32 i = 0; i < nSendCount; ++i)
    {
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
                        type == ZCAN_USBCANFD_MINI ||
                        type == GC_USBCANFD;
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

/* 设置自定义波特率, 需要从CANMaster目录下的baudcal生成字符串 */
bool can_driver::custom_baud_rate_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  char path[50] = {0};
  sprintf(path, "%d/baud_rate_custom", channel_state.channel_num);
  return 1 == ZCAN_SetValue(device_handle_, path, custom_baudrate_.toUtf8().data());
}

void can_driver::receice_data(const CHANNEL_STATE_Typedef_t &channel_state)
{
  switch(brand_)
  {
    case ZLG_CAN_BRAND:
      {
        ZCAN_Receive_Data can_data[100];
        ZCAN_ReceiveFD_Data canfd_data[100];
        quint32 len;
        /* 获取can数据长度 */
        len = ZCAN_GetReceiveNum(channel_state.channel_hadle, TYPE_CAN);
        if(0 < len)
        {
          len = ZCAN_Receive(channel_state.channel_hadle, can_data, 100, 0);
          show_message(channel_state, can_data, len);
          emit signal_show_can_msg();
        }

        /* 获取canfd数据长度 */
        len = ZCAN_GetReceiveNum(channel_state.channel_hadle, TYPE_CANFD);
        if(0 < len)
        {
          len = ZCAN_ReceiveFD(channel_state.channel_hadle, canfd_data, 100, 0);
          show_message(channel_state, canfd_data, len);
          emit signal_show_can_msg();
        }
        break;
      }

    case GC_CAN_BRAND:
      {
        /* 确定型号 */
        switch(kDeviceType[device_type_index_].device_type)
        {
          case GC_USBCAN2:
            {
              ulong len;
              GC_ERR_INFO vei;
              GC_CAN_OBJ can_data[100];
              len = GetReceiveNum(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num);
              if(0 < len)
              {
                len = Receive(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, can_data, 100, 0);
                if(4294967295 == len)
                {
                  if(STATUS_ERR != ReadErrInfo(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, &vei))
                  {
                    qDebug() << "gc read data err" << "err code:" << QString::number(vei.ErrCode, 16);
                  }
                  break;
                }
                show_message(channel_state, can_data, len, CAN_RX_DIRECT);
                emit signal_show_can_msg();
              }
              break;
            }

          case GC_USBCANFD:
            {
              GC_CANFD_OBJ can_data[100];
              ulong len = 100;
              ReceiveFD(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, can_data, &len);
              if(0 < len)
              {
                show_message(channel_state, can_data, (quint32)len, CAN_RX_DIRECT);
                emit signal_show_can_msg();
              }
              break;
            }

          default:
              return;
        }
        break;
      }

    default:
      break;
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

void can_driver::add_msg_filter(quint32 can_id, CircularQueue *cq_obj_, quint8 channel_)
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

void can_driver::msg_to_cq_buf(quint32 can_id, quint8 channel_num, const quint8 *data, quint32 data_len)
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
    CircularQueue::CQ_putData(msg_filter_list.value(index).cq_obj->get_cq_handle(), data, data_len);
  }
}

void can_driver::msg_to_ui_cq_buf(quint32 can_id, quint8 channel_num, CAN_DIRECT_Typedef_t direction, \
                                  PROTOCOL_TYPE_Typedef_t protocol, FRAME_TYPE_Typedef_t frame_type, \
                                  FRAME_DATA_TYPE_Typedef_t frame_data_type, \
                                  const quint8 *data, quint8 data_len)
{
  quint32 id_temp;
  id_temp = (can_id & can_id_mask_);
  if(nullptr == cq_obj || (can_id != id_temp && false != can_id_mask_en_))
  {
    return;
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

  sem.tryAcquire();
  if(true == CircularQueue::CQ_canSaveLength(cq_obj->get_cq_handle(), sizeof(ui_msg)))
  {
    CircularQueue::CQ_putData(cq_obj->get_cq_handle(), (quint8 *)&ui_msg, sizeof(ui_msg));
  }
  sem.release();

  QDateTime dt = QDateTime::currentDateTime();
  emit signal_can_driver_msg(can_id, data, data_len, (quint8)direction, channel_num, (quint8)protocol, dt.toMSecsSinceEpoch());
}

void can_driver::show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_Receive_Data *data, quint32 len)
{
  QString item;
  quint16 can_id = 0;
  for(quint32 i = 0; i < len; ++i)
  {
    const ZCAN_Receive_Data& can = data[i];
    const canid_t& id = can.frame.can_id;
    const bool is_eff = IS_EFF(id);
    const bool is_rtr = IS_RTR(id);
    can_id = GET_ID(id);
    item = QString::asprintf(tr("[%u]Rx CAN ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
                             channel_state.channel_num, \
                             can_id, is_eff ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data(), \
                             is_rtr ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), \
                             can.frame.can_dlc);
    for(quint32 i = 0; i < can.frame.can_dlc; ++i)
    {
      item += QString::asprintf("%02X ", can.frame.data[i]);
    }

    /* 消息分发到UI显示cq */
    msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_RX_DIRECT, \
                     CAN_PROTOCOL_TYPE, is_eff ? EXT_FRAME_TYPE : STD_FRAME_TYPE, \
                     is_rtr ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE, \
                     can.frame.data, can.frame.can_dlc);

    /* 消息过滤分发 */
    msg_to_cq_buf(can_id, (quint8)channel_state.channel_num, can.frame.data, can.frame.can_dlc);
  }
}

void can_driver::show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_ReceiveFD_Data *data, quint32 len)
{
  QString item;
  quint16 can_id = 0;
  for(quint32 i = 0; i < len; ++i)
  {
    const ZCAN_ReceiveFD_Data& canfd = data[i];
    const canid_t& id = canfd.frame.can_id;
    can_id = GET_ID(id);
    item = QString::asprintf(tr("[%u]Rx CANFD ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
            channel_state.channel_num, can_id, IS_EFF(id) ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data()
        , IS_RTR(id) ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), canfd.frame.len);
    for (quint32 i = 0; i < canfd.frame.len; ++i)
    {
      item += QString::asprintf("%02X ", canfd.frame.data[i]);
    }

    /* 消息分发到UI显示cq */
    msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_RX_DIRECT, \
                     CANFD_PROTOCOL_TYPE, IS_EFF(id) ? EXT_FRAME_TYPE : STD_FRAME_TYPE, \
                     IS_RTR(id) ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE, \
                     canfd.frame.data, canfd.frame.len);

    /* 消息过滤分发 */
    msg_to_cq_buf(can_id, (quint8)channel_state.channel_num, canfd.frame.data, canfd.frame.len);
  }
}

void can_driver::show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_Transmit_Data *data, quint32 len)
{
  Q_UNUSED(channel_state)
  Q_UNUSED(data)
  Q_UNUSED(len)
#if 0
  QString item;
  quint16 can_id = 0;
  for(quint32 i = 0; i < len; ++i)
  {
    const ZCAN_Transmit_Data& can = data[i];
    const canid_t &id = can.frame.can_id;
    can_id = GET_ID(id);
    item = QString::asprintf(tr("[%u]Tx CAN ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
                             channel_state.channel_num, \
                             can_id, IS_EFF(id) ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data(), \
          IS_RTR(id) ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), can.frame.can_dlc);
    for(quint32 i = 0; i < can.frame.can_dlc; ++i)
    {
      item += QString::asprintf("%02X ", can.frame.data[i]);
    }

    /* 消息分发到UI显示cq */
    msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_TX_DIRECT, \
                     CAN_PROTOCOL_TYPE, IS_EFF(id) ? EXT_FRAME_TYPE : STD_FRAME_TYPE, \
                     IS_RTR(id) ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE, \
                     can.frame.data, can.frame.can_dlc);
  }
#endif
}

void can_driver::show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_TransmitFD_Data *data, quint32 len)
{
  Q_UNUSED(channel_state)
  Q_UNUSED(data)
  Q_UNUSED(len)
#if 0
  QString item;
  quint16 can_id = 0;
  for(quint32 i = 0; i < len; ++i)
  {
    const ZCAN_TransmitFD_Data& can = data[i];
    const canid_t &id = can.frame.can_id;
    can_id = GET_ID(id);
    item = QString::asprintf(tr("[%u]Tx CANFD ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
                             channel_state.channel_num, \
                             can_id, IS_EFF(id) ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data(), \
          IS_RTR(id) ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), can.frame.len);
    for(quint32 i = 0; i < can.frame.len; ++i)
    {
      item += QString::asprintf("%02X ", can.frame.data[i]);
    }

    /* 消息分发到UI显示cq */
    msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_TX_DIRECT, \
                     CANFD_PROTOCOL_TYPE, IS_EFF(id) ? EXT_FRAME_TYPE : STD_FRAME_TYPE, \
                     IS_RTR(id) ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE, \
                     can.frame.data, can.frame.len);
  }
#endif
}

void can_driver::show_message(const CHANNEL_STATE_Typedef_t &channel_state, const GC_CAN_OBJ *data, quint32 len, CAN_DIRECT_Typedef_t dir)
{
  switch(dir)
  {
    case CAN_TX_DIRECT:
      {
#if 0
        QString item;
        quint32 can_id = 0;
        for(quint32 i = 0; i < len; ++i)
        {
          const GC_CAN_OBJ& can = data[i];
          const canid_t &id = can.ID;
          const bool is_eff = can.ExternFlag;
          const bool is_rtr = can.RemoteFlag;
          can_id = GET_ID(id);
          item = QString::asprintf(tr("[%u]Tx CAN ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
                                   channel_state.channel_num, \
                                   can_id, is_eff ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data(), \
                is_rtr ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), can.DataLen);
          for(quint32 i = 0; i < can.DataLen; ++i)
          {
            item += QString::asprintf("%02X ", can.Data[i]);
          }

          /* 消息分发到UI显示cq */
          msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_TX_DIRECT, \
                           CAN_PROTOCOL_TYPE, is_eff ? EXT_FRAME_TYPE : STD_FRAME_TYPE, \
                           is_rtr ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE, \
                           can.Data, can.DataLen);
        }
#endif
        break;
      }

    case CAN_RX_DIRECT:
      {
        QString item;
        quint32 can_id = 0;
        for(quint32 i = 0; i < len; ++i)
        {
          const GC_CAN_OBJ& can = data[i];
          const canid_t& id = can.ID;
          const bool is_eff = can.ExternFlag;
          const bool is_rtr = can.RemoteFlag;
          can_id = GET_ID(id);
          item = QString::asprintf(tr("[%u]Rx CAN ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
                                   channel_state.channel_num, \
                                   can_id, is_eff ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data(), \
                                   is_rtr ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), \
                                   can.DataLen);
          for(quint32 i = 0; i < can.DataLen; ++i)
          {
            item += QString::asprintf("%02X ", can.Data[i]);
          }

          /* 消息分发到UI显示cq */
          msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_RX_DIRECT, \
                           CAN_PROTOCOL_TYPE, is_eff ? EXT_FRAME_TYPE : STD_FRAME_TYPE, \
                           is_rtr ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE, \
                           can.Data, can.DataLen);

          /* 消息过滤分发 */
          msg_to_cq_buf(can_id, (quint8)channel_state.channel_num, can.Data, can.DataLen);
        }
        break;
      }

    case UNKNOW_DIRECT:
    default:
      return;
  }
}

void can_driver::show_message(const CHANNEL_STATE_Typedef_t &channel_state, const GC_CANFD_OBJ *data, quint32 len, CAN_DIRECT_Typedef_t dir)
{
  switch(dir)
  {
    case CAN_TX_DIRECT:
      {
#if 0
        QString item;
        quint16 can_id = 0;
        for(quint32 i = 0; i < len; ++i)
        {
          const GC_CANFD_OBJ& can = data[i];
          const canid_t &id = can.ID;
          const bool is_eff = can.CanORCanfdType.format;
          const bool is_rtr = can.CanORCanfdType.type;
          const bool is_fd = can.CanORCanfdType.proto;
          can_id = GET_ID(id);
          item = QString::asprintf(tr("[%u]Tx CAN%s ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
                                   channel_state.channel_num, \
                                   is_fd ? tr("FD").toUtf8().data() : tr("").toUtf8().data(), \
                                   can_id, is_eff ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data(), \
                is_rtr ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), can.DataLen);
          for(quint32 i = 0; i < can.DataLen; ++i)
          {
            item += QString::asprintf("%02X ", can.Data[i]);
          }

          /* 消息分发到UI显示cq */
          msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_TX_DIRECT, \
                           is_fd ? CANFD_PROTOCOL_TYPE : CAN_PROTOCOL_TYPE, \
                           is_eff ? EXT_FRAME_TYPE : STD_FRAME_TYPE, \
                           is_rtr ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE, \
                           can.Data, can.DataLen);
        }
#endif
        break;
      }

    case CAN_RX_DIRECT:
      {
        QString item;
        quint16 can_id = 0;
        for(quint32 i = 0; i < len; ++i)
        {
          const GC_CANFD_OBJ& can = data[i];
          const canid_t& id = can.ID;
          const bool is_eff = can.CanORCanfdType.format;
          const bool is_rtr = can.CanORCanfdType.type;
          const bool is_fd = can.CanORCanfdType.proto;
          can_id = GET_ID(id);
          item = QString::asprintf(tr("[%u]Rx CAN%s ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
                                   channel_state.channel_num, \
                                   is_fd ? tr("FD").toUtf8().data() : tr("").toUtf8().data(), \
                                   can_id, is_eff ? tr("EXT_FRAME").toUtf8().data() : tr("STD_FRAME").toUtf8().data(), \
                                   is_rtr ? tr("REMOTE_FRAME").toUtf8().data() : tr("DATA_FRAME").toUtf8().data(), \
                                   can.DataLen);
          for(quint32 i = 0; i < can.DataLen; ++i)
          {
            item += QString::asprintf("%02X ", can.Data[i]);
          }

          /* 消息分发到UI显示cq */
          msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_RX_DIRECT, \
                           is_fd ? CANFD_PROTOCOL_TYPE : CAN_PROTOCOL_TYPE, \
                           is_eff ? EXT_FRAME_TYPE : STD_FRAME_TYPE, \
                           is_rtr ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE, \
                           can.Data, can.DataLen);

          /* 消息过滤分发 */
          msg_to_cq_buf(can_id, (quint8)channel_state.channel_num, can.Data, can.DataLen);
        }
        break;
      }

    case UNKNOW_DIRECT:
    default:
      return;
  }
}

void can_driver::show_message(const QString &str, quint32 channel_num, \
                              CAN_DIRECT_Typedef_t direct, const quint8 *data, quint32 data_len, quint32 can_id, bool thread_mode)
{
  message = str;
  /* 输出到显示框 */
  if(false == thread_mode)
  {
    emit signal_show_message(message, channel_num, (quint8)direct, data, data_len, can_id);
  }
  else
  {
    emit signal_show_thread_message(message, channel_num, (quint8)direct, data, data_len, can_id);
    return;
  }
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

/* 设置终端电阻使能 */
bool can_driver::resistance_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  char path[50] = {0};
  sprintf(path, "%d/initenal_resistance", channel_state.channel_num);
  char value[10] = {0};
  sprintf(value, "%d", resistance_enable_);
  return 1 == ZCAN_SetValue(device_handle_, path, value);
}

/* 设置CAN卡波特率 */
bool can_driver::baud_rate_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  qDebug() << "/baud_rate_config";
  char path[50] = {0};
  sprintf(path, "%d/baud_rate", channel_state.channel_num);
  char value[10] = {0};
  sprintf(value, "%d", kBaudrate[baud_index_]);
  return 1 == ZCAN_SetValue(device_handle_, path, value);
}

/* 设置USBCANFD卡波特率 */
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
  csRet = QString::asprintf(tr("[%s]").toUtf8().data(), \
          nRet > 0 ? tr(" ok ").toUtf8().data() : tr(" faild ").toUtf8().data());
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
  bool bDelay = frm_delay_flag_;
  quint32 nDelayTime = frm_delay_time_;

  memset(&can_data, 0, sizeof(can_data));
  if(true && bDelay)
  {
    can_data.frame.__pad |= TX_DELAY_SEND_FLAG;
    can_data.frame.__res0 = (BYTE)(nDelayTime & 0xFF);
    can_data.frame.__res1 = (BYTE)((nDelayTime >> 8) & 0xFF);
  }
  can_data.frame.can_id = MAKE_CAN_ID(id, frame_type, 0, 0);

  quint8 len = size > CAN_MAX_DLEN ? CAN_MAX_DLEN : (BYTE)size;
  memcpy_s(can_data.frame.data, sizeof(can_data.frame.data), data, len);

  can_data.frame.can_dlc = len;
  can_data.transmit_type = send_type_index_;
}

void can_driver::can_frame_packed(ZCAN_TransmitFD_Data &canfd_data, quint32 id, quint32 frame_type, const quint8 *data, quint8 size)
{
  memset(&canfd_data, 0, sizeof(canfd_data));
  bool bDelay = frm_delay_flag_;
  quint32 nDelayTime = frm_delay_time_;
  if(true && bDelay)
  {
    canfd_data.frame.flags |= TX_DELAY_SEND_FLAG;
    canfd_data.frame.__res0 = (BYTE)(nDelayTime & 0xFF);
    canfd_data.frame.__res1 = (BYTE)((nDelayTime >> 8) & 0xFF);
  }
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
