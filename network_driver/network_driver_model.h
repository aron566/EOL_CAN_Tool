/**
 *  @file network_driver_model.hpp
 *
 *  @date 2023年11月20日 15:18:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief 网络驱动模型.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-11-20 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 */
#ifndef NETWORK_DRIVER_MODEL_H
#define NETWORK_DRIVER_MODEL_H
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
#include <QQueue>
#include <QDebug>
#include "utilities/circularqueue.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class network_driver_model : public QObject
{
  Q_OBJECT
public:
  explicit network_driver_model(QObject *parent = nullptr);

public:
  /* 网络角色 */
  typedef enum
  {
    NETWORK_SERVER_ROLE = 0,  /**< 服务器 */
    NETWORK_CLIENT_ROLE,      /**< 客户端 */
  }NETWORK_WORK_ROLE_Typedef_t;

  /* 网络类型 */
  typedef enum
  {
    NETWORK_UDP_TYPE = 0,     /**< UDP网络类型 */
    NETWORK_TCP_TYPE,         /**< TCP网络类型 */
  }NETWORK_TYPE_Typedef_t;

  /* 对方通讯信息 */
  typedef struct
  {
    QString peer_addr;
    quint32 id;/**< for tcp */
    NETWORK_TYPE_Typedef_t net_type;
    quint8 buffer[512U];
    CircularQueue::CQ_handleTypeDef cq;
    CircularQueue::CQ_handleTypeDef *pcq_obj;
  }NETWORK_PEER_INFO_Typedef_t;

public:

  QQueue<network_driver_model::NETWORK_PEER_INFO_Typedef_t>client_rec_msg_list;/**< 客户端接收服务器消息列表 */
  QQueue<network_driver_model::NETWORK_PEER_INFO_Typedef_t>server_rec_msg_list;/**< 服务器接收客户端消息列表 */

signals:

  /**
   * @brief 发送信号显示当前消息
   *
   * @param message 消息内容t
   */
  void signal_show_message(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data, quint32 data_len = 0, QString ip = "");
  void signal_show_thread_message(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data, quint32 data_len = 0, QString ip = "");

  /**
   * @brief signal_show_message_bytes
   * @param bytes 字节数
   * @param channel_num 消息通道号 0上框客户端 1下框服务端
   * @param direct 消息方向，0发送（红色） or 1接收（白色） 0xFF状态消息
   */
  void signal_show_message_bytes(quint32 bytes, quint32 channel_num, quint8 direct);
public:

  /**
   * @brief network_init 网络初始化
   * @param ip ip地址
   * @param port 端口
   * @param role 角色
   * @param net_type 网络协议类型
   * @return true成功
   */
  virtual bool network_init(QString &ip, QString &port, NETWORK_WORK_ROLE_Typedef_t role = NETWORK_CLIENT_ROLE, NETWORK_TYPE_Typedef_t net_type = NETWORK_UDP_TYPE) = 0;

  /**
   * @brief network_start 网络启动
   * @return true成功
   */
  virtual bool network_start() = 0;

  /**
   * @brief network_start 网络停止
   * @return true成功
   */
  virtual bool network_stop() = 0;

  /**
   * @brief network_send_data 发送网络数据包
   * @param data 数据
   * @param len 数据长度
   * @param ip 对方ip地址
   * @param port 对方端口号
   * @param role 本机角色
   * @param net_type 本机网络类型
   * @return
   */
  virtual bool network_send_data(const quint8 *data, quint32 len, const QString &ip, const QString &port, NETWORK_WORK_ROLE_Typedef_t role = NETWORK_CLIENT_ROLE, NETWORK_TYPE_Typedef_t net_type = NETWORK_UDP_TYPE) = 0;

  /**
   * @brief network_get_rec_data 获取网络数据
   * @param ip ip地址
   * @param role 角色 0服务器 1客户端
   * @return
   */
  virtual CircularQueue::CQ_handleTypeDef *network_get_rec_data(const QString &ip, NETWORK_WORK_ROLE_Typedef_t role = NETWORK_CLIENT_ROLE) = 0;

public:
  /**
   * @brief 显示消息
   * @param str 消息数据
   * @param channel_num 消息通道号 0上框客户端 1下框服务端
   * @param direct 消息方向，0发送（红色） or 1接收（白色） 0xFF状态消息
   * @param data 数据
   * @param data_len 数据长度
   * @param thread_mode 当前消息是否来自线程
   * @param ip
   */
  void show_message(const QString &str, quint32 channel_num = 0, quint8 direct = 0xFFU, const quint8 *data = nullptr, quint32 data_len = 0, bool thread_mode = false, QString ip = "");

  /**
   * @brief show_message_bytes
   * @param bytes 字节数
   * @param channel_num 消息通道号 0上框客户端 1下框服务端
   * @param direct 消息方向，0发送（红色） or 1接收（白色） 0xFF状态消息
   */
  void show_message_bytes(quint32 bytes, quint32 channel_num, quint8 direct);

private:
};

#endif // NETWORK_DRIVER_MODEL_H
/******************************** End of file *********************************/
