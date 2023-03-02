#include "eol_window.h"
#include "qtimer.h"
#include "ui_eol_window.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include "utility.h"

eol_window::eol_window(QString titile, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::eol_window)
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
  this->setWindowTitle(titile);

  /* 初始化定时器 */
  timer_init();

  /* 设置不可添加文件 */
  ui->add_list_pushButton->setEnabled(false);
  ui->upload_pushButton->setEnabled(false);

  /* 初始化状态 */
  reset_ui_info();

  connect(this, &eol_window::signal_update_show_table_list, this, &eol_window::slot_update_show_table_list);
}

eol_window::~eol_window()
{
  /* 停止协议栈 */
  eol_protocol_obj->stop();

  run_state = false;

  /* 等待线程结束 */
  while(thread_run_state)
  {
    utility::delay_ms(1);
  }

  delete ui;
}

void eol_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  /* 终止并清空传输任务 */
  run_state = false;

  /* 停止协议栈 */
  eol_protocol_obj->stop();

  this->hide();
  emit signal_eol_window_closed();
}

void eol_window::timer_init()
{
  timer_obj = new QTimer(this);
  connect(timer_obj, &QTimer::timeout, this, &eol_window::slot_timeout);
  timer_obj->setInterval(1000);
}

void eol_window::slot_timeout()
{
  time_cnt++;
  ui->time_cnt_val_label->setNum((int)time_cnt);
}

void eol_window::slot_update_show_table_list()
{
  update_show_table_list();
}

void eol_window::reset_ui_info()
{
  timer_obj->stop();

  reset_base_ui_info();

  if(table_list.size() > 0)
  {
    ui->update_pushButton->setEnabled(true);
  }
  else
  {
    ui->update_pushButton->setEnabled(false);
  }
  ui->upload_pushButton->setEnabled(true);
}

void eol_window::update_show_table_list()
{
  ui->transfer_list_val_label->clear();
  QString show_info;
  show_info.clear();
  for(qint32 i = 0; i < table_list.size(); i++)
  {
    show_info += table_list.value(i).show_info;
  }
  ui->transfer_list_val_label->setText(show_info);
}

void eol_window::csv_data_analysis(QByteArray &data, quint64 line_num, int table_type_index, int data_type)
{
  num_str_list = QString(data).split(",");
  if(num_str_list.isEmpty())
  {
    return;
  }
  switch((eol_protocol::DOA_TABLE_Typedef_t)table_type_index)
  {
    /* 方位导向矢量表数据传输 */
    case eol_protocol::DOA_SV_AZIMUTH_TABLE:
    {
      quint8 unit_byets;
      quint32 num = utility::str2num(&num_buf[sv_table_info.data_size], num_str_list, (utility::NUM_TYPE_Typedef_t)data_type, &unit_byets);
      sv_table_info.data_type = data_type;
      sv_table_info.data_size += (num * unit_byets);
      break;
    }
    /* 俯仰导向矢量表数据传输 */
    case eol_protocol::DOA_SV_ELEVATION_TABLE:
    {
      quint8 unit_byets;
      quint32 num = utility::str2num(&num_buf[sv_table_info.data_size], num_str_list, (utility::NUM_TYPE_Typedef_t)data_type, &unit_byets);
      sv_table_info.data_type = data_type;
      sv_table_info.data_size += (num * unit_byets);
      break;
    }
    /* 天线间距坐标信息表 */
    /* 天线初相信息表 */
    /* 天线间距坐标与初相信息表，双表合并 */
//    case eol_protocol::DAA_ANT_POS_TABLE:
//    case eol_protocol::DOA_PHASE_COMPS_TABLE:
    case eol_protocol::DOA_ANT_BOTH_TABLE:
    {
      quint8 unit_byets;
      quint32 num = utility::str2num(&num_buf[ant_table_info.data_size], num_str_list, (utility::NUM_TYPE_Typedef_t)data_type, &unit_byets);
      /* 天线间距 */
      if(1 == line_num)
      {
        ant_table_info.table_index = (int)eol_protocol::DOA_ANT_BOTH_TABLE;
        ant_table_info.data_type = data_type;
        ant_table_info.channel_num = num / 2;
        ant_table_info.data_size += (num * unit_byets);
        qDebug() << "channel_num " << ant_table_info.channel_num;
        qDebug() << "ant_table_info.data_size " << ant_table_info.data_size;
      }

      /* 通道补偿 */
      if(2 <= line_num)
      {
        ant_table_info.data_size += (ant_table_info.channel_num * unit_byets);
        qDebug() << "ant_table_info.data_size " << ant_table_info.data_size;
      }

      break;
    }
    default:
      break;
  }
}

