/**
 *  @file can_driver_model.hpp
 *
 *  @date 2023年10月24日 15:07:45 星期二
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-10-24 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 */
#ifndef CAN_DRIVER_MODEL_H
#define CAN_DRIVER_MODEL_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
#include <limits.h> /**< need variable max value    */
//#include <stdalign.h> /**< need alignof    */
//#include <stdarg.h> /**< need va_start    */
//#include <ctype.h> /**< need isalpha isdigit */
//#include <stdatomic.h> /**< need atomic_compare_exchange_weak */
//#include <assert.h> /**< need assert( a > b ); */
//#include <setjmp.h> /**< need jmp_buf buf setjmp(buf); longjmp(buf,1) */
/** Private includes ---------------------------------------------------------*/
#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QCoreApplication>
#include <QThread>
#include <QRunnable>
#include <QSemaphore>
#include <QReadWriteLock>
#include <QDebug>
#include "circularqueue.h"
#include "utility.h"
// #include "can_driver_sender.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros ----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class can_driver_sender;

class can_driver_model : public QObject, public QRunnable
{
  Q_OBJECT

public:
  /* 品牌列表 */
  typedef enum
  {
    ZLG_CAN_BRAND = 0,  /**< 周立功 */
    GC_CAN_BRAND,       /**< 广成 */
    TS_CAN_BRAND,       /**< 同星 */
    KVASER_BRAND,       /**< kvaser */
  }CAN_BRAND_Typedef_t;

  /* 设备信息 */
  typedef struct _DeviceInfo
  {
    const char *device_type_str;  /**< 品牌设备名 */
    CAN_BRAND_Typedef_t brand;    /**< 品牌 */
    quint16 device_type;          /**< 设备类型 */
    quint8 channel_count;         /**< 设备的通道个数 */
  }DEVICE_INFO_Typedef_t;

  /* 标准/扩展帧类型 */
  typedef enum
  {
    STD_FRAME_TYPE = 0,
    EXT_FRAME_TYPE,
  }FRAME_TYPE_Typedef_t;

  /* 数据帧/远程帧 */
  typedef enum
  {
    DATA_FRAME_TYPE = 0,
    REMOTE_FRAME_TYPE,
  }FRAME_DATA_TYPE_Typedef_t;

  /* can cnafd协议 */
  typedef enum
  {
    CAN_PROTOCOL_TYPE = 0,
    CANFD_PROTOCOL_TYPE,
  }PROTOCOL_TYPE_Typedef_t;

  /* 设备运行信息 */
  typedef  struct
  {
    void *channel_handle;
    quint64 device_handle; /**< 同星 - kvaser */
    qint32 channel_num;
    bool channel_en;
  }CHANNEL_STATE_Typedef_t;

  /* tx/rx方向 */
  typedef enum
  {
    CAN_TX_DIRECT = 0,
    CAN_RX_DIRECT,
    UNKNOW_DIRECT = 0xFF,
  }CAN_DIRECT_Typedef_t;

  /* 显示消息 */
  typedef struct
  {
    quint32 can_id;
    CAN_DIRECT_Typedef_t direction;
    quint8 channel_num;
    PROTOCOL_TYPE_Typedef_t can_protocol;
    FRAME_TYPE_Typedef_t frame_type;
    FRAME_DATA_TYPE_Typedef_t frame_data_type;
    quint8 msg_data[64];
    quint8 data_len;
  }CAN_MSG_DISPLAY_Typedef_t;

  /* 异步消息发送数据结构 */
  typedef struct
  {
    QString data;
    quint32 id;
    FRAME_TYPE_Typedef_t frame_type;
    PROTOCOL_TYPE_Typedef_t protocol;
    quint8 channel_num;
  }SEND_MSG_Typedef_t;

