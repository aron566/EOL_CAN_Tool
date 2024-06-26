/**
 *  @file eol_window.cpp
 *
 *  @date 2024年01月18日 11:11:54 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 *
 *  @brief eol窗口.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2024.01.18 12:11 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-01-18 <td>v0.0.1  <td>aron566 <td>初始版本
 *  <tr><td>2024-03-08 <td>v0.0.2  <td>aron566 <td>优化重新打开关闭driver通讯设置未生效问题
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QScrollArea>
#include <QDebug>
/** Private includes ---------------------------------------------------------*/
#include "eol_window.h"
#include "qtimer.h"
#include "ui_eol_window.h"
#include "utility.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define CONFIG_VER_STR            "0.0.2"                 /**< 配置文件版本 */
#define USE_TEMP_FILE_TO_SAVE     0                       /**< 使用临时文件存储 */
#define SET_CURRENT_TASK_RUN_STATE(set_val) \
do{ \
      current_task_complete_state = set_val; \
      qDebug() << "[" << __FUNCTION__ << "]" << "SET VAL" << set_val; \
}while(0)
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

eol_window::eol_window(QString title, QWidget *parent) :
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
  this->setWindowTitle(title);

  /* 初始化定时器 */
  timer_init();

  /* 默认按钮不可用状态 */
  ui->add_list_pushButton->setEnabled(false);
  ui->upload_pushButton->setEnabled(false);
  ui->update_pushButton->setEnabled(false);
  ui->export_all_pushButton->setEnabled(false);
  ui->reboot_pushButton->setEnabled(false);
  ui->debug_pushButton->setEnabled(false);
  ui->eol_device_rw_func_pushButton->setEnabled(false);
  ui->ant_calibration_func_pushButton->setEnabled(false);
  ui->rcs_calibration_func_pushButton->setEnabled(false);

  /* 关闭按钮显示 */
  ui->rts_crl_pushButton->setVisible(false);
  ui->open_rts_pushButton->setVisible(false);
  ui->close_rts_pushButton->setVisible(false);
  ui->plc_crl_pushButton->setVisible(false);
  ui->open_plc_pushButton->setVisible(false);
  ui->close_plc_pushButton->setVisible(false);

  /* 设置提示值 */
  ui->com_config_lineEdit->setPlaceholderText("0 or 1 or 255");
  ui->dev_addr_lineEdit->setPlaceholderText("85 or 4 5 6 7");
  ui->rts_addr_lineEdit->setPlaceholderText("192.168.3.181:12001");
  ui->plc_addr_lineEdit->setPlaceholderText("192.168.3.182:503");

  /* 设置悬浮提示 */
  ui->com_hw_comboBox->setToolTip(tr("eol hw select"));
  ui->com_config_lineEdit->setToolTip(tr("eol pcom channel to rec msg or send msg 255 is send all channel"));
  ui->vcom_config_lineEdit->setToolTip(tr("eol vcom channel to rec msg or send msg 255 is send all channel"));
  ui->dev_addr_lineEdit->setToolTip(tr("set dev addr, specify device address 85 is broadcast addr"));

  /* 初始化状态 */
  reset_base_ui_info();

  /* 本窗口子线程更新传输列表动作 */
  connect(this, &eol_window::signal_update_show_table_list, this, &eol_window::slot_update_show_table_list, Qt::BlockingQueuedConnection);
  connect(this, &eol_window::signal_upload_all_table_task_ok, this, &eol_window::slot_recv_eol_data_complete);

  /* 设备信息读写窗口初始化 */
  eol_sub_window_init(tr("EOL CAN Tool - Device Info RW"));

  /* 天线校准初始化-2DFFT */
  eol_2dfft_calibration_window_init(tr("EOL CAN Tool - 2DFFT Calibration"));

  /* rcs校准初始化-目标列表 */
  eol_rcs_calibration_window_init(tr("EOL CAN Tool - RCS Calibration"));

  /* 调试参数初始化 */
  eol_debug_window_init(tr("EOL CAN Tool - Debug Par"));

  /* rts手动控制窗口初始化 */
  rts_ctrl_window_init(tr("EOL CAN Tool - RTS CTL"));

  /* 设置临时文件模板名称 */
  QString strFileName = QDir::tempPath() + QDir::separator() +
                        QCoreApplication::applicationName() + "_XXXXXX." + "csvtmp";

  tmpFile.setFileTemplate(strFileName);

  /* 设置为自动删除 */
  tmpFile.setAutoRemove(true);
}

eol_window::~eol_window()
{
  /* 保存参数 */
  save_cfg();

  run_state = false;
  qDebug() << "eol window wait to end";

  /* 等待线程结束 */
  while(thread_run_state)
  {
    utility::delay_ms(1);
  }

  /* 删除子窗口 */
  delete eol_sub_window_obj;
  qDebug() << "del eol_sub_window_obj";

  delete eol_calibration_window_obj;
  qDebug() << "del eol_calibration_window_obj";

  delete eol_2dfft_calibration_window_obj;
  qDebug() << "del eol_2dfft_calibration_window_obj";

  delete debug_window_window_obj;
  qDebug() << "del debug_window_window_obj";

  delete rts_ctrl_window_obj;
  qDebug() << "del rts_ctrl_window_obj";

  delete ui;
  qDebug() << "del eol_window";
}

void eol_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  /* 终止并清空传输任务 */
  run_state = false;

  /* 停止协议栈 */
  if(nullptr != eol_protocol_obj)
  {
    eol_protocol_obj->stop_task();
  }

  this->hide();
  emit signal_eol_window_closed();
}

/**
 * @brief 信息读取子窗口
 */
void eol_window::eol_sub_window_init(QString title)
{
  eol_sub_window_obj = new eol_sub_window(title);
  connect(eol_sub_window_obj, &eol_sub_window::signal_window_closed, this, &eol_window::slot_show_this_window);
}

/**
 * @brief rcs校准子窗口
 */
void eol_window::eol_rcs_calibration_window_init(QString title)
{
  eol_calibration_window_obj = new eol_calibration_window(title);
  connect(eol_calibration_window_obj, &eol_calibration_window::signal_window_closed, this, &eol_window::slot_show_this_window);

  /* 校准配置更新 */
  connect(this, &eol_window::signal_clear_profile_info, eol_calibration_window_obj, &eol_calibration_window::slot_clear_profile_info);
  connect(this, &eol_window::signal_profile_info_update, eol_calibration_window_obj, &eol_calibration_window::slot_profile_info_update);
}

/**
 * @brief 2dfft校准子窗口
 */
void eol_window::eol_2dfft_calibration_window_init(QString title)
{
  eol_2dfft_calibration_window_obj = new eol_angle_calibration_window(title);
  connect(eol_2dfft_calibration_window_obj, &eol_angle_calibration_window::signal_window_closed, this, &eol_window::slot_show_this_window);

  /* 校准配置更新 */
  connect(this, &eol_window::signal_clear_profile_info, eol_2dfft_calibration_window_obj, &eol_angle_calibration_window::slot_clear_profile_info);
  connect(this, &eol_window::signal_profile_info_update, eol_2dfft_calibration_window_obj, &eol_angle_calibration_window::slot_profile_info_update);
}

/**
 * @brief 调试窗口初始化
 */
void eol_window::eol_debug_window_init(QString title)
{
  debug_window_window_obj = new debug_window(title);
  connect(debug_window_window_obj, &debug_window::signal_window_closed, this, &eol_window::slot_show_this_window);
  connect(debug_window_window_obj, &debug_window::signal_window_closed, this, &eol_window::slot_close_shell_window);

  connect(debug_window_window_obj, &debug_window::signal_send_command, this, &eol_window::slot_send_command);
  connect(debug_window_window_obj, &debug_window::signal_send_command_char, this, &eol_window::slot_send_command_char);
}

void eol_window::rts_ctrl_window_init(QString title)
{
  rts_ctrl_window_obj = new rts_ctrl_window(title);
  connect(rts_ctrl_window_obj, &rts_ctrl_window::signal_window_closed, this, &eol_window::slot_show_this_window);
}

void eol_window::set_rts_driver_obj(network_driver_model *network_driver_send_obj, network_driver_model *network_driver_rec_obj)
{
  /* 初始化rts协议栈 */
  rts_protocol_init(network_driver_send_obj, network_driver_rec_obj);

  /* 接收网络驱动断开信号 */
  connect(network_driver_send_obj, &network_driver_model::signal_network_is_closed, this, [this]{
    this->rts_protocol_obj->stop_task();
    /* 关闭按钮显示 */
    ui->rts_crl_pushButton->setVisible(false);
    ui->open_rts_pushButton->setVisible(false);
    ui->close_rts_pushButton->setVisible(false);
  });

  /* 显示按钮 */
  ui->rts_crl_pushButton->setVisible(true);
  ui->open_rts_pushButton->setVisible(true);
  ui->close_rts_pushButton->setVisible(true);

  /* 设置手动控制界面驱动接口 */
  rts_ctrl_window_obj->set_rts_protocol_obj(rts_protocol_obj);
}

void eol_window::set_can_driver_obj(can_driver_model *can_driver_obj)
{
  /* 接收can驱动断开信号 */
  connect(can_driver_obj, &can_driver_model::signal_can_driver_reset, this, [this]{
    this->eol_protocol_obj->stop_task();
  });
  connect(can_driver_obj, &can_driver_model::signal_can_is_closed, this, [this]{
    this->eol_protocol_obj->stop_task();
  });
  eol_protocol_init(can_driver_obj);
}

void eol_window::eol_protocol_init(can_driver_model *can_driver_obj)
{
  /* eol协议栈初始化 */
  eol_protocol_obj = new eol_protocol(can_driver_obj);

  connect(eol_protocol_obj, &eol_protocol::signal_protocol_error_occur, this, &eol_window::slot_protocol_error_occur, Qt::BlockingQueuedConnection);
  connect(eol_protocol_obj, &eol_protocol::signal_protocol_no_response, this, &eol_window::slot_protocol_no_response, Qt::BlockingQueuedConnection);
  connect(eol_protocol_obj, &eol_protocol::signal_protocol_crc_check_failed, this, &eol_window::slot_protocol_crc_check_failed);
  //  connect(eol_protocol_obj, &eol_protocol::signal_recv_eol_table_data, this, &eol_window::slot_recv_eol_table_data);
  connect(eol_protocol_obj, &eol_protocol::signal_recv_eol_table_data, this, &eol_window::slot_recv_eol_table_data, Qt::BlockingQueuedConnection);
  connect(eol_protocol_obj, &eol_protocol::signal_send_progress, this, &eol_window::slot_send_progress);
  connect(eol_protocol_obj, &eol_protocol::signal_recv_eol_data_complete, this, &eol_window::slot_recv_eol_data_complete);
  connect(eol_protocol_obj, &eol_protocol::signal_send_eol_data_complete, this, &eol_window::slot_send_eol_data_complete);
  connect(eol_protocol_obj, &eol_protocol::signal_protocol_timeout, this, &eol_window::slot_protocol_timeout);
  //  connect(eol_protocol_obj, &eol_protocol::signal_device_mode, this, &eol_window::slot_device_mode, Qt::BlockingQueuedConnection);
  connect(eol_protocol_obj, &eol_protocol::signal_device_mode, this, &eol_window::slot_device_mode);
  connect(eol_protocol_obj, &eol_protocol::signal_send_rec_one_frame, this, &eol_window::slot_send_rec_one_frame, Qt::QueuedConnection);
  connect(eol_protocol_obj, &eol_protocol::signal_protocol_rw_err, this, &eol_window::slot_protocol_rw_err, Qt::BlockingQueuedConnection);
  connect(eol_protocol_obj, &eol_protocol::signal_rw_device_ok, this, &eol_window::slot_rw_device_ok, Qt::QueuedConnection);
  connect(eol_protocol_obj, &eol_protocol::signal_eol_protol_is_start, this, &eol_window::slot_eol_protol_is_start, Qt::QueuedConnection);

  /* 设置线程池 */
  eol_protocol_obj->set_thread_pool(g_thread_pool);

  /* 禁止线程完成后执行析构对象 */
  eol_protocol_obj->setAutoDelete(false);

  /* 设置can驱动接口 */
  eol_protocol_obj->set_can_driver_obj(can_driver_obj);

  /* 设置通讯 */
  eol_protocol_obj->set_eol_com_config_hw((eol_protocol::EOL_SEND_HW_Typedef_t)ui->com_hw_comboBox->currentIndex());
  eol_protocol_obj->set_eol_com_config_channel(ui->com_config_lineEdit->text());
  eol_protocol_obj->set_eol_vcom_config_channel(ui->vcom_config_lineEdit->text());
  eol_protocol_obj->set_eol_dev_addr(ui->dev_addr_lineEdit->text());

  /* 设置eol协议栈 */
  eol_sub_window_obj->set_eol_protocol_obj(eol_protocol_obj);

  /* 设置eol协议栈 */
  eol_calibration_window_obj->set_eol_protocol_obj(eol_protocol_obj);

  /* 设置eol协议栈 */
  eol_2dfft_calibration_window_obj->set_eol_protocol_obj(eol_protocol_obj);

  /* 恢复参数 */
  read_cfg();
}

