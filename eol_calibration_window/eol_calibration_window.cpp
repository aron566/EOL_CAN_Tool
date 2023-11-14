#include "eol_calibration_window.h"
#include "ui_eol_calibration_window.h"
#include <QFile>
#include <QFileDialog>
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

  ui->target_total_range_lineEdit->setPlaceholderText("-100 100m ±2");
  ui->target_total_azi_lineEdit->setPlaceholderText("-80 80deg ±5");
  ui->target_total_ele_lineEdit->setPlaceholderText("-20 20deg ±5");
  ui->target_total_velocity_lineEdit->setPlaceholderText("-10 10km/h ±2");
  ui->target_total_rcs_lineEdit->setPlaceholderText("10 20dB ±5");
  ui->target_total_snr_lineEdit->setPlaceholderText("-10 10dB ±5");
  ui->target_total_mag_lineEdit->setPlaceholderText("-10 10dB ±5");

  /* 设置悬浮提示 */
  ui->target_total_range_lineEdit->setToolTip(tr("this conditions is not used if it is empty"));
  ui->target_total_azi_lineEdit->setToolTip(tr("this conditions is not used if it is empty"));
  ui->target_total_ele_lineEdit->setToolTip(tr("this conditions is not used if it is empty"));
  ui->target_total_velocity_lineEdit->setToolTip(tr("this conditions is not used if it is empty"));
  ui->target_total_rcs_lineEdit->setToolTip(tr("this conditions is not used if it is empty"));
  ui->target_total_snr_lineEdit->setToolTip(tr("this conditions is not used if it is empty"));
  ui->target_total_mag_lineEdit->setToolTip(tr("this conditions is not used if it is empty"));

  /* 设置表格 */
//  ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  ui->tableWidget->verticalHeader()->setFixedWidth(25);
  ui->target_cnt_tableWidget->verticalHeader()->setFixedWidth(30);
  ui->target_cnt_tableWidget->setVisible(false);
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

  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 设置校准工作波形 */
  task.reg = EOL_RW_TX_WAVE_MODE_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  task.buf[0] = 0;
  task.len = 1;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 启动eol线程 */
  eol_protocol_obj->start_task();

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

  /* 设置正常工作波形 */
  task.reg = EOL_RW_TX_WAVE_MODE_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  task.buf[0] = 1;
  task.len = 1;
  eol_protocol_obj->eol_master_common_rw_device(task);

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
  connect(eol_protocol_obj, &eol_protocol::signal_protocol_rw_err, this, &eol_calibration_window::slot_protocol_rw_err, Qt::BlockingQueuedConnection);
  connect(eol_protocol_obj, &eol_protocol::signal_rw_device_ok, this, &eol_calibration_window::slot_rw_device_ok, Qt::QueuedConnection);
}

/**
   * @brief 目标统计信息过滤
   * @param target 目标信息
   * @return true 需要过滤
   */
bool eol_calibration_window::obj_cnt_filter_check(TARGET_CNT_LIST_Typedef_t &target)
{
  /* 检查筛选设置项是否为空，不为空则有效 */

  /* 距离 */
  if(false == ui->target_total_range_lineEdit->text().isEmpty())
  {
    qint32 val = ui->target_total_range_lineEdit->text().toInt();
    if(abs((float)val - target.distance) > 2.0)
    {
      return true;
    }
  }

  /* 方位 */
  if(false == ui->target_total_azi_lineEdit->text().isEmpty())
  {
    qint32 val = ui->target_total_azi_lineEdit->text().toInt();
    if(abs((float)val - target.azi_angle) > 5.0)
    {
      return true;
    }
  }

  /* 俯仰 */
  if(false == ui->target_total_ele_lineEdit->text().isEmpty())
  {
    qint32 val = ui->target_total_ele_lineEdit->text().toInt();
    if(abs((float)val - target.ele_angle) > 5.0)
    {
      return true;
    }
  }

  /* 速度 */
  if(false == ui->target_total_velocity_lineEdit->text().isEmpty())
  {
    qint32 val = ui->target_total_velocity_lineEdit->text().toInt();
    if(abs((float)val - target.speed) > 2.0)
    {
      return true;
    }
  }

  /* rcs */
  if(false == ui->target_total_rcs_lineEdit->text().isEmpty())
  {
    qint32 val = ui->target_total_rcs_lineEdit->text().toInt();
    if(abs((float)val - target.rcs) > 5.0)
    {
      return true;
    }
  }

  /* snr */
  if(false == ui->target_total_snr_lineEdit->text().isEmpty())
  {
    qint32 val = ui->target_total_snr_lineEdit->text().toInt();
    if(abs((float)val - target.snr) > 5.0)
    {
      return true;
    }
  }

  /* mag */
  if(false == ui->target_total_mag_lineEdit->text().isEmpty())
  {
    qint32 val = ui->target_total_mag_lineEdit->text().toInt();
    if(abs((float)val - target.mag) > 5.0)
    {
      return true;
    }
  }

  return false;
}