  /* 数据缓冲队列 */
  static CircularQueue *cq_obj;
  static bool device_opened_;
  static can_driver_model::CAN_BRAND_Typedef_t brand_;/**< 品牌 */
  static quint8 device_index_;     /**< 设备索引 */
  static qint16 device_type_index_;/**< 设备名索引 */
  static bool start_;
  static quint32 channel_index_;
  static quint32 work_mode_index_;
  static quint32 filter_mode_;
  static quint32 net_mode_index_;
  static QString acc_code_;
  static QString acc_mask_;
  static QString service_ip_str;
  static QString service_port_str;
  static QString local_port_str;
  static QString custom_baudrate_;
  static bool custom_baud_enable_;
  static bool send_queue_mode;
  static quint8 send_type_index_;
  static bool resistance_enable_;
  static quint32 baud_index_;
  static quint32 abit_baud_index_;
  static quint32 dbit_baud_index_;
  static quint32 auto_send_index_;
  static quint32 auto_send_period_;//ms
  static QString id_;
  static QString datas_;
  static quint32 frame_type_index_;
  static quint32 protocol_index_;
  static quint32 canfd_exp_index_;

  static void *device_handle_;
  static void *channel_handle_;
  static quint32 frm_delay_time_;             //队列帧延时时间ms
  static bool frm_delay_flag_;                //队列帧延时标记
  static bool support_delay_send_;            //设备是否支持队列发送
  static bool support_delay_send_mode_;       //设备队列发送是否需要设置队列发送模式,USBCANFD系列，PCIECANFD系列设备需要设置发送模式才可以进行队列发送
  static bool support_get_send_mode_;         //设备是否支持查询当前模式
  static bool thread_run_state;
  static quint32 can_id_mask_;
  static bool can_id_mask_en_;
  static QList<CHANNEL_STATE_Typedef_t>channel_state_list;

  QList<SEND_MSG_Typedef_t>send_msg_list;

  /* 周期发送 */
  typedef struct
  {
    QString data;           /**< 发送数据 */
    quint8 channel_num;     /**< 发送通道 */
    quint32 can_id;         /**< canid */
    qint32 can_fd_exp;      /**< 是否加速 */
    quint32 period_time;    /**< 周期时间 >= 1ms */
    qint32 send_cnt;        /**< 发送总数，为-1永久发送，否则发送指定次数 */
    quint64 last_send_time; /**< 记录上次发送时间 */
    can_driver_model::PROTOCOL_TYPE_Typedef_t protocol_type;
    can_driver_model::FRAME_TYPE_Typedef_t frame_type;
  }PERIOD_SEND_MSG_Typedef_t;

  QList<PERIOD_SEND_MSG_Typedef_t>period_send_msg_list;
  can_driver_sender *sender_obj = nullptr;

  /* 消息过滤器 */
  typedef struct
  {
    quint32 can_id;
    CircularQueue *cq_obj;
    quint8 channel;
  }MSG_FILTER_Typedef_t;

  QList<MSG_FILTER_Typedef_t>msg_filter_list;

  QSemaphore cq_sem;
  QSemaphore tx_sem;
  QSemaphore tx_msg_sem;
  QReadWriteLock tx_msg_rw_lock;

  /* 功能显示控制 */
  typedef struct
  {
    /* 工作模式是否可选 */
    bool work_mode_can_use;

    /* 终端电阻使能是否可选 */
    bool resistance_cs_use;

    /* 波特率选择是否可选 */
    bool bauds_can_use;

    /* 仲裁域，数据域波特率是否可选 */
    bool arbitration_data_bauds_can_use;

    /* 自定义波特率选择是否可选 */
    bool diy_bauds_can_use;

    /* 过滤模式是否可选（验收码，屏蔽码） */
    bool filter_can_use;

    /* 网络相关可选设置 */
    /* 本地端口是否可选 */
    bool local_port_can_use;

    /* 远程端口是否可选 */
    bool remote_port_can_use;

    /* 远程地址是否可选 */
    bool remote_addr_can_use;

    /* 通道数 */
    quint8 channel_num;

    /* 设备名列表 */
    QStringList device_list;

    /* canfd仲裁域波特率 */
    QStringList abitrate_list;

    /* canfd数据域波特率 */
    QStringList datarate_list;

    /* can波特率 */
    QStringList baudrate_list;
  }SET_FUNCTION_CAN_USE_Typedef_t;

public:

