/**
 *  @file mainwindow.cpp
 *
 *  @date 2023年05月07日 14:36:17 星期天
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief 主窗口.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.05.07 14:36 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-05-07 <td>v0.0.1  <td>aron566 <td>初始版本
 *  <tr><td>2023-05-07 <td>v0.0.2  <td>aron566 <td>eol协议缓冲区增大至4K
 *  <tr><td>2023-05-15 <td>v0.0.3  <td>aron566 <td>完善rcs页面，消息刷新机制更新支持滚轮查询
 *  <tr><td>2023-05-16 <td>v0.0.4  <td>aron566 <td>优化部分代码
 *  <tr><td>2023-05-17 <td>v0.0.5  <td>aron566 <td>优化部分代码
 *  <tr><td>2023-05-21 <td>v0.0.6  <td>aron566 <td>增加天线校准功能与导出2dfft数据功能
 *  <tr><td>2023-05-21 <td>v0.0.7  <td>aron566 <td>适配新EOL协议v0.0.8增加底噪表传输功能
 *  <tr><td>2023-05-22 <td>v0.0.8  <td>aron566 <td>修复上翻显示色彩异常问题
 *  <tr><td>2023-06-01 <td>v0.0.9  <td>aron566 <td>增加帧诊断窗口，优化eol协议栈发送机制，允许发送多种硬件端口
 *  <tr><td>2023-06-05 <td>v0.0.10 <td>aron566 <td>修复析构会导致的问题
 *  <tr><td>2023-06-06 <td>v0.0.11 <td>aron566 <td>增加chip pmic sn看门狗测试接口，报文导出txt接口，增加读取rcs补偿值接口
 *  <tr><td>2023-06-08 <td>v0.0.12 <td>aron566 <td>修复导出csv checksum可能不对问题，增加设置校准模式按钮
 *  <tr><td>2023-06-09 <td>v0.0.13 <td>aron566 <td>增加手动输入crc计算功能
 *  <tr><td>2023-06-12 <td>v0.0.14 <td>aron566 <td>增加数据曲线功能，打印格式：【$%d %d;】
 *  <tr><td>2023-06-13 <td>v0.0.15 <td>aron566 <td>修复信息读写的时对版本写的bug
 *  <tr><td>2023-06-14 <td>v0.0.16 <td>aron566 <td>修复csv表传输，某些字段未传输问题
 *  <tr><td>2023-06-19 <td>v0.0.17 <td>aron566 <td>增加命令终端功能
 *  <tr><td>2023-06-20 <td>v0.0.18 <td>aron566 <td>适配EOL协议v0.0.15版本，增加gpio寄存器，shell寄存器
 *  <tr><td>2023-06-25 <td>v0.0.19 <td>aron566 <td>适配GCcan驱动
 *  <tr><td>2023-06-26 <td>v0.0.20 <td>aron566 <td>手动发送将自动依据发送的字节数分包发送，增加回环工作模式，为通讯配置增加配置文件
 *  <tr><td>2023-06-27 <td>v0.0.21 <td>aron566 <td>适配GCcanfd驱动
 *  <tr><td>2023-06-28 <td>v0.0.22 <td>aron566 <td>修复GCcanfd驱动，初始化波特率设置不对问题，提示信息不对问题
 *  <tr><td>2023-06-29 <td>v0.0.23 <td>aron566 <td>修复GCcanfd驱动，关闭问题避免二次关闭异常
 *  <tr><td>2023-06-29 <td>v0.0.24 <td>aron566 <td>优化帧诊断，屏蔽black信号刷新避免卡顿，修复gc发送帧协议类型不对问题
 *  <tr><td>2023-07-03 <td>v0.0.25 <td>aron566 <td>增加more页面数据保存，统一一个配置文件管理
 *  <tr><td>2023-07-05 <td>v0.0.26 <td>aron566 <td>配置版本号改为下划线
 *  <tr><td>2023-07-10 <td>v0.0.27 <td>aron566 <td>调试窗口应用参数后立即生效
 *  <tr><td>2023-07-14 <td>v0.0.28 <td>aron566 <td>EOL显示校准配置信息显示优化
 *  <tr><td>2023-07-15 <td>v0.0.29 <td>aron566 <td>窗口按钮大小优化
 *  <tr><td>2023-07-21 <td>v0.0.30 <td>aron566 <td>弹窗字体大小修改
 *  <tr><td>2023-07-25 <td>v0.0.31 <td>aron566 <td>mag类型修改为无符号
 *  <tr><td>2023-07-31 <td>v1.0.0  <td>aron566 <td>增加远程更新功能
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
/** Private includes ---------------------------------------------------------*/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define CONFIG_VER_STR            "0.0.1"                /**< 配置文件版本 */
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

  /* 图形字体加载 */
  font_file_load();

  /* 设置窗口标题 */
  this->setWindowTitle(qApp->applicationName() + " v" + qApp->applicationVersion());

  /* 获取线程池 */
  g_thread_pool = QThreadPool::globalInstance();
  g_thread_pool->setExpiryTimeout(-1);
  g_thread_pool->setMaxThreadCount(static_cast<qint32>(std::thread::hardware_concurrency()));
  qDebug() << "main thread id:" << QThread::currentThreadId() \
           << " max:" << g_thread_pool->maxThreadCount() \
           << " stack size:" << g_thread_pool->stackSize();

  /* can驱动初始化 */
  can_driver_init();

  /* 更新窗口初始化 */
  updater_window_init("EOL CAN Tool - Updater");

  /* 子窗口初始化 */
  more_window_init(tr("EOL CAN Tool - More"));

  /* 恢复参数 */
  read_cfg();
}

