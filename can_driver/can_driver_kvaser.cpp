/**
 *  @file can_driver_kvaser.cpp
 *
 *  @date 2023年11月01日 11:11:54 星期三
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief kvaser驱动.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.11.01 11:11 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-11-01 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
/** Private includes ---------------------------------------------------------*/
#include "can_driver_kvaser.h"
#include <QDateTime>
#include <QMessageBox>
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define USE_HW_CAN_SEND_64_DATA_EN  1/**< 是否开启非CANFD设备发送大于64Bytes的数据，采用分包发送 */
#define CAN_MSG_NUM_MAX             64U/**< can接收一次消息最大允许帧数 */
/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/

/* 列表数据需要和对话框中设备列表数据一一对应 */
static const can_driver_model::DEVICE_INFO_Typedef_t kDeviceType[] = {
  /* kvasercan */
  {"VIRTUAL",            can_driver_model::KVASER_BRAND, canHWTYPE_VIRTUAL        , 1},///< The virtual CAN bus
  {"LAPCAN",             can_driver_model::KVASER_BRAND, canHWTYPE_LAPCAN         , 1},///< LAPcan Family
  {"CANPARI",            can_driver_model::KVASER_BRAND, canHWTYPE_CANPARI        , 1},///< CANpari (obsolete).
  {"PCCAN",              can_driver_model::KVASER_BRAND, canHWTYPE_PCCAN          , 1},///< PCcan Family
  {"PCICAN",             can_driver_model::KVASER_BRAND, canHWTYPE_PCICAN         , 1},///< PCIcan Family
  {"USBCAN",             can_driver_model::KVASER_BRAND, canHWTYPE_USBCAN         , 1},///< USBcan (obsolete).
  {"PCICAN_II",          can_driver_model::KVASER_BRAND, canHWTYPE_PCICAN_II      , 1},///< PCIcan II family
  {"USBCAN_II",          can_driver_model::KVASER_BRAND, canHWTYPE_USBCAN_II      , 1},///< USBcan II, USBcan Rugged, Kvaser Memorator
  {"SIMULATED",          can_driver_model::KVASER_BRAND, canHWTYPE_SIMULATED      , 1},///< Simulated CAN bus for Kvaser Creator (obsolete).
  {"ACQUISITOR",         can_driver_model::KVASER_BRAND, canHWTYPE_ACQUISITOR     , 1},///< Kvaser Acquisitor (obsolete).
  {"LEAF",               can_driver_model::KVASER_BRAND, canHWTYPE_LEAF           , 1},///< Kvaser Leaf Family
  {"PC104_PLUS",         can_driver_model::KVASER_BRAND, canHWTYPE_PC104_PLUS     , 1},///< Kvaser PC104+
  {"PCICANX_II",         can_driver_model::KVASER_BRAND, canHWTYPE_PCICANX_II     , 1},///< Kvaser PCIcanx II
  {"MEMORATOR_II",       can_driver_model::KVASER_BRAND, canHWTYPE_MEMORATOR_II   , 1},///< Kvaser Memorator Professional family
  {"MEMORATOR_PRO",      can_driver_model::KVASER_BRAND, canHWTYPE_MEMORATOR_PRO  , 1},///< Kvaser Memorator Professional family
  {"USBCAN_PRO",         can_driver_model::KVASER_BRAND, canHWTYPE_USBCAN_PRO     , 1},///< Kvaser USBcan Professional
  {"IRIS",               can_driver_model::KVASER_BRAND, canHWTYPE_IRIS           , 1},///< Obsolete name, use canHWTYPE_BLACKBIRD instead
  {"BLACKBIRD",          can_driver_model::KVASER_BRAND, canHWTYPE_BLACKBIRD      , 1},///< Kvaser BlackBird
  {"MEMORATOR_LIGHT",    can_driver_model::KVASER_BRAND, canHWTYPE_MEMORATOR_LIGHT, 1},///< Kvaser Memorator Light
  {"MINIHYDRA",          can_driver_model::KVASER_BRAND, canHWTYPE_MINIHYDRA      , 1},///< Obsolete name, use canHWTYPE_EAGLE instead
  {"EAGLE",              can_driver_model::KVASER_BRAND, canHWTYPE_EAGLE          , 1},///< Kvaser Eagle family
  {"BAGEL",              can_driver_model::KVASER_BRAND, canHWTYPE_BAGEL          , 1},///< Obsolete name, use canHWTYPE_BLACKBIRD_V2 instead
  {"BLACKBIRD_V2",       can_driver_model::KVASER_BRAND, canHWTYPE_BLACKBIRD_V2   , 1},///< Kvaser BlackBird v2
  {"MINIPCIE",           can_driver_model::KVASER_BRAND, canHWTYPE_MINIPCIE       , 1},///< Kvaser Mini PCI Express
  {"USBCAN_KLINE",       can_driver_model::KVASER_BRAND, canHWTYPE_USBCAN_KLINE   , 1},///< USBcan Pro HS/K-Line
  {"ETHERCAN",           can_driver_model::KVASER_BRAND, canHWTYPE_ETHERCAN       , 1},///< Kvaser Ethercan
  {"USBCAN_LIGHT",       can_driver_model::KVASER_BRAND, canHWTYPE_USBCAN_LIGHT   , 1},///< Kvaser USBcan Light
  {"USBCAN_PRO2",        can_driver_model::KVASER_BRAND, canHWTYPE_USBCAN_PRO2    , 5},///< Kvaser USBcan Pro 5xHS and variants
  {"PCIE_V2",            can_driver_model::KVASER_BRAND, canHWTYPE_PCIE_V2        , 2},///< Kvaser PCIEcan 4xHS and variants 2个虚拟通道不可用
  {"MEMORATOR_PRO2",     can_driver_model::KVASER_BRAND, canHWTYPE_MEMORATOR_PRO2 , 5},///< Kvaser Memorator Pro 5xHS and variants
  {"LEAF2",              can_driver_model::KVASER_BRAND, canHWTYPE_LEAF2          , 2},///< Kvaser Leaf Pro HS v2 and variants
  {"MEMORATOR_V2",       can_driver_model::KVASER_BRAND, canHWTYPE_MEMORATOR_V2   , 1},///< Kvaser Memorator (2nd generation)
  {"CANLINHYBRID",       can_driver_model::KVASER_BRAND, canHWTYPE_CANLINHYBRID   , 1},///< Kvaser Hybrid CAN/LIN
  {"DINRAIL",            can_driver_model::KVASER_BRAND, canHWTYPE_DINRAIL        , 1},///< Kvaser DIN Rail SE400S and variants
  {"U100",               can_driver_model::KVASER_BRAND, canHWTYPE_U100           , 1},///< Kvaser U100 and variants
  {"LEAF3",              can_driver_model::KVASER_BRAND, canHWTYPE_LEAF3          , 1},///< Kvaser Kvaser Leaf v3 and variants
};