  /**
   * @brief set_device_brand 设置当前设备品牌
   * @param brand 品牌
   * @return 该品牌下的设备名列表
   */
  virtual QStringList set_device_brand(CAN_BRAND_Typedef_t brand) = 0;

  /**
   * @brief 设置设备类型
   * @param device_type_str 设备类型
   * @return 设备通道数量
   */
  virtual quint8 set_device_type(const QString &device_type_str) = 0;

  /**
   * @brief open 打开设备
   * @return true 成功
   */
  virtual bool open() = 0;

  /**
   * @brief init 初始化设备
   * @return true 成功
   */
  virtual bool init() = 0;

  /**
   * @brief start 启动设备
   * @return true 成功
   */
  virtual bool start() = 0;

  /**
   * @brief reset 复位设备
   * @return true 成功
   */
  virtual bool reset() = 0;

  /**
   * @brief close 关闭设备
   * @return true 成功
   */
  virtual bool close() = 0;

  /**
   * @brief read_info 读取设备信息
   * @return true 成功
   */
  virtual bool read_info() = 0;

  /**
   * @brief function_can_use_update 功能更新
   */
  virtual void function_can_use_update() = 0;

  /**
   * @brief send 发送数据
   * @param channel_state 通道信息
   * @param data 数据
   * @param size 数据字节长度
   * @param id canid
   * @param frame_type 0标准帧 1扩展帧
   * @param protocol 0can 1canfd
   * @return true发送成功
   */
  virtual bool send(const CHANNEL_STATE_Typedef_t &channel_state, const quint8 *data, quint8 size, quint32 id, FRAME_TYPE_Typedef_t frame_type, PROTOCOL_TYPE_Typedef_t protocol) = 0;

  /**
   * @brief receive_data 接收数据
   * @param channel_state
   */
  virtual void receive_data(const CHANNEL_STATE_Typedef_t &channel_state) = 0;

  explicit can_driver_model(QObject *parent = nullptr);

  virtual ~can_driver_model()
  {
    /* 删除cq */
    delete cq_obj;
    qDebug() << "del base class can_driver_model";
  }

  /**
   * @brief can线程启动
   */
  virtual void run() override
  {
    thread_run_state = true;
    while(start_)
    {
      send_data();
      receive_data();
      /* 刷新界面 */
      emit signal_show_can_msg();
      QThread::usleep(0);
    }
    /* 清空关闭发送 */
    clear_send_data();

    /* 等待完全关闭 */
    while(sender_task_is_running())
    {
      QThread::msleep(1);
    }
    qDebug() << "[thread]" << QThread::currentThreadId() << "can driver task end";
    thread_run_state = false;
  }

public:

  /**
   * @brief reset_driver
   */
  bool reset_driver()
  {
    start_ = false;
    /* 等待完全关闭 */
    while(thread_run_state)
    {
      QCoreApplication::processEvents();
      QThread::msleep(1);
    }
    bool ret = reset();
    /* 发送can复位状态 */
    emit signal_can_driver_reset();
    return ret;
  }

  /**
   * @brief close_driver
   */
  bool close_driver()
  {
    start_ = false;
    /* 等待完全关闭 */
    while(thread_run_state)
    {
      QCoreApplication::processEvents();
      QThread::msleep(1);
    }
    bool ret = close();
    /* 发送can关闭状态 */
    emit signal_can_is_closed();
    return ret;
  }

  void clear_send_data();

  /**
   * @brief sender_task_is_running
   * @return true正运行
   */
  bool sender_task_is_running();

