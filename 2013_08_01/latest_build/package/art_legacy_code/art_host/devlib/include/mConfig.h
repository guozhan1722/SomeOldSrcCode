/*
 *  Copyright � 2001 Atheros Communications, Inc.,  All Rights Reserved.
 */

#ifndef	__INCmconfigh
#define	__INCmconfigh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifndef WIN32
#include <unistd.h>
#ifndef EILSEQ  
    #define EILSEQ EIO
#endif	// EILSEQ
#endif	// #ifndef WIN32

//#ifdef LINUX
//#undef ARCH_BIG_ENDIAN
//#endif
#include "wlanproto.h"

#include "athreg.h"
#include "mdata.h"
#include "manlib.h"
#include "mEeprom.h"
#include "ar5212/mEEPROM_d.h"
#include "ar2413/mEEPROM_g.h"

#include "art_ani.h"
#include "dk_cmds.h"

#include <time.h>

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef WIN32
#ifndef __ATH_DJGPPDOS__
#include <windows.h>
#else //__ATH_DJGPPDOS__
#include <dos.h>
#endif //__ATH_DJGPPDOS__
#endif	// #ifdef WIN32

#ifdef WIN32
#pragma pack (push, 1)
#endif

/* PCI Config space mapping */
#define F2_PCI_CMD				0x04		/* address of F2 PCI config command reg */
#define F2_PCI_CACHELINESIZE    0x0C        /* address of F2 PCI cache line size value */

/* PCI Config space bitmaps */
#define MEM_ACCESS_ENABLE		0x002       /* bit mask to enable mem access for PCI */
#define MASTER_ENABLE           0x004       /* bit mask to enable bus mastering for PCI */
#define MEM_WRITE_INVALIDATE    0x010       /* bit mask to enable write and invalidate combos for PCI */
#define SYSTEMERROR_ENABLE      0x100		/* bit mask to enable system error */


// General Macros
#define LIB_CFG_RANGE 256
#define LIB_REG_RANGE 65536


/*
#ifdef THIN_CLIENT_BUILD  // For predator
#define LIB_MEMORY_RANGE (100*1024)
#define CACHE_LINE_SIZE 16
#else
#define LIB_MEMORY_RANGE 1048576
#define CACHE_LINE_SIZE 0x20
#endif
*/

#define UNKNOWN_INIT_INDEX     0xffffffff

#define FRAME_BODY_SIZE     4096

#define PCDAC_REG_STRING		"bb_pcdac_"
#define FEZ_PCDAC_REG_STRING	"rf_pcdac"
#define GAIN_DELTA_REG_STRING	"rf_gain_delta"
#define TXPOWER_REG_STRING		"bb_powertx_"

#define NOT_READ			0xffffffff

#define LOWEST_XR_RATE_INDEX		15
#define HIGHEST_XR_RATE_INDEX		19

//new defines for compression
#define COMP_SCRATCH_BUFFER_SIZE    0x2200

//extract mdk packet header that gets added after 802.11 header
typedef struct mdkPacketHeader
{
	A_UINT16 pktType;
	A_UINT32 numPackets;
	//A_UINT16 qcuIndex;
} __ATTRIB_PACK MDK_PACKET_HEADER;


// __TODO__  JUST USING THIS TO TEST WITH pre 1.6 MDK
//
//extract mdk packet header that gets added after 802.11 header
//typedef struct mdkPacketHeaderX
//{
//	A_UINT16 pktType;
//	A_UINT32 numPackets;
//} MDK_PACKET_HEADERX;



#include "rate_constants.h"

// EEPROM defines
#define REGULATORY_DOMAIN_OFFSET 0xBF // EEPROM Location of the current RD
#define EEPROM_PROTECT_OFFSET 0x3F    // EEPROM Protect Bits
#define ATHEROS_EEPROM_OFFSET 0xC0    // Atheros EEPROM defined values start at 0xC0
#define ATHEROS_EEPROM_ENTRIES 64     // 128 bytes of EEPROM settings
#define REGULATORY_DOMAINS 4          // Number of Regulatory Domains supported
#define CHANNELS_SUPPORTED 5          // Number of Channel calibration groups supported
#define TP_SETTINGS_OFFSET 0x09       // Start location of the transmit power settings
#define TP_SETTINGS_SIZE 11           // Number of EEPROM locations per Channel group
#define TP_SCALING_ENTRIES 11         // Number of entries in the transmit power dBm->pcdac

