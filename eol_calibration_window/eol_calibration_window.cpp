#include "eol_calibration_window.h"
#include "ui_eol_calibration_window.h"
#include <QFile>
#include <QDateTime>
#include <QMessageBox>

eol_calibration_window::eol_calibration_window(QString title, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::eol_calibration_window)
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

  /* 初始化定时器 */
  timer_init();

  /* 设置提示值 */
  ui->mag_threshold_max_lineEdit->setPlaceholderText("20m105 - 50m85");
  ui->mag_threshold_min_lineEdit->setPlaceholderText("20m90 - 50m70");
  ui->snr_threshold_max_lineEdit->setPlaceholderText("20m70 - 50m50");
  ui->snr_threshold_min_lineEdit->setPlaceholderText("20m45 - 50m30");
  ui->rts_lineEdit->setPlaceholderText("15 - 25 - 35");

  /* 设置表格 */

}

eol_calibration_window::~eol_calibration_window()
{
  delete ui;
}

/**
 * @brief 窗口关闭事件
 * @param event
 */
void eol_calibration_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}

/**
 * @brief 窗口显示事件
 * @param event
 */
void eol_calibration_window::showEvent(QShowEvent *event)
{
  Q_UNUSED(event)

  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 读取校准模式 */
  task.reg = EOL_RW_CALI_MODE_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}

/**
 * @brief 定时器初始化
 */
void eol_calibration_window::timer_init()
{
  timer_obj = new QTimer(this);
  timer_obj->setInterval(ui->refresh_time_lineEdit->text().toInt());
  connect(timer_obj, &QTimer::timeout, this, &eol_calibration_window::slot_timeout);
}

/**
 * @brief 设置eol协议栈对象
 * @param obj
 */
void eol_calibration_window::set_eol_protocol_obj(eol_protocol *obj)
{
  if(nullptr == obj)
  {
    return;
  }
  eol_protocol_obj = obj;
  /* 数据接收 */
//  connect(eol_protocol_obj, &eol_protocol::signal_rw_device_ok, this, &eol_calibration_window::slot_rw_device_ok);
  connect(eol_protocol_obj, &eol_protocol::signal_protocol_rw_err, this, &eol_calibration_window::slot_protocol_rw_err);
  connect(eol_protocol_obj, &eol_protocol::signal_rw_device_ok, this, &eol_calibration_window::slot_rw_device_ok, Qt::BlockingQueuedConnection);
}

/**
 * @brief 刷新目标数据
 */
void eol_calibration_window::refresh_obj_list_info(quint8 profile_id, quint16 obj_num, const quint8 *data)
{
  if(obj_num == 0xFFFF && profile_id == 0xFF)
  {
    return;
  }

  frame_cnt++;

  qDebug() << "profile " << profile_id << "obj_num " << obj_num;

  /* 设置当前表格行数 */
  ui->tableWidget->clearContents();
  ui->tableWidget->setRowCount(obj_num);

  /* 刷新数据 */
  quint16 index = 0;
  qint16 speed;
  qint16 azi_angle;
  quint32 distance;
  qint16 mag;
  qint16 rcs;
  qint16 snr;
  qint16 ele_angle;

  for(quint16 i = 0; i < obj_num; i++)
  {
    memcpy(&speed, data + index, sizeof(speed));
    index += sizeof(speed);

    memcpy(&azi_angle, data + index, sizeof(azi_angle));
    index += sizeof(azi_angle);

    memcpy(&distance, data + index, sizeof(distance));
    index += sizeof(distance);

    memcpy(&mag, data + index, sizeof(mag));
    index += sizeof(mag);

    memcpy(&rcs, data + index, sizeof(rcs));
    index += sizeof(rcs);

    memcpy(&snr, data + index, sizeof(snr));
    index += sizeof(snr);

    memcpy(&ele_angle, data + index, sizeof(ele_angle));
    index += sizeof(ele_angle);

    /* 目标序号 配置ID 速度 方位角 距离 mag rcs snr 俯仰角 */
    ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(profile_id)));
    ui->tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number((double)speed/100)));
    ui->tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number((double)azi_angle/100)));
    ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number((double)distance/100)));
    ui->tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number((double)mag/10)));
    ui->tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number((double)rcs/10)));
    ui->tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number((double)snr/10)));
    ui->tableWidget->setItem(i, 7, new QTableWidgetItem(QString::number((double)ele_angle/100)));
  }
}

/**
 * @brief 添加阈值到列表
 */
