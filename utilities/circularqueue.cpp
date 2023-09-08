/**
 *  @file circularqueue.cpp
 *
 *  @date 2023年02月20日 17:05:52 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief 环形缓冲区管理.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.02.20 17:05 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-02-20 <td>v0.0.1  <td>aron566 <td>初始版本
 *  <tr><td>2023-09-08 <td>v1.0.1  <td>aron566 <td>完善版本，增加安全机制
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <QDebug>
/** Private includes ---------------------------------------------------------*/
#include "circularqueue.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define GET_MIN(a, b)    (((a) < (b)) ? (a) : (b))
#define IS_POWER_OF_2(x) ((x) != 0 && (((x) & ((x)-1U)) == 0))

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

CircularQueue::CircularQueue(DATA_TYPE_Typedef_t type, CQ_BUF_SIZE_ENUM_TypeDef size, QObject *parent)
    : QObject{parent}
{
  if (size == 0)
  {
    return;
  }

  if (type == UINT8_DATA_BUF)
  {
    CQ_Handle = CQ_create(size);
  }
}

bool CircularQueue::CQ_init(CQ_handleTypeDef *CircularQueue, uint8_t *memAdd, uint32_t len)
{
  CircularQueue->size = len;

  if (!IS_POWER_OF_2(CircularQueue->size))
  {
    return false;
  }

  if (memAdd == nullptr)
  {
    return false;
  }

  CircularQueue->Buffer.data8Buffer = memAdd;

  memset(CircularQueue->Buffer.data8Buffer, 0, len);
  CircularQueue->entrance = CircularQueue->exit = 0;

  return true;
}

bool CircularQueue::CQ_isEmpty(CQ_handleTypeDef *CircularQueue)
{
  if (CircularQueue->entrance == CircularQueue->exit)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool CircularQueue::CQ_isFull(CQ_handleTypeDef *CircularQueue)
{
  if ((CircularQueue->entrance - CircularQueue->exit) == CircularQueue->size)
  {
    return true;
  }
  else
  {
    return false;
  }
}

uint32_t CircularQueue::CQ_getLength(CQ_handleTypeDef *CircularQueue)
{
  return (CircularQueue->entrance - CircularQueue->exit);
}

bool CircularQueue::CQ_canSaveLength(CQ_handleTypeDef *CircularQueue, uint32_t len)
{
  uint32_t size = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);
  if (len != size)
  {
    return false;
  }
  return true;
}

void CircularQueue::CQ_emptyData(CQ_handleTypeDef *CircularQueue)
{
  CircularQueue->entrance = CircularQueue->exit = 0;
}

uint32_t CircularQueue::CQ_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len)
{
  uint32_t size = 0;

  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1U)));
  memcpy(targetBuf, CircularQueue->Buffer.data8Buffer + (CircularQueue->exit & (CircularQueue->size - 1U)), size);
  memcpy(targetBuf + size, CircularQueue->Buffer.data8Buffer, len - size);

  CircularQueue->exit += len;

  return len;
}

uint32_t CircularQueue::CQ_putData(CQ_handleTypeDef *CircularQueue, const uint8_t *sourceBuf, uint32_t len)
{
  uint32_t size = 0;

  len = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1U)));
  memcpy(CircularQueue->Buffer.data8Buffer + (CircularQueue->entrance & (CircularQueue->size - 1U)), sourceBuf, size);
  memcpy(CircularQueue->Buffer.data8Buffer, sourceBuf + size, len - size);

  CircularQueue->entrance += len;

  return len;
}

uint32_t CircularQueue::CQ_skipInvaildU8Header(CQ_handleTypeDef *CircularQueue, uint8_t headerData)
{
  uint8_t header = 0;
  while (CQ_getLength(CircularQueue) >= 1U)
  {
    header = CQ_manualGetOffsetData(CircularQueue, 0);

    if (header != headerData)
    {
      CQ_manualOffsetInc(CircularQueue, 1U);
    }
    else
    {
      return CQ_getLength(CircularQueue);
    }
  }
  return 0;
}

uint32_t CircularQueue::CQ_skipInvaildU16Header(CQ_handleTypeDef *CircularQueue, uint16_t headerData)
{
  uint16_t header = 0;
  while (CQ_getLength(CircularQueue) >= 2U)
  {
    header = CQ_manualGetOffsetData(CircularQueue, 0);
    header |= (((uint16_t)CQ_manualGetOffsetData(CircularQueue, 1U)) << 8);

    if (header != headerData)
    {
      CQ_manualOffsetInc(CircularQueue, 1U);
    }
    else
    {
      return CQ_getLength(CircularQueue);
    }
  }
  return 0;
}