void eol_window::csv_header_analysis(QByteArray &data, int table_type_index)
{
  num_str_list = QString(data).split(",");
  if(num_str_list.isEmpty())
  {
    return;
  }
  switch((eol_protocol::DOA_TABLE_Typedef_t)table_type_index)
  {
    /* 方位导向矢量表 */
    case eol_protocol::DOA_SV_AZIMUTH_TABLE:
    {
      /* 候选角度320,通道16,-80,80 */
      qDebug() << num_str_list.mid(0, 4);
      if(num_str_list.size() < 4)
      {
        return;
      }
      sv_table_info.table_index = table_type_index;
      sv_table_info.angle_points = num_str_list.value(0).toUShort();
      sv_table_info.channel_num = (quint8)num_str_list.value(1).toUShort();
      break;
    }
    /* 俯仰导向矢量表 */
    case eol_protocol::DOA_SV_ELEVATION_TABLE:
    {
      /* 候选角度41,通道4,-20,20 */
      qDebug() << num_str_list.mid(0, 4);
      if(num_str_list.size() < 4)
      {
        return;
      }
      sv_table_info.table_index = table_type_index;
      sv_table_info.angle_points = num_str_list.value(0).toUShort();
      sv_table_info.channel_num = (quint8)num_str_list.value(1).toUShort();
      break;
    }
    default:
      break;
  }
}

/* 解析csv文件 */
bool eol_window::csv_analysis(QString &file_path, int table_type_index, int data_type)
{
  if(file_path.isEmpty())
  {
    return false;
  }

  /* 选择文件前先关闭 */
  if(current_file.isOpen() == true)
  {
    current_file.close();
  }
  current_file.setFileName(file_path);
  bool is_ok = current_file.open(QIODevice::ReadOnly);
  if(is_ok == false)
  {
    qDebug() << "只读打开文件失败 78";
    return false;
  }

  /* 清空记录表 */
  memset(&sv_table_info, 0, sizeof(sv_table_info));
  memset(&ant_table_info, 0, sizeof(ant_table_info));
  memset(num_buf, 0, sizeof(num_buf));

  quint64 size = current_file.size();
  quint64 line_num = 0;
  /* 读取每一行 */
  for(quint64 i = 0; i < size;)
  {
    QByteArray line_data = current_file.readLine();
    i += line_data.size();
    if(line_data.size() > 0)
    {
      line_num++;
    }

    /* 解析头部 */
    if((0 == table_type_index || 1 == table_type_index) && \
       1 == line_num)
    {
      csv_header_analysis(line_data, table_type_index);
      continue;
    }

    /* 解析数据 */
    csv_data_analysis(line_data, line_num, table_type_index, data_type);
  }

  /* 启动协议栈发送数据 */
  eol_protocol::DOA_TABLE_HEADER_Typedef_t table_info;
  if((0 == table_type_index || 1 == table_type_index))
  {
    table_info.Table_Type = (eol_protocol::DOA_TABLE_Typedef_t)sv_table_info.table_index;
    table_info.Data_Size = sv_table_info.data_size;
  }
  else
  {
    table_info.Table_Type = (eol_protocol::DOA_TABLE_Typedef_t)ant_table_info.table_index;
    table_info.Data_Size = ant_table_info.data_size;
  }
  table_info.Data_Type = (eol_protocol::DOA_DATA_Typedef_t)data_type;
  table_info.Class_ID_Num = 0x0566U;
  table_info.Version_MAJOR = 0;
  table_info.Version_MINOR = 0;
  table_info.Version_REVISION = 1;

  /* 添加到eol发送数据任务 */
  bool ret = eol_protocol_obj->eol_master_send_table_data(table_info, (const quint8 *)num_buf);
  if(false == ret)
  {
    qDebug() << "add task error";
    current_task_complete_state = TASK_ERROR;
    return false;
  }

  /* 启动发送数据线程 */
  g_thread_pool->start(eol_protocol_obj);

  /* 设置当前任务状态 */
  current_task_complete_state = TASK_RUNNING;
  return true;
}

