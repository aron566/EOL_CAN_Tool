#include "eol_angle_calibration_window.h"
#include "ui_eol_angle_calibration_window.h"
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QDateTime>

eol_angle_calibration_window::eol_angle_calibration_window(QString title, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::eol_angle_calibration_window)
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

  /* 设置悬浮提示 */
  ui->angle_ele_at_set_lineEdit->setToolTip(tr("this dimension is not used if it is empty"));
  ui->angle_azi_at_set_lineEdit->setToolTip(tr("this dimension is not used if it is empty"));

  /* 初始化定时器 */
  timer_init();
}

eol_angle_calibration_window::~eol_angle_calibration_window()
{
  delete ui;
}

/**
 * @brief 窗口关闭事件
 * @param event
 */
void eol_angle_calibration_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}

/**
 * @brief 定时器初始化
 */
void eol_angle_calibration_window::timer_init()
{
  timer_obj = new QTimer(this);
  timer_obj->setInterval(100);
  connect(timer_obj, &QTimer::timeout, this, &eol_angle_calibration_window::slot_timeout);
}

/**
 * @brief 设置eol协议栈对象
 * @param obj
 */
void eol_angle_calibration_window::set_eol_protocol_obj(eol_protocol *obj)
{
  if(nullptr == obj)
  {
    return;
  }
  eol_protocol_obj = obj;
  /* 数据接收 */
  // connect(eol_protocol_obj, &eol_protocol::signal_rw_device_ok, this, &eol_angle_calibration_window::slot_rw_device_ok);
  connect(eol_protocol_obj, &eol_protocol::signal_protocol_rw_err, this, &eol_angle_calibration_window::slot_protocol_rw_err, Qt::BlockingQueuedConnection);
  connect(eol_protocol_obj, &eol_protocol::signal_rw_device_ok, this, &eol_angle_calibration_window::slot_rw_device_ok, Qt::BlockingQueuedConnection);
}

/**
 * @brief 添加配置
 */
