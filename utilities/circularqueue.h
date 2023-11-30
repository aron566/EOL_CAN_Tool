/**
 *  @file circularqueue.hpp
 *
 *  @date 2023年02月20日 17:08:26 星期一
 *
 *  @author Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @version v1.0.0 aron566 2023.02.20 17:08 初始版本.
 */
#ifndef CIRCULARQUEUE_H
  #define CIRCULARQUEUE_H
  /** Includes -----------------------------------------------------------------*/
  #include <stdint.h> /**< need definition of uint8_t */
  #include <stddef.h> /**< need definition of NULL    */
  // #include <stdbool.h>/**< need definition of BOOL    */
  #include <stdio.h> /**< if need printf             */
  #include <stdlib.h>
  #include <string.h>
  /** Private includes ---------------------------------------------------------*/
  #include <QObject>
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/
class CircularQueue : public QObject
{
  Q_OBJECT
  public:
  explicit CircularQueue(QObject *parent = nullptr);

  // signals:
  public:
  /** 数据结构体 */
  typedef struct
  {
    union
    {
      uint8_t  *data8Buffer;  /**< for 8bit buffer*/
      uint16_t *data16Buffer; /**< for 16bit buffer*/
      uint32_t *data32Buffer; /**< for 32bit buffer*/
      uint64_t *data64Buffer; /**< for 64bit buffer*/
    } Buffer;
    quint32 size;
    quint32 entrance;
    quint32 exit;
    bool    isMalloc;
  } CQ_handleTypeDef;

  /* 缓冲区长度 */
  typedef enum
  {
    CQ_BUF_128 = 1U << 7U,
    CQ_BUF_256 = 1U << 8U,
    CQ_BUF_512 = 1U << 9U,
    CQ_BUF_1K  = 1U << (10U * 1U),   ///< 1*10
    CQ_BUF_2K  = 1U << 11U,
    CQ_BUF_4K  = 1U << 12U,
    CQ_BUF_8K  = 1U << 13U,
    CQ_BUF_1M  = 1U << (10U * 2U),   ///< 2*10
    CQ_BUF_1G  = 1U << (10U * 3U),   ///< 3*10
    // CQ_BUF_1T 	= 1 << (10 * 4), ///< 4*10
    // CQ_BUF_1P 	= 1 << (10 * 5), ///< 5*10
  } CQ_BUF_SIZE_ENUM_TypeDef;

  typedef enum
  {
    UINT8_DATA_BUF = 0,
    UINT16_DATA_BUF,
    UINT32_DATA_BUF,
    UINT64_DATA_BUF
  } DATA_TYPE_Typedef_t;

  public:
  CircularQueue(DATA_TYPE_Typedef_t type, CQ_BUF_SIZE_ENUM_TypeDef size, QObject *parent = nullptr);
  ~CircularQueue()
  {
    if(nullptr != CQ_Handle)
    {
      delete[] CQ_Handle->Buffer.data64Buffer; /**< 删除缓冲区 */
      delete CQ_Handle;                        /**< 删除句柄 */
    }
  }

  public:
  /**
   * @brief 缓冲区是否为空
   * @param CircularQueue 缓冲区句柄
   * @return true 为空
   */
  static bool CQ_isEmpty(CircularQueue::CQ_handleTypeDef *CircularQueue);

  /**
   * @brief 缓冲区是否为满
   * @param CircularQueue 缓冲区句柄
   * @return true 为满
   */
  static bool CQ_isFull(CircularQueue::CQ_handleTypeDef *CircularQueue);

  /**
   * @brief 清空缓冲区数据
   * @param CircularQueue 缓冲区句柄
   */
  static void CQ_emptyData(CircularQueue::CQ_handleTypeDef *CircularQueue);

  /**
   * @brief 获取缓冲区可用数据长度
   * @param CircularQueue 缓冲区句柄
   * @return 长度
   */
  static uint32_t CQ_getLength(CircularQueue::CQ_handleTypeDef *CircularQueue);

  /**
   * @brief 判断缓冲区是否可存入指定长度数据
   * @param CircularQueue 缓冲区句柄
   * @param len 指定长度
   * @return true 可存
   */
  static bool CQ_canSaveLength(CQ_handleTypeDef *CircularQueue, uint32_t len);

  /**
   * @brief 手动增加已取出长度
   * @param CircularQueue 缓冲区句柄
   * @param len 指定长度
   */
  static void CQ_manualOffsetInc(CircularQueue::CQ_handleTypeDef *CircularQueue, uint32_t len);

  /**
   * @brief 跳转到指定有效数据，用于快速丢弃无效头部数据
   * @param CircularQueue 缓冲区句柄
   * @param headerData 需要的头部数据
   * @return uint32_t 缓冲区可读长度
   */
  static uint32_t CQ_skipInvaildU8Header(CircularQueue::CQ_handleTypeDef *CircularQueue, uint8_t headerData);

  /**
   * @brief 跳转到指定有效数据，用于快速丢弃无效头部数据 modbus 高位在前-低位在后存储
   * @param CircularQueue 缓冲区句柄
   * @param headerData 需要的头部数据
   * @return uint32_t 缓冲区可读长度
   */
  static uint32_t CQ_skipInvaildModbusU16Header(CircularQueue::CQ_handleTypeDef *CircularQueue, uint16_t headerData);