void eol_window::rts_protocol_init(network_driver_model *network_driver_send_obj, network_driver_model *network_driver_rec_obj)
{
  rts_network_driver_send_obj = network_driver_send_obj;
  rts_network_driver_rec_obj = network_driver_rec_obj;

  /* rts协议栈初始化 */
  if(nullptr == rts_protocol_obj)
  {
    rts_protocol_obj = new rts_protocol(this);
  }

  /* 禁止线程完成后执行析构对象 */
  rts_protocol_obj->setAutoDelete(false);

  /* 设置rts通讯地址 */
  rts_protocol_obj->set_network_device(rts_network_driver_send_obj, rts_network_driver_rec_obj, ui->rts_addr_lineEdit->text());

  connect(rts_protocol_obj, &rts_protocol::signal_protocol_error_occur, this, &eol_window::slot_rts_protocol_error_occur);
  connect(rts_protocol_obj, &rts_protocol::signal_protocol_timeout, this, &eol_window::slot_rts_protocol_timeout);
  connect(rts_protocol_obj, &rts_protocol::signal_protocol_rw_err, this, &eol_window::slot_rts_protocol_rw_err);
  connect(rts_protocol_obj, &rts_protocol::signal_protocol_rw_ok, this, &eol_window::slot_rts_protocol_rw_ok);
}

void eol_window::timer_init()
{
  timer_obj = new QTimer(this);
  connect(timer_obj, &QTimer::timeout, this, &eol_window::slot_timeout);
  timer_obj->setInterval(1000);
}


void eol_window::save_cfg()
{
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  /* com hw */
  setting.setValue("eol_window_v" CONFIG_VER_STR "/eol_com_hw", ui->com_hw_comboBox->currentIndex());
  /* com chnannel */
  setting.setValue("eol_window_v" CONFIG_VER_STR "/eol_com_channel", ui->com_config_lineEdit->text());
  setting.setValue("eol_window_v" CONFIG_VER_STR "/eol_vcom_channel", ui->vcom_config_lineEdit->text());
  setting.setValue("eol_window_v" CONFIG_VER_STR "/eol_dev_addr", ui->dev_addr_lineEdit->text());
  /* rts */
  setting.setValue("eol_window_v" CONFIG_VER_STR "/rts_dev_addr", ui->rts_addr_lineEdit->text());
  /* plc */
  setting.setValue("eol_window_v" CONFIG_VER_STR "/plc_dev_addr", ui->plc_addr_lineEdit->text());
  setting.sync();
}

void eol_window::read_cfg()
{
  QFile file("./eol_tool_cfg.ini");
  if(false == file.exists())
  {
    return;
  }
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  if(false == setting.contains("eol_window_v" CONFIG_VER_STR "/eol_com_hw"))
  {
    qDebug() << "err eol_window config not exist";
    return;
  }
  /* com hw */
  ui->com_hw_comboBox->setCurrentIndex(setting.value("eol_window_v" CONFIG_VER_STR "/eol_com_hw").toInt());
  /* com chnannel */
  ui->com_config_lineEdit->setText(setting.value("eol_window_v" CONFIG_VER_STR "/eol_com_channel").toString());
  ui->vcom_config_lineEdit->setText(setting.value("eol_window_v" CONFIG_VER_STR "/eol_vcom_channel").toString());
  ui->dev_addr_lineEdit->setText(setting.value("eol_window_v" CONFIG_VER_STR "/eol_dev_addr").toString());

  /* rts */
  ui->rts_addr_lineEdit->setText(setting.value("eol_window_v" CONFIG_VER_STR "/rts_dev_addr").toString());
  /* plc */
  ui->plc_addr_lineEdit->setText(setting.value("eol_window_v" CONFIG_VER_STR "/plc_dev_addr").toString());
  setting.sync();
}

void eol_window::slot_show_this_window()
{
  this->show();
  if(true == this->isMinimized())
  {
    this->showNormal();
  }
  this->activateWindow();
}

void eol_window::slot_close_shell_window()
{
  /* shell窗口关闭停止监听停止eol协议 */
  eol_protocol_obj->stop_task();
}

void eol_window::slot_send_command(QString text)
{
  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 发送shell命令 */
  task.reg = EOL_RW_SHELL_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  quint16 len = (quint16)text.toStdString().size();
  len = len > sizeof(task.buf) ? sizeof(task.buf) : len;
  memcpy_s(task.buf, sizeof(task.buf), text.toStdString().data(), text.toStdString().size());
  task.len = len;
  eol_protocol_obj->eol_master_common_rw_device(task);
  eol_protocol_obj->start_task(true);
}

void eol_window::slot_send_command_char(char c)
{
  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 发送shell命令 */
  task.reg = EOL_RW_SHELL_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  task.buf[0] = c;
  task.len = 1;
  eol_protocol_obj->eol_master_common_rw_device(task, false);

  /* 启动eol协议栈并监听 */
  eol_protocol_obj->start_task(true);
}

void eol_window::slot_timeout()
{
  if(run_state == false && true == all_table_file_name.isEmpty())
  {
    timer_obj->stop();
  }

  time_cnt++;
  ui->time_cnt_val_label->setNum((int)time_cnt);
}

void eol_window::slot_update_show_table_list()
{
  update_show_table_list();
}

void eol_window::update_show_table_list()
{
  ui->transfer_list_val_label->clear();
  QString show_info;
  show_info.clear();
  TABLE_LIST_Typedef_t table;
  CSV_INFO_Typedef_t csv;
  for(qint32 i = 0; i < csv_list.size(); i++)
  {
    csv = csv_list.value(i);
    for(qint32 y = 0 ;y < csv.table_list.size(); y++)
    {
      show_info += csv.table_list.value(y).show_info;
    }
  }
  ui->transfer_list_val_label->setText(show_info);
}

bool eol_window::csv_data_analysis(QByteArray &data, quint64 line_num, int table_type_index, int data_type)
{
  Q_UNUSED(line_num)
  num_str_list = QString(data).split(",");
  if(num_str_list.isEmpty())
  {
    return false;
  }

  /* 申请内存 */
  if(nullptr != num_buf)
  {
    delete[] num_buf;
    num_buf = nullptr;
  }
  num_buf = new uint8_t[num_str_list.size() * utility::num_type_to_bytes((utility::NUM_TYPE_Typedef_t)data_type)];
  if(nullptr == num_buf)
  {
    return false;
  }

  switch((eol_protocol::TABLE_CLASS_Typedef_t)table_type_index)
  {
    /* 方位导向矢量表数据传输 */
    case eol_protocol::SV_AZIMUTH_TABLE:
      {
        quint8 unit_byets;
        quint32 num = utility::str2num(&num_buf[common_table_info.Common_Info.Data_Size], num_str_list, (utility::NUM_TYPE_Typedef_t)data_type, &unit_byets);
        common_table_info.Common_Info.Data_Size += (num * unit_byets);
        qDebug() << "Data_Size" << common_table_info.Common_Info.Data_Size << "num" << num;
        break;
      }

    /* 俯仰导向矢量表数据传输 */
    case eol_protocol::SV_ELEVATION_TABLE:
      {
        quint8 unit_byets;
        quint32 num = utility::str2num(&num_buf[common_table_info.Common_Info.Data_Size], num_str_list, (utility::NUM_TYPE_Typedef_t)data_type, &unit_byets);
        common_table_info.Common_Info.Data_Size += (num * unit_byets);
        qDebug() << "SV_ELEVATION_TABLE Data_Size" << common_table_info.Common_Info.Data_Size << "num" << num;
        break;
      }
      /* 天线间距坐标信息表 */
      /* 天线初相信息表 */
      /* 天线间距坐标与初相信息表，双表合并 */
      //    case eol_protocol::DAA_ANT_POS_TABLE:
      //    case eol_protocol::DOA_PHASE_COMPS_TABLE:
    /* 天线间距 */
    /* 通道补偿 */
    case eol_protocol::ANT_BOTH_TABLE:
      {
        quint8 unit_byets;
        quint32 num = utility::str2num(&num_buf[common_table_info.Common_Info.Data_Size], num_str_list, (utility::NUM_TYPE_Typedef_t)data_type, &unit_byets);
        common_table_info.Common_Info.Data_Size += (num * unit_byets);
        qDebug() << "ANT_BOTH_TABLE Data_Size " << common_table_info.Common_Info.Data_Size << "num" << num;
        break;
      }

    /* 通道补偿 */
    case eol_protocol::PATTERN_TABLE:
      {
        quint8 unit_byets;
        quint32 num = utility::str2num(&num_buf[common_table_info.Common_Info.Data_Size], num_str_list, (utility::NUM_TYPE_Typedef_t)data_type, &unit_byets);
        common_table_info.Common_Info.Data_Size += (num * unit_byets);
        qDebug() << "PATTERN_TABLE Data_Size " << common_table_info.Common_Info.Data_Size << "num" << num;
        break;
      }

    /* 通道底噪 */
    case eol_protocol::BACKGROUND_NOISE_TABLE:
      {
        quint8 unit_byets;
        quint32 num = utility::str2num(&num_buf[common_table_info.Common_Info.Data_Size], num_str_list, (utility::NUM_TYPE_Typedef_t)data_type, &unit_byets);
        common_table_info.Common_Info.Data_Size += (num * unit_byets);
        qDebug() << "BACKGROUND_NOISE_TABLE Data_Size " << common_table_info.Common_Info.Data_Size << "num" << num;
        break;
      }

    default:
      qDebug() << "unknow table type";
      return false;
  }
  return true;
}

