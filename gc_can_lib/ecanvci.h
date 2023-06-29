#ifndef _V_ECANVCI_H_
#define _V_ECANVCI_H_

#include <QObject>

// #define Dll_EXPORTS

/* 接口卡类型定义 */
#define GC_USBCAN1 3
#define GC_USBCAN2 4

/* CAN错误码 */
#define ERR_CAN_OVERFLOW    0x0001/**< CAN控制器内部FIFO溢出 */
#define ERR_CAN_ERRALARM    0x0002/**< CAN控制器错误报警 */
#define ERR_CAN_PASSIVE     0x0004/**< CAN控制器消极错误 */
#define ERR_CAN_LOSE        0x0008/**< CAN控制器仲裁丢失 */
#define ERR_CAN_BUSERR      0x0010/**< CAN控制器总线错误 */
#define ERR_CAN_REG_FULL    0x0020/**< CAN接收寄存器满 */
#define ERR_CAN_REG_OVER    0x0040/**< CAN接收寄存器溢出 */
#define ERR_CAN_ZHUDONG     0x0080/**< CAN控制器主动错误 */

/* 通用错误码 */
#define ERR_DEVICEOPENED    0x0100/**< 设备已经打开 */
#define ERR_DEVICEOPEN      0x0200/**< 打开设备错误 */
#define ERR_DEVICENOTOPEN   0x0400/**< 设备没有打开 */
#define ERR_BUFFEROVERFLOW  0x0800/**< 缓冲区溢出 */
#define ERR_DEVICENOTEXIST  0x1000/**< 此设备不存在 */
#define ERR_LOADKERNELDLL   0x2000/**< 装载动态库失败 */
#define ERR_CMDFAILED       0x4000/**< 执行命令失败错误码 */
#define ERR_BUFFERCREATE    0x8000/**< 内存不足 */

/* 函数调用返回状态值 */
#define GC_STATUS_OK  1
#define GC_STATUS_ERR 0

#define CMD_DESIP 0
#define CMD_DESPORT 1
#define CMD_CHGDESIPANDPORT 2

/* 1.接口卡信息的数据类型 */
typedef struct _GC_CAN_BOARD_INFO
{
  quint16 hw_Version;
  quint16 fw_Version;
  quint16 dr_Version;
  quint16 in_Version;
  quint16 irq_Num;
  quint8 can_Num;
  qint8 str_Serial_Num[20];
  qint8 str_hw_Type[40];
  quint16 Reserved[4];
} GC_CAN_BOARD_INFO, *P_GC_CAN_BOARD_INFO;

/* 2.定义CAN信息帧的数据类型 */
typedef struct _CAN_OBJ
{
  quint32 ID;
  quint32 TimeStamp;
  quint8 TimeFlag;
  quint8 SendType;
  quint8 RemoteFlag; /**< 是否是远程帧 */
  quint8 ExternFlag; /**< 是否是扩展帧 */
  quint8 DataLen;
  quint8 Data[8];
  quint8 Reserved[3];
} GC_CAN_OBJ, *P_CAN_OBJ;

/* 3.定义CAN控制器状态的数据类型 */
typedef struct _CAN_STATUS
{
  quint8 ErrInterrupt;
  quint8 regMode;
  quint8 regStatus;
  quint8 regALCapture;
  quint8 regECCapture;
  quint8 regEWLimit;
  quint8 regRECounter;
  quint8 regTECounter;
  ulong Reserved;
} CAN_STATUS, *P_CAN_STATUS;

/* 4.定义错误信息的数据类型 */
typedef struct _ERR_INFO
{
  quint32 ErrCode;
  quint8 Passive_ErrData[3];
  quint8 ArLost_ErrData;
} GC_ERR_INFO, *P_ERR_INFO;

/* 5.定义初始化CAN的数据类型 */
typedef struct _INIT_CONFIG
{
  ulong AccCode;
  ulong AccMask;
  ulong Reserved;
  quint8 Filter;
  quint8 Timing0;
  quint8 Timing1;
  quint8 Mode;
} GC_INIT_CONFIG, *P_INIT_CONFIG;

typedef struct _tagChgDesIPAndPort
{
  qint8 szpwd[10];
  qint8 szdesip[20];
  qint32 desport;
} CHGDESIPANDPORT;