  /**
   * @brief 跳转到指定有效数据，用于快速丢弃无效头部数据 modbus 高位在前-低位在后存储
   * @param CircularQueue 缓冲区句柄
   * @param headerData 需要的头部数据
   * @return uint32_t 缓冲区可读长度
   */
  static uint32_t CQ_skipInvaildModbusU32Header(CircularQueue::CQ_handleTypeDef *CircularQueue, uint32_t headerData);

  /**
   * @brief 跳转到指定有效数据，用于快速丢弃无效头部数据 低位在前-高位在后
   * @param CircularQueue 缓冲区句柄
   * @param headerData 需要的头部数据
   * @return uint32_t 缓冲区可读长度
   */
  static uint32_t CQ_skipInvaildU32Header(CircularQueue::CQ_handleTypeDef *CircularQueue, uint32_t headerData);

  /**
   * @brief 跳转到指定有效数据，用于快速丢弃无效头部数据 低位在前-高位在后
   * @param CircularQueue 缓冲区句柄
   * @param headerData 需要的头部数据
   * @return uint32_t 缓冲区可读长度
   */
  static uint32_t CQ_skipInvaildU16Header(CircularQueue::CQ_handleTypeDef *CircularQueue, uint16_t headerData);

  /**
   * @brief 删除一个由CQ_xx_create创建的缓冲区
   * @param CircularQueue 缓冲区句柄
   */
  void CQ_delete(CircularQueue::CQ_handleTypeDef *CircularQueue);

  /**
   * @brief 环形缓冲区初始化
   * @param CircularQueue 缓冲区句柄
   * @param memAdd 缓冲区地址
   * @param len 缓冲区字节数
   * @return true 初始化成功
   * @return false 初始化失败
   */
  static bool CQ_init(CircularQueue::CQ_handleTypeDef *CircularQueue, uint8_t *memAdd, uint32_t len);
  static bool CQ_16_init(CircularQueue::CQ_handleTypeDef *CircularQueue, uint16_t *memAdd, uint32_t len);
  static bool CQ_32_init(CircularQueue::CQ_handleTypeDef *CircularQueue, uint32_t *memAdd, uint32_t len);

  /**
   * @brief 动态申请缓冲区并进行初始化，需要手动释放
   * @param buffSize 缓冲区大小
   * @return CQ_handleTypeDef* 缓冲区句柄
   */
  static CQ_handleTypeDef *CQ_create(uint32_t buffSize);
  static CQ_handleTypeDef *CQ_16_create(uint32_t buffSize);
  static CQ_handleTypeDef *CQ_32_create(uint32_t buffSize);

  /**
   * @brief 获取缓冲区数据
   * @param CircularQueue 缓冲区句柄
   * @param targetBuf 存储区
   * @param len 需要获取的字节数
   * @return uint32_t 实际获取的长度
   */
  static uint32_t CQ_getData(CircularQueue::CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len);
  static uint32_t CQ_16_getData(CircularQueue::CQ_handleTypeDef *CircularQueue, uint16_t *targetBuf, uint32_t len);
  static uint32_t CQ_32_getData(CircularQueue::CQ_handleTypeDef *CircularQueue, uint32_t *targetBuf, uint32_t len);

  /**
   * @brief 加入数据到缓冲区
   * @param CircularQueue 缓冲区句柄
   * @param sourceBuf 加入数据的地址
   * @param len 加入数据的字节数
   * @return uint32_t 实际加入的字节数
   */
  static uint32_t CQ_putData(CircularQueue::CQ_handleTypeDef *CircularQueue, const uint8_t *sourceBuf, uint32_t len);
  static uint32_t CQ_16_putData(CircularQueue::CQ_handleTypeDef *CircularQueue, const uint16_t *sourceBuf, uint32_t len);
  static uint32_t CQ_32_putData(CircularQueue::CQ_handleTypeDef *CircularQueue, const uint32_t *sourceBuf, uint32_t len);

  /**
   * @brief 临时获取缓冲区数据，不增加已取长度
   * @param CircularQueue 缓冲区句柄
   * @param targetBuf 数据存储区
   * @param len 获取的长度
   * @return uint32_t 实际获取的长度
   */
  static uint32_t CQ_manualGetDataTemp(CircularQueue::CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len);

  /**
   * @brief 获取缓冲区指定偏移地址的数据
   * @param CircularQueue 缓冲区句柄
   * @param index 索引
   * @return uint8_t 数据
   */
  static uint8_t CQ_manualGetOffsetData(CircularQueue::CQ_handleTypeDef *CircularQueue, uint32_t index);

  /**
   * @brief 获取CQ_句柄
   * @return CQ_handleTypeDef* 缓冲区句柄
   */
  CQ_handleTypeDef *CQ_getCQHandle()
  {
    return CQ_Handle;
  }

  private:
  CQ_handleTypeDef *CQ_Handle = nullptr;
};
#endif   // CIRCULARQUEUE_H
/******************************** End of file *********************************/
