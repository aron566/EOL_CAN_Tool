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
// #include <limits.h> /**< need variable max value    */
// #include <stdalign.h> /**< need alignof    */
// #include <stdarg.h> /**< need va_start    */
// #include <ctype.h> /**< need isalpha isdigit */
// #include <stdatomic.h> /**< need atomic_compare_exchange_weak */
// #include <assert.h> /**< need assert( a > b ); */
// #include <setjmp.h> /**< need jmp_buf buf setjmp(buf); longjmp(buf,1) */
/** Private includes ---------------------------------------------------------*/
#include <QObject>
#include <QMutex>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QSemaphore>
#include <QDebug>
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
  }
public:
  /**
   * @brief can线程启动
   */
  virtual void run() override
  {
    while(start_)
    {
      send_data_task();
      QThread::usleep(0);
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
  }

  void stop_task()
  {
    period_send_list_clear();
    start_ = false;
    qDebug() << "can driver sender stop_task";
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
  can_driver_model *can_driver_obj = nullptr;
  QList<can_driver_model::PERIOD_SEND_MSG_Typedef_t>period_send_msg_list;
  QAtomicInt thread_run_statex;
signals:

private:

  /**
   * @brief send_data_task
   */
  void send_data_task();
};

#endif // CAN_DRIVER_SENDER_H
/******************************** End of file *********************************/





