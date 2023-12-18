/**
 *  @file can_driver_ts.hpp
 *
 *  @date 2023年10月31日 11:00:45 星期二
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief 同星can驱动.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-10-31 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 */
#ifndef CAN_DRIVER_TS_H
#define CAN_DRIVER_TS_H
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
#include <QObject>
#include <QDebug>
#include "can_driver_model.h"
#include "TSCANLINApi.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class can_driver_ts : public can_driver_model
{
  Q_OBJECT
public:
  explicit can_driver_ts(QObject *parent = nullptr);

  virtual ~can_driver_ts()
  {
    /* 关闭设备 */
    can_driver_ts::close();

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
  static can_driver_model::SET_FUNCTION_CAN_USE_Typedef_t function_can_use_update_for_choose(const QString &device_type_str = "TS_USBCANFD_1014");

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
signals:

public:


private:
  bool init(CHANNEL_STATE_Typedef_t &channel_state);
  bool start(const CHANNEL_STATE_Typedef_t &channel_state);
  bool reset(const CHANNEL_STATE_Typedef_t &channel_state);
  void close_channel(const CHANNEL_STATE_Typedef_t &channel_state);

  /**
   * @brief ts_can_send 发送消息
   * @param channel_state 句柄
   * @param data 数据
   * @param size 数据大小
   * @param id can id
   * @param frame_type 帧类型
   * @param protocol 协议类型
   * @return 发送成功帧数
   */
  quint32 ts_can_send(const CHANNEL_STATE_Typedef_t &channel_state, \
                      const quint8 *data, quint8 size, quint32 id, \
                      FRAME_TYPE_Typedef_t frame_type, \
                      PROTOCOL_TYPE_Typedef_t protocol);

  /* ts消息 */
  void show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const TLibCAN *data, quint32 len);
  void show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const TLibCANFD *data, quint32 len);

private:
  /* 同星can */
  QSharedPointer<TSCANLINApi> ts_can_obj;

  /* 初始化标识 */
  bool ts_init_flag = false;
};

#endif // CAN_DRIVER_TS_H
/******************************** End of file *********************************/
