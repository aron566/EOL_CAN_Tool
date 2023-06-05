#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "more_window.h"
#include "eol_window.h"
#include "can_driver.h"
#include "eol_protocol.h"
#include <QThread>
#include <QThreadPool>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public:
    QThreadPool *g_thread_pool = nullptr;

private slots:
  void on_open_device_pushButton_clicked();

  void on_init_can_pushButton_clicked();

  void on_start_can_pushButton_clicked();

  void on_reset_device_pushButton_clicked();

  void on_close_device_pushButton_clicked();

  void on_more_pushButton_clicked();

private:
  Ui::MainWindow *ui;

  more_window *more_window_obj = nullptr;

private:
  can_driver *can_driver_obj = nullptr;
private:

  /**
   * @brief 下级窗口初始化
   * @param titile
   */
  void more_window_init(QString titile);

private:
  /**
   * @brief can驱动初始化
   *
   */
  void can_driver_init();

  /**
   * @brief 恢复参数
   */
  void para_restore_init();
private slots:
  /**
   * @brief 显示主窗口
   *
   */
  void slot_show_this_window();

  /**
   * @brief can已经打开
   */
  void slot_can_is_opened(void);

  /**
   * @brief can已经关闭
   */
  void slot_can_is_closed(void);

  /**
   * @brief 相关控件是否可用
   *
   * @param can_use true 可用
   */
  void slot_work_mode_can_use(bool can_use);
  void slot_resistance_cs_use(bool can_use);
  void slot_bauds_can_use(bool can_use);
  void slot_arbitration_data_bauds_can_use(bool can_use);
  void slot_diy_bauds_can_use(bool can_use);
  void slot_filter_can_use(bool can_use);
  void slot_local_port_can_use(bool can_use);
  void slot_remote_port_can_use(bool can_use);
  void slot_remote_addr_can_use(bool can_use);
  void slot_send_queue_delay_can_use(bool can_use);
  void slot_get_tx_available_can_use(bool can_use);
  void slot_clear_tx_queue_can_use(bool can_use);
  void slot_queue_delay_flag_can_use(bool can_use);
  void slot_get_sen_mode_can_use(bool can_use);
  void slot_send_queue_mode_can_use(bool can_use);
  void slot_auto_send_dev_index_can_use(bool can_use);
  void slot_auto_send_period_can_use(bool can_use);
  void slot_auto_send_add_can_use(bool can_use);
  void slot_auto_send_start_can_use(bool can_use);
  void slot_auto_send_stop_can_use(bool can_use);
  void slot_auto_send_cancel_once_can_use(bool can_use);
  void slot_get_dev_auto_send_list_can_use(bool can_use);

  void on_device_list_comboBox_currentTextChanged(const QString &arg1);
  void on_device_index_comboBox_currentTextChanged(const QString &arg1);
  void on_end_resistance_checkBox_clicked(bool checked);
  void on_bps_comboBox_currentIndexChanged(int index);
  void on_arbitration_bps_comboBox_currentIndexChanged(int index);
  void on_data_bps_comboBox_currentIndexChanged(int index);
  void on_mode_comboBox_currentIndexChanged(int index);
  void on_filter_mode_comboBox_currentIndexChanged(int index);
  void on_verification_code_lineEdit_textChanged(const QString &arg1);
  void on_mask_code_lineEdit_textChanged(const QString &arg1);
  void on_role_comboBox_currentIndexChanged(int index);
  void on_diy_bps_checkBox_clicked(bool checked);
  void on_channel_num_comboBox_currentIndexChanged(int index);
};
#endif // MAINWINDOW_H
