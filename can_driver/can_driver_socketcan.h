/**
 *  @file can_driver_socketcan.hpp
 *
 *  @date 2026年9月27日 08:50:00 星期日
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief SocketCAN 驱动，基于 Qt SerialBus 自带的 socketcan 插件实现，
 *         Linux 下无需第三方库即可驱动真实 CAN 设备（如 can0、vcan0）。
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2026-09-27 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2026 aron566 <aron566@163.com>.
 */
#ifndef CAN_DRIVER_SOCKETCAN_H
#define CAN_DRIVER_SOCKETCAN_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
/** Private includes ---------------------------------------------------------*/
#include <QDebug>
#include <QMutex>
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include "can_driver_model.h"
#include "utility.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class can_driver_socketcan : public can_driver_model
{
  Q_OBJECT
public:
  explicit can_driver_socketcan(QObject *parent = nullptr);

  virtual ~can_driver_socketcan()
  {
    /* 关闭设备 */
    can_driver_socketcan::close();

    /* 等待线程结束 */
    while(thread_run_state)
    {
      utility::delay_ms(1);
    }

    qDebug() << "del can_driver socketcan model";
  }

  /**
   * @brief function_can_use_update_for_choose
   * @param device_type_str 接口名，如 "can0"
   * @return
   */
  static can_driver_model::SET_FUNCTION_CAN_USE_Typedef_t function_can_use_update_for_choose(const QString &device_type_str = "can0");

  /**
   * @brief set_device_brand 设置当前设备品牌
   * @param brand 品牌（待 can_driver_model.h 增加 SOCKETCAN_CAN_BRAND 后按 brand 过滤，
   *              目前直接返回本机枚举到的 socketcan 接口列表）
   * @return 该品牌下的设备名列表（即 socketcan 接口名列表，如 can0、vcan0）
   */
  virtual QStringList set_device_brand(CAN_BRAND_Typedef_t brand) override;

  /**
   * @brief 设置设备类型（此处设备类型即 socketcan 接口名）
   * @param device_type_str 接口名，如 "can0"
   * @return 设备通道数量（每个接口固定 1 个通道）
   */
  virtual quint8 set_device_type(const QString &device_type_str) override;

  /**
   * @brief open 打开设备（校验接口存在且已启用，不创建 socket）
   * @return true 成功
   */
  virtual bool open() override;

  /**
   * @brief init 初始化设备（创建 QCanBusDevice 并 connectDevice）
   * @return true 成功
   */
  virtual bool init() override;

  /**
   * @brief start 启动设备
   * @return true 成功
   */
  virtual bool start() override;

  /**
   * @brief reset 复位设备（断开并重建 socket，清空收发缓冲）
   * @return true 成功
   */
  virtual bool reset() override;

  /**
   * @brief close 关闭设备
   * @return true 成功
   */
  virtual bool close() override;

  /**
   * @brief read_info 读取设备信息
   * @return true 成功
   */
  virtual bool read_info() override;

  /**
   * @brief function_can_use_update 功能更新
   */
  virtual void function_can_use_update() override;

  /**
   * @brief send 发送数据
   * @param channel_state 通道信息
   * @param data 数据
   * @param size 数据字节长度（经典 CAN ≤ 8，CANFD ≤ 64）
   * @param id canid
   * @param frame_type 0标准帧 1扩展帧
   * @param protocol 0can 1canfd
   * @return true发送成功
   */
  virtual bool send(const CHANNEL_STATE_Typedef_t &channel_state, const quint8 *data, quint8 size, quint32 id, FRAME_TYPE_Typedef_t frame_type, PROTOCOL_TYPE_Typedef_t protocol) override;

  /**
   * @brief 数据接收
   * @note socketcan 采用 framesReceived 信号驱动接收（见 on_socketcan_frames_received），
   *       QCanBusDevice 与其内部帧队列只在其所属线程（GUI 线程）中访问；
   *       此处不再轮询，避免工作线程与 GUI 线程并发访问设备队列。
   */
  virtual void receive_data(const CHANNEL_STATE_Typedef_t &channel_state) override;

private slots:
  /**
   * @brief QCanBusDevice::framesReceived 信号槽：读出全部缓存帧，
   *        转调基类接收回调（msg_to_ui_cq_buf / msg_to_cq_buf），
   *        与其它驱动保持相同的回调路径
   */
  void on_socketcan_frames_received();

  /**
   * @brief QCanBusDevice::errorOccurred 信号槽：上报总线错误
   */
  void on_socketcan_error_occurred(QCanBusDevice::CanBusError error);

private:
  /**
   * @brief 枚举本机可用的 socketcan 接口
   * @return 接口名列表，如 {"can0", "vcan0"}，无接口或无插件时返回空
   * @note 底层通过 QCanBus::availableDevices("socketcan") 实现，
   *       socketcan 插件即按 /sys/class/net 下各接口的 type 属性是否为 ARPHRD_CAN(280) 来枚举
   */
  static QStringList enumerate_interfaces();

  /**
   * @brief 读取 /sys/class/net/<if_name>/<attr> 的内容
   * @param if_name 接口名
   * @param attr 属性名，如 "operstate"
   * @return 属性内容（去首尾空白），读不到返回空字符串
   */
  static QString read_sys_net_attr(const QString &if_name, const QString &attr);

  /**
   * @brief 打包发送载荷
   * @param data 数据
   * @param size 数据字节长度
   * @param protocol 0can 1canfd
   * @param payload 输出的载荷（经典 CAN 要求 size ≤ 8；
   *                CANFD 按有效长度 0-8,12,16,20,24,32,48,64 对齐，不足补 0）
   * @return true 打包成功
   */
  static bool pack_payload(const quint8 *data, quint8 size, PROTOCOL_TYPE_Typedef_t protocol, QByteArray &payload);

  /**
   * @brief 打开单个通道：createDevice("socketcan", 接口名) 并 connectDevice
   * @param channel_state 通道信息（成功时 channel_handle 填入 QCanBusDevice*）
   * @return true 成功
   */
  bool open_channel(CHANNEL_STATE_Typedef_t &channel_state);

  /**
   * @brief 复位单个通道：断开并重建 socket
   * @param channel_state 通道信息
   * @return true 成功
   */
  bool reset_channel(const CHANNEL_STATE_Typedef_t &channel_state);

  /**
   * @brief 关闭单个通道：disconnectDevice 并释放 QCanBusDevice
   * @param channel_state 通道信息
   */
  void close_channel(const CHANNEL_STATE_Typedef_t &channel_state);

  /**
   * @brief 读出指定设备当前全部缓存帧并分发
   * @param device socketcan 设备
   * @param channel_state 通道信息
   */
  void drain_received_frames(QCanBusDevice *device, const CHANNEL_STATE_Typedef_t &channel_state);

  /**
   * @brief 单帧接收分发：转调基类 msg_to_ui_cq_buf / msg_to_cq_buf
   * @param channel_state 通道信息
   * @param frame 收到的 CAN 帧
   */
  void show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const QCanBusFrame &frame);

  QStringList socketcan_if_list_; /**< set_device_brand/set_device_type 枚举到的接口列表 */
  QMutex send_mutex_;             /**< 发送串行化锁：writeFrame 可能被收发工作线程并发调用 */
};

#endif // CAN_DRIVER_SOCKETCAN_H
/******************************** End of file *********************************/