void eol_calibration_window::on_add_pushButton_clicked()
{
  THRESHOLD_SET_INFO_Typedef_t threshold_info;
  threshold_info.distance_m = (quint8)ui->distance_comboBox->currentText().toUShort();
  threshold_info.mag_dB_threshold_down = (qint8)ui->mag_threshold_min_lineEdit->text().toShort();
  threshold_info.mag_dB_threshold_up = (qint8)ui->mag_threshold_max_lineEdit->text().toShort();
  threshold_info.snr_dB_threshold_down = (qint8)ui->snr_threshold_min_lineEdit->text().toShort();
  threshold_info.snr_dB_threshold_up = (qint8)ui->snr_threshold_max_lineEdit->text().toShort();
  threshold_info.rts_dBsm = (quint8)ui->rts_lineEdit->text().toUShort();

  threshold_info.str = QString::asprintf("%um,mag:>%u <%u,snr:>%u <%u,rts:%u\r\n", \
                                         threshold_info.distance_m, \
                                         threshold_info.mag_dB_threshold_down, \
                                         threshold_info.mag_dB_threshold_up, \
                                         threshold_info.snr_dB_threshold_down, \
                                         threshold_info.snr_dB_threshold_up, \
                                         threshold_info.rts_dBsm);
  threshold_list.append(threshold_info);
  QString str;
  ui->threshold_list_label->clear();
  for(qint32 i = 0; i < threshold_list.size(); i++)
  {
    str += threshold_list.value(i).str;
  }
  ui->threshold_list_label->setText(str);
}

/**
 * @brief 清空阈值列表
 */
void eol_calibration_window::on_reset_pushButton_clicked()
{
  /* 清空传输列表 */
  threshold_list.clear();
  ui->threshold_list_label->clear();
  timer_obj->stop();
  time_ms = 0;
  time_s = 0;
  frame_cnt = 0;
  fps = 0;
}

/**
 * @brief 启动测试
 */
void eol_calibration_window::on_test_start_pushButton_clicked()
{
  if(threshold_list.size() == 0)
  {
    return;
  }

  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 读取目标 */
  task.reg = EOL_R_OBJ_LIST_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 启动->停止 */
  if(true == timer_obj->isActive())
  {
    time_ms = 0;
    time_s = 0;
    frame_cnt = 0;
    timer_obj->stop();
    eol_protocol_obj->stop_task();
    ui->test_start_pushButton->setText(tr("start"));
    ui->read_rcs_offset_pushButton->setEnabled(true);
    return;
  }

  /* 启动mag snr rcs 数值监测任务，及校准任务 */
  /* 停止->启动，目标获取 */
  timer_obj->start();
  ui->test_start_pushButton->setText(tr("stop"));
  ui->read_rcs_offset_pushButton->setEnabled(false);

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}

/**
 * @brief 数据接收
 */
void eol_calibration_window::slot_rw_device_ok(quint8 reg_addr, const quint8 *data, quint16 data_size)
{
  Q_UNUSED(data_size)
  switch(reg_addr)
  {
    case EOL_W_SAVE_PAR_REG:
      break;
    /* rcs校准，目标测试 */
    case EOL_R_OBJ_LIST_REG:
      {  
        quint8 profile_id = data[0];
        quint16 obj_num = 0;
        memcpy(&obj_num, data + 1, sizeof(obj_num));
        refresh_obj_list_info(profile_id, obj_num, data + 3);
        err_cnt = 0;

        /* 再次更新 */
        eol_protocol::EOL_TASK_LIST_Typedef_t task;
        task.param = nullptr;

        /* 读取目标 */
        task.reg = EOL_R_OBJ_LIST_REG;
        task.command = eol_protocol::EOL_READ_CMD;
        task.buf[0] = 0;
        task.len = 0;
        eol_protocol_obj->eol_master_common_rw_device(task, false);
      }
      break;
    case EOL_RW_PROFILE_ID_REG:
      break;

    /* 角度校准 */
    case EOL_W_2DFFT_CONDITION_REG:
    case EOL_R_2DFFT_DATA_REG:
      break;
    case EOL_RW_RCS_OFFSET_REG:
      {
        /* 写入 */
        if(nullptr == data)
        {

        }
        /* 读取 */
        else
        {
          quint32 index = 0;
          quint8 profile_num = data[index++];
          qint16 rcs_offset = 0;
          quint8 profile_id = 0;
          QString tips_str;
          for(quint8 i = 0; i < profile_num; i++)
          {
            profile_id = data[index++];
            memcpy(&rcs_offset, data + index, sizeof(rcs_offset));
            index += sizeof(rcs_offset);
            tips_str += QString("<font size='5' color='green'><div align='legt'>profile id[%1] rcs offset:</div> <div align='right'>%2</div> </font>\r\n").arg(profile_id).arg((float)rcs_offset / 10.f);
          }
          /* 显示rcs offset信息 */
          QMessageBox message(QMessageBox::Information, tr("RCS Info"), tr(tips_str.toUtf8()), QMessageBox::Yes, nullptr);
          message.exec();
        }
      }
      break;
    case EOL_W_PAR_RESET_REG:
      break;
    case EOL_RW_CALI_MODE_REG:
      {
        /* 写入 */
        if(nullptr == data)
        {
          ui->rcs_rsl_state_show_label->setText(tr("set ok"));
        }
        else
        {
          /* 读取 */
          quint32 index = 0;
          quint8 profile_num = data[index++];
          qint8 cali_mode = 0;
          quint8 profile_id = 0;
          QString tips_str;
          for(quint8 i = 0; i < profile_num; i++)
          {
            profile_id = data[index++];
            memcpy(&cali_mode, data + index, sizeof(cali_mode));
            index += sizeof(cali_mode);
            tips_str += QString("<font size='5' color='green'><div align='legt'>profile id[%1] cali_mode:</div> <div align='right'>%2</div> </font>\r\n").arg(profile_id).arg(cali_mode);
          }
          /* 显示Cali Mode信息 */
          QMessageBox message(QMessageBox::Information, tr("Cali Mode Info"), tr(tips_str.toUtf8()), QMessageBox::Yes, nullptr);
          message.exec();
        }
      }
      break;
  }
}

