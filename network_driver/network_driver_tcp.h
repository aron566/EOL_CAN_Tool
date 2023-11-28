/**
 *  @file network_driver_tcp.hpp
 *
 *  @date 2023年11月20日 15:26:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief tcp驱动.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-11-20 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 */
#ifndef NETWORK_DRIVER_TCP_H
#define NETWORK_DRIVER_TCP_H
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
#include "network_driver_model.h"
#include "hv/TcpServer.h"
#include "hv/TcpClient.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class network_driver_tcp : public network_driver_model
{
  Q_OBJECT
public:
  explicit network_driver_tcp(QObject *parent = nullptr);

  ~network_driver_tcp();
public:

  /**
   * @brief network_init 网络初始化
   * @param ip ip地址
   * @param port 端口
   * @param role 角色
   * @param net_type 网络协议类型
   * @return true成功
   */
  virtual bool network_init(QString &ip, QString &port, NETWORK_WORK_ROLE_Typedef_t role = NETWORK_CLIENT_ROLE, NETWORK_TYPE_Typedef_t net_type = NETWORK_UDP_TYPE) override;

  /**
   * @brief network_start 网络启动
   * @return true成功
   */
  virtual bool network_start() override;

  /**
   * @brief network_start 网络停止
   * @return true成功
   */
  virtual bool network_stop() override;

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
  virtual bool network_send_data(const quint8 *data, quint32 len, const QString &ip, const QString &port, NETWORK_WORK_ROLE_Typedef_t role = NETWORK_CLIENT_ROLE, NETWORK_TYPE_Typedef_t net_type = NETWORK_UDP_TYPE);

public:

  QQueue<network_driver_model::NETWORK_PEER_INFO_Typedef_t>client_rec_msg_list;/**< 客户端接收服务器消息列表 */
  QQueue<network_driver_model::NETWORK_PEER_INFO_Typedef_t>server_rec_msg_list;/**< 服务器接收客户端消息列表 */

private:
  /**
   * @brief repeat_check 重复检测
   * @param ip 地址
   * @param id 通讯唯一id
   * @param msg_list 消息列表
   * @return -1不重复 重复所在index
   */
  qint32 repeat_check(const QString &ip, quint32 id, const QQueue<network_driver_model::NETWORK_PEER_INFO_Typedef_t> &msg_list);

  /**
   * @brief get_msg_id
   * @param ip 地址
   * @param msg_list 消息列表
   * @return 0xFFFFFFFFU不存在， 消息id
   */
  quint32 get_msg_id(const QString &ip, const QQueue<network_driver_model::NETWORK_PEER_INFO_Typedef_t> &msg_list);
private:

  hv::TcpServer *server = nullptr;/**< tcp服务端 */
  hv::TcpClient *client = nullptr;/**< tcp客户端 */
};

#endif // NETWORK_DRIVER_TCP_H
/******************************** End of file *********************************/
