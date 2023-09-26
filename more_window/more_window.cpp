#include "more_window.h"
#include "ui_more_window.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <QWheelEvent>

#define SHOW_MSG_SAVE_NUM_MAX     200U                    /**< 最大显示消息数 */
#define SHOW_MSG_ONE_SCORLL       (5U)                    /**< 上翻每次刷新列表数 */
#define SAVE_MSG_BUF_MAX          (1024U*1024U*1U)        /**< 最大缓存消息数 */

#define SHOW_LINE_CHAR_NUM_MAX    (1024U)                 /**< 一行最大显示多少字符 */
#define SHOW_CHAR_TIMEOUT_MS_MAX  (1000U)                 /**< 最大等待无换行符时间ms */

#define CONFIG_VER_STR            "0.0.2"                 /**< 配置文件版本 */

more_window::more_window(QString title, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::more_window)
{
  ui->setupUi(this);

  /* Apply style sheet */
  QFile file(":/qdarkstyle/dark/style.qss");
  if(file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    this->setStyleSheet(file.readAll());
    file.close();
  }

  /* 设置标题 */
  this->setWindowTitle(title);

  /* EOL窗口初始化 */
  eol_window_init(tr("EOL CAN Tool - EOL"));

  /* 工具窗口初始化 */
  tool_window_init(tr("EOL CAN Tool - TOOL"));

  /* 帧诊断调试窗口初始化 */
  frame_diagnosis_window_init(tr("EOL CAN Tool - Frame Diagnosis"));

  /* 初始化定时器 */
  timer_init();

  ui->ch1_receive_data_textEdit->setVisible(true);
  ui->ch2_receive_data_textEdit->setVisible(true);
  ui->ch1_receive_data_textEdit->setUndoRedoEnabled(false);
  ui->ch2_receive_data_textEdit->setUndoRedoEnabled(false);

  /* 限制显示行数 */
  ui->ch1_receive_data_textEdit->setReadOnly(true);
  ui->ch2_receive_data_textEdit->setReadOnly(true);
  ui->ch1_receive_data_textEdit->document()->setMaximumBlockCount(SHOW_MSG_SAVE_NUM_MAX);
  ui->ch2_receive_data_textEdit->document()->setMaximumBlockCount(SHOW_MSG_SAVE_NUM_MAX);

  /* 设置高亮器 */
  ch1_line_highlighter.setDocument(ui->ch1_receive_data_textEdit->document());
  ch2_line_highlighter.setDocument(ui->ch2_receive_data_textEdit->document());
  ch1_line_highlighter.set_text_color("Tx", Qt::red);
  ch2_line_highlighter.set_text_color("Tx", Qt::red);
  ch1_line_highlighter.set_text_color("send", Qt::red);
  ch2_line_highlighter.set_text_color("send", Qt::red);

  /* 设置提示值 */
  ui->data_lineEdit->setPlaceholderText("05 66");
  ui->id_lineEdit->setPlaceholderText("157");
  ui->display_str_id_limit_lineEdit->setPlaceholderText("FFFF");

  /* 设置悬浮提示 */
  ui->display_mask_lineEdit->setToolTip(tr("show msg can id = mask & can id"));
  ui->display_str_id_limit_lineEdit->setToolTip(tr("FFFF show all can str msg"));
}

more_window::~more_window()
{
  /* 保存参数 */
  save_cfg();

  delete frame_diagnosis_obj;
  qDebug() << "del frame_diagnosis_obj";

  delete eol_window_obj;
  qDebug() << "del eol_window_obj";

  delete ui;
  qDebug() << "del more_window";
}

/* EOL调试子窗口 */
void more_window::eol_window_init(QString titile)
{
  eol_window_obj = new eol_window(titile);

  /* 设置线程池 first */
  eol_window_obj->set_thread_pool(QThreadPool::globalInstance());

  /* 禁止线程完成后执行析构对象 */
  eol_window_obj->setAutoDelete(false);
}

/* 工具集子窗口 */
void more_window::tool_window_init(QString titile)
{
  tool_window_obj = new tool_window(titile);
}

