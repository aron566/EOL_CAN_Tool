/**
 *  @file msg_log_buffer.cpp
 *
 *  @date 2026年09月27日
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief 消息日志缓冲临时文件实现,详见 msg_log_buffer.h。
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2026-09-27 <td>v1.0.0  <td>aron566 <td>初始版本
 *  <tr><td>2026-09-27 <td>v1.0.1  <td>aron566 <td>拆分为抽象接口+临时文件实现
 *  </table>
 *  @copyright Copyright (c) 2026 aron566 <aron566@163.com>.
 */
/** Includes -----------------------------------------------------------------*/
#include <QDir>
#include <QCoreApplication>
/** Private includes ---------------------------------------------------------*/
#include "msg_log_buffer.h"
/** Use C compiler -----------------------------------------------------------*/
/** Private macros -----------------------------------------------------------*/
/* 文件记录头:u32 utf8长度 + u32 通道号 + u8 方向 + 3字节保留 = 12字节 */
#define MSG_LOG_RECORD_HEAD_LEN   (12U)
/** Private typedef ----------------------------------------------------------*/
/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/** Private function prototypes ----------------------------------------------*/
/** Private user code --------------------------------------------------------*/
/** Private application code -------------------------------------------------*/

msg_log_buffer *msg_log_buffer::create_file_buffer(const QString &tag, quint8 channel)
{
  return new file_msg_log_buffer(tag, channel);
}

file_msg_log_buffer::file_msg_log_buffer(const QString &tag, quint8 channel)
  : tag_(tag), channel_(channel)
{
  /* 临时文件放在系统临时目录,文件名带进程号避免多开冲突,退出时删除 */
  file_path_ = QString("%1/EOL_CAN_Tool_%2_ch%3_%4.tmp")
               .arg(QDir::tempPath(), tag_, QString::number(channel_),
                    QString::number(QCoreApplication::applicationPid()));
  /* 残留的旧文件先删掉 */
  QFile::remove(file_path_);
  file_.setFileName(file_path_);
  file_ok_ = file_.open(QIODevice::ReadWrite);
}

file_msg_log_buffer::~file_msg_log_buffer()
{
  if(file_.isOpen())
  {
    file_.close();
  }
  /* 删除临时文件 */
  QFile::remove(file_path_);
}

void file_msg_log_buffer::append(const SHOW_MSG_Typedef_t &msg)
{
  mem_list_.append(msg);
  total_count_++;

  /* 内存超出上限:把最老的 FILE_BATCH 条批量写入临时文件 */
  if((quint32)mem_list_.size() > MEM_CACHE_MAX)
  {
    flush_batch();
  }
}

bool file_msg_log_buffer::get(quint32 index, SHOW_MSG_Typedef_t &msg)
{
  if(index >= total_count_)
  {
    return false;
  }

  /* 近期消息命中内存缓存 */
  quint64 mem_base = (quint64)total_count_ - (quint64)mem_list_.size();
  if((quint64)index >= mem_base)
  {
    msg = mem_list_.at(index - (quint32)mem_base);
    return true;
  }

  /* 更早的历史从临时文件读取 */
  if(false == file_ok_)
  {
    return false;
  }
  quint32 block = index / FILE_BATCH;
  quint32 offset_in_block = index % FILE_BATCH;
  if(block >= (quint32)block_offsets_.size())
  {
    return false;
  }
  if(false == file_.seek((qint64)block_offsets_.at(block)))
  {
    return false;
  }
  for(quint32 i = 0; i < offset_in_block; i++)
  {
    if(false == skip_record())
    {
      return false;
    }
  }
  return read_record(msg);
}

SHOW_MSG_Typedef_t file_msg_log_buffer::value(quint32 index)
{
  SHOW_MSG_Typedef_t msg;
  msg.channel_num = 0;
  msg.direct = 0;
  if(false == get(index, msg))
  {
    msg.str.clear();
  }
  return msg;
}

