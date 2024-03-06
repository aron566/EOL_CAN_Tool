/**
 *  @file can_log_sender_window.hpp
 *
 *  @date 2024年03月05日 11:12:45 星期二
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief can log发送.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-03-05 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 */
#ifndef CAN_LOG_SENDER_WINDOW_H
#define CAN_LOG_SENDER_WINDOW_H
/** Includes -----------------------------------------------------------------*/
#include <QWidget>
#include <QFile>
#include <QAtomicInt>
#include <QThread>
#include <QRunnable>
#include <QDebug>
/** Private includes ---------------------------------------------------------*/
#include "can_driver_model.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class can_driver_model;

namespace Ui {
class can_log_sender;
}

class can_log_sender_window : public QWidget, public QRunnable
{
  Q_OBJECT

public:
  explicit can_log_sender_window(QString title, QWidget *parent = nullptr);
  ~can_log_sender_window();

  /**
   * @brief eol协议窗口线程启动
   */
  virtual void run() override
  {
    thread_run_state = true;
    while(run_state)
    {
      can_log_sender_task();
    }
    qDebug() << "[thread]" << QThread::currentThreadId() << "can log sender window end";
    thread_run_state = false;

    /* 原子操作 */
    if(thread_run_statex.testAndSetRelaxed(1, 0))
    {

    }
    else
    {

    }
  }

  /**
   * @brief 设置can驱动接口
   * @param can_driver_obj
   */
  void set_can_driver_obj(can_driver_model *_can_driver_obj);

  /**
   * @brief read_cfg
   */
  void read_cfg();

  /**
   * @brief save_cfg
   */
  void save_cfg();
private:
  /**
   * @brief can_log_sender_task
   */
  void can_log_sender_task();

  /**
   * @brief 超时检测
   * @param start_time
   * @param time_out_ms
   * @return 即将超时剩余时间
   */
  quint64 is_timeout(quint64 start_time, quint64 time_out_ms);

protected:

  /**
   * @brief closeEvent
   * @param event
   */
  virtual void closeEvent(QCloseEvent *event) override;

signals:
  /**
   * @brief 发送窗口关闭信号
   */
  void signal_window_closed();

  void signal_update_progress(qint32 size);

private slots:
  void on_set_file_pushButton_clicked();

  void on_start_pushButton_clicked();

  void slot_update_progress(qint32 size);
private:
  Ui::can_log_sender *ui;
  can_driver_model *can_driver_obj = nullptr;
private:
  QFile file;             /**< 文件对象 */
  QString filename;       /**< 打开的文件名称 */
  quint64 filesize;       /**< 文件大小 */
  QString last_file_path = "../"; /**< 上次打开文件夹 */

  QAtomicInt thread_run_statex;
  bool run_state = false;
  bool thread_run_state = false;

private:
  // QString split_str;
  QList<quint32>can_id_permissable_list;
  QList<quint32>can_id_response_list;
  QList<quint32>wait_can_data_list;
  quint32 wait_can_data_index = 0;
  quint32 wait_response_delay_ms = 0;
  quint32 can_id_field_index = 0;
  quint32 send_bytes = 0;
  quint32 data_field_index_s = 0;
  quint8 send_channel = 0;
  quint32 send_delay_ms = 0;
};

#endif // CAN_LOG_SENDER_WINDOW_H
/******************************** End of file *********************************/