void more_window::timer_init()
{
  timer_obj = new QTimer(this);
  timer_obj->setInterval(1);
  connect(timer_obj, &QTimer::timeout, this, &more_window::slot_timeout);
  timer_obj->start();
}

void more_window::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
}

void more_window::wheelEvent(QWheelEvent *event)
{
#if 1
//  qDebug() << "wheel event angleDelta" << event->angleDelta().y();
//  qDebug() << "wheel event phase" << event->phase();
//  qDebug() << "wheel event inverted" << event->inverted();
//  qDebug() << "wheel event pixelDelta" << event->pixelDelta().y();
//  qDebug() << "wheel event globalPosition " << event->globalPosition().x() << event->globalPosition().y();

  int xp1, yp1, xp2, yp2;
  quint32 *pchx_scroll_cnt = nullptr;
  QList<SHOW_MSG_Typedef_t> *pList = nullptr;
  QPlainTextEdit *text_edit_widget = nullptr;
  QPlainTextEdit *text_edit_widget_temp = ui->ch1_receive_data_textEdit;
  QPoint A = QWidget::mapToGlobal(text_edit_widget_temp->pos());
  xp1 = A.x();
  yp1 = A.y();
  xp2 = text_edit_widget_temp->rect().width() + xp1;
  yp2 = text_edit_widget_temp->rect().height() + yp1;

  if(event->globalPosition().x() >= (qreal)xp1 && \
     event->globalPosition().x() <= (qreal)xp2 && \
     event->globalPosition().y() >= (qreal)yp1 && \
     event->globalPosition().y() <= (qreal)yp2)
  {
//    qDebug() << "ch1";
    text_edit_widget = ui->ch1_receive_data_textEdit;
    pchx_scroll_cnt = &ch1_scroll_cnt;
    pList = &ch1_show_msg_list;
  }

  text_edit_widget_temp = ui->ch2_receive_data_textEdit;
  A = QWidget::mapToGlobal(text_edit_widget_temp->pos());
  xp1 = A.x();
  yp1 = A.y();
  xp2 = text_edit_widget_temp->rect().width() + xp1;
  yp2 = text_edit_widget_temp->rect().height() + yp1;

  if(event->globalPosition().x() >= (qreal)xp1 && \
     event->globalPosition().x() <= (qreal)xp2 && \
     event->globalPosition().y() >= (qreal)yp1 && \
     event->globalPosition().y() <= (qreal)yp2)
  {
//    qDebug() << "ch2";
    text_edit_widget = ui->ch2_receive_data_textEdit;
    pchx_scroll_cnt = &ch2_scroll_cnt;
    pList = &ch2_show_msg_list;
  }

  if(nullptr == text_edit_widget)
  {
    return;
  }

  for(quint32 i = 0; i < SHOW_MSG_ONE_SCORLL; i++)
  {
    /* 向下滚动 */
    if(event->angleDelta().y() < 0)
    {
      /* 是否能下翻 */
      if((*pchx_scroll_cnt) + 1 > (quint32)pList->size())
      {
//        qDebug() << " down " << (*pchx_scroll_cnt) << pList->size();
        return;
      }
      *pchx_scroll_cnt = (*pchx_scroll_cnt) + 1;
      quint32 bottom_index = (*pchx_scroll_cnt) - 1;

      /* 刷新当前消息到尾部 */
      update_show_msg(text_edit_widget, pList, bottom_index, true);
    }

    /* 向上滚动 */
    if(event->angleDelta().y() > 0)
    {
      quint32 top_index = 0;
      /* 找到顶部索引 */
      if((*pchx_scroll_cnt) >= SHOW_MSG_SAVE_NUM_MAX)
      {
        top_index = (*pchx_scroll_cnt) - SHOW_MSG_SAVE_NUM_MAX;
      }
      else
      {
//        qDebug() << " up " << (*pchx_scroll_cnt) << SHOW_MSG_SAVE_NUM_MAX;
        return;
      }

      /* 是否能上翻 */
      if(top_index > 0)
      {
        top_index--;
        *pchx_scroll_cnt = (*pchx_scroll_cnt) - 1;
      }
      else
      {
//        qDebug() << " up top_index" << top_index;
        return;
      }

      /* 刷新当前消息到尾部 */
      update_show_msg(text_edit_widget, pList, top_index, false);
    }
  }

#endif
  QWidget::wheelEvent(event);
}

