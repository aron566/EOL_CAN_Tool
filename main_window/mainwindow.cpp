#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>

#define PC_SOFTWARE_VERSION "v0.0.1"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
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
  this->setWindowTitle(tr("EOL CAN Tool ") + tr(PC_SOFTWARE_VERSION));

  /* can驱动初始化 */
  can_driver_init();

  /* 子窗口初始化 */
  eol_window_init(tr("EOL CAN Tool - EOL调试"));

  /* 子窗口初始化 */
  more_window_init(tr("EOL CAN Tool - More"));

  /* eol协议栈初始化 */
  eol_protocol_init();

  /* 恢复参数 */
  para_restore_init();

  /* 获取线程池 */
  g_thread_pool = QThreadPool::globalInstance();
  g_thread_pool->setExpiryTimeout(-1);
  g_thread_pool->setMaxThreadCount(static_cast<qint32>(std::thread::hardware_concurrency()));
  qDebug() << "main thread id:" << QThread::currentThreadId();
}

MainWindow::~MainWindow()
{
  delete can_driver_obj;
  delete eol_window_obj;
  delete more_window_obj;
  delete ui;
}

/**
 * @brief 菜单子窗口
 */
void MainWindow::more_window_init(QString titile)
{
  more_window_obj = new more_window(titile);
  connect(more_window_obj, &more_window::signal_more_window_closed, this, &MainWindow::slot_show_this_window);

  /* 提供子级窗口操作接口 */
  more_window_obj->eol_ui = eol_window_obj;

  /* 设置can驱动接口 */
  more_window_obj->set_can_driver_obj(can_driver_obj);
}

/**
 * @brief EOL调试子窗口
 */
void MainWindow::eol_window_init(QString titile)
{
  eol_window_obj = new eol_window(titile);
  connect(eol_window_obj, &eol_window::signal_eol_window_closed, this, &MainWindow::slot_show_this_window);
}

void MainWindow::can_driver_init()
{
  can_driver_obj = new can_driver();

  /* 禁止线程完成后执行析构对象 */
  can_driver_obj->setAutoDelete(false);

  connect(can_driver_obj, &can_driver::signal_can_is_opened, this, &MainWindow::slot_can_is_opened);
  connect(can_driver_obj, &can_driver::signal_can_is_closed, this, &MainWindow::slot_can_is_closed);
  connect(can_driver_obj, &can_driver::signal_work_mode_can_use, this, &MainWindow::slot_work_mode_can_use);
  connect(can_driver_obj, &can_driver::signal_resistance_cs_use, this, &MainWindow::slot_resistance_cs_use);
  connect(can_driver_obj, &can_driver::signal_bauds_can_use, this, &MainWindow::slot_bauds_can_use);
  connect(can_driver_obj, &can_driver::signal_arbitration_data_bauds_can_use, this, &MainWindow::slot_arbitration_data_bauds_can_use);
  connect(can_driver_obj, &can_driver::signal_diy_bauds_can_use, this, &MainWindow::slot_diy_bauds_can_use);
  connect(can_driver_obj, &can_driver::signal_filter_can_use, this, &MainWindow::slot_filter_can_use);
  connect(can_driver_obj, &can_driver::signal_local_port_can_use, this, &MainWindow::slot_local_port_can_use);
  connect(can_driver_obj, &can_driver::signal_remote_port_can_use, this, &MainWindow::slot_remote_port_can_use);
  connect(can_driver_obj, &can_driver::signal_remote_addr_can_use, this, &MainWindow::slot_remote_addr_can_use);
  connect(can_driver_obj, &can_driver::signal_send_queue_delay_can_use, this, &MainWindow::slot_send_queue_delay_can_use);
  connect(can_driver_obj, &can_driver::signal_get_tx_available_can_use, this, &MainWindow::slot_get_tx_available_can_use);
  connect(can_driver_obj, &can_driver::signal_clear_tx_queue_can_use, this, &MainWindow::slot_clear_tx_queue_can_use);
  connect(can_driver_obj, &can_driver::signal_queue_delay_flag_can_use, this, &MainWindow::slot_queue_delay_flag_can_use);
  connect(can_driver_obj, &can_driver::signal_get_sen_mode_can_use, this, &MainWindow::slot_get_sen_mode_can_use);
  connect(can_driver_obj, &can_driver::signal_send_queue_mode_can_use, this, &MainWindow::slot_send_queue_mode_can_use);
  connect(can_driver_obj, &can_driver::signal_auto_send_dev_index_can_use, this, &MainWindow::slot_auto_send_dev_index_can_use);
  connect(can_driver_obj, &can_driver::signal_auto_send_period_can_use, this, &MainWindow::slot_auto_send_period_can_use);
  connect(can_driver_obj, &can_driver::signal_auto_send_add_can_use, this, &MainWindow::slot_auto_send_add_can_use);
  connect(can_driver_obj, &can_driver::signal_auto_send_start_can_use, this, &MainWindow::slot_auto_send_start_can_use);
  connect(can_driver_obj, &can_driver::signal_auto_send_stop_can_use, this, &MainWindow::slot_auto_send_stop_can_use);
  connect(can_driver_obj, &can_driver::signal_auto_send_cancel_once_can_use, this, &MainWindow::slot_auto_send_cancel_once_can_use);
  connect(can_driver_obj, &can_driver::signal_get_dev_auto_send_list_can_use, this, &MainWindow::slot_get_dev_auto_send_list_can_use);
  connect(can_driver_obj, &can_driver::signal_show_message, this, &MainWindow::slot_show_message);
  /* 线程同步 */
  connect(can_driver_obj, &can_driver::signal_show_thread_message, this, &MainWindow::slot_show_message, Qt::BlockingQueuedConnection);
}