/* USBCANFD */
static const quint32 kAbitTimingUSB[] = {
  8000070U,//8Mbps 70P
  8000080U,//8Mbps 80P
  8000060U,//8Mbps 60P
  4000000U,//4Mbps 80P
  2000060U,//2Mbps 60P
  2000080U,//2Mbps 80P
  1000000U,//1Mbps 80P
  500000U, //500Kbps 80P
};

static const char *kAbitTimingSample[] = {
  "70P",
  "80P",
  "60P",
  "80P",
  "60P",
  "80P",
  "80P",
  "80P",
};

static const quint32 kDbitTimingUSB[] = {
  8000070U,//8Mbps 70P
  8000080U,//8Mbps 80P
  8000060U,//8Mbps 60P
  4000000U,//4Mbps 80P
  2000060U,//2Mbps 60P
  2000080U,//2Mbps 80P
  1000000U,//1Mbps 80P
  500000U, //500Kbps 80P
};

static const char *kDbitTimingSample[] = {
  "70P",
  "80P",
  "60P",
  "80P",
  "60P",
  "80P",
  "80P",
  "80P",
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

can_driver_kvaser::can_driver_kvaser(QObject *parent)
    : can_driver_model{parent}
{

}

can_driver_model::SET_FUNCTION_CAN_USE_Typedef_t can_driver_kvaser::function_can_use_update_for_choose(const QString &device_type_str)
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
  const bool usbcanfd = type == canHWTYPE_PCIE_V2;
  const bool pciecanfd = type == canHWTYPE_PCIE_V2;

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
  function_can_use.diy_bauds_can_use = false;

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
    function_can_use.abitrate_list.append(bps_number2str(kAbitTimingUSB[i]) + kAbitTimingSample[i]);
  }
  for(quint16 i = 0; i < sizeof(kDbitTimingUSB) / sizeof(kDbitTimingUSB[0]); i++)
  {
    function_can_use.datarate_list.append(bps_number2str(kDbitTimingUSB[i]) + kDbitTimingSample[i]);
  }
  for(quint16 i = 0; i < sizeof(kBaudrate) / sizeof(kBaudrate[0]); i++)
  {
    function_can_use.baudrate_list.append(bps_number2str(kBaudrate[i]));
  }

  return function_can_use;
}

