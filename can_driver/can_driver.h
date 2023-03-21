/**
 *  @file can_driver.h
 *
 *  @date 2023年02月21日 14:08:26 星期二
 *
 *  @author Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @version v1.0.0 aron566 2023.02.21 14:08 初始版本.
 */
#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
//#include <stdbool.h>/**< need definition of bool    */
//#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
/** Private includes ---------------------------------------------------------*/
#include <QObject>
#include <QDebug>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QThread>
#include <QRunnable>
#include "zlgcan.h"
#include "circularqueue.h"
#include "utility.h"
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/

/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/


class can_driver : public QObject, public QRunnable
{
  Q_OBJECT
public:
  explicit can_driver(QObject *parent = nullptr);

  ~can_driver()
  {
    /* 关闭设备 */
    close();

    /* 等待线程结束 */
    while(thread_run_state)
    {
      utility::delay_ms(1);
    }

    /* 删除cq */
    delete cq_obj;
  }

  /**
   * @brief 监听线程启动
   */
  virtual void run() override
  {
    thread_run_state = true;
    while(start_)
    {
      receice_data();
      QThread::msleep(1);
    }
    qDebug() << "[thread]" << QThread::currentThreadId() << "can driver listen end";
    thread_run_state = false;
  }

public:

  typedef enum
  {
    STD_FRAME_TYPE = 0,
    EXT_FRAME_TYPE,
  }FRAME_TYPE_Typedef_t;

  typedef enum
  {
    CAN_PROTOCOL_TYPE = 0,
    CANFD_PROTOCOL_TYPE,
  }PROTOCOL_TYPE_Typedef_t;

  typedef  struct
  {
    void *channel_hadle;
    qint32 channel_num;
    bool channel_en;
  }CHANNEL_STATE_Typedef_t;

  /* 数据缓冲队列 */
  CircularQueue *cq_obj = nullptr;
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
   * @param message 消息内容
   */
  void signal_show_message(const QString &message);
  void signal_show_thread_message(const QString &message);
  void signal_show_message_rx_bytes(quint8 bytes);

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

public:

  /**
   * @brief 打开
   *
   * @return true 成功
   * @return false 失败
   */
  bool open();

  /**
   * @brief 初始化
   *
   * @return true 成功
   * @return false 失败
   */
  bool init();

  /**
   * @brief 启动
   *
   * @return true 成功
   * @return false 失败
   */
  bool start();

  /**
   * @brief 复位
   *
   * @return true 成功
   * @return false 失败
   */
  bool reset();

  /**
   * @brief 关闭
   *
   */
  void close();

  /**
   * @brief 数据接收
   */
  void receice_data();

  /**
   * @brief 设置canid掩码
   * @param mask 掩码
   * @param en 掩码使能
   * @param plast_can_id_mask 上次掩码
   * @param plast_en 上次掩码使能
   */
  void set_msg_canid_mask(quint32 can_id_mask, bool en = false, quint32 *plast_can_id_mask = nullptr, bool *plast_en = nullptr)
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
   * @brief 添加消息过滤器
   * @param can_id can id
   * @param cq_obj_ 消息存储区
   */
  void add_msg_filter(quint32 can_id, CircularQueue *cq_obj_);

  /**
   * @brief 移除消息过滤器
   * @param can_id can id
   */
  void remove_msg_filter(quint32 can_id);

  /**
   * @brief 触发发送数据
   * @param 通道号
   */
  void send(quint8 channel_index = 0);

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
  bool send(const quint8 *data, quint8 size, quint32 id, FRAME_TYPE_Typedef_t frame_type, PROTOCOL_TYPE_Typedef_t protocol, quint8 channel_num = 0xFF);

  /**
   * @brief 设置设备类型
   * @param device_type_str 设备类型
   * @return 设备通道数量
   */
  quint8 set_device_type(const QString &device_type_str);