void MainWindow::eol_protocol_init()
{
  /* eol协议栈初始化 */
  eol_protocol_obj = new eol_protocol(this);

  /* 禁止线程完成后执行析构对象 */
  eol_protocol_obj->setAutoDelete(false);

  /* 设置can驱动接口 */
  eol_protocol_obj->set_can_driver_obj(can_driver_obj);
}

void MainWindow::para_restore_init()
{
  /* 设置默认值 */
  ui->device_list_comboBox->setCurrentText("ZCAN_USBCANFD_200U");
}

void MainWindow::slot_show_this_window()
{
  this->show();
}

void MainWindow::slot_can_is_opened(bool opened)
{
  /* can设备打开状态，则置为不可选择状态 */
  ui->device_index_comboBox->setEnabled(!opened);
  ui->device_list_comboBox->setEnabled(!opened);
  ui->channel_num_comboBox->setEnabled(!opened);
}

void MainWindow::slot_can_is_closed(bool closed)
{
  /* 关闭设备状态时，启动与初始化为可用状态 */
  ui->start_can_pushButton->setEnabled(closed);
  ui->init_can_pushButton->setEnabled(closed);
}

void MainWindow::slot_work_mode_can_use(bool can_use)
{
  ui->mode_comboBox->setEnabled(can_use);
}

void MainWindow::slot_resistance_cs_use(bool can_use)
{
  ui->end_resistance_checkBox->setEnabled(can_use);
}

void MainWindow::slot_bauds_can_use(bool can_use)
{
  ui->bps_comboBox->setEnabled(can_use);
}

void MainWindow::slot_arbitration_data_bauds_can_use(bool can_use)
{
  ui->arbitration_bps_comboBox->setEnabled(can_use);
}

void MainWindow::slot_diy_bauds_can_use(bool can_use)
{
  ui->diy_bps_groupBox->setEnabled(can_use);
}

void MainWindow::slot_filter_can_use(bool can_use)
{
  ui->filter_par_groupBox->setEnabled(can_use);
}

void MainWindow::slot_local_port_can_use(bool can_use)
{
  ui->local_port_lineEdit->setEnabled(can_use);
}

void MainWindow::slot_remote_port_can_use(bool can_use)
{
  ui->remote_port_lineEdit->setEnabled(can_use);
}

void MainWindow::slot_remote_addr_can_use(bool can_use)
{
  ui->remote_ip_lineEdit->setEnabled(can_use);
}

void MainWindow::slot_send_queue_delay_can_use(bool can_use)
{
  /* 队列帧发送延时标记 */
  Q_UNUSED(can_use)
}

void MainWindow::slot_get_tx_available_can_use(bool can_use)
{
  /* 获取发送队列可用空间 */
  Q_UNUSED(can_use)
}

void MainWindow::slot_clear_tx_queue_can_use(bool can_use)
{
  /* 清除队列 */
  Q_UNUSED(can_use)
}

void MainWindow::slot_queue_delay_flag_can_use(bool can_use)
{
  /* 标记队列帧是否延时 */
  Q_UNUSED(can_use)
}

void MainWindow::slot_get_sen_mode_can_use(bool can_use)
{
  /* 查询设备模式 */
  Q_UNUSED(can_use)
}

void MainWindow::slot_send_queue_mode_can_use(bool can_use)
{
  Q_UNUSED(can_use)
}

void MainWindow::slot_show_message(const QString &message)
{
  /* 显示消息到接收框 */
  more_window_obj->show_message(message);
}

void MainWindow::slot_auto_send_dev_index_can_use(bool can_use)
{
  Q_UNUSED(can_use)
}