void can_driver_kvaser::show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const KVASER_CAN_OBJ_Typedef_t *data, quint32 len, CAN_DIRECT_Typedef_t dir)
{
  switch(dir)
  {
    case CAN_TX_DIRECT:
      {
        break;
      }

    case CAN_RX_DIRECT:
      {
        quint16 can_id = 0;
        for(quint32 i = 0; i < len; ++i)
        {
          const quint32 id = data[i].id;
          const bool is_eff = data[i].extern_flag;
          const bool is_rtr = data[i].remote_flag;
          const bool is_fd = data[i].is_canfd_flag;
          can_id = (id & 0x1FFFFFFFU);

          /* 消息分发到UI显示cq */
          msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_RX_DIRECT, \
                           is_fd ? CANFD_PROTOCOL_TYPE : CAN_PROTOCOL_TYPE, \
                           is_eff ? EXT_FRAME_TYPE : STD_FRAME_TYPE, \
                           is_rtr ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE, \
                           data[i].data, data[i].data_len);

          /* 消息过滤分发 */
          msg_to_cq_buf(can_id, (quint8)channel_state.channel_num, data[i].data, data[i].data_len);
        }
        break;
      }

    case UNKNOW_DIRECT:
    default:
      return;
  }
}

void can_driver_kvaser::check_for_error(const char *cmd, canStatus stat)
{
  /* if stat not ok, print error */
  if (stat != canOK)
  {
    char buf[255];
    buf[0] = '\0';
    p_canGetErrorText(stat, buf, sizeof(buf));

    qDebug("[%s] %s: failed, stat=%d\n", cmd, buf, (int)stat);
  }
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

QStringList can_driver_kvaser::set_device_brand(CAN_BRAND_Typedef_t brand)
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

quint8 can_driver_kvaser::set_device_type(const QString &device_type_str)
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

bool can_driver_kvaser::open()
{
  /* 设置动态库搜索路径 */
  QString exePath = QCoreApplication::applicationDirPath();
  QString libraryPath;

  /* kvaser */
  if (QSysInfo::WordSize == 64)
  {
    /* 64位平台 */
    libraryPath = exePath + "/kvaser_can_x64/kvalapw2.dll";
  }
  else
  {
    /* 32位平台 */
    libraryPath = exePath + "/kvaser_can_x86/kvalapw2.dll";
  }

  /* 首先加载 */
  myLibrary.setFileName(libraryPath);
  if(false == myLibrary.load())
  {
    qDebug() << "kvaser 动态库加载失败：" << myLibrary.errorString();
    return false;
  }

  /* 最后加载 */
  if (QSysInfo::WordSize == 64)
  {
    /* 64位平台 */
    libraryPath = exePath + "/kvaser_can_x64/canlib32.dll";
  }
  else
  {
    /* 32位平台 */
    libraryPath = exePath + "/kvaser_can_x86/canlib32.dll";
  }
  myLibrary.setFileName(libraryPath);
  if(false == myLibrary.load())
  {
    qDebug() << "kvaser 动态库加载失败：" << myLibrary.errorString();
    return false;
  }

  qDebug() << "kvaser 动态库加载成功！";

  p_canInitializeLibrary = (canInitializeLibrary_t)myLibrary.resolve("canInitializeLibrary");
  p_canGetNumberOfChannels = (canGetNumberOfChannels_t)myLibrary.resolve("canGetNumberOfChannels");
  if(p_canGetNumberOfChannels != nullptr)
  {
//    show_message(tr("p_canGetNumberOfChannels load ok"));
  }
  p_canGetChannelData = (canGetChannelData_t)myLibrary.resolve("canGetChannelData");
  if(p_canGetChannelData != nullptr)
  {
//    show_message(tr("p_canGetChannelData load ok"));
  }
  p_canOpenChannel = (canOpenChannel_t)myLibrary.resolve("canOpenChannel");
  if(p_canOpenChannel != nullptr)
  {
//    show_message(tr("p_canOpenChannel load ok"));
  }
  p_canGetErrorText = (canGetErrorText_t)myLibrary.resolve("canGetErrorText");
  if(p_canGetErrorText != nullptr)
  {
//    show_message(tr("p_canGetErrorText load ok"));
  }
  p_canIoCtl = (canIoCtl_t)myLibrary.resolve("canIoCtl");
  if(p_canIoCtl != nullptr)
  {
//    show_message(tr("p_canIoCtl load ok"));
  }
  p_canSetBusOutputControl = (canSetBusOutputControl_t)myLibrary.resolve("canSetBusOutputControl");
  if(p_canSetBusOutputControl != nullptr)
  {
//    show_message(tr("p_canSetBusOutputControl load ok"));
  }
  p_canSetBusParams = (canSetBusParams_t)myLibrary.resolve("canSetBusParams");
  if(p_canSetBusParams != nullptr)
  {
//    show_message(tr("p_canSetBusParams load ok"));
  }
  p_canSetBusParamsFd = (canSetBusParamsFd_t)myLibrary.resolve("canSetBusParamsFd");
  if(p_canSetBusParamsFd != nullptr)
  {
//    show_message(tr("p_canSetBusParamsFd load ok"));
  }
  p_canBusOn = (canBusOn_t)myLibrary.resolve("canBusOn");
  if(p_canBusOn != nullptr)
  {
//    show_message(tr("p_canBusOn load ok"));
  }
  p_canBusOff = (canBusOff_t)myLibrary.resolve("canBusOff");
  if(p_canBusOff != nullptr)
  {
//    show_message(tr("p_canBusOff load ok"));
  }
  p_canClose = (canClose_t)myLibrary.resolve("canClose");
  if(p_canClose != nullptr)
  {
//    show_message(tr("p_canClose load ok"));
  }
  p_canUnloadLibrary = (canUnloadLibrary_t)myLibrary.resolve("canUnloadLibrary");
  if(p_canUnloadLibrary != nullptr)
  {
//    show_message(tr("p_canUnloadLibrary load ok"));
  }
  p_canWrite = (canWrite_t)myLibrary.resolve("canWrite");
  if(p_canWrite != nullptr)
  {
//    show_message(tr("p_canWrite load ok"));
  }
  p_canWriteSync = (canWriteSync_t)myLibrary.resolve("canWriteSync");
  if(p_canWriteSync != nullptr)
  {
//    show_message(tr("p_canWriteSync load ok"));
  }
  p_canRead = (canRead_t)myLibrary.resolve("canRead");
  if(p_canRead != nullptr)
  {
//    show_message(tr("p_canRead load ok"));
  }
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    default:
      p_canInitializeLibrary();
      break;
  }

  /* 发送can打开状态 */
  device_opened_ = true;
  emit signal_can_is_opened();
  show_message(tr("open device ok"));
  return true;
}

