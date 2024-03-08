/**
 *  @file safe_queue.hpp
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
#ifndef SAFE_QUEUE_H
#define SAFE_QUEUE_H
/** Includes -----------------------------------------------------------------*/
#include <QCoreApplication>
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

template<typename T>
class safe_queue
{
public:
  /* 添加元素到队列尾部 */
  void enqueue(const T& value)
  {
    QMutexLocker locker(&m_mutex);
    m_queue.enqueue(value);
  }

  /* 从队列头部移除一个元素，并返回它 */
  T dequeue()
  {
    QMutexLocker locker(&m_mutex);
    if (m_queue.isEmpty())
    {
      return T();
    }
    return m_queue.dequeue();
  }

  /* 返回队列是否为空 */
  bool is_empty() const
  {
    QMutexLocker locker(&m_mutex);
    return m_queue.isEmpty();
  }

private:
  QQueue<T> m_queue;
  mutable QMutex m_mutex;
};

#endif // BLOCK_QUEUE_H
/******************************** End of file *********************************/
