/**
 *  @file can_log_sender_window.cpp
 *
 *  @date 2024年03月05日 11:11:54 星期二
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 *
 *  @brief can log发送.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2024.03.05 12:11 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-03-05 <td>v0.0.1  <td>aron566 <td>初始版本
 *  <tr><td>2024-03-08 <td>v0.0.2  <td>aron566 <td>改为直接发送方式
 *  <tr><td>2024-03-20 <td>v0.0.3  <td>aron566 <td>发送的数据长度改为索引解析方式
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <QFileDialog>
#include <QDateTime>
#include <QThreadPool>
#include <QStringList>
#include <QSettings>
/** Private includes ---------------------------------------------------------*/
#include "can_log_sender_window.h"
#include "ui_can_log_sender_window.h"
#include "utility.h"
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

can_log_sender_window::can_log_sender_window(QString title, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::can_log_sender)
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

  /* 禁止线程完成后执行析构对象 */
  this->setAutoDelete(false);

  connect(this, &can_log_sender_window::signal_update_progress, this, &can_log_sender_window::slot_update_progress);

  /* 设置悬浮提示 */
  ui->circle_lineEdit->setToolTip(tr("default 1"));
  ui->s_index_lineEdit->setToolTip(tr("must set"));
  ui->bytes_index_lineEdit->setToolTip(tr("must set"));
  ui->can_id_index_lineEdit->setToolTip(tr("must set"));
  ui->send_delay_ms_lineEdit->setToolTip(tr("must set"));
  ui->permissable_can_id_lineEdit->setToolTip(tr("must set & use hex data"));
  ui->response_can_data_lineEdit->setToolTip(tr("this conditions is not used if it's empty"));
  ui->response_can_id_lineEdit->setToolTip(tr("this conditions is not used if it's empty"));
  ui->wait_can_data_index_lineEdit->setToolTip(tr("default 0"));
  ui->wait_response_ms_lineEdit->setToolTip(tr("default 0"));
  ui->send_channel_lineEdit->setToolTip(tr("must set"));

  /* 恢复参数 */
  read_cfg();
}

can_log_sender_window::~can_log_sender_window()
{
  /* 保存参数 */
  save_cfg();

  delete ui;
}

void can_log_sender_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)
  run_state = false;
  this->hide();
  emit signal_window_closed();
}

void can_log_sender_window::read_cfg()
{
  QFile file("./eol_tool_cfg.ini");
  if(false == file.exists())
  {
    return;
  }
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  if(false == setting.contains("can_log_sender_window/last_can_log_dir"))
  {
    qDebug() << "err can_log_sender_window config not exist";
    return;
  }
  /* last_can_log_dir */
  last_file_path = setting.value("can_log_sender_window/last_can_log_dir").toString();
  ui->s_index_lineEdit->setText(setting.value("can_log_sender_window/data_field_s_index").toString());
  ui->bytes_index_lineEdit->setText(setting.value("can_log_sender_window/data_bytes_index").toString());
  ui->can_id_index_lineEdit->setText(setting.value("can_log_sender_window/can_id_field_index").toString());
  ui->send_delay_ms_lineEdit->setText(setting.value("can_log_sender_window/send_delay_ms").toString());
  ui->send_channel_lineEdit->setText(setting.value("can_log_sender_window/send_channel").toString());
  ui->permissable_can_id_lineEdit->setText(setting.value("can_log_sender_window/permissable_can_id_list").toString());
  ui->response_can_id_lineEdit->setText(setting.value("can_log_sender_window/response_can_id_list").toString());
  ui->wait_response_ms_lineEdit->setText(setting.value("can_log_sender_window/wait_response_ms").toString());
  ui->response_can_data_lineEdit->setText(setting.value("can_log_sender_window/response_can_data_list").toString());
  ui->wait_can_data_index_lineEdit->setText(setting.value("can_log_sender_window/wait_can_data_field_index").toString());
  setting.sync();
}