bool can_driver_kvaser::read_info()
{
  QString show_info;

  if(nullptr == p_canGetNumberOfChannels
      || nullptr == p_canGetChannelData)
  {
    return false;
  }
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    default:
      {
        canStatus stat;
        int number_of_channels;

        int device_channel;
        char device_name[255];

        /* Get number of channels */
        stat = p_canGetNumberOfChannels(&number_of_channels);
        check_for_error("canGetNumberOfChannels", stat);

        if (number_of_channels > 0)
        {
          qDebug("%s %d %s\n", "Found", number_of_channels, "channels");
        }
        else
        {
          qDebug("Could not find any CAN interface.\n");
        }

        /* Loop and print all channels */
        for (int i = 0; i < number_of_channels; i++)
        {
          stat = p_canGetChannelData(i, canCHANNELDATA_DEVDESCR_ASCII, device_name, sizeof(device_name));
          check_for_error("canGetChannelData", stat);

          stat = p_canGetChannelData(i, canCHANNELDATA_CHAN_NO_ON_CARD, &device_channel, sizeof(device_channel));
          check_for_error("canGetChannelData", stat);

          qDebug("Found channel: %d %s %d\n", i, device_name, (device_channel + 1));
        }

        /* 获取驱动版本号 */
        quint16 dl_ver[4];
        stat = p_canGetChannelData(0, canCHANNELDATA_DLL_PRODUCT_VERSION, dl_ver, sizeof(dl_ver));
        check_for_error("canGetChannelData", stat);
        show_info += QString("<font size='5' color='green'><div align='legt'>dr_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(QString::asprintf("%u.%u.%u", dl_ver[1], dl_ver[2], dl_ver[3]));

        stat = p_canGetChannelData(0, canCHANNELDATA_CARD_SERIAL_NO, device_name, sizeof(device_name));
        check_for_error("canGetChannelData", stat);
        show_info += QString("<font size='5' color='green'><div align='legt'>str_Serial_Num:</div> <div align='right'>");
        QString serial_num = QString::asprintf("%02X%02X%02X%02X%02X%02X%02X%02X",
                                               device_name[0],
                                               device_name[1],
                                               device_name[2],
                                               device_name[3],
                                               device_name[4],
                                               device_name[5],
                                               device_name[6],
                                               device_name[7]);
        show_info += serial_num;
        show_info += QString("</div> </font>\r\n");

        /* 获取硬件类型 */
        quint32 hw_type = 0;
        stat = p_canGetChannelData(0, canCHANNELDATA_CARD_TYPE, &hw_type, sizeof(hw_type));
        check_for_error("canGetChannelData", stat);
        show_info += QString("<font size='5' color='green'><div align='legt'>str_hw_Type:</div> <div align='right'>");
        for(quint16 i = 0; i < sizeof(kDeviceType) / sizeof(kDeviceType[0]); i++)
        {
          if(kDeviceType[i].device_type == hw_type)
          {
            show_info += QString::asprintf("%s", kDeviceType[i].device_type_str);
          }
        }
        show_info += QString("</div> </font>\r\n");

        /* 通道数 */
        show_info += QString("<font size='5' color='green'><div align='legt'>can_Num:</div> <div align='right'>%1</div> </font>\r\n").arg(number_of_channels);
      }
      break;
  }
  QMessageBox message(QMessageBox::Information, tr("Info"), show_info, QMessageBox::Yes, nullptr);
  message.exec();
  return true;
}

