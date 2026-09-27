#ifndef __LIBTSCAN_H
#define __LIBTSCAN_H

#include <stdint.h>
#include <stddef.h>


#pragma   pack(1)
typedef uint8_t u8;
typedef uint8_t byte;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
typedef wchar_t wchar;
typedef struct _u8x8 { u8 d[8]; } u8x8;
typedef struct _u8x64 { u8 d[64]; } u8x64;

// typedef unsigned __int8 u8;
// typedef signed __int8 s8;
// typedef unsigned __int16 u16;
// typedef signed __int16 s16;
// typedef unsigned __int32 u32;
// typedef signed __int32 s32;
// typedef unsigned __int64 u64;
// typedef signed __int64 s64;
// pointer definition
// typedef unsigned __int8* pu8;
// typedef signed __int8* ps8;
// typedef unsigned __int16* pu16;
// typedef signed __int16* ps16;
// typedef unsigned __int32* pu32;
// // typedef signed __int32* ps32;
// typedef unsigned __int64* pu64;
// #define ps64 s64*
// typedef float* pfloat;
// typedef double* pdouble;


#define TSAPI(ret) __declspec(dllexport) ret 
// #ifdef DLLTEST_EXPORT
// #define TSAPI(ret) __declspec(dllexport) ret 
// #else
// #define TSAPI(ret) __declspec(dllimport) ret 
// #endif

typedef enum
{
	CHN1,
	CHN2,
	CHN3,
	CHN4,
	CHN5,
	CHN6,
	CHN7,
	CHN8,
	CHN9,
	CHN10,
	CHN11,
	CHN12,
	CHN13,
	CHN14,
	CHN15,
	CHN16,
	CHN17,
	CHN18,
	CHN19,
	CHN20,
	CHN21,
	CHN22,
	CHN23,
	CHN24,
	CHN25,
	CHN26,
	CHN27,
	CHN28,
	CHN29,
	CHN30,
	CHN31,
	CHN32
}APP_CHANNEL;


typedef enum :int
{ lfdtCAN = 0, 
  lfdtISOCAN = 1, 
  lfdtNonISOCAN = 2
}TLIBCANFDControllerType;

typedef enum:int
{
	lfdmNormal = 0, 
	lfdmACKOff = 1, 
	lfdmRestricted = 2
}TLIBCANFDControllerMode;


typedef enum:byte
{
	ONLY_RX_MESSAGES,  //ֻ��ȡ���յ�����
	TX_RX_MESSAGES     //���ͳ�ȥ�ͽ��յ����ݶ���ȡ����
}READ_TX_RX_DEF;

typedef union {
	u8 value;
	struct {
		u8 istx : 1;
		u8 remoteframe : 1;
		u8 extframe : 1;
		u8 tbd : 4;
		u8 iserrorframe : 1;
	}bits;
}TCANProperty;

//typedef struct _TCAN {
//	u8 FIdxChn;           // channel index starting from 0
//	TCANProperty FProperties;       // default 0, masked status:
//						  // [7] 0-normal frame, 1-error frame
//						  // [6-3] tbd
//						  // [2] 0-std frame, 1-extended frame
//						  // [1] 0-data frame, 1-remote frame
//						  // [0] dir: 0-RX, 1-TX
//	u8 FDLC;              // dlc from 0 to 8
//	u8 FReserved;         // reserved to keep alignment
//	s32 FIdentifier;      // CAN identifier
//	u64 FTimeUS;          // timestamp in us  //Modified by Eric 0321
//	u8x8 FData;           // 8 data bytes to send
//} TCAN;

// typedef struct _TLibCAN {
// 	u8 FIdxChn;           // channel index starting from 0
// 	TCANProperty FProperties;       // default 0, masked status:
// 						  // [7] 0-normal frame, 1-error frame
// 						  // [6-3] tbd
// 						  // [2] 0-std frame, 1-extended frame
// 						  // [1] 0-data frame, 1-remote frame
// 						  // [0] dir: 0-RX, 1-TX
// 	u8 FDLC;              // dlc from 0 to 8
// 	u8 FReserved;         // reserved to keep alignment
// 	s32 FIdentifier;      // CAN identifier
// 	u64 FTimeUS;          // timestamp in us  //Modified by Eric 0321
// 	u8 FData[8];           // 8 data bytes to send
// } TLibCAN,*PLibCAN;
// CAN definitions
#define MASK_CANProp_DIR_TX 0x01
#define MASK_CANProp_REMOTE 0x02
#define MASK_CANProp_EXTEND 0x04
#define MASK_CANProp_ERROR  0x80
#define MASK_CANProp_LOGGED 0x60
// CAN FD message properties
#define MASK_CANFDProp_IS_FD 0x01
#define MASK_CANFDProp_IS_EDL MASK_CANFDProp_IS_FD
#define MASK_CANFDProp_IS_BRS 0x02
#define MASK_CANFDProp_IS_ESI 0x04

// LIN message properties
#define MASK_LINProp_DIR_TX         0x01
#define MASK_LINProp_SEND_BREAK     0x02
#define MASK_LINProp_RECEIVED_BREAK 0x04
#define MASK_LINProp_SEND_SYNC      0x80
#define MASK_LINProp_RECEIVED_SYNC  0x10
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


typedef struct _TLibCAN {
	u8 FIdxChn;
	u8 FProperties;
	u8 FDLC;
	u8 FReserved;
	s32 FIdentifier;
	s64 FTimeUs;
	u8  FData[8];
	void LoadData(u8* a) {
		for (u32 i = 0; i < 8; i++) {
			FData[i] = *a++;
		}
	}
	void SetStdId(s32 AId, s32 ADLC) {
		FIdxChn = 0;
		FIdentifier = AId;
		FDLC = ADLC;
		FReserved = 0;
		FProperties = 0;
		SetTX(false);
		SetStd(true);
		SetData(true);
		*(u64*)(&FData[0]) = 0;
		FTimeUs = 0;
	}
	void SetExtId(s32 AId, s32 ADLC) {
		FIdxChn = 0;
		FIdentifier = AId;
		FDLC = ADLC;
		FReserved = 0;
		FProperties = 0;
		SetTX(false);
		SetStd(false);
		SetData(true);
		*(u64*)(&FData[0]) = 0;
		FTimeUs = 0;
	}
	inline bool GetTX() {
		return (FProperties & MASK_CANProp_DIR_TX) != 0;
	}
	inline bool GetData() {
		return (FProperties & MASK_CANProp_REMOTE) == 0;
	}
	inline bool GetStd() {
		return (FProperties & MASK_CANProp_EXTEND) == 0;
	}
	inline bool GetErr() {
		return (FProperties & MASK_CANProp_ERROR) != 0;
	}
	inline void SetTX(bool a) {
		if (a) {
			FProperties = FProperties | MASK_CANProp_DIR_TX;
		} else {
			FProperties = FProperties & (~MASK_CANProp_DIR_TX);
		}
	}
	inline void SetData(bool a) {
		if (a) {
			FProperties = FProperties & (~MASK_CANProp_REMOTE);
		} else {
			FProperties = FProperties | MASK_CANProp_REMOTE;
		}
	}
	inline void SetStd(bool a) {
		if (a) {
			FProperties = FProperties & (~MASK_CANProp_EXTEND);
		} else {
			FProperties = FProperties | MASK_CANProp_EXTEND;
		}
	}
	inline void SetErr(bool a) {
		if (a) {
			FProperties = FProperties & (~MASK_CANProp_ERROR);
		} else {
			FProperties = FProperties | MASK_CANProp_ERROR;
		}
	}
} TLibCAN,*PLibCAN;;