typedef struct tpcMap
{
    A_UINT8 pcdac[TP_SCALING_ENTRIES];
    A_UINT8 gainF[TP_SCALING_ENTRIES];
    A_UINT8 rate36;
    A_UINT8 rate48;
    A_UINT8 rate54;
    A_UINT8 regdmn[REGULATORY_DOMAINS];
} TPC_MAP;

typedef struct mdk_eepMap
{
    A_BOOL eepromChecked;
    A_BOOL infoValid;
    A_BOOL turboDisabled;  // Turbo Disabled Bit
    A_UINT16 protect;      // Protect Bit Mask
    A_UINT16 version;      // Version field
    A_UINT16 antenna;      // Antenna Settings
    A_UINT16 biasCurrents; // OB, DB
    A_UINT8 currentRD;     // The currently set regulatory domain
    A_UINT8 thresh62;      // thresh62
    A_UINT8 xlnaOn;        // external LNA timing
    A_UINT8 xpaOff;        // external output stage timing
    A_UINT8 xpaOn;         // extneral output stage timing
    A_UINT8 rfKill;        // Single low bit signalling if RF Kill is implemented
    A_UINT8 devType;       // Relates the board's card type - PCI, miniPCI, CB
    A_UINT8 regDomain[REGULATORY_DOMAINS];
	A_UINT8 XPD;
	A_UINT8 XPD_GAIN;
        // Records the regulatory domain's that have been calibrated
    TPC_MAP tpc[CHANNELS_SUPPORTED];
        // Structs to the EEPROM Map;
} mdk_EEP_MAP;

#define EEPROM_ENTRIES_FOR_TXPOWER 32 // 64 bytes of EEPROM calibration data
#define EEPROM_TXPOWER_OFFSET 0xE0    // Power values start at 0xE0
#define MAX_TX_QUEUE	16
#define PROBE_QUEUE 2 //(MAX_TX_QUEUE - 1)

typedef struct txPowerSetup
{
	A_UINT16 eepromPwrTable[EEPROM_ENTRIES_FOR_TXPOWER];
    A_UINT16 tpScale;             // Scaling factor to be applied to max TP 
} TXPOWER_SETUP;

typedef struct txSetup
{
	A_UINT32		txEnable;     // Transmit has been setup
	A_UINT32		pktAddress;	  // physical address of transmit packet
	A_UINT32		descAddress;  // Physical address to start of descriptors				  
	A_UINT32		endPktAddr;	  // Address of special end packet
	A_UINT32		endPktDesc;	  // Address of descriptor of special end packet
	A_UINT32		numDesc;	  // number of descriptors created
    A_UINT32        dataBodyLen;  // number of bytes in pkt body
    A_UINT32        retryValue;   // copy of retry register value (to write back)
	WLAN_MACADDR    destAddr;     // destination address for packets
	TX_STATS_STRUCT txStats[STATS_BINS];   // Transmit stats 
    A_UINT32        haveStats;    // set to true when txStats contains stats values
    A_UINT32        broadcast;    // set to true if broadcast is enabled
    A_UINT32        contType;     // type of continuous transmit that has been enabled
	A_UINT16		dcuIndex;	  // DCU index of for this tx queue is setup
	A_UINT32        compBufferAddr; //address of scratch buffer to be used for compression
} TX_SETUP;

typedef struct rxSetup
{
	A_UINT32		rxEnable;     // Transmit has been setup
	A_UINT32		descAddress;  // Physical address to start of descriptors				  
	A_UINT32		bufferSize;	  // Size, in bytes, of packet
	A_UINT32		bufferAddress;// physical address to start of memory buffers
	A_UINT32		lastDescAddress;
	A_UINT32		numDesc;	  // number of descriptors created
	RX_STATS_STRUCT rxStats[MAX_TX_QUEUE][STATS_BINS];   // Receive stats 
	//RX_STATS_STRUCT rxStats[STATS_BINS];   // Receive stats 
    A_UINT32        haveStats;    // set to true when rxStats contains stats values
    A_UINT32        enablePPM;    // PPM was enabled by setup
	A_UINT32		rxMode;		  //receive mode
	A_UINT32		numExpectedPackets; 
	A_BOOL			overlappingDesc;
} RX_SETUP;

typedef struct memSetup
{
	A_UINT32      allocMapSize;  // Size of the allocation map
	A_UCHAR       *pAllocMap;    // The bitMap for tracking memory allocation	
	A_UINT16      *pIndexBlocks; // Number of blocks for the specified index
    A_BOOL        usingExternalMemory; // TRUE if using other driver memory map
} MEM_SETUP;

//++JC++
typedef struct AR2413_txgain_tbl {
  A_UCHAR desired_gain ;
  A_UCHAR bb1_gain ;
  A_UCHAR bb2_gain ;
  A_UCHAR if_gain ;
  A_UCHAR rf_gain ;
 } AR2413_TXGAIN_TBL; 
