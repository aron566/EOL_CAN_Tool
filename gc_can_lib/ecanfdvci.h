#ifndef _ECANFDVCI_H_
#define _ECANFDVCI_H_

#include <QObject>

/* 接口卡类型定义 */
#define GC_USBCANFD 6 // 不可更改

#define CANFD_DEVICE_MAX_NUMBER 16         // 定义一台电脑最大可以连接的USBCANFD的数量，用户可根据需求进行修改
#define CANFD_ERRFRAME_MAX_NUMBER 1000     // 定义错误帧缓冲的大小，用户可根据需求进行修改
#define CANFD_RECEFBUFFER_MAX_NUMBER 10000 // 定义数据帧缓冲的大小，用户可根据需求进行修改

#define CAN_0 0 // CANFD设备通道1的宏定义
#define CAN_1 1 // CANFD设备通道2的宏定义

#define NONUSE 0
#define USED 1

/* 无错误时为0 */
#define GC_CANFD_STATUS_OK 0
/* CAN错误码 */
#define ERR_CAN_NOINIT 0x0001  // CAN未初始化就被用于发送
#define ERR_CAN_DISABLE 0x0002 // CAN未使能就被用于发送
#define ERR_CAN_BUSOFF 0x0004  // CAN已经离线

#define ERR_DATA_LEN 0x0010  // 数据长度错误
#define ERR_USB_WRITE 0x0020 // USB写数据失败
#define ERR_USB_READ 0x0040  // USB读数据失败

/* 通用错误码 */
#define ERR_DEVICEOPENED 0x0100   // 设备已经打开
#define ERR_DEVICEOPEN 0x0200     // 打开设备错误
#define ERR_DEVICENOTOPEN 0x0400  // 设备没有打开
#define ERR_BUFFEROVERFLOW 0x0800 // 缓冲区溢出
#define ERR_DEVICENOTEXIST 0x1000 // 此设备不存在
#define ERR_DEVICECLOSE 0x2000    // 关闭设备失败

/* 定义发送模式选择 */
typedef enum
{
  POSITIVE_SEND = 0,  /**< 主动发送模式，不用发送缓冲，需要等数据都发完才能再次发送数据 */
  PASSIVE_SEND,       /**< 被动发送模式，使用发送缓冲，只要发送缓冲没用完就能发送数据 */
} GC_SEND_MODE_FD;

/* 定义接收模式选择 */
typedef enum
{
  SPECIFIED_RECEIVE = 0,  /**< 特定接收模式，最多可以使用8个滤波器 */
  GLOBAL_STANDARD_RECEIVE,/**< 标准帧接收模式 */
  GLOBAL_EXTENDED_RECEIVE,/**< 扩展帧接收模式 */
  GLOBAL_STANDARD_AND_EXTENDED_RECEIVE,/**< 标准帧和扩展帧都接收模式 */
} GC_RECEIVE_MODE_FD;

/* 定义CAN波特率的选择类型 */
typedef enum
{
  BAUDRATE_1M = 0,
  BAUDRATE_800K,
  BAUDRATE_500K,
  BAUDRATE_400K,
  BAUDRATE_250K,
  BAUDRATE_200K,
  BAUDRATE_125K,
  BAUDRATE_100K,
  BAUDRATE_80K,
  BAUDRATE_62500,
  BAUDRATE_50K,
  BAUDRATE_40K,
  BAUDRATE_25K,
  BAUDRATE_20K,
  BAUDRATE_10K,
  BAUDRATE_5K,
} GC_BAUD_RATE_FD;

/* 定义数据波特率的选择类型 */
typedef enum
{
  DATARATE_5M = 0,
  DATARATE_4M,
  DATARATE_2M,
  DATARATE_1M,
  DATARATE_800K,
  DATARATE_500K,
  DATARATE_400K,
  DATARATE_250K,
  DATARATE_200K,
  DATARATE_125K,
  DATARATE_100K,
  DATARATE_80K,
  DATARATE_62500,
  DATARATE_50K,
  DATARATE_40K,
  DATARATE_25K,
  DATARATE_20K,
  DATARATE_10K,
  DATARATE_5K,
} GC_DATA_RATE_FD;

/* 1.系列接口卡信息的数据类型。共75字节 */
typedef struct _BOARD_INFO_GC_CANFD
{
  quint16 hw_Version;       // 2
  quint16 fw_Version;       // 2
  quint16 dr_Version;       // 2
  quint16 in_Version;       // 2
  quint16 irq_Num;          // 2
  quint8 can_Num;           // 1
  qint8 str_Serial_Num[20]; // 20
  qint8 str_hw_Type[40];    // 40
  quint16 Reserved[4];      // 4
} GC_CANFD_BOARD_INFO, *P_BOARD_INFO_GC_CANFD;

