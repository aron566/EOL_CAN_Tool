/**
 *  @file eol_protocol.cpp
 *
 *  @date 2023年02月20日 17:05:52 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @details None.
 *
 *  @version v1.0.0 aron566 2023.02.20 17:05 初始版本.
 */
/** Includes -----------------------------------------------------------------*/
/** Private includes ---------------------------------------------------------*/
#include "eol_protocol.h"
#include <QDebug>
#include <QFile>
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
/*
上位机写入
           /------------/------------/------------/------------/------------/------------/-----------/
         /     帧头    /    功能码   /  寄存器地址  /   数据长度  /  变长数据   /    CRC16_L  /  CRC16_H  /
       /------------/------------/------------/------------/------------/-------------/-----------/
     /   2Bytes   /    1Byte   /   2Bytes   /   2Bytes   /     ...    /    1Byte    /    1Byte   /
   /------------/------------/------------/------------/------------/-------------/------------/
 *//***********C*************R***************C**********************/
/*
PS：
默认小端模式：低8位在前
帧长最小为：10Bytes
总帧长为：9Bytes + 数据长度
 */

/*
回复
           /------------/------------/------------/------------/------------/
         /    帧头     /    功能码   /  寄存器地址  / ACK RESULT /   MESSAGE  /
       /------------/------------/------------/------------/------------/
     /   2Bytes   /    1Byte   /   2Bytes   /   1Bytes   /   1Bytes   /
   /------------/------------/------------/------------/------------/
PS：
帧长固定为：7Bytes
*/

/*
上位机读取
           /------------/------------/------------/------------/------------/
         /    帧头    /    功能码   /   寄存器地址  /  CRC16_L   /   CRC16_H  /
       /------------/------------/-------------/------------/------------/
     /   2Bytes   / 1Byte 0x04 /    2Bytes   /   1Byte    /    1Byte   /
   /------------/------------/-------------/------------/------------/
 *//******C********R********C*************/
/*
PS：
默认小端模式：低8位在前
帧长固定为：7Bytes
*/

/*
回复
           /------------/------------/------------/------------/------------/------------/------------/
         /    帧头     /    功能码   /  寄存器地址  /   数据长度   /   变长数据  /  CRC16_L   /   CRC16_H  /
       /------------/------------/-------------/------------/------------/------------/------------/
     /   2Bytes   /    1Byte   /    2Bytes   /   2Bytes   /     ...    /    1Byte   /    1Byte   /
   /------------/------------/-------------/------------/------------/------------/------------/
 *//*******************C******************R*************C************/
/*
PS：
默认小端模式：低8位在前
帧长最短为：10Bytes
总帧长为：9Bytes + 数据长度
*/
#define ENABLE_SEND_DELAY             1       /**< 为1开启分包发送 */
#define ENABLE_SEND_DELAY_MS          5U      /**< 分包发送间隔ms >5ms */
#define ENABLE_SEND_DELAY_LIMIT_SIZE  8U      /**< >8Bytes时开启发送 */
#define SEND_ONE_PACKET_SIZE_MAX      8U      /**< 每包发送大小 */

#define MASTER_EOL_FRAME_HEADER       0x7AU   /**< 上位机帧头 */
#define SLAVE_EOL_FRAME_HEADER        0x75U   /**< 下位机帧头 */

#define EOL_FRAME_MIN_SIZE            7U      /**< 最小帧长 */

#define EOL_DEVICE_COM_ADDR           0x55U   /**< 本机通讯地址 */
#define EOL_PROTOCOL_REPLY_CAN_ID     0x666U  /**< 回复上位机CAN ID */
#define EOL_TEMP_BUF_SIZE_MAX         64U     /**< 临时缓冲区大小 */
/* 包重复检测 */
#define PACKAGE_REPEAT_CHECK_SIZE     10U     /**< 10包循环检测 */

/* 最小帧长 7Bytes DATA + 4Bytes ID */
#define FRAME_MIN_SIZE                (7U + 4U)

#define FRAME_TEMP_BUF_SIZE           64U
/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/*
		  	/-------/-------/-------/-------/-------/
			/   FF  /  RTR  /  DLC  /   ID  /  DATA /
		/-------/-------/-------/-------/-------/
	/ 1 bit / 1 bit / 4 bit / 11 bit/8 Bytes/
/-------/-------/-------/-------/-------/
标准帧 数据帧（8 Bytes）
*/
/** Private function prototypes ----------------------------------------------*/

/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

eol_protocol::eol_protocol(QObject *parent)
    : QObject{parent}
{

}

void eol_protocol::run_eol_task()
{
  /* 执行EOL协议解析 */
  qDebug() << "start protocal stack current thread:" << QThread::currentThreadId();
  quint8 frame_buf[FRAME_TEMP_BUF_SIZE];
  quint32 len = 0;
  quint32 id = 0;
  while(run_state)
  {
    //QCoreApplication::processEvents();
    can_driver_obj->receice_data();
    QThread::msleep(1);
    len = cq_obj->CQ_getLength(cq_obj->get_cq_handle());

    /* 检测是否满足最小帧长 */
    if(len < FRAME_MIN_SIZE)
    {
      continue;
    }

    /* 检测ID */
    cq_obj->CQ_ManualGetData(cq_obj->get_cq_handle(), frame_buf, 4);
    memcpy(&id, frame_buf, 4);

    switch(id)
    {
      case 0x0566:
        break;
      default:
        break;
    }

    /* 发出数据 */
//  emit signal_post_data(frame_buf+FRAME_DATA_OFFSET, data_len);//<! 由于线程原因需加上连接参数Qt::BlockingQueuedConnection进行同步
  }
  qDebug() << "eol protocol stack end";
}

/**
 * @brief 设置协议栈数据源
 * @param cq_
 */
void eol_protocol::set_cq(CircularQueue *cq_)
{
  cq_obj = cq_;
}
/******************************** End of file *********************************/