/* 解析csv头 */
eol_protocol::TABLE_Typedef_t eol_window::csv_header_analysis(QByteArray &data, int table_type_index)
{
  num_str_list = QString(data).split(",");
  if(num_str_list.isEmpty())
  {
    return eol_protocol::UNKNOW_TABLE;
  }

  /* 如果类型不确定则先解析表类型 */
  if(0xFF == table_type_index)
  {
    table_type_index = num_str_list.value(0).toUShort();
    qDebug() << "read header type" << table_type_index;
  }

  if(num_str_list.size() < 5)
  {
    return eol_protocol::UNKNOW_TABLE;
  }

  common_table_info.Common_Info.Class_ID_Num = 0x66;
  common_table_info.Common_Info.Version_MAJOR = (quint8)(num_str_list.value(1).toUInt() & 0xFF);
  common_table_info.Common_Info.Version_MINOR = (quint8)((num_str_list.value(1).toUInt() >> 8) & 0xFF);
  common_table_info.Common_Info.Version_REVISION = (quint8)(num_str_list.value(1).toUInt() >> 16) & 0xFF;
  common_table_info.Common_Info.Data_Type = (eol_protocol::DATA_Typedef_t)num_str_list.value(2).toUShort();
  common_table_info.Common_Info.Data_Size = 0;
  common_table_info.Common_Info.Crc_Val = num_str_list.value(4).toUInt();
  quint8 profile_id = 0;
  switch((eol_protocol::TABLE_CLASS_Typedef_t)table_type_index)
  {
    /* 方位导向矢量表 */
    /* 俯仰导向矢量表 */
    case eol_protocol::SV_AZIMUTH_TABLE:
    case eol_protocol::SV_ELEVATION_TABLE:
    case eol_protocol::SV_ELEVATION_AZI_N45_TABLE:
    case eol_protocol::SV_ELEVATION_AZI_P45_TABLE:
      {
        /* table class, version, data type, data size, data crc, points, channel num,
         start angle*10, end angle*10, ele angle*10, tx_order, profile_id, check sum */
        qDebug() << num_str_list.mid(0, 13);
        if(num_str_list.size() < 13)
        {
          return eol_protocol::UNKNOW_TABLE;
        }

        /* 校验表头信息 */
        qint64 check_sum = 0;
        quint8 i = 0;
        for(;i < 12; i++)
        {
          check_sum += num_str_list.value(i).toLongLong();
        }
        if(check_sum != num_str_list.value(i).toLongLong())
        {
          qDebug() << "check_sum err expectance " << check_sum << " ? " << num_str_list.value(i).toLongLong();
          return eol_protocol::UNKNOW_TABLE;
        }

        /* 私有头 */
        table_info.Points = num_str_list.value(5).toUShort();
        table_info.Channel_Num = (quint8)num_str_list.value(6).toUShort();
        table_info.Start_Angle = (float)num_str_list.value(7).toShort() / 10;
        table_info.End_Angle = (float)num_str_list.value(8).toShort() / 10;
        table_info.Azi_Ele_Angle = (float)num_str_list.value(9).toShort() / 10;
        quint32 tx_order = num_str_list.value(10).toUInt();
        table_info.Clibration_Tx_Order[0] = (quint8)tx_order;
        table_info.Clibration_Tx_Order[1] = (quint8)(tx_order >> 8);
        table_info.Clibration_Tx_Order[2] = (quint8)(tx_order >> 16);
        table_info.Clibration_Tx_Order[3] = (quint8)(tx_order >> 24);
        profile_id = table_info.Profile_ID = (quint8)num_str_list.value(11).toUShort();

        /* 私有头大小 */
        common_table_info.Common_Info.Header_Size = sizeof(table_info.Start_Angle) + \
                                                    sizeof(table_info.End_Angle) + \
                                                    sizeof(table_info.Channel_Num) + \
                                                    sizeof(table_info.Points) + \
                                                    sizeof(table_info.Azi_Ele_Angle) + \
                                                    sizeof(table_info.Clibration_Tx_Order) + \
                                                    sizeof(table_info.Profile_ID);
        quint32 index = 0;
        memcpy(common_table_info.private_header + index, &table_info.Start_Angle, sizeof(table_info.Start_Angle));
        index += sizeof(table_info.Start_Angle);
        memcpy(common_table_info.private_header + index, &table_info.End_Angle, sizeof(table_info.End_Angle));
        index += sizeof(table_info.End_Angle);
        memcpy(common_table_info.private_header + index, &table_info.Points, sizeof(table_info.Points));
        index += sizeof(table_info.Points);
        memcpy(common_table_info.private_header + index, &table_info.Channel_Num, sizeof(table_info.Channel_Num));
        index += sizeof(table_info.Channel_Num);
        memcpy(common_table_info.private_header + index, &table_info.Azi_Ele_Angle, sizeof(table_info.Azi_Ele_Angle));
        index += sizeof(table_info.Azi_Ele_Angle);
        memcpy(common_table_info.private_header + index, &table_info.Clibration_Tx_Order[0], sizeof(table_info.Clibration_Tx_Order));
        index += sizeof(table_info.Clibration_Tx_Order);
        memcpy(common_table_info.private_header + index, &table_info.Profile_ID, sizeof(table_info.Profile_ID));
        //      index += sizeof(table_info.Profile_ID);
        break;
      }

    /* 天线信息表 */
    case eol_protocol::ANT_BOTH_TABLE:
      {
        /* table class, version, data type, data size, data crc, points, channel num, tx_order, profile_id, check sum */
        qDebug() << num_str_list.mid(0, 10);
        if(num_str_list.size() < 10)
        {
          qDebug() << "num_str_list.size()" << num_str_list.size() << "< 10";
          return eol_protocol::UNKNOW_TABLE;
        }

        /* 校验表头信息 */
        qint64 check_sum = 0;
        quint8 i = 0;
        for(; i < 9; i++)
        {
          check_sum += num_str_list.value(i).toLongLong();
        }
        if(check_sum != num_str_list.value(i).toLongLong())
        {
          qDebug() << "check_sum err expectance " << check_sum << " ? " << num_str_list.value(i).toLongLong();
          return eol_protocol::UNKNOW_TABLE;
        }

        /* 私有头 */
        ant_table_info.Points = num_str_list.value(5).toUShort();
        ant_table_info.Channel_Num = (quint8)num_str_list.value(6).toUShort();
        quint32 tx_order = num_str_list.value(7).toUInt();
        ant_table_info.Clibration_Tx_Order[0] = (quint8)tx_order;
        ant_table_info.Clibration_Tx_Order[1] = (quint8)(tx_order >> 8);
        ant_table_info.Clibration_Tx_Order[2] = (quint8)(tx_order >> 16);
        ant_table_info.Clibration_Tx_Order[3] = (quint8)(tx_order >> 24);
        profile_id = ant_table_info.Profile_ID = (quint8)num_str_list.value(8).toUShort();

        /* 私有头大小 */
        common_table_info.Common_Info.Header_Size = sizeof(ant_table_info.Channel_Num) + \
                                                    sizeof(ant_table_info.Points) + \
                                                    sizeof(ant_table_info.Clibration_Tx_Order) + \
                                                    sizeof(ant_table_info.Profile_ID);
        quint32 index = 0;
        memcpy(common_table_info.private_header + index, &ant_table_info.Points, sizeof(ant_table_info.Points));
        index += sizeof(ant_table_info.Points);
        memcpy(common_table_info.private_header + index, &ant_table_info.Channel_Num, sizeof(ant_table_info.Channel_Num));
        index += sizeof(ant_table_info.Channel_Num);
        memcpy(common_table_info.private_header + index, &ant_table_info.Clibration_Tx_Order[0], sizeof(ant_table_info.Clibration_Tx_Order));
        index += sizeof(ant_table_info.Clibration_Tx_Order);
        memcpy(common_table_info.private_header + index, &ant_table_info.Profile_ID, sizeof(ant_table_info.Profile_ID));
        //      index += sizeof(ant_table_info.Profile_ID);
        break;
      }

    /* 方向表 */
    case eol_protocol::PATTERN_TABLE:
      {
        /* table class, version, data type, data size, data crc,
       * points, channel num, start angle*10, end angle*10, unit,
       * tx_order, profile_id, check sum */
        qDebug() << num_str_list.mid(0, 13);
        if(num_str_list.size() < 13)
        {
          return eol_protocol::UNKNOW_TABLE;
        }

        /* 校验表头信息 */
        qint64 check_sum = 0;
        quint8 i = 0;
        for(; i < 12; i++)
        {
          check_sum += num_str_list.value(i).toLongLong();
        }
        if(check_sum != num_str_list.value(i).toLongLong())
        {
          qDebug() << "check_sum err expectance " << check_sum << " ? " << num_str_list.value(i).toLongLong();
          return eol_protocol::UNKNOW_TABLE;
        }

        /* 私有头 */
        pattern_table_info.Points = num_str_list.value(5).toUShort();
        pattern_table_info.Channel_Num = (quint8)num_str_list.value(6).toUShort();
        pattern_table_info.Start_Angle = (float)num_str_list.value(7).toShort() / 10;
        pattern_table_info.End_Angle = (float)num_str_list.value(8).toShort() / 10;
        pattern_table_info.Unit = (quint8)num_str_list.value(9).toUShort();
        quint32 tx_order = num_str_list.value(10).toUInt();
        pattern_table_info.Clibration_Tx_Order[0] = (quint8)tx_order;
        pattern_table_info.Clibration_Tx_Order[1] = (quint8)(tx_order >> 8);
        pattern_table_info.Clibration_Tx_Order[2] = (quint8)(tx_order >> 16);
        pattern_table_info.Clibration_Tx_Order[3] = (quint8)(tx_order >> 24);
        profile_id = pattern_table_info.Profile_ID = (quint8)num_str_list.value(11).toUShort();

        /* 私有头大小 */
        common_table_info.Common_Info.Header_Size = sizeof(pattern_table_info.Start_Angle) + \
                                                    sizeof(pattern_table_info.End_Angle) + \
                                                    sizeof(pattern_table_info.Channel_Num) + \
                                                    sizeof(pattern_table_info.Points) + \
                                                    sizeof(pattern_table_info.Unit) + \
                                                    sizeof(pattern_table_info.Clibration_Tx_Order) + \
                                                    sizeof(pattern_table_info.Profile_ID);
        quint32 index = 0;
        memcpy(common_table_info.private_header + index, &pattern_table_info.Start_Angle, sizeof(pattern_table_info.Start_Angle));
        index += sizeof(pattern_table_info.Start_Angle);
        memcpy(common_table_info.private_header + index, &pattern_table_info.End_Angle, sizeof(pattern_table_info.End_Angle));
        index += sizeof(pattern_table_info.End_Angle);
        memcpy(common_table_info.private_header + index, &pattern_table_info.Points, sizeof(pattern_table_info.Points));
        index += sizeof(pattern_table_info.Points);
        memcpy(common_table_info.private_header + index, &pattern_table_info.Channel_Num, sizeof(pattern_table_info.Channel_Num));
        index += sizeof(pattern_table_info.Channel_Num);
        memcpy(common_table_info.private_header + index, &pattern_table_info.Unit, sizeof(pattern_table_info.Unit));
        index += sizeof(pattern_table_info.Unit);
        memcpy(common_table_info.private_header + index, &pattern_table_info.Clibration_Tx_Order[0], sizeof(pattern_table_info.Clibration_Tx_Order));
        index += sizeof(pattern_table_info.Clibration_Tx_Order);
        memcpy(common_table_info.private_header + index, &pattern_table_info.Profile_ID, sizeof(pattern_table_info.Profile_ID));
        //      index += sizeof(pattern_table_info.Profile_ID);
        break;
      }

    /* 底噪表 */
    case eol_protocol::BACKGROUND_NOISE_TABLE:
      {
        /* table class, version, data type, data size, data crc,
       * channel num0, channel num1, channel num2, channel num3,
       * unit, tx_order0, tx_order1, tx_order2, tx_order3, check sum */
        qDebug() << num_str_list.mid(0, 15);
        if(num_str_list.size() < 15)
        {
          return eol_protocol::UNKNOW_TABLE;
        }

        /* 校验表头信息 */
        qint64 check_sum = 0;
        quint8 i = 0;
        for(; i < 12; i++)
        {
          check_sum += num_str_list.value(i).toLongLong();
        }
        if(check_sum != num_str_list.value(i).toLongLong())
        {
          qDebug() << "check_sum err expectance " << check_sum << " ? " << num_str_list.value(i).toLongLong();
          return eol_protocol::UNKNOW_TABLE;
        }

        /* 私有头 */
        noise_table_info.Channel_Num[0] = (quint8)num_str_list.value(5).toUShort();
        noise_table_info.Channel_Num[1] = (quint8)num_str_list.value(6).toUShort();
        noise_table_info.Channel_Num[2] = (quint8)num_str_list.value(7).toUShort();
        noise_table_info.Channel_Num[3] = (quint8)num_str_list.value(8).toUShort();
        noise_table_info.Unit = (quint8)num_str_list.value(9).toUShort();

        quint32 tx_order = num_str_list.value(10).toUInt();
        noise_table_info.Clibration_Tx_Order[0][0] = (quint8)tx_order;
        noise_table_info.Clibration_Tx_Order[0][1] = (quint8)(tx_order >> 8);
        noise_table_info.Clibration_Tx_Order[0][2] = (quint8)(tx_order >> 16);
        noise_table_info.Clibration_Tx_Order[0][3] = (quint8)(tx_order >> 24);
        tx_order = num_str_list.value(11).toUInt();
        noise_table_info.Clibration_Tx_Order[1][0] = (quint8)tx_order;
        noise_table_info.Clibration_Tx_Order[1][1] = (quint8)(tx_order >> 8);
        noise_table_info.Clibration_Tx_Order[1][2] = (quint8)(tx_order >> 16);
        noise_table_info.Clibration_Tx_Order[1][3] = (quint8)(tx_order >> 24);
        tx_order = num_str_list.value(12).toUInt();
        noise_table_info.Clibration_Tx_Order[2][0] = (quint8)tx_order;
        noise_table_info.Clibration_Tx_Order[2][1] = (quint8)(tx_order >> 8);
        noise_table_info.Clibration_Tx_Order[2][2] = (quint8)(tx_order >> 16);
        noise_table_info.Clibration_Tx_Order[2][3] = (quint8)(tx_order >> 24);
        tx_order = num_str_list.value(13).toUInt();
        noise_table_info.Clibration_Tx_Order[3][0] = (quint8)tx_order;
        noise_table_info.Clibration_Tx_Order[3][1] = (quint8)(tx_order >> 8);
        noise_table_info.Clibration_Tx_Order[3][2] = (quint8)(tx_order >> 16);
        noise_table_info.Clibration_Tx_Order[3][3] = (quint8)(tx_order >> 24);

        /* 私有头大小 */
        common_table_info.Common_Info.Header_Size = sizeof(noise_table_info.Channel_Num) + \
                                                    sizeof(noise_table_info.Unit) + \
                                                    sizeof(noise_table_info.Clibration_Tx_Order);

        quint32 index = 0;
        memcpy(common_table_info.private_header + index, &noise_table_info.Channel_Num[0], sizeof(noise_table_info.Channel_Num));
        index += sizeof(noise_table_info.Channel_Num);
        memcpy(common_table_info.private_header + index, &noise_table_info.Unit, sizeof(noise_table_info.Unit));
        index += sizeof(noise_table_info.Unit);
        memcpy(common_table_info.private_header + index, &noise_table_info.Clibration_Tx_Order[0][0], sizeof(noise_table_info.Clibration_Tx_Order));
        //      index += sizeof(noise_table_info.Clibration_Tx_Order);
        break;
      }

    default:
      break;
  }
  common_table_info.Common_Info.Table_Type = eol_protocol::get_table_type(profile_id, (eol_protocol::TABLE_CLASS_Typedef_t)table_type_index);
  return (eol_protocol::TABLE_Typedef_t)table_type_index;
}