/**
 * @brief eol_window::执行eol文件解析任务
 */
void eol_window::run_eol_window_file_decode_task()
{
  QString str;
  /* 列表循环 */
  for(int i = 0; i < table_list.size(); i++)
  {
    TABLE_INFO_Typedef_t table = table_list.value(i);

    /* 移除上一次的状态 */
    str = table_list.value(i).show_info.replace(" -- [ok]", "");
    str = table_list.value(i).show_info.replace(" -- [err]", "");
    table.show_info = str;
    table_list.replace(i, table);

    /* 更新显示列表 */
    emit signal_update_show_table_list();

    /* 设置传输的文件 */
    current_file_path = table_list.value(i).file_path;

    csv_analysis(current_file_path, table_list.value(i).table_type, table_list.value(i).data_type);

    while(current_task_complete_state == TASK_RUNNING)
    {
      QThread::msleep(1);
    }

    if(TASK_COMPLETE == current_task_complete_state)
    {
      /* 更新为ok状态 */
      str = table_list.value(i).show_info.replace("\r\n", " -- [ok]\r\n");
    }
    else
    {
      /* 更新为error状态 */
      str = table_list.value(i).show_info.replace("\r\n", " -- [err]\r\n");
    }

    table = table_list.value(i);
    table.show_info = str;
    table_list.replace(i, table);

    /* 更新显示列表 */
    emit signal_update_show_table_list();
  }

  /* 任务结束 */
  run_state = false;
}

void eol_window::on_file_sel_pushButton_clicked()
{
  /* 选择文件前先关闭 */
  if(current_file.isOpen() == true)
  {
    current_file.close();
  }

  /* 选择文件 */
  if(last_file_path.isEmpty())
  {
    last_file_path = "../";
  }
  current_file_path = QFileDialog::getOpenFileName(this, tr("Open File"), last_file_path, tr("csv (*.csv);;BIN (*.bin)"));
  if(current_file_path.isEmpty() == false)
  {
    current_file_name.clear();
    current_filesize = 0;
    /* 获取文件信息 */
    QFileInfo info(current_file_path);
    current_file_name = info.fileName();
    current_filesize = info.size();

    /* 更新最近路径信息 */
    last_file_path = info.absolutePath();

    /* 检查非法空格字段 */
    current_file_name.replace(QChar(' '), QChar('_'));

    /* 设置待写入表信息 */
    if(table_list.size() == 0)
    {
      /* 设置文件名 */
      current_file.setFileName(current_file_path);
    }

    /* 选择框显示更新 */
    ui->file_name_lineEdit->setText(current_file_name);
    ui->file_size_lineEdit->setText(QString("%1").arg((float)current_filesize / 1024.f));

    /* 允许点击更新 */
    ui->update_pushButton->setEnabled(true);
  }
  else
  {
    qDebug() << "打开文件错误 62";
  }
}


void eol_window::on_upload_pushButton_clicked()
{
  eol_protocol::DOA_TABLE_Typedef_t table_type = (eol_protocol::DOA_TABLE_Typedef_t)ui->table_type_comboBox->currentIndex();
  eol_protocol_obj->eol_master_get_table_data(table_type);

  /* 重置界面 */
  reset_base_ui_info();

  /* 重置错误统计 */
  err_constantly_cnt = 0;

  /* 重置计时 */
  time_cnt = 0;

  /* 启动计时 */
  timer_obj->start();

  /* 启动接收数据线程 */
  g_thread_pool->start(eol_protocol_obj);

  /* 本按钮不可用 */
  ui->upload_pushButton->setEnabled(false);
  ui->add_list_pushButton->setEnabled(false);
}


