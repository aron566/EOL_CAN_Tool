/**
 *  @file more_window.hpp
 *
 *  @date 2024年01月18日 11:12:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-01-18 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 */
#ifndef MORE_WINDOW_H
#define MORE_WINDOW_H
/** Includes -----------------------------------------------------------------*/
#include <QWidget>
#include <QTimer>
#include <QTextEdit>
#include <QAtomicInt>
#include <QDebug>
/** Private includes ---------------------------------------------------------*/
#include "eol_window.h"
#include "can_driver_model.h"
#include "frame_diagnosis_window/frame_diagnosis.h"
#include "tool_window/tool_window.h"
#include "network_window/network_window.h"
#include "updatefw_window/updatefw_window.h"
#include "utilities/line_highlighter.h"
#include "can_log_sender_window/can_log_sender_window.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

namespace Ui {
class more_window;
}

class more_window : public QWidget
{
  Q_OBJECT

public:
  explicit more_window(QString title, QWidget *parent = nullptr);
  ~more_window();

  /**
   * @brief 设置can驱动
   * @param can_driver_
   */
  void set_can_driver_obj(can_driver_model *can_driver_)
  {
    can_driver_obj = can_driver_;
    /* 设置can驱动 after */
    eol_window_obj->set_can_driver_obj(can_driver_obj);
    updatefw_window_obj->set_can_driver_obj(can_driver_obj);
    can_log_sender_window_obj->set_can_driver_obj(can_driver_obj);

    /* 接收can驱动断开信号 */
    connect(can_driver_obj, &can_driver_model::signal_can_driver_reset, this, &more_window::slot_check_send_timer);
    connect(can_driver_obj, &can_driver_model::signal_can_is_closed, this, &more_window::slot_check_send_timer);

    connect(can_driver_obj, &can_driver_model::signal_show_message_bytes, this, &more_window::slot_show_message_bytes);
    connect(can_driver_obj, &can_driver_model::signal_show_message, this, &more_window::slot_show_message);

    /* 线程同步 */
    connect(can_driver_obj, &can_driver_model::signal_show_thread_message, this, &more_window::slot_show_message_block, Qt::QueuedConnection);

    connect(can_driver_obj, &can_driver_model::signal_show_can_msg, this, &more_window::slot_show_can_msg, Qt::BlockingQueuedConnection);
    connect(can_driver_obj, &can_driver_model::signal_show_can_msg_asynchronous, this, &more_window::slot_show_can_msg);

    /* 设置帧诊断can驱动 */
    frame_diagnosis_obj->set_can_driver_obj(can_driver_);
  }

  /**
   * @brief 设置通道数量
   * @param channel_num 通道数量
   */
  void set_channel_num(quint8 channel_num);

  /**
   * @brief get_network_window_obj
   * @return 网络调试窗口句柄
   */
  network_window *get_network_window_obj()
  {
    return network_window_obj;
  }

public:

protected:
  /**
     * @brief closeEvent
     * @param event
     */
  virtual void closeEvent(QCloseEvent *event);
  virtual void resizeEvent(QResizeEvent *event);
  virtual void wheelEvent(QWheelEvent *event);
  //    virtual void showEvent(QShowEvent *event);

private:

  /**
   * @brief EOL调试子窗口
   * @param titile
   */
  void eol_window_init(QString titile);

  /**
   * @brief 网络调试窗口
   * @param titile
   */
  void network_window_init(QString titile);

  /**
   * @brief 更新固件窗口初始化
   * @param titile
   */
  void updatefw_window_init(QString titile);

  /**
   * @brief 工具子窗口
   * @param titile
   */
  void tool_window_init(QString titile);

  /**
   * @brief 定时器初始化
   */
  void timer_init();

  /**
   * @brief read_cfg
   */
  void read_cfg();

  /**
   * @brief save_cfg
   */
  void save_cfg();

  /**
   * @brief 待显示是否为空
   * @return true空
   */
  bool ch1_show_msg_is_empty();
  bool ch2_show_msg_is_empty();

  /**
   * @brief show_txt
   */
  void show_txt();

  /**
   * @brief 获取显示的索引
   * @param current_show_index
   * @param total_size
   * @return 显示索引
   */
  quint32 get_show_index(quint32 current_show_index, quint32 total_size);

  /**
   * @brief 字符转字符串
   * @param data 字符
   * @param data_len 字符长度
   * @param msg 字符串填充
   * @return true 完整字符串需显示
   */
  bool char2str(const quint8 *data, quint32 data_len, QString &msg);

signals:
  void signal_more_window_closed();

  void signal_can_wave_msg(QByteArray data);
private slots:
  void on_frame_type_comboBox_currentIndexChanged(int index);

  void on_protocol_comboBox_currentIndexChanged(int index);

  void on_canfd_pluse_comboBox_currentIndexChanged(int index);

