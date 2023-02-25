/**
 *  @file eol_protocol.hpp
 *
 *  @date 2023年02月20日 17:08:26 星期一
 *
 *  @author Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @version v1.0.0 aron566 2023.02.20 17:08 初始版本.
 */
#ifndef _EOL_PROTOCOL_H
#define _EOL_PROTOCOL_H
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
/** Private includes ---------------------------------------------------------*/
#include <QObject>
#include <QCoreApplication>
#include <QThread>
#include <QRunnable>
#include "circularqueue.h"
#include "can_driver.h"
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/

/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class eol_protocol : public QObject, public QRunnable
{
  Q_OBJECT
public:
  explicit eol_protocol(QObject *parent = nullptr);

  ~eol_protocol()
  {

  }

  /* 操作返回值 */
  typedef enum
  {
    DOA_TABLE_OPT_OK = 0,                 /**< 无错误 */
    DOA_TABLE_OPT_CRC_ERR,                /**< CRC校验错误 */
    DOA_TABLE_OPT_R_HEADER_ERR,           /**< 读表头部错误 */
    DOA_TABLE_OPT_R_DATA_ERR,             /**< 读数据错误 */
    DOA_TABLE_OPT_W_HEADER_ERR,           /**< 写表头部错误 */
    DOA_TABLE_OPT_W_DATA_ERR,             /**< 写数据错误 */
    DOA_TABLE_OPT_RESEV0_ERR,             /**< 未使用错误 */
    DOA_TABLE_OPT_RESEV1_ERR,             /**< 未使用错误 */
    DOA_TABLE_OPT_ERASE_ERR,              /**< 擦除错误 */
    DOA_UNKNOW_TAB_ERR,                   /**< 表类型错误 */
    DOA_TABLE_OVER_SIZE,                  /**< 数据过大溢出 */
    DOA_UNKNOW_CMD_ERR,                   /**< 未知命令类型错误 */
  }DOA_TABLE_OPT_STATUS_Typedef_t;

  /* 导向矢量数据格式 */
  typedef enum
  {
    DOA_CALTERAH_CFX_28BIT_DATA_TYPE = 0, /**< 加特兰CFX 28位数据格式 */
    DOA_COMPLEX_FLOAT_DATA_TYPE,          /**< 浮点类型 */
    DOA_COMPLEX_INT16_DATA_TYPE,          /**< 16位整型复数 @ref Complex_I16_t */
    DOA_FLOAT32_DATA_TYPE,                /**< 32位浮点类型 */
    DOA_INT32_DATA_TYPE,                  /**< 整型32位 */
    DOA_INT16_DAYA_TYPE,                  /**< 整型16位 */
    DOA_INT8_DATA_TYPE,                   /**< 整型8位 */
    DOA_UINT32_DATA_TYPE,                 /**< 无符号整型32位 */
    DOA_UINT16_DAYA_TYPE,                 /**< 无符号整型16位 */
    DOA_UINT8_DATA_TYPE,                  /**< 无符号整型8位 */
    DOA_UNKNOW_DATA_TYPE = 0xFF,          /**< 未知数据类型 */
  }DOA_DATA_Typedef_t;

  /* 存储的导向矢量表类型 */
  typedef enum
  {
    DOA_SV_AZIMUTH_TABLE = 0, /**< 方位导向矢量表 */
    DOA_SV_ELEVATION_TABLE,   /**< 俯仰导向矢量表 */
    DAA_ANT_POS_TABLE,        /**< 天线间距坐标信息表 */
    DOA_PHASE_COMPS_TABLE,    /**< 天线初相信息表 */
    DOA_ANT_BOTH_TABLE,       /**< 天线间距坐标与初相信息表，双表合并 */
    DOA_UNKNOW_TABLE = 0xFF,  /**< 未知表类型 */
  }DOA_TABLE_Typedef_t;

public:
  /**
   * @brief eol协议线程
   */
  virtual void run() override
  {
    run_state = true;
    run_eol_task();
  }

  void stop()
  {
    run_state = false;
  }

  /**
   * @brief 设置协议栈数据源
   * @param cq_
   */
  void set_cq(CircularQueue *cq_ = nullptr);

  void set_can_driver_obj(can_driver *can_driver_ = nullptr)
  {
    can_driver_obj = can_driver_;
    if(nullptr == can_driver_)
    {
      return;
    }
    cq_obj = can_driver_obj->cq_obj;
  }

private:
  void run_eol_task();
private:
  bool run_state = false;
  CircularQueue *cq_obj = nullptr;
  can_driver *can_driver_obj = nullptr;
};
#endif
/******************************** End of file *********************************/