uint32_t CircularQueue::CQ_skipInvaildU32Header(CQ_handleTypeDef *CircularQueue, uint32_t headerData)
{
  uint32_t header = 0;
  while (CQ_getLength(CircularQueue) >= 4U)
  {
    header = CQ_manualGetOffsetData(CircularQueue, 0);
    header |= (((uint16_t)CQ_manualGetOffsetData(CircularQueue, 1U)) << 8);
    header |= (((uint16_t)CQ_manualGetOffsetData(CircularQueue, 2U)) << 16);
    header |= (((uint16_t)CQ_manualGetOffsetData(CircularQueue, 3)) << 24);

    if (header != headerData)
    {
      CQ_manualOffsetInc(CircularQueue, 1U);
    }
    else
    {
      return CQ_getLength(CircularQueue);
    }
  }
  return 0;
}

uint32_t CircularQueue::CQ_skipInvaildModbusU16Header(CQ_handleTypeDef *CircularQueue, uint16_t headerData)
{
  uint16_t header = 0;
  while (CQ_getLength(CircularQueue) >= 2U)
  {
    header = (((uint16_t)CQ_manualGetOffsetData(CircularQueue, 0)) << 8U);
    header |= CQ_manualGetOffsetData(CircularQueue, 1U);

    if (header != headerData)
    {
      CQ_manualOffsetInc(CircularQueue, 1U);
    }
    else
    {
      return CQ_getLength(CircularQueue);
    }
  }
  return 0;
}

uint32_t CircularQueue::CQ_skipInvaildModbusU32Header(CQ_handleTypeDef *CircularQueue, uint32_t headerData)
{
  uint32_t header = 0;
  while (CQ_getLength(CircularQueue) >= 4U)
  {
    header = (((uint16_t)CQ_manualGetOffsetData(CircularQueue, 0)) << 24);
    header |= (((uint16_t)CQ_manualGetOffsetData(CircularQueue, 1U)) << 16);
    header |= (((uint16_t)CQ_manualGetOffsetData(CircularQueue, 2U)) << 8);
    header |= CQ_manualGetOffsetData(CircularQueue, 3);

    if (header != headerData)
    {
      CQ_manualOffsetInc(CircularQueue, 1U);
    }
    else
    {
      return CQ_getLength(CircularQueue);
    }
  }
  return 0;
}

uint32_t CircularQueue::CQ_manualGetDataTemp(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len)
{
  uint32_t size = 0;

  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1U)));
  memmove(targetBuf, CircularQueue->Buffer.data8Buffer + (CircularQueue->exit & (CircularQueue->size - 1U)), size);
  memmove(targetBuf + size, CircularQueue->Buffer.data8Buffer, len - size);

  return len;
}

uint8_t CircularQueue::CQ_manualGetOffsetData(CQ_handleTypeDef *CircularQueue, uint32_t index)
{
  uint32_t readOffset = ((CircularQueue->exit + index) & (CircularQueue->size - 1U));

  uint8_t data = *((uint8_t *)CircularQueue->Buffer.data8Buffer + readOffset);

  return data;
}

void CircularQueue::CQ_manualOffsetInc(CQ_handleTypeDef *CircularQueue, uint32_t len)
{
  len = GET_MIN(CQ_getLength(CircularQueue), len);
  CircularQueue->exit += len;
}

CircularQueue::CQ_handleTypeDef *CircularQueue::CQ_create(uint32_t buffSize)
{
  if (!IS_POWER_OF_2(buffSize))
  {
    qDebug() << "not power 2U";
    return nullptr;
  }

  CQ_handleTypeDef *CircularQueue = new CQ_handleTypeDef;
  if (nullptr == CircularQueue)
  {
    qDebug() << "nullptr == CircularQueue";
    return nullptr;
  }
  CircularQueue->size     = buffSize;
  CircularQueue->exit     = 0;
  CircularQueue->entrance = 0;
  // the buff never release!
  CircularQueue->Buffer.data8Buffer = new uint8_t[CircularQueue->size];
  if (nullptr == CircularQueue->Buffer.data8Buffer)
  {
    return nullptr;
  }
  CircularQueue->isMalloc = true;
  return CircularQueue;
}

void CircularQueue::CQ_delete(CQ_handleTypeDef *CircularQueue)
{
  if (CircularQueue == nullptr)
  {
    return;
  }
  if (CircularQueue->isMalloc == false)
  {
    return;
  }
  delete[] CircularQueue->Buffer.data8Buffer;
  delete CircularQueue;
}

bool CircularQueue::CQ_16_init(CQ_handleTypeDef *CircularQueue, uint16_t *memAdd, uint32_t len)
{
  CircularQueue->size = len;

  if (!IS_POWER_OF_2(CircularQueue->size))
  {
    return false;
  }

  if (memAdd == nullptr)
  {
    return false;
  }

  CircularQueue->Buffer.data16Buffer = memAdd;

  memset(CircularQueue->Buffer.data16Buffer, 0, len * 2U);
  CircularQueue->entrance = CircularQueue->exit = 0;

  return true;
}

