/**
 *  @file can_driver_socketcan.cpp
 *
 *  @date 2026年9月27日 08:50:00 星期日
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2026 aron566 <aron566@163.com>.
 *
 *  @brief SocketCAN 驱动实现，基于 Qt SerialBus 自带的 socketcan 插件.
 *
 *  @details 使用说明（Linux）：
 *          socketcan 接口的波特率/使能状态由系统预先配置，Qt 侧不下发，
 *          打开设备前请先执行（以 can0、500Kbps 为例）：
 *            sudo ip link set can0 up type can bitrate 500000
 *          CANFD（仲裁域波特率 abitrate、数据域波特率 dbitrate）示例：
 *            sudo ip link set can0 up type can bitrate 500000 dbitrate 2000000 fd on
 *          虚拟接口测试：
 *            sudo modprobe vcan
 *            sudo ip link add dev vcan0 type vcan
 *            sudo ip link set vcan0 up
 *
 *  @version v0.0.1 aron566 2026.09.27 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2026-09-27 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
/** Private includes ---------------------------------------------------------*/
#include "can_driver_socketcan.h"
#include <QMessageBox>
#include <QFile>
#include <QMutexLocker>
#include <QCanBusDeviceInfo>
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/

/* 波特率表（仅界面展示用：socketcan 波特率由 ip link 预先配置，Qt 侧不下发） */
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

/* CANFD 仲裁域/数据域波特率表（仅界面展示用） */
static const quint32 kAbitTiming[] = {
  1000000U,//1Mbps
  800000U, //800kbps
  500000U, //500kbps
  250000U, //250kbps
  125000U, //125kbps
  100000U, //100kbps
  50000U,  //50kbps
};
static const quint32 kDbitTiming[] = {
  5000000U,//5Mbps
  4000000U,//4Mbps
  2000000U,//2Mbps
  1000000U,//1Mbps
};

/* CANFD 有效载荷长度：0-8, 12, 16, 20, 24, 32, 48, 64 */
static const quint8 kCanFdValidLens[] = {
  0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 12U, 16U, 20U, 24U, 32U, 48U, 64U
};

static const char kSocketCanPluginName[] = "socketcan"; /**< Qt SerialBus socketcan 插件名 */

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

can_driver_socketcan::can_driver_socketcan(QObject *parent)
    : can_driver_model{parent}
{

}

can_driver_model::SET_FUNCTION_CAN_USE_Typedef_t can_driver_socketcan::function_can_use_update_for_choose(const QString &device_type_str)
{
  Q_UNUSED(device_type_str);
  SET_FUNCTION_CAN_USE_Typedef_t function_can_use;

  /* 设备列表即本机 socketcan 接口列表 */
  function_can_use.device_list = can_driver_socketcan::enumerate_interfaces();

  /* 每个接口固定 1 个通道 */
  function_can_use.channel_num = 1;

  /* 工作模式由 ip link 决定，界面不可选 */
  function_can_use.work_mode_can_use = false;

  /* 终端电阻使能不可选 */
  function_can_use.resistance_cs_use = false;

  /* 波特率由 ip link 预先配置，Qt 侧不下发，界面不可选 */
  function_can_use.bauds_can_use = false;

  /* 仲裁域/数据域波特率不可选 */
  function_can_use.arbitration_data_bauds_can_use = false;

  /* 自定义波特率不可选 */
  function_can_use.diy_bauds_can_use = false;

  /* 验收滤波不可选 */
  function_can_use.filter_can_use = false;

  /* 非网络设备 */
  function_can_use.local_port_can_use = false;
  function_can_use.remote_port_can_use = false;
  function_can_use.remote_addr_can_use = false;

  for(quint16 i = 0; i < sizeof(kAbitTiming) / sizeof(kAbitTiming[0]); i++)
  {
    function_can_use.abitrate_list.append(bps_number2str(kAbitTiming[i]));
  }
  for(quint16 i = 0; i < sizeof(kDbitTiming) / sizeof(kDbitTiming[0]); i++)
  {
    function_can_use.datarate_list.append(bps_number2str(kDbitTiming[i]));
  }
  for(quint16 i = 0; i < sizeof(kBaudrate) / sizeof(kBaudrate[0]); i++)
  {
    function_can_use.baudrate_list.append(bps_number2str(kBaudrate[i]));
  }

  return function_can_use;
}