void eol_angle_calibration_window::on_add_config_pushButton_clicked()
{
  /* rts */
  bool rts_cfg_is_valid = true;
  if(ui->azi_rts_rang_lineEdit->text().isEmpty())
  {
    rts_cfg_is_valid = false;
  }
  if(ui->rts_velocity_lineEdit->text().isEmpty())
  {
    rts_cfg_is_valid = false;
  }
  if(true == rts_cfg_is_valid)
  {
    rts_range = ui->azi_rts_rang_lineEdit->text().toFloat();
    rts_velocity = ui->rts_velocity_lineEdit->text().toFloat();
    rts_rcs =  ui->rts_rcs_lineEdit->text().toFloat();
    rts_start_frequency = ui->rts_start_frequency_lineEdit->text().toFloat();
    rts_bandwidth = ui->rts_bandwidth_lineEdit->text().toFloat();
  }

  /* 水平组 */
  bool azi_cfg_is_valid = true;
  if(ui->angle_step_set_lineEdit->text().isEmpty())
  {
    azi_cfg_is_valid = false;
  }
  if(ui->angle_start_set_lineEdit->text().isEmpty())
  {
    azi_cfg_is_valid = false;
  }
  if(ui->angle_end_set_lineEdit->text().isEmpty())
  {
    azi_cfg_is_valid = false;
  }
  if(ui->angle_ele_at_set_lineEdit->text().isEmpty())
  {
    azi_cfg_is_valid = false;
  }
  if(true == azi_cfg_is_valid)
  {
    /* 水平转动 */
    azi_left_angle_start = ui->angle_start_set_lineEdit->text().toFloat();
    azi_right_angle_end = ui->angle_end_set_lineEdit->text().toFloat();
    azi_direction_ele_angle = ui->angle_ele_at_set_lineEdit->text().toFloat();
    azi_step_angle = ui->angle_step_set_lineEdit->text().toFloat();

    /* 配置数 */
    FFT_REQUEST_CONDITION_Typedef_t condition;

    for(int i = 0; i < calibration_profile_info_list.size(); i++)
    {
      condition.profile_id[i] = calibration_profile_info_list.value(i).profile_id;
      condition.channel_num[i] = calibration_profile_info_list.value(i).channel_num;
    }
    condition.profile_num = (quint8)calibration_profile_info_list.size();
    condition.s_angle = azi_left_angle_start;
    condition.e_angle = azi_right_angle_end;
    condition.step_angle = azi_step_angle;

    for(float s = azi_left_angle_start; s <= azi_right_angle_end; s += azi_step_angle)
    {
      condition.current_angle = s;
      condition.azi_ele_angle = azi_direction_ele_angle;
      condition.direction = (quint8)AZI_DIRECTION;
      condition.rts_range = rts_range;
      condition.rts_velocity = rts_velocity;
      fft_request_list.append(condition);
    }
  }

  /* 俯仰组 */
  bool ele_cfg_is_valid = true;
  if(ui->angle_step_ele_set_lineEdit->text().isEmpty())
  {
    ele_cfg_is_valid = false;
  }
  if(ui->angle_start_ele_set_lineEdit->text().isEmpty())
  {
    ele_cfg_is_valid = false;
  }
  if(ui->angle_end_set_lineEdit->text().isEmpty())
  {
    ele_cfg_is_valid = false;
  }
  if(ui->angle_azi_at_set_lineEdit->text().isEmpty())
  {
    ele_cfg_is_valid = false;
  }
  if(true == ele_cfg_is_valid)
  {
    /* 俯仰转动 */
    ele_up_angle_end = ui->angle_end_ele_set_lineEdit->text().toFloat();
    ele_down_angle_start = ui->angle_start_ele_set_lineEdit->text().toFloat();
    ele_direction_azi_angle = ui->angle_azi_at_set_lineEdit->text().toFloat();
    ele_step_angle = ui->angle_step_ele_set_lineEdit->text().toFloat();

    /* 配置数 */
    FFT_REQUEST_CONDITION_Typedef_t condition;

    for(int i = 0; i < calibration_profile_info_list.size(); i++)
    {
      condition.profile_id[i] = calibration_profile_info_list.value(i).profile_id;
      condition.channel_num[i] = calibration_profile_info_list.value(i).channel_num;
    }
    condition.profile_num = (quint8)calibration_profile_info_list.size();
    condition.s_angle = ele_down_angle_start;
    condition.e_angle = ele_up_angle_end;
    condition.step_angle = ele_step_angle;

    for(float s = ele_down_angle_start; s <= ele_up_angle_end; s += ele_step_angle)
    {
      condition.current_angle = s;
      condition.azi_ele_angle = ele_direction_azi_angle;
      condition.direction = (quint8)ELE_DIRECTION;
      condition.rts_range = rts_range;
      condition.rts_velocity = rts_velocity;
      fft_request_list.append(condition);
    }
  }

  /* 清除 */
  ui->tableWidget->clearContents();
  ui->tableWidget->setRowCount(fft_request_list.size());

  /* 刷新数据 */
  quint8 column = 0;
  QString profile_str, channel_num_str;
  FFT_REQUEST_CONDITION_Typedef_t condition;
  for(quint16 i = 0; i < fft_request_list.size(); i++)
  {
    condition = fft_request_list.value(i);
    column = 0;
    /* 维度 TX配置 配置ID列表 配置下通道数 辅助角 rts距离 rts速度 当前角度 获取2D是否成功 */
    if(AZI_DIRECTION == condition.direction)
    {
      ui->tableWidget->setItem(i, column++, new QTableWidgetItem(QString("AZI")));
    }
    if(ELE_DIRECTION == condition.direction)
    {
      ui->tableWidget->setItem(i, column++, new QTableWidgetItem(QString("ELE")));
    }
    /* tx配置 */
    ui->tableWidget->setItem(i, column++, new QTableWidgetItem(QString("")));

    /* 使用的配置id */
    profile_str.clear();
    channel_num_str.clear();
    for(quint8 profile_index = 0; profile_index < condition.profile_num; profile_index++)
    {
      profile_str += QString::number(condition.profile_id[profile_index]);
      channel_num_str += QString::number(condition.channel_num[profile_index]);
      if((profile_index + 1) < condition.profile_num)
      {
        profile_str += ",";
        channel_num_str += ",";
      }
    }
    ui->tableWidget->setItem(i, column++, new QTableWidgetItem(profile_str));

    /* 使用的通道数 */
    ui->tableWidget->setItem(i, column++, new QTableWidgetItem(channel_num_str));

    ui->tableWidget->setItem(i, column++, new QTableWidgetItem(QString::number(condition.azi_ele_angle)));
    ui->tableWidget->setItem(i, column++, new QTableWidgetItem(QString::number(condition.rts_range)));
    ui->tableWidget->setItem(i, column++, new QTableWidgetItem(QString::number(condition.rts_velocity)));
    ui->tableWidget->setItem(i, column++, new QTableWidgetItem(QString::number(condition.current_angle)));
  }
}

