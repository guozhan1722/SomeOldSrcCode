/*
 *  Copyright � 2002 Atheros Communications, Inc.,  All Rights Reserved.
 */
/* mdata.h - Type definitions needed for data transfer functions */

/* Copyright 2000, T-Span Systems Corporation */
/* Copyright 2000, Atheros Communications Inc. */

#ifndef	__INCmdatah
#define	__INCmdatah

#if 0
#ident  "ACI $Id: //depot/sw/src/dk/mdk/manlib/mdata.h#1 $, $Header: //depot/sw/src/dk/mdk/manlib/mdata.h#1 $"
#endif

//#ifdef LINUX
//#undef ARCH_BIG_ENDIAN
//#endif

#include "wlanproto.h"

#define PPM_DATA_SIZE           56
#define BUFF_BLOCK_SIZE			0x100  /* size of a buffer block */
//#define MAX_RETRIES             0x000fffff
#define MAX_RETRIES             0x0000000f
#define MDK_PKT_TIMEOUT         0x50
#define MAX_PKT_BODY_SIZE       4031   // 4095 - 64 bytes of fifo used for internal comm (per jsk)
#define STATS_BINS              21    


//802.11 related 
#define FCS_FIELD				4		   /* size of FCS */				
#define WEP_IV_FIELD			4		   /* wep IV field size */
#define WEP_ICV_FIELD			4		   /* wep ICV field size */
#define WEP_FIELDS	(WEP_IV_FIELD + WEP_ICV_FIELD) /* total size of wep fields needed */
#define ADDRESS1_START          4
#define ADDRESS2_START			10
#define ADDRESS3_START          16
#define SEQ_CONTROL_START		22

#define MAX_FRAME_INFO_SIZE    100

//descriptor offsets and field definitions
#define LINK_POINTER			0
#define BUFFER_POINTER			4
#define FIRST_CONTROL_WORD		8
#define SECOND_CONTROL_WORD		12
#define FIRST_STATUS_WORD		16
#define SECOND_STATUS_WORD		20
#define DESC_DONE				0x00000001
#define DESC_MORE				0x00001000
#define DESC_FRM_RCV_OK			0x00000002
#define DESC_FRM_XMIT_OK		0x00000001
#define DESC_TX_INTER_REQ_START	29
#define DESC_TX_INTER_REQ		(1<<DESC_TX_INTER_REQ_START)
#define DESC_RX_INTER_REQ_START	13
#define DESC_RX_INTER_REQ		(1<<DESC_RX_INTER_REQ_START)
#define DESC_CRC_ERR			0x00000004
#define DESC_FIFO_UNDERRUN		0x00000004
#define DESC_EXCESS_RETIES		0x00000002
#define DESC_DATA_LENGTH_FIELDS 0x00000fff
#define DESC_DECRYPT_ERROR      0x00000010
#define BITS_TO_SHORT_RETRIES	4
#define BITS_TO_LONG_RETRIES	8
#define BITS_TO_TX_XMIT_RATE    18
#define BITS_TO_TX_HDR_LEN      12
#define BITS_TO_TX_ANT_MODE     25
#define BITS_TO_TX_SIG_STRENGTH 13
#define BITS_TO_RX_DATA_RATE    15
#define BITS_TO_RX_SIG_STRENGTH 19
#define BITS_TO_ENCRYPT_VALID   30
#define BITS_TO_ENCRYPT_KEY     13
#define BITS_TO_NOACK           23		// WORD - 3
#define RETRIES_MASK			0xf
#define SIG_STRENGTH_MASK		0xff
#define DATA_RATE_MASK          0xf

//New defines for Venice, may move these somewhere else later
#define THIRD_CONTROL_WORD		    16
#define FOURTH_CONTROL_WORD		    20
#define FIRST_VENICE_STATUS_WORD	24
#define SECOND_VENICE_STATUS_WORD   28