void eol_calibration_window::refresh_obj_cnt_list_info(TARGET_CNT_LIST_Typedef_t &target)
{
  if(ui->target_cnt_en_checkBox->isChecked() == false)
  {
    return;
  }

  /* 检查筛选设置 */
  if(true == obj_cnt_filter_check((target)))
  {
    return;
  }

  /* 匹配 配置id 距离 速度 方位角度 俯仰 mag */
  TARGET_CNT_LIST_Typedef_t cur_target;
  int index = 0;
  for(int i = 0; i < target_cnt_list.size(); i++)
  {
    cur_target = target_cnt_list.value(i);
    if(cur_target.profile_id == target.profile_id &&
        (cur_target.azi_angle >= target.azi_angle - 0.05f && cur_target.azi_angle <= target.azi_angle + 0.05f) &&
        (cur_target.ele_angle >= target.ele_angle - 0.05f && cur_target.ele_angle <= target.ele_angle + 0.05f) &&
        (cur_target.distance >= target.distance - 0.05f && cur_target.distance <= target.distance + 0.05f) &&
        (cur_target.speed >= target.speed - 0.05f && cur_target.speed <= target.speed + 0.05f) &&
        (cur_target.mag >= target.mag - 3.001f && cur_target.mag <= target.mag + 3.001f))
    {
      cur_target.frame_cnt++;
      cur_target.rcs = target.rcs;
      cur_target.snr = target.snr;
      cur_target.live_percentage = (float)cur_target.frame_cnt / (float)frame_cnt;
      target_cnt_list.replace(i, cur_target);
      index = i;
      goto _UPDATE_TARGET_CNT_INFO;
    }
  }

  target_cnt_list.append(target);

  /* 设置当前表格行数 */
  ui->target_cnt_tableWidget->setRowCount(target_cnt_list.size());

  index = target_cnt_list.size() - 1;
_UPDATE_TARGET_CNT_INFO:

  cur_target = target_cnt_list.value(index);
  /* 目标序号 配置ID 距离 方位角 俯仰角 速度 rcs snr mag 存在率 */
  ui->target_cnt_tableWidget->setItem(index, 0, new QTableWidgetItem(QString::number(cur_target.profile_id)));
  ui->target_cnt_tableWidget->setItem(index, 1, new QTableWidgetItem(QString::number(cur_target.distance)));
  ui->target_cnt_tableWidget->setItem(index, 2, new QTableWidgetItem(QString::number(cur_target.azi_angle)));
  ui->target_cnt_tableWidget->setItem(index, 3, new QTableWidgetItem(QString::number(cur_target.ele_angle)));
  ui->target_cnt_tableWidget->setItem(index, 4, new QTableWidgetItem(QString::number(cur_target.speed)));
  ui->target_cnt_tableWidget->setItem(index, 5, new QTableWidgetItem(QString::number(cur_target.rcs)));
  ui->target_cnt_tableWidget->setItem(index, 6, new QTableWidgetItem(QString::number(cur_target.snr)));
  ui->target_cnt_tableWidget->setItem(index, 7, new QTableWidgetItem(QString::number(cur_target.mag)));
  ui->target_cnt_tableWidget->setItem(index, 8, new QTableWidgetItem(QString::asprintf("%.2f%%", cur_target.live_percentage * 100.f)));
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

//  qDebug() << "profile " << profile_id << "obj_num " << obj_num;

  /* 设置当前表格行数 */
  ui->tableWidget->clearContents();
  ui->tableWidget->setRowCount(obj_num < 64 ? 64 : obj_num);

  /* 刷新数据 */
  quint16 index = 0;
  qint16 speed;
  qint16 azi_angle;
  quint32 distance;
  quint16 mag;
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

    /* 目标序号 配置ID 距离 方位角 俯仰角 速度 rcs snr mag */
    TARGET_CNT_LIST_Typedef_t target;
    target.profile_id = profile_id;
    target.speed = (float)speed * 0.01f;
    target.azi_angle = (float)azi_angle * 0.01f;
    target.distance = (float)distance * 0.01f;
    target.mag = (float)mag * 0.1f;
    target.rcs = (float)rcs * 0.1f;
    target.snr = (float)snr * 0.1f;
    target.ele_angle = (float)ele_angle * 0.01f;

    ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(target.profile_id)));
    ui->tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(target.distance)));
    ui->tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(target.azi_angle)));
    ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(target.ele_angle)));
    ui->tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(target.speed)));
    ui->tableWidget->setItem(i, 7, new QTableWidgetItem(QString::number(target.mag)));
    ui->tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(target.rcs)));
    ui->tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(target.snr)));

    /* 更新统计信息 */
    refresh_obj_cnt_list_info(target);
  }
}