bool can_driver_kvaser::init(CHANNEL_STATE_Typedef_t &channel_state)
{
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    default:
      {
        canStatus stat;
        canHandle hnd;

        qDebug("canOpenChannel, channel %d... ", channel_state.channel_num);
        hnd = p_canOpenChannel((qint32)channel_state.channel_num, canOPEN_CAN_FD | canOPEN_OVERRIDE_EXCLUSIVE);
        if (hnd < 0)
        {
          check_for_error("canOpenChannel", (canStatus)hnd);
          show_message(tr("kvaser canfd ch %1 init failed").arg(channel_state.channel_num), channel_state.channel_num);
          return false;
        }

        /* 清空 */
        p_canIoCtl(hnd, canIOCTL_FLUSH_TX_BUFFER, NULL, NULL);
        p_canIoCtl(hnd, canIOCTL_FLUSH_RX_BUFFER, NULL, NULL);

        /* 设置工作模式 */
        /* 0正常模式 1为只听模式 */
        p_canSetBusOutputControl(hnd, (work_mode_index_ == 0) ? canDRIVER_NORMAL : canDRIVER_SILENT);

        /*
          Using our new shiny handle, we specify the baud rate
          using one of the convenient canBITRATE_xxx constants.

          The bit layout is in depth discussed in most CAN
          controller data sheets, and on the web at
          http://www.kvaser.se.
        */
        qDebug("Setting the bus speed...");
        //set up the bus
        long m_usedBaudRate = 0;
        switch(kAbitTimingUSB[abit_baud_index_])
        {
          case 1000000U:
            m_usedBaudRate = canFD_BITRATE_1M_80P;
            break;
          case 500000U:
            m_usedBaudRate = canFD_BITRATE_500K_80P;
            break;
          case 2000080U:
            m_usedBaudRate = canFD_BITRATE_2M_80P;
            break;
          case 2000060U:
            m_usedBaudRate = canFD_BITRATE_2M_60P;
            break;
          case 4000000U:
            m_usedBaudRate = canFD_BITRATE_4M_80P;
            break;
          case 8000080U:
            m_usedBaudRate = canFD_BITRATE_8M_80P;
            break;
          case 8000060U:
            m_usedBaudRate = canFD_BITRATE_8M_60P;
            break;
          case 8000070U:
            m_usedBaudRate = canFD_BITRATE_8M_70P;
            break;
          default:
            qDebug("Baudrate set to 500 kbit/s. \n");
            m_usedBaudRate = canFD_BITRATE_500K_80P;
            break;
        }

        // set the arbitration bitrate to 500 kbit/s, with sampling point to 80%,
        // and data phase bitrate to 1000 kbit/s, with sampling point at 80%
        stat = p_canSetBusParams(hnd, m_usedBaudRate, 0, 0, 0, 0, 0);
        check_for_error("canSetBusParams", stat);

        switch(kDbitTimingUSB[abit_baud_index_])
        {
          case 1000000U:
            m_usedBaudRate = canFD_BITRATE_1M_80P;
            break;
          case 500000U:
            m_usedBaudRate = canFD_BITRATE_500K_80P;
            break;
          case 2000080U:
            m_usedBaudRate = canFD_BITRATE_2M_80P;
            break;
          case 2000060U:
            m_usedBaudRate = canFD_BITRATE_2M_60P;
            break;
          case 4000000U:
            m_usedBaudRate = canFD_BITRATE_4M_80P;
            break;
          case 8000080U:
            m_usedBaudRate = canFD_BITRATE_8M_80P;
            break;
          case 8000060U:
            m_usedBaudRate = canFD_BITRATE_8M_60P;
            break;
          case 8000070U:
            m_usedBaudRate = canFD_BITRATE_8M_70P;
            break;
          default:
            qDebug("Baudrate set to 500 kbit/s. \n");
            m_usedBaudRate = canFD_BITRATE_500K_80P;
            break;
        }
        stat = p_canSetBusParamsFd(hnd, m_usedBaudRate, 0, 0, 0);
        check_for_error("canSetBusParamsFD", stat);
        if (stat < 0)
        {
          qDebug("canSetBusParams failed, stat=%d\n", stat);
          show_message(tr("kvaser canfd ch %1 init failed").arg(channel_state.channel_num), channel_state.channel_num);
          return false;
        }
        channel_state.device_handle = hnd;
        show_message(tr("kvaser canfd ch %1 intit ok").arg(channel_state.channel_num), channel_state.channel_num);
        return true;
      }
  }
  return true;
}