//++JC++

// devState Defines
#define INIT_STATE 1
#define CONT_ACTIVE_STATE 2
#define RESET_STATE 3

typedef	struct libDevInfo1
{
	A_UINT32 devNum;	   //Copy of the devices devNum
	DEVICE_MAP    devMap;      // The deviceMap given during deviceInitialize
	A_UINT32		 ar5kInitIndex;

	A_UINT32      macRev;      // The Mac revision number
	TXPOWER_SETUP txPowerData; // Struct for the EEPROM calibration data
	mdk_EEP_MAP       eepData;     // Struct for holding the new EEPROM calibration data
	WLAN_MACADDR  macAddr;	   // MAC address for this device
	WLAN_MACADDR  bssAddr;	   // BSS address
	TX_SETUP      tx[MAX_TX_QUEUE];		// Struct with data frame transmit info
	A_BOOL      txProbePacketNext;		// Struct with data frame transmit info
	A_UINT16    backupSelectQueueIndex;
	RX_SETUP      rx;		// Struct with data frame receive info
	MEM_SETUP     mem;         // Struct with memory setup info
	A_UINT32      turbo;       // Turbo state of this device
	A_UINT32      devState;    // Current state of the device
	A_UINT32      remoteStats; // set if have remote stats - specifies stats type
	TX_STATS_STRUCT txRemoteStats[MAX_TX_QUEUE][STATS_BINS]; // transmit stats from remote station 
	RX_STATS_STRUCT rxRemoteStats[MAX_TX_QUEUE][STATS_BINS]; // Receive stats from remote station 
	ATHEROS_REG_FILE *regArray;
	PCI_REG_VALUES   *pciValuesArray;
	A_UINT16         sizeRegArray;
	A_UINT16         sizePciValuesArray;
	A_UINT32         rfRegArrayIndex;
	PCI_REG_VALUES   *pRfPciValues;
	RF_REG_INFO		 rfBankInfo[NUM_RF_BANKS];
	MODE_INFO *pModeArray;
	A_UINT16		 sizeModeArray;
	A_BOOL           regFileRead;
	A_CHAR           regFilename[128];
	A_UINT32         aRevID;    //analog revID
	A_UINT32		 aBeanieRevID;
	A_UINT32         hwDevID;    //pci devID read from hardware
	A_UINT32         swDevID;    //more unique identifier of chipsets used by sw
	A_UINT32		 bbRevID;
	A_UINT32		 subSystemID;
	A_BOOL           wepEnable;
	A_UCHAR          wepKey;
	A_BOOL			 eePromLoad;		//set by user on whether to load eeprom
	A_BOOL			 eePromHeaderLoad;
	A_BOOL			eepromHeaderChecked;
	A_UINT16		sizePowerTable;
	A_UINT16		selQueueIndex;
	A_UINT32			start;						// start time for tx start/begin
	A_UINT16		enablePAPreDist;
	A_UINT16		paRate;
	A_UINT32		paPower;
	A_UCHAR			mode;
	MDK_PCDACS_ALL_MODES	*pCalibrationInfo;
	MDK_EEP_HEADER_INFO		*p16kEepHeader;
	MDK_TRGT_POWER_ALL_MODES *p16KTrgtPowerInfo;
//	MDK_TRGT_POWER_INFO		 *p16KTrgtPowerInfo;
	MDK_RD_EDGES_POWER		*p16KRdEdgesPower;
	A_UINT32		freqForResetDevice;			//frequency value set in resetDevice
	A_BOOL			adjustTxThresh;
	A_BOOL			specialTx100Pkt;
	A_BOOL			readThrBeforeBackoff;
	A_UINT32		suppliedFalseDetBackoff[3];
	A_UINT32		txDescStatus1;
	A_UINT32		txDescStatus2;
	A_UINT32		decryptErrMsk;
	A_UINT32		bitsToRxSigStrength;
	A_UINT32		rxDataRateMsk;
	RAW_DATA_STRUCT_GEN3 *pGen3RawData[3];
	EEPROM_FULL_DATA_STRUCT_GEN3 *pGen3CalData;
	LIB_PARAMS		libCfgParams;	
	A_INT32 mdkErrno;
    A_CHAR mdkErrStr[SIZE_ERROR_BUFFER];
    struct earHeader    *pEarHead;          /* All EAR information */
    A_UINT16		use_init;
	A_INT16         pwrIndexOffset;
	ART_ANI_LADDER  artAniLadder[3];  // 1 for each NI/BI/SI
	ART_ANI_SETUP   artAniSetup;
	A_UINT32        eepromStartLoc; // eeprom start location. default = 0x00, but for dual 11a set to 0x400
	A_INT32         maxLinPwrx4;    // 4 x max linear power at current operating channel. 
	                                // valid only for eep_map = 1 format
	A_BOOL			eepromHeaderApplied[4];
	RAW_DATA_STRUCT_GEN5 *pGen5RawData[3];
	EEPROM_FULL_DATA_STRUCT_GEN5 *pGen5CalData;
	A_UINT32		channelMasks;  //get passed into resetDevice via the turbo param
	A_UINT32		registerSpecificPatch;
	A_UINT32		spur_frequency_5g[EEPROM_SPUR_11A_SIZE];
	A_UINT32		spur_frequency_2p4g[EEPROM_SPUR_11G_SIZE];
} LIB_DEV_INFO;

