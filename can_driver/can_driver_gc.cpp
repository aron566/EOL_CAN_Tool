/**
 *  @file can_driver_gc.cpp
 *
 *  @date 2023年10月31日 10:58:54 星期二
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief None.
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
#include "can_driver_gc.h"
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
  /* 广成can */
  {"GC_USBCAN_II",            can_driver_model::GC_CAN_BRAND,  GC_USBCAN2,              2},
  {"GC_USBCANFD_II",          can_driver_model::GC_CAN_BRAND,  GC_USBCANFD,             2},
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

can_driver_gc::can_driver_gc(QObject *parent)
    : can_driver_model{parent}
{

}

can_driver_model::SET_FUNCTION_CAN_USE_Typedef_t can_driver_gc::function_can_use_update_for_choose(const QString &device_type_str)
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
  const bool usbcanfd = type == GC_USBCANFD;
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

void can_driver_gc::show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const GC_CAN_OBJ *data, quint32 len, CAN_DIRECT_Typedef_t dir)
{
  switch(dir)
  {
    case CAN_TX_DIRECT:
      {
        break;
      }

    case CAN_RX_DIRECT:
      {
        quint32 can_id = 0;
        for(quint32 i = 0; i < len; ++i)
        {
          const GC_CAN_OBJ& can = data[i];
          const quint32& id = can.ID;
          const bool is_eff = can.ExternFlag;
          const bool is_rtr = can.RemoteFlag;
          can_id = (id & 0x1FFFFFFFU);

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

void can_driver_gc::show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const GC_CANFD_OBJ *data, quint32 len, CAN_DIRECT_Typedef_t dir)
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
          const GC_CANFD_OBJ& can = data[i];
          const quint32& id = can.ID;
          const bool is_eff = can.CanORCanfdType.format;
          const bool is_rtr = can.CanORCanfdType.type;
          const bool is_fd = can.CanORCanfdType.proto;
          can_id = (id & 0x1FFFFFFFU);

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
/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

QStringList can_driver_gc::set_device_brand(CAN_BRAND_Typedef_t brand)
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

quint8 can_driver_gc::set_device_type(const QString &device_type_str)
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

bool can_driver_gc::open()
{
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    case GC_USBCAN2:
      {
        if(GC_STATUS_OK != OpenDevice(kDeviceType[device_type_index_].device_type, device_index_, 0))
        {
          show_message(tr("open gc device failed"));
          return false;
        }
        break;
      }

    case GC_USBCANFD:
      {
        if(GC_CANFD_STATUS_OK != OpenDeviceFD(kDeviceType[device_type_index_].device_type, device_index_))
        {
          show_message(tr("open gc canfd device failed"));
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

bool can_driver_gc::read_info()
{
  QString show_info;

  if(nullptr == device_handle_)
  {
    return false;
  }
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
      return false;
  }
  QMessageBox message(QMessageBox::Information, tr("Info"), show_info, QMessageBox::Yes, nullptr);
  message.exec();
  return true;
}

bool can_driver_gc::init(CHANNEL_STATE_Typedef_t &channel_state)
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
          show_message(tr("gc can ch %1 init failed").arg(channel_state.channel_num), channel_state.channel_num);
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
          show_message(tr("gc canfd ch %1 init failed").arg(channel_state.channel_num), channel_state.channel_num);
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

bool can_driver_gc::init()
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

bool can_driver_gc::start(const CHANNEL_STATE_Typedef_t &channel_state)
{
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    case GC_USBCAN2:
      {
        if(GC_STATUS_OK != StartCAN(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num))
        {
          show_message(tr("gc start can ch %1 failed").arg(channel_state.channel_num), channel_state.channel_num);
          return false;
        }
        show_message(tr("gc start can ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
        return true;
      }

    case GC_USBCANFD:
      {
        if(GC_CANFD_STATUS_OK != StartCANFD(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num))
        {
          show_message(tr("gc start canfd ch %1 failed").arg(channel_state.channel_num), channel_state.channel_num);
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

bool can_driver_gc::start()
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

bool can_driver_gc::reset(const CHANNEL_STATE_Typedef_t &channel_state)
{
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    case GC_USBCAN2:
      {
        if(GC_STATUS_OK != ResetCAN(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num))
        {
          show_message(tr("gc reset can ch %1 failed").arg(channel_state.channel_num), channel_state.channel_num);
          return false;
        }
        show_message(tr("gc reset can ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
        return true;
      }

    case GC_USBCANFD:
      {
        if(GC_CANFD_STATUS_OK != ResetCANFD(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num))
        {
          show_message(tr("gc reset canfd ch %1 failed").arg(channel_state.channel_num), channel_state.channel_num);
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

bool can_driver_gc::reset()
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
  return ret;
}

void can_driver_gc::close_channel(const CHANNEL_STATE_Typedef_t &channel_state)
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
}

bool can_driver_gc::close()
{
  /* 保护 */
  if(false == device_opened_)
  {
    show_message(tr("device is not open "));
    return false;
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

  device_opened_ = false;
  return true;
}

quint32 can_driver_gc::gc_can_send(const CHANNEL_STATE_Typedef_t &channel_state, \
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
              can_data.CanORCanfdType.bitratemode = BITRATESITCH_ON;

              can_data.TimeStamp.mday = 0;
              can_data.TimeStamp.hour = 0;
              can_data.TimeStamp.minute = 0;
              can_data.TimeStamp.second = 0;
              can_data.TimeStamp.millisecond = 0;
              can_data.TimeStamp.microsecond = 0;

              can_data.DataLen = size;
              memset(can_data.Data, 0, sizeof(can_data.Data));
              memcpy(can_data.Data, data, size);
              /* 对齐 */
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

/* 发送固定协议帧长 */
bool can_driver_gc::send(const CHANNEL_STATE_Typedef_t &channel_state, \
                          const quint8 *data, quint8 size, quint32 id, \
                          FRAME_TYPE_Typedef_t frame_type, \
                          PROTOCOL_TYPE_Typedef_t protocol)
{
  /* 需要发送的帧数 */
  quint32 nSendCount = 1;

  /* 实际发送的帧数 */
  quint32 result = 0;

  bool ret = false;

  result = gc_can_send(channel_state, data, size, id, frame_type, protocol);

  /* 消息分发到UI显示cq */
  msg_to_ui_cq_buf(id, (quint8)channel_state.channel_num, CAN_TX_DIRECT, \
                   protocol, \
                   frame_type, \
                   DATA_FRAME_TYPE, \
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
  msg_to_ui_cq_buf(id, (quint8)channel_state.channel_num, UNKNOW_DIRECT, \
                   protocol, \
                   frame_type, \
                   DATA_FRAME_TYPE, \
                   (const quint8 *)result_info_str.toUtf8().data(), result_info_str.size());
  return ret;
}

void can_driver_gc::function_can_use_update()
{
  quint32 type = kDeviceType[device_type_index_].device_type;
  const bool cloudDevice = false;
  const bool netcanfd = false;
  const bool netcan = false;
  const bool netDevice = (netcan || netcanfd);
  const bool tcpDevice = false;
  const bool udpDevice = false;
  const bool usbcanfd = type == GC_USBCANFD;
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

void can_driver_gc::receive_data(const CHANNEL_STATE_Typedef_t &channel_state)
{
  /* 确定型号 */
  switch(kDeviceType[device_type_index_].device_type)
  {
    case GC_USBCAN2:
      {
        ulong len;
        GC_ERR_INFO vei;
        GC_CAN_OBJ can_data[CAN_MSG_NUM_MAX];
        len = GetReceiveNum(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num);
        if(0 < len)
        {
          len = Receive(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, can_data, CAN_MSG_NUM_MAX, 0);
          if(4294967295 == len)
          {
            if(GC_STATUS_ERR != ReadErrInfo(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, &vei))
            {
              qDebug() << "gc read data err" << "err code:" << QString::number(vei.ErrCode, 16);
            }
            break;
          }
          show_rec_message(channel_state, can_data, len, CAN_RX_DIRECT);
        }
        break;
      }

    case GC_USBCANFD:
      {
        GC_CANFD_OBJ can_data[CAN_MSG_NUM_MAX];
        ulong len = CAN_MSG_NUM_MAX;
        ReceiveFD(kDeviceType[device_type_index_].device_type, device_index_, channel_state.channel_num, can_data, &len);
        if(0 < len)
        {
          show_rec_message(channel_state, can_data, (quint32)len, CAN_RX_DIRECT);
        }
        break;
      }

    default:
      return;
  }
}

/******************************** End of file *********************************/