  /**
   * @brief delay_send_can_use_update
   * @param delay_send
   * @param send_queue_mode
   * @param get_send_mode
   */
  void delay_send_can_use_update(bool delay_send, bool send_queue_mode, bool get_send_mode);

  /**
   * @brief auto_send_can_use_update
   * @param support_can
   * @param support_canfd
   * @param support_index
   * @param support_single_cancel
   * @param support_get_autosend_list
   */
  void auto_send_can_use_update(bool support_can, bool support_canfd, bool support_index, bool support_single_cancel, bool support_get_autosend_list);

  /**
   * @brief 设置通讯通道号
   *
   * @param index
   */
  static void set_channel_index(quint8 index = 0);

  /**
   * @brief 设置自动发送通道索引
   *
   * @param index 索引
   */
  static void set_auto_send_index(quint32 index)
  {
    auto_send_index_ = index;
  }

  /**
   * @brief 设置自动发送周期
   *
   * @param ms ms
   */
  static void set_auto_send_period_ms(quint32 ms)
  {
    auto_send_period_ = ms;
  }

  /**
   * @brief 设置滤波模式
   *
   * @param index 0双滤波 1单滤波
   */
  static void set_filter_mode(quint32 index)
  {
    filter_mode_ = index;
  }

  /**
   * @brief 设置验收码
   *
   * @param code 16进制字符串
   */
  static void set_acc_code(const QString &code)
  {
    acc_code_ = code;
  }

  /**
   * @brief 设置屏蔽掩码
   *
   * @param code 16进制字符串
   */
  static void set_acc_mask_code(const QString &code)
  {
    acc_mask_ = code;
  }

  /**
   * @brief 设置自定义波特率是否启用
   *
   * @param en true启用
   */
  static void set_diy_baud_rate_en(bool en)
  {
    custom_baud_enable_ = en;
  }

  /**
   * @brief 设置队列帧延时启用
   *
   * @param flag true使能
   */
  static void set_queue_delay_flag(bool en)
  {
    frm_delay_flag_ = en;
  }

  /**
   * @brief 设置队列帧延时时间
   *
   * @param ms ms
   */
  static void set_queue_delay_ms(quint32 ms)
  {
    frm_delay_time_ = ms;
  }

  /**
   * @brief 设置帧类型
   *
   * @param index 帧类型索引号 0标准帧 1扩展帧
   */
  static void set_frame_type(quint32 index)
  {
    frame_type_index_ = index;
  }

  /**
   * @brief 设置协议类型
   *
   * @param index 0CAN 1CANFD
   */
  static void set_protocol_type(quint32 index)
  {
    protocol_index_ = index;
  }

  /**
   * @brief 设置canfd加速
   *
   * @param index 0不加速 1加速
   */
  static void set_can_fd_exp(quint32 index)
  {
    canfd_exp_index_ = index;
  }

  /**
   * @brief 设置自定义波特率
   *
   * @param bps 波特率
   */
  static void set_diy_bauds(const QString &bps)
  {
    custom_baudrate_ = bps;
  }

  /**
   * @brief 设置发送方式
   *
   * @param send_type 0正常发送、1单次发送、2自发自收
   */
  static void set_send_type(quint8 send_type = 0)
  {
    send_type_index_ = send_type;
  }

  /**
   * @brief 设置终端电阻是否使能
   *
   * @param en true 使能
   */
  static void set_resistance_enbale(bool en)
  {
    resistance_enable_ = en;
  }

  /**
   * @brief 设置波特率
   *
   * @param index 索引
   */
  static void set_bps(quint32 index)
  {
    baud_index_ = index;
  }

  /**
   * @brief 设置仲裁域波特率
   *
   * @param index 索引
   */
  static void set_abit_bps(quint32 index)
  {
    abit_baud_index_ = index;
  }

  /**
   * @brief 设置数据域波特率
   *
   * @param index 索引
   */
  static void set_dbit_bps(quint32 index)
  {
    dbit_baud_index_ = index;
  }