/* 解析csv文件 */
bool eol_window::csv_analysis(QString &file_path, CSV_INFO_Typedef_t &csv, int csv_file_index, bool check_table_en)
{
  Q_UNUSED(csv_file_index)
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

  CSV_DECODE_STEP_Typedef_t csv_decode_step = CSV_HEADER_COMMENT_DECODE;
  /* 解析头部 */
  eol_protocol::TABLE_Typedef_t table_type_index;
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
    else
    {
      continue;
    }

    /* 头部 */
    if(1 == line_num)
    {
      csv_decode_step = CSV_HEADER_COMMENT_DECODE;
    }

    switch(csv_decode_step)
    {
      case CSV_HEADER_COMMENT_DECODE:
        {
          /* 清空记录表 */
          memset(&common_table_info, 0, sizeof(common_table_info));
          if(true == QString(line_data).split(",").value(0).contains("table "))
          {
            qDebug() << "find header :" << QString(line_data);
            csv_decode_step = CSV_HEADER_DATA_DECODE;
          }
          continue;
        }
        break;

      case CSV_HEADER_DATA_DECODE:
        {
          table_type_index = csv_header_analysis(line_data);
          if(eol_protocol::UNKNOW_TABLE == table_type_index)
          {
            qDebug() << "unknow table";
            csv_decode_step = CSV_HEADER_COMMENT_DECODE;
            continue;
          }
          /* 使能检测时不进行数据解析 */
          if(false == check_table_en)
          {
            csv_decode_step = CSV_DATA_DECODE;
          }
          else
          {
            TABLE_LIST_Typedef_t table;
            table.table_type = table_type_index;
            table.show_info = QString("%1 -- table:%2 -- [wait]\r\n").arg(csv.table_list.size() + 1).arg(table_type_index);
            csv.table_list.append(table);
            qDebug() << "add table:" << table.show_info;
            csv_decode_step = CSV_HEADER_COMMENT_DECODE;
          }
          continue;
        }
        break;

      case CSV_DATA_DECODE:
        {
          if(eol_protocol::UNKNOW_TABLE == table_type_index)
          {
            csv_decode_step = CSV_HEADER_COMMENT_DECODE;
            continue;
          }
          /* 解析数据 */
          if(false == csv_data_analysis(line_data, line_num, table_type_index, common_table_info.Common_Info.Data_Type))
          {
            csv_decode_step = CSV_HEADER_COMMENT_DECODE;

            QString str;
            TABLE_LIST_Typedef_t table;
            /* 找到当前发送的表 */
            int i = 0;
            for(; i < csv.table_list.size(); i++)
            {
              table = csv.table_list.value(i);
              if(table.table_type == table_type_index)
              {
                break;
              }
            }

            /* 更新为error状态 */
            str = table.show_info.replace("\r\n", " -- [check err]\r\n");
            table.show_info = str;
            csv.table_list.replace(i, table);
            csv_list.replace(csv_file_index, csv);

            /* 更新显示列表 */
            emit signal_update_show_table_list();

            continue;
          }
          qDebug() << "table data send task start";
          csv_decode_step = CSV_HEADER_COMMENT_DECODE;

          /* 添加到eol发送数据任务 */
          bool ret = eol_protocol_obj->eol_master_send_table_data(common_table_info, (const quint8 *)num_buf);
          if(false == ret)
          {
            qDebug() << "add task error";
            SET_CURRENT_TASK_RUN_STATE(TASK_ERROR);
            continue;
          }

          /* 设置当前任务状态 */
          SET_CURRENT_TASK_RUN_STATE(TASK_RUNNING);
          while(current_task_complete_state == TASK_RUNNING && run_state)
          {
            /* 启动发送数据线程 */
            eol_protocol_obj->start_task();
            QThread::msleep(1);
          }

          /* 释放 */
          if(nullptr != num_buf)
          {
            delete[] num_buf;
            num_buf = nullptr;
          }

          QString str;
          TABLE_LIST_Typedef_t table;
          /* 找到当前发送的表 */
          int i = 0;
          for(; i < csv.table_list.size(); i++)
          {
            table = csv.table_list.value(i);
            if(table.table_type == table_type_index)
            {
              break;
            }
          }

          if(TASK_COMPLETE == current_task_complete_state)
          {
            /* 更新为ok状态 */
            str = table.show_info.replace("\r\n", " -- [ok]\r\n");
          }
          else
          {
            /* 更新为error状态 */
            str = table.show_info.replace("\r\n", " -- [err]\r\n");
          }

          table.show_info = str;
          csv.table_list.replace(i, table);
          csv_list.replace(csv_file_index, csv);

          /* 更新显示列表 */
          emit signal_update_show_table_list();
          continue;
        }

        break;
      default:
        break;
    }
  }

  return true;
}

/**
 * @brief eol_window::执行eol文件解析任务
 */
void eol_window::run_eol_window_file_decode_task()
{
  /* csv列表循环 */
  for(int i = 0; i < csv_list.size(); i++)
  {
    CSV_INFO_Typedef_t csv = csv_list.value(i);

    /* 移除状态 */
    QString str;
    for(int y = 0; y < csv.table_list.size(); y++)
    {
      TABLE_LIST_Typedef_t table = csv.table_list.value(y);

      /* 移除上一次的状态 */
      str = table.show_info.replace(" -- [ok]", "");
      str = table.show_info.replace(" -- [err]", "");
      str = table.show_info.replace(" -- [check err]", "");

      table.show_info = str;
      csv.table_list.replace(y, table);
      csv_list.replace(i, csv);
    }
  }

  /* 更新显示列表 */
  emit signal_update_show_table_list();

  /* csv列表循环 */
  for(int i = 0; i < csv_list.size(); i++)
  {
    CSV_INFO_Typedef_t csv = csv_list.value(i);

    /* 设置传输的文件 */
    current_file_path = csv.file_path;

    /* 解析csv文件 */
    csv_analysis(current_file_path, csv, i);

    /* 传输完毕关闭 */
    if(current_file.isOpen() == true)
    {
      current_file.close();
    }
  }

  /* 任务结束 */
  run_state = false;

  /* 本按钮可用 */
  ui->update_pushButton->setEnabled(true);
  ui->add_list_pushButton->setEnabled(true);
  ui->export_all_pushButton->setEnabled(true);
}

void eol_window::run_eol_window_export_one_table_task()
{
  /* 启动线程 */
  eol_protocol::TABLE_Typedef_t table_type = (eol_protocol::TABLE_Typedef_t)ui->table_type_comboBox->currentIndex();
  eol_protocol_obj->eol_master_get_table_data(table_type);

  /* 设置当前任务状态 */
  SET_CURRENT_TASK_RUN_STATE(TASK_RUNNING);
  while(current_task_complete_state == TASK_RUNNING && run_state)
  {
    /* 启动接收数据线程 */
    eol_protocol_obj->start_task();
    QThread::msleep(1);
  }

  /* 等待任务结束 */
  run_state = false;
}

