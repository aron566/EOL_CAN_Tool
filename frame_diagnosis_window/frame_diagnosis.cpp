#include "frame_diagnosis.h"
#include "ui_frame_diagnosis.h"
#include <QFile>
#include <cstdint>

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
  quint8 direct, quint32 channel_num, quint8 protocol_type)
{
  CAN_MSG_LIST_Typedef_t msg;
  for(qint32 i = 0; i < can_msg_list.size(); i++)
  {
    msg = can_msg_list.value(i);
    if(id == msg.id)
    {
      msg.cnt++;
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
      can_msg_list.replace(i, msg);
      goto _update_show_table;
    }
  }
  msg.id = id;
  msg.direct = direct;
  msg.channel_num = channel_num;
  msg.protocol_type = protocol_type;
  memset(msg.data, 0, sizeof(msg.data));
  memcpy(msg.data, data, len);
  msg.cnt = 1;
  msg.repeat_cnt = 0;
  can_msg_list.append(msg);

_update_show_table:
  /* 刷新显示 */
  ui->frame_cnt_tableWidget->clearContents();
  ui->frame_cnt_tableWidget->setRowCount(can_msg_list.size());
  for(qint32 i = 0; i < can_msg_list.size(); i++)
  {
    msg = can_msg_list.value(i);
    int column = 0;
    /* id 方向 通道 协议 接收帧数 重复帧数 */
    ui->frame_cnt_tableWidget->setItem(i, column++, new QTableWidgetItem(QString::asprintf("%04X", msg.id)));
    if(0 == msg.direct)
    {
      ui->frame_cnt_tableWidget->setItem(i, column++, new QTableWidgetItem(QString("TX")));
    }
    else
    {
      ui->frame_cnt_tableWidget->setItem(i, column++, new QTableWidgetItem(QString("RX")));
    }

    ui->frame_cnt_tableWidget->setItem(i, column++, new QTableWidgetItem(QString::number(msg.channel_num)));
    if(0 == msg.protocol_type)
    {
      ui->frame_cnt_tableWidget->setItem(i, column++, new QTableWidgetItem(QString("CAN")));
    }
    else
    {
      ui->frame_cnt_tableWidget->setItem(i, column++, new QTableWidgetItem(QString("CAN FD")));
    }
    ui->frame_cnt_tableWidget->setItem(i, column++, new QTableWidgetItem(QString::number(msg.cnt)));
    ui->frame_cnt_tableWidget->setItem(i, column++, new QTableWidgetItem(QString::number(msg.repeat_cnt)));
  }
}

void frame_diagnosis::clear()
{
  ui->frame_cnt_tableWidget->clearContents();
  ui->frame_cnt_tableWidget->setRowCount(0);
  can_msg_list.clear();
}