/**
 * @brief 添加阈值到列表
 */
void eol_calibration_window::on_add_pushButton_clicked()
{
  THRESHOLD_SET_INFO_Typedef_t threshold_info;
  threshold_info.profile_id = (quint8)ui->profile_id_comboBox->currentText().toUShort();
  threshold_info.distance_m = (quint8)ui->distance_comboBox->currentText().toUShort();
  threshold_info.mag_dB_threshold_down = (qint8)ui->mag_threshold_min_lineEdit->text().toShort();
  threshold_info.mag_dB_threshold_up = (qint8)ui->mag_threshold_max_lineEdit->text().toShort();
  threshold_info.snr_dB_threshold_down = (qint8)ui->snr_threshold_min_lineEdit->text().toShort();
  threshold_info.snr_dB_threshold_up = (qint8)ui->snr_threshold_max_lineEdit->text().toShort();
  threshold_info.rts_dBsm = (quint8)ui->rts_lineEdit->text().toUShort();

  threshold_info.str = QString::asprintf("id:%u,%um,mag:>%u <%u,snr:>%u <%u,rts:%u\r\n", \
                                         threshold_info.profile_id, \
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

  /* 清除目标统计 */
  target_cnt_list.clear();
  ui->target_cnt_tableWidget->clearContents();

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
    case EOL_RW_TX_WAVE_MODE_REG:
      break;

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
            /* 更新显示 */
            if(ui->profile_id_comboBox->currentIndex() == profile_id)
            {
              ui->cali_mode_comboBox->setCurrentIndex(cali_mode);
            }

            tips_str += QString("<font size='5' color='green'><div align='legt'>profile id[%1] cali_mode:</div> <div align='right'>%2</div> </font>\r\n").arg(profile_id).arg(cali_mode);
          }
          /* 显示Cali Mode信息 */
          QMessageBox message(QMessageBox::Information, tr("Cali Mode Info"), tr(tips_str.toUtf8()), QMessageBox::Yes, nullptr);
          message.exec();
        }
      }
      break;
    default:
      break;
  }
}

/**
 * @brief 从机读写无反应信号
 */
void eol_calibration_window::slot_protocol_rw_err(quint8 reg, quint8 command)
{
  Q_UNUSED(command)
  switch(reg)
  {
    case EOL_RW_TX_WAVE_MODE_REG:
      if(this->isHidden() == false)
      {
        eol_protocol::EOL_TASK_LIST_Typedef_t task;
        task.param = nullptr;

        /* 设置正常工作波形 */
        task.reg = EOL_RW_TX_WAVE_MODE_REG;
        task.command = eol_protocol::EOL_WRITE_CMD;
        task.buf[0] = 1;
        task.len = 1;
        eol_protocol_obj->eol_master_common_rw_device(task);
      }
      else
      {
        eol_protocol::EOL_TASK_LIST_Typedef_t task;
        task.param = nullptr;

        /* 设置校准波形 */
        task.reg = EOL_RW_TX_WAVE_MODE_REG;
        task.command = eol_protocol::EOL_WRITE_CMD;
        task.buf[0] = 0;
        task.len = 1;
        eol_protocol_obj->eol_master_common_rw_device(task);
      }
      break;

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

    case EOL_RW_RCS_OFFSET_REG:
    case EOL_W_PAR_RESET_REG:
    case EOL_RW_CALI_MODE_REG:
      break;
    default:
      break;
  }
}

/**
 * @brief 目标距离更新
 */
void eol_calibration_window::on_distance_comboBox_currentTextChanged(const QString &arg1)
{
  Q_UNUSED(arg1)
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


void eol_calibration_window::on_target_cnt_en_checkBox_stateChanged(int arg1)
{
  ui->target_cnt_tableWidget->setVisible((bool)arg1);
}

void eol_calibration_window::on_export_total_list_pushButton_clicked()
{
  /* 导出统计的目标列表 */
  QString current_file_path = QFileDialog::getSaveFileName(this, tr("Save  "), last_file_path, tr("csv (*.csv)"));
  if(current_file_path.isEmpty() == false)
  {
    /* 获取文件信息 */
    QFileInfo info(current_file_path);

    /* 更新最近路径信息 */
    last_file_path = info.absolutePath();

    utility::export_table2csv_file(ui->target_cnt_tableWidget, current_file_path);
  }
}