typedef struct _FILTER_RECORD
{
  ulong ExtFrame; /**< 是否为扩展帧 */
  ulong Start;
  ulong End;
} FILTER_RECORD, *P_FILTER_RECORD;

class gc_can_lib_tool
{
  public:
    /**
     * @brief 获取波特率配置
     * @param bauds_kb 1000代表需设置1Mb的波特率
     * @param Timing0 返回配置
     * @param Timing1 返回配置
     */
    static void get_bauds(quint32 bauds_kb, quint8 *Timing0, quint8 *Timing1)
    {
      switch(bauds_kb)
      {
        case 1000:
          *Timing0 = 0;
          *Timing1 =0x14;
          break;

        case 800:
          *Timing0 = 0;
          *Timing1 = 0x16;
          break;

        case 666:
          *Timing0 = 0x80;
          *Timing1 = 0xb6;
          break;

        case 500:
          *Timing0 = 0;
          *Timing1 = 0x1c;
          break;

        case 400:
          *Timing0 = 0x80;
          *Timing1 = 0xfa;
          break;

        case 250:
          *Timing0 = 0x01;
          *Timing1 = 0x1c;
          break;

        case 200:
          *Timing0 = 0x81;
          *Timing1 = 0xfa;
          break;

        case 125:
          *Timing0 = 0x03;
          *Timing1 = 0x1c;
          break;

        case 100:
          *Timing0 = 0x04;
          *Timing1 = 0x1c;
          break;

        case 80:
          *Timing0 = 0x83;
          *Timing1 = 0xff;
          break;

        case 50:
          *Timing0 = 0x09;
          *Timing1 = 0x1c;
          break;

        case 40:
          *Timing0 = 0x87;
          *Timing1 = 0xff;
          break;

        case 20:
          *Timing0 = 0x18;
          *Timing1 = 0x1c;
          break;

        case 10:
          *Timing0 = 0x31;
          *Timing1 = 0x1c;
          break;

        case 5:
          *Timing0 = 0xbf;
          *Timing1 = 0xff;
          break;

        default://500
          *Timing0 = 0;
          *Timing1 = 0x1c;
          break;
      }
    }
};

#ifdef Dll_EXPORTS
#define DllAPI __declspec(dllexport)
#else
#define DllAPI __declspec(dllimport)
#endif

#define EXTERNC extern "C"
#define CALL __stdcall //__cdecl

#ifdef __cplusplus ///< use C compiler
extern "C" {
#endif

DllAPI ulong CALL OpenDevice(ulong DeviceType, ulong DeviceInd, ulong Reserved);
DllAPI ulong CALL CloseDevice(ulong DeviceType, ulong DeviceInd);

DllAPI ulong CALL InitCAN(ulong DeviceType, ulong DeviceInd, ulong CANInd, P_INIT_CONFIG pInitConfig);

DllAPI ulong CALL ReadBoardInfo(ulong DeviceType, ulong DeviceInd, P_GC_CAN_BOARD_INFO pInfo);
DllAPI ulong CALL ReadErrInfo(ulong DeviceType, ulong DeviceInd, ulong CANInd, P_ERR_INFO pErrInfo);
DllAPI ulong CALL ReadCANStatus(ulong DeviceType, ulong DeviceInd, ulong CANInd, P_CAN_STATUS pCANStatus);

//DllAPI ulong CALL GetReference(ulong DeviceType, ulong DeviceInd, ulong CANInd, ulong RefType, void *pData);
//DllAPI ulong CALL SetReference(ulong DeviceType, ulong DeviceInd, ulong CANInd, ulong RefType, void *pData);

DllAPI ulong CALL GetReceiveNum(ulong DeviceType, ulong DeviceInd, ulong CANInd);
DllAPI ulong CALL ClearBuffer(ulong DeviceType, ulong DeviceInd, ulong CANInd);

DllAPI ulong CALL StartCAN(ulong DeviceType, ulong DeviceInd, ulong CANInd);
DllAPI ulong CALL ResetCAN(ulong DeviceType, ulong DeviceInd, ulong CANInd);

DllAPI ulong CALL Transmit(ulong DeviceType, ulong DeviceInd, ulong CANInd, P_CAN_OBJ pSend, ulong Len);
DllAPI ulong CALL Receive(ulong DeviceType, ulong DeviceInd, ulong CANInd, P_CAN_OBJ pReceive, ulong Len, qint32 WaitTime);

#ifdef __cplusplus ///<end extern c
}
#endif
#endif
