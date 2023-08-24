/**
 *  @file eol_protocol.hpp
 *
 *  @date 2023年02月20日 17:08:26 星期一
 *
 *  @author Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @version v1.0.0 aron566 2023.02.20 17:08 初始版本.
 */
#ifndef _EOL_PROTOCOL_H
#define _EOL_PROTOCOL_H
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
/** Private includes ---------------------------------------------------------*/
#include <QObject>
#include <QCoreApplication>
#include <QTimer>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QSemaphore>
#include <QAtomicInt>
#include "circularqueue.h"
#include "can_driver.h"
#include "utility.h"
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/

/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/* 寄存器表 */

/* 握手 */
#define EOL_R_ACCESS_CODE_REG          0x00U  /**< 读访问码 */
#define EOL_W_RUN_MODE_REG             0x01U  /**< 写运行模式 */

/* 常规测试 */
#define EOL_R_PROFILE_NUM_CH_NUM       0x02U  /**< 读EOL下配置数和通道数 */
#define EOL_RW_VERSION_REG             0x03U  /**< 读取软硬件版本 */
#define EOL_W_VCAN_TEST_REG            0x04U  /**< VCAN测试 */
#define EOL_RW_SN_REG                  0x05U  /**< SN读写 */
#define EOL_R_MOUNTID_REG              0x06U  /**< 读取moundID */

/* 读写表 */
#define EOL_RW_TABLE_DATA_REG          0x07U  /**< 读写表数据，帧计数为0时传输表信息，其他为数据帧 */
#define EOL_W_TABLE_SEL_REG            0x08U  /**< 设置需要读取的表 */
#define EOL_W_TABLE_DATA_SEL_REG       0x09U  /**< 设置需要特定帧的表数据（用于丢帧重传） */

#define EOL_W_DEVICE_REBOOT_REG        0x0AU  /**< 设置设备重启 */

#define EOL_W_SAVE_PAR_REG             0x0BU  /**< 保存参数到flash */

/* rcs校准，目标测试 */
#define EOL_R_OBJ_LIST_REG             0x0CU  /**< 读取目标信息 */
#define EOL_RW_PROFILE_ID_REG          0x0DU  /**< 读写配置ID */

/* 角度校准 */
#define EOL_W_2DFFT_CONDITION_REG      0x0EU  /**< 设置获取2DFFT数据条件 */
#define EOL_R_2DFFT_DATA_REG           0x0FU  /**< 获取2DFFT数据 */

#define EOL_RW_RCS_OFFSET_REG          0x10U  /**< 读写RCS值 */
#define EOL_W_PAR_RESET_REG            0x11U  /**< 重置所有参数 */
#define EOL_RW_CALI_MODE_REG           0x12U  /**< 读写校准模式 */

#define EOL_W_TEST_PAR_REG             0x13U  /**< 退出或者进入设置测试模式参数 */

#define EOL_R_BG_NOISE_REG             0x14U  /**< 读取通道底噪 */

#define EOL_W_UPDATE_BG_NOISE_REG      0x15U  /**< 更新底噪 */
#define EOL_RW_VOLTAGE_REG             0x16U  /**< 读写电压 */
#define EOL_R_CHIP_SN_REG              0x17U  /**< 读取芯片SN */
#define EOL_R_PMIC_SN_REG              0x18U  /**< 读取pmicSN */
#define EOL_R_CLIENT_DID_VER_REG       0x19U  /**< 读取客户did版本信息 */
#define EOL_RW_RDM_DATA_REG            0x1AU  /**< 读写RDM数据 */
#define EOL_W_WDG_REG                  0x1BU  /**< 设置看门狗 */
#define EOL_RW_GPIO_REG                0x1CU  /**< 读写GPIO */
#define EOL_RW_SHELL_REG               0x1DU  /**< 读写shell，当读命令时持续监听eol协议数据 */
#define EOL_RW_SPI_REG                 0x1EU  /**< 读写spi测试 */
#define EOL_RW_I2C_REG                 0x1FU  /**< 读写i2c测试 */
#define EOL_RW_DTC_REG                 0x20U  /**< 读写dtc测试 */

