/**
 *  @file updatefw_window.cpp
 *
 *  @date 2024年02月21日 11:58:58 星期三
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 *
 *  @brief 更新固件窗口.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2024.02.21 11:58 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-02-21 <td>v0.0.1  <td>aron566 <td>初始版本
 *  <tr><td>2024-03-07 <td>v0.0.2  <td>aron566 <td>添加退出升级及升级失败提示信息
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
/** Private includes ---------------------------------------------------------*/
#include "updatefw_window.h"
#include "ui_updatefw_window.h"
#include "updatefw_protocol/updatefw_protocol.h"
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

/**
 * @brief updatefw_window::updatefw_window
 * @param parent
 */
updatefw_window::updatefw_window(QString title, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::updatefw_window)
{
  ui->setupUi(this);

  this->setWindowTitle(title);

  /* 应用样式主题 */
  QFile file(":/qdarkstyle/dark/style.qss");
  if(file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    this->setStyleSheet(file.readAll());
    file.close();
  }

  timer = new QTimer(this);
  timer->setInterval(1000);
  connect(timer, &QTimer::timeout, this, &updatefw_window::slot_timer_timeout);

  /* 设置临时文件模板名称 */
  QString strFileName = QDir::tempPath() + QDir::separator() +
                        QCoreApplication::applicationName() + "_XXXXXX." + "htpkgtmp";

  tmpFile.setFileTemplate(strFileName);

  /* 设置为自动删除 */
  tmpFile.setAutoRemove(true);

  /* 恢复参数 */
  read_cfg();
}

/**
 * @brief updatefw_window::~updatefw_window
 */
updatefw_window::~updatefw_window()
{
  /* 保存参数 */
  save_cfg();

  delete ui;
}

void updatefw_window::save_cfg()
{
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  /* last_fw_dir */
  setting.setValue("updatefw_window/last_fw_dir", last_file_path);
  setting.sync();
}

void updatefw_window::read_cfg()
{
  QFile file("./eol_tool_cfg.ini");
  if(false == file.exists())
  {
    return;
  }
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  if(false == setting.contains("updatefw_window/last_fw_dir"))
  {
    qDebug() << "err updatefw_window config not exist";
    return;
  }
  /* last_fw_dir */
  last_file_path = setting.value("updatefw_window/last_fw_dir").toString();
  setting.sync();
}

void updatefw_window::set_can_driver_obj(can_driver_model *can_driver_obj)
{
  /* 接收can驱动断开信号 */
  connect(can_driver_obj, &can_driver_model::signal_can_driver_reset, this, [this]{
    this->protocol_stack_obj->stop_task();
    run_state = false;
  });
  connect(can_driver_obj, &can_driver_model::signal_can_is_closed, this, [this]{
    this->protocol_stack_obj->stop_task();
    run_state = false;
  });

  /* 创建协议栈 */
  protocol_stack_obj = new updatefw_protocol(can_driver_obj);
  connect(protocol_stack_obj, &updatefw_protocol::signal_protocol_error_occur, this, &updatefw_window::slot_send_data_timeout_occured);
  connect(protocol_stack_obj, &updatefw_protocol::signal_send_progress, this, &updatefw_window::slot_send_progress, Qt::QueuedConnection);
  connect(protocol_stack_obj, &updatefw_protocol::signal_protocol_rw_err, this, &updatefw_window::slot_protocol_rw_err);
  connect(protocol_stack_obj, &updatefw_protocol::signal_protocol_timeout, this, &updatefw_window::slot_protocol_timeout);
  connect(protocol_stack_obj, &updatefw_protocol::signal_protocol_run_step_msg, this, &updatefw_window::slot_protocol_run_step_msg);

  /* 禁止线程完成后执行析构对象 */
  protocol_stack_obj->setAutoDelete(false);

  protocol_stack_obj->set_can_driver_obj(can_driver_obj);
}

/**
 * @brief updatefw_window::reset_send_info
 */
void updatefw_window::reset_send_info()
{
  /*设置进度条*/
  ui->upgrade_progressBar->setMinimum(0);
  ui->upgrade_progressBar->setMaximum(static_cast<int>(filesize));
  ui->upgrade_progressBar->setValue(0);

  /*重置已发送大小为0*/
  sendfilesize = 0;

  current_sec = 0;
  current_min = 0;

  timeout_cnt = 0;

  /*重置时间*/
  ui->min_lcdNumber->display(0);
  ui->sec_lcdNumber->display(0);

  /*重置已发字节统计*/
  ui->bytes_label->setText(tr("已发字节"));
  ui->bytes_lcdNumber->display(0);

  /*重置发送包数统计*/
  ui->package_label->setText(tr("已发包数"));
  ui->package_lcdNumber->display(0);

  /*重置错误统计*/
  ui->error_cnt_label->setNum(0);
}