typedef union {
	u8 value;
	struct {
		u8 EDL : 1;
		u8 BRS : 1;
		u8 ESI : 1;
		u8 tbd : 5;
	}bits;
}TCANFDProperty;
// CAN FD frame definition = 80 B
  // CAN FD frame definition = 80 B
// typedef struct _TLibCANFD {
// 	u8 FIdxChn;           // channel index starting from 0        = CAN
// 	TCANProperty FProperties;       // default 0, masked status:            = CAN
// 						   // [7] 0-normal frame, 1-error frame
// 						   // [6] 0-not logged, 1-already logged
// 						   // [5-3] tbd
// 						   // [2] 0-std frame, 1-extended frame
// 						   // [1] 0-data frame, 1-remote frame
// 						   // [0] dir: 0-RX, 1-TX
// 	u8 FDLC;              // dlc from 0 to 15                     = CAN
// 	TCANFDProperty FFDProperties;      // [7-3] tbd                            <> CAN
// 						   // [2] ESI, The E RROR S TATE I NDICATOR (ESI) flag is transmitted dominant by error active nodes, recessive by error passive nodes. ESI does not exist in CAN format frames
// 						   // [1] BRS, If the bit is transmitted recessive, the bit rate is switched from the standard bit rate of the A RBITRATION P HASE to the preconfigured alternate bit rate of the D ATA P HASE . If it is transmitted dominant, the bit rate is not switched. BRS does not exist in CAN format frames.
// 						   // [0] EDL: 0-normal CAN frame, 1-FD frame, added 2020-02-12, The E XTENDED D ATA L ENGTH (EDL) bit is recessive. It only exists in CAN FD format frames
// 	s32  FIdentifier;      // CAN identifier                       = CAN
// 	u64 FTimeUS;          // timestamp in us                      = CAN
//     u8 FData[64];          // 64 data bytes to send                <> CAN
// }TLibCANFD, * PLibCANFD;
typedef struct _TLibCANFD {
	u8 FIdxChn;
	u8 FProperties;
	u8 FDLC;
	u8 FFDProperties;
	s32 FIdentifier;
	s64 FTimeUs;
	u8  FData[64];
	void LoadData(u8* a) {
		for (u32 i = 0; i < 64; i++) {
			FData[i] = *a++;
		}
	}
	void SetStdId(s32 AId, s32 ADLC) {
		s32 i;
		FIdxChn = 0;
		FIdentifier = AId;
		FDLC = ADLC;
		FProperties = 0;
		FFDProperties = MASK_CANFDProp_IS_FD;
		SetTX(false);
		SetStd(true);
		SetData(true);
		for (i = 0; i < 64; i++) FData[i] = 0;
		FTimeUs = 0;
	}
	void SetExtId(s32 AId, s32 ADLC) {
		s32 i;
		FIdxChn = 0;
		FIdentifier = AId;
		FDLC = ADLC;
		FFDProperties = MASK_CANFDProp_IS_FD;
		FProperties = 0;
		SetTX(false);
		SetStd(false);
		SetData(true);
		for (i = 0; i < 64; i++) FData[i] = 0;
		FTimeUs = 0;
	}
	inline bool GetTX() {
		return (FProperties & MASK_CANProp_DIR_TX) != 0;
	}
	inline bool GetData() {
		return (FProperties & MASK_CANProp_REMOTE) == 0;
	}
	inline bool GetStd() {
		return (FProperties & MASK_CANProp_EXTEND) == 0;
	}
	inline bool GetErr() {
		return (FProperties & MASK_CANProp_ERROR) != 0;
	}
	inline void SetTX(bool a) {
		if (a) {
			FProperties = FProperties | MASK_CANProp_DIR_TX;
		} else {
			FProperties = FProperties & (~MASK_CANProp_DIR_TX);
		}
	}
	inline void SetData(bool a) {
		if (a) {
			FProperties = FProperties & (~MASK_CANProp_REMOTE);
		} else {
			FProperties = FProperties | MASK_CANProp_REMOTE;
		}
	}
	inline void SetStd(bool a) {
		if (a) {
			FProperties = FProperties & (~MASK_CANProp_EXTEND);
		} else {
			FProperties = FProperties | MASK_CANProp_EXTEND;
		}
	}
	inline void SetErr(bool a) {
		if (a) {
			FProperties = FProperties & (~MASK_CANProp_ERROR);
		} else {
			FProperties = FProperties | MASK_CANProp_ERROR;
		}
	}
	inline bool GetIsFD() {
		return (FFDProperties & MASK_CANFDProp_IS_FD) != 0;
	}
	inline void SetIsFD(bool a) {
		if (a) {
			FFDProperties = FFDProperties | MASK_CANFDProp_IS_FD;
		} else {
			FFDProperties = FFDProperties & (~MASK_CANFDProp_IS_FD);
		}
	}
	inline bool GetIsBRS() {
		return (FFDProperties & MASK_CANFDProp_IS_BRS) != 0;
	}
	inline void SetIsBRS(bool a) {
		if (a) {
			FFDProperties = FFDProperties | MASK_CANFDProp_IS_BRS;
		} else {
			FFDProperties = FFDProperties & (~MASK_CANFDProp_IS_BRS);
		}
	}
	inline bool GetIsESI() {
		return (FFDProperties & MASK_CANFDProp_IS_ESI) != 0;
	}
	inline void SetIsESI(bool a) {
		if (a) {
			FFDProperties = FFDProperties | MASK_CANFDProp_IS_ESI;
		} else {
			FFDProperties = FFDProperties & (~MASK_CANFDProp_IS_ESI);
		}
	}
	s32 GetDataLength() {
		const u8 DLC_DATA_BYTE_CNT[16] = {
			0, 1, 2, 3, 4, 5, 6, 7,
			8, 12, 16, 20, 24, 32, 48, 64
		};
		s32 l = MIN(FDLC, 15);
		l = MAX(l, 0);
		return DLC_DATA_BYTE_CNT[l];
	}
} TLibCANFD, * PLibCANFD;
typedef union
{
	u8 value;
	struct {
		u8 istx : 1;
		u8 breaksended : 1;
		u8 breakreceived : 1;
		u8 syncreceived : 1;
		u8 hwtype : 2;
		u8 isLogged : 1;
		u8 iserrorframe : 1;
	}bits;
}TLINProperty;

