/**
  *  @file circularqueue.cpp
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
#include <QDebug>
/** Private includes ---------------------------------------------------------*/
#include "circularqueue.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define GET_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define IS_POWER_OF_2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))

/**
 * @name 返回值定义
 * @{
 */
#define TRUE true
#define FALSE false
/** @}*/
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

/** Public application code --------------------------------------------------*/
/*******************************************************************************
 *
 *       Public code
 *
 ********************************************************************************
 */

CircularQueue::CircularQueue(QObject *parent)
    : QObject{parent}
{

}

/**
 * @brief CircularQueue::CircularQueue 构造环形缓冲区
 * @param type
 * @param size
 */
CircularQueue::CircularQueue(DATA_TYPE_Typedef_t type, CQ_BUF_SIZE_ENUM_TypeDef size, QObject *parent)
    : QObject{parent}
{
  if(size == 0)
  {
      return;
  }

  if(type == UINT8_DATA_BUF)
  {
      CQ_Handle = cb_create(size);
  }
}

/**
 * [CQ_init 环形缓冲区初始化]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  memAdd        [数据存储区]
 * @param  len           [缓冲区大小]
 * @return               [初始化成功状态]
 */
bool CircularQueue::CQ_init(CQ_handleTypeDef *CircularQueue, uint8_t *memAdd, uint32_t len)
{
  CircularQueue->size = len;

  if (!IS_POWER_OF_2(CircularQueue->size))
  {
      return FALSE;
  }

  if(memAdd == nullptr)
  {
      return FALSE;
  }

  CircularQueue->Buffer.data8Buffer = memAdd;

  memset(CircularQueue->Buffer.data8Buffer, 0, len);
  CircularQueue->entrance = CircularQueue->exit = 0;

  return TRUE;
}

/**
 * [CQ_isEmpty 环形缓冲区判断是否为空]
 * @param  CircularQueue [环形缓冲区句柄]
 * @return               [TRUE 为空]
 */
bool CircularQueue::CQ_isEmpty(CQ_handleTypeDef *CircularQueue)
{
  if (CircularQueue->entrance == CircularQueue->exit)
  {
      return TRUE;
  }
  else
  {
      return FALSE;
  }
}

/**
 * [CQ_isFull 环形缓冲区判断是否为满]
 * @param  CircularQueue [环形缓冲区句柄]
 * @return               [TRUE 为满]
 */
bool CircularQueue::CQ_isFull(CQ_handleTypeDef *CircularQueue)
{
  if ((CircularQueue->entrance - CircularQueue->exit) == CircularQueue->size)
  {
      return TRUE;
  }
  else
  {
      return FALSE;
  }
}

/**
 * [CQ_getLength 环形缓冲区获取剩余空间长度]
 * @param  CircularQueue [环形缓冲区句柄]
 * @return               [剩余长度]
 */
uint32_t CircularQueue::CQ_getLength(CQ_handleTypeDef *CircularQueue)
{
  return (CircularQueue->entrance - CircularQueue->exit);
}

/**
 * @brief 环形缓冲区是否够存指定长度数据
 * @param CircularQueue cq
 * @param len 指定长度
 * @return true 能够存入
 */
bool CircularQueue::CQ_canSaveLength(CQ_handleTypeDef *CircularQueue, uint32_t len)
{
  uint32_t size = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);
  if(len != size)
  {
    return false;
  }
  return true;
}

/**
 * [CQ_emptyData 环形缓冲区清空操作]
 * @param  CircularQueue [环形缓冲区句柄]
 * @return               [None]
 */
void CircularQueue::CQ_emptyData(CQ_handleTypeDef*CircularQueue)
{
  CircularQueue->entrance = CircularQueue->exit = 0;
}

/**
 * [CQ_getData 环形缓冲区读走数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  targetBuf     [目标缓冲区]
 * @return               [读取的长度]
 */
uint32_t CircularQueue::CQ_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len)
{
  uint32_t size = 0;

  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));
  memcpy(targetBuf, CircularQueue->Buffer.data8Buffer + (CircularQueue->exit & (CircularQueue->size - 1)), size);
  memcpy(targetBuf + size, CircularQueue->Buffer.data8Buffer, len - size);

  CircularQueue->exit += len;

  return len;
}