typedef struct _BYTE_TYPE_FD
{
  quint8 Bit0 : 1;
  quint8 Bit1 : 1;
  quint8 Bit2 : 1;
  quint8 Bit3 : 1;
  quint8 Bit4 : 1;
  quint8 Bit5 : 1;
  quint8 Bit6 : 1;
  quint8 Bit7 : 1;
} GC_BYTE_TYPE_FD;

typedef struct _CANFDFRAME_TYPE
{
  quint8 proto : 1;       // CAN/CANFD
  quint8 format : 1;      // STD/EXD
  quint8 type : 1;        // DATA/RTR
  quint8 bitratemode : 1; // bitrateswitch on/off
  quint8 reserved : 4;
} GC_CANFDFRAME_TYPE;

/* 定义初始化CANFD的数据类型 */
typedef struct _INIT_CONFIG_GC_CANFD
{                                 // 78字节
  quint8 CanReceMode;             // 接收模式设置
  quint8 CanSendMode;             // 发送模式设置
  ulong NominalBitRate;           // 不需要用户设定	Nominal Bit Rate
  ulong DataBitRate;              // 不需要用户设定	Data Bit Rate 在数据波特率可变时才是有意义的
  GC_BYTE_TYPE_FD FilterUsedBits; // 定义接收模式为 SPECIFIED_RECEIVE 时，决定滤波器1~8的是否被使用
  GC_BYTE_TYPE_FD StdOrExdBits;   // 定义接收模式为 SPECIFIED_RECEIVE 时，决定滤波器1~8属于标准还是扩展
  quint8 NominalBitRateSelect;    // 需要用户设定
  quint8 DataBitRateSelect;       // 需要用户设定
  /* 定义接收模式为 SPECIFIED_RECEIVE 时，下面的8个滤波器的配置才有意义，并且至少需要使用和配置一个 */
  /* 滤波器1的CANID设置和屏蔽设置 */
  ulong StandardORExtendedfilter1;     // Standard Or Extended filter1
  ulong StandardORExtendedfilter1Mask; // Standard Or Extended filter1 Mask
  /* 滤波器2的CANID设置和屏蔽设置 */
  ulong StandardORExtendedfilter2;     // Extended Or Extended filter2
  ulong StandardORExtendedfilter2Mask; // Extended Or Extended filter2 Mask
  /* 滤波器3的CANID设置和屏蔽设置 */
  ulong StandardORExtendedfilter3;     // Standard Or Extended filter3
  ulong StandardORExtendedfilter3Mask; // Standard Or Extended filter3 Mask
  /* 滤波器4的CANID设置和屏蔽设置 */
  ulong StandardORExtendedfilter4;     // Extended Or Extended filter4
  ulong StandardORExtendedfilter4Mask; // Extended Or Extended filter4 Mask
  /* 滤波器5的CANID设置和屏蔽设置 */
  ulong StandardORExtendedfilter5;     // Standard Or Extended filter5
  ulong StandardORExtendedfilter5Mask; // Standard Or Extended filter5 Mask
  /* 滤波器6的CANID设置和屏蔽设置 */
  ulong StandardORExtendedfilter6;     // Extended Or Extended filter6
  ulong StandardORExtendedfilter6Mask; // Extended Or Extended filter6 Mask
  /* 滤波器7的CANID设置和屏蔽设置 */
  ulong StandardORExtendedfilter7;     // Standard Or Extended filter5
  ulong StandardORExtendedfilter7Mask; // Standard Or Extended filter5 Mask
  /* 滤波器8的CANID设置和屏蔽设置 */
  ulong StandardORExtendedfilter8;     // Extended Or Extended filter6
  ulong StandardORExtendedfilter8Mask; // Extended Or Extended filter6 Mask
} GC_INIT_CONFIG_FD, *P_INIT_CONFIG_GC_CANFD;

#define GC_STANDARD_FORMAT 0 // 标准帧
#define GC_EXTENDED_FORMAT 1 // 扩展帧

#define GC_DATA_TYPE 0 // 数据帧
#define GC_RTR_TYPE 1  // 远程帧

#define GC_CAN_TYPE 0   // 标准CAN
#define GC_CANFD_TYPE 1 // CANFD

#define BITRATESITCH_ON 1  // 数据波特率可变
#define BITRATESITCH_OFF 0 // 数据波特率不变

typedef struct _TIMESTAMP_TYPE
{
  quint8 mday;
  quint8 hour;
  quint8 minute;
  quint8 second;
  quint16 millisecond;
  quint16 microsecond;
} GC_TIMESTAMP_TYPE_FD;

