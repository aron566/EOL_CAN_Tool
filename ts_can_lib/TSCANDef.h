#ifndef __LIBTSCAN_H
#define __LIBTSCAN_H

#include <iostream>
#include <windows.h>

// #define DLLIMPORT __declspec(dllimport)

#pragma pack(1)
#include <stdint.h>
typedef uint8_t  u8;
typedef int8_t   s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
typedef uint64_t u64;
typedef int64_t  s64;
typedef wchar_t  wchar;
typedef struct _u8x8
{
  u8 d[8];
} u8x8;
typedef struct _u8x64
{
  u8 d[64];
} u8x64;

typedef enum
{
  TS_CHN0,
  TS_CHN1,
  TS_CHN2,
  TS_CHN3,
  TS_CHN4,
  TS_CHN5,
  TS_CHN6,
  TS_CHN7,
  TS_CHN8,
  TS_CHN9,
  TS_CHN10,
  TS_CHN11,
  TS_CHN12,
  TS_CHN13,
  TS_CHN14,
  TS_CHN15,
  TS_CHN16,
  TS_CHN17,
  TS_CHN18,
  TS_CHN19,
  TS_CHN20,
  TS_CHN21,
  TS_CHN22,
  TS_CHN23,
  TS_CHN24,
  TS_CHN25,
  TS_CHN26,
  TS_CHN27,
  TS_CHN28,
  TS_CHN29,
  TS_CHN30,
  TS_CHN31,
} TS_APP_CHANNEL;

typedef enum : int
{
  lfdtCAN       = 0,
  lfdtISOCAN    = 1,
  lfdtNonISOCAN = 2
} TLIBCANFDControllerType;

typedef enum : int
{
  lfdmNormal     = 0,
  lfdmACKOff     = 1,
  lfdmRestricted = 2
} TLIBCANFDControllerMode;

typedef enum : byte
{
  ONLY_RX_MESSAGES,   // 只读取接收的数据
  TX_RX_MESSAGES      // 发送出去和接收的数据都读取出来
} READ_TX_RX_DEF;

typedef union
{
  u8 value;
  struct
  {
    u8 istx         : 1;
    u8 remoteframe  : 1;
    u8 extframe     : 1;
    u8 tbd          : 4;
    u8 iserrorframe : 1;
  } bits;
} TCANProperty;

// typedef struct _TCAN
// {
//   u8           FIdxChn;       // channel index starting from 0
//   TCANProperty FProperties;   // default 0, masked status:
//                               // [7] 0-normal frame, 1-error frame
//                               // [6-3] tbd
//                               // [2] 0-std frame, 1-extended frame
//                               // [1] 0-data frame, 1-remote frame
//                               // [0] dir: 0-RX, 1-TX
//   u8   FDLC;                  // dlc from 0 to 8
//   u8   FReserved;             // reserved to keep alignment
//   s32  FIdentifier;           // CAN identifier
//   u64  FTimeUS;               // timestamp in us  //Modified by Eric 0321
//   u8x8 FData;                 // 8 data bytes to send
// } TCAN;

typedef struct _TLibCAN
{
  u8           FIdxChn;       // channel index starting from 0
  TCANProperty FProperties;   // default 0, masked status:
                              // [7] 0-normal frame, 1-error frame
                              // [6-3] tbd
                              // [2] 0-std frame, 1-extended frame
                              // [1] 0-data frame, 1-remote frame
                              // [0] dir: 0-RX, 1-TX
  u8   FDLC;                  // dlc from 0 to 8
  u8   FReserved;             // reserved to keep alignment
  s32  FIdentifier;           // CAN identifier
  u64  FTimeUS;               // timestamp in us  //Modified by Eric 0321
  u8x8 FData;                 // 8 data bytes to send
} TLibCAN, *PLibCAN;

