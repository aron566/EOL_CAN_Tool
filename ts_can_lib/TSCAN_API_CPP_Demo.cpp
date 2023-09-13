#include "TSCANLINApi.h"

// #define TEST_CAN_API
#define TEST_CANFD_API
// #define TEST_LIN_API

extern void ProcessLINMsg1(TLibLIN AMsg);

/// <summary>
/// CAN报文接收回调函数
/// </summary>
/// <param name="AData">CAN报文指针</param>
/// <returns></returns>
void __stdcall ReceiveCANMessage1(const TLibCAN *AData)
{
}

/*如果注册了本函数，当驱动收到了一帧LIN报文过后，就会触发此函数*/
/// <summary>
/// LIN报文接收回调函数
/// </summary>
/// <param name="AData">LIN报文指针</param>
/// <returns></returns>
void __stdcall ReceiveLINMessage1(const TLibLIN *AData)
{
  // 通过回调函数读取数据
  printf("Receive Recall\n");
  ProcessLINMsg1(*AData);
}

/*如果注册了本函数，当驱动收到了一帧LIN报文过后，就会触发此函数*/
/// <summary>
/// LIN报文接收回调函数
/// </summary>
/// <param name="AData">LIN报文指针</param>
/// <returns></returns>
void __stdcall ReceiveFastLINMessage1(const TLibLIN *AData)
{
  // 通过回调函数读取数据
  printf("Receive FastLIN Recall\n");
  ProcessLINMsg1(*AData);
}

/// <summary>
/// 处理收到的LIN报文数据
/// </summary>
/// <param name="AMsg">LIN报文</param>
/// <returns></returns>
void ProcessLINMsg1(TLibLIN AMsg)
{
  if (AMsg.FProperties.bits.istx)
  {
    printf("Translate message:");
  }
  else
  {
    printf("Receive message:");
  }
  printf("ID:%x ", AMsg.FIdentifier);
  printf("Datalength:%d ", AMsg.FDLC);
  printf("Datas:");
  for (int i = 0; i < AMsg.FDLC; i++)
  {
    printf(" %x", AMsg.FData.d[i]);
  }
  printf("\n");
}

/// <summary>
/// 处理收到的LIN报文数据
/// </summary>
/// <param name="AMsg">LIN报文</param>
/// <returns></returns>
void ProcessCANMsg1(TLibCAN AMsg)
{
  if (AMsg.FProperties.bits.istx)
  {
    printf("Translate message:");
  }
  else
  {
    printf("Receive message:");
  }
  printf("ID:%x ", AMsg.FIdentifier);
  printf("Datalength:%d ", AMsg.FDLC);
  printf("Datas:");
  for (int i = 0; i < AMsg.FDLC; i++)
  {
    printf(" %x", AMsg.FData.d[i]);
  }
  printf("\n");
}