typedef struct _TLIN {
    u8  FIdxChn;
    u8  FErrStatus;
    u8  FProperties;
    u8  FDLC;
    u8  FIdentifier;
    u8  FChecksum;
    u8  FStatus;
    s64 FTimeUs;
    u8  FData[8];
    void LoadData(u8* a) {
        for (u32 i = 0; i < 8; i++) {
            FData[i] = *a++;
        }
    }
	inline bool GetTX() {
        return (FProperties & MASK_LINProp_DIR_TX) != 0;
    }
	inline bool GetData() {
        return true;
    }
	inline void SetTX(bool a) {
        if (a) {
            FProperties = FProperties | MASK_LINProp_DIR_TX;
        } else {
            FProperties = FProperties & (~MASK_LINProp_DIR_TX);
        }
    }
    void SetId(const s32 AId, const s32 ADLC) {
        FIdxChn = 0;
        FErrStatus = 0;
        FProperties = 0;
        FDLC = ADLC;
        FIdentifier = AId;
        for(int i =0;i<FDLC;i++)
			FData[i] = 0;
        FChecksum = 0;
		FStatus = 0;
        FTimeUs = 0;
    }
} TLibLIN, *PLibLIN;

typedef enum : byte  //new property in C++ 11
{
	LIN_Protocol_13,
	LIN_Protocol_20,
	LIN_Protocol_21,
	LIN_Protocol_J2602
}TLINProtocol;

typedef enum
{
	MasterNode,
	SlaveNode,
	MonitorNode
}TLIN_FUNCTION_TYPE;

