/**
 *  @file network_driver_udp.cpp
 *
 *  @date 2023年11月20日 15:23:54 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief udp驱动.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.11.20 15:23 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-11-20 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
/** Private includes ---------------------------------------------------------*/
#include "network_driver_udp.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/

/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
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

network_driver_udp::network_driver_udp(QObject *parent)
    : network_driver_model{parent}
{

}

 network_driver_udp::~network_driver_udp()
{
  network_driver_udp::network_stop();
}

qint32 network_driver_udp::repeat_check(const QString &ip, const QQueue<network_driver_model::NETWORK_PEER_INFO_Typedef_t> &msg_list)
{
  for(qint32 i = 0; i < msg_list.size(); i++)
  {
    if(true == msg_list.value(i).peer_addr.contains(ip))
    {
      return i;
    }
  }
  return -1;
}

qint32 network_driver_udp::get_peer_port(const QString &ip)
{
  for(qint32 i = 0; i < com_info_list.size(); i++)
  {
    if(true == com_info_list.value(i).peer_addr.contains(ip))
    {
      QStringList info = com_info_list.value(i).peer_addr.split(":");
      if(2 > info.size())
      {
        return -1;
      }
      return info.value(1).toUInt();
    }
  }
  return -1;
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/
/**
   * @brief network_init 网络初始化
   * @param ip ip地址
   * @param port 端口
   * @param role 角色
   * @param net_type 网络协议类型
   * @return true成功
   */
bool network_driver_udp::network_init(QString &ip, QString &port, NETWORK_WORK_ROLE_Typedef_t role, NETWORK_TYPE_Typedef_t net_type)
{
  if(NETWORK_UDP_TYPE != net_type)
  {
    return false;
  }

  QString tips;
  switch(role)
  {
    case NETWORK_SERVER_ROLE:
      {
        if(nullptr != server)
        {
          return false;
        }
        com_info_list.clear();
        /* 服务器 */
        server = new hv::UdpServer;
        int bindfd = server->createsocket(port.toInt(), ip.toUtf8().data());
        if (0 > bindfd)
        {
          tips = tr("udp server init failed!");
          return false;
        }
        /* 注册消息回调 */
        server->onMessage = [this](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
          network_driver_model::NETWORK_PEER_INFO_Typedef_t peer_data;

          /* 对方地址 */
          peer_data.peer_addr = QString::fromStdString(channel->peeraddr());

          /* 地址查询 */
          QStringList info = peer_data.peer_addr.split(":");
          if(2 > info.size())
          {
            return;
          }
          qint32 index = repeat_check(info.value(0), com_info_list);
          if(-1 != index)
          {
            peer_data = com_info_list.value(index);
            /* 判断是否是重连，更新对方地址信息 */
            if(peer_data.peer_addr != QString::fromStdString(channel->peeraddr()))
            {
              peer_data.peer_addr = QString::fromStdString(channel->peeraddr());
            }
            com_info_list.replace(index, peer_data);
            goto _show_server_rx_msg;
          }

          /* 对方网络类型 */
          peer_data.net_type = NETWORK_UDP_TYPE;
          com_info_list.append(peer_data);

_show_server_rx_msg:
          /* 加入数据到缓冲区 */
          network_put_data((const quint8 *)buf->base, buf->size(), info.value(0));
          QString tips;
          tips = QString::asprintf("[UDP SERVER]Rx ADDR:%s LEN:%llu DATA:", peer_data.peer_addr.toStdString().data(), buf->size());
          for(quint32 i = 0; i < buf->size(); ++i)
          {
            tips += QString::asprintf("%02X ", (quint8)buf->base[i]);
          }
          this->show_message(tips, 1U, 1U, (const quint8 *)buf->base, buf->size(), true, info.value(0));
          this->show_message_bytes(buf->size(), 1U, 1U);
        };

        /* 注册发送完成回调 */
        server->onWriteComplete = [this](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
          QString tips = "[UDP SERVER]";
          tips += QString::asprintf("Tx ADDR:%s LEN:%llu DATA:", channel->peeraddr().c_str(), buf->size());
          for(quint32 i = 0; i < buf->size(); ++i)
          {
            tips += QString::asprintf("%02X ", (quint8)buf->base[i]);
          }
          this->show_message(tips, 1U, 0U, (const quint8 *)buf->base, buf->size(), false, QString::fromStdString(channel->peeraddr().c_str()));
          this->show_message_bytes(buf->size(), 1U, 0U);

          tips = tr("[UDP SERVER]send data sucessful! to addr:%1").arg(QString::fromStdString(channel->peeraddr()));
          this->show_message(tips, 1U, 0U);
        };

        tips = tr("udp server init ok!");
      }
      break;

    case NETWORK_CLIENT_ROLE:
      {
        if(nullptr != client)
        {
          return false;
        }
        com_info_list.clear();
        /* 客户端 */
        client = new hv::UdpClient;
        int bindfd = client->createsocket(port.toInt(), ip.toUtf8().data());
        if (0 > bindfd)
        {
          tips = tr("udp client init failed!");
          return false;
        }
        /* 注册消息回调 */
        client->onMessage = [this](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
          network_driver_model::NETWORK_PEER_INFO_Typedef_t peer_data;

          /* 对方地址 */
          peer_data.peer_addr = QString::fromStdString(channel->peeraddr());

          /* 地址查询 */
          QStringList info = peer_data.peer_addr.split(":");
          if(2 > info.size())
          {
            return;
          }
          qint32 index = repeat_check(info.value(0), com_info_list);
          if(-1 != index)
          {
            peer_data = com_info_list.value(index);
            /* 判断是否是重连，更新对方地址信息 */
            if(peer_data.peer_addr != QString::fromStdString(channel->peeraddr()))
            {
              peer_data.peer_addr = QString::fromStdString(channel->peeraddr());
            }
            com_info_list.replace(index, peer_data);
            goto _show_client_rx_msg;
          }

          /* 对方网络类型 */
          peer_data.net_type = NETWORK_UDP_TYPE;
          com_info_list.append(peer_data);

_show_client_rx_msg:
          /* 加入数据到缓冲区 */
          network_put_data((const quint8 *)buf->base, buf->size(), info.value(0));
          QString tips;
          tips = QString::asprintf("[UDP CLIENT]Rx ADDR:%s LEN:%llu DATA:", peer_data.peer_addr.toStdString().data(), buf->size());
          for(quint32 i = 0; i < buf->size(); ++i)
          {
            tips += QString::asprintf("%02X ", (quint8)buf->base[i]);
          }
          this->show_message(tips, 0U, 1U, (const quint8 *)buf->base, buf->size(), true, info.value(0));
          this->show_message_bytes(buf->size(), 0U, 1U);
        };

        /* 注册发送完成回调 */
        client->onWriteComplete = [this](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
          QString tips = "[UDP CLIENT]";
          tips += QString::asprintf("Tx ADDR:%s LEN:%llu DATA:", channel->peeraddr().c_str(), buf->size());
          for(quint32 i = 0; i < buf->size(); ++i)
          {
            tips += QString::asprintf("%02X ", (quint8)buf->base[i]);
          }
          this->show_message(tips, 0U, 0U, (const quint8 *)buf->base, buf->size(), false, QString::fromStdString(channel->peeraddr().c_str()));
          this->show_message_bytes(buf->size(), 0U, 0U);

          tips = tr("[UDP CLIENT]send data sucessful! to addr:%1").arg(QString::fromStdString(channel->peeraddr()));
          this->show_message(tips, 0U, 0U);
        };

        tips = tr("udp client init ok!");
      }
      break;

    default:
      return false;
  }

  /* 发送提示信息 */
  show_message(tips);
  return true;
}

/**
   * @brief network_start 网络启动
   * @return true成功
   */
bool network_driver_udp::network_start()
{
  /* 合法性检测 */
  if(nullptr == server && nullptr == client)
  {
    show_message(tr("udp start failed!"));
    return false;
  }

  if(nullptr != server)
  {
    server->start();
    show_message(tr("udp server start ok!"));
  }
  if(nullptr != client)
  {
    client->start();
    show_message(tr("udp client start ok!"));
  }
  return true;
}

/**
   * @brief network_start 网络停止
   * @return true成功
   */
bool network_driver_udp::network_stop()
{
  if(nullptr != server)
  {
    server->stop();
    show_message(tr("udp server stop ok!"));
  }
  if(nullptr != client)
  {
    client->stop();
    show_message(tr("udp client stop ok!"));
  }

  SAFE_DELETE(server);
  SAFE_DELETE(client);
  emit signal_network_is_closed();
  return true;
}

/**
   * @brief network_send_data 发送网络数据包
   * @param data 数据
   * @param len 数据长度
   * @param ip 对方ip地址
   * @param port 对方端口号
   * @param role 本机角色
   * @param net_type 本机网络类型
   * @return true发送成功
   */
bool network_driver_udp::network_send_data(const quint8 *data, quint32 len, const QString &ip, const QString &port, NETWORK_WORK_ROLE_Typedef_t role, NETWORK_TYPE_Typedef_t net_type)
{
  if(NETWORK_UDP_TYPE != net_type)
  {
    return false;
  }

  sockaddr_u peeraddr;
  memset(&peeraddr, 0, sizeof(peeraddr));

  QString tips;
  QString addr = QString(ip + ":" + port);

  switch(role)
  {
    case NETWORK_SERVER_ROLE:
      {
        /* 服务器 */
        if(nullptr == server)
        {
          tips = "udp server is not open!";
          this->show_message(tips, 1U, 0xFFU);
          return false;
        }
        tips = "[UDP SERVER]";
        qint32 peer_port = get_peer_port(ip);
        qint32 ret = sockaddr_set_ipport(&peeraddr, ip.toUtf8().data(), peer_port);
        if (ret != 0 || -1 == peer_port)
        {
          tips += tr("send data failed! unknown peer_addr %1").arg(ip);
          this->show_message(tips, 1U, 0U);
          return false;
        }
        if(0 > server->sendto(data, len, &peeraddr.sa))
        {
          tips += tr("send data failed! to addr:%1").arg(ip + ":" + QString::number(get_peer_port(ip)));
          this->show_message(tips, 1U, 0U);
          return false;
        }
      }
      break;

    case NETWORK_CLIENT_ROLE:
      {
        /* 客户端 */
        if(nullptr == client)
        {
          tips = "udp client is not open!";
          this->show_message(tips, 0U, 0xFFU);
          return false;
        }
        tips = "[UDP CLIENT]";
        qint32 ret = sockaddr_set_ipport(&peeraddr, ip.toUtf8().data(), port.toInt());
        if (ret != 0)
        {
          tips += tr("send data failed! unknown peer_addr %1").arg(ip);
          this->show_message(tips, 0U, 0U);
          return false;
        }
        if(0 > client->sendto(data, len, &peeraddr.sa))
        {
          tips = tr("send data failed! to addr:%1").arg(addr);
          this->show_message(tips, 0U, 0U);
          return false;
        }
      }
      break;

    default:
      return false;
  }
  return true;
}

/******************************** End of file *********************************/