void TestLINAPI(TSCANLINApi *tsCANLINAPIObj, size_t ADeviceHandle)
{
  s32 retValue;
  /*定义CAN发送报文，并填充数据*/
  TLibLIN msg;
  // 把当前设备设置为主节点模式：主节点模式下，才能够主动调度报文，发送报文头等
  if (tsCANLINAPIObj->tslin_set_node_funtiontype(ADeviceHandle, TS_CHN0, (u8)MasterNode) == 0)
  {
    printf("LIN Device with handle:%zu set function type success\n", ADeviceHandle);
  }
  else
  {
    printf("LIN Device with handle:%zu set function type failed\n", ADeviceHandle);
  }
  // 清除内部ldf文件
  if (tsCANLINAPIObj->tslin_apply_download_new_ldf(ADeviceHandle, TS_CHN0) == 0)
  {
    printf("LIN Device with handle:%zu apply new ldf callback success\n", ADeviceHandle);
  }
  else
  {
    printf("LIN Device with handle:%zu apply new ldf callback failed\n", ADeviceHandle);
  }
  // 注册普通LIN接收回调函数：类似于接收中断
  if (tsCANLINAPIObj->tslin_register_event_lin(ADeviceHandle, ReceiveLINMessage1) == 0)
  {
    printf("LIN Device with handle:%zu register rev callback success\n", ADeviceHandle);
  }
  else
  {
    printf("LIN Device with handle:%zu register rev callback failed\n", ADeviceHandle);
  }
  // 注册FalstLIN接收回调函数：类似于接收中断
  if (tsCANLINAPIObj->tscan_register_event_fastlin(ADeviceHandle, ReceiveFastLINMessage1) == 0)
  {
    printf("LIN Device with handle:%zu register rev callback success\n", ADeviceHandle);
  }
  else
  {
    printf("LIN Device with handle:%zu register rev callback failed\n", ADeviceHandle);
  }
  // 设置波特率
  if (tsCANLINAPIObj->tslin_config_baudrate(ADeviceHandle, TS_CHN0, 100) == 0)
  {
    printf("LIN Device with handle:%zu set baudrate 100k success\n", ADeviceHandle);
  }
  else
  {
    printf("LIN Device with handle:%zu set baudrate 100k failed\n", ADeviceHandle);
  }
  // 同步函数发送LIN报文
  msg.FIdentifier           = 0x3C;
  msg.FDLC                  = 3;
  msg.FIdxChn               = 0;
  msg.FProperties.value     = 0x00;
  msg.FProperties.bits.istx = 1;
  msg.FData.d[0]            = (byte)0x12;
  msg.FData.d[1]            = (byte)0x34;
  msg.FData.d[2]            = (byte)0x56;
  // 普通LIN发送函数
  if (tsCANLINAPIObj->tslin_transmit_lin_sync(ADeviceHandle, &msg, 500) == 0)
  {
    printf("LIN Device with handle:%zu sync send lin message success\n", ADeviceHandle);
  }
  else
  {
    printf("LIN Device with handle:%zu sync send lin message failed\n", ADeviceHandle);
  }
  // LIN Receive Function
  Sleep(3);
  // 普通LIN读取数据API
  TLibLIN recMessageBuffs[5];
  int     cnt = 0;
  retValue    = 5;
  tsCANLINAPIObj->tsfifo_receive_lin_msgs(ADeviceHandle, recMessageBuffs, &retValue, TS_CHN0, TX_RX_MESSAGES);
  printf("%d  messages received\n", retValue);
  for (int i = 0; i < (int)retValue; i++)
  {
    ProcessLINMsg1(recMessageBuffs[i]);
  }
  // 注意：对于LIN总线来说，如果要接收报文，也要调用报文发送函数sendMsgAsync，把istx设置为0，实际上
  // 是把报文帧头发送出去并读取总线上的报文
  msg.FIdentifier           = 0x31;
  msg.FProperties.value     = 0x00;
  msg.FProperties.bits.istx = 0;   // as rx frame
  msg.FDLC                  = 3;
  msg.FIdxChn               = 0;
  if (tsCANLINAPIObj->tslin_transmit_lin_async(ADeviceHandle, &msg) == 0)
  {
    printf("LIN Device with handle:%zu async send lin message success\n", ADeviceHandle);
  }
  else
  {
    printf("LIN Device with handle:%zu async send lin message failed\n", ADeviceHandle);
  }
  // 对于接收类型的报文，如果从节点没有及时回复，则retValue =0,接收失败
  Sleep(10);
  // 采用延时等待的方式，直到读取到LIN报文或者超时
  cnt = 0;
  while ((retValue == 0) && (cnt < 100))
  {
    retValue = 5;   // Set Receive Buffer Size
    if (tsCANLINAPIObj->tsfifo_receive_lin_msgs(ADeviceHandle, recMessageBuffs, &retValue, TS_CHN0, TX_RX_MESSAGES) == 0x00)
    {
      cnt++;
      Sleep(10);
    }
  }
  // 如果超时都还收不到数据，则接收数据失败
  printf("%d  messages received\n", retValue);
  for (int i = 0; i < (int)retValue; i++)
  {
    ProcessLINMsg1(recMessageBuffs[i]);
  }
  msg.FIdentifier = 0x32;
  if (tsCANLINAPIObj->tslin_transmit_lin_async(ADeviceHandle, &msg) == 0)
  {
    printf("LIN Device with handle:%zu async fast lin send lin message success\n", ADeviceHandle);
  }
  else
  {
    printf("LIN Device with handle:%zu async fast lin send lin message failed\n", ADeviceHandle);
  }
  // FastLIN接收函数
  Sleep(10);
  retValue = 0;
  cnt      = 0;
  // 采用延时等待的方式，直到读取到LIN报文或者超时
  while ((retValue == 0) && (cnt < 100))
  {
    retValue = 5;   // set receive buffer size
    if (tsCANLINAPIObj->tsfifo_receive_fastlin_msgs(ADeviceHandle, recMessageBuffs, &retValue, TS_CHN0, TX_RX_MESSAGES) == 0x00)
    {
      cnt++;
      Sleep(10);
    }
  }
  // 如果超时都还收不到数据，则接收数据失败
  printf("%d  messages received\n", retValue);
  for (int i = 0; i < (int)retValue; i++)
  {
    ProcessLINMsg1(recMessageBuffs[i]);
  }
  // 反注册接收回调函数
  if (tsCANLINAPIObj->tslin_unregister_event_lin(ADeviceHandle, ReceiveLINMessage1) == 0)
  {
    printf("LIN Device with handle:%zu unregister rev callback success\n", ADeviceHandle);
  }
  else
  {
    printf("LIN Device with handle:%zu unregister rev callback failed\n", ADeviceHandle);
  }
  // 反注册FastLIN接收回调函数
  if (tsCANLINAPIObj->tscan_unregister_event_fastlin(ADeviceHandle, ReceiveFastLINMessage1) == 0)
  {
    printf("LIN Device with handle:%zu unregister rev callback success\n", ADeviceHandle);
  }
  else
  {
    printf("LIN Device with handle:%zu unregister rev callback failed\n", ADeviceHandle);
  }
}

