/**
 *  @file block_queue.hpp
 *
 *  @date 2024年03月07日 11:12:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-03-07 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 */
#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H
/** Includes -----------------------------------------------------------------*/
#include <QCoreApplication>
#include <QWaitCondition>
#include <QQueue>
#include <QMutex>
#include <QThread>
#include <QDebug>
/** Private includes ---------------------------------------------------------*/

/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

template <typename T>
class block_queue
{
public:
  block_queue() {}

  void put(const T& value)
  {
    QMutexLocker locker(&m_mutex);
    m_queue.enqueue(value);
    m_condition.wakeOne();
  }

  T take()
  {
    QMutexLocker locker(&m_mutex);
    while (m_queue.isEmpty())
    {
      m_condition.wait(&m_mutex);
    }
    return m_queue.dequeue();
  }

  bool is_empty() const
  {
    QMutexLocker locker(&m_mutex);
    return m_queue.isEmpty();
  }

  int size() const
  {
    QMutexLocker locker(&m_mutex);
    return m_queue.size();
  }

private:
  QQueue<T> m_queue;
  mutable QMutex m_mutex;/* mutable的作用是允许在const成员函数中修改block_queue类的m_mutex和m_notEmpty成员变量 */
  QWaitCondition m_condition;
};

#endif // BLOCK_QUEUE_H
/******************************** End of file *********************************/