void eol_window::run_eol_window_export_task()
{
  /* 检测导出文件名是否为空 */
  if(true == all_table_file_name.isEmpty())
  {
    return;
  }

  /* 先关闭 */
  if(all_table_file.isOpen() == true)
  {
    all_table_file.close();
  }

  /* 关联文件名 */
  all_table_file.setFileName(all_table_file_name);

  /* 打开文件，只写方式 */
  if(all_table_file.open(QIODevice::WriteOnly) == false)
  {
    goto _end_task;
  }

  /* 启动线程 */
  /* 轮询所有表导出 */
  for(quint8 i = 0; i < (quint8)eol_protocol::TABLE_TYPE_NUM_MAX; i++)
  {
    eol_protocol::TABLE_Typedef_t table_type = (eol_protocol::TABLE_Typedef_t)i;
    eol_protocol_obj->eol_master_get_table_data(table_type);

    /* 设置当前任务状态 */
    SET_CURRENT_TASK_RUN_STATE(TASK_RUNNING);
    while(current_task_complete_state == TASK_RUNNING && run_state)
    {
      /* 启动接收数据线程 */
      eol_protocol_obj->start_task();
      QThread::msleep(1);
    }
  }

_end_task:

  /* 任务结束 */
  run_state = false;

  /* 清除文件名 */
  all_table_file_name.clear();

  /* 关闭 */
  if(all_table_file.isOpen() == true)
  {
    all_table_file.close();
  }

  /* 任务完成 */
  emit signal_upload_all_table_task_ok();
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
    //    current_file_name.replace(QChar(' '), QChar('_'));

    /* 设置待写入表信息 */
    if(csv_list.size() == 0)
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

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

void eol_window::on_upload_pushButton_clicked()
{
  /* 清空更新表 */
  csv_list.clear();

  /* 清空一键导出 */
  all_table_file_name.clear();

  /* 更新显示列表 */
  update_show_table_list();

  /* 重置界面 */
  reset_base_ui_info();

  /* 重置错误统计 */
  err_constantly_cnt = 0;

  /* 重置计时 */
  time_cnt = 0;

  /* 本按钮不可用 */
  ui->upload_pushButton->setEnabled(false);
  ui->add_list_pushButton->setEnabled(false);
  ui->export_all_pushButton->setEnabled(false);

  /* 设置线程任务 */
  current_running_task = UPLOAD_TABLE_TASK;

  /* 启动线程 */
  run_state = true;

  /* 原子操作 */
  if(thread_run_statex.testAndSetRelaxed(0, 1))
  {
    /* 启动任务线程 */
    g_thread_pool->start(this);
  }
  else
  {
    qDebug() << "eol window task is running";
    return;
  }

  /* 启动计时 */
  timer_obj->start();
}

void eol_window::on_update_pushButton_clicked()
{
  /* 传输列表检测 */
  if(csv_list.isEmpty())
  {
    return;
  }

  /* 重置错误统计 */
  err_constantly_cnt = 0;

  /* 重置计时 */
  time_cnt = 0;

  /* 重置界面 */
  reset_base_ui_info();

  /* 设置线程任务 */
  current_running_task = UPDATE_TABLE_TASK;

  /* 启动传输 */
  run_state = true;

  /* 原子操作 */
  if(thread_run_statex.testAndSetRelaxed(0, 1))
  {
    /* 启动任务线程 */
    g_thread_pool->start(this);
  }
  else
  {
    qDebug() << "eol window task is running";
    return;
  }

  /* 启动计时 */
  timer_obj->start();

  /* 本按钮不可用 */
  ui->update_pushButton->setEnabled(false);
  ui->add_list_pushButton->setEnabled(false);
  ui->export_all_pushButton->setEnabled(false);
}

void eol_window::on_add_list_pushButton_clicked()
{
  if(current_file_path.isEmpty())
  {
    qDebug() << "file path is empty";
    return;
  }

  CSV_INFO_Typedef_t csv;
  /* 查重 */
  for(qint32 i = 0; i < csv_list.size(); i++)
  {
    if(current_file_path == csv_list.value(i).file_path)
    {
      qDebug() << "file path is existed in list";
      return;
    }
  }

  /* 显示到列表区域 */
  csv.file_name = current_file_name;
  csv.file_path = current_file_path;

  /* 解析csv文件传输列表 */
  csv_analysis(current_file_path, csv, 0, true);

  /* 解析完毕关闭 */
  if(current_file.isOpen() == true)
  {
    current_file.close();
  }

  /* 加入csv文件列表 */
  csv_list.append(csv);

  /* 更新显示列表 */
  update_show_table_list();

  /* 发送按钮可用 */
  ui->update_pushButton->setEnabled(true);
}

void eol_window::on_clear_list_pushButton_clicked()
{
  /* 清空传输列表 */
  for(qint32 i = 0; i < csv_list.size(); i++)
  {
    csv_list.value(i).table_list.clear();
  }
  csv_list.clear();
  ui->transfer_list_val_label->clear();
}

/**
 * @brief 从机无应答超时时间
 */
void eol_window::slot_protocol_timeout(quint32 sec)
{
  Q_UNUSED(sec)
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
    eol_protocol_obj->stop_task();

    /* 设置错误状态 */
    SET_CURRENT_TASK_RUN_STATE(TASK_ERROR);

    /* 本按钮可用 */
    if(csv_list.size() > 0)
    {
      ui->update_pushButton->setEnabled(true);
    }
    else
    {
      ui->update_pushButton->setEnabled(false);
    }
    ui->upload_pushButton->setEnabled(true);
    ui->add_list_pushButton->setEnabled(true);
    ui->export_all_pushButton->setEnabled(true);
  }
}

/**
 * @brief crc检测到错误
 */
void eol_window::slot_protocol_crc_check_failed()
{
  ui->msg_str_label->setText("CRC Check Failed");
}

/* 错误码解析 */
void eol_window::slot_protocol_error_occur(quint8 error_msg)
{
  /* 显示错误消息 */
  qDebug() << "ack msg:" << error_msg;
  QString msg;
  switch(error_msg)
  {
    case eol_protocol::EOL_OPT_OK:            msg = "OPT_OK";break;
    case eol_protocol::EOL_OPT_CRC_ERR:       msg = "CRC_ERR";break;
    case eol_protocol::EOL_OPT_R_HEADER_ERR:  msg = "R_HEADER_ERR";break;
    case eol_protocol::EOL_OPT_R_DATA_ERR:    msg = "R_DATA_ERR";break;
    case eol_protocol::EOL_OPT_W_HEADER_ERR:  msg = "W_HEADER_ERR";break;
    case eol_protocol::EOL_OPT_W_DATA_ERR:    msg = "W_DATA_ERR";break;
    case eol_protocol::EOL_OPT_HEADER_CRC_ERR:msg = "HEADER_CRC_ERR";break;
    case eol_protocol::EOL_OPT_RW_ERR:        msg = "RW_ERR";break;
    case eol_protocol::EOL_OPT_ERASE_ERR:     msg = "ERASE_ERR";break;
    case eol_protocol::UNKNOW_TAB_ERR:        msg = "UNKNOW_TAB_ERR";break;
    case eol_protocol::EOL_OVER_SIZE:         msg = "TABLE_OVER_SIZE";break;
    case eol_protocol::UNKNOW_CMD_ERR:        msg = "UNKNOW_CMD_ERR";break;
    default:
      break;
  }
  ui->msg_str_label->setText(msg);
}

/**
   * @brief 一键导出所有表
   * @param frame_num 帧计数
   * @param data 数据
   * @return 导出成功
   */
bool eol_window::one_key_rec_all_table_data_silent(quint16 frame_num, const QByteArray &data, bool is_table_header)
{
  if(true == all_table_file_name.isEmpty())
  {
    return false;
  }

  all_table_file.seek(all_table_file.pos());

  if(0U == frame_num && is_table_header && 0 < all_table_file.pos())
  {
    /* 数据尾部添加换行 */
    all_table_file.write("\r\n");
  }

  /* 写入文件 */
  all_table_file.write(data);
  return true;
}

/**
 * @brief 表数据
 * @param frame_num 帧号
 * @param data 数据
 * @param data_len 数据长度
 */