void can_log_sender_window::save_cfg()
{
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  /* last_can_log_dir */
  setting.setValue("can_log_sender_window/last_can_log_dir", last_file_path);
  setting.setValue("can_log_sender_window/data_field_s_index", ui->s_index_lineEdit->text());
  setting.setValue("can_log_sender_window/data_bytes_index", ui->bytes_index_lineEdit->text());
  setting.setValue("can_log_sender_window/can_id_field_index", ui->can_id_index_lineEdit->text());
  setting.setValue("can_log_sender_window/send_delay_ms", ui->send_delay_ms_lineEdit->text());
  setting.setValue("can_log_sender_window/send_channel", ui->send_channel_lineEdit->text());
  setting.setValue("can_log_sender_window/permissable_can_id_list", ui->permissable_can_id_lineEdit->text());
  setting.setValue("can_log_sender_window/response_can_id_list", ui->response_can_id_lineEdit->text());
  setting.setValue("can_log_sender_window/wait_response_ms", ui->wait_response_ms_lineEdit->text());
  setting.setValue("can_log_sender_window/response_can_data_list", ui->response_can_data_lineEdit->text());
  setting.setValue("can_log_sender_window/wait_can_data_field_index", ui->wait_can_data_index_lineEdit->text());
  setting.sync();
}

void can_log_sender_window::set_can_driver_obj(can_driver_model *_can_driver_obj)
{
  can_driver_obj = _can_driver_obj;

  /* 接收can驱动断开信号 */
  connect(can_driver_obj, &can_driver_model::signal_can_driver_reset, this, [this]{
    run_state = false;
  });
  connect(can_driver_obj, &can_driver_model::signal_can_is_closed, this, [this]{
    run_state = false;
    can_driver_obj = nullptr;
  });
}

void can_log_sender_window::can_log_sender_task()
{
  /* 检测次数 */
  if(0U == circle_times)
  {
    run_state = false;
    return;
  }

  /* 打开文件句柄 */
  if(true == file.fileName().isEmpty())
  {
    return;
  }
  if(file.isOpen() == true)
  {
    file.close();
  }
  if(false == file.open(QIODevice::ReadOnly))
  {
    qDebug() << "open file failed";
    return;
  }

  quint32 send_bytes = 0;
  send_data_list.clear();
  quint64 size = file.size();
  /* 读取每一行 */
  for(quint64 i = 0; i < size && true == run_state;)
  {
    QByteArray line_data = file.readLine();
    i += line_data.size();
    emit signal_update_progress(i, size);
    if(line_data.size() <= 0)
    {
      continue;
    }
    QString str = utility::line_data2split(QString(line_data));
    QRegExp split_rx("\\s+");
    QStringList data_list = str.split(split_rx, Qt::SkipEmptyParts);
    /* 检测数据长度索引是否合法 */
    if(send_bytes_index >= (quint32)data_list.size())
    {
      // qDebug() << "send_bytes_index:" << data_list.size();
      continue;
    }
    send_bytes = data_list.value(send_bytes_index).toUInt(nullptr, 16);

    /* 检测数据长度是否合法 */
    if((send_bytes + data_field_index_s) > (quint32)data_list.size())
    {
      // qDebug() << "field num:" << data_list.size();
      continue;
    }
    if(can_id_field_index >= (quint32)data_list.size())
    {
      // qDebug() << "can_id_field_index" << can_id_field_index << "> num:" << data_list.size();
      continue;
    }
    bool ok;
    quint32 can_id = data_list.value(can_id_field_index).toUInt(&ok, 16);
    if(false == can_id_permissable_list.contains(can_id))
    {
      // qDebug() << "contains id" << can_id  << "err" << data_list.value(can_id_field_index);
      continue;
    }
    QStringList data_str;
    for(quint32 strindex = data_field_index_s; strindex < send_bytes + data_field_index_s; strindex++)
    {
      data_str.append(data_list.value(strindex));
    }

    SEND_DATA_INFO_Typedef_t send_info;
    send_info.data = data_str.join(" ");
    send_info.can_id = can_id;
    send_data_list.append(send_info);
  }

  if(file.isOpen() == true)
  {
    file.close();
  }

  /* 发送 */
  do{
    for(qint32 i = 0; i < send_data_list.size() && true == run_state; i++)
    {
      emit signal_update_progress(i, send_data_list.size());
      if(nullptr == can_driver_obj)
      {
        continue;
      }
      SEND_DATA_INFO_Typedef_t send_info;
      send_info = send_data_list.value(i);
      quint64 start_time = QDateTime::currentMSecsSinceEpoch();

      // qDebug() << "id:" << data_list.value(can_id_field_index) << "data:" << data;
      // can_driver_obj->period_send_set(send_info.can_id,
      //                                 utility::line_data2split(send_info.data),
      //                                 0, 1, send_channel,
      //                                 can_driver_model::STD_FRAME_TYPE,
      //                                 8U < send_bytes ? can_driver_model::CANFD_PROTOCOL_TYPE : can_driver_model::CAN_PROTOCOL_TYPE,
      //                                 0U);
      // while(can_driver_obj->get_period_send_list_size() > 0 && true == run_state)
      // {
      //   QThread::usleep(500);
      // }

      /* 拷贝数据 */
      quint8 data_buf[64];
      QStringList data_list = send_info.data.split(' ');
      quint32 data_size = (quint32)data_list.length();
      data_size = data_size > send_bytes ? send_bytes : data_size;
      for(quint8 index = 0; index < data_size; index++)
      {
        data_buf[index] = (quint8)data_list[index].toUShort(nullptr, 16);
      }
      /* 立即发送 */
      can_driver_obj->send(data_buf, data_size,
                           send_info.can_id,
                           can_driver_model::STD_FRAME_TYPE,
                           8U < send_bytes ? can_driver_model::CANFD_PROTOCOL_TYPE : can_driver_model::CAN_PROTOCOL_TYPE,
                           send_channel);

      quint32 delay_time_cnt = is_timeout(start_time, send_delay_ms);
      if(0U < delay_time_cnt)
      {
        QThread::msleep((quint32)delay_time_cnt);
      }

      /* 等待特殊canid帧 */
      if(false == can_id_response_list.isEmpty())
      {
        if(true == can_id_response_list.contains(send_info.can_id))
        {
          /* 延时剩余时间 */
          quint32 current_delay = wait_response_delay_ms > delay_time_cnt ? wait_response_delay_ms - delay_time_cnt : 0U;
          QThread::msleep(current_delay);
          delay_time_cnt += current_delay;
        }
      }

      /* 等待特殊数据帧 */
      if(false == wait_can_data_list.isEmpty())
      {
        QStringList data_list = send_info.data.split(" ");
        if(data_list.size() <= (qint32)wait_can_data_index)
        {
          continue;
        }
        quint32 can_data = data_list.value(wait_can_data_index).toUInt(nullptr, 16);
        if(true == wait_can_data_list.contains(can_data))
        {
          /* 延时剩余时间 */
          quint32 current_delay = wait_response_delay_ms > delay_time_cnt ? wait_response_delay_ms - delay_time_cnt : 0U;
          QThread::msleep(current_delay);
          // qDebug("wait 0x%02X %ums", can_data, current_delay);
          // delay_time_cnt += current_delay;
        }
      }
    }
    emit signal_update_progress(send_data_list.size(), send_data_list.size());
  }while((--circle_times) && true == run_state);

  run_state = false;
}