void eol_window::on_update_pushButton_clicked()
{
  /* 传输列表检测 */
  if(table_list.isEmpty())
  {
    return;
  }

  /* 重置错误统计 */
  err_constantly_cnt = 0;

  /* 重置计时 */
  time_cnt = 0;

  /* 重置界面 */
  reset_base_ui_info();

  /* 启动计时 */
  timer_obj->start();

  /* 启动传输 */
  run_state = true;

  /* 启动任务线程 */
  g_thread_pool->start(this);

  /* 本按钮不可用 */
  ui->update_pushButton->setEnabled(false);
  ui->add_list_pushButton->setEnabled(false);
}

void eol_window::on_add_list_pushButton_clicked()
{
  if(current_file_path.isEmpty())
  {
    return;
  }

  TABLE_INFO_Typedef_t table;
  /* 查重 */
  for(qint32 i = 0; i < table_list.size(); i++)
  {
    if(current_file_path == table_list.value(i).file_path)
    {
      return;
    }
  }

  /* 显示到列表区域 */
  table.table_type = ui->table_type_comboBox->currentIndex();
  table.data_type = ui->data_type_comboBox->currentIndex();
  table.file_name = current_file_name;
  table.file_path = current_file_path;
  table.show_info = QString("%1 -- %2 -- [wait]\r\n").arg(table_list.size() + 1).arg(current_file_name);
  table_list.append(table);

  /* 更新显示列表 */
  update_show_table_list();

  /* 发送按钮可用 */
  ui->update_pushButton->setEnabled(true);
}


void eol_window::on_clear_list_pushButton_clicked()
{
  /* 清空传输列表 */
  table_list.clear();
  ui->transfer_list_val_label->clear();
  reset_ui_info();
}

/**
 * @brief 从机无应答超时时间
 */
void eol_window::slot_protocol_timeout(quint32 sec)
{
  /* 错误累计 */
  quint32 cnt = ui->error_num_val_label->text().toUInt();
  cnt++;
  ui->error_num_val_label->setText(QString("%1").arg(cnt));
}

/**
 * @brief 从机无应答信号
 */
void eol_window::slot_protocol_no_response()
{
  /* 连续错误统计 */
  err_constantly_cnt++;
  if(err_constantly_cnt > 0)
  {
    /* 重置错误统计 */
    err_constantly_cnt = 0;

    /* 停止协议栈 */
    eol_protocol_obj->stop();

    timer_obj->stop();

    /* 设置错误状态 */
    current_task_complete_state = TASK_ERROR;

    /* 本按钮可用 */
    if(table_list.size() > 0)
    {
      ui->update_pushButton->setEnabled(true);
    }
    else
    {
      ui->update_pushButton->setEnabled(false);
    }
    ui->upload_pushButton->setEnabled(true);
    ui->add_list_pushButton->setEnabled(true);
  }
}

void eol_window::slot_protocol_error_occur(quint8 error_msg)
{
  /* 显示错误消息 */
  qDebug() << "ack msg:" << error_msg;
}

/**
 * @brief 表数据
 * @param frame_num 帧号
 * @param data 数据
 * @param data_len 数据长度
 */