typedef union
{
  u8 value;
  struct
  {
    u8 EDL : 1;
    u8 BRS : 1;
    u8 ESI : 1;
    u8 tbd : 5;
  } bits;
} TCANFDProperty;
// CAN FD frame definition = 80 B
// CAN FD frame definition = 80 B
typedef struct _TLibCANFD
{
  u8           FIdxChn;           // channel index starting from 0        = CAN
  TCANProperty FProperties;       // default 0, masked status:            = CAN
                                  // [7] 0-normal frame, 1-error frame
                                  // [6] 0-not logged, 1-already logged
                                  // [5-3] tbd
                                  // [2] 0-std frame, 1-extended frame
                                  // [1] 0-data frame, 1-remote frame
                                  // [0] dir: 0-RX, 1-TX
  u8             FDLC;            // dlc from 0 to 15                     = CAN
  TCANFDProperty FFDProperties;   // [7-3] tbd                            <> CAN
                                  // [2] ESI, The E RROR S TATE I NDICATOR (ESI) flag is transmitted dominant by error active nodes, recessive by error passive nodes. ESI does not exist in CAN format frames
                                  // [1] BRS, If the bit is transmitted recessive, the bit rate is switched from the standard bit rate of the A RBITRATION P HASE to the preconfigured alternate bit rate of the D ATA P HASE . If it is transmitted dominant, the bit rate is not switched. BRS does not exist in CAN format frames.
                                  // [0] EDL: 0-normal CAN frame, 1-FD frame, added 2020-02-12, The E XTENDED D ATA L ENGTH (EDL) bit is recessive. It only exists in CAN FD format frames
  s32   FIdentifier;              // CAN identifier                       = CAN
  u64   FTimeUS;                  // timestamp in us                      = CAN
  u8x64 FData;                    // 64 data bytes to send                <> CAN
} TLibCANFD, *PLibCANFD;

typedef union
{
  u8 value;
  struct
  {
    u8 istx          : 1;
    u8 breaksended   : 1;
    u8 breakreceived : 1;
    u8 syncreceived  : 1;
    u8 hwtype        : 2;
    u8 isLogged      : 1;
    u8 iserrorframe  : 1;
  } bits;
} TLINProperty;
typedef struct _TLIN
{
  u8           FIdxChn;       // channel index starting from 0
  u8           FErrCode;      // 0: normal
  TLINProperty FProperties;   // default 0, masked status:
                              // [7] tbd
                              // [6] 0-not logged, 1-already logged
                              // [5-4] FHWType //DEV_MASTER,DEV_SLAVE,DEV_LISTENER
                              // [3] 0-not ReceivedSync, 1- ReceivedSync
                              // [2] 0-not received FReceiveBreak, 1-Received Break
                              // [1] 0-not send FReceiveBreak, 1-send Break
                              // [0] dir: 0-RX, 1-TX
  u8   FDLC;                  // dlc from 0 to 8
  u8   FIdentifier;           // LIN identifier:0--64
  u8   FChecksum;             // LIN checksum
  u8   FStatus;               // place holder 1
  u64  FTimeUS;               // timestamp in us  //Modified by Eric 0321
  u8x8 FData;                 // 8 data bytes to send
} TLibLIN, *PLibLIN;

typedef enum : byte   // new property in C++ 11
{
  LIN_Protocol_13,
  LIN_Protocol_20,
  LIN_Protocol_21,
  LIN_Protocol_J2602
} TLINProtocol;

typedef enum
{
  MasterNode,
  SlaveNode,
  MonitorNode
} TLIN_FUNCTION_TYPE;
#pragma pack()
// function pointer type
// 回调函数类型
// TS系列工具连接回调函数
typedef void(__stdcall *TTSCANConnectedCallback_t)(const size_t ADevicehandle);
// TS系列工具断开回调函数
typedef void(__stdcall *TTSCANDisConnectedCallback_t)(const size_t ADevicehandle);
// 高精度时钟回调函数
typedef void(__stdcall *THighResTimerCallback_t)(const u32 ADevicehandle);
// CAN报文接收回调函数
typedef void(__stdcall *TCANQueueEvent_Win32_t)(const TLibCAN *AData);
// CANFD报文接收回调函数
typedef void(__stdcall *TCANFDQueueEvent_Win32_t)(const TLibCANFD *AData);
// LIN报文接收回调函数
typedef void(__stdcall *TLINQueueEvent_Win32_t)(const TLibLIN *AData);

// 回调函数注册函数
// 注册CAN报文接收回调函数
typedef u32(__stdcall *tscan_register_event_can_t)(const size_t ADeviceHandle, const TCANQueueEvent_Win32_t ACallback);
// 反注册CAN报文接收回调函数
typedef u32(__stdcall *tscan_unregister_event_can_t)(const size_t ADeviceHandle, const TCANQueueEvent_Win32_t ACallback);

