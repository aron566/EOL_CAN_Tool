/**
 * @file tscan_linux_types.h
 * @brief 同星 TSCAN Linux 兼容头 (仅 x86_64)
 *
 * 官方新版 TSCANDef.hpp 已跨平台 (无 windows.h, __linux__ 下回调为默认调用约定),
 * 但 TSCANLINApi.cpp 用 QLibrary 动态解析符号, 需要 tscan_*_t / tsdiag_*_t 等
 * 函数指针类型, 而新头里没有定义这些。
 *
 * 本文件: 先包含官方 TSCANDef.hpp (结构体/枚举/回调类型), 再补上从老 TSCANDef.h
 * 提取的函数指针 typedef (去掉 __stdcall, Linux 下为空)。
 * Windows 构建不受影响 (继续用 ts_can_lib/TSCANDef.h)。
 */
#ifndef TSCAN_LINUX_TYPES_H
#define TSCAN_LINUX_TYPES_H

#include "TSCANDef.hpp"

/* 老头里的 TS_APP_CHANNEL 在新官方头里改名为 APP_CHANNEL
   (枚举值相同, 都是从 0 开始的通道号); 起别名保持 TSCANLINApi.cpp 不动 */
typedef APP_CHANNEL TS_APP_CHANNEL;

/* 以下函数指针类型从 ts_can_lib/TSCANDef.h (v1.0.0.26 时期) 提取,
   仅去掉 __stdcall; 供 TSCANLINApi.cpp 经 QLibrary::resolve 做类型转换 */

// 回调函数注册函数
// 注册CAN报文接收回调函数
typedef u32(*tscan_register_event_can_t)(const size_t ADeviceHandle, const TCANQueueEvent_Win32_t ACallback);
// 反注册CAN报文接收回调函数
typedef u32(*tscan_unregister_event_can_t)(const size_t ADeviceHandle, const TCANQueueEvent_Win32_t ACallback);

// 注册CAN报文接收回调函数
typedef u32(*tscan_register_event_canfd_t)(const size_t ADeviceHandle, const TCANFDQueueEvent_Win32_t ACallback);
// 反注册CAN报文接收回调函数
typedef u32(*tscan_unregister_event_canfd_t)(const size_t ADeviceHandle, const TCANFDQueueEvent_Win32_t ACallback);