typedef struct _TFlexRayErrorEventCycloneI {
	u8   FErrorFlags;
} TFlexRayErrorEventCycloneI;
typedef struct _TFlexRayErrorEventCycloneII {
	u8   FErrorPacketFlag;     // 0 = No error
                               // 1 = FlexCard overflow
                               // 2 = PCO error mode changed
                               // 3 = Sync frames below minimum
                               // 4 = Sync frame overflow
                               // 5 = Clock correction failure
                               // 6 = Parity error
                               // 7 = Receive FIFO overrun
                               // 8 = Empty FIFO access
                               // 9 = Illegal input buffer access
                               // 10 = Illegal output buffer access
                               // 11 = Syntax error
                               // 12 = Content error
                               // 13 = Slot boundary violation
                               // 14 = Transmission across boundary
                               // 15 = Latest transmit violation
	u32  FErrorData;           // error packet = 2
                               //                0 = Unknown state
                               //                1 = FlexRay protocol spec. > CONFIG
                               //                2 = FlexRay protocol spec. > NORMAL_ACTIVE
                               //                3 = FlexRay protocol spec. > NORMAL_PASSIVE
                               //                4 = FlexRay protocol spec. > HALT
                               //                5 = FlexRay protocol spec. > READY
                               //                6 = FlexRay protocol spec. > STARTUP
                               //                7 = FlexRay protocol spec. > WAKEUP
	                           // error packet = 3 or 4
                               //                Bits 0..3 > Sync frames even on channel A
                               //                Bits 4..7 > Sync frames even on channel B
                               //                Bits 8..11 > Sync frames odd on channel A
                               //                Bits 12..15 > Sync frames odd on channel B
	                           // error packet = 5
                               //                Bit 0 > Missing rate correction
                               //                Bit 1 > Rate correction limit reached
                               //                Bit 2 > Offset correction limit reached
                               //                Bit 3 > Missing offset correction
                               //                Bit 4..7 > Sync frames even on channel A
                               //                Bits 8..11 > Sync frames even on channel B
                               //                Bits 12..15 > Sync frames odd on channel A
                               //                Bits 16..19 > Sync frames odd on channel B
	                           // error packet = 11..15
	                           //                LOW - uint16_t of mData[1] > Channel
	                           //                HI - uint16_t of mData[1] > Slot count
} TFlexRayErrorEventCycloneII;
typedef struct _TFlexRayErrorEventBUSDOCTOR {
	u8   FErrorFlags;		   // error tag
                               // 0 = FR_ERROR_POC_MODE
                               // 1 = FR_ERROR_SYNC_FRAMES_BELOWMIN
                               // 2 = FR_ERROR_SYNC_FRAMES_OVERLOAD
                               // 3 = FR_ERROR_CLOCK_CORR_FAILURE
                               // 4 = FR_ERROR_NIT_FAILURE
                               // 5 = FR_ERROR_CC_ERROR
                               // 6 = FR_ERROR_OVERFLOW
	u32  FErrorData;           // error tag value = 0
                               //                 0 FR_ERROR_POC_ACTIVE
                               //                 1 FR_ERROR_POC_PASSIVE
                               //                 2 FR_ERROR_POC_COMM_HALT
	                           // error tag value = 1 or 2
                               //                 Bits 0..3 Sync frames even on channel A
                               //                 Bits 4..7 Sync frames even on channel B
                               //                 Bits 8..11 Sync frames odd on channel A
                               //                 Bits 12..15 Sync frames odd on channel B
	                           // error tag value = 3
                               //                 Bit 0 Missing rate correction
                               //                 Bit 1 Missing rate correction limit reached
                               //                 Bit 2 Offset correction limit reached
                               //                 Bit 3 Missing offset correction
                               //                 Bits 4..19 Clock correction failed counter
                               //                 Bit 20..23 Sync frames even on channel A
                               //                 Bit 24..27 Sync frames even on channel B
                               //                 Bit 28..31 Sync frames odd on channel A
                               //                 Bit 32..35 Sync frames odd on channel B
	                           // error tag value = 4
                               //                 1 FR_ERROR_NIT_SENA
                               //                 2 FR_ERROR_NIT_SBNA
                               //                 4 FR_ERROR_NIT_SENB
                               //                 8 FR_ERROR_NIT_SBNB
	                           // error tag value = 5
	                           //                 0x00000001 POC Error Mode Changed
                               //                 0x00000004 Sync Frames Below Minimum
                               //                 0x00000008 Sync Frame Overflow
                               //                 0x00000010 Clock Correction Failure
                               //                 0x00000040 Parity Error, data from MHDS(internal ERay error)
                               //                 0x00000200 Illegal Input Buffer Access(internal ERay error)
                               //                 0x00000400 Illegal Output Buffer Access(internal ERay error)
                               //                 0x00000800 Message Handler Constraints Flag data from MHDF (internal ERay error)
                               //                 0x00010000 Error Detection on channel A, data from ACS
                               //                 0x00020000 Latest Transmit Violation on channel A
                               //                 0x00040000 Transmit Across Boundary on Channel A
                               //                 0x01000000 Error Detection on channel B, data from ACS
                               //                 0x02000000 Latest Transmit Violation on channel B
                               //                 0x04000000 Transmit Across Boundary on Channel B
} TFlexRayErrorEventBUSDOCTOR;
typedef struct _TFlexRayErrorEventVN {
	u8   FErrorTag;
	u32  FErrorData1;
	u32  FErrorData2;
} TFlexRayErrorEventVN;
typedef union _TFlexRayErrorEvent {
	TFlexRayErrorEventCycloneI  FCycloneI;
	TFlexRayErrorEventCycloneII FCycloneII;
	TFlexRayErrorEventBUSDOCTOR FBusDoctor;
	TFlexRayErrorEventVN        FVN;
} TFlexRayErrorEvent, *PFlexRayErrorEvent;
// FlexRay Status Type --------------------------------------------------------------
typedef struct _TFlexRayStatusEvent {
	u32 FPOCState;             // POC state of E-Ray register CCSV. Only valid for Vector interfaces
	u32 FWakeupState;          // WakeUp state. Only valid for Vector interfacesand for Cyclone II, if symbol is void (mReserved[0] = 0)
                               // 0 UNDEFINED
                               // 1 RECEIVED_HEADER
                               // 2 RECEIVED_WUP
                               // 3 COLLISION_HEADER
                               // 4 COLLISION_WUP
                               // 5 COLLISION_UNKNOWN
                               // 6 TRANSMITTED
                               // 7 EXTERNAL_WAKEUP
                               // 8 WUP_RECEIVED_WITHOUT_WUS_TX
	u32 FSyncState;            // Sync-State, only valid for Cyclone 1, for Cyclone II if the wakup state value is 0
                               // 0 = Not synced passive
                               // 1 = Synced active
                               // 2 = Not synced
	u16 FSymbol;               // 0: void, 1: CAS, 2: MTS, 3: WUS
} TFlexRayStatusEvent, *PFlexRayStatusEvent;
// FlexRay Start Cycle Type ---------------------------------------------------------
typedef struct _TFlexRayStartCycleEvent {
	u32 FSyncCorrection;                // Sync correction of CC, read from	RCV register
	u32 FOffsetCorrection;              // Offset correction of CC, read from OCV register
	u32 FCyclesWithNoCorrection;        // Cycles with no correction, read from CCEV register
	u32 FCyclesWithCorrectionInPassive; // Cycles with correction in passive mode, read from CCEV register
	u32 FSyncFrameStatus;               // Sync Frame status, read from	SFS register
	u16 FNMVectorLength;                // Length of NM-Vector in bytes
	u8  FDataBytes[12];                 // Array of databytes (NM vector max. length)
} TFlexRayStartCycleEvent, *PFlexRayStartCycleEvent;
// FlexRay Frame Type ---------------------------------------------------------------
//TFlexRay f0 = { 0,1,0,0,0,0,5,2,0x0,0x0,0x0,0,0x0,0x0,0,0,87057,{0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };

