#ifndef EOL_WINDOW_H
#define EOL_WINDOW_H

#include <QWidget>
#include <QTimer>
#include <QList>
#include <QFile>
#include <QThreadPool>
#include <QThread>
#include <QRunnable>
#include <QAtomicInt>
#include <QTemporaryFile>
#include <eol_protocol.h>
#include "eol_sub_window.h"
#include "eol_calibration_window.h"
#include "eol_angle_calibration_window/eol_angle_calibration_window.h"
#include "debug_window/debug_window.h"
#include "rts_ctrl_window/rts_ctrl_window.h"
#include "rts_protocol/rts_protocol.h"
#include "network_driver/network_driver_model.h"

namespace Ui {
class eol_window;
}

class eol_window : public QWidget, public QRunnable
{
  Q_OBJECT

public:
  explicit eol_window(QString title, QWidget *parent = nullptr);
  ~eol_window();

  /**
   * @brief 设置线程池
   * @param g_thread_pool_
   */
  void set_thread_pool(QThreadPool *g_thread_pool_)
  {
    g_thread_pool = g_thread_pool_;
  }

  /**
   * @brief 设置can驱动接口
   * @param can_driver_obj
   */
  void set_can_driver_obj(can_driver_model *can_driver_obj);

  /**
   * @brief 设置rts网络驱动接口
   * @param network_driver_send_obj 发送
   * @param network_driver_rec_obj 接收
   */
  void set_rts_driver_obj(network_driver_model *network_driver_send_obj, network_driver_model *network_driver_rec_obj);

  /**
   * @brief 设置rts网络驱动接口
   * @param can_driver_obj
   */
//  void set_plc_driver_obj(network_driver_model *network_driver_obj);

  /**
   * @brief eol协议窗口线程启动
   */
  virtual void run() override
  {
    thread_run_state = true;
    while(run_state)
    {
      /* 检查任务合法性 */
      switch(current_running_task)
      {
        case UPDATE_TABLE_TASK:
          run_eol_window_file_decode_task();
          break;
        case UPLOAD_ALL_TABLE_TASK:
          run_eol_window_export_task();
          break;
        case UPLOAD_TABLE_TASK:
          run_eol_window_export_one_table_task();
          break;
        case NO_TASK_RUNNING:
        default:
          /* todo nothing */
          break;
      }
      current_running_task = NO_TASK_RUNNING;
    }
    qDebug() << "[thread]" << QThread::currentThreadId() << "eol window end";
    thread_run_state = false;

    /* 原子操作 */
    if(thread_run_statex.testAndSetRelaxed(1, 0))
    {
//      qDebug() << "eol window thread_run_statex was successfully updated.";
    }
    else
    {
//      qDebug() << "eol window thread_run_statex was not updated.";
    }
  }
protected:
    /**
     * @brief closeEvent
     * @param event
     */
    virtual void closeEvent(QCloseEvent *event) override;
private:
  Ui::eol_window *ui;

  QTimer *timer_obj = nullptr;

private slots:
  /**
   * @brief 协议栈无回复超时
   * @param sec 秒
   */
  void slot_protocol_timeout(quint32 sec);

  /**
   * @brief 从机无应答信号
   */
  void slot_protocol_no_response();

  /**
   * @brief crc检测到错误
   */
  void slot_protocol_crc_check_failed();

  /**
   * @brief ack错误反馈
   * @param error_msg
   */
  void slot_protocol_error_occur(quint8 error_msg);

  /**
   * @brief 表数据
   * @param frame_num 帧号
   * @param data 数据
   * @param data_len 数据长度
   */
  void slot_recv_eol_table_data(quint16 frame_num, const quint8 *data, quint16
                                   data_len);

  /**
   * @brief 当前发送进度
   * @param current_size 当前大小
   * @param total_size 总大小
   */
  void slot_send_progress(quint32 current_size, quint32 total_size);

  /**
   * @brief 发送eol数据完成
   */
  void slot_send_eol_data_complete();

  /**
   * @brief 接收数据完成
   */
  void slot_recv_eol_data_complete();

  /**
   * @brief 设备当前模式
   * @param pass_data 返回数据[0]模式 [1]配置数 [2]天线通道数
   */
  void slot_device_mode(const void *pass_data);

  /**
   * @brief 传输一帧触发计数
   */
  void slot_send_rec_one_frame();

  /**
   * @brief 从机读写无反应
   * @param reg 读写寄存器地址
   * @param command 读写标识
   */
  void slot_protocol_rw_err(quint8 reg, quint8 command);