quint64 can_log_sender_window::is_timeout(quint64 start_time, quint64 time_out_ms)
{
  quint64 time = QDateTime::currentMSecsSinceEpoch() - start_time;
  if(time < time_out_ms)
  {
    return time_out_ms - time;
  }
  return 0;
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

void can_log_sender_window::slot_update_progress(qint32 size, qint32 total)
{
  ui->send_progressBar->setValue(size);
  ui->send_progressBar->setMaximum(total);
  if(size == total)
  {
    current_circle_times++;
    ui->current_circle_lineEdit->setText(QString::number(current_circle_times - 1U));
  }
}

void can_log_sender_window::on_set_file_pushButton_clicked()
{
  /* 选择文件前先关闭 */
  if(file.isOpen() == true)
  {
    file.close();
  }

  /* 选择文件 */
  QString filepath = QFileDialog::getOpenFileName(this, tr("Open Can Log File"), last_file_path, tr("BIN (*.txt);;HEX (*.hex);;HTPKG (*.htpkg)"));
  if(filepath.isEmpty() == true)
  {
    qDebug() << "打开文件错误！";
    return;
  }

  /* 设置文件信息 */
  file.setFileName(filepath);

  /* 获取文件信息 */
  QFileInfo info(filepath);
  filename = info.fileName();
  filesize = info.size();

  /* 更新最近路径信息 */
  last_file_path = info.absolutePath();

  /* 显示文件名 */
  ui->send_progressBar->setValue(0);
  ui->send_progressBar->setMinimum(0);
  ui->file_name_lineEdit->setText(filename);
}

void can_log_sender_window::on_start_pushButton_clicked()
{
  /* 检测是否空闲 */
  if(true == thread_run_state)
  {
    return;
  }

  if(true == filename.isEmpty())
  {
    return;
  }

  /* 重复次数 */
  if(true == ui->circle_lineEdit->text().isEmpty())
  {
    circle_times = 1;
    ui->circle_lineEdit->setText("1");
  }
  circle_times = (quint32)ui->circle_lineEdit->text().toInt();

  /* 数据起始索引 */
  if(true == ui->s_index_lineEdit->text().isEmpty())
  {
    return;
  }
  data_field_index_s = ui->s_index_lineEdit->text().toUInt();

  /* 字节数索引 */
  if(true == ui->bytes_index_lineEdit->text().isEmpty())
  {
    return;
  }
  send_bytes_index = ui->bytes_index_lineEdit->text().toUInt();

  /* can id索引 */
  if(true == ui->can_id_index_lineEdit->text().isEmpty())
  {
    return;
  }
  can_id_field_index = ui->can_id_index_lineEdit->text().toUInt();

  /* 帧间发送延时设置 */
  if(true == ui->send_delay_ms_lineEdit->text().isEmpty())
  {
    return;
  }
  send_delay_ms = ui->send_delay_ms_lineEdit->text().toUInt();

  /* 发送通道设置 */
  if(true == ui->send_channel_lineEdit->text().isEmpty())
  {
    return;
  }
  send_channel = (quint8)ui->send_channel_lineEdit->text().toUShort();

  /* 允许发送的can id列表 */
  if(true == ui->permissable_can_id_lineEdit->text().isEmpty())
  {
    return;
  }
  QString str = utility::line_data2split(ui->permissable_can_id_lineEdit->text());
  QRegExp split_rx("\\s+");
  QStringList data_list = str.split(split_rx, Qt::SkipEmptyParts);
  can_id_permissable_list.clear();
  for(qint32 i = 0; i < data_list.size(); i++)
  {
    can_id_permissable_list.append(data_list.value(i).toUInt(nullptr, 16));
  }

  /* 等待应答的can id列表 */
  if(true == ui->response_can_id_lineEdit->text().isEmpty())
  {
    can_id_response_list.clear();
  }
  else
  {
    QString str = utility::line_data2split(ui->response_can_id_lineEdit->text());
    QRegExp split_rx("\\s+");
    QStringList data_list = str.split(split_rx, Qt::SkipEmptyParts);
    can_id_response_list.clear();
    for(qint32 i = 0; i < data_list.size(); i++)
    {
      can_id_response_list.append(data_list.value(i).toUInt(nullptr, 16));
    }
  }

  /* 等待时间设置 */
  if(true == ui->wait_response_ms_lineEdit->text().isEmpty())
  {
    wait_response_delay_ms = 0;
    ui->wait_response_ms_lineEdit->setText("0");
  }
  else
  {
    wait_response_delay_ms = ui->wait_response_ms_lineEdit->text().toUInt();
  }

  /* 特殊等待can数据列表 */
  if(true == ui->response_can_data_lineEdit->text().isEmpty())
  {
    wait_can_data_list.clear();
  }
  else
  {
    QString str = utility::line_data2split(ui->response_can_data_lineEdit->text());
    QRegExp split_rx("\\s+");
    wait_can_data_list.clear();
    QStringList data_list = str.split(split_rx, Qt::SkipEmptyParts);
    for(qint32 i = 0; i < data_list.size(); i++)
    {
      wait_can_data_list.append(data_list.value(i).toUInt(nullptr, 16));
    }
  }

  /* 特殊等待can数据索引号 */
  if(true == ui->wait_can_data_index_lineEdit->text().isEmpty())
  {
    wait_can_data_index = 0;
    ui->wait_can_data_index_lineEdit->setText("0");
  }
  else
  {
    wait_can_data_index = ui->wait_can_data_index_lineEdit->text().toUInt();
  }

  /* 启动线程 */
  run_state = true;

  /* 原子操作 */
  if(thread_run_statex.testAndSetRelaxed(0, 1))
  {
    /* 重置计数 */
    current_circle_times = 0;
    ui->current_circle_lineEdit->setText("0");

    /* 启动任务线程 */
    QThreadPool::globalInstance()->start(this);
  }
  else
  {
    qDebug() << "can log sender window task is running";
    return;
  }
}

/******************************** End of file *********************************/