/*Flexray*/
typedef struct _TLibFlexRay {
    u8 FIdxChn;               // channel index starting from 0
    u8 FChannelMask;          // 0: reserved, 1: A, 2: B, 3: AB
    u8 FDir;                  // 0: Rx, 1: Tx, 2: Tx Request
    u8 FPayloadLength;        // payload length in bytes
    u8 FActualPayloadLength;  // actual data bytes
    u8 FCycleNumber;          // cycle number: 0~63
    u8 FCCType;               // 0 = Architecture independent, 1 = Invalid CC type, 2 = Cyclone I, 3 = BUSDOCTOR, 4 = Cyclone II, 5 = Vector VN interface, 6 = VN - Sync - Pulse(only in Status Event, for debugging purposes only)
    u8 FFrameType;		// // 0 = raw flexray frame, 1 = error event, 2 = status, 3 = start cycle
    u16 FHeaderCRCA;          // header crc A
    u16 FHeaderCRCB;          // header crc B
    u16 FFrameStateInfo;      // bit 0~15, error flags
    u16 FSlotId;              // static seg: 0~1023
    u32 FFrameFlags;          // bit 0~22
                             // 0 1 = Null frame.
                             // 1 1 = Data segment contains valid data
                             // 2 1 = Sync bit
                             // 3 1 = Startup flag
                             // 4 1 = Payload preamble bit
                             // 5 1 = Reserved bit
                             // 6 1 = Error flag(error frame or invalid frame)
                             // 7 Reserved
                             // 8 Internally used in CANoe / CANalyzer
                             // 9 Internally used in CANoe / CANalyzer
                             // 10 Internally used in CANoe / CANalyzer
                             // 11 Internally used in CANoe / CANalyzer
                             // 12 Internally used in CANoe / CANalyzer
                             // 13 Internally used in CANoe / CANalyzer
                             // 14 Internally used in CANoe / CANalyzer
                             // 15 1 = Async.monitoring has generated this event
                             // 16 1 = Event is a PDU
                             // 17 Valid for PDUs only.The bit is set if the PDU is valid(either if the PDU has no  // update bit, or the update bit for the PDU was set in the received frame).
                             // 18 Reserved
                             // 19 1 = Raw frame(only valid if PDUs are used in the configuration).A raw frame may  // contain PDUs in its payload
                             // 20 1 = Dynamic segment	0 = Static segment
                             // 21 This flag is only valid for frames and not for PDUs.	1 = The PDUs in the payload of  // this frame are logged in separate logging entries. 0 = The PDUs in the payload of this  // frame must be extracted out of this frame.The logging file does not contain separate  // PDU - entries.
                             // 22 Valid for PDUs only.The bit is set if the PDU has an update bit
    u32 FFrameCRC;            // frame crc
    u64 FReserved1;           // 8 reserved bytes
    u64 FReserved2;           // 8 reserved bytes
    u64 FTimeUs;              // timestamp in us
    u8  FData[254];// 254 data bytes
}TLibFlexRay, * PLibFlexRay;
//Flexray
typedef struct _TLibFlexray_controller_config {
    u8 NETWORK_MANAGEMENT_VECTOR_LENGTH;
    u8 PAYLOAD_LENGTH_STATIC;
    u16 FReserved;
    u16 LATEST_TX;
        // __ prtc1Control
    u16 T_S_S_TRANSMITTER;
    u8 CAS_RX_LOW_MAX;
    u8 SPEED;      //0 for 10m, 1 for 5m, 2 for 2.5m, convert from Database
    u16 WAKE_UP_SYMBOL_RX_WINDOW;
    u8 WAKE_UP_PATTERN;
        // __ prtc2Control
    u8 WAKE_UP_SYMBOL_RX_IDLE;
    u8 WAKE_UP_SYMBOL_RX_LOW;
    u8 WAKE_UP_SYMBOL_TX_IDLE;
    u8 WAKE_UP_SYMBOL_TX_LOW;
        // __ succ1Config
    u8 channelAConnectedNode;      // Enable ChannelA: 0: Disable 1: Enable
    u8 channelBConnectedNode;      // Enable ChannelB: 0: Disable 1: Enable
    u8 channelASymbolTransmitted; // Enable Symble Transmit function of Channel A: 0: Disable 1: Enable
    u8 channelBSymbolTransmitted; // Enable Symble Transmit function of Channel B: 0: Disable 1: Enable
    u8 ALLOW_HALT_DUE_TO_CLOCK;
    u8 SINGLE_SLOT_ENABLED;        // FALSE_0, TRUE_1
    u8 wake_up_idx;                // Wake up channe: 0:ChannelA�� 1:ChannelB
    u8 ALLOW_PASSIVE_TO_ACTIVE;
    u8 COLD_START_ATTEMPTS;
    u8 synchFrameTransmitted;      // Need to transmit sync frame
    u8 startupFrameTransmitted;    // Need to transmit startup frame
    // __ succ2Config
    u32 LISTEN_TIMEOUT;
    u8 LISTEN_NOISE;               //2_16
    // __ succ3Config
    u8 MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE;
    u8 MAX_WITHOUT_CLOCK_CORRECTION_FATAL;
    u8 REVERS0;                    //Memory Align
    // __ gtuConfig
    // __ gtu01Config
    u32 MICRO_PER_CYCLE;
        // __ gtu02Config
    u16 Macro_Per_Cycle;
    u8 SYNC_NODE_MAX;
    u8 REVERS1;  //Memory Align
    // __ gtu03Config
    u8 MICRO_INITIAL_OFFSET_A;
    u8 MICRO_INITIAL_OFFSET_B;
    u8 MACRO_INITIAL_OFFSET_A;
    u8 MACRO_INITIAL_OFFSET_B;
        // __ gtu04Config
    u16 N_I_T;
    u16 OFFSET_CORRECTION_START;
        // __ gtu05Config
    u8 DELAY_COMPENSATION_A;
    u8 DELAY_COMPENSATION_B;
    u8 CLUSTER_DRIFT_DAMPING;
    u8 DECODING_CORRECTION;
        // __ gtu06Config
    u16 ACCEPTED_STARTUP_RANGE;
    u16 MAX_DRIFT;
        // __ gtu07Config
    u16 STATIC_SLOT;
    u16 NUMBER_OF_STATIC_SLOTS;
        // __ gtu08Config
    u8 MINISLOT;
    u8 REVERS2;  //Memory Align
    u16 NUMBER_OF_MINISLOTS;
        // __ gtu09Config
    u8 DYNAMIC_SLOT_IDLE_PHASE;
    u8 ACTION_POINT_OFFSET;
    u8 MINISLOT_ACTION_POINT_OFFSET;
    u8 REVERS3;  //Memory Align
    // __ gtu10Config
    u16 OFFSET_CORRECTION_OUT;
    u16 RATE_CORRECTION_OUT;
        // __ gtu11Config
    u8 EXTERN_OFFSET_CORRECTION;
    u8 EXTERN_RATE_CORRECTION;
        //
    u8 config_byte1;  //Memory Align
    //bit0:1:ͨ������halt���Զ�����   0��������

    u8 config_byte;  //Memory Align
    //bit0: 1������cha���ն˵���  0��������
    //bit1: 1������chb���ն˵���  0��������
    //bit2: 1�����ý���FIFO    0��������
    //bit3: 1��FIFO֧�ֽ��վ�̬֡��FIFO������ʱ��Ч������Ĭ�Ͽ���todo��   0:������
    //bit4: 1��cha�Ž�ʹ��             0����ʹ��
    //bit5: 1��chb�Ž�ʹ��             0����ʹ��
    //bit6: 1��FIFO֧�ֽ��տ�֡��FIFO������ʱ��Ч������Ĭ�Ϲر�todo��   0:������
    //bit7: 1����������ʱ����wakeup-pattern   0:��������ʱ������wakeup-pattern
}TLibFlexray_controller_config, * PLibFlexray_controller_config;

typedef struct _TLibTrigger_def {
    u16 slot_id;
    u8 frame_idx;
    u8 cycle_code;//BASE-CYCLE + CYCLE-REPETITION
    u8 config_byte;
    //bit 0:�Ƿ�ʹ��ͨ��A
    //bit 1:�Ƿ�ʹ��ͨ��B
    //bit 2:�Ƿ�����������ģ� dir Ϊrxʱ����
    //bit 3:����ģʽ��0��ʾ�������䣬1��ʾ���δ����� dir Ϊrxʱ����
    //bit 4:�Ƿ�Ϊ���������ģ�ֻ�л�����0������1�� dir Ϊrxʱ����
    //bit 5:�Ƿ�Ϊͬ�����ģ�ֻ�л�����0/1������1�� dir Ϊrxʱ����
    //bit 6:dir: 0-tx  1-rx
    //bit 7:
    u8 rev;
}TLibTrigger_def, * PLibTrigger_def;


