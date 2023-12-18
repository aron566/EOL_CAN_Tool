/**
 *  @file network_driver_model.cpp
 *
 *  @date 2023年11月20日 15:11:54 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief 网络驱动模型.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.11.20 15:11 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-11-20 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
/** Private includes ---------------------------------------------------------*/
#include "network_driver_model.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/

/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
QString network_driver_model::rec_ip_mask = "255.255.255.255";
/** Private function prototypes ----------------------------------------------*/

/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

network_driver_model::network_driver_model(QObject *parent)
    : QObject{parent}
{

}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

quint32 network_driver_model::network_put_data(const quint8 *data, quint32 len, const QString ip)
{
  NETWORK_MSG_FILTER_Typedef_t t;
  for(qint32 i = 0; i < rec_msg_list.size(); i++)
  {
    t = rec_msg_list.value(i);
    if(t.ip == ip)
    {
      if(nullptr != t.cq_obj)
      {
        return CircularQueue::CQ_putData(t.cq_obj->CQ_getCQHandle(), data, len);
      }
    }
  }
  return 0;
}

bool network_driver_model::network_register_rec_msg(QString ip, CircularQueue *cq)
{
  NETWORK_MSG_FILTER_Typedef_t t;
  for(qint32 i = 0; i < rec_msg_list.size(); i++)
  {
    t = rec_msg_list.value(i);
    if(t.ip == ip)
    {
      t.cq_obj = cq;
      rec_msg_list.replace(i, t);
      return true;
    }
  }
  t.ip = ip;
  t.cq_obj = cq;
  rec_msg_list.append(t);
  return true;
}

void network_driver_model::show_message(const QString &str, quint32 channel_num, \
                                    quint8 direct, const quint8 *data, quint32 data_len, bool thread_mode, QString ip)
{
  if(1U == direct)
  {
    QStringList mask = rec_ip_mask.split(".");
    QStringList source_ip = ip.split(".");
    if(4 == mask.size() && 4 == source_ip.size())
    {
      /* 检查接收ip */
      quint32 mask0 = mask.value(0).toUInt();
      quint32 source_ip0 = source_ip.value(0).toUInt();
      if((mask0 & source_ip0) != source_ip0)
      {
        return;
      }
      quint32 mask1 = mask.value(1).toUInt();
      quint32 source_ip1 = source_ip.value(1).toUInt();
      if((mask1 & source_ip1) != source_ip1)
      {
        return;
      }
      quint32 mask2 = mask.value(2).toUInt();
      quint32 source_ip2 = source_ip.value(2).toUInt();
      if((mask2 & source_ip2) != source_ip2)
      {
        return;
      }
      quint32 mask3 = mask.value(3).toUInt();
      quint32 source_ip3 = source_ip.value(3).toUInt();
      if((mask3 & source_ip3) != source_ip3)
      {
        return;
      }
    }
    /* to do nothing */
  }
  /* 输出到显示框 */
  if(false == thread_mode)
  {
    emit signal_show_message(str, channel_num, direct, data, data_len, ip);
  }
  else
  {
    emit signal_show_thread_message(str, channel_num, direct, data, data_len, ip);
    return;
  }
}

void network_driver_model::show_message_bytes(quint32 bytes, quint32 channel_num, quint8 direct)
{
  emit signal_show_message_bytes(bytes, channel_num, direct);
}

/******************************** End of file *********************************/