// 注册CAN报文接收回调函数
typedef u32(__stdcall *tscan_register_event_canfd_t)(const size_t ADeviceHandle, const TCANFDQueueEvent_Win32_t ACallback);
// 反注册CAN报文接收回调函数
typedef u32(__stdcall *tscan_unregister_event_canfd_t)(const size_t ADeviceHandle, const TCANFDQueueEvent_Win32_t ACallback);

// 注册LIN报文接收回调函数
typedef u32(__stdcall *tslin_register_event_lin_t)(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
// 反注册LIN报文接收回调函数
typedef u32(__stdcall *tslin_unregister_event_lin_t)(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);

// 注册FastLIN报文接收回调函数
typedef u32(__stdcall *tscan_register_event_fastlin_t)(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
// 反注册FastLIN报文接收回调函数
typedef u32(__stdcall *tscan_unregister_event_fastlin_t)(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);

// 功能函数类型
// 扫描在线的设备
typedef uint32_t(__stdcall *tscan_scan_devices_t)(uint32_t *ADeviceCount);
// 连接设备，ADeviceSerial !=NULL：连接指定的设备；ADeviceSerial == NULL：连接默认设备
typedef uint32_t(__stdcall *tscan_connect_t)(const char *ADeviceSerial, size_t *AHandle);
// 断开指定设备
typedef u32(__stdcall *tscan_disconnect_by_handle_t)(const size_t ADeviceHandle);
// 断开所有设备
typedef u32(__stdcall *tscan_disconnect_all_devices_t)(void);
// 初始化TSCANAPI模块
typedef void(__stdcall *initialize_lib_tscan_t)(bool AEnableFIFO, bool AEnableErrorFrame, bool AEnableTurbe);
// 释放TSCANAPI模块
typedef void(__stdcall *finalize_lib_tscan_t)(void);

// CAN工具相关
// 同步发送CAN报文
typedef u32(__stdcall *tscan_transmit_can_sync_t)(const size_t ADeviceHandle, const TLibCAN *ACAN, const u32 ATimeoutMS);
// 异步发送CAN报文
typedef u32(__stdcall *tscan_transmit_can_async_t)(const size_t ADeviceHandle, const TLibCAN *ACAN);
// 设置CAN报文波特率参数
typedef u32(__stdcall *tscan_config_can_by_baudrate_t)(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double ARateKbps, const u32 A120OhmConnected);

// CAN周期函数
// 添加周期发送CAN报文
typedef u32(__stdcall *tscan_add_cyclic_msg_can_t)(const size_t ADeviceHandle, const TLibCAN *ACAN, const float APeriodMS);   // float is single
// 去除周期发送CAN报文
typedef u32(__stdcall *tscan_delete_cyclic_msg_can_t)(const size_t ADeviceHandle, const TLibCAN *ACAN);
// 添加周期发送CANFD报文
typedef u32(__stdcall *tscan_add_cyclic_msg_canfd_t)(const size_t ADeviceHandle, const TLibCANFD *ACANFD, const float APeriodMS);   // single
// 去除周期发送CANFD报文
typedef u32(__stdcall *tscan_delete_cyclic_msg_canfd_t)(const size_t ADeviceHandle, const TLibCANFD *ACANFD);

// 读取CAN报文
// ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ACANBufferSize：存储数组的长度
// 返回值：实际收到的报文数量
typedef u32(__stdcall *tsfifo_receive_can_msgs_t)(const size_t ADeviceHandle, const TLibCAN *ACANBuffers, s32 *ACANBufferSize, u8 AChn, u8 ARXTX);

// CANFD工具相关
// 同步发送CANFD报文
typedef u32(__stdcall *tscan_transmit_canfd_sync_t)(const size_t ADeviceHandle, const TLibCANFD *ACAN, const u32 ATimeoutMS);
// 异步发送CANFD报文
typedef u32(__stdcall *tscan_transmit_canfd_async_t)(const size_t ADeviceHandle, const TLibCANFD *ACAN);
// 设置CANFD报文波特率参数
typedef u32(__stdcall *tscan_config_canfd_by_baudrate_t)(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double AArbRateKbps, const double ADataRateKbps, const TLIBCANFDControllerType AControllerType, const TLIBCANFDControllerMode AControllerMode, const u32 A120OhmConnected);
// 读取CANFD报文
// ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ACANBufferSize：存储数组的长度
// 返回值：实际收到的报文数量
typedef u32(__stdcall *tsfifo_receive_canfd_msgs_t)(const size_t ADeviceHandle, const TLibCANFD *ACANBuffers, s32 *ACANBufferSize, u8 AChn, u8 ARXTX);

// LIN工具相关
// 设置节点类型:ADeviceHandle:句柄；AChnIdx:通道号;0:MasterNode;1:SlaveNode;2:MonitorNode
typedef u32(__stdcall *tslin_set_node_funtiontype_t)(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const u8 AFunctionType);
// 请求下载新的ldf文件：该命令会清除设备中现存的所有ldf文件
typedef u32(__stdcall *tslin_apply_download_new_ldf_t)(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx);
// 同步发送LIN报文
typedef u32(__stdcall *tslin_transmit_lin_sync_t)(const size_t ADeviceHandle, const TLibLIN *ALIN, const u32 ATimeoutMS);
// 异步发送LIN报文
typedef u32(__stdcall *tslin_transmit_lin_async_t)(const size_t ADeviceHandle, const TLibLIN *ALIN);
// 异步发送LIN报文
typedef u32(__stdcall *tslin_transmit_fastlin_async_t)(const size_t ADeviceHandle, const TLibLIN *ALIN);
// 设置LIN报文波特率参数
typedef u32(__stdcall *tslin_config_baudrate_t)(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double ARateKbps, TLINProtocol AProtocol);

// 读取LIN报文
// ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ALINBufferSize：存储数组的长度
// 返回值：实际收到的报文数量
typedef u32(__stdcall *tsfifo_receive_lin_msgs_t)(const size_t ADeviceHandle, const TLibLIN *ALINBuffers, s32 *ALINBufferSize, u8 AChn, u8 ARXTX);

// 读取LIN报文
// ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ALINBufferSize：存储数组的长度
// 返回值：实际收到的报文数量
typedef u32(__stdcall *tsfifo_receive_fastlin_msgs_t)(const size_t ADeviceHandle, const TLibLIN *ALINBuffers, s32 *ALINBufferSize, u8 AChn, u8 ARXTX);

// 获取错误编码代表的意义
typedef u32(__stdcall *tscan_get_error_description_t)(const u32 ACode, char **ADesc);

// 高精度回放API
typedef s32(__stdcall *tsreplay_add_channel_map_t)(const size_t ADeviceHandle, TS_APP_CHANNEL ALogicChannel, TS_APP_CHANNEL AHardwareChannel);
typedef void(__stdcall *tsreplay_clear_channel_map_t)(const size_t ADeviceHandle);
typedef s32(__stdcall *tsreplay_start_blf_t)(const size_t ADeviceHandle, char *ABlfFilePath, int ATriggerByHardware, u64 AStartUs, u64 AEndUs);
typedef s32(__stdcall *tsreplay_stop_t)(const size_t ADeviceHandle);

typedef s32(__stdcall *tsdiag_can_create_t)(int *pDiagModuleIndex,
                                            u32  AChnIndex,
                                            byte ASupportFDCAN,
                                            byte AMaxDLC,
                                            u32  ARequestID,
                                            bool ARequestIDIsStd,
                                            u32  AResponseID,
                                            bool AResponseIDIsStd,
                                            u32  AFunctionID,
                                            bool AFunctionIDIsStd);
typedef s32(__stdcall *tsdiag_can_delete_t)(int ADiagModuleIndex);
typedef s32(__stdcall *tsdiag_can_delete_all_t)(void);
typedef s32(__stdcall *tsdiag_can_attach_to_tscan_tool_t)(int ADiagModuleIndex, size_t ACANToolHandle);
/*TP Raw Function*/
typedef s32(__stdcall *tstp_can_send_functional_t)(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, int ATimeOutMs);
typedef s32(__stdcall *tstp_can_send_request_t)(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, int ATimeOutMs);
typedef s32(__stdcall *tstp_can_request_and_get_response_t)(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, byte *AReturnArray, int *AReturnArraySize, int ATimeOutMs);

typedef s32(__stdcall *tsdiag_can_session_control_t)(int ADiagModuleIndex, byte ASubSession, byte ATimeoutMS);
typedef s32(__stdcall *tsdiag_can_routine_control_t)(int ADiagModuleIndex, byte AARoutineControlType, u16 ARoutintID, int ATimeoutMS);
typedef s32(__stdcall *tsdiag_can_communication_control_t)(int ADiagModuleIndex, byte AControlType, int ATimeOutMs);
typedef s32(__stdcall *tsdiag_can_security_access_request_seed_t)(int ADiagModuleIndex, int ALevel, byte *ARecSeed, int *ARecSeedSize, int ATimeoutMS);
typedef s32(__stdcall *tsdiag_can_security_access_send_key_t)(int ADiagModuleIndex, int ALevel, byte *ASeed, int ASeedSize, int ATimeoutMS);
typedef s32(__stdcall *tsdiag_can_request_download_t)(int ADiagModuleIndex, u32 AMemAddr, u32 AMemSize, int ATimeoutMS);
typedef s32(__stdcall *tsdiag_can_request_upload_t)(int ADiagModuleIndex, u32 AMemAddr, u32 AMemSize, int ATimeoutMS);
typedef s32(__stdcall *tsdiag_can_transfer_data_t)(int ADiagModuleIndex, byte *ASourceDatas, int ASize, int AReqCase, int ATimeoutMS);
typedef s32(__stdcall *tsdiag_can_request_transfer_exit_t)(int ADiagModuleIndex, int ATimeoutMS);
typedef s32(__stdcall *tsdiag_can_write_data_by_identifier_t)(int ADiagModuleIndex, u16 ADataIdentifier, byte *AWriteData, int AWriteDataSize, int ATimeOutMs);
typedef s32(__stdcall *tsdiag_can_read_data_by_identifier_t)(int ADiagModuleIndex, u16 ADataIdentifier, byte *AReturnArray, int *AReturnArraySize, int ATimeOutMs);

#ifdef __cplusplus
extern "C"
{
#endif
  // TS系列工具连接回调函数
  void __stdcall TTSCANConnectedCallback(const size_t ADevicehandle);
  // TS系列工具断开回调函数
  void __stdcall TTSCANDisConnectedCallback(const size_t ADevicehandle);
  // 高精度时钟回调函数
  void __stdcall THighResTimerCallback(const u32 ADevicehandle);
  // CAN报文接收回调函数
  void __stdcall TCANQueueEvent_Win32(const TLibCAN *AData);
  // CANFD报文接收回调函数
  void __stdcall TCANFDQueueEvent_Win32(const TLibCANFD *AData);
  // LIN报文接收回调函数
  void __stdcall TLINQueueEvent_Win32(const TLibLIN *AData);

  // 回调函数注册函数
  // 注册CAN报文接收回调函数
  u32 __stdcall tscan_register_event_can(const size_t ADeviceHandle, const TCANQueueEvent_Win32_t ACallback);
  // 反注册CAN报文接收回调函数
  u32 __stdcall tscan_unregister_event_can(const size_t ADeviceHandle, const TCANQueueEvent_Win32_t ACallback);

  // 注册CAN报文接收回调函数
  u32 __stdcall tscan_register_event_canfd(const size_t ADeviceHandle, const TCANFDQueueEvent_Win32_t ACallback);
  // 反注册CAN报文接收回调函数
  u32 __stdcall tscan_unregister_event_canfd(const size_t ADeviceHandle, const TCANFDQueueEvent_Win32_t ACallback);

  // 注册LIN报文接收回调函数
  u32 __stdcall tslin_register_event_lin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
  // 反注册LIN报文接收回调函数
  u32 __stdcall tslin_unregister_event_lin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);

  // 注册FastLIN报文接收回调函数
  u32 __stdcall tscan_register_event_fastlin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
  // 反注册FastLIN报文接收回调函数
  u32 __stdcall tscan_unregister_event_fastlin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);

  // 功能函数类型
  // 扫描在线的设备
  u32 __stdcall tscan_scan_devices(u32 *ADeviceCount);
  // 连接设备，ADeviceSerial !=NULL：连接指定的设备；ADeviceSerial == NULL：连接默认设备
  u32 __stdcall tscan_connect(const char *ADeviceSerial, size_t *AHandle);
  // 断开指定设备
  u32 __stdcall tscan_disconnect_by_handle(const size_t ADeviceHandle);
  // 断开所有设备
  u32 __stdcall tscan_disconnect_all_devices(void);
  // 初始化TSCANAPI模块
  void __stdcall initialize_lib_tscan(bool AEnableFIFO, bool AEnableErrorFrame, bool AEnableTurbe);
  // 释放TSCANAPI模块
  void __stdcall finalize_lib_tscan(void);

  // CAN工具相关
  // 同步发送CAN报文
  u32 __stdcall tscan_transmit_can_sync(const size_t ADeviceHandle, const TLibCAN *ACAN, const u32 ATimeoutMS);
  // 异步发送CAN报文
  u32 __stdcall tscan_transmit_can_async(const size_t ADeviceHandle, const TLibCAN *ACAN);
  // 设置CAN报文波特率参数
  u32 __stdcall tscan_config_can_by_baudrate(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double ARateKbps, const u32 A120OhmConnected);

  // CAN周期函数
  // 添加周期发送CAN报文
  u32 __stdcall tscan_add_cyclic_msg_can(const size_t ADeviceHandle, const TLibCAN *ACAN, const float APeriodMS);   // float is single
  // 去除周期发送CAN报文
  u32 __stdcall tscan_delete_cyclic_msg_can(const size_t ADeviceHandle, const TLibCAN *ACAN);
  // 添加周期发送CANFD报文
  u32 __stdcall tscan_add_cyclic_msg_canfd(const size_t ADeviceHandle, const TLibCANFD *ACANFD, const float APeriodMS);   // single
  // 去除周期发送CANFD报文
  u32 __stdcall tscan_delete_cyclic_msg_canfd(const size_t ADeviceHandle, const TLibCANFD *ACANFD);

  // 读取CAN报文
  // ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ACANBufferSize：存储数组的长度
  // 返回值：实际收到的报文数量
  u32 __stdcall tsfifo_receive_can_msgs(const size_t ADeviceHandle, const TLibCAN *ACANBuffers, s32 *ACANBufferSize, u8 AChn, u8 ARXTX);

  // CANFD工具相关
  // 同步发送CANFD报文
  u32 __stdcall tscan_transmit_canfd_sync(const size_t ADeviceHandle, const TLibCANFD *ACAN, const u32 ATimeoutMS);
  // 异步发送CANFD报文
  u32 __stdcall tscan_transmit_canfd_async(const size_t ADeviceHandle, const TLibCANFD *ACAN);
  // 设置CANFD报文波特率参数
  u32 __stdcall tscan_config_canfd_by_baudrate(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double AArbRateKbps, const double ADataRateKbps, const TLIBCANFDControllerType AControllerType, const TLIBCANFDControllerMode AControllerMode, const u32 A120OhmConnected);
  // 读取CANFD报文
  // ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ACANBufferSize：存储数组的长度
  // 返回值：实际收到的报文数量
  u32 __stdcall tsfifo_receive_canfd_msgs(const size_t ADeviceHandle, const TLibCANFD *ACANBuffers, s32 *ACANBufferSize, u8 AChn, u8 ARXTX);

  // LIN工具相关
  // 设置节点类型:ADeviceHandle:句柄；AChnIdx:通道号;0:MasterNode;1:SlaveNode;2:MonitorNode
  u32 __stdcall tslin_set_node_funtiontype(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const u8 AFunctionType);
  // 请求下载新的ldf文件：该命令会清除设备中现存的所有ldf文件
  u32 __stdcall tslin_apply_download_new_ldf(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx);
  // 同步发送LIN报文
  u32 __stdcall tslin_transmit_lin_sync(const size_t ADeviceHandle, const TLibLIN *ALIN, const u32 ATimeoutMS);
  // 异步发送LIN报文
  u32 __stdcall tslin_transmit_lin_async(const size_t ADeviceHandle, const TLibLIN *ALIN);
  // 异步发送LIN报文
  u32 __stdcall tslin_transmit_fastlin_async(const size_t ADeviceHandle, const TLibLIN *ALIN);
  // 设置LIN报文波特率参数
  u32 __stdcall tslin_config_baudrate(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double ARateKbps, TLINProtocol AProtocol);

  // 读取LIN报文
  // ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ALINBufferSize：存储数组的长度
  // 返回值：实际收到的报文数量
  u32 __stdcall tsfifo_receive_lin_msgs(const size_t ADeviceHandle, const TLibLIN *ALINBuffers, s32 *ALINBufferSize, u8 AChn, u8 ARXTX);

  // 读取LIN报文
  // ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ALINBufferSize：存储数组的长度
  // 返回值：实际收到的报文数量
  u32 __stdcall tsfifo_receive_fastlin_msgs(const size_t ADeviceHandle, const TLibLIN *ALINBuffers, s32 *ALINBufferSize, u8 AChn, u8 ARXTX);

  // 获取错误编码代表的意义
  u32 __stdcall tscan_get_error_description(const u32 ACode, char **ADesc);

  // 高精度回放API
  s32 __stdcall tsreplay_add_channel_map(const size_t ADeviceHandle, TS_APP_CHANNEL ALogicChannel, TS_APP_CHANNEL AHardwareChannel);
  void __stdcall tsreplay_clear_channel_map(const size_t ADeviceHandle);
  s32 __stdcall tsreplay_start_blf(const size_t ADeviceHandle, char *ABlfFilePath, int ATriggerByHardware, u64 AStartUs, u64 AEndUs);
  s32 __stdcall tsreplay_stop(const size_t ADeviceHandle);

  s32 __stdcall tsdiag_can_create(int *pDiagModuleIndex,
                                              u32  AChnIndex,
                                              byte ASupportFDCAN,
                                              byte AMaxDLC,
                                              u32  ARequestID,
                                              bool ARequestIDIsStd,
                                              u32  AResponseID,
                                              bool AResponseIDIsStd,
                                              u32  AFunctionID,
                                              bool AFunctionIDIsStd);
  s32 __stdcall tsdiag_can_delete(int ADiagModuleIndex);
  s32 __stdcall tsdiag_can_delete_all(void);
  s32 __stdcall tsdiag_can_attach_to_tscan_tool(int ADiagModuleIndex, size_t ACANToolHandle);
  /*TP Raw Function*/
  s32 __stdcall tstp_can_send_functional(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, int ATimeOutMs);
  s32 __stdcall tstp_can_send_request(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, int ATimeOutMs);
  s32 __stdcall tstp_can_request_and_get_response(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, byte *AReturnArray, int *AReturnArraySize, int ATimeOutMs);

  s32 __stdcall tsdiag_can_session_control(int ADiagModuleIndex, byte ASubSession, byte ATimeoutMS);
  s32 __stdcall tsdiag_can_routine_control(int ADiagModuleIndex, byte AARoutineControlType, u16 ARoutintID, int ATimeoutMS);
  s32 __stdcall tsdiag_can_communication_control(int ADiagModuleIndex, byte AControlType, int ATimeOutMs);
  s32 __stdcall tsdiag_can_security_access_request_seed(int ADiagModuleIndex, int ALevel, byte *ARecSeed, int *ARecSeedSize, int ATimeoutMS);
  s32 __stdcall tsdiag_can_security_access_send_key(int ADiagModuleIndex, int ALevel, byte *ASeed, int ASeedSize, int ATimeoutMS);
  s32 __stdcall tsdiag_can_request_download(int ADiagModuleIndex, u32 AMemAddr, u32 AMemSize, int ATimeoutMS);
  s32 __stdcall tsdiag_can_request_upload(int ADiagModuleIndex, u32 AMemAddr, u32 AMemSize, int ATimeoutMS);
  s32 __stdcall tsdiag_can_transfer_data(int ADiagModuleIndex, byte *ASourceDatas, int ASize, int AReqCase, int ATimeoutMS);
  s32 __stdcall tsdiag_can_request_transfer_exit(int ADiagModuleIndex, int ATimeoutMS);
  s32 __stdcall tsdiag_can_write_data_by_identifier(int ADiagModuleIndex, u16 ADataIdentifier, byte *AWriteData, int AWriteDataSize, int ATimeOutMs);
  s32 __stdcall tsdiag_can_read_data_by_identifier(int ADiagModuleIndex, u16 ADataIdentifier, byte *AReturnArray, int *AReturnArraySize, int ATimeOutMs);

#ifdef __cplusplus
}
#endif
#endif