  /**
   * @brief 设置工作模式
   *
   * @param index 0正常 1只听
   */
  void set_work_mode(quint32 index)
  {
    work_mode_index_ = index;
    function_can_use_update();
  }

  /**
   * @brief 设置网络工作模式
   *
   * @param index 0服务器 1客户端
   */
  void set_net_work_role(quint32 index)
  {
    net_mode_index_ = index;
    function_can_use_update();
  }

  /**
   * @brief 设置服务器端口
   *
   * @param port 端口号
   */
  void set_device_remote_port(const QString &port)
  {
    service_port_str = port;
  }

  /**
   * @brief 设置本地设备端口
   *
   * @param port 端口号
   */
  void set_device_local_port(const QString &port)
  {
    local_port_str = port;
  }

  /**
   * @brief 设置服务器ip
   *
   * @param ip ip地址
   */
  void set_device_remote_ip(const QString &ip)
  {
    service_ip_str = ip;
  }

  /**
   * @brief 设置消息id
   *
   * @param id
   */
  void set_message_id(const QString &id)
  {
    id_ = id;
  }

  /**
   * @brief 设置消息数据
   *
   * @param data 消息负载16进制字符串，空格分割每个字节
   */
  void set_message_data(const QString &data)
  {
    datas_ = data;
  }

  /**
   * @brief 设置设备索引号，多个同样的设备接入时，index不同
   *
   * @param index 索引号
   */
  void set_device_index(quint8 index = 0)
  {
    device_index_ = index;
  }

  /**
   * @brief 设置通讯通道号
   *
   * @param index
   */
  void set_channel_index(quint8 index = 0);

  /**
   * @brief 设置自定义波特率
   *
   * @param bps 波特率
   */
  void set_diy_bauds(const QString &bps)
  {
    custom_baudrate_ = bps;
  }

  /**
   * @brief 设置发送方式
   *
   * @param send_type 0正常发送、1单次发送、2自发自收
   */
  void set_send_type(quint32 send_type = 0)
  {
    send_type_index_ = send_type;
  }

  /**
   * @brief 设置终端电阻是否使能
   *
   * @param en true 使能
   */
  void set_resistance_enbale(bool en)
  {
    resistance_enable_ = en;
  }

  /**
   * @brief 设置波特率
   *
   * @param index 索引
   */
  void set_bps(quint32 index)
  {
    baud_index_ = index;
  }

  /**
   * @brief 设置仲裁域波特率
   *
   * @param index 索引
   */
  void set_abit_bps(quint32 index)
  {
    abit_baud_index_ = index;
  }

  /**
   * @brief 设置数据域波特率
   *
   * @param index 索引
   */
  void set_dbit_bps(quint32 index)
  {
    dbit_baud_index_ = index;
  }

  /**
   * @brief 设置队列发送模式
   *
   * @param en
   * @return true 开启
   * @return false 关闭
   */
  bool set_send_queue_mode(bool en);

  /**
   * @brief 设置自动发送通道索引
   *
   * @param index 索引
   */
  void set_auto_send_index(quint32 index)
  {
    auto_send_index_ = index;
  }

  /**
   * @brief 设置自动发送周期
   *
   * @param ms ms
   */
  void set_auto_send_period_ms(quint32 ms)
  {
    auto_send_period_ = ms;
  }

  /**
   * @brief 设置滤波模式
   *
   * @param index 0双滤波 1单滤波
   */
  void set_filter_mode(quint32 index)
  {
    filter_mode_ = index;
  }

  /**
   * @brief 设置验收码
   *
   * @param code 16进制字符串
   */
  void set_acc_code(const QString &code)
  {
    acc_code_ = code;
  }

  /**
   * @brief 设置屏蔽掩码
   *
   * @param code 16进制字符串
   */
  void set_acc_mask_code(const QString &code)
  {
    acc_mask_ = code;
  }

