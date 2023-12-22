/**
 *  @file network_window.hpp
 *
 *  @date 2023年11月24日 15:24:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief 网络调试窗口.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-11-24 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 */
#ifndef NETWORK_WINDOW_H
#define NETWORK_WINDOW_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
//#include <limits.h> /**< need variable max value    */
//#include <stdalign.h> /**< need alignof    */
//#include <stdarg.h> /**< need va_start    */
//#include <ctype.h> /**< need isalpha isdigit */
//#include <stdatomic.h> /**< need atomic_compare_exchange_weak */
//#include <assert.h> /**< need assert( a > b ); */
//#include <setjmp.h> /**< need jmp_buf buf setjmp(buf); longjmp(buf,1) */
/** Private includes ---------------------------------------------------------*/
#include <QWidget>
#include <QTimer>
#include <QTextEdit>
#include <QDebug>
#include "network_driver/network_driver_tcp.h"
#include "network_driver/network_driver_udp.h"
#include "utilities/line_highlighter.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

namespace Ui {
class network_window;
}

class network_window : public QWidget
{
  Q_OBJECT

public:
  explicit network_window(QString title, QWidget *parent = nullptr);
  ~network_window();

  /* 网络设备类型 */
  typedef enum
  {
    RTS_NETWORK_DEVICE = 0,
    PLC_NETWORK_DEVICE,
    EXTERN_NETWORK_DEVICE,
    NETWORK_DEIVCE_MAX,
  }NETWORK_DEVICE_Typedef_t;

  /* 网络设备信息 */
  typedef struct
  {
    QString ip;
    QString port;
    network_driver_model::NETWORK_WORK_ROLE_Typedef_t role;
    network_driver_model::NETWORK_TYPE_Typedef_t net_type;
    NETWORK_DEVICE_Typedef_t device_type;
    network_driver_model *network_driver_obj;/**< 网络驱动sernd */
    network_driver_model *network_driver_rec_obj;/**< 网络驱动receive for rts */
  }NETWORK_DEVICE_INFO_Typedef_t;

  /**
   * @brief set_network_par 设置网络参数加入列表
   * @param ip
   * @param port
   * @param role
   * @param net_type
   * @param device_type
   */
  void set_network_par(const QString &ip, const QString &port,
                       network_driver_model::NETWORK_WORK_ROLE_Typedef_t role,
                       network_driver_model::NETWORK_TYPE_Typedef_t net_type,
                       NETWORK_DEVICE_Typedef_t device_type);

  /**
   * @brief network_start 启动网络
   * @param device_type 设备类型
   * @return true成功
   */
  bool network_start(NETWORK_DEVICE_Typedef_t device_type);

  /**
   * @brief network_stop 停止网络
   * @param device_type 设备类型
   * @return true成功
   */
  bool network_stop(NETWORK_DEVICE_Typedef_t device_type);

  /**
   * @brief get_device_obj_info 获取网络驱动信息
   * @param device_type 设备类型
   * @param ok 是否ok
   * @return 网络驱动信息
   */
  NETWORK_DEVICE_INFO_Typedef_t get_device_obj_info(NETWORK_DEVICE_Typedef_t device_type, bool *ok = nullptr)
  {
    NETWORK_DEVICE_INFO_Typedef_t t;
    t.network_driver_obj = nullptr;
    t.network_driver_rec_obj = nullptr;
    qint32 index = get_device_obj(device_type);
    if(-1 != index)
    {
      if(nullptr != ok)
      {
        *ok = true;
      }
      return network_device_list.value(index);
    }
    if(nullptr != ok)
    {
      *ok = false;
    }
    return t;
  }
protected:

  /**
   * @brief closeEvent
   * @param event
   */
  virtual void closeEvent(QCloseEvent *event);
  virtual void wheelEvent(QWheelEvent *event);

signals:
  /**
   * @brief 发送窗口关闭信号
   */
  void signal_window_closed();

  /**
   * @brief signal_network_start
   * @param device_type
   */
  void signal_network_start(NETWORK_DEVICE_Typedef_t device_type);

  /**
   * @brief signal_network_stop
   * @param device_type
   */
  void signal_network_stop(NETWORK_DEVICE_Typedef_t device_type);