void more_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_more_window_closed();
}

void more_window::save_cfg()
{
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  /* can id */
  setting.setValue("more_window_v" CONFIG_VER_STR "/can_id", ui->id_lineEdit->text());
  /* 数据 */
  QString plaintext = ui->data_lineEdit->text();
  plaintext.replace(' ', ',');;
  setting.setValue("more_window_v" CONFIG_VER_STR "/data_edit", plaintext);
  setting.sync();
}

void more_window::read_cfg()
{
  QFile file("./eol_tool_cfg.ini");
  if(false == file.exists())
  {
    return;
  }
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  if(false == setting.contains("more_window_v" CONFIG_VER_STR "/can_id"))
  {
    qDebug() << "err more_window config not exist";
    return;
  }
  /* can id */
  ui->id_lineEdit->setText(setting.value("more_window_v" CONFIG_VER_STR "/can_id").toString());
  /* 数据 */
  QString plaintext = setting.value("more_window_v" CONFIG_VER_STR "/data_edit").toString();
  QString data_str = plaintext.replace(',', ' ');
  ui->data_lineEdit->setText(data_str);
  setting.sync();
}

bool more_window::ch1_show_msg_is_empty()
{
  if(ch1_add_msg_index == ch1_show_msg_index)
  {
    return true;
  }
  return false;
}

bool more_window::ch2_show_msg_is_empty()
{
  if(ch2_add_msg_index == ch2_show_msg_index)
  {
    return true;
  }
  return false;
}

quint32 more_window::get_show_index(quint32 current_show_index, quint32 totaol_size)
{
  quint32 index = 0;
  quint32 remain = totaol_size - current_show_index;
  if(SAVE_MSG_BUF_MAX <= current_show_index)
  {
    index = SAVE_MSG_BUF_MAX - remain;
    return index;
  }
  index = current_show_index % SAVE_MSG_BUF_MAX;
  return index;
}

void more_window::show_txt()
{
  /* 显示 */
  QPlainTextEdit *text_edit_widget = nullptr;
  SHOW_MSG_Typedef_t show_messagex;

  if(ch1_show_msg_is_empty() == false)
  {
    quint32 show_index = get_show_index(ch1_show_msg_index, ch1_add_msg_index);
    show_messagex = ch1_show_msg_list.value(show_index);
    ch1_show_msg_index++;
    ch1_scroll_cnt = show_index + 1;

    text_edit_widget = ui->ch1_receive_data_textEdit;
    text_edit_widget->appendPlainText(show_messagex.str);
  }

  if(ch2_show_msg_is_empty() == false)
  {
    quint32 show_index = get_show_index(ch2_show_msg_index, ch2_add_msg_index);
    show_messagex = ch2_show_msg_list.value(show_index);
    ch2_show_msg_index++;
    ch2_scroll_cnt = show_index + 1;

    text_edit_widget = ui->ch2_receive_data_textEdit;
    text_edit_widget->appendPlainText(show_messagex.str);
  }
}