//function pointer type
//Device connectted event
typedef void(* TTSCANConnectedCallback_t)(const size_t ADevicehandle);
//Device disconnected event
typedef void(* TTSCANDisConnectedCallback_t)(const size_t ADevicehandle);
//High precise timer event
typedef void(* THighResTimerCallback_t)(const u32 ADevicehandle);
//CAN Msg Received Event
typedef void(* TCANQueueEvent_Win32_t)(const TLibCAN* AData);
//LIN Msg Received Event
typedef void(* TLINQueueEvent_Win32_t)(const TLibLIN* AData);
//CANFD Msg Received Event
typedef void(* TCANFDQueueEvent_Win32_t)(const TLibCANFD* AData);


#if defined(_WIN32)
typedef  void(__stdcall* TFlexrayQueueEvent_WHandle)(size_t* obj,const PLibFlexRay AData);
typedef void(__stdcall* TCANQueueEvent_WHandle)(size_t* obj,const PLibCAN AData);
//LIN Msg Received Event
typedef void(__stdcall* TLINQueueEvent_WHandle)(size_t* obj,const PLibLIN AData);
//CANFD Msg Received Event
typedef void(__stdcall* TCANFDQueueEvent_WHandle)(size_t* obj,const PLibCANFD AData);
#endif
#if defined(__linux__)
typedef  void(*TFlexrayQueueEvent_WHandle)(size_t* obj,const PLibFlexRay AData);
typedef void(* TCANQueueEvent_WHandle)(size_t* obj,const PLibCAN AData);
//LIN Msg Received Event
typedef void(* TLINQueueEvent_WHandle)(size_t* obj,const PLibLIN AData);
//CANFD Msg Received Event
typedef void(* TCANFDQueueEvent_WHandle)(size_t* obj,const PLibCANFD AData);
#endif
//Register callback functions
//Register CAN message received event
extern "C" u32 tscan_register_event_can(const size_t ADeviceHandle, const TCANQueueEvent_Win32_t ACallback);
//unregister CAN message received event
extern "C" u32 tscan_unregister_event_can(const size_t ADeviceHandle, const TCANQueueEvent_Win32_t ACallback);

//Register CANFD message received event
extern "C" u32 tscan_register_event_canfd(const size_t ADeviceHandle, const TCANFDQueueEvent_Win32_t ACallback);
//UnRegister CAN message received event
extern "C" u32 tscan_unregister_event_canfd(const size_t ADeviceHandle, const TCANFDQueueEvent_Win32_t ACallback);


//Register LIN message received event
extern "C" u32 tslin_register_event_lin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
//UnRegister LIN message received event
extern "C" u32 tslin_unregister_event_lin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);


extern "C" u32 tscan_register_pretx_event_canfd_whandle(const size_t ADeviceHandle, const TCANFDQueueEvent_WHandle ACallback);
//UnRegister CAN message received event
extern "C" u32 tscan_unregister_pretx_event_canfd_whandle(const size_t ADeviceHandle, const TCANFDQueueEvent_WHandle ACallback);

extern "C" u32 tscan_register_event_canfd_whandle(const size_t ADeviceHandle, const TCANFDQueueEvent_WHandle ACallback);
//UnRegister CAN message received event
extern "C" u32 tscan_unregister_event_canfd_whandle(const size_t ADeviceHandle, const TCANFDQueueEvent_WHandle ACallback);


extern "C" u32 tscan_register_event_can_whandle(const size_t ADeviceHandle, const TCANQueueEvent_WHandle ACallback);
//UnRegister CAN message received event
extern "C" u32 tscan_unregister_event_can_whandle(const size_t ADeviceHandle, const TCANQueueEvent_WHandle ACallback);

//Register LIN message received event
extern "C" u32 tslin_register_event_lin_whandle(const size_t ADeviceHandle, const TLINQueueEvent_WHandle ACallback);
//UnRegister LIN message received event
extern "C" u32 tslin_unregister_event_lin_whandle(const size_t ADeviceHandle, const TLINQueueEvent_WHandle ACallback);

extern "C" u32 tsflexray_register_event_flexray_whandle(const size_t ADeviceHandle, const TFlexrayQueueEvent_WHandle ACallback);
//UnRegister LIN message received event
extern "C" u32 tsflexray_unregister_event_flexray_whandle(const size_t ADeviceHandle, const TFlexrayQueueEvent_WHandle ACallback);
extern "C" u32 tsflexray_register_pretx_event_flexray_whandle(const size_t ADeviceHandle, const TFlexrayQueueEvent_WHandle ACallback);
extern "C" u32 tsflexray_unregister_pretx_event_flexray_whandle(const size_t ADeviceHandle, const TFlexrayQueueEvent_WHandle ACallback);

//Register FastLIN message received event
extern "C" u32 tscan_register_event_fastlin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);
//UnRegister FastLIN message received event
extern "C" u32 tscan_unregister_event_fastlin(const size_t ADeviceHandle, const TLINQueueEvent_Win32_t ACallback);

//functional
//Scan the devices online
extern "C" u32 tscan_scan_devices(uint32_t* ADeviceCount);
//get device information such as: manufactor, product id, serial no string
extern "C" u32  tscan_get_device_info(
  const u32 ADeviceIndex,
  char** AFManufacturer,
  char** AFProduct,
  char** AFSerial
  );
//Connect device��ADeviceSerial !=NULL��connect the appointted device with serial no string��ADeviceSerial == NULL��connect default device
extern "C" uint32_t tscan_connect(const char* ADeviceSerial, u64* AHandle);
//Disconnect the appointted device
extern "C" u32 tscan_disconnect_by_handle(const size_t ADeviceHandle);
//Disconnect all devices
extern "C" u32 tscan_disconnect_all_devices(void);
//initial libtscan driver module, which should be called before calling apis
extern "C" void initialize_lib_tscan(bool AEnableFIFO, bool AEnableErrorFrame, bool AEnableTurbe);
//finalize libtscan driver module
extern "C" void finalize_lib_tscan(void);

//CAN�������
//Synchronous transmit can message
extern "C" u32 tscan_transmit_can_sync(const size_t ADeviceHandle, const TLibCAN* ACAN, const u32 ATimeoutMS);
//ASynchronous transmit can message
extern "C" u32 tscan_transmit_can_async(const size_t ADeviceHandle, const TLibCAN* ACAN);

