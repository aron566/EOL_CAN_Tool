/**
 *  @file can_driver_sender.cpp
 *
 *  @date 2024年01月08日 10:54:54 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief can消息发送器.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2024.01.08 11:54 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-01-08 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <QDateTime>
/** Private includes ---------------------------------------------------------*/
#include "can_driver_sender.h"
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

can_driver_sender::can_driver_sender(can_driver_model *obj, QObject *parent)
    : QObject{parent}
{
  can_driver_obj = obj;
}


void can_driver_sender::send_data_task()
{
  /* 定时发送 */
  if(false == period_send_msg_list.isEmpty())
  {
    can_driver_model::PERIOD_SEND_MSG_Typedef_t send_task;
    qint32 size = period_send_msg_list.size();
    for(qint32 i = 0; i < size; i++)
    {
      if(true == clear_flag)
      {
        clear_flag = false;
        period_send_msg_list.clear();
        return;
      }
      send_task = period_send_msg_list.value(i);
      quint64 current_time = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
      /* 检测周期、发送帧数 */
      if((current_time - send_task.last_send_time) >= send_task.period_time
          && (0 < send_task.send_cnt || -1 == send_task.send_cnt))
      {
        /* 更新时间 */
        send_task.last_send_time = current_time;

        /* 可允许发送次数减小 */
        if(0 < send_task.send_cnt)
        {
          send_task.send_cnt--;
        }

        // /* 设置发送的canid */
        // can_driver_model::set_message_id(QString::number(send_task.can_id, 16));
        // /* 设置canfd加速 */
        // can_driver_model::set_can_fd_exp(send_task.can_fd_exp);
        // /* 设置发送协议类型 */
        // can_driver_model::set_protocol_type(send_task.protocol_type);
        // /* 设置发送帧类型 */
        // can_driver_model::set_frame_type(send_task.frame_type);
        // /* 设置消息数据 */
        // can_driver_model::set_message_data(send_task.data);
        // /* 发送 */
        // can_driver_obj->send(send_task.channel_num);

        /* 需要发送的帧数 */
        quint32 nSendCount = 1;

        quint32 pack_len = 0;
        switch(send_task.protocol_type)
        {
          /* can */
          case can_driver_model::CAN_PROTOCOL_TYPE:
            {
              pack_len = 8U;
              break;
            }

          /* canfd */
          case can_driver_model::CANFD_PROTOCOL_TYPE:
            {
              pack_len = 64U;
              break;
            }

          default:
            return;
        }
        bool ok;
        QStringList data_list = send_task.data.split(' ');
        quint32 data_size = (quint32)data_list.length();

        /* 计算分包数 */
        nSendCount = (data_size + pack_len - 1U) / pack_len;

        quint8 send_size = 0;
        if(nSendCount > 0)
        {
          for(quint32 i = 0; i < nSendCount; ++i)
          {
            quint8 data_buf[64] = {0};
            if((i * pack_len + pack_len) > data_size)
            {
              send_size = (quint8)(data_size - i * pack_len);
            }
            else
            {
              send_size = (quint8)pack_len;
            }
            /* 拷贝数据 */
            for(quint8 index = 0; index < send_size; index++)
            {
              data_buf[index] = (quint8)data_list[index + i * pack_len].toUShort(&ok, 16);
            }
            /* 立即发送 */
            can_driver_obj->send(data_buf, (quint8)(send_size),
                                 send_task.can_id,
                                 send_task.frame_type,
                                 send_task.protocol_type,
                                 (quint8)send_task.channel_num);
          }
        }

        /* 更新记录 */
        period_send_msg_list.replace(i, send_task);
      }
    }

    /* 删除发送次数为0的 */
    QList<can_driver_model::PERIOD_SEND_MSG_Typedef_t>::iterator it = period_send_msg_list.begin();
    while(0 < period_send_msg_list.size() && it != period_send_msg_list.end())
    {
      if(0 == (*it).send_cnt)
      {
        it = period_send_msg_list.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/
/******************************** End of file *********************************/