/**
 * @brief 清除配置
 */
void eol_angle_calibration_window::on_clear_config_pushButton_clicked()
{
  /* 清除 */
  ui->tableWidget->clearContents();
  ui->tableWidget->setRowCount(0);
  fft_request_list.clear();
}

/**
 * @brief 启动校准
 */
void eol_angle_calibration_window::on_start_pushButton_clicked()
{
  /* 转台配置是否有效 */
  if(0 >= fft_request_list.size())
  {
    return;
  }

  /* 停止 */
  if(tr("stop") == ui->start_pushButton->text())
  {
    eol_protocol_obj->stop_task();
    ui->start_pushButton->setText(tr("start"));
    return;
  }

  ui->start_pushButton->setText(tr("stop"));

  /* 设置转台条件 */
  FFT_REQUEST_CONDITION_Typedef_t condition;
  angle_position_index = 0;
  condition = fft_request_list.value(angle_position_index);

  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = &time_ms_s;

  /* 设置转台后读取2DFFT */
  task.reg = EOL_W_2DFFT_CONDITION_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  task.buf[0] = condition.direction;
  qint8 angle = (qint8)condition.current_angle;
  task.buf[1] = (quint8)angle;
  task.buf[2] = (quint8)condition.rts_range;
  qint8 rts_velocity = (qint8)condition.rts_velocity;
  task.buf[3] = (quint8)rts_velocity;
  task.len = 4;
  eol_protocol_obj->eol_master_common_rw_device(task);

#if 0
  /* 设置转台后读取2DFFT */
  task.reg = EOL_R_2DFFT_DATA_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);
#endif

  /* 启动轮询定时器 */
//  timer_obj->stop();

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}

/**
 * @brief eol_angle_calibration_window::更新2d数据
 * @param data 数据
 * @param size 数据长度
 * @return true 有效数据
 */