void file_msg_log_buffer::clear()
{
  mem_list_.clear();
  block_offsets_.clear();
  total_count_ = 0;
  if(file_ok_)
  {
    file_.resize(0);
    file_.seek(0);
  }
}

bool file_msg_log_buffer::flush_batch()
{
  /* 文件不可用时降级:直接丢弃最老的一批,保证内存不膨胀 */
  if(false == file_ok_)
  {
    while((quint32)mem_list_.size() > MEM_CACHE_MAX - FILE_BATCH)
    {
      mem_list_.removeFirst();
    }
    return false;
  }

  /* 记录本批在文件中的起始偏移 */
  if(false == file_.seek(file_.size()))
  {
    return false;
  }
  block_offsets_.append((quint64)file_.pos());

  for(quint32 i = 0; i < FILE_BATCH; i++)
  {
    if(false == write_record(mem_list_.at((int)i)))
    {
      return false;
    }
  }
  file_.flush();

  /* 从内存移除已落盘的一批 */
  for(quint32 i = 0; i < FILE_BATCH; i++)
  {
    mem_list_.removeFirst();
  }
  return true;
}

bool file_msg_log_buffer::write_record(const SHOW_MSG_Typedef_t &msg)
{
  QByteArray utf8 = msg.str.toUtf8();
  quint32 len = (quint32)utf8.size();
  quint32 ch = msg.channel_num;
  quint8 head[MSG_LOG_RECORD_HEAD_LEN];
  head[0] = (quint8)(len & 0xFFU);
  head[1] = (quint8)((len >> 8) & 0xFFU);
  head[2] = (quint8)((len >> 16) & 0xFFU);
  head[3] = (quint8)((len >> 24) & 0xFFU);
  head[4] = (quint8)(ch & 0xFFU);
  head[5] = (quint8)((ch >> 8) & 0xFFU);
  head[6] = (quint8)((ch >> 16) & 0xFFU);
  head[7] = (quint8)((ch >> 24) & 0xFFU);
  head[8] = msg.direct;
  head[9] = 0;
  head[10] = 0;
  head[11] = 0;
  if(file_.write((const char *)head, MSG_LOG_RECORD_HEAD_LEN) != (qint64)MSG_LOG_RECORD_HEAD_LEN)
  {
    return false;
  }
  if(0U < len && file_.write(utf8.constData(), (qint64)len) != (qint64)len)
  {
    return false;
  }
  return true;
}

bool file_msg_log_buffer::read_record(SHOW_MSG_Typedef_t &msg)
{
  quint8 head[MSG_LOG_RECORD_HEAD_LEN];
  if(file_.read((char *)head, MSG_LOG_RECORD_HEAD_LEN) != (qint64)MSG_LOG_RECORD_HEAD_LEN)
  {
    return false;
  }
  quint32 len = (quint32)head[0] | ((quint32)head[1] << 8) | ((quint32)head[2] << 16) | ((quint32)head[3] << 24);
  msg.channel_num = (quint32)head[4] | ((quint32)head[5] << 8) | ((quint32)head[6] << 16) | ((quint32)head[7] << 24);
  msg.direct = head[8];
  QByteArray utf8;
  utf8.resize((int)len);
  if(0U < len && file_.read(utf8.data(), (qint64)len) != (qint64)len)
  {
    return false;
  }
  msg.str = QString::fromUtf8(utf8);
  return true;
}

bool file_msg_log_buffer::skip_record()
{
  quint8 head[MSG_LOG_RECORD_HEAD_LEN];
  if(file_.read((char *)head, MSG_LOG_RECORD_HEAD_LEN) != (qint64)MSG_LOG_RECORD_HEAD_LEN)
  {
    return false;
  }
  quint32 len = (quint32)head[0] | ((quint32)head[1] << 8) | ((quint32)head[2] << 16) | ((quint32)head[3] << 24);
  if(false == file_.seek(file_.pos() + (qint64)len))
  {
    return false;
  }
  return true;
}

/******************************** End of file *********************************/