  void on_id_lineEdit_textChanged(const QString &arg1);

  void on_data_lineEdit_textChanged(const QString &arg1);

  void on_eol_test_pushButton_2_clicked();

  void on_clear_pushButton_clicked();

  void on_send_pushButton_clicked();


private slots:
  void slot_timeout();

  void on_display_mask_lineEdit_textChanged(const QString &arg1);

  void on_mask_en_checkBox_clicked(bool checked);

  /**
     * @brief 发送信号显示当前的can消息
     * @param can_id id
     * @param data 数据
     * @param len 数据长度
     * @param direct 方向
     * @param channel_num 通道号
     * @param protocol_type 协议类型 0 can 1 canfd'
     * @param ms 当前时间
     */
  void slot_can_driver_msg(quint16 can_id, const quint8 *data, quint32 len, \
                           quint8 direct, quint32 channel_num, quint8 protocol_type, quint64 ms);

  void slot_show_message_bytes(quint8 bytes, quint32 channel_num, quint8 direct);
  void slot_show_message(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data = nullptr, quint32 data_len = 0, quint32 can_id = 0);
  void slot_show_message_block(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data = nullptr, quint32 data_len = 0, quint32 can_id = 0);

  /**
     * @brief 刷新显示can消息
     */
  void slot_show_can_msg();

  /**
   * @brief 检测定时发送队列
   */
  void slot_check_send_timer();

  void on_frame_diagnosis_pushButton_clicked();

  void on_export_txt_pushButton_clicked();

  void on_crc_pushButton_clicked();

  void on_tool_pushButton_clicked();

  /**
     * @brief 显示窗口
     */
  void slot_show_window();
  void on_display_str_id_limit_lineEdit_textChanged(const QString &arg1);

  void on_network_test_pushButton_clicked();

  void on_add_send_timer_pushButton_clicked();

  void on_clear_send_timer_pushButton_clicked();

  void on_timer_checkBox_clicked(bool checked);

  void on_update_pushButton_clicked();

  void on_log_send_pushButton_clicked();

private:
  Ui::more_window *ui;
  eol_window *eol_window_obj = nullptr;
  network_window *network_window_obj = nullptr;
  tool_window *tool_window_obj = nullptr;
  updatefw_window *updatefw_window_obj = nullptr;
  can_log_sender_window *can_log_sender_window_obj = nullptr;
private:
  can_driver_model *can_driver_obj = nullptr;
  frame_diagnosis *frame_diagnosis_obj = nullptr;

  QTimer *timer_obj = nullptr;

  quint32 rx_frame_cnt = 0;
  quint32 tx_frame_cnt = 0;
  quint32 rx_byte_cnt = 0;
  quint32 tx_byte_cnt = 0;
  quint32 last_canid_mask = 0;
  bool last_canid_mask_en = false;

  typedef struct
  {
    QString str;
    quint32 channel_num;
    quint8 direct;
  }SHOW_MSG_Typedef_t;

  line_highlighter ch1_line_highlighter;
  QList<SHOW_MSG_Typedef_t>ch1_show_msg_list;

  line_highlighter ch2_line_highlighter;
  QList<SHOW_MSG_Typedef_t>ch2_show_msg_list;

  /* 字符显示 */
  QString show_line_str;
  bool show_line_str_force = false;
  quint64 last_show_line_str_time_ms = 0;
  quint64 current_show_line_str_time_ms = 0;
  quint32 limit_str_canid = 0xFFFF;
private:
  /**
     * @brief update_show_msg 显示指定索引消息
     * @param text_edit_widget 控件
     * @param pList 消息链表
     * @param show_index 索引
     * @param downward_flag true 下翻标识 false 上翻标识
     */
  void update_show_msg(QPlainTextEdit *text_edit_widget, QList<SHOW_MSG_Typedef_t> *pList, quint32 show_index, bool downward_flag);

  /**
     * @brief 帧诊断窗口初始化
     * @param title 窗口标题
     */
  void frame_diagnosis_window_init(QString title);

  /**
     * @brief can log文件发送窗口初始化
     * @param title 窗口标题
     */
  void can_log_sender_window_init(QString title);

  /**
     * @brief 显示can消息
     * @param msg 消息
     */
  void show_can_msg(can_driver_model::CAN_MSG_DISPLAY_Typedef_t &msg);
private:
  /* 已显示消息 */
  quint32 ch1_show_msg_index = 0;
  quint32 ch2_show_msg_index = 0;
  /* 已添加消息 */
  quint32 ch1_add_msg_index = 0;
  quint32 ch2_add_msg_index = 0;

  /* 滚动计数 */
  quint32 ch1_scroll_cnt = 0;
  /* 滚动计数 */
  quint32 ch2_scroll_cnt = 0;
};

#endif // MORE_WINDOW_H
/******************************** End of file *********************************/