#define DESC_COMPRESSION_DISABLE	0x3
#define DESC_ENABLE_CTS				0x80000000
#define BITS_TO_RTS_CTS_RATE		20
#define BITS_TO_COMPRESSION			25
#define BITS_TO_DATA_TRIES0			16
#define BITS_TO_TX_DATA_RATE0		0
#define BITS_TO_DATA_TRIES1			20
#define BITS_TO_TX_DATA_RATE1		5
#define BITS_TO_DATA_TRIES2			24
#define BITS_TO_TX_DATA_RATE2		10
#define BITS_TO_DATA_TRIES3			28
#define BITS_TO_TX_DATA_RATE3		15
#define BITS_TO_VENICE_NOACK		24
#define VENICE_DESC_DECRYPT_ERROR   0x00000008
#define VENICE_BITS_TO_RX_SIG_STRENGTH 20
#define VENICE_DATA_RATE_MASK       0x1f
//End of Venice Defines




#define RX_NORMAL				1
#define RX_FIXED_NUMBER			0
#define RX_PROMISCUOUS_MODE		2

//temp redefinition of some register bits that changed
//#undef F2_STA_ID1_DESC_ANT
//#define F2_STA_ID1_DESC_ANT    0x01000000 // Descriptor selects antenna
//#undef F2_STA_ID1_DEFAULT_ANT
//#define F2_STA_ID1_DEFAULT_ANT 0x02000000 // Toggles the antenna setting

//#undef F2_IMR
//#define F2_IMR  0x00a0  // MAC Primary interrupt mask register





typedef struct mdkAtherosDesc		/* hardware descriptor structure */
	{
	A_UINT32	nextPhysPtr;		/* phys address of next descriptor */
	A_UINT32	bufferPhysPtr;		/* phys address of this descriptior buffer */
	A_UINT32    hwControl[2];		/* hardware control variables */
	A_UINT32    hwStatus[2];		/* hardware status varables */
	A_UINT32    hwExtra[2];			/* Added for Venice support */
	A_UINT8    dummy_word[0x6c];		/*added by Dudi for RMA7265 to make desciptor not align 0x20 boundary*/
	} MDK_ATHEROS_DESC;

typedef struct mdkVeniceDesc		/* To help with readability also define this for venice */
	{
	A_UINT32	nextPhysPtr;		/* phys address of next descriptor */
	A_UINT32	bufferPhysPtr;		/* phys address of this descriptior buffer */
	A_UINT32    hwControl[4];		/* hardware control variables */
	A_UINT32    hwStatus[2];		/* hardware status varables */
	} MDK_VENICE_DESC;

typedef struct sigStrengthStats
	{
	volatile A_INT8 rxAvrgSignalStrength;	/* Note order is important here */
	volatile A_INT8 rxMinSigStrength;		/* the ave signal strength, max and */
	volatile A_INT8 rxMaxSigStrength;		/* min stats need to be first */
	volatile A_INT32 rxAccumSignalStrength;
	volatile A_UINT32 rxNumSigStrengthSamples;
	} SIG_STRENGTH_STATS;

typedef struct MDataFnTable
{
	int (*sendTxEndPacket)(A_UINT32 devNum,  A_UINT16 queueIndex);
	int (*setupAntenna)(A_UINT32 devNum, A_UINT32 antenna, A_UINT32* antModePtr);
	void (*setRetryLimit)(A_UINT32 devNum, A_UINT16 queueIndex);
	void (*txBeginConfig)(A_UINT32 devNum);
	void (*beginSendStatsPkt)(A_UINT32 devNum, A_UINT32 DescAddress);
	void (*setDescriptor)(A_UINT32, MDK_ATHEROS_DESC*, A_UINT32, A_UINT32, 
								A_UINT32, A_UINT32, A_UINT32);

} MDATA_FNTABLE;