/**
 * [CQ_putData 环形缓冲区加入新数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  sourceBuf     [为实际存储区地址]
 * @param  len           [数据存入长度]
 * @return               [存入的长度]
 */
uint32_t CircularQueue::CQ_putData(CQ_handleTypeDef *CircularQueue, const uint8_t *sourceBuf, uint32_t len)
{
  uint32_t size = 0;

  len = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1)));
  memcpy(CircularQueue->Buffer.data8Buffer + (CircularQueue->entrance & (CircularQueue->size - 1)), sourceBuf, size);
  memcpy(CircularQueue->Buffer.data8Buffer, sourceBuf + size, len - size);

  CircularQueue->entrance += len;

  return len;
}

/**
 * [DQ_putData 环形缓冲区加入新数据：每次数据帧开头先存入本帧的数据长度，所以每次先取一个字节得到包长度，再按长度取包]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  sourceBuf     [为实际存储区地址]
 * @param  len           [数据存入长度]
 * @return               [存入的长度]
 */
uint32_t CircularQueue::DQ_putData(CQ_handleTypeDef *CircularQueue, const uint8_t *sourceBuf, uint32_t len)
{
  uint32_t size = 0;
  uint32_t lenth = 1;
  uint32_t pack_len = len;

  len = GET_MIN(len+lenth, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);//长度上头部加上数据长度记录

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1)));
  memcpy(CircularQueue->Buffer.data8Buffer + (CircularQueue->entrance & (CircularQueue->size - 1)), &pack_len, lenth);
  memcpy(CircularQueue->Buffer.data8Buffer + (CircularQueue->entrance & (CircularQueue->size - 1))+lenth, sourceBuf, size-lenth);
  memcpy(CircularQueue->Buffer.data8Buffer, sourceBuf + size - lenth, len - size);

  CircularQueue->entrance += len;

  return len;
}

/**
 * [DQ_getData 环形缓冲区读走数据：DQ会调用CQ取走一字节数据用来判断本次数据包长度]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  targetBuf     [目标缓冲区]
 * @return               [读取的长度]
 */
uint32_t CircularQueue::DQ_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf)
{
  uint32_t size = 0;
  uint32_t len = 0;

  uint8_t package_len[1];

  CQ_getData(CircularQueue, (uint8_t *)package_len, 1);
  len = package_len[0];

  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));
  memcpy(targetBuf, CircularQueue->Buffer.data8Buffer + (CircularQueue->exit & (CircularQueue->size - 1)), size);
  memcpy(targetBuf + size, CircularQueue->Buffer.data8Buffer, len - size);


  CircularQueue->exit += len;

  return len;
}

/**
  ******************************************************************
  * @brief   跳过8位无效帧头数据
  * @param   [in]cb 缓冲区
  * @param   [in]header_data 无效数据
  * @return  缓冲区可读长度
  * @author  aron566
  * @version V1.0
  * @date    2020-09-20
  ******************************************************************
  */
uint32_t CircularQueue::CQ_skipInvaildU8Header(CQ_handleTypeDef *cb, uint8_t header_data)
{
  uint8_t header = 0;
  while(CQ_getLength(cb) >= 1)
  {
      header = CQ_ManualGet_Offset_Data(cb ,0);

      if(header != header_data)
      {
          CQ_ManualOffsetInc(cb, 1);
      }
      else
      {
          return CQ_getLength(cb);
      }
  }
  return 0;
}

/**
  ******************************************************************
  * @brief   跳过16位无效帧头数据
  * @param   [in]cb 缓冲区
  * @param   [in]header_data 无效数据
  * @return  缓冲区可读长度
  * @author  aron566
  * @version V1.0
  * @date    2020-09-20
  ******************************************************************
  */
uint32_t CircularQueue::CQ_skipInvaildU16Header(CQ_handleTypeDef *cb, uint16_t header_data)
{
  uint16_t header = 0;
  while(CQ_getLength(cb) >= 2)
  {
      header = CQ_ManualGet_Offset_Data(cb, 0);
      header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 1))<<8);

      if(header != header_data)
      {
          CQ_ManualOffsetInc(cb, 1);
      }
      else
      {
          return CQ_getLength(cb);
      }
  }
  return 0;
}

/**
  ******************************************************************
  * @brief   跳过32位无效帧头数据
  * @param   [in]cb 缓冲区
  * @param   [in]header_data 无效数据
  * @return  缓冲区可读长度
  * @author  aron566
  * @version V1.0
  * @date    2020-09-20
  ******************************************************************
  */