  /**
   * @brief 设置自定义波特率是否启用
   *
   * @param en true启用
   */
  void set_diy_baud_rate_en(bool en)
  {
    custom_baud_enable_ = en;
  }

  /**
   * @brief 设置队列帧延时启用
   *
   * @param flag true使能
   */
  void set_queue_delay_flag(bool en)
  {
    frm_delay_flag_ = en;
  }

  /**
   * @brief 设置队列帧延时时间
   *
   * @param ms ms
   */
  void set_queue_delay_ms(quint32 ms)
  {
    frm_delay_time_ = ms;
  }

  /**
   * @brief 设置帧类型
   *
   * @param index 帧类型索引号 0标准帧 1扩展帧
   */
  void set_frame_type(quint32 index)
  {
    frame_type_index_ = index;
  }

  /**
   * @brief 设置协议类型
   *
   * @param index 0CAN 1CANFD
   */
  void set_protocol_type(quint32 index)
  {
    protocol_index_ = index;
  }

  /**
   * @brief 设置canfd加速
   *
   * @param index 0不加速 1加速
   */
  void set_can_fd_exp(quint32 index)
  {
    canfd_exp_index_ = index;
  }

  /**
   * @brief 显示发送模式
   *
   */
  void show_send_mode();

  /**
   * @brief 显示发送队列可用空间
   *
   */
  void show_tx_queue_available();

  /**
   * @brief 显示设备自动发送队列
   *
   */
  void show_dev_auto_send();

  /**
   * @brief 清空发送队列
   *
   */
  void clear_tx_queue();

  /**
   * @brief 添加自动发送
   *
   */
  void add_auto_send();

  /**
   * @brief 启动自动发送
   *
   */
  void auto_send_start();

  /**
   * @brief 停止自动发送
   *
   */
  void auto_send_stop();

  /**
   * @brief 停单个自动发送
   *
   */
  void auto_send_stop_single();

  /**
   * @brief 功能可用更新
   */
  void function_can_use_update();

private:
  bool init(CHANNEL_STATE_Typedef_t &channel_state);
  bool start(const CHANNEL_STATE_Typedef_t &channel_state);
  bool reset(const CHANNEL_STATE_Typedef_t &channel_state);
  void close(const CHANNEL_STATE_Typedef_t &channel_state);
  void send(const CHANNEL_STATE_Typedef_t &channel_state);
  bool send(const CHANNEL_STATE_Typedef_t &channel_state, const quint8 *data, quint8 size, quint32 id, FRAME_TYPE_Typedef_t frame_type, PROTOCOL_TYPE_Typedef_t protocol);
  void receice_data(const CHANNEL_STATE_Typedef_t &channel_state);
  void delay_send_can_use_update(bool delay_send, bool send_queue_mode, bool get_send_mode);
  void auto_send_can_use_update(bool support_can, bool support_canfd, bool support_index, bool support_single_cancel, bool support_get_autosend_list);
  bool custom_baud_rate_config(const CHANNEL_STATE_Typedef_t &channel_state);
  void show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_Receive_Data *data, quint32 len);
  void show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_ReceiveFD_Data *data, quint32 len);
  void show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_Transmit_Data *data, quint32 len);
  void show_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_TransmitFD_Data *data, quint32 len);

  /**
   * @brief 显示消息
   * @param data 消息数据
   * @param thread_mode 当前消息是否来自线程
   */
  void show_message(const QString &data, bool thread_mode = false);
  void show_message_rx_bytes(quint8 bytes);
  bool transmit_type_config(const CHANNEL_STATE_Typedef_t &channel_state);
  bool resistance_config(const CHANNEL_STATE_Typedef_t &channel_state);
  bool baud_rate_config(const CHANNEL_STATE_Typedef_t &channel_state);
  bool cand_fd_bps_config(const CHANNEL_STATE_Typedef_t &channel_state);

  void show_tx_queue_available(const CHANNEL_STATE_Typedef_t &channel_state);
  void clear_tx_queue(const CHANNEL_STATE_Typedef_t &channel_state);
  bool set_send_queue_mode(const CHANNEL_STATE_Typedef_t &channel_state, bool en);
  void show_send_mode(const CHANNEL_STATE_Typedef_t &channel_state);
  /**
   * @brief 是否是相关类型设备
   *
   * @param type 设备类型
   * @return true 是
   * @return false 不是
   */
  bool is_net_can_type(quint32 type);
  bool is_net_can_fd_type(quint32 type);
  bool is_net_tcp_type(quint32 type);
  bool is_net_udp_type(quint32 type);

  /**
   * @brief 配置can 自动发送
   *
   * @param nEnable 该通道是否使能
   */
  void add_auto_can(quint32 nEnable);

  /**
   * @brief 配置can fd自动发送
   *
   * @param nEnable 该通道是否使能
   */
  void add_auto_can_fd(quint32 nEnable);

  /**
   * @brief can数据打包
   *
   * @param can_data can数据包
   * @param add_delay_flag 延时标识
   */
  void can_frame_packed(ZCAN_Transmit_Data &can_data, bool add_delay_flag);
  void can_frame_packed(ZCAN_TransmitFD_Data &canfd_data, bool add_delay_flag);
  void can_frame_packed(ZCAN_Transmit_Data &canfd_data, quint32 id, quint32 frame_type, const quint8 *data, quint8 size);
  void can_frame_packed(ZCAN_TransmitFD_Data &canfd_data, quint32 id, quint32 frame_type, const quint8 *data, quint8 size);

