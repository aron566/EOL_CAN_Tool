/**
 *  @file updatefw_protocol.hpp
 *
 *  @date 2024年02月22日 09:14:45 星期五
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief 更新固件协议.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-02-22 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 */
#ifndef RTS_PROTOCOL_H
#define RTS_PROTOCOL_H
/** Includes -----------------------------------------------------------------*/
#include <QTimer>
#include <QElapsedTimer>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QSemaphore>
#include <QDebug>
/** Private includes ---------------------------------------------------------*/

#include "utilities/circularqueue.h"
#include "utility.h"
#include "can_driver_model.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/

/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class updatefw_protocol : public QObject, public QRunnable
{
  Q_OBJECT
public:
  explicit updatefw_protocol(QObject *parent = nullptr);

  ~updatefw_protocol()
  {
    stop_task();

    /* 等待线程结束 */
    while(thread_run_state)
    {
      utility::delay_ms(1);
    }

    /* 删除cq */
    delete cq_obj;
    qDebug() << "updatefw protocol delete";
  }

public:

  /* 任务队列 */
  typedef struct RTS_TASK_LIST_
  {
    QString fw_file_name;
    void *param;
    quint32 start_addr;
    bool (updatefw_protocol::*task)(void *param);
  }UPDATEFW_TASK_LIST_Typedef_t;

  /* 响应队列 */
  typedef struct
  {
    uint8_t send_data[8];       /**< 等待响应的寄存器地址 */
    uint64_t start_time;        /**< 等待相应的起始时间 */
    uint32_t id;                /**< 等待的id类型 */
    QString channel_num;        /**< 通讯硬件端口 */
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
    RTS_OPT_ERR,                        /**< 操作未成功 */
  }RTS_OPT_STATUS_Typedef_t;

public:

  /**
   * @brief rts协议线程
   */
  virtual void run() override
  {
    qDebug() << "updatefw protocol wait to run";
    run_state = true;
    thread_run_state = true;
    run_rts_task();
    thread_run_state = false;
    run_state = false;
    qDebug() << "[thread]" << QThread::currentThreadId() << "updatefw protocol end";

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
    qDebug() << "updatefw protocol stop_task";
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
      qDebug() << "updatefw protocol is running";
      return;
    }

    listen_run_state = listen_cs;
    if(thread_run_state)
    {
      qDebug() << "updatefw protocol is running";
      return;
    }
    updatefw_protocol_clear();
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
   * @brief 设置can驱动对象
   * @param can_driver_
   */
  void set_can_driver_obj(can_driver_model *can_driver_)
  {
    if(nullptr == can_driver_)
    {
      return;
    }
    can_driver_obj = can_driver_;
  }

  /**
   * @brief 设置通讯端口
   * @param channel_num_ 通道号 "0" or "1"
   */
  void set_com_config_channel(QString channel_num_ = "0")
  {
    channel_num = channel_num_;
  }

  /**
   * @brief 设置通讯端口
   * @param channel_num_ 通道号 "0" or "1"
   */
  void set_vcom_config_channel(QString channel_num_ = "1")
  {
    vchannel_num = channel_num_;
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

  /**
   * @brief 发送进度信号
   * @param current_size 当前大小
   * @param total_size 总大小
   */
  void signal_send_progress(quint32 current_size, quint32 total_size);
private slots:

private:
  bool run_state = false;
  bool thread_run_state = false;
  bool listen_run_state = false;
  QAtomicInt thread_run_statex;
  QSemaphore sem;

  QThreadPool *g_thread_pool = nullptr;
  CircularQueue *cq_obj = nullptr;

  can_driver_model *can_driver_obj = nullptr;
  QString channel_num = "0";          /**< 通讯硬件端口 */
  QString vchannel_num = "1";         /**< 通讯硬件端口 */

  /* 错误统计 */
  quint32 acc_error_cnt = 0;
  quint32 retry_num_max = 3U;

  /* 任务队列 */
  QList<UPDATEFW_TASK_LIST_Typedef_t>updatefw_task_list;

  /* 响应队列 */
  QList<WAIT_RESPONSE_LIST_Typedef_t>wait_response_list;

public:
  /**
   * @brief 更新app任务
   * @param task 任务
   * @param check_repeat true任务查重
   * @return true正常
   */
  bool updatefw_device_app(UPDATEFW_TASK_LIST_Typedef_t &task, bool check_repeat = true);

private:

  /**
   * @brief set_timeout
   * @param sec 超时时间
   */
  void set_timeout(quint32 sec = 3U);

  /**
   * @brief 更新app任务
   * @param param_
   * @return
   */
  bool update_device_app_task(void *param_);

  /**
   * @brief updatefw_protocol_clear
   */
  void updatefw_protocol_clear();

  /**
   * @brief run_rts_task
   */
  void run_rts_task();

  /**
   * @brief protocol_stack_create_task
   * @param can_id id
   * @param data 数据
   * @param data_len 数据长度
   * @return 任务处理结果
   */
  RETURN_TYPE_Typedef_t protocol_stack_create_task(uint32_t can_id,
                                                   const uint8_t *data,
                                                   uint16_t data_len);

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
   * @param send_data 发送的数据
   * @return 0正常
   */
  RTS_OPT_STATUS_Typedef_t decode_ack_frame(const quint8 *temp_buf, const quint8 *send_data);

  /**
   * @brief decode_data_frame 解析读取状态回复
   * @param temp_buf 状态回复数据
   * @param data_len 数据长度
   * @return 回复状态码
   */
  RETURN_TYPE_Typedef_t decode_data_frame(const quint8 *temp_buf, quint32 data_len);

  /**
   * @brief send_data_port 发送数据
   * @param can_id id
   * @param data 数据
   * @param data_len 数据长度
   * @param channel_num 通讯端口选择
   * @return true发送成功 false发送失败
   */
  bool send_data_port(uint32_t can_id, const uint8_t *data, uint16_t data_len,
                      QString &channel_num);
};

#endif // RTS_PROTOCOL_H
/******************************** End of file *********************************/
