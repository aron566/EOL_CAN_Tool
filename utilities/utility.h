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
#include <QTableWidget>

/** Private defines ----------------------------------------------------------*/
/* 包重复检测 */
#define EVEN_PACKAGE_REPEAT_CHECK_SIZE 10U    /**< 10包偶数循环检测 */
#define ODD_PACKAGE_REPEAT_CHECK_SIZE  (EVEN_PACKAGE_REPEAT_CHECK_SIZE / 2U)

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
    COMPLEX_INT16_DATA_TYPE,          /**< 16位整型复数 */
    FLOAT32_DATA_TYPE,                /**< 32位浮点类型 */
    INT32_DATA_TYPE,                  /**< 整型32位 */
    INT16_DAYA_TYPE,                  /**< 整型16位 */
    INT8_DATA_TYPE,                   /**< 整型8位 */
    UINT32_DATA_TYPE,                 /**< 无符号整型32位 */
    UINT16_DAYA_TYPE,                 /**< 无符号整型16位 */
    UINT8_DATA_TYPE,                  /**< 无符号整型8位 */
    FLOAT32_BIN_DATA_TYPE,            /**< 32位浮点数二进制类型 */
    CALTERAH_CFL_32BIT_DATA_TYPE,     /**< 加特兰CFL 32位数据格式 */
    COMPLEX_INT32_DATA_TYPE,          /**< 32位整型复数 */
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
  /* 复数 */
  typedef struct
  {
    qint32 real;
    qint32 image;
  }Complex_I32_t;

  /* 键值映射 */
  typedef struct
  {
    int key_val;                      /**< 按键键值 */
    char key_ascii;                   /**< ascii键值 */
  }KEY_MAP_Typedef_t;

  /* 包重复检测 */
  typedef struct
  {
    uint8_t State_Record_Even[EVEN_PACKAGE_REPEAT_CHECK_SIZE];
    uint8_t State_Record_Odd[ODD_PACKAGE_REPEAT_CHECK_SIZE];
  }PACK_REPEAT_CHECK_Typedef_t;

  /**
   * @brief 延时ms数，保持事件循环
   * @param ms
   */
  static void delay_ms(int ms);

  /**
   * @brief 字节序翻转
   * @param array
   * @param len
   */
  static void bytes_invert(quint8 *array, quint32 len);

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
   * @brief 数据转为转换后的字符串
   * @param data 数据
   * @param Type 数据类型
   * @return 字符串
   */
  static QString data2str(const quint8 *data, NUM_TYPE_Typedef_t Type = INT32_DATA_TYPE);

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
  static quint32 get_crc32_with_tab1(const quint8 *data_blk_ptr, quint32 data_blk_size, quint32 crc_accum = 0);
  static quint32 get_crc32_with_tab2(const quint8 *data_blk_ptr, quint32 data_blk_size, quint32 crc_accum = 0);
  static quint32 get_crc32_with_tab2_for_upfw(const quint8 *data_blk_ptr, quint32 data_blk_size, quint32 crc_accum = 0xFFFFFFFFU, bool end = false);

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
   * @brief line_data2split 发送数据转换为标准空格16进制数据
   * @param line_data 支持以"0xaa , ac  56"解析
   * @return "aa ac 56"
   */
  static QString line_data2split(const QString &line_data);

  /**
   * @brief array2hexstr 数组转字符串
   * @param data 数据
   * @param len 数据长度
   * @param split 分割
   * @return 字符串
   */
  static QString array2hexstr(const quint8 *data, quint32 len, const QString &split);

  /**
   * @brief 包号重复检测
   *
   * @param pRecord 记录句柄
   * @param Pack_Num 包号
   * @param Init_En 初始化使能，初始化使能时，仅进行初始化，
   * @param Check_En 检测使能，检测使能时检测，false时记录
   * @return 返回true代表重复包
   */
  static bool Check_Current_Pack_Num_Is_Repeat(PACK_REPEAT_CHECK_Typedef_t *pRecord, uint32_t Pack_Num, \
                                               bool Init_En, bool Check_En);

  /**
   * @brief 导出表空间数据到csv文件
   * @param tableWidget 表控件
   * @param filePath 文件名
   */
  static void export_table2csv_file(QTableWidget *tableWidget, QString filePath);

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
