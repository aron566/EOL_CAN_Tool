#ifndef _TSCANLIN_API_H_
#define _TSCANLIN_API_H_

#include "TSCANDef.h"
#include <QLibrary>
#include <QDebug>

class TSCANLINApi
{
private:
  QLibrary myLibrary;
  QLibrary ts_myLibrary;
  quint8 dllIsLoaded;

  // function Pointers
  initialize_lib_tscan_t           p_initialize_lib_tscan;
  finalize_lib_tscan_t             p_finalize_lib_tscan;
  tscan_register_event_can_t       p_tscan_register_event_can;
  tscan_unregister_event_can_t     p_tscan_unregister_event_can;
  tscan_register_event_canfd_t     p_tscan_register_event_canfd;
  tscan_unregister_event_canfd_t   p_tscan_unregister_event_canfd;
  tslin_register_event_lin_t       p_tslin_register_event_lin;
  tslin_unregister_event_lin_t     p_tslin_unregister_event_lin;
  tscan_register_event_fastlin_t   p_tscan_register_event_fastlin;
  tscan_unregister_event_fastlin_t p_tscan_unregister_event_fastlin;
  tscan_scan_devices_t             p_tscan_scan_devices;
  tscan_connect_t                  p_tscan_connect;
  tscan_disconnect_by_handle_t     p_tscan_disconnect_by_handle;
  tscan_disconnect_all_devices_t   p_tscan_disconnect_all_devices;
  // CAN Bus
  tscan_transmit_can_sync_t      p_tscan_transmit_can_sync;
  tscan_transmit_can_async_t     p_tscan_transmit_can_async;
  tscan_config_can_by_baudrate_t p_tscan_config_can_by_baudrate;
  tsfifo_receive_can_msgs_t      p_tsfifo_receive_can_msgs;
  // CANFD Bus
  tscan_transmit_canfd_sync_t      p_tscan_transmit_canfd_sync;
  tscan_transmit_canfd_async_t     p_tscan_transmit_canfd_async;
  tscan_config_canfd_by_baudrate_t p_tscan_config_canfd_by_baudrate;
  tsfifo_receive_canfd_msgs_t      p_tsfifo_receive_canfd_msgs;
  // cyclic message
  tscan_add_cyclic_msg_can_t p_tscan_add_cyclic_msg_can;
  // 去除周期发送CAN报文
  tscan_delete_cyclic_msg_can_t p_tscan_delete_cyclic_msg_can;
  // 添加周期发送CANFD报文
  tscan_add_cyclic_msg_canfd_t p_tscan_add_cyclic_msg_canfd;
  // 去除周期发送CANFD报文
  tscan_delete_cyclic_msg_canfd_t p_tscan_delete_cyclic_msg_canfd;
  // LIN Bus
  tslin_set_node_funtiontype_t   p_tslin_set_node_funtiontype;
  tslin_apply_download_new_ldf_t p_tslin_apply_download_new_ldf;
  tslin_transmit_lin_sync_t      p_tslin_transmit_lin_sync;
  tslin_transmit_lin_async_t     p_tslin_transmit_lin_async;
  tslin_transmit_fastlin_async_t p_tslin_transmit_fastlin_async;
  tslin_config_baudrate_t        p_tslin_config_baudrate;
  tsfifo_receive_lin_msgs_t      p_tsfifo_receive_lin_msgs;
  tsfifo_receive_fastlin_msgs_t  p_tsfifo_receive_fastlin_msgs;
  // Precise Replay
  tsreplay_add_channel_map_t   pReplay_RegisterMapChannel;
  tsreplay_clear_channel_map_t pReplay_ClearMapChannel;
  tsreplay_start_blf_t         pReplay_Start_Blf;
  tsreplay_stop_t              pReplay_Stop_Blf;
  // System
  tscan_get_error_description_t p_tscan_get_error_description;
  // Diagnostic
  tsdiag_can_create_t                       p_tsdiag_can_create;
  tsdiag_can_delete_t                       p_tsdiag_can_delete;
  tsdiag_can_delete_all_t                   p_tsdiag_can_delete_all;
  tsdiag_can_attach_to_tscan_tool_t         p_tsdiag_can_attach_to_tscan_tool;
  tstp_can_send_functional_t                p_tstp_can_send_functional;
  tstp_can_send_request_t                   p_tstp_can_send_request;
  tstp_can_request_and_get_response_t       p_tstp_can_request_and_get_response;
  tsdiag_can_session_control_t              p_tsdiag_can_session_control;
  tsdiag_can_routine_control_t              p_tsdiag_can_routine_control;
  tsdiag_can_communication_control_t        p_tsdiag_can_communication_control;
  tsdiag_can_security_access_request_seed_t p_tsdiag_can_security_access_request_seed;
  tsdiag_can_security_access_send_key_t     p_tsdiag_can_security_access_send_key;
  tsdiag_can_request_download_t             p_tsdiag_can_request_download;
  tsdiag_can_request_upload_t               p_tsdiag_can_request_upload;
  tsdiag_can_transfer_data_t                p_tsdiag_can_transfer_data;
  tsdiag_can_request_transfer_exit_t        p_tsdiag_can_request_transfer_exit;
  tsdiag_can_write_data_by_identifier_t     p_tsdiag_can_write_data_by_identifier;
  tsdiag_can_read_data_by_identifier_t      p_tsdiag_can_read_data_by_identifier;
  byte                                      LoadAPI();
  void                                      InitializePointers();

public:
  TSCANLINApi();
  ~TSCANLINApi();
  void     InitTSCANAPI(bool AEnableFIFO, bool AErrorFrame, bool AEnableTurbe);
  void     FreeTSCANApi(void);
  u32      tscan_register_event_can(const size_t ADeviceHandle, const TCANQueueEvent_Win32_t ACallback);
  u32      tscan_unregister_event_can(const size_t ADeviceHandle, const TCANQueueEvent_Win32_t ACallback);
  u32      tscan_register_event_canfd(const size_t ADeviceHandle, const TCANFDQueueEvent_Win32_t ACallback);
  u32      tscan_unregister_event_canfd(const size_t ADeviceHandle, const TCANFDQueueEvent_Win32_t ACallback);
  u32      tslin_register_event_lin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
  u32      tslin_unregister_event_lin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
  u32      tscan_register_event_fastlin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
  u32      tscan_unregister_event_fastlin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
  u32      tscan_scan_devices(u32 *ADeviceCount);
  u32      tscan_connect(const char *ADeviceSerial, size_t *AHandle);
  u32      tscan_disconnect_by_handle(const size_t ADeviceHandle);
  u32      tscan_disconnect_all_devices(void);
  u32      tscan_transmit_can_sync(const size_t ADeviceHandle, const TLibCAN *ACAN, const u32 ATimeoutMS);
  u32      tscan_transmit_can_async(const size_t ADeviceHandle, const TLibCAN *ACAN);
  u32      tscan_config_can_by_baudrate(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double ARateKbps, const u32 A120OhmConnected);
  u32      tsfifo_receive_can_msgs(const size_t ADeviceHandle, const TLibCAN *ACANBuffers, s32 *ACANBufferSize, u8 AChn, u8 ARxTx);
  u32      tscan_transmit_canfd_sync(const size_t ADeviceHandle, const TLibCANFD *ACAN, const u32 ATimeoutMS);
  u32      tscan_transmit_canfd_async(const size_t ADeviceHandle, const TLibCANFD *ACAN);
  // cyclic message
  u32  tscan_add_cyclic_msg_can(const size_t ADeviceHandle, const TLibCAN *ACAN, const float APeriodMS);         // float is single
  u32  tscan_delete_cyclic_msg_can(const size_t ADeviceHandle, const TLibCAN *ACAN);                             // 去除周期发送CAN报文
  u32  tscan_add_cyclic_msg_canfd(const size_t ADeviceHandle, const TLibCANFD *ACANFD, const float APeriodMS);   // 添加周期发送CANFD报文
  u32  tscan_delete_cyclic_msg_canfd(const size_t ADeviceHandle, const TLibCANFD *ACANFD);                       // 去除周期发送CANFD报文
  u32  tscan_config_canfd_by_baudrate(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double AArbRateKbps, const double ADataRateKbps, const TLIBCANFDControllerType AControllerType, const TLIBCANFDControllerMode AControllerMode, const u32 A120OhmConnected);
  u32  tsfifo_receive_canfd_msgs(const size_t ADeviceHandle, const TLibCANFD *ACANBuffers, s32 *ACANBufferSize, u8 AChn, u8 ARxTx);
  u32  tslin_set_node_funtiontype(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const u8 AFunctionType);
  u32  tslin_apply_download_new_ldf(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx);
  u32  tslin_transmit_lin_sync(const size_t ADeviceHandle, const TLibLIN *ALIN, const u32 ATimeoutMS);
  u32  tslin_transmit_lin_async(const size_t ADeviceHandle, const TLibLIN *ALIN);
  u32  tslin_transmit_fastlin_async(const size_t ADeviceHandle, const TLibLIN *ALIN);
  u32  tslin_config_baudrate(const size_t ADeviceHandle, const TS_APP_CHANNEL AChnIdx, const double ARateKbps);
  u32  tsfifo_receive_lin_msgs(const size_t ADeviceHandle, const TLibLIN *ALINBuffers, s32 *ALINBufferSize, u8 AChn, u8 ARxTx);
  u32  tsfifo_receive_fastlin_msgs(const size_t ADeviceHandle, const TLibLIN *ALINBuffers, s32 *ALINBufferSize, u8 AChn, u8 ARxTx);
  u32  tscan_get_error_description(const u32 ACode, char **ADesc);
  u32  Replay_RegisterReplayMapChannel(const size_t ADeviceHandle, TS_APP_CHANNEL ALogicChannel, TS_APP_CHANNEL AHardwareChannel);
  void Replay_ClearReplayMapChannel(const size_t ADeviceHandle);
  u32  Replay_Start_Blf(const size_t ADeviceHandle, char *ABlfFilePath, int ATriggerByHardware, u64 AStartUs, u64 AEndUs);
  u32  Replay_Stop(const size_t ADeviceHandle);
  // Diagnostics
  s32 tsdiag_can_create(int *pDiagModuleIndex,
                        u32  AChnIndex,
                        byte ASupportFDCAN,
                        byte AMaxDLC,
                        u32  ARequestID,
                        bool ARequestIDIsStd,
                        u32  AResponseID,
                        bool AResponseIDIsStd,
                        u32  AFunctionID,
                        bool AFunctionIDIsStd);
  s32 tsdiag_can_delete(int ADiagModuleIndex);
  s32 tsdiag_can_delete_all(void);
  s32 tsdiag_can_attach_to_tscan_tool(int ADiagModuleIndex, size_t ACANToolHandle);
  /*TP Raw Function*/
  s32 tstp_can_send_functional(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, int ATimeOutMs);
  s32 tstp_can_send_request(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, int ATimeOutMs);
  s32 tstp_can_request_and_response(int ADiagModuleIndex, byte *AReqArray, int AReqArraySize, byte *AReturnArray, int *AReturnArraySize, int ATimeOutMs);