MainWindow::~MainWindow()
{
  /* 保存参数 */
  save_cfg();

  delete can_driver_obj;
  qDebug() << "del can_driver_obj";

  delete more_window_obj;
  qDebug() << "del more_window_obj";

  g_thread_pool->waitForDone();
  g_thread_pool->clear();

  delete updater_window_obj;
  qDebug() << "del updater_window_obj";

  delete ui;
  qDebug() << "del mainwindow";
}

/**
 * @brief MainWindow::font_file_load
 */
void MainWindow::font_file_load()
{
  if(!fontDb.families().contains("FontAwesome"))
  {
    int fontId = fontDb.addApplicationFont(":/ico_ttf/fontawesome-webfont.ttf");
    QStringList fontName = fontDb.applicationFontFamilies(fontId);
    if(fontName.count() == 0)
    {
        qDebug() << "load fontawesome-webfont.ttf error";
    }
  }
}

/**
 * @brief 更新子窗口
 */
void MainWindow::updater_window_init(QString titile)
{
  updater_window_obj = new updater_window(titile);
}

/**
 * @brief 菜单子窗口
 */
void MainWindow::more_window_init(QString titile)
{
  more_window_obj = new more_window(titile);
  connect(more_window_obj, &more_window::signal_more_window_closed, this, &MainWindow::slot_show_this_window);

  /* 设置can驱动接口 */
  more_window_obj->set_can_driver_obj(can_driver_obj);
}

void MainWindow::can_driver_init()
{
  can_driver_obj = new can_driver(this);

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

  /* 读取设备信息不可用状态 */
  ui->device_info_pushButton->setEnabled(false);
}

void MainWindow::save_cfg()
{
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setValue("com_v" CONFIG_VER_STR "/device_brand", ui->brand_comboBox->currentIndex());
  setting.setValue("com_v" CONFIG_VER_STR "/device_name", ui->device_list_comboBox->currentText());
  setting.setValue("com_v" CONFIG_VER_STR "/arbitration_bps", ui->arbitration_bps_comboBox->currentIndex());
  setting.setValue("com_v" CONFIG_VER_STR "/data_bps", ui->data_bps_comboBox->currentIndex());
  setting.setValue("com_v" CONFIG_VER_STR "/bps", ui->bps_comboBox->currentIndex());
  setting.setValue("com_v" CONFIG_VER_STR "/end_resistance_en", (int)ui->end_resistance_checkBox->checkState());
  setting.sync();
}