/* 定义CANFD信息帧的数据类型 */
typedef struct _GC_CANFD_OBJ
{                                    // 80字节
  GC_CANFDFRAME_TYPE CanORCanfdType; // 定义帧的类型
  quint8 DataLen;                    // 有效数据长度
  quint8 Reserved[2];                // 保留
  ulong ID;                          // CANID
  GC_TIMESTAMP_TYPE_FD TimeStamp;    // 时间戳
  quint8 Data[64];                   // 数据字节
} GC_CANFD_OBJ, *P_CANFD_OBJ_GC_CANFD;

typedef struct _CANFD_ECR_TYPE
{
  ulong TEC : 8;
  ulong REC : 7;
  ulong RP : 1;
  ulong CEL : 8;
  ulong : 8;
} GC_CANFD_ECR_TYPE;

typedef struct _CANFD_PSR_TYPE
{
  ulong LEC : 3;
  ulong ACT : 2;
  ulong EP : 1;
  ulong EW : 1;
  ulong BO : 1;
  ulong DLEC : 3;
  ulong RESI : 1;
  ulong RBRS : 1;
  ulong RFDF : 1;
  ulong PXE : 1;
  ulong : 1;
  ulong TDCV : 7;
  ulong : 9;
} GC_CANFD_PSR_TYPE;

/* 定义CANFD错误帧的数据类型 */
typedef struct _ERR_FRAME_FD
{                                     // 错误帧
  GC_TIMESTAMP_TYPE_FD can_timestamp; // 时间戳
  GC_CANFD_ECR_TYPE can_ecr_register; // 错误计数寄存器
  GC_CANFD_PSR_TYPE can_psr_register; // 协议状态寄存器
} GC_CANFD_ERR_FRAME, *P_ERR_FRAME_GC_CANFD;

/* 定义CANFD设备状态的数据类型 */
typedef struct _CANFD_STATUS
{

  quint16 LeftSendBufferNum;           // 未使用的发送缓冲数，被动发送模式使用发动缓冲，主动模式不使用
  GC_TIMESTAMP_TYPE_FD can0_timestamp; // 时间戳
  GC_CANFD_ECR_TYPE can0_ecr_register; // 错误计数寄存器
  GC_CANFD_PSR_TYPE can0_psr_register; // 协议状态寄存器
  ulong can0_RxLost_Cnt;               // 接收失败计数
  ulong can0_TxFail_Cnt;               // 发送失败计数
  float can0_Load_Rate;                // 负载率

  GC_TIMESTAMP_TYPE_FD can1_timestamp; // 时间戳
  GC_CANFD_ECR_TYPE can1_ecr_register; // 错误计数寄存器
  GC_CANFD_PSR_TYPE can1_psr_register; // 协议状态寄存器
  ulong can1_RxLost_Cnt;               // 接收失败计数
  ulong can1_TxFail_Cnt;               // 发送失败计数
  float can1_Load_Rate;                // 负载率
} GC_CANFD_STATUS, *P_CANFD_STATUS_GC_CANFD;


class gc_canfd_lib_tool
{
  public:
    /**
     * @brief 获取波特率配置
     * @param AbitTiming_kb 1000代表需设置1Mb的仲裁域波特率
     * @param DbitTiming_kb 数据域速率
     * @param Timing0 返回配置
     * @param Timing1 返回配置
     */
    static void get_bauds(quint32 AbitTiming_kb, quint32 DbitTiming_kb, quint8 *Timing0, quint8 *Timing1)
    {
      switch(AbitTiming_kb)
      {
        case 1000:
          *Timing0 = BAUDRATE_1M;
          break;

        case 800:
          *Timing0 = BAUDRATE_800K;
          break;

        case 500:
          *Timing0 = BAUDRATE_500K;
          break;

        case 400:
          *Timing0 = BAUDRATE_400K;
          break;

        case 250:
          *Timing0 = BAUDRATE_250K;
          break;

        case 200:
          *Timing0 = BAUDRATE_200K;
          break;

        case 125:
          *Timing0 = BAUDRATE_125K;
          break;

        case 100:
          *Timing0 = BAUDRATE_100K;
          break;

        case 80:
          *Timing0 = BAUDRATE_80K;
          break;

        case 50:
          *Timing0 = BAUDRATE_50K;
          break;

        case 40:
          *Timing0 = BAUDRATE_40K;
          break;

        case 20:
          *Timing0 = BAUDRATE_20K;
          break;

        case 10:
          *Timing0 = BAUDRATE_10K;
          break;

        case 5:
          *Timing0 = BAUDRATE_5K;
          break;

        default://500
          *Timing0 = BAUDRATE_500K;
          break;
      }

      switch(DbitTiming_kb)
      {
        case 5000:
          *Timing1 = DATARATE_5M;
          break;

        case 4000:
          *Timing1 = DATARATE_4M;
          break;

        case 2000:
          *Timing1 = DATARATE_2M;
          break;

        case 1000:
          *Timing1 = DATARATE_1M;
          break;

        case 800:
          *Timing1 = DATARATE_800K;
          break;

        case 500:
          *Timing1 = DATARATE_500K;
          break;

        case 400:
          *Timing1 = DATARATE_400K;
          break;

        case 250:
          *Timing1 = DATARATE_250K;
          break;

        case 200:
          *Timing1 = DATARATE_200K;
          break;

        case 125:
          *Timing1 = DATARATE_125K;
          break;

        case 100:
          *Timing1 = DATARATE_100K;
          break;

        case 80:
          *Timing1 = DATARATE_80K;
          break;

        case 50:
          *Timing1 = DATARATE_50K;
          break;

        case 40:
          *Timing1 = DATARATE_40K;
          break;

        case 20:
          *Timing1 = DATARATE_20K;
          break;

        case 10:
          *Timing1 = DATARATE_10K;
          break;

        case 5:
          *Timing1 = DATARATE_5K;
          break;

        default://500
          *Timing1 = DATARATE_500K;
          break;
      }
    }