void TestCANAPI(TSCANLINApi *tsCANLINAPIObj, size_t ADeviceHandle)
{
  s32 retValue = 0;
  /*定义CAN发送报文，并填充数据*/
  TLibCAN msg;
  msg.FIdentifier                  = 0x03;
  msg.FProperties.bits.extframe    = 0;
  msg.FProperties.bits.remoteframe = 0x00;   // not remote frame,standard frame
  msg.FDLC                         = 3;
  msg.FIdxChn                      = 0;
  // 注册接收回调函数：类似于接收中断
  if (tsCANLINAPIObj->tscan_register_event_can(ADeviceHandle, ReceiveCANMessage1) == 0)
  {
    printf("CAN Device with handle:%zu register rev callback success\n", ADeviceHandle);
  }
  else
  {
    printf("CAN Device with handle:%zu register rev callback failed\n", ADeviceHandle);
  }
  // 设置波特率
  if (tsCANLINAPIObj->tscan_config_can_by_baudrate(ADeviceHandle, TS_CHN0, 500, 1) == 0)
  {
    printf("CAN Device with handle:%zu set baudrate 500k success\n", ADeviceHandle);
  }
  else
  {
    printf("CAN Device with handle:%zu set baudrate 500k failed\n", ADeviceHandle);
  }
  // 同步函数发送CAN报文
  if (tsCANLINAPIObj->tscan_transmit_can_sync(ADeviceHandle, &msg, 500) == 0)
  {
    printf("CAN Device with handle:%zu sync send can message success\n", ADeviceHandle);
  }
  else
  {
    printf("CAN Device with handle:%zu sync send can message failed\n", ADeviceHandle);
  }
  // 异步函数发送CAN报文：
  if (tsCANLINAPIObj->tscan_transmit_can_async(ADeviceHandle, &msg) == 0)
  {
    printf("CAN Device with handle:%zu async send can message success\n", ADeviceHandle);
  }
  else
  {
    printf("CAN Device with handle:%zu async send can message failed\n", ADeviceHandle);
  }
  // 普通CAN读取数据API
  TLibCAN recMessageBuffs[5];
  int     cnt = 0;
  retValue    = 5;
  if (tsCANLINAPIObj->tsfifo_receive_can_msgs(ADeviceHandle, recMessageBuffs, &retValue, TS_CHN0, TX_RX_MESSAGES) == 0x00)
  {
    printf("%d  messages received\n", retValue);
    for (int i = 0; i < (int)retValue; i++)
    {
      ProcessCANMsg1(recMessageBuffs[i]);
    }
  }
  else
    printf("tsfifo_receive_can_msgs executed failed");
}

