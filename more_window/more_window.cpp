#include "more_window.h"
#include "ui_more_window.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>

more_window::more_window(QString titile, QWidget *parent) :
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
  this->setWindowTitle(titile);

  /* 初始化定时器 */
  timer_init();
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

void more_window::slot_show_message(const QString &message)
{
  QString show_message;
  show_message.clear();

  /* 时间戳 */
  if(ui->display_time_stamp_checkBox_2->isChecked())
  {
    QDateTime dt = QDateTime::currentDateTime();
    show_message += dt.toString("hh:mm:ss.zzz");
  }

  show_message += message;
  show_message_list.append(show_message);
//  ui->receive_data_textBrowser_2->append(show_message);
//  ui->receive_data_textBrowser_2->moveCursor(QTextCursor::End);
//  if(show_message.contains("Rx"))
//  {
//    rx_frame_cnt++;
//    ui->frame_num_label_2->setNum((int)rx_frame_cnt);
//    rx_byte_cnt += 8;
//    ui->byte_num_label_2->setNum((int)rx_byte_cnt);
//  }
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


void more_window::on_clear_pushButton_2_clicked()
{
  rx_frame_cnt = 0;
  ui->frame_num_label_2->setNum((int)rx_frame_cnt);
  rx_byte_cnt = 0;
  ui->byte_num_label_2->setNum((int)rx_byte_cnt);
  ui->receive_data_textBrowser_2->clear();
  show_message_list.clear();
}

void more_window::set_channel_num(quint8 channel_num)
{
  /* 根据所选设备类型，更新通道数量 */
  ui->channel_num_comboBox->clear();
  for(quint8 i = 0; i < channel_num; i++)
  {
    ui->channel_num_comboBox->addItem(QString("%1").arg(i));
  }
  /* 打开全部通道 */
  ui->channel_num_comboBox->addItem("ALL");
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

  /* 定时刷新显示 */
  if(show_message_list.isEmpty() == true)
  {
    return;
  }
  QString show_message;
  show_message = show_message_list.takeFirst();
  ui->receive_data_textBrowser_2->append(show_message);
  ui->receive_data_textBrowser_2->moveCursor(QTextCursor::End);
}

void more_window::slot_show_message_rx_bytes(quint8 bytes)
{
  rx_frame_cnt++;
  ui->frame_num_label_2->setNum((int)rx_frame_cnt);
  rx_byte_cnt += bytes;
  ui->byte_num_label_2->setNum((int)rx_byte_cnt);
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