bool can_driver_kvaser::init()
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

bool can_driver_kvaser::start(const CHANNEL_STATE_Typedef_t &channel_state)
{
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    default:
      {
        canStatus stat;
        stat = p_canBusOn(channel_state.device_handle);
        if (0 > stat)
        {
          qDebug("canBusOn failed, stat=%d", stat);
          show_message(tr("kvaser start can ch %1 failed").arg(channel_state.channel_num), channel_state.channel_num);
          return false;
        }
        show_message(tr("kvaser start can ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
        return true;
      }
  }
  return true;
}

bool can_driver_kvaser::start()
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

bool can_driver_kvaser::reset(const CHANNEL_STATE_Typedef_t &channel_state)
{
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    default:
      {
//        canResetBus((qint32)channel_state.device_handle);
        if(canOK != p_canBusOff((qint32)channel_state.device_handle))
        {
          show_message(tr("kvaser reset can ch %1 failed").arg(channel_state.channel_num), channel_state.channel_num);
          return false;
        }
        show_message(tr("kvaser reset can ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
        return true;
      }
  }
  return true;
}

bool can_driver_kvaser::reset()
{
  /* 复位对应通道号 */
  bool ret = false;

  start_ = false;
  send_msg_list.clear();
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

void can_driver_kvaser::close_channel(const CHANNEL_STATE_Typedef_t &channel_state)
{
  Q_UNUSED(channel_state)
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    default:
      (void)p_canBusOff((qint32)channel_state.device_handle);
      (void)p_canClose((qint32)channel_state.device_handle);
      break;
  }
}

bool can_driver_kvaser::close()
{
  /* 保护 */
  if(false == device_opened_)
  {
    show_message(tr("device is not open "));
    return false;
  }

  start_ = false;
  send_msg_list.clear();
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
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    default:
      p_canUnloadLibrary();
      break;
  }

  /* 发送can关闭状态 */
  emit signal_can_is_closed();

  device_opened_ = false;
  return true;
}

quint32 can_driver_kvaser::kvaser_can_send(const CHANNEL_STATE_Typedef_t &channel_state, \
                                   const quint8 *data, quint8 size, quint32 id, \
                                   FRAME_TYPE_Typedef_t frame_type, \
                                   PROTOCOL_TYPE_Typedef_t protocol)
{
  /* 实际发送的帧数 */
  quint32 result = 0;

  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    default:
      {
        canStatus stat;
        quint32 flags = 0;
        flags |= frame_type == STD_FRAME_TYPE ? canMSG_STD : canMSG_EXT;
        flags |= protocol == CAN_PROTOCOL_TYPE ? 0 : canFDMSG_FDF;

        /* 长度对齐 */
        /* 0-8, 12, 16, 20, 24, 32, 48, 64. */
        if(CANFD_PROTOCOL_TYPE == protocol)
        {
          size = can_driver_kvaser::get_send_len(size);
        }
        stat = p_canWrite((qint32)channel_state.device_handle, id, (void *)data, size, flags);
        if (stat < 0)
        {
          qDebug("ERROR TransmitMessage() FAILED Err= %d <line: %d>\n", stat, __LINE__);
          return result;
        }
        // After sending, we wait for at most 100 ms for the message to be sent, using
        // canWriteSync.
        stat = p_canWriteSync((qint32)channel_state.device_handle, 100);
        check_for_error("canWriteSync", stat);
        result = 1U;
      }
      break;
  }
  return result;
}