  /**
   * @brief 设置工作模式
   *
   * @param index 0正常 1只听
   */
  static void set_work_mode(quint32 index)
  {
    work_mode_index_ = index;
  }

  /**
   * @brief 设置网络工作模式
   *
   * @param index 0服务器 1客户端
   */
  static void set_net_work_role(quint32 index)
  {
    net_mode_index_ = index;
  }

  /**
   * @brief 设置服务器端口
   *
   * @param port 端口号
   */
  static void set_device_remote_port(const QString &port)
  {
    service_port_str = port;
  }

  /**
   * @brief 设置本地设备端口
   *
   * @param port 端口号
   */
  static void set_device_local_port(const QString &port)
  {
    local_port_str = port;
  }

  /**
   * @brief 设置服务器ip
   *
   * @param ip ip地址
   */
  static void set_device_remote_ip(const QString &ip)
  {
    service_ip_str = ip;
  }

  /**
   * @brief 设置消息id
   *
   * @param id
   */
  static void set_message_id(const QString &id)
  {
    id_ = id;
  }

  /**
   * @brief 设置消息数据
   *
   * @param data 消息负载16进制字符串，空格分割每个字节
   */
  static void set_message_data(const QString &data)
  {
    datas_ = data;
  }

  /**
   * @brief 设置设备索引号，多个同样的设备接入时，index不同
   *
   * @param index 索引号
   */
  static void set_device_index(quint8 index = 0)
  {
    device_index_ = index;
  }

  /**
   * @brief 设置canid掩码
   * @param mask 掩码
   * @param en 掩码使能
   * @param plast_can_id_mask 上次掩码
   * @param plast_en 上次掩码使能
   */
  static void set_msg_canid_mask(quint32 can_id_mask, bool en = false, quint32 *plast_can_id_mask = nullptr, bool *plast_en = nullptr)
  {
    if(nullptr != plast_can_id_mask)
    {
      *plast_can_id_mask = can_id_mask_;
    }
    if(nullptr != plast_en)
    {
      *plast_en = can_id_mask_en_;
    }
    can_id_mask_en_ = en;
    can_id_mask_ = can_id_mask;
  }

  /**
   * @brief bps_number2str 将 1000bps转为"1Kbps"字符串 1000000bps转为"1Mbps"字符串
   * @param bps
   * @return 字符串
   */
  static QString bps_number2str(quint32 bps)
  {
    if(1000000U <= bps)
    {
      return QString::asprintf("%uMbps", bps / 1000000U);
    }
    else
    {
      return QString::asprintf("%uKbps", bps / 1000U);
    }
  }

  /**
   * @brief 添加消息过滤器
   * @param can_id can id
   * @param cq_obj_ 消息存储区
   * @param channel_ CAN通道 0xFFU代表不过滤特定通道
   */
  void add_msg_filter(quint32 can_id, CircularQueue *cq_obj_, quint8 channel_ = 0xFFU);

  /**
   * @brief 移除消息过滤器
   * @param can_id can id
   */
  void remove_msg_filter(quint32 can_id);

  /**
   * @brief msg_to_cq_buf can消息过滤分发到cq
   * @param can_id can id
   * @param channel_num 通道号
   * @param data 数据
   * @param data_len 数据长度
   */
  void msg_to_cq_buf(quint32 can_id, quint8 channel_num, const quint8 *data, quint32 data_len);