  /**
   * @brief 读写设备正常
   * @param reg 寄存器
   * @param data 读出的数据，为nullptr代表写设备成功
   * @param data_len 代表数据长度，为0时代表写设备成功
   */
  void slot_rw_device_ok(quint8 reg, const quint8 *data = nullptr, quint16 data_len = 0);

  /**
   * @brief eol协议栈已启动
   */
  void slot_eol_protol_is_start();


  /**
   * @brief 协议栈无回复超时
   * @param sec 秒
   */
  void slot_rts_protocol_timeout(quint32 sec);

  /**
   * @brief 从机返回错误消息
   * @param error_msg 错误消息
   */
  void slot_rts_protocol_error_occur(quint8 error_msg);

  /**
   * @brief signal_protocol_rw_err 读写错误信号
   * @param cmd 命令
   */
  void slot_rts_protocol_rw_err(QString cmd);
private:
  quint32 time_cnt = 0;

  typedef enum
  {
    CSV_HEADER_COMMENT_DECODE = 0,  /**< 解析头部注释 */
    CSV_HEADER_DATA_DECODE,         /**< 解析头部信息 */
    CSV_DATA_DECODE,                /**< 解析csv数据 */
  }CSV_DECODE_STEP_Typedef_t;

  typedef struct
  {
    QString show_info;
    quint8 table_type;
  }TABLE_LIST_Typedef_t;

  typedef struct
  {
    QString file_path;
    QString file_name;
    QList <TABLE_LIST_Typedef_t>table_list;
  }CSV_INFO_Typedef_t;
  QList <CSV_INFO_Typedef_t> csv_list;

  QString current_file_path;
  QFile current_file;
  QString current_file_name;
  quint64 current_filesize;

  QStringList num_str_list;

  QString last_file_path;
  bool run_state = false;

  QThreadPool *g_thread_pool = nullptr;
  eol_protocol *eol_protocol_obj = nullptr;
  eol_sub_window *eol_sub_window_obj = nullptr;
  eol_calibration_window *eol_calibration_window_obj = nullptr;
  eol_angle_calibration_window *eol_2dfft_calibration_window_obj = nullptr;
  debug_window *debug_window_window_obj = nullptr;
  rts_protocol *rts_protocol_obj = nullptr;
  rts_ctrl_window *rts_ctrl_window_obj = nullptr;
  network_driver_model *rts_network_driver_send_obj = nullptr;
  network_driver_model *rts_network_driver_rec_obj = nullptr;

  bool thread_run_state = false;

  quint8 *num_buf = nullptr;

  typedef struct
  {
    quint16 angle_points;
    qint8 start_angle;
    qint8 end_angle;
    quint8 channel_num;
    int table_index;
    int data_type;
    quint32 data_size;
  }SV_TABLE_INFO_Typedef_t;

  typedef struct
  {
    quint8 channel_num;
    int table_index;
    int data_type;
    quint32 data_size;
  }ANT_TABLE_INFO_Typedef_t;

  eol_protocol::DOA_TABLE_HEADER_Typedef_t table_info;
  eol_protocol::SYS_PATTERN_TABLE_HEADER_Typedef_t pattern_table_info;
  eol_protocol::SYS_NOISE_TABLE_HEADER_Typedef_t noise_table_info;
  eol_protocol::ANT_TABLE_HEADER_Typedef_t ant_table_info;
  eol_protocol::COMMON_TABLE_HEADER_Typedef_t common_table_info;

  quint32 err_constantly_cnt = 0;

  QFile recv_file;
  QFile recv_file_origin;
  QTemporaryFile tmpFile; /**< 临时文件 */

  typedef enum
  {
    TASK_RUNNING = 0,
    TASK_COMPLETE,
    TASK_ERROR,
  }TASK_STATE_Tyedef_t;

  TASK_STATE_Tyedef_t current_task_complete_state = TASK_COMPLETE;

  QFile all_table_file;
  QString all_table_file_name = "";

  typedef enum
  {
    UPDATE_TABLE_TASK = 0,  /**< 写入表 */
    UPLOAD_TABLE_TASK,      /**< 读取表 */
    UPLOAD_ALL_TABLE_TASK,      /**< 读取全部表 */
    NO_TASK_RUNNING,        /**< 空闲 */
  }EOL_WINDOW_TASK_Typedef_t;

  EOL_WINDOW_TASK_Typedef_t current_running_task = NO_TASK_RUNNING;
  QAtomicInt thread_run_statex;
private:
  /**
   * @brief 设备信息读写窗口
   * @param title 窗口标题
   */
  void eol_sub_window_init(QString title);

  /**
   * @brief rcs校准窗口初始化
   * @param title 窗口标题
   */
  void eol_rcs_calibration_window_init(QString title);

