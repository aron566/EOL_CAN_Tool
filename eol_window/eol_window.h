#ifndef EOL_WINDOW_H
#define EOL_WINDOW_H

#include <QWidget>
#include <QTimer>
#include <QList>
#include <QFile>
#include <QThreadPool>
#include <QThread>
#include <QRunnable>
#include <eol_protocol.h>

namespace Ui {
class eol_window;
}

class eol_window : public QWidget, public QRunnable
{
  Q_OBJECT

public:
  explicit eol_window(QString titile, QWidget *parent = nullptr);
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
   * @brief 设置协议栈
   * @param eol_protocol_
   */
  void set_eol_protocol(eol_protocol *eol_protocol_)
  {
    eol_protocol_obj = eol_protocol_;
  }

  /**
   * @brief eol协议窗口线程启动
   */
  virtual void run() override
  {
    thread_run_state = true;
    while(run_state)
    {
      run_eol_window_task();
      QThread::msleep(15);
    }
    qDebug() << "[thread]" << QThread::currentThreadId() << " eol window end";
    thread_run_state = false;
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

public slots:
  /**
   * @brief 从机无应答信号
   */
  void slot_protocol_no_response();

  void slot_protocol_error_occur(quint8 error_msg);

  /**
   * @brief 表数据
   * @param frame_num 帧号
   * @param data 数据
   * @param data_len 数据长度
   */
  void slot_recv_eol_table_data(quint16 frame_num, const quint8 *data, quint16
                                   data_len);

  void slot_send_progress(quint32 current_size, quint32 total_size);

  /**
   * @brief 接收数据完成
   */
  void slot_recv_eol_data_complete();
private:
  quint32 time_cnt = 0;

  typedef struct
  {
    QString file_path;
    QString file_name;
    QString show_info;
    int table_type;
    int data_type;
  }TABLE_INFO_Typedef_t;
  QList <TABLE_INFO_Typedef_t>table_list;

  QString current_file_path;
  QFile current_file;
  QString current_file_name;
  quint64 current_filesize;

  QStringList num_str_list;

  QString last_file_path;
  bool run_state = false;

  QThreadPool *g_thread_pool = nullptr;
  eol_protocol *eol_protocol_obj = nullptr;

  bool thread_run_state = false;

  qint8 num_buf[25 * 1024];

  typedef struct
  {
    quint16 angle_points;
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

  SV_TABLE_INFO_Typedef_t sv_table_info;

  ANT_TABLE_INFO_Typedef_t ant_table_info;

  eol_protocol::DOA_TABLE_HEADER_Typedef_t table_info;

private:
  /**
   * @brief 定时器初始化
   */
  void timer_init();

  /**
   * @brief 重置显示
   */
  void reset_ui_info();

  /**
   * @brief 更新显示列表
   */
  void update_show_table_list();

  /**
   * @brief csv文件解析
   * @param 文件路径
   * @param table_type_index 表类型
   * @return true执行正确
   */
  bool csv_analysis(QString &file_path, int table_type_index, int data_type);

  /**
   * @brief csv头部解析
   * @param data 数据
   * @param table_type_index 表类型
   */
  void csv_header_analysis(QByteArray &data, int table_type_index);

  /**
   * @brief csv数据解析
   * @param data 数据内容
   * @param line_num 行号，从1开始
   * @param table_type_index 表类型
   * @param data_type 数据类型
   */
  void csv_data_analysis(QByteArray &data, quint64 line_num, int table_type_index, int data_type);

  /**
   * @brief 执行eol协议
   */
  void run_eol_window_task();
signals:
  void signal_eol_window_closed();

  /**
   * @brief 更新列表
   */
  void signal_update_show_table_list();
private slots:
  void slot_timeout();
  void slot_update_show_table_list();
  void on_file_sel_pushButton_clicked();
  void on_upload_pushButton_clicked();
  void on_update_pushButton_clicked();
  void on_add_list_pushButton_clicked();
  void on_clear_list_pushButton_clicked();
};

#endif // EOL_WINDOW_H