QStringList can_driver_socketcan::enumerate_interfaces()
{
  QStringList if_list;
  QCanBus *bus = QCanBus::instance();
  if(nullptr == bus)
  {
    return if_list;
  }
  if(false == bus->plugins().contains(QString::fromLatin1(kSocketCanPluginName)))
  {
    qDebug() << "qt socketcan plugin not found";
    return if_list;
  }
  const QList<QCanBusDeviceInfo> dev_infos = bus->availableDevices(QString::fromLatin1(kSocketCanPluginName));
  for(const QCanBusDeviceInfo &info : dev_infos)
  {
    if(false == info.name().isEmpty())
    {
      if_list.append(info.name());
    }
  }
  if_list.sort();
  return if_list;
}

QString can_driver_socketcan::read_sys_net_attr(const QString &if_name, const QString &attr)
{
  QFile file(QString("/sys/class/net/%1/%2").arg(if_name).arg(attr));
  if(false == file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    return QString();
  }
  return QString::fromLatin1(file.readAll()).trimmed();
}

bool can_driver_socketcan::pack_payload(const quint8 *data, quint8 size, PROTOCOL_TYPE_Typedef_t protocol, QByteArray &payload)
{
  payload.clear();
  if(nullptr == data)
  {
    return false;
  }
  switch(protocol)
  {
    /* 经典 CAN：单帧最多 8 字节 */
    case CAN_PROTOCOL_TYPE:
    {
      if(8U < size)
      {
        return false;
      }
      payload = QByteArray((const char *)data, (int)size);
      break;
    }

    /* CANFD：按有效长度对齐，不足补 0 */
    case CANFD_PROTOCOL_TYPE:
    {
      if(64U < size)
      {
        return false;
      }
      quint8 aligned_size = 64U;
      for(quint16 i = 0; i < sizeof(kCanFdValidLens) / sizeof(kCanFdValidLens[0]); i++)
      {
        if(size <= kCanFdValidLens[i])
        {
          aligned_size = kCanFdValidLens[i];
          break;
        }
      }
      payload = QByteArray((const char *)data, (int)size);
      if(aligned_size > size)
      {
        payload.append((int)(aligned_size - size), char(0));
      }
      break;
    }

    default:
      return false;
  }
  return true;
}

/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

QStringList can_driver_socketcan::set_device_brand(CAN_BRAND_Typedef_t brand)
{
  /* socketcan 品牌下的"设备"即本机 CAN 网络接口，动态枚举；
     待 can_driver_model.h 增加 SOCKETCAN_CAN_BRAND 后，此处可按 brand 过滤 */
  brand_ = brand;
  socketcan_if_list_ = enumerate_interfaces();
  return socketcan_if_list_;
}

quint8 can_driver_socketcan::set_device_type(const QString &device_type_str)
{
  /* 接口可能热插拔，每次选择时重新枚举 */
  socketcan_if_list_ = enumerate_interfaces();
  for(quint16 i = 0; i < (quint16)socketcan_if_list_.size(); i++)
  {
    if(0 == QString::compare(device_type_str, socketcan_if_list_.at(i)))
    {
      device_type_index_ = (qint16)i;

      /* 更新功能列表 */
      function_can_use_update();

      /* 每个 socketcan 接口固定 1 个通道 */
      return 1;
    }
  }
  return 0;
}