  s32 tsdiag_can_session_control(int ADiagModuleIndex, byte ASubSession, byte ATimeoutMS);
  s32 tsdiag_can_routine_control(int ADiagModuleIndex, byte AARoutineControlType, u16 ARoutintID, int ATimeoutMS);
  s32 tsdiag_can_communication_control(int ADiagModuleIndex, byte AControlType, int ATimeOutMs);
  s32 tsdiag_can_security_access_request_seed(int ADiagModuleIndex, int ALevel, byte *ARecSeed, int *ARecSeedSize, int ATimeoutMS);
  s32 tsdiag_can_security_access_send_key(int ADiagModuleIndex, int ALevel, byte *ASeed, int ASeedSize, int ATimeoutMS);
  s32 tsdiag_can_request_download(int ADiagModuleIndex, u32 AMemAddr, u32 AMemSize, int ATimeoutMS);
  s32 tsdiag_can_request_upload(int ADiagModuleIndex, u32 AMemAddr, u32 AMemSize, int ATimeoutMS);
  s32 tsdiag_can_transfer_data(int ADiagModuleIndex, byte *ASourceDatas, int ASize, int AReqCase, int ATimeoutMS);
  s32 tsdiag_can_request_transfer_exit(int ADiagModuleIndex, int ATimeoutMS);
  s32 tsdiag_can_write_data_by_identifier(int ADiagModuleIndex, u16 ADataIdentifier, byte *AWriteData, int AWriteDataSize, int ATimeOutMs);
  s32 tsdiag_can_read_data_by_identifier(int ADiagModuleIndex, u16 ADataIdentifier, byte *AReturnArray, int *AReturnArraySize, int ATimeOutMs);
};

#endif