  /**
   * @brief msg_to_ui_cq_buf can消息过滤分发到显示cq
   * @param can_id can id
   * @param channel_num 通道号
   * @param direction 方向
   * @param protocol 协议
   * @param frame_type 帧类型
   * @param frame_data_type 帧数据类型
   * @param data 数据
   * @param data_len 数据长度
   */
  void msg_to_ui_cq_buf(quint32 can_id, quint8 channel_num, CAN_DIRECT_Typedef_t direction, \
                        PROTOCOL_TYPE_Typedef_t protocol, FRAME_TYPE_Typedef_t frame_type, \
                        FRAME_DATA_TYPE_Typedef_t frame_data_type, \
                        const quint8 *data, quint8 data_len);
  /**
   * @brief 显示消息
   * @param str 消息数据
   * @param channel_num 消息通道号
   * @param direct 消息方向，发送 or 接收
   * @param data 数据
   * @param data_len 数据长度
   * @param can_id 消息canid
   * @param thread_mode 当前消息是否来自线程
   */
  void show_message(const QString &str, quint32 channel_num = 0, CAN_DIRECT_Typedef_t direct = CAN_RX_DIRECT, const quint8 *data = nullptr, quint32 data_len = 0, quint32 can_id = 0, bool thread_mode = false);
  void show_message_bytes(quint8 bytes, quint32 channel_num, CAN_DIRECT_Typedef_t direct);

  /**
   * @brief 触发发送数据
   * @param 通道号
   */
  void send(quint8 channel_index = 0);

  /**
   * @brief period_send_set 周期发送设置
   * @param id
   * @param data
   * @param period_time 周期时间
   * @param send_cnt 发送次数 -1不限制
   * @param channel_num
   * @param frame_type
   * @param protocol_type
   * @param can_fd_exp
   */
  void period_send_set(quint32 id, QString data, quint32 period_time,
                                         qint32 send_cnt, quint8 channel_num,
                                         FRAME_TYPE_Typedef_t frame_type,
                                         PROTOCOL_TYPE_Typedef_t protocol_type,
                                         quint32 can_fd_exp);

  /**
   * @brief 获取周期发送列表长度
   * @return 长度
   */
  qint32 get_period_send_list_size();

  void period_send_list_clear();

  /**
   * @brief send 发送数据
   * @param data 数据
   * @param size 数据字节长度
   * @param id canid
   * @param frame_type 0标准帧 1扩展帧
   * @param protocol 0can 1canfd
   * @param channel_num 通道号，不写时默认用第一个可用的通道
   * @return true发送成功
   */
  bool send(const quint8 *data, quint8 size, quint32 id, FRAME_TYPE_Typedef_t frame_type, PROTOCOL_TYPE_Typedef_t protocol, quint8 channel_num = 0xFFU);

private:
  /**
   * @brief send_data 发送数据（查询待发列表）
   * @return true 成功
   */
  bool send_data();

  /**
   * @brief send
   * @param channel_state
   */
  void send(const CHANNEL_STATE_Typedef_t &channel_state);

  /**
   * @brief receive_data 接收数据
   * @return true 成功
   */
  bool receive_data();
signals:
  /**
   * @brief 发送信号can开启还是关闭状态
   */
  void signal_can_is_opened();

  /**
   * @brief 发送信号can是否是关闭状态
   */
  void signal_can_is_closed();

  /**
   * @brief 发送信号can重置状态
   */
  void signal_can_driver_reset();

  /**
   * @brief 发送信号工作模式是否可选
   *
   * @param can_use true可选
   */
  void signal_work_mode_can_use(bool can_use);

  /**
   * @brief 发送信号终端电阻使能是否可选
   *
   * @param can_use true可选
   */
  void signal_resistance_cs_use(bool can_use);

  /**
   * @brief 发送信号波特率选择是否可选
   *
   * @param can_use true可选
   */
  void signal_bauds_can_use(bool can_use);

  /**
   * @brief 发送信号仲裁域，数据域波特率是否可选
   *
   * @param can_use true可选
   */
  void signal_arbitration_data_bauds_can_use(bool can_use);

  /**
   * @brief 发送信号自定义波特率选择是否可选
   *
   * @param can_use true可选
   */
  void signal_diy_bauds_can_use(bool can_use);

  /**
   * @brief 发送信号过滤模式是否可选（验收码，屏蔽码）
   *
   * @param can_use true可选
   */
  void signal_filter_can_use(bool can_use);