void MainWindow::slot_auto_send_period_can_use(bool can_use)
{
  Q_UNUSED(can_use)
}

void MainWindow::slot_auto_send_add_can_use(bool can_use)
{
  Q_UNUSED(can_use)
}

void MainWindow::slot_auto_send_start_can_use(bool can_use)
{
  Q_UNUSED(can_use)
}

void MainWindow::slot_auto_send_stop_can_use(bool can_use)
{
  Q_UNUSED(can_use)
}

void MainWindow::slot_auto_send_cancel_once_can_use(bool can_use)
{
  Q_UNUSED(can_use)
}

void MainWindow::slot_get_dev_auto_send_list_can_use(bool can_use)
{
  Q_UNUSED(can_use)
}

void MainWindow::on_open_device_pushButton_clicked()
{
  /* 打开设备 */
  can_driver_obj->open();
}

void MainWindow::on_init_can_pushButton_clicked()
{
  /* 初始化设备 */
  if(false == can_driver_obj->init())
  {
    return;
  }
  /* 禁用初始化按钮 */
  ui->init_can_pushButton->setEnabled(false);
}

void MainWindow::on_start_can_pushButton_clicked()
{
  /* 启动设备 */
  if(false == can_driver_obj->start())
  {
    return;
  }
  /* 禁用启动按钮 */
  ui->start_can_pushButton->setEnabled(false);
  g_thread_pool->start(can_driver_obj);
}

void MainWindow::on_reset_device_pushButton_clicked()
{
  /* 复位设备 */
  eol_protocol_obj->stop();
  if(false == can_driver_obj->reset())
  {
    return;
  }
  /* 启用启动按钮 */
  ui->start_can_pushButton->setEnabled(true);
}

void MainWindow::on_close_device_pushButton_clicked()
{
  /* 关闭设备 */
  eol_protocol_obj->stop();
  can_driver_obj->close();
}

void MainWindow::on_more_pushButton_clicked()
{
//  this->hide();
  more_window_obj->show();
}

void MainWindow::on_device_list_comboBox_currentTextChanged(const QString &arg1)
{
  quint8 channel_num = can_driver_obj->set_device_type(arg1);
  qDebug() << " set device type " << arg1 << "CH " << channel_num;
  /* 根据所选设备类型，更新通道数量 */
  ui->channel_num_comboBox->clear();
  for(quint8 i = 0; i < channel_num; i++)
  {
    ui->channel_num_comboBox->addItem(QString("%1").arg(i));
  }
}

void MainWindow::on_device_index_comboBox_currentTextChanged(const QString &arg1)
{
  /* 设置设备索引号，多个同样的设备接入时，index不同 */
  can_driver_obj->set_device_index(arg1.toUShort());
}

void MainWindow::on_end_resistance_checkBox_clicked(bool checked)
{
  /* 设置终端电阻启用状态 */
  can_driver_obj->set_resistance_enbale(checked);
}

void MainWindow::on_bps_comboBox_currentIndexChanged(int index)
{
  /* 设置can卡波特率 */
  can_driver_obj->set_bps((quint32)index);
}

void MainWindow::on_arbitration_bps_comboBox_currentIndexChanged(int index)
{
  /* 设置仲裁域波特率 */
  can_driver_obj->set_abit_bps((quint32)index);
}

void MainWindow::on_data_bps_comboBox_currentIndexChanged(int index)
{
  /* 设置数据域波特率 */
  can_driver_obj->set_dbit_bps((quint32)index);
}


void MainWindow::on_mode_comboBox_currentIndexChanged(int index)
{
  /* 设置工作模式 */
  can_driver_obj->set_work_mode((quint32)index);
}


void MainWindow::on_filter_mode_comboBox_currentIndexChanged(int index)
{
  /* 设置过滤模式 */
  can_driver_obj->set_filter_mode((quint32)index);
}


void MainWindow::on_verification_code_lineEdit_textChanged(const QString &arg1)
{
  /* 设置验收码 */
  can_driver_obj->set_acc_code(arg1);
}


void MainWindow::on_mask_code_lineEdit_textChanged(const QString &arg1)
{
  /* 设置掩码 */
  can_driver_obj->set_acc_mask_code(arg1);
}


void MainWindow::on_role_comboBox_currentIndexChanged(int index)
{
  /* 设置网络角色 */
  can_driver_obj->set_net_work_role((quint32)index);
}


void MainWindow::on_diy_bps_checkBox_clicked(bool checked)
{
  /* 设置自定义波特率是否启用 */
  can_driver_obj->set_diy_baud_rate_en(checked);
}

void MainWindow::on_channel_num_comboBox_currentIndexChanged(int index)
{
  /* 设置通道号 */
  can_driver_obj->set_channel_index(index);
}

