/**
 *  @file rts_protocol.hpp
 *
 *  @date 2023年12月01日 15:14:45 星期五
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-12-01 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 */
#ifndef RTS_PROTOCOL_H
#define RTS_PROTOCOL_H
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
#include <QTimer>
#include <QElapsedTimer>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QSemaphore>
#include <QDebug>
#include "utilities/circularqueue.h"
#include "utility.h"
#include "network_driver/network_driver_model.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
#define RTS_CONNECT       "System:Interaction:Remote"         /**< 开启远程连接 */
#define RTS_OPEN_DEVICE   "VRTS:Open"                         /**< 打开模拟器 */
#define RTS_GET_STATUS    "VRTS:Status?"                      /**< 回复字符串长度≥5 内容为: A(空格)B(空格)C B为VRTS连接状态，1为连接其他字符为未连接 当B不为1且C不为1时，则为VRTS报错 */
#define RTS_SET_PARAMETER "VRTS:Parameters 77.000/24.000 76.500 1.000 40.000(EIRPTX) 2.200(模拟器到雷达之间距离)"
#define RTS_SET_PARAMETER_PORT(freqband,freq,bandwidth,distance) (QString("VRTS:Parameters %1 %2 %3 40.000 %4").arg(freqband, freq, bandwidth, distance))
#define RTS_SET_FREQUENCY "VRTS:Radar 76.300"                 /**< 设置中心频率 */
#define RTS_SET_FREQUENCY_PORT(freq) (QString("VRTS:Radar %1").arg(freq))
#define RTS_SET_TARGET    "VRTS:Targets 20.000 0.000 10.000"  /**< r m v m/s rcs dBsm */
#define RTS_SET_TARGET_PORT(range,speed,dBsm) (QString("VRTS:Targets %1 %2 %3").arg(range, speed, dBsm))
#define RTS_CLOSE_DEVICE  "VRTS:Close"                        /**< 关闭模拟器 */
#define RTS_DISCONNECT    "System:Interaction:Local"          /**< 关闭远程连接 */
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class rts_protocol : public QObject, public QRunnable
{
  Q_OBJECT
public:
  explicit rts_protocol(QObject *parent = nullptr);

  ~rts_protocol()
  {
    stop_task();

    /* 等待线程结束 */
    while(thread_run_state)
    {
      utility::delay_ms(1);
    }

    /* 删除cq */
    delete cq_obj;
    qDebug() << "rts protocol delete";
  }

public:

  /* 任务队列 */
  typedef struct RTS_TASK_LIST_
  {
    QString cmd;
    void *param;
    bool (rts_protocol::*task)(void *param);
  }RTS_TASK_LIST_Typedef_t;

  /* 响应队列 */
  typedef struct
  {
    uint64_t start_time;    /**< 等待相应的起始时间 */
    QString peer_addr;      /**< 通讯硬件端口:192.168.3.181:12001 */
    QString cmd;            /**< 请求的命令 */
  }WAIT_RESPONSE_LIST_Typedef_t;

  /* 等待回复结果 */
  typedef enum
  {
    RETURN_OK = 0,
    RETURN_TIMEOUT,
    RETURN_WAITTING,              /**< 等待中 */
    RETURN_ERROR,                 /**< 错误 */
  }RETURN_TYPE_Typedef_t;

  /* 操作返回值 */
  typedef enum
  {
    RTS_OPT_OK = 0,                     /**< 无错误 */
    RTS_OPT_ERR,                        /**< RTS操作未成功 */
  }RTS_OPT_STATUS_Typedef_t;

public:

  /**
   * @brief rts协议线程
   */
  virtual void run() override
  {
    qDebug() << "rts protocol wait to run";
    run_state = true;
    thread_run_state = true;
    run_rts_task();
    thread_run_state = false;
    run_state = false;
    qDebug() << "[thread]" << QThread::currentThreadId() << "rts protocol end";

    /* 原子操作 */
    if(thread_run_statex.testAndSetRelaxed(1, 0))
    {
      qDebug() << "thread_run_statex was successfully updated.";
    }
    else
    {
      qDebug() << "thread_run_statex was not updated.";
    }
  }

  void stop_task()
  {
    run_state = false;
    listen_run_state = false;
    qDebug() << "rts protocol stop_task";
  }

  /**
   * @brief start_task
   * @param listen_cs 设置eol监听使能，true使能 false失能
   */
  void start_task(bool listen_cs = false)
  {
    /* 原子操作 */
    if(thread_run_statex.testAndSetRelaxed(0, 1))
    {

    }
    else
    {
      qDebug() << "rts protocol is running";
      return;
    }

    listen_run_state = listen_cs;
    if(thread_run_state)
    {
      qDebug() << "rts protocol is running";
      return;
    }
    rts_protocol_clear();
    g_thread_pool->start(this);
  }

  /**
   * @brief 任务是否在运行
   * @return true正在运行
   */
  bool task_is_runing()
  {
    return thread_run_state;
  }

  /**
   * @brief set_network_device 设置网络驱动设备
   * @param send_obj
   * @param rec_obj
   * @param peer_addr_ 地址
   */
  void set_network_device(network_driver_model *send_obj, network_driver_model *rec_obj, QString peer_addr_)
  {
    network_device_obj = send_obj;
    network_rec_device_obj = rec_obj;
    peer_addr = peer_addr_;
  }

signals:

  /**
   * @brief 协议栈无回复超时
   * @param sec 秒
   */
  void signal_protocol_timeout(quint32 sec);

  /**
   * @brief 从机返回错误消息
   * @param error_msg 错误消息
   */
  void signal_protocol_error_occur(quint8 error_msg);

  /**
   * @brief signal_protocol_rw_err 读写错误信号
   * @param cmd 命令
   */
  void signal_protocol_rw_err(QString cmd);

  /**
   * @brief signal_protocol_rw_ok 读写ok信号
   * @param cmd 命令
   */
  void signal_protocol_rw_ok(QString cmd);

private slots:

private:
  QElapsedTimer timer_obj;
  bool run_state = false;
  bool thread_run_state = false;
  bool listen_run_state = false;
  QAtomicInt thread_run_statex;
  QSemaphore sem;

  QThreadPool *g_thread_pool = nullptr;
  CircularQueue *cq_obj = nullptr;
  network_driver_model *network_device_obj = nullptr;
  network_driver_model *network_rec_device_obj = nullptr;
  QString peer_addr;

  /* 错误统计 */
  quint32 acc_error_cnt = 0;

  /* 任务队列 */
  QList<RTS_TASK_LIST_Typedef_t>rts_task_list;

  /* 响应队列 */
  QList<WAIT_RESPONSE_LIST_Typedef_t>wait_response_list;

public:
  /**
   * @brief 读写设备
   * @param task 任务
   * @param check_repeat true任务查重
   * @return true正常
   */
  bool rts_master_common_rw_device(RTS_TASK_LIST_Typedef_t &task, bool check_repeat = true);

private:

  /**
   * @brief common_rw_device_task 读写任务
   * @param param_
   * @return
   */
  bool common_rw_device_task(void *param_);

  /**
   * @brief rts_protocol_clear
   */
  void rts_protocol_clear();

  /**
   * @brief run_rts_task
   */
  void run_rts_task();

  /**
   * @brief protocol_stack_create_task
   * @param cmd
   * @return
   */
  RETURN_TYPE_Typedef_t protocol_stack_create_task(const QString &cmd);

  /**
   * @brief response_is_timeout 检查等待句柄是否已超时
   * @param wait 等待句柄
   * @return true 超时
   */
  bool response_is_timeout(WAIT_RESPONSE_LIST_Typedef_t &wait);

  /**
   * @brief protocol_stack_wait_reply_start 协议栈等待回复
   * @param listen_mode 是否是监听模式
   * @return 回复
   */
  RETURN_TYPE_Typedef_t protocol_stack_wait_reply_start(bool listen_mode = false);

  /**
   * @brief decode_ack_frame 解析设置命令回复
   * @param temp_buf 回复数据
   * @return 0正常
   */
  RTS_OPT_STATUS_Typedef_t decode_ack_frame(const quint8 *temp_buf);

  /**
   * @brief decode_data_frame 解析读取状态回复
   * @param temp_buf 状态回复数据
   * @param data_len 数据长度
   * @return 回复状态码
   */
  RETURN_TYPE_Typedef_t decode_data_frame(const quint8 *temp_buf, quint32 data_len);

  /**
   * @brief rts_send_data_port 发送数据
   * @param data 数据
   * @param data_len 数据长度
   * @param peer_addr 对方地址
   * @return true发送成功
   */
  bool rts_send_data_port(const uint8_t *data, uint16_t data_len,
                                        QString &peer_addr);
};

#endif // RTS_PROTOCOL_H
/******************************** End of file *********************************/