bool eol_angle_calibration_window::update_2dfft_result(const quint8 *data, quint16 size)
{
  if(11U > size)
  {
    return false;
  }

  if(0xFF == data[5] && 0xFF == data[6] \
     && 0 == data[7] \
     && 0 == data[8] \
     && 0 == data[9] \
     && 0 == data[10])
  {
    return false;
  }

  /* 记录到达时间 */
  QDateTime dt = QDateTime::currentDateTime();
  time_ms_e = dt.toMSecsSinceEpoch();

  /* 返回<所有配置>下的2dfft数据
   * DATA[0]：数据类型
   * 12：代表*int32位数据格式复数（实部int32、虚部int32每个数据占8Bytes）
   * 数据放大系数n int32
   * DATA[1]：Byte0(最低位)
   * DATA[2]：Byte1
   * DATA[3]：Byte2
   * DATA[4]：Byte3(最高位)
   *
   * DATA[5]：配置ID
   * DATA[6]：通道数（1 - 32~max~）
   * TX发波次序：
   * DATA[7]：TX0的发波次序，0代表未启用
   * DATA[8]：TX1的发波次序
   * DATA[9]：TX2的发波次序
   * DATA[10]：TX3的发波次序
   * 通道0的2DFFT数据，数值放大n倍后，实部虚部顺序排列：
   * 实部：
   * 虚部： */
  quint16 index = 5;
  quint8 profile_id = 0;
  quint8 channel_num = 0;
  quint32 bit_tx_order = 0;
  QStringList profile_fft_list;

  /* 数据类型 */
  fft_data_type = data[0];
  utility::NUM_TYPE_Typedef_t data_type = (utility::NUM_TYPE_Typedef_t)fft_data_type;

  /* 放大系数 */
  memcpy(&fft_data_factor, &data[1], sizeof(fft_data_factor));

  FFT_REQUEST_CONDITION_Typedef_t condition = fft_request_list.value(angle_position_index);
  memset(condition.bit_tx_order, 0, sizeof(condition.bit_tx_order));
  profile_fft_list.clear();

  QStringList bit_tx_order_str;
  QStringList channel_num_str;

  for(quint16 i = 0; i < calibration_profile_info_list.size(); i++)
  {
    /* 过程合法性检查 */
    if(size < (index + 6U))
    {
      return false;
    }

    profile_id = data[index++];
    channel_num = data[index++];
    quint8 tx0 =  data[index++];
    quint8 tx1 =  data[index++];
    quint8 tx2 =  data[index++];
    quint8 tx3 =  data[index++];
    bit_tx_order = tx3;
    bit_tx_order <<= 8;
    bit_tx_order |= tx2;
    bit_tx_order <<= 8;
    bit_tx_order |= tx1;
    bit_tx_order <<= 8;
    bit_tx_order |= tx0;

    /* 过程合法性检查 */
    if(size < (index + channel_num * utility::num_type_to_bytes((data_type))))
    {
      return false;
    }

    /* 更新此配置下的tx order */
    condition.bit_tx_order[i] = bit_tx_order;

    /* 维度 TX配置x 配置ID 通道数x 辅助角 rts距离 rts速度 当前角度 获取2D是否成功 */
    bit_tx_order_str.append(QString::asprintf("%08X", bit_tx_order));
    channel_num_str.append(QString::number(channel_num));

    profile_fft_list.append(QString("profile:%1").arg(profile_id));
    profile_fft_list.append(QString("channel_num:%1").arg(channel_num));
    /* 添加配置下所有通道fft */
    for(quint8 ch = 0; ch < channel_num; ch++)
    {     
      QString real = utility::data2str(data + index, data_type);
      index += (utility::num_type_to_bytes(data_type) / 2U);
      profile_fft_list.append(real);

      QString image = utility::data2str(data + index, data_type);
      index += (utility::num_type_to_bytes(data_type) / 2U);
      profile_fft_list.append(image);

      /* 存储该配置下所有通道2dfft数据 */
      condition.fft_data[i][ch].real = real.toFloat();
      condition.fft_data[i][ch].image = image.toFloat();
    }
    qint64 time_ms = time_ms_e - time_ms_s;
    QString str_time = QString("ok,%1ms").arg(time_ms);
    ui->tableWidget->setItem(angle_position_index, 8, new QTableWidgetItem(str_time));
    QTableWidgetItem *item = ui->tableWidget->item(angle_position_index, 8);
    if(100 < time_ms)
    {
      item->setForeground(QBrush(Qt::red));
    }
    else
    {
      item->setForeground(QBrush(Qt::white));
    }
  }

  QString str = bit_tx_order_str.join(",");
  ui->tableWidget->setItem(angle_position_index, 1, new QTableWidgetItem(str));
  str = channel_num_str.join(",");
  ui->tableWidget->setItem(angle_position_index, 3, new QTableWidgetItem(str));

  /* 更新结果到队列 */
  fft_request_list.replace(angle_position_index, condition);
  return true;
}

