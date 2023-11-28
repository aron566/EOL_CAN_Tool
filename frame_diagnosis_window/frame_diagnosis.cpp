/**
 *  @file frame_diagnosis.cpp
 *
 *  @date 2023年11月27日 11:11:54 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief 帧诊断页面.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.11.27 11:11 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-11-27 <td>v0.0.1  <td>aron566 <td>初始版本
 *
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <QFile>
#include <QDateTime>
#include <QDebug>
/** Private includes ---------------------------------------------------------*/
#include "frame_diagnosis.h"
#include "ui_frame_diagnosis.h"
#include "utilities/utility.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/

/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/

/** Private function prototypes ----------------------------------------------*/

/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

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

void frame_diagnosis::frame_translation(uint16_t id, const quint8 *data, quint32 len, \
                                        quint8 direct, quint32 channel_num, quint8 protocol_type)
{
  if(can_driver_obj == nullptr)
  {
    return;
  }
  if(ui->frame_translation_comboBox->currentText() == "NULL")
  {
    return;
  }

  /* 检测是否Rx */
  if(0U == direct)
  {
    return;
  }

  /* 检测通道号 */
  if((quint32)ui->frame_translation_ch_comboBox->currentIndex() != channel_num && ui->frame_translation_ch_comboBox->currentText() != "ALL")
  {
    return;
  }

  /* 检测ID */
  bool ok;
  if(ui->id_start_lineEdit->text().toUInt(&ok, 16) > id && ui->id_start_lineEdit->text().toUInt(&ok, 16) != 0xFFFFU)
  {
    return;
  }
  if(ui->id_end_lineEdit->text().toUInt(&ok, 16) < id && ui->id_end_lineEdit->text().toUInt(&ok, 16) != 0xFFFFU)
  {
    return;
  }

  /* 转换can to canfd */
  if(ui->frame_translation_comboBox->currentText() == "CAN2CANFD" && 0U == protocol_type)
  {
    /* 设置发送的canid */
    can_driver_obj->set_message_id(QString::number(id, 16));
    /* 设置发送协议类型 */
    can_driver_obj->set_protocol_type((quint32)1U);
    /* 设置发送帧类型 */
    can_driver_obj->set_frame_type((quint32)0U);

    QString data_str = utility::array2hexstr(data, len, " ");
    /* 设置消息数据 */
    can_driver_obj->set_message_data(data_str);
    /* 发送 */
    can_driver_obj->send((quint8)channel_num);
  }
  /* 转换 canfd to can */
  else if(ui->frame_translation_comboBox->currentText() == "CANFD2CAN" && 1U == protocol_type)
  {
    /* 设置发送的canid */
    can_driver_obj->set_message_id(QString::number(id, 16));
    /* 设置发送协议类型 */
    can_driver_obj->set_protocol_type((quint32)0U);
    /* 设置发送帧类型 */
    can_driver_obj->set_frame_type((quint32)0U);

    quint32 index = 0;
    quint32 loop_time = (len + 7U) / 8U;
    for(quint32 i = 0; i < loop_time; i++)
    {
      if((index + 8U) > len)
      {
        QString data_str = utility::array2hexstr(data + index, (quint32)len - index, " ");
        /* 设置消息数据 */
        can_driver_obj->set_message_data(data_str);
        /* 发送 */
//        can_driver_obj->send((quint8)channel_num);
        can_driver_obj->send(data + index, (quint8)(len - index),
                             id,
                             can_driver_model::STD_FRAME_TYPE,
                             can_driver_model::CAN_PROTOCOL_TYPE,
                             (quint8)channel_num);
        return;
      }
      else
      {
        QString data_str = utility::array2hexstr(data + index, 8U, " ");
        /* 设置消息数据 */
        can_driver_obj->set_message_data(data_str);
        /* 发送 */
//        can_driver_obj->send((quint8)channel_num);
        can_driver_obj->send(data + index, 8U,
                             id,
                             can_driver_model::STD_FRAME_TYPE,
                             can_driver_model::CAN_PROTOCOL_TYPE,
                             (quint8)channel_num);
      }
      index += 8U;
    }
  }
  else
  {
    /* to do nothing */
  }
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

void frame_diagnosis::set_channel_num(quint8 channel_num)
{
  /* 根据所选设备类型，更新通道数量 */
  ui->frame_translation_ch_comboBox->clear();
  for(quint8 i = 0; i < channel_num; i++)
  {
    ui->frame_translation_ch_comboBox->addItem(QString("%1").arg(i));
  }
  /* 打开全部通道 */
  ui->frame_translation_ch_comboBox->addItem("ALL");
}

void frame_diagnosis::add_msg_to_table(uint16_t id, const quint8 *data, quint32 len, \
                                       quint8 direct, quint32 channel_num, quint8 protocol_type, quint64 ms)
{
  /* 报文转换 */
  frame_translation(id, data, len, direct, channel_num, protocol_type);

  CAN_MSG_LIST_Typedef_t msg;
  quint32 msg_index = 0;
  for(qint32 i = 0; i < can_msg_list.size(); i++)
  {
    msg = can_msg_list.value(i);
    if(id == msg.id
        && direct == msg.direct \
        && channel_num == msg.channel_num \
        && protocol_type == msg.protocol_type)
    {
      /* 检测是否重复 */
      if(0 == memcmp(msg.data, data, len) \
          && id == msg.id \
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

/******************************** End of file *********************************/
