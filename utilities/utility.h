/**
 *  @file utility.hpp
 *
 *  @date 2023年02月21日 17:59:10 星期二
 *
 *  @author Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @version v1.0.0 aron566 2023.02.21 17:59 初始版本.
 */
#ifndef UTILITY_H
#define UTILITY_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
// #include <limits.h> /**< need variable max value    */
// #include <stdalign.h> /**< need alignof    */
// #include <stdarg.h> /**< need va_start    */
/** Private includes ---------------------------------------------------------*/
#include <QObject>
#include <QEventLoop>
#include <QTimer>
#include <QStringList>
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/**
 * @brief 求结构体成员偏移字节
 * @param struct_type 结构体类型
 * @param member 成员名
 */
#define __HAL_OFFSETOF(struct_type, member) ((size_t)(&(((struct_type*)0)->member)))
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class utility : public QObject
{
  Q_OBJECT
public:
  explicit utility(QObject *parent = nullptr);

signals:

public:
  typedef enum
  {
    CALTERAH_CFX_28BIT_DATA_TYPE = 0, /**< 加特兰CFX 28位数据格式 */
    COMPLEX_FLOAT_DATA_TYPE,          /**< 浮点类型 */
    COMPLEX_INT16_DATA_TYPE,          /**< 16位整型复 */
    FLOAT32_DATA_TYPE,                /**< 32位浮点类型 */
    INT32_DATA_TYPE,                  /**< 整型32位 */
    INT16_DAYA_TYPE,                  /**< 整型16位 */
    INT8_DATA_TYPE,                   /**< 整型8位 */
    UINT32_DATA_TYPE,                 /**< 无符号整型32位 */
    UINT16_DAYA_TYPE,                 /**< 无符号整型16位 */
    UINT8_DATA_TYPE,                  /**< 无符号整型8位 */
    FLOAT32_BIN_DATA_TYPE,            /**< 32位浮点数二进制类型 */
    UNKNOW_DATA_TYPE = 0xFF,          /**< 未知数据类型 */
  }NUM_TYPE_Typedef_t;


  /* 复数 */
  typedef struct
  {
    float real;
    float image;
  }Complex_t;

  /* 复数 */
  typedef struct
  {
    qint16 real;
    qint16 image;
  }Complex_I16_t;

  /* 键值映射 */
  typedef struct
  {
    int key_val;                      /**< 按键键值 */
    char key_ascii;                   /**< ascii键值 */
  }KEY_MAP_Typedef_t;

  /**
   * @brief 延时ms数，保持事件循环
   * @param ms
   */
  static void delay_ms(int ms);

  /**
   * @brief 数据类型转字节数
   * @param Type 类型
   * @return 字节数
   */
  static quint32 num_type_to_bytes(utility::NUM_TYPE_Typedef_t Type);

  /**
   * @brief 字符串转特定类型数值
   * @param buf 转换后存储区
   * @param num_str_list 字符串列表
   * @param unit_bytes 转换后的类型占字节数，存储区
   * @param 转换相应类型数据的数量，两个浮点字符串转为复数浮点，则返回1
   */
  static quint32 str2num(void *buf, const QStringList &num_str_list, NUM_TYPE_Typedef_t Type = INT32_DATA_TYPE, quint8 *unit_bytes = nullptr);

  /**
   * @brief 验证数据crc是否正确
   * @param data 数据及crc（小端低字节在前）
   * @param data_len 数据区域长度
   * @return true正确
   */
  static bool get_modbus_crc16_rsl_with_tab(const uint8_t *data, uint16_t data_len);

  /**
   * @brief 计算数据的crc
   * @param data 数据
   * @param data_len 数据长度
   * @return crc结果
   */
  static uint16_t get_modbus_crc16_with_tab(const uint8_t *data, uint16_t data_len);

  /**
   * @brief 计算数据的crc
   * @param data 数据
   * @param data_len 数据长度
   * @return crc结果
   */
  static quint32 get_crc32_with_tab(const quint8 *data_blk_ptr, quint32 data_blk_size, quint32 crc_accum = 0);

  /**
   * @brief 验证数据crc是否正确
   * @param data 数据及crc（小端低字节在前）
   * @param data_len 数据区域长度
   * @return true正确
   */
  static bool get_crc32_rsl_with_tab(const quint8 *data_blk_ptr, quint32 data_blk_size, quint32 crc_accum = 0);

  /**
   * @brief 验证和校验是否正确
   * @param data 数据
   * @param len 累积数据长度
   * @return true正确
   */
  static bool get_sum_rsl(const quint8 *data, quint32 len);

  /**
   * @brief 获取数据和
   * @param data 数据
   * @param len 累积数据长度
   * @return sum
   */
  static quint8 get_data_sum(const quint8 *data, quint32 len);

  /**
   * @brief 16进制格式调试打印
   *
   * @param msg 数据
   * @param msg_len 数据长度
   * @param prefix_str 前缀
   */
  static void debug_print(const uint8_t *msg, uint32_t msg_len, QString prefix_str = "");

  /**
   * @brief unicode_to_gb2312
   * @param unicode_str
   * @return 转换后的字符串
   */
  static QString unicode_to_gb2312(const QString &unicode_str);

  /**
   * @brief gb2312_to_unicode
   * @param gb2312_str
   * @return 转换后的字符串
   */
  static QString gb2312_to_unicode(const QString &gb2312_str);
};

#endif // UTILITY_H
/******************************** End of file *********************************/