// 注册LIN报文接收回调函数
typedef u32(*tslin_register_event_lin_t)(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
// 反注册LIN报文接收回调函数
typedef u32(*tslin_unregister_event_lin_t)(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);

// 注册FastLIN报文接收回调函数
typedef u32(*tscan_register_event_fastlin_t)(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
// 反注册FastLIN报文接收回调函数
typedef u32(*tscan_unregister_event_fastlin_t)(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);

// 功能函数类型
// 扫描在线的设备
typedef uint32_t(*tscan_scan_devices_t)(uint32_t *ADeviceCount);
// 连接设备，ADeviceSerial !=NULL：连接指定的设备；ADeviceSerial == NULL：连接默认设备
typedef uint32_t(*tscan_connect_t)(const char *ADeviceSerial, size_t *AHandle);
// 断开指定设备
typedef u32(*tscan_disconnect_by_handle_t)(const size_t ADeviceHandle);
// 断开所有设备
typedef u32(*tscan_disconnect_all_devices_t)(void);
// 初始化TSCANAPI模块
typedef void(*initialize_lib_tscan_t)(bool AEnableFIFO, bool AEnableErrorFrame, bool AEnableTurbe);
// 释放TSCANAPI模块
typedef void(*finalize_lib_tscan_t)(void);

// CAN工具相关
// 同步发送CAN报文
typedef u32(*tscan_transmit_can_sync_t)(const size_t ADeviceHandle, const TLibCAN *ACAN, const u32 ATimeoutMS);
// 异步发送CAN报文
typedef u32(*tscan_transmit_can_async_t)(const size_t ADeviceHandle, const TLibCAN *ACAN);
// 设置CAN报文波特率参数
typedef u32(*tscan_config_can_by_baudrate_t)(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double ARateKbps, const u32 A120OhmConnected);

// CAN周期函数
// 添加周期发送CAN报文
typedef u32(*tscan_add_cyclic_msg_can_t)(const size_t ADeviceHandle, const TLibCAN *ACAN, const float APeriodMS);   // float is single
// 去除周期发送CAN报文
typedef u32(*tscan_delete_cyclic_msg_can_t)(const size_t ADeviceHandle, const TLibCAN *ACAN);
// 添加周期发送CANFD报文
typedef u32(*tscan_add_cyclic_msg_canfd_t)(const size_t ADeviceHandle, const TLibCANFD *ACANFD, const float APeriodMS);   // single
// 去除周期发送CANFD报文
typedef u32(*tscan_delete_cyclic_msg_canfd_t)(const size_t ADeviceHandle, const TLibCANFD *ACANFD);

// 读取CAN报文
// ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ACANBufferSize：存储数组的长度
// 返回值：0成功
typedef u32(*tsfifo_receive_can_msgs_t)(const size_t ADeviceHandle, const TLibCAN *ACANBuffers, s32 *ACANBufferSize, u8 AChn, u8 ARXTX);

// CANFD工具相关
// 同步发送CANFD报文
typedef u32(*tscan_transmit_canfd_sync_t)(const size_t ADeviceHandle, const TLibCANFD *ACAN, const u32 ATimeoutMS);
// 异步发送CANFD报文
typedef u32(*tscan_transmit_canfd_async_t)(const size_t ADeviceHandle, const TLibCANFD *ACAN);
// 设置CANFD报文波特率参数
typedef u32(*tscan_config_canfd_by_baudrate_t)(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double AArbRateKbps, const double ADataRateKbps, const TLIBCANFDControllerType AControllerType, const TLIBCANFDControllerMode AControllerMode, const u32 A120OhmConnected);
// 读取CANFD报文
// ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ACANBufferSize：存储数组的长度
// 返回值：实际收到的报文数量
typedef u32(*tsfifo_receive_canfd_msgs_t)(const size_t ADeviceHandle, const TLibCANFD *ACANBuffers, s32 *ACANBufferSize, u8 AChn, u8 ARXTX);

// LIN工具相关
// 设置节点类型:ADeviceHandle:句柄；AChnIdx:通道号;0:MasterNode;1:SlaveNode;2:MonitorNode
typedef u32(*tslin_set_node_funtiontype_t)(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const u8 AFunctionType);
// 请求下载新的ldf文件：该命令会清除设备中现存的所有ldf文件
typedef u32(*tslin_apply_download_new_ldf_t)(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx);
// 同步发送LIN报文
typedef u32(*tslin_transmit_lin_sync_t)(const size_t ADeviceHandle, const TLibLIN *ALIN, const u32 ATimeoutMS);
// 异步发送LIN报文
typedef u32(*tslin_transmit_lin_async_t)(const size_t ADeviceHandle, const TLibLIN *ALIN);
// 异步发送LIN报文
typedef u32(*tslin_transmit_fastlin_async_t)(const size_t ADeviceHandle, const TLibLIN *ALIN);
// 设置LIN报文波特率参数
typedef u32(*tslin_config_baudrate_t)(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double ARateKbps, TLINProtocol AProtocol);

// 读取LIN报文
// ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ALINBufferSize：存储数组的长度
// 返回值：实际收到的报文数量
typedef u32(*tsfifo_receive_lin_msgs_t)(const size_t ADeviceHandle, const TLibLIN *ALINBuffers, s32 *ALINBufferSize, u8 AChn, u8 ARXTX);

// 读取LIN报文
// ADeviceHandle：设备句柄；ACANBuffers:存储接收报文的数组；ALINBufferSize：存储数组的长度
// 返回值：实际收到的报文数量
typedef u32(*tsfifo_receive_fastlin_msgs_t)(const size_t ADeviceHandle, const TLibLIN *ALINBuffers, s32 *ALINBufferSize, u8 AChn, u8 ARXTX);

// 获取错误编码代表的意义
typedef u32(*tscan_get_error_description_t)(const u32 ACode, char **ADesc);

// 高精度回放API
typedef s32(*tsreplay_add_channel_map_t)(const size_t ADeviceHandle, TS_APP_CHANNEL ALogicChannel, TS_APP_CHANNEL AHardwareChannel);
typedef void(*tsreplay_clear_channel_map_t)(const size_t ADeviceHandle);
typedef s32(*tsreplay_start_blf_t)(const size_t ADeviceHandle, char *ABlfFilePath, int ATriggerByHardware, u64 AStartUs, u64 AEndUs);
typedef s32(*tsreplay_stop_t)(const size_t ADeviceHandle);

typedef s32(*tsdiag_can_create_t)(int *pDiagModuleIndex,
                                            u32  AChnIndex,
                                            byte ASupportFDCAN,
                                            byte AMaxDLC,
                                            u32  ARequestID,
                                            bool ARequestIDIsStd,
                                            u32  AResponseID,
                                            bool AResponseIDIsStd,
                                            u32  AFunctionID,
                                            bool AFunctionIDIsStd);
typedef s32(*tsdiag_can_delete_t)(int ADiagModuleIndex);
typedef s32(*tsdiag_can_delete_all_t)(void);
typedef s32(*tsdiag_can_attach_to_tscan_tool_t)(int ADiagModuleIndex, size_t ACANToolHandle);
/*TP Raw Function*/
typedef s32(*tstp_can_send_functional_t)(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, int ATimeOutMs);
typedef s32(*tstp_can_send_request_t)(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, int ATimeOutMs);
typedef s32(*tstp_can_request_and_get_response_t)(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, byte *AReturnArray, int *AReturnArraySize, int ATimeOutMs);

typedef s32(*tsdiag_can_session_control_t)(int ADiagModuleIndex, byte ASubSession, byte ATimeoutMS);
typedef s32(*tsdiag_can_routine_control_t)(int ADiagModuleIndex, byte AARoutineControlType, u16 ARoutintID, int ATimeoutMS);
typedef s32(*tsdiag_can_communication_control_t)(int ADiagModuleIndex, byte AControlType, int ATimeOutMs);
typedef s32(*tsdiag_can_security_access_request_seed_t)(int ADiagModuleIndex, int ALevel, byte *ARecSeed, int *ARecSeedSize, int ATimeoutMS);
typedef s32(*tsdiag_can_security_access_send_key_t)(int ADiagModuleIndex, int ALevel, byte *ASeed, int ASeedSize, int ATimeoutMS);
typedef s32(*tsdiag_can_request_download_t)(int ADiagModuleIndex, u32 AMemAddr, u32 AMemSize, int ATimeoutMS);
typedef s32(*tsdiag_can_request_upload_t)(int ADiagModuleIndex, u32 AMemAddr, u32 AMemSize, int ATimeoutMS);
typedef s32(*tsdiag_can_transfer_data_t)(int ADiagModuleIndex, byte *ASourceDatas, int ASize, int AReqCase, int ATimeoutMS);
typedef s32(*tsdiag_can_request_transfer_exit_t)(int ADiagModuleIndex, int ATimeoutMS);
typedef s32(*tsdiag_can_write_data_by_identifier_t)(int ADiagModuleIndex, u16 ADataIdentifier, byte *AWriteData, int AWriteDataSize, int ATimeOutMs);
typedef s32(*tsdiag_can_read_data_by_identifier_t)(int ADiagModuleIndex, u16 ADataIdentifier, byte *AReturnArray, int *AReturnArraySize, int ATimeOutMs);

#endif /* TSCAN_LINUX_TYPES_H */