void more_window::update_show_msg(QPlainTextEdit *text_edit_widget, QList<SHOW_MSG_Typedef_t> *pList, quint32 show_index, bool downward_flag)
{
  SHOW_MSG_Typedef_t show_messagex = pList->value(show_index);

  /* 下翻 */
  if(downward_flag)
  {
    text_edit_widget->moveCursor(QTextCursor::End);
    QTextCursor cursor = text_edit_widget->textCursor();
    cursor.insertText(show_messagex.str + '\n');
    text_edit_widget->setTextCursor(cursor);

    text_edit_widget->moveCursor(QTextCursor::Start);
    text_edit_widget->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    text_edit_widget->moveCursor(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
//    qDebug() << "sel1 :" << text_edit_widget->textCursor().selectedText();
    text_edit_widget->textCursor().removeSelectedText();
    text_edit_widget->moveCursor(QTextCursor::End);
  }
  /* 上翻 */
  else
  {
    text_edit_widget->moveCursor(QTextCursor::Start);
    QTextCursor cursor = text_edit_widget->textCursor();
    cursor.insertText(show_messagex.str + '\n');
    text_edit_widget->setTextCursor(cursor);

    /* 删除尾部行数据 */
    text_edit_widget->moveCursor(QTextCursor::End);
    text_edit_widget->moveCursor(QTextCursor::PreviousBlock, QTextCursor::KeepAnchor);
    text_edit_widget->moveCursor(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    //    qDebug() << "sel2 :" << text_edit_widget->textCursor().selectedText();
    text_edit_widget->textCursor().removeSelectedText();

    /* 回到头部显示 */
    text_edit_widget->moveCursor(QTextCursor::Start);
  }
}

void more_window::frame_diagnosis_window_init(QString title)
{
  frame_diagnosis_obj = new frame_diagnosis(title);

  connect(frame_diagnosis_obj, &frame_diagnosis::signal_window_closed, this, &more_window::slot_show_window);
}

void more_window::slot_show_window()
{
  disconnect(can_driver_obj, &can_driver::signal_can_driver_msg, this, &more_window::slot_can_driver_msg);
  CircularQueue::CQ_handleTypeDef *cq = can_driver_obj->cq_obj->CQ_getCQHandle();
  /* 清空 */
  CircularQueue::CQ_emptyData(cq);
  connect(can_driver_obj, &can_driver::signal_show_can_msg, this, &more_window::slot_show_can_msg, Qt::QueuedConnection);
  connect(can_driver_obj, &can_driver::signal_show_can_msg_asynchronous, this, &more_window::slot_show_can_msg);
  this->show();
}

void more_window::slot_can_driver_msg(quint16 can_id, const quint8 *data, quint32 len, \
  quint8 direct, quint32 channel_num, quint8 protocol_type, quint64 ms)
{
  frame_diagnosis_obj->add_msg_to_table(can_id, data, len, direct, channel_num, protocol_type, ms);
}

bool more_window::char2str(const quint8 *data, quint32 data_len, QString &msg)
{
  if(0U == data_len)
  {
    return false;
  }
  char str_buf[65] = {0};
  size_t size = data_len > 64U ? 64U : data_len;

  /* 剔除无效字符 */
  for(size_t i = 0; i < size; i++)
  {
    if(data[i] == '\0')
    {
      size = i;
      break;
    }
  }

  /* 查看字符的尾部是否是换行符 */
  if(data[size - 1U] == '\r' || data[size - 1U] == '\n')
  {
    memcpy(str_buf, data, size);
    /* 去除尾部换行 */
    if(str_buf[size - 1U] == '\r' || str_buf[size - 1U] == '\n')
    {
      str_buf[size - 1U] = '\0';
    }
    if(str_buf[size - 2U] == '\r' || str_buf[size - 2U] == '\n')
    {
      str_buf[size - 2U] = '\0';
    }
    QString str = QString::asprintf("%s", str_buf);
    if(show_line_str.isEmpty() == false)
    {
      show_line_str.append(str);
      msg.append(show_line_str);
//      qDebug() << "尾部有换行拼接" << a << ":" << show_line_str;
      show_line_str.clear();
    }
    else
    {
//      qDebug() << "尾部有换行" << a << ":" << str;
      msg.append(str);
    }
    return true;
  }
  /* 找到最后一个\r\n的index */
  else
  {
    size_t index = 0U;
    for(size_t i = 0; i < size; i++)
    {
      if('\r' == data[i] || '\n' == data[i])
      {
        index = i;
      }
    }
    /* 找不到，无换行符 */
    if(0U == index)
    {
      memcpy(str_buf, data, size);
      QString str = QString::asprintf("%s", str_buf);
      show_line_str.append(str);
      last_show_line_str_time_ms = current_show_line_str_time_ms;
      if(true == show_line_str_force)
      {
        show_line_str_force = false;
        msg.append(show_line_str);
//        qDebug() << "无换行符强制" << a << ":" << show_line_str;
        show_line_str.clear();
        return true;
      }
      return false;
    }
    /* 找到，中间有换行符 */
    else
    {
      /* 输出时换行符 */
      memcpy(str_buf, data, index);
      if(str_buf[index - 1U] == '\r' || str_buf[index - 1U] == '\n')
      {
        str_buf[index - 1U] = '\0';
      }
      QString str = QString::asprintf("%s", str_buf);
      if(show_line_str.isEmpty() == false)
      {
        show_line_str.append(str);
        msg.append(show_line_str);
//        qDebug() << "中间有换行符拼接" << a << ":" << show_line_str;
        show_line_str.clear();
      }
      else
      {
//        qDebug() << "中间有换行符" << a << ":" << str;
        msg.append(str);
      }
      /* 剩余部分加到下一行显示 */
      memset(str_buf, 0, sizeof(str_buf));
      memcpy(str_buf, &data[index + 1U], data_len - index - 1U);
      str = QString::asprintf("%s", str_buf);
//      qDebug() << "中间有换行符，剩余部分" << a << ":" << str;
      show_line_str.append(str);
      last_show_line_str_time_ms = current_show_line_str_time_ms;
    }
  }
  return true;
}

void more_window::slot_show_message(const QString &message, quint32 channel_num, \
  quint8 direct, const quint8 *data, quint32 data_len, quint32 can_id)
{
  QString show_message;

  /* 时间戳 */
  if(ui->display_time_stamp_checkBox->isChecked())
  {
    QDateTime dt = QDateTime::currentDateTime();
    show_message += dt.toString("hh:mm:ss.zzz");
  }

  /* 显示字符 */
  if(ui->display_str_checkBox->isChecked())
  {
    if(((quint32)ui->display_ch_comboBox->currentIndex() == channel_num || ui->display_ch_comboBox->currentText() == "ALL")
        && can_driver::CAN_RX_DIRECT == direct
        && 0U < data_len)
    {
      /* canid限制 */
      if(limit_str_canid != 0xFFFFU
          && limit_str_canid != can_id)
      {
        return;
      }
      if(false == char2str(data, data_len, show_message))
      {
        return;
      }
    }
    else
    {
      show_message.append(message);
    }
  }
  else
  {
    show_message.append(message);
  }

  SHOW_MSG_Typedef_t msg;
  msg.channel_num = channel_num;
  msg.str = show_message;
  msg.direct = direct;

  /* 限制缓冲区大小 */
  if(0 == channel_num)
  {
    if((quint32)ch1_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
    {
      ch1_show_msg_list.removeFirst();
    }
    ch1_show_msg_list.append(msg);
    ch1_add_msg_index++;
    goto __show_msg;
  }

  if((quint32)ch2_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
  {
    ch2_show_msg_list.removeFirst();
  }
  ch2_show_msg_list.append(msg);
  ch2_add_msg_index++;

__show_msg:

  /* 显示 */
  show_txt();
}


void more_window::slot_show_message_block(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data, quint32 data_len, quint32 can_id)
{
  /* 线程刷新显示 */
  QString show_message;

  /* 时间戳 */
  if(ui->display_time_stamp_checkBox->isChecked())
  {
    QDateTime dt = QDateTime::currentDateTime();
    show_message += dt.toString("hh:mm:ss.zzz");
  }

  /* 显示字符 */
  if(ui->display_str_checkBox->isChecked())
  {
    if(((quint32)ui->display_ch_comboBox->currentIndex() == channel_num || ui->display_ch_comboBox->currentText() == "ALL")
        && can_driver::CAN_RX_DIRECT == direct
        && 0U < data_len)
    {
      /* canid限制 */
      if(limit_str_canid != 0xFFFFU
          && limit_str_canid != can_id)
      {
        return;
      }
      if(false == char2str(data, data_len, show_message))
      {
        return;
      }
    }
    else
    {
      show_message.append(message);
    }
  }
  else
  {
    show_message.append(message);
  }

  SHOW_MSG_Typedef_t msg;
  msg.channel_num = channel_num;
  msg.str = show_message;
  msg.direct = direct;

  /* 限制缓冲区大小 */
  if(0 == channel_num)
  {
    if((quint32)ch1_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
    {
      ch1_show_msg_list.removeFirst();
    }
    ch1_show_msg_list.append(msg);
    ch1_add_msg_index++;

    goto __show_msg;
  }

  if((quint32)ch2_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
  {
    ch2_show_msg_list.removeFirst();
  }
  ch2_show_msg_list.append(msg);
  ch2_add_msg_index++;

__show_msg:

  /* 显示 */
  show_txt();
}

void more_window::show_can_msg(can_driver::CAN_MSG_DISPLAY_Typedef_t &msg)
{
  QString item;
  if(msg.direction == can_driver::CAN_TX_DIRECT || msg.direction == can_driver::CAN_RX_DIRECT)
  {
      item = QString::asprintf(tr("[%u]%sx CAN%s ID:%08X %s %s LEN:%d DATA:").toUtf8().data(), \
      msg.channel_num, \
      msg.direction == can_driver::CAN_TX_DIRECT ? "T" : "R",\
      msg.can_protocol == can_driver::CANFD_PROTOCOL_TYPE ? "FD" : "", \
      msg.can_id, msg.frame_type == can_driver::EXT_FRAME_TYPE ? "EXT_FRAME" : "STD_FRAME", \
      msg.frame_data_type == can_driver::REMOTE_FRAME_TYPE ? "REMOTE_FRAME" : "DATA_FRAME", \
      msg.data_len);
    for(quint32 i = 0; i < msg.data_len; ++i)
    {
      item += QString::asprintf("%02X ", msg.msg_data[i]);
    }
    slot_show_message(item, (quint32)msg.channel_num, (quint8)msg.direction, msg.msg_data, msg.data_len, msg.can_id);
    slot_show_message_bytes(msg.data_len, msg.channel_num, (quint8)msg.direction);
  }
  else if(msg.direction == can_driver::UNKNOW_DIRECT)
  {
    item = QString::asprintf("%s", msg.msg_data);
    slot_show_message(item, (quint32)msg.channel_num, (quint8)can_driver::CAN_TX_DIRECT);
  }
}

void more_window::slot_show_can_msg()
{
  CircularQueue::CQ_handleTypeDef *cq = can_driver_obj->cq_obj->CQ_getCQHandle();

  /* 判断可读长度 */
  quint32 len = CircularQueue::CQ_getLength(cq);
  if(len < sizeof(can_driver::CAN_MSG_DISPLAY_Typedef_t))
  {
    return;
  }
  quint32 msg_len = len / sizeof(can_driver::CAN_MSG_DISPLAY_Typedef_t);
//  qDebug() << "msg" << msg_len;
  msg_len = msg_len > 64U ? 64U : msg_len;
  can_driver::CAN_MSG_DISPLAY_Typedef_t msg;
  for(quint32 i = 0; i < msg_len; i++)
  {
    CircularQueue::CQ_getData(cq, (quint8 *)&msg, sizeof(can_driver::CAN_MSG_DISPLAY_Typedef_t));
    show_can_msg(msg);
  }
}

void more_window::on_frame_type_comboBox_currentIndexChanged(int index)
{
  /* 设置发送帧类型 */
  can_driver_obj->set_frame_type((quint32)index);
}


void more_window::on_protocol_comboBox_currentIndexChanged(int index)
{
  /* 设置发送协议类型 */
  can_driver_obj->set_protocol_type((quint32)index);
}


void more_window::on_canfd_pluse_comboBox_currentIndexChanged(int index)
{
  /* 设置canfd加速 */
  can_driver_obj->set_can_fd_exp((quint32)index);
}


void more_window::on_id_lineEdit_textChanged(const QString &arg1)
{
  /* 设置发送的canid */
  can_driver_obj->set_message_id(arg1);
}


void more_window::on_data_lineEdit_textChanged(const QString &arg1)
{
  /* 设置消息数据 */
  can_driver_obj->set_message_data(arg1);
}


void more_window::on_eol_test_pushButton_2_clicked()
{
  eol_window_obj->show();
}


void more_window::on_clear_pushButton_clicked()
{
  rx_frame_cnt = 0;
  ui->rx_frame_num_label->setNum((int)rx_frame_cnt);
  rx_byte_cnt = 0;
  ui->rx_byte_num_label->setNum((int)rx_byte_cnt);

  tx_frame_cnt = 0;
  ui->tx_frame_num_label->setNum((int)tx_frame_cnt);
  tx_byte_cnt = 0;
  ui->tx_byte_num_label->setNum((int)tx_byte_cnt);

  ui->ch1_receive_data_textEdit->clear();
  ui->ch2_receive_data_textEdit->clear();

  ch1_show_msg_list.clear();
  ch2_show_msg_list.clear();

  ch1_add_msg_index = ch1_show_msg_index = 0;
  ch2_add_msg_index = ch2_show_msg_index = 0;

  frame_diagnosis_obj->clear();
}

void more_window::set_channel_num(quint8 channel_num)
{
  /* 根据所选设备类型，更新通道数量 */
  ui->channel_num_comboBox->clear();
  ui->display_ch_comboBox->clear();
  for(quint8 i = 0; i < channel_num; i++)
  {
    ui->channel_num_comboBox->addItem(QString("%1").arg(i));
    ui->display_ch_comboBox->addItem(QString("%1").arg(i));
  }
  /* 打开全部通道 */
  ui->channel_num_comboBox->addItem("ALL");
  ui->display_ch_comboBox->addItem("ALL");
}

void more_window::on_send_pushButton_clicked()
{
  can_driver_obj->send(ui->channel_num_comboBox->currentIndex());
}


void more_window::on_period_lineEdit_textChanged(const QString &arg1)
{
  timer_obj->setInterval(arg1.toUInt() == 0 ? 1 : arg1.toUInt());
}

void more_window::slot_timeout()
{
  /* 定时发送 */
  if(ui->timer_checkBox->isChecked())
  {
    can_driver_obj->send(ui->channel_num_comboBox->currentIndex());
  }

  current_show_line_str_time_ms++;
  /* 检查没有换行符的字符显示 */
  if((current_show_line_str_time_ms - last_show_line_str_time_ms) > SHOW_CHAR_TIMEOUT_MS_MAX || (quint32)show_line_str.size() > SHOW_LINE_CHAR_NUM_MAX)
  {
    show_line_str_force = true;
    last_show_line_str_time_ms = current_show_line_str_time_ms;
  }
}

void more_window::slot_show_message_bytes(quint8 bytes, quint32 channel_num, quint8 direct)
{
  Q_UNUSED(channel_num)
  if(can_driver::CAN_RX_DIRECT == direct)
  {
    rx_frame_cnt++;
    ui->rx_frame_num_label->setNum((int)rx_frame_cnt);
    rx_byte_cnt += bytes;
    ui->rx_byte_num_label->setNum((int)rx_byte_cnt);
  }

  if(can_driver::CAN_TX_DIRECT == direct)
  {
    tx_frame_cnt++;
    ui->tx_frame_num_label->setNum((int)tx_frame_cnt);
    tx_byte_cnt += bytes;
    ui->tx_byte_num_label->setNum((int)tx_byte_cnt);
  }

}

void more_window::on_display_mask_lineEdit_textChanged(const QString &arg1)
{
  bool ok;
  quint32 canid_mask = arg1.toUInt(&ok, 16);
  if(ok)
  {
    can_driver_obj->set_msg_canid_mask(canid_mask, ui->mask_en_checkBox->isChecked());
  }
  else
  {
    can_driver_obj->set_msg_canid_mask(0xFFFF, ui->mask_en_checkBox->isChecked());
  }
}


void more_window::on_mask_en_checkBox_clicked(bool checked)
{
  bool ok;
  quint32 canid_mask = ui->display_mask_lineEdit->text().toUInt(&ok, 16);
  if(ok)
  {
    can_driver_obj->set_msg_canid_mask(canid_mask, checked);
  }
  else
  {
    can_driver_obj->set_msg_canid_mask(0xFFFF, checked);
  }
}

void more_window::on_frame_diagnosis_pushButton_clicked()
{
  disconnect(can_driver_obj, &can_driver::signal_show_can_msg, this, &more_window::slot_show_can_msg);
  disconnect(can_driver_obj, &can_driver::signal_show_can_msg_asynchronous, this, &more_window::slot_show_can_msg);
  connect(can_driver_obj, &can_driver::signal_can_driver_msg, this, &more_window::slot_can_driver_msg, Qt::QueuedConnection);
  frame_diagnosis_obj->show();
}


void more_window::on_export_txt_pushButton_clicked()
{
  /* 导出 */

  /* 设置导出路径 */
  /* 选择文件存储区域 */
  /* 参数：父对象，标题，默认路径，格式 */
  QString path = QFileDialog::getSaveFileName(this, tr("Save  "), "../", tr("TXT (*.txt)"));
  if(path.isEmpty() == true)
  {
    return;
  }

  QFile export_txt_file;
  /* 先关闭 */
  if(export_txt_file.isOpen() == true)
  {
    export_txt_file.close();
  }

  /* 关联文件名 */
  export_txt_file.setFileName(path);

  /* 打开文件，只写方式 */
  if(export_txt_file.open(QIODevice::WriteOnly) == false)
  {
    return;
  }

  QString txt;
  SHOW_MSG_Typedef_t msg;
  QList<SHOW_MSG_Typedef_t> *pmsg_list;

  /* 轮询显示通道 */
  for(int i = 0; i < ui->channel_num_comboBox->count() - 1; i++)
  {
    if(0 == i)
    {
      pmsg_list = &ch1_show_msg_list;
    }
    else
    {
      pmsg_list = &ch2_show_msg_list;
    }
    txt.clear();
    txt.append(QString::asprintf("====CH %d Message====\r\n", i));
    export_txt_file.write(txt.toUtf8());
    for(qint32 line = 0; line < pmsg_list->size(); line++)
    {
      export_txt_file.write(pmsg_list->value(line).str.toUtf8() + "\r\n");
    }
  }

  export_txt_file.close();
}

void more_window::on_crc_pushButton_clicked()
{
  if(true == ui->data_lineEdit->text().isEmpty())
  {
    return;
  }
  if(0 == ui->crc_comboBox->currentIndex())
  {
    return;
  }

  bool ok;
  quint8 temp_data[2048];
  QStringList data_list = ui->data_lineEdit->text().split(' ');
  quint32 len = (quint32)data_list.length() > sizeof(temp_data) ? sizeof(temp_data) : (quint32)data_list.length();

  for(quint32 i = 0; i < len; i++)
  {
    temp_data[i] = (quint8)data_list[i].toUShort(&ok, 16);
  }

  switch(ui->crc_comboBox->currentIndex())
  {
    case 1:
      {
        quint16 crc = utility::get_modbus_crc16_with_tab(temp_data, len);
        data_list.append(QString::asprintf("%02X", crc & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 8) & 0xFF));
        ui->data_lineEdit->setText(data_list.join(" "));
      }
      break;

    case 2:
      {
        quint32 crc = utility::get_crc32_with_tab(temp_data, len);
        data_list.append(QString::asprintf("%02X", crc & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 8) & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 16) & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 24) & 0xFF));
        ui->data_lineEdit->setText(data_list.join(" "));
      }
      break;
    case 3:
      {
        quint8 crc = utility::get_data_sum(temp_data, len);
        data_list.append(QString::asprintf("%02X", crc & 0xFF));
        ui->data_lineEdit->setText(data_list.join(" "));
      }
      break;

    default:
      break;
  }
}


void more_window::on_tool_pushButton_clicked()
{
  tool_window_obj->show();
}

void more_window::on_display_str_id_limit_lineEdit_textChanged(const QString &arg1)
{
  bool ok;
  limit_str_canid = arg1.toUInt(&ok, 16);
}
