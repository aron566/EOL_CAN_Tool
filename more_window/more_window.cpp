#include "more_window.h"
#include "ui_more_window.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QWheelEvent>

#define SHOW_MSG_SAVE_NUM_MAX     100U                    /**< 最大显示消息数 */
#define SHOW_MSG_ONE_SCORLL       (5U)                    /**< 上翻每次刷新列表数 */
#define SAVE_MSG_BUF_MAX          (1024U*1024U*10U)       /**< 最大缓存消息数 */

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

  /* 帧诊断调试窗口初始化 */
  frame_diagnosis_window_init(tr("EOL CAN Tool - Frame Diagnosis"));

  /* 初始化定时器 */
  timer_init();

  ui->ch1_receive_data_textEdit->setVisible(true);
  ui->ch2_receive_data_textEdit->setVisible(true);
  ui->ch1_receive_data_textEdit->setUndoRedoEnabled(false);
  ui->ch2_receive_data_textEdit->setUndoRedoEnabled(false);
}

more_window::~more_window()
{
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
  QTextEdit *text_edit_widget = nullptr;
  QTextEdit *text_edit_widget_temp = ui->ch1_receive_data_textEdit;
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
  QTextEdit *text_edit_widget = nullptr;
  SHOW_MSG_Typedef_t show_messagex;
  bool remove_line_flag = false;

  if(ch1_show_msg_is_empty() == false)
  {
    quint32 show_index = get_show_index(ch1_show_msg_index, ch1_add_msg_index);
    show_messagex = ch1_show_msg_list.value(show_index);
    ch1_show_msg_index++;
    ch1_scroll_cnt = show_index + 1;

    text_edit_widget = ui->ch1_receive_data_textEdit;
    if(SHOW_MSG_SAVE_NUM_MAX < ch1_show_msg_index)
    {
      remove_line_flag = true;
    }

    /* 设置颜色 */
    if(can_driver::CAN_RX_DIRECT == show_messagex.direct)
    {
      text_edit_widget->setTextColor(QColor("white"));
    }
    else
    {
      text_edit_widget->setTextColor(QColor("red"));
    }

    text_edit_widget->append(show_messagex.str);

    if(true == remove_line_flag)
    {
      text_edit_widget->moveCursor(QTextCursor::Start);
      text_edit_widget->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
      text_edit_widget->moveCursor(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
  //    qDebug() << "sel2 :" << text_edit_widget->textCursor().selectedText();
      text_edit_widget->textCursor().removeSelectedText();
      text_edit_widget->moveCursor(QTextCursor::End);
    }
  }

  if(ch2_show_msg_is_empty() == false)
  {
    quint32 show_index = get_show_index(ch2_show_msg_index, ch2_add_msg_index);
    show_messagex = ch2_show_msg_list.value(show_index);
    ch2_show_msg_index++;
    ch2_scroll_cnt = show_index + 1;

    text_edit_widget = ui->ch2_receive_data_textEdit;
    if(SHOW_MSG_SAVE_NUM_MAX < ch2_show_msg_index)
    {
      remove_line_flag = true;
    }

    /* 设置颜色 */
    if(can_driver::CAN_RX_DIRECT == show_messagex.direct)
    {
      text_edit_widget->setTextColor(QColor("white"));
    }
    else
    {
      text_edit_widget->setTextColor(QColor("red"));
    }

    text_edit_widget->append(show_messagex.str);

    if(true == remove_line_flag)
    {
      text_edit_widget->moveCursor(QTextCursor::Start);
      text_edit_widget->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
      text_edit_widget->moveCursor(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
  //    qDebug() << "sel2 :" << text_edit_widget->textCursor().selectedText();
      text_edit_widget->textCursor().removeSelectedText();
      text_edit_widget->moveCursor(QTextCursor::End);
    }
  }
}

void more_window::update_show_msg(QTextEdit *text_edit_widget, QList<SHOW_MSG_Typedef_t> *pList, quint32 show_index, bool downward_flag)
{
  SHOW_MSG_Typedef_t show_messagex = pList->value(show_index);

  /* 下翻 */
  if(downward_flag)
  { 
    text_edit_widget->moveCursor(QTextCursor::End);
    QTextCursor cursor = text_edit_widget->textCursor();
    QTextCharFormat format;
    /* 设置颜色 */
    if(can_driver::CAN_RX_DIRECT == show_messagex.direct)
    {
      format.setForeground(Qt::white);
    }
    else
    {
      format.setForeground(Qt::red);
    }
    cursor.setCharFormat(format);
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
    text_edit_widget->moveCursor(QTextCursor::End);
    text_edit_widget->moveCursor(QTextCursor::PreviousBlock, QTextCursor::KeepAnchor);
    text_edit_widget->moveCursor(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
//    qDebug() << "sel2 :" << text_edit_widget->textCursor().selectedText();
    text_edit_widget->textCursor().removeSelectedText();

    text_edit_widget->moveCursor(QTextCursor::Start);
    QTextCursor cursor = text_edit_widget->textCursor();
    QTextCharFormat format;
    /* 设置颜色 */
    if(can_driver::CAN_RX_DIRECT == show_messagex.direct)
    {
      format.setForeground(Qt::white);
    }
    else
    {
      format.setForeground(Qt::red);
    }
    cursor.setCharFormat(format);
    cursor.insertText(show_messagex.str + '\n');
    text_edit_widget->setTextCursor(cursor);
  }
}

void more_window::frame_diagnosis_window_init(QString title)
{
  frame_diagnosis_obj = new frame_diagnosis(title);
}

void more_window::slot_can_driver_msg(quint16 can_id, const quint8 *data, quint32 len, \
  quint8 direct, quint32 channel_num, quint8 protocol_type, quint64 ms)
{
  frame_diagnosis_obj->add_msg_to_table(can_id, data, len, direct, channel_num, protocol_type, ms);
}

void more_window::slot_show_message(const QString &message, quint32 channel_num, \
  quint8 direct, const quint8 *data, quint32 data_len)
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
    if(ui->display_ch_comboBox->currentIndex() == channel_num && can_driver::CAN_RX_DIRECT == direct)
    {
      char str_buf[65];
      size_t size = data_len > 64 ? 64 : data_len;
      memcpy(str_buf, data, size);
      if(str_buf[size - 1] != '\0')
      {
        str_buf[size] = '\0';
      }
      show_message.append(QString("[%1]RX:").arg(channel_num));
      QString str = QString::asprintf("%s", str_buf);
      show_message.append(str);
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
    if(ch1_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
    {
      ch1_show_msg_list.removeFirst();
    }
    ch1_show_msg_list.append(msg);
    ch1_add_msg_index++;

    goto __show_msg;
  }

  if(ch2_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
  {
    ch2_show_msg_list.removeFirst();
  }
  ch2_show_msg_list.append(msg);
  ch2_add_msg_index++;

__show_msg:

  /* 显示 */
  show_txt();
}


void more_window::slot_show_message_block(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data, quint32 data_len)
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
    if(ui->display_ch_comboBox->currentIndex() == channel_num && can_driver::CAN_RX_DIRECT == direct)
    {
      char str_buf[65];
      size_t size = data_len > 64 ? 64 : data_len;
      memcpy(str_buf, data, size);
      if(str_buf[size - 1] != '\0')
      {
        str_buf[size] = '\0';
      }
      show_message.append(QString("[%1]RX:").arg(channel_num));
      QString str = QString::asprintf("%s", str_buf);
      show_message.append(str);
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
    if(ch1_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
    {
      ch1_show_msg_list.removeFirst();
    }
    ch1_show_msg_list.append(msg);
    ch1_add_msg_index++;

    goto __show_msg;
  }

  if(ch2_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
  {
    ch2_show_msg_list.removeFirst();
  }
  ch2_show_msg_list.append(msg);
  ch2_add_msg_index++;

__show_msg:

  /* 显示 */
  show_txt();
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
  if(nullptr == can_driver_obj)
  {
    return;
  }
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
//  static quint32 i = 0;
//  slot_show_message(QString("message %1").arg(i++), 0, \
//    0, nullptr, 0);
//  if(i == 255)
//  {
//    timer_obj->stop();
//  }
#if 0
  /* 显示 */
  show_txt();
#endif
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
  frame_diagnosis_obj->show();
}