  /**
   * @brief 2dfft校准窗口初始化
   * @param title 窗口标题
   */
  void eol_2dfft_calibration_window_init(QString title);

  /**
   * @brief 调试窗口初始化
   * @param title 窗口标题
   */
  void eol_debug_window_init(QString title);

  /**
   * @brief RTS控制页面初始化
   * @param title 窗口标题
   */
  void rts_ctrl_window_init(QString title);

  /**
   * @brief eol协议栈初始化
   * @param can_driver_obj can驱动接口
   */
  void eol_protocol_init(can_driver_model *can_driver_obj);

  /**
   * @brief rts协议栈初始化
   * @param network_driver_send_obj 网络驱动send接口
   * @param network_driver_rec_obj 网络驱动rec接口
   */
  void rts_protocol_init(network_driver_model *network_driver_send_obj, network_driver_model *network_driver_rec_obj);

  /**
   * @brief 保存配置
   */
  void save_cfg();

  /**
   * @brief 恢复配置
   */
  void read_cfg();

  /**
   * @brief 定时器初始化
   */
  void timer_init();

  /**
   * @brief 重置传输信息显示
   */
  void reset_base_ui_info();

  /**
   * @brief 更新显示列表
   */
  void update_show_table_list();

  /**
   * @brief csv文件解析
   * @param 文件路径
   * @param csv csv文件信息、
   * @param csv_file_index文件在列表中索引
   * @param check_table_en 是否检测表头，使能时只检测csv表信息
   * @return true执行正确
   */
  bool csv_analysis(QString &file_path, CSV_INFO_Typedef_t &csv, int csv_file_index, bool check_table_en = false);

  /**
   * @brief csv头部解析
   * @param data 数据
   * @param table_type_index 表类型
   * @return 表类型
   */
  eol_protocol::TABLE_Typedef_t csv_header_analysis(QByteArray &data, int table_type_index = 0xFF);

  /**
   * @brief csv数据解析
   * @param data 数据内容
   * @param line_num 行号，从1开始
   * @param table_type_index 表类型
   * @param data_type 数据类型
   * @return true 成功
   */
  bool csv_data_analysis(QByteArray &data, quint64 line_num, int table_type_index, int data_type);

  /**
   * @brief 一键导出所有表
   * @param frame_num 帧计数
   * @param data 数据
   * @param is_table_header 是表头
   * @return 导出成功
   */
  bool one_key_rec_all_table_data_silent(quint16 frame_num, const QByteArray &data, bool is_table_header = false);

  /**
   * @brief 执行eol文件解析任务
   */
  void run_eol_window_file_decode_task();

  /**
   * @brief 执行eol单个导出任务
   */
  void run_eol_window_export_one_table_task();

  /**
   * @brief 执行eol导出任务
   */
  void run_eol_window_export_task();

signals:
  void signal_eol_window_closed();

  /**
   * @brief 更新列表
   */
  void signal_update_show_table_list();

  /**
   * @brief 更新校准配置信息
   * @param info 信息
   */
  void signal_profile_info_update(eol_protocol::CALIBRATION_PROFILE_INFO_Typedef_t &info);

  /**
   * @brief 清除配置信息
   */
  void signal_clear_profile_info();

  /**
   * @brief 导出表任务完成
   */
  void signal_upload_all_table_task_ok();

private slots:
  void slot_show_this_window();
  void slot_close_shell_window();
  void slot_send_command(QString text);
  void slot_send_command_char(char c);
  void slot_timeout();
  void slot_update_show_table_list();
  void on_file_sel_pushButton_clicked();
  void on_upload_pushButton_clicked();
  void on_update_pushButton_clicked();
  void on_add_list_pushButton_clicked();
  void on_clear_list_pushButton_clicked();
  void on_entry_produce_mode_pushButton_clicked();

  void on_eol_device_rw_func_pushButton_clicked();
  void on_ant_calibration_func_pushButton_clicked();
  void on_rcs_calibration_func_pushButton_clicked();
  void on_reboot_pushButton_clicked();
  void on_debug_pushButton_clicked();
  void on_com_hw_comboBox_currentIndexChanged(int index);
  void on_com_config_lineEdit_textChanged(const QString &arg1);
  void on_vcom_config_lineEdit_textChanged(const QString &arg1);
  void on_dev_addr_lineEdit_textChanged(const QString &arg1);
  void on_export_all_pushButton_clicked();
  void on_rts_crl_pushButton_clicked();
  void on_open_rts_pushButton_clicked();
  void on_close_rts_pushButton_clicked();
  void on_rts_addr_lineEdit_editingFinished();
};

#endif // EOL_WINDOW_H