/* 发送固定协议帧长 */
bool can_driver_kvaser::send(const CHANNEL_STATE_Typedef_t &channel_state, \
                         const quint8 *data, quint8 size, quint32 id, \
                         FRAME_TYPE_Typedef_t frame_type, \
                         PROTOCOL_TYPE_Typedef_t protocol)
{
  /* 需要发送的帧数 */
  quint32 nSendCount = 1;

  /* 实际发送的帧数 */
  quint32 result = 0;

  bool ret = false;

  result = kvaser_can_send(channel_state, data, size, id, frame_type, protocol);

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
    ret = false;
    result_info_str = tr("[%1]send data failed! ").arg(channel_state.channel_num) + csText;
  }
  else
  {
    ret = true;
    result_info_str = tr("[%1]send data sucessful! ").arg(channel_state.channel_num) + csText;
  }
  msg_to_ui_cq_buf(id, (quint8)channel_state.channel_num, UNKNOW_DIRECT, \
                   protocol, \
                   frame_type, \
                   DATA_FRAME_TYPE, \
                                    (const quint8 *)result_info_str.toUtf8().data(), result_info_str.size());
  return ret;
}

void can_driver_kvaser::function_can_use_update()
{
  quint32 type = kDeviceType[device_type_index_].device_type;
  const bool cloudDevice = false;
  const bool netcanfd = false;
  const bool netcan = false;
  const bool netDevice = (netcan || netcanfd);
  const bool tcpDevice = false;
  const bool udpDevice = false;
  const bool usbcanfd = type == canHWTYPE_PCIE_V2;
  const bool pciecanfd = type == canHWTYPE_PCIE_V2;

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

void can_driver_kvaser::receive_data(const CHANNEL_STATE_Typedef_t &channel_state)
{
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    default:
      {
        canStatus stat;
        long id;
        quint8 data[8];
        quint32 dlc;
        quint32 flags;
        DWORD time;

        stat = p_canRead((qint32)channel_state.device_handle, &id, &data[0], &dlc, &flags, &time);
        switch (stat)
        {
          case canOK:
            {
              KVASER_CAN_OBJ_Typedef_t can_data;
              can_data.remote_flag = (flags & canMSG_RTR) > 0U ? true : false;
              can_data.extern_flag = (flags & canMSG_EXT) > 0U ? true : false;
              can_data.is_canfd_flag = (flags & canFDMSG_FDF) > 0U ? true : false;
              can_data.data_len = dlc;
              can_data.id = id;
              can_data.time_stamp = time;
              memcpy_s(can_data.data, sizeof(can_data.data), data, dlc);
              if(0U < (flags & canMSG_ERROR_FRAME))
              {
//                show_message(tr("receive code %1").arg(flags));
                return;
              }
              show_rec_message(channel_state, &can_data, (quint32)1U, CAN_RX_DIRECT);
            }
            break;

          case canERR_NOMSG:
            /* No more data on this handle */
            break;

          default:
            qDebug("ERROR canRead() FAILED, Err= %d <line: %d>\n", stat, __LINE__);
            break;
        }
        break;
      }
  }
}

/******************************** End of file *********************************/
