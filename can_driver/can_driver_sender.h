/**
 *  @file can_driver_sender.hpp
 *
 *  @date 2024年01月08日 11:54:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-01-08 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 */
#ifndef CAN_DRIVER_SENDER_H
#define CAN_DRIVER_SENDER_H
/** Includes -----------------------------------------------------------------*/
#include <QObject>
#include <QMutex>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QSemaphore>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>
/** Private includes ---------------------------------------------------------*/
#include <can_driver_model.h>
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/
class can_driver_sender : public QObject, public QRunnable
{
  Q_OBJECT
public:
  explicit can_driver_sender(can_driver_model *obj, QObject *parent = nullptr);

  ~can_driver_sender()
  {
    start_ = false;
    clear_flag = true;
    qDebug() << "del can driver sender";
  }
public:
  /**
   * @brief can线程启动
   */
  virtual void run() override
  {
    thread_run_state = true;
    while(start_)
    {
      send_data_task();
      QThread::msleep(1);
    }
    qDebug() << "[thread]" << QThread::currentThreadId() << "can driver sender task end";

    /* 原子操作 */
    if(thread_run_statex.testAndSetRelaxed(1, 0))
    {
      // qDebug() << "thread_run_statex was successfully updated.";
    }
    else
    {
      // qDebug() << "thread_run_statex was not updated.";
    }
    period_send_msg_list.clear();
    thread_run_state = false;
  }

  void stop_task()
  {
    start_ = false;
    qDebug() << "can driver sender stop_task";
  }

  /**
   * @brief task_is_running
   * @return true正在运行
   */
  bool task_is_running()
  {
    return thread_run_state;
  }

  /**
   * @brief start_task
   * @return bool true成功
   */
  bool start_task()
  {
    bool ret = false;
    /* 原子操作 */
    if(thread_run_statex.testAndSetRelaxed(0, 1))
    {
      ret = true;
    }
    else
    {
      ret = false;
      // qDebug() << "can driver sender task is running";
      return ret;
    }

    start_ = true;
    QThreadPool::globalInstance()->start(this);
    return ret;
  }

  /**
   * @brief period_send_set
   * @param send_task
   */
  void period_send_set(const can_driver_model::PERIOD_SEND_MSG_Typedef_t &send_task)
  {
    QMutexLocker locker(&m_mutex);
    /* 增加 */
    period_send_msg_list.append(send_task);
  }

  /**
   * @brief 获取周期发送列表长度
   * @return 长度
   */
  qint32 get_period_send_list_size()
  {
    return period_send_msg_list.size();
  }

  void period_send_list_clear()
  {
    clear_flag = true;
  }

private:
  bool start_ = false;
  bool clear_flag = false;
  bool thread_run_state = false;
  can_driver_model *can_driver_obj = nullptr;
  QList<can_driver_model::PERIOD_SEND_MSG_Typedef_t>period_send_msg_list;
  QAtomicInt thread_run_statex;
  QMutex m_mutex;

signals:

  /**
   * @brief signal_sender_update_ui
   */
  void signal_sender_update_ui();

public:
  /**
   * @brief send_data_task
   */
  void send_data_task();
};

#endif // CAN_DRIVER_SENDER_H
/******************************** End of file *********************************/