uint32_t CircularQueue::CQ_skipInvaildU32Header(CQ_handleTypeDef *cb, uint32_t header_data)
{
  uint32_t header = 0;
  while(CQ_getLength(cb) >= 4)
  {
      header = CQ_ManualGet_Offset_Data(cb, 0);
      header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 1))<<8);
      header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 2))<<16);
      header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 3))<<24);

      if(header != header_data)
      {
          CQ_ManualOffsetInc(cb, 1);
      }
      else
      {
          return CQ_getLength(cb);
      }
  }
  return 0;
}

/**
  ******************************************************************
  * @brief   跳过Modbus16位无效帧头数据
  * @param   [in]cb 缓冲区
  * @param   [in]header_data 无效数据
  * @return  缓冲区可读长度
  * @author  aron566
  * @version V1.0
  * @date    2020-09-20
  ******************************************************************
  */
uint32_t CircularQueue::CQ_skipInvaildModbusU16Header(CQ_handleTypeDef *cb, uint16_t header_data)
{
  uint16_t header = 0;
  while(CQ_getLength(cb) >= 2)
  {
      header = (((uint16_t)CQ_ManualGet_Offset_Data(cb, 0))<<8);
      header |= CQ_ManualGet_Offset_Data(cb, 1);

      if(header != header_data)
      {
          CQ_ManualOffsetInc(cb, 1);
      }
      else
      {
          return CQ_getLength(cb);
      }
  }
  return 0;
}

/**
  ******************************************************************
  * @brief   跳过Modbus32位无效帧头数据
  * @param   [in]cb 缓冲区
  * @param   [in]header_data 无效数据
  * @return  缓冲区可读长度
  * @author  aron566
  * @version V1.0
  * @date    2020-09-20
  ******************************************************************
  */
uint32_t CircularQueue::CQ_skipInvaildModbusU32Header(CQ_handleTypeDef *cb, uint32_t header_data)
{
  uint32_t header = 0;
  while(CQ_getLength(cb) >= 4)
  {
      header = (((uint16_t)CQ_ManualGet_Offset_Data(cb, 0))<<24);
      header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 1))<<16);
      header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 2))<<8);
      header |= CQ_ManualGet_Offset_Data(cb, 3);

      if(header != header_data)
      {
          CQ_ManualOffsetInc(cb, 1);
      }
      else
      {
          return CQ_getLength(cb);
      }
  }
  return 0;
}

/**
 * [CQ_ManualGetData 手动缓冲区长度记录---适用于modbus解析]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  targetBuf     [目标缓冲区]
 * @param  len           [数据读取长度]
 * @return               [读取的长度]
 */
uint32_t CircularQueue::CQ_ManualGetData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len)
{
  uint32_t size = 0;

  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));
  memmove(targetBuf, CircularQueue->Buffer.data8Buffer + (CircularQueue->exit & (CircularQueue->size - 1)), size);
  memmove(targetBuf + size, CircularQueue->Buffer.data8Buffer, len - size);

  return len;
}

/**
 * [CQ_ManualGet_Offset_Data 读取指定索引号的数据]
 * @param CircularQueue [环形缓冲区句柄]
 * @param index         [索引号]
 */
uint8_t CircularQueue::CQ_ManualGet_Offset_Data(CQ_handleTypeDef *CircularQueue, uint32_t index)
{
  uint32_t read_offset = ((CircularQueue->exit + index) & (CircularQueue->size - 1));

  uint8_t data = *((uint8_t*)CircularQueue->Buffer.data8Buffer + read_offset);

  return data;
}

/**
 * [CQ_ManualOffsetInc 手动增加已取出长度]
 * @param CircularQueue [环形缓冲区句柄]
 * @param len           [偏移长度]
 */
void CircularQueue::CQ_ManualOffsetInc(CQ_handleTypeDef *CircularQueue, uint32_t len)
{
  len = GET_MIN(CQ_getLength(CircularQueue), len);
  CircularQueue->exit += len;
}

/**
 * [cb_create 申请并初始化环形缓冲区]
 * @param  buffsize [申请环形缓冲区大小]
 * @return          [环形队列管理句柄]
 */