    /**
     * @brief 获取fd发送长度
     * @param len 预期发送长度
     * @return 实际需要发送的长度
     */
    static quint8 get_send_len(quint8 len)
    {
      if(8U >= len)
      {
        return len;
      }

      if(12 >= len)
      {
        return 12U;
      }

      if(16 >= len)
      {
        return 16U;
      }

      if(20 >= len)
      {
        return 20U;
      }

      if(24 >= len)
      {
        return 24U;
      }

      if(32 >= len)
      {
        return 32U;
      }

      if(48 >= len)
      {
        return 48U;
      }

      if(64 >= len)
      {
        return 64U;
      }
      return len;
    }
};

#ifdef __cplusplus
extern "C"
{
#endif

  ulong __stdcall OpenDeviceFD(ulong DeviceType, ulong DeviceInd);                                                                 // 打开CANFD设备
  ulong __stdcall CloseDeviceFD(ulong DeviceType, ulong DeviceInd);                                                                // 关闭CANFD设备
  ulong __stdcall InitCANFD(ulong DeviceType, ulong DeviceInd, quint8 CANInd, P_INIT_CONFIG_GC_CANFD pInitConfig);                 // 初始化CANFD设备通道1或2
  ulong __stdcall TransmitFD(ulong DeviceType, ulong DeviceInd, quint8 CANInd, P_CANFD_OBJ_GC_CANFD pCanfdMQ, ulong Len);          // CANFD设备通道1或2发送数据
  ulong __stdcall Receive_buffer_thread(ulong DeviceType, ulong DeviceInd, ulong WaitTime);                                        // CANFD设备通道1和2共用的接收线程，把数据帧和错误帧放入缓冲中，需要一直运行
  ulong __stdcall ReceiveFD(ulong DeviceType, ulong DeviceInd, quint8 CANInd, P_CANFD_OBJ_GC_CANFD pCanfdMQ, ulong *Len);          // 从CANFD设备通道1或2的数据帧缓冲中读出数据帧
  ulong __stdcall GetErrFrame(ulong DeviceType, ulong DeviceInd, quint8 CANInd, P_ERR_FRAME_GC_CANFD pCanfdErrbuffer, ulong *Len); // 从CANFD设备通道1或2的错误帧缓冲中读出错误帧
  ulong __stdcall ResetCANFD(ulong DeviceType, ulong DeviceInd, quint8 CANInd);                                                    // 复位CANFD设备通道1或2
  ulong __stdcall StartCANFD(ulong DeviceType, ulong DeviceInd, quint8 CANInd);                                                    // 开始CANFD设备通道1或2
  ulong __stdcall StopCANFD(ulong DeviceType, ulong DeviceInd, quint8 CANInd);                                                     // 停止CANFD设备通道1或2
  ulong __stdcall GetCanfdBusStatus(ulong DeviceType, ulong DeviceInd, P_CANFD_STATUS_GC_CANFD p_canfd_status);                    // 获取CANFD设备状态信息

  /**
   * @brief GetReference
   * @param DeviceType
   * @param DeviceInd
   * @param CANInd
   * @param RefType 为1获取设备信息
   * @param pData
   * @return 0操作成功 1操作失败
   */
  ulong __stdcall GetReference(ulong DeviceType, ulong DeviceInd, quint8 CANInd, ulong RefType, void *pData);
//  ulong __stdcall SetReference(ulong DeviceType, ulong DeviceInd, quint8 CANInd, ulong RefType, void *pData);

#ifdef __cplusplus
}
#endif

#endif