bool can_driver_socketcan::open()
{
  if(false == QCanBus::instance()->plugins().contains(QString::fromLatin1(kSocketCanPluginName)))
  {
    show_message(tr("未找到 Qt socketcan 插件，请确认 Qt SerialBus 已正确安装"));
    return false;
  }

  socketcan_if_list_ = enumerate_interfaces();
  if(device_type_index_ < 0 || device_type_index_ >= (qint16)socketcan_if_list_.size())
  {
    show_message(tr("未选择有效的 socketcan 接口"));
    return false;
  }
  const QString if_name = socketcan_if_list_.at(device_type_index_);

  /* 接口必须存在且已启用；波特率等参数由 ip link 预先配置 */
  const QString operstate = read_sys_net_attr(if_name, "operstate");
  if(0 == QString::compare(operstate, QString("down"), Qt::CaseInsensitive))
  {
    show_message(tr("socketcan 接口 %1 未启用，请先执行：sudo ip link set %1 up type can bitrate 500000").arg(if_name));
    return false;
  }

  /* 发送can打开状态 */
  device_opened_ = true;
  emit signal_can_is_opened();
  show_message(tr("open socketcan device %1 ok").arg(if_name));
  return true;
}

bool can_driver_socketcan::open_channel(CHANNEL_STATE_Typedef_t &channel_state)
{
  if(device_type_index_ < 0 || device_type_index_ >= (qint16)socketcan_if_list_.size())
  {
    show_message(tr("未选择有效的 socketcan 接口"), channel_state.channel_num);
    return false;
  }
  const QString if_name = socketcan_if_list_.at(device_type_index_);

  QString error_str;
  QCanBusDevice *device = QCanBus::instance()->createDevice(QString::fromLatin1(kSocketCanPluginName), if_name, &error_str);
  if(nullptr == device)
  {
    show_message(tr("创建 socketcan 设备 %1 失败：%2").arg(if_name).arg(error_str), channel_state.channel_num);
    show_message(tr("请确认接口存在且已启用，例如先执行：sudo ip link set %1 up type can bitrate 500000").arg(if_name), \
                 channel_state.channel_num);
    return false;
  }

  /* 使能 CANFD 收发（经典 CAN 不受影响）；
     CANFD 的仲裁域/数据域波特率由 ip link 的 bitrate/dbitrate 预先配置 */
  device->setConfigurationParameter(QCanBusDevice::CanFdKey, true);

  /* 接收用信号槽：framesReceived 在设备所属线程（GUI 线程）发射 */
  connect(device, &QCanBusDevice::framesReceived, this, &can_driver_socketcan::on_socketcan_frames_received);
  connect(device, &QCanBusDevice::errorOccurred, this, &can_driver_socketcan::on_socketcan_error_occurred);

  if(false == device->connectDevice())
  {
    const QString err = device->errorString();
    delete device;
    show_message(tr("连接 socketcan 接口 %1 失败：%2").arg(if_name).arg(err), channel_state.channel_num);
    show_message(tr("请先执行：sudo ip link set %1 up type can bitrate 500000").arg(if_name), channel_state.channel_num);
    return false;
  }

  channel_state.channel_handle = static_cast<void *>(device);
  show_message(tr("socketcan %1 ch %2 connect ok").arg(if_name).arg(channel_state.channel_num), channel_state.channel_num);
  return true;
}

bool can_driver_socketcan::init()
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

    ret = open_channel(channel_state);

    channel_state_list.insert(i, channel_state);
  }
  return ret;
}

bool can_driver_socketcan::start()
{
  if(false == device_opened_)
  {
    show_message(tr("device is not open "));
    return false;
  }

  /* socketcan 在 init() 的 connectDevice() 时已就绪，此处仅标记启动 */
  bool ret = true;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }
    show_message(tr("socketcan ch %1 start ok").arg(channel_state_list.value(i).channel_num), \
                 channel_state_list.value(i).channel_num);
  }

  start_ = true;

  return ret;
}