/**
 * @brief 获取特定配置下的通道数
 */
quint8 eol_angle_calibration_window::get_profile_channel_num(quint8 profile_id)
{
  /* 配置下的通道数 */
  for(quint16 i = 0; i < calibration_profile_info_list.size(); i++)
  {
    if(profile_id == calibration_profile_info_list.value(i).profile_id)
    {
      return calibration_profile_info_list.value(i).channel_num;
    }
  }
  return 0;
}

/**
 * @brief 导出2dfft到csv文件
 */
void eol_angle_calibration_window::export_2dfft_csv_file()
{
  /*
  table type,version,data type,assigned angle,start angle,end angle,angle step,
  scale factor,profile0 channel,profile1 channel,profile2 channel,
  profile3 channel,tx order
  */
  if(false == ui->auto_export_csv_checkBox->isChecked())
  {
    return;
  }

  /* 设置导出路径 */
  /* 选择文件存储区域 */
  /* 参数：父对象，标题，默认路径，格式 */
  QString path = QFileDialog::getSaveFileName(this, tr("Save  "), "../", tr("csv (*.csv)"));
  if(path.isEmpty() == true)
  {
    return;
  }

  /* 先关闭 */
  if(fft_csv_file.isOpen() == true)
  {
    fft_csv_file.close();
  }

  /* 关联文件名 */
  fft_csv_file.setFileName(path);

  /* 打开文件，只写方式 */
  if(fft_csv_file.open(QIODevice::WriteOnly) == false)
  {
    return;
  }

  QString csv_header = tr("table type,"
                          "version,"
                          "data type,"
                          "assigned angle,"
                          "start angle,"
                          "end angle,"
                          "angle step,"
                          "scale factor,"
                          "profile0 channel,"
                          "profile1 channel,"
                          "profile2 channel,"
                          "profile3 channel,"
                          "tx order0,"
                          "tx order1,"
                          "tx order2,"
                          "tx order3"
                          "\r\n");

  /* 表头说明写入 */
  QString str;

  /* 转台维度 */
  quint8 last_direction = 0xFF;

  /* 转台维度 */
  float last_azi_ele_angle = 361.f;

  /* 轮询列表 */
  FFT_REQUEST_CONDITION_Typedef_t condition;
  for(int index = 0; index < fft_request_list.size(); )
  {
    condition = fft_request_list.value(index);
    /* 新的维度建立新的csv表 */
    if(last_direction != condition.direction || last_azi_ele_angle != condition.azi_ele_angle)
    {
      /* 表标题写入文件 */
      fft_csv_file.write(csv_header.toUtf8());

      /* 写入标题内容 */
      str.clear();
      str = QString::asprintf("%d,%u,%u,%.1f,%.1f,%.f,%.f,%d,%u,%u,%u,%u,%u,%u,%u,%u\r\n",
                    (int)condition.direction, \
                    fft_data_version, \
                    fft_data_type, \
                    condition.azi_ele_angle, \
                    condition.s_angle, \
                    condition.e_angle, \
                    condition.step_angle, \
                    fft_data_factor, \
                    get_profile_channel_num(0), \
                    get_profile_channel_num(1), \
                    get_profile_channel_num(2), \
                    get_profile_channel_num(3), \
                    condition.bit_tx_order[0], \
                    condition.bit_tx_order[1], \
                    condition.bit_tx_order[2], \
                    condition.bit_tx_order[3]);
      fft_csv_file.write(str.toUtf8());
      last_direction = condition.direction;
      last_azi_ele_angle = condition.azi_ele_angle;
    }
    /* 写入csv表数据 */
    else
    {
      str.clear();
      QStringList data;
      /* 当前角度，所有通道fft数据 */
      data.append(QString::asprintf("%d", (int)condition.current_angle));

      /* 配置下所有通道fft */
      for(quint16 i = 0; i < calibration_profile_info_list.size(); i++)
      {
        for(quint8 ch = 0; ch < calibration_profile_info_list.value(i).channel_num; ch++)
        {
          data.append(QString::asprintf("%d", condition.fft_data[i][ch].real));
          data.append(QString::asprintf("%d", condition.fft_data[i][ch].image));
        }
      }
      str = data.join(",");
      str += "\r\n";
      fft_csv_file.write(str.toUtf8());
      index++;
    }
  }

  fft_csv_file.close();

  /* 原始表先关闭 */
  if(fft_csv_file_origin.isOpen() == true)
  {
    fft_csv_file_origin.close();
  }

  /* 获取文件信息 */
  QFileInfo info(path);
  path = info.absolutePath() + "/" + info.baseName() + tr("_origin.") + tr("csv");

  /* 原始表关联文件名 */
  fft_csv_file_origin.setFileName(path);

  /* 原始表打开文件，只写方式 */
  if(fft_csv_file_origin.open(QIODevice::WriteOnly) == false)
  {
    return;
  }

  last_direction = 0xFF;

  /* 轮询列表 */
  for(int index = 0; index < fft_request_list.size(); )
  {
    condition = fft_request_list.value(index);
    /* 新的维度建立新的csv表 */
    if(last_direction != condition.direction)
    {
      /* 表标题写入文件 */
      fft_csv_file_origin.write(csv_header.toUtf8());

      /* 写入标题内容 */
      str.clear();
      str = QString::asprintf("%d,%u,%u,%.1f,%.1f,%.f,%.f,%d,%u,%u,%u,%u,%u,%u,%u,%u\r\n",
                    (int)condition.direction,
                    fft_data_version,
                    fft_data_type,
                    condition.azi_ele_angle,
                    condition.s_angle,
                    condition.e_angle,
                    condition.step_angle,
                    1,
                    get_profile_channel_num(0),
                    get_profile_channel_num(1),
                    get_profile_channel_num(2),
                    get_profile_channel_num(3),
                    condition.bit_tx_order[0],
                    condition.bit_tx_order[1],
                    condition.bit_tx_order[2],
                    condition.bit_tx_order[3]);
      fft_csv_file_origin.write(str.toUtf8());
      last_direction = condition.direction;
    }
    /* 写入csv表数据 */
    else
    {
      str.clear();
      QStringList data;
      /* 当前角度，所有通道fft数据 */
      data.append(QString::asprintf("%d", (int)condition.current_angle));

      /* 配置下所有通道fft */
      for(quint16 i = 0; i < calibration_profile_info_list.size(); i++)
      {
        for(quint8 ch = 0; ch < calibration_profile_info_list.value(i).channel_num; ch++)
        {
          data.append(QString::asprintf("%f", (float)condition.fft_data[i][ch].real / (float)fft_data_factor));
          data.append(QString::asprintf("%f", (float)condition.fft_data[i][ch].image / (float)fft_data_factor));
        }
      }
      str = data.join(",");
      str += "\r\n";
      fft_csv_file_origin.write(str.toUtf8());
      index++;
    }
  }
  fft_csv_file_origin.close();
}

