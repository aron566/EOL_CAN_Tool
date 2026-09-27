/**
 *  @file msg_log_buffer.h
 *
 *  @date 2026年09月27日
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief 消息显示日志缓冲抽象类及临时文件实现。
 *
 *  @details
 *  原 more_window/network_window 把全部历史消息缓存在内存 QList 中(上限约52万条),
 *  每条新消息触发 removeFirst() 全量搬移,且 QString 常驻内存,大数据量时界面卡顿。
 *  这里抽象出 msg_log_buffer 接口,窗口只依赖抽象类自行使用;具体实现
 *  file_msg_log_buffer 采用两级存储:
 *   - 内存:只保留最近 MEM_CACHE_MAX(2000)条,用于实时显示与近期翻查;
 *   - 临时文件:内存超出上限时,把最老的 FILE_BATCH(1000)条一次性追加写入
 *     系统临时目录下的文件,并记录每批的文件偏移索引;
 *   - 翻查历史时按全局索引读取:近期命中内存,更早的从临时文件 seek 读取。
 *  窗口关闭/清空时删除临时文件,不污染用户目录。
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2026-09-27 <td>v1.0.0  <td>aron566 <td>初始版本
 *  <tr><td>2026-09-27 <td>v1.0.1  <td>aron566 <td>拆分为抽象接口+临时文件实现
 *  </table>
 *  @copyright Copyright (c) 2026 aron566 <aron566@163.com>.
 */
#ifndef MSG_LOG_BUFFER_H
#define MSG_LOG_BUFFER_H

/** Includes -----------------------------------------------------------------*/
#include <QFile>
#include <QList>
#include <QVector>
#include <QString>
/** Private includes ---------------------------------------------------------*/
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/* 消息框显示记录(与原 more_window/network_window 内嵌结构体一致) */
typedef struct
{
  QString str;
  quint32 channel_num;
  quint8 direct;
} SHOW_MSG_Typedef_t;
/** Exported constants -------------------------------------------------------*/
/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

/**
 * @brief 消息日志缓冲抽象类,窗口只依赖该接口
 */
class msg_log_buffer
{
public:
  virtual ~msg_log_buffer() = default;

  /**
   * @brief 追加一条消息
   * @param msg 消息
   */
  virtual void append(const SHOW_MSG_Typedef_t &msg) = 0;

  /**
   * @brief 按全局索引读取一条消息(0 为最老)
   * @param index 全局索引
   * @param msg 输出
   * @return true 成功,false 索引越界
   */
  virtual bool get(quint32 index, SHOW_MSG_Typedef_t &msg) = 0;

  /**
   * @brief 按全局索引读取,越界返回空记录
   * @param index 全局索引
   * @return 消息记录
   */
  virtual SHOW_MSG_Typedef_t value(quint32 index) = 0;

  /**
   * @brief 历史总条数(内存+文件)
   * @return 条数
   */
  virtual quint32 size() const = 0;

  /**
   * @brief 是否为空
   * @return true 空
   */
  virtual bool is_empty() const = 0;

  /**
   * @brief 清空缓冲
   */
  virtual void clear() = 0;

  /**
   * @brief 工厂:创建临时文件实现
   * @param tag 临时文件名标识(如 "more"/"net"),区分窗口
   * @param channel 通道号(1/2),区分同一窗口的两个通道
   * @return 抽象类指针,调用方负责释放
   */
  static msg_log_buffer *create_file_buffer(const QString &tag, quint8 channel);
};

/**
 * @brief 临时文件实现:内存只保留最近少量消息,历史按批写入临时文件
 */
class file_msg_log_buffer : public msg_log_buffer
{
public:
  static const quint32 FILE_BATCH = 1000U;     /**< 每次写入临时文件的条数 */
  static const quint32 MEM_CACHE_MAX = 2000U;  /**< 内存保留的最大条数 */

  /**
   * @brief 构造
   * @param tag 临时文件名标识(如 "more"/"net"),区分窗口
   * @param channel 通道号(1/2),区分同一窗口的两个通道
   */
  explicit file_msg_log_buffer(const QString &tag, quint8 channel);
  ~file_msg_log_buffer() override;

  /* 禁止拷贝(持有文件句柄) */
  file_msg_log_buffer(const file_msg_log_buffer &) = delete;
  file_msg_log_buffer &operator=(const file_msg_log_buffer &) = delete;

  void append(const SHOW_MSG_Typedef_t &msg) override;
  bool get(quint32 index, SHOW_MSG_Typedef_t &msg) override;
  SHOW_MSG_Typedef_t value(quint32 index) override;
  quint32 size() const override
  {
    return total_count_;
  }
  bool is_empty() const override
  {
    return 0U == total_count_;
  }
  void clear() override;

  /**
   * @brief 临时文件路径(调试用)
   * @return 路径
   */
  QString file_path() const
  {
    return file_path_;
  }

private:
  /**
   * @brief 把内存中最老的 FILE_BATCH 条写入临时文件
   * @return true 成功(或文件不可用时已丢弃)
   */
  bool flush_batch();

  /**
   * @brief 写一条记录到文件当前位置
   */
  bool write_record(const SHOW_MSG_Typedef_t &msg);

  /**
   * @brief 从文件当前位置读一条记录
   */
  bool read_record(SHOW_MSG_Typedef_t &msg);

  /**
   * @brief 从文件当前位置跳过一条记录
   */
  bool skip_record();

  QString tag_;
  quint8 channel_ = 0;
  QString file_path_;
  QFile file_;
  bool file_ok_ = false;

  QList<SHOW_MSG_Typedef_t> mem_list_;  /**< 内存缓存:最近的消息 */
  QVector<quint64> block_offsets_;      /**< 每批(1000条)在文件中的起始偏移 */
  quint32 total_count_ = 0;             /**< 历史总条数 */
};

#endif // MSG_LOG_BUFFER_H
/******************************** End of file *********************************/