void TestCANFDAPI(TSCANLINApi *tsCANLINAPIObj, size_t ADeviceHandle)
{
  s32 retValue = 0;
  /*定义CAN发送报文，并填充数据*/
  // 注册接收回调函数：类似于接收中断
  if (tsCANLINAPIObj->tscan_register_event_can(ADeviceHandle, ReceiveCANMessage1) == 0)
  {
    printf("CAN Device with handle:%zu register rev callback success\n", ADeviceHandle);
  }
  else
  {
    printf("CAN Device with handle:%zu register rev callback failed\n", ADeviceHandle);
  }
  // 设置通讯波特率相关参数：
  // 设置通道1，仲裁场波特率为500，数据长波特率：2000（当经典CAN模式下，数据场波特率无意义）,lfdtCAN:经典CAN模式，普通工作模式，使能内部120Ω终端电阻
  int ret = tsCANLINAPIObj->tscan_config_canfd_by_baudrate(ADeviceHandle, TS_CHN0, 1000, 2000, lfdtCAN, lfdmNormal, 1);
  if (ret == 0)
  {
    printf("CANFD Device with handle:%zu set baudrate 1000k , 2000k success\n", ADeviceHandle);
  }
  else
  {
    printf("CANFD Device with handle:%zu set baudrate 1000k,2000k failed\n", ADeviceHandle);
  }
  // 同步函数发送CAN报文
  // FD Message Send
  TLibCAN canMsg;
  canMsg.FIdentifier                  = 0x03;
  canMsg.FProperties.bits.extframe    = 0;
  canMsg.FProperties.bits.remoteframe = 0x00;   // not remote frame,standard frame
  canMsg.FData.d[0]                   = 0x01;
  canMsg.FData.d[1]                   = 0x02;
  canMsg.FData.d[2]                   = 0x03;
  canMsg.FDLC                         = 3;
  canMsg.FIdxChn                      = TS_CHN0;
  // CAN Message Send
  if (tsCANLINAPIObj->tscan_transmit_can_sync(ADeviceHandle, &canMsg, 500) == 0)
  {
    printf("CAN Device with handle:%zu sync send can message success\n", ADeviceHandle);
  }
  else
  {
    printf("CAN Device with handle:%zu sync send can message failed\n", ADeviceHandle);
  }
  // 异步函数发送CAN报文：
  if (tsCANLINAPIObj->tscan_transmit_can_async(ADeviceHandle, &canMsg) == 0)
  {
    printf("CAN Device with handle:%zu async send can message success\n", ADeviceHandle);
  }
  else
  {
    printf("CAN Device with handle:%zu async send can message failed\n", ADeviceHandle);
  }
  canMsg.FIdentifier = 0x55;
  if (tsCANLINAPIObj->tscan_add_cyclic_msg_can(ADeviceHandle, &canMsg, 500) == 0)
  {
    printf("CAN Device with handle:%zu start send cyclic can message success\n", ADeviceHandle);
  }
  else
  {
    printf("CAN Device with handle:%zu async send cyclic can message failed\n", ADeviceHandle);
  }
  Sleep(3000);
  if (tsCANLINAPIObj->tscan_delete_cyclic_msg_can(ADeviceHandle, &canMsg) == 0)
  {
    printf("CAN Device with handle:%zu stop send cyclic can message success\n", ADeviceHandle);
  }
  else
  {
    printf("CAN Device with handle:%zu async stop send cyclic can message failed\n", ADeviceHandle);
  }
  // FD Message Send
  TLibCANFD CANFDMsg;
  CANFDMsg.FIdentifier                  = 0x03;
  CANFDMsg.FProperties.bits.extframe    = 0;
  CANFDMsg.FProperties.bits.remoteframe = 0x00;   // not remote frame,standard frame
  CANFDMsg.FDLC                         = 3;
  CANFDMsg.FIdxChn                      = TS_CHN0;
  // 同步函数发送CANFD报文
  if (tsCANLINAPIObj->tscan_transmit_canfd_sync(ADeviceHandle, &CANFDMsg, 500) == 0)
  {
    printf("CAN Device with handle:%zu sync send can message success\n", ADeviceHandle);
  }
  else
  {
    printf("CAN Device with handle:%zu sync send can message failed\n", ADeviceHandle);
  }
  // 异步函数发送CANFD报文：
  if (tsCANLINAPIObj->tscan_transmit_canfd_async(ADeviceHandle, &CANFDMsg) == 0)
  {
    printf("CAN Device with handle:%zu async send can message success\n", ADeviceHandle);
  }
  else
  {
    printf("CAN Device with handle:%zu async send can message failed\n", ADeviceHandle);
  }
  // 读取CAN报文
  TLibCAN recMessageCANBuffs[5];
  // TX_RX_DATA 发送出去的数据和接收的数据都读取进来，
  retValue = 5;   // set receive buffer size
  if (tsCANLINAPIObj->tsfifo_receive_can_msgs(ADeviceHandle, recMessageCANBuffs, &retValue, TS_CHN0, TX_RX_MESSAGES) == 0x00)
  {
    printf("%d CAN messages received\n", retValue);
    for (int i = 0; i < (int)retValue; i++)
    {
      ProcessCANMsg1(recMessageCANBuffs[i]);
    }
  }
  else
  {
    printf("tsfifo_receive_can_msgs executed  failed");
  }
  // 读取CANFD报文
  TLibCANFD recMessageCANFDBuffs[5];
  retValue = 5;
  if (tsCANLINAPIObj->tsfifo_receive_canfd_msgs(ADeviceHandle, recMessageCANFDBuffs, &retValue, TS_CHN0, TX_RX_MESSAGES) == 0x00)
  {
    printf("%d CANFD messages received\n", retValue);
    for (int i = 0; i < (int)retValue; i++)
    {
      // ProcessCANFDMsg1(recMessageCANFDBuffs[i]);
    }
  }
  else
  {
    printf("tsfifo_receive_canfd_msgs executed failed");
  }
}