void eol_window::slot_recv_eol_table_data(quint16 frame_num, const quint8 *data, quint16
                                 data_len)
{
  /* 重置错误统计 */
  err_constantly_cnt = 0;

  /* 0帧为表信息数据 */
  if(0 == frame_num)
  {
    table_info.Table_Type = (eol_protocol::DOA_TABLE_Typedef_t)data[2];
    table_info.Version_MAJOR = data[3] & 0x0F;
    table_info.Version_MINOR = (quint8)(data[3] >> 4);
    table_info.Version_REVISION = data[4];
    table_info.Data_Type = (eol_protocol::DOA_DATA_Typedef_t)data[5];
    table_info.Data_Size = data[7];
    table_info.Data_Size <<= 8;
    table_info.Data_Size |= data[6];
    memcpy(&table_info.Crc_Val, data + 8, 4);
    ui->transfer_progressBar->setMaximum(table_info.Data_Size);
    /* 显示接收完成 */
    QString tips_str = QString("<font size='5' color='green'><div align='legt'> Table Type: </div> <div align='right'> %1 </div> </font>\n"
                               "<font size='5' color='orange'><div align='legt'> Ver: </div> <div align='right'> %2.%3.%4 </div> </font>\n"
                               "<font size='5' color='blue'><div align='legt'> Data Type: </div> <div align='right'> %5 </div> </font>\n"
                               "<font size='5' color='blue'><div align='legt'> Data Size: </div> <div align='right'> %6 </div> </font>\n")
                                .arg(table_info.Data_Type)
                                .arg(table_info.Version_MAJOR)
                                .arg(table_info.Version_MINOR)
                                .arg(table_info.Version_REVISION)
                                .arg(table_info.Data_Type)
                                .arg(table_info.Data_Size);
    QMessageBox message(QMessageBox::Information, tr("Table Info"), tr(tips_str.toUtf8()), QMessageBox::Yes, nullptr);
    message.exec();
    /* 选择文件存储区域 */
    /* 参数：父对象，标题，默认路径，格式 */
    QString path = QFileDialog::getSaveFileName(this, tr("Save  "), "../", tr("BIN(*.bin)"));
    if(path.isEmpty() == true)
    {
      run_state = false;
      return;
    }

    /* 先关闭 */
    if(recv_file.isOpen() == true)
    {
      recv_file.close();
    }

    /* 关联文件名 */
    recv_file.setFileName(path);

    /* 打开文件，只写方式 */
    recv_file.open(QIODevice::WriteOnly);
    return;
  }

  if(0 < frame_num && 0xFFFF > frame_num)
  {
    /* 表大小，每帧4字节，显示进度 */
    quint32 size = 4 * frame_num;
    if(recv_file.isOpen() == false)
    {
      return;
    }
    qDebug() << "get pack num: " << frame_num;
    recv_file.seek((frame_num - 1) * 4);
    recv_file.write((const char *)data, 4);

    ui->transfer_progressBar->setValue(size > table_info.Data_Size ? table_info.Data_Size : size);
    ui->bytes_lcdNumber->display((int)size);
  }
  else
  {
    /* 关闭文件 */
    recv_file.close();

    /* 停止运行 */
    run_state = false;
    QMessageBox message(QMessageBox::Information, tr(" Info "), tr("<font size='10' color='green'>上载完成！</font>"), QMessageBox::Yes, nullptr);
    message.exec();

    /* 本按钮可用 */
    if(table_list.size() > 0)
    {
      ui->update_pushButton->setEnabled(true);
    }
    else
    {
      ui->update_pushButton->setEnabled(false);
    }
    ui->upload_pushButton->setEnabled(true);
    ui->add_list_pushButton->setEnabled(true);
  }
}

void eol_window::slot_send_progress(quint32 current_size, quint32 total_size)
{
  /* 重置错误统计 */
  err_constantly_cnt = 0;

  /* 显示发送进度 */
  ui->transfer_progressBar->setMaximum((int)total_size);

  ui->transfer_progressBar->setValue(current_size > total_size ? total_size : current_size);
  ui->bytes_lcdNumber->display((int)current_size);
}

/**
 * @brief 接收发送完成信号
 */
void eol_window::slot_send_eol_data_complete()
{
  /* 设置完成状态 */
  current_task_complete_state = TASK_COMPLETE;
  /* 本按钮可用 */
  if(table_list.size() > 0)
  {
    ui->update_pushButton->setEnabled(true);
  }
  else
  {
    ui->update_pushButton->setEnabled(false);
  }
  ui->add_list_pushButton->setEnabled(true);

  /* 停止计时 */
  timer_obj->stop();
}

/**
 * @brief 接收数据完成
 */
void eol_window::slot_recv_eol_data_complete()
{
  /* 停止计时 */
  timer_obj->stop();

  /* 设置完成状态 */
  current_task_complete_state = TASK_COMPLETE;

  /* 显示接收完成 */
  QMessageBox message(QMessageBox::Information, "通知", "<font size='10' color='green'>上载传输完成！</font>", QMessageBox::Yes, nullptr);
  message.exec();

  /* 本按钮可用 */
  if(table_list.size() > 0)
  {
    ui->update_pushButton->setEnabled(true);
  }
  else
  {
    ui->update_pushButton->setEnabled(false);
  }
  ui->upload_pushButton->setEnabled(true);
  ui->add_list_pushButton->setEnabled(true);
}