bool can_driver_socketcan::reset_channel(const CHANNEL_STATE_Typedef_t &channel_state)
{
  QCanBusDevice *device = static_cast<QCanBusDevice *>(channel_state.channel_handle);
  if(nullptr == device)
  {
    show_message(tr("socketcan ch %1 device is null").arg(channel_state.channel_num), channel_state.channel_num);
    return false;
  }

  /* 重建 socket，清空内核与 Qt 侧收发缓冲 */
  device->disconnectDevice();
  if(false == device->connectDevice())
  {
    show_message(tr("socketcan ch %1 reconnect failed: %2").arg(channel_state.channel_num).arg(device->errorString()), \
                 channel_state.channel_num);
    return false;
  }
  show_message(tr("socketcan ch %1 reset ok").arg(channel_state.channel_num), channel_state.channel_num);
  return true;
}

bool can_driver_socketcan::reset()
{
  /* 复位对应通道号 */
  bool ret = true;

  start_ = false;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }
    ret = reset_channel(channel_state_list.value(i));
  }
  return ret;
}

void can_driver_socketcan::close_channel(const CHANNEL_STATE_Typedef_t &channel_state)
{
  QCanBusDevice *device = static_cast<QCanBusDevice *>(channel_state.channel_handle);
  if(nullptr == device)
  {
    return;
  }
  /* 先断开信号，避免关闭过程中再触发接收槽 */
  disconnect(device, nullptr, this, nullptr);
  device->disconnectDevice();
  delete device;
  show_message(tr("socketcan ch %1 closed").arg(channel_state.channel_num), channel_state.channel_num);
}

bool can_driver_socketcan::close()
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

  device_opened_ = false;
  return true;
}

bool can_driver_socketcan::read_info()
{
  if(socketcan_if_list_.isEmpty() \
     || device_type_index_ < 0 \
     || device_type_index_ >= (qint16)socketcan_if_list_.size())
  {
    return false;
  }
  const QString if_name = socketcan_if_list_.at(device_type_index_);

  QString show_info;
  show_info += QString("<font size='5' color='green'><div align='legt'>interface:</div> <div align='right'>%1</div> </font>\r\n").arg(if_name);
  show_info += QString("<font size='5' color='green'><div align='legt'>operstate:</div> <div align='right'>%1</div> </font>\r\n").arg(read_sys_net_attr(if_name, "operstate"));
  show_info += QString("<font size='5' color='green'><div align='legt'>type:</div> <div align='right'>%1</div> </font>\r\n").arg(read_sys_net_attr(if_name, "type"));

  /* 真实 CAN 控制器会导出这些属性，vcan 等虚拟接口可能没有 */
  const QString bitrate = read_sys_net_attr(if_name, "can_bitrate");
  if(false == bitrate.isEmpty())
  {
    show_info += QString("<font size='5' color='green'><div align='legt'>can_bitrate:</div> <div align='right'>%1</div> </font>\r\n").arg(bitrate);
  }
  const QString can_state = read_sys_net_attr(if_name, "can_state");
  if(false == can_state.isEmpty())
  {
    show_info += QString("<font size='5' color='green'><div align='legt'>can_state:</div> <div align='right'>%1</div> </font>\r\n").arg(can_state);
  }
  const QString mtu = read_sys_net_attr(if_name, "mtu");
  if(false == mtu.isEmpty())
  {
    show_info += QString("<font size='5' color='green'><div align='legt'>mtu:</div> <div align='right'>%1</div> </font>\r\n").arg(mtu);
  }

  QMessageBox message(QMessageBox::Information, tr("Info"), show_info, QMessageBox::Yes, nullptr);
  message.exec();
  return true;
}