void TSCANLINApi_CPP_Demo()
{
  TSCANLINApi *tsCANLINAPIObj = new TSCANLINApi();
  uint32_t     ADeviceCount;
  size_t       ADeviceHandle;
  uint32_t     retValue;

  printf("TOSUN TSCANLINAPI CPP Demo\n");
  // 扫描存在的设备：不是必须调用的
  if (tsCANLINAPIObj->tscan_scan_devices(&ADeviceCount) == 0)
  {
    printf("TSCAN Device Count:%d\n", ADeviceCount);
  }
  // 连接设备：使用设备前必须调用
  retValue = tsCANLINAPIObj->tscan_connect(0, &ADeviceHandle);
  if ((retValue == 0) || (retValue == 5))
  {
    printf("TSCANLIN Device with handle:%zu connectted\n", ADeviceHandle);
  }
#ifdef TEST_CAN_API
  TestCANAPI(tsCANLINAPIObj, ADeviceHandle);
#endif
#ifdef TEST_CANFD_API
  TestCANFDAPI(tsCANLINAPIObj, ADeviceHandle);
#endif
#ifdef TEST_LIN_API
  TestLINAPI(tsCANLINAPIObj, ADeviceHandle);
#endif
  // Sample: Precise Replay
  // First Set Mapping
#ifdef TEST_REPLAY
  tsCANLINAPIObj->Replay_RegisterReplayMapChannel(ADeviceHandle, CHN2, TS_CHN0);   // Mapping channel2 data to channel 1 of ADeviceHandle
  tsCANLINAPIObj->Replay_RegisterReplayMapChannel(ADeviceHandle, CHN2, CHN3);   // Mapping channel2 data to channel 3 of ADeviceHandle
  tsCANLINAPIObj->Replay_Start_Blf(ADeviceHandle, (char *)"a.blf", 0, 0, 0xFFFFFFFF);
#endif
  Sleep(3000);
  if (tsCANLINAPIObj->tscan_disconnect_by_handle(ADeviceHandle) == 0)
  {
    printf("Disconnect device with handle:%zu success\n", ADeviceHandle);
  }
  tsCANLINAPIObj->FreeTSCANApi();
  // 释放API库
  free(tsCANLINAPIObj);
  // std::cout << "end dll!\n";
  printf("End C++ TSCANAPI CPP Demo\n");
}