CircularQueue::CQ_handleTypeDef *CircularQueue::cb_create(uint32_t buffsize)
{
  if (!IS_POWER_OF_2(buffsize))
  {
      qDebug() << "not power 2";
      return nullptr;
  }

  CQ_handleTypeDef *cb = new CQ_handleTypeDef;//(CQ_handleTypeDef *)calloc(1, sizeof(CQ_handleTypeDef));
  if(nullptr == cb)
  {
      qDebug() << "nullptr == cb";
      return nullptr;
  }
  cb->size = buffsize;
  cb->exit = 0;
  cb->entrance = 0;
  //the buff never release!
  cb->Buffer.data8Buffer = new uint8_t[cb->size];//(uint8_t *)calloc((size_t)cb->size, sizeof(uint8_t));
  if(nullptr == cb->Buffer.data8Buffer)
  {
      return nullptr;
  }
  cb->is_malloc = true;
  return cb;
}

/**
 * @brief 删除一个缓冲区
 *
 * @param CircularQueue
 */
void CircularQueue::cb_delete(CQ_handleTypeDef *CircularQueue)
{
  if(CircularQueue == nullptr)
  {
      return;
  }
  if(CircularQueue->is_malloc == false)
  {
      return;
  }
  delete [] CircularQueue->Buffer.data8Buffer;
  delete CircularQueue;
//    free(CircularQueue->Buffer.data8Buffer);
//    free(CircularQueue);
}

/**
 * [CQ_16_init 静态初始化16bit环形缓冲区]
 * @param  CircularQueue [缓冲区指针]
 * @param  memAdd        [uint16_t 缓冲区地址]
 * @param  len           [缓冲区长度>1]
 * @return               [初始化状态]
 */
bool CircularQueue::CQ_16_init(CQ_handleTypeDef *CircularQueue, uint16_t *memAdd, uint32_t len)
{
  CircularQueue->size = len;

  if (!IS_POWER_OF_2(CircularQueue->size))
  {
      return FALSE;
  }

  if(memAdd == nullptr)
  {
      return FALSE;
  }

  CircularQueue->Buffer.data16Buffer = memAdd;

  memset(CircularQueue->Buffer.data16Buffer, 0, len*2);
  CircularQueue->entrance = CircularQueue->exit = 0;

  return TRUE;
}

/**
 * [cb_16create 动态申请并初始化环形缓冲区]
 * @param  buffsize [申请环形缓冲区大小]
 * @return          [环形队列管理句柄]
 */
CircularQueue::CQ_handleTypeDef *CircularQueue::cb_16create(uint32_t buffsize)
{
  if (!IS_POWER_OF_2(buffsize))
  {
      return nullptr;
  }

  CQ_handleTypeDef *cb = new CQ_handleTypeDef;//(CQ_handleTypeDef *)calloc(1, sizeof(CQ_handleTypeDef));
  if(nullptr == cb)
  {
      return nullptr;
  }
  buffsize = (buffsize <= 2048 ? buffsize : 2048);
  cb->size = buffsize;
  cb->exit = 0;
  cb->entrance = 0;
  //the buff never release!
  cb->Buffer.data16Buffer = new uint16_t[cb->size];//(uint16_t *)calloc((size_t)cb->size, sizeof(uint16_t));
  if(nullptr == cb->Buffer.data16Buffer)
  {
      return nullptr;
  }
  cb->is_malloc = true;
  return cb;
}

/**
 * [CQ_16getData 取出数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  targetBuf     [目标地址]
 * @param  len           [取出长度]
 * @return               [取出长度]
 */
uint32_t CircularQueue::CQ_16getData(CQ_handleTypeDef *CircularQueue, uint16_t *targetBuf, uint32_t len)
{
  uint32_t size = 0;
  uint32_t len_temp = 0;
  uint32_t size_temp = 0;

  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));

  len_temp = 2*len;
  size_temp = 2*size;

  memcpy(targetBuf, CircularQueue->Buffer.data16Buffer + (CircularQueue->exit & (CircularQueue->size - 1)), size_temp);
  memcpy(targetBuf + size, CircularQueue->Buffer.data16Buffer, len_temp - size_temp);

  CircularQueue->exit += len;

  return len;
}


/**
 * [CQ_16putData 加入数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  sourceBuf     [源地址]
 * @param  len           [长度]
 * @return               [加入数据长度]
 */