void can_driver_socketcan::function_can_use_update()
{
  /* 更新设备通道列表：每个 socketcan 接口固定 1 个通道，默认开启 */
  channel_state_list.clear();
  CHANNEL_STATE_Typedef_t channel_state;
  channel_state.channel_en = true;
  channel_state.channel_num = 0;
  channel_state.channel_handle = nullptr;
  channel_state.device_handle = 0;
  channel_state_list.append(channel_state);

  /* 队列发送不支持 */
  support_delay_send_ = false;
  support_delay_send_mode_ = false;
  support_get_send_mode_ = false;
  delay_send_can_use_update(support_delay_send_, support_delay_send_mode_, support_get_send_mode_);

  /* 队列发送模式是否启用 */
  send_queue_mode = false;
  emit signal_send_queue_mode_can_use(send_queue_mode);

  /* 定时发送不支持（Qt socketcan 无硬件定时发送能力） */
  auto_send_can_use_update(false, false, false, false, false);

  /* 工作模式是否可选（由 ip link 决定） */
  emit signal_work_mode_can_use(false);

  /* 终端电阻使能是否可选 */
  emit signal_resistance_cs_use(false);

  /* 波特率选择是否可选（由 ip link 预先配置，Qt 侧不下发） */
  emit signal_bauds_can_use(false);

  /* 仲裁域，数据域波特率是否可选 */
  emit signal_arbitration_data_bauds_can_use(false);

  /* 自定义波特率选择是否可选 */
  emit signal_diy_bauds_can_use(false);

  /* 过滤模式是否可选（验收码，屏蔽码） */
  emit signal_filter_can_use(false);

  /* 网络相关可选设置 */
  /* 本地端口是否可选 */
  emit signal_local_port_can_use(false);

  /* 远程端口是否可选 */
  emit signal_remote_port_can_use(false);

  /* 远程地址是否可选 */
  emit signal_remote_addr_can_use(false);
}

bool can_driver_socketcan::send(const CHANNEL_STATE_Typedef_t &channel_state, \
                                const quint8 *data, quint8 size, quint32 id, \
                                FRAME_TYPE_Typedef_t frame_type, \
                                PROTOCOL_TYPE_Typedef_t protocol)
{
  /* 需要发送的帧数 */
  const quint32 nSendCount = 1;

  /* 实际发送的帧数 */
  quint32 result = 0;

  bool ret = false;

  static const quint8 kEmptyByte = 0;
  QByteArray payload;
  QCanBusDevice *device = static_cast<QCanBusDevice *>(channel_state.channel_handle);
  if(nullptr != device && true == pack_payload(data, size, protocol, payload))
  {
    QCanBusFrame frame(id, payload);
    frame.setFrameType(QCanBusFrame::DataFrame);
    /* setFrameId 在 id > 11bit 时会自动置扩展标志，此处再按参数显式覆盖，保证与所选帧类型一致 */
    frame.setExtendedFrameFormat(EXT_FRAME_TYPE == frame_type);
    if(CANFD_PROTOCOL_TYPE == protocol)
    {
      frame.setFlexibleDataRateFormat(true);
      /* canfd_exp_index_：0 不加速，1 加速（BRS） */
      frame.setBitrateSwitch(0U != canfd_exp_index_);
    }

    /* writeFrame 可能被收发工作线程并发调用，串行化保护 */
    QMutexLocker locker(&send_mutex_);
    if(true == device->writeFrame(frame))
    {
      result = nSendCount;
    }
    else
    {
      show_message(tr("[%1]socketcan write frame failed: %2").arg(channel_state.channel_num).arg(device->errorString()), \
                   channel_state.channel_num);
    }
  }
  else
  {
    show_message(tr("[%1]socketcan pack frame failed, size %2 not supported").arg(channel_state.channel_num).arg(size), \
                 channel_state.channel_num);
  }

  /* 消息分发到UI显示cq（与其它驱动相同的回调路径） */
  const quint8 *tx_data = payload.isEmpty() ? &kEmptyByte : (const quint8 *)payload.constData();
  msg_to_ui_cq_buf(id, (quint8)channel_state.channel_num, CAN_TX_DIRECT,
                   protocol,
                   frame_type,
                   DATA_FRAME_TYPE,
                   tx_data, (quint8)payload.size());

  QString result_info_str;
  if(result != nSendCount)
  {
    ret = false;
    result_info_str = tr("[%1]send data failed! ").arg(channel_state.channel_num);
  }
  else
  {
    ret = true;
    result_info_str = tr("[%1]send data sucessful! ").arg(channel_state.channel_num);
  }
  result_info_str += QString("send num:%1, sucess num:%2").arg(nSendCount).arg(result);
  /* CAN_MSG_DISPLAY_Typedef_t::msg_data 仅 64 字节，截断防溢出 */
  const QByteArray info_utf8 = result_info_str.toUtf8().left(64);
  msg_to_ui_cq_buf(id, (quint8)channel_state.channel_num, UNKNOW_DIRECT,
                   protocol,
                   frame_type,
                   DATA_FRAME_TYPE,
                   (const quint8 *)info_utf8.constData(), (quint8)info_utf8.size());
  return ret;
}