/**
 * @brief 定时器
 */
void eol_angle_calibration_window::slot_timeout()
{
  if(nullptr == eol_protocol_obj)
  {
    return;
  }
}

/**
 * @brief 数据接收
 */
void eol_angle_calibration_window::slot_rw_device_ok(quint8 reg_addr, const quint8 *data, quint16 data_size)
{
  switch(reg_addr)
  {
    case EOL_W_SAVE_PAR_REG:
      break;

    case EOL_RW_PROFILE_ID_REG:
      break;

    /* 角度校准 */
    case EOL_W_2DFFT_CONDITION_REG:
      {
#if 1
        eol_protocol::EOL_TASK_LIST_Typedef_t task;
        task.param = nullptr;

        /* 设置转台后读取2DFFT */
        task.reg = EOL_R_2DFFT_DATA_REG;
        task.command = eol_protocol::EOL_READ_CMD;
        task.buf[0] = 0;
        task.len = 0;
        eol_protocol_obj->eol_master_common_rw_device(task);

        /* 启动eol线程 */
        eol_protocol_obj->start_task();
#endif
      }
      break;

    case EOL_R_2DFFT_DATA_REG:
      {
        /* 刷新获取结果 */
        if(false == update_2dfft_result(data, data_size))
        {
          /* 继续读取 */
          eol_protocol::EOL_TASK_LIST_Typedef_t task;
          task.param = nullptr;

          /* 设置转台后读取2DFFT */
          task.reg = EOL_R_2DFFT_DATA_REG;
          task.command = eol_protocol::EOL_READ_CMD;
          task.buf[0] = 0;
          task.len = 0;
          eol_protocol_obj->eol_master_common_rw_device(task);

          /* 启动eol线程 */
          eol_protocol_obj->start_task();
          return;
        }

        /* 测量下一个角度 */
        angle_position_index++;

        /* 是否校准完毕 */
        if(angle_position_index >= fft_request_list.size())
        {
          on_start_pushButton_clicked();
          /* 生成csv */
          export_2dfft_csv_file();
          return;
        }

        /* 设置转台条件 */
        FFT_REQUEST_CONDITION_Typedef_t condition;
        condition = fft_request_list.value(angle_position_index);

        eol_protocol::EOL_TASK_LIST_Typedef_t task;
        task.param = &time_ms_s;

        /* 设置转台条件 */
        task.reg = EOL_W_2DFFT_CONDITION_REG;
        task.command = eol_protocol::EOL_WRITE_CMD;
        task.buf[0] = condition.direction;
        qint8 angle = (qint8)condition.current_angle;
        task.buf[1] = (quint8)angle;
        task.buf[2] = (quint8)condition.rts_range;
        qint8 rts_velocity = (qint8)condition.rts_velocity;
        task.buf[3] = (quint8)rts_velocity;
        task.len = 4;
        eol_protocol_obj->eol_master_common_rw_device(task);
#if 0
        /* 设置转台后读取2DFFT */
        task.reg = EOL_R_2DFFT_DATA_REG;
        task.command = eol_protocol::EOL_READ_CMD;
        task.buf[0] = 0;
        task.len = 0;
        eol_protocol_obj->eol_master_common_rw_device(task);
#endif

        /* 启动eol线程 */
        eol_protocol_obj->start_task();
      }
      break;

    case EOL_RW_RCS_OFFSET_REG:
    case EOL_W_PAR_RESET_REG:
    case EOL_RW_CALI_MODE_REG:
      break;
  }
}

/**
 * @brief 从机读写无反应信号
 */
void eol_angle_calibration_window::slot_protocol_rw_err(quint8 reg, quint8 command)
{
  Q_UNUSED(command)
  switch(reg)
  {
    case EOL_W_SAVE_PAR_REG:
      break;
    /* rcs校准，目标测试 */
    case EOL_R_OBJ_LIST_REG:
      break;

    case EOL_RW_PROFILE_ID_REG:

    /* 角度校准 */
    case EOL_W_2DFFT_CONDITION_REG:
    case EOL_R_2DFFT_DATA_REG:
      /* 停止转台 */
      on_start_pushButton_clicked();
      break;

    case EOL_RW_RCS_OFFSET_REG:
    case EOL_W_PAR_RESET_REG:
    case EOL_RW_CALI_MODE_REG:
      break;
  }
}