private:
  bool device_opened_;
  DEVICE_HANDLE device_handle_ = nullptr;
  qint16 device_type_index_ = 0;
  quint8 device_index_ = 0;
  quint32 channel_index_ = 0;
  quint32 work_mode_index_ = 0;
  quint32 filter_mode_ = 0;
  quint32 net_mode_index_;
  QString acc_code_ = "00000000";
  QString acc_mask_ = "FFFFFFFF";
  QString service_ip_str;
  QString service_port_str;
  QString local_port_str;
  QStringList data_bps_list;
  QStringList arbitration_bps_list;
  QString custom_baudrate_;
  bool custom_baud_enable_ = false;
  bool send_queue_mode = false;
  quint32 send_type_index_ = 0;
  bool resistance_enable_ = false;
  quint32 baud_index_ = 0;
  quint32 abit_baud_index_ = 0;
  quint32 dbit_baud_index_ = 0;
  quint32 auto_send_index_ = 0;
  quint32 auto_send_period_ = 1000;//ms
  QString id_;
  QString datas_;
  QString message;
  quint32 frame_type_index_ = 0;
  quint32 protocol_index_ = 0;
  quint32 canfd_exp_index_ = 0;
  CHANNEL_HANDLE channel_handle_ = nullptr;
  IProperty *property_;
  bool start_ = false;
  quint32 send_count_once_ = 1;
  quint32 frm_delay_time_;//队列帧延时时间ms
  bool frm_delay_flag_ = false;//队列帧延时标记
  bool support_delay_send_ = false;            //设备是否支持队列发送
  bool support_delay_send_mode_ = false;       //设备队列发送是否需要设置队列发送模式,USBCANFD系列，PCIECANFD系列设备需要设置发送模式才可以进行队列发送
  bool support_get_send_mode_ = false;         //设备是否支持查询当前模式

  bool thread_run_state = false;

  quint32 can_id_mask_ = 0xFFFF;
  bool can_id_mask_en_ = false;
  /* 消息过滤器 */
  typedef struct
  {
    quint32 can_id;
    CircularQueue *cq_obj;
  }MSG_FILTER_Typedef_t;

  QList<MSG_FILTER_Typedef_t>msg_filter_list;
  QList<CHANNEL_STATE_Typedef_t>channel_state_list;
};

#endif // CAN_DRIVER_H
/******************************** End of file *********************************/