/**
 * @brief 从机读写无反应信号
 */
void eol_calibration_window::slot_protocol_rw_err(quint8 reg, quint8 command)
{
  switch(reg)
  {
    case EOL_W_SAVE_PAR_REG:
      break;
    /* rcs校准，目标测试 */
    case EOL_R_OBJ_LIST_REG:
      {
        if(false == timer_obj->isActive())
        {
          err_cnt = 0;
          return;
        }
        err_cnt++;
        if(err_cnt > 10)
        {
          time_ms = 0;
          time_s = 0;
          frame_cnt = 0;
          timer_obj->stop();
          ui->test_start_pushButton->setText(tr("start"));
        }
      }
      break;

    case EOL_RW_PROFILE_ID_REG:

    /* 角度校准 */
    case EOL_W_2DFFT_CONDITION_REG:
    case EOL_R_2DFFT_DATA_REG:

    case EOL_RW_RCS_OFFSET_REG:
    case EOL_W_PAR_RESET_REG:
    case EOL_RW_CALI_MODE_REG:
      break;
  }
}

/**
 * @brief 目标距离更新
 */
void eol_calibration_window::on_distance_comboBox_currentTextChanged(const QString &arg1)
{

}

/**
 * @brief 定时器
 */
void eol_calibration_window::slot_timeout()
{
  if(nullptr == eol_protocol_obj)
  {
    return;
  }

  time_ms += (quint32)timer_obj->interval();
  if(time_ms >= 1000)
  {
    time_s++;
    time_ms = 0;
  }

  if(time_s != 0)
  {
    fps = (double)frame_cnt / (double)time_s;
    ui->fps_lineEdit->setText(QString::number(fps));
  }

  /* 检测是否刷新 */
  if(true == eol_protocol_obj->task_is_runing())
  {
    return;
  }

  /* 再次更新 */
  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 读取目标 */
  task.reg = EOL_R_OBJ_LIST_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task, false);

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}

/**
 * @brief 配置id选择
 * @param index
 */
void eol_calibration_window::on_profile_id_comboBox_currentIndexChanged(int index)
{
  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 软硬件版本号 */
  task.reg = EOL_RW_PROFILE_ID_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  task.buf[0] = quint8(index);
  task.len = 1;
  eol_protocol_obj->eol_master_common_rw_device(task, false);

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}

/**
 * @brief 刷新周期更新
 */
void eol_calibration_window::on_refresh_time_lineEdit_editingFinished()
{
  timer_obj->setInterval(ui->refresh_time_lineEdit->text().toInt());
  time_ms = 0;
  time_s = 0;
  frame_cnt = 0;
  eol_protocol_obj->stop_task();
}

/**
 * @brief 读取rcs补偿值
 */
void eol_calibration_window::on_read_rcs_offset_pushButton_clicked()
{
  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 软硬件版本号 */
  task.reg = EOL_RW_RCS_OFFSET_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}


void eol_calibration_window::on_set_cali_mode_pushButton_clicked()
{
  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 设置校准模式 */
  task.reg = EOL_RW_CALI_MODE_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  task.buf[0] = 1;
  task.buf[1] = (quint8)ui->profile_id_comboBox->currentIndex();
  task.buf[2] = (quint8)ui->cali_mode_comboBox->currentIndex();
  task.len = 3;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 保存 */
  task.reg = EOL_W_SAVE_PAR_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  task.buf[0] = 1;
  task.len = 1;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}