typedef	struct pwrCtrlParams
{
	A_BOOL        initialized[3];	   //flag to capture the first time
	A_UINT32      rf_wait_I[3];       // pwr ctl params need to be stored.
	A_UINT32      rf_wait_S[3];       // pwr ctl params need to be stored.
	A_UINT32      rf_max_time[3];       // pwr ctl params need to be stored.
	A_UINT32      bb_active_to_receive[3];       // pwr ctl params need to be stored.
	A_UINT32      rf_pd_period_a[3];       // pwr ctl params need to be stored.
	A_UINT32      rf_pd_period_xr[3];       // pwr ctl params need to be stored.
	A_UINT32      rf_pd_delay_a[3];       // pwr ctl params need to be stored.
	A_UINT32      rf_pd_delay_xr[3];       // pwr ctl params need to be stored.
	A_UINT32      bb_tx_frame_to_tx_d_start[3];  
} PWR_CTL_PARAMS;

#define 	MDK_INIT_CODE		0
#define 	DRIVER_INIT_CODE	1

// Data prototypes

/* LIB_INFO structure will hold the library global information.
 */
typedef struct libInfo	{
	A_UINT32           devCount;                  // No. of currently connected devices 
	struct libDevInfo1 *pLibDevArray[LIB_MAX_DEV]; // Array of devinfo pointers 
} LIB_INFO;

// Macros to devMap defined functions
#define REGR(x, y) (gLibInfo.pLibDevArray[x]->devMap.OSregRead(x, y + (gLibInfo.pLibDevArray[x]->devMap.DEV_REG_ADDRESS)))
#define REGW(x, y, z) (gLibInfo.pLibDevArray[x]->devMap.OSregWrite(x, y + (gLibInfo.pLibDevArray[x]->devMap.DEV_REG_ADDRESS), z))

#if defined(SOC_LINUX)
#define sysRegRead(y) (gLibInfo.pLibDevArray[devNum]->devMap.OSapRegRead32(devNum, y))
#define sysRegWrite(y, z) (gLibInfo.pLibDevArray[devNum]->devMap.OSapRegWrite32(devNum, y, z))
#endif

// Func Prototypes
int mError(A_UINT32 devNum, A_UINT32 error, const char * format, ...);
A_UINT32 reverseBits(A_UINT32 val, int bit_count);
A_BOOL checkDevNum(A_UINT32 devNum); 
A_BOOL parseAtherosRegFile(LIB_DEV_INFO *pLibDev, char *filename);
A_BOOL createPciRegWrites(A_UINT32 devNum);
A_BOOL  setupEEPromMap(A_UINT32 devNum);
A_INT32 findDevTableEntry(A_UINT32 devNum);

//extern void memDisplay(A_UINT32 devNum, A_UINT32 address, A_UINT32 nWords);

void griffin_cl_cal(A_UINT32 devNum);   //++JC++
void sleep_cal(clock_t wait );          //++JC++

#define A_MIN(x, y)  (((x) < (y)) ? (x) : (y))
// Integer divide of x/y with round-up
#define A_DIV_UP(x, y) ( ((x) + (y) - 1) / y)
#define I2DBM(x) ((A_UCHAR)((x * 2) + 3))


#include "common_defs.h"

extern A_UINT32 checkSumLengthLib; //default to 16k
extern A_UINT32 eepromSizeLib;
#ifndef THIN_CLIENT_BUILD
// Global externs
extern LIB_INFO gLibInfo;
extern A_UINT32 lib_memory_range;
extern A_UINT32 cache_line_size;
extern PCI_VALUES pciValues[MAX_PCI_ENTRIES];
#endif

#ifdef WIN32
#pragma pack (pop)
#endif	

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INCmconfigh
