#include "more_window.h"
#include "ui_more_window.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>

#define USE_TEXT_BROWSER_WIDDGET  0/**< 是否 使用textbrowser控件 */
#define SHOW_MSG_SAVE_NUM_MAX     10000U/**< 最大保存消息数 */

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

  /* 初始化定时器 */
  timer_init();

#if USE_TEXT_BROWSER_WIDDGET
  ui->ch1_receive_data_textBrowser->setVisible(true);
  ui->ch2_receive_data_textBrowser->setVisible(true);
  ui->ch1_receive_data_textEdit->setVisible(false);
  ui->ch2_receive_data_textEdit->setVisible(false);
  ui->ch1_receive_data_textBrowser->setUndoRedoEnabled(false);
  ui->ch2_receive_data_textBrowser->setUndoRedoEnabled(false);
#else
  ui->ch1_receive_data_textBrowser->setVisible(false);
  ui->ch2_receive_data_textBrowser->setVisible(false);
  ui->ch1_receive_data_textEdit->setVisible(true);
  ui->ch2_receive_data_textEdit->setVisible(true);
  ui->ch1_receive_data_textEdit->setUndoRedoEnabled(false);
  ui->ch2_receive_data_textEdit->setUndoRedoEnabled(false);
#endif
}

more_window::~more_window()
{
  delete ui;
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
  /* 切换窗口大小时，缓冲区有数据则清除一次 */
  if(show_msg_list.isEmpty() == false)
  {
    on_clear_pushButton_clicked();
  }

  QWidget::resizeEvent(event);
}

void more_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  /* 暂停其他数据接收显示 */
//  can_driver_obj->set_msg_canid_mask(0x0000, true, &last_canid_mask, &last_canid_mask_en);

  this->hide();
  emit signal_more_window_closed();
}

//void more_window::showEvent(QShowEvent *event)
//{
//  Q_UNUSED(event)

//  /* 恢复数据接收显示 */
//  can_driver_obj->set_msg_canid_mask(last_canid_mask, last_canid_mask_en);
//}

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
  show_msg_list.append(msg);
}


void more_window::slot_show_message_block(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data, quint32 data_len)
{
  bool remove_line_flag = false;

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

#if USE_TEXT_BROWSER_WIDDGET
  QTextBrowser *browser_widget=  nullptr;
  if(channel_num == 0)
  {
    browser_widget = ui->ch1_receive_data_textBrowser;
  }
  else
  {
    browser_widget = ui->ch2_receive_data_textBrowser;
  }
  browser_widget->append(show_message);
  browser_widget->moveCursor(QTextCursor::End);
#else
  QTextEdit *text_edit_widget=  nullptr;
  if(channel_num == 0)
  {
    text_edit_widget = ui->ch1_receive_data_textEdit;
    ch1_show_msg_cnt++;
    if(SHOW_MSG_SAVE_NUM_MAX < ch1_show_msg_cnt)
    {
      remove_line_flag = true;
    }
  }
  else
  {
    text_edit_widget = ui->ch2_receive_data_textEdit;
    ch2_show_msg_cnt++;
    if(SHOW_MSG_SAVE_NUM_MAX < ch2_show_msg_cnt)
    {
      remove_line_flag = true;
    }
  }

  /* 设置颜色 */
  if(can_driver::CAN_RX_DIRECT == direct)
  {
    text_edit_widget->setTextColor(QColor("white"));
  }
  else
  {
    text_edit_widget->setTextColor(QColor("red"));
  }
  text_edit_widget->append(show_message);

#endif

  if(true == remove_line_flag)
  {
    text_edit_widget->moveCursor(QTextCursor::Start);
    text_edit_widget->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    text_edit_widget->moveCursor(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
//    qDebug() << "sel1 :" << text_edit_widget->textCursor().selectedText();
    text_edit_widget->textCursor().removeSelectedText();
    text_edit_widget->moveCursor(QTextCursor::End);
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
  eol_ui->show();
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

#if USE_TEXT_BROWSER_WIDDGET
  ui->ch1_receive_data_textBrowser->clear();
  ui->ch2_receive_data_textBrowser->clear();
#else
  ui->ch1_receive_data_textEdit->clear();
  ui->ch2_receive_data_textEdit->clear();
#endif

  show_msg_list.clear();

  ch1_show_msg_cnt = 0;
  ch2_show_msg_cnt = 0;
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
  bool remove_line_flag = false;

  /* 定时发送 */
  if(ui->timer_checkBox->isChecked())
  {
    can_driver_obj->send(ui->channel_num_comboBox->currentIndex());
  }

  /* 定时刷新显示 */
  if(show_msg_list.isEmpty() == true)
  {
    return;
  }
  SHOW_MSG_Typedef_t show_message;
  show_message = show_msg_list.takeFirst();

#if USE_TEXT_BROWSER_WIDDGET
  QTextBrowser *browser_widget=  nullptr;
  if(show_message.channel_num == 0)
  {
    browser_widget = ui->ch1_receive_data_textBrowser;
  }
  else
  {
    browser_widget = ui->ch2_receive_data_textBrowser;
  }
  browser_widget->append(show_message.str);
  browser_widget->moveCursor(QTextCursor::End);

#else
  QTextEdit *text_edit_widget=  nullptr;
  if(show_message.channel_num == 0)
  {
    text_edit_widget = ui->ch1_receive_data_textEdit;
    ch1_show_msg_cnt++;
    if(SHOW_MSG_SAVE_NUM_MAX < ch1_show_msg_cnt)
    {
      remove_line_flag = true;
    }
  }
  else
  {
    text_edit_widget = ui->ch2_receive_data_textEdit;
    ch2_show_msg_cnt++;
    if(SHOW_MSG_SAVE_NUM_MAX < ch2_show_msg_cnt)
    {
      remove_line_flag = true;
    }
  }

  /* 设置颜色 */
  if(can_driver::CAN_RX_DIRECT == show_message.direct)
  {
    text_edit_widget->setTextColor(QColor("white"));
  }
  else
  {
    text_edit_widget->setTextColor(QColor("red"));
  }

  text_edit_widget->append(show_message.str);
#endif

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

