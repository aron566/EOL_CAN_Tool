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

void network_driver_model::show_message(const QString &str, quint32 channel_num, \
                                    quint8 direct, const quint8 *data, quint32 data_len, bool thread_mode, QString ip)
{
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