/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class eol_protocol : public QObject, public QRunnable
{
  Q_OBJECT
public:
  explicit eol_protocol(QObject *parent = nullptr);

  ~eol_protocol()
  {
    stop_task();

    /* 等待线程结束 */
    while(thread_run_state)
    {
      utility::delay_ms(1);
    }

    /* 删除cq */
    delete cq_obj;
    qDebug() << "eol protocol delete";
  }

  /* 设备工作模式 */
  typedef enum
  {
    NORMAL_MODE_RUN = 0,
    PRODUCE_MODE_NORMAL = 1,            /**< 雷达EOL校准模式 */
    PRODUCE_MODE_DEBUG = 2              /**< 雷达调试模式 */
  }DEVICE_MODE_Typedef_t;

  /* 校准配置信息 */
  typedef struct
  {
    quint32 time_sec;                   /**< 时间戳 */
    quint8 profile_id;                  /**< 配置ID */
    quint8 channel_num;                 /**< 通道数 */
    quint8 tx_order[4];                 /**< 校准配置信息 */
  }CALIBRATION_PROFILE_INFO_Typedef_t;

  /* 操作返回值 */
  typedef enum
  {
    EOL_OPT_OK = 0,                     /**< 无错误 */
    EOL_OPT_CRC_ERR,                    /**< CRC校验错误 */
    EOL_OPT_R_HEADER_ERR,               /**< 读表头部错误 */
    EOL_OPT_R_DATA_ERR,                 /**< 读数据错误 */
    EOL_OPT_W_HEADER_ERR,               /**< 写表头部错误 */
    EOL_OPT_W_DATA_ERR,                 /**< 写数据错误 */
    EOL_OPT_HEADER_CRC_ERR,             /**< 头部CRC校验错误 */
    EOL_OPT_RW_ERR,                     /**< 读写错误 */
    EOL_OPT_ERASE_ERR,                  /**< 擦除错误 */
    UNKNOW_TAB_ERR,                     /**< 表类型错误 */
    EOL_OVER_SIZE,                      /**< 数据过大溢出 */
    UNKNOW_CMD_ERR,                     /**< 未知命令类型错误 */
  }EOL_OPT_STATUS_Typedef_t;

  /* 数据格式 */
  typedef enum
  {
    CALTERAH_CFX_28BIT_DATA_TYPE = 0,     /**< 加特兰CFX 28位数据格式 */
    COMPLEX_FLOAT_DATA_TYPE,              /**< 浮点类型 */
    COMPLEX_INT16_DATA_TYPE,              /**< 16位整型复数 @ref Complex_I16_t */
    FLOAT32_DATA_TYPE,                    /**< 32位浮点类型 */
    INT32_DATA_TYPE,                      /**< 整型32位 */
    INT16_DAYA_TYPE,                      /**< 整型16位 */
    INT8_DATA_TYPE,                       /**< 整型8位 */
    UINT32_DATA_TYPE,                     /**< 无符号整型32位 */
    UINT16_DAYA_TYPE,                     /**< 无符号整型16位 */
    UINT8_DATA_TYPE,                      /**< 无符号整型8位 */
    FLOAT32_BIN_DATA_TYPE,                /**< 32位浮点数二进制类型 */
    UNKNOW_DATA_TYPE = 0xFF,              /**< 未知数据类型 */
  }DATA_Typedef_t;

  /* 单位 */
  typedef enum
  {
    METER_UNIT = 0,                       /**< 单位：m */
    DB_UNIT,                              /**< 单位：dB */
    UNIT_MAX = 0xFF                       /**< 单位：未知 */
  }UNIT_Typedef_t;

  /* 表分类 */
  typedef enum
  {
    SV_AZIMUTH_TABLE = 0,                 /**< 方位导向矢量表@ELE+0deg */
    SV_ELEVATION_TABLE,                   /**< 俯仰导向矢量表@AZI+0deg */
    ANT_BOTH_TABLE,                       /**< 天线间距坐标与初相信息表，双表合并 */
    PATTERN_TABLE,                        /**< 方向图表 */
    SV_ELEVATION_AZI_N45_TABLE,           /**< 俯仰导向矢量表@AZI-45deg */
    SV_ELEVATION_AZI_P45_TABLE,           /**< 俯仰导向矢量表@AZI+45deg */
    BACKGROUND_NOISE_TABLE,               /**< 通道底噪表 */
    UNKNOW_CLASS_TABLE = 0xFF,            /**< 未知表类 */
  }TABLE_CLASS_Typedef_t;


  /* 表类型 */
  typedef enum
  {
    PROFILE_0_SV_AZIMUTH_TABLE = 0,       /**< 方位导向矢量表@ELE+0deg */
    PROFILE_1_SV_AZIMUTH_TABLE,           /**< 方位导向矢量表@ELE+0deg */
    PROFILE_2_SV_AZIMUTH_TABLE,           /**< 方位导向矢量表@ELE+0deg */
    PROFILE_3_SV_AZIMUTH_TABLE,           /**< 方位导向矢量表@ELE+0deg */

    PROFILE_0_SV_ELEVATION_TABLE,         /**< 俯仰导向矢量表@AZI+0deg */
    PROFILE_1_SV_ELEVATION_TABLE,         /**< 俯仰导向矢量表@AZI+0deg */
    PROFILE_2_SV_ELEVATION_TABLE,         /**< 俯仰导向矢量表@AZI+0deg */
    PROFILE_3_SV_ELEVATION_TABLE,         /**< 俯仰导向矢量表@AZI+0deg */

    PROFILE_0_ANT_BOTH_TABLE,             /**< 天线间距坐标与初相信息表，双表合并 */
    PROFILE_1_ANT_BOTH_TABLE,             /**< 天线间距坐标与初相信息表，双表合并 */
    PROFILE_2_ANT_BOTH_TABLE,             /**< 天线间距坐标与初相信息表，双表合并 */
    PROFILE_3_ANT_BOTH_TABLE,             /**< 天线间距坐标与初相信息表，双表合并 */

    PROFILE_0_PATTERN_TABLE,              /**< 方向图表 */
    PROFILE_1_PATTERN_TABLE,              /**< 方向图表 */
    PROFILE_2_PATTERN_TABLE,              /**< 方向图表 */
    PROFILE_3_PATTERN_TABLE,              /**< 方向图表 */

    PROFILE_0_SV_ELEVATION_AZI_N45_TABLE, /**< 俯仰导向矢量表@AZI-45deg */
    PROFILE_1_SV_ELEVATION_AZI_N45_TABLE, /**< 俯仰导向矢量表@AZI-45deg */
    PROFILE_2_SV_ELEVATION_AZI_N45_TABLE, /**< 俯仰导向矢量表@AZI-45deg */
    PROFILE_3_SV_ELEVATION_AZI_N45_TABLE, /**< 俯仰导向矢量表@AZI-45deg */

    PROFILE_0_SV_ELEVATION_AZI_P45_TABLE, /**< 俯仰导向矢量表@AZI+45deg */
    PROFILE_1_SV_ELEVATION_AZI_P45_TABLE, /**< 俯仰导向矢量表@AZI+45deg */
    PROFILE_2_SV_ELEVATION_AZI_P45_TABLE, /**< 俯仰导向矢量表@AZI+45deg */
    PROFILE_3_SV_ELEVATION_AZI_P45_TABLE, /**< 俯仰导向矢量表@AZI+45deg */

    PROFILE_ALL_BACKGROUND_NOISE_TABLE,   /**< 所有配置下通道底噪表 */

    UNKNOW_TABLE = 0xFF,                  /**< 未知表类型 */
  }TABLE_Typedef_t;

  /* 公共表头信息 */
  typedef struct
  {
    uint8_t Class_ID_Num;                 /**< 公共头部字段 */
    uint8_t Headr_Check_Sum;              /**< 私有头部数据校验和 */
    uint8_t Header_Size;                  /**< 私有头部数据大小长度 */
    uint8_t Version_MAJOR;                /**< 主版本号 v0 - 255 */
    uint8_t Version_MINOR;                /**< 副版本号 0 - 255 */
    uint8_t Version_REVISION;             /**< 修订版本号 0 - 255 */
    uint8_t Table_Type;                   /**< 表类型 @ref TABLE_Typedef_t */
    uint8_t Data_Type;                    /**< 存储在flash中的数据类型 @ref DATA_Typedef_t */
    uint32_t Data_Size;                   /**< 表数据字节数，也是校验CRC部分大小 */
    uint32_t Crc_Val;                     /**< 表数据CRC */
  }TABLE_HEADER_Typedef_t;

  /* DoA导向矢量表头信息 */
  typedef struct
  {
    TABLE_HEADER_Typedef_t Common_Info;   /**< 公共头部信息 */
    float Start_Angle;                    /**< 起始角度 */
    float End_Angle;                      /**< 结束角度 */
    uint16_t Points;                      /**< 点数 */
    uint8_t Channel_Num;                  /**< 通道数量 */
    float Azi_Ele_Angle;                  /**< 水平导向矢量@俯仰角，俯仰导向矢量@水平角 */
    uint8_t Clibration_Tx_Order[4];       /**< TX在校准时的发射配置 */
    uint8_t Profile_ID;                   /**< 当前配置属于哪个校准Profile */
  }DOA_TABLE_HEADER_Typedef_t;

  /* DoA天线补偿表头信息 */
  typedef struct
  {
    TABLE_HEADER_Typedef_t Common_Info;   /**< 公共头部信息 */
    uint16_t Points;                      /**< 点数 */
    uint8_t Channel_Num;                  /**< 通道数量 */
    uint8_t Clibration_Tx_Order[4];       /**< TX在校准时的发射配置 */
    uint8_t Profile_ID;                   /**< 当前配置属于哪个校准Profile */
  }ANT_TABLE_HEADER_Typedef_t;

  /* 通道底噪表头信息 */
  typedef struct
  {
    TABLE_HEADER_Typedef_t Common_Info;   /**< 公共头部信息 */
    uint8_t Channel_Num[4];               /**< Profile下通道数量 */
    uint8_t Unit;                         /**< 单位 @ref UNIT_Typedef_t */
    uint8_t Clibration_Tx_Order[4][4];    /**< Profile下TX在校准时的发射配置 */
  }SYS_NOISE_TABLE_HEADER_Typedef_t;

  /* 方向图即角度能量图表头信息 */
  typedef struct
  {
    TABLE_HEADER_Typedef_t Common_Info;   /**< 公共头部信息 */
    float Start_Angle;                    /**< 起始角度 */
    float End_Angle;                      /**< 结束角度 */
    uint16_t Points;                      /**< 点数 */
    uint8_t Channel_Num;                  /**< 通道数量 */
    uint8_t Unit;                         /**< 单位 @ref UNIT_Typedef_t */
    uint8_t Clibration_Tx_Order[4];       /**< TX在校准时的发射配置 */
    uint8_t Profile_ID;                   /**< 当前配置属于哪个校准Profile */
  }SYS_PATTERN_TABLE_HEADER_Typedef_t;

  /* 传输表头信息 */
  typedef struct
  {
    TABLE_HEADER_Typedef_t Common_Info;   /**< 公共头部信息 */
    quint8 private_header[64];            /**< 私有头部信息 */
  }COMMON_TABLE_HEADER_Typedef_t;

  /* 功能码 */
  typedef enum
  {
    EOL_WRITE_CMD = 0x00,
    EOL_READ_CMD  = 0x01,

    EOL_META_CMD  = 0xFF,
  }EOL_CMD_Typedef_t;

  /* 通讯硬件选择 */
  typedef enum
  {
    EOL_CAN_HW = 0,       /**< can发送数据 */
    EOL_SERIAL_HW,        /**< 串口发送数据 */
    EOL_ETH_HW,           /**< 网络发送数据 */
  }EOL_SEND_HW_Typedef_t;

  typedef struct EOL_TASK_LIST_
  {
    void *param;/**< 传输表数据，使用 */
    quint8 reg;
    EOL_CMD_Typedef_t command;
    quint16 len;
    quint8 buf[256];
    EOL_SEND_HW_Typedef_t com_hw;
    QString channel_num;
    bool (eol_protocol::*task)(void *param);
  }EOL_TASK_LIST_Typedef_t;
private:
  /* 等待回复结果 */
  typedef enum
  {
    RETURN_OK = 0,
    RETURN_UPGRADE_FRAME_REDIRECT,/**< 升级帧重定向 */
    RETURN_UPLOAD_FRAME_REDIRECT,
    RETURN_TIMEOUT,
    RETURN_WAITTING,              /**< 等待中 */
    RETURN_UPLOAD_END,            /**< 升级结束 */
    RETURN_ERROR,
    RETURN_LOST_FRAME,            /**< 丢帧 */
  }RETURN_TYPE_Typedef_t;

  /* 响应队列 */
  typedef struct
  {
    uint16_t reg_addr;      /**< 等待响应的寄存器地址 */
    uint32_t start_time;    /**< 等待相应的起始时间 */
    uint8_t command;        /**< 等待的命令类型 */
    EOL_SEND_HW_Typedef_t com_hw; /**< 通讯硬件选择 */
    QString channel_num;          /**< 通讯硬件端口 */
  }WAIT_RESPONSE_LIST_Typedef_t;

  /* 分包分送句柄 */
  typedef struct
  {
    quint32 last_send_time_ms;
    quint32 hw_send_err_times;
    uint32_t wait_send_size;
    uint32_t current_send_index;
    uint32_t data_total_size;
    uint8_t *buf_ptr;
    EOL_SEND_HW_Typedef_t com_hw; /**< 通讯硬件选择 */
    QString channel_num;          /**< 通讯硬件端口 */
  }SEND_TASK_LIST_Typedef_t;

  /* 发送检测状态 */
  typedef enum
  {
    WAIT_NOTHING = 0,
    WAIT_SEND,
    SEND_ERR
  }SNED_CHECK_STATUS_Typedef_t;

  /* 发送表数据任务参数 */
  typedef struct
  {
    COMMON_TABLE_HEADER_Typedef_t head_data;
    const quint8 *data;
    quint8 reg;
    EOL_CMD_Typedef_t command;
    quint16 len;
    quint8 buf[256];
  }EOL_SEND_DATA_Typedef_t;

public:

  /**
   * @brief eol协议线程
   */
  virtual void run() override
  {
    qDebug() << "eol protocol wait to run";
    emit signal_eol_protol_is_start();
    run_state = true;
    thread_run_state = true;
    run_eol_task();
    thread_run_state = false;
    run_state = false;    
    qDebug() << "[thread]" << QThread::currentThreadId() << "eol protocol end";

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
    qDebug() << "eol protocol stop_task";
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
      qDebug() << "eol protocol is running";
      return;
    }

    listen_run_state = listen_cs;
    if(thread_run_state)
    {
      qDebug() << "eol protocol is running";
      return;
    }
    eol_protocol_clear();
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
   * @brief 设置线程池
   * @param g_thread_pool_
   */
  void set_thread_pool(QThreadPool *g_thread_pool_)
  {
    g_thread_pool = g_thread_pool_;
  }

  /**
   * @brief 设置can驱动对象
   * @param can_driver_
   */
  void set_can_driver_obj(can_driver *can_driver_ = nullptr);

  /**
   * @brief 设置eol通讯端口
   * @param com_hw_ 硬件
   */
  void set_eol_com_config_hw(EOL_SEND_HW_Typedef_t com_hw_)
  {
    com_hw = com_hw_;
  }

  /**
   * @brief 设置eol通讯端口
   * @param channel_num_ 通道号 "0" or "1"
   */
  void set_eol_com_config_channel(QString channel_num_)
  {
    channel_num = channel_num_;
  }

  /**
   * @brief 设置eol通讯端口
   * @param channel_num_ 通道号 "0" or "1"
   */
  void set_eol_vcom_config_channel(QString channel_num_)
  {
    vchannel_num = channel_num_;
  }

  /**
   * @brief 添加设置设备工作模式任务
   * @param mode 0生产普通 1生产校准
   * @return true 任务添加成功
   */
  bool eol_master_set_device_mode(DEVICE_MODE_Typedef_t mode);

  /**
   * @brief 发送表数据
   * @param table_info 表信息
   * @param data 表数据
   * @return true正确
   */
  bool eol_master_send_table_data(COMMON_TABLE_HEADER_Typedef_t &table_info, const quint8 *data);

  /**
   * @brief 获取表数据
   * @param table_type 表类型
   * @return true正确
   */
  bool eol_master_get_table_data(TABLE_Typedef_t table_type);

  /**
   * @brief 读写设备
   * @param task 任务
   * @param check_repeat true任务查重
   * @return true正常
   */
  bool eol_master_common_rw_device(EOL_TASK_LIST_Typedef_t &task, bool check_repeat = true);

  /**
   * @brief 获取表类型
   * @param profile_id 配置ID
   * @param class_type 类ID
   * @return 表类型
   */
  static TABLE_Typedef_t get_table_type(quint8 profile_id, TABLE_CLASS_Typedef_t class_type);

  /**
   * @brief 获取表分类
   * @param table_type 表类型
   * @return 表类
   */
  static TABLE_CLASS_Typedef_t get_table_class(TABLE_Typedef_t table_type);
signals:
  /**
   * @brief 协议栈启动信号
   */
  void signal_eol_protol_is_start();

  /**
   * @brief 协议栈无回复超时
   * @param sec 秒
   */
  void signal_protocol_timeout(quint32 sec);

  /**
   * @brief 从机无应答信号-表操作
   */
  void signal_protocol_no_response();

  /**
   * @brief 从机返回错误消息
   * @param error_msg 错误消息
   */
  void signal_protocol_error_occur(quint8 error_msg);

  /**
   * @brief 从机读写无反应信号
   * @param reg 读写寄存器地址
   * @param command 读写标识
   */
  void signal_protocol_rw_err(quint8 reg, quint8 command);

  /**
   * @brief 读写设备正常
   * @param reg 寄存器
   * @param data 读出的数据，为nullptr代表写设备成功
   * @param data_len 代表数据长度，为0时代表写设备成功
   */
  void signal_rw_device_ok(quint8 reg, const quint8 *data = nullptr, quint16 data_len = 0);

  /**
   * @brief 表数据
   * @param frame_num 帧号
   * @param data 数据
   * @param data_len 数据长度
   */
  void signal_recv_eol_table_data(quint16 frame_num, const quint8 *data, quint16
                                   data_len);

  /**
   * @brief 发送进度信号
   * @param current_size 当前大小
   * @param total_size 总大小
   */
  void signal_send_progress(quint32 current_size, quint32 total_size);

  /**
   * @brief 发送完成信号
   */
  void signal_send_eol_data_complete();

  /**
   * @brief 接收数据完成-表传输
   */
  void signal_recv_eol_data_complete();

  /**
   * @brief 设备当前模式
   * @param pass_data 返回数据[0]模式 [1]配置数 [2]天线通道数
   */
  void signal_device_mode(const void *pass_data);

  /**
   * @brief 传输一帧触发计数
   * @param dir true发送
   */
  void signal_send_rec_one_frame(bool dir);
private:
  /**
   * @brief 定时器初始化
   */
  void timer_init();

  /**
   * @brief 清除等待列表，缓冲区数据
   */
  void eol_protocol_clear();

  /**
   * @brief eol任务
   */
  void run_eol_task();

  /**
   * @brief eol发送消息接口
   * @param data 数据
   * @param data_len 数据长度
   * @param com_hw 通讯硬件选择
   * @param channel_num 通讯端口选择
   * @return true发送成功 false发送失败
   */
  bool eol_send_data_port(const uint8_t *data, uint16_t data_len, \
    EOL_SEND_HW_Typedef_t com_hw, QString &channel_num);

  /**
   * @brief 待回复任务检测
   * @param force 强制发送
   * @return 状态
   */
  SNED_CHECK_STATUS_Typedef_t check_wait_send_task(bool force = false);

  /**
   * @brief 创建一帧报文并发送
   * @param command 命令类型
   * @param reg_addr 寄存器地址
   * @param data 数据
   * @param data_len 数据长度
   * @param com_hw 硬件通讯选择
   * @param channel_num 通讯端口
   * @return 报文状态
   */
  RETURN_TYPE_Typedef_t protocol_stack_create_task(EOL_CMD_Typedef_t command, \
      uint8_t reg_addr, const uint8_t *data, uint16_t data_len, \
      EOL_SEND_HW_Typedef_t com_hw = EOL_CAN_HW, QString channel_num = "0");

  /**
   * @brief 等待回复数据
   * @param listen_mode 监听模式，监听模式不要求等待列表不为空，即不要求有请求报文
   * @return RETURN_OK 正常
   */
  RETURN_TYPE_Typedef_t protocol_stack_wait_reply_start(bool listen_mode = false);

  /**
   * @brief 解析ack应答帧
   * @param reg_addr 寄存器地址
   * @param data 帧数据
   * @return 应答消息
   */
  EOL_OPT_STATUS_Typedef_t decode_ack_frame(quint8 reg_addr, const quint8 *data);

  /**
   * @brief 解析接收数据
   * @param reg_addr 寄存器地址
   * @param data 数据
   * @param data_len 数据长度
   * @return 状态
   */
  RETURN_TYPE_Typedef_t decode_data_frame(quint8 reg_addr, const quint8 *data, quint16 data_len);

  /**
   * @brief 回复超时检测
   * @param wait 队列
   * @return true代表超时
   */
  bool response_is_timeout(WAIT_RESPONSE_LIST_Typedef_t &wait);

  /**
   * @brief 缓冲区有效数据长度
   * @param cq 环形队列
   * @return 数据长度
   */
  uint32_t check_can_read(CircularQueue::CQ_handleTypeDef *cq);

  /**
   * @brief 线程任务--获取表数据
   * @param param 参数
   * @return true任务成功
   */
  bool get_eol_table_data_task(void *param_);

  /**
   * @brief 线程任务--发送表数据
   * @param param_ 参数
   * @return true任务成功
   */
  bool send_eol_table_data_task(void *param_);

  /**
   * @brief 线程任务--设置设备模式
   * @param param_ 参数
   * @return true任务成功
   */
  bool set_device_mode_task(void *param_);

  /**
   * @brief 线程任务--通用读写设备
   * @param param_ 参数
   * @return true任务成功
   */
  bool common_rw_device_task(void *param_);
private:
  /* 定时器 */
  QTimer *protocol_Timer = nullptr;
  quint64 current_time_sec = 0;
  quint64 current_time_ms = 0;

  /* 错误统计 */
  quint32 acc_error_cnt = 0;

  /* 发送队列 */
  SEND_TASK_LIST_Typedef_t send_task_handle;

  /* 响应队列 */
  QList<WAIT_RESPONSE_LIST_Typedef_t>wait_response_list;

  /* 任务队列 */
  QList<EOL_TASK_LIST_Typedef_t>eol_task_list;

private slots:

  /**
   * @brief 定时器超时处理
   */
  void slot_timer_timeout();

private:
  QSemaphore sem;
  bool run_state = false;
  bool thread_run_state = false;
  bool listen_run_state = false;

  QThreadPool *g_thread_pool = nullptr;
  CircularQueue *cq_obj = nullptr;
  can_driver *can_driver_obj = nullptr;

  quint32 current_send_can_id;

  typedef struct
  {
    quint8 data_buf[25 * 1024];
    quint16 frame_num;
    quint32 data_size;
  }DATA_RECORD_Typedef_t;

  DATA_RECORD_Typedef_t data_record;

  EOL_SEND_DATA_Typedef_t send_table_data;

  quint32 access_code;
  quint8 current_run_mode;

  EOL_SEND_HW_Typedef_t com_hw = EOL_CAN_HW; /**< 通讯硬件选择 */
  QString channel_num = "0";          /**< 通讯硬件端口 */
  QString vchannel_num = "255";          /**< 通讯硬件端口 */
  QAtomicInt thread_run_statex;
};
#endif
/******************************** End of file *********************************/