//Configuration of can baudrate
//ADeviceHandle:[In] Device Handle;
//AChnIdx:[In] Channel Index
//ARateKbps:[In] Baudrate(kbps), such as 500 means 500kbps
//A120OhmConnected:[In] Enable internal 120O terminal resistor
extern "C" u32 tscan_config_can_by_baudrate(const size_t ADeviceHandle, const APP_CHANNEL AChnIdx, const double ARateKbps, const u32 A120OhmConnected);

//Read CAN Message from FIFO
//ADeviceHandle��[In]Device Handle��
//ACANBuffers:[In]Message Buffer��
//ACANBufferSize��[In,Out] In: message buffer size; Out: real message numer read from driver cache
//AChn:0-31: read the message from channel AChn; 0xFF: read the message from all channels of the device
//ARxTx:1: read transmitted and received messages; 0: only read the received messages, ignore the transmitted message
//return: ==0: success to read the data;
//          Other:error code
extern "C" u32 tsfifo_receive_can_msgs(const size_t ADeviceHandle, const TLibCAN* ACANBuffers, s32* ACANBufferSize, u8 AChn, u8 ARXTX);

//CANFD�������
//Synchronous transmit canfd message
extern "C" u32 tscan_transmit_canfd_sync(const size_t ADeviceHandle, const TLibCANFD* ACAN, const u32 ATimeoutMS);
//ASynchronous transmit canfd message
extern "C" u32 tscan_transmit_canfd_async(const size_t ADeviceHandle, const TLibCANFD* ACAN);

extern "C" u32 tscan_add_cyclic_msg_can(const size_t ADeviceHandle, const PLibCAN AData, const float ATimeMs); 
extern "C" u32 tscan_add_cyclic_msg_canfd(const size_t ADeviceHandle, const PLibCANFD AData, const float ATimeMs);

extern "C" u32  tscan_add_precise_cyclic_message(const size_t ADeviceHandle, const s32 AID, const u8 AChn,const u8 AIsExt,const float APeriodMS,const s32 ATimeout);

extern "C" u32  tscan_delete_precise_cyclic_message(const size_t ADeviceHandle, const s32 AID, const u8 AChn,const u8 AIsExt,const s32 ATimeout);


extern "C" u32 tscan_delete_cyclic_msg_can(const size_t ADeviceHandle, const PLibCAN AData); 
extern "C" u32 tscan_delete_cyclic_msg_canfd(const size_t ADeviceHandle, const PLibCANFD AData);

//Configuration of canfd baudrate
//ADeviceHandle:[In] Device Handle;
//AChnIdx:[In] Channel Index
//AArbRateKbps:[In] Baudrate(kbps) of arb, such as 500 means 500kbps
//ADataRateKbps:[In] Baudrate(kbps) of data, such as 2000 means 2000kbps
//AControllerType[In]: Controller type: classic CAN, ISO-FDCAN, NoISO-FDCAN
//AControllerMode: Controller work mode
//A120OhmConnected:[In] Enable internal 120O terminal resistor
extern "C" u32 tscan_config_canfd_by_baudrate(const size_t  ADeviceHandle, const APP_CHANNEL AChnIdx, const double AArbRateKbps, const double ADataRateKbps, const TLIBCANFDControllerType AControllerType,
	const TLIBCANFDControllerMode AControllerMode, const u32 A120OhmConnected);

//Read CANFD Message from FIFO
//ADeviceHandle��[In]Device Handle��
//ACANBuffers:[In]Message Buffer��
//ACANBufferSize��[In,Out] In: message buffer size; Out: real message numer read from driver cache
//AChn:0-31: read the message from channel AChn; 0xFF: read the message from all channels of the device
//ARxTx:1: read transmitted and received messages; 0: only read the received messages, ignore the transmitted message
//return: ==0: success to read the data;
//          Other:error code
extern "C" u32 tsfifo_receive_canfd_msgs(const size_t ADeviceHandle, const TLibCANFD* ACANBuffers, s32* ACANBufferSize, u8 AChn, u8 ARXTX);

extern "C" u32 tsfifo_clear_canfd_receive_buffers(const size_t ADeviceHandle, const s32 AIdxChn);

//Flexray�������
extern "C" u32 tsflexray_set_controller_frametrigger(const size_t ADeviceHandle, const int ANodeIndex,
        const PLibFlexray_controller_config AControllerConfig,
        const int* AFrameLengthArray, const int AFrameNum,
        const PLibTrigger_def AFrameTrigger, const int AFrameTriggerNum, const int ATimeoutMs);
extern "C" u32 tsflexray_set_controller(const size_t ADeviceHandle, const int ANodeIndex,
    const PLibFlexray_controller_config AControllerConfig, const int ATimeoutMs);
extern "C" u32 tsflexray_set_frametrigger(const size_t ADeviceHandle, const int ANodeIndex,
    const int* AFrameLengthArray, const int AFrameNum,
    const PLibTrigger_def AFrameTrigger, const int AFrameTriggerNum, const int ATimeoutMs);
extern "C" u32 tsflexray_cmdreq(const size_t ADeviceHandle, const int AChnIdx, const int Action, const u8* AWriteDataBuffer, const s32 AWriteBufferSize, const u8* AReadDataBuffer, const s32* AReadDataBufferSize, const int  ATimeoutMs);
extern "C" u32 tsflexray_transmit_sync(const size_t ADeviceHandle, const PLibFlexRay AData, const int ATimeoutMs);
extern "C" u32 tsflexray_transmit_async(const size_t ADeviceHandle, const PLibFlexRay AData);

extern "C" u32 tsfifo_receive_flexray_msgs(const size_t ADeviceHandle, PLibFlexRay ADataBuffers, s32* ADataBufferSize, u8 AIdxChn, u8 ARxTx);
extern "C" u32 tsfifo_clear_flexray_receive_buffers(const size_t ADeviceHandle, const s32 AIdxChn);
extern "C" u32 tsflexray_start_net(const size_t ADeviceHandle, const int AChnIdx, const int ATimeoutMs);
extern "C" u32 tsflexray_stop_net(const size_t ADeviceHandle, const int AChnIdx, const int ATimeoutMs);
extern "C" u32 tsfifo_read_flexray_buffer_frame_count(const size_t ADeviceHandle, const int AIdxChn, int* ACount);
extern "C" u32 tsfifo_read_flexray_tx_buffer_frame_count(const size_t ADeviceHandle, const int AIdxChn, int* ACount);
extern "C" u32 tsfifo_read_flexray_rx_buffer_frame_count(const size_t ADeviceHandle, const int AIdxChn, int* ACount);