  /**
   * @brief 发送信号本地端口是否可选
   *
   * @param can_use true可选
   */
  void signal_local_port_can_use(bool can_use);

  /**
   * @brief 发送信号远程端口是否可选
   *
   * @param can_use true可选
   */
  void signal_remote_port_can_use(bool can_use);

  /**
   * @brief 发送信号远程地址是否可选
   *
   * @param can_use true可选
   */
  void signal_remote_addr_can_use(bool can_use);

  /**
   * @brief 发送信号队列帧发送延时是否可选
   *
   * @param can_use true可选
   */
  void signal_send_queue_delay_can_use(bool can_use);

  /**
   * @brief 发送信号获取队列可用空间按钮可用状态
   *
   * @param can_use true可选
   */
  void signal_get_tx_available_can_use(bool can_use);

  /**
   * @brief 发送信号清除队列按钮可用状态
   *
   * @param can_use true可选
   */
  void signal_clear_tx_queue_can_use(bool can_use);

  /**
   * @brief 发送信号队列帧延时标记是否可选
   *
   * @param can_use true可选
   */
  void signal_queue_delay_flag_can_use(bool can_use);

  /**
   * @brief 发送信号获取发送模式按钮可用状态
   *
   * @param can_use true可选
   */
  void signal_get_sen_mode_can_use(bool can_use);

  /**
   * @brief 发送信号队列发送模式是否启用
   *
   * @param en true启用
   */
  void signal_send_queue_mode_can_use(bool can_use);

  /**
   * @brief 发送信号显示当前消息
   *
   * @param message 消息内容t
   */
  Q_DECL_DEPRECATED void signal_show_message(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data = nullptr, quint32 data_len = 0, quint32 can_id = 0);
  Q_DECL_DEPRECATED void signal_show_thread_message(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data = nullptr, quint32 data_len = 0, quint32 can_id = 0);
  Q_DECL_DEPRECATED void signal_show_message_bytes(quint8 bytes, quint32 channel_num, quint8 direct);

  /**
   * @brief 发送信号-刷新消息显示
   */
  void signal_show_can_msg();

  /**
   * @brief 发送信号-刷新消息显示 asynchronous
   */
  void signal_show_can_msg_asynchronous();

  /**
   * @brief 发送信号显示当前的can消息
   * @param can_id id
   * @param data 数据
   * @param len 数据长度
   * @param direct 方向
   * @param channel_num 通道号
   * @param protocol_type 协议类型
   * @param ms 当前时间
   */
  void signal_can_driver_msg(quint16 can_id, const quint8 *data, quint32 len, quint8 direct, quint32 channel_num, quint8 protocol_type, quint64 ms);

  /**
   * @brief 发送信号设置自动发送设备索引可用状态
   *
   * @param can_use true可选
   */
  void signal_auto_send_dev_index_can_use(bool can_use);

  /**
   * @brief 发送信号设置自动发送周期可用状态
   *
   * @param can_use true可选
   */
  void signal_auto_send_period_can_use(bool can_use);

  /**
   * @brief 发送信号添加自动发送按钮可用状态
   *
   * @param can_use true可选
   */
  void signal_auto_send_add_can_use(bool can_use);

  /**
   * @brief 发送信号自动发送启动可用状态
   *
   * @param can_use true可选
   */
  void signal_auto_send_start_can_use(bool can_use);

  /**
   * @brief 发送信号自动发送停止可用状态
   *
   * @param can_use true可选
   */
  void signal_auto_send_stop_can_use(bool can_use);

  /**
   * @brief 发送信号取消单条自动发送按钮可用状态
   *
   * @param can_use true可选
   */
  void signal_auto_send_cancel_once_can_use(bool can_use);

  /**
   * @brief 发送信号获取设备自动发送列表可用状态
   *
   * @param can_use true可选
   */
  void signal_get_dev_auto_send_list_can_use(bool can_use);

};

#endif // CAN_DRIVER_MODEL_H
/******************************** End of file *********************************/

