/**
 *  @file can_driver_zlg.hpp
 *
 *  @date 2023年10月24日 16:37:54 星期二
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-09-18 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 */
#ifndef CAN_DRIVER_ZLG_H
#define CAN_DRIVER_ZLG_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
//#include <limits.h> /**< need variable max value    */
//#include <stdalign.h> /**< need alignof    */
//#include <stdarg.h> /**< need va_start    */
//#include <ctype.h> /**< need isalpha isdigit */
//#include <stdatomic.h> /**< need atomic_compare_exchange_weak */
//#include <assert.h> /**< need assert( a > b ); */
//#include <setjmp.h> /**< need jmp_buf buf setjmp(buf); longjmp(buf,1) */
/** Private includes ---------------------------------------------------------*/
#include <QDebug>
#include "can_driver_model.h"
#include "zlgcan.h"
#include "utility.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class can_driver_zlg : public can_driver_model
{
  Q_OBJECT
public:
  explicit can_driver_zlg(QObject *parent = nullptr);

  virtual ~can_driver_zlg()
  {
    /* 关闭设备 */
    close();

    /* 等待线程结束 */
    while(thread_run_state)
    {
      utility::delay_ms(1);
    }

    qDebug() << "del can_driver model";
  }

  /**
   * @brief function_can_use_update_for_choose
   * @param device_type_str
   * @param is_client_work_mode 是否是客户端模式
   * @return
   */
  static can_driver_model::SET_FUNCTION_CAN_USE_Typedef_t function_can_use_update_for_choose(const QString &device_type_str = "ZCAN_USBCANFD_200U");

  /**
   * @brief set_device_brand 设置当前设备品牌
   * @param brand 品牌
   * @return 该品牌下的设备名列表
   */
  virtual QStringList set_device_brand(CAN_BRAND_Typedef_t brand) override;

  /**
   * @brief 设置设备类型
   * @param device_type_str 设备类型
   * @return 设备通道数量
   */
  virtual quint8 set_device_type(const QString &device_type_str) override;

  /**
   * @brief open 打开设备
   * @return true 成功
   */
  virtual bool open() override;

  /**
   * @brief init 初始化设备
   * @return true 成功
   */
  virtual bool init() override;

  /**
   * @brief start 启动设备
   * @return true 成功
   */
  virtual bool start() override;

  /**
   * @brief reset 复位设备
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
   * @param size 数据字节长度
   * @param id canid
   * @param frame_type 0标准帧 1扩展帧
   * @param protocol 0can 1canfd
   * @return true发送成功
   */
  virtual bool send(const CHANNEL_STATE_Typedef_t &channel_state, const quint8 *data, quint8 size, quint32 id, FRAME_TYPE_Typedef_t frame_type, PROTOCOL_TYPE_Typedef_t protocol) override;

  /**
   * @brief 数据接收
   */
  virtual void receive_data(const CHANNEL_STATE_Typedef_t &channel_state) override;

public:
  /**
   * @brief 设置队列发送模式
   *
   * @param en
   * @return true 开启
   * @return false 关闭
   */
  bool set_send_queue_mode(bool en);

  /**
   * @brief 显示发送模式
   *
   */
  void show_send_mode();

  /**
   * @brief 显示发送队列可用空间
   *
   */
  void show_tx_queue_available();

  /**
   * @brief 显示设备自动发送队列
   *
   */
  void show_dev_auto_send();

  /**
   * @brief 清空发送队列
   *
   */
  void clear_tx_queue();

  /**
   * @brief 添加自动发送
   *
   */
  void add_auto_send();

  /**
   * @brief 启动自动发送
   *
   */
  void auto_send_start();

  /**
   * @brief 停止自动发送
   *
   */
  void auto_send_stop();

  /**
   * @brief 停单个自动发送
   *
   */
  void auto_send_stop_single();

private:
  bool init(CHANNEL_STATE_Typedef_t &channel_state);
  bool start(const CHANNEL_STATE_Typedef_t &channel_state);
  bool reset(const CHANNEL_STATE_Typedef_t &channel_state);
  void close_channel(const CHANNEL_STATE_Typedef_t &channel_state);

  bool custom_baud_rate_config(const CHANNEL_STATE_Typedef_t &channel_state);

  /* zlg消息 */
  void show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_Receive_Data *data, quint32 len);
  void show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_ReceiveFD_Data *data, quint32 len);

  /**
   * @brief zlg_can_send 发送消息
   * @param channel_state 句柄
   * @param data 数据
   * @param size 数据大小
   * @param id can id
   * @param frame_type 帧类型
   * @param protocol 协议类型
   * @return 发送成功帧数
   */
  quint32 zlg_can_send(const CHANNEL_STATE_Typedef_t &channel_state, \
                       const quint8 *data, quint8 size, quint32 id, \
                       FRAME_TYPE_Typedef_t frame_type, \
                       PROTOCOL_TYPE_Typedef_t protocol);


  bool transmit_type_config(const CHANNEL_STATE_Typedef_t &channel_state);
  bool resistance_config(const CHANNEL_STATE_Typedef_t &channel_state);
  bool baud_rate_config(const CHANNEL_STATE_Typedef_t &channel_state);
  bool cand_fd_bps_config(const CHANNEL_STATE_Typedef_t &channel_state);

  void show_tx_queue_available(const CHANNEL_STATE_Typedef_t &channel_state);
  void clear_tx_queue(const CHANNEL_STATE_Typedef_t &channel_state);
  bool set_send_queue_mode(const CHANNEL_STATE_Typedef_t &channel_state, bool en);
  void show_send_mode(const CHANNEL_STATE_Typedef_t &channel_state);
  /**
   * @brief 是否是相关类型设备
   *
   * @param type 设备类型
   * @return true 是
   * @return false 不是
   */
  static bool is_net_can_type(quint32 type);
  static bool is_net_can_fd_type(quint32 type);
  static bool is_net_tcp_type(quint32 type);
  static bool is_net_udp_type(quint32 type);

  /**
   * @brief 配置can 自动发送
   *
   * @param nEnable 该通道是否使能
   */
  void add_auto_can(quint32 nEnable);

  /**
   * @brief 配置can fd自动发送
   *
   * @param nEnable 该通道是否使能
   */
  void add_auto_can_fd(quint32 nEnable);

  /**
   * @brief can数据打包
   *
   * @param can_data can数据包
   * @param add_delay_flag 延时标识
   */
  void can_frame_packed(ZCAN_Transmit_Data &can_data, bool add_delay_flag);
  void can_frame_packed(ZCAN_TransmitFD_Data &canfd_data, bool add_delay_flag);
  void can_frame_packed(ZCAN_Transmit_Data &canfd_data, quint32 id, quint32 frame_type, const quint8 *data, quint8 size);
  void can_frame_packed(ZCAN_TransmitFD_Data &canfd_data, quint32 id, quint32 frame_type, const quint8 *data, quint8 size);

};

#endif // CAN_DRIVER_ZLG_H
/******************************** End of file *********************************/