  /**
   * @brief signal_net_wave_msg
   * @param data
   */
  void signal_net_wave_msg(QByteArray data);
private:
  /* 消息框显示 */
  typedef struct
  {
    QString str;
    quint32 channel_num;
    quint8 direct;
  }SHOW_MSG_Typedef_t;

private:

  /**
   * @brief 定时器初始化
   */
  void timer_init();

  /**
   * @brief save_cfg
   */
  void save_cfg();

  /**
   * @brief read_cfg
   */
  void read_cfg();

  /**
   * @brief update_show_msg
   * @param text_edit_widget
   * @param pList
   * @param show_index
   * @param downward_flag
   */
  void update_show_msg(QPlainTextEdit *text_edit_widget, QList<SHOW_MSG_Typedef_t> *pList, quint32 show_index, bool downward_flag);

  /**
   * @brief ch1_show_msg_is_empty
   * @return
   */
  bool ch1_show_msg_is_empty();

  /**
   * @brief ch2_show_msg_is_empty
   * @return
   */
  bool ch2_show_msg_is_empty();

  /**
   * @brief get_show_index
   * @param current_show_index
   * @param totaol_size
   * @return
   */
  quint32 get_show_index(quint32 current_show_index, quint32 totaol_size);

  /**
   * @brief char2str 字符转字符串
   * @param data
   * @param data_len
   * @param msg
   * @return
   */
  bool char2str(const quint8 *data, quint32 data_len, QString &msg);

  /**
   * @brief show_txt
   */
  void show_txt();

  /**
   * @brief get_device_obj
   * @param device_type
   * @return 返回对象信息所在index
   */
  qint32 get_device_obj(NETWORK_DEVICE_Typedef_t device_type);

public slots:
  /**
   * @brief 消息信号处理
   * @param str 消息数据
   * @param channel_num 消息通道号 0上框客户端 1下框服务端
   * @param direct 消息方向，0发送（红色） or 1接收（白色） 0xFF状态消息
   * @param data 数据
   * @param data_len 数据长度
   * @param ip
   */
  void slot_show_message(const QString &message, quint32 channel_num, \
                         quint8 direct, const quint8 *data, \
                         quint32 data_len, QString ip);
  void slot_show_message_block(const QString &message, quint32 channel_num, \
                               quint8 direct, const quint8 *data, \
                               quint32 data_len, QString ip);

  /**
   * @brief slot_show_message_bytes
   * @param bytes 字节数
   * @param channel_num 消息通道号 0上框客户端 1下框服务端
   * @param direct 消息方向，0发送（红色） or 1接收（白色） 0xFF状态消息
   */
  void slot_show_message_bytes(quint32 bytes, quint32 channel_num, quint8 direct);

private slots:

  /**
   * @brief slot_timeout
   */
  void slot_timeout();

  void on_hex_lineEdit_editingFinished();

  void on_str_lineEdit_editingFinished();

  void on_period_lineEdit_editingFinished();

  void on_send_pushButton_clicked();

  void on_clear_pushButton_clicked();

  void on_export_txt_pushButton_clicked();

  void on_crc_pushButton_clicked();

  void on_role_comboBox_currentIndexChanged(int index);

  void on_net_type_comboBox_currentIndexChanged(int index);

private:
  Ui::network_window *ui;

private:

  QTimer *timer_obj = nullptr;/**< 定时器 */

  quint32 rx_frame_cnt = 0;
  quint32 tx_frame_cnt = 0;
  quint32 rx_byte_cnt = 0;
  quint32 tx_byte_cnt = 0;

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
  line_highlighter ch1_line_highlighter;
  QList<SHOW_MSG_Typedef_t>ch1_show_msg_list;

  line_highlighter ch2_line_highlighter;
  QList<SHOW_MSG_Typedef_t>ch2_show_msg_list;

  /* 字符显示 */
  QString show_line_str;
  bool show_line_str_force = false;
  quint64 last_show_line_str_time_ms = 0;
  quint64 current_show_line_str_time_ms = 0;

  QList<NETWORK_DEVICE_INFO_Typedef_t>network_device_list;
};

#endif // NETWORK_WINDOW_H
/******************************** End of file *********************************/
