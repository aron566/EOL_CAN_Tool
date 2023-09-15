#include "frame_diagnosis.h"
#include "ui_frame_diagnosis.h"
#include <QFile>
#include <QDateTime>
#include <QDebug>

frame_diagnosis::frame_diagnosis(QString title, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::frame_diagnosis)
{
  ui->setupUi(this);

  /* Apply style sheet */
  QFile file(":/qdarkstyle/dark/style.qss");
  if(file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    this->setStyleSheet(file.readAll());
    file.close();
  }

  /* 设置窗口标题 */
  this->setWindowTitle(title);
}

frame_diagnosis::~frame_diagnosis()
{
  delete ui;
}

void frame_diagnosis::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}

void frame_diagnosis::add_msg_to_table(uint16_t id, const quint8 *data, quint32 len, \
  quint8 direct, quint32 channel_num, quint8 protocol_type, quint64 ms)
{
  CAN_MSG_LIST_Typedef_t msg;
  quint32 msg_index = 0;
  for(qint32 i = 0; i < can_msg_list.size(); i++)
  {
    msg = can_msg_list.value(i);
    if(id == msg.id)
    {
      /* 检测是否重复 */
      if(0 == memcmp(msg.data, data, len) \
         && direct == msg.direct \
         && channel_num == msg.channel_num \
         && protocol_type == msg.protocol_type)
      {
        msg.repeat_cnt++;
      }
      memset(msg.data, 0, sizeof(msg.data));
      memcpy(msg.data, data, len);

      quint64 current_ms = ms;
      quint64 elpased_ms = current_ms - msg.last_time_ms;

      msg.last_time_ms = current_ms;
      msg.cycle_time_ms += (elpased_ms - (double)msg.cycle_time_ms) / ((float)msg.cnt + 1.f);

      msg.fps = 1000 / msg.cycle_time_ms;
      msg.cnt++;
      can_msg_list.replace(i, msg);
      msg_index = i;
      goto _update_show_table;
    }
  }
  msg.id = id;
  msg.direct = direct;
  msg.channel_num = channel_num;
  msg.protocol_type = protocol_type;
  memset(msg.data, 0, sizeof(msg.data));
  memcpy(msg.data, data, len);

  msg.last_time_ms = ms;
  msg.cycle_time_ms = 0;
  msg.fps = 1;
  msg.cnt = 1;
  msg.repeat_cnt = 0;
  can_msg_list.append(msg);
  msg_index = (quint32)can_msg_list.size() - 1U;

_update_show_table:
  /* 刷新显示 */
  ui->frame_cnt_tableWidget->setRowCount(can_msg_list.size());

  msg = can_msg_list.value(msg_index);
  int column = 0;
  /* id 方向 通道 协议 接收帧数 重复帧数 帧周期ms 帧率fps */
  ui->frame_cnt_tableWidget->setItem(msg_index, column++, new QTableWidgetItem(QString::asprintf("%04X", msg.id)));
  if(0 == msg.direct)
  {
    ui->frame_cnt_tableWidget->setItem(msg_index, column++, new QTableWidgetItem(QString("TX")));
  }
  else
  {
    ui->frame_cnt_tableWidget->setItem(msg_index, column++, new QTableWidgetItem(QString("RX")));
  }

  ui->frame_cnt_tableWidget->setItem(msg_index, column++, new QTableWidgetItem(QString::number(msg.channel_num)));
  if(0 == msg.protocol_type)
  {
    ui->frame_cnt_tableWidget->setItem(msg_index, column++, new QTableWidgetItem(QString("CAN")));
  }
  else
  {
    ui->frame_cnt_tableWidget->setItem(msg_index, column++, new QTableWidgetItem(QString("CAN FD")));
  }
  ui->frame_cnt_tableWidget->setItem(msg_index, column++, new QTableWidgetItem(QString::number(msg.cnt)));
  ui->frame_cnt_tableWidget->setItem(msg_index, column++, new QTableWidgetItem(QString::number(msg.repeat_cnt)));

  ui->frame_cnt_tableWidget->setItem(msg_index, column++, new QTableWidgetItem(QString::number(msg.cycle_time_ms)));

  ui->frame_cnt_tableWidget->setItem(msg_index, column++, new QTableWidgetItem(QString::number(msg.fps)));
}

void frame_diagnosis::clear()
{
  ui->frame_cnt_tableWidget->clearContents();
  ui->frame_cnt_tableWidget->setRowCount(0);
  can_msg_list.clear();
}