void can_driver_socketcan::on_socketcan_frames_received()
{
  QCanBusDevice *device = qobject_cast<QCanBusDevice *>(sender());
  if(nullptr == device)
  {
    return;
  }

  /* 找到信号来源设备对应的通道 */
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    const CHANNEL_STATE_Typedef_t &channel_state = channel_state_list.at(i);
    if(true == channel_state.channel_en
        && static_cast<void *>(device) == channel_state.channel_handle)
    {
      drain_received_frames(device, channel_state);
      break;
    }
  }
}

void can_driver_socketcan::on_socketcan_error_occurred(QCanBusDevice::CanBusError error)
{
  Q_UNUSED(error);
  QCanBusDevice *device = qobject_cast<QCanBusDevice *>(sender());
  if(nullptr == device)
  {
    return;
  }
  show_message(tr("socketcan error: %1").arg(device->errorString()));
}

void can_driver_socketcan::drain_received_frames(QCanBusDevice *device, const CHANNEL_STATE_Typedef_t &channel_state)
{
  if(nullptr == device)
  {
    return;
  }
  while(0 < device->framesAvailable())
  {
    const QCanBusFrame frame = device->readFrame();
    show_rec_message(channel_state, frame);
  }
}

void can_driver_socketcan::show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const QCanBusFrame &frame)
{
  if(false == frame.isValid())
  {
    return;
  }
  /* 错误帧不进入消息流（与 kvaser 驱动处理一致） */
  if(QCanBusFrame::ErrorFrame == frame.frameType())
  {
    return;
  }

  quint32 can_id = frame.frameId();
  FRAME_TYPE_Typedef_t frame_type;
  if(true == frame.hasExtendedFrameFormat())
  {
    can_id &= 0x1FFFFFFFU;
    frame_type = EXT_FRAME_TYPE;
  }
  else
  {
    can_id &= 0x7FFU;
    frame_type = STD_FRAME_TYPE;
  }

  FRAME_DATA_TYPE_Typedef_t frame_data_type = \
      (QCanBusFrame::RemoteRequestFrame == frame.frameType()) ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE;
  PROTOCOL_TYPE_Typedef_t protocol = \
      (true == frame.hasFlexibleDataRateFormat()) ? CANFD_PROTOCOL_TYPE : CAN_PROTOCOL_TYPE;

  const QByteArray payload = frame.payload();
  const quint8 data_len = (quint8)payload.size(); /* CANFD 最多 64 字节 */
  static const quint8 kEmptyByte = 0;
  const quint8 *rx_data = payload.isEmpty() ? &kEmptyByte : (const quint8 *)payload.constData();

  /* 消息分发到UI显示cq */
  msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_RX_DIRECT,
                   protocol,
                   frame_type,
                   frame_data_type,
                   rx_data, data_len);

  /* 消息过滤分发 */
  msg_to_cq_buf(can_id, (quint8)channel_state.channel_num, rx_data, (quint32)payload.size());
}

void can_driver_socketcan::receive_data(const CHANNEL_STATE_Typedef_t &channel_state)
{
  Q_UNUSED(channel_state);
  /* 空实现，见头文件注释：接收完全由 framesReceived 信号槽驱动 */
}

/******************************** End of file *********************************/
