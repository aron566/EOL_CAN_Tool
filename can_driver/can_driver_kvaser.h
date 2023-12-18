/**
 *  @file can_driver_kvaser.hpp
 *
 *  @date 2023年11月01日 11:11:54 星期三
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief kvaser驱动.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-11-01 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 */
#ifndef CAN_DRIVER_KVASER_H
#define CAN_DRIVER_KVASER_H
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
#include "can_driver_model.h"
#include "kvaser_canlib.h"
#include <QLibrary>
#include <QObject>
#include <QDebug>
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class can_driver_kvaser : public can_driver_model
{
  Q_OBJECT
public:
  explicit can_driver_kvaser(QObject *parent = nullptr);

  virtual ~can_driver_kvaser()
  {
    /* 关闭设备 */
    can_driver_kvaser::close();

    /* 等待线程结束 */
    while(thread_run_state)
    {
      utility::delay_ms(1);
    }

    qDebug() << "del can_driver model";

    /* 卸载驱动 */
    if (true == myLibrary.isLoaded())
    {
      myLibrary.unload();
    }
  }

  /**
   * @brief function_can_use_update_for_choose
   * @param device_type_str
   * @param is_client_work_mode 是否是客户端模式
   * @return
   */
  static can_driver_model::SET_FUNCTION_CAN_USE_Typedef_t function_can_use_update_for_choose(const QString &device_type_str = "PCIE_V2");

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
   * @brief kvaser_can_send 发送消息
   * @param channel_state 句柄
   * @param data 数据
   * @param size 数据大小
   * @param id can id
   * @param frame_type 帧类型
   * @param protocol 协议类型
   * @return 发送成功帧数
   */
  quint32 kvaser_can_send(const CHANNEL_STATE_Typedef_t &channel_state, \
                      const quint8 *data, quint8 size, quint32 id, \
                      FRAME_TYPE_Typedef_t frame_type, \
                      PROTOCOL_TYPE_Typedef_t protocol);

  /* kvaser消息 */
  /* CAN信息帧的数据类型 */
  typedef struct
  {
    quint32 id;
    quint64 time_stamp;
    bool remote_flag; /**< 是否是远程帧 */
    bool extern_flag; /**< 是否是扩展帧 */
    bool is_canfd_flag;
    quint8 data_len;
    quint8 data[64];
  } KVASER_CAN_OBJ_Typedef_t;
  void show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const KVASER_CAN_OBJ_Typedef_t *data, quint32 len, CAN_DIRECT_Typedef_t dir);

  /**
     * @brief 获取fd发送长度
     * @param len 预期发送长度
     * @return 实际需要发送的长度
     */
  static quint8 get_send_len(quint8 len)
  {
    if(8U >= len)
    {
      return len;
    }

    if(12 >= len)
    {
      return 12U;
    }

    if(16 >= len)
    {
      return 16U;
    }

    if(20 >= len)
    {
      return 20U;
    }

    if(24 >= len)
    {
      return 24U;
    }

    if(32 >= len)
    {
      return 32U;
    }

    if(48 >= len)
    {
      return 48U;
    }

    if(64 >= len)
    {
      return 64U;
    }
    return len;
  }

  void check_for_error(const char *cmd, canStatus stat);

  QLibrary myLibrary;

  typedef void (*canInitializeLibrary_t)(void);
  typedef canStatus (*canGetNumberOfChannels_t)(int *channelCount);
  typedef canStatus (*canGetChannelData_t)(int channel,
                                        int item,
                                        void *buffer,
                                        size_t bufsize);
  typedef CanHandle (*canOpenChannel_t)(int channel, int flags);
  typedef canStatus (*canGetErrorText_t)(canStatus err, char *buf, unsigned int bufsiz);
  typedef canStatus (*canIoCtl_t)(const CanHandle hnd,
                               unsigned int func,
                               void *buf,
                               unsigned int buflen);
  typedef canStatus (*canSetBusOutputControl_t)(const CanHandle hnd,
                                             const unsigned int drivertype);

  typedef canStatus (*canSetBusParams_t)(const CanHandle hnd,
                                      long freq,
                                      unsigned int tseg1,
                                      unsigned int tseg2,
                                      unsigned int sjw,
                                      unsigned int noSamp,
                                      unsigned int syncmode);

  typedef canStatus (*canSetBusParamsFd_t)(const CanHandle hnd,
                                        long freq_brs,
                                        unsigned int tseg1_brs,
                                        unsigned int tseg2_brs,
                                        unsigned int sjw_brs);
  typedef canStatus (*canBusOn_t)(const CanHandle hnd);
  typedef canStatus (*canBusOff_t)(const CanHandle hnd);
  typedef canStatus (*canClose_t)(const CanHandle hnd);
  typedef canStatus (*canUnloadLibrary_t)(void);

  typedef canStatus (*canWrite_t)(const CanHandle hnd,
                               long id,
                               void *msg,
                               unsigned int dlc,
                               unsigned int flag);
  typedef canStatus (*canWriteSync_t)(const CanHandle hnd, unsigned long timeout);
  typedef canStatus (*canRead_t)(const CanHandle hnd,
                              long *id,
                              void *msg,
                              unsigned int *dlc,
                              unsigned int *flag,
                              unsigned long *time);

  canInitializeLibrary_t p_canInitializeLibrary = nullptr;
  canGetNumberOfChannels_t p_canGetNumberOfChannels = nullptr;
  canGetChannelData_t p_canGetChannelData = nullptr;
  canOpenChannel_t p_canOpenChannel = nullptr;
  canGetErrorText_t p_canGetErrorText = nullptr;
  canIoCtl_t p_canIoCtl = nullptr;
  canSetBusOutputControl_t p_canSetBusOutputControl = nullptr;
  canSetBusParams_t p_canSetBusParams = nullptr;
  canSetBusParamsFd_t p_canSetBusParamsFd = nullptr;
  canBusOn_t p_canBusOn = nullptr;
  canBusOff_t p_canBusOff = nullptr;
  canClose_t p_canClose = nullptr;
  canUnloadLibrary_t p_canUnloadLibrary = nullptr;
  canWrite_t p_canWrite = nullptr;
  canWriteSync_t p_canWriteSync = nullptr;
  canRead_t p_canRead = nullptr;
};

#endif // CAN_DRIVER_KVASER_H
/******************************** End of file *********************************/