//LINC hannel
//ADeviceHandle[In]:Device Channel��
//AChnIdx[In]:Channel of device;
//AFunctionType[In]: funtion type of LIN Node, 0:MasterNode;1:SlaveNode;2:MonitorNode
extern "C" u32 tslin_set_node_functiontype(const size_t ADeviceHandle, const APP_CHANNEL AChnIdx, const u8 AFunctionType);
//Apply donwload new ldf file, which will clear the existing ldf information
extern "C" u32 tslin_apply_download_new_ldf(const size_t ADeviceHandle, const APP_CHANNEL AChnIdx);
//Synchronous transmit lin message
extern "C" u32 tslin_transmit_lin_sync(const size_t ADeviceHandle, const TLibLIN* ALIN, const u32 ATimeoutMS);
//ASynchronous transmit lin message
extern "C" u32 tslin_transmit_lin_async(const size_t ADeviceHandle, const TLibLIN* ALIN);
//Synchronous transmit Fast lin message
extern "C" u32 tslin_transmit_fastlin_async(const size_t ADeviceHandle, const TLibLIN* ALIN);
//Configuration of lin baudrate
extern "C" u32 tslin_config_baudrate(const size_t ADeviceHandle, const APP_CHANNEL AChnIdx, const double ARateKbps, TLINProtocol AProtocol);

//Read LIN Message from FIFO
//ADeviceHandle��[In]Device Handle��
//ACANBuffers:[In]Message Buffer��
//ACANBufferSize��[In,Out] In: message buffer size; Out: real message numer read from driver cache
//AChn:0-31: read the message from channel AChn; 0xFF: read the message from all channels of the device
//ARxTx:1: read transmitted and received messages; 0: only read the received messages, ignore the transmitted message
//return: ==0: success to read the data;
//          Other:error code
extern "C" u32 tsfifo_receive_lin_msgs(const size_t ADeviceHandle, const TLibLIN* ALINBuffers, s32* ALINBufferSize, u8 AChn, u8 ARXTX);

//Read LIN Message from FIFO
//ADeviceHandle��[In]Device Handle��
//ACANBuffers:[In]Message Buffer��
//ACANBufferSize��[In,Out] In: message buffer size; Out: real message numer read from driver cache
//AChn:0-31: read the message from channel AChn; 0xFF: read the message from all channels of the device
//ARxTx:1: read transmitted and received messages; 0: only read the received messages, ignore the transmitted message
//return: ==0: success to read the data;
//          Other:error code
extern "C" u32 tsfifo_receive_fastlin_msgs(const size_t ADeviceHandle, const TLibLIN* ALINBuffers, s32* ALINBufferSize, u8 AChn, u8 ARXTX);

//get the error description
extern "C" u32 tscan_get_error_description(const u32 ACode, char** ADesc);

//High precision playback API
extern "C" s32 tsreplay_add_channel_map(const size_t ADeviceHandle, APP_CHANNEL ALogicChannel, APP_CHANNEL AHardwareChannel);
extern "C" void tsreplay_clear_channel_map(const size_t ADeviceHandle);
extern "C" s32 tsreplay_start_blf(const size_t ADeviceHandle, char* ABlfFilePath, int ATriggerByHardware, u64 AStartUs,u64 AEndUs);
extern "C" s32 tsreplay_stop(const size_t ADeviceHandle);
extern "C" s32 tsdiag_can_create(int* pDiagModuleIndex,
											u32 AChnIndex,
											byte ASupportFDCAN,
											byte AMaxDLC,
											u32 ARequestID,
											bool ARequestIDIsStd,
											u32 AResponseID,
											bool AResponseIDIsStd,
											u32 AFunctionID,
											bool AFunctionIDIsStd);

//diagnostic API
extern "C" s32 tsdiag_can_delete(int ADiagModuleIndex);
extern "C" s32 tsdiag_can_delete_all(void);
extern "C" s32 tsdiag_can_attach_to_tscan_tool(int ADiagModuleIndex, size_t ACANToolHandle);
/*TP Raw Function*/
extern "C" s32 tstp_can_send_functional(int ADiagModuleIndex,byte* AReqArray,int AReqArraySize, int ATimeOutMs);
extern "C" s32 tstp_can_send_request(int ADiagModuleIndex, byte* AReqArray, int AReqArraySize, int ATimeOutMs);
extern "C" s32 tstp_can_request_and_response(int ADiagModuleIndex,byte* AReqArray, int AReqArraySize,byte* AReturnArray,int* AReturnArraySize, int ATimeOutMs);

extern "C" s32 tsdiag_can_session_control(int ADiagModuleIndex,byte ASubSession, byte ATimeoutMS);
extern "C" s32 tsdiag_can_routine_control(int ADiagModuleIndex, byte AARoutineControlType,u16 ARoutintID, int  ATimeoutMS);
extern "C" s32 tsdiag_can_communication_control(int ADiagModuleIndex, byte AControlType, int ATimeOutMs);
extern "C" s32 tsdiag_can_security_access_request_seed(int ADiagModuleIndex,int ALevel,
                                                                  byte* ARecSeed, int* ARecSeedSize, int ATimeoutMS);
extern "C" s32 tsdiag_can_security_access_send_key(int ADiagModuleIndex,int ALevel,byte* ASeed,int ASeedSize, int ATimeoutMS);
extern "C" s32 tsdiag_can_request_download(int ADiagModuleIndex,u32 AMemAddr,u32 AMemSize, int ATimeoutMS);
extern "C" s32 tsdiag_can_request_upload(int ADiagModuleIndex, u32 AMemAddr,u32 AMemSize, int ATimeoutMS);
extern "C" s32 tsdiag_can_transfer_data(int ADiagModuleIndex,byte* ASourceDatas, int ASize,int AReqCase, int ATimeoutMS);
extern "C" s32 tsdiag_can_request_transfer_exit(int ADiagModuleIndex,int ATimeoutMS);
extern "C" s32 tsdiag_can_write_data_by_identifier(int ADiagModuleIndex,u16 ADataIdentifier,byte* AWriteData,
                                                              int AWriteDataSize, int ATimeOutMs);
extern "C" s32 tsdiag_can_read_data_by_identifier(int ADiagModuleIndex, u16 ADataIdentifier, byte* AReturnArray,
 int* AReturnArraySize,int ATimeOutMs);



#pragma pack()

#endif