void eol_window::on_entry_produce_mode_pushButton_clicked()
{
  /* 是否时正在执行任务 */
  if(TASK_RUNNING == current_task_complete_state)
  {
    return;
  }
  bool ret = false;
  if(ui->entry_produce_mode_pushButton->text() == "entry produce mode")
  {
    if(ui->produce_noral_radioButton->isChecked())
    {
      ret = eol_protocol_obj->eol_master_set_device_mode(eol_protocol::PRODUCE_MODE_NORMAL);
    }
    else
    {
      ret = eol_protocol_obj->eol_master_set_device_mode(eol_protocol::PRODUCE_MODE_CALIBRATION);
    }
    goto __TASK_STATE_CHANGE;
  }
  if(ui->entry_produce_mode_pushButton->text() == "exit produce mode")
  {
    ret = eol_protocol_obj->eol_master_set_device_mode(eol_protocol::NORMAL_MODE_RUN);
    goto __TASK_STATE_CHANGE;
  }
  return;
__TASK_STATE_CHANGE:
  if(true == ret)
  {
    current_task_complete_state = TASK_RUNNING;
    g_thread_pool->start(eol_protocol_obj);
  }
  else
  {
    current_task_complete_state = TASK_ERROR;
  }
}

void eol_window::slot_device_mode(const void *pass_data)
{
  const quint8 *data_ptr = (const quint8 *)pass_data;
  eol_protocol::DEVICE_MODE_Typedef_t mode = (eol_protocol::DEVICE_MODE_Typedef_t)data_ptr[0];
  if(ui->produce_noral_radioButton->isChecked() && eol_protocol::PRODUCE_MODE_NORMAL == mode)
  {
    ui->add_list_pushButton->setEnabled(true);
    ui->upload_pushButton->setEnabled(true);
    if(table_list.size() > 0)
    {
      ui->update_pushButton->setEnabled(true);
    }
    else
    {
      ui->update_pushButton->setEnabled(false);
    }
    ui->entry_produce_mode_pushButton->setText("exit produce mode");
  }
  else if(ui->produce_calibratio_radioButton->isChecked() && eol_protocol::PRODUCE_MODE_CALIBRATION == mode)
  {
    ui->add_list_pushButton->setEnabled(true);
    ui->upload_pushButton->setEnabled(true);
    if(table_list.size() > 0)
    {
      ui->update_pushButton->setEnabled(true);
    }
    else
    {
      ui->update_pushButton->setEnabled(false);
    }
    ui->entry_produce_mode_pushButton->setText("exit produce mode");
  }
  else if(eol_protocol::NORMAL_MODE_RUN == mode)
  {
    ui->add_list_pushButton->setEnabled(false);
    ui->upload_pushButton->setEnabled(false);
    ui->update_pushButton->setEnabled(false);
    ui->entry_produce_mode_pushButton->setText("entry produce mode");
  }
  current_task_complete_state = TASK_COMPLETE;
  /* 显示设备信息 */
  QMessageBox message(QMessageBox::Information, "device info", \
                      tr("<font size='10' color='green'>profile amount:%1</font>\r\n").arg(data_ptr[1]) +
                      tr("<font size='10' color='green'>channel amount:%1</font>\r\n").arg(data_ptr[2]), \
                      QMessageBox::Yes, nullptr);
  message.exec();
}

void eol_window::reset_base_ui_info()
{
  /* 帧数 */
  ui->frame_lcdNumber->display(0);

  /* 字节数 */
  ui->bytes_lcdNumber->display(0);

  /* 耗时 */
  ui->time_cnt_val_label->setNum(0);

  /* 进度 */
  ui->transfer_progressBar->setValue(0);

  /* 错误 */
  ui->error_num_val_label->setNum(0);
}

void eol_window::slot_send_rec_one_frame()
{
  qint32 frame_num_display = ui->frame_lcdNumber->value();
  ui->frame_lcdNumber->display(++frame_num_display);
}
