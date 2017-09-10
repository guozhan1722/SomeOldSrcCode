/* test.c - contians an example test harness using the manufacturing library */

/* Copyright (c) 2000 Atheros Communications, Inc., All Rights Reserved */
#if 0
#ident  "ACI $Id: //depot/sw/branches/ART_V53/sw/src/dk/mdk/devmld/test.c#35 $, $Header: //depot/sw/branches/ART_V53/sw/src/dk/mdk/devmld/test.c#35 $"
#endif

#ifdef __ATH_DJGPPDOS__
#include <unistd.h>
#ifndef EILSEQ  
    #define EILSEQ EIO
#endif	// EILSEQ

 #define __int64	long long
 #define HANDLE long
 typedef unsigned long DWORD;
 #define Sleep	delay
 #include <bios.h>
 #include <dir.h>
#endif	// #ifdef __ATH_DJGPPDOS__

#ifndef REGULATORY_REL
#ifdef _WINDOWS
 #include <windows.h>
#endif
#include "common_hw.h"
#include "ar5211/ar5211reg.h"
#ifdef JUNGO
#include "mld.h"
#endif

#ifdef LINUX
#include "linux_ansi.h"
#endif

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#ifndef LINUX
#include <conio.h>
#include <io.h>
#endif
#include <string.h>
#include <ctype.h>
#include "wlantype.h"
#include "wlanproto.h"
#include "athreg.h"
#include "manlib.h"     /* The Manufacturing Library */
#include "manlibInst.h" /* The Manufacturing Library Instrument Library extension */

#include "art_if.h"
#include "pci.h"        /* PCI Config Space definitions */
#include "art_ani.h"
#include "test.h"
#include "cmdTest.h"
#include "parse.h"
#include "dynamic_optimizations.h"
#include "maui_cal.h"
#include "ar5523defs.h"
#include "ar2413reg.h"
#include "rate_constants.h"
#include "stats_routines.h"

#ifndef __ATH_DJGPPDOS__
#include "mlibif.h"     /* Manufacturing Library low level driver support functions */
#include "dk_cmds.h"
#else
#include "mlibif_dos.h"
#endif

#ifdef LINUX
#include <unistd.h>
#endif

#define EEPROM_BLOCK_SIZE 256

#define OFDM_11G_IDX		20
#define CCK_11G_IDX			5
#define OFDM_CCK_DEF_IDX	0

#include "ear_externs.h"

 A_UINT32 sent_bytes=0, received_bytes=0;

extern A_UINT16 curr_pwr_index_offset;

extern GAIN_OPTIMIZATION_LADDER gainLadder;
extern GAIN_OPTIMIZATION_LADDER gainLadder_derby2;
extern GAIN_OPTIMIZATION_LADDER gainLadder_derby1;
GAIN_OPTIMIZATION_LADDER *pCurrGainLadder;

extern A_BOOL printLocalInfo;
extern  A_UINT32	devlibModeFor[3];
extern  A_UINT32	calModeFor[3];
	   
// extern declarations for dut-golden sync
extern ART_SOCK_INFO *artSockInfo;
extern ART_SOCK_INFO *pArtPrimarySock;
extern ART_SOCK_INFO *pArtSecondarySock;
extern A_UINT32 **EEPROM_DATA_BUFFER;

/* === Functional Declarations === */
static A_BOOL parseCmdLine(A_INT32 argc,A_CHAR *argv[]);
static void contMenu(A_UINT32 devNum);
static void contRxMenu(A_UINT32 devNum);
static void linkMenu(A_UINT32 devNum);
static void sniffMenu(A_UINT32 devNum);
static void utilityMenu(A_UINT32 devNum);
#if 0
static void changeDomain(A_UINT32 devNum); 
#endif
static A_BOOL parseConfig(void);
static A_BOOL	processCommonOptions( A_UINT32 devNum, A_INT16 inputKey);
static A_BOOL setRxGain(A_UINT32 devNum);
static A_BOOL setRxGain_11bg(A_UINT32 devNum);
static void setRegistersFromConfig(A_UINT32 devNum);
static void updateConfigFromRegValues(A_UINT32 devNum);
static A_BOOL initTest(A_UINT32 devNum);
static A_BOOL setupMode(void);
static void printDeviceInfo(A_UINT32 devNum);
A_BOOL EEPROM_Routine(A_UINT32 devNum);	
static void eraseBlock(A_UINT32 devNum);
void Display_EEPROM_menu(void);			
#if 0
static A_UINT16 promptForCardType(A_UINT16 *pSubSystemID);
#endif
static A_BOOL parseCmdChannelList(A_CHAR *listString);
static A_BOOL updateStructFromLabel(void);
#ifndef __ATH_DJGPPDOS__
void progBlankEEPROM(A_UINT32 devNum);
A_BOOL setEepFile(A_UINT32 devNum);
A_BOOL addToCfgTable(A_UINT16 subSystemID,A_CHAR *pFilename, A_CHAR *pEarFilename);
#endif //__ATH_DJGPPDOS__
void printConfigSettings(A_UINT32 devNum);
A_BOOL supportMultiModes(SUPPORTED_MODES *pModes);
void throughputMenu(A_UINT32 devNum);
void compute_EEPROM_Checksum(A_UINT32 devNum,A_UINT32 location);
void getSupportedModes (A_UINT32 devNum, SUPPORTED_MODES *pModes);
A_UINT32 getNextMode (SUPPORTED_MODES *pModes);
A_UINT32 getFirstMode (SUPPORTED_MODES *pModes);

extern void topCalibrationEntry(A_UINT32 *pdevNum_inst1, A_UINT32 *pdevNum_inst2) ;

extern void displayDomain(A_UINT32 domain);

extern YIELD_LOG_STRUCT yldStruct;

#ifdef __ATH_DJGPPDOS__
extern void pci_scan(void);
extern void dump_pcicfg_queue(void);
extern void free_pcicfg_queue(void);

#define _makepath(path,drive,dir,name,ext)\
	fnmerge(path,drive,dir,name,ext)
#endif


static A_UCHAR  bssID[6]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static A_UCHAR  rxStation[6] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};	// DUT
static A_UCHAR  txStation[6] = {0x7f, 0xff, 0xff, 0xff, 0xff, 0xff};	// Golden
//                                                     1L    2L    2S   5.5L  5.5S  11L   11S   
A_UCHAR  DataRate[] =  {6, 9, 12, 18, 24, 36, 48, 54, 0xb1, 0xb2, 0xd2, 0xb5, 0xd5, 0xbb, 0xdb, 
//                        XR0.25  XR0.5  XR1   XR2   XR3
							0xea, 0xeb,  0xe1, 0xe2, 0xe3};
//                                                     1L    2L    2S   5.5L  5.5S  11L   11S   
const A_UCHAR DataRateCode[] = {11, 15, 10, 14, 9, 13, 8, 12,    
//                       1L   2L   2S   5.5L 5.5S 11L  11S
                         0x1b,0x1a,0x1e,0x19,0x1d,0x18,0x1c,
//						 0.25 0.5 1  2  3
						 3,   7,  2, 6, 1
};

const A_CHAR  *DataRateStr[] = {" 6 Mbps", " 9 Mbps", "12 Mbps",
	        "18 Mbps", "24 Mbps", "36 Mbps", "48 Mbps", "54 Mbps",
            "1 Mbps long", "2 Mbps long", "2 Mbps short",
			"5.5 Mbps long", "5.5 Mbps short", "11 Mbps long", "11 Mbps short",
			"XR 0.25 Mbps", "XR 0.5 Mbps", "XR 1 Mbps", "XR 2 Mbps", "XR 3 Mbps"};
const A_CHAR  *DataRateStrTurbo[] = {" 12 Mbps", " 18 Mbps", "24 Mbps",
	        "36 Mbps", "48 Mbps", "72 Mbps", "96 Mbps", "108 Mbps",
            "1 Mbps long", "2 Mbps long", "2 Mbps short",
			"5.5 Mbps long", "5.5 Mbps short", "11 Mbps long", "11 Mbps short",
//            "2 Mbps long", "4 Mbps long", "4 Mbps short",
//			"11 Mbps long", "11 Mbps short", "22 Mbps long", "22 Mbps short",
			"XR 0.5 Mbps", "XR 1 Mbps", "XR 2 Mbps", "XR 4 Mbps", "XR 6 Mbps"};
const A_CHAR  *DataRateStrHalf[] = {" 3 Mbps", " 4.5 Mbps", "6 Mbps",
	        "9 Mbps", "12 Mbps", "18 Mbps", "24 Mbps", "27 Mbps",
            "0.5 Mbps long", "1 Mbps long", "1 Mbps short",
			"2.75 Mbps long", "2.75 Mbps short", "5.5 Mbps long", "5.5 Mbps short",
			"XR 0.125 Mbps", "XR 0.25 Mbps", "XR 0.5 Mbps", "XR 1 Mbps", "XR 1.5 Mbps"};

const A_CHAR  *DataRateStrQuarter[] = {" 1.5 Mbps", " 2.75 Mbps", "3 Mbps",
	        "4.5 Mbps", "6 Mbps", "9 Mbps", "12 Mbps", "13.5 Mbps",
            "0.25 Mbps long", ".5 Mbps long", ".5 Mbps short",
			"1.375 Mbps long", "1.375 Mbps short", "2.75 Mbps long", "2.75 Mbps short",
			"XR 0.065 Mbps", "XR 0.125 Mbps", "XR 0.25 Mbps", "XR .5 Mbps", "XR .75 Mbps"};

const A_CHAR  *DataRate_11b[] =  {"1 Mbps long", "1 Mbps long", 
									"2 Mbps long", "2 Mbps short",
									"5.5 Mbps long", "5.5 Mbps short",
									"11 Mbps long", "11 Mbps short",
									"1 Mbps long", //repeat strings incase 11g CCK
									"2 Mbps long", "2 Mbps short",
									"5.5 Mbps long", "5.5 Mbps short",
									"11 Mbps long", "11 Mbps short"};

A_UINT32 swDeviceID;
A_UINT32 hwDeviceID;
A_UINT32 subSystemID;
A_UINT32 macRev;
A_UINT32 bbRev;
A_UINT32 analogProdRev;
WLAN_MACADDR  macAddr;
A_BOOL printLocalInfo;
A_BOOL progProm = 0;
A_UINT16 gainIMax = GAINI_MAX;
A_UINT16 gainIMin = GAINI_MIN;

A_BOOL thin_client = FALSE;
A_BOOL usb_client = FALSE;
A_UINT32 userEepromSize = 0x400;
A_BOOL sizeWarning;

A_CHAR  calsetupFileName[128] = "";
char *machName = NULL;
A_UINT32 checkSumLength=0x400; //default to 16k
A_UINT32 eepromSize=0;
A_UINT32 glbl_devNum=0;

//Arrays needed for calculating the rx gain
//static double antennaArr[] = {4.7, -6.3};
//static double rfvgaGaindBArr[] = {2.73, -9.15, 19.32, 18.57}; // [rfgain1,rfatten0]
//static double ifvgaGaindBArr[] = {-1.66, 2.42, 7.23, 11.9, 16.3, 16.3, 16.3, -6.09};
//static A_INT32 pga1GainArr[]  = {0, 6, 12, 18};
//static A_INT32 pga2GainArr[]  = {0, 6, 12, 18};
//static A_INT32 pga3GainArr[]  = {0, 1, 2, 3, 4, 5};

//defaults for initial configuration, although a lot of these will be overwritten
//based on the contents of the external configuration file and the register file.
MLD_CONFIG configSetup = 
{
	5600,						//channel
	0,							//eeprom load
	0,							//eeprom load override
	0,							//eeprom Header load
	"",							//create fez config file
	"",							//maui sombrero config file
	"",							//maui sombrero beanie config file
	"",							//oahu sombrero config file
	"",							//venice sombrero config file
	NULL,						//pointer to current config file 
	MAX_SOM_CHANNEL,			//max channel 5G
	MIN_CHANNEL,				//min channel 5G
	MAX_2G_CHANNEL,				//Max channel 2G
	MIN_2G_CHANNEL,				//Min channel 2G
	0,							//power override on or off
	0,							//external power on or off
	0,							//xpdGainIndex
//	3,							//xpdGainIndex2
	0,							//xpdGainIndex2
	1,							//applyXpdGain
	0,							//data rate index
	6,							//data rate index throughput
	USE_REG_FILE,				//pcdac value, initially use values in reg file
	NO_PWR_CTL,					//power control method. start with none.
	1,							//ob						
	1,							//db
	1,							//b_ob						
	1,							//b_db
	50,							//gainI
	CONT_TX99,					//continuous mode
	USE_DESC_ANT | DESC_ANT_A,	//antenna
	PN9_PATTERN,				//data pattern
	1,							//turbo
	USE_REG_FILE,				//rxGain
	0,							//rf_gainBoost
	0,							//overwriteRxGain
	0,							//remote
	1,							//remote_exec
	".",						//machname
	1,							//instance
	0,							//user instance override
	1,							//validInstance
	MODE_11A,					//mode			
    0,                          //use_init
	5360,						//channel5
	2412,						//channel2_4
	USE_REG_FILE,				//power output
	USE_REG_FILE,				//rxGain 5GHz
	USE_REG_FILE,				//rxGain 2.4GHz
	1,							//packet interleave
	0,							//logging
	"",							//log file
	0x0000,						// dut SSID. an illegal value as default.
//	"BLANK",					//dut card type
//	"BLANK",					//cmd line dut card type
	0,							//don't open all 2GHz channels
	0xff,						//rate mask
	"",							//eep File Directory
	{0,							//cfg table num elements
	NULL,						//cfg table ptr current
	NULL},						//cfg table ptr elements
	0,							//blank eep subsystemID
	0,							//cmd line subsystemID
	0,							//cmd line subsystemID
	1500,						//packet size for throughput
	MIN_NUM_RETRIES,			//num retries for throughput
	MAX_NUM_PKTS,				//num packets for throughput
	0,							//use unicast packets for throughput
	0,							//init primary ART to not AP
	1,							//enablePrint
	0,							//num slots
	0,							//eeprom contains valid calibration data 
	0,							// use target powers
	FALSE,						//cmd line Test
	0,							//cmd line test mask
	NULL,						//test channel list
	0,							//Num test channels
	ANTENNA_A_MASK,				//antenna Mask
	ANTENNA_A_MASK,            	//gold antenna mask
	0,							//iterations
	{{0}},							//beacon bssid
	0,							//range Logging
	"",							//range Logfile
	1000,						//link packet size
	100,						//link num packets
	REF_CLK_DYNAMIC,			//default refClock
	0,							//beanie2928Mode
	5,							//5gChannelStep size
	0,							//enable XR
	0,							//load ear
	0,							//eep file version
	0,                          //ear file version
	0,                          //ear file identifier <filename_1.ear ==> 1>
	DO_OFSET_CAL | DO_NF_CAL,   //hw calibration
//	ART_ANI_ENABLED,			//ART ANI enabled by default
	ART_ANI_DISABLED,			//ART ANI disabled initially
	ART_ANI_REUSE_ON,			//ART ANI levels reuse on by default
	{0,0,0},					//ART ANI levels
	100,						// max RX gain
	0,							// min RX gain
	FALSE,						// userDutIdOverride
	0x00,                       // eeprom2StartLocation
	0,                          // computeCalsetupName
	"",							// eep backup filename
	"",							// eep restore filename
	0,							// compare single value
	0,							// apply ctl limit
	0,							// ctl to apply
	0,							// debugInfo flag
	"INVALID",		     		// manufName
	"",                         // yieldLogFile 
	0,							// enable label scheme
	0,                      // Scramble mode on by default
	0,							// print pci writes
	0,							//quarter channel
	"atheros-eep.txt",			// EEP file for cb/mb cards.
	"atheros-usb-eep.txt",		// EEP file for UB cards.
	"atheros-express-eep.txt",   // EEP file for PCI_express
	0,							// override pll value
	0,							// pll value
	0,							// no eeprom unlock (ie default to allow write).
    5,                          // sniffTimeout in seconds
	2000,                       // sniffBufferSize
	1000,                       // sniffNumBuffers
#ifdef __ATH_DJGPPDOS__
	-1,			// bus number to scan. -1 to scan all.
	-1,          //deviceInPCI
#endif
	0,							//enable compression
	"",							// SSID
};

//lookup table for when mapping the the pci config space first time
EEPROM_PCICONFIG_MAP fullMapping[] = {
	{ 0x00, 0x00, 0x01 },    //deviceID, vendorID
	{ 0x08, 0x02, 0x03 },    //23:8 of class code, 7:0 of class code and revisionID
	{ 0x28, 0x06, 0x05 },     //bits 31:16 of cis_ptr, bits 15:0 cis_ptr
	{ 0x2c, 0x07, 0x08 },    //subsystemID, subvendorID
	{ 0x3c, 0x09, 0x0a },	//max_lat and min_gnt, int_pin, reserved
//	{ 0x40, 0x0c, READ_CONFIG }, //pm_capabilities, keep default config value
//now doing the wow configuration in remapPCIConfigInfo
};

A_UINT32 numMappings = sizeof(fullMapping)/sizeof(EEPROM_PCICONFIG_MAP);

typedef struct {
	A_CHAR		cardName[20];
	A_UINT32    chipID;
	A_UINT32	productID;
	A_UINT32	subsystemID;
	A_UINT32	numEthMacs;
	A_UINT32	numWlanMacs;
} LABEL_INFO_STRUCT;

//for now create a static array of card label lookups.  Need to replace this with 
//an array parsed from the label.txt file
LABEL_INFO_STRUCT labelTable[] = {
	{ "CB32", NO_CHIP_IDENTIFIER, 0, 0x1031, 0, 1 },
	{ "CB31", NO_CHIP_IDENTIFIER, 1, 0x1030, 0, 1 },
	{ "MB32", NO_CHIP_IDENTIFIER, 3, 0x2031, 0, 1 },
	{ "MB31", NO_CHIP_IDENTIFIER, 4, 0x2030, 0, 1 },
	{ "AP30", NO_CHIP_IDENTIFIER, 6, 0xa034, 1, 2 },
	{ "AP31", NO_CHIP_IDENTIFIER, 7, 0xa033, 1, 1 },
	{ "AP33", NO_CHIP_IDENTIFIER, 8, 0xa037, 1, 1 },
	{ "AP36", NO_CHIP_IDENTIFIER, 9, 0xa038, 1, 1 },
	{ "AP38", NO_CHIP_IDENTIFIER, 10, 0xa035, 1, 1 },
	{ "AP39", NO_CHIP_IDENTIFIER, 11, 0xa036, 1, 1 },
	{ "CB42", NO_CHIP_IDENTIFIER, 26, 0x1042, 0, 1 },
	{ "CB41", NO_CHIP_IDENTIFIER, 27, 0x1041, 0, 1 },
	{ "CB43", NO_CHIP_IDENTIFIER, 28, 0x1043, 0, 1 },
	{ "MB42", NO_CHIP_IDENTIFIER, 29, 0x2042, 0, 1 },
	{ "MB41", NO_CHIP_IDENTIFIER, 30, 0x2041, 0, 1 },
	{ "AP43", NO_CHIP_IDENTIFIER, 31, 0xa043, 1, 1 },
	{ "AP48", NO_CHIP_IDENTIFIER, 32, 0xa048, 1, 1 },
	{ "AP41", NO_CHIP_IDENTIFIER, 33, 0xa041, 1, 1 },
	{ "MB43", NO_CHIP_IDENTIFIER, 36, 0x2043, 0, 1 },
	{ "MB44", NO_CHIP_IDENTIFIER, 37, 0x2044, 0, 1 },
	{ "TB91", NO_CHIP_IDENTIFIER, 38, 0x2049, 0, 1 },
	{ "UB51", NO_CHIP_IDENTIFIER, 44, 0xb051, 0, 1 },
	{ "UB52", NO_CHIP_IDENTIFIER, 43, 0xb052, 0, 1 },
	{ "AV10", NO_CHIP_IDENTIFIER, 42, 0x2052, 0, 1 },
	{ "CB51", 4, 40, 0x1051, 0, 1 },	//griffin
	{ "CB51", 9, 54, 0x1051, 0, 1 },	//griffin
	{ "CB51", 0, 46, 0x1052, 0, 1 },	//griffin lite
	{ "CB53", 4, 45, 0x1053, 0, 1 },	//griffin
	{ "CB53", 0, 47, 0x1054, 0, 1 },    //griffin lite
	{ "CB55", 0, 48, 0x1055, 0, 1 },	//nala
	{ "MB51", 4, 41, 0x2051, 0, 1 },	//griffin
	{ "MB51", 9, 53, 0x2051, 0, 1 },	//griffin
	{ "MB51", 0, 49, 0x2052, 0, 1 },	//griffin lite
	{ "MB53", 4, 50, 0x2053, 0, 1 },	//griffin 
	{ "MB53", 0, 51, 0x2054, 0, 1 },	//griffin lite
	{ "MB55", 0, 52, 0x2055, 0, 1 },	//nala
	{ "CU63", NO_CHIP_IDENTIFIER, 66, 0xb063, 0, 1 },
	{ "MB62", 9, 55, 0x2062, 0, 1 },	//eagle
	{ "MB62", 4, 58, 0x2062, 0, 1 },	//eagle
	{ "CB62", 4, 57, 0x1062, 0, 1 },	//eagle
	{ "MB62", 0, 60, 0x2063, 0, 1 },	//eagle-lite
	{ "CB62", 0, 61, 0x1063, 0, 1 },	//eagle-lite
	{ "AP51", 4, 56, 0xa051, 1, 1 },	//cobra
	{ "AP51", 0, 62, 0xa052, 1, 1 },	//cobra-lite
	{ "AP53", 4, 63, 0xa053, 1, 2 },	//cobra-AP53
	{ "AP53", 0, 64, 0xa054, 1, 2 },	//cobra-AP53-lite
	{ "XB62", 0, 68, 0x3063, 1, 1 },	//condor-lite
	{ "XB62", 4, 67, 0x3062, 1, 1 },	//condor
	{ "XB61", 4, 69, 0x3061, 1, 1 },	//hawk g only
	{ "XB61", 0, 70, 0x3065, 1, 1 },	//hawk-lite g only
	{ "QQ85",  0, 65, 0x4051, 0, 1 },	//griffin lite customer board
	{ "MB61", 4, 72, 0x2061, 0, 1 },	//talon
	{ "CB61", 4, 70, 0x1061, 0, 1 },	//talon
	{ "MB61", 0, 73, 0x2065, 0, 1 },	//talon-lite
	{ "CB61", 0, 71, 0x1065, 0, 1 },	//talon-lite
	{ "XB63", 0, 0, 0x3067, 0, 1 },     //swan
	{ "XB65", 0, 0, 0x3069, 0, 1 },     //swan xpa
	{ "HB63", 0, 77, 0x3064, 0, 1 },     //swan pcie half minicard
	{ "TB22", NO_CHIP_IDENTIFIER, 65, 0x3e67, 0, 1 }, //TB212 - for swan
};

A_UINT32 sizeLabelTable = sizeof(labelTable)/sizeof(LABEL_INFO_STRUCT);

//#ifndef __ATH_DJGPPDOS__
ART_SOCK_INFO *artSockInfo = NULL;
ART_SOCK_INFO *pArtPrimarySock = NULL;
ART_SOCK_INFO *pArtSecondarySock = NULL;
//#else
//void *artSockInfo = NULL;
//#endif

static RX_GAIN_REGS_11bg rxGainValues_11bg[] =
{
#include  "AR5111_rx_gain_2ghz.tbl"
} ;

static RX_GAIN_REGS rxGainValues[] = 
{
#include  "AR5111_rx_gain_5ghz.tbl"	
};

static RX_GAIN_REGS_AR5112 rxGainValues_derby[] = 
{
#include  "AR5112_rx_gain_5ghz.tbl"	
};

static RX_GAIN_REGS_AR5112 rxGainValues_derby_11bg[] = 
{
#include  "AR5112_rx_gain_2ghz.tbl"	
};
static RX_GAIN_REGS_AR5112 rxGainValues_griffin[] = 
{
#include  "griffin_rx_gain_2ghz.tbl"	
};

static XPD_GAIN_INFO xpdGainValues[] =
{
	{ 18,		7 },
	{ 12,		11 },
	{ 6,		13 },
	{ 0,		14 },
};
static XPD_GAIN_INFO xpdGainValues_derby[] =
{
	{ 18,		3 },
	{ 12,		2 },
	{ 6,		1 },
	{ 0,		0 },
};

XPD_GAIN_INFO *gainValues;

static char delimiters[]   = " \t";

#ifndef __ATH_DJGPPDOS__
void findEepFile()
{
	A_INT32 tempDevNum;
	A_UINT32 tempDevID;
	SUB_DEV_INFO devStruct;

	//printf("Inside findEepFile, about to get tempDevNum\n");
	tempDevNum = art_setupDevice(configSetup.instance);
	if(tempDevNum < 0) {
		uiPrintf("main: Error attaching to the device - ending test\n");
		closeEnvironment();
		exit(0);
	}
	
	//do a cfg read to determine which device is safe to access
	tempDevID = art_cfgRead(tempDevNum, 0);


	if(((tempDevID >> 16) & 0xffff) == 0xa017) {
		if(configSetup.instance != 2) {
		//should use the second radio
		art_teardownDevice(tempDevNum); 
		
		//attempt to setup second device
		tempDevNum = art_setupDevice(2);
		if(tempDevNum < 0) {
			uiPrintf("main: Error attaching to the device - ending test\n");
			closeEnvironment();
			exit(0);
		}
	}
	}
	//parse the correct eep file
    if(!setEepFile(tempDevNum)) {
        art_teardownDevice(tempDevNum);
        closeEnvironment();
		exit(0);
	}

	//get the deviceID from the library and use the swDevID
	art_getDeviceInfo(tempDevNum, &devStruct);
	swDeviceID = devStruct.swDevID;
	hwDeviceID = devStruct.hwDevID;

	//parse the cal section of the eep file
//	load_eep_vals(tempDevNum);
	parseSetup(tempDevNum);

	// calsetup and .eep files have been parsed so far
	if ( configSetup.remote  && 
		 ( (CalSetup.modeMaskForRadio[0] & 0x2) == 2) &&
		 ( (CalSetup.modeMaskForRadio[1] & 0x2) == 2) ) {
		// if both radios support 11a operation
		//configSetup.eeprom2StartLocation = 0x400;
		  configSetup.eeprom2StartLocation = checkSumLength;
		
	}

	//cleanup and let the main code setup the correct devices based on findings in eep file
	art_teardownDevice(tempDevNum); 
//	Sleep(100);  added for condor emulation
}
#endif
int
main(int argc,char *argv[]) {

    A_BOOL exitLoop = FALSE;
    A_INT32  tempDevNum;
	A_UINT32 devNumInst[3];  //start from 1, ignore 0
	A_UINT32 devNum=INVALID_INSTANCE;
	A_UINT32 devNum_2g=0;
	A_UINT32 devNum_5g=0;
	SUPPORTED_MODES supportedModes;
    A_CHAR commentBuffer[MAX_SIZE_COMMENT_BUFFER] = {'a', 'b', 'c'};
	A_UINT32 tempBackoff[3];
	SUB_DEV_INFO devStruct;
	A_UINT32 returnCode;
    A_UINT16 gainTableSize;
#ifdef LINUX
    char *tmp;
	tmp = getenv("HOSTNAME");
    machName = strtok(tmp, ".");
#else
	machName = getenv("COMPUTERNAME");
#endif

	devNumInst[1] = INVALID_INSTANCE;
	devNumInst[2] = INVALID_INSTANCE;

	if (parseCmdLine(argc,argv) == FALSE) {
		uiPrintf("main: Failed to parse command line arguments \n");
		exit (0);
	}
	
    if(initializeEnvironment(configSetup.remote) == FALSE) {
        uiPrintf("main: Failed to initialize the Driver Environment\n");
        exit(0);
    }

	//set the printLocalInfo flag dependent on whether already printed by lib
	//or if have a remove client
	printLocalInfo = 0;
#ifdef NO_LIB_PRINT
	printLocalInfo = 1;
#endif
	
	if(configSetup.remote) {
		printLocalInfo = 1;
	}

#ifndef CUSTOMER_REL
	uiPrintf("Using %d card \n",configSetup.instance);
#endif
	if(!parseConfig()) {
		uiPrintf("Problem parsing %s, exiting...\n", CONFIGSETUP_FILE);
		exit(0);
	}
#ifndef __ATH_DJGPPDOS__
  
	if(configSetup.enableLabelScheme) {
		//first try to get the label with auto detect
		printf("Looking for label\n");
		if(!promptForLabel(1)) {
			//if auto detect doesn't work,  continue to prompt for entry
			while (!promptForLabel(0));
		}
	}
	else {
		if (configSetup.computeCalsetupName > 0) {
			if (machName) {
				sprintf(calsetupFileName, "calsetup_%s_%c%c", strlwr(machName), 
												tolower(yldStruct.cardLabel[0]),
												tolower(yldStruct.cardLabel[1]));
				if(yldStruct.chipIdentifier != NO_CHIP_IDENTIFIER) {
					sprintf(calsetupFileName, "%s_%1d.txt", calsetupFileName,
														yldStruct.chipIdentifier);
				}
				else {
					sprintf(calsetupFileName, "%s.txt", calsetupFileName);
				}
			} else {
				uiPrintf("Fatal Error : Could Not Retrieve Machine Name\n");
				exit(0);
			}
		} else {
			sprintf(calsetupFileName, "calsetup.txt");
		} 
	}

	uiPrintf("Calsetupfile Used : %s\n", calsetupFileName);

//#ifndef CONDOR_HACK

	findEepFile();
//#endif
	if(configSetup.userInstanceOverride) {
		devNum = art_setupDevice(configSetup.instance);
		if(devNum < 0) {
			uiPrintf("main: Error attaching to the device - ending test\n");
			closeEnvironment();
			exit(0);
		}
		art_configureLibParams(devNum);
       //printf("SNOOP:::before reset device::Sent bytes = %d::Received bytes=%d\n", sent_bytes, received_bytes);
        //sent_bytes=0;received_bytes=0;
		art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
        //printf("SNOOP:::after reset device::Sent bytes = %d::Received bytes=%d\n", sent_bytes, received_bytes);
		uiPrintf("Attached to the Device for instance = %d\n", configSetup.instance);

		//for now make the assumption that both a and g are supported
		devNum_5g = devNum;
		devNum_2g = devNum;
	}
	else {
		//enable the instances that are valid
		if(CalSetup.modeMaskForRadio[0] != 0) {
			devNumInst[1] = art_setupDevice(1);
			if(devNumInst[1] < 0) {
				uiPrintf("main: Error attaching to the device - ending test\n");
				closeEnvironment();
				exit(0);
			}

//#ifdef CONDOR_HACK
			//parse the correct eep file
			if(!setEepFile(devNumInst[1])) {
				art_teardownDevice(devNumInst[1]);
				closeEnvironment();
				exit(0);
			}

			//get the deviceID from the library and use the swDevID
			art_getDeviceInfo(devNumInst[1], &devStruct);
			swDeviceID = devStruct.swDevID;
			hwDeviceID = devStruct.hwDevID;

			//parse the cal section of the eep file
		//	load_eep_vals(tempDevNum);
			parseSetup(devNumInst[1]);
//#endif
			// calsetup and .eep files have been parsed so far
			if ( configSetup.remote  && 
				 ( (CalSetup.modeMaskForRadio[0] & 0x2) == 2) &&
				 ( (CalSetup.modeMaskForRadio[1] & 0x2) == 2) ) {
				// if both radios support 11a operation
				//configSetup.eeprom2StartLocation = 0x400;
				  configSetup.eeprom2StartLocation = checkSumLength;
				
			}

			art_configureLibParams(devNumInst[1]);
			art_resetDevice(devNumInst[1], txStation, bssID, configSetup.channel, configSetup.turbo);
			uiPrintf("Attached to the Device for instance = %d\n", 1);
		}
		if(CalSetup.modeMaskForRadio[1] != 0) {
			devNumInst[2] = art_setupDevice(2);
			if(devNumInst[2] < 0) {
				uiPrintf("main: Error attaching to the device - ending test\n");
				closeEnvironment();
				exit(0);
			}
			art_configureLibParams(devNumInst[2]);
			art_resetDevice(devNumInst[2], txStation, bssID, configSetup.channel, configSetup.turbo);
			uiPrintf("Attached to the Device for instance = %d\n", 2);
		}
		//Setup the 2g and 5g instances
		devNum_5g = devNumInst[CalSetup.instanceForMode[MODE_11a]];
		devNum_2g = devNumInst[CalSetup.instanceForMode[MODE_11g]];
		//check that we have legal devNums for both.  If not, point to a valid
		//devNum, manufacturing expects 2 devNums to be valid
		//if all board text files are setup correctly, should not use it.
		if((devNum_5g == INVALID_INSTANCE)&& (devNum_2g == INVALID_INSTANCE)) {
			uiPrintf("Unable to allocate a valid devNum, exiting\n");
		    closeEnvironment();
			exit(0);			
		}
		else if (devNum_5g == INVALID_INSTANCE) {
			devNum_5g = devNum_2g;
		}
		else if (devNum_2g == INVALID_INSTANCE) {
			devNum_2g = devNum_5g;
		}
	}
	//setup devnum correctly
	if(configSetup.mode == MODE_11A) {
		if(devNum_5g != INVALID_INSTANCE) {
			devNum = devNum_5g;
		}
		else {
			//assume 11g
			devNum = devNum_2g;
		}
	}
	else {
		if(devNum_2g != INVALID_INSTANCE) {
			devNum = devNum_2g;
		}
		else {
			//assume 11a
			devNum = devNum_5g;
		}
	}
#endif
#ifdef __ATH_DJGPPDOS__
    pci_scan();
	tempDevNum = art_setupDevice(configSetup.instance);
    if(tempDevNum < 0) {
        uiPrintf("main: Error attaching to the device - ending test\n");
        closeEnvironment();
        exit(0);
    }
	devNum = tempDevNum;
	art_configureLibParams(devNum);
	printf("calling reset device\n");
	art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
	uiPrintf("Attached to the Device\n");
#endif

    glbl_devNum = devNum;

	//extra safety check
	if(devNum == INVALID_INSTANCE) {
		uiPrintf("Unable to get allocate a valid devNum, exiting\n");
        closeEnvironment();
		exit(0);
	}

	//get the deviceID from the library and use the swDevID
	art_getDeviceInfo(devNum, &devStruct);
	swDeviceID = devStruct.swDevID;
	hwDeviceID = devStruct.hwDevID;
	if(!get_eeprom_size(devNum,&eepromSize,&checkSumLength))  // To read eeprom size and caluculates checksum length
		uiPrintf("BOARD PARAMETERS EEPROM_SIZE = %x  checkSumLength = %x\n",eepromSize,checkSumLength);
	else
		uiPrintf("BOOTING EEPROM_SIZE = %x  checkSumLength = %x\n",eepromSize,checkSumLength);

#ifndef __ATH_DJGPPDOS__
	checkUserSize(devNum);					
#endif

//	uiPrintf("EEPROM_SIZE = %x  checkSumLength = %x\n",eepromSize,checkSumLength);


#ifndef __ATH_DJGPPDOS__

    if(!setEepFile(devNum)) {
        art_teardownDevice(devNum);
        closeEnvironment();
		exit(0);
	}
	parseSetup(devNum);
	if (!configSetup.userInstanceOverride) {

		if (CalSetup.Amode || CalSetup.enableJapanMode11aNew) {
			if (CalSetup.Gmode) {
				if ((CalSetup.instanceForMode[MODE_11a] != 1) && (CalSetup.instanceForMode[MODE_11g] != 1)) {
					configSetup.validInstance = 2;
				}
			} else {
				if (CalSetup.instanceForMode[MODE_11a] != 1) {
					configSetup.validInstance = 2;
				}
			}
		} else if (CalSetup.Gmode) {
			if (CalSetup.instanceForMode[MODE_11g] != 1) {
				configSetup.validInstance = 2;
			}
		}
	}
#else

	devNum_5g = devNum;
	devNum_2g = devNum;
#endif //__ATH_DJGPPDOS__

	if ((swDeviceID & 0xff) >= 0x14) { // venice or hainan with derby 1/2
		//if (((swDeviceID & 0xFF) == 0x16) && (((devStruct.aRevID >> 4) & 0xf) >= 5)) {
		if ((swDeviceID & 0xFF) >= 0x16) {
			pCurrGainLadder = &(gainLadder_derby2);
		} else {
			pCurrGainLadder = &(gainLadder_derby1);
		}
	} else {
		pCurrGainLadder = &(gainLadder);
	}


#ifdef __ATH_DJGPPDOS__
	if(!configSetup.eepromLoad) {
		uiPrintf("EEPROM must be loaded, enabling loading\n");
		configSetup.eepromLoad = 1;
	}

#endif
	if(configSetup.mode == MODE_11B) {
		configSetup.turbo = 0;
	}
	gainValues = xpdGainValues;
	//setup the params prior to reset
	if ((swDeviceID == 0x0007) || (swDeviceID == 0x0005) ) {
		configSetup.pCfgFile = configSetup.creteFezCfg;
		configSetup.maxChannel5G = MAX_FEZ_CHANNEL;
	}
	else if (swDeviceID == 0x0011) {
		configSetup.pCfgFile = configSetup.qmacSombreroCfg;
		configSetup.maxChannel5G = MAX_SOM_CHANNEL;
	}
	else if (swDeviceID == 0xe011) {
		configSetup.pCfgFile = configSetup.qmacSombreroBeanieCfg;
		configSetup.maxChannel5G = MAX_SOM_CHANNEL;
	}
	else if ((swDeviceID & 0xff) == 0x0012)  {
		configSetup.pCfgFile = configSetup.oahuCfg;
		configSetup.maxChannel5G = MAX_SOM_CHANNEL;
		configSetup.maxChannel2G = MAX_2G_CHANNEL;
		configSetup.minChannel2G = MIN_2G_CHANNEL;
	}
	else if (((swDeviceID & 0xff) == 0x0013) || ((swDeviceID & 0xff) == 0x0015)){
		configSetup.pCfgFile = configSetup.veniceCfg;
		configSetup.maxChannel5G = MAX_SOM_CHANNEL;
		configSetup.maxChannel2G = MAX_2G_CHANNEL;
		configSetup.minChannel2G = MIN_2G_CHANNEL;
	}
	else if (((swDeviceID & 0xff) >= 0x0014)&&((swDeviceID & 0xff) != 0x0015))  {
		configSetup.pCfgFile = configSetup.veniceCfg;
		configSetup.minChannel5G = MIN_CHANNEL_DERBY;
		configSetup.maxChannel5G = MAX_SOM_CHANNEL;
		gainValues = xpdGainValues_derby;
		gainIMax = DERBY_GAINI_MAX;
		configSetup.maxChannel2G = MAX_2G_CHANNEL_DERBY;
		configSetup.minChannel2G = MIN_2G_CHANNEL_DERBY;
	}

	if(isEagle(swDeviceID)) {
		gainIMax = GRIFFIN_GAIN_MAX;
	}

    if (isGriffin(swDeviceID)) {
        gainTableSize = sizeof(rxGainValues_griffin) / sizeof(RX_GAIN_REGS_AR5112);
        configSetup.minRxGain = rxGainValues_griffin[gainTableSize - 1].desiredGain;
        configSetup.maxRxGain = rxGainValues_griffin[0].desiredGain;
    }

    //turn on logging if needed
	if(configSetup.logging) {
		if(configSetup.logFile[0] == '\0') {
			uiPrintf("\nPlease enter filename for logging: ");
			scanf("%s", configSetup.logFile);
		}
#ifndef __ATH_DJGPPDOS__
		uilog(configSetup.logFile, 1);
#endif
	}


	if((hwDeviceID == 0xff12) || (hwDeviceID >= 0xff13)) {
		//need to indicate eeprom size
    	art_writeField(devNum, "mc_eeprom_size_ovr", 3);
	}
//#ifndef __ATH_DJGPPDOS__
    if (!configSetup.eepromLoadOverride) {
	  if (eeprom_verify_checksum(devNum))
	  {
		configSetup.validCalData  =  TRUE;
		configSetup.eepromLoad    =  1;
	  } else
	  {
		configSetup.validCalData  =  FALSE;
		configSetup.eepromLoad    =  0;
		invalidEepromMessage(2000);
	  }
   }
   else {
		configSetup.eepromLoad    =  0;
   }
//#else
	//assume for now.
//	configSetup.validCalData  =  TRUE;
//	configSetup.eepromLoad    =  1;
//#endif //__ATH_DJGPPDOS__

	//setup the hw calibration per artsetup.txt
	art_enableHwCal(devNum, configSetup.enableCal);

	if ((((swDeviceID & 0xff) >= 0x0014) && ((swDeviceID & 0xff) != 0x0015)) && (configSetup.eepromLoad)) {
		//xpdgain gets set by devlib, don't want to apply here unless there
		//is an override.
		configSetup.applyXpdGain = 0;
	}
	art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
			(A_BOOL)1, (A_UCHAR)configSetup.mode, configSetup.use_init);
	art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
	
//    if(!setEepFile(devNum)) {
//        closeEnvironment();
//		exit(0);
//	}

#ifndef __ATH_DJGPPDOS__
    if(!setEepFile(devNum)) {
        art_teardownDevice(devNum);
        closeEnvironment();
		exit(0);
	}

	if((devNum_5g != devNum_2g)&&(!configSetup.userInstanceOverride)) {
		tempDevNum = ((devNum == devNum_5g) ? devNum_2g : devNum_5g);
		if(tempDevNum != INVALID_INSTANCE) {
			if(!setEepFile(tempDevNum)) {
                art_teardownDevice(tempDevNum);
				closeEnvironment();
				exit(0);
			}
		}
	}

//	parseSetup(devNum);
	//push down the false detect values
	//need to map the mode specific backoffs in the correct order
	tempBackoff[MODE_11A] = CalSetup.falseDetectBackoff[MODE_11a];
	tempBackoff[MODE_11B] = CalSetup.falseDetectBackoff[MODE_11b];
	tempBackoff[MODE_11G] = CalSetup.falseDetectBackoff[MODE_11g];
	art_supplyFalseDetectbackoff(devNum, tempBackoff);
#endif //__ATH_DJGPPDOS__		


#ifndef __ATH_DJGPPDOS__
	if (progProm) {
		progBlankEEPROM(devNum);
		uiPrintf("Press any key to continue or <ESC> to exit\n");
		if(getch() == 0x1b) {
            art_teardownDevice(devNum);
	        closeEnvironment();
			exit(0);
		}
	}
#endif //__ATH_DJGPPDOS__

	//setup the mode
	getSupportedModes(devNum, &supportedModes);
	configSetup.mode = getFirstMode(&supportedModes);
	if(!setupMode()) {
		uiPrintf("Illegal mode value set in %s - exiting\n");
        art_teardownDevice(devNum);
		closeEnvironment();
		exit(0);
	}

	//check if the mode changed and we need to change the devNum
	if(configSetup.mode == MODE_11A) {
		devNum = devNum_5g;
	}
	else {
		devNum = devNum_2g;
	}

	//spot check
	if(devNum == INVALID_INSTANCE) {
		uiPrintf("Unable to get devNum for selected mode\n");
        closeEnvironment();
		exit(0);
	}

	//check to see if turbo has been disabled in eep file.
	//if so, don't let turbo be set through artsetup.txt

#ifndef __ATH_DJGPPDOS__
	if(CalSetup.turboDisable || CalSetup.turboDisable_11g) {
		if(configSetup.turbo) {
			configSetup.turbo = 0;
		}
	}
#endif

	art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
			(A_BOOL)configSetup.eepromHeaderLoad, (A_UCHAR)configSetup.mode, configSetup.use_init);		
	//this is needed incase we changed modes
	art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);

	//test card/installation
	if(!initTest(devNum)) {
        uiPrintf("::main: Error testing device - exiting\n");
        art_teardownDevice(devNum);
        closeEnvironment();
       exit(0);
	}

	if (configSetup.cmdLineTest) {
		//just perform the command line tests then get out
		setInitialPowerControlMode(devNum, configSetup.dataRateIndex);
		returnCode = performCmdLineTests(devNum);
		uiPrintf("closing device\n");
		art_teardownDevice(devNum); 
		uiPrintf("closing environment\n");
		closeEnvironment();
		uiPrintf("exiting\n");
		exit(returnCode);
	}

 #ifndef __ATH_DJGPPDOS__
	show_eep_label(devNum, (A_BOOL)(configSetup.enableLabelScheme));
#endif
	//Read register values read in from resetDevice
	updateConfigFromRegValues(devNum);
    //printf("SNOOP:::Sent bytes = %d::Received bytes=%d\n", sent_bytes, received_bytes);
    sent_bytes = 0;
    received_bytes = 0;

//SNOOP
//	for (i = 0; i < 28; i++) {
//		if ((i % 4 == 0) && (	i != 0)) {
//			printf("\n");
//		}
//		printf("0x%08x ", art_cfgRead(devNum, i*4));
//	}

//	i = art_cfgRead(devNum, 0x44);
//	art_cfgWrite(devNum, 0x44, i | 0x200);
//	printf("0x44 = 0x%08x\n",art_cfgRead(devNum, 0x44));
//END SNOOP
    
	while(exitLoop == FALSE) {

        printf("\n");
        printf("============================================\n");
        printf("| Test Harness Main Options:               |\n");
#ifndef CUSTOMER_REL
		printf("|   n - Toggle i(N)it code use             |\n");
#endif
		if (supportMultiModes(&supportedModes))
		{
			printf("|   o - Toggle M(o)de                      |\n");
		}
#ifndef __ATH_DJGPPDOS__
        if(configSetup.eepromLoad) {
			printf("|   e - Ignore (E)EPROM Calibration        |\n");
		}
		else {
			printf("|   e - Load (E)EPROM Calibration          |\n");
		}
#endif	// __ATH_DJGPPDOS__
        printf("|   c - (C)ontinuous transmit mode         |\n");
        printf("|   r - Continuous RF (R)eceive mode       |\n");
		printf("|   l - (L)ink test menu                   |\n");
		printf("|   t - (T)hroughput test menu             |\n");
        printf("|   p - EE(P)ROM function                  |\n");
#ifndef __ATH_DJGPPDOS__
        printf("|   s - (S)witch test card                 |\n");
        printf("|   m - (M)anufacturing/Calibration Test   |\n");
#endif	// __ATH_DJGPPDOS__
        if(configSetup.logging) {
			printf("|   g - Disable lo(g)ging                  |\n");
			printf("|   w - (W)rite Comment to Log File        |\n");
		}
		else {
			printf("|   g - Enable lo(g)ging                   |\n");
		}
		printf("|   u - (U)tility Menu                     |\n");
		printf("|   f - Recieve Sni(f)f Menu               |\n");
		printf("|   i - (N)oise Immunity Menu              |\n");
//take out for now		printf("|   x - Additional Tests                   |\n");
        printf("|   q - (Q)uit                             |\n");
        printf("============================================\n");

        switch(toupper(getch())) {
        case 'N':
                configSetup.use_init = !configSetup.use_init;
	            art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
			            (A_BOOL)configSetup.eepromHeaderLoad, (A_UCHAR)configSetup.mode, configSetup.use_init);		
                if (configSetup.use_init) {
                    printf("Using Driver init code\n");
                }
                else {
                    printf("Using MDK init code\n");
                }
	            art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
                break;

		case 'O':
			if (supportMultiModes(&supportedModes))
 			{
				//before changing mode, take a copy of the current frequency and rxGain for mode
				switch(configSetup.mode) {
				case MODE_11A:
					configSetup.channel5 = configSetup.channel;
					configSetup.rxGain5 = configSetup.rxGain;
					break;
				case MODE_11G:
				case MODE_11O:
					configSetup.channel2_4 = configSetup.channel;
					configSetup.rxGain2_4 = configSetup.rxGain;
					break;
				case MODE_11B:
					configSetup.channel2_4 = configSetup.channel;
					configSetup.rxGain2_4 = configSetup.rxGain;		//Not sure if will have another rxGain for b??
					break;
				}
			
				configSetup.mode = getNextMode(&supportedModes);
				if(configSetup.mode == MODE_11B) {
					configSetup.turbo = 0;
				}
				if((configSetup.mode != MODE_11G) && (configSetup.dataRateIndex > 7)) {
					configSetup.dataRateIndex = 0;
				}

				uiPrintf("\n");

				if ( ( (swDeviceID & 0xFF) == 0x14 ) || ( (swDeviceID & 0xFF) >= 0x16 ) ){
					configSetup.maxRxGain = RX_GAIN_MAX_AR5112;
					configSetup.minRxGain = RX_GAIN_MIN_AR5112;
				} else {
					if(configSetup.mode == MODE_11A) {
						configSetup.maxRxGain = RX_GAIN_MAX;
						configSetup.minRxGain = RX_GAIN_MIN;
					} else {
						configSetup.maxRxGain = RX_GAIN_MAX_11bg;
						configSetup.minRxGain = RX_GAIN_MIN_11bg;
					}
				}

                if (isGriffin(swDeviceID)) {
                    gainTableSize = sizeof(rxGainValues_griffin) / sizeof(RX_GAIN_REGS_AR5112);
                    configSetup.maxRxGain = rxGainValues_griffin[gainTableSize - 1].desiredGain;
                    configSetup.minRxGain = rxGainValues_griffin[0].desiredGain;
                }
				if (configSetup.rxGain > configSetup.maxRxGain) {
					configSetup.rxGain = configSetup.maxRxGain;
				} 

				if (configSetup.rxGain < configSetup.minRxGain) {
					configSetup.rxGain = configSetup.minRxGain;
				} 

				if(((swDeviceID == 0xa014)||(swDeviceID == 0xa016)||(swDeviceID == 0xa017) || (subSystemID == AP53_FULL_SUBSYSTEM_ID) || (subSystemID == AP53_LITE_SUBSYSTEM_ID))&&(!configSetup.userInstanceOverride)) {
					if(configSetup.mode != MODE_11A) {
						devNum = devNum_2g;
						if((subSystemID == AP53_FULL_SUBSYSTEM_ID) || (subSystemID == AP53_LITE_SUBSYSTEM_ID)) {
							swDeviceID = 0xa018;
						    hwDeviceID = 0xa018;
						}
					}
					else {
						devNum = devNum_5g;
						if((subSystemID == AP53_FULL_SUBSYSTEM_ID) || (subSystemID == AP53_LITE_SUBSYSTEM_ID)) {
							swDeviceID = 0x19;
						    hwDeviceID = 0xff16;
						}
					}
				}
				setupMode(); 
				art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
					(A_BOOL)configSetup.eepromHeaderLoad, (A_BOOL)configSetup.mode, configSetup.use_init);		
				art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
				if (configSetup.useTargetPower == TRUE) {
					configSetup.powerOutput = art_getMaxPowerForRate(devNum, (A_UINT16)configSetup.channel, DataRate[configSetup.dataRateIndex]);
				}
				updateConfigFromRegValues(devNum);
				
			}
			else {
	            uiPrintf("Unknown command\n");
			}
			break;
 #ifndef __ATH_DJGPPDOS__
       case 'E':
			if(configSetup.eepromLoad) {
				configSetup.eepromLoad = 0;
				//force the reload of eep file to override eeprom settings.
//					processEepFile(devNum, configSetup.cfgTable.pCurrentElement->eepFilename, &configSetup.eepFileVersion);
			}
			else {
				if (eeprom_verify_checksum(devNum))
				{
				configSetup.validCalData  =  TRUE;
				configSetup.eepromLoad = 1;
				} else
				{
				configSetup.validCalData  =  FALSE;
				configSetup.eepromLoad    =  0;
				invalidEepromMessage(2000);
				}
			}
			if (configSetup.validCalData)
			{	   
				//revert back to defaults for config setup params
				configSetup.pcDac = USE_REG_FILE;
				configSetup.powerOutput = USE_REG_FILE;
				configSetup.rxGain = USE_REG_FILE;
				configSetup.powerControlMethod = NO_PWR_CTL;

				if (((swDeviceID & 0xff) >= 0x0014) && ((swDeviceID & 0xff) != 0x0015)) {
					//xpdgain gets set by devlib, don't want to apply here unless there
					//is an override.
					configSetup.applyXpdGain = (configSetup.eepromLoad == 1) ? 0 : 1;
				}
				//update the library with the new eepromload flag setting.
				art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
					(A_BOOL)1, (A_BOOL)configSetup.mode, configSetup.use_init);
				art_rereadProm(devNum);
				art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
				if(!configSetup.eepromLoad) {
					//force the reload of eep file to override eeprom settings.
					processEepFile(devNum, configSetup.cfgTable.pCurrentElement->eepFilename, &configSetup.eepFileVersion);
				}
				
				//Read register values read in from resetDevice
				updateConfigFromRegValues(devNum);
			}

			break;
#endif //__ATH_DJGPPDOS__
        case 'C':
			contMenu(devNum);
			break;
        case 'R':
			contRxMenu(devNum);
			break;
        case 'L':
			linkMenu(devNum);
			break;
        case 'F':
			sniffMenu(devNum);
			break;
		case 'T':
			throughputMenu(devNum);
			break;
        case 'P':
				if(sizeWarning)
				{
					uiPrintf("! Warning EEP EEPROM_SIZE and EEPROM Location size are Different \n");
					uiPrintf(" May Lead to illegal operation \n");
					uiPrintf(" Do you want to continue (Y/N)\n");
					while(!kbhit())	;
						if(toupper(getch())=='Y') {
							EEPROM_Routine(devNum);
						}
						else
							break;
				}
				
			EEPROM_Routine(devNum);
            break;
		case 'U':
			utilityMenu(devNum);
			break;
		case 'I':
			noiseImmunityMenu(devNum);
			break;
		case 'G':
			if (configSetup.logging) {
				configSetup.logging = 0;

				//turn off logging
				uilogClose();
			}
			else {
				configSetup.logging = 1;

				if(configSetup.logFile[0] == '\0') {
					uiPrintf("\nPlease enter filename for logging: ");
					scanf("%s", configSetup.logFile);
				}
				#ifndef __ATH_DJGPPDOS__
				uilog(configSetup.logFile, 1);
				#endif
			}
			
			if(configSetup.logging) {
			}
	
			break;
#ifndef __ATH_DJGPPDOS__
        case 'S':
            art_teardownDevice(devNum);
			if ((devNum_2g != devNum_5g) &&(!configSetup.userInstanceOverride)) {
				if(devNum == devNum_2g) {
					art_teardownDevice(devNum_5g);
				} else {
					art_teardownDevice(devNum_2g); 
				}
				
			}
            uiPrintf("Device has been stopped and may be removed\n");
            uiPrintf("Press Q to exit or any other key to setup a new device\n");
            if( toupper(getch()) == 'Q' ) {
                uiPrintf("closing environment\n");
                closeEnvironment();
                exit(0);            
            }
			if (configSetup.remote)
 			{
 					uiPrintf("IP Address of the new AP: ");
 					scanf("%s",configSetup.machName);
 			}


			findEepFile();

			
			if(configSetup.userInstanceOverride) {
			  devNum = art_setupDevice(configSetup.instance);
			  if(devNum < 0) {
				  uiPrintf("main: Error attaching to the device - ending test\n");
				  closeEnvironment();
				  exit(0);
			  }
			  art_configureLibParams(devNum);
			  art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
			  uiPrintf("Attached to the Device for instance = %d\n", configSetup.instance);

				//for now make the assumption that both a and g are supported
				devNum_5g = devNum;
				devNum_2g = devNum;
			}
			else {
				//enable the instances that are valid
				if(CalSetup.modeMaskForRadio[0] != 0) {
					devNumInst[1] = art_setupDevice(1);
					if(devNumInst[1] < 0) {
						uiPrintf("main: Error attaching to the device - ending test\n");
						closeEnvironment();
						exit(0);
			}
					art_configureLibParams(devNumInst[1]);
					art_resetDevice(devNumInst[1], txStation, bssID, configSetup.channel, configSetup.turbo);
					uiPrintf("Attached to the Device for instance = %d\n", 1);
			}

				if(CalSetup.modeMaskForRadio[1] != 0) {
					devNumInst[2] = art_setupDevice(2);
					if(devNumInst[2] < 0) {
						uiPrintf("main: Error attaching to the device - ending test\n");
						closeEnvironment();
						exit(0);
					}
					art_configureLibParams(devNumInst[2]);
					art_resetDevice(devNumInst[2], txStation, bssID, configSetup.channel, configSetup.turbo);
					uiPrintf("Attached to the Device for instance = %d\n", 2);
					}

				//Setup the 2g and 5g instances
				devNum_5g = devNumInst[CalSetup.instanceForMode[MODE_11a]];
				devNum_2g = devNumInst[CalSetup.instanceForMode[MODE_11g]];

				//check that we have legal devNums for both.  If not, point to a valid
				//devNum, manufacturing expects 2 devNums to be valid
				//if all board text files are setup correctly, should not use it.
				if((devNum_5g == INVALID_INSTANCE)&& (devNum_2g == INVALID_INSTANCE)) {
					uiPrintf("Unable to allocate a valid devNum, exiting\n");
						closeEnvironment();
						exit(0);
					}
				else if (devNum_5g == INVALID_INSTANCE) {
						devNum_5g = devNum_2g;
					}
				else if (devNum_2g == INVALID_INSTANCE) {
					devNum_2g = devNum_5g;
				}
			}

			//setup devnum correctly
			if(configSetup.mode == MODE_11A) {
				if(devNum_5g != INVALID_INSTANCE) {
					devNum = devNum_5g;
				}
				else {
					//assume 11g
					devNum = devNum_2g;
				}
			}
			else {
				if(devNum_2g != INVALID_INSTANCE) {
					devNum = devNum_2g;
				}
				else {
					//assume 11a
					devNum = devNum_5g;
				}
				} 

			//extra safety check
			if(devNum == INVALID_INSTANCE) {
				uiPrintf("Unable to get allocate a valid devNum, exiting\n");
				closeEnvironment();
				exit(0);
			}
            
					
			//get the deviceID from the library and use the swDevID
			art_getDeviceInfo(devNum, &devStruct);
					swDeviceID = devStruct.swDevID;
					hwDeviceID = devStruct.hwDevID;					

			if (!configSetup.userInstanceOverride) {

				if (CalSetup.Amode || CalSetup.enableJapanMode11aNew) {
					if (CalSetup.Gmode) {
						if ((CalSetup.instanceForMode[MODE_11a] != 1) && (CalSetup.instanceForMode[MODE_11g] != 1)) {
							configSetup.validInstance = 2;
						}
					} else {
						if (CalSetup.instanceForMode[MODE_11a] != 1) {
							configSetup.validInstance = 2;
						}
					}
				} else if (CalSetup.Gmode) {
					if (CalSetup.instanceForMode[MODE_11g] != 1) {
						configSetup.validInstance = 2;
					}
				}
			}
				

			configSetup.powerControlMethod = NO_PWR_CTL;
			art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
			if (((swDeviceID & 0xff) == 0x14)||((swDeviceID & 0xff) >= 0x16)) {
				if ((swDeviceID & 0xFF) >= 0x16) {
					pCurrGainLadder = &(gainLadder_derby2);
				} else {
					pCurrGainLadder = &(gainLadder_derby1);
				}
			} else {
				pCurrGainLadder = &(gainLadder);
			}

			if (eeprom_verify_checksum(devNum))
			{
				configSetup.validCalData  =  TRUE;
				configSetup.eepromLoad    =  1;
				if (((swDeviceID & 0xff) >= 0x0014)&&((swDeviceID & 0xff) != 0x0015) ){
					//xpdgain gets set by devlib, don't want to apply here unless there
					//is an override.
					configSetup.applyXpdGain = 0;
				}
			} else
			{
				configSetup.validCalData  =  FALSE;
				configSetup.eepromLoad    =  0;
				invalidEepromMessage(2000);
				configSetup.applyXpdGain = 1;
			}

			art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
				(A_BOOL)configSetup.eepromHeaderLoad, (A_UCHAR)configSetup.mode, configSetup.use_init);		
	   #ifndef __ATH_DJGPPDOS__
			if(!setEepFile(devNum)) {
                art_teardownDevice(devNum);
				closeEnvironment();
				exit(0);
			}

			if(((swDeviceID == 0xa014)||(swDeviceID == 0xa016)||(swDeviceID == 0xa017))&&(!configSetup.userInstanceOverride)) {
				if(!setEepFile((devNum == devNum_5g) ? devNum_2g : devNum_5g)) {
                    art_teardownDevice((devNum == devNum_5g) ? devNum_2g : devNum_5g);
					closeEnvironment();
					exit(0);
				}
			}
		#endif
			//test card/installation
			if(!initTest(devNum)) {
                uiPrintf("main: Error testing device - exiting\n");
                art_teardownDevice(devNum);
                closeEnvironment();
                exit(0);
			}

    		//Read register values read in from resetDevice
	    	updateConfigFromRegValues(devNum);
            break;
		case 'W':
			if(configSetup.logging) {
				printf("Enter comment below.  Put a period (.) on a seperate line to complete\n");
				while(commentBuffer[0] != '.') {   
					if( fgets( commentBuffer, MAX_SIZE_COMMENT_BUFFER, stdin ) == NULL) {
						printf( "fgets error\n" );
						break;
					}
					if(commentBuffer[0] != '.') {	
						uiWriteToLog(commentBuffer);
					}
				}
				//remote the . from comment buffer otherwise will fall through next time
				commentBuffer[0] = 'a';
			}
			else {
				uiPrintf("Unknown command\n");
			}
			break;
        case 'M':

			if((swDeviceID == 0xa014)||(swDeviceID == 0xa016)||(swDeviceID == 0xa017)
				|| (subSystemID == AP53_FULL_SUBSYSTEM_ID) || (subSystemID == AP53_LITE_SUBSYSTEM_ID)) {
				if (configSetup.userInstanceOverride) {
					if (configSetup.instance == 1) {
						tempDevNum = art_setupDevice(2);
						if(tempDevNum < 0) {
							uiPrintf("main: Error attaching to the second device - ending test\n");
							closeEnvironment();
							exit(0);
						}
						art_configureLibParams(tempDevNum);
						
						if (CalSetup.instanceForMode[MODE_11g] == 2) {
							devNum_2g = tempDevNum;
						} else {
							devNum_2g = devNum;
						}
						
						if (CalSetup.instanceForMode[MODE_11a] == 2) {
							devNum_5g = tempDevNum;
						} else {
							devNum_5g = devNum;
						}
						
					} else {
						tempDevNum = art_setupDevice(1);
						if(tempDevNum < 0) {
							uiPrintf("main: Error attaching to the second device - ending test\n");
							closeEnvironment();
							exit(0);
						}						
						art_configureLibParams(tempDevNum);
	
						if (CalSetup.instanceForMode[MODE_11g] == 1) {
							devNum_2g = tempDevNum;
						} else {
							devNum_2g = devNum;
						}
						
						if (CalSetup.instanceForMode[MODE_11a] == 1) {
							devNum_5g = tempDevNum;
						} else {
							devNum_5g = devNum;
						}
					}
//				art_teardownDevice(devNum_2g);
//				devNum = devNum_5g;
				} 
			} else {
				devNum_2g = devNum;
				devNum_5g = devNum;
			}

			art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
            topCalibrationEntry(&devNum_5g, &devNum_2g);

            break;
#endif  // __ATH_DJGPPDOS__
//Take out for now		case 'X':
//			testMenu(devNum);
//			break;
        case 0x1b:
        case 'Q':
            exitLoop = TRUE;
            uiPrintf("exiting\n");
            break;
			
        default:
            uiPrintf("Unknown command\n");
            break;
        }
    }
	
#ifdef __ATH_DJGPPDOS__
    free_pcicfg_queue();
#endif

    if(configSetup.logging) {
		uilogClose();
	}
	uiPrintf("closing device\n");
    art_teardownDevice(devNum); 
	if((devNum_2g != devNum_5g) && ((swDeviceID == 0xa014)||(swDeviceID == 0xa016)||(swDeviceID == 0xa017))&&(!configSetup.userInstanceOverride)) {
		if(devNum == devNum_2g) {
			art_teardownDevice(devNum_5g);
		} else {
			art_teardownDevice(devNum_2g); 
		}
	}
    uiPrintf("closing environment\n");
    closeEnvironment();
    uiPrintf("exiting\n");
    exit(0);
}

void getSupportedModes (A_UINT32 devNum, SUPPORTED_MODES *pModes)
{
	A_UINT16 tempValue;

	if(configSetup.eepromLoad) {
		//get the mode flags from reading eeprom
		tempValue = (A_UINT16)art_eepromRead(devNum, 0xc2);
		pModes->aMode = (A_BOOL)(tempValue & 0x01);
		pModes->bMode = (A_BOOL)((tempValue >> 1) & 0x01);
		pModes->gMode = (A_BOOL)((tempValue >> 2) & 0x01);
		tempValue = (A_UINT16)art_eepromRead(devNum, 0xca);
		//check to see if the new japen a mode flag is also set
		//don't need to check eeprom version since in older
		//eeprom it defaults to 0
		pModes->aMode = pModes->aMode || (A_BOOL)((tempValue >> 11) & 0x01);
	}
	else {
#ifndef __ATH_DJGPPDOS__
		//get the values from the eep file
		load_cal_section();
		pModes->aMode = (A_BOOL)CalSetup.Amode || (A_BOOL)CalSetup.enableJapanMode11aNew;
		pModes->bMode = (A_BOOL)CalSetup.Bmode;
		pModes->gMode = (A_BOOL)CalSetup.Gmode;
#endif //__ATH_DJGPPDOS__
	}

	// For AP53.
	if(((subSystemID == AP53_FULL_SUBSYSTEM_ID) || (subSystemID == AP53_LITE_SUBSYSTEM_ID)) 
		&& configSetup.userInstanceOverride) {
		if(configSetup.instance == 1) { // cobra
            pModes->aMode = (A_BOOL)0x0;
		}
		else if(configSetup.instance == 2) { //eagle
			pModes->bMode = (A_BOOL)0x0;
			pModes->gMode = (A_BOOL)0x0;
		}
	}
}

A_UINT32 getFirstMode (SUPPORTED_MODES *pModes) {
	A_UINT32 firstMode = configSetup.mode;

	switch(configSetup.mode) {
	case MODE_11A:
		if(!pModes->aMode) {
			if (pModes->gMode) {
				firstMode = MODE_11G;
			}
			else if (pModes->bMode) {
				firstMode = MODE_11B;
			}
		}
		break;
	case MODE_11G:
	case MODE_11O:
		if(!pModes->gMode) {
			if (pModes->bMode) {
				firstMode = MODE_11B;
			}
			else if (pModes->aMode) {
				firstMode = MODE_11A;
			}
		}
		break;
	case MODE_11B:
		if(!pModes->bMode) {
			if(pModes->aMode) {
				firstMode = MODE_11A;
			}
			else if (pModes->gMode) {
				firstMode = MODE_11G;
			}
		}
		break;
	default:
		uiPrintf("FATAL ERROR: Illegal mode inside getFirstMode()\n");
		exit(1);
	}
	return(firstMode);

}

A_UINT32 getNextMode (SUPPORTED_MODES *pModes)
{
	A_UINT32 nextMode = configSetup.mode;

	switch(configSetup.mode) {
	case MODE_11A:
		if (pModes->gMode) {
			nextMode = MODE_11G;
		}
		else if (pModes->bMode) {
			nextMode = MODE_11B;
		}
		break;
	case MODE_11G:
		if((swDeviceID & 0xff) >= 0x13) {
			nextMode = MODE_11O;
		}
		else if (pModes->bMode) {
				nextMode = MODE_11B;
		}
		else if (pModes->aMode) {
			nextMode = MODE_11A;
		}
		break;
	case MODE_11O:
		if (pModes->bMode) {
			nextMode = MODE_11B;
		}
		else if (pModes->aMode) {
			nextMode = MODE_11A;
		}
		break;
	case MODE_11B:
		if(pModes->aMode) {
			nextMode = MODE_11A;
		}
		else if (pModes->gMode) {
			nextMode = MODE_11G;
		}
		break;
	default:
		uiPrintf("FATAL ERROR: Illegal mode inside getNextMode()\n");
		exit(1);
	}
	return(nextMode);

}


A_BOOL supportMultiModes(SUPPORTED_MODES *pModes)
{
	A_BOOL multiModes = FALSE;

	if((pModes->aMode && pModes->bMode) ||
		(pModes->aMode && pModes->gMode) ||
		(pModes->gMode && pModes->bMode)) {
	
		multiModes = TRUE;
	}
	return(multiModes);
}

void printDeviceInfo
(
 A_UINT32 devNum
)
{
	SUB_DEV_INFO devStruct;

	art_getDeviceInfo(devNum, &devStruct);

	subSystemID = devStruct.subSystemID;
	macRev = devStruct.macRev;
	bbRev = devStruct.bbRevID;
	analogProdRev = devStruct.aRevID & 0xf;
#ifdef CUSTOMER_REL
	if(isGriffin(swDeviceID)) {
		if (isGriffin_1_0(macRev) || isGriffin_1_1(macRev) || isGriffin_2_0(macRev)) {
			uiPrintf("\n\n###################################################################\n");
			uiPrintf("Versions prior to AR2413 00 are not supported, please obtain an upgrade\n");
			uiPrintf("###################################################################\n\n");
			exit(0);
		}
	}
#endif
	//only want to print this info if it has not already been done so by library
	//or its the AP
	if(printLocalInfo) {

		
		uiPrintf("                     =============================================\n");
		uiPrintf("                     ");                                   
		switch(devStruct.subSystemID) {
		case ATHEROS_CB21:	uiPrintf("|               AR5001a_cb                  |\n"); break;
		case ATHEROS_CB22:	uiPrintf("|               AR5001x_cb                  |\n"); break;
		case ATHEROS_MB22:	uiPrintf("|               AR5001x_mb                  |\n"); break;
		case ATHEROS_MB23:	uiPrintf("|               AR5001a_mb                  |\n"); break;
		case ATHEROS_AP21:	uiPrintf("|               AR5001ap_ap                 |\n"); break;
		case ATHEROS_CB21G: uiPrintf("|               AR5001g_cb                  |\n"); break;
		case ATHEROS_CB22G: uiPrintf("|               AR5001xp_cb                 |\n"); break;
		case ATHEROS_CB55:	uiPrintf("|               AR5007g_cb55g               |\n"); break;
		case ATHEROS_MB21G: uiPrintf("|               AR5001g_mb                  |\n"); break;
		case ATHEROS_MB22G:	uiPrintf("|               AR5001xp_mb                 |\n"); break;
		case ATHEROS_MB55:	uiPrintf("|               AR5007g_mb55g               |\n"); break;
		
		case ATHEROS_MB31G_DESTUFF :	uiPrintf("|          AR5002g_mb31g (de-stuffed)       |\n"); break;        
		case ATHEROS_MB32AG :			uiPrintf("|          AR5002x_mb32ag                   |\n"); break;               
		case ATHEROS_MB22G_DESTUFF :	uiPrintf("|          AR5001g_mb22g (de-stuffed)       |\n"); break;        
		case ATHEROS_MB22AG_SINGLE :	uiPrintf("|          AR5001x_mb22ag (single-sided)    |\n"); break;        
		case ATHEROS_MB23_JAPAN :		uiPrintf("|          AR5001x_mb23j                    |\n"); break;                 
		case ATHEROS_CB31G_DESTUFF :	uiPrintf("|          AR5002g_cb31g (de-stuffed)       |\n"); break;        
		case ATHEROS_CB32AG :			uiPrintf("|          AR5002x_cb32ag                   |\n"); break;               
		case ATHEROS_CB22G_DESTUFF :	uiPrintf("|          AR5001g_cb22g (de-stuffed)       |\n"); break;        
		case ATHEROS_AP30 :				uiPrintf("|          AR5002a_ap30                     |\n"); break;                 
		case ATHEROS_AP30_040 :			uiPrintf("|          AR5002a_ap30 (040)               |\n"); break;                 
		case ATHEROS_AP41 :				uiPrintf("|          AR5004ap_ap41g (g)               |\n"); break;                 
		case ATHEROS_AP43 :				uiPrintf("|          AR5004ap_ap43g (g)               |\n"); break;                 
		case ATHEROS_AP48 :				uiPrintf("|          AR5004ap_ap48ag (a/g)            |\n"); break;                 
		case ATHEROS_CB41 :				uiPrintf("|          AR5004g_cb41g (g)                |\n"); break;                 
		case ATHEROS_CB42 :				uiPrintf("|          AR5004x_cb42ag (a/g)             |\n"); break;                 
		case ATHEROS_CB43 :				uiPrintf("|          AR5004g_cb43g (g)                |\n"); break;                 
		case ATHEROS_MB41 :				uiPrintf("|          AR5004g_mb41g (g)                |\n"); break;                 
		case ATHEROS_MB42 :				uiPrintf("|          AR5004x_mb42ag (a/g)             |\n"); break;                 
		case ATHEROS_MB43 :				uiPrintf("|          AR5004g_mb43g (g)                |\n"); break;                 
		case ATHEROS_MB44 :				uiPrintf("|          AR5004x_mb44ag (a/g)             |\n"); break;                 
		case ATHEROS_CB51 :				uiPrintf("|          AR5005gs_cb51g (g super)         |\n"); break;                 
		case ATHEROS_CB51_LITE :		uiPrintf("|          AR5005g_cb51g (g)                |\n"); break;                 
		case ATHEROS_MB51 :				uiPrintf("|          AR5005gs_mb51g (g super)         |\n"); break;                 
		case ATHEROS_MB51_LITE :		uiPrintf("|          AR5005g_mb51g (g)                |\n"); break;                 
		case ATHEROS_USB_UB51 :         uiPrintf("|          AR5005ug_UB51 (g)                |\n"); break;                 
		case ATHEROS_USB_UB52 :  	    uiPrintf("|          AR5005ux_UB52 (a/g)              |\n"); break;                 
		case ATHEROS_CB62 :				uiPrintf("|          AR5006xs_cb62ag (a/g super)      |\n"); break;                 
		case ATHEROS_CB62_LITE :		uiPrintf("|          AR5006x_cb62ag (a/g)             |\n"); break;                 
		case ATHEROS_MB62 :				uiPrintf("|          AR5006xs_mb62ag (a/g super)      |\n"); break;                 
		case ATHEROS_MB62_LITE :		uiPrintf("|          AR5006x_mb62g (a/g)              |\n"); break;                 
		case ATHEROS_XB63:      		uiPrintf("|          ar5007eg_xb63g (g)               |\n"); break;
		case ATHEROS_HB63:      		uiPrintf("|          ar5007eg_hb63g (g)               |\n"); break;
		case ATHEROS_AP51_FULL :  	    uiPrintf("|          AR5006apgs_AP51 (g super)        |\n"); break;                 
		case ATHEROS_AP51_LITE :  	    uiPrintf("|          AR5006apg_AP51 (g)               |\n"); break;                 
		case ATHEROS_AP53_FULL :  	    uiPrintf("|          AR5006apgs_AP53 (g super)        |\n"); break;                 
		case ATHEROS_AP53_LITE :  	    uiPrintf("|          AR5006apg_AP53 (g)               |\n"); break;                 

		default: uiPrintf("|              UNKNOWN BOARD                |\n"); break;
		}		
		uiPrintf("                     =============================================\n");

		uiPrintf(devStruct.libRevStr);
		uiPrintf("Devices detected:\n");
		uiPrintf("   PCI deviceID  : 0x%04lx\t", devStruct.hwDevID);
		uiPrintf("Sub systemID  : 0x%04lx\n", devStruct.subSystemID);
		uiPrintf("   MAC revisionID: 0x%02lx  \t", devStruct.macRev & 0xff);
		uiPrintf("BB  revisionID: 0x%02lx\n", devStruct.bbRevID & 0xff);
		uiPrintf("   RF  productID : 0x%01lx   \t", devStruct.aRevID & 0xf);
		uiPrintf("RF  revisionID: 0x%01lx\n\n", (devStruct.aRevID >> 4) & 0xf);
        
		if(devStruct.defaultConfig) {
			uiPrintf("Using defaults from %s\n\n", devStruct.regFilename);
		}
		else {
		    uiPrintf("Using the values from configuration file %s\n\n", devStruct.regFilename);
		}

	}
}


A_BOOL setupMode(void) 
{
	switch (configSetup.mode) {
	case MODE_11A:     
		configSetup.channel = configSetup.channel5;
		configSetup.rxGain = configSetup.rxGain5;
		break;
	case MODE_11G: 
	case MODE_11O:
		configSetup.channel = configSetup.channel2_4;
		configSetup.rxGain = configSetup.rxGain2_4;
		break;
	case MODE_11B:     
		configSetup.channel = configSetup.channel2_4;
		configSetup.rxGain = configSetup.rxGain2_4;
		break;
	default:
		uiPrintf("Error: setupMode - mode not set correctly\n");
		return(FALSE);
	}
	
	printMode();
	return(TRUE);
}

A_BOOL printMode (void)
{
	char  *modeStr[] = {"11a", "11g", "11b", "OFDM@2.4"}; 

	if(configSetup.quarterChannel) {
		uiPrintf("Operating in %s at channel %1.4fGHz\n", modeStr[configSetup.mode], (float)((configSetup.channel + 2.5)/1000));
	}
	else {
		uiPrintf("Operating in %s at channel %1.3fGHz\n", modeStr[configSetup.mode], (float)configSetup.channel/1000);
	}
	return (TRUE);
}

A_BOOL initTest
(
 A_UINT32 devNum
) 
{
	A_UINT8  temp;
	A_UINT16 wlanNum = 0;
	A_UINT16 ii;

	//don't need this one any more
//	art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
	if (((hwDeviceID == 0xff12) || (hwDeviceID >= 0xff13)) 
		&& (subSystemID != AP53_FULL_SUBSYSTEM_ID) && (subSystemID != AP53_LITE_SUBSYSTEM_ID)){
		art_changeField(devNum, "mc_eeprom_size_ovr", 3); //force eeprom size to 16K
		art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
	}
	if(art_mdkErrNo) {
		// tmp Swan hack
		if(!isSwan(swDeviceID))
		return FALSE;
	}
	if(((subSystemID == AP53_FULL_SUBSYSTEM_ID) || (subSystemID == AP53_LITE_SUBSYSTEM_ID))
		&& !configSetup.userInstanceOverride) {
        printDeviceInfo(0);
		printDeviceInfo(1);
        uiPrintf("Base Addr: 0x%08lX  Interrupt: %d\n", art_cfgRead(0, PCI_BAR_0_REGISTER), 
             (art_cfgRead(0, PCI_INTERRUPT_LINE) & 0xff));
		uiPrintf("Base Addr: 0x%08lX  Interrupt: %d\n", art_cfgRead(1, PCI_BAR_0_REGISTER), 
             (art_cfgRead(1, PCI_INTERRUPT_LINE) & 0xff));
	}
	else {
	    printDeviceInfo(devNum);
        uiPrintf("Base Addr: 0x%08lX  Interrupt: %d\n", art_cfgRead(devNum, PCI_BAR_0_REGISTER), 
             (art_cfgRead(devNum, PCI_INTERRUPT_LINE) & 0xff));
	}

	//read the ethernet address if ap21 or ap30/ap31
	if((configSetup.remote) && ((swDeviceID == 0x0011) || (swDeviceID == 0xa014) || (swDeviceID == 0xa016) ||(swDeviceID == 0xa017) || (subSystemID == AP53_FULL_SUBSYSTEM_ID) || (subSystemID == AP53_LITE_SUBSYSTEM_ID))) {
		for (ii = 0; ii < 2; ii++) {
			art_GetMacAddr(devNum, 0, ii, (A_UCHAR *)macAddr.octets);
			wlanNum = ii;

			if ((macAddr.octets[0] != 0xFF) || (macAddr.octets[1] != 0xFF) || (macAddr.octets[2] != 0xFF) ||
				(macAddr.octets[3] != 0xFF) || (macAddr.octets[4] != 0xFF) || (macAddr.octets[5] != 0xFF) ) {
					uiPrintf("Ethernet %d MAC ADDR: 0x%02X%02X_%02X%02X_%02X%02X\n", wlanNum, macAddr.octets[0], macAddr.octets[1], macAddr.octets[2],
								macAddr.octets[3], macAddr.octets[4], macAddr.octets[5]); 
			}
		}

		for (ii = 0; ii < 2; ii++) {
			art_GetMacAddr(devNum, 1, ii, (A_UCHAR *)macAddr.octets);
			wlanNum = ii;

			if ((macAddr.octets[0] != 0xFF) || (macAddr.octets[1] != 0xFF) || (macAddr.octets[2] != 0xFF) ||
				(macAddr.octets[3] != 0xFF) || (macAddr.octets[4] != 0xFF) || (macAddr.octets[5] != 0xFF) ) {
					uiPrintf("Wireless %d MAC ADDR: 0x%02X%02X_%02X%02X_%02X%02X\n", wlanNum, macAddr.octets[0], macAddr.octets[1], macAddr.octets[2],
								macAddr.octets[3], macAddr.octets[4], macAddr.octets[5]); 
			}
		}
	}
	else {
        if(configSetup.remote) { //PB22
			*(A_UINT16 *)&macAddr.octets[0] = (A_UINT16)art_eepromRead(devNum, 0x1d);
			*(A_UINT16 *)&macAddr.octets[2] = (A_UINT16)art_eepromRead(devNum, 0x1e);
			*(A_UINT16 *)&macAddr.octets[4] = (A_UINT16)art_eepromRead(devNum, 0x1f);
//			uiPrintf("EEPROM Stored MAC ADDR: 0x%04X_%04X_%04X\n", art_eepromRead(devNum, 0x1f), art_eepromRead(devNum, 0x1e),
//                  art_eepromRead(devNum, 0x1d));
		} else {
			art_GetMacAddr(devNum, 1, 0, (A_UCHAR *)macAddr.octets);
		}
		uiPrintf("Wireless MAC ADDR: 0x%02X%02X_%02X%02X_%02X%02X\n", macAddr.octets[5], macAddr.octets[4], macAddr.octets[3],
			macAddr.octets[2], macAddr.octets[1], macAddr.octets[0]); 

		//reverse the mac address so can use for test if needed
		if(configSetup.cmdLineTestMask & MAC_ADDR_MASK) {
			temp = macAddr.octets[0];
			macAddr.octets[0] = macAddr.octets[5];
			macAddr.octets[5] = temp;
			temp = macAddr.octets[1];
			macAddr.octets[1] = macAddr.octets[4];
			macAddr.octets[4] = temp;
			temp = macAddr.octets[2];
			macAddr.octets[2] = macAddr.octets[3];
			macAddr.octets[3] = temp;
		}
   	}


	if(!art_testLib(devNum, 1000)) {
		uiPrintf("Unable to receive interrupt, check hardware and software installation and rerun\n");
		return(FALSE);
	}

	
	
	return(TRUE);
}

#ifndef __ATH_DJGPPDOS__
A_BOOL setEepFile
(
 A_UINT32 devNum
)
{
	A_UINT16	searchSubsystemID;
	A_UINT32	i;

	//load the eep file, first do a reset device, needed load config values
	art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);

	//get the subsystemID that will use for the table lookup
	if(!configSetup.cmdLineSubsystemID) {
		if((hwDeviceID == 0xff12) || (hwDeviceID >= 0xff13)) {
			searchSubsystemID = (A_UINT16)configSetup.blankEepID;
		}
		else if(swDeviceID == 0x0011) {
			//hardcode this for spirit for now
			searchSubsystemID = 0xa021;
		}
		else {
			//read the subsystemID from the eeprom
			searchSubsystemID = (A_UINT16)art_eepromRead(devNum, 0x0007);
		}
	}
	else {
		//commandline subsystemID overrides everything, including eeprom
		searchSubsystemID = (A_UINT16)configSetup.cmdLineSubsystemID;
	}


	//lookup the subsystemID in the table
	for(i = 0; i < configSetup.cfgTable.sizeCfgTable; i++) {
		if(searchSubsystemID == configSetup.cfgTable.pCfgTableElements[i].subsystemID) {
			configSetup.cfgTable.pCurrentElement = &(configSetup.cfgTable.pCfgTableElements[i]);
			break;
		}
	}
	if (i == configSetup.cfgTable.sizeCfgTable) {
	  printf("ERROR:::Unable to find device details for subsystem id=%x, \n\t\tuse \\id param to force a subsystem id\n", searchSubsystemID);
	  return FALSE;
	}

	// update ear file identifier and version for current ssid
    if (configSetup.cfgTable.pCurrentElement->earFilename[0] != '\0') {
	   configSetup.earFileIdentifier = getEarFileIdentifier(configSetup.cfgTable.pCurrentElement->earFilename);
	   configSetup.earFileVersion = getEarPerforceVersion(configSetup.cfgTable.pCurrentElement->earFilename);
    }
    else {
	   configSetup.earFileIdentifier =(A_UINT32) -1;
	   configSetup.earFileVersion = (A_UINT32)-1;
    }

	if(i == configSetup.cfgTable.sizeCfgTable) {
		uiPrintf("ERROR: unable to find an EEP file for device.\n");
		uiPrintf("Make sure an entry for this device exists in config table in artsetup\n");
		uiPrintf("and that device has correct subsystem ID \n");
		uiPrintf("or one has been specified in the BLANK_EEP_ID flag in artsetup\n");
		return FALSE;
	}

	uiPrintf("Loading values for devNum [%d] from eep file %s\n\n", devNum, configSetup.cfgTable.pCurrentElement->eepFilename);
	
//	art_setResetParams(devNum, configSetup.cfgTable.pCurrentElement->eepFilename, (A_BOOL)configSetup.eepromLoad,
//			(A_BOOL)configSetup.eepromHeaderLoad, (A_UCHAR)configSetup.mode, configSetup.use_init);		

//	art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
	return(processEepFile(devNum, configSetup.cfgTable.pCurrentElement->eepFilename, &configSetup.eepFileVersion));
}
#endif //__ATH_DJGPPDOS__


/*
 * Parse the command line arguments
 * Currently one argument is supported. The format is 
 * \AP=<ip_adddr> (only for AP)
 */

A_BOOL parseCmdLine(A_INT32 argc, A_CHAR *argv[])
{
	A_UINT32	i;
	A_CHAR buffer[256];
	A_CHAR *temp;

//	if (argc > 4)
//	{
//		uiPrintf("ParseCmdLine: Only 3 command line arguments are supported \n");
//		return FALSE;
//	}

	// Client cards has no arguments.
	if (argc == 1) return TRUE;

	for( i = argc; i > 1; i--) {
		if (strncmp(argv[i-1],"\\id=",4) == 0)
		{
//			strncpy(configSetup.cmdLineDutCardType, &(argv[i-1][6]), 112);	
            if(!sscanf(&argv[i-1][4], "%x", &configSetup.cmdLineSubsystemID)) {
                uiPrintf("Unable to get subsystem ID from \\id\n");
				return 0;
            }
		}
#ifndef __ATH_DJGPPDOS__
		else if (strncmp(argv[i-1],"\\prog",5) == 0)
		{
			progProm = 1;	
		}
		else if (strncmp(argv[i-1],"\\remote=",8) == 0)
		{
			configSetup.remote = 1;
			configSetup.primaryAP = 1;
			strncpy(configSetup.machName, &(argv[i-1][8]), 256);
			if (strnicmp(configSetup.machName, "USB", 3) == 0)  {
				usb_client = TRUE;
			}
		}
#else

		else if (strncmp(argv[i - 1], "\\bus=", 5) == 0)

		{

			if (!sscanf(&argv[i-1][5], "%d", &configSetup.bus))

			{

				uiPrintf("Unable to get PCI bus number from \\bus\n");

				return 0;

			}

		}

		else if (strncmp(argv[i - 1], "\\devinpci=", 10) == 0)

		{

			if (!sscanf(&argv[i-1][10], "%d", &configSetup.devInPci))

			{

				uiPrintf("Unable to get PCI bus number from \\devinpci\n");

				return 0;

			}

		}

#endif
		else if (strncmp(argv[i-1],"\\instance=",10) == 0)
		{
			configSetup.instance = atoi(&(argv[i-1][10]));
			configSetup.userInstanceOverride = 1;
		}

		else if (strncmp(argv[i - 1], "\\txtest", 7) == 0)
		{
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | TX_TEST_MASK;
		}
		else if (strncmp(argv[i - 1], "\\rxtest", 7) == 0)
		{
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | RX_TEST_MASK;
		}
		else if (strncmp(argv[i - 1], "\\golden", 7) == 0)
		{
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | GOLDEN_TEST_MASK;
		}
		else if (strncmp(argv[i - 1], "\\tptestup", 9) == 0)
		{
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | TP_TEST_UP_MASK;
		}
		else if (strncmp(argv[i - 1], "\\tptestdown", 11) == 0)
		{
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | TP_TEST_DOWN_MASK;
		}
		else if (strncmp(argv[i - 1], "\\macaddr", 7) == 0)
		{
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | MAC_ADDR_MASK;
		}
		else if(strncmp(argv[i - 1], "\\ch=", 4) == 0)
		{
			//parse the channel list
			if(!parseCmdChannelList(&argv[i - 1][4])) {
				return FALSE;
			}
		}
		else if (strncmp(argv[i - 1], "\\ant=", 5) == 0)
		{
			if((argv[i - 1][5] == 'a') || (argv[i - 1][5] == 'a')) {
				configSetup.antennaMask = ANTENNA_A_MASK;
			}
			else if ((argv[i - 1][5] == 'b') || (argv[i - 1][5] == 'b')) {
				configSetup.antennaMask = ANTENNA_B_MASK;
			}
			else if ((argv[i - 1][5] == 'm') || (argv[i - 1][5] == 'm')) {
				configSetup.antennaMask = ANTENNA_B_MASK | ANTENNA_A_MASK;
			}
			else {
				uiPrintf("Illegal value specified for antenna, valid choises are a, b or m (multiple/both)\n");
				return FALSE;
			}
		}
		else if (strncmp(argv[i - 1], "\\goldant=", 9) == 0)
		{
			if((argv[i - 1][9] == 'a') || (argv[i - 1][9] == 'a')) {
				configSetup.goldAntennaMask = ANTENNA_A_MASK;
			}
			else if ((argv[i - 1][9] == 'b') || (argv[i - 1][9] == 'b')) {
				configSetup.goldAntennaMask = ANTENNA_B_MASK;
			}
			else if ((argv[i - 1][9] == 'm') || (argv[i - 1][9] == 'm')) {
				configSetup.goldAntennaMask = ANTENNA_B_MASK | ANTENNA_A_MASK;
			}
			else {
				uiPrintf("Illegal value specified for golden antenna, valid choises are a, b or m (multiple/both)\n");
				return FALSE;
			}
		}
		else if (strncmp(argv[i - 1], "\\beacon=", 8) == 0) {
			//get the bssid string
			strncpy((char *)buffer, strtok(&argv[i - 1][8], " \\"), 256);
			getBssidFromString(configSetup.beaconBSSID.octets, buffer);
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | BEACON_TEST_MASK;
		}
		else if (strncmp(argv[i - 1], "\\ssid=", 6) == 0) {
            temp=strtok(&argv[i - 1][6], "\"");
			if(temp!=NULL){
				strncpy((char *)buffer, temp, 256);
//#ifndef __ATH_DJGPPDOS__
				strcpy(configSetup.SSID, buffer);
			}
//#endif
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | SSID_TEST_MASK;
		}
        else if (strncmp(argv[i - 1], "\\getmacaddress=", 15) == 0) {
			//get the bssid string
			strncpy((char *)buffer, strtok(&argv[i - 1][15], " \\"), 256);
			strcpy(configSetup.macAddrFileName, buffer);
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | GET_MAC_ADDR_MASK;
		}
		else if (strncmp(argv[i-1],"\\log=",5) == 0)
		{
			configSetup.logging = 1;
			strncpy(configSetup.logFile, &(argv[i-1][5]), MAX_FILE_LENGTH);
		}
		else if (strncmp(argv[i-1],"\\rangelog=",10) == 0)
		{
			configSetup.rangeLogging = 1;
			strncpy(configSetup.rangeLogFile, &(argv[i-1][10]), MAX_FILE_LENGTH);
		}
		else if (strncmp(argv[i-1],"\\iterations=",12) == 0) {
			configSetup.iterations = atoi(&(argv[i-1][12]));
		}
		else if (strncmp(argv[i-1],"\\dutID=",7) == 0) {
			configSetup.dutSSID = atoi(&(argv[i-1][7]));
			configSetup.userDutIdOverride = TRUE;
		}
		else if (strncmp(argv[i - 1], "\\backup=", 8) == 0) {
			//get the filename
			strncpy((char *)configSetup.eepBackupFilename, strtok(&argv[i - 1][8], " "), MAX_FILE_LENGTH);
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | BACKUP_EEPROM_MASK;
		}
		else if (strncmp(argv[i - 1], "\\restore=", 9) == 0) {
			//get the filename
			strncpy((char *)configSetup.eepRestoreFilename, strtok(&argv[i - 1][9], " "), MAX_FILE_LENGTH);
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | RESTORE_EEPROM_MASK;
		}
		else if (strncmp(argv[i - 1], "\\eepcomp=", 9) == 0) {
			//get the eeprom value
            if(!sscanf(&argv[i - 1][9], "%x", &configSetup.eepromCompareSingleValue)) {
                uiPrintf("Unable to read the eeprom value \n");
				return FALSE;
            }
			configSetup.cmdLineTest = TRUE;
			configSetup.cmdLineTestMask = configSetup.cmdLineTestMask | EEP_COMPARE_MASK;
		}
		else {
			uiPrintf("Command line cannot be parsed. \n");
#ifndef __ATH_DJGPPDOS__
			uiPrintf("Usage art [\\remote=<ip_addr>] [\\id=<subsystemID>] [\\prog] [\\instance=<instance_number>] \n");
#else
			uiPrintf("Usage art [\\id=<subsystemID] [\\instance=<instance_number>]\n ");
#endif
			return FALSE;
		}
	}

	return TRUE;
}



//each item in the channel list should be of the format NNNNx
//where NNNN is the channel in MHz x should be either a, b or g.
A_BOOL parseCmdChannelList
(
 A_CHAR *listString
)
{
	A_UINT16 roughChannelCount;
	A_CHAR *token;
	A_CHAR delimiters[]   = ",";
	A_UINT16 channelCount = 0;
	
	//get a rough channel count
	roughChannelCount = strlen(listString)/5;
	
	configSetup.testChannelList = (TEST_CHANNEL_INFO *)malloc(roughChannelCount * sizeof(TEST_CHANNEL_INFO));
	if(!configSetup.testChannelList) {
		uiPrintf("Unable to allocate memory for testChannel list\n");
		return FALSE;
	}
	token = strtok(listString, delimiters);
	while(token) {
		if(strlen(token) != 5) {
			uiPrintf("Syntax Error: channel should be of format NNNNx\n");
			uiPrintf("where NNNN is channel in MHz, x is mode: a, b or g\n");
			A_FREE(configSetup.testChannelList);
			return FALSE;
		}

		//extract mode
		switch (token[4]) {
		case 'a':
		case 'A':
			configSetup.testChannelList[channelCount].mode = MODE_11A;
			configSetup.testChannelList[channelCount].turbo = 0;
			break;
		case 't':
		case 'T':
			configSetup.testChannelList[channelCount].mode = MODE_11A;
			configSetup.testChannelList[channelCount].turbo = TURBO_ENABLE;
			break;
		case 'b':
		case 'B':
			configSetup.testChannelList[channelCount].mode = MODE_11B;
			configSetup.testChannelList[channelCount].turbo = 0;
			break;
		case 'g':
		case 'G':
			configSetup.testChannelList[channelCount].mode = MODE_11G;
			configSetup.testChannelList[channelCount].turbo = 0;
			break;
		case 'u':
		case 'U':
			configSetup.testChannelList[channelCount].mode = MODE_11G;
			configSetup.testChannelList[channelCount].turbo = TURBO_ENABLE;
			break;
		case 'o':
		case 'O':
			configSetup.testChannelList[channelCount].mode = MODE_11O;
			configSetup.testChannelList[channelCount].turbo = 0;
			break;
		default:
			uiPrintf("Illegal mode specified in channel list must be a, b or g\n");
			return FALSE;
		} //end switch
		
		token[4] = '\0';
		
        if(!sscanf(token, "%d", &(configSetup.testChannelList[channelCount].channel))) {
            uiPrintf("Unable to read channel from channel list\n");
			A_FREE(configSetup.testChannelList);
			return FALSE;
        }

		//error check the channel value
		if (configSetup.testChannelList[channelCount].mode == MODE_11A) {
			if((configSetup.testChannelList[channelCount].channel < MIN_CHANNEL) || 
				(configSetup.testChannelList[channelCount].channel > MAX_SOM_CHANNEL))
			{
				uiPrintf("Illegal channel value for mode 11a\n");
				A_FREE(configSetup.testChannelList);
				return FALSE;
			}
		}
		else { //mode is b or g
			if((configSetup.testChannelList[channelCount].channel < MIN_2G_CHANNEL) || 
				(configSetup.testChannelList[channelCount].channel > MAX_2G_CHANNEL))
			{
				uiPrintf("Illegal channel value for 2.4 GHz modes\n");
				A_FREE(configSetup.testChannelList);
				return FALSE;
			}

		}
#ifdef _DEBUG  
		uiPrintf("Added channel %d, mode %d to list\n", 
			configSetup.testChannelList[channelCount].channel, configSetup.testChannelList[channelCount].mode);
#endif
		token = strtok(NULL, delimiters);
		channelCount++;
	}
	//update exact number of channels
	configSetup.numListChannels = channelCount;

	return TRUE;
}

#ifndef __ATH_DJGPPDOS__
/**************************************************************************
* reverseBits - Reverses bit_count bits in val
*
* RETURNS: Bit reversed value
*/
A_UINT32 reverseBits
(
 A_UINT32 val, 
 int bit_count
)
{
	A_UINT32	retval = 0;
	A_UINT32	bit;
	int			i;

	for (i = 0; i < bit_count; i++)
	{
		bit = (val >> i) & 1;
		retval = (retval << 1) | bit;
	}
	return retval;
}
#endif	// #ifdef __ATH_DJGPPDOS__

void contMenu(A_UINT32 devNum)
{
    A_BOOL done = FALSE;
	A_UINT32 frameLength = 1000;
	A_UINT32 contMode;
	A_UINT32 maxRateIndex = 7;
	A_INT16		inputKey;
	A_UINT32 rateIndex = 7;
	A_UINT32 regVal_r1, regVal_r2;
//	A_UINT32 time1;
	A_UINT32    mValue, value;
	A_UINT32    dReg;
	A_UINT32    dBank;
	A_UINT32    numShifts, i;
	A_UINT32    outputSelect;
	A_UINT32    tempNumShifts;

	A_BOOL	 newLadderState;

	A_UINT32 pcdac_pwr = OFDM_CCK_DEF_IDX;
    A_BOOL   enableGainFReadback = TRUE;
	A_INT32 dBm_pwr;
	A_UINT32 rddata;

//#if !defined(CUSTOMER_REL) && !defined(__ATH_DJGPPDOS__)
#if 1
	A_INT32		devSA_local, failNum=0 ;
#endif

/*
//++JC++
	if (isGriffin(swDeviceID)) {
		configSetup.powerOvr = 0;
		configSetup.powerControlMethod = PCDAC_PWR_CTL;
	}
//++JC++
*/

	initializeGainLadder(pCurrGainLadder);
	programNewGain(pCurrGainLadder, devNum, 0);

	if(configSetup.powerOvr == 1)  {
		pCurrGainLadder->active = FALSE;
	}

	if((swDeviceID & 0xff) >= 0x0013)  {
		if(configSetup.mode == MODE_11G) {
			maxRateIndex = (configSetup.enableXR) ? 19 : 14;
		}
	}

	setInitialPowerControlMode(devNum, configSetup.dataRateIndex);

    while(!done) {
		//do a reset device to clear any provious cont test, need to be in the correct
		//state incase we have to do a setSingleTransmitPower
		art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
		if (configSetup.useTargetPower == TRUE)
			configSetup.powerOutput = art_getMaxPowerForRate(devNum, (A_UINT16)configSetup.channel, DataRate[configSetup.dataRateIndex]);
		if (configSetup.contMode == CONT_TX100) {
			contMode = CONT_DATA;
		}
		else if (configSetup.contMode == CONT_TX99) {
			contMode = CONT_FRAMED_DATA;
		}
		else if (configSetup.contMode == CONT_FRAME) {
			contMode = CONT_FRAMED_DATA;
		}
		switch(configSetup.contMode) {
			case CONT_TX100:	
				if((macRev == 0x40)&&(!(configSetup.mode == MODE_11B))) {
					configSetup.dataRateIndex = 0;
				}
			case CONT_TX99:
				
				rateIndex = configSetup.dataRateIndex;
			
                //if (!thin_client) 
				    art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);

			    forcePowerOrPcdac(devNum);

				setRegistersFromConfig(devNum);

//				time1 = milliTime();
//				uiPrintf("SNOOP: rateindex=%d, datarate=0x%x\n", rateIndex, DataRate[rateIndex]);
				art_txContBegin(devNum, contMode, configSetup.dataPattern, 
							DataRate[rateIndex], configSetup.antenna);
/*				if(configSetup.contMode == CONT_TX99) {
					time1 = milliTime();
					art_txContEnd(devNum);
					//Sleep(100);
					gainF = dump_a2_pc_out(devNum);
					uiPrintf("   [Gainf = %d]", gainF);					
				art_txContBegin(devNum, contMode, configSetup.dataPattern, 
							DataRate[rateIndex], configSetup.antenna);
				}
*/
				break;

			case CONT_FRAME:
				rateIndex = configSetup.dataRateIndex;
                //if (!thin_client) 
				    art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
			    forcePowerOrPcdac(devNum);
				setRegistersFromConfig(devNum);

				if(configSetup.mode != MODE_11B) {
					switch(configSetup.dataRateIndex)	{
						case 0:
							frameLength = 224; //410;//386;//254;
							break;
						case 1:
							frameLength = 353; //632;//596;//383;
							break;
						case 2:
							frameLength = 482; //852;//806;//512;
							break;
						case 3:
							frameLength = 740; //1298;//1226;//770;
							break;
						case 4:
							frameLength = 998; //1742;//1646;//1028;
							break;
						case 5:
							frameLength = 1514; //2630;//2486;//1544;
							break;
						case 6:
							frameLength = 2030; //3518;//3326;//2060;
							break;
						case 7:
							frameLength = 2288; //3962;//3746;//2300;
							break;
					}
				}
				else {
					switch(configSetup.dataRateIndex)	{
						case 0:
							frameLength = 41; //166; //300;
							break;
						case 1:
							frameLength = 41; //166; //300;
							break;
						case 2:
							frameLength = 116;  //366; //634;
							break;
						case 3:
							frameLength = 140; //366; //634;
							break;
						case 4:
							frameLength = 379; //1066; //1803;
							break;
						case 5:
							frameLength = 445; //1066; //1803;
							break;
						case 6:
							frameLength = 791; //2166; //3630;
							break;
						case 7:
							frameLength = 923; //2166; //3640;
							break;
					}

				}

				art_txContFrameBegin(devNum, frameLength, configSetup.numSlots, configSetup.dataPattern, 
							DataRate[rateIndex], configSetup.antenna, 1, 0, NULL);
				break;

			case CONT_CARRIER:		// single carrier
    		    //if (!thin_client) 
				     art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
			    forcePowerOrPcdac(devNum);
				setRegistersFromConfig(devNum);
				{
				SUB_DEV_INFO devStruct;
				art_getDeviceInfo(devNum, &devStruct);
				  if (devStruct.subSystemID == 0xb052) { // For UB52 boards
					regVal_r1 = art_getFieldForMode(devNum, "bb_switch_table_r1", configSetup.mode, configSetup.turbo);
					regVal_r2 = art_getFieldForMode(devNum, "bb_switch_table_r2", configSetup.mode, configSetup.turbo);
					if (configSetup.mode == MODE_11A) { // Select ANTA
						art_writeField(devNum, "bb_switch_table_r1", regVal_r1 | 0x1);
						art_writeField(devNum, "bb_switch_table_r2", regVal_r2 | 0x1);
					}
					else { // Select ANTB
						art_writeField(devNum, "bb_switch_table_r1", regVal_r1 | 0x2);
						art_writeField(devNum, "bb_switch_table_r2", regVal_r2 | 0x2);
					}
				  }
				}
				art_txContBegin(devNum, CONT_SINGLE_CARRIER, 511, 511, configSetup.antenna);

				break;
		}

		printContMenu(devNum);
#ifdef __ATH_DJGPPDOS__
		while (!_bios_keybrd(_KEYBRD_READY))
#else
		while (!kbhit())      
#endif
		{

			if ((configSetup.contMode == CONT_TX99) && enableGainFReadback)
			{
				dBm_pwr = art_getPowerIndex(devNum, (A_INT32) configSetup.powerOutput) ;
				if((configSetup.mode == MODE_11G)&&((swDeviceID & 0xff) >= 0x0013)) {
					pcdac_pwr  = OFDM_11G_IDX;
					if ((configSetup.dataRateIndex <= 14) && (configSetup.dataRateIndex >= 8)) {
//						pcdac_pwr = CCK_11G_IDX;

						dBm_pwr -= ( OFDM_11G_IDX - CCK_11G_IDX);
						if (dBm_pwr < 0) dBm_pwr = 0;
					}
				}
//				uiPrintf("SNOOP: dbm_pwr = %d, pcdac_pwr = %d\n", dBm_pwr, pcdac_pwr);
				//		if (configSetup.powerOvr == 1)
				if (0)
				{
					pCurrGainLadder->currGain = configSetup.gainI;
				} else
				{
					readGainFInContTx(pCurrGainLadder, devNum, (configSetup.powerControlMethod == DB_PWR_CTL) ? dBm_pwr : pcdac_pwr);	
				}
				tweakGainF(pCurrGainLadder, devNum, 1);
				Sleep(200);				
				readGainFInContTx(pCurrGainLadder, devNum, (configSetup.powerControlMethod == DB_PWR_CTL) ? dBm_pwr : pcdac_pwr);	
				if (invalidGainReadback(pCurrGainLadder, devNum))
				{
					uiPrintf("invalid gainF - resetting.\n");
					break;
				}
//++JC++
				if ((configSetup.powerOvr != 1) && (!isGriffin(swDeviceID)) )
				//if (configSetup.powerOvr != 1)
				{
					configSetup.b_ob = art_getFieldForMode(devNum, "rf_b_ob", configSetup.mode, configSetup.turbo);
					configSetup.b_db = art_getFieldForMode(devNum, "rf_b_db", configSetup.mode, configSetup.turbo);
				}

				printContMenu(devNum);
				if (isGriffin(swDeviceID) || isEagle(swDeviceID)) {
					rddata = art_regRead(devNum, 0xa264);
					uiPrintf(" PDADC = %d, gain = %d dacGain = %d  [0x%08x, 0x%08x, 0x%08x] ", 
						(rddata >> 1) & 0xFF, (rddata >> 14) & 0x3f, (rddata >> 9) & 0x1f, art_regRead(devNum, 0xa258), art_regRead(devNum, 0xa26c), rddata);
					//uiPrintf(" Gainf = %d [%s]  ", pCurrGainLadder->currGain, pCurrGainLadder->currStep->stepName);
				} else {
					uiPrintf(" Gainf = %d [%s]  ", pCurrGainLadder->currGain, pCurrGainLadder->currStep->stepName);
					uiPrintf("  %s\n", (pCurrGainLadder->active ? "" : "inactive"));
				}
				Sleep(100);
/*
printf("SNOOP: contMenu: vals : 0x%x, 0x%x, 0x%x, 0x%x\n", art_getFieldForMode(devNum, "mc_cmp_buf_size", MODE_11G, 0),
	                art_getFieldForMode(devNum, "bb_clk_obs_select_3", MODE_11G, 0),
					art_getFieldForMode(devNum, "rf_bbfgain", MODE_11G, 0),
					art_getFieldForMode(devNum, "bb_dyn_ofdm_cck_mode", MODE_11G, 0) );
*/

			} 
        //    printf("SNOOP:::Cont Menu::Sent bytes = %d::Received bytes=%d\n", sent_bytes, received_bytes);
            sent_bytes = 0;
            received_bytes = 0;

		}
		
		if (!invalidGainReadback(pCurrGainLadder, devNum))
		{
			inputKey = getch();
		} else
		{
			inputKey = '8';
		}

		if(!processCommonOptions(devNum, inputKey)) {
			switch(inputKey) {
			case '8' :
				break;
			case '9':  // toggle dynamic gain adj. on or off
//			  if((swDeviceID & 0xff) >= 0x14) {
				if(!isGriffin(swDeviceID) && !isEagle(swDeviceID)) {
				if (0) {
					uiPrintf("deviceID = %x does not need dynamic optimization\n", swDeviceID);
				  } else {

					newLadderState = !(pCurrGainLadder->active);
//					if (configSetup.mode == MODE_11A) {
					if (0) {      // to make behavior consistent, now that derby/sombrero both are supported
						setGainLadderForMaxGain(pCurrGainLadder); // OBSOLETED. gen2 - to match cal data for pcdacs
					} else {
						initializeGainLadder(pCurrGainLadder);
					}
					programNewGain(pCurrGainLadder, devNum, 0);
					uiPrintf("Changing dynamic adjust status from %d to %d\n", pCurrGainLadder->active, newLadderState);
					pCurrGainLadder->active = newLadderState;
				  }
				}
				break;
			case '7' :
				enableGainFReadback = !enableGainFReadback;
				break;
			case 'o':
			case 'O':
				if((macRev != 0x40) || (configSetup.contMode != CONT_TX100) || (configSetup.mode == MODE_11B)) {
					configSetup.dataRateIndex = 
						(configSetup.dataRateIndex==maxRateIndex) ? 0 : configSetup.dataRateIndex+1;
					if (configSetup.useTargetPower == TRUE)
						configSetup.powerOutput = art_getMaxPowerForRate(devNum, (A_UINT16)configSetup.channel, DataRate[configSetup.dataRateIndex]);
				}
				break;
			case 'k':
			case 'K':
				if((macRev != 0x40) || (configSetup.contMode != CONT_TX100) || (configSetup.mode == MODE_11B)){
					configSetup.dataRateIndex = 
						(configSetup.dataRateIndex==0) ? maxRateIndex : configSetup.dataRateIndex-1;
					if (configSetup.useTargetPower == TRUE)
						configSetup.powerOutput = art_getMaxPowerForRate(devNum, (A_UINT16)configSetup.channel, DataRate[configSetup.dataRateIndex]);				
				}
				break;

			case 'D':
			case 'd':
				configSetup.dataPattern++;
				if (configSetup.dataPattern > RANDOM_PATTERN) {
					configSetup.dataPattern = ZEROES_PATTERN;
				}
				break;
			case 's':
			case 'S':
				if(configSetup.contMode==CONT_TX100) {
					configSetup.contMode = CONT_TX99;	
				}
				else if (configSetup.contMode==CONT_TX99) {
					configSetup.contMode = CONT_CARRIER;
				}
				else if (configSetup.contMode == CONT_CARRIER) {
					configSetup.contMode = CONT_FRAME;
				}
				else if (configSetup.contMode == CONT_FRAME) {
					configSetup.contMode = CONT_TX100;
				}
				break;
			case 'z':
			case 'Z':
				if(bbRev >= 0x30) {
					configSetup.scrambleModeOff = !(configSetup.scrambleModeOff);					
					art_changeField(devNum, "bb_disable_scrambler", configSetup.scrambleModeOff);
				}
				break;
			
			case 't':
				if(configSetup.contMode == CONT_FRAME) {
					configSetup.numSlots +=1;
					if(configSetup.numSlots > MAX_NUM_SLOTS) {
						configSetup.numSlots = MIN_NUM_SLOTS;
					}
				}
				break;

			case 'T':
				if(configSetup.contMode == CONT_FRAME) {
					configSetup.numSlots +=20;
					if(configSetup.numSlots > MAX_NUM_SLOTS) {
						configSetup.numSlots = MIN_NUM_SLOTS;
					}
				}
				break;

			case 'r':
				if(configSetup.contMode == CONT_FRAME) {
					configSetup.numSlots -=1;
					if(configSetup.numSlots < MIN_NUM_SLOTS) {
						configSetup.numSlots = MAX_NUM_SLOTS;
					}
				}
				break;

			case 'R':
				if(configSetup.contMode == CONT_FRAME) {
					configSetup.numSlots -=20;
					if(configSetup.numSlots < MIN_NUM_SLOTS) {
						configSetup.numSlots = MAX_NUM_SLOTS;
					}
				}
				break;

#if !defined(__ATH_DJGPPDOS__)
			case 'e':
			case 'E':

				if (CalSetup.useInstruments)
				{
				devSA_local = spaInit(CalSetup.saGPIBaddr, CalSetup.saModel);

				uiPrintf("\n\n\nSpectral Mask Measurement ");
				if ((configSetup.mode == MODE_11B)||
				    ((configSetup.mode == MODE_11G)&&((swDeviceID & 0xff) >= 0x13)&&(configSetup.dataRateIndex >=8))) {
				  uiPrintf("(CCK) : ");
					failNum = spaMeasSpectralMask11b(devSA_local,configSetup.channel*1e6,60e6,1,0,0);
				} else
				{
				  uiPrintf("(OFDM) : ");
					failNum = spaMeasSpectralMask(devSA_local,configSetup.channel*1e6,60e6,1,0,0);
				}
				uiPrintf("%d points failed. %s", failNum, ((failNum>20) ? "[FAIL]" : "[PASS]"));
				Sleep(3000);
				}
				break;
#endif

		case '6':
			uiPrintf("Enter os1:os0 value: ");
			scanf("%x", &outputSelect);
			if (outputSelect == 0) {
				value = (0x14 | reverseBits(outputSelect, 2));
			}
			else if(outputSelect == 1) {
				uiPrintf("Enter M3:M0 value: ");
				scanf("%x", &mValue);
				value = (reverseBits(mValue, 5) << 8) | 0x14 | reverseBits(outputSelect & 0x3, 2);
//printf("Value = %x\n", value);
			}
			else if(outputSelect == 2) {
				uiPrintf("Enter data register (0-3): ");
				scanf("%x", &dReg);
				uiPrintf("Enter bank (0-7): ");
				scanf("%x", &dBank);
				value = ( ((reverseBits(dBank, 3) << 3) | reverseBits(dReg, 2)) << 16) | 0x14 | reverseBits(outputSelect & 0x3, 2);
			}
			else {
				uiPrintf("NOnly output select values of 0,1 and 2 are currently selected\n");
				break;
			}
			uiPrintf("\nEnter number of shifts: ");
			scanf("%x", &numShifts);
			art_regWrite(devNum, (PHY_BASE+(0x34<<2)), value);
			while(numShifts > 0) {
				if(numShifts > 32) {
					tempNumShifts = 32;
					numShifts -= 32;
				}
				else {
					tempNumShifts = numShifts;
					numShifts = 0;
				}
				for (i=0; i<tempNumShifts; i++) {   
					Sleep(1);
					art_regWrite(devNum, (PHY_BASE+(0x20<<2)), 0x00010000);
				}	
				Sleep(1);
				value = (art_regRead(devNum, PHY_BASE + (256<<2)) >> (32 - tempNumShifts));   
				printf("\nvalue = 0x%x ", value); 
				if(tempNumShifts > 1) {
					printf("(or 0x%x after %d bit reverse)\n", reverseBits(value, tempNumShifts), tempNumShifts);
				}
				else {
					printf("\n");
				}
			}
			uiPrintf("Press any key to continue\n");
			getch();
			break;

			case 0x1b:
				//put scramble mode back to normal
				if(bbRev >= 0x30) {
					art_changeField(devNum, "bb_disable_scrambler", 0);
				}
				art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
				done = TRUE;
				uiPrintf("\nExit Continuous Mode .....\n");
				break;
			default:
				uiPrintf("\nUnknown command...\n");
				break;
			}
		}
	}
}

void contRxMenu(A_UINT32 devNum)
{
    A_BOOL done = FALSE;
	A_INT16	inputKey;

	art_changeField(devNum, "bb_disable_agc_to_a2", 0x1);
	//disable noisefloor cal by only enabling offset cal in resetDevice
	//art_enableHwCal(DO_OFSET_CAL);
	art_enableHwCal(devNum, 0);
	art_resetDevice(devNum, rxStation, bssID, configSetup.channel, configSetup.turbo);
    while(!done) {

		clearScreen();
		printf("\n");
		printf("=================================================================\n");
		printf("| Continous RF Receive Options                                  |\n");
		printf("|   p - Increase Center Frequency by 10 MHz (P inc by 100 MHz)  |\n");
		printf("|   l - Decrease Center Frequency by 10 MHz (L dec by 100 MHz)  |\n");
		printf("|   i - Increase rx Gain (I inc by 10)                          |\n");
		printf("|   j - Decrease rx Gain (J dec by 10)                          |\n");
		printf("|   a - Toggle antenna                                          |\n");
		printf("|   s - Loop through antenna switch table                       |\n");
	 	printf("| ESC - exit                                                    |\n");
		printf("=================================================================\n\n");

		printMode();
		uiPrintf("\n");
		if(configSetup.antenna == (USE_DESC_ANT | DESC_ANT_A))
			uiPrintf("ANT_A", (unsigned int)configSetup.channel);
		else
			uiPrintf("ANT_B", (unsigned int)configSetup.channel);


		//setup the change fields then reset device	
		if(configSetup.mode == MODE_11A)
		{
		  if(!setRxGain(devNum)) {
		    	uiPrintf("Unable to set gain.  Returning to main menu\n");
			  return;
		  }
		} else
		{
		  if(!setRxGain_11bg(devNum)) {
		    	uiPrintf("Unable to set gain.  Returning to main menu\n");
			  return;
		  }
		}
		
		if(configSetup.rxGain == USE_REG_FILE) {
			uiPrintf(" receive Gain set externally\n");
		}
		else {
			uiPrintf(" receive Gain = %d\n", configSetup.rxGain);
		}

		art_resetDevice(devNum, rxStation, bssID, configSetup.channel, configSetup.turbo);
        art_setAntenna(devNum, configSetup.antenna);
#ifdef __ATH_DJGPPDOS__
		while (!_bios_keybrd(_KEYBRD_READY))
#else
		while (!kbhit())      
#endif
			;
		inputKey = getch();
		switch(inputKey) {
			case 'p':
			case 'P':
			case 'l':
			case 'L':
				updateChannel(inputKey);
				break;
			case 'a':
			case 'A':
				configSetup.antenna = 
					(configSetup.antenna==(USE_DESC_ANT|DESC_ANT_A)) ? (USE_DESC_ANT|DESC_ANT_B) : (USE_DESC_ANT|DESC_ANT_A);
				break;
			case 'i':
			case 'I':
			case 'j':
			case 'J':
				updateGainI(inputKey);
				break;
			case 's':
			case 'S' :
				switchTableLoop(devNum);
				break;
			case 0x1b:
				done = TRUE;
				uiPrintf("exiting\n");
				art_changeField(devNum, "bb_disable_agc_to_a2", 0x0);
				art_enableHwCal(devNum, DO_OFSET_CAL | DO_NF_CAL);
				art_resetDevice(devNum, rxStation, bssID, configSetup.channel, configSetup.turbo);
				return;
			default:
				uiPrintf("Unknown command\n");
				break;
		}
	}
}

void updateGainI
(
 A_INT16 inputKey
)
{
	switch(inputKey) {
	case 'i':
		if(configSetup.rxGain == USE_REG_FILE) {
			configSetup.rxGain = INITIAL_RXGAIN;
		}
		else {
			configSetup.rxGain ++;
		}
		if(configSetup.rxGain > configSetup.maxRxGain) {
			configSetup.rxGain = configSetup.maxRxGain;
		}
		configSetup.overwriteRxGain = 1;
		break;
	case 'I':
		if(configSetup.rxGain == USE_REG_FILE) {
			configSetup.rxGain = INITIAL_RXGAIN;
		}
		else {
			configSetup.rxGain +=10;
		}
		if(configSetup.rxGain > configSetup.maxRxGain) {
			configSetup.rxGain = configSetup.maxRxGain;
		}
		configSetup.overwriteRxGain = 1;
		break;
	case 'j':
		if(configSetup.rxGain == USE_REG_FILE) {
			configSetup.rxGain = INITIAL_RXGAIN;
		}
		else {
			configSetup.rxGain --;
		}
		if(configSetup.rxGain < configSetup.minRxGain) {
			configSetup.rxGain = configSetup.minRxGain;
		}
		configSetup.overwriteRxGain = 1;
		break;
	case 'J':
		if(configSetup.rxGain == USE_REG_FILE) {
			configSetup.rxGain = INITIAL_RXGAIN;
		}
		else {
			configSetup.rxGain -=10;
		}
		if(configSetup.rxGain < configSetup.minRxGain) {
			configSetup.rxGain = configSetup.minRxGain;
		}
		configSetup.overwriteRxGain = 1;
		break;
	default:
		uiPrintf("INTERNAL SOFTWARE ERROR: in updateChannel, should not get here\n");
		return;
	}
	return;
}

void linkMenu(A_UINT32 devNum)
{
    A_BOOL exitLoop = FALSE;
	A_UINT32 rateMask;
	A_UINT32 framesPerRate = configSetup.linkNumPackets;
	A_UINT32 frameLength = configSetup.linkPacketSize;
	A_UINT32 retry = 0;		// broadcast mode, disable retry
	A_UINT32 broadcast = 1;	// enable broadcast mode
	A_UINT32 waitTime = 6000; //3000;
	A_UINT32 timeOut  = 8000; //5000;
	A_UINT32 RemoteStats = NO_REMOTE_STATS;
//	A_UINT32 RemoteStats = SKIP_SOME_STATS | (43 << NUM_TO_SKIP_S);
    RX_STATS_STRUCT rStats;
	A_UINT32 count = 0, i;
    A_UINT32 mode = 0;
	A_INT16		inputKey;
    A_UCHAR  pattern[2] = {0xaa, 0x55};		//hard coded for now, will external data pattern later
	A_UINT32 ppm = 1;
	A_UINT32 maxRateIndex = 7;
	A_UINT32 startRateIndex = 0;

	A_UINT32 gainF=0;
	A_BOOL	 newLadderState;
	A_UINT32 numRates;
	A_UINT32 tempRateMask;

	A_UINT32 pcdac_pwr = OFDM_CCK_DEF_IDX;
	A_INT32  dBm_pwr;
	A_UINT32 rddata;

/*  SNOOP
//++JC++
	if (isGriffin(swDeviceID)) {
		configSetup.powerOvr = 0;
		configSetup.powerControlMethod = PCDAC_PWR_CTL;
	}
//++JC++
*/
    art_mdkErrNo=0; // Clear any errors
	if((configSetup.mode == MODE_11G)&&((swDeviceID & 0xff) >= 0x0013)) {
		pcdac_pwr  = OFDM_11G_IDX; // linktest sends 2 pkts at rate_54 to read gainF
	}

	if((swDeviceID == 0xf11b) || (configSetup.mode == MODE_11B)) {
		ppm = 0;				//this is not supported in FPGA or for 11b
	}
	
//	if(configSetup.mode == MODE_11G) {
//		frameLength = 500;
//	}
	

	initializeGainLadder(pCurrGainLadder);
	programNewGain(pCurrGainLadder, devNum, 0);

	//give a longer wait time for AP
	if(configSetup.remote ) {
		waitTime = 6000;
		timeOut = 8000;
	}
/*    if (thin_client) {
		waitTime = 16000;
		timeOut = 18000;
	}
*/

	printf("\n");
	printf("=================================================================\n");
	printf("| Link Test Mode                                                |\n");
	printf("|   t - Tx                                                      |\n");
	printf("|   r - Rx                                                      |\n");
	printf("|   p - Increase Center Frequency by 10 MHz (P inc by 100 MHz)  |\n");
	printf("|   l - Decrease Center Frequency by 10 MHz (L dec by 100 MHz)  |\n");
	printf("|   i - Increase pcdac (I inc by 10)                            |\n");
	printf("|   j - Decrease pcdac (J dec by 10)                            |\n");
	printf("|   f - Increase power output by 0.5dBm (F inc by 5dBm)         |\n");
	printf("|   c - Decrease power output by 0.5dBm (C dec by 5dBm)         |\n");
	printf("|   u - Increase ob by 1 (w - increase b-ob)                    |\n");
	printf("|   h - Increase db by 1 (q - increase b-db)                    |\n");
	printf("|   z - Toggle packet interleave                                |\n");
	if (!isGriffin(swDeviceID) && !isEagle(swDeviceID) ) {
	    printf("|   v - Toggle power override (ovr)                             |\n");
	    if (!configSetup.powerOvr) {
		    printf("|   x - Toggle external power                                   |\n");
	    }
	    else {
    		printf("|   y - Increase gainI by 1 (Y inc by 10)                       |\n");
	    	printf("|   g - decrease gainI by 1 (G dec by 10)                       |\n");
		    if(((swDeviceID & 0xff) <= 0x13) || ((swDeviceID & 0xff) == 0x15)) {
			    printf("|   m - Increase rf Gain Boost                                  |\n");
		    }
		    else {
			    printf("|   m - Increase rfgain_I                                       |\n");
		    }
	    }
	    if (!configSetup.powerOvr) {
		    printf("|   n - Step xpd gain by 6dB                                    |\n");
	    }
	}
	if(configSetup.mode != MODE_11B) {
		printf("|   b - Toggle turbo mode                                       |\n");
	}
	printf("|   a - Toggle antenna                                          |\n");
	if (!isGriffin(swDeviceID) && !isEagle(swDeviceID) ) {
    	printf("|   9 - Toggle dynamic optimization                             |\n");
	    if(!pCurrGainLadder->active) {
		    printf("|   4 - Increment Fixed gain                                    |\n");
	    }
	}
    printf("|   D - Xmit and Wait for Recv |\n");
	printf("|   E - Recv and Loopback frames|\n");
	printf("| ESC - exit                                                    |\n");
	printf("=================================================================\n");

	printMode();
	uiPrintf("\n");
    while(exitLoop == FALSE) {
#ifdef __ATH_DJGPPDOS__
		if (_bios_keybrd(_KEYBRD_READY)) {
#else
		if(kbhit()) {     
#endif
           	inputKey = getch();

			if(!processCommonOptions(devNum, inputKey)) {
				switch(inputKey) {
				case '9':  // toggle dynamic gain adj. on or off
//				  if((swDeviceID & 0xff) >= 0x14) {
					if (!isGriffin(swDeviceID) && !isEagle(swDeviceID) ) {
				    	if (0) {
					    	uiPrintf("deviceID = %x does not need dynamic optimization\n", swDeviceID);
					      } else {

						    newLadderState = !(pCurrGainLadder->active);
						    uiPrintf("Changing dynamic adjust status from %d to %d\n", pCurrGainLadder->active, newLadderState);
						    if (0) { 
    							setGainLadderForMaxGain(pCurrGainLadder); // OBSOLETED. gen2 - to match cal data for pcdacs
	    					} else {
		    					initializeGainLadder(pCurrGainLadder);
			    			}
				    		programNewGain(pCurrGainLadder, devNum,0);
					    	pCurrGainLadder->active = newLadderState;
					      }
					}
					break;
				case 'T':
				case 't':
					mode = 1;
					break;
				case 'R':
				case 'r':
					mode = 2;
					break; 
				case 'Z':
				case 'z':
					configSetup.pktInterleave = !(configSetup.pktInterleave);
					break;
#ifndef CUSTOMER_REL
				case 'D':
				case 'd':
					mode = 3;
					printf("Xmit mode: at data rate %s\n", DataRateStr[dataRateIndex]);
					break;
				case 'E':
				case 'e':
					printf("Recv mode: at data rate %s\n", DataRateStr[dataRateIndex]);
					mode = 4;
					break;
				case 'o':
				case 'O':
					dataRateIndex = (dataRateIndex==maxRateIndex) ? 0 : dataRateIndex+1;
					printf("Data rate %s\n", DataRateStr[dataRateIndex]);
					break;
				case 'k':
				case 'K':
					dataRateIndex = (dataRateIndex==maxRateIndex) ? 0 : dataRateIndex-1;
					printf("Data rate %s\n", DataRateStr[dataRateIndex]);
				break;
#endif
				case 0x1b:
					exitLoop = TRUE;
					uiPrintf("exiting\n");

					//cleanup descriptor queues, to free up mem
					art_cleanupTxRxMemory(devNum, TX_CLEAN | RX_CLEAN);
					return;
				default:
					uiPrintf("Unknown command\n");
					break;
				}
			}

			//Perform all change fields here so keep change from last time takes
			//effect, or any field changes from outside take effect.  Note that
			//have a little bit of overhead setting all these each time, but don't 
			//think it will be noticeable
//			setRegistersFromConfig(devNum);

			maxRateIndex = 7;
			if((swDeviceID & 0xff) >= 0x0013) {
				if((configSetup.mode == MODE_11G)&&(!configSetup.turbo)) {
					maxRateIndex = 14;
					ppm = 0;
				}
				
				if(configSetup.enableXR) {
					maxRateIndex = 19;
				}

				if(configSetup.mode == MODE_11B) {
					startRateIndex = 8;
					maxRateIndex = 14;
				}
			}

			setInitialPowerControlMode(devNum, configSetup.dataRateIndex);

			printConfigSettings(devNum);

			if(configSetup.antenna == (USE_DESC_ANT | DESC_ANT_A))
				if(configSetup.quarterChannel) {
					uiPrintf("Freq = %7.1lf, ANT_A", configSetup.channel + 2.5);
				}
				else {
					uiPrintf("Freq = %d, ANT_A", (unsigned int)configSetup.channel);
				}
			else
				uiPrintf("Freq = %d, ANT_B", (unsigned int)configSetup.channel);

			if(configSetup.mode == MODE_11B) {
				rateMask = configSetup.rateMask & 0xfd;
			}
			else {
				rateMask = configSetup.rateMask;
				if(configSetup.enableXR) {
					rateMask = rateMask & 0xf80ff;
				}
				else if((maxRateIndex == 7) || (configSetup.turbo)) {
					rateMask = rateMask & 0xff;
				}
				else {
					rateMask = rateMask & 0x7fff;
				}
				
			}
			if (rateMask == 0) {
				rateMask = 0x22222;
			}
			if (!configSetup.pktInterleave) {
				rateMask = RATE_GROUP | rateMask;
                uiPrintf(", pkt interleave: off");
			}
            else { 
                uiPrintf(", pkt interleave: on");
            }
			if (configSetup.turbo==TURBO_ENABLE) uiPrintf(", Turbo\n");
			else if (configSetup.turbo==HALF_SPEED_MODE) uiPrintf(", Half rate\n");
			else if (configSetup.turbo==QUARTER_SPEED_MODE) uiPrintf(", Quarter rate\n");
			else uiPrintf("\n");

			switch(mode) {
				case 1:
					uiPrintf("Beginning broadcast transmission of 100 packets across all rates\n");
					//moved from above txDataBegin
					art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
					//setupGainFRead(devNum);
					art_txDataSetup(devNum, rateMask, rxStation, framesPerRate, frameLength, pattern, 2, retry, configSetup.antenna, broadcast);
					forcePowerOrPcdac(devNum);
					setRegistersFromConfig(devNum);
					//end of moved section
					break;
				case 2:
					uiPrintf("Beginning receive of 100 packets per rate displayed as: good-packets(RSSI)\n");
					if(configSetup.mode == MODE_11B) {
						uiPrintf("              1Mb/s   2Mb/s   2Mb/s  5.5Mb/s 5.5Mb/s 11Mb/s  11Mb/s CRCs\n"); 
						uiPrintf("Preamble:     long    long    short  long    short   long    short  CRCs\n"); 
					} 
					else if ((configSetup.mode == MODE_11G) &&
						((swDeviceID & 0xff) >= 0x0013) && (!configSetup.turbo) && (!configSetup.enableXR)) {
						uiPrintf("      6   9   12  18  24  36  48  54  1L  2L  2S  5L  5S 11L 11S   CRCs\n"); 	
						uiPrintf("     --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---   ----\n\n");
					}
					else if (((swDeviceID & 0xff) >= 0x0013) && (configSetup.mode != MODE_11B) 
						&& (configSetup.enableXR)) {
						uiPrintf("      6   9   12  18  24  36  48  54 .25  .5 1XR 2XR 3XR CRCs\n"); 	
						uiPrintf("     --- --- --- --- --- --- --- --- --- --- --- --- --- ----\n\n");
					}					
					else {
						if(configSetup.turbo == TURBO_ENABLE) {
							uiPrintf("     12Mb/s  18Mb/s  24Mb/s  36Mb/s  48Mb/s  72Mb/s  96Mb/s  108Mb/s CRCs PPM\n"); 
						} 
						else if (configSetup.turbo == HALF_SPEED_MODE) {
							uiPrintf("      3Mb/s 4.5Mb/s   6Mb/s   9Mb/s  12Mb/s  18Mb/s  24Mb/s  27Mb/s CRCs PPM\n"); 	
						} 
						else if (configSetup.turbo == QUARTER_SPEED_MODE) {
							uiPrintf("      1.5Mb/s 2.75Mb/s   3Mb/s   4.5Mb/s  6Mb/s  9Mb/s  12Mb/s  13.5Mb/s CRCs PPM\n"); 	
						} else {
							uiPrintf("      6Mb/s   9Mb/s  12Mb/s  18Mb/s  24Mb/s  36Mb/s  48Mb/s  54Mb/s CRCs PPM\n"); 	
						}
					}

					//moved from above rxDataBegin
					art_resetDevice(devNum, rxStation, bssID, configSetup.channel, configSetup.turbo);
					art_setAntenna(devNum, configSetup.antenna);
					setRegistersFromConfig(devNum);

					tempRateMask = rateMask;
					numRates = 0;
					while(tempRateMask) {
						if(tempRateMask & 0x01) {
							numRates++;
						}
						tempRateMask >>= 1;
					}
					art_rxDataSetup(devNum, (framesPerRate * numRates) + 10, frameLength, ppm);
					//end of moved section
					break; 

#ifndef CUSTOMER_REL
				case 3: // xmit and recv
				case 4: // recv and xmit
				    art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
					//Allocate desc/buf ptr
					lb_frame_len = MAX_LB_FRAME_LEN;
				    tx_desc_ptr = art_memAlloc(MAX_DESC_WORDS * 4, 0, devNum); 
				    tx_buf_ptr = art_memAlloc(lb_frame_len, 0, devNum); 
				    rx_desc_ptr = art_memAlloc(MAX_DESC_WORDS * 4, 0, devNum); 
				    rx_buf_ptr = art_memAlloc(lb_frame_len, 0, devNum); 
					//printf("TDP=%x:tbp=%x:rdp=%x:rbp=%x\n", tx_desc_ptr, tx_buf_ptr, rx_desc_ptr, rx_buf_ptr);
					*((A_UINT32 *)frameBuffer) = lb_frame_len;
				    for(iIndex=4;iIndex<lb_frame_len;iIndex++) {
					  frameBuffer[iIndex] = 0x5a;
					}
					break;
#endif
			}
		}

        switch(mode) {
        case 0: // Idle Mode
     		Sleep(200);	// delay 200 ms
            break;
        case 1:			// transmit mode

//MOVE THIS BACK TO ABOVE FOR griffin
/**************
art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);

//setupGainFRead(devNum);
art_txDataSetup(devNum, rateMask, rxStation, framesPerRate, frameLength, pattern, 2, retry, 
	configSetup.antenna, broadcast);
forcePowerOrPcdac(devNum);
setRegistersFromConfig(devNum);
**************/
//end section

			art_txDataBegin(devNum, timeOut, SKIP_STATS_COLLECTION);
            if (art_mdkErrNo!=0) {
				uiPrintf("Transmit Iteration Failed - error %d\n", art_mdkErrNo);
                break;
            }
            else {
//#if !defined(CUSTOMER_REL) && !defined(__ATH_DJGPPDOS__)
#if 1
				if ((((rateMask & 0x7fff) == 0x7fff) && ((configSetup.mode == MODE_11G)&&((swDeviceID & 0xff) >= 0x0013))) || 
					((rateMask & 0xff) == 0xff))
				{
					uiPrintf("Sent %d frames at all rates - iteration %d. ", framesPerRate, ++count, gainF);
				} else
				{
					uiPrintf("Sent %d frames at ratemask 0x%x - iteration %d. ", framesPerRate, rateMask, ++count);
				}
				Sleep(1);	
			    //Sleep(3000); // Only to be slow enough for predator
//				time1 = milliTime();
				if (configSetup.powerOvr == 1)
				{
					pCurrGainLadder->currGain = configSetup.gainI;
				} else
				{
					dBm_pwr = art_getPowerIndex(devNum, (A_INT32) configSetup.powerOutput) ;
					if((configSetup.mode == MODE_11G)&&((swDeviceID & 0xff) >= 0x0013)) {
						pcdac_pwr  = OFDM_11G_IDX;
					}
					readGainF(pCurrGainLadder, devNum, (configSetup.powerControlMethod == DB_PWR_CTL) ? dBm_pwr : pcdac_pwr);
				}
				if (isGriffin(swDeviceID) || isEagle(swDeviceID)) {
					rddata = art_regRead(devNum, 0xa264);
					uiPrintf(" PDADC = %d, gain = %d dacGain = %d\n", 
						(rddata >> 1) & 0xFF, (rddata >> 14) & 0x3f, (rddata >> 9) & 0x1f, art_regRead(devNum, 0xa258), art_regRead(devNum, 0xa26c), rddata);
					//uiPrintf(" Gainf = %d [%s]  ", pCurrGainLadder->currGain, pCurrGainLadder->currStep->stepName);
				} else { 
					tweakGainF(pCurrGainLadder, devNum, 0);
					uiPrintf(" gainF = %d [%s]", pCurrGainLadder->currGain, pCurrGainLadder->currStep->stepName);
					uiPrintf("  %s\n", (pCurrGainLadder->active ? "" : "inactive"));
				}
#else            
				if ((rateMask & 0xff) == 0xff)
				{
					uiPrintf("Sent %d frames at all rates - iteration %d.\n", framesPerRate, ++count);
				} else
				{
					uiPrintf("Sent %d frames at ratemask 0x%x - iteration %d. \n", framesPerRate, rateMask & 0xff, ++count);
				}

#endif
            }
			//had to make this longer (was 500) incase other side is an ap.
			Sleep(1500);	// delay 1500 ms between packet sends
            break;
        case 2:			// receive mode

//MOVE THIS BACK TO ABOVE FOR PREDATOR
/**************
art_resetDevice(devNum, rxStation, bssID, configSetup.channel, configSetup.turbo);
art_setAntenna(devNum, configSetup.antenna);
setRegistersFromConfig(devNum);

tempRateMask = rateMask;
numRates = 0;
while(tempRateMask) {
	if(tempRateMask & 0x01) {
		numRates++;
	}
	tempRateMask >>= 1;
}
art_rxDataSetup(devNum, (framesPerRate * numRates) + 10, frameLength, ppm);
**************/
//end moved section
			art_rxDataBegin(devNum, waitTime, timeOut, RemoteStats, 0, pattern, 2);
			if (art_mdkErrNo!=0)
				break;

            uiPrintf("RX:  ");
			if(configSetup.mode == MODE_11B) {
				uiPrintf("        ");
			}

			for(i = startRateIndex; i < maxRateIndex+1; i++) {
				if((configSetup.enableXR) &&  
					((configSetup.mode == MODE_11G) || (configSetup.mode == MODE_11A))){
					if((i > 7) && (i < 15)) {
						continue;
					}
				}
				if(!(( i == 1) && (configSetup.mode ==MODE_11B))) {
					art_rxGetStats(devNum, DataRate[i], 0, &rStats);
					if((maxRateIndex > 7) && (configSetup.mode != MODE_11B)) {
						uiPrintf("%3d ", rStats.goodPackets);
					}
					else {
						uiPrintf("%3d(%2d) ", rStats.goodPackets, rStats.DataSigStrengthAvg);
					}
				}
			}
	        art_rxGetStats(devNum, 0, 0, &rStats);
			uiPrintf("%4d ", rStats.crcPackets); 
			if (ppm) {
				uiPrintf("%3d", rStats.ppmAvg); 
			}
			uiPrintf("\n");
			//print the RSSI if needed
			if((maxRateIndex > 7) && (configSetup.mode != MODE_11B)) {
	            uiPrintf("RSSI ");
				for(i = 0; i < maxRateIndex+1; i++) {
					if((i > 7) && (i < 15) && (configSetup.enableXR) && 
						((configSetup.mode == MODE_11G) || (configSetup.mode == MODE_11A))) {
						continue;
					}
					art_rxGetStats(devNum, DataRate[i], 0, &rStats);
					uiPrintf("(%2d)", rStats.DataSigStrengthAvg);
				}
				uiPrintf("\n\n");
			}
			if(configSetup.remote ) {
				Sleep(100);
			}
			else {
				Sleep(500);
			}
            break;
#ifndef CUSTOMER_REL		  
           case 3:
					start_time = milliTime();
					retVal = (A_INT32)art_send_frame_and_recv(devNum, frameBuffer, tx_desc_ptr, tx_buf_ptr, rx_desc_ptr, rx_buf_ptr, 0xb);

					if (retVal > 0) {
						air_time += retVal;
					    pktCount++;
						end_time = milliTime();
						total_time += (end_time - start_time);
					}
					else {
						retVal = 0;
					}

					if (pktCount == BLOCK_PKT_COUNT) {
						thru_put = ((2.0 * MAX_LB_FRAME_LEN * BLOCK_PKT_COUNT * 8 * 1000.0)/(total_time * 1.0))/(1024.0*1024.0);
						printf("Time stats for %d pkts is: air_time=%dus:total_time=%dms:throughput=%lf mbps\n", BLOCK_PKT_COUNT, air_time, total_time,  thru_put);
						pktCount=0; air_time=0; total_time=0;
					}
			break;
		  case 4:
				retVal = (A_INT32)art_recv_frame_and_xmit(devNum, tx_desc_ptr, tx_buf_ptr, rx_desc_ptr, rx_buf_ptr, 0xb);
				printf("Recv ret val = %d\n", retVal);
			break;
#endif
        }
	}
}

#define MAX_SNIFF_BUFFER_SIZE 3000
#define MAX_NUM_SNIFF_BUFFERS 4000
#define MAX_SNIFF_TIMEOUT     30
void sniffMenu(A_UINT32 devNum)
{
	A_UINT32 ppm = 1;
	A_UINT32 waitTime = 5000;
	A_UINT32 maxRateIndex = 7;
	A_UINT32 startRateIndex = 0;
	A_BOOL exitLoop = FALSE;
	A_BOOL firstTime = TRUE;
    A_UCHAR  pattern[2] = {0xaa, 0x55};		//hard coded for now, will external data pattern later
	A_UINT16 i;
    RX_STATS_STRUCT rStats;
	A_UINT16 inputKey;

	if(configSetup.mode == MODE_11B) {
		ppm = 0;				//this is not supported in FPGA or for 11b
	}

	maxRateIndex = 7;
	if((swDeviceID & 0xff) >= 0x0013) {
		if((configSetup.mode == MODE_11G)&&(!configSetup.turbo)) {
			maxRateIndex = 14;
			ppm = 0;
		}
		
		if(configSetup.enableXR) {
			maxRateIndex = 19;
		}

		if(configSetup.mode == MODE_11B) {
			startRateIndex = 8;
			maxRateIndex = 14;
		}
	}

    while(exitLoop == FALSE) {
		clearScreen(); 
		printf("\n");
		printf("=================================================================\n");
		printf("| Sniff Mode                                                    |\n");
		printf("|   p - Increase Center Frequency by 10 MHz (P inc by 100 MHz)  |\n");
		printf("|   l - Decrease Center Frequency by 10 MHz (L dec by 100 MHz)  |\n");
		if(configSetup.mode != MODE_11B) {
			printf("|   b - Toggle turbo mode                                       |\n");
		}
		printf("|   a - Toggle antenna                                          |\n");
//		printf("|   o - Increase Buffer Size by 100 (O inc by 1000)             |\n");
//		printf("|   k - Decrease Buffer Size by 100 (K dec by 1000)             |\n");
		printf("|   u - Increase number buffers by 100 (U inc by 1000)          |\n");
		printf("|   h - Decrease number buffers by 100 (H dec by 1000)          |\n");
		printf("|   i - Increase timeout my 1 second (I inc by 5secs)           |\n");
		printf("|   j - Decrease timeout my 1 second (J dec by 5secs)           |\n");
		printf("| ESC - exit                                                    |\n");
		printf("=================================================================\n");
		printf("\n\n");

		uiPrintf(" Number Buffers = %d,  Timeout = %d\n", 
			configSetup.sniffNumBuffers, configSetup.sniffTimeout);
		uiPrintf(" Channel = %d, ", configSetup.channel);
		if(configSetup.antenna == (USE_DESC_ANT | DESC_ANT_A)) {
			uiPrintf(" ANTA");
		} else {
			uiPrintf(" ANTB");
		}

		if (configSetup.turbo==TURBO_ENABLE) uiPrintf(", Turbo,");
		else if (configSetup.turbo==HALF_SPEED_MODE) uiPrintf(", Half rate,");	
		else if (configSetup.turbo==QUARTER_SPEED_MODE) uiPrintf(", Quarter rate,");

		uiPrintf("\n\n");

		if(configSetup.mode == MODE_11B) {
			uiPrintf("              1Mb/s   2Mb/s   2Mb/s  5.5Mb/s 5.5Mb/s 11Mb/s  11Mb/s CRCs\n"); 
			uiPrintf("Preamble:     long    long    short  long    short   long    short  CRCs\n"); 
		} 
		else if ((configSetup.mode == MODE_11G) &&
			((swDeviceID & 0xff) >= 0x0013) && (!configSetup.turbo) && (!configSetup.enableXR)) {
			uiPrintf("      6   9   12  18  24  36  48  54  1L  2L  2S  5L  5S 11L 11S   CRCs\n"); 	
			uiPrintf("     --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---   ----\n\n");
		}
		else if (((swDeviceID & 0xff) >= 0x0013) && (configSetup.mode != MODE_11B) 
			&& (configSetup.enableXR)) {
			uiPrintf("      6   9   12  18  24  36  48  54 .25  .5 1XR 2XR 3XR CRCs\n"); 	
			uiPrintf("     --- --- --- --- --- --- --- --- --- --- --- --- --- ----\n\n");
		}					
		else {
			if(configSetup.turbo == TURBO_ENABLE) {
				uiPrintf("     12Mb/s  18Mb/s  24Mb/s  36Mb/s  48Mb/s  72Mb/s  96Mb/s  108Mb/s CRCs PPM\n"); 
			} 
			else if (configSetup.turbo == HALF_SPEED_MODE) {
				uiPrintf("      3Mb/s 4.5Mb/s   6Mb/s   9Mb/s  12Mb/s  18Mb/s  24Mb/s  27Mb/s CRCs PPM\n"); 	
			} 
			else if (configSetup.turbo == QUARTER_SPEED_MODE) {
				uiPrintf("      1.5Mb/s 2.75Mb/s   3Mb/s   4.5Mb/s  6Mb/s  9Mb/s  12Mb/s  13.5Mb/s CRCs PPM\n"); 	
			} else {
				uiPrintf("      6Mb/s   9Mb/s  12Mb/s  18Mb/s  24Mb/s  36Mb/s  48Mb/s  54Mb/s CRCs PPM\n"); 	
			}
		}

		if(firstTime) {
			//setup the required number of descriptors
			art_resetDevice(devNum, rxStation, bssID, configSetup.channel, configSetup.turbo);
			art_setAntenna(devNum, configSetup.antenna);

			art_rxDataSetup(devNum, configSetup.sniffNumBuffers, configSetup.sniffBufferSize, ppm);
			art_rxDataSniff(devNum, waitTime, configSetup.sniffTimeout*1000, 0, pattern, 2);
			firstTime = FALSE;
		}

		uiPrintf("RX:  ");
		if(configSetup.mode == MODE_11B) {
			uiPrintf("        ");
		}

		for(i = startRateIndex; i < maxRateIndex+1; i++) {
			if((configSetup.enableXR) &&  
				((configSetup.mode == MODE_11G) || (configSetup.mode == MODE_11A))){
				if((i > 7) && (i < 15)) {
					continue;
				}
			}
			if(!(( i == 1) && (configSetup.mode ==MODE_11B))) {
				art_rxGetStats(devNum, DataRate[i], 0, &rStats);
				if((maxRateIndex > 7) && (configSetup.mode != MODE_11B)) {
					uiPrintf("%3d ", rStats.goodPackets);
				}
				else {
					uiPrintf("%3d(%2d) ", rStats.goodPackets, rStats.DataSigStrengthAvg);
				}
			}
		}
		art_rxGetStats(devNum, 0, 0, &rStats);
		uiPrintf("%4d ", rStats.crcPackets); 
		if (ppm) {
			uiPrintf("%3d", rStats.ppmAvg); 
		}
		uiPrintf("\n");
		//print the RSSI if needed
		if((maxRateIndex > 7) && (configSetup.mode != MODE_11B)) {
			uiPrintf("RSSI ");
			for(i = 0; i < maxRateIndex+1; i++) {
				if((i > 7) && (i < 15) && (configSetup.enableXR) && 
					((configSetup.mode == MODE_11G) || (configSetup.mode == MODE_11A))) {
					continue;
				}
				art_rxGetStats(devNum, DataRate[i], 0, &rStats);
				uiPrintf("(%2d)", rStats.DataSigStrengthAvg);
			}
			uiPrintf("\n\n");
		}
		
		//check for keyboard press
#ifdef __ATH_DJGPPDOS__
		if (_bios_keybrd(_KEYBRD_READY)) {
#else
		if(kbhit()) {     
#endif
           	inputKey = getch();

			switch(inputKey) {
				case 'p':
				case 'P':
				case 'l':
				case 'L':
					updateChannel(inputKey);
					break;
				case 'B':
				case 'b':
					// HALF_SPEED and QUARTER_SPEED only made available in 11a and 11o at ART level yet.
					// in 11g, not clear what cck will do.
					if(configSetup.mode != MODE_11B) {
						switch(configSetup.turbo) {
						case 0:
			#ifndef __ATH_DJGPPDOS__
							if((configSetup.mode == MODE_11A) && (CalSetup.turboDisable)) {
			#else
							if(configSetup.mode == MODE_11A) {
			#endif
								//don't go into turbo mode, go to half speed instead
								configSetup.turbo = HALF_SPEED_MODE;
							}
			#ifndef __ATH_DJGPPDOS__
							else if((configSetup.mode == MODE_11G) && (CalSetup.turboDisable_11g)) {
			#else
							else if(configSetup.mode == MODE_11G) {
			#endif
								//don't go into turbo mode, go to half speed instead
								configSetup.turbo = HALF_SPEED_MODE;
							}
							else {
								configSetup.turbo = TURBO_ENABLE;
							}

							break;
						case TURBO_ENABLE:
			//				if ((configSetup.mode == MODE_11A) || (configSetup.mode == MODE_11O)){
							if (configSetup.mode != MODE_11B){
								configSetup.turbo = HALF_SPEED_MODE;
							} else {
								configSetup.turbo = 0;
							}
							break;
						case HALF_SPEED_MODE:
							// if half_speed is allowed, so is quarter_speed
							configSetup.turbo = QUARTER_SPEED_MODE;
							break;
						case QUARTER_SPEED_MODE:
							configSetup.turbo = 0;
							break;			
						}
						if(configSetup.turbo) {
							if((configSetup.dataRateIndex > 7)	&& (configSetup.dataRateIndex < 15)) {
								configSetup.dataRateIndex = 0;
							}
							if((configSetup.dataRateIndexTP > 7) && (configSetup.dataRateIndexTP < 15)) {
								configSetup.dataRateIndexTP = 0;
							}
						}

						updateConfigFromRegValues(devNum);
					}
					break;

				case 'a':
				case 'A':
					configSetup.antenna = 
						(configSetup.antenna==(USE_DESC_ANT|DESC_ANT_A)) ? (USE_DESC_ANT|DESC_ANT_B) : (USE_DESC_ANT|DESC_ANT_A);
					break;
//				case 'o':
//					configSetup.sniffBufferSize += 100;
//					if (configSetup.sniffBufferSize > MAX_SNIFF_BUFFER_SIZE) {
//						configSetup.sniffBufferSize = 100;
//					}
//					break;
//				case 'O':
//					configSetup.sniffBufferSize += 1000;
//					if (configSetup.sniffBufferSize > MAX_SNIFF_BUFFER_SIZE) {
//						configSetup.sniffBufferSize = 1000;
//					}
//					break;

//				case 'k':
//					if(configSetup.sniffBufferSize <= 100) {
//						configSetup.sniffBufferSize = MAX_SNIFF_BUFFER_SIZE;
//					}
//					else {
//						configSetup.sniffBufferSize -= 100;
//					}
//					break;

//				case 'K':
//					if(configSetup.sniffBufferSize <= 1000) {
//						configSetup.sniffBufferSize = MAX_SNIFF_BUFFER_SIZE;
//					}
//					else {
//						configSetup.sniffBufferSize -= 1000;
//					}
					
//					break;

				case 'u':
					configSetup.sniffNumBuffers += 100;
					if (configSetup.sniffNumBuffers > MAX_NUM_SNIFF_BUFFERS) {
						configSetup.sniffNumBuffers = 100;
					}
					break;
				case 'U':
					configSetup.sniffNumBuffers += 1000;
					if (configSetup.sniffNumBuffers > MAX_NUM_SNIFF_BUFFERS) {
						configSetup.sniffNumBuffers = 1000;
					}
					break;


				case 'h':
					if(configSetup.sniffNumBuffers <= 100) {
						configSetup.sniffNumBuffers = MAX_NUM_SNIFF_BUFFERS;
					}
					else {
						configSetup.sniffNumBuffers -= 100;
					}

					break;
				case 'H':
					if(configSetup.sniffNumBuffers <= 1000) {
						configSetup.sniffNumBuffers = MAX_NUM_SNIFF_BUFFERS;
					}
					else {
						configSetup.sniffNumBuffers -= 1000;
					}

					break;

				case 'i':
					configSetup.sniffTimeout += 1;
					if (configSetup.sniffTimeout > MAX_SNIFF_TIMEOUT) {
						configSetup.sniffTimeout = 1;
					}
					break;
				case 'I':
					configSetup.sniffTimeout += 5;
					if (configSetup.sniffTimeout > MAX_SNIFF_TIMEOUT) {
						configSetup.sniffTimeout = 5;
					}

					break;

				case 'j':
					if (configSetup.sniffTimeout <= 1) {
						configSetup.sniffTimeout = MAX_SNIFF_TIMEOUT;
					}
					else {
						configSetup.sniffTimeout -= 1;
					}
					break;
				case 'J':
					if (configSetup.sniffTimeout <= 5) {
						configSetup.sniffTimeout = MAX_SNIFF_TIMEOUT;
					}
					else {
						configSetup.sniffTimeout -= 5;
					}

					break;

				case 0x1b:
					exitLoop = TRUE;
					uiPrintf("exiting\n");

					//cleanup descriptor queues, to free up mem
					art_cleanupTxRxMemory(devNum, TX_CLEAN | RX_CLEAN);
					return;
				default:
					uiPrintf("Unknown command\n");
					break;
				}
			}
		
		//setup the next iteration and wait on packets before clearing out previous display
		if(!firstTime) {
			//setup the required number of descriptors
			art_resetDevice(devNum, rxStation, bssID, configSetup.channel, configSetup.turbo);
			art_setAntenna(devNum, configSetup.antenna);

			art_rxDataSetup(devNum, configSetup.sniffNumBuffers, configSetup.sniffBufferSize, ppm);
			art_rxDataSniff(devNum, waitTime, configSetup.sniffTimeout*1000, 0, pattern, 2);
		}
	
	} //end while
}


void throughputMenu(A_UINT32 devNum)
{
    A_BOOL exitLoop = FALSE;
	A_UINT32 rateMask;
	A_UINT32 waitTime = 3000;
	A_UINT32 timeOut  = 5000;
    TX_STATS_STRUCT tStats;
    A_UINT32 mode = 0;
	A_INT16		inputKey;
    A_UCHAR  pattern[2] = {0xaa, 0x55};		//hard coded for now, will external data pattern later
	A_UINT32 ppm = 1;
	A_BOOL	 inputChange = 1;
	RX_STATS_SNAPSHOT rxStats;
	A_BOOL	 newLadderState;
	A_UINT32 maxRateIndex = 7;
	A_UINT32 pcdac_pwr = OFDM_CCK_DEF_IDX;
	A_INT32  dBm_pwr;
	A_UINT32  max_num_pkts;
/* SNOOP
//++JC++
	if (isGriffin(swDeviceID)) {
		configSetup.powerOvr = 0;
		configSetup.powerControlMethod = PCDAC_PWR_CTL;
	}
//++JC++
*/
	if(!isGriffin(swDeviceID) && !isEagle(swDeviceID)) {
		art_writeField(devNum, "bb_max_rx_length", 0xFFF);
		art_writeField(devNum, "bb_en_err_length_illegal", 0);
	}

	if((swDeviceID == 0xf11b) || (configSetup.mode == MODE_11B)) {
		ppm = 0;				//this is not supported in FPGA or for 11b
	}
	
	if((swDeviceID & 0xff) >= 0x0013) {
		if(configSetup.mode == MODE_11G) {
			maxRateIndex = 14;
		}
		else if(configSetup.dataRateIndexTP > 7) {
			configSetup.dataRateIndexTP = 0;
		}

		if(configSetup.enableXR) {
			if(configSetup.mode != MODE_11B) {
				maxRateIndex = 19;

				if((configSetup.mode == MODE_11A) && (configSetup.dataRateIndexTP > 7)
					&& (configSetup.dataRateIndexTP < 15)) {
					configSetup.dataRateIndexTP = 15;
				}
			}
		}
	}

	initializeGainLadder(pCurrGainLadder);
	programNewGain(pCurrGainLadder, devNum, 0);

	//give a longer wait time for AP
	if(configSetup.remote ) {
		waitTime = 6000;
		timeOut = 8000;
	}

	printf("\n");
	printf("=================================================================\n");
	printf("| Throughput Test Mode                                          |\n");
	printf("|   t - Tx                                                      |\n");
	printf("|   r - Rx                                                      |\n");
	printf("|   p - Increase Center Frequency by 10 MHz (P inc by 100 MHz)  |\n");
	printf("|   l - Decrease Center Frequency by 10 MHz (L dec by 100 MHz)  |\n");
	printf("|   o - Increase Data Rate                                      |\n");
	printf("|   k - Decrease Data Rate                                      |\n");
	printf("|   s - Toggle packet size (500 | 1000 | 1500 | 2000)           |\n");
	printf("|   e - Increase HW retries                                     |\n");
	printf("|   d - Decrease HW retries                                     |\n");
	printf("|   z - Toggle number of packets (5000 | 10000)                 |\n");
	printf("|   i - Increase pcdac (I inc by 10)                            |\n");
	printf("|   j - Decrease pcdac (J dec by 10)                            |\n");
	printf("|   f - Increase power output by 0.5dBm (F inc by 5dBm)         |\n");
	printf("|   c - Decrease power output by 0.5dBm (C dec by 5dBm)         |\n");
	printf("|   u - Increase ob by 1 (w - increase b-ob)                    |\n");
	printf("|   h - Increase db by 1 (q - increase b-db)                    |\n");
	if (!isGriffin(swDeviceID)  && !isEagle(swDeviceID) ) {
    	printf("|   v - Toggle power override (ovr)                             |\n");
	    if (!configSetup.powerOvr) {
		    printf("|   x - Toggle external power                                   |\n");
	    }
	    else {
		    printf("|   y - Increase gainI by 1 (Y inc by 10)                       |\n");
		    printf("|   g - decrease gainI by 1 (G dec by 10)                       |\n");
		    if(((swDeviceID & 0xff) <= 0x13) || ((swDeviceID & 0xff) == 0x15)) {
			    printf("|   m - Increase rf Gain Boost                                  |\n");
		    }
		    else {
			    printf("|   m - Increase rfgain_I                                       |\n");
		    }
	    }
	    if (!configSetup.powerOvr) {
		    printf("|   n - Step xpd gain by 6dB                                    |\n");
	    }
	}
	if(configSetup.mode != MODE_11B) {
		printf("|   b - Toggle turbo mode                                       |\n");
	}
	printf("|   a - Toggle antenna                                          |\n");
	printf("|   1 - Toggle unicast/broadcast packets                        |\n");
	if (!isGriffin(swDeviceID) && !isEagle(swDeviceID) ) {
    	printf("|   9 - Toggle dynamic optimization                             |\n");
	    if(!pCurrGainLadder->active) {
		    printf("|   4 - Increment Fixed gain                                    |\n");
	    }
	}
	printf("| ESC - exit                                                    |\n");
	printf("=================================================================\n");

	printMode();
	uiPrintf("\n");
	if (configSetup.useTargetPower == TRUE)
		configSetup.powerOutput = art_getMaxPowerForRate(devNum, (A_UINT16)configSetup.channel, DataRate[configSetup.dataRateIndexTP]);
    while(exitLoop == FALSE) {
#ifdef __ATH_DJGPPDOS__
		if (_bios_keybrd(_KEYBRD_READY)) {
#else
		if(kbhit()) {     
#endif
           	inputKey = getch();

			inputChange = 1;
			if(!processCommonOptions(devNum, inputKey)) {
				switch(inputKey) {
				case '9':  // toggle dynamic gain adj. on or off
//				  if((swDeviceID & 0xff) >= 0x14) {
			    	if (!isGriffin(swDeviceID) && !isEagle(swDeviceID) ) {
				    	if (0) {
					    	uiPrintf("deviceID = %x does not need dynamic optimization\n", swDeviceID);
					      } else {
						
						    newLadderState = !(pCurrGainLadder->active);
						    uiPrintf("Changing dynamic adjust status from %d to %d\n", pCurrGainLadder->active, newLadderState);
						    if (0) {
							    setGainLadderForMaxGain(pCurrGainLadder); // OBSOLETED. gen2 - to match cal data for pcdacs
						    } else {
							    initializeGainLadder(pCurrGainLadder);
						    }
						    programNewGain(pCurrGainLadder, devNum,0);
						    pCurrGainLadder->active = newLadderState;
					    }
					}
					break;
				case 'T':
				case 't':
					mode = 1;
					break;
				case 'R':
				case 'r':
					mode = 2;
					break; 
				case 'o':
				case 'O':
					configSetup.dataRateIndexTP = 
						(configSetup.dataRateIndexTP==maxRateIndex) ? 0 : configSetup.dataRateIndexTP+1;

					if((configSetup.dataRateIndexTP > 7) && (configSetup.dataRateIndexTP < 15)
						&& ((configSetup.mode == MODE_11A) || (configSetup.mode == MODE_11O) || 
						(configSetup.turbo != 0) || (configSetup.enableXR))) {
						configSetup.dataRateIndexTP = 15;
						if(configSetup.dataRateIndexTP > maxRateIndex) {
							configSetup.dataRateIndexTP = 0;
						}
					}
					if (configSetup.useTargetPower == TRUE)
						configSetup.powerOutput = art_getMaxPowerForRate(devNum, (A_UINT16)configSetup.channel, DataRate[configSetup.dataRateIndexTP]);
					break;
				case 'k':
				case 'K':
					configSetup.dataRateIndexTP = 
						(configSetup.dataRateIndexTP==0) ? maxRateIndex : configSetup.dataRateIndexTP-1;
					
					if((configSetup.dataRateIndexTP > 7) && (configSetup.dataRateIndexTP < 15) 
						&& ((configSetup.mode == MODE_11A) || (configSetup.mode == MODE_11O) || 
						(configSetup.turbo != 0) || (configSetup.enableXR))) {
						configSetup.dataRateIndexTP = 7;
					}
					if (configSetup.useTargetPower == TRUE)
						configSetup.powerOutput = art_getMaxPowerForRate(devNum, (A_UINT16)configSetup.channel, DataRate[configSetup.dataRateIndexTP]);
					break;
				case 's':
				case 'S':
					configSetup.pktSizeTP += 500;
					if(configSetup.pktSizeTP > MAX_TP_PKT_SIZE) {
						configSetup.pktSizeTP = MIN_TP_PKT_SIZE;
					}

					break;
				case 'e':
				case 'E':
					configSetup.numRetriesTP = 
						(configSetup.numRetriesTP==MAX_NUM_RETRIES) ? MIN_NUM_RETRIES : configSetup.numRetriesTP+1;
					break;
				case 'd':
				case 'D':
					configSetup.numRetriesTP = 
						(configSetup.numRetriesTP==MIN_NUM_RETRIES) ? MAX_NUM_RETRIES : configSetup.numRetriesTP-1;
					break;
				case 'z':
				case 'Z':
					configSetup.numPacketsTP += 1000;
					if(configSetup.numPacketsTP > MAX_NUM_PKTS) {
						configSetup.numPacketsTP = MIN_NUM_PKTS;
					}

					break;
				case '1':
					configSetup.broadcastTP = !(configSetup.broadcastTP);
					break;
				case 0x1b:
					exitLoop = TRUE;
					uiPrintf("exiting\n");
					//cleanup descriptor queues, to free up mem
					art_cleanupTxRxMemory(devNum, TX_CLEAN | RX_CLEAN);
					return;
				default:
					uiPrintf("Unknown command\n");
					inputChange = 0;
					break;
				}
			}

			//Perform all change fields here so keep change from last time takes
			//effect, or any field changes from outside take effect.  Note that
			//have a little bit of overhead setting all these each time, but don't 
			//think it will be noticeable
//			setRegistersFromConfig(devNum);
			
			setInitialPowerControlMode(devNum, configSetup.dataRateIndexTP);

			printConfigSettings(devNum);

			if(configSetup.antenna == (USE_DESC_ANT | DESC_ANT_A))
				if(configSetup.quarterChannel) {
					uiPrintf("Freq = %7.1lf, ANT_A", configSetup.channel + 2.5);
				} else {
					uiPrintf("Freq = %d, ANT_A", (unsigned int)configSetup.channel);
				}
			else
				uiPrintf("Freq = %d, ANT_B", (unsigned int)configSetup.channel);

			if (configSetup.turbo==TURBO_ENABLE) uiPrintf(", Turbo\n");
			else if (configSetup.turbo==HALF_SPEED_MODE) uiPrintf(", Half rate\n");
			else if (configSetup.turbo==QUARTER_SPEED_MODE) uiPrintf(", Quarter rate\n");
			else uiPrintf("\n");

			switch(mode) {
				case 1:
					uiPrintf("Beginning transmission at rate ");
					if(configSetup.mode == MODE_11B) {
						uiPrintf("%s, ", DataRate_11b[configSetup.dataRateIndexTP]);
					}
					else {
						if(configSetup.turbo==TURBO_ENABLE) {
							uiPrintf("%s, ", DataRateStrTurbo[configSetup.dataRateIndexTP] );
						}
						else if (configSetup.turbo == HALF_SPEED_MODE)
						{
							uiPrintf("%s, ", DataRateStrHalf[configSetup.dataRateIndexTP] );
						}
						else if (configSetup.turbo == QUARTER_SPEED_MODE)
						{
							uiPrintf("%s, ", DataRateStrQuarter[configSetup.dataRateIndexTP] );
						}
						else {
							uiPrintf("%s, ", DataRateStr[configSetup.dataRateIndexTP]);
						}
					}
					uiPrintf("%d retries, %d pkts of size %d\n", 
						configSetup.numRetriesTP, configSetup.numPacketsTP, configSetup.pktSizeTP);
					break;
				case 2:
					uiPrintf("Beginning passive receive mode for tx throughput calculations\n");
					break; 
			}
		}

        switch(mode) {
        case 0: // Idle Mode
       		Sleep(200);	// delay 200 ms
            break;
        case 1:			// transmit mode
				if (thin_client) 
						max_num_pkts = MAX_NUM_PKTS_THIN_CLIENT;
				else
						max_num_pkts = MAX_NUM_PKTS;
				if(configSetup.numPacketsTP > max_num_pkts) {
						configSetup.numPacketsTP = MIN_NUM_PKTS;
				}

//			if(inputChange) {
				rateMask = 0x01 << configSetup.dataRateIndexTP;
				art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
		        art_txDataSetup(devNum, rateMask, rxStation, configSetup.numPacketsTP, configSetup.pktSizeTP, pattern, 2, 
					configSetup.numRetriesTP, configSetup.antenna, configSetup.broadcastTP);
				forcePowerOrPcdac(devNum);
				setRegistersFromConfig(devNum);
//				inputChange = 0;
//			}

			dBm_pwr = art_getPowerIndex(devNum, (A_INT32) configSetup.powerOutput) ;

			if((configSetup.mode == MODE_11G)&&((swDeviceID & 0xff) >= 0x0013)) {
				pcdac_pwr  = OFDM_11G_IDX;
				if ((configSetup.dataRateIndexTP <= 14) && (configSetup.dataRateIndexTP >= 8)) {
					pcdac_pwr = CCK_11G_IDX;
					dBm_pwr -= ( OFDM_11G_IDX - CCK_11G_IDX);				
				}
			}

			if((configSetup.mode == MODE_11B) && (configSetup.dataRateIndexTP < 4)) {
				art_txDataBegin(devNum, 90000, 0);
			}
			else {
				art_txDataBegin(devNum, 30000, 0);
			}
            //if (art_mdkErrNo!=0) {
			//	uiPrintf("Transmit Iteration Failed - error %d\n", art_mdkErrNo);
                //break;
            //}
			//get stats
			art_txGetStats(devNum, 0, 0, &tStats);
			uiPrintf("Good tx = %4d, failed tx = %4d, throughput = %5.2fMbps,", 
				tStats.goodPackets, tStats.excessiveRetries, (float)tStats.throughput/1000);

			// dynamic optimization
			if (configSetup.powerOvr == 1)
			{
				pCurrGainLadder->currGain = configSetup.gainI;
			} else
			{
				readGainF(pCurrGainLadder, devNum, (configSetup.powerControlMethod == DB_PWR_CTL) ? dBm_pwr : pcdac_pwr);
			}
			tweakGainF(pCurrGainLadder, devNum, 0);		
			uiPrintf(" gainF = %d [%s]", pCurrGainLadder->currGain, pCurrGainLadder->currStep->stepName);
			uiPrintf(" %s\n", (pCurrGainLadder->active ? "" : "off"));

//rddata = art_regRead(devNum, 0xa264);
//uiPrintf(" PDADC = %d, gain = %d dacGain = %d  [0x%08x, 0x%08x, 0x%08x] \n", 
//		(rddata >> 1) & 0xFF, (rddata >> 14) & 0x3f, (rddata >> 9) & 0x1f, art_regRead(devNum, 0xa258), art_regRead(devNum, 0xa26c), rddata);
            
			//had to make this longer (was 500) incase other side is an ap.
            break;
        case 2:			// receive mode
            if(inputChange) {
				art_resetDevice(devNum, rxStation, bssID, configSetup.channel, configSetup.turbo);
				art_setAntenna(devNum, configSetup.antenna);
				setRegistersFromConfig(devNum);
				art_rxDataSetup(devNum, 40, 2500, 0);
				art_rxDataStart(devNum);
				inputChange = 0;
			}
			else {
				//get a snapshot of receive
				Sleep(1000);	
				if(art_rxLastDescStatsSnapshot(devNum, &rxStats)) {
					if(rxStats.goodPackets) {
						uiPrintf("Last rx snapshot, good pkt, RSSI = %d, data length = %d", rxStats.DataSigStrength,
							rxStats.bodySize);

						if(configSetup.mode == MODE_11B) {
							rxStats.dataRate -= 7;
							uiPrintf(", Rate = %s\n", DataRate_11b[rxStats.dataRate]);
						}
						else {
							if(configSetup.turbo==TURBO_ENABLE) {
								uiPrintf(", Rate = %s\n", DataRateStrTurbo[rxStats.dataRate] );
							}
							else if(configSetup.turbo == HALF_SPEED_MODE) {
								uiPrintf(", Rate = %s\n", DataRateStrHalf[rxStats.dataRate] );
							}
							else if (configSetup.turbo == QUARTER_SPEED_MODE)
							{
								uiPrintf(", Rate = %s\n", DataRateStrQuarter[configSetup.dataRateIndexTP] );
							}
							else {
								uiPrintf(", Rate = %s\n", DataRateStr[rxStats.dataRate] );
							}
						}
					}
					else {
						if(rxStats.crcPackets) {
							uiPrintf("Last rx snapshot was CRC error\n");
						}
						else if(rxStats.decrypErrors) {
							uiPrintf("Last rx snapshot was decryptError\n");
						}
						else {
							uiPrintf("Last rx snapshot failed for unknown reason\n");
						}
					}
				} 
			}
            break;
        }
	}

}

A_BOOL	processCommonOptions 
(
 A_UINT32 devNum,
 A_INT16 inputKey
)
{
	A_BOOL processedKey = 1;
	int pcdac;

	switch(inputKey) {
	case 'p':
	case 'P':
	case 'l':
	case 'L':
		updateChannel(inputKey);
		break;
	case 'B':
	case 'b':
		// HALF_SPEED and QUARTER_SPEED only made available in 11a and 11o at ART level yet.
		// in 11g, not clear what cck will do.
		if(configSetup.mode != MODE_11B) {
			switch(configSetup.turbo) {
			case 0:
#ifndef __ATH_DJGPPDOS__
				if((configSetup.mode == MODE_11A) && (CalSetup.turboDisable)) {
#else
				if(configSetup.mode == MODE_11A) {
#endif
					//don't go into turbo mode, go to half speed instead
					configSetup.turbo = HALF_SPEED_MODE;
				}
#ifndef __ATH_DJGPPDOS__
				else if((configSetup.mode == MODE_11G) && (CalSetup.turboDisable_11g)) {
#else
				else if(configSetup.mode == MODE_11G) {
#endif
					//don't go into turbo mode, go to half speed instead
					configSetup.turbo = HALF_SPEED_MODE;
				}
				else {
					configSetup.turbo = TURBO_ENABLE;
				}

				break;
			case TURBO_ENABLE:
//				if ((configSetup.mode == MODE_11A) || (configSetup.mode == MODE_11O)){
				if (configSetup.mode != MODE_11B){
					configSetup.turbo = HALF_SPEED_MODE;
				} else {
					configSetup.turbo = 0;
				}
				break;
			case HALF_SPEED_MODE:
				// if half_speed is allowed, so is quarter_speed
				configSetup.turbo = QUARTER_SPEED_MODE;
				break;
			case QUARTER_SPEED_MODE:
				configSetup.turbo = 0;
				break;			
			}
			if(configSetup.turbo) {
				if((configSetup.dataRateIndex > 7)	&& (configSetup.dataRateIndex < 15)) {
					configSetup.dataRateIndex = 0;
				}
				if((configSetup.dataRateIndexTP > 7) && (configSetup.dataRateIndexTP < 15)) {
					configSetup.dataRateIndexTP = 0;
				}
			}

			updateConfigFromRegValues(devNum);
		}
		break;
	case '|':
		if (configSetup.pcDac & RAW_PCDAC_MASK)
			configSetup.pcDac &= ~RAW_PCDAC_MASK;
		else configSetup.pcDac |= RAW_PCDAC_MASK;
		break;
	case 'i':
		if(configSetup.pcDac == USE_REG_FILE) {
			configSetup.pcDac = INITIAL_PCDAC;
		}
		else {
			configSetup.pcDac++;
		}
		pcdac = configSetup.pcDac & ~RAW_PCDAC_MASK;
		if(pcdac > (((swDeviceID & 0xFF) >= 0x16) ? PCDAC_MAX_DERBY2 : PCDAC_MAX)) {
			configSetup.pcDac = PCDAC_MIN | (configSetup.pcDac & RAW_PCDAC_MASK);
		}
		configSetup.powerControlMethod = PCDAC_PWR_CTL;
		configSetup.useTargetPower = FALSE;					
		if ((((swDeviceID & 0xff) >= 0x0014)&&((swDeviceID & 0xff) != 0x0015)) && (configSetup.eepromLoad)) {
			//xpdgain gets set by devlib, don't want to apply here unless there
			//is an override.
			configSetup.applyXpdGain = 1;
		}
		break;
	case 'I':
		if(configSetup.pcDac == USE_REG_FILE) {
			configSetup.pcDac = INITIAL_PCDAC;
		}
		else {
			configSetup.pcDac+=10;
		}
		pcdac = configSetup.pcDac & ~RAW_PCDAC_MASK;
		if(pcdac > (((swDeviceID & 0xFF) >= 0x16) ? PCDAC_MAX_DERBY2 : PCDAC_MAX)) {
			configSetup.pcDac = PCDAC_MIN | (configSetup.pcDac & RAW_PCDAC_MASK);
		}
		configSetup.powerControlMethod = PCDAC_PWR_CTL;
		configSetup.useTargetPower = FALSE;					
		if ((((swDeviceID & 0xff) >= 0x0014)||((swDeviceID & 0xff) != 0x0016)) && (configSetup.eepromLoad)) {
			//xpdgain gets set by devlib, don't want to apply here unless there
			//is an override.
			configSetup.applyXpdGain = 1;
		}
		break;
	case 'j':
		if(configSetup.pcDac == USE_REG_FILE) {
			configSetup.pcDac = INITIAL_PCDAC;
		}
		else {
			configSetup.pcDac--;
		}
		pcdac = configSetup.pcDac & ~RAW_PCDAC_MASK;
		if(pcdac < PCDAC_MIN) {
			configSetup.pcDac = (((swDeviceID & 0xFF) >= 0x16) ? PCDAC_MAX_DERBY2 : PCDAC_MAX) | (configSetup.pcDac & RAW_PCDAC_MASK);
		}
		configSetup.powerControlMethod = PCDAC_PWR_CTL;
		configSetup.useTargetPower = FALSE;					
		if ((((swDeviceID & 0xff) >= 0x0014)&&((swDeviceID & 0xff) != 0x0015)) && (configSetup.eepromLoad)) {
			//xpdgain gets set by devlib, don't want to apply here unless there
			//is an override.
			configSetup.applyXpdGain = 1;
		}
		break;
	case 'J':
		if(configSetup.pcDac == USE_REG_FILE) {
			configSetup.pcDac = INITIAL_PCDAC;
		}
		else {
			configSetup.pcDac-=10;
		}
		pcdac = configSetup.pcDac & ~RAW_PCDAC_MASK;
		if(pcdac < PCDAC_MIN) {
			configSetup.pcDac = (((swDeviceID & 0xFF) >= 0x16) ? PCDAC_MAX_DERBY2 : PCDAC_MAX) | (configSetup.pcDac & RAW_PCDAC_MASK);
		}
		configSetup.powerControlMethod = PCDAC_PWR_CTL;
		configSetup.useTargetPower = FALSE;					
		if ((((swDeviceID & 0xff) >= 0x0014)&&((swDeviceID & 0xff) != 0x0015)) && (configSetup.eepromLoad)) {
			//xpdgain gets set by devlib, don't want to apply here unless there
			//is an override.
			configSetup.applyXpdGain = 1;
		}
		break;
	case 'f':
		if (configSetup.validCalData)
		{
			if(configSetup.powerOutput == USE_REG_FILE){
				configSetup.powerOutput = INITIAL_POWER_OUT;
			}
			else {
				configSetup.powerOutput++;
			}
#ifndef __ATH_DJGPPDOS__
			if(configSetup.powerOutput > POWER_OUT_MAX) {
				configSetup.powerOutput = CalSetup.minimumPower[calModeFor[configSetup.mode]];
			}
#endif
			configSetup.powerControlMethod = DB_PWR_CTL;
			configSetup.useTargetPower = FALSE;					
			
			if ((((swDeviceID & 0xff) >= 0x0014)&&((swDeviceID & 0xff) != 0x0015)) && configSetup.eepromLoad) {
				//xpdgain gets set by devlib, don't want to apply here unless there
				//is an override.
				configSetup.applyXpdGain = 0;
			}
		} else
		{
				invalidEepromMessage(3000);
		}
		break;
	case 'F':
		if (configSetup.validCalData)
		{
			if(configSetup.powerOutput == USE_REG_FILE) {
				configSetup.powerOutput = INITIAL_POWER_OUT;
			}
			else {
				configSetup.powerOutput+=10;
			}
#ifndef __ATH_DJGPPDOS__
			if(configSetup.powerOutput > POWER_OUT_MAX) {
				configSetup.powerOutput = CalSetup.minimumPower[calModeFor[configSetup.mode]];
			}
#endif
			configSetup.powerControlMethod = DB_PWR_CTL;
			configSetup.useTargetPower = FALSE;					
			if ((((swDeviceID & 0xff) >= 0x0014)&&((swDeviceID & 0xff) != 0x0015)) && configSetup.eepromLoad) {
				//xpdgain gets set by devlib, don't want to apply here unless there
				//is an override.
				configSetup.applyXpdGain = 0;
			}
		} else
		{
				invalidEepromMessage(3000);
		}
		break;
	case 'c':
		if (configSetup.validCalData)
		{
			if(configSetup.powerOutput == USE_REG_FILE) {
				configSetup.powerOutput = INITIAL_POWER_OUT;
			}
			else {
#ifndef __ATH_DJGPPDOS__
				if(configSetup.powerOutput == 
					CalSetup.minimumPower[calModeFor[configSetup.mode]]) {
					configSetup.powerOutput = POWER_OUT_MAX;
				}
				else {
					configSetup.powerOutput--;
				}
#endif
			}
			configSetup.powerControlMethod = DB_PWR_CTL;
			configSetup.useTargetPower = FALSE;					
			if ((((swDeviceID & 0xff) >= 0x0014)&&((swDeviceID & 0xff) != 0x0015)) && configSetup.eepromLoad) {
				//xpdgain gets set by devlib, don't want to apply here unless there
				//is an override.
				configSetup.applyXpdGain = 0;
			}
		} else
		{
				invalidEepromMessage(3000);
		}
		break;
	case 'C':
		if (configSetup.validCalData)
		{
			if(configSetup.powerOutput == USE_REG_FILE) {
				configSetup.powerOutput = INITIAL_POWER_OUT;
			}
			else {
#ifndef __ATH_DJGPPDOS__
				if(configSetup.powerOutput < 
					CalSetup.minimumPower[calModeFor[configSetup.mode]] + 10) {
					configSetup.powerOutput = POWER_OUT_MAX;
				}
				else {
					configSetup.powerOutput-=10;
				}
#endif
			}
			configSetup.powerControlMethod = DB_PWR_CTL;
			configSetup.useTargetPower = FALSE;					
			if ((((swDeviceID & 0xff) >= 0x0014)&&((swDeviceID & 0xff) != 0x0015)) && configSetup.eepromLoad) {
				//xpdgain gets set by devlib, don't want to apply here unless there
				//is an override.
				configSetup.applyXpdGain = 0;
			}
		} else
		{
				invalidEepromMessage(3000);
		}
		break;
	case 'u':
	case 'U':
		configSetup.ob++;
		if (configSetup.ob > OB_MAX) {
			configSetup.ob = OB_MIN;
		}
		break;
	case 'h':
	case 'H':
		configSetup.db++;
		if (configSetup.db > DB_MAX) {
			configSetup.db = DB_MIN;
		}
		break;
	case 'w':
	case 'W':
		configSetup.b_ob++;
		if (configSetup.b_ob > B_OB_MAX) {
			configSetup.b_ob = B_OB_MIN;
		}
		break;
	case 'q':
	case 'Q':
		configSetup.b_db++;
		if (configSetup.b_db > B_DB_MAX) {
			configSetup.b_db = B_DB_MIN;
		}
		break;
	case 'v':
	case 'V':
		if (!isGriffin(swDeviceID) ) {
		    if(configSetup.powerOvr == 0) {
			    //turn power override on
			    configSetup.powerOvr = 1;
				if(!isEagle(swDeviceID)) {
					initializeGainLadder(pCurrGainLadder);
					programNewGain(pCurrGainLadder, devNum, 0);
					pCurrGainLadder->active = FALSE;
				}
		    }
		    else {
			    //turn power override off
			    configSetup.powerOvr = 0;
			    if(!isEagle(swDeviceID)) {
					pCurrGainLadder->active = ((swDeviceID & 0xff) >= 0x14) ? FALSE : TRUE;
				}
		    }
	    }
		break;

	case 'm':
	case 'M':
		if(((swDeviceID & 0xff) <= 0x13) || ((swDeviceID & 0xff) == 0x15)) {
			if(configSetup.rfGainBoost == 5) {
				configSetup.rfGainBoost = 0;
			}
			else {
				configSetup.rfGainBoost = 5;
			}
		}
		else if ((swDeviceID & 0xff) >= 0x14) {
			configSetup.rfGainBoost ++;
			if (configSetup.rfGainBoost > 6) {
				configSetup.rfGainBoost = 0;
			}
		}
		break;

	case 'x':
	case 'X':
		if (!isGriffin(swDeviceID) && !isEagle(swDeviceID) ) {
	    	if(configSetup.externalPower == 0) {
		    	//turn externalPower on
			    configSetup.externalPower = 1;
		    }
		    else {
			    //turn externalPower off
			    configSetup.externalPower = 0;
		    }
		}
		break;
	case 'n':
//		if (!isGriffin(swDeviceID) && !isEagle(swDeviceID) ) {
	    	configSetup.xpdGainIndex++;
		    if(configSetup.xpdGainIndex >= MAX_XPD_GAIN_INDEX) {
			    configSetup.xpdGainIndex = 0;
		    }
		    configSetup.applyXpdGain = 1;  //we are in override mode now
//		}
		break;
	case 'N':
//		if ((swDeviceID & 0xFF) <= 0x15) {
//		if (!isGriffin(swDeviceID) && !isEagle(swDeviceID) ) {
	    	if (1) {
		    	configSetup.xpdGainIndex++;
			    if(configSetup.xpdGainIndex >= MAX_XPD_GAIN_INDEX) {
				    configSetup.xpdGainIndex = 0;
			    }
			    configSetup.applyXpdGain = 1;  //we are in override mode now
		    } else {
			    configSetup.xpdGainIndex2++;
			    if(configSetup.xpdGainIndex2 >= MAX_XPD_GAIN_INDEX) {
				    configSetup.xpdGainIndex2 = 0;
			    }
			    configSetup.applyXpdGain = 1;  //we are in override mode now
		    }
//		}
		break;
	case 'y':
		configSetup.gainI++;
		if(configSetup.gainI > gainIMax) {
			configSetup.gainI = gainIMin;
		}
		break;
	case 'Y':
		configSetup.gainI+=10;
		if(configSetup.gainI > gainIMax) {
			configSetup.gainI = gainIMin;
		}
		break;
	case 'g':
		configSetup.gainI--;
		if(configSetup.gainI < gainIMin) {
			configSetup.gainI = gainIMax;
		}
		break;
	case 'G':
		configSetup.gainI-=10;
		if(configSetup.gainI < gainIMin) {
			configSetup.gainI = gainIMax;
		}
		break;
	case 'a':
	case 'A':
		configSetup.antenna = 
			(configSetup.antenna==(USE_DESC_ANT|DESC_ANT_A)) ? (USE_DESC_ANT|DESC_ANT_B) : (USE_DESC_ANT|DESC_ANT_A);
		break;
	case '4':
		if (!isGriffin(swDeviceID) && !isEagle(swDeviceID) ) {
		if(!pCurrGainLadder->active) {
			loopIncrementFixedGain(pCurrGainLadder);
			programNewGain(pCurrGainLadder, devNum, 0);
		}
		else {
			processedKey = 0;
		}
		}
		break;
	default:
		processedKey = 0;
		break;
	}
	return(processedKey);
}

/*
void    toggleRefClock
( 
)
{ 
	if(configSetup.mode != MODE_11A) {
		uiPrintf("ref Clock changing not currently supported for 2.4Ghz modes\n");
		return;
	}

	//increment (or wrap) to the next ref clock
	if(configSetup.refClk == REF_CLK_20) {
		configSetup.refClk = REF_CLK_DYNAMIC;
	}
	else if(configSetup.refClk == REF_CLK_DYNAMIC) {
		configSetup.refClk = REF_CLK_2_5;
	}
	else {
		configSetup.refClk++;
	}

	//Alter the channel bounds as needed for the new refClock
	switch(configSetup.refClk) {
	case REF_CLK_2_5:
//		configSetup.maxChannel5G = ;
		break;

	}

}
*/

void	updateChannel
(
 A_INT16 inputKey
)
{
	switch(inputKey) {
	case 'p':
		if(configSetup.mode == MODE_11A) {
			configSetup.channel += configSetup.channelStep5g;

			if(configSetup.channel > configSetup.maxChannel5G) {
				configSetup.channel= configSetup.minChannel5G;
			}
		}
		else {
			if ((configSetup.channel >= 2412) && (configSetup.channel < 2472)) {
				configSetup.channel +=5;
			}

			else if ((configSetup.channel == 2472)  && (!configSetup.all2GChan)) {
				configSetup.channel = 2484;
			}
			else if ((configSetup.channel == 2484)  && (!configSetup.all2GChan)) {
				configSetup.channel = 2412;
			}
			else if(configSetup.channel == configSetup.maxChannel2G) {
				configSetup.channel = configSetup.minChannel2G;
			}
			else {
				if (configSetup.channel == 2484) {
					configSetup.channel -= 2;
				}
				configSetup.channel += 10;
			}
		}
	    break;
	case 'P':
		//P will have same effect as p for 2.4
		if(configSetup.mode == MODE_11A) {
			configSetup.channel += 100;
			if(configSetup.channel > configSetup.maxChannel5G) {
				configSetup.channel= configSetup.minChannel5G;
			}
		}
		else {
			if(!configSetup.all2GChan) {
				if ((configSetup.channel >= 2412) && (configSetup.channel < 2472)) {
					configSetup.channel +=5;
				}

				else if (configSetup.channel == 2472) {
					configSetup.channel = 2484;
				}
				else if (configSetup.channel == 2484) {
					configSetup.channel = 2412;
				}
			}
			else {
				if (configSetup.channel == 2484) {
					configSetup.channel -= 2;
				}

				configSetup.channel += 100;
				if(!(configSetup.channel % 5) && (configSetup.channel % 10)) {
					configSetup.channel += 5;
				}


				if(configSetup.channel > configSetup.maxChannel2G) {
					configSetup.channel = configSetup.minChannel2G;
				}

			}
		}
		break;
	case 'l':
		if(configSetup.mode == MODE_11A) {
			configSetup.channel -= configSetup.channelStep5g;
			
			if(configSetup.channel < configSetup.minChannel5G) {
				configSetup.channel=configSetup.maxChannel5G;
			}
		}
		else {
			if ((configSetup.channel > 2412) && (configSetup.channel <= 2472)) {
				configSetup.channel -=5;
			}

			else if (configSetup.channel == 2484) {
				configSetup.channel = 2472;
			}
			else if ((configSetup.channel == 2412)  && (!configSetup.all2GChan)) {
				configSetup.channel = 2484;
			}
			else if(configSetup.channel == configSetup.minChannel2G) {
				configSetup.channel = configSetup.maxChannel2G;
			}
			else  {
				configSetup.channel -= 10;
			}
		}
		break;
	case 'L':
		//L will have same effect as l for 2.4
		if(configSetup.mode == MODE_11A) {
			configSetup.channel -= 100;
			if (configSetup.channel < 5725)
				configSetup.channel = 10*((A_UINT16) configSetup.channel/10) ;
			if(configSetup.channel < configSetup.minChannel5G) {
				configSetup.channel=configSetup.maxChannel5G;
			}
		}
		else {
			if(!configSetup.all2GChan) {
				if ((configSetup.channel > 2412) && (configSetup.channel <= 2472)) {
					configSetup.channel -=5;
				}

				else if (configSetup.channel == 2484) {
					configSetup.channel = 2472;
				}
				else if (configSetup.channel == 2412) {
					configSetup.channel = 2484;
				}
			}
			else {
				if (configSetup.channel == 2484) {
					configSetup.channel -= 2;
				}

				configSetup.channel -= 100;
				if(!(configSetup.channel % 5) && (configSetup.channel % 10)) {
					configSetup.channel -= 5;
				}

				if(configSetup.channel < configSetup.minChannel2G) {
					configSetup.channel = configSetup.maxChannel2G;
				}
			}
		
		}
		break;
	default:
		uiPrintf("INTERNAL SOFTWARE ERROR: in updateChannel, should not get here\n");
		return;
	}
	return;
}

#define NUM_DOMAINS 4

#if 0
void changeDomain(A_UINT32 devNum) 
{
	A_UINT32 origDomain, validDomains = 0, i, j;
	A_UINT32 lastvalid, newDomain;
	A_UINT32 domain[NUM_DOMAINS];
	A_BOOL done = FALSE;

    origDomain  = art_eepromRead(devNum, 0xbf)&0xff;

	domain[0] = (art_eepromRead(devNum, 0xc6)>>8)&0xff;
	domain[1] = art_eepromRead(devNum, 0xc6)&0xff;
	domain[2] = (art_eepromRead(devNum, 0xc7)>>8)&0xff;
	domain[3] = art_eepromRead(devNum, 0xc7)&0xff;

    for(i = 0; (i < NUM_DOMAINS) && (done == FALSE); i++) {
        if(domain[i] != 0xFF) {
            validDomains++;
            lastvalid = i;
        }
        if(origDomain == domain[i]) {
            for(j = 1; j <= NUM_DOMAINS; j++) {
                if(domain[(i+j) % NUM_DOMAINS] != 0xFF) {
					newDomain = (i+j) % NUM_DOMAINS;
                    art_eepromWrite(devNum, 0xbf, domain[newDomain] );
                    uiPrintf("Changed to regulatory domain %d\n", newDomain + 1);
					done = TRUE;
					break;
                }
            }

        }
    }
    if(i == NUM_DOMAINS) { 
        if(validDomains) {
			newDomain = lastvalid;
            art_eepromWrite(devNum, 0xbf, domain[newDomain] );
            uiPrintf("Changed to regulatory domain %d\n", newDomain+1);
        }
        else {
            uiPrintf("No valid regulatory domains on card - no domain change possible\n");
            return;
        }
    }

#ifndef LINUX
#ifndef __ATH_DJGPPDOS__
	displayDomain(domain[newDomain]);
#endif
#endif

	// Set the next resetDevice up to reread the EEPROM
	art_rereadProm(devNum);
}
#endif


A_BOOL parseConfig(void) 
{
    FILE *fStream;
    char lineBuf[222], *pLine;
	char tempFilename[MAX_FILE_LENGTH];
	char tempEarFilename[MAX_FILE_LENGTH];
	A_UINT32 tempSubSystemID;

    uiPrintf("\nReading in Configuration Setup from %s\n", CONFIGSETUP_FILE);
    if( (fStream = fopen( CONFIGSETUP_FILE, "r")) == NULL ) {
        uiPrintf("Failed to open %s - using Defaults\n", CONFIGSETUP_FILE);
        return 0;
    }

    while(fgets(lineBuf, 120, fStream) != NULL) {
        pLine = lineBuf;
        while(isspace(*pLine)) pLine++;
        if((*pLine == '#') || (*pLine == 0)) {
            continue;
        }
        else if(strnicmp("5_CHANNEL_MHZ", pLine, strlen("5_CHANNEL_MHZ")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.channel5)) {
                uiPrintf("Unable to read the 5_CHANNEL_MHZ from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("2_4_CHANNEL_MHZ", pLine, strlen("2_4_CHANNEL_MHZ")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.channel2_4)) {
                uiPrintf("Unable to read the 2_4_CHANNEL_MHZ from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("EEPROM_LOAD_OVERRIDE", pLine, strlen("EEPROM_LOAD_OVERRIDE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.eepromLoadOverride)) {
                uiPrintf("Unable to read the EEPROM_LOAD_OVERRIDE from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
#ifdef HEADER_LOAD_SCHEME
        else if(strnicmp("EEPROM_HEADER_LOAD", pLine, strlen("EEPROM_HEADER_LOAD")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%ld", &configSetup.eepromHeaderLoad)) {
                uiPrintf("Unable to read the EEPROM_HEADER_LOAD from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
#endif //HEADER_LOAD_SCHEME
        else if(strnicmp("COMPUTE_CALSETUP_NAME", pLine, strlen("COMPUTE_CALSETUP_NAME")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.computeCalsetupName)) {
                uiPrintf("Unable to read the COMPUTE_CALSETUP_NAME from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("TURBO", pLine, strlen("TURBO")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.turbo)) {
                uiPrintf("Unable to read the TURBO from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("MODE", pLine, strlen("MODE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.mode)) {
                uiPrintf("Unable to read MODE from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("LOGGING", pLine, strlen("LOGGING")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.logging)) {
                uiPrintf("Unable to read LOGGING from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("LOG_FILE", pLine, strlen("LOG_FILE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%s", configSetup.logFile)) {
                uiPrintf("Unable to read LOG_FILE from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("ENABLE_PRINT", pLine, strlen("ENABLE_PRINT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.enablePrint)) {
                uiPrintf("Unable to read ENABLE_PRINT from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("ENABLE_CAL", pLine, strlen("ENABLE_CAL")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.enableCal)) {
                uiPrintf("Unable to read ENABLE_CAL from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
         else if(strnicmp("DUT_CARD_SSID", pLine, strlen("DUT_CARD_SSID")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

			if (!configSetup.userDutIdOverride) {
				if(!sscanf(pLine, "%x", &configSetup.dutSSID)) {
					uiPrintf("Unable to read the DUT_CARD_SSID from %s\n", CONFIGSETUP_FILE);
					return 0;
				}
			}
        }
        else if(strnicmp("ENABLE_LABEL_SCHEME", pLine, strlen("ENABLE_LABEL_SCHEME")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.enableLabelScheme)) {
                uiPrintf("Unable to read ENABLE_LABEL_SCHEME from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
		 else if(strnicmp("ALL_2G_CHANNELS", pLine, strlen("ALL_2G_CHANNELS")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.all2GChan)) {
                uiPrintf("Unable to read the ALL_2G_CHANNELS from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("MANUFACTURER_NAME", pLine, strlen("MANUFACTURER_NAME")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%s", configSetup.manufName)) {
                uiPrintf("Unable to read MANUFACTURER_NAME from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("AR5210_AR5110_CFG", pLine, strlen("AR5210_AR5110_CFG")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%s", configSetup.creteFezCfg)) {
                uiPrintf("Unable to read AR5210_AR5110_CFG from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("AR5211_AR5111_CFG", pLine, strlen("AR5211_AR5111_CFG")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%s", configSetup.qmacSombreroCfg)) {
                uiPrintf("Unable to read AR5210_AR5111_CFG from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("AR5211_AR5111_AR2111_CFG", pLine, strlen("AR5211_AR5111_AR2111_CFG")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%s", configSetup.qmacSombreroBeanieCfg)) {
                uiPrintf("Unable to read AR5210_AR5111_AR2111_CFG from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("AR5001_CFG", pLine, strlen("AR5001_CFG")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%s", configSetup.oahuCfg)) {
                uiPrintf("Unable to read AR5001_CFG from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("AR5213_CFG", pLine, strlen("AR5213_CFG")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%s", configSetup.veniceCfg)) {
                uiPrintf("Unable to read AR5213_CFG from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("RATE_MASK", pLine, strlen("RATE_MASK")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%x", &configSetup.rateMask)) {
                uiPrintf("Unable to read the RATE_MASK from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
			configSetup.rateMask = configSetup.rateMask;// & 0xff;
        }

        else if(strnicmp("LINK_PACKET_SIZE", pLine, strlen("LINK_PACKET_SIZE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.linkPacketSize)) {
                uiPrintf("Unable to read the LINK_PACKET_SIZE from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }

        else if(strnicmp("LINK_NUM_PACKETS", pLine, strlen("LINK_NUM_PACKETS")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.linkNumPackets)) {
                uiPrintf("Unable to read the LINK_NUM_PACKETS from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }

        else if(strnicmp("USE_ALTERNATE_BEANIE_CHAN", pLine, strlen("USE_ALTERNATE_BEANIE_CHAN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.beanie2928Mode)) {
                uiPrintf("Unable to read the USE_ALTERNATE_BEANIE_CHAN from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("EEP_FILE_DIR", pLine, strlen("EEP_FILE_DIR")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%s", configSetup.eepFileDir)) {
                uiPrintf("Unable to read EEP_FILE_DIR from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("CAL_EEP_FILE_NAME", pLine, strlen("CAL_EEP_FILE_NAME")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%s", configSetup.cal_eepFileName)) {
                uiPrintf("Unable to read CAL_EEP_FILE_NAME from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("CAL_USB_EEP_FILE_NAME", pLine, strlen("CAL_USB_EEP_FILE_NAME")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%s", configSetup.cal_usb_eepFileName)) {
                uiPrintf("Unable to read CAL_USB_EEP_FILE_NAME from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }

        else if(strnicmp("BLANK_EEP_ID", pLine, strlen("BLANK_EEP_ID")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%x", &configSetup.blankEepID)) {
                uiPrintf("Unable to read the BLANK_EEP_ID from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }

        else if(strnicmp("ENABLE_XR", pLine, strlen("ENABLE_XR")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.enableXR)) {
                uiPrintf("Unable to read the ENABLE_XR from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("LOAD_EAR", pLine, strlen("LOAD_EAR")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.loadEar)) {
                uiPrintf("Unable to read the LOAD_EAR from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("NO_EEPROM_UNLOCK", pLine, strlen("NO_EEPROM_UNLOCK")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.noEepromUnlock)) {
                uiPrintf("Unable to read the NO_EEPROM_UNLOCK from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("REF_CLOCK", pLine, strlen("REF_CLOCK")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.refClk)) {
                uiPrintf("Unable to read the REF_CLOCK from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("APPLY_CTL", pLine, strlen("APPLY_CTL")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.applyCtlLimit)) {
                uiPrintf("Unable to read APPLY_CTL from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("CTL_VALUE", pLine, strlen("CTL_VALUE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%hx", &configSetup.ctlToApply)) {
                uiPrintf("Unable to read CTL_VALUE from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
        else if(strnicmp("OVERRIDE_PLL", pLine, strlen("OVERRIDE_PLL")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%x", &configSetup.pllValue)) {
                uiPrintf("Unable to read OVERRIDE_PLL from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
			configSetup.applyPLLOverride = 1;
        }
        else if(strnicmp("DEBUG_INFO", pLine, strlen("DEBUG_INFO")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &configSetup.debugInfo)) {
                uiPrintf("Unable to read DEBUG_INFO from %s\n", CONFIGSETUP_FILE);
				return 0;
            }
        }
#ifndef __ATH_DJGPPDOS__
        else if(strnicmp("CFG_TABLE", pLine, strlen("CFG_TABLE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        
			//get the subsystemID
			pLine = strtok( pLine, delimiters ); //get past any white space etc
            
			if(!sscanf(pLine, "%x", &tempSubSystemID)) {
                uiPrintf("Unable to read the CFG_TABLE subsystemID from %s\n", CONFIGSETUP_FILE);
				return 0;
            }

			//get the filename
			pLine = strtok( NULL, delimiters ); //get past any white space etc
            
			if(!sscanf(pLine, "%s", tempFilename)) {
                uiPrintf("Unable to read the CFG_TABLE filename from %s\n", CONFIGSETUP_FILE);
				return 0;
            }

			//get the ear filename if there is one
			pLine = strtok( NULL, delimiters );

			if(pLine == NULL) {
				//no ear file is present
				tempEarFilename[0] = '\0';
			}
			else if (pLine[0] == '#') {
				//There is a comment at the end, so no ear file
				tempEarFilename[0] = '\0';
			}
			else {
				if(!sscanf(pLine, "%s", tempEarFilename)) {
					uiPrintf("Unable to read the CFG_TABLE ear filename from %s\n", CONFIGSETUP_FILE);
					return 0;
				}
			}
			//add new items to the table
			if(!addToCfgTable((A_UINT16)tempSubSystemID, tempFilename, tempEarFilename)) {
				uiPrintf("Unable to store cfg table\n");
				return(0);
			}
		}
#endif //__ATH_DJGPPDOS__

	}  // End while (get line)
    fclose(fStream);
	return 1;
} 


#ifndef __ATH_DJGPPDOS__
A_BOOL addToCfgTable
(
 A_UINT16 subSystemID,
 A_CHAR *pFilename,
 A_CHAR *pEarFilename
)
{
	A_CHAR temp[1000];
	CFG_TABLE_ELEMENT	*tempPtr=NULL;
	A_INT32  tempFileIdentifier;

	//put together the full filename (including path and check for existence
	_makepath(temp, NULL, configSetup.eepFileDir, pFilename, NULL);
	
	//check for read access to the file
//	if(_access(temp, 2)) {
//		uiPrintf("Unable to open filename %s\n");
//		return FALSE;
//	}

	//create a new table
	tempPtr = configSetup.cfgTable.pCfgTableElements;
	configSetup.cfgTable.pCfgTableElements = (CFG_TABLE_ELEMENT *)malloc(sizeof(CFG_TABLE_ELEMENT) * (configSetup.cfgTable.sizeCfgTable + 1));
	if(NULL == configSetup.cfgTable.pCfgTableElements) {
		uiPrintf("Unable to allocate memory for new cfg table element\n");
		return FALSE;
	}

	//copy over the existing elements to the new table
	memcpy(configSetup.cfgTable.pCfgTableElements, tempPtr, sizeof(CFG_TABLE_ELEMENT) * configSetup.cfgTable.sizeCfgTable);
	
	//update the new element
	configSetup.cfgTable.pCfgTableElements[configSetup.cfgTable.sizeCfgTable].subsystemID = subSystemID;
	strcpy(configSetup.cfgTable.pCfgTableElements[configSetup.cfgTable.sizeCfgTable].eepFilename, temp);
	if(pEarFilename[0] != '\0') {
		_makepath(temp, NULL, configSetup.eepFileDir, pEarFilename, NULL);
		strcpy(configSetup.cfgTable.pCfgTableElements[configSetup.cfgTable.sizeCfgTable].earFilename, temp);
		configSetup.earFileVersion = getEarPerforceVersion(temp);
		if(configSetup.earFileVersion == 0) {
			uiPrintf("addToCfgTable::Error: unable to get perforce version ID from ear file\n");
			return FALSE;
		}
		tempFileIdentifier = getEarFileIdentifier(pEarFilename);
		if(tempFileIdentifier == -1) {
			uiPrintf("Unable to get file identifier from ear file\n");
			return FALSE;
		}

		configSetup.earFileIdentifier = (A_UINT32)tempFileIdentifier;
	}
	else {
		configSetup.cfgTable.pCfgTableElements[configSetup.cfgTable.sizeCfgTable].earFilename[0] = '\0';
	}

//	uiPrintf("SNOOP: EAR file = %s\n", configSetup.cfgTable.pCfgTableElements[configSetup.cfgTable.sizeCfgTable].earFilename); 

	if(tempPtr) {
		A_FREE(tempPtr);
	}
	configSetup.cfgTable.sizeCfgTable++;
	return TRUE;
}
#endif //__ATH_DJGPPDOS__

void clearScreen(void)
{
#ifdef LINUX
	system("clear");
#else
	//windows way to do this
	system("cls");
#endif
}

void updateConfigFromRegValues(A_UINT32 devNum)
{
	A_UINT32 pwdxpd;
	A_UINT32 ploSelect;
	A_UCHAR i;
	A_UINT32 xpdGain;
	A_UINT32 rfGainI;
	A_UINT32 rfGainSel;

	//read OB
	configSetup.ob = art_getFieldForMode(devNum, "rf_ob", configSetup.mode, configSetup.turbo);

	//read DB
	configSetup.db = art_getFieldForMode(devNum, "rf_db", configSetup.mode, configSetup.turbo);
	
//++JC++
	if(((swDeviceID & 0xff) >= 0x0012) && (!isGriffin(swDeviceID)) && (!isSwan(swDeviceID))) {  // For Griffin
	//++JC++ if((swDeviceID & 0xff) >= 0x0012) {
		//read B_OB
		configSetup.b_ob = art_getFieldForMode(devNum, "rf_b_ob", configSetup.mode, configSetup.turbo);

		//read B_DB
		configSetup.b_db = art_getFieldForMode(devNum, "rf_b_db", configSetup.mode, configSetup.turbo);
	}

	if (isGriffin(swDeviceID) || isEagle(swDeviceID)) {
		//read power override, incase we are loading ear, need to read register,
		//can't use field.
//		configSetup.powerOvr = art_getFieldForMode(devNum, "bb_force_dac_gain", configSetup.mode, configSetup.turbo);
//		configSetup.powerOvr = art_regRead(devNum, TPCRG1_REG) & BB_FORCE_DAC_GAIN_SET;
	} else {
		//read power override
		configSetup.powerOvr = art_getFieldForMode(devNum, "rf_ovr", configSetup.mode, configSetup.turbo);
	}

	//read gainI
	if(!isEagle(swDeviceID) && !isGriffin(swDeviceID)) {
		configSetup.gainI = art_getFieldForMode(devNum, "rf_gain_I", configSetup.mode, configSetup.turbo);

	    //calculate the gainBoost
    	rfGainI = art_getFieldForMode(devNum, "rf_rfgain_I", configSetup.mode, configSetup.turbo);

	    if(((swDeviceID & 0xff) <= 0x13) || ((swDeviceID & 0xff) == 0x15)) {
		    rfGainSel = art_getFieldForMode(devNum, "rf_rfgainsel", configSetup.mode, configSetup.turbo);
		
	        if((rfGainI == 0)&&(rfGainSel == 0)) {
			    configSetup.rfGainBoost = 0;
		    }
		    else if((rfGainI == 0)&&(rfGainSel == 1)) {
			    configSetup.rfGainBoost = 5;
		    }
	    }
	    else if ((swDeviceID & 0xff) >= 0x14) {
		    configSetup.rfGainBoost = (A_UCHAR)rfGainI;
	    }
	}

	//read and calculate external power
	if(((swDeviceID & 0xff) <= 0x13) || ((swDeviceID & 0xff) == 0x15)) {
		pwdxpd = art_getFieldForMode(devNum, "rf_pwdxpd", configSetup.mode, configSetup.turbo);
		ploSelect = art_getFieldForMode(devNum, "rf_plo_sel", configSetup.mode, configSetup.turbo);

		if (pwdxpd && !ploSelect) {
			configSetup.externalPower = 0;
		}
		else if (!pwdxpd && ploSelect) {
			configSetup.externalPower = 1;
		}
		else {
			configSetup.externalPower = EXTERNAL_PWR_UNKNOWN;
		}
	} else {
		if (isGriffin(swDeviceID) || isEagle(swDeviceID)) {
			configSetup.externalPower = 1;
		} else {
			configSetup.externalPower = (A_UCHAR) art_getFieldForMode(devNum, "rf_xpdsel", configSetup.mode, configSetup.turbo);
		}
	}
	
	//read the xpdGain
	if((swDeviceID & 0xff) <= 0x0015) {
		xpdGain = art_getFieldForMode(devNum, "rf_xpd_gain", configSetup.mode, configSetup.turbo);
	}
	else {
		xpdGain = art_getXpdgainForPower(devNum, configSetup.powerOutput);
	}

	for (i = 0; i < MAX_XPD_GAIN_INDEX; i++) {
		if (gainValues[i].regValue == xpdGain) {
			break;
		}
	}
	configSetup.xpdGainIndex = i;	//if match not found, then MAX_XPD_GAIN_INDEX will cause
									//an undefined message to be displayed
	configSetup.xpdGainIndex2 = i;

//#ifndef __ATH_DJGPPDOS__
	configSetup.artAniLevel[ART_ANI_TYPE_NI] = art_getArtAniLevel(devNum, ART_ANI_TYPE_NI);
	configSetup.artAniLevel[ART_ANI_TYPE_BI] = art_getArtAniLevel(devNum, ART_ANI_TYPE_BI);
	configSetup.artAniLevel[ART_ANI_TYPE_SI] = art_getArtAniLevel(devNum, ART_ANI_TYPE_SI);
//#endif

	return;
}

//check to see if should force the power of pcdac values
void forcePowerOrPcdac
(
 A_UINT32 devNum
)
{
	A_UINT16	powerVals[NUM_RATES];
	A_UINT16	i;

	if(configSetup.powerControlMethod == DB_PWR_CTL)
	{
		if (isGriffin(swDeviceID) || isEagle(swDeviceID)) {
			//incase we are loading ear, can't use griffin specific field names
			//art_writeField(devNum, "bb_force_dac_gain", 0);
    		art_regWrite(devNum, TPCRG1_REG, art_regRead(devNum, TPCRG1_REG) & ~BB_FORCE_DAC_GAIN_SET);
			//art_writeField(devNum, "bb_force_tx_gain", 0);
			art_regWrite(devNum, TPCRG7_REG, art_regRead(devNum, TPCRG7_REG) & ~BB_FORCE_TX_GAIN_SET);
		}

		if((configSetup.powerOutput != USE_REG_FILE) &&
			(!configSetup.useTargetPower)){
			for(i = 0; i < NUM_RATES; i++) {
				powerVals[i] = configSetup.powerOutput;
			}
			art_forcePowerTxMax(devNum, powerVals);
		}
	} else if (configSetup.powerControlMethod == PCDAC_PWR_CTL)
	{
		if (isGriffin(swDeviceID) || isEagle(swDeviceID)) {
			//art_writeField(devNum, "bb_force_dac_gain", 1);
			art_regWrite(devNum, TPCRG1_REG, art_regRead(devNum, TPCRG1_REG) | BB_FORCE_DAC_GAIN_SET);
			//art_writeField(devNum, "bb_force_tx_gain", 1);
			art_regWrite(devNum, TPCRG7_REG, art_regRead(devNum, TPCRG7_REG) | BB_FORCE_TX_GAIN_SET);
		}
	
		for(i = 0; i < NUM_RATES; i++) {
			if((configSetup.mode == MODE_11G)&&((swDeviceID & 0xff) >= 0x0013)) {
				powerVals[i] = OFDM_11G_IDX;
			} else {
				powerVals[i] = 0;
			}
		}
//		if (!isGriffin(swDeviceID)) {
		art_forcePowerTxMax(devNum, powerVals);
//		}
		if (configSetup.pcDac != USE_REG_FILE) {
			art_ForceSinglePCDACTable(devNum, (A_UINT16)configSetup.pcDac);
		}
		
	} 

	return;
}


void setRegistersFromConfig(A_UINT32 devNum)
{
	A_UINT32 rfGainSel;
	A_UINT32 rfGainI;

	art_writeField(devNum, "rf_ob", configSetup.ob);
	art_writeField(devNum, "rf_db", configSetup.db);

//++JC++
	if (isGriffin(swDeviceID)) {
//		art_writeField(devNum, "bb_force_dac_gain", configSetup.powerOvr);
//		art_writeField(devNum, "bb_force_tx_gain", configSetup.powerOvr);
		art_writeField(devNum, "bb_force_pdadc_gain", configSetup.powerOvr);
	}
	else if( isEagle(swDeviceID) && (!isSwan(swDeviceID))) {
		art_writeField(devNum, "rf_b_ob", configSetup.b_ob);
		art_writeField(devNum, "rf_b_db", configSetup.b_db); 
		art_writeField(devNum, "bb_force_pdadc_gain", configSetup.powerOvr);
	} else {
		if( ((swDeviceID & 0xff) >= 0x0012) && (!isSwan(swDeviceID))) {
			art_writeField(devNum, "rf_b_ob", configSetup.b_ob);
			art_writeField(devNum, "rf_b_db", configSetup.b_db); 
		}

		if(!isSwan(swDeviceID)){
			art_writeField(devNum, "rf_ovr", configSetup.powerOvr);
		}
	
		if(((swDeviceID & 0xff) <= 0x13) || ((swDeviceID & 0xff) == 0x15)) {
			if(configSetup.externalPower == 1) {
				art_writeField(devNum, "rf_pwdxpd", 0);
				art_writeField(devNum, "rf_plo_sel", 1);
			}
			else {
				art_writeField(devNum, "rf_pwdxpd", 1);
				art_writeField(devNum, "rf_plo_sel", 0);
			}
		}
		else if(!isSwan(swDeviceID)) {
			art_writeField(devNum, "rf_xpdsel", configSetup.externalPower);
		}
	}
//++JC++
	
	if(configSetup.powerOvr) {
		if(isEagle(swDeviceID) || isGriffin(swDeviceID)) {
			ForceSinglePCDACTableGriffin(devNum, configSetup.gainI, GAIN_OVERRIDE);			
		}
		else {
			art_writeField(devNum, "rf_gain_I", configSetup.gainI);

			if(((swDeviceID & 0xff) <= 0x13) || ((swDeviceID & 0xff) == 0x15)) {
				switch(configSetup.rfGainBoost) {
				case 0:
					rfGainSel = 0;
					rfGainI = 0;
					break;
				case 5:
					rfGainSel = 1;
					rfGainI = 0;
					break;
		//		case 10:
		//			rfGainI = 1;
		//			break;
				default:
					uiPrintf("ERROR: Illegal rfGainBoost value, should never get here. Exit\n");
					exit(0);
				}


				art_writeField(devNum, "rf_rfgainsel", rfGainSel);
				art_writeField(devNum, "rf_rfgain_I", rfGainI);
			}
			else if((swDeviceID & 0xff) >= 0x14) {
				art_writeField(devNum, "rf_rfgain_I", configSetup.rfGainBoost);
			}
		}
	}

	//set xpdgain if needed
	if((configSetup.powerOvr == 0) && (configSetup.applyXpdGain)) {
		if(configSetup.xpdGainIndex < MAX_XPD_GAIN_INDEX) {
			if((swDeviceID & 0xff) <= 0x0015) {
				art_writeField(devNum, "rf_xpd_gain", gainValues[configSetup.xpdGainIndex].regValue);
			}
//++JC++
			else if (isGriffin(swDeviceID) || isEagle(swDeviceID)) {
				art_writeField(devNum, "bb_force_pdadc_gain", 1);
				art_writeField(devNum, "bb_forced_pdadc_gain", configSetup.xpdGainIndex);
			}
//++JC++
			else {
				art_writeField(devNum, "rf_pdgain_lo", gainValues[configSetup.xpdGainIndex].regValue);
//				art_writeField(devNum, "rf_pdgain_hi", gainValues[configSetup.xpdGainIndex2].regValue);
				art_writeField(devNum, "rf_pdgain_hi", gainValues[configSetup.xpdGainIndex].regValue);
			}
		}
	}
	if(configSetup.loadEar && isGriffin(swDeviceID)) {
		//need to force a resetDevice so can get the correct EAR bank 6 write
		art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);		
	}
	return;
}

void printConfigSettings(A_UINT32 devNum)
{
	A_UINT32 xpdGain;
	A_UCHAR i;
	CTL_POWER_INFO ctlPowerInfo;
	A_INT32 ctlPower = 0xff;
	CTL_PWR_RANGE *pChannelPowerInfo;

	//print the values that have been set
	if(configSetup.powerOvr == 0) {
		uiPrintf("Power control mode:\n");
		if (configSetup.powerControlMethod != DB_PWR_CTL)
		{
			if (configSetup.pcDac == USE_REG_FILE) {
				uiPrintf("PCDAC set externally, ");
			}
			else {
				uiPrintf("PCDAC = %d%s, ", (A_UCHAR)(configSetup.pcDac & ~RAW_PCDAC_MASK), (configSetup.pcDac & RAW_PCDAC_MASK) ? "raw" : "");
			}
		} else
		{
			if (configSetup.powerOutput == USE_REG_FILE) {
				uiPrintf("Output power set externally, ");
			} else if (configSetup.useTargetPower) {
				//if we are applying a ctl, then need to check for being ctl limited
				if((configSetup.applyCtlLimit) && art_getCtlPowerInfo(devNum, &ctlPowerInfo)) {					
					if((configSetup.mode == MODE_11B) || 
						((configSetup.mode == MODE_11G) && (configSetup.dataRateIndex >= 8) && (configSetup.dataRateIndex <= 14))) 
					{
						ctlPower = ctlPowerInfo.powerCck;
						pChannelPowerInfo = ctlPowerInfo.ctlPowerChannelRangeCck;
					}
					else {
						ctlPower = ctlPowerInfo.powerOfdm;
						pChannelPowerInfo = ctlPowerInfo.ctlPowerChannelRangeOfdm;
					}

					//print the channel/power profile if debug is asked for
					if(configSetup.debugInfo) {
						uiPrintf("Ctl Channel Profile: \n");
						for(i = 0; pChannelPowerInfo[i].lowChannel; i++) {
							//note subtract 1 from the high channel since algorithm is
							//lowChannel >= channel < highChannel
							uiPrintf(" %d - %d: %4.1f\n", pChannelPowerInfo[i].lowChannel,
								pChannelPowerInfo[i].highChannel - 1, ((float)pChannelPowerInfo[i].twicePower)/2);
						}
					}
				}
				if(ctlPower == configSetup.powerOutput) {
					uiPrintf("Ctl Limited Power = %4.1f, ", ((float)configSetup.powerOutput)/2);
				}
				else {
					if (configSetup.debugInfo) {
						uiPrintf("Target Power = %4.1f[%4.1f], ", ((float)configSetup.powerOutput)/2, art_getMaxLinPower(devNum));
					} else {
						uiPrintf("Target Power = %4.1f, ", ((float)configSetup.powerOutput)/2);
					}
				}
			} else {
				if (configSetup.debugInfo) {
					uiPrintf("Output power = %4.1f[%4.1f], ", ((float)configSetup.powerOutput)/2, art_getMaxLinPower(devNum));
				} else {
					uiPrintf("Output power = %4.1f, ", ((float)configSetup.powerOutput)/2);
				}
			}
		}
		uiPrintf("ext power detector = %d,", configSetup.externalPower); 
		if(!configSetup.powerOvr) {

			if(!configSetup.applyXpdGain) {
				//This is the case where the xpdgain is applyed in library, get that value
//				art_getField(devNum, "rf_xpd_gain", &baseValue, &turboValue);
//				xpdGain = ((turbo == TURBO_ENABLE) ? turboValue : baseValue);
				xpdGain = art_getXpdgainForPower(devNum, configSetup.powerOutput);
				for (i = 0; i < MAX_XPD_GAIN_INDEX; i++) {
					if (gainValues[i].regValue == xpdGain) {
						break;
					}
				}
				configSetup.xpdGainIndex = i;	//if match not found, then MAX_XPD_GAIN_INDEX will cause
												//an undefined message to be displayed
				configSetup.xpdGainIndex2 = i;

			}
			if(configSetup.xpdGainIndex < MAX_XPD_GAIN_INDEX) {
//++JC++
				if (isGriffin(swDeviceID) || isEagle(swDeviceID)) {
					uiPrintf(" xpdGain = %d, ", configSetup.xpdGainIndex);
				} else {
					uiPrintf(" xpdGain = %d, ", gainValues[configSetup.xpdGainIndex]);
				}
//++JC++
			}
			else {
				uiPrintf(" xpdGain value is undefined, ");
			}

//			if ((swDeviceID & 0xFF) >= 0x15) {
			if (0) {
				if(configSetup.xpdGainIndex2 < MAX_XPD_GAIN_INDEX) {
					uiPrintf(" xpdGain_hi = %d, ", gainValues[configSetup.xpdGainIndex2]);
				}
				else {
					uiPrintf(" xpdGain_hi value is undefined, ");
				}
			}

		}
	}
	else {
		uiPrintf("Override power control mode:\n");
		uiPrintf("gainI = %d, ", configSetup.gainI);
		
		if(((swDeviceID & 0xff) <= 0x13) || ((swDeviceID & 0xff) == 0x15)) {
			uiPrintf("rf_GainBoost = %d, ", configSetup.rfGainBoost);
		}
		else if((swDeviceID & 0xff) >= 0x14) {
			uiPrintf("rfgain_I = %d, ", configSetup.rfGainBoost);
		}
	}

	uiPrintf("\nob = %d, db = %d", configSetup.ob, configSetup.db); 
	if(!(isSwan(swDeviceID) || isNala(swDeviceID)))	
		uiPrintf("b_ob = %d, b_db = %d",  configSetup.b_ob, configSetup.b_db);
	uiPrintf("\n");
	return;
}


A_BOOL setRxGain(A_UINT32 devNum)
{
	A_INT32 gain = configSetup.rxGain;

	if ((configSetup.overwriteRxGain) && (configSetup.rxGain != USE_REG_FILE)) {
		//do some validity checking on gain value - internal software check
		if((gain < configSetup.minRxGain) || (gain > configSetup.maxRxGain)) {
			uiPrintf("setRxGain: illegal rxGain value %d - gain not changed\n", gain);
			return(0);
		}

		//change the approriate Fields
		if ( ( (swDeviceID & 0xFF) == 0x14 ) || ( (swDeviceID & 0xFF) >= 0x16 ) ){
			if(configSetup.rxGain != rxGainValues_derby[gain].desiredGain) {
				uiPrintf("setRxGain: internal software program, gain of %d not found in lookup table\n", gain);
				return(0);
			}
			changeRxGainFields_derby(devNum, &rxGainValues_derby[gain]);
		} else {
			if(configSetup.rxGain != rxGainValues[gain].desiredGain) {
				uiPrintf("setRxGain: internal software program, gain of %d not found in lookup table\n", gain);
				return(0);
			}
			changeRxGainFields(devNum, &rxGainValues[gain]);
		}

	}
	configSetup.overwriteRxGain = 0;
	return(1);
}

void changeRxGainFields
(
 A_UINT32 devNum,
 RX_GAIN_REGS *pGainValues
) 
{
	art_changeField(devNum, "rf_rf_gain", pGainValues->rfGain);
	art_changeField(devNum, "rf_rf_atten0", pGainValues->rfAtten0);
	art_changeField(devNum, "rf_if_gain", pGainValues->ifGain);
	art_changeField(devNum, "rf_bb_gain1", pGainValues->bbGain1);
	art_changeField(devNum, "rf_bb_gain2", pGainValues->bbGain2);
	art_changeField(devNum, "rf_bb_gain3", pGainValues->bbGain3);
	art_changeField(devNum, "rf_pga_gain", pGainValues->pgaGain);

//	uiPrintf("SNOOP: changed rxgain vals to %d\n", pGainValues->desiredGain);

	return;
}

A_BOOL setRxGain_11bg(A_UINT32 devNum)
{
	A_INT32 gain = configSetup.rxGain;
    A_UINT32 gainTableSize, i;

	if ((configSetup.overwriteRxGain) && (configSetup.rxGain != USE_REG_FILE)) {
		//do some validity checking on gain value - internal software check
		if((gain < configSetup.minRxGain) || (gain > configSetup.maxRxGain)) {
			uiPrintf("setRxGain: illegal rxGain value %d - gain not changed\n", gain);
			return(0);
		}

		if (isGriffin(swDeviceID)) {
            gainTableSize = sizeof(rxGainValues_griffin) / sizeof(RX_GAIN_REGS_AR5112);

            for (i = 0; i < gainTableSize; i++) {
                //uiPrintf("i=%d:rxG=%d:dG=%d\n", i, configSetup.rxGain, rxGainValues_dragon[i].desiredGain);
                if(configSetup.rxGain == rxGainValues_griffin[i].desiredGain) {
                    gain = i;
                    break;
                }
            }
            if (i >= gainTableSize){
                uiPrintf("setRxGain: internal software program, gain of %d not found in lookup table\n", gain);
                return(0);
            }

            changeRxGainFields_singleChip(devNum, &rxGainValues_griffin[i]);
        } else if ( ( (swDeviceID & 0xFF) == 0x14 ) || ( (swDeviceID & 0xFF) >= 0x16 ) ){
			if(configSetup.rxGain != rxGainValues_derby_11bg[gain].desiredGain) {
				uiPrintf("setRxGain: internal software program, gain of %d not found in lookup table\n", gain);
				return(0);
			}
			changeRxGainFields_derby(devNum, &rxGainValues_derby_11bg[gain]);
        } else {
			if(configSetup.rxGain != rxGainValues_11bg[gain].desiredGain) {
				uiPrintf("setRxGain: internal software program, gain of %d not found in lookup table\n", gain);
				return(0);
			}
			changeRxGainFields_11bg(devNum, &rxGainValues_11bg[gain]);
		}

//       art_changeField(devNum, "beanie_switch", rxGainValues_11bg[gain].beanie_switch);
//       art_changeField(devNum, "bridge_switch", rxGainValues_11bg[gain].bridge_switch);

	}
	configSetup.overwriteRxGain = 0;
	return(1);
}

void changeRxGainFields_11bg
(
 A_UINT32 devNum,
 RX_GAIN_REGS_11bg *pGainValues_11bg
) 
{											
    art_changeField(devNum, "rf_b_lnagain", pGainValues_11bg->rf_b_lnagain);
    art_changeField(devNum, "rf_b_rfgain2", pGainValues_11bg->rf_b_rfgain2);
    art_changeField(devNum, "rf_b_rfgain1", pGainValues_11bg->rf_b_rfgain1);
    art_changeField(devNum, "rf_b_rfgain0", pGainValues_11bg->rf_b_rfgain0);
    art_changeField(devNum, "rf_rf_gain", pGainValues_11bg->rf_rf_gain);
    art_changeField(devNum, "rf_rf_atten0", pGainValues_11bg->rf_rf_atten0);
    art_changeField(devNum, "rf_if_gain", pGainValues_11bg->rf_if_gain);
    art_changeField(devNum, "rf_bb_gain1", pGainValues_11bg->rf_bb_gain1);
    art_changeField(devNum, "rf_bb_gain2", pGainValues_11bg->rf_bb_gain2);
    art_changeField(devNum, "rf_bb_gain3", pGainValues_11bg->rf_bb_gain3);
    art_changeField(devNum, "rf_b_ifgain1", pGainValues_11bg->rf_b_ifgain1);
    art_changeField(devNum, "rf_b_ifgain0", pGainValues_11bg->rf_b_ifgain0);
    art_changeField(devNum, "rf_pga_gain", pGainValues_11bg->rf_pga_gain);

	return;
}

void changeRxGainFields_derby
(
 A_UINT32 devNum,
 RX_GAIN_REGS_AR5112 *pGainValues
) 
{											
	art_changeField(devNum, "bb_rxtx_flag", pGainValues->rxtxFlag);
	art_changeField(devNum, "rf_LNAGain", pGainValues->lnaGain);
	art_changeField(devNum, "rf_RFGain", pGainValues->rfGain);
	art_changeField(devNum, "rf_IF_Gain", pGainValues->ifGain);
	art_changeField(devNum, "rf_BBgain", pGainValues->bbGainCoarse);
	art_changeField(devNum, "rf_BBgainf", pGainValues->bbGainFine);

	return;
}

void changeRxGainFields_singleChip
(
 A_UINT32 devNum,
 RX_GAIN_REGS_AR5112 *pGainValues
) 
{											
        art_changeField(devNum, "bb_gain_force", 1);
        art_changeField(devNum, "rf_LNAon",     pGainValues->rxtxFlag);
        art_changeField(devNum, "rf_lnagain",   pGainValues->lnaGain);
        art_changeField(devNum, "rf_rfvgagain", pGainValues->rfGain); 
        art_changeField(devNum, "rf_mixgain",   pGainValues->ifGain); 
        art_changeField(devNum, "rf_bbgain",    pGainValues->bbGainCoarse); 
        art_changeField(devNum, "rf_bbfgain",   pGainValues->bbGainFine); 
        uiPrintf("RX Gain Values: lnaon=%d:mixgain=%d:lnagain=%d:rfvgagain=%d:bbgain=%d:bbfgain=%d\n", 
            pGainValues->rxtxFlag, pGainValues->ifGain, pGainValues->lnaGain, pGainValues->rfGain, pGainValues->bbGainCoarse, pGainValues->bbGainFine); 

	return;
}

 #ifndef __ATH_DJGPPDOS__
void compute_EEPROM_Checksum(A_UINT32 devNum,A_UINT32 location) {
	A_UINT32 xor_data=0, addr, data;
	A_UINT32 i, length;
	//A_UINT32 eepromValue[1024];
	A_UINT32   *eepromValue;
	A_UINT32   eepromLower, eepromUpper,eepromLength;

	A_BOOL flag;

	length = location;
	flag = sizeWarning;

	//eepromValue = (A_UINT32 *) calloc(eepromSize , sizeof(A_UINT32));
	eepromValue = (A_UINT32 *) calloc(length , sizeof(A_UINT32));
	if(eepromValue == NULL)	{
		printf(" Memory is not allocated in compute_EEPROM_Checksum() \n");
		exit(1);
	}		

	for(i=0;i< length;i++)
		eepromValue[i] = 0xffff;

	if(length <= 0x400)	{
		eepromLength = 0x0000;
		length = 0x400;
	}

	art_resetDevice(devNum, rxStation, bssID, configSetup.channel, 0);	
//fjc	uiPrintf("\nOld Checksum=%04x", (A_UINT32)art_eepromRead(devNum, 0xc0));
	xor_data=0;

	addr = 0xC1;
	
	length = length - 0xc1;

	art_eepromReadBlock(devNum,addr,length,eepromValue);
	for (i=0;i<length;i++) {
	data = eepromValue[i];
	xor_data ^= data;
	}
	xor_data = ~xor_data & 0xffff;
	

	if(sizeWarning)	{

		uiPrintf(" Do u wnat override the checkSum value in 0xC0 locaiton (Y/N) \n");

		while(!kbhit())						;

		if(toupper(getch())=='Y') {

			sizeWarning =0;

		}

	}

	art_eepromWrite(devNum, 0xc0, xor_data);
//fjc uiPrintf("\nNew Checksum=%04x", xor_data);

	//write the 2 eeprom locations to specify the size
	length+=0xc1;

	//if(checkSumLength > 0x400) {
		if(length > 0x400) {
		//write the values to 0x1c and 0x1b
			eepromLower = (A_UINT16)(length & 0xffff);
			eepromUpper = (A_UINT16)((length & 0xffff0000) >> 11);

//			eepromLength = art_eepromRead(devNum,0x1c);
			eepromLength = (A_UINT16)set_eeprom_size(devNum,length);

			eepromLength = eepromLength & 0x1f;
			
					
			eepromUpper = eepromUpper | eepromLength ;  //fix the size to 32k for now, but need this to be variable
		art_eepromWrite(devNum, 0x1c, eepromUpper);
		art_eepromWrite(devNum, 0x1b, eepromLower);
			checkSumLength = length;
	}
	else {
			checkSumLength = length;
			art_eepromWrite(devNum, 0x1b, 0);
			art_eepromWrite(devNum, 0x1c, 0);
	}
	
	sizeWarning = flag;
	free(eepromValue);
}
#endif
A_BOOL EEPROM_Routine(A_UINT32 devNum)

{

	A_BOOL	exitLoop = FALSE;
	A_UINT32 addr, data;
	char filename[20];
	A_UINT32 i;
	A_UINT32 length;
	//A_UINT32 eepromValue[1024];
	A_UINT32 *eepromValue;
	A_UINT32 address = 0xffffffff;
	A_UINT32 value = 0xffffffff;
	A_UINT32 ear[MAX_EAR_LOCATIONS];
	char earcfg_filename[250] = "./ear.cfg";
	A_UINT32 condorMappingAddr;
	A_UINT32 mappedValue;

    	eepromValue = (A_UINT32 *) calloc(eepromSize , sizeof(A_UINT32));
	if(eepromValue == NULL)
	{
		printf(" Memory for %d bytes is not allocated in EEPROM_Routine() \n", checkSumLength);
		exit(1);
	}		

		for(i=0;i<eepromSize;i++)
			eepromValue[i]=0xffff;
		
	Display_EEPROM_menu();

	while(exitLoop == FALSE) {

#ifdef __ATH_DJGPPDOS__
		if (_bios_keybrd(_KEYBRD_READY)) {
#else
		if(kbhit()) {     
#endif
            switch(toupper(getch())) {
#ifndef __ATH_DJGPPDOS__
            case 'P':
				progBlankEEPROM(devNum);
				Display_EEPROM_menu();
				break;
#endif //__ATH_DJGPPDOS__
			case 'B':	//Backup
					if(sizeWarning)
						uiPrintf(" Only HALF THE EEPROM contents are taken back up\n");
					
				uiPrintf("\nPlease enter filename to backup: ");
				scanf("%s", &filename[0]);

				backupEeprom(devNum, filename);

				Display_EEPROM_menu();
	            break;
		    case 'R':	//Restore
					if(sizeWarning)
						uiPrintf(" Only HALF THE EEPROM contents are restored \n");
					
				uiPrintf("\nPlease enter filename to restore data from: ");
				scanf("%s", &filename[0]);

				restoreEeprom(devNum, filename);

				Display_EEPROM_menu();
				break;

			case 'E':
					if(sizeWarning)
						uiPrintf(" Only HALF THE EEPROM contents are erased \n");

				uiPrintf("\nYou are about to erase the EEPROM content! Do you want to proceed (Y/N)?");
				while(!kbhit())
					;
				if(toupper(getch())=='Y') {
					uiPrintf("\nErasing EEPROM ...");
					art_resetDevice(devNum, rxStation, bssID, configSetup.channel, 0);	
						for (i=0;i<eepromSize;i++) 
					{
						eepromValue[i] = 0xffff;
					}

					addr = 0;
					//length = 0x400;
						length = eepromSize;// Changed to reflect new memory size Giri
					art_eepromWriteBlock(devNum,addr,length,eepromValue);
					uiPrintf("\nEEPROM is erased.");
				}
				else
					uiPrintf("\nAbort");
				Display_EEPROM_menu();
				break;
			
			case 'W':
				if(sizeWarning)
					uiPrintf(" Enter the location Less than %d \n",eepromSize);
					
				uiPrintf("Enter EEPROM address for location: ");
				scanf("%x", &address);

				//read the value of the eeprom first
				value = art_eepromRead(devNum, address);
				if(!isCondor(swDeviceID) || (isCondor(swDeviceID) && (address > 2)))  {
					uiPrintf("Current value of location is 0x%x. Enter new value to write: ", value);
					scanf("%x", &value);

					//if(address > 0x400) 
						if(address > (eepromSize)) 
					{
						uiPrintf("Illegal eeprom address\n");
						Display_EEPROM_menu();
						break;
					}

					if (value > 0xffff) {
						uiPrintf("Value too large.  Must be 16 bit or less\n");
						Display_EEPROM_menu();
						break;
					}

				}
				//write the eeprom value
				art_resetDevice(devNum, rxStation, bssID, configSetup.channel, 0);	
				if(!isCondor(swDeviceID)) {
					art_eepromWrite(devNum, address, value);
				}
				else {
					//need to check for this being eeprom locatoin 0, 1, or 2.  Don't
					//want to write to these locations for condor, will override the
					//pci config pointer information
					if (address > 2) {
						art_eepromWrite(devNum, address, value);						
					}
					else {
						printf("Writing to eeprom locations 0, 1 and 2 is not allowed for this device\n");
						printf("Write to mapped location for value change to take affect for pci config loading\n\n");
					}
					
					//also look for the alternate mapping
					if(getPCIConfigMapping(devNum, address, &condorMappingAddr))
					{						
						mappedValue = art_eepromRead(devNum, condorMappingAddr);
						uiPrintf("Also mapped to address 0x%x for pci config loading, contents = 0x%x\n",
							condorMappingAddr, mappedValue);
						uiPrintf("Write to this location also (y/n)?\n");
						while(!kbhit())
							;
						if(toupper(getch())=='Y') {
							if(address <= 2) {
								uiPrintf("Enter new value to write ");
								scanf("%x", &value);

								//if(address > 0x400) 
									if(address > (eepromSize)) 
								{
									uiPrintf("Illegal eeprom address\n");
									Display_EEPROM_menu();
									break;
								}

								if (value > 0xffff) {
									uiPrintf("Value too large.  Must be 16 bit or less\n");
									Display_EEPROM_menu();
									break;
								}
							}
							art_eepromWrite(devNum, condorMappingAddr, value);	
						}
						else {
							printf("NOT writing to mapped location\n");
						}
					}
				}


				if(address < 0xc0) {
					//we are done writing
					Display_EEPROM_menu();
					break;
				}
				
				//otherwise, fall through and calculate the new checksum
				uiPrintf("Regenerating checksum...\n");

 #ifndef __ATH_DJGPPDOS__
			case 'S':					
				compute_EEPROM_Checksum(devNum, checkSumLength);
					
				if(!eeprom_verify_checksum(devNum)){
					uiPrintf("Checksum Failed\n");					
				}
				Display_EEPROM_menu();
				break;
#endif
			case 'G':
					
				if(sizeWarning)
					uiPrintf(" Enter the location Less than %d \n",eepromSize);

				uiPrintf("Enter EEPROM address for location: ");
				scanf("%x", &address);

				if(address > (eepromSize)) 	{
					uiPrintf("Illegal eeprom address\n");
					Display_EEPROM_menu();
					break;
				}

				//read the value of the eeprom 
				value = art_eepromRead(devNum, address);
				uiPrintf("Current value of location is 0x%x. \n", value);
				//if this is condor, also look for the alternate mapping
				if(isCondor(swDeviceID)) {
					if(getPCIConfigMapping(devNum, address, &condorMappingAddr))
					{						
						mappedValue = art_eepromRead(devNum, condorMappingAddr);
						uiPrintf("Also mapped to address 0x%x for pci config loading, contents = 0x%x\n",
							condorMappingAddr, mappedValue);
					}
				}
				Display_EEPROM_menu();
				break;
				
            case 'D':	//Display
			    // reset device in order to read eeprom content
					if(sizeWarning)
						uiPrintf(" Only the  %d Location are shown \n",eepromSize);
				art_resetDevice(devNum, rxStation, bssID, configSetup.channel, 0);	
				addr = 0;
				//length = 0x400;
					length = eepromSize; // Changed to reflect new memory size giri
					
				art_eepromReadBlock(devNum,addr,length,eepromValue);
				for (i=0;i<length;i++) {
						data = eepromValue[i];
						uiPrintf("\n%04x    ;%04x", addr+i,data);
				        }
			
				Display_EEPROM_menu();
	            break;

			case 'C':
				if (swDeviceID != 0x1107) {
					//Fix this so that don't need to have eeprom load flag set to read this.
					//Will need to think about this again when add programming
					art_setResetParams(devNum, configSetup.pCfgFile, 1,
						(A_BOOL)configSetup.eepromHeaderLoad, (A_BOOL)configSetup.mode, configSetup.use_init);		
					art_rereadProm(devNum);
					art_resetDevice(devNum, rxStation, bssID, configSetup.channel, 0);
					if (!art_checkProm(devNum, 1)) {			
						//ie prom is valid
						printEepromStruct_16K(devNum, configSetup.mode);
					}
					Display_EEPROM_menu();
				}
				break;
#ifndef __ATH_DJGPPDOS__
    	    case 'L': // Write EAR contents into EEPROM
				{
					if(sizeWarning)
						uiPrintf(" Enter the EAR file with less than %d Location \n",eepromSize);

					uiPrintf(" EEPROM_SIZE = %x  checkSumLength = %x\n",eepromSize,checkSumLength);
					uiPrintf("\nPlease enter ear config filename : ");
					scanf("%s", earcfg_filename);
					writeEarToEeprom(devNum, earcfg_filename);
					Display_EEPROM_menu();
				}
				break;
	        case 'A': // Display EAR contents in EEPROM
				{
					A_UINT16 earLocation;
					A_UINT16 eepromVersion;

					eepromVersion = (A_UINT16)art_eepromRead(devNum, 0xc1);
					earLocation = (A_UINT16)art_eepromRead(devNum, 0xc4);
					if ((eepromVersion >= 0x4000) && (earLocation != 0)) {
						uiPrintf("EEPROM Version = %x\n", (eepromVersion ));
						uiPrintf("EEPROM Major Version = %x\n", (eepromVersion >> 12));
						uiPrintf("EEPROM Minor Version = %x\n", (eepromVersion & 0xfff ));
						uiPrintf("EAR location at 0xc4 = %x\n", earLocation);
						earLocation &= 0xfff;
						uiPrintf("EAR will be read from location %x::", earLocation);
						art_eepromReadBlock(devNum, earLocation, MAX_EAR_LOCATIONS, ear);  
					   }
					else {
						uiPrintf("EAR not supported \n");
						uiPrintf("EEPROM Major Version = %x\n", (eepromVersion >> 12));
						uiPrintf("EEPROM Minor Version = %x\n", ((eepromVersion << 4) >> 12));
						uiPrintf("EAR location at 0xc4 = %x\n", earLocation);
					}

					displayEar(ear, MAX_EAR_LOCATIONS);
					Display_EEPROM_menu();
				}
				break;
			case 'O':
				eraseBlock(devNum);
				Display_EEPROM_menu();
				break;
#endif
 #ifndef __ATH_DJGPPDOS__		
			case 'U':
				uiPrintf("\nPlease enter bootrom filename: ");
				scanf("%s", &filename[0]);
				updateBootRom(devNum, filename);
				Display_EEPROM_menu();
				break;
#endif
            case 0x1b:
            case 'Q':
                exitLoop = TRUE;
					return TRUE;
            default:
                break;
            }
        }
	}

	free(eepromValue);
		return TRUE;

}

 #ifndef __ATH_DJGPPDOS__

A_BOOL updateEepromBootData(A_UINT32 devNum, A_UINT32 *data, A_UINT32 size) {
		printf("WARNING::Update EEPROM Boot data not supported\n");
  return 1;
}

void wait_status_clear(A_UINT32 devNum) {
   A_UINT32 val;

 	val = 1;
	while(val & 1) {
	  art_ap_reg_write(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS + 0x4, 0x05);
	  art_ap_reg_write(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS , 0x111);
	  while(art_ap_reg_read(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS) & 0x10000) {
	  }
	  val = art_ap_reg_read(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS + 0x8);
	}

}

void erase_sector(A_UINT32 devNum, A_INT8 sectorNum, A_UINT32 sectorSize) {
   art_ap_reg_write(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS + 0x4, 0x6);
   art_ap_reg_write(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS, 0x101);
   wait_status_clear(devNum);
   art_ap_reg_write(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS +  0x4, 0xd8+(sectorSize*sectorNum));
   art_ap_reg_write(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS, 0x104);
   wait_status_clear(devNum);
}

A_BOOL updateFlashBootData(A_UINT32 devNum, A_UINT32 *data, A_UINT32 size) {
  A_UINT32 fl_addr, iIndex;
  A_UINT32 sector_size, rd_data;

  sector_size = 32 * 1024;
  erase_sector(devNum, 0, sector_size);
  erase_sector(devNum, 1, sector_size);
  
  fl_addr = 0x0;
  for(iIndex=0; iIndex<size; iIndex+=4) {
  	art_ap_reg_write(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS + 0x4, 0x6);
  	art_ap_reg_write(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS + 0x0, 0x101);
  	wait_status_clear(devNum);
  	art_ap_reg_write(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS + 0x4, (iIndex << 8) | 0x02);
  	art_ap_reg_write(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS + 0x8, data[iIndex]);
  	art_ap_reg_write(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS + 0x0, 0x108);
  	wait_status_clear(devNum);
  	printf(".");
    fl_addr += 0x4;
  }
  printf("Verifying EEPROM for %d bytes\n", (size*4));
  for(iIndex=0; iIndex<size; iIndex++) {
	  rd_data = art_ap_reg_read(devNum, AR5523_FLASH_ADDRESS+(iIndex*4));
	  printf(".");
	  if (data[iIndex] != rd_data) {
		  printf("ERROR::Mismatch in data at %x:Read=%x:Expected=%x\n", (AR5523_FLASH_ADDRESS+iIndex), rd_data, data[iIndex]);
		  return 0;
	  }
  }
  return 1;
}

A_BOOL updateBootRom(A_UINT32 devNum, A_CHAR *filename) {

    A_INT8 flash_if;
	A_UINT8 bin_ch0, bin_ch1, bin_ch2, bin_ch3;
	FILE *fp;
	A_BOOL status=0;
	A_UINT32 data[MAX_BOOT_DATA_WORDS];
//	A_UINT32 rd_data[MAX_BOOT_DATA_WORDS];
	A_UINT32 size = MAX_BOOT_DATA_WORDS, lAddr;
	A_UINT32 iIndex;
    A_UINT16 devIndex;
    MDK_WLAN_DEV_INFO    *pdevInfo;

	devIndex = (A_UINT16)dev2drv(devNum);
	pdevInfo = globDrvInfo.pDevInfoArray[devIndex];

	fp = (FILE *) fopen(filename, "rb");
	if (fp == NULL) {
		printf("ERROR::Unable to open bootrom file %s\n", filename);
		return 0;
	}
	iIndex=0;
	uiPrintf("SNOOP::Reading data from file %s:\n", filename);
	while(!feof(fp)) {
		bin_ch0 = (A_UINT8)getc(fp);
		bin_ch1 = (A_UINT8)getc(fp);
		bin_ch2 = (A_UINT8)getc(fp);
		bin_ch3 = (A_UINT8)getc(fp);
		data[iIndex++] = (bin_ch0 << 24) | (bin_ch1<<16) | (bin_ch2<<8) | bin_ch3;

        if (iIndex >= MAX_BOOT_DATA_WORDS) {
          printf("File Size greater than Max code size (%d bytes) \n", MAX_CODE_SIZE);
		  fclose(fp);
 	      return -1;
 	    }

	}

	size = iIndex;

	fclose(fp);
	lAddr = art_memAlloc(size*4, 0, devNum) ;
    printf("Updating boot image from file %s\n", filename);
	art_load_and_program_code(devNum, lAddr, size*4, (A_UCHAR *)data, 0);
	art_memFree(lAddr, devNum);

	return status;

    printf("Programming Boot Data for %d bytes\n", (size*4));
/*  //  for(iIndex=0;iIndex<size;iIndex++) {
//		art_eepromWrite(devNum,(iIndex*4)|0x10000000,data[iIndex]);
		art_eepromWriteBlock(devNum,0x10000000, size, data);
//	    printf(".");
//	}
 
    printf("Verifying Boot Data for %d bytes\n", (size*4));
	art_eepromReadBlock(devNum, 0x10000000, size,rd_data);
    for(iIndex=0; iIndex<size; iIndex++) {
	  //rd_data = art_ap_reg_read(devNum, AR5523_FLASH_ADDRESS+(iIndex*4));
	  //printf(".");
	  if (data[iIndex] != rd_data[iIndex]) {
		  printf("ERROR::Mismatch in data at %x:Read=%x:Expected=%x\n", (AR5523_FLASH_ADDRESS+iIndex), rd_data[iIndex], data[iIndex]);
		  return 0;
	  }
    }

    return 0;
*/

    flash_if = (A_INT8)(art_ap_reg_read(devNum, AR5523_FLASH_CONTROL_BASE_ADDRESS + 0x1c) & 0x1);
	if (flash_if == I2C) {
		status = updateEepromBootData(devNum, data, size);
	}
	else {
		status = updateFlashBootData(devNum, data, size);	
	}

   return status;
}
#endif

A_BOOL backupEeprom(A_UINT32 devNum, A_CHAR *filename)
{
	A_UINT32 addr, data;
	A_UINT32 length;
	//A_UINT32 eepromValue[1024];	
	A_UINT32 *eepromValue;
	FILE *fStream;
	A_UINT32 i;

	eepromValue = (A_UINT32 *) calloc(eepromSize , sizeof(A_UINT32));
	if(eepromValue == NULL)
	{
		printf(" Memory is not allocated backupEeprom() \n");
		exit(1);
	}

	if( (fStream = fopen( filename, "r")) != NULL ) {
		fclose(fStream);
		uiPrintf("\nFile '%s' already exist! Overwrite (Y/N)?", filename);
		while (!kbhit())
			;
		if(toupper(getch())!='Y') {
			uiPrintf("\nBackup aborted!\n");
			return FALSE;
		}
	}

	if( (fStream = fopen( filename, "w")) == NULL ) {
		uiPrintf("Failed to open %s\n", filename);
		return FALSE;
	}
	// reset device in order to read eeprom content
	art_resetDevice(devNum, rxStation, bssID, configSetup.channel, 0);	
	addr = 0;
	//length = 0x400;
	length = eepromSize;// Changed to reflect new memory size Giri
	art_eepromReadBlock(devNum,addr,length,eepromValue);
	for (i=0;i<length;i++) {
			data = eepromValue[i];
		fprintf(fStream, "%04x    ;%04x\n", data, i);
	}

	uiPrintf("\nBackup EEPROM content to file '%s' is completed !\n", filename);
	fclose(fStream);
	free(eepromValue);
	return(TRUE);
}

A_BOOL restoreEeprom(A_UINT32 devNum, A_CHAR *filename)
{
	A_UINT32 addr, data;
	A_UINT32 length;
	//A_UINT32 eepromValue[1024];
	A_UINT32 *eepromValue;
	FILE *fStream;
    char lineBuf[82], *pLine;
	A_UINT32 prevAddr,startAddr,firstAddr;

	eepromValue = (A_UINT32 *) calloc(eepromSize , sizeof(A_UINT32));
	if(eepromValue == NULL)
	{
		printf(" Memory is not allocated for %d words in restoreEeprom()\n", checkSumLength);
		exit(1);
	}

	if( (fStream = fopen( filename, "r")) == NULL ) {
		uiPrintf("Failed to open file '%s'\n", filename);
		return FALSE;
	}
	// reset device in order to read eeprom content
	art_resetDevice(devNum, rxStation, bssID, configSetup.channel, 0);	

	length = 0;
	prevAddr = 0;
	startAddr = 0;
	firstAddr = 1;
	while(fgets(lineBuf, 80, fStream) != NULL) {
		pLine = lineBuf;
		if(strchr(pLine, ';')==NULL)
			continue;
		sscanf(pLine, "%04x", &data);
		while(*pLine++ !=';')
			;
		sscanf(pLine, "%04x", &addr);
		uiPrintf("\naddr=%04x, data=%04x", addr, data); 
							// Collect a block of contigous data and write 
		if (firstAddr) {
			startAddr = addr;
			firstAddr = 0;
		}
	
		if (((addr != (prevAddr+1)) && length) || (length == EEPROM_BLOCK_SIZE)) {
				art_eepromWriteBlock(devNum,startAddr,length,eepromValue);
				length = 0;
				startAddr = addr;
	}
		eepromValue[length] = data;
		prevAddr = addr;
		length++;
	}
	if (length) {
		art_eepromWriteBlock(devNum,startAddr,length,eepromValue);
	}

	uiPrintf("\nRestore EEPROM content from file '%s' is completed !\n", filename);
	fclose(fStream);
	free(eepromValue);
	return TRUE;
}

 #ifndef __ATH_DJGPPDOS__
A_BOOL
writeEarToEeprom
(
 A_UINT32 devNum,
 char *pEarFile
)
{
	A_UINT32 numValues,startAddr,ii,length;
	A_UINT16 earLocation;
	A_UINT16 eepromVersion;
	A_UINT32 ear[MAX_EAR_LOCATIONS];
	A_UINT32 earFileVersion,numWords;
	A_INT32 earFileIdentifier;
	A_UINT16 tempEepromValue;
	A_BOOL   atCal = FALSE;
	A_UINT32 *word;
	A_UINT32 eepromLoadPrev = configSetup.eepromLoad;

	word = ( A_UINT32 *)calloc(eepromSize,sizeof(A_UINT32));
	if(word == NULL)
	{
		printf(" Memory not allocated in writeEarToEeprom() \n");
		return FALSE;
	}

//	for(ii=0;ii<eepromSize;ii++)
//		word[ii]=0xffff;
	
	
	//before we write to the eeprom, check to see if doing dynamic ear,
	//then need to try to load the eeprom
	if (isGriffin(swDeviceID) && CalSetup.enableDynamicEAR) 
	{
		//first check to see if the eeprom is loaded, if not need to try to force
		//loading so can do dynamic ear
		if(!configSetup.eepromLoad) {
			if (eeprom_verify_checksum(devNum))
			{
				configSetup.validCalData  =  TRUE;
				configSetup.eepromLoad    =  1;

				art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
					(A_BOOL)1, (A_UCHAR)configSetup.mode, configSetup.use_init);	
				art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
			} else {
				uiPrintf("Error: Unable to load EEPROM, cannot write dynamic EAR\n");
				free(word);
				configSetup.eepromLoad = eepromLoadPrev;
				art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
					(A_BOOL)1, (A_UCHAR)configSetup.mode, configSetup.use_init);	
				rereadProm(devNum);
				art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
				return FALSE;
			}
		}
	}

	if (parseLoadEar(pEarFile, ear, &numValues, 0) == -1) 
	{
		uiPrintf("Unable to parse ear file\n");
		return FALSE;
	}

	//get the ear file identifier and ear file verison so we can write these also
	earFileVersion = getEarPerforceVersion(pEarFile);
	if(earFileVersion == 0) 
	{
		uiPrintf("Error: unable to get perforce version ID from ear file\n");
		return FALSE;
	}
	earFileIdentifier = getEarFileIdentifier(pEarFile);
	if(earFileIdentifier == -1) 
	{
		uiPrintf("Unable to get file identifier from ear file\n");
		return FALSE;
	}

	eepromVersion = (A_UINT16)art_eepromRead(devNum, 0xc1);
	earLocation = (A_UINT16)art_eepromRead(devNum, 0xc4);

	if ((eepromVersion >= 0x4000) && (earLocation != 0)) 
	{
//fjc		uiPrintf("EEPROM Version = %x\n", (eepromVersion ));
//fjc		uiPrintf("EEPROM Major Version = %x\n", (eepromVersion >> 12));
//fjc		uiPrintf("EEPROM Minor Version = %x\n", (eepromVersion & 0xfff ));
//fjc		uiPrintf("EAR location at 0xc4 = %x\n", earLocation);
		earLocation &= 0xfff;

		if(numValues + earLocation  > eepromSize)	{
			uiPrintf("Error: cannot write ear as the  number of locations exceed eepromSize\n");
			free(word);
			return FALSE;
		}
//fjc		uiPrintf("EAR will be written at  location %x::", earLocation);
//fjc		uiPrintf("Number of values = %d\n", numValues);
		art_eepromWriteBlock(devNum, earLocation, numValues, ear);  

		//write the identifier and version also
		tempEepromValue = (A_UINT16)art_eepromRead(devNum, 0xc6);
		tempEepromValue = (A_UINT16)((tempEepromValue & 0xff00) | (earFileVersion & 0xff));
		art_eepromWrite(devNum, 0xc6, tempEepromValue);
		tempEepromValue = (A_UINT16)art_eepromRead(devNum, 0xc7);
		tempEepromValue = (tempEepromValue & 0xff00) | (earFileIdentifier & 0xff);
		art_eepromWrite(devNum, 0xc7, tempEepromValue);
			
		if(configSetup.debugInfo) 
		{
			uiPrintf("Writing EarFileVersion = %d and earFileIdentifier = %d\n",
				earFileVersion, earFileIdentifier);
		}
		CalSetup.EARLen = numValues;
		CalSetup.EARStartAddr = earLocation;
		startAddr = earLocation + numValues;			
		// Program Dynamic EAR for Griffin if enabled
		if (isGriffin(swDeviceID) && CalSetup.enableDynamicEAR) 
		{			
			atCal = FALSE;	
			if(configSetup.debugInfo) 
			{
				uiPrintf("SNOOP: dynaEarStart = 0x%x\n", startAddr);
			}
			startAddr -= 1;
			for (ii = 0; ii < CalSetup.numDynamicEARChannels; ii++ ) 
			{
				numWords = art_getEARCalAtChannel(devNum, atCal, (A_UINT16)(CalSetup.dynamicEARChannels[ii]), &(word[0]), (A_UINT16)(CalSetup.cal_mult_xpd_gain_mask[MODE_11g]), CalSetup.dynamicEARVersion);

				if(startAddr + numWords+1  > eepromSize)	{
					uiPrintf(" Error: cannot write dynamic ear as the  number of locations exceed eepromSize\n" );
					free(word);
					return FALSE;
				}
				art_eepromWriteBlock(devNum, startAddr, numWords, word) ;
				startAddr += numWords;
				CalSetup.EARLen += numWords;
				length = startAddr;
				
				if(configSetup.debugInfo) 
				{
					uiPrintf("SNOOP: num_dynaEar_words  = %d \n", numWords);
				}
			}
		}
		// write a 0x0000 to mark the end of EAR
		art_eepromWrite(devNum, startAddr, 0x0000);
		length+=1;			
		if ((needsUartPciCfg(swDeviceID)) && (CalSetup.uartEnable)) 
		{
			startAddr = CalSetup.EARStartAddr + CalSetup.EARLen + 2;
			length+=2;
			CalSetup.UartPciCfgLen = programUartPciCfgFile(devNum, UART_PCI_CFG_FILE, startAddr ,FALSE);
			length+=CalSetup.UartPciCfgLen;	
		}				
		//compute_EEPROM_Checksum(devNum, earLocation + numValues);
		compute_EEPROM_Checksum(devNum, length);
	}

	else 
	{
		uiPrintf("EAR not supported \n");
		uiPrintf("EEPROM Major Version = %x\n", (eepromVersion >> 12));
		uiPrintf("EEPROM Minor Version = %x\n", ((eepromVersion << 4) >> 12));
		uiPrintf("EAR location at 0xc4 = %x\n", earLocation);
		free(word);
		configSetup.eepromLoad = eepromLoadPrev;						
		art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
			(A_BOOL)1, (A_UCHAR)configSetup.mode, configSetup.use_init);	
		art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
		return FALSE;
	}
	free(word);
	configSetup.eepromLoad = eepromLoadPrev;
	art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
		(A_BOOL)1, (A_UCHAR)configSetup.mode, configSetup.use_init);	
	art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
	return TRUE;
}

void eraseBlock(A_UINT32 devNum) {
	A_UINT32 startAddress;
	A_UINT32 endAddress;
	A_UINT32 address;
	
	uiPrintf("Enter EEPROM start address: ");
	scanf("%x", &startAddress);

	if(startAddress > eepromSize) 
	{
		uiPrintf("eeprom start address is larger than eeprom size %x\n", eepromSize);
		return;
	}
			
	uiPrintf("Enter EEPROM end address (erase will be inclusive of this address): ");
	scanf("%x", &endAddress);

	if(endAddress > eepromSize) 
	{
		uiPrintf("eeprom end address is larger than eeprom size %x\n", eepromSize);
		return;
	}

	uiPrintf("Erase eeprom locations 0x%04x through 0x%04x\n", startAddress, endAddress);
	uiPrintf("Press any key to continue or <ESC> to cancel\n");
	
	if(getch() == 0x1b) {
		return;
	}
	
	art_resetDevice(devNum, rxStation, bssID, configSetup.channel, 0);	
	for (address = startAddress; address <= endAddress; address++) {
		art_eepromWrite(devNum, address, 0xffff);
	}
	
	//regenerate the checksum
	compute_EEPROM_Checksum(devNum, checkSumLength);
		
	if(!eeprom_verify_checksum(devNum)){
		uiPrintf("Checksum Failed\n");					
	}
}
#endif
#ifndef __ATH_DJGPPDOS__
void progBlankEEPROM(A_UINT32 devNum)

{

	A_UINT32 i,k;


	uiPrintf("Adapter will be programmed with subsystem ID %x\n", configSetup.cfgTable.pCurrentElement->subsystemID);
	uiPrintf("Press any key to continue or <ESC> to cancel\n");
	
	if(getch() == 0x1b) {
		return;
	}
	EEPROM_DATA_BUFFER =(A_UINT32 **) malloc(sizeof(A_UINT32 *) * NUMEEPBLK);

	if(EEPROM_DATA_BUFFER != NULL)	{
		for (i = 0; i < NUMEEPBLK; i++) {
			EEPROM_DATA_BUFFER[i]= (A_UINT32 *)malloc(sizeof(A_UINT32) * eepromSize);
			if(EEPROM_DATA_BUFFER[i] == NULL){
				printf(" Memory Not allocated in progBlankEeprom() \n");
				exit(1);
			}
		}
	}

	else {
		printf(" Memory Not allocated in progBlankEeprom() \n");
		exit(1);
	}

	for(i=0;i<NUMEEPBLK;i++)	{
		for(k =0;k < eepromSize;k++)	{
			EEPROM_DATA_BUFFER[i][k]=0xffff;			
		}
	}

	art_resetDevice(devNum, rxStation, bssID, configSetup.channel, 0);	
	art_changeField(devNum, "mc_eeprom_size_ovr", 3);
	art_resetDevice(devNum, rxStation, bssID, configSetup.channel, 0);	

	parseSetup(devNum);
	Write_eeprom_Common_Data(devNum, 0, 1);

	for( i =0; i < NUMEEPBLK; i++){
		free(EEPROM_DATA_BUFFER[i]);
	}	

	free(EEPROM_DATA_BUFFER);

	uiPrintf("\nEEPROM programmed, but not calibrated.");
	return;
}
#endif //__ATH_DJGPPDOS__


void Display_EEPROM_menu(void)
{
	printf("\n");
    printf("=======================================================\n");
	printf("| EEPROM                                              |\n");
#ifndef __ATH_DJGPPDOS__
    printf("|   P -  Blank EEPROM (P)rogramming mode              |\n");
#endif //__ATH_DJGPPDOS__
    printf("|   B - (B)ack up EEPROM content to file              |\n");
	printf("|   R - (R)estore EEPROM content from file            |\n");
	printf("|   E - (E)rase EEPROM content                        |\n");
	printf("|   S - Re-calculate check(S)um for calibration date  |\n");
	if (swDeviceID != 0x1107) {
		printf("|   C - Display (C)alibration data                    |\n");
	}
	printf("|   D - (D)isplay EEPROM content on the screen        |\n");
	printf("|   W - (W)rite single EEPROM location                |\n");
	printf("|   G - (G)et (read) single EEPROM location           |\n");
	printf("|   L - (L)oad EAR into EEPROM                        |\n");
	printf("|   A - Display E(A)R contents in EEPROM              |\n");
	printf("|   O - Erase EEPROM Bl(o)ck                          |\n");
	if (usb_client) {
	    printf("|   U - Update bootrom contents of USB device         |\n");
	}

	printf("| ESC - exit                                          |\n");
	printf("=======================================================\n");
}


#if 0
A_UINT16 promptForCardType(A_UINT16 *pSubSystemID) {
	A_UINT16	cardType;
	A_BOOL		getSelection = TRUE;
	
	while (getSelection) {
		uiPrintf("Select card type to program\n");
		uiPrintf("  1. CB21\n");
		uiPrintf("  2. MB22\n");
		uiPrintf("  3. MB23\n");
		uiPrintf("  4. CB22\n");

		while(!kbhit());

		switch(getch()) {
		case '1':
			cardType = devID_CB21;
			*pSubSystemID = ATHEROS_CB21;
			getSelection = FALSE;
			break;
		case '2':
			cardType = devID_MB22;
			*pSubSystemID = ATHEROS_MB22;
			getSelection = FALSE;
			break;

		case '3':
			cardType = devID_MB23;
			*pSubSystemID = ATHEROS_MB23;
			getSelection = FALSE;
			break;

		case '4':
			cardType = devID_CB22;
			*pSubSystemID = ATHEROS_CB22;
			getSelection = FALSE;
			break;
		default:
			uiPrintf("Illegal selection\n");
			break;
		}
	}
	return cardType;
}
#endif

void setInitialPowerControlMode(A_UINT32 devNum, A_UINT32 rateIndex)
{
/*  SNOOP
//++JC++
	if (isGriffin(swDeviceID)) {
		configSetup.powerOvr = 0;
		configSetup.powerControlMethod = PCDAC_PWR_CTL;
		return;
	}
//++JC++
*/
	if (isGriffin(swDeviceID)  || isEagle(swDeviceID) ) {
		configSetup.powerOvr = 0;
	}

	if (configSetup.powerControlMethod == NO_PWR_CTL) 
	{
		if (configSetup.validCalData && configSetup.eepromLoad)
		{			
			configSetup.powerOutput = art_getMaxPowerForRate(devNum, (A_UINT16)configSetup.channel, DataRate[rateIndex]);
			configSetup.powerControlMethod = DB_PWR_CTL;
			configSetup.useTargetPower = TRUE;
		} else
		{
			configSetup.pcDac = INITIAL_PCDAC; 
			configSetup.powerControlMethod = PCDAC_PWR_CTL;
		}
	}		
}

void invalidEepromMessage (A_UINT32 airtime)
{
		uiPrintf("\n\n\n**********************************\n");
		uiPrintf(      "invalid calibration data in EEPROM \n");
		uiPrintf(	   "**********************************\n\n\n\n");
		Sleep(airtime);
}

void printContMenu(A_UINT32 devNum)
{

	A_UINT32 rateIndex = 0;
	A_UINT32 contMode;
	A_UINT32 tempRate;

	clearScreen(); 
	printf("\n");
	printf("=================================================================\n");
	printf("| Continuous Transmit Options                                   |\n");
	if (configSetup.mode == MODE_11A) {
		printf("|   p - Increase Center Frequency by 10 MHz (P inc by 100 MHz)  |\n");
		printf("|   l - Decrease Center Frequency by 10 MHz (L dec by 100 MHz)  |\n");
	}
	else {
		printf("|   p - Increase Center Frequency by 5 MHz                      |\n");
		printf("|   l - Decrease Center Frequency by 5 MHz                      |\n");
	}
	printf("|   o - Increase Data Rate                                      |\n");
	printf("|   k - Decrease Data Rate                                      |\n");
	printf("|   i - Increase pcdac (I inc by 10)                            |\n");
	printf("|   j - Decrease pcdac (J dec by 10)                            |\n");
	printf("|   f - Increase power output by 0.5dBm (F inc by 5dBm)         |\n");
	printf("|   c - Decrease power output by 0.5dBm (C dec by 5dBm)         |\n");
	printf("|   u - Increase ob by 1 (w - increase b-ob)                    |\n");
	printf("|   h - Increase db by 1 (q - increase b-db)                    |\n");
	if (!isGriffin(swDeviceID) && !isEagle(swDeviceID) ) {
    	printf("|   v - Toggle power override (ovr)                             |\n");
	
	    if (!configSetup.powerOvr) {
		    printf("|   x - Toggle external power                                   |\n");
	    }
	    else {
		    printf("|   y - Increase gainI by 1 (Y inc by 10)                       |\n");
		    printf("|   g - decrease gainI by 1 (G dec by 10)                       |\n");
		    if(((swDeviceID & 0xff) <= 0x13) || ((swDeviceID & 0xff) == 0x15)) { 
			    printf("|   m - Increase rf Gain Boost                                  |\n");
		    }
		    else {
			    printf("|   m - Increase rfgain_I                                       |\n");
		    }
	    }
	    if (!configSetup.powerOvr) {
		    printf("|   n - Step xpd gain by 6dB                                    |\n");
	    }
	}
	printf("|   s - Toggle output mode (tx100 | tx99 | single carrier)      |\n");
	if(configSetup.mode != MODE_11B) {
		printf("|   b - Toggle turbo mode                                       |\n");
	}
	printf("|   a - Toggle antenna                                          |\n");
	printf("|   d - Toggle Data Pattern                                     |\n");
	if(bbRev >= 0x30) {
		printf("|   z - Toggle Scramble mode                                    |\n");
	}
	if (configSetup.contMode == CONT_FRAME) {
		printf("|   t - Increase interframe spacing slots of 1 (T inc by 20)    |\n");
		printf("|   r - Decrease interframe spacing slots of 1 (R dec by 20)    |\n");
	}
	if (!isGriffin(swDeviceID) && !isEagle(swDeviceID) ) {
    	printf("|   9 - Toggle dynamic optimization                             |\n");
	   	printf("|   7 - Toggle gainF readback screen refresh                    |\n");
	    if(!pCurrGainLadder->active) {
		    printf("|   4 - Increment Fixed gain                                    |\n");
	    }
	}
#if !defined(CUSTOMER_REL) && !defined(__ATH_DJGPPDOS__)
	printf("|   e - Extract spectral mask                                   |\n");
#endif		
	printf("| ESC - exit                                                    |\n");
	printf("=================================================================\n\n");

	printMode();
	uiPrintf("\n");

	printConfigSettings(devNum);

	if(configSetup.antenna == (USE_DESC_ANT | DESC_ANT_A))
		uiPrintf("ANT_A", (unsigned int)configSetup.channel);
	else
		uiPrintf("ANT_B", (unsigned int)configSetup.channel);

	
	if (configSetup.contMode == CONT_TX100) {
		contMode = CONT_DATA;
		uiPrintf(", [TX100]");
	}
	else if (configSetup.contMode == CONT_TX99) {
		contMode = CONT_FRAMED_DATA;
		uiPrintf(", [TX99]");
	}
	else if (configSetup.contMode == CONT_FRAME) {
		contMode = CONT_FRAMED_DATA;
		uiPrintf(", [FRAME]");
	}

	if(configSetup.scrambleModeOff) {
		uiPrintf(", scramble off");
	}

//
	switch(configSetup.contMode) {
		case CONT_TX100:	
			if((macRev == 0x40)&&(!(configSetup.mode == MODE_11B))) {
				configSetup.dataRateIndex = 0;
			}
		case CONT_TX99:
			if(configSetup.mode == MODE_11B) {
				uiPrintf(", Rate = %s", DataRate_11b[configSetup.dataRateIndex]);
			}
			else {
				if(configSetup.turbo == TURBO_ENABLE) {
					uiPrintf(", Rate = %s", DataRateStrTurbo[configSetup.dataRateIndex]);
				}
				else if(configSetup.turbo == HALF_SPEED_MODE) {
					uiPrintf(", Rate = %s", DataRateStrHalf[configSetup.dataRateIndex]);
				}
				else if (configSetup.turbo == QUARTER_SPEED_MODE)
				{
					uiPrintf(", Rate = %s", DataRateStrQuarter[configSetup.dataRateIndex] );
				}
				else {
					uiPrintf(", Rate = %s", DataRateStr[configSetup.dataRateIndex]);
				}				
			}
			rateIndex = configSetup.dataRateIndex;
			switch(configSetup.dataPattern)	{
				case ZEROES_PATTERN:
					uiPrintf(", ALL ZEROES");
					break;
				case ONES_PATTERN:
					uiPrintf(", ALL ONES");
					break;
				case REPEATING_5A:
					uiPrintf(", REPEAT 5A");
					break;
				case COUNTING_PATTERN:
					uiPrintf(", COUNTING");
					break;
				case PN7_PATTERN:
					uiPrintf(", PN7");
					break;
				case PN9_PATTERN:
					uiPrintf(", PN9");
					break;
				case REPEATING_10:
					uiPrintf(", REPEAT 10");
					break;
				case RANDOM_PATTERN:
					uiPrintf(", RANDOM");
					break;
			}
			if (configSetup.turbo==TURBO_ENABLE) uiPrintf(", Turbo,");
			else if (configSetup.turbo==HALF_SPEED_MODE) uiPrintf(", Half rate,");	
			else if (configSetup.turbo==QUARTER_SPEED_MODE) uiPrintf(", Quarter rate,");
			break;

		case CONT_FRAME:
			if(configSetup.mode == MODE_11B) {
				uiPrintf(", Rate = %s", DataRate_11b[configSetup.dataRateIndex]);
			}
			else {
				tempRate = DataRate[configSetup.dataRateIndex];
				if(configSetup.turbo == TURBO_ENABLE) {
					uiPrintf(", Rate = %s", DataRateStrTurbo[configSetup.dataRateIndex]);
				}
				else if(configSetup.turbo == HALF_SPEED_MODE) {
					uiPrintf(", Rate = %s", DataRateStrHalf[configSetup.dataRateIndex]);
				}
				else if (configSetup.turbo == QUARTER_SPEED_MODE)
				{
					uiPrintf(", Rate = %s", DataRateStrQuarter[configSetup.dataRateIndex] );
				}
				else {
					uiPrintf(", Rate = %s", DataRateStr[configSetup.dataRateIndex]);
				}
			}							

			switch(configSetup.dataPattern)	{
				case ZEROES_PATTERN:
					uiPrintf(", ALL ZEROES");
					break;
				case ONES_PATTERN:
					uiPrintf(", ALL ONES");
					break;
				case REPEATING_5A:
					uiPrintf(", REPEAT 5A");
					break;
				case COUNTING_PATTERN:
					uiPrintf(", COUNTING");
					break;
				case PN7_PATTERN:
					uiPrintf(", PN7");
					break;
				case PN9_PATTERN:
					uiPrintf(", PN9");
					break;
				case REPEATING_10:
					uiPrintf(", REPEAT 10");
					break;
				case RANDOM_PATTERN:
					uiPrintf(", RANDOM");
					break;
			}
			if (configSetup.turbo==TURBO_ENABLE) uiPrintf(", Turbo");
			if (configSetup.turbo==HALF_SPEED_MODE) uiPrintf(", Half rate");
			if (configSetup.turbo==QUARTER_SPEED_MODE) uiPrintf(", Quarter rate");

			uiPrintf(", Num slots = %d", configSetup.numSlots);

			break;

		case CONT_CARRIER:		// single carrier
			uiPrintf(", [Single Carrier]");
			break;
	}


#ifndef __ATH_DJGPPDOS__
	if (configSetup.artAniEnable) {
		uiPrintf("\nART ANI : NI%d, BI%d, SI%d\n", configSetup.artAniLevel[ART_ANI_TYPE_NI],
					configSetup.artAniLevel[ART_ANI_TYPE_BI],
					configSetup.artAniLevel[ART_ANI_TYPE_SI] );
	}
#endif //__ATH_DJGPPDOS__

}

void displayUtilityMenu(void)
{
	clearScreen();
	printf("\n");
	printf("=================================================================\n");
	printf("| Utility Menu                                                  |\n");
	printf("|   r - Read a register offset                                  |\n");
	printf("|   w - Write to a register offset                              |\n");
//	printf("|   g - (G)et a field value                                     |\n");
	printf("|   p - (P)ut/write a field                                     |\n");
	printf("|   c - (C)hange current channel value                          |\n");
	printf("|   n - (N)oise floor histogram                                 |\n");
	printf("|   s - (S)leep mode toggle                                     |\n");
	printf("|   d - (D)Display PCI writes for resetDevice                   |\n");
	printf("|   o - Test GPI(O)s                                            |\n");
#ifndef CUSTOMER_REL
	printf("|   f - Read r(f) USER data                                     |\n");
#endif
	printf("| ESC - exit                                                    |\n");
	printf("=================================================================\n\n");

	printf("All values entered are treated as Hexadecimal\n\n");
	return;
}

#define DRIVE_LOW       0
#define DRIVE_HIGH      1
#define READ_INPUT      2
#define GPIO_CONTROL_REG    0x4014
#define GPIO_OUTPUT_REG     0x4018
#define GPIO_INPUT_REG      0x401c


void configureGPIOOutput
(
 A_UINT32 devNum,	
 A_UINT16 gpioNumber
)
{
	A_UINT32 configValue;

	configValue = 0x3 << gpioNumber * 2;
	configValue |= REGR(devNum, GPIO_CONTROL_REG);

	REGW(devNum, GPIO_CONTROL_REG, configValue);
	return;
}

void configureGPIOInput
(
 A_UINT32 devNum,	
 A_UINT16 gpioNumber
)
{
	A_UINT32 configValue;

	configValue = ~(0x3 << gpioNumber * 2);
	configValue &= REGR(devNum, GPIO_CONTROL_REG);

	REGW(devNum, GPIO_CONTROL_REG, configValue);
}

void driveGPIO
(
 A_UINT32 devNum,	
 A_UINT16 gpioNumber,
 A_UINT16 value
)
{
	A_UINT32 outputValue;
	
	value = value & 0x1;
	outputValue = REGR(devNum, GPIO_OUTPUT_REG);
	outputValue &= ~(0x1 << gpioNumber); //clear the register bit
	outputValue |= (value << gpioNumber);

	REGW(devNum, GPIO_OUTPUT_REG, outputValue); 
}

A_UINT16 readGPIO
(
 A_UINT32 devNum,	
 A_UINT16 gpioNumber
)
{
	A_UINT32 inputValue;

	inputValue = REGR(devNum, GPIO_INPUT_REG);
	inputValue = (inputValue >> gpioNumber) & 0x1;
	
	return((A_UINT16)inputValue);
}

void utilityMenu
(
 A_UINT32 devNum
)
{
    A_BOOL		done = FALSE;
	A_INT16		inputKey;
	A_UINT32	offset;
	A_UINT32	value;
	A_CHAR		buffer[256];
	A_UINT16	numCh;
	A_UINT16	chList[50];
	A_INT16		nfHist[50];
//	A_UINT32	rd_data;
	A_UINT32	sleep_state = 0;
	double		tempChannel;    //to allow entry to quarter channels
	A_UINT32    temp;
	A_UINT32    mValue;
	A_UINT32    dReg;
	A_UINT32    dBank;
	A_UINT32    numShifts, i;
	A_UINT32    outputSelect;
	A_UINT32    tempNumShifts;

	art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
	displayUtilityMenu();

	while(!done) {

#ifdef __ATH_DJGPPDOS__
		while (!_bios_keybrd(_KEYBRD_READY))
#else
		while (!kbhit())      
#endif
			;

		displayUtilityMenu();
		inputKey = getch();
		switch(inputKey) {
		case 'e':
		case 'E':
			configSetup.loadEar = !configSetup.loadEar;
			art_configureLibParams(devNum);
			art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
			displayUtilityMenu();
			break;
		case 'r':
		case 'R':
			uiPrintf("Enter register offset: ");
			scanf("%x", &offset);
			value = art_regRead(devNum, offset);
			uiPrintf("\nRegister 0x%04x = 0x%08x\n", offset, value);
			break;

		case 'w':
		case 'W':
			uiPrintf("Enter register offset: ");
			scanf("%x", &offset);
			uiPrintf("\nEnter register value: ");
			scanf("%x", &value);
			art_regWrite(devNum, offset, value);
			uiPrintf("\nRegister 0x%04x is written with value 0x%08x\n", offset, value);
			break;

		case 'f':
		case 'F':
			uiPrintf("Enter os1:os0 value: ");
			scanf("%x", &outputSelect);
			if (outputSelect == 0) {
				value = (0x14 | reverseBits(outputSelect, 2));
			}
			else if(outputSelect == 1) {
				uiPrintf("Enter M3:M0 value: ");
				scanf("%x", &mValue);
				value = (reverseBits(mValue, 5) << 8) | 0x14 | reverseBits(outputSelect & 0x3, 2);
//printf("Value = %x\n", value);
			}
			else if(outputSelect == 2) {
				uiPrintf("Enter data register (0-3): ");
				scanf("%x", &dReg);
				uiPrintf("Enter bank (0-7): ");
				scanf("%x", &dBank);
				value = ( ((reverseBits(dBank, 3) << 3) | reverseBits(dReg, 2)) << 16) | 0x14 | reverseBits(outputSelect & 0x3, 2);
			}
			else {
				uiPrintf("NOnly output select values of 0,1 and 2 are currently selected\n");
				break;
			}
			uiPrintf("\nEnter number of shifts: ");
			scanf("%x", &numShifts);
			art_regWrite(devNum, (PHY_BASE+(0x34<<2)), value);
			while(numShifts > 0) {
				if(numShifts > 32) {
					tempNumShifts = 32;
					numShifts -= 32;
				}
				else {
					tempNumShifts = numShifts;
					numShifts = 0;
				}
				for (i=0; i<tempNumShifts; i++) {   
					art_regWrite(devNum, (PHY_BASE+(0x20<<2)), 0x00010000);
				}	
				value = (art_regRead(devNum, PHY_BASE + (256<<2)) >> (32 - tempNumShifts));   
				printf("\nvalue = 0x%x ", value); 
				if(tempNumShifts > 1) {
					printf("(or 0x%x after %d bit reverse)\n", reverseBits(value, tempNumShifts), tempNumShifts);
				}
				else {
					printf("\n");
				}
			}
			break;
		
		case 'g':
		case 'G':
			uiPrintf("Get field not implemented\n");
//			uiPrintf("Enter Field name: ");
//			scanf("%s", buffer);
			
			break;

		case 'p':
		case 'P':
			uiPrintf("Enter field name: ");
			scanf("%s", buffer);
			uiPrintf("\nEnter field value: ");
			scanf("%x", &value);
			art_writeField(devNum, &buffer[0], value);
			uiPrintf("\n Field \"%s\" has been written with the value 0x%04x\n", buffer, value);
			break;

		case 'd':
		case 'D':
			configSetup.printPciWrites = 1;
			art_configureLibParams(devNum);
			art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
			configSetup.printPciWrites = 0;
			art_configureLibParams(devNum);
			break;
		case 'c':
		case 'C':
			uiPrintf("Enter new channel value in MHz: ");
			scanf("%lf", &tempChannel);
			temp = (A_UINT32)(tempChannel * 10);
			if((temp % 25) != 0) {
				uiPrintf("ERROR: Illegal channel value, must be step size of 2.5MHz\n");
				break;
			}
			if(((temp % 25) == 0) && (temp % 50) != 0) {
				//this is a quarter channel
				configSetup.quarterChannel = 1;
				configSetup.channel = (temp - 25)/10;
			}
			else {
				configSetup.quarterChannel = 0;
				configSetup.channel = temp/10;
			}

			if(configSetup.mode == MODE_11A) {
				configSetup.channel5 = configSetup.channel;
			}
			else {
				configSetup.channel2_4 = configSetup.channel;
			}
			if(configSetup.quarterChannel) {
				uiPrintf("Channel will be changed to %7.1lf\n", configSetup.channel + 2.5);
			}
			else {
				uiPrintf("Channel will be changed to %d\n", configSetup.channel);
			}
			break;

		case 't':
		case 'T':
			configSetup.printPciWrites = !configSetup.printPciWrites;
			art_configureLibParams(devNum);
 			break;

		case 0x1b:
			done = TRUE;
			clearScreen();
			clearKeepAGCDisable();
			break;

		case 'n':
		case 'N':
			numCh = getNFChList((A_UINT16)configSetup.channel, &(chList[0]));
			while (!kbhit()) {
				getNoiseFloor(devNum, numCh, chList, &(nfHist[0]));
				plotNFHist(numCh, chList, nfHist);
			}
			break;
		case 's':
		case 'S':
//			rd_data = art_regRead(devNum, 0x400c);
			if (sleep_state < 1) {
//			  rd_data = (rd_data & 0xFFFFFFFC) | 0x1;
			  uiPrintf("Entered sleep mode\n");
			} else {
			  if((swDeviceID & 0xff) >= 0x13) {
//			    rd_data = (rd_data & 0xFFFFFFFC) | 0x2;
			    uiPrintf("Exited sleep mode\n");
			  } 
			}
//			art_regWrite(devNum, 0x400c, rd_data);
			sleep_state = 1 - sleep_state;
			art_writeField(devNum, "mc_sleep_en", sleep_state);
			break;		
		case 'o':
		case 'O':
			{
				A_UINT16 gpioNumber;
				A_UINT16 gpioTest;

				uiPrintf("Enter gpio to test: ");
				scanf("%hd", &gpioNumber);

				//should do some checking for valid gpio
				//basic testing to stop register overrun, need to test per chip
				if(gpioNumber > 15) {
					uiPrintf("Illegal gpio number\n");
					break;
				}
				uiPrintf("Enter gpio test type (%d = drive 0, %d = drive1, %d = read input): ",
					DRIVE_LOW, DRIVE_HIGH, READ_INPUT);
				scanf("%hd", &gpioTest);

				if(gpioTest > 2) {
					uiPrintf("Illegal gpio test entered\n");
					break;
				}

				switch(gpioTest) {
				case DRIVE_LOW:
					uiPrintf("Driving GPIO %d low\n", gpioNumber);
					configureGPIOOutput(devNum, gpioNumber);
					driveGPIO(devNum, gpioNumber, 0);
					break;
				case DRIVE_HIGH:
					uiPrintf("Driving GPIO %d high\n", gpioNumber);
					configureGPIOOutput(devNum, gpioNumber);
					driveGPIO(devNum, gpioNumber, 1);
					break;
				case READ_INPUT:
					configureGPIOInput(devNum, gpioNumber);
					uiPrintf("GPIO is configured for input. Press a key when ready to read");
					getch();
					uiPrintf("\nRead %d from the GPIO %d\n", readGPIO(devNum, gpioNumber),
						gpioNumber);
					break;
					
				}

			}
			break;
		
		case 'a':
		case 'A':
			setKeepAGCDisable();
			printf("Setting flag to disable AGC, will take effect on next rf write field\n");
			break;
		}

	}	

	return;
}

void switchTableLoop(A_UINT32 devNum)
{

	A_BOOL     done = FALSE;
	A_UINT32   fieldval;
	A_INT16    inputKey = ' ';

	while(!done) {
		clearScreen();
		printf("\n");
		printf("=================================================================\n");
		printf("| Loop Through Antenna Switch Table                             |\n");
		printf("|   0 - switch_table_t1                                         |\n");
		printf("|   1 - switch_table_r1                                         |\n");
		printf("|   2 - switch_table_r1x1                                       |\n");
		printf("|   3 - switch_table_r1x2                                       |\n");
		printf("|   4 - switch_table_r1x12                                      |\n");
		printf("|   5 - switch_table_t2                                         |\n");
		printf("|   6 - switch_table_r2                                         |\n");
		printf("|   7 - switch_table_r2x1                                       |\n");
		printf("|   8 - switch_table_r2x2                                       |\n");
		printf("|   9 - switch_table_r2x12                                      |\n");
		printf("| ESC - exit                                                    |\n");
		printf("=================================================================\n\n");

		uiPrintf("Current Selection: %c\n", inputKey);

		while (!kbhit())      
			;

		inputKey = getch();
		switch (inputKey) {
			case '0':
				fieldval = art_getFieldForMode(devNum, "bb_switch_table_t1", configSetup.mode, configSetup.turbo);
				break;
			case '1':
				fieldval = art_getFieldForMode(devNum, "bb_switch_table_r1", configSetup.mode, configSetup.turbo);
				break;
			case '2':
				fieldval = art_getFieldForMode(devNum, "bb_switch_table_r1x1", configSetup.mode, configSetup.turbo);
				break;
			case '3':
				fieldval = art_getFieldForMode(devNum, "bb_switch_table_r1x2", configSetup.mode, configSetup.turbo);
				break;
			case '4':
				fieldval = art_getFieldForMode(devNum, "bb_switch_table_r1x12", configSetup.mode, configSetup.turbo);
				break;
			case '5':
				fieldval = art_getFieldForMode(devNum, "bb_switch_table_t2", configSetup.mode, configSetup.turbo);
				break;
			case '6':
				fieldval = art_getFieldForMode(devNum, "bb_switch_table_r2", configSetup.mode, configSetup.turbo);
				break;
			case '7':
				fieldval = art_getFieldForMode(devNum, "bb_switch_table_r2x1", configSetup.mode, configSetup.turbo);
				break;
			case '8':
				fieldval = art_getFieldForMode(devNum, "bb_switch_table_r2x2", configSetup.mode, configSetup.turbo);
				break;
			case '9':
				fieldval = art_getFieldForMode(devNum, "bb_switch_table_r2x12", configSetup.mode, configSetup.turbo);
				break;
			case 0x1b:
				done = TRUE;
				uiPrintf("exiting\n");
				art_resetDevice(devNum, rxStation, bssID, configSetup.channel, configSetup.turbo);
				return;
			default:
				uiPrintf("Unknown command\n");
				Sleep(1);
				break;
		}

		changeRegValueField(devNum, "bb_switch_table_t1", fieldval);
		changeRegValueField(devNum, "bb_switch_table_r1", fieldval);
		changeRegValueField(devNum, "bb_switch_table_r1x1", fieldval);
		changeRegValueField(devNum, "bb_switch_table_r1x2", fieldval);
		changeRegValueField(devNum, "bb_switch_table_r1x12", fieldval);
		changeRegValueField(devNum, "bb_switch_table_t2", fieldval);
		changeRegValueField(devNum, "bb_switch_table_r2", fieldval);
		changeRegValueField(devNum, "bb_switch_table_r2x1", fieldval);
		changeRegValueField(devNum, "bb_switch_table_r2x2", fieldval);
		changeRegValueField(devNum, "bb_switch_table_r2x12", fieldval);
	}
}

		
A_UINT16 getNFChList(A_UINT16 ch, A_UINT16 *channels)
{
	A_UINT16	chStep, ii;
	A_UINT16	loCh, hiCh;
	A_UINT16	NUM_SHOW_CHANNELS = 30;
	A_UINT16	min_2g_chan_lim = MIN_2G_CHANNEL;
	A_UINT16	max_2g_chan_lim = MAX_2G_CHANNEL;

#if defined(CUSTOMER_REL) 
	min_2g_chan_lim = 2312;
	max_2g_chan_lim = 2482;
#endif		


	//printf("ch = %d\n", ch);

	if (configSetup.mode != MODE_11A) {
	  ch = (ch < min_2g_chan_lim) ? min_2g_chan_lim : ch;
	  ch = (ch > max_2g_chan_lim) ? max_2g_chan_lim : ch;
	} else {
	  ch = (ch < MIN_CHANNEL) ? MIN_CHANNEL : ch;
	  ch = (ch > MAX_SOM_CHANNEL) ? MAX_SOM_CHANNEL : ch;
	}

	chStep = ((ch <= max_2g_chan_lim) && (ch >= min_2g_chan_lim)) ? 5 : 10;
	
	loCh = (A_UINT16)(ch - chStep*(NUM_SHOW_CHANNELS/2)) ;
	hiCh = (A_UINT16)(ch + chStep*(NUM_SHOW_CHANNELS/2)) ;
	
	if (chStep == 5) {
		if (((loCh % 10) == 7) &&((loCh >= 2472)||(loCh < 2412))) {
			loCh += 5;
		}
		if (((hiCh % 10) == 7) &&((hiCh >= 2472)||(hiCh < 2412))) {
			hiCh -= 5;
		}
		if (loCh < min_2g_chan_lim) {
		  loCh =  min_2g_chan_lim ;
		  hiCh = loCh + chStep*NUM_SHOW_CHANNELS;
		}
		if (hiCh > max_2g_chan_lim) {
		  hiCh =  max_2g_chan_lim ;
		  loCh = hiCh - chStep*NUM_SHOW_CHANNELS;
		}
	} else {
		if (loCh < MIN_CHANNEL) {
		  loCh =  MIN_CHANNEL ;
		  hiCh = loCh + chStep*NUM_SHOW_CHANNELS;
		}
		if (hiCh > MAX_SOM_CHANNEL) {
		  hiCh =  MAX_SOM_CHANNEL ;
		  loCh = hiCh - chStep*NUM_SHOW_CHANNELS;
		}
	}
	
	
	//	printf("hich=%d, loch=%d\n", hiCh, loCh);
	ii = 0;
	channels[ii] = loCh;	

	while ((ii <= NUM_SHOW_CHANNELS) && (channels[ii] < hiCh)) {
		if (chStep == 10) {
			chStep = (channels[ii] >= 5725) ? 5 : chStep;
			chStep = ((channels[ii] >= 2412)&&(channels[ii] < 2472)) ? 5 : chStep;
		} else {
			if (channels[ii] < max_2g_chan_lim) {
				if ((channels[ii] >= 2472)||(channels[ii] < 2412)) {
					chStep = 10 ;
					hiCh = channels[ii] + (NUM_SHOW_CHANNELS - ii)*chStep;
					hiCh = (hiCh < max_2g_chan_lim) ? hiCh : max_2g_chan_lim;
				}
			}
		}
		ii++;
		channels[ii] = channels[ii-1] + chStep;
	}

	ii++;

	//	printf("hich=%d, loch=%d, numCh=%d\n", hiCh, loCh, ii);
	return(ii);
}

						
void getNoiseFloor(A_UINT32 devNum, A_UINT16 numCh, A_UINT16 *channels, A_INT16 *nfHist)
{
	A_UINT16	ch, ii;
	A_INT16		nf;
	A_UINT32	rd_data;

	for (ii=0; ii < numCh; ii++) {
			ch = channels[ii];
			if (ch == 2482)
				ch = 2484;
			art_resetDevice(devNum, txStation, bssID, ch, configSetup.turbo);
			rd_data = art_regRead(devNum, 0x9800 + (25<<2));
			nf = (A_INT16)((rd_data >> 19) & 0x1FF);
			nf = ( (nf & 0x100) != 0) ? (0 - ((nf^0x1FF) + 1)) : nf;
			//nf *= -1;
			nfHist[ii] = nf;
			//printf("nf[ii] = %d\n", nfHist[ii]);
	}
}


void plotNFHist(A_UINT16 numCh, A_UINT16 *chList, A_INT16 *nfHist)
{
	A_INT16		ymin = -108 ;
	A_INT16		ymax = -72;
	A_UINT16	max_ydelta = 40;
	A_INT16		ii, jj, dig;
	A_UINT16	scale_factor;

	for (ii=0; ii < numCh; ii++) {
		ymax = (nfHist[ii] > ymax) ? nfHist[ii] : ymax;
		ymin = (nfHist[ii] < ymin) ? nfHist[ii] : ymin;	
	}
	ymax += 2;
	ymin -= 2;

	if ((ymax - ymin) > max_ydelta) {
		ymax = ymin + max_ydelta;
	} else {
		ymin = ymax - max_ydelta;
	}

	clearScreen();
	printf("\n\n\n");

	for (jj = ymax; jj >= ymin; jj--) {
		if ((jj % 5) == 0) {
			printf("%5d =  ", jj);
		} else {
			printf("      |  ");
		}
		
		for (ii=0; ii < numCh; ii++) {
			printf((nfHist[ii] >= jj) ? "* " : "  ");
		}

		printf("\n");
	}

	printf("       -----------------------------------------------------------------\n");
	scale_factor = 1000;
	for(jj=0; jj<4; jj++) {
		printf("         ");		
		for (ii=0; ii < numCh; ii++) {
			dig = (A_UINT16)(chList[ii]/scale_factor);
			printf("%1d ", (dig % 10));
		}
		printf("\n");
		scale_factor /= 10;
	}
}

 #ifndef __ATH_DJGPPDOS__
A_BOOL show_eep_label (A_UINT32 devNum, A_BOOL print) {

	A_UINT32 tmpWord[0xF];
    A_UINT32 chksum = 0;

	art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);
	art_eepromReadBlock(devNum, 0xb0, 0xF, tmpWord);

    chksum = dataArr_get_checksum(devNum, 0, 0xE, tmpWord);

                                         
    if ((tmpWord[0xD] == 0xF000) && (chksum == tmpWord[0xE])){ //matches label format version number
        sprintf(yldStruct.cardLabel, "%c%c%c%c-%c%c-%c%c%c%c%c", ((tmpWord[0] & 0xFF) >> 0),
										 ((tmpWord[0] & 0xFFFF) >> 8),										 										 
             
										 ((tmpWord[1] & 0xFF) >> 0),
										 ((tmpWord[1] & 0xFFFF) >> 8),
										 ((tmpWord[2] & 0xFF) >> 0),
										 ((tmpWord[2] & 0xFFFF) >> 8),
										 ((tmpWord[3] & 0xFF) >> 0),
										 //((tmpWord[3] & 0xFFFF) >> 8),
										 ((tmpWord[4] & 0xFF) >> 0),
										 ((tmpWord[4] & 0xFFFF) >> 8),
										 ((tmpWord[5] & 0xFF) >> 0),
										 ((tmpWord[5] & 0xFFFF) >> 8));
									//	 ((tmpWord[6] & 0xFF) >> 0),
									//	 ((tmpWord[6] & 0xFFFF) >> 8) );
										 
        if (print) uiPrintf("Card EEPROM Label = %s\n", yldStruct.cardLabel);
		return TRUE;
	}
    else if ((tmpWord[0xD] == 0xF001) && (chksum == tmpWord[0xE])){ //matches label format version number
        sprintf(yldStruct.cardLabel, "%c%c%c%c-%c%c%c-%c%c%c%c%c", ((tmpWord[0] & 0xFF) >> 0),
										 ((tmpWord[0] & 0xFFFF) >> 8),										 										 
             
										 ((tmpWord[1] & 0xFF) >> 0),
										 ((tmpWord[1] & 0xFFFF) >> 8),
										 ((tmpWord[7] & 0xFF) >> 0),										 
										 ((tmpWord[2] & 0xFF) >> 0),
										 ((tmpWord[2] & 0xFFFF) >> 8),
										 ((tmpWord[3] & 0xFF) >> 0),
										 //((tmpWord[3] & 0xFFFF) >> 8),
										 ((tmpWord[4] & 0xFF) >> 0),
										 ((tmpWord[4] & 0xFFFF) >> 8),
										 ((tmpWord[5] & 0xFF) >> 0),
										 ((tmpWord[5] & 0xFFFF) >> 8));
										 
        if (print) uiPrintf("Card EEPROM Label = %s\n", yldStruct.cardLabel);
		return TRUE;
	}
	//checksum did not match, no label or bad label
	return FALSE;
}


void setupAtherosCalLogging() {

	A_CHAR  cardfilename[128];
	A_CHAR  tmpStr[512];
    time_t   current;
    struct   tm * bt;

    current = time(NULL);

    bt = localtime(&current);

	// make the card label. all the card info must be filled in the yldStruct by this time
	if(yldStruct.labelFormatID == 0xf000) {
	sprintf(yldStruct.cardLabel, "%4s_%02d_%s%04d", yldStruct.cardType, 
													   yldStruct.cardRev, 
													   yldStruct.mfgID,
													   yldStruct.cardNum);
	}
	else if (yldStruct.labelFormatID == 0xf001) {
		sprintf(yldStruct.cardLabel, "%4s_%1d%02d_%s%04d", yldStruct.cardType,
														   yldStruct.chipIdentifier,
														   yldStruct.cardRev, 
														   yldStruct.mfgID,
														   yldStruct.cardNum);
	}
	else {
		uiPrintf("ERROR: Invalid labelFormatID = %x\n", yldStruct.labelFormatID);
		exit(0);
	}

	sprintf(cardfilename, "%s.log", yldStruct.cardLabel);

	if(strnicmp("INVALID", configSetup.manufName, strlen("INVALID")) == 0) {
		uiPrintf("ERROR: A valid MANUFACTURER_NAME must be specified in artsetup.txt when ATHEROS_LOGGING_SCHEME = 1\n");
		exit(0);
	}

	sprintf(configSetup.logFile, "%s\\%s\\FORMATTED_LOG\\%s", 
		(CalSetup.logFilePath[0] == '\0') ? BASE_LOG_DIR : CalSetup.logFilePath, configSetup.manufName, cardfilename);
	sprintf(configSetup.yieldLogFile, "%s\\%s\\YIELD_LOG\\%s", 
		(CalSetup.logFilePath[0] == '\0') ? BASE_LOG_DIR : CalSetup.logFilePath, configSetup.manufName, cardfilename);

	incrementLogFile(configSetup.logFile);
	incrementLogFile(configSetup.yieldLogFile);

	uilog(configSetup.logFile, 0);
	uiOpenYieldLog(configSetup.yieldLogFile, 0);

	sprintf(tmpStr, "CAL FORMAT VERSION : %3.1f\n\n", CAL_FORMAT_VERSION);
	uiWriteToLog(tmpStr);
	sprintf(tmpStr, "YIELD FORMAT VERSION : %3.1f\n\n", YIELD_FORMAT_VERSION);
	uiYieldLog(tmpStr);
	
	sprintf(tmpStr, "Cal Date: %4d/%02d/%02d\n", (1900+bt->tm_year), (bt->tm_mon + 1), bt->tm_mday);
	uiWriteToLog(tmpStr);
	uiYieldLog(tmpStr);
	
	sprintf(tmpStr, "Cal Time: %02d:%02d\n\n", bt->tm_hour, bt->tm_min);
	uiWriteToLog(tmpStr);
	uiYieldLog(tmpStr);
	
	sprintf(tmpStr, "card_macID card_type card_rev card_mfg card_num test_name devNum mode test_param1 test_param2 test_param3 test_result meas_name test_target test_meas test_unit test_meas2_name test_meas2 test_meas2_unit additional_mac_id1 additional_mac_id2 additional_mac_id3 enet_macID_1 enet_macID_2 enet_macID_3 enet_macID_4 \n\n");
	uiYieldLog(tmpStr);
}
				
void closeAtherosCalLogging() {
	uilogClose();
	uiCloseYieldLog();
}
#endif
void incrementLogFile(A_CHAR *filename) {

	A_CHAR      newfilename[128];
	A_CHAR      lineBuf[1024];
	A_UINT32    ii, jj;
	FILE        *file;
	FILE        *newfile;

	//uiPrintf("SNOOP: trying new filename %s\n", filename);
	file = fopen(filename, "r");

	if (file == NULL) {
		return;
	} else {
		fclose(file);
		for (ii=1; ii<50; ii++) {
			jj = 0;
			while (filename[jj] != '.') {
				newfilename[jj] = filename[jj];
				jj++;
			}
			sprintf(&(newfilename[jj]), "_%02d.log", ii);
			//uiPrintf("SNOOP: trying new filename %s\n", newfilename);
			file = fopen(newfilename, "r");
			if (file == NULL) {
				break;
			} else {
				fclose(file);
			}
		}
		if (ii==50) {
			return; // overwrite if already done cal 50 times
		}

		file = fopen(filename, "r");
		newfile = fopen(newfilename, "w");
		while(fgets(lineBuf, 1023, file) != NULL) {
			fprintf(newfile, "%s", lineBuf);
		}
		fclose(newfile);
		fclose(file);
	}
}
 #ifndef __ATH_DJGPPDOS__
A_BOOL promptForLabel(A_BOOL autoDetect)
{
	A_INT32  tempDevNum;
	A_UINT32 tempDevID;
	A_BOOL	  gotLabel = FALSE;

	//Want to look to see if the card is plugged in, use art_setupDevice as the test
	//assume that we have not already setup for any device yet
	//autoDetect set to 0 overrides this auto detect mechanism
	if(autoDetect) {
		tempDevNum = art_setupDevice(configSetup.instance);
		if(tempDevNum >= 0) {
			//a card is already plugged in, see if we can read the label from the card
			//note that we need to do a resetDevice here, so care needs to be taken that
			//we are accessing the correct device 
		
			//do a cfg read to determine which device is safe to access
			tempDevID = art_cfgRead(tempDevNum, 0);

			if(((tempDevID >> 16) & 0xffff) == 0xa017) {
				if(configSetup.instance != 2) {
					//should use the second radio
					art_teardownDevice(tempDevNum); 
					
					//attempt to setup second device
					tempDevNum = art_setupDevice(2);
					if(tempDevNum < 0) {
						uiPrintf("main: Error attaching to the device - ending test\n");
						closeEnvironment();
						exit(0);
					}
				}
			}

			//If requested attemp to read the label from the eeprom
			//note: show_eep_label will fill the label into yldStruct.cardLabel
			if(show_eep_label(tempDevNum, 0)) {
				gotLabel = TRUE;
			}
		}
	}

	if(!gotLabel) {
		//will come in here either if no card is plugged in, or we are unable 
		//to read the label from the card.
		uiPrintf("Please enter label for card: ");
		scanf("%s", yldStruct.cardLabel);
	}

	while(kbhit())		// Flush out previous key press.
		getch();

	//extract the info from the label and fill in the other yldStruct fields
	if(!updateStructFromLabel()) {
		if(autoDetect) art_teardownDevice(tempDevNum); 
		return FALSE;
	}
	if(autoDetect) art_teardownDevice(tempDevNum); 
	//if we didn't get the label by reading from the card, then prompt for card 
	//insertion
	if(!gotLabel) {
		uiPrintf("Insert card now. Press Q to exit or any other key to continue\n");
		if( toupper(getch()) == 'Q' ) {
			exit(0);            
	    }
	}

	if (configSetup.computeCalsetupName > 0) {
		if (machName) {
			sprintf(calsetupFileName, "calsetup_%s_%c%c", strlwr(machName), 
				                            tolower(yldStruct.cardLabel[0]),
											tolower(yldStruct.cardLabel[1]));
			if(yldStruct.chipIdentifier != NO_CHIP_IDENTIFIER) {
				sprintf(calsetupFileName, "%s_%1d.txt", calsetupFileName,
													yldStruct.chipIdentifier);
			}
			else {
				sprintf(calsetupFileName, "%s.txt", calsetupFileName);
			}
		} else {
			uiPrintf("Fatal Error : Could Not Retrieve Machine Name\n");
			exit(0);
		}
	} else {
		sprintf(calsetupFileName, "calsetup.txt");
	} 

	return TRUE;
}

A_BOOL updateStructFromLabel(void)
{
	A_CHAR tempBuffer[10];
	A_UINT32 partialMacId;
	A_UINT32 *pMacId;
	A_UINT32 i, j;
    A_CHAR      *token;
	A_CHAR   cardLabel[256];
	char   delimiters[] = " -\n";

	strncpy(cardLabel, yldStruct.cardLabel, 256);

	//get the board name from the label, should be the first 4 chars of the string
	token = strtok(cardLabel, delimiters);

	if(NULL == token) {
		uiPrintf("Error: unable to get board name from the card label %s\n", yldStruct.cardLabel);
		return FALSE;
	}
	strcpy(yldStruct.cardType, token);

	//get the card rev from the string, should be 'middle' digits
	token = strtok(NULL, delimiters);
	if(NULL == token) {
		uiPrintf("Error: unable to get fab/bom rev from the card label %s\n", yldStruct.cardLabel);
		return FALSE;
	}

	if(strlen(token) == 2) {
		yldStruct.chipIdentifier = NO_CHIP_IDENTIFIER;
		tempBuffer[0] = token[0];
		tempBuffer[1] = token[1];
	}
	else if(strlen(token) == 3) {
		tempBuffer[0] = token[0];
		tempBuffer[1] = '\0';

		if(!sscanf(tempBuffer, "%d", &yldStruct.chipIdentifier)) {
			uiPrintf("Error: unable to get chip identifier from the card label %s\n", yldStruct.cardLabel);
			return FALSE;
		}

		tempBuffer[0] = token[1];
		tempBuffer[1] = token[2];
		yldStruct.labelFormatID = 0xF001;
	}
	else {
			uiPrintf("Error: unable to get chip identifier from the card label %s\n", yldStruct.cardLabel);
			return FALSE;
	}
//    strncpy(tempBuffer, &yldStruct.cardLabel[5], 2); 
	if(!sscanf(tempBuffer, "%d", &yldStruct.cardRev)) {
		uiPrintf("Error: unable to get fab/bom rev from the card label %s\n", yldStruct.cardLabel);
		return FALSE;
	}

	token = strtok(NULL, delimiters);
	if(NULL == token) {
		uiPrintf("Error: unable to get mfg id or serial number from the card label %s\n", yldStruct.cardLabel);
		return FALSE;
	}
	//get the mfg id from the card label
	yldStruct.mfgID[0] = token[0];

	//Get the serial number from the card label, should be the last 4 digits
    if(!sscanf(&token[1], "%04d", &yldStruct.cardNum)) {
		uiPrintf("Error: unable to get serial number from the card label %s\n", yldStruct.cardLabel);
		return FALSE;
	}
	
	//create the macIDs from the productID and serial number lookup.
	//First need to lookup the productID from the board name
	for(i = 0; i < sizeLabelTable; i++) {
		if( (strcmp(labelTable[i].cardName, strupr(yldStruct.cardType))==0) &&
		 (((yldStruct.chipIdentifier == NO_CHIP_IDENTIFIER) && (labelTable[i].chipID == NO_CHIP_IDENTIFIER)) ||
		  (yldStruct.chipIdentifier == labelTable[i].chipID)) )
		{
			break;
		}
	}

	if(i == sizeLabelTable) {
		uiPrintf("Error: illegal card Name in card label %s\n", yldStruct.cardLabel);
		return FALSE;
	}

	//get the last 3 chars of the macID
	partialMacId = ((labelTable[i].productID & 0x7ff) << 13) | (yldStruct.cardNum & 0x1fff);

	//fill in the needed WLAN macIDs
	for(j = 0; j < labelTable[i].numWlanMacs; j++) {
		switch(j) {
		case 0: 
			pMacId = yldStruct.macID1;
			break;
		case 1: 
			pMacId = yldStruct.macID2;
			break;
		case 2: 
			pMacId = yldStruct.macID3;
			break;
		case 3: 
			pMacId = yldStruct.macID4;
			break;
		default:
			uiPrintf("Error: Lookup for card %s contains an illegal number of WLAN MAC Ids (%d)\n",
				labelTable[i].cardName, labelTable[i].numEthMacs);
			return FALSE;
		}

		pMacId[0] = 0x0003;
		pMacId[1] = 0x7f00 | ((partialMacId & 0xff0000) >> 16);
		pMacId[2] = (( partialMacId & 0xffff)  + j);
	}

	//fill in the needed ethernet macIDs
	for(j = 0; j < labelTable[i].numEthMacs; j++) {
		switch(j) {
		case 0: 
			pMacId = yldStruct.enetID1;
			break;
		case 1: 
			pMacId = yldStruct.enetID2;
			break;
		case 2: 
			pMacId = yldStruct.enetID3;
			break;
		case 3: 
			pMacId = yldStruct.enetID4;
			break;
		default:
			uiPrintf("Error: Lookup for card %s contains an illegal number of ethernet MAC Ids (%d)\n",
				labelTable[i].cardName, labelTable[i].numEthMacs);
			return FALSE;
		}

		pMacId[0] = 0x0003;
		pMacId[1] = 0x7f00 | ((partialMacId & 0xff0000) >> 16);
		pMacId[2] = (partialMacId & 0xffff) + labelTable[i].numWlanMacs + j;
	}

	
	//update the blank ssid and dut ssid with this new ssid
	configSetup.blankEepID = labelTable[i].subsystemID;
	configSetup.dutSSID = labelTable[i].subsystemID;
	configSetup.cmdLineSubsystemID = labelTable[i].subsystemID;

	if(configSetup.debugInfo) {
		uiPrintf("Card Label                = %s\n", yldStruct.cardLabel);
		uiPrintf("Card name                 = %s\n", yldStruct.cardType);
		uiPrintf("Card Rev                  = %d\n", yldStruct.cardRev);
		uiPrintf("Chip identifier           = %d\n", yldStruct.chipIdentifier);
		uiPrintf("Card Manufacturing ID     = %c\n", yldStruct.mfgID[0]);
		uiPrintf("Card Serial Number        = %d\n", yldStruct.cardNum);
		for(j = 0; j < labelTable[i].numEthMacs; j++) {
			switch(j) {
			case 0: 
				pMacId = yldStruct.enetID1;
				break;
			case 1: 
				pMacId = yldStruct.enetID2;
				break;
			case 2: 
				pMacId = yldStruct.enetID3;
				break;
			case 3: 
				pMacId = yldStruct.enetID4;
				break;
			}
			uiPrintf("Ethernet MAC ID %d = %x:%x:%x:%x:%x:%x\n", j, 
				(pMacId[0] >> 8) & 0xff, pMacId[0] & 0xff,
				(pMacId[1] >> 8) & 0xff, pMacId[1] & 0xff,
				(pMacId[2] >> 8) & 0xff, pMacId[2] & 0xff);
		}
		for(j = 0; j < labelTable[i].numWlanMacs; j++) {
			switch(j) {
			case 0: 
				pMacId = yldStruct.macID1;
				break;
			case 1: 
				pMacId = yldStruct.macID2;
				break;
			case 2: 
				pMacId = yldStruct.macID3;
				break;
			case 3: 
				pMacId = yldStruct.macID4;
				break;
			}
			uiPrintf("Wireless MAC ID %d = %x:%x:%x:%x:%x:%x\n", j, 
				(pMacId[0] >> 8) & 0xff, pMacId[0] & 0xff,
				(pMacId[1] >> 8) & 0xff, pMacId[1] & 0xff,
				(pMacId[2] >> 8) & 0xff, pMacId[2] & 0xff);
		}
	}
	return TRUE;
}
#endif
void displayNoiseImmunityMenu(void)
{
//	clearScreen();
	printf("\n");
	printf("=================================================================\n");
	printf("| Noise Immunity Menu                                           |\n");
if (configSetup.artAniEnable == ART_ANI_DISABLED) {
	printf("|   e - (E)nable noise Immunity Level                           |\n");
} else {
	printf("|   e - Disable noise Immunity Level                            |\n");
}
	printf("|   n - (N)oise Immunity Level                                  |\n");
	printf("|   b - (B)arker Immunity Level                                 |\n");
	printf("|   s - (S)pur Immunity Level                                   |\n");
	printf("|   a - (A)utomatic Noise Immunity [Not available yet]          |\n");
	printf("| ESC - exit                                                    |\n");
	printf("=================================================================\n\n");

	printf("NI:%d, BI:%d, SI:%d\n\n", configSetup.artAniLevel[ART_ANI_TYPE_NI],
									  configSetup.artAniLevel[ART_ANI_TYPE_BI],
									  configSetup.artAniLevel[ART_ANI_TYPE_SI]);
	return;
}


void noiseImmunityMenu
(
 A_UINT32 devNum
)
{
    A_BOOL		done = FALSE;
	A_INT16		inputKey;
	
	art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);

	displayNoiseImmunityMenu();
	while(!done) {

#ifdef __ATH_DJGPPDOS__
		while (!_bios_keybrd(_KEYBRD_READY))
#else
		while (!kbhit())      
#endif
			;

		inputKey = getch();
		switch(inputKey) {
		case 'n':
		case 'N':
			configSetup.artAniLevel[ART_ANI_TYPE_NI] += 1;
			if (configSetup.artAniLevel[ART_ANI_TYPE_NI] > 4) {
				configSetup.artAniLevel[ART_ANI_TYPE_NI] = 0;
			}
			#if defined(ART_BUILD) || defined(__ATH_DJGPPDOS__)
			art_setArtAniLevel(devNum, ART_ANI_TYPE_NI, configSetup.artAniLevel[ART_ANI_TYPE_NI]);			
			#endif
			break;
		case 'b':
		case 'B':
			configSetup.artAniLevel[ART_ANI_TYPE_BI] += 1;
			if (configSetup.artAniLevel[ART_ANI_TYPE_BI] > 2) {
				configSetup.artAniLevel[ART_ANI_TYPE_BI] = 0;
			}
			#if defined(ART_BUILD) || defined(__ATH_DJGPPDOS__)
			art_setArtAniLevel(devNum, ART_ANI_TYPE_NI, configSetup.artAniLevel[ART_ANI_TYPE_NI]);			
			#endif
			break;

		case 's':
		case 'S':
			configSetup.artAniLevel[ART_ANI_TYPE_SI] += 1;
			if (configSetup.artAniLevel[ART_ANI_TYPE_SI] > 1) {
				configSetup.artAniLevel[ART_ANI_TYPE_SI] = 0;
			}
			#if defined(ART_BUILD) || defined(__ATH_DJGPPDOS__)
			art_setArtAniLevel(devNum, ART_ANI_TYPE_NI, configSetup.artAniLevel[ART_ANI_TYPE_NI]);			
			#endif
			break;
		case 'e':
		case 'E':
			if (configSetup.artAniEnable == ART_ANI_DISABLED) {
				configSetup.artAniEnable = ART_ANI_ENABLED;
			} else {
				configSetup.artAniEnable = ART_ANI_DISABLED;
			}
			break;
		case 0x1b:
        case 'Q':
            done = TRUE;
            uiPrintf("exiting\n");
            break;
		
		}
		art_configureLibParams(devNum);
		art_resetDevice(devNum, txStation, bssID, configSetup.channel, configSetup.turbo);				
		displayNoiseImmunityMenu();		
	}	
	return;
}

#endif  // REGULATORY_REL

A_BOOL getPCIConfigMapping
(
	A_UINT32 devNum,
	A_UINT32 eepromLocation,
	A_UINT32 *remapLocation
)
{
	A_UINT32 i;
	A_UINT32 pciConfigAddress;
	A_BOOL   msbLocation = FALSE;
	A_UINT32 eepromValue;
	A_UINT32 address;

	for(i = 0; i < numMappings; i++) {
		//check to see if this eeprom location is an MSB mapping
		if(eepromLocation == fullMapping[i].eepromLocation_MSB) {
			pciConfigAddress = (0x5000 + fullMapping[i].pciConfigLocation)/4;
			msbLocation = TRUE;
			break;
		}
		
		//else check to see if it is an LSB mapping
		if(eepromLocation == fullMapping[i].eepromLocation_LSB) {
			pciConfigAddress = (0x5000 + fullMapping[i].pciConfigLocation)/4;
			break;
		}
		
	}

	if( i != numMappings ) {
		//Now look for the eeprom location where this address has been written
		address = art_eepromRead(devNum, 2) * 2;
		while ((eepromValue = art_eepromRead(devNum, address)) != 0xffff) {
			if(eepromValue == pciConfigAddress) {
				//there is a match, so return the address
				*remapLocation = (msbLocation == TRUE) ? (address + 2) : (address + 1);
				return TRUE;
			}

			address += 3;
		}
	}
	return FALSE;
}