uint32_t CircularQueue::CQ_16putData(CQ_handleTypeDef *CircularQueue, const uint16_t *sourceBuf, uint32_t len)
{
  uint32_t size = 0;
  uint32_t len_temp = 0;
  uint32_t size_temp = 0;

  len = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1)));

  len_temp = 2*len;
  size_temp = 2*size;

  memcpy(CircularQueue->Buffer.data16Buffer + (CircularQueue->entrance & (CircularQueue->size - 1)), sourceBuf, size_temp);
  memcpy(CircularQueue->Buffer.data16Buffer, sourceBuf + size, len_temp - size_temp);

  CircularQueue->entrance += len;

  return len;
}

/**
 * [CQ_32_init 静态初始化32bit环形缓冲区]
 * @param  CircularQueue [缓冲区指针]
 * @param  memAdd        [uint32_t 缓冲区地址]
 * @param  len           [缓冲区长度>1]
 * @return               [初始化状态]
 */
bool CircularQueue::CQ_32_init(CQ_handleTypeDef *CircularQueue, uint32_t *memAdd, uint32_t len)
{
  CircularQueue->size = len;

  if (!IS_POWER_OF_2(CircularQueue->size))
  {
      return FALSE;
  }

  if(memAdd == nullptr)
  {
      return FALSE;
  }

  CircularQueue->Buffer.data32Buffer = memAdd;

  memset(CircularQueue->Buffer.data32Buffer, 0, len*4);
  CircularQueue->entrance = CircularQueue->exit = 0;

  return TRUE;
}

/**
 * [cb_32create 动态申请并初始化环形缓冲区]
 * @param  buffsize [申请环形缓冲区大小]
 * @return          [环形队列管理句柄]
 */
CircularQueue::CQ_handleTypeDef *CircularQueue::cb_32create(uint32_t buffsize)
{
  if (!IS_POWER_OF_2(buffsize))
  {
      return nullptr;
  }

  CQ_handleTypeDef *cb = new CQ_handleTypeDef;//(CQ_handleTypeDef *)calloc(1, sizeof(CQ_handleTypeDef));
  if(nullptr == cb)
  {
      return nullptr;
  }
  buffsize = (buffsize <= 2048 ? buffsize : 2048);
  cb->size = buffsize;
  cb->exit = 0;
  cb->entrance = 0;

  cb->Buffer.data32Buffer = new uint32_t[cb->size];//(uint32_t *)calloc((size_t)cb->size, sizeof(uint32_t));
  if(nullptr == cb->Buffer.data32Buffer)
  {
      return nullptr;
  }
  cb->is_malloc = true;
  return cb;
}

/**
 * [CQ_32putData 加入数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  sourceBuf     [源地址]
 * @param  len           [长度]
 * @return               [加入数据长度]
 */
uint32_t CircularQueue::CQ_32putData(CQ_handleTypeDef *CircularQueue, const uint32_t * sourceBuf, uint32_t len)
{
  uint32_t size = 0;
  uint32_t len_temp = 0;
  uint32_t size_temp = 0;

  len = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1)));

  len_temp = 4*len;
  size_temp = 4*size;

  memcpy(CircularQueue->Buffer.data32Buffer + (CircularQueue->entrance & (CircularQueue->size - 1)), sourceBuf, size_temp);
  memcpy(CircularQueue->Buffer.data32Buffer, sourceBuf + size, len_temp - size_temp);

  CircularQueue->entrance += len;

  return len;
}

/**
 * [CQ_32getData 取出数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  targetBuf     [目标地址]
 * @param  len           [取出长度]
 * @return               [取出长度]
 */
uint32_t CircularQueue::CQ_32getData(CQ_handleTypeDef *CircularQueue, uint32_t *targetBuf, uint32_t len)
{
  uint32_t size = 0;
  uint32_t len_temp = 0;
  uint32_t size_temp = 0;

  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));

  len_temp = 4*len;
  size_temp = 4*size;

  memcpy(targetBuf, CircularQueue->Buffer.data32Buffer + (CircularQueue->exit & (CircularQueue->size - 1)), size_temp);
  memcpy(targetBuf + size, CircularQueue->Buffer.data32Buffer, len_temp - size_temp);

  CircularQueue->exit += len;

  return len;
}
/******************************** End of file *********************************/