void eol_window::slot_recv_eol_table_data(quint16 frame_num, const quint8 *data,
                                          quint16 data_len)
{
  /* 重置错误统计 */
  err_constantly_cnt = 0;

  /* 0帧为表信息数据 */
  if(0 == frame_num)
  {
    memcpy_s(&common_table_info.Common_Info, sizeof(common_table_info), data, data_len);

    /* 组织表头信息 */
    ui->transfer_progressBar->setMaximum((qint32)common_table_info.Common_Info.Data_Size);

    eol_protocol::TABLE_CLASS_Typedef_t table_class = eol_protocol::get_table_class((eol_protocol::TABLE_Typedef_t)common_table_info.Common_Info.Table_Type);
    quint32 version = common_table_info.Common_Info.Version_REVISION;
    version <<= 8;
    version |= common_table_info.Common_Info.Version_MINOR;
    version <<= 8;
    version |= common_table_info.Common_Info.Version_MAJOR;

    /* 数据个数 */
    quint32 data_num = utility::num_type_to_bytes((utility::NUM_TYPE_Typedef_t)common_table_info.Common_Info.Data_Type);
    data_num = common_table_info.Common_Info.Data_Size / data_num;

    qDebug() << "data size " << common_table_info.Common_Info.Data_Size << " crc " << common_table_info.Common_Info.Crc_Val;
    qint64 check_sum = 0;
    check_sum += (qint64)table_class +
                 version +
                 common_table_info.Common_Info.Data_Type +
                 data_num +
                 common_table_info.Common_Info.Crc_Val;
    QString csv_header;
    QString str;
    str = QString::asprintf("%u,"
                            "%u,"
                            "%u,"
                            "%u,"
                            "%u",
                            table_class,
                            version,
                            common_table_info.Common_Info.Data_Type,
                            data_num,
                            common_table_info.Common_Info.Crc_Val);
    QString tips_str;
    tips_str = QString("<font size='5' color='green'><div align='legt'> Table Type: </div> <div align='right'> %1 </div> </font>\n"
                       "<font size='5' color='orange'><div align='legt'> Ver: </div> <div align='right'> %2.%3.%4 </div> </font>\n"
                       "<font size='5' color='blue'><div align='legt'> Data Type: </div> <div align='right'> %5 </div> </font>\n"
                       "<font size='5' color='red'><div align='legt'> Data Size: </div> <div align='right'> %6 </div> </font>\n")
                   .arg(table_class)
                   .arg(common_table_info.Common_Info.Version_MAJOR)
                   .arg(common_table_info.Common_Info.Version_MINOR)
                   .arg(common_table_info.Common_Info.Version_REVISION)
                   .arg(common_table_info.Common_Info.Data_Type)
                   .arg(common_table_info.Common_Info.Data_Size);

    /* 私有表头解析 */
    quint32 Index = sizeof(common_table_info.Common_Info);
    switch(table_class)
    {
      /* 方位导向矢量表 */
      /* 俯仰导向矢量表 */
      case eol_protocol::SV_AZIMUTH_TABLE:
      case eol_protocol::SV_ELEVATION_TABLE:
      case eol_protocol::SV_ELEVATION_AZI_N45_TABLE:
      case eol_protocol::SV_ELEVATION_AZI_P45_TABLE:
        {
          memcpy_s(&table_info.Common_Info, sizeof(table_info.Common_Info), &common_table_info.Common_Info, Index);
          memcpy(&table_info.Start_Angle, data + Index, sizeof(table_info.Start_Angle));
          Index += sizeof(table_info.Start_Angle);
          memcpy(&table_info.End_Angle, data + Index, sizeof(table_info.End_Angle));
          Index += sizeof(table_info.End_Angle);
          memcpy(&table_info.Points, data + Index, sizeof(table_info.Points));
          Index += sizeof(table_info.Points);
          memcpy(&table_info.Channel_Num, data + Index, sizeof(table_info.Channel_Num));
          Index += sizeof(table_info.Channel_Num);
          memcpy(&table_info.Azi_Ele_Angle, data + Index, sizeof(table_info.Azi_Ele_Angle));
          Index += sizeof(table_info.Azi_Ele_Angle);
          quint32 tx_order = 0;
          memcpy(&tx_order, data + Index, sizeof(tx_order));
          Index += sizeof(tx_order);
          memcpy(&table_info.Profile_ID, data + Index, sizeof(table_info.Profile_ID));
          //        Index += sizeof(table_info.Profile_ID);

          qint32 Start_Angle = (qint32)(table_info.Start_Angle * 10.f);
          qint32 End_Angle = (qint32)(table_info.End_Angle * 10.f);
          qint32 Azi_Ele_Angle = (qint32)(table_info.Azi_Ele_Angle * 10.f);
          check_sum += Start_Angle + End_Angle + Azi_Ele_Angle + table_info.Points + table_info.Channel_Num + tx_order + table_info.Profile_ID;
          csv_header = tr("table class, version, data type, data size, data crc, points, channel num, start angle*10, end angle*10, ele angle*10, tx_order, profile_id, check sum\r\n");
          str += QString::asprintf(",%u,%u,%d,%d,%d,%u,%u,"
                                   "%lld\r\n",
                                   table_info.Points,
                                   table_info.Channel_Num,
                                   Start_Angle,
                                   End_Angle,
                                   Azi_Ele_Angle,
                                   tx_order,
                                   table_info.Profile_ID,
                                   check_sum);

          tips_str += QString("<font size='5' color='black'><div align='legt'> Channel Num: </div> <div align='right'> %1 </div> </font>\n"
                              "<font size='5' color='pink'><div align='legt'> Point Num: </div> <div align='right'> %2 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Start Angle: </div> <div align='right'> %3 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> End Angle: </div> <div align='right'> %4 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Azi_Ele_Angle: </div> <div align='right'> %5 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Tx Order: </div> <div align='right'> %6 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Profile_ID: </div> <div align='right'> %7 </div> </font>\n")
                          .arg(table_info.Channel_Num)
                          .arg(table_info.Points)
                          .arg(table_info.Start_Angle)
                          .arg(table_info.End_Angle)
                          .arg(table_info.Azi_Ele_Angle)
                          .arg(tx_order)
                          .arg(table_info.Profile_ID);
          break;
        }

      /* 天线信息表 */
      case eol_protocol::ANT_BOTH_TABLE:
        {
          memcpy_s(&ant_table_info.Common_Info, sizeof(ant_table_info.Common_Info), &common_table_info.Common_Info, Index);
          memcpy(&ant_table_info.Points, data + Index, sizeof(ant_table_info.Points));
          Index += sizeof(ant_table_info.Points);
          memcpy(&ant_table_info.Channel_Num, data + Index, sizeof(ant_table_info.Channel_Num));
          Index += sizeof(ant_table_info.Channel_Num);
          quint32 tx_order = 0;
          memcpy(&tx_order, data + Index, sizeof(tx_order));
          Index += sizeof(tx_order);
          memcpy(&ant_table_info.Profile_ID, data + Index, sizeof(ant_table_info.Profile_ID));
          //        Index += sizeof(ant_table_info.Profile_ID);

          check_sum += ant_table_info.Points + ant_table_info.Channel_Num + tx_order + ant_table_info.Profile_ID;

          csv_header = tr("table class, version, data type, data size, data crc, points, channel num, tx_order, profile_id, check sum\r\n");
          str += QString::asprintf(",%u,%u,%u,%u,"
                                   "%lld\r\n",
                                   ant_table_info.Points,
                                   ant_table_info.Channel_Num,
                                   tx_order,
                                   ant_table_info.Profile_ID,
                                   check_sum);

          tips_str += QString("<font size='5' color='black'><div align='legt'> Channel Num: </div> <div align='right'> %1 </div> </font>\n"
                              "<font size='5' color='pink'><div align='legt'> Point Num: </div> <div align='right'> %2 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Tx Order: </div> <div align='right'> %3 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Profile_ID: </div> <div align='right'> %4 </div> </font>\n")
                          .arg(ant_table_info.Channel_Num)
                          .arg(ant_table_info.Points)
                          .arg(tx_order)
                          .arg(ant_table_info.Profile_ID);
          break;
        }

      /* 方向表 */
      case eol_protocol::PATTERN_TABLE:
        {
          memcpy_s(&pattern_table_info.Common_Info, sizeof(pattern_table_info.Common_Info), &common_table_info.Common_Info, Index);
          memcpy(&pattern_table_info.Start_Angle, data + Index, sizeof(pattern_table_info.Start_Angle));
          Index += sizeof(pattern_table_info.Start_Angle);
          memcpy(&pattern_table_info.End_Angle, data + Index, sizeof(pattern_table_info.End_Angle));
          Index += sizeof(pattern_table_info.End_Angle);
          memcpy(&pattern_table_info.Points, data + Index, sizeof(pattern_table_info.Points));
          Index += sizeof(pattern_table_info.Points);
          memcpy(&pattern_table_info.Channel_Num, data + Index, sizeof(pattern_table_info.Channel_Num));
          Index += sizeof(pattern_table_info.Channel_Num);
          memcpy(&pattern_table_info.Unit, data + Index, sizeof(pattern_table_info.Unit));
          Index += sizeof(pattern_table_info.Unit);
          quint32 tx_order = 0;
          memcpy(&tx_order, data + Index, sizeof(tx_order));
          Index += sizeof(tx_order);
          memcpy(&pattern_table_info.Profile_ID, data + Index, sizeof(pattern_table_info.Profile_ID));
          //        Index += sizeof(pattern_table_info.Profile_ID);

          qint32 Start_Angle = (qint32)(pattern_table_info.Start_Angle * 10.f);
          qint32 End_Angle = (qint32)(pattern_table_info.End_Angle * 10.f);
          check_sum += Start_Angle + End_Angle + pattern_table_info.Points + pattern_table_info.Channel_Num + pattern_table_info.Unit + tx_order + pattern_table_info.Profile_ID;
          csv_header = tr("table class, version, data type, data size, data crc, points, channel num, start angle*10, end angle*10, unit, tx_order, profile_id, check sum\r\n");
          str += QString::asprintf(",%u,%u,%d,%d,%u,%u,%u,"
                                   "%lld\r\n",
                                   pattern_table_info.Points,
                                   pattern_table_info.Channel_Num,
                                   Start_Angle,
                                   End_Angle,
                                   pattern_table_info.Unit,
                                   tx_order,
                                   pattern_table_info.Profile_ID,
                                   check_sum);

          tips_str += QString("<font size='5' color='black'><div align='legt'> Channel Num: </div> <div align='right'> %1 </div> </font>\n"
                              "<font size='5' color='pink'><div align='legt'> Point Num: </div> <div align='right'> %2 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Start Angle: </div> <div align='right'> %3 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> End Angle: </div> <div align='right'> %4 </div> </font>\n"
                              "<font size='5' color='purple'><div align='legt'> Unit: </div> <div align='right'> %5 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Tx Order: </div> <div align='right'> %6 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Profile_ID: </div> <div align='right'> %7 </div> </font>\n")
                          .arg(pattern_table_info.Channel_Num)
                          .arg(pattern_table_info.Points)
                          .arg(pattern_table_info.Start_Angle)
                          .arg(pattern_table_info.End_Angle)
                          .arg(pattern_table_info.Unit)
                          .arg(tx_order)
                          .arg(pattern_table_info.Profile_ID);
        }
        break;

      /* 底噪表 */
      case eol_protocol::BACKGROUND_NOISE_TABLE:
        {
          memcpy(&noise_table_info.Channel_Num[0], data + Index, sizeof(noise_table_info.Channel_Num));
          Index += sizeof(noise_table_info.Channel_Num);
          memcpy(&noise_table_info.Unit, data + Index, sizeof(noise_table_info.Unit));
          Index += sizeof(noise_table_info.Unit);
          quint32 tx_order[4] = {0};
          memcpy(&tx_order[0], data + Index, sizeof(tx_order));
          memcpy(&noise_table_info.Clibration_Tx_Order[0][0], data + Index, sizeof(noise_table_info.Clibration_Tx_Order));
          //        Index += sizeof(noise_table_info.Clibration_Tx_Order);

          check_sum += noise_table_info.Channel_Num[0] +
                       noise_table_info.Channel_Num[1] +
                       noise_table_info.Channel_Num[2] +
                       noise_table_info.Channel_Num[3] +
                       noise_table_info.Unit +
                       tx_order[0] +
                       tx_order[1] +
                       tx_order[2] +
                       tx_order[3];
          /* table class, version, data type, data size, data crc,
         * channel num0, channel num1, channel num2, channel num3,
         * unit, tx_order0, tx_order1, tx_order2, tx_order3, check sum */
          csv_header = tr("table class, version, data type, data size, data crc, "
                          "channel num0, channel num1, channel num2, channel num3, "
                          "unit, tx_order0, tx_order1, tx_order2, tx_order3, check sum\r\n");
          str += QString::asprintf(",%u,%u,%u,%u,%u,%u,%u,"
                                   "%u,%u,%lld\r\n",
                                    noise_table_info.Channel_Num[0],
                                    noise_table_info.Channel_Num[1],
                                    noise_table_info.Channel_Num[2],
                                    noise_table_info.Channel_Num[3],
                                    noise_table_info.Unit,
                                    tx_order[0],
                                    tx_order[1],
                                    tx_order[2],
                                    tx_order[3],
                                    check_sum);

          tips_str += QString("<font size='5' color='black'><div align='legt'> Channel Num0: </div> <div align='right'> %1 </div> </font>\n"
                              "<font size='5' color='black'><div align='legt'> Channel Num1: </div> <div align='right'> %2 </div> </font>\n"
                              "<font size='5' color='black'><div align='legt'> Channel Num2: </div> <div align='right'> %3 </div> </font>\n"
                              "<font size='5' color='black'><div align='legt'> Channel Num3: </div> <div align='right'> %4 </div> </font>\n"
                              "<font size='5' color='purple'><div align='legt'> Unit: </div> <div align='right'> %5 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Tx Order0: </div> <div align='right'> %6 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Tx Order1: </div> <div align='right'> %7 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Tx Order2: </div> <div align='right'> %8 </div> </font>\n"
                              "<font size='5' color='blue'><div align='legt'> Tx Order3: </div> <div align='right'> %9 </div> </font>\n")
                          .arg(noise_table_info.Channel_Num[0])
                          .arg(noise_table_info.Channel_Num[1])
                          .arg(noise_table_info.Channel_Num[2])
                          .arg(noise_table_info.Channel_Num[3])
                          .arg(noise_table_info.Unit)
                          .arg(tx_order[0])
                          .arg(tx_order[1])
                          .arg(tx_order[2])
                          .arg(tx_order[3]);
        }
        break;

      default:
        return;
    }

    if(true == one_key_rec_all_table_data_silent(frame_num, csv_header.toUtf8(), true))
    {
      one_key_rec_all_table_data_silent(frame_num, str.toUtf8());
      return;
    }

    /* 显示表信息 */
    QMessageBox message(QMessageBox::Information, tr("Table Info"), tr(tips_str.toUtf8()), QMessageBox::Yes, nullptr);
    message.exec();

#if USE_TEMP_FILE_TO_SAVE == 0
    /* 选择文件存储区域 */
    /* 参数：父对象，标题，默认路径，格式 */
    QString path = QFileDialog::getSaveFileName(this, tr("Save  "), "../", tr("csv (*.csv)"));
    if(path.isEmpty() == true)
    {
      run_state = false;

      /* 停止协议栈 */
      eol_protocol_obj->stop_task();
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
    if(recv_file.open(QIODevice::WriteOnly) == false)
    {
      return;
    }

    /* 表头写入文件 */
    recv_file.write(csv_header.toUtf8());
    recv_file.write(str.toUtf8());

    /* 原始表先关闭 */
    if(recv_file_origin.isOpen() == true)
    {
      recv_file_origin.close();
    }

    /* 获取文件信息 */
    QFileInfo info(path);
    path = info.absolutePath() + "/" + info.baseName() + tr("_origin.") + tr("csv");

    /* 原始表关联文件名 */
    recv_file_origin.setFileName(path);

    /* 原始表打开文件，只写方式 */
    if(recv_file_origin.open(QIODevice::WriteOnly) == false)
    {
      return;
    }

    /* 原始表写入文件 */
    recv_file_origin.write(csv_header.toUtf8());
    recv_file_origin.write(str.toUtf8());
    return;
#else
    /* 创建临时文件 */
    tmpFile.remove();
    if(tmpFile.open() == false)
    {
      qDebug() << "open tmp file faild";
      return;
    }

    /* 关联文件名 */
    recv_file.setFileName(tmpFile.fileName());

    /* 打开文件，只写方式 */
    if(recv_file.open(QIODevice::WriteOnly) == false)
    {
      return;
    }

    /* 表头写入文件 */
    recv_file.write(csv_header.toUtf8());
    recv_file.write(str.toUtf8());
    return;
#endif
  }

  /* 表检测 */
  if(eol_protocol::UNKNOW_TABLE == common_table_info.Common_Info.Table_Type)
  {
    return;
  }

  if(0U < frame_num && 0xFFFFU > frame_num)
  {
    /* 表大小，每帧256字节，显示进度 */
    quint32 size = 256U * (frame_num - 1U) + data_len;

    /* 写入数据 */

    qDebug() << "get pack num:" << frame_num << "ok";
    //    recv_file.seek((frame_num - 1) * 256);
    //    recv_file.write((const char *)data, data_len);

    QStringList origin_data_list;

    QStringList data_list;
    QString data_str;
    for(quint16 i = 0; i < data_len;)
    {
      switch(common_table_info.Common_Info.Data_Type)
      {
        case eol_protocol::CALTERAH_CFX_28BIT_DATA_TYPE:
        case eol_protocol::CALTERAH_CFL_32BIT_DATA_TYPE:
          {
            quint32 Val = 0;
            memcpy(&Val, data + i, 4U);
            data_list.append(QString("%1").arg(Val));
            origin_data_list.append(QString("%1").arg(Val));
            i += 4U;
          }
          break;

        case eol_protocol::FLOAT32_DATA_TYPE:
        case eol_protocol::FLOAT32_BIN_DATA_TYPE:
          {
            float Val = 0;
            memcpy(&Val, data + i, 4U);
            data_list.append(QString("%1").arg(Val));

            quint32 originVal = 0;
            memcpy(&originVal, data + i, 4U);
            origin_data_list.append(QString("%1").arg(originVal));
            i += 4U;
          }
          break;

        case eol_protocol::INT16_DAYA_TYPE:
          {
            switch (common_table_info.Common_Info.Table_Type)
            {
              default:
                {
                  qint16 Val = 0;
                  memcpy_s(&Val, 2U, data + i, 2U);
                  data_list.append(QString("%1").arg(Val));
                  origin_data_list.append(QString("%1").arg(Val));
                  i += sizeof(Val);
                }
                break;
            }
          }
          break;

        default:
          break;
      }
    }
    data_str = data_list.join(",");
    if(size < common_table_info.Common_Info.Data_Size)
    {
      data_str += ",";
    }

    /* 一键导出 */
    if(true == one_key_rec_all_table_data_silent(frame_num, data_str.toUtf8()))
    {
      ui->transfer_progressBar->setValue(size > common_table_info.Common_Info.Data_Size ? common_table_info.Common_Info.Data_Size : size);
      qDebug() << "set val" << size;
      ui->bytes_lcdNumber->display((int)size);
      return;
    }

    if(recv_file.isOpen() == false)
    {
      return;
    }
    recv_file.write(data_str.toUtf8());

    data_str = origin_data_list.join(",");
    if(size < common_table_info.Common_Info.Data_Size)
    {
      data_str += ",";
    }
    if(recv_file_origin.isOpen() == false)
    {
      return;
    }
    recv_file_origin.write(data_str.toUtf8());

    ui->transfer_progressBar->setValue(size > common_table_info.Common_Info.Data_Size ? common_table_info.Common_Info.Data_Size : size);
    ui->bytes_lcdNumber->display((int)size);
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
  SET_CURRENT_TASK_RUN_STATE(TASK_COMPLETE);
  /* 本按钮可用 */
  if(csv_list.size() > 0)
  {
    ui->update_pushButton->setEnabled(true);
  }
  else
  {
    ui->update_pushButton->setEnabled(false);
  }
  ui->add_list_pushButton->setEnabled(true);
  ui->export_all_pushButton->setEnabled(true);
  ui->export_all_pushButton->setEnabled(true);
}

/**
 * @brief 接收数据完成
 */
void eol_window::slot_recv_eol_data_complete()
{
  /* 设置完成状态 */
  SET_CURRENT_TASK_RUN_STATE(TASK_COMPLETE);

  /* 检测接收任务是否全部完成 */
  if(false == all_table_file_name.isEmpty())
  {
    return;
  }

  /* 关闭文件 */
  recv_file.close();
  recv_file_origin.close();

  /* 显示接收完成 */
  QMessageBox message(QMessageBox::Information, tr("Info"), tr("<font size='10' color='green'>upload ok!</font>"), QMessageBox::Yes, nullptr);
  message.exec();

  /* 本按钮可用 */
  if(csv_list.size() > 0)
  {
    ui->update_pushButton->setEnabled(true);
  }
  else
  {
    ui->update_pushButton->setEnabled(false);
  }
  ui->upload_pushButton->setEnabled(true);
  ui->add_list_pushButton->setEnabled(true);
  ui->export_all_pushButton->setEnabled(true);
}

void eol_window::on_entry_produce_mode_pushButton_clicked()
{
  if(nullptr == eol_protocol_obj)
  {
    return;
  }
  /* 是否正在执行任务 */
  if(TASK_RUNNING == current_task_complete_state)
  {
    qDebug() << "TASK_RUNNING";
    if(true == eol_protocol_obj->task_is_runing())
    {
      return;
    }
  }

  bool ret = false;
  if(ui->entry_produce_mode_pushButton->text() == "entry mode")
  {
    if(ui->produce_noral_radioButton->isChecked())
    {
      ret = eol_protocol_obj->eol_master_set_device_mode(eol_protocol::PRODUCE_MODE_NORMAL);
    }
    else
    {
      ret = eol_protocol_obj->eol_master_set_device_mode(eol_protocol::PRODUCE_MODE_DEBUG);
    }
    goto __TASK_STATE_CHANGE;
  }
  if(ui->entry_produce_mode_pushButton->text() == "exit mode")
  {
    ret = eol_protocol_obj->eol_master_set_device_mode(eol_protocol::NORMAL_MODE_RUN);
    goto __TASK_STATE_CHANGE;
  }
  return;
__TASK_STATE_CHANGE:
  if(true == ret)
  {
    /* 重置帧计数 */
    reset_base_ui_info();
    SET_CURRENT_TASK_RUN_STATE(TASK_RUNNING);
    eol_protocol_obj->start_task();
  }
  else
  {
    SET_CURRENT_TASK_RUN_STATE(TASK_ERROR);
  }
}

void eol_window::slot_device_mode(const void *pass_data)
{
  const quint8 *data_ptr = (const quint8 *)pass_data;

  quint16 index = 0;
  eol_protocol::CALIBRATION_PROFILE_INFO_Typedef_t info;
  memcpy_s(&info.time_sec, sizeof(info.time_sec), data_ptr + index, sizeof(info.time_sec));
  index += sizeof(info.time_sec);
  eol_protocol::DEVICE_MODE_Typedef_t mode = (eol_protocol::DEVICE_MODE_Typedef_t)data_ptr[index];
  index++;

  quint8 profile_num = data_ptr[index];
  index++;

  /* 生产-普通模式 */
  if(ui->produce_noral_radioButton->isChecked() && eol_protocol::PRODUCE_MODE_NORMAL == mode)
  {
    ui->add_list_pushButton->setEnabled(true);
    ui->upload_pushButton->setEnabled(true);
    ui->export_all_pushButton->setEnabled(true);
    if(csv_list.size() > 0)
    {
      ui->update_pushButton->setEnabled(true);
    }
    else
    {
      ui->update_pushButton->setEnabled(false);
    }
    ui->reboot_pushButton->setEnabled(true);
    ui->eol_device_rw_func_pushButton->setEnabled(true);
    ui->ant_calibration_func_pushButton->setEnabled(true);
    ui->rcs_calibration_func_pushButton->setEnabled(true);
    ui->debug_pushButton->setEnabled(false);
    ui->entry_produce_mode_pushButton->setText(tr("exit mode"));
  }

  /* 生产-调试模式 */
  else if(ui->produce_debug_radioButton->isChecked() && eol_protocol::PRODUCE_MODE_DEBUG == mode)
  {
    ui->add_list_pushButton->setEnabled(true);
    ui->upload_pushButton->setEnabled(true);
    ui->export_all_pushButton->setEnabled(true);
    if(csv_list.size() > 0)
    {
      ui->update_pushButton->setEnabled(true);
    }
    else
    {
      ui->update_pushButton->setEnabled(false);
    }
    ui->reboot_pushButton->setEnabled(true);
    ui->eol_device_rw_func_pushButton->setEnabled(true);
    ui->ant_calibration_func_pushButton->setEnabled(true);
    ui->rcs_calibration_func_pushButton->setEnabled(true);
    ui->debug_pushButton->setEnabled(true);
    ui->entry_produce_mode_pushButton->setText(tr("exit mode"));
  }

  /* 正常-运行模式 */
  else if(eol_protocol::NORMAL_MODE_RUN == mode)
  {
    ui->add_list_pushButton->setEnabled(false);
    ui->upload_pushButton->setEnabled(false);
    ui->export_all_pushButton->setEnabled(false);
    ui->update_pushButton->setEnabled(false);
    ui->reboot_pushButton->setEnabled(false);
    ui->eol_device_rw_func_pushButton->setEnabled(false);
    ui->ant_calibration_func_pushButton->setEnabled(false);
    ui->rcs_calibration_func_pushButton->setEnabled(false);
    ui->debug_pushButton->setEnabled(false);
    ui->entry_produce_mode_pushButton->setText(tr("entry mode"));
  }
  SET_CURRENT_TASK_RUN_STATE(TASK_COMPLETE);
  /* 显示设备信息 */
  emit signal_clear_profile_info();

  QString msg;
  msg += QString("<font size='5' color='green'><div align='legt'>profile num:%1</div> </font>").arg(profile_num);

  for(quint8 i = 0; i < profile_num; i++)
  {
    info.profile_id = data_ptr[index];
    index++;
    info.channel_num = data_ptr[index];
    index++;
    memcpy_s(info.tx_order, sizeof(info.tx_order), data_ptr + index, sizeof(info.tx_order));
    index += sizeof(info.tx_order);

    msg += QString("<font size='5' color='green'><div align='legt'>profile id:%1</div> </font>").arg(info.profile_id);
    msg += QString("<font size='5' color='green'><div align='legt'>channel num:%1</div> </font>").arg(info.channel_num);
    msg += QString("<font size='5' color='green'><div align='legt'>tx order:");
    for(quint8 t = 0; t < sizeof(info.tx_order); t++)
    {
      msg += QString("[%1]:%2").arg(t).arg(info.tx_order[t]);
    }
    msg += QString("</div> </font>");
    emit signal_profile_info_update(info);
  }
  QMessageBox message(QMessageBox::Information, tr("device info"), msg, \
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
  ui->msg_str_label->clear();
}

void eol_window::slot_send_rec_one_frame()
{
  qint32 frame_num_display = ui->frame_lcdNumber->value();
  ui->frame_lcdNumber->display(++frame_num_display);
}

/**
 * @brief 从机读写无反应
 * @param reg 读写寄存器地址
 * @param command 读写标识
 */
void eol_window::slot_protocol_rw_err(quint8 reg, quint8 command)
{
  Q_UNUSED(command)
  QString tips;
  switch(reg)
  {
    case EOL_W_DEVICE_REBOOT_REG:
      tips = tr("reboot err");
      break;

    case EOL_R_ACCESS_CODE_REG:
    case EOL_W_RUN_MODE_REG:
    case EOL_R_PROFILE_NUM_CH_NUM:
      tips = tr("set mode err");
      /* 设置错误状态 */
      SET_CURRENT_TASK_RUN_STATE(TASK_ERROR);
      break;

    default:
      return;
  }
  QMessageBox message(QMessageBox::Information, tr("Info"), tips, QMessageBox::Yes, nullptr);
  message.exec();
}

/**
 * @brief 读写设备正常
 * @param reg 寄存器
 * @param data 读出的数据，为nullptr代表写设备成功
 * @param data_len 代表数据长度，为0时代表写设备成功
 */
void eol_window::slot_rw_device_ok(quint8 reg, const quint8 *data, quint16 data_len)
{
  Q_UNUSED(data)
  Q_UNUSED(data_len)
  QString tips;
  switch(reg)
  {
    case EOL_W_DEVICE_REBOOT_REG:
      {
        tips = tr("reboot ok");
      }
      break;

    case EOL_R_ACCESS_CODE_REG:
    case EOL_W_RUN_MODE_REG:
    case EOL_R_PROFILE_NUM_CH_NUM:
      {
        tips = tr("rw ok");
        SET_CURRENT_TASK_RUN_STATE(TASK_COMPLETE);
      }
      break;

    case EOL_RW_SHELL_REG:
      {
        if(0 == data_len || nullptr == data)
        {
          /* 发送命令成功 */
          return;
        }
        debug_window_window_obj->rec_shell_data(data, data_len);
      }
      return;

    default:
      return;
  }
  QMessageBox message(QMessageBox::Information, tr("Info"), tips, QMessageBox::Yes, nullptr);
  message.exec();
}

void eol_window::on_eol_device_rw_func_pushButton_clicked()
{
  eol_sub_window_obj->show();
  if(true == eol_sub_window_obj->isMinimized())
  {
    eol_sub_window_obj->showNormal();
  }
  eol_sub_window_obj->activateWindow();
}

void eol_window::slot_eol_protol_is_start()
{

}

void eol_window::on_ant_calibration_func_pushButton_clicked()
{
  eol_2dfft_calibration_window_obj->show();
  if(true == eol_2dfft_calibration_window_obj->isMinimized())
  {
    eol_2dfft_calibration_window_obj->showNormal();
  }
  eol_2dfft_calibration_window_obj->activateWindow();
}


void eol_window::on_rcs_calibration_func_pushButton_clicked()
{
  eol_calibration_window_obj->show();
  if(true == eol_calibration_window_obj->isMinimized())
  {
    eol_calibration_window_obj->showNormal();
  }
  eol_calibration_window_obj->activateWindow();
}

void eol_window::on_reboot_pushButton_clicked()
{
  if(nullptr == eol_protocol_obj)
  {
    return;
  }
  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;
  task.reg = EOL_W_DEVICE_REBOOT_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  task.buf[0] = 1;
  task.len = 1;

  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}

void eol_window::on_debug_pushButton_clicked()
{
  debug_window_window_obj->show();
  if(true == debug_window_window_obj->isMinimized())
  {
    debug_window_window_obj->showNormal();
  }
  debug_window_window_obj->activateWindow();
}

void eol_window::on_com_hw_comboBox_currentIndexChanged(int index)
{
  eol_protocol_obj->set_eol_com_config_hw((eol_protocol::EOL_SEND_HW_Typedef_t)index);
}

void eol_window::on_com_config_lineEdit_textChanged(const QString &arg1)
{
  eol_protocol_obj->set_eol_com_config_channel(arg1);
}


void eol_window::on_vcom_config_lineEdit_textChanged(const QString &arg1)
{
  eol_protocol_obj->set_eol_vcom_config_channel(arg1);
}

void eol_window::on_dev_addr_lineEdit_textChanged(const QString &arg1)
{
  eol_protocol_obj->set_eol_dev_addr(arg1);
}

void eol_window::on_export_all_pushButton_clicked()
{
  if(false == all_table_file_name.isEmpty())
  {
    return;
  }

  /* 导出统计的目标列表 */
  all_table_file_name = QFileDialog::getSaveFileName(this, tr("Save  "), last_file_path, tr("csv (*.csv)"));
  if(true == all_table_file_name.isEmpty())
  {
    return;
  }

  /* 获取文件信息 */
  QFileInfo info(current_file_path);

  /* 更新最近路径信息 */
  last_file_path = info.absolutePath();

  /* 清空更新表 */
  csv_list.clear();

  /* 更新显示列表 */
  update_show_table_list();

  /* 重置界面 */
  reset_base_ui_info();

  /* 重置错误统计 */
  err_constantly_cnt = 0;

  /* 重置计时 */
  time_cnt = 0;

  /* 本按钮不可用 */
  ui->upload_pushButton->setEnabled(false);
  ui->add_list_pushButton->setEnabled(false);
  ui->export_all_pushButton->setEnabled(false);

  /* 设置线程任务 */
  current_running_task = UPLOAD_ALL_TABLE_TASK;

  /* 启动线程 */
  run_state = true;

  /* 原子操作 */
  if(thread_run_statex.testAndSetRelaxed(0, 1))
  {
    /* 启动任务线程 */
    g_thread_pool->start(this);
  }
  else
  {
    qDebug() << "eol window task is running";
    return;
  }

  /* 启动计时 */
  timer_obj->start();
}

void eol_window::on_rts_crl_pushButton_clicked()
{
  rts_ctrl_window_obj->show();
  if(true == rts_ctrl_window_obj->isMinimized())
  {
    rts_ctrl_window_obj->showNormal();
  }
  rts_ctrl_window_obj->activateWindow();
  connect(rts_protocol_obj, &rts_protocol::signal_protocol_error_occur, rts_ctrl_window_obj, &rts_ctrl_window::slot_protocol_error_occur);
  connect(rts_protocol_obj, &rts_protocol::signal_protocol_timeout, rts_ctrl_window_obj, &rts_ctrl_window::slot_protocol_timeout);
  connect(rts_protocol_obj, &rts_protocol::signal_protocol_rw_err, rts_ctrl_window_obj, &rts_ctrl_window::slot_protocol_rw_err);
}

void eol_window::on_open_rts_pushButton_clicked()
{
  rts_protocol::RTS_TASK_LIST_Typedef_t task;

  /* 连接 */
  task.cmd = RTS_CONNECT;
  rts_protocol_obj->rts_master_common_rw_device(task);

  /* 打开 */
  task.cmd = RTS_OPEN_DEVICE;
  rts_protocol_obj->rts_master_common_rw_device(task);

  /* 启动协议栈 */
  rts_protocol_obj->start_task();
}

void eol_window::on_close_rts_pushButton_clicked()
{
  rts_protocol::RTS_TASK_LIST_Typedef_t task;

  /* 关闭 */
  task.cmd = RTS_CLOSE_DEVICE;
  rts_protocol_obj->rts_master_common_rw_device(task);

  /* 关闭连接 */
  task.cmd = RTS_DISCONNECT;
  rts_protocol_obj->rts_master_common_rw_device(task);

  /* 启动协议栈 */
  rts_protocol_obj->start_task();
}

/**
   * @brief 协议栈无回复超时
   * @param sec 秒
   */
void eol_window::slot_rts_protocol_timeout(quint32 sec)
{
  ui->msg_str_label->setText(tr("rts:timeout %1 sec").arg(sec));
}

/**
   * @brief 从机返回错误消息
   * @param error_msg 错误消息
   */
void eol_window::slot_rts_protocol_error_occur(quint8 error_msg)
{
  if(0U == error_msg)
  {
    ui->msg_str_label->setText("rts:ok");
  }
  else
  {
    ui->msg_str_label->setText("rts:err");
  }
}

/**
   * @brief signal_protocol_rw_err 读写错误信号
   * @param cmd 命令
   */
void eol_window::slot_rts_protocol_rw_err(QString cmd)
{
  ui->msg_str_label->setText(tr("rts:cmd [%1] retry err").arg(cmd));
}

void eol_window::on_rts_addr_lineEdit_editingFinished()
{
  if(rts_protocol_obj != nullptr)
  {
    /* 更新rts通讯地址 */
    rts_protocol_obj->set_network_device(rts_network_driver_send_obj, rts_network_driver_rec_obj, ui->rts_addr_lineEdit->text());
  }
}

void eol_window::slot_rts_protocol_rw_ok(QString cmd)
{
  ui->msg_str_label->setText("rts:ok");
  if(RTS_OPEN_DEVICE != cmd && RTS_CLOSE_DEVICE != cmd)
  {
    return;
  }
  QString tips = QString("set RTS:%1 ok").arg(cmd);
  QMessageBox message(QMessageBox::Information, tr("Info"), tips, QMessageBox::Yes, nullptr);
  message.exec();
}
/******************************** End of file *********************************/
