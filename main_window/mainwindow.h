#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QThreadPool>
#include "more_window.h"
#include "eol_window.h"
#include "can_driver_model.h"
#include "can_driver_zlg.h"
#include "can_driver_gc.h"
#include "can_driver_ts.h"
#include "can_driver_kvaser.h"
#include "eol_protocol.h"
#include "updater_window.h"

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

  updater_window *updater_window_obj = nullptr;
private:
  can_driver_model *can_driver_obj = nullptr;

  /**
   * @brief fontDb
   */
  QFontDatabase fontDb;
private:

  /**
   * @brief 载入字体
   */
  void font_file_load();

    /**
   * @brief 更新子窗口初始化
   * @param titile
   */
  void updater_window_init(QString titile);

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
  void read_cfg();

  /**
   * @brief 保存参数
   */
  void save_cfg();

  /**
   * @brief update_can_use 更新页面状态显示
   * @param function_can_use
   */
  void update_can_use(can_driver_model::SET_FUNCTION_CAN_USE_Typedef_t &function_can_use);
private slots:
  /**
   * @brief 显示主窗口
   *
   */
  void slot_show_this_window();

  /**
   * @brief 显示最新版本信息
   * @param version
   * @param change_log
   */
  void slot_lasted_version_info(QString version, QString change_log);

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
  void on_brand_comboBox_currentIndexChanged(int index);
  void on_device_info_pushButton_clicked();
  void on_updater_pushButton_clicked();
  void on_rts_start_pushButton_clicked();
  void on_plc_start_pushButton_clicked();
  void on_plc_stop_pushButton_clicked();
  void on_rts_stop_pushButton_clicked();
  void on_rts_role_comboBox_currentIndexChanged(int index);
  void on_plc_role_comboBox_currentIndexChanged(int index);
};
#endif // MAINWINDOW_H