// some temp info need to hold while processing stats
typedef struct rxStatsTempInfo
{
	A_UINT32	descToRead;		// address of next descriptor to check complete	
    A_UINT32    descRate;       // Rate for this descriptor
	A_UINT32	status1;		// first status word of descriptor
	A_UINT32	status2;		// second status word of descriptor
	A_UINT32	bufferAddr;		// adddress of buffer containing packet
	A_UINT32	totalBytes;		// count of number of bytes received
	A_BOOL   	gotHeaderInfo;			// set when get the header info for duplicate processing 
	A_BOOL		controlFrameReceived;	// set to true if this is a control frame 
	A_BOOL		illegalBuffSize;		// set if not able to get addr and sequence because
										// not fully contained in first buffer of packet
										//  may add support for this later 
	WLAN_MACADDR addrPacket;			// mac address expect to see on received packet 
	A_BOOL		 gotMacAddr;			// set to true when we have the address for received packets 
	SEQ_CONTROL	 seqCurrentPacket;		// sequence control of current packet received 
	A_BOOL		 retry;					// retry bit of header 
	A_BOOL		 badPacket;				// set on a bad address match 
	A_UINT32	 lastRxSeqNo;			// Last frame's sequence number 
	A_UINT32	 lastRxFragNo;			// Last frame's fragment number 
	A_UINT32	 oneBeforeLastRxSeqNo;	// Memory of the sequence no. before last
	SIG_STRENGTH_STATS sigStrength[STATS_BINS];		// accumulated signal strength stats
    double       ppmAccum[STATS_BINS];              // accumulated ppm value
    A_INT32      ppmSamples[STATS_BINS];            // total number of ppm samples accumulated
	A_UINT16	 mdkPktType;			// type of packet received, ie normal, last or stats packet
    A_UCHAR      *pCompareData;         // pointer to data to use for pkt comparison
	A_UINT16	 qcuIndex;				// QCU index from where the pkt came
	A_UCHAR		frame_info[MAX_FRAME_INFO_SIZE];
	A_UINT32 	frame_info_len;
	A_UCHAR		ppm_data[MAX_FRAME_INFO_SIZE];
	A_UINT32 	ppm_data_len;
	MDK_ATHEROS_DESC desc;
	A_UINT32    descNumber;
	A_UINT32    descPreviousTime;
	A_UINT32    lastDescRate;
	A_UINT32    bitsReceived;
	A_UINT32    totalTPBytes;		//total number of bytes for tp calculates
									//ie removes 802.11 header, crc bytes etc
    A_BOOL      addressMismatch;
} RX_STATS_TEMP_INFO;

typedef struct txStatsTempInfo
{
	A_UINT32	descToRead;		// address of next descriptor to check complete	
    A_UINT32    descRate;       // Rate for this descriptor
	A_UINT32	status1;		// first status word of descriptor
	A_UINT32	status2;		// second status word of descriptor
	A_UINT32	totalBytes;		// count of number of bytes transmitted
	SIG_STRENGTH_STATS sigStrength[STATS_BINS];		//  accumulated signal strength stats
	A_UCHAR		frame_info[MAX_FRAME_INFO_SIZE];
	MDK_ATHEROS_DESC desc;
} TX_STATS_TEMP_INFO;

void createTransmitPacket (A_UINT32 devNum,  A_UINT16 mdkType, A_UCHAR *dest,  A_UINT32 numDesc, 
	A_UINT32 dataBodyLength, A_UCHAR *dataPattern, A_UINT32 dataPatternLength, A_UINT32 broadcast, 
 	A_UINT16 queueIndex, A_UINT32 *pPktSize, A_UINT32 *pPktAddress);

void
sendEndPacket
(
 A_UINT32 devNum,
 A_UINT16 queueIndex
);

void
createEndPacket
(
 A_UINT32 devNum,
 A_UINT16 queueIndex,
 A_UCHAR  *dest,
 A_UINT32 antMode,
 A_BOOL probePkt
);

void internalRxDataSetup
(
 A_UINT32 devNum, 
 A_UINT32 numDesc, 
 A_UINT32 dataBodyLength,
 A_UINT32 enablePPM, 		
 A_UINT32 mode
);

extern void zeroDescriptorStatus ( A_UINT32	devNumIndex, MDK_ATHEROS_DESC *pDesc, A_UINT32 swDevID );
extern void writeDescriptors ( A_UINT32	devNumIndex, A_UINT32	descAddress, MDK_ATHEROS_DESC *pDesc, A_UINT32   numDescriptors);
extern void writeDescriptor ( A_UINT32	devNum, A_UINT32	descAddress, MDK_ATHEROS_DESC *pDesc);

#endif //__INCmdatah