CircularQueue::CQ_handleTypeDef *CircularQueue::CQ_16_create(uint32_t buffSize)
{
  if (!IS_POWER_OF_2(buffSize))
  {
    return nullptr;
  }

  CQ_handleTypeDef *CircularQueue = new CQ_handleTypeDef;
  if (nullptr == CircularQueue)
  {
    return nullptr;
  }
  buffSize     = (buffSize <= 2048U ? buffSize : 2048U);
  CircularQueue->size     = buffSize;
  CircularQueue->exit     = 0;
  CircularQueue->entrance = 0;
  // the buff never release!
  CircularQueue->Buffer.data16Buffer = new uint16_t[CircularQueue->size];
  if (nullptr == CircularQueue->Buffer.data16Buffer)
  {
    return nullptr;
  }
  CircularQueue->isMalloc = true;
  return CircularQueue;
}

uint32_t CircularQueue::CQ_16_getData(CQ_handleTypeDef *CircularQueue, uint16_t *targetBuf, uint32_t len)
{
  uint32_t size      = 0;
  uint32_t len_temp  = 0;
  uint32_t size_temp = 0;

  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1U)));

  len_temp  = 2U * len;
  size_temp = 2U * size;

  memcpy(targetBuf, CircularQueue->Buffer.data16Buffer + (CircularQueue->exit & (CircularQueue->size - 1U)), size_temp);
  memcpy(targetBuf + size, CircularQueue->Buffer.data16Buffer, len_temp - size_temp);

  CircularQueue->exit += len;

  return len;
}

uint32_t CircularQueue::CQ_16_putData(CQ_handleTypeDef *CircularQueue, const uint16_t *sourceBuf, uint32_t len)
{
  uint32_t size      = 0;
  uint32_t len_temp  = 0;
  uint32_t size_temp = 0;

  len = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1U)));

  len_temp  = 2U * len;
  size_temp = 2U * size;

  memcpy(CircularQueue->Buffer.data16Buffer + (CircularQueue->entrance & (CircularQueue->size - 1U)), sourceBuf, size_temp);
  memcpy(CircularQueue->Buffer.data16Buffer, sourceBuf + size, len_temp - size_temp);

  CircularQueue->entrance += len;

  return len;
}

bool CircularQueue::CQ_32_init(CQ_handleTypeDef *CircularQueue, uint32_t *memAdd, uint32_t len)
{
  CircularQueue->size = len;

  if (!IS_POWER_OF_2(CircularQueue->size))
  {
    return false;
  }

  if (memAdd == nullptr)
  {
    return false;
  }

  CircularQueue->Buffer.data32Buffer = memAdd;

  memset(CircularQueue->Buffer.data32Buffer, 0, len * 4U);
  CircularQueue->entrance = CircularQueue->exit = 0;

  return true;
}

CircularQueue::CQ_handleTypeDef *CircularQueue::CQ_32_create(uint32_t buffSize)
{
  if (!IS_POWER_OF_2(buffSize))
  {
    return nullptr;
  }

  CQ_handleTypeDef *CircularQueue = new CQ_handleTypeDef;
  if (nullptr == CircularQueue)
  {
    return nullptr;
  }
  buffSize     = (buffSize <= 2048U ? buffSize : 2048U);
  CircularQueue->size     = buffSize;
  CircularQueue->exit     = 0;
  CircularQueue->entrance = 0;

  CircularQueue->Buffer.data32Buffer = new uint32_t[CircularQueue->size];
  if (nullptr == CircularQueue->Buffer.data32Buffer)
  {
    return nullptr;
  }
  CircularQueue->isMalloc = true;
  return CircularQueue;
}

uint32_t CircularQueue::CQ_32_putData(CQ_handleTypeDef *CircularQueue, const uint32_t *sourceBuf, uint32_t len)
{
  uint32_t size      = 0;
  uint32_t len_temp  = 0;
  uint32_t size_temp = 0;

  len = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1U)));

  len_temp  = 4U * len;
  size_temp = 4U * size;

  memcpy(CircularQueue->Buffer.data32Buffer + (CircularQueue->entrance & (CircularQueue->size - 1U)), sourceBuf, size_temp);
  memcpy(CircularQueue->Buffer.data32Buffer, sourceBuf + size, len_temp - size_temp);

  CircularQueue->entrance += len;

  return len;
}

uint32_t CircularQueue::CQ_32_getData(CQ_handleTypeDef *CircularQueue, uint32_t *targetBuf, uint32_t len)
{
  uint32_t size      = 0;
  uint32_t len_temp  = 0;
  uint32_t size_temp = 0;

  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);

  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1U)));

  len_temp  = 4U * len;
  size_temp = 4U * size;

  memcpy(targetBuf, CircularQueue->Buffer.data32Buffer + (CircularQueue->exit & (CircularQueue->size - 1U)), size_temp);
  memcpy(targetBuf + size, CircularQueue->Buffer.data32Buffer, len_temp - size_temp);

  CircularQueue->exit += len;

  return len;
}

/******************************** End of file *********************************/