/**
 * @brief updatefw_window::reset_recv_info
 */
void updatefw_window::reset_recv_info()
{
  /*设置进度条*/
  ui->upgrade_progressBar->setMinimum(0);
  ui->upgrade_progressBar->setMaximum(ui->frimware_size->text().toInt());
  ui->upgrade_progressBar->setValue(0);

  /*重置已接收大小为0*/
  recvfilesize = 0;

  current_sec = 0;
  current_min = 0;

  timeout_cnt = 0;

  /*重置时间*/
  ui->min_lcdNumber->display(0);
  ui->sec_lcdNumber->display(0);

  /*重置已发字节统计*/
  ui->bytes_label->setText(tr("已收字节"));
  ui->bytes_lcdNumber->display(0);

  /*重置发送包数统计*/
  ui->package_label->setText(tr("已收包数"));
  ui->package_lcdNumber->display(0);

  /*重置错误统计*/
  ui->error_cnt_label->setNum(0);
}

void updatefw_window::set_channel_num(quint8 channel_num)
{
  /* 根据所选设备类型，更新通道数量 */
  ui->update_ch_comboBox->clear();
  for(quint8 i = 0; i < channel_num; i++)
  {
    ui->update_ch_comboBox->addItem(QString("CH%1").arg(i));
  }
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

/**
 * @brief updatefw_window::slot_send_data_timeout_occured
 */
void updatefw_window::slot_send_data_timeout_occured()
{
  timeout_cnt++;
  ui->error_cnt_label->setNum(timeout_cnt);
}

void updatefw_window::slot_protocol_timeout(quint32 ms)
{
  QString info = ui->run_info_msg->text();
  QStringList infos = info.split(",");
  info = infos.value(0);
  info += QString(",timeout %1").arg(ms);
  ui->run_info_msg->setText(info);
}

void updatefw_window::slot_protocol_rw_err(QString cmd)
{
  Q_UNUSED(cmd)
  run_state = false;

  QString tips;
  /* 检测是否是主动关闭状态 */
  if(true == this->isHidden())
  {
    tips = "updatefw exit!";
  }
  else
  {
    tips = "updatefw err!";
  }
  QMessageBox message(QMessageBox::Information, tr("通知"), tr("<font size='10' color='red'>%1</font>").arg(tips), QMessageBox::Yes, nullptr);
  message.exec();
}

void updatefw_window::slot_protocol_run_step_msg(QString msg)
{
  ui->run_info_msg->setText("run_info:"+msg);
}

void updatefw_window::slot_send_progress(quint32 current_size, quint32 total_size)
{
  /* 更新包号 */
  qint32 packet_num_cnt = ui->package_lcdNumber->value();
  ui->package_lcdNumber->display(static_cast<int>(++packet_num_cnt));

  /* 更新进度 */
  ui->upgrade_progressBar->setValue(static_cast<int>(current_size));

  /* 更新已发字节 */
  ui->bytes_lcdNumber->display(static_cast<int>(current_size));

  if(current_size == total_size)
  {
    run_state = false;
    ui->frimware_sel_button->setEnabled(true);
    /* 正确发送完成-允许再次点击更新按钮 */
    ui->start_update_button->setEnabled(true);
    QMessageBox message(QMessageBox::Information, tr("通知"), tr("<font size='10' color='green'>固件升级完成！</font>"), QMessageBox::Yes, nullptr);
    message.exec();
  }
}

/**
 * @brief updatefw_window::slot_recv_frimware_data
 * @param data
 * @param package_num
 */
void updatefw_window::slot_recv_frimware_data(const char *data, quint16 package_num)
{
  if(run_state == false)
  {
    if(recv_file.isOpen() == true)
    {
      recv_file.close();
    }
    return;
  }

  /*固件信息*/
  if(package_num == 0x0000)
  {
    QString str;
    str = QString::asprintf("%s", data);
    QStringList list = str.split('+');
    qDebug() << "读取固件信息成功";
    if(list.size() == 2)
    {
      ui->frimware_name->setText(list.value(0));
      ui->frimware_size->setText(list.value(1));
      ui->upgrade_progressBar->setMaximum(list.value(1).toInt());

      /*选择文件存储区域*/
      /*参数：父对象，标题，默认路径，格式*/
      QString path = QFileDialog::getSaveFileName(this, tr("Save  ") + ui->frimware_name->text(), "../", tr("BIN(*.bin)"));
      if(path.isEmpty() == true)
      {
        run_state = false;
        return;
      }

      /*先关闭*/
      if(recv_file.isOpen() == true)
      {
        recv_file.close();
      }

      /*关联文件名*/
      recv_file.setFileName(path);

      /*打开文件，只写方式*/
      recv_file.open(QIODevice::WriteOnly);
    }
  }
  /*固件数据*/
  else if(package_num < 0xFFFF && package_num > 0)
  {
    if(recv_file.isOpen() == false)
    {
      return;
    }
    qDebug() << "读取固件数据成功 包" << package_num;
    recv_file.seek((package_num-1)*128);
    recv_file.write(data, 128);
    recvfilesize = package_num*128;
    recvfilesize = recvfilesize > ui->frimware_size->text().toLong()?ui->frimware_size->text().toLong():recvfilesize;
    /*更新接收到的包数*/
    ui->package_lcdNumber->display(package_num);
    /*更新接收到的字节数*/
    ui->bytes_lcdNumber->display(static_cast<int>(recvfilesize));
    /*更新进度条*/
    ui->upgrade_progressBar->setValue(static_cast<int>(recvfilesize));

  }
  else
  {
    /*关闭文件*/
    recv_file.close();
    /*停止运行*/
    run_state = false;
    QMessageBox message(QMessageBox::Information, tr("通知"), tr("<font size='10' color='green'>固件上载完成！</font>"), QMessageBox::Yes, nullptr);
    message.exec();
  }
}

/**
 * @brief updatefw_window::on_frimware_sel_button_clicked
 */
void updatefw_window::on_frimware_sel_button_clicked()
{
  /* 选择文件前先关闭 */
  if(file.isOpen() == true)
  {
    file.close();
  }

  /* 选择文件 */
  QString filepath = QFileDialog::getOpenFileName(this, tr("Open Update File"), last_file_path, tr("BIN (*.bin);;HEX (*.hex);;HTPKG (*.htpkg)"));
  if(filepath.isEmpty() == true)
  {
    qDebug() << "打开文件错误！62";
  }

  filename.clear();
  filesize = 0;
  /* 获取文件信息 */
  QFileInfo info(filepath);
  filename = info.fileName();
  filesize = info.size();

  /* 更新最近路径信息 */
  last_file_path = info.absolutePath();

  /* 检查非法空格字段 */
  // filename.replace(QChar(' '), QChar('_'));

  /* 检查文件名后缀 */
  QString suffix = info.suffix();
  if("htpkg" == suffix)
  {
    /* 解密 */
    /* 创建临时文件 */
    tmpFile.remove();
    if(tmpFile.open() == false)
    {
      qDebug() << "open tmp file faild";
      return;
    }
    // cmd_process cmd_process_obj;
    // if(cmd_process_obj.cmd_process_start(filepath, tmpFile.fileName(), cmd_process::DECRYPT_MODE) != cmd_process::CMD_PROCESS_OK)
    // {
    //   qDebug() << "DECRYPT Faild";
    //   return;
    // }
    /* 设置解密后的文件名 */
    filename = info.completeBaseName() + tr(".") + tr("bin");
    filepath = tmpFile.fileName();
  }

  /* 设置待写入固件信息 */
  ui->file_name->setText(filename);
  ui->file_size->setText(QString("%1").arg(filesize));

  /* 设置文件名 */
  file.setFileName(filepath);
  qDebug() << "tempfile:" << filepath;

  /* 允许点击更新 */
  ui->start_update_button->setEnabled(true);
}

/**
 * @brief updatefw_window::on_start_update_button_clicked
 */
void updatefw_window::on_start_update_button_clicked()
{
  if(filesize <= 0 || run_state == true)
  {
    return;
  }

  if(nullptr == protocol_stack_obj)
  {
    return;
  }

  /* 重置发送信息 */
  reset_send_info();

  /* 发送中-禁用选择文件 */
  ui->frimware_sel_button->setEnabled(false);
  ui->start_update_button->setEnabled(false);

  /* 获取文件信息 */
  QFileInfo info(file.fileName());
  filesize = info.size();

  /* 更新进度条 */
  ui->upgrade_progressBar->setMaximum(static_cast<int>(filesize));

  /* 更新文件大小 */
  ui->file_size->setText(QString("%1").arg(filesize));

  /* 设置更新分区 */
  // protocol_stack_obj->protocol_stack_stop();
  // protocol_stack_obj->protocol_stack_create_task(0x03, 0x0008, &download_app_partition, 1);

  /* 运行 */
  run_state = true;

  /*启动计时*/
  timer->start();

  /* 设置通讯参数 */
  protocol_stack_obj->set_com_config_channel(QString::number(ui->update_ch_comboBox->currentIndex()));

  /* 启动更新 */
  updatefw_protocol::UPDATEFW_TASK_LIST_Typedef_t task;
  task.fw_file_name = file.fileName();
  task.start_addr = 0x50000U;
  task.param = nullptr;
  protocol_stack_obj->updatefw_device_app(task, true);
  protocol_stack_obj->start_task();

  /* 设置更新时机 */
  // protocol_stack_obj->protocol_stack_stop();
  // protocol_stack_obj->protocol_stack_create_task(0x03, 0x0009, &right_now_update, 1);
}

/**
 * @brief updatefw_window::on_start_upload_button_clicked
 */
void updatefw_window::on_start_upload_button_clicked()
{
  /*检查是否正在运行*/
  if(run_state == true)
  {
      return;
  }

  /*重置接收信息*/
  reset_recv_info();

  /*运行态*/
  run_state = true;
  // protocol_stack_obj->protocol_upgrade_frame_stack_start_stop(true);

  /*计时器启动*/
  timer->start();

  /*获取固件信息*/
  // protocol_stack_obj->protocol_stack_stop();
  // protocol_stack_obj->protocol_stack_upload_frimware_info_start();

  /*启动读取固件数据流*/
  // protocol::RETURN_TYPE_Typedef_t ret = protocol_stack_obj->protocol_stack_upload_frimware_data_start();
  // if(ret == protocol::RETURN_UPLOAD_END || ret == protocol::RETURN_ERROR || ret == protocol::RETURN_TIMEOUT)
  // {
  //     run_state = false;
  //     qDebug() << "退出上传";
  // }
}

/**
 * @brief updatefw_window::on_right_now_radioButton_clicked
 */
void updatefw_window::on_right_now_radioButton_clicked()
{
  right_now_update = 1;
}

/**
 * @brief updatefw_window::on_next_reboot_radioButton_clicked
 */
void updatefw_window::on_next_reboot_radioButton_clicked()
{
  right_now_update = 0;
}

/**
 * @brief updatefw_window::on_app_radioButton_clicked
 */
void updatefw_window::on_app_radioButton_clicked()
{
  download_app_partition = 0;
}

/**
 * @brief updatefw_window::on_boot_radioButton_clicked
 */
void updatefw_window::on_boot_radioButton_clicked()
{
  download_app_partition = 2;
}

/**
 * @brief updatefw_window::on_restore_radioButton_clicked
 */
void updatefw_window::on_restore_radioButton_clicked()
{
  download_app_partition = 1;
}

/**
 * @brief updatefw_window::closeEvent
 * @param event
 */
void updatefw_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  /*停止运行*/
  run_state = false;

  /* 允许再次点击更新按钮 */
  ui->frimware_sel_button->setEnabled(true);
  ui->start_update_button->setEnabled(true);
  ui->start_upload_button->setEnabled(true);

  this->hide();
  emit signal_window_closed();
}

/**
 * @brief updatefw_window::slot_timer_timeout
 */
void updatefw_window::slot_timer_timeout()
{
  if(run_state == false)
  {
    timer->stop();
    /* 允许再次点击更新按钮 */
    ui->frimware_sel_button->setEnabled(true);
    ui->start_update_button->setEnabled(true);
    ui->start_upload_button->setEnabled(true);

    if(nullptr == protocol_stack_obj)
    {
      return;
    }
    protocol_stack_obj->stop_task();
  }

  current_sec++;
  if(current_sec == 60)
  {
    current_min++;
    current_sec = 0;
  }

  /*更新时间*/
  ui->sec_lcdNumber->display(current_sec);
  ui->min_lcdNumber->display(current_min);
}

/******************************** End of file *********************************/