void MainWindow::read_cfg()
{
  QFile file("./eol_tool_cfg.ini");
  if(false == file.exists())
  {
    /* 设置默认值 */
    ui->brand_comboBox->setCurrentIndex(can_driver::ZLG_CAN_BRAND);
    ui->device_list_comboBox->setCurrentText("ZCAN_USBCANFD_200U");
    ui->arbitration_bps_comboBox->setCurrentIndex(2);
    ui->data_bps_comboBox->setCurrentIndex(2);
    /* 设置终端电阻启用状态 */
    can_driver_obj->set_resistance_enbale(ui->end_resistance_checkBox->isChecked());
    return;
  }
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  if(false == setting.contains("com_v" CONFIG_VER_STR "/device_brand"))
  {
    qDebug() << "err main_window config not exist";
    return;
  }
  ui->brand_comboBox->setCurrentIndex((can_driver::CAN_BRAND_Typedef_t)setting.value("com_v" CONFIG_VER_STR "/device_brand").toInt());
  ui->device_list_comboBox->setCurrentText(setting.value("com_v" CONFIG_VER_STR "/device_name").toString());
  ui->arbitration_bps_comboBox->setCurrentIndex(setting.value("com_v" CONFIG_VER_STR "/arbitration_bps").toInt());
  ui->data_bps_comboBox->setCurrentIndex(setting.value("com_v" CONFIG_VER_STR "/data_bps").toInt());
  ui->bps_comboBox->setCurrentIndex(setting.value("com_v" CONFIG_VER_STR "/bps").toInt());
  /* 设置终端电阻启用状态 */
  ui->end_resistance_checkBox->setCheckState((Qt::CheckState)setting.value("com_v" CONFIG_VER_STR "/end_resistance_en").toInt());
  can_driver_obj->set_resistance_enbale(ui->end_resistance_checkBox->isChecked());
  setting.sync();
}

void MainWindow::slot_show_this_window()
{
  this->show();
}

void MainWindow::slot_can_is_opened(void)
{
  /* can设备打开状态，则置为不可选择状态 */
  ui->device_index_comboBox->setEnabled(false);
  ui->device_list_comboBox->setEnabled(false);
  ui->channel_num_comboBox->setEnabled(false);
  ui->start_can_pushButton->setEnabled(true);
  ui->init_can_pushButton->setEnabled(true);
  ui->device_info_pushButton->setEnabled(true);
}

void MainWindow::slot_can_is_closed(void)
{
  /* 关闭设备状态时，启动与初始化为可用状态 */
  ui->device_index_comboBox->setEnabled(true);
  ui->device_list_comboBox->setEnabled(true);
  ui->channel_num_comboBox->setEnabled(true);
  ui->start_can_pushButton->setEnabled(true);
  ui->init_can_pushButton->setEnabled(true);
  ui->device_info_pushButton->setEnabled(false);
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
  ui->data_bps_comboBox->setEnabled(can_use);
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

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

void MainWindow::on_open_device_pushButton_clicked()
{
  /* 打开设备 */
  can_driver_obj->open();
  /* 禁用打开设备按钮 */
  ui->open_device_pushButton->setEnabled(false);
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
  can_driver_obj->close();
  /* 禁用打开设备按钮 */
  ui->open_device_pushButton->setEnabled(true);
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

  /* 打开全部通道 */
  ui->channel_num_comboBox->addItem("ALL");

  /* 设置可选通道 */
  more_window_obj->set_channel_num(channel_num);
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
  qDebug() << " set channel index:" << index;
  can_driver_obj->set_channel_index(index);
}

void MainWindow::on_brand_comboBox_currentIndexChanged(int index)
{
  /* 设置品牌 */
  QStringList device_list = can_driver_obj->set_device_brand((can_driver::CAN_BRAND_Typedef_t)index);
  ui->device_list_comboBox->clear();

  /* 更新列表 */
  ui->device_list_comboBox->addItems(device_list);
}

void MainWindow::on_device_info_pushButton_clicked()
{
  /* 读取设备信息 */
  can_driver_obj->read_info();
}

void MainWindow::on_updater_pushButton_clicked()
{
  updater_window_obj->show();
}
/******************************** End of file *********************************/
