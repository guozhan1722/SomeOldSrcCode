#include "project.h"

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

#ifdef _WINDOWS 
#include <windows.h>
#endif 
#include <stdio.h>
#ifndef LINUX
#include <conio.h>
#endif
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include "wlantype.h"   /* typedefs for A_UINT16 etc.. */
#include "wlanproto.h"
#include "athreg.h"
#include "manlib.h"     /* The Manufacturing Library */
#include "MLIBif.h"     /* Manufacturing Library low level driver support functions */
#ifdef JUNGO
#include "mld.h"        /* Low level driver information */
#endif
#include "common_hw.h"
#ifdef __ATH_DJGPPDOS__
#include "mlibif_dos.h"
#endif
#include "manlibInst.h" /* The Manufacturing Library Instrument Library extension */
#include "mEeprom.h"        /* Definitions for the data structure */
#include "test.h"
#include "parse.h"
//daniel #include "dynArray.h"

#include "art_if.h"
#include "dynamic_optimizations.h"
#include "maui_cal.h"        /* Definitions for the Calibration Library */
#include "dk_ver.h"

// non-ansi functions are mapped to some equivalent functions
#ifdef LINUX
//daniel #include "linux_ansi.h"
#endif

A_UINT16 numRAWChannels_2p4 = 3; 

GOLDEN_PARAMS  goldenParams  = { 4900,5850,20,70,0,63, 1,
								11,{0,10,20,30,40,50,60,70,80,90,100} } ;

CAL_SETUP CalSetup = {42, 0, ATHEROS_CB22, 0x01, EEPROM_SIZE_16K, 0x168C, 
					0, 0, 0x010,  // country code, WWR, default domain 0x010 (FCC)
					0, // calpower
					"R:\\nOFDM\\restoreMHX_1.eep",
					0,
					10.0, //offsetCalpower
					0,    // useFastCal
					0, 0, 0, 0, 0, 0, 0, 0, 0, // 11a test flags
					{0,0,0}, 0.5, 1.0, 0.5, 1.0, 5.0, 5.0, 54, // target power test flags: g/b/a modes
//					{-68, -68, -68, -68, -68, -68, -68, -68}, //target 11a sensitivity at 54 mbps
					0, 0, 3, 11.0, // 11a golden power cal data
					{3,3}, {-2.0, -2.0}, // 11g and 11b golden pcdacs and power
					PM_436A, 13,  // PM_model number and GPIB addresses
					SPA_E4404B, 18,   // SA_model number and GPIB addresses
					ATT_11713A, 6,    // ATT_model number and GPIB addresses
					DMM_FLUKE_45, 10,  //DMM (current meter) model number and GPIB address
					11.3, 11.3, 11.3, 50, // attenuation factors
					{10.0, 10.0},{10.0, 10.0},{10.0, 10.0}, {30.0, 30.0}, //2.5G atten factors
                    1, // numEthernetPorts
					0, // startEthernetPort
					0, 0, 0, // turbo_disable, turbo_disable_11g, rf_silent
					1,    // deviceType
					15, 15, // turboMaxPwr_5G, TurboMaxPower_11g
					1,0,0, // Amode, Bmode, Gmode
					0,0,  // antennaGain5G and 2.5G
					
					//Current Consumption Test parameters
					{ 0, 
						{0,0,0},
						{0,0,0},
						0.0, 0.0,
						{{600.0, 600.0, 600.0}, {400.0, 400.0, 400.0}},
						{{100.0, 100.0, 100.0}, {50.0, 500.0, 50.0}},
					}, 
					// end CURR_CONSMPN_STTNGS 

					0,   //test continuity
					3,   //continuity output gpio (based on xb63)
					0,   //continuity input gpio (based on xb63)

					// 11a parameters
					0x2D, 0xB, 2, 2, 2,2, 1,2, 1,2, 0x1C, 0, 0, 0xE, 1, 13, 14, -85, -32, -72,
					{0,0,0,0,0,0,0,0,0,0,0}, 
					{0, 0, 0},    // fixed_bias
					TRUE,         // do_iq_cal
					{0, 0, 0},    // iqcal_i_corr
					{0, 0, 0},    // iqcal_q_corr
					//ofdm@2.4 and 11b parameters
					{0x2D,0x2D}, {0xB, 0xB}, 
					{2,2}, {2,2}, {3,3}, {3,3},  // ob/db, b_ob/b_db
					{0x1C, 0x1C}, {0,0}, {0,0}, 
					{0xE, 0xE}, {1,1}, {13,13}, {13,13}, {-70,-75}, {-32,-32}, {-72,-72},
					{{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}}, 
					0, "cal_AR5211_power.log", // readFromFile
					0, // customrDebug
					0, // showTimingReport
					FALSE, // endTestOnFail
					"R:\\nOFDM\\macid.txt",
					"",
					0, {0,0}, 0, // force piers, list, num_piers
					{0,0}, {{2312, 2412, 2484}, {2412, 2442, 2484}}, {3,3}, // force piers info for 11g, 11b
					FALSE,    // useOneCal for 11g/11b
//					40, // pcdac_5dB
					"calTargetPower_cb21.txt",
					{FALSE, FALSE},
					{"cal_AR5211_Power_11g.log", "cal_AR5211_Power_11b.log"},
//					{FALSE, FALSE}, // was calpower_11g, 11b
//					{1,1},
					{FALSE, FALSE},   //spectral mask
					{FALSE, FALSE},   //channel accuracy, no 11b channel accuracy?
					{FALSE, FALSE},   //per
					{FALSE, FALSE},   //sens
					{FALSE, FALSE},  // no turbo mode available in 11b, though
					{FALSE, FALSE, FALSE},  // testTempMargin
					FALSE,  // test32KHzSleepCrystal
					{FALSE, FALSE, FALSE}, // testDataIntegrity
					{FALSE, FALSE, FALSE}, // testThroughput
					{FALSE, FALSE, FALSE}, // testRxThroughput
					FALSE,                 // testTXPER_margin
					{{-68, -68, -68}, {-88, -88},{-68, -68, -68, -68, -68, -68, -68, -68}}, // target sensitivity for 11g, 11b and 11a modes
					//{{4, 1, 1, 0}, {4, 1, 1, 0}, {4, 1, 1, 0}, {4, 1, 1, 0}},
					"0.0.0.0", // golden IP address
					45,   // case Temperature
					{0, 0, 0},    // falsedetect backoff for all modes
					90, 90, 9, -9, 7, -7, 0, // test limits
					FALSE,               // use channel matrix for limit on fail points in spec. mask test
					{30.0, 30.0, 30.0},  // maxPowerCap for 11a/11g/11b modes
					1.5,   // cck_ofdm_delta in dB
					1.5,   // ch14_filter_cck_delta in dB
					3,      // maxRetestIters
					{INVALID_FG, INVALID_FG, INVALID_FG},
					{INVALID_FG, INVALID_FG, INVALID_FG},
					0,     // eeprom_map  0 - gen2, 1 - gen3
					{0x3, 0x3, 0x3},      // cal_mult_xpd_gain_mask  was 9 9 9 Siavash changed from 6 6 6
					0x2BF,  // EAR start addr
					0,      // EAR Len
					0,      // UartPciCfgLen
					0x1A5,   // TrgtPwr start 
					0x150,   //calStart address
					{100, 100, 100},  // number of packets to use for sensitivity
					10,		// txperBackoff
					0,       // Enable_32khz - enable sleep crystal.
					0,       // Enable_WOW  - enable Wake_On_WLAN
					{16, 16, 11}, // rxtx_margin_2ghz for all modes
					7.5,      // ofdm_cck_gain_delta
					{1, 1, 1},  // instanceForMode
					{0x3, 0},   // modeMaskForRadio
					0, 0, 0, 0,  // iq_coeffs for 5G 2G
					FALSE,     // atherosLoggingScheme
					0,		  //ftpdownload info, disable ftpdownload
					"", "", "", "", "",   //hostname, username, password, remote and local filename

					{0x2D,0x2D, 0x2D},  // switchSettling_Turbo 
					{0xB, 0xB, 0xB},    // txrxAtten_Turbo
					{-32,-32, -32},          // adcDesiredSize_Turbo
					{-72,-72, -72},          // pgaDesiredSize_Turbo
					{16, 16, 11}, // rxtx_margin_2ghz for all modes Turbo
					0, 0, 0, //uart enable. compression disable, fast frame disable
					0, 0,    //bursting disble, AES disable
					0, 0,		  //max num QCU - 0 defaults to current max, key cache size - 0 defaults to current max
					0,            //register specific patch; 0: normal swan, 1: for board #040
					0,            //enableHeavyClip
					0,            //xr disable
					0, 0, 0, 0, 0, 0, //new japan flags 
					1,            // enableDynamicEAR
					1,            // numDynamicEARChannels
					{2442,0,0,0,0,0,0,0},  // dynamicEARChannels
					0x2000,       // dynamicEARVersion
					0,
					63,							// 11a max pcdac
					63,							// 11b max pcdac
					63,							// 11g max pcdac
					2,                   // numPdGains
					{22, 48, 48, 48},    // pdGainBoundary
					10,                   // pdGainOverlap
					0,							// 11a attempt power if max pcdac < 63 does not get there
					0,							// 11b attempt power
					0,							// 11g attempt power
					0,							//num retries for throughput test
                    FALSE,
                    FALSE,
                    FALSE,
                    FALSE,
                    100,                    //extra delay for 0dbm test (in milliseconds)
					FALSE,                  //enable serdes programming
					TRUE,                   //enable ASPM acceptable latency
					TRUE,					//enable WA
					0,0,0,                  //minimum allowed power to test in 0.5dbm steps
					3,                      //Level of ASPM support valid values 0-3
					0,						//set true if attenuator should be zero'd before power or spec mask 
					                        //measurements
					0,
					0
			}; 

YIELD_LOG_STRUCT yldStruct = {  {0xFFFF, 0xFFFF, 0xFFFF},
								{0xFFFF, 0xFFFF, 0xFFFF},
								{0xFFFF, 0xFFFF, 0xFFFF},
								{0xFFFF, 0xFFFF, 0xFFFF},
								{0xFFFF, 0xFFFF, 0xFFFF},
								{0xFFFF, 0xFFFF, 0xFFFF},
								{0xFFFF, 0xFFFF, 0xFFFF},
								{0xFFFF, 0xFFFF, 0xFFFF},
								"mb62", // cardType
								35,   // cardRev
								9999,   // cardNum
								"unk",  // testName
								9999.9, // param1
								9999.9, // param2
								9999.9, // param3
								"unk",  // result
								"unk",  // measName
								9999.9, // target
								9999.9, // meas
								"unk",  // measUnit
								"unk",  // meas2Name
								9999.9, // meas2
								"unk",  // meas2Unit
								"unk",  // mode
								9999,   // devNum
								"A",    // manufID
								"a0",   // reworkID
								0xF000, // label format version
								"unk"   // card_label
};

TARGETS_SET	  TargetsSet ;
TARGETS_SET	  TargetsSet_11b ;
TARGETS_SET	  TargetsSet_11g ;
TARGETS_SET	  *pTargetsSet = &TargetsSet ;
TARGETS_SET	  *pTargetsSet_2p4[2] = {&TargetsSet_11g, &TargetsSet_11b} ;

extern A_UCHAR calsetupFileName[];

extern A_UINT32 VERIFY_DATA_PACKET_LEN;

extern A_UINT32 **EEPROM_DATA_BUFFER;
extern A_BOOL usb_client;

extern A_UINT32 subSystemID;

TEST_GROUP_SET TG_Set ;
TEST_GROUP_SET *pTestGroupSet= &TG_Set ;

TEST_SET	  TestSet_11g ;
TEST_SET	  TestSet_11b ;
TEST_SET	  TestSet_11a ;
TEST_SET	  *pTestSet[3] = {&TestSet_11g, &TestSet_11b, &TestSet_11a} ;

char modeName[3][122] = {"11g", "11b", "11a"};

double     ofdm_gain[] = {-30, -12, -6, -2.5, 0, 2, 3.5, 5};
double     cck_gain[] = {0, -6, -12};

//extern GOLDEN_PARAMS  goldenParams ;
A_UINT16    global_prev_gainf = 0;

static void remapPCIConfigInfo
(
 A_UINT32 devNum,   
 A_UINT32 *eepromData
);
 
static void write_spur_info
(
 A_UINT32 devNum,
 A_UINT32 *common_eeprom_data
);

void load_cal_section(void)
{
    FILE *fStream;
    char lineBuf[122], *pLine;
    A_UINT32 testVal;
	A_UINT16	ii ;
	A_BOOL		parsingCal = FALSE ;
	A_UINT16	parsingLine1 = 1;
	char delimiters[] = " \t\n\r;=" ;
	char *eep_file = configSetup.cfgTable.pCurrentElement->eepFilename;

//    uiPrintf("\nSNOOP: Reading in Cal Section from %s\n", eep_file);
    if( (fStream = fopen( eep_file, "r")) == NULL ) {
        uiPrintf("Failed to open %s - using Defaults\n", eep_file);
        return;
    }

	memset(CalSetup.spur_frequency_5g, 0, sizeof(CalSetup.spur_frequency_5g));
	memset(CalSetup.spur_frequency_2p4g, 0, sizeof(CalSetup.spur_frequency_2p4g));

    while(fgets(lineBuf, 120, fStream) != NULL) {
        pLine = lineBuf;
        while(isspace(*pLine)) pLine++;

		if(strnicmp("@cal_section_begin", pLine, strlen("@cal_section_begin")) == 0)  {
            parsingCal = TRUE;
			ii = 0;
			continue;
        }

		// skip comments
        if(*pLine == '#') {
            continue;
        }

        while(parsingCal && (fgets(lineBuf, 120, fStream) != NULL)) {
			pLine = lineBuf;
//			pLine = strtok(pLine, delimiters);
		    // while(isspace(*pLine)) pLine++;
			if(pLine == NULL) continue;

			if(strnicmp("@cal_section_end", pLine, strlen("@cal_section_end")) == 0)  {
		        parsingCal = FALSE;
			}

			if (pLine[0] == '#') {
				continue ;
			}
			else if((strnicmp("TARGET_POWER_FILENAME", pLine, strlen("TARGET_POWER_FILENAME")) == 0) &&
					((pLine[strlen("TARGET_POWER_FILENAME")] == ' ') || 
					 (pLine[strlen("TARGET_POWER_FILENAME")] == '\t') ) ){
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%s", CalSetup.tgtPwrFilename)) {
					uiPrintf("Unable to read the TARGET_POWER_FILENAME from %s\n", eep_file);
				} 
			}
			else if(strnicmp("EEPROM_MAP_TYPE", pLine, strlen("EEPROM_MAP_TYPE")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &CalSetup.eeprom_map)) {
					uiPrintf("Unable to read the EEPROM_MAP_TYPE from %s\n", eep_file);
				}
			}
			else if(strnicmp("SUBSYSTEM_ID", pLine, strlen("SUBSYSTEM_ID")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				CalSetup.subsystemID = (A_UINT16) strtoul(pLine, NULL, 0);

			}
			else if(strnicmp("PRODUCT_ID", pLine, strlen("PRODUCT_ID")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				CalSetup.productID = (A_UINT16) strtoul(pLine, NULL, 0);

			}
			else if(strnicmp("EEPROM_SIZE", pLine, strlen("EEPROM_SIZE")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				CalSetup.eepromLength = (A_UINT32) strtoul(pLine, NULL, 0);
				
				if(CalSetup.eepromLength == 0)
				{
					userEepromSize = 0x400;					
				}
				else
					userEepromSize = (CalSetup.eepromLength*1024)/16;

			}
			else if(strnicmp("NUM_ETHERNET_PORTS", pLine, strlen("NUM_ETHERNET_PORTS")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &CalSetup.numEthernetPorts)) {
					uiPrintf("Unable to read the NUM_ETHERNET_PORTS from %s\n", eep_file);
				}
			}
			else if(strnicmp("START_ETHERNET_PORT", pLine, strlen("START_ETHERNET_PORT")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &CalSetup.startEthernetPort)) {
					uiPrintf("Unable to read the START_ETHERNET_PORT from %s\n", eep_file);
				}
			}
		   else if(strnicmp("TURBO_DISABLE", pLine, strlen("TURBO_DISABLE")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the TURBO_DISABLE from %s\n", eep_file);
				}
				else {
					CalSetup.turboDisable = (testVal) ? TRUE : FALSE;
				}
			}
		   else if(strnicmp("11g_TURBO_DISABLE", pLine, strlen("11g_TURBO_DISABLE")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the 11g_TURBO_DISABLE from %s\n", eep_file);
				}
				else {
					CalSetup.turboDisable_11g = (testVal) ? TRUE : FALSE;
				}
			}
			
		   else if(strnicmp("UART_ENABLE", pLine, strlen("UART_ENABLE")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the UART_ENABLE from %s\n", eep_file);
				}
				else {
					CalSetup.uartEnable = (testVal) ? TRUE : FALSE;
				}
			}
		   else if(strnicmp("DISABLE_DYNAMIC_EAR", pLine, strlen("DISABLE_DYNAMIC_EAR")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the DISABLE_DYNAMIC_EAR from %s\n", eep_file);
				}
				else {
					//reverse the flag entered by user (disable) to internal enable flag
					CalSetup.enableDynamicEAR = (testVal) ? FALSE : TRUE;
				}
			}
		   else if(strnicmp("XR_DISABLE", pLine, strlen("XR_DISABLE")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the XR_DISABLE from %s\n", eep_file);
				}
				else {
					CalSetup.xrDisable = (testVal) ? TRUE : FALSE;
				}
			}
		   else if(strnicmp("COMPRESSION_DISABLE", pLine, strlen("COMPRESSION_DISABLE")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the COMPRESSION_DISABLE from %s\n", eep_file);
				}
				else {
					CalSetup.compressionDisable = (testVal) ? TRUE : FALSE;
				}
			}
		   else if(strnicmp("AES_DISABLE", pLine, strlen("AES_DISABLE")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the AES_DISABLE from %s\n", eep_file);
				}
				else {
					CalSetup.aesDisable = (testVal) ? TRUE : FALSE;
				}
			}
		   else if(strnicmp("ENABLE_HEAVY_CLIP", pLine, strlen("ENABLE_HEAVY_CLIP")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the ENABLE_HEAVY_CLIP from %s\n", eep_file);
				}
				else {
					CalSetup.enableHeavyClip = (testVal) ? TRUE : FALSE;
				}
			}
		   else if(strnicmp("FAST_FRAME_DISABLE", pLine, strlen("FAST_FRAME_DISABLE")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the FAST_FRAME_DISABLE from %s\n", eep_file);
				}
				else {
					CalSetup.fastFrameDisable = (testVal) ? TRUE : FALSE;
				}
			}
		   else if(strnicmp("BURSTING_DISABLE", pLine, strlen("BURSTING_DISABLE")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the BURSTING_DISABLE from %s\n", eep_file);
				}
				else {
					CalSetup.burstingDisable = (testVal) ? TRUE : FALSE;
				}
			}
		   else if(strnicmp("MAX_NUM_QCU", pLine, strlen("MAX_NUM_QCU")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &CalSetup.maxNumQCU)) {
					uiPrintf("Unable to read the MAX_NUM_QCU from %s\n", eep_file);
				}
			}
		   else if(strnicmp("KEY_CACHE_SIZE", pLine, strlen("KEY_CACHE_SIZE")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &CalSetup.keyCacheSize)) {
					uiPrintf("Unable to read the KEY_CACHE_SIZE from %s\n", eep_file);
				}
			}
		   else if(strnicmp("HB63_SPECIFIC_PATCH", pLine, strlen("HB63_SPECIFIC_PATCH")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &CalSetup.registerSpecificPatch)) {
					uiPrintf("Unable to read the KEY_CACHE_SIZE from %s\n", eep_file);
				}
			}

		   //import dynamic 5G/2.4G spur from .eep file
		   else if ((strnicmp("SPUR_FREQUENCY_5G", pLine, strlen("SPUR_FREQUENCY_5G")) == 0) &&
			     ((pLine[strlen("SPUR_FREQUENCY_5G")] == ' ') || 
				   (pLine[strlen("SPUR_FREQUENCY_5G")] == '\t')) )
		   	{
				pLine = strchr(pLine, '=');
				pLine++;
				pLine = strtok(pLine,";#");
				
				ii = 0;
				pLine = strtok(pLine, " ,");
				while  ((pLine != NULL) && (pLine[0] != ';'))
				{
					
					if(!sscanf(pLine, "%d", &testVal )){						
						uiPrintf("Unable to read next 5G spur frequency from -->%s<--\n", pLine);
					} else
					{
						if (ii < (EEPROM_SPUR_11A_SIZE-1))
						{
							CalSetup.spur_frequency_5g[ii++] = testVal;
						}
						else
						{
							break;
						}
					}
					pLine = strtok(NULL, " ,");
				}

				if (ii < 1) {
					uiPrintf("Unable to read the SPUR_FREQUENCY_5G from %s\n", calsetupFileName);
				}
			}
		   else if ((strnicmp("SPUR_FREQUENCY_2P4G", pLine, strlen("SPUR_FREQUENCY_2P4G")) == 0) &&
			     ((pLine[strlen("SPUR_FREQUENCY_2P4G")] == ' ') || 
				   (pLine[strlen("SPUR_FREQUENCY_2P4G")] == '\t')) )
		   	{
				pLine = strchr(pLine, '=');
				pLine++;
				pLine = strtok(pLine,";#");
				
				ii = 0;
				pLine = strtok(pLine, " ,");
				while  ((pLine != NULL) && (pLine[0] != ';'))
				{
					
					if(!sscanf(pLine, "%d", &testVal )){						
						uiPrintf("Unable to read next 2.4G spur frequency from -->%s<--\n", pLine);
					} else
					{
						if (ii < (EEPROM_SPUR_11G_SIZE-1))
						{
							CalSetup.spur_frequency_2p4g[ii++] = testVal;
						}
						else
						{
							break;
						}
					}
					pLine = strtok(NULL, " ,");
				}

				if (ii < 1) {
					uiPrintf("Unable to read the SPUR_FREQUENCY_2P4G from %s\n", calsetupFileName);
				}
			}

		   else if(strnicmp("ENABLE_FCC_MIDBAND", pLine, strlen("ENABLE_FCC_MIDBAND")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the ENABLE_FCC_MIDBAND from %s\n", eep_file);
				}
				else {
					CalSetup.enableFCCMid = (testVal) ? TRUE : FALSE;
				}
			}

		   else if(strnicmp("ENABLE_JAPAN_EVEN_CHANNELS_UNI1_BAND", pLine, strlen("ENABLE_JAPAN_EVEN_CHANNELS_UNI1_BAND")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the ENABLE_JAPAN_EVEN_CHANNELS_UNI1_BAND from %s\n", eep_file);
				}
				else {
					CalSetup.enableJapanEvenU1 = (testVal) ? TRUE : FALSE;
				}
			}

		   else if(strnicmp("ENABLE_JAPAN_UNI2_BAND", pLine, strlen("ENABLE_JAPAN_UNI2_BAND")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the ENABLE_JAPAN_UNI2_BAND from %s\n", eep_file);
				}
				else {
					CalSetup.enableJapenU2 = (testVal) ? TRUE : FALSE;
				}
			}

		   else if(strnicmp("ENABLE_JAPAN_MIDBAND", pLine, strlen("ENABLE_JAPAN_MIDBAND")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the ENABLE_JAPAN_MIDBAND from %s\n", eep_file);
				}
				else {
					CalSetup.enableJapnMid = (testVal) ? TRUE : FALSE;
				}
			}

		   else if(strnicmp("ENABLE_JAPAN_ODD_CHANNELS_UNI1_BAND", pLine, strlen("ENABLE_JAPAN_ODD_CHANNELS_UNI1_BAND")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the ENABLE_JAPAN_ODD_CHANNELS_UNI1_BAND from %s\n", eep_file);
				}
				else {
					CalSetup.enableJapanOddU1 = (testVal) ? TRUE : FALSE;
				}
			}

		   else if(strnicmp("ENABLE_JAPAN_MODE_11A_NEW", pLine, strlen("ENABLE_JAPAN_MODE_11A_NEW")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the ENABLE_JAPAN_MODE_11A_NEW from %s\n", eep_file);
				}
				else {
					CalSetup.enableJapanMode11aNew = (testVal) ? TRUE : FALSE;
				}
			}

		   else if(strnicmp("RF_SILENT", pLine, strlen("RF_SILENT")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the RF_SILENT from %s\n", eep_file);
				}
				else {
					CalSetup.RFSilent = (testVal) ? TRUE : FALSE;
				}
			}
		   else if(strnicmp("LOs_BYPASS", pLine, strlen("LOs_BYPASS")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the LOs_BYPASS from %s\n", eep_file);
				}
				else {
					CalSetup.LOsBypass = (testVal) ? TRUE : FALSE;
				}
			}
		   else if(strnicmp("SERDES_PROGRAMMING", pLine, strlen("SERDES_PROGRAMMING")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the SERDES_PROGRAMMING from %s\n", eep_file);
                } else if(testVal > 2) {
					uiPrintf("Unexpected SERDES_PROGRAMMING value from %s, must be <= 2\n", eep_file);
                }
				else {
					CalSetup.enableSerdesProgramming = testVal;
				}
			}

		   else if(strnicmp("ASPM_LATENCY", pLine, strlen("ASPM_LATENCY")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the ASPM_LATENCY from %s\n", eep_file);
				}
				else {
					CalSetup.enableASPMLatency = (testVal) ? TRUE : FALSE;
				}
			}

		   else if(strnicmp("ASPM_SUPPORT", pLine, strlen("ASPM_SUPPORT")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the ASPM_SUPPORT from %s\n", eep_file);
				}
				else {
					CalSetup.aspmSupport = testVal;
				}
			}

		   else if((strnicmp("ENABLE_WA", pLine, strlen("ENABLE_WA")) == 0) &&
				((pLine[strlen("ENABLE_WA")] == ' ') || 
				 (pLine[strlen("ENABLE_WA")] == '\t') ) ){
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the ENABLE_WA from %s\n", eep_file);
				}
				else {
					CalSetup.enableWA = (testVal) ? TRUE : FALSE;
				}
			}

		    else if((strnicmp("DEVICE_TYPE", pLine, strlen("DEVICE_TYPE")) == 0) &&
				((pLine[strlen("DEVICE_TYPE")] == ' ') || 
				 (pLine[strlen("DEVICE_TYPE")] == '\t') ) ){

				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				pLine = strtok(pLine," ;#");
				if(!sscanf(pLine, "%d", &CalSetup.deviceType)) {
					uiPrintf("Unable to read the DEVICE_TYPE from %s\n", eep_file);
				}			
			}
			else if((strnicmp("STA_CARD_ON_AP", pLine, strlen("STA_CARD_ON_AP")) == 0) &&
				((pLine[strlen("STA_CARD_ON_AP")] == ' ') || 
				 (pLine[strlen("STA_CARD_ON_AP")] == '\t') ) ){

				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				pLine = strtok(pLine," ;#");
			    if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the STA_CARD_ON_AP from %s\n", eep_file);
				}			
				else {
					CalSetup.staCardOnAP = (testVal) ? TRUE : FALSE;
				}
			}			        
			
			else if((strnicmp("ENABLE_32KHZ", pLine, strlen("ENABLE_32KHZ")) == 0) &&
					((pLine[strlen("ENABLE_32KHZ")] == ' ') || 
					 (pLine[strlen("ENABLE_32KHZ")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.Enable_32khz)) {
					uiPrintf("Unable to read the ENABLE_32KHZ from %s\n", eep_file);
					}			
			}			        
			else if((strnicmp("MINIMUM_POWER_11A", pLine, strlen("MINIMUM_POWER_11A")) == 0) &&
					((pLine[strlen("MINIMUM_POWER_11A")] == ' ') || 
					 (pLine[strlen("MINIMUM_POWER_11A")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.minimumPower[MODE_11a])) {
					uiPrintf("Unable to read the MINIMUM_POWER_11A from %s\n", eep_file);
					}			
			}			        
			else if((strnicmp("MINIMUM_POWER_11G", pLine, strlen("MINIMUM_POWER_11G")) == 0) &&
					((pLine[strlen("MINIMUM_POWER_11G")] == ' ') || 
					 (pLine[strlen("MINIMUM_POWER_11G")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.minimumPower[MODE_11g])) {
					uiPrintf("Unable to read the MINIMUM_POWER_11G from %s\n", eep_file);
					}			
			}			        
			else if((strnicmp("MINIMUM_POWER_11B", pLine, strlen("MINIMUM_POWER_11B")) == 0) &&
					((pLine[strlen("MINIMUM_POWER_11B")] == ' ') || 
					 (pLine[strlen("MINIMUM_POWER_11B")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.minimumPower[MODE_11b])) {
					uiPrintf("Unable to read the MINIMUM_POWER_11B from %s\n", eep_file);
					}			
			}			        
			else if((strnicmp("ENABLE_WAKE_ON_WLAN", pLine, strlen("ENABLE_WAKE_ON_WLAN")) == 0) &&
					((pLine[strlen("ENABLE_WAKE_ON_WLAN")] == ' ') || 
					 (pLine[strlen("ENABLE_WAKE_ON_WLAN")] == '\t') ) ){
					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
//					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the ENABLE_WAKE_ON_WLAN from %s\n", eep_file);
					}			
					else {
						CalSetup.Enable_WOW = (testVal) ? TRUE : FALSE;
					}
			}			        
		    else if(strnicmp("TURBO_MAXPOWER_5G", pLine, strlen("TURBO_MAXPOWER_5G")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%lf", &CalSetup.TurboMaxPower_5G)) {
					uiPrintf("Unable to read the TURBO_MAXPOWER_5G from %s\n", eep_file);
				} 	
			}
			else if(strnicmp("TURBO_MAXPOWER_2p5G", pLine, strlen("TURBO_MAXPOWER_2p5G")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%lf", &CalSetup.TurboMaxPower_11g)) {
					uiPrintf("Unable to read the TURBO_MAXPOWER_2p5G from %s\n", eep_file);
				} 	
			}
			else if(strnicmp("CCK_OFDM_DELTA", pLine, strlen("CCK_OFDM_DELTA")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%lf", &CalSetup.cck_ofdm_delta)) {
					uiPrintf("Unable to read the CCK_OFDM_DELTA from %s\n", eep_file);
				} 	
			}
			else if(strnicmp("CH14_FILTER_CCK_DELTA", pLine, strlen("CH14_FILTER_CCK_DELTA")) == 0) {
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				if(!sscanf(pLine, "%lf", &CalSetup.ch14_filter_cck_delta)) {
					uiPrintf("Unable to read the CH14_FILTER_CCK_DELTA from %s\n", eep_file);
				} 	
			}
			else if((strnicmp("A_MODE", pLine, strlen("A_MODE")) == 0) &&
					((pLine[strlen("A_MODE")] == ' ') || 
					 (pLine[strlen("A_MODE")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.Amode)) {
					uiPrintf("Unable to read the A_MODE from %s\n", eep_file);
					}
			}			        
			else if((strnicmp("B_MODE", pLine, strlen("B_MODE")) == 0) &&
					((pLine[strlen("B_MODE")] == ' ') || 
					 (pLine[strlen("B_MODE")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.Bmode)) {
					uiPrintf("Unable to read the B_MODE from %s\n", eep_file);
					}
			}	
			else if((strnicmp("G_MODE", pLine, strlen("G_MODE")) == 0) &&
					((pLine[strlen("G_MODE")] == ' ') || 
					 (pLine[strlen("G_MODE")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.Gmode)) {
					uiPrintf("Unable to read the G_MODE from %s\n", eep_file);
					}
			}	
			else if((strnicmp("ANTENNA_GAIN_5G", pLine, strlen("ANTENNA_GAIN_5G")) == 0) &&
					((pLine[strlen("ANTENNA_GAIN_5G")] == ' ') || 
					(pLine[strlen("ANTENNA_GAIN_5G")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.antennaGain5G)) {
					uiPrintf("Unable to read the ANTENNA_GAIN_5G from %s\n", eep_file);
					} 
			}			        
			else if((strnicmp("ANTENNA_GAIN_2p5G", pLine, strlen("ANTENNA_GAIN_2p5G")) == 0) &&
					((pLine[strlen("ANTENNA_GAIN_2p5G")] == ' ') || 
					(pLine[strlen("ANTENNA_GAIN_2p5G")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.antennaGain2p5G)) {
					uiPrintf("Unable to read the ANTENNA_GAIN_2p5G from %s\n", eep_file);
					} 
			}			        
			else if((strnicmp("XLNA_GAIN", pLine, strlen("XLNA_GAIN")) == 0) &&
					((pLine[strlen("XLNA_GAIN")] == ' ') || 
					 (pLine[strlen("XLNA_GAIN")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.xlnaGain)) {
					uiPrintf("Unable to read the XLNA_GAIN from %s\n", eep_file);
					}
			}			        
			else if((strnicmp("NOISE_FLOOR_THRESHOLD", pLine, strlen("NOISE_FLOOR_THRESHOLD")) == 0) &&
					((pLine[strlen("NOISE_FLOOR_THRESHOLD")] == ' ') || 
					(pLine[strlen("NOISE_FLOOR_THRESHOLD")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					
					if(!sscanf(pLine, "%d", &CalSetup.noisefloor_thresh)) {
					uiPrintf("Unable to read the NOISE_FLOOR_THRESHOLD from %s\n", eep_file);
					}
					
			}			        
			else if((strnicmp("11b_XLNA_GAIN", pLine, strlen("11b_XLNA_GAIN")) == 0) &&
					((pLine[strlen("11b_XLNA_GAIN")] == ' ') || 
					 (pLine[strlen("11b_XLNA_GAIN")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.xlnaGain_2p4[MODE_11b])) {
					uiPrintf("Unable to read the 11b_XLNA_GAIN from %s\n", eep_file);
					}
			}			        
			else if((strnicmp("11b_NOISE_FLOOR_THRESHOLD", pLine, strlen("11b_NOISE_FLOOR_THRESHOLD")) == 0) &&
					((pLine[strlen("11b_NOISE_FLOOR_THRESHOLD")] == ' ') || 
					(pLine[strlen("11b_NOISE_FLOOR_THRESHOLD")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.noisefloor_thresh_2p4[MODE_11b])) {
					uiPrintf("Unable to read the 11b_NOISE_FLOOR_THRESHOLD from %s\n", eep_file);
					}
			}			        
			else if((strnicmp("11g_XLNA_GAIN", pLine, strlen("11g_XLNA_GAIN")) == 0) &&
					((pLine[strlen("11g_XLNA_GAIN")] == ' ') || 
					 (pLine[strlen("11g_XLNA_GAIN")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.xlnaGain_2p4[MODE_11g])) {
					uiPrintf("Unable to read the 11b_XLNA_GAIN from %s\n", eep_file);
					}
			}			        
			else if((strnicmp("11g_NOISE_FLOOR_THRESHOLD", pLine, strlen("11g_NOISE_FLOOR_THRESHOLD")) == 0) &&
					((pLine[strlen("11g_NOISE_FLOOR_THRESHOLD")] == ' ') || 
					(pLine[strlen("11g_NOISE_FLOOR_THRESHOLD")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.noisefloor_thresh_2p4[MODE_11g])) {
					uiPrintf("Unable to read the 11b_NOISE_FLOOR_THRESHOLD from %s\n", eep_file);
					}
			}			        
			else if((strnicmp("11a_FALSE_DETECT_BACKOFF", pLine, strlen("11a_FALSE_DETECT_BACKOFF")) == 0) &&
					((pLine[strlen("11a_FALSE_DETECT_BACKOFF")] == ' ') || 
					 (pLine[strlen("11a_FALSE_DETECT_BACKOFF")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.falseDetectBackoff[MODE_11a])) {
					uiPrintf("Unable to read the 11a_FALSE_DETECT_BACKOFF from %s\n", eep_file);
					}
			}			        
			else if((strnicmp("11b_FALSE_DETECT_BACKOFF", pLine, strlen("11b_FALSE_DETECT_BACKOFF")) == 0) &&
					((pLine[strlen("11b_FALSE_DETECT_BACKOFF")] == ' ') || 
					 (pLine[strlen("11b_FALSE_DETECT_BACKOFF")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.falseDetectBackoff[MODE_11b])) {
					uiPrintf("Unable to read the 11b_FALSE_DETECT_BACKOFF from %s\n", eep_file);
					}
			}			        
			else if((strnicmp("11g_FALSE_DETECT_BACKOFF", pLine, strlen("11g_FALSE_DETECT_BACKOFF")) == 0) &&
					((pLine[strlen("11g_FALSE_DETECT_BACKOFF")] == ' ') || 
					 (pLine[strlen("11g_FALSE_DETECT_BACKOFF")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.falseDetectBackoff[MODE_11g])) {
					uiPrintf("Unable to read the 11g_FALSE_DETECT_BACKOFF from %s\n", eep_file);
					}
			}			        
			else if((strnicmp("MODE_MASK_FOR_RADIO_0", pLine, strlen("MODE_MASK_FOR_RADIO_0")) == 0) &&
					((pLine[strlen("MODE_MASK_FOR_RADIO_0")] == ' ') || 
					 (pLine[strlen("MODE_MASK_FOR_RADIO_0")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.modeMaskForRadio[0])) {
						uiPrintf("Unable to read the MODE_MASK_FOR_RADIO_0 from %s\n", eep_file);
					} 
			}			        
			else if((strnicmp("MODE_MASK_FOR_RADIO_1", pLine, strlen("MODE_MASK_FOR_RADIO_1")) == 0) &&
					((pLine[strlen("MODE_MASK_FOR_RADIO_1")] == ' ') || 
					 (pLine[strlen("MODE_MASK_FOR_RADIO_1")] == '\t') ) ){

					pLine = strchr(pLine, '=');
					pLine = strtok(pLine, delimiters);
					pLine = strtok(pLine," ;#");
					if(!sscanf(pLine, "%d", &CalSetup.modeMaskForRadio[1])) {
						uiPrintf("Unable to read the MODE_MASK_FOR_RADIO_1 from %s\n", eep_file);
					} 
			}			        
        else if(strnicmp("MAX_PCDAC_11A", pLine, strlen("MAX_PCDAC_11A")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &CalSetup.max_pcdac_11a)) {
                uiPrintf("Unable to read MAX_PCDAC_11A from %s\n", eep_file);
				return;
            }
        }
        else if(strnicmp("MAX_PCDAC_11B", pLine, strlen("MAX_PCDAC_11B")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &CalSetup.max_pcdac_11b)) {
                uiPrintf("Unable to read MAX_PCDAC_11B from %s\n", eep_file);
				return;
            }
        }
        else if(strnicmp("MAX_PCDAC_11G", pLine, strlen("MAX_PCDAC_11G")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &CalSetup.max_pcdac_11g)) {
                uiPrintf("Unable to read MAX_PCDAC_11G from %s\n", eep_file);
				return;
            }
        }
        else if(strnicmp("ATTEMPT_POWER_11A", pLine, strlen("ATTEMPT_POWER_11A")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &CalSetup.attempt_pcdac_11a)) {
                uiPrintf("Unable to read ATTEMPT_POWER_11A from %s\n", eep_file);
				return;
            }
        }
        else if(strnicmp("ATTEMPT_POWER_11B", pLine, strlen("ATTEMPT_POWER_11B")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &CalSetup.attempt_pcdac_11b)) {
                uiPrintf("Unable to read ATTEMPT_POWER_11B from %s\n", eep_file);
				return;
            }
        }
        else if(strnicmp("ATTEMPT_POWER_11G", pLine, strlen("ATTEMPT_POWER_11G")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, delimiters ); //get past any white space etc

            if(!sscanf(pLine, "%d", &CalSetup.attempt_pcdac_11g)) {
                uiPrintf("Unable to read ATTEMPT_POWER_11G from %s\n", eep_file);
				return;
            }
        } 
		} // done parsing cal section
	} // end of file					
	if(CalSetup.eepromLength == 0)
		userEepromSize =0x400;
}

A_UINT32 read_gainf_with_probe_packet(A_UINT32 devNum, A_UINT32 power) {

	A_UINT32 rddata, retVal;

	art_writeField(devNum, "bb_probe_powertx", power);
	art_writeField(devNum, "bb_probe_next_tx", 1);
	//Sleep(1);
	usleep(1000);
	rddata = art_regRead(devNum, 0x9930);
	retVal = (rddata >> 25) ;

	return(retVal);
}

A_UINT32 read_gainf_twice(A_UINT32 devNum) 
{
	A_UINT32 gain1, gain2;
	// index of pcdac table to read gainF for. set to 0 for all contTX
	gain1 = read_gainf_with_probe_packet(devNum, 0);
	if (abs(gain1 - global_prev_gainf) > 10) {
		// index of pcdac table to read gainF for. set to 0 for all contTX
		gain2 = read_gainf_with_probe_packet(devNum, 0);
	} else {
		gain2 = gain1;
	}
	global_prev_gainf = (A_UINT16) gain1;
	return( (gain1 >= gain2) ? gain1 : gain2);
}
	

A_UINT32 dump_a2_pc_out(A_UINT32 devNum)
{

    A_UINT32 a2_data=0;
    A_UINT32 tmp;
	A_UINT32 OS_1   = 0;
    A_UINT32 OS_0   = 1;
    A_UINT32 M_2    = 0;
    A_UINT32 M_1    = 1;
    A_UINT32 M_0    = 1;
    A_UINT32 DREG_1 = 0;
    A_UINT32 DREG_0 = 0;
    A_UINT32 DA_2   = 0;
    A_UINT32 DA_1   = 0;
    A_UINT32 DA_0   = 0;
    A_UINT32 NUM_SHIFTS = 7; //used to be 6 in crete-fez
    A_UINT32 indata, outdata, bit, pos;


    tmp = art_regRead(devNum, 0x9808) | (0x1<<27);

    art_regWrite(devNum, 0x9808, tmp);

	// now dump
    tmp =  ((M_2 << 10) | (M_1 << 11) | (M_0 << 12) | (OS_1 << 0) | (OS_0 << 1) | (DREG_1 << 16)
		   | (DREG_0 << 17) | (DA_2 << 18) | (DA_1 << 19) | (DA_0 << 20) | (0x5 << 2));

    art_regWrite(devNum, 0x9800+(0x34<<2), tmp);

	// shift out 32 bits
    for (tmp=0;tmp<NUM_SHIFTS;tmp++)
	{
       art_regWrite(devNum, 0x9800+(0x20<<2), 0x10000);
    }

    a2_data = art_regRead(devNum, 0x9800+(256<<2));

	// reverse 7
	indata = ((a2_data>>25)&0x7f);  
	outdata=bit=0;
	for (pos=0; pos<NUM_SHIFTS; pos++) {
		bit = (indata >> pos) & 0x1;
		outdata = (outdata << 1) | bit;
	}
    a2_data = outdata;

    // clear out register 5
    art_regWrite(devNum, 0x9800+(0x34<<2), 0x14);

	// d2_enable_agc_to_a2();
    tmp = art_regRead(devNum, 0x9808) & ~(0x1<<27);
    art_regWrite(devNum, 0x9808, tmp);

    return(a2_data);

}

void parseSetup(A_UINT32 devNum) 
{
  //printf("\n Start parsesetup\n");
	load_calsetup_vals() ;
	load_eep_vals(devNum);
	if(userEepromSize == 0x400)
	{
		checkSumLength = eepromSize = 0x400;
		//uiPrintf("In parseSetup() checkSumLength = %x eepromSize = %x \n",checkSumLength,eepromSize);
	}
	else
	{
		eepromSize = userEepromSize;
		//uiPrintf("In ParseSetup() eepromSize = %x \n",eepromSize);
	}
	//printf("\n  End parsesetup\n");
}

void load_calsetup_vals(void) 
{
    FILE *fStream;
    char lineBuf[122], *pLine;
    A_UINT32 testVal;
	A_INT32  signedTempVal;
	A_UINT16 ii;
	char delimiters[] = " \t\n\r;=";

	uiPrintf("\nReading in Calibration Setup from %s\n", calsetupFileName);
    if( (fStream = fopen( (const char*) calsetupFileName, "r")) == NULL ) {
		uiPrintf("\n\n**************************************************\n");
        uiPrintf("Failed to open calsetup file : %s \n", calsetupFileName);
		uiPrintf("Make Sure COMPUTE_CALSETUP_FILE is set to correct \n");
		uiPrintf("value in artsetup.txt\n");
		uiPrintf("**************************************************\n\n");
        return;
    }

    while(fgets(lineBuf, 120, fStream) != NULL) {
        pLine = lineBuf;

        while(isspace(*pLine)) pLine++;

		if(strnicmp("#BEGIN_11a_TEST_CHANNEL_MATRIX", pLine, strlen("#BEGIN_11a_TEST_CHANNEL_MATRIX")) == 0) {
			parseTestChannels(fStream, pLine, MODE_11a);
			continue;
        }

		if(strnicmp("#BEGIN_11b_TEST_CHANNEL_MATRIX", pLine, strlen("#BEGIN_11b_TEST_CHANNEL_MATRIX")) == 0) {
			parseTestChannels(fStream, pLine, MODE_11b);
			continue;
        }

		if(strnicmp("#BEGIN_11g_TEST_CHANNEL_MATRIX", pLine, strlen("#BEGIN_11g_TEST_CHANNEL_MATRIX")) == 0) {
			parseTestChannels(fStream, pLine, MODE_11g);
			continue;
        }

        if(*pLine == '#') {
            continue;
        }
		else if(strnicmp("SUBSYSTEM_ID", pLine, strlen("SUBSYSTEM_ID")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
	    CalSetup.subsystemID = (A_UINT16) strtoul(pLine, NULL, 0);
        }
        else if(strnicmp("USE_INSTRUMENTS", pLine, strlen("USE_INSTRUMENTS")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the USE_INSTRUMENTS from %s\n", calsetupFileName);
            }
            else {
                CalSetup.useInstruments = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("COUNTRY_OR_DOMAIN_FLAG", pLine, strlen("COUNTRY_OR_DOMAIN_FLAG")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.countryOrDomain)) {
                uiPrintf("Unable to read the COUNTRY_OR_DOMAIN_FLAG flag from %s\n", calsetupFileName);
            } 
        }
        else if(strnicmp("WORLD_WIDE_ROAMING_FLAG", pLine, strlen("WORLD_WIDE_ROAMING_FLAG")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.worldWideRoaming)) {
                uiPrintf("Unable to read the WORLD_WIDE_ROAMING_FLAG flag from %s\n", calsetupFileName);
            } 
        }
        else if(strnicmp("COUNTRY_OR_DOMAIN_CODE", pLine, strlen("COUNTRY_OR_DOMAIN_CODE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
			CalSetup.countryOrDomainCode = (A_UINT16) strtoul(pLine, NULL, 0);  
       }
	   else if(strnicmp("READ_FROM_FILE", pLine, strlen("READ_FROM_FILE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            if(!sscanf(pLine, "%d", &testVal)) {
                printf("Unable to read the READ_FROM_FILE from %s\n", calsetupFileName);
            }
            else {
                CalSetup.readFromFile = (testVal) ? TRUE : FALSE;
            }
        }
	   else if(strnicmp("RAW_DATA_FILENAME", pLine, strlen("RAW_DATA_FILENAME")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.rawDataFilename)) {
                printf("Unable to read the RAW_DATA_FILENAME from %s\n", calsetupFileName);
            }
            else {
                //printf("Will read data from file %s\n", CalSetup.rawDataFilename);
            }
        }

	   //read the info for ftpdownload
	   else if(strnicmp("DO_FTP_DOWNLOAD", pLine, strlen("DO_FTP_DOWNLOAD")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            if(!sscanf(pLine, "%d", &testVal)) {
                printf("Unable to read the DO_FTP_DOWNLOAD from %s\n", calsetupFileName);
            }
            else {
                CalSetup.ftpdownloadFileInfo.downloadRequired = (testVal) ? TRUE : FALSE;
            }
        }
	   else if(strnicmp("FTP_HOSTNAME", pLine, strlen("FTP_HOSTNAME")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.ftpdownloadFileInfo.hostname)) {
                printf("Unable to read the FTP_HOSTNAME from %s\n", calsetupFileName);
            }
        }
	   else if(strnicmp("FTP_USERNAME", pLine, strlen("FTP_USERNAME")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.ftpdownloadFileInfo.username)) {
                printf("Unable to read the FTP_USERNAME from %s\n", calsetupFileName);
            }
        }
	   else if(strnicmp("FTP_PASSWORD", pLine, strlen("FTP_PASSWORD")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.ftpdownloadFileInfo.password)) {
                printf("Unable to read the FTP_PASSWORD from %s\n", calsetupFileName);
            }
        }
	   else if(strnicmp("FTP_REMOTE_FILE", pLine, strlen("FTP_REMOTE_FILE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.ftpdownloadFileInfo.remotefile)) {
                printf("Unable to read the FTP_REMOTE_FILE from %s\n", calsetupFileName);
            }
        }
	   else if(strnicmp("FTP_LOCAL_FILE", pLine, strlen("FTP_LOCAL_FILE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.ftpdownloadFileInfo.localfile)) {
                printf("Unable to read the FTP_LOCAL_FILE from %s\n", calsetupFileName);
            }
        }

	   else if(strnicmp("11b_READ_FROM_FILE", pLine, strlen("11b_READ_FROM_FILE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            if(!sscanf(pLine, "%d", &testVal)) {
                printf("Unable to read the 11b_READ_FROM_FILE from %s\n", calsetupFileName);
            }
            else {
                CalSetup.readFromFile_2p4[MODE_11b] = (testVal) ? TRUE : FALSE;
            }
        }
	   else if(strnicmp("11b_RAW_DATA_FILENAME", pLine, strlen("11b_RAW_DATA_FILENAME")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.rawDataFilename_2p4[MODE_11b])) {
                printf("Unable to read the 11b_RAW_DATA_FILENAME from %s\n", calsetupFileName);
            }
            else {
                //printf("Will read data from file %s\n", CalSetup.rawDataFilename);
            }
        }
	   else if(strnicmp("11g_READ_FROM_FILE", pLine, strlen("11g_READ_FROM_FILE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            if(!sscanf(pLine, "%d", &testVal)) {
                printf("Unable to read the 11g_READ_FROM_FILE from %s\n", calsetupFileName);
            }
            else {
                CalSetup.readFromFile_2p4[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
	   else if(strnicmp("11g_RAW_DATA_FILENAME", pLine, strlen("11g_RAW_DATA_FILENAME")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.rawDataFilename_2p4[MODE_11g])) {
                printf("Unable to read the 11g_RAW_DATA_FILENAME from %s\n", calsetupFileName);
            }
            else {
                //printf("Will read data from file %s\n", CalSetup.rawDataFilename);
            }
        }
        else if(strnicmp("TEST_32KHZ_SLEEP_CRYSTAL", pLine, strlen("TEST_32KHZ_SLEEP_CRYSTAL")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_32KHZ_SLEEP_CRYSTAL from %s\n", calsetupFileName);
            }
            else {
                CalSetup.test32KHzSleepCrystal = (testVal) ? TRUE : FALSE;
            }
        }
		else if(strnicmp("TEST_TEMP_MARGIN", pLine, strlen("TEST_TEMP_MARGIN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_TEMP_MARGIN from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTempMargin[MODE_11a] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11b_TEST_TEMP_MARGIN", pLine, strlen("11b_TEST_TEMP_MARGIN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11b_TEST_TEMP_MARGIN from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTempMargin[MODE_11b] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11g_TEST_TEMP_MARGIN", pLine, strlen("11g_TEST_TEMP_MARGIN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_TEMP_MARGIN from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTempMargin[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11g_TEST_DATA_INTEGRITY", pLine, strlen("11g_TEST_DATA_INTEGRITY")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_DATA_INTEGRITY from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testDataIntegrity[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11b_TEST_DATA_INTEGRITY", pLine, strlen("11b_TEST_DATA_INTEGRITY")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11b_TEST_DATA_INTEGRITY from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testDataIntegrity[MODE_11b] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("TEST_DATA_INTEGRITY", pLine, strlen("TEST_DATA_INTEGRITY")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_DATA_INTEGRITY from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testDataIntegrity[MODE_11a] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11g_TEST_THROUGHPUT", pLine, strlen("11g_TEST_THROUGHPUT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_THROUGHPUT from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testThroughput[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11b_TEST_THROUGHPUT", pLine, strlen("11b_TEST_THROUGHPUT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11b_TEST_THROUGHPUT from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testThroughput[MODE_11b] = (testVal) ? TRUE : FALSE;
            }
        }
	   else if(strnicmp("DO_CCK_IN_11G", pLine, strlen("DO_CCK_IN_11G")) == 0) {
           pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the DO_CCK_IN_11G from %s\n", calsetupFileName);
            }
            else {
				CalSetup.doCCKin11g = (testVal) ? TRUE : FALSE;
            }
        }
	   else if(strnicmp("FIX_ZERO_POWER", pLine, strlen("FIX_ZERO_POWER")) == 0) {
           pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the FIX_ZERO_POWER from %s\n", calsetupFileName);
            }
            else {
				CalSetup.fixZeroPowerDuringCal = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("TARGET_POWER_ZERO_TOLERANCE_UPPER", pLine, strlen("TARGET_POWER_ZERO_TOLERANCE_UPPER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.targetPowerToleranceUpperZero)) {
                uiPrintf("Unable to read the TARGET_POWER_ZERO_TOLERANCE_UPPER from %s\n", calsetupFileName);
            } 
        }
        else if(strnicmp("TARGET_POWER_ZERO_TOLERANCE_LOWER", pLine, strlen("TARGET_POWER_ZERO_TOLERANCE_LOWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.targetPowerToleranceLowerZero)) {
                uiPrintf("Unable to read the TARGET_POWER_ZERO_TOLERANCE_LOWER from %s\n", calsetupFileName);
            } 
        }
        else if(strnicmp("TARGET_POWER_ZERO_DELAY", pLine, strlen("TARGET_POWER_ZERO_DELAY")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.targetPowerZeroDelay)) {
                uiPrintf("Unable to read the TARGET_POWER_ZERO_DELAY from %s\n", calsetupFileName);
            } 
        }
        else if(strnicmp("TEST_THROUGHPUT", pLine, strlen("TEST_THROUGHPUT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_THROUGHPUT from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testThroughput[MODE_11a] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11g_TEST_RX_THROUGHPUT", pLine, strlen("11g_TEST_RX_THROUGHPUT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_RX_THROUGHPUT from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testRxThroughput[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11b_TEST_RX_THROUGHPUT", pLine, strlen("11b_TEST_RX_THROUGHPUT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11b_TEST_RX_THROUGHPUT from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testRxThroughput[MODE_11b] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("TEST_RX_THROUGHPUT", pLine, strlen("TEST_RX_THROUGHPUT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_RX_THROUGHPUT from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testRxThroughput[MODE_11a] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("TEST_TXPER_MARGIN", pLine, strlen("TEST_TXPER_MARGIN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_TXPER_MARGIN from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTXPER_margin = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("ATHEROS_LOGGING_SCHEME", pLine, strlen("ATHEROS_LOGGING_SCHEME")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the ATHEROS_LOGGING_SCHEME from %s\n", calsetupFileName);
            }
            else {
                CalSetup.atherosLoggingScheme = (testVal) ? TRUE : FALSE;
            }
        }
#if !defined(CUSTOMER_REL) && !defined(__ATH_DJGPPDOS__)
		else if(strnicmp("USE_FAST_CAL", pLine, strlen("USE_FAST_CAL")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the USE_FAST_CAL from %s\n", calsetupFileName);
            }
            else {
                CalSetup.useFastCal = (testVal) ? TRUE : FALSE;
            }
        }
#endif
		else if(strnicmp("CAL_POWER", pLine, strlen("CAL_POWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the CAL_POWER from %s\n", calsetupFileName);
            }
            else {
                CalSetup.calPower = (testVal) ? TRUE : FALSE;
            }
		}
        else if(strnicmp("OFFSET_CAL_POWER", pLine, strlen("OFFSET_CAL_POWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.offsetCalPower)) {
                uiPrintf("Unable to read the OFFSET_CAL_POWER from %s\n", calsetupFileName);
			} else{ //Siavash
				configSetup.offsetCalPower2x = (unsigned int)(2*CalSetup.offsetCalPower);
			}
        }
       else if(strnicmp("REPROGRAM_TARGET_POWER", pLine, strlen("REPROGRAM_TARGET_POWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the REPROGRAM_TARGET_POWER from %s\n", calsetupFileName);
            }
            else {
                CalSetup.reprogramTargetPwr = (testVal) ? TRUE : FALSE;
            }
        }
/*        else if(strnicmp("TEST_OBW_MASK", pLine, strlen("TEST_OBW_MASK")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
			CalSetup.testOBW = (A_UINT16) strtoul(pLine, NULL, 2);			
        }
*/
	   else if(strnicmp("TEST_OBW_MASK", pLine, strlen("TEST_OBW_MASK")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_OBW_MASK from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testOBW = (testVal) ? TRUE : FALSE;
            }
        }
	   else if(strnicmp("TEST_SPEC_MASK", pLine, strlen("TEST_SPEC_MASK")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_SPEC_MASK from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testSpecMask = (testVal) ? TRUE : FALSE;
            }
        }
	   else if(strnicmp("TEST_CHANNEL_ACCURACY", pLine, strlen("TEST_CHANNEL_ACCURACY")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_CHANNEL_ACCURACY from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testChannelAccuracy = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("TEST_TXPER", pLine, strlen("TEST_TXPER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_TXPER from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTXPER = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("TEST_RXSEN", pLine, strlen("TEST_RXSEN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_RXSEN from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testRXSEN = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("TEST_TURBO_MODE", pLine, strlen("TEST_TURBO_MODE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_TURBO_MODE from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTURBO= (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11g_TEST_TURBO_MODE", pLine, strlen("11g_TEST_TURBO_MODE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_TURBO_MODE from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTURBO_2p4[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
		else if(strnicmp("TEST_HALF_RATE_MODE", pLine, strlen("TEST_HALF_RATE_MODE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_HALF_RATE_MODE from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testHALFRATE= (testVal) ? TRUE : FALSE;
            }
        }
		else if(strnicmp("TEST_QUARTER_RATE_MODE", pLine, strlen("TEST_QUARTER_RATE_MODE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_QUARTER_RATE_MODE from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testQUARTERRATE= (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("TEST_TARGET_POWER", pLine, strlen("TEST_TARGET_POWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the TEST_TARGET_POWER from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTargetPowerControl[MODE_11a] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11b_TEST_TARGET_POWER", pLine, strlen("11b_TEST_TARGET_POWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11b_TEST_TARGET_POWER from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTargetPowerControl[MODE_11b] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11g_TEST_TARGET_POWER", pLine, strlen("11g_TEST_TARGET_POWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_TARGET_POWER from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTargetPowerControl[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("TARGET_POWER_TOLERANCE_UPPER", pLine, strlen("TARGET_POWER_TOLERANCE_UPPER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.targetPowerToleranceUpper)) {
                uiPrintf("Unable to read the TARGET_POWER_TOLERANCE_UPPER from %s\n", calsetupFileName);
            } 
        }
        else if(strnicmp("TARGET_POWER_TOLERANCE_LOWER", pLine, strlen("TARGET_POWER_TOLERANCE_LOWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.targetPowerToleranceLower)) {
                uiPrintf("Unable to read the TARGET_POWER_TOLERANCE_LOWER from %s\n", calsetupFileName);
            } 
        }

        else if(strnicmp("TARGET_POWER_ANTB_TOLERANCE_UPPER", pLine, strlen("TARGET_POWER_ANTB_TOLERANCE_UPPER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.targetPowerToleranceUpperAntB)) {
                uiPrintf("Unable to read the TARGET_POWER_ANTB_TOLERANCE_UPPER from %s\n", calsetupFileName);
            } 
        }
        else if(strnicmp("TARGET_POWER_ANTB_TOLERANCE_LOWER", pLine, strlen("TARGET_POWER_ANTB_TOLERANCE_LOWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.targetPowerToleranceLowerAntB)) {
                uiPrintf("Unable to read the TARGET_POWER_ANTB_TOLERANCE_LOWER from %s\n", calsetupFileName);
            } 
        }
        else if(strnicmp("TARGET_POWER_ANTB_RATE", pLine, strlen("TARGET_POWER_ANTB_RATE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.targetPowerAntBRate)) {
                uiPrintf("Unable to read the TARGET_POWER_ANTB_RATE from %s\n", calsetupFileName);
            }
        }

		else if(strnicmp("11a_CURR_TX_HI", pLine, strlen("11a_CURR_TX_HI")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.hi[CURR_CNSMPN_TX][MODE_11a])) {
                uiPrintf("Unable to read the 11a_CURR_TX_HI from %s\n", calsetupFileName);
            } 
        }
		else if(strnicmp("11a_CURR_TX_LO", pLine, strlen("11a_CURR_TX_LO")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.lo[CURR_CNSMPN_TX][MODE_11a])) {
                uiPrintf("Unable to read the 11a_CURR_TX_LO from %s\n", calsetupFileName);
            } 
        }
		else if(strnicmp("11a_CURR_RX_HI", pLine, strlen("11a_CURR_RX_HI")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.hi[CURR_CNSMPN_RX][MODE_11a])) {
                uiPrintf("Unable to read the 11a_CURR_RX_HI from %s\n", calsetupFileName);
            } 
        }
		else if(strnicmp("11a_CURR_RX_LO", pLine, strlen("11a_CURR_RX_LO")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.lo[CURR_CNSMPN_RX][MODE_11a])) {
                uiPrintf("Unable to read the 11a_PWR_RX_LO from %s\n", calsetupFileName);
            } 
        }
		else if(strnicmp("11b_CURR_TX_HI", pLine, strlen("11b_CURR_TX_HI")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.hi[CURR_CNSMPN_TX][MODE_11b])) {
                uiPrintf("Unable to read the 11b_CURR_TX_HI from %s\n", calsetupFileName);
            } 
        }
		else if(strnicmp("11b_CURR_TX_LO", pLine, strlen("11b_CURR_TX_LO")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.lo[CURR_CNSMPN_TX][MODE_11b])) {
                uiPrintf("Unable to read the 11b_CURR_TX_LO from %s\n", calsetupFileName);
            } 
        }
		else if(strnicmp("11b_CURR_RX_HI", pLine, strlen("11b_CURR_RX_HI")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.hi[CURR_CNSMPN_RX][MODE_11b])) {
                uiPrintf("Unable to read the 11b_CURR_RX_HI from %s\n", calsetupFileName);
            } 
        }
		else if(strnicmp("11b_CURR_RX_LO", pLine, strlen("11b_CURR_RX_LO")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.lo[CURR_CNSMPN_RX][MODE_11b])) {
                uiPrintf("Unable to read the 11b_CURR_RX_LO from %s\n", calsetupFileName);
            } 
        }
		else if(strnicmp("11g_CURR_TX_HI", pLine, strlen("11g_CURR_TX_HI")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.hi[CURR_CNSMPN_TX][MODE_11g])) {
                uiPrintf("Unable to read the 11g_CURR_TX_HI from %s\n", calsetupFileName);
            } 
        }
		else if(strnicmp("11g_CURR_TX_LO", pLine, strlen("11g_CURR_TX_LO")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.lo[CURR_CNSMPN_TX][MODE_11g])) {
                uiPrintf("Unable to read the 11g_CURR_TX_LO from %s\n", calsetupFileName);
            } 
        }
		else if(strnicmp("11g_CURR_RX_HI", pLine, strlen("11g_CURR_RX_HI")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.hi[CURR_CNSMPN_RX][MODE_11g])) {
                uiPrintf("Unable to read the 11g_CURR_RX_HI from %s\n", calsetupFileName);
            } 
        }
		else if(strnicmp("11g_CURR_RX_LO", pLine, strlen("11g_CURR_RX_LO")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.lo[CURR_CNSMPN_RX][MODE_11g])) {
                uiPrintf("Unable to read the 11g_CURR_RX_LO from %s\n", calsetupFileName);
            } 
        }
 
// 11b test setup
        else if(strnicmp("11b_TEST_SPEC_MASK", pLine, strlen("11b_TEST_SPEC_MASK")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11b_TEST_SPEC_MASK from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testSpecMask_2p4[MODE_11b] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11b_TEST_TXPER", pLine, strlen("11b_TEST_TXPER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11b_TEST_TXPER from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTXPER_2p4[MODE_11b] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11b_TEST_RXSEN", pLine, strlen("11b_TEST_RXSEN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11b_TEST_RXSEN from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testRXSEN_2p4[MODE_11b] = (testVal) ? TRUE : FALSE;
            }
        }

// ofdm@2p4 test setup
        else if(strnicmp("11g_TEST_SPEC_MASK", pLine, strlen("11g_TEST_SPEC_MASK")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_SPEC_MASK from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testSpecMask_2p4[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11g_TEST_CHANNEL_ACCURACY", pLine, strlen("11g_TEST_CHANNEL_ACCURACY")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_CHANNEL_ACCURACY from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testChannelAccuracy_2p4[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11g_TEST_TXPER", pLine, strlen("11g_TEST_TXPER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_TXPER from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testTXPER_2p4[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("11g_TEST_RXSEN", pLine, strlen("11g_TEST_RXSEN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_RXSEN from %s\n", calsetupFileName);
            }
            else {
                CalSetup.testRXSEN_2p4[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("SUB_VENDOR_ID", pLine, strlen("SUB_VENDOR_ID")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%x", &CalSetup.subVendorID)) {
                uiPrintf("Unable to read the SUB_VENDOR_ID from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("GOLDEN_PPM", pLine, strlen("GOLDEN_PPM")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.goldenPPM)) {
                uiPrintf("Unable to read the GOLDEN_PPM from %s\n", calsetupFileName);
            }
        }
       else if(strnicmp("DISABLE_GOLDEN_PPM_TEST", pLine, strlen("DISABLE_GOLDEN_PPM_TEST")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &testVal)) {
                uiPrintf("Unable to read the DISABLE_GOLDEN_PPM_TEST from %s\n", calsetupFileName);
            }
            else {
                CalSetup.goldenPPMDisable = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("NUM_RETRIES_THROUGHPUT", pLine, strlen("NUM_RETRIES_THROUGHPUT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.numRetriesThruput)) {
                uiPrintf("Unable to read the NUM_RETRIES_THROUGHPUT from %s\n", calsetupFileName);
            }
        }

//snoop: take it out. debug only.     
		else if(strnicmp("VERIFY_DATA_PACKET_LEN", pLine, strlen("VERIFY_DATA_PACKET_LEN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &VERIFY_DATA_PACKET_LEN)) {
                uiPrintf("Unable to read the VERIFY_DATA_PACKET_LEN from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("11a_TEST_CURR_CNSMPN_TX", pLine, strlen("11a_TEST_CURR_CNSMPN_TX")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11a_TEST_CURR_CNSMPN_TX from %s\n", calsetupFileName);
            }
			else {
                CalSetup.CURR_CNSMPN_STTNGS.txTestEnable[MODE_11a] = (testVal) ? TRUE : FALSE;
            }
        }
		else if(strnicmp("11a_TEST_CURR_CNSMPN_RX", pLine, strlen("11a_TEST_CURR_CNSMPN_RX")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11a_TEST_CURR_CNSMPN_RX from %s\n", calsetupFileName);
            }
			else {
                CalSetup.CURR_CNSMPN_STTNGS.rxTestEnable[MODE_11a] = (testVal) ? TRUE : FALSE;
            }
        }
		else if(strnicmp("11b_TEST_CURR_CNSMPN_TX", pLine, strlen("11b_TEST_CURR_CNSMPN_TX")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11b_TEST_CURR_CNSMPN_TX from %s\n", calsetupFileName);
            }
			else {
                CalSetup.CURR_CNSMPN_STTNGS.txTestEnable[MODE_11b] = (testVal) ? TRUE : FALSE;
            }
        }
		else if(strnicmp("11b_TEST_CURR_CNSMPN_RX", pLine, strlen("11b_TEST_CURR_CNSMPN_RX")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
				uiPrintf("Unable to read the 11b_TEST_CURR_CNSMPN_RX from %s\n", calsetupFileName);
            }
			else {
                CalSetup.CURR_CNSMPN_STTNGS.rxTestEnable[MODE_11b] = (testVal) ? TRUE : FALSE;
            }
        }
		else if(strnicmp("11g_TEST_CURR_CNSMPN_TX", pLine, strlen("11g_TEST_CURR_CNSMPN_TX")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_CURR_CNSMPN_TX from %s\n", calsetupFileName);
            }
			else {
                CalSetup.CURR_CNSMPN_STTNGS.txTestEnable[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
		else if(strnicmp("11g_TEST_CURR_CNSMPN_RX", pLine, strlen("11g_TEST_CURR_CNSMPN_RX")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the 11g_TEST_CURR_CNSMPN_RX from %s\n", calsetupFileName);
            }
			else {
                CalSetup.CURR_CNSMPN_STTNGS.rxTestEnable[MODE_11g] = (testVal) ? TRUE : FALSE;
            }
        }
		else if(strnicmp("TEST_IDLE_CURR_CNSMPN", pLine, strlen("TEST_IDLE_CURR_CNSMPN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.CURR_CNSMPN_STTNGS.idleTestEnable)) {
                uiPrintf("Unable to read TEST_IDLE_CURR_CNSMPN from %s\n", calsetupFileName);
            }
        }
		else if(strnicmp("ZERO_ATTENUATOR", pLine, strlen("ZERO_ATTENUATOR")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.zeroAttenuator)) {
                uiPrintf("Unable to read ZERO_ATTENUATOR from %s\n", calsetupFileName);
            }
        }
		else if(strnicmp("TEST_CONTINUITY", pLine, strlen("TEST_CONTINUITY")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.testContinuity)) {
                uiPrintf("Unable to read TEST_CONTINUITY from %s\n", calsetupFileName);
            }
        }
		else if(strnicmp("CONTINUITY_OUTPUT_GPIO", pLine, strlen("CONTINUITY_OUTPUT_GPIO")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.continuityOutputGpio)) {
                uiPrintf("Unable to read CONTINUITY_OUTPUT_GPIO from %s\n", calsetupFileName);
            }
        }
		else if(strnicmp("CONTINUITY_INPUT_GPIO", pLine, strlen("CONTINUITY_INPUT_GPIO")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.continuityInputGpio)) {
                uiPrintf("Unable to read CONTINUITY_INPUT_GPIO from %s\n", calsetupFileName);
            }
        }
		/*
		else if(strnicmp("SUPPLY_VOLTAGE", pLine, strlen("SUPPLY_VOLTAGE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.supplyVoltage)) {
                uiPrintf("Unable to read SUPPLY_VOLTAGE from %s\n", calsetupFileName);
            }
        }
		*/
		else if(strnicmp("IDLE_CURR_HI", pLine, strlen("IDLE_CURR_HI")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.idleHi )) {
                uiPrintf("Unable to read the IDLE_CURR_HI from %s\n", calsetupFileName);
            }
		}
		else if(strnicmp("IDLE_CURR_LO", pLine, strlen("IDLE_CURR_LO")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.CURR_CNSMPN_STTNGS.idleLo)) {
                uiPrintf("Unable to read the IDLE_CURR_LO from %s\n", calsetupFileName);
            }
		}
		else if(strnicmp("GOLDEN_TX_POWER", pLine, strlen("GOLDEN_TX_POWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.goldenTXPower)) {
                uiPrintf("Unable to read the GOLDEN_TX_POWER from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("11b_GOLDEN_TX_POWER", pLine, strlen("11b_GOLDEN_TX_POWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.goldenTXPower_2p4[MODE_11b])) {
                uiPrintf("Unable to read the 11b_GOLDEN_TX_POWER from %s\n", calsetupFileName);
            } 
        }
        else if(strnicmp("11g_GOLDEN_TX_POWER", pLine, strlen("11g_GOLDEN_TX_POWER")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.goldenTXPower_2p4[MODE_11g])) {
                uiPrintf("Unable to read the 11g_GOLDEN_TX_POWER from %s\n", calsetupFileName);
            }
        }
		else if((strnicmp("PM_MODEL", pLine, strlen("PM_MODEL")) == 0) &&
				((pLine[strlen("PM_MODEL")] == ' ') || 
				 (pLine[strlen("PM_MODEL")] == '\t') ) ){

				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				pLine = strtok(pLine," ;#");
				if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the PM_MODEL from %s\n", calsetupFileName);
				}
				switch (testVal) {
				case 1:
					CalSetup.pmModel = PM_436A;
					break;
				case 2:
					CalSetup.pmModel= PM_E4416A;
					break;
				case 3:
					CalSetup.pmModel = PM_4531;
					break;
				case 4:
					CalSetup.pmModel = NRP_Z11;
					break;
				case 5:
					CalSetup.pmModel = USBPM;
					break;
				case 99:
					CalSetup.pmModel = NO_PM;
					break;
				default:
					uiPrintf("ERROR parsing PM_MODEL:\nIllegal value %d. Choose from 1, 2 or 3\n", testVal);
					uiPrintf("Using the default power meter model HP436A\n");
					CalSetup.pmModel = PM_436A;
				}			
		}else if(strnicmp("PM_GPIB_ADDR", pLine, strlen("PM_GPIB_ADDR")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.pmGPIBaddr)) {
                uiPrintf("Unable to read the PM_GPIB_ADDR from %s\n", calsetupFileName);
            }
        }
		else if((strnicmp("SA_MODEL", pLine, strlen("SA_MODEL")) == 0) &&
				((pLine[strlen("SA_MODEL")] == ' ') || 
				 (pLine[strlen("SA_MODEL")] == '\t') ) ){

				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				pLine = strtok(pLine," ;#");
				if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the SA_MODEL from %s\n", calsetupFileName);
				}
				switch (testVal) {
				case 1:
					CalSetup.saModel= SPA_E4404B;
					break;
				case 2:
					CalSetup.saModel= SPA_8595E;
					break;
				case 3:
					CalSetup.saModel= SPA_R3162;
					break;
				case 4:
					CalSetup.saModel= SPA_FSL;
					break;
				case 99:
					CalSetup.saModel= SPA_NONE;
					break;
				default:
					uiPrintf("ERROR parsing SA_MODEL:\nIllegal value %d. Choose from 1 or 2 \n", testVal);
					uiPrintf("Using the default spectrum analyzer model SPA_E4404B\n");
					CalSetup.saModel = SPA_E4404B;
				}			
		}			        
        else if(strnicmp("SA_GPIB_ADDR", pLine, strlen("SA_GPIB_ADDR")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.saGPIBaddr)) {
                uiPrintf("Unable to read the SA_GPIB_ADDR from %s\n", calsetupFileName);
            }
        }
		else if((strnicmp("ATT_MODEL", pLine, strlen("ATT_MODEL")) == 0) &&
				((pLine[strlen("ATT_MODEL")] == ' ') || 
				 (pLine[strlen("ATT_MODEL")] == '\t') ) ){

				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				pLine = strtok(pLine," ;#");
				if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the ATT_MODEL from %s\n", calsetupFileName);
				}
				switch (testVal) {
				case 99:
					CalSetup.attModel= NO_ATT;
					break;
				case 1:
					CalSetup.attModel= ATT_11713A;
					break;
				case 2:
					CalSetup.attModel= ATT_11713A_110;
					break;
				case 3:
					CalSetup.attModel= ATT_JFW; //50bA-002-95 JFW 200-6000 MHz
					break;
				default:
					uiPrintf("ERROR parsing ATT_MODEL:\nIllegal value %d. Choose from 1 or 2 \n", testVal);
					uiPrintf("Using the default spectrum analyzer model SPA_E4404B\n");
					CalSetup.attModel = NO_ATT;
				}			
		}			        
        else if(strnicmp("ATT_GPIB_ADDR", pLine, strlen("ATT_GPIB_ADDR")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.attGPIBaddr)) {
                uiPrintf("Unable to read the ATT_GPIB_ADDR from %s\n", calsetupFileName);
            }
        }
		else if((strnicmp("DMM_MODEL", pLine, strlen("DMM_MODEL")) == 0) &&
			((pLine[strlen("DMM_MODEL")] == ' ') || 
				(pLine[strlen("DMM_MODEL")] == '\t') ) ){

				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				pLine = strtok(pLine," ;#");
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the DMM_MODEL from %s\n", calsetupFileName);
				}
				switch (testVal) {
				case 99: 
					CalSetup.dmmModel = NO_DMM;
					uiPrintf("NO DMM_MODEL:\n", testVal);
					break;
				case 1:
					CalSetup.dmmModel = DMM_FLUKE_45;
					break;
				default:
					uiPrintf("ERROR parsing DMM_MODEL:\nIllegal value %d.  Choose 1\n", testVal);
					CalSetup.dmmModel = NO_DMM;
				}
		}
		else if(strnicmp("DMM_GPIB_ADDR", pLine, strlen("DMM_GPIB_ADDR")) == 0) {
			pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.dmmGPIBaddr)) { 
                uiPrintf("Unable to read the DMM_GPIB_ADDR from %s\n", calsetupFileName);
            }
        }
		else if(strnicmp("ATTEN_DUT_PM", pLine, strlen("ATTEN_DUT_PM")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.attenDutPM)) {
                uiPrintf("Unable to read the ATTEN_DUT_PM from %s\n", calsetupFileName);
            }
        }
       else if(strnicmp("ATTEN_ANTB_DUT_PM", pLine, strlen("ATTEN_ANTB_DUT_PM")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.attenDutPMAux)) {
                uiPrintf("Unable to read the ATTEN_ANTB_DUT_PM from %s\n", calsetupFileName);
            }
        }
       else if(strnicmp("ATTEN_DUT_SA", pLine, strlen("ATTEN_DUT_SA")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.attenDutSA)) {
                uiPrintf("Unable to read the ATTEN_DUT_SA from %s\n", calsetupFileName);
            }
        }
       else if(strnicmp("ATTEN_FIXED_DUT_GOLDEN", pLine, strlen("ATTEN_FIXED_DUT_GOLDEN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.attenDutGolden)) {
                uiPrintf("Unable to read the ATTEN_FIXED_DUT_GOLDEN from %s\n", calsetupFileName);
            }
        }
       else if(strnicmp("11b_ATTEN_DUT_PM", pLine, strlen("11b_ATTEN_DUT_PM")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.attenDutPM_2p4[MODE_11b])) {
                uiPrintf("Unable to read the 11b_ATTEN_DUT_PM from %s\n", calsetupFileName);
            } else
			{
				CalSetup.attenDutPM_2p4[MODE_11g] = CalSetup.attenDutPM_2p4[MODE_11b];
			}	
        }
       else if(strnicmp("11b_ATTEN_ANTB_DUT_PM", pLine, strlen("11b_ATTEN_ANTB_DUT_PM")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.attenDutPMAux_2p4[MODE_11b])) {
                uiPrintf("Unable to read the 11b_ATTEN_ANTB_DUT_PM from %s\n", calsetupFileName);
            } else
			{
				CalSetup.attenDutPMAux_2p4[MODE_11g] = CalSetup.attenDutPMAux_2p4[MODE_11b];
			}	
        }
       else if(strnicmp("11b_ATTEN_DUT_SA", pLine, strlen("11b_ATTEN_DUT_SA")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.attenDutSA_2p4[MODE_11b])) {
                uiPrintf("Unable to read the 11b_ATTEN_DUT_SA from %s\n", calsetupFileName);
            }else
			{
				CalSetup.attenDutSA_2p4[MODE_11g] = CalSetup.attenDutSA_2p4[MODE_11b];
			}	
        }
       else if(strnicmp("11b_ATTEN_FIXED_DUT_GOLDEN", pLine, strlen("11b_ATTEN_FIXED_DUT_GOLDEN")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.attenDutGolden_2p4[MODE_11b])) {
                uiPrintf("Unable to read the 11b_ATTEN_FIXED_DUT_GOLDEN from %s\n", calsetupFileName);
            }else
			{
				CalSetup.attenDutGolden_2p4[MODE_11g] = CalSetup.attenDutGolden_2p4[MODE_11b];
				//uiPrintf("CalSetup.goldenTXPower_2p4 = %d, %f\n",CalSetup.attenDutGolden_2p4[MODE_11g], CalSetup.attenDutGolden_2p4[MODE_11b]);
			}	
        }
       else if(strnicmp("11b_MAX_POWER_CAP", pLine, strlen("11b_MAX_POWER_CAP")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.maxPowerCap[MODE_11b])) {
                uiPrintf("Unable to read the 11b_MAX_POWER_CAP from %s\n", calsetupFileName);
            }	
        }
       else if(strnicmp("MAX_POWER_CAP", pLine, strlen("MAX_POWER_CAP")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.maxPowerCap[MODE_11a])) {
                uiPrintf("Unable to read the MAX_POWER_CAP from %s\n", calsetupFileName);
            }	
        }
       else if(strnicmp("11g_MAX_POWER_CAP", pLine, strlen("11g_MAX_POWER_CAP")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%lf", &CalSetup.maxPowerCap[MODE_11g])) {
                uiPrintf("Unable to read the 11g_MAX_POWER_CAP from %s\n", calsetupFileName);
            }	
        }
	   else if(strnicmp("11b_NUM_SENS_PACKETS", pLine, strlen("11b_NUM_SENS_PACKETS")) == 0) {			
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);		
				if(!sscanf(pLine, "%d", &CalSetup.numSensPackets[MODE_11b])) {
				uiPrintf("Unable to read the 11b_NUM_SENS_PACKETS from %s\n", calsetupFileName);
				}
		}			        
	   else if(strnicmp("11g_NUM_SENS_PACKETS", pLine, strlen("11g_NUM_SENS_PACKETS")) == 0) {			
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);		
				if(!sscanf(pLine, "%d", &CalSetup.numSensPackets[MODE_11g])) {
				uiPrintf("Unable to read the 11g_NUM_SENS_PACKETS from %s\n", calsetupFileName);
				}
		}			        
	   else if(strnicmp("NUM_SENS_PACKETS", pLine, strlen("NUM_SENS_PACKETS")) == 0) {			
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);		
				if(!sscanf(pLine, "%d", &CalSetup.numSensPackets[MODE_11a])) {
				uiPrintf("Unable to read the NUM_SENS_PACKETS from %s\n", calsetupFileName);
				}
		}			        
	   else if(strnicmp("TXPER_BACKOFF_ATTEN", pLine, strlen("TXPER_BACKOFF_ATTEN")) == 0) {			
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);		
				if(!sscanf(pLine, "%d", &CalSetup.txperBackoff)) {
				uiPrintf("Unable to read the TXPER_BACKOFF_ATTEN from %s\n", calsetupFileName);
				}
		}			        
	   else if(strnicmp("11b_CAL_FIXED_GAIN", pLine, strlen("11b_CAL_FIXED_GAIN")) == 0) {			
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);		
				if(!sscanf(pLine, "%d", &CalSetup.cal_fixed_gain[MODE_11b])) {
				uiPrintf("Unable to read the 11b_CAL_FIXED_GAIN from %s\n", calsetupFileName);
				}
		}			        
	   else if(strnicmp("11g_CAL_FIXED_GAIN", pLine, strlen("11g_CAL_FIXED_GAIN")) == 0) {			
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);		
				if(!sscanf(pLine, "%d", &CalSetup.cal_fixed_gain[MODE_11g])) {
				uiPrintf("Unable to read the 11g_CAL_FIXED_GAIN from %s\n", calsetupFileName);
				} 
		}			        
	   else if(strnicmp("CAL_FIXED_GAIN", pLine, strlen("CAL_FIXED_GAIN")) == 0) {			
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);		
				if(!sscanf(pLine, "%d", &CalSetup.cal_fixed_gain[MODE_11a])) {
				uiPrintf("Unable to read the CAL_FIXED_GAIN from %s\n", calsetupFileName);
				}
		}			        
	   else if(strnicmp("11b_CAL_FIXED_HIGHGAIN", pLine, strlen("11b_CAL_FIXED_HIGHGAIN")) == 0) {			
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);		
				if(!sscanf(pLine, "%d", &CalSetup.cal_fixed_highgain[MODE_11b])) {
				uiPrintf("Unable to read the 11b_CAL_FIXED_HIGHGAIN from %s\n", calsetupFileName);
				}
		}			        
	   else if(strnicmp("11g_CAL_FIXED_HIGHGAIN", pLine, strlen("11g_CAL_FIXED_HIGHGAIN")) == 0) {			
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);		
				if(!sscanf(pLine, "%d", &CalSetup.cal_fixed_highgain[MODE_11g])) {
				uiPrintf("Unable to read the 11g_CAL_FIXED_HIGHGAIN from %s\n", calsetupFileName);
				} 
		}			        
	   else if(strnicmp("CAL_FIXED_HIGHGAIN", pLine, strlen("CAL_FIXED_HIGHGAIN")) == 0) {			
				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);		
				if(!sscanf(pLine, "%d", &CalSetup.cal_fixed_highgain[MODE_11a])) {
				uiPrintf("Unable to read the CAL_FIXED_HIGHGAIN from %s\n", calsetupFileName);
				}
		}			        
	   else if(strnicmp("DO_IQ_CAL", pLine, strlen("DO_IQ_CAL")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the DO_IQ_CAL from %s\n", calsetupFileName);
            }
            else {
                CalSetup.do_iq_cal = (testVal) ? TRUE : FALSE;
            }
        }
	   else if(strnicmp("CUSTOMER_DEBUG", pLine, strlen("CUSTOMER_DEBUG")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the CUSTOMER_DEBUG from %s\n", calsetupFileName);
            }
            else {
                CalSetup.customerDebug = (testVal) ? TRUE : FALSE;
            }
        }
	   	   else if(strnicmp("MANUFACTURING_EEPROM", pLine, strlen("MANUFACTURING_EEPROM")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the MANUFACTURING_EEPROM from %s\n", calsetupFileName);
            }
            else {
                CalSetup.restoreEeprom = (testVal) ? TRUE : FALSE;
            }
        }
	   else if(strnicmp("MAX_RETEST_NUM", pLine, strlen("MAX_RETEST_NUM")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.maxRetestIters)) {
                uiPrintf("Unable to read the MAX_RETEST_NUM from %s\n", calsetupFileName);
            }
			if (CalSetup.maxRetestIters < 1) {
				CalSetup.maxRetestIters = 1;
			}
        }
        else if(strnicmp("SHOW_TIMING_REPORT", pLine, strlen("SHOW_TIMING_REPORT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the SHOW_TIMING_REPORT from %s\n", calsetupFileName);
            }
            else {
                CalSetup.showTimingReport = (testVal) ? TRUE : FALSE;
            }
        }
        else if(strnicmp("END_TEST_ON_FAIL", pLine, strlen("END_TEST_ON_FAIL")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the END_TEST_ON_FAIL from %s\n", calsetupFileName);
            }
            else {
                CalSetup.endTestOnFail = (testVal) ? TRUE : FALSE;
            }
        }
		else if(strnicmp("MASK_POINTS_USE_MATRIX", pLine, strlen("MASK_POINTS_USE_MATRIX")) == 0) {
			pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)){
                uiPrintf("Unable to read the MASK_POINTS_USE_MATRIX from %s\n", calsetupFileName);
            }
			else {
				CalSetup.maskFailPointsUseMatrix = (testVal) ? TRUE : FALSE;
			}
        }

        else if(strnicmp("PER_PASS_LIMIT", pLine, strlen("PER_PASS_LIMIT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.perPassLimit)) {
                uiPrintf("Unable to read the PER_PASS_LIMIT from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("SEN_PASS_LIMIT", pLine, strlen("SEN_PASS_LIMIT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.senPassLimit)) {
                uiPrintf("Unable to read the SEN_PASS_LIMIT from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("PPM_MAX_LIMIT", pLine, strlen("PPM_MAX_LIMIT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.ppmMaxLimit)) {
                uiPrintf("Unable to read the PPM_MAX_LIMIT from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("PPM_MAX_QUARTER_LIMIT", pLine, strlen("PPM_MAX_QUARTER_LIMIT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.ppmMaxQuarterLimit)) {
                uiPrintf("Unable to read the PPM_MAX_QUARTER_LIMIT from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("AUTO_PIER_START", pLine, strlen("AUTO_PIER_START")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &goldenParams.channelStart)) {
                uiPrintf("Unable to read the AUTO_PIER_START from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("AUTO_PIER_STOP", pLine, strlen("AUTO_PIER_STOP")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &goldenParams.channelStop)) {
                uiPrintf("Unable to read the AUTO_PIER_STOP from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("AUTO_PIER_STEP", pLine, strlen("AUTO_PIER_STEP")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &goldenParams.measurementStep)) {
                uiPrintf("Unable to read the AUTO_PIER_STEP from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("PPM_MIN_LIMIT", pLine, strlen("PPM_MIN_LIMIT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.ppmMinLimit)) {
                uiPrintf("Unable to read the PPM_MIN_LIMIT from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("PPM_MIN_QUARTER_LIMIT", pLine, strlen("PPM_MIN_QUARTER_LIMIT")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.ppmMinQuarterLimit)) {
                uiPrintf("Unable to read the PPM_MIN_QUARTER_LIMIT from %s\n", calsetupFileName);
            }
        }
        else if(strnicmp("NUM_MASK_FAIL_POINTS", pLine, strlen("NUM_MASK_FAIL_POINTS")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.maskFailLimit)) {
                uiPrintf("Unable to read the NUM_MASK_FAIL_POINTS from %s\n", calsetupFileName);
            }
        }
		else if((strnicmp("FORCE_PIERS", pLine, strlen("FORCE_PIERS")) == 0) &&
			    ((pLine[strlen("FORCE_PIERS")] == ' ') || 
				 (pLine[strlen("FORCE_PIERS")] == '\t') ) ){

				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				pLine = strtok(pLine," ;#");
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the FORCE_PIERS from %s\n", calsetupFileName);
				}
				else {
					CalSetup.forcePiers = (testVal) ? TRUE : FALSE;
				}
		}			        
		else if((strnicmp("FORCE_PIERS_LIST", pLine, strlen("FORCE_PIERS_LIST")) == 0) &&
			    ((pLine[strlen("FORCE_PIERS_LIST")] == ' ') || 
				 (pLine[strlen("FORCE_PIERS_LIST")] == '\t') ) ){

				pLine = strchr(pLine, '=');
				pLine++;
				pLine = strtok(pLine,";#");
				
				ii = 0;
				pLine = strtok(pLine, " ,");
				while  ((pLine != NULL) && (pLine[0] != ';') && (ii < 10))
				{
					
					if(!sscanf(pLine, "%d", &signedTempVal )){						
						uiPrintf("Unable to read next forced pier from -->%s<--\n", pLine);
					} else
					{
						CalSetup.piersList[ii++] = (A_UINT16) signedTempVal;
					}
					pLine = strtok(NULL, " ,");
				}
				CalSetup.numForcedPiers = ii;
				if (ii < 1) {
					uiPrintf("Unable to read the FORCE_PIERS_LIST from %s\n", calsetupFileName);
				}
// This restriction on force_piers_list begin and end taken out starting eeprom version 3.3 to
// support 4.9G band.  - PD 9/02
/*
				if((CalSetup.piersList[0] != goldenParams.channelStart) || (CalSetup.piersList[CalSetup.numForcedPiers-1] != goldenParams.channelStop))
				{
					uiPrintf("ERROR: The first and last entries in the FORCE_PIERS_LIST in calsetup.txt are \n");
					uiPrintf("required to be %d and %d, respectively.\n", goldenParams.channelStart, goldenParams.channelStop);
					exit(0);
				}
*/
		}
		else if((strnicmp("FORCE_PIERS_11b", pLine, strlen("FORCE_PIERS_11b")) == 0) &&
			    ((pLine[strlen("FORCE_PIERS_11b")] == ' ') || 
				 (pLine[strlen("FORCE_PIERS_11b")] == '\t') ) ){

				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				pLine = strtok(pLine," ;#");
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the FORCE_PIERS_11b from %s\n", calsetupFileName);
				}
				else {
					CalSetup.forcePiers_2p4[MODE_11b] = (testVal) ? TRUE : FALSE;
				}
		}			        
		else if((CalSetup.forcePiers_2p4[MODE_11b]) &&
				((strnicmp("FORCE_PIERS_LIST_11b", pLine, strlen("FORCE_PIERS_LIST_11b")) == 0) &&
			     ((pLine[strlen("FORCE_PIERS_LIST_11b")] == ' ') || 
				   (pLine[strlen("FORCE_PIERS_LIST_11b")] == '\t') ) ) ){

				pLine = strchr(pLine, '=');
				pLine++;
				pLine = strtok(pLine,";#");
				
				ii = 0;
				pLine = strtok(pLine, " ,");
				while  ((pLine != NULL) && (pLine[0] != ';') && (ii < numRAWChannels_2p4))
				{
					
					if(!sscanf(pLine, "%d", &signedTempVal )){						
						uiPrintf("Unable to read next 11b forced pier from -->%s<--\n", pLine);
					} else
					{
						CalSetup.piersList_2p4[MODE_11b][ii++] = (A_UINT16) signedTempVal;
					}
					pLine = strtok(NULL, " ,");
				}
				CalSetup.numForcedPiers_2p4[MODE_11b] = ii;
				if (ii < 1) {
					uiPrintf("Unable to read the FORCE_PIERS_LIST_11b from %s\n", calsetupFileName);
				}
		}
		else if((strnicmp("FORCE_PIERS_11g", pLine, strlen("FORCE_PIERS_11g")) == 0) &&
			    ((pLine[strlen("FORCE_PIERS_11g")] == ' ') || 
				 (pLine[strlen("FORCE_PIERS_11g")] == '\t') ) ){

				pLine = strchr(pLine, '=');
				pLine = strtok(pLine, delimiters);
				pLine = strtok(pLine," ;#");
				if(!sscanf(pLine, "%d", &testVal)) {
					uiPrintf("Unable to read the FORCE_PIERS_11g from %s\n", calsetupFileName);
				}
				else {
					CalSetup.forcePiers_2p4[MODE_11g] = (testVal) ? TRUE : FALSE;
				}
		}			        
		else if((CalSetup.forcePiers_2p4[MODE_11g]) &&
				((strnicmp("FORCE_PIERS_LIST_11g", pLine, strlen("FORCE_PIERS_LIST_11g")) == 0) &&
			     ((pLine[strlen("FORCE_PIERS_LIST_11g")] == ' ') || 
				   (pLine[strlen("FORCE_PIERS_LIST_11g")] == '\t') ) ) ){

				pLine = strchr(pLine, '=');
				pLine++;
				pLine = strtok(pLine,";#");
				
				ii = 0;
				pLine = strtok(pLine, " ,");
				while  ((pLine != NULL) && (pLine[0] != ';') && (ii < numRAWChannels_2p4))
				{
					
					if(!sscanf(pLine, "%d", &signedTempVal )){						
						uiPrintf("Unable to read next 11g forced pier from -->%s<--\n", pLine);
					} else
					{
						CalSetup.piersList_2p4[MODE_11g][ii++] = (A_UINT16) signedTempVal;
					}
					pLine = strtok(NULL, " ,");
				}
				CalSetup.numForcedPiers_2p4[MODE_11g] = ii;
				if (ii < 1) {
					uiPrintf("Unable to read the FORCE_PIERS_LIST_11g from %s\n", calsetupFileName);
				}
		}
		else if(strnicmp("USE_11g_CAL_FOR_11b", pLine, strlen("USE_11g_CAL_FOR_11b")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &testVal)) {
                uiPrintf("Unable to read the USE_11g_CAL_FOR_11b from %s\n", calsetupFileName);
            }
            else {
                CalSetup.useOneCal = (testVal) ? TRUE : FALSE;
            }
        }
		else if((strnicmp("MACID_FILENAME", pLine, strlen("MACID_FILENAME")) == 0) &&
			    ((pLine[strlen("MACID_FILENAME")] == ' ') || 
				 (pLine[strlen("MACID_FILENAME")] == '\t') ) ){
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.macidFile)) {
                uiPrintf("Unable to read the MACID_FILENAME from %s\n", calsetupFileName);
            }
            else {
            }
        }
		else if((strnicmp("RESTORE_EEPROM_FILE", pLine, strlen("RESTORE_EEPROM_FILE")) == 0) &&
			    ((pLine[strlen("RESTORE_EEPROM_FILE")] == ' ') || 
				 (pLine[strlen("RESTORE_EEPROM_FILE")] == '\t') ) ){
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.restoreFilename)) {
                uiPrintf("Unable to read the RESTORE_EEPROM_FILE from %s\n", calsetupFileName);
            }
            else {
            }
        }
		else if((strnicmp("LOGGING_PATH", pLine, strlen("LOGGING_PATH")) == 0) &&
			    ((pLine[strlen("LOGGING_PATH")] == ' ') || 
				 (pLine[strlen("LOGGING_PATH")] == '\t') ) ){
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.logFilePath)) {
                uiPrintf("Unable to read the LOGGING_PATH from %s\n", calsetupFileName);
            }
            else {
            }
        }
        else if(strnicmp("CASE_TEMPERATURE", pLine, strlen("CASE_TEMPERATURE")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%d", &CalSetup.caseTemperature)) {
                uiPrintf("Unable to read the CASE_TEMPERATURE from %s\n", calsetupFileName);
            }
        }
		else if(strnicmp("I_COEFF_5G", pLine, strlen("I_COEFF_5G")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.i_coeff_5G)) {
                uiPrintf("Unable to read the I_COEFF_5G from %s\n", calsetupFileName);
            }
        }
		else if(strnicmp("Q_COEFF_5G", pLine, strlen("Q_COEFF_5G")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.q_coeff_5G)) {
                uiPrintf("Unable to read the Q_COEFF_5G from %s\n", calsetupFileName);
            }
        }
		else if(strnicmp("I_COEFF_2G", pLine, strlen("I_COEFF_2G")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.i_coeff_2G)) {
                uiPrintf("Unable to read the I_COEFF_2G from %s\n", calsetupFileName);
            }
        }
		else if(strnicmp("Q_COEFF_2G", pLine, strlen("Q_COEFF_2G")) == 0) {
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%ld", &CalSetup.q_coeff_2G)) {
                uiPrintf("Unable to read the Q_COEFF_2G from %s\n", calsetupFileName);
            }
        }
  		else if((strnicmp("GOLDEN_IP_ADDR", pLine, strlen("GOLDEN_IP_ADDR")) == 0) &&
			    ((pLine[strlen("GOLDEN_IP_ADDR")] == ' ') || 
				 (pLine[strlen("GOLDEN_IP_ADDR")] == '\t') ) ){
            pLine = strchr(pLine, '=');
            pLine = strtok(pLine, delimiters);
            if(!sscanf(pLine, "%s", CalSetup.goldenIPAddr)) {
                uiPrintf("Unable to read the GOLDEN_IP_ADDR from %s\n", calsetupFileName);
            }
           else {
			//	uiPrintf("read the golden IP addr to be %s\n", CalSetup.goldenIPAddr);
            }
        }

    }  // End while (get line)
	fclose(fStream);	
}

void load_eep_vals(A_UINT32 devNum) 
{
	A_UINT32	pd_lo = 15; // if not set (for griffin), then will be masked out
	A_UINT32    pd_hi = 15; // if not set (for griffin), then will be masked out
	A_UINT32    numPdGainsUsed = 0;
	A_UINT32    xpd_gain_mask = 0;
	A_UINT32    tx_clip, cck_scale;

	// load board specific cal section from the appropriate eep file
	load_cal_section();

	if ((CalSetup.modeMaskForRadio[0] & 0x1) == 0x1) {
		CalSetup.instanceForMode[MODE_11g] = 1;
	} else if ((CalSetup.modeMaskForRadio[1] & 0x1) == 0x1) {
		CalSetup.instanceForMode[MODE_11g] = 2;
	}
	CalSetup.instanceForMode[MODE_11b] = CalSetup.instanceForMode[MODE_11g];

	if ((CalSetup.modeMaskForRadio[0] & 0x2) == 0x2) {
		CalSetup.instanceForMode[MODE_11a] = 1;
	} else if ((CalSetup.modeMaskForRadio[1] & 0x2) == 0x2) {
		CalSetup.instanceForMode[MODE_11a] = 2;
	}
	// load mode specific configuration settings for this board.
	// eep file should already be parsed by ART for these

#pragma warning(disable: 4244) 
	if (CalSetup.eeprom_map >= 2) {
		numPdGainsUsed = art_getFieldForModeChecked(devNum, "bb_num_pd_gain", MODE_11A, BASE);
		CalSetup.numPdGains = numPdGainsUsed + 1;
		CalSetup.pdGainOverlap = art_getFieldForModeChecked(devNum, "bb_pd_gain_overlap", MODE_11A, BASE);
		if (numPdGainsUsed >= 0) {
			pd_lo = XPD_LO;
//			pd_lo = art_getFieldForModeChecked(devNum, "bb_pd_gain_setting1", MODE_11A, BASE);
			CalSetup.pdGainBoundary[0] = art_getFieldForModeChecked(devNum, "bb_pd_gain_boundary_1", MODE_11A, BASE);;
			xpd_gain_mask = (1 << pd_lo);
		}
		if (numPdGainsUsed >= 1) {
			pd_hi = XPD_HI;
//			pd_hi = art_getFieldForModeChecked(devNum, "bb_pd_gain_setting2", MODE_11A, BASE);
			CalSetup.pdGainBoundary[1] = art_getFieldForModeChecked(devNum, "bb_pd_gain_boundary_2", MODE_11A, BASE);;
			xpd_gain_mask |= (1 << pd_hi);
		}
		if (numPdGainsUsed >= 2) {
			pd_hi = art_getFieldForModeChecked(devNum, "bb_pd_gain_setting3", MODE_11A, BASE);
			CalSetup.pdGainBoundary[2] = art_getFieldForModeChecked(devNum, "bb_pd_gain_boundary_3", MODE_11A, BASE);;
			xpd_gain_mask |= (1 << pd_hi);
		}
		if (numPdGainsUsed >= 3) {
			xpd_gain_mask = 0xF;
			CalSetup.pdGainBoundary[3] = art_getFieldForModeChecked(devNum, "bb_pd_gain_boundary_4", MODE_11A, BASE);;
		}
	} 
#pragma warning(default: 4244) 

	if((CalSetup.Amode || CalSetup.enableJapanMode11aNew) && 
		!(isCobra(swDeviceID) && ((subSystemID == AP53_FULL_SUBSYSTEM_ID) || (subSystemID == AP53_LITE_SUBSYSTEM_ID)))) {
		// Begin loading 11a Parameters
		CalSetup.ob_1 = art_getFieldForModeChecked(devNum,"rf_ob", MODE_11A, BASE);
		CalSetup.db_1 = art_getFieldForModeChecked(devNum,"rf_db", MODE_11A, BASE);
		CalSetup.ob_2 = art_getFieldForModeChecked(devNum,"rf_ob", MODE_11A, BASE);
		CalSetup.db_2 = art_getFieldForModeChecked(devNum,"rf_db", MODE_11A, BASE);
		CalSetup.ob_3 = art_getFieldForModeChecked(devNum,"rf_ob", MODE_11A, BASE);
		CalSetup.db_3 = art_getFieldForModeChecked(devNum,"rf_db", MODE_11A, BASE);
		CalSetup.ob_4 = art_getFieldForModeChecked(devNum,"rf_ob", MODE_11A, BASE);
		CalSetup.db_4 = art_getFieldForModeChecked(devNum,"rf_db", MODE_11A, BASE);
		CalSetup.switchSettling = art_getFieldForModeChecked(devNum,"bb_switch_settling", MODE_11A, BASE);
		CalSetup.switchSettling_Turbo[MODE_11a] = art_getFieldForModeChecked(devNum,"bb_switch_settling", MODE_11A, TURBO);
		CalSetup.txrxAtten = art_getFieldForModeChecked(devNum,"bb_txrxatten", MODE_11A, BASE);
		CalSetup.txrxAtten_Turbo[MODE_11a] = art_getFieldForModeChecked(devNum,"bb_txrxatten", MODE_11A, TURBO);
		CalSetup.rxtx_margin[MODE_11a] = art_getFieldForModeChecked(devNum,"bb_rxtx_margin_2ghz", MODE_11A, BASE);
		CalSetup.rxtx_margin_Turbo[MODE_11a] = art_getFieldForModeChecked(devNum,"bb_rxtx_margin_2ghz", MODE_11A, TURBO);
		CalSetup.thresh62 = art_getFieldForModeChecked(devNum,"bb_thresh62", MODE_11A, BASE);
		CalSetup.txEndToXLNAOn = art_getFieldForModeChecked(devNum,"bb_tx_end_to_xlna_on", MODE_11A, BASE);
		CalSetup.txEndToXPAOff = art_getFieldForModeChecked(devNum,"bb_tx_end_to_xpaa_off", MODE_11A, BASE);
		CalSetup.txFrameToXPAOn = art_getFieldForModeChecked(devNum,"bb_tx_frame_to_xpaa_on", MODE_11A, BASE);
		if (isEagle(swDeviceID)) {
			CalSetup.xpd = 1;
			CalSetup.xgain  = art_getFieldForModeChecked(devNum,"bb_pd_gain_setting1", MODE_11B, BASE);
		} else {
			if(((swDeviceID & 0xff) >= 0x14) && ((swDeviceID & 0xff) != 0x15)) {
				CalSetup.xpd = art_getFieldForModeChecked(devNum,"rf_xpdsel", MODE_11A, BASE);
			} else {
				CalSetup.xpd = 1 - art_getFieldForModeChecked(devNum,"rf_pwdxpd", MODE_11A, BASE);
			}
			if((swDeviceID & 0xff) >= 0x16) {
				CalSetup.xgain = art_getFieldForModeChecked(devNum,"rf_pdgain_lo", MODE_11A, BASE);
			} else {
				CalSetup.xgain = art_getFieldForModeChecked(devNum,"rf_xpd_gain", MODE_11A, BASE);
			}
		}
		CalSetup.adcDesiredSize = art_getFieldForModeChecked(devNum,"bb_adc_desired_size", MODE_11A, BASE);
		CalSetup.adcDesiredSize_Turbo[MODE_11a] = art_getFieldForModeChecked(devNum,"bb_adc_desired_size", MODE_11A, TURBO);
		CalSetup.pgaDesiredSize = art_getFieldForModeChecked(devNum,"bb_pga_desired_size", MODE_11A, BASE);
		CalSetup.pgaDesiredSize_Turbo[MODE_11a] = art_getFieldForModeChecked(devNum,"bb_pga_desired_size", MODE_11A, TURBO);
		CalSetup.antennaControl[0]  = art_getFieldForModeChecked(devNum,"bb_switch_table_idle", MODE_11A, BASE);
		CalSetup.antennaControl[1]  = art_getFieldForModeChecked(devNum,"bb_switch_table_t1", MODE_11A, BASE);
		CalSetup.antennaControl[2]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r1", MODE_11A, BASE);
		CalSetup.antennaControl[3]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r1x1", MODE_11A, BASE);
		CalSetup.antennaControl[4]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r1x2", MODE_11A, BASE);
		CalSetup.antennaControl[5]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r1x12", MODE_11A, BASE);
		CalSetup.antennaControl[6]  = art_getFieldForModeChecked(devNum,"bb_switch_table_t2", MODE_11A, BASE);
		CalSetup.antennaControl[7]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r2", MODE_11A, BASE);
		CalSetup.antennaControl[8]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r2x1", MODE_11A, BASE);
		CalSetup.antennaControl[9]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r2x2", MODE_11A, BASE);
		CalSetup.antennaControl[10] = art_getFieldForModeChecked(devNum,"bb_switch_table_r2x12", MODE_11A, BASE);
		if(((swDeviceID & 0xff) >= 0x14)&&((swDeviceID & 0xff) != 0x15)) {
			if (CalSetup.eeprom_map < 2) {
				CalSetup.fixed_bias[MODE_11a] = art_getFieldForModeChecked(devNum, "rf_Afixed_bias", MODE_11A, BASE);
			}
			if (CalSetup.eeprom_map >= 2) {
					CalSetup.cal_mult_xpd_gain_mask[MODE_11a] = xpd_gain_mask;
			} else if ((CalSetup.eeprom_map == 1) || ((swDeviceID & 0xff) >= 0x16)) { //derby2
				pd_lo = art_getFieldForModeChecked(devNum, "rf_pdgain_lo", MODE_11A, BASE);
				pd_hi = art_getFieldForModeChecked(devNum, "rf_pdgain_hi", MODE_11A, BASE);
				CalSetup.cal_mult_xpd_gain_mask[MODE_11a] = (1 << pd_lo) | (1 << pd_hi);
			}
				
		}
		// End loading 11a Parameters
	}

	if(CalSetup.Bmode && !(isEagle(swDeviceID) && ((subSystemID == AP53_FULL_SUBSYSTEM_ID) || (subSystemID == AP53_LITE_SUBSYSTEM_ID)))) {
		// Begin loading 11b Parameters
		CalSetup.ob_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"rf_ob", MODE_11B, BASE);
		CalSetup.db_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"rf_db", MODE_11B, BASE);
		CalSetup.switchSettling_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"bb_switch_settling", MODE_11B, BASE);
		CalSetup.txrxAtten_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"bb_txrxatten", MODE_11B, BASE);
		CalSetup.rxtx_margin[MODE_11b] = art_getFieldForModeChecked(devNum,"bb_rxtx_margin_2ghz", MODE_11B, BASE);
		CalSetup.thresh62_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"bb_thresh62", MODE_11B, BASE);
		if (isGriffin(swDeviceID)) {
			CalSetup.b_ob_2p4[MODE_11b] = 0x00;
			CalSetup.b_db_2p4[MODE_11b] = 0x00;
			CalSetup.txEndToXLNAOn_2p4[MODE_11b] = 0x00; // unused in griffin
			CalSetup.txEndToXPAOff_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"bb_tx_end_to_xpab_off", MODE_11B, BASE);
			CalSetup.txFrameToXPAOn_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"bb_tx_frame_to_xpab_on", MODE_11B, BASE);
			CalSetup.xpd_2p4[MODE_11b] = 1;
			CalSetup.xgain_2p4[MODE_11b]  = art_getFieldForModeChecked(devNum,"bb_pd_gain_setting1", MODE_11B, BASE);
		} else {
			CalSetup.b_ob_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"rf_b_ob", MODE_11B, BASE);
			CalSetup.b_db_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"rf_b_db", MODE_11B, BASE);
			CalSetup.txEndToXLNAOn_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"bb_tx_end_to_xlna_on", MODE_11B, BASE);
			CalSetup.txEndToXPAOff_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"bb_tx_end_to_xpaa_off", MODE_11B, BASE);
			CalSetup.txFrameToXPAOn_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"bb_tx_frame_to_xpaa_on", MODE_11B, BASE);
			if (isEagle(swDeviceID)) {
				CalSetup.xpd_2p4[MODE_11b] = 1;
				CalSetup.xgain_2p4[MODE_11b]  = art_getFieldForModeChecked(devNum,"bb_pd_gain_setting1", MODE_11B, BASE);
			}
			else {
				if(((swDeviceID & 0xff) >= 0x14) && ((swDeviceID & 0xff) != 0x15)) {
					CalSetup.xpd_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"rf_xpdsel", MODE_11B, BASE);
				} else {
					CalSetup.xpd_2p4[MODE_11b] = 1 - art_getFieldForModeChecked(devNum,"rf_pwdxpd", MODE_11B, BASE);
				}		
				if((swDeviceID & 0xff) >= 0x16) {
					CalSetup.xgain_2p4[MODE_11b]  = art_getFieldForModeChecked(devNum,"rf_pdgain_lo", MODE_11B, BASE);
				} else {
					CalSetup.xgain_2p4[MODE_11b]  = art_getFieldForModeChecked(devNum,"rf_xpd_gain", MODE_11B, BASE);
				}
			}
		}
		
		if(((swDeviceID & 0xff) >= 0x14) && ((swDeviceID & 0xff) != 0x15)) {
			if (CalSetup.eeprom_map >= 2) {
					CalSetup.cal_mult_xpd_gain_mask[MODE_11b] = xpd_gain_mask;
			} else if ((CalSetup.eeprom_map == 1) || ((swDeviceID & 0xff) >= 0x16)) { //derby2
				pd_lo = art_getFieldForModeChecked(devNum, "rf_pdgain_lo", MODE_11B, BASE);
				pd_hi = art_getFieldForModeChecked(devNum, "rf_pdgain_hi", MODE_11B, BASE);
				CalSetup.cal_mult_xpd_gain_mask[MODE_11b] = (1 << pd_lo) | (1 << pd_hi);
			}
		} 
		
		CalSetup.adcDesiredSize_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"bb_adc_desired_size", MODE_11B, BASE);
		CalSetup.pgaDesiredSize_2p4[MODE_11b] = art_getFieldForModeChecked(devNum,"bb_pga_desired_size", MODE_11B, BASE);
		CalSetup.antennaControl_2p4[MODE_11b][0]  = art_getFieldForModeChecked(devNum,"bb_switch_table_idle", MODE_11B, BASE);
		CalSetup.antennaControl_2p4[MODE_11b][1]  = art_getFieldForModeChecked(devNum,"bb_switch_table_t1", MODE_11B, BASE);
		CalSetup.antennaControl_2p4[MODE_11b][2]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r1", MODE_11B, BASE);
		CalSetup.antennaControl_2p4[MODE_11b][3]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r1x1", MODE_11B, BASE);
		CalSetup.antennaControl_2p4[MODE_11b][4]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r1x2", MODE_11B, BASE);
		CalSetup.antennaControl_2p4[MODE_11b][5]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r1x12", MODE_11B, BASE);
		CalSetup.antennaControl_2p4[MODE_11b][6]  = art_getFieldForModeChecked(devNum,"bb_switch_table_t2", MODE_11B, BASE);
		CalSetup.antennaControl_2p4[MODE_11b][7]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r2", MODE_11B, BASE);
		CalSetup.antennaControl_2p4[MODE_11b][8]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r2x1", MODE_11B, BASE);
		CalSetup.antennaControl_2p4[MODE_11b][9]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r2x2", MODE_11B, BASE);
		CalSetup.antennaControl_2p4[MODE_11b][10]  = art_getFieldForModeChecked(devNum,"bb_switch_table_r2x12", MODE_11B, BASE);
		//End loading 11b Parameters
	}

	if(CalSetup.Gmode && !(isEagle(swDeviceID) && ((subSystemID == AP53_FULL_SUBSYSTEM_ID) || (subSystemID == AP53_LITE_SUBSYSTEM_ID)) )) {
		// Begin loading 11g Parameters
		CalSetup.ob_2p4[MODE_11g] = art_getFieldForModeChecked(devNum,"rf_ob", MODE_11G, BASE);
		CalSetup.db_2p4[MODE_11g] = art_getFieldForModeChecked(devNum,"rf_db", MODE_11G, BASE);
		CalSetup.switchSettling_2p4[MODE_11g] = art_getFieldForMode(devNum,"bb_switch_settling", MODE_11G, BASE);
		CalSetup.switchSettling_Turbo[MODE_11g] = art_getFieldForMode(devNum,"bb_switch_settling", MODE_11G, TURBO);
		CalSetup.txrxAtten_2p4[MODE_11g] = art_getFieldForMode(devNum,"bb_txrxatten", MODE_11G, BASE);
		CalSetup.txrxAtten_Turbo[MODE_11g] = art_getFieldForMode(devNum,"bb_txrxatten", MODE_11G, TURBO);
		CalSetup.rxtx_margin[MODE_11g] = art_getFieldForMode(devNum,"bb_rxtx_margin_2ghz", MODE_11G, BASE);
		CalSetup.rxtx_margin_Turbo[MODE_11g] = art_getFieldForMode(devNum,"bb_rxtx_margin_2ghz", MODE_11G, TURBO);
		CalSetup.thresh62_2p4[MODE_11g] = art_getFieldForMode(devNum,"bb_thresh62", MODE_11G, BASE);
		if (isGriffin(swDeviceID)) {
			CalSetup.b_ob_2p4[MODE_11g] = 0x00;
			CalSetup.b_db_2p4[MODE_11g] = 0x00;
			CalSetup.txEndToXLNAOn_2p4[MODE_11g] = 0x00; // unused in griffin
			CalSetup.txEndToXPAOff_2p4[MODE_11g] = art_getFieldForMode(devNum,"bb_tx_end_to_xpab_off", MODE_11G, BASE);
			CalSetup.txFrameToXPAOn_2p4[MODE_11g] = art_getFieldForMode(devNum,"bb_tx_frame_to_xpab_on", MODE_11G, BASE);
			CalSetup.xpd_2p4[MODE_11g] = 1;
			CalSetup.xgain_2p4[MODE_11g]  = art_getFieldForMode(devNum,"bb_pd_gain_setting1", MODE_11G, BASE);
		} else {
			if(!isSwan(swDeviceID)){     //swan does not have b_ob/b_db
				CalSetup.b_ob_2p4[MODE_11g] = art_getFieldForMode(devNum,"rf_b_ob", MODE_11G, BASE);
				CalSetup.b_db_2p4[MODE_11g] = art_getFieldForMode(devNum,"rf_b_db", MODE_11G, BASE);
			}
			CalSetup.txEndToXLNAOn_2p4[MODE_11g] = art_getFieldForMode(devNum,"bb_tx_end_to_xlna_on", MODE_11G, BASE);
			CalSetup.txEndToXPAOff_2p4[MODE_11g] = art_getFieldForMode(devNum,"bb_tx_end_to_xpaa_off", MODE_11G, BASE);
			CalSetup.txFrameToXPAOn_2p4[MODE_11g] = art_getFieldForMode(devNum,"bb_tx_frame_to_xpaa_on", MODE_11G, BASE);
			if(isEagle(swDeviceID)) {
				CalSetup.xpd_2p4[MODE_11g] = 1;
				CalSetup.xgain_2p4[MODE_11g]  = art_getFieldForMode(devNum,"bb_pd_gain_setting1", MODE_11G, BASE);
			} else {
				if(((swDeviceID & 0xff) >= 0x14) && ((swDeviceID & 0xff) != 0x15)){
					CalSetup.xpd_2p4[MODE_11g] = art_getFieldForMode(devNum,"rf_xpdsel", MODE_11G, BASE);
				} else {
					CalSetup.xpd_2p4[MODE_11g] = 1 - art_getFieldForMode(devNum,"rf_pwdxpd", MODE_11G, BASE);
				}
				if((swDeviceID & 0xff) >= 0x16) {
					CalSetup.xgain_2p4[MODE_11g]  = art_getFieldForMode(devNum,"rf_pdgain_lo", MODE_11G, BASE);
				} else {
					CalSetup.xgain_2p4[MODE_11g]  = art_getFieldForMode(devNum,"rf_xpd_gain", MODE_11G, BASE);
				}
			}
		}

		CalSetup.adcDesiredSize_2p4[MODE_11g] = art_getFieldForMode(devNum,"bb_adc_desired_size", MODE_11G, BASE);
		CalSetup.adcDesiredSize_Turbo[MODE_11g] = art_getFieldForMode(devNum,"bb_adc_desired_size", MODE_11G, TURBO);
		CalSetup.pgaDesiredSize_2p4[MODE_11g] = art_getFieldForMode(devNum,"bb_pga_desired_size", MODE_11G, BASE);
		CalSetup.pgaDesiredSize_Turbo[MODE_11g] = art_getFieldForMode(devNum,"bb_pga_desired_size", MODE_11G, TURBO);
		CalSetup.antennaControl_2p4[MODE_11g][0]  = art_getFieldForMode(devNum,"bb_switch_table_idle", MODE_11G, BASE);
		CalSetup.antennaControl_2p4[MODE_11g][1]  = art_getFieldForMode(devNum,"bb_switch_table_t1", MODE_11G, BASE);
		CalSetup.antennaControl_2p4[MODE_11g][2]  = art_getFieldForMode(devNum,"bb_switch_table_r1", MODE_11G, BASE);
		CalSetup.antennaControl_2p4[MODE_11g][3]  = art_getFieldForMode(devNum,"bb_switch_table_r1x1", MODE_11G, BASE);
		CalSetup.antennaControl_2p4[MODE_11g][4]  = art_getFieldForMode(devNum,"bb_switch_table_r1x2", MODE_11G, BASE);
		CalSetup.antennaControl_2p4[MODE_11g][5] = art_getFieldForMode(devNum,"bb_switch_table_r1x12", MODE_11G, BASE);
		CalSetup.antennaControl_2p4[MODE_11g][6]  = art_getFieldForMode(devNum,"bb_switch_table_t2", MODE_11G, BASE);
		CalSetup.antennaControl_2p4[MODE_11g][7]  = art_getFieldForMode(devNum,"bb_switch_table_r2", MODE_11G, BASE);
		CalSetup.antennaControl_2p4[MODE_11g][8]  = art_getFieldForMode(devNum,"bb_switch_table_r2x1", MODE_11G, BASE);
		CalSetup.antennaControl_2p4[MODE_11g][9]  = art_getFieldForMode(devNum,"bb_switch_table_r2x2", MODE_11G, BASE);
		CalSetup.antennaControl_2p4[MODE_11g][10]  = art_getFieldForMode(devNum,"bb_switch_table_r2x12", MODE_11G, BASE);
		if(((swDeviceID & 0xff) >= 0x14) && ((swDeviceID & 0xff) != 0x15)){
			CalSetup.fixed_bias[MODE_11g] = art_getFieldForMode(devNum, "rf_Bfixed_bias", MODE_11G, BASE);
			if (CalSetup.eeprom_map >= 2) {
				CalSetup.cal_mult_xpd_gain_mask[MODE_11g] = xpd_gain_mask;
			} else if ((CalSetup.eeprom_map == 1) || ((swDeviceID & 0xff) >= 0x16)) { //derby2
				pd_lo = art_getFieldForMode(devNum, "rf_pdgain_lo", MODE_11G, BASE);
				pd_hi = art_getFieldForMode(devNum, "rf_pdgain_hi", MODE_11G, BASE);
				CalSetup.cal_mult_xpd_gain_mask[MODE_11g] = (1 << pd_lo) | (1 << pd_hi);
			} 
		}
        q_uiPrintf("3035: \n\n--------------------------------------------------\n\n");
        q_uiPrintf("3038: swDeviceID = %d, isGriffin(swDeviceID)=%d, isEagle(swDeviceID)=%d.\n", 
			 swDeviceID, isGriffin(swDeviceID), isEagle(swDeviceID));
		if (isGriffin(swDeviceID) || isEagle(swDeviceID)) {				
			// names are meaningless, just re-used for convenience
			tx_clip = art_getFieldForMode(devNum, "bb_desired_scale_0", MODE_11G, BASE);
			cck_scale = art_getFieldForMode(devNum, "bb_desired_scale_cck", MODE_11G, BASE);	
			// tx_clip - cck_scale is taken care of by the TPC state machine. ideally this should be 0
			CalSetup.ofdm_cck_gain_delta = 7.5 - 0.5*(tx_clip - cck_scale); 
            q_uiPrintf("3040: tx_clip=0x%x, cck_scale=0x%x, CalSetup.ofdm_cck_gain_delta=%f.\n", 
				 tx_clip, cck_scale, CalSetup.ofdm_cck_gain_delta);
		} else {           
			tx_clip = art_getFieldForMode(devNum, "bb_tx_clip", MODE_11G, BASE);
			cck_scale = art_getFieldForMode(devNum, "bb_tx_dac_scale_cck", MODE_11G, BASE);
			if ((tx_clip >= 0) && (tx_clip <= 7) && (cck_scale >= 0) && (cck_scale <= 2)) {
				CalSetup.ofdm_cck_gain_delta = 7.5 + cck_gain[cck_scale] - ofdm_gain[tx_clip];
			} else {
				uiPrintf("WARNING: ofdm baseband scaling (%d) out of range [0-7], or \n", tx_clip);
				uiPrintf("         cck  baseband scaling (%d) out of range [0-2] \n", cck_scale);
				CalSetup.ofdm_cck_gain_delta = 7.5;
			}
		}
		//End loading 11g Parameters
	}

}

A_UINT32 programUartPciCfgFile(A_UINT32 devNum, const char *filename, A_UINT32 startAddr,A_BOOL atCal) 
{
	A_UINT32  i,len = 0;//length;
	FILE *fStream;
    A_CHAR lineBuf[122], *pLine;
    A_CHAR inStr[50];
    A_INT32 offset, byte1, byte2, line;
	//A_UINT32 dataArray[0x400];
	A_UINT32 *dataArray;

	dataArray = (A_UINT32 *)calloc(eepromSize,sizeof(A_UINT32));

	if(dataArray == NULL)
	{
			printf(" Memory not Allocated in programUartPciCfgFile\n");
			exit(1);
	}
	for(i=0;i<eepromSize;i++)
		dataArray[i]= 0xffff;
		
	uiPrintf("Programming uart cfg file ... ");

	// Parse eeprom file
    if( (fStream = fopen( filename, "r")) == NULL ) {
        uiPrintf("Failed to open %s\n", filename);
        uiPrintf("-EEPROM common data for UART PCI config will not be written\n");
		free(dataArray);
        return 0;
    }

    offset = 0;
    byte1 = -1;
    line = 0;
    while(fgets(lineBuf, 120, fStream) != NULL) {
        line++;
        pLine = lineBuf;
        if((*pLine == ';') || isspace(*pLine)) {
            continue;
        }
        if ( sscanf(pLine, "%s", inStr) == 0) {
            uiPrintf("ERROR: readUartPciCfgFile : unable to parse line %d\n", line);
            uiPrintf("-EEPROM common data for UART PCI config will not be written\n");
			free(dataArray);
            return 0;
        }
        if ( strlen(inStr) == 2 ) {
            // a single byte was read
            if (byte1 != -1) {
                sscanf(inStr, "%x", &byte2);
                dataArray[offset] = ((byte2 << 8) & 0xFF00 | byte1);
                offset++;
                byte1 = -1;
            }
            else {
                sscanf(inStr, "%x", &byte1);
            }
        }
        else if ( strlen(inStr) == 4 ) {
            // a big-endian 16-bit word was read
            if ( byte1 != -1) { 
                uiPrintf("ERROR: readUartPciCfgFile : lonely single byte found on line %d.\n", line);
                uiPrintf("-EEPROM common data for UART PCI config will not be written\n");
                fclose(fStream);
				free(dataArray);
                return 0;
            }
            sscanf(inStr, "%x", &dataArray[offset]);
            offset++;
        }
        else {
            uiPrintf("ERROR: readUartPciCfgFile : unable to parse line %d\n", line);
            uiPrintf("-EEPROM common data for UART PCI config will not be written\n");
            fclose(fStream);
			free(dataArray);
            return 0;
        }
    } // End line parsing
    fclose(fStream);

	//if ((startAddr + offset) > 0x400)
	if ((startAddr + offset) > eepromSize) 		
	{
		uiPrintf("ERROR : readUartPciCfgFile : file %s too big for 16K EEPROM\n", filename);
		free(dataArray);
		return 0;
	}
	if(atCal)
	{

		virtual_eeprom0Write(devNum, 0x20, startAddr);	
		virtual_eeprom0Write(devNum, 0x21, (startAddr + 0x10));	
		virtual_eeprom0Write(devNum, 0x4, 0x80); //pci config register to enable multi-function
		virtual_eeprom0WriteBlock(devNum, startAddr, offset, dataArray);
	}
	else
	{
		eepromWrite(devNum, 0x20, startAddr);	
		eepromWrite(devNum, 0x21, (startAddr + 0x10));	
		eepromWrite(devNum, 0x4, 0x80); //pci config register to enable multi-function
		eepromWriteBlock(devNum, startAddr, offset, dataArray);
	}

	uiPrintf("Done\n");
	free(dataArray);

	return offset;
}

void Write_eeprom_Common_Data(A_UINT32 devNum, A_BOOL writeHeader, A_BOOL immediateWrite) 
{
     FILE *fStream;
    A_CHAR lineBuf[122], *pLine;
    A_INT32 offset, byte1, byte2, line;
    A_CHAR inStr[50];
	A_UINT32 common_eeprom_data[0x100];
//	A_UCHAR tmpStr[10];
	A_UINT32 macID[3] ;
	A_UINT32 tmp[0xF] ;
	A_UINT32 ii;
    A_CHAR cal_eepFileName[MAX_FILE_LENGTH];

    memset(common_eeprom_data, 0xff, sizeof(A_UINT32) * 0x100);

	if (usb_client == TRUE) {
		strcpy(cal_eepFileName, configSetup.cal_usb_eepFileName);
    }
	else if (isCondor(swDeviceID)) {
		strcpy(cal_eepFileName, configSetup.cal_express_eepFileName);
	}
	else {
		strcpy(cal_eepFileName, configSetup.cal_eepFileName);
	}
	// Parse eeprom file
    if( (fStream = fopen( cal_eepFileName, "r")) == NULL ) {
        uiPrintf("Failed to open %s\n", cal_eepFileName);
        uiPrintf("-EEPROM common data will not be written\n");
        return;
    }
    offset = 0;
    byte1 = -1;
    line = 0;
    while(fgets(lineBuf, 120, fStream) != NULL) {
        line++;
        pLine = lineBuf;
        if((*pLine == ';') || isspace(*pLine)) {
            continue;
        }
        if ( sscanf(pLine, "%s", inStr) == 0) {
            uiPrintf("ERROR: unable to parse line %d\n", line);
            uiPrintf("-EEPROM common data will not be written\n");
            return;
        }
        if ( strlen(inStr) == 2 ) {
            // a single byte was read
            if (byte1 != -1) {
                sscanf(inStr, "%x", &byte2);
                common_eeprom_data[offset] = ((byte2 << 8) & 0xFF00 | byte1);
                offset++;
                byte1 = -1;
            }
            else {
                sscanf(inStr, "%x", &byte1);
            }
        }
        else if ( strlen(inStr) == 4 ) {
            // a big-endian 16-bit word was read
            if ( byte1 != -1) { 
                uiPrintf("ERROR: lonely single byte found on line %d.\n", line);
                uiPrintf("-EEPROM common data will not be written\n");
                fclose(fStream);
                return;
            }
            sscanf(inStr, "%x", &common_eeprom_data[offset]);
            offset++;
        }
        else {
            uiPrintf("ERROR: unable to parse line %d\n", line);
            uiPrintf("-EEPROM common data will not be written\n");
            fclose(fStream);
            return;
        }
    } // End line parsing
    fclose(fStream);

    if (offset > 0xBF ) {
        uiPrintf("ERROR: more than 191 common 16-bit words read\n");
        uiPrintf("-EEPROM common data will not be written\n");
        return;
    }

    common_eeprom_data[0x07] = CalSetup.subsystemID;
    common_eeprom_data[0x08] = CalSetup.subVendorID;
	
	//FJC, changed this to use mac rev to decide on deviceID
	switch (macRev) {
	case 0x40:
	case 0x42:
		common_eeprom_data[0x00] = 0x0012;
		break;

	case 0x50:
	case 0x51:
	case 0x53:
	case 0x55:
	case 0x59:
	case 0x56:
	case 0x74:    // griffin 1.0
	case 0x75:    // griffin 1.1
	case 0x76:    // griffin 2.0
    case 0x79:    // griffin 2.1
		common_eeprom_data[0x00] = 0x0013;
		break;

	case 0x78:    // griffin lite
		common_eeprom_data[0x00] = 0x001a;
		break;

	case 0xa1:    // eagle 1.0 
	case 0xa0:    //eagle 1.0, swan 1.1
	case 0xa2:    //eagle 2.0
	case 0xa3:    //eagle 2.0
	case 0xa4:    //eagle 2.1
	case 0xa5:    //eagle 2.1
		if(isCondor(swDeviceID)) {
			common_eeprom_data[0x00] = 0x001c;
		} else {
			common_eeprom_data[0x00] = 0x001b;
		}

		break;

	case 0x91:    //condor 1.0
	case 0x93:    //condor 1.0
	case 0x92:    //condor 1.0
	case 0x9a:    //condor 2.0
	case 0x9b:    //condor 2.0
	case 0x98:    //condor 2.0
	case 0x99:    //condor 2.0
	case 0x9c:    //condor 3.1
	case 0x9d:    //condor 3.1
	case 0x9e:    //condor 3.1
	case 0x9f:    //condor 3.1
	case 0:       //condor with pci config eeprom load.
	//case 0xa0:     //swan1.1
		common_eeprom_data[0x00] = 0x001c;
		break;

	case 0xe0:       //condor with pci config eeprom load, swan 1.2
	case 0xe1: //swan 1.3
	case 0xe2: //swan 1.4
		common_eeprom_data[0x00] = 0x001c;
		break;
	case 0xf0:     //  nala
		common_eeprom_data[0] = 0x1d;
		break;
	case 0x52:
    case 0x57:
		if(configSetup.remote) {
			//this is an ap
			common_eeprom_data[0x00] = 0xa014;
			break;
		}
	
    case 0x58:
		if(configSetup.remote) {
			//this is an ap
			common_eeprom_data[0x00] = 0xa017;
			break;
		}

	case 0x30:
	case 0x31:
		if(configSetup.remote) {
			//this is an ap
			common_eeprom_data[0x00] = 0x0011;
			break;
		}
	case 0x80:
	case 0x81:
		if(configSetup.remote) {
			common_eeprom_data[0x00] = 0x00b0;
			break;
		}
	case 0xb0:   // cobra
		if(configSetup.remote) {
			common_eeprom_data[0x00] = 0x0013;
			break;
		}
		//else we want to drop through to the printf and error message
	default :
		uiPrintf("Card Type not supported.\n");
		exit(0);
		break;
	}

	/*
	// CalSetup.registerSpecificPatch default value is 0; same as the value defined in atheros-express-eep.txt
	// At this moment, these is a patch about spur and tx power should be applied to HB63
	// 0: normal swan; 1: for board #040
	*/
	if (isSwan(swDeviceID)) common_eeprom_data[0x0b] = CalSetup.registerSpecificPatch;

/*
// scheme changed to fill all 0s as per susanna 5/14/02.

	// overwrite the subsystemID in the 6th tuple (product ID)
	sprintf(tmpStr, "%4.4x", getSubsystemIDfromCardType(configSetup.dutCardType));
	common_eeprom_data[0x69] = ((tmpStr[0] & 0xff) << 8) | (common_eeprom_data[0x69] & 0xFF) ;
	common_eeprom_data[0x6A] = ((tmpStr[2] & 0xff) << 8) | (tmpStr[1] & 0xFF);
	common_eeprom_data[0x6B] = (common_eeprom_data[0x6B] & 0xff00)  | (tmpStr[3] & 0xFF);

	
	// overwrite the deviceID in the 6th tuple (product ID)
	sprintf(tmpStr, "%4.4x", common_eeprom_data[0x00]);
	common_eeprom_data[0x6C] = ((tmpStr[1] & 0xff) << 8) | (tmpStr[0] & 0xFF);
	common_eeprom_data[0x6D] = ((tmpStr[3] & 0xff) << 8) | (tmpStr[2] & 0xFF);
*/

	
	if (usb_client) {
		//set the productID
		common_eeprom_data[0x40] = CalSetup.productID;
	}

	// Preserve the macID
	art_eepromReadBlock(devNum, 0x1d,3,macID);
	common_eeprom_data[0x1d] = macID[0];
	common_eeprom_data[0x1e] = macID[1];
	common_eeprom_data[0x1f] = macID[2];
	if(!isSwan(swDeviceID)) {
		common_eeprom_data[0xa5] = SWAP_BYTES16(macID[2]);
		common_eeprom_data[0xa6] = SWAP_BYTES16(macID[1]);
		common_eeprom_data[0xa7] = SWAP_BYTES16(macID[0]);
	}

	// Preserve the label
	art_eepromReadBlock(devNum, 0xb0,0xF,tmp);
	for (ii=0; ii<0xF; ii++) {
		common_eeprom_data[0xb0 + ii] = tmp[ii];
	}

//	printf("SNOOP: swDeviceID = %d\n", swDeviceID);
	if(isCondor(swDeviceID) || isSwan(swDeviceID)) {

		remapPCIConfigInfo(devNum, common_eeprom_data);
	}

	virtual_eeprom0WriteBlock(devNum, 0, (0xBF-1), common_eeprom_data);
	if(immediateWrite) {
		art_eepromWriteBlock(devNum, 0, (0xBF-1), common_eeprom_data);
	}

	if (writeHeader > 0)
	{
        Write_eeprom_header(devNum) ;		
	} 

}


static void remapPCIConfigInfo
(
 A_UINT32 devNum,   
 A_UINT32 *eepromData
)
{
	A_UINT32 i;
	A_UINT32 newEepromLocation;
	A_UINT32 rfSilentInfo;
	A_UINT32 pciEepromPtr;

	//write the new data
    newEepromLocation = PCI_EXPRESS_CONFIG_PTR;
	if( isSwan(swDeviceID) && CalSetup.enableSerdesProgramming ) {
		//need to move the pointer to earlier in the eeprom
		newEepromLocation = PCI_EXPRESS_CONFIG_PTR_SERDES;
	}
	
	//take a copy of the eeprom pointer for writing later
	pciEepromPtr = newEepromLocation;

    for (i = 0; i < numMappings; i++) {
		//write the pci config address
		eepromData[newEepromLocation] = (0x5000 + fullMapping[i].pciConfigLocation)/4;

		//write the MS 16 bits
		if(fullMapping[i].eepromLocation_LSB == READ_CONFIG) {
			eepromData[newEepromLocation + 1] = 
				art_cfgRead(devNum, fullMapping[i].pciConfigLocation) & 0xffff;
		} else {
			eepromData[newEepromLocation + 1] = eepromData[fullMapping[i].eepromLocation_LSB];
		}
		
		//write the LS 16 bits
		if(fullMapping[i].eepromLocation_MSB == READ_CONFIG) {
			eepromData[newEepromLocation + 2] = 
				(art_cfgRead(devNum, fullMapping[i].pciConfigLocation) >> 16)& 0xffff;
		} else {
			eepromData[newEepromLocation + 2] = eepromData[fullMapping[i].eepromLocation_MSB]; 
		}
        newEepromLocation += 3;
	}
	
	//work out what the pm_cap should be - dependent on WOW
	eepromData[newEepromLocation] = (0x5000 + 0x40)/4;
	eepromData[newEepromLocation + 1] = art_cfgRead(devNum, 0x40) & 0xffff;
	if (CalSetup.Enable_WOW > 0) {
		if(isSwan(swDeviceID)){
			eepromData[newEepromLocation + 2] = 0xffc2;  // PM_CAP info, Swan is capable of power save.
		}
		else if(!isCondor(swDeviceID)){
			eepromData[newEepromLocation + 2] = 0xf9c2;  // PM_CAP info
		}
	} else {
		eepromData[newEepromLocation + 2] = 0x01c2;  // PM_CAP info
	}
	newEepromLocation += 3;

    if(isSwan(swDeviceID)){
		if(CalSetup.Enable_WOW > 0){
			//need to unlock the register to let the pmCap write go through
    		eepromData[newEepromLocation] = 0x101A; 
            if(CalSetup.enableSerdesProgramming == 2) {
                eepromData[newEepromLocation + 1] = 0x042A;
            } else {
                eepromData[newEepromLocation + 1] = 0x052A;
            }
    		eepromData[newEepromLocation + 2] = 0x0000;
			newEepromLocation += 3;
		}
		else if(CalSetup.Enable_WOW == 0){
			eepromData[newEepromLocation] = 0x101A; 
            if(CalSetup.enableSerdesProgramming == 2) {
                eepromData[newEepromLocation + 1] = 0x040A;
            } else {
                eepromData[newEepromLocation + 1] = 0x050A;
            }
    		eepromData[newEepromLocation + 2] = 0x0000;
			newEepromLocation += 3;
		}
    }

	if(!CalSetup.LOsBypass) {
		//adjust L0s and L1 latencies for Condor mode
		eepromData[newEepromLocation] = 0x15c3; 
		eepromData[newEepromLocation + 1] = 0x3f01;
		eepromData[newEepromLocation + 2] = 0x0f00;
		newEepromLocation += 3;
	}

	//enable L1s state
	if(CalSetup.enableASPMLatency) {
		eepromData[newEepromLocation] = 0x1419; 
		eepromData[newEepromLocation + 1] = 0x0CC0;
		eepromData[newEepromLocation + 2] = 0x0504;
		newEepromLocation += 3;
	}

	if(CalSetup.aspmSupport < 3) {
		eepromData[newEepromLocation] = 0x141b;
		switch(CalSetup.aspmSupport) {
		case 0:
			eepromData[newEepromLocation + 1] = 0x3011;
			break;
		case 1:
			eepromData[newEepromLocation + 1] = 0x3411;
			break;
		case 2:
			eepromData[newEepromLocation + 1] = 0x3811;
			break;
		}
		eepromData[newEepromLocation + 2] = 0x0003;
		newEepromLocation += 3;
	}

	if(CalSetup.RFSilent) {
		//need to program RF silent GPIO, details should be held 
		//in eeprom location 0x0f
		rfSilentInfo = eepromData[0xf];

		//eeprom data should be specified to the same bit layout of register
		//except force bit0 to one to enable rfsilent function
		rfSilentInfo = (rfSilentInfo | 0x01) & 0x1f;
		
		//drive the gpios as inputs by default.  Any auto gpio config would 
		//need to change this
		eepromData[newEepromLocation] = 0x1005; 
		eepromData[newEepromLocation + 1] = 0x0000;
		eepromData[newEepromLocation + 2] = 0x0000;
		newEepromLocation += 3;
		
		//configure the rfsilent config register
		eepromData[newEepromLocation] = 0x101e; 
		eepromData[newEepromLocation + 1] = rfSilentInfo;
		eepromData[newEepromLocation + 2] = 0x0000;
		newEepromLocation += 3;
	}

    if( isSwan(swDeviceID) && CalSetup.enableWA ) {
        //WA register
        eepromData[newEepromLocation] = 0x101f; 
		eepromData[newEepromLocation + 1] = 0x000f;
		eepromData[newEepromLocation + 2] = 0x0000;
		newEepromLocation += 3;

    }

	//new triplets needed for serdes programming 
	if( CalSetup.enableSerdesProgramming ) {
        eepromData[newEepromLocation] = 0x1020; 
		eepromData[newEepromLocation + 1] = 0xfc00;
		eepromData[newEepromLocation + 2] = 0x9248;
		newEepromLocation += 3;
	
        eepromData[newEepromLocation] = 0x1020; 
		eepromData[newEepromLocation + 1] = 0x4924;
		eepromData[newEepromLocation + 2] = 0x2492;
		newEepromLocation += 3;

        eepromData[newEepromLocation] = 0x1020; 
		eepromData[newEepromLocation + 1] = 0x0039;
		eepromData[newEepromLocation + 2] = 0x2800;
		newEepromLocation += 3;

        eepromData[newEepromLocation] = 0x1020; 
		eepromData[newEepromLocation + 1] = 0x0824;
		eepromData[newEepromLocation + 2] = 0x5316;
		newEepromLocation += 3;

        eepromData[newEepromLocation] = 0x1020; 
		eepromData[newEepromLocation + 1] = 0x0579;
		eepromData[newEepromLocation + 2] = 0xf680;
		newEepromLocation += 3;

        eepromData[newEepromLocation] = 0x1020; 
        if( CalSetup.enableSerdesProgramming == 2) {
            eepromData[newEepromLocation + 1] = 0xffff;
        } else {
            eepromData[newEepromLocation + 1] = 0xefff;
        }
		eepromData[newEepromLocation + 2] = 0x001d;
		newEepromLocation += 3;

        eepromData[newEepromLocation] = 0x1020; 
		eepromData[newEepromLocation + 1] = 0xbe40;
		eepromData[newEepromLocation + 2] = 0x1aaa;
		newEepromLocation += 3;

        eepromData[newEepromLocation] = 0x1020; 
		eepromData[newEepromLocation + 1] = 0x5554;
		eepromData[newEepromLocation + 2] = 0xbe10;
		newEepromLocation += 3;

        eepromData[newEepromLocation] = 0x1020; 
		eepromData[newEepromLocation + 1] = 0x3007;
		eepromData[newEepromLocation + 2] = 0x000e;
		newEepromLocation += 3;

        eepromData[newEepromLocation] = 0x1021; 
		eepromData[newEepromLocation + 1] = 0x0000;
		eepromData[newEepromLocation + 2] = 0x0000;
		newEepromLocation += 3;
	}
	
	//write end info
	eepromData[newEepromLocation] = 0xffff;

	//write the 3 control words 
	eepromData[0] = 0xa55a;       //magic ID
	eepromData[1] = eepromData[0x3f]; //grab the protection bits from old location
	eepromData[2] = pciEepromPtr/2;  //pointer to where cfg data will be
}

void Write_eeprom_header(A_UINT32 devNum) 
{
    A_INT32 offset;
	A_UINT32 common_eeprom_data[0x150];
	A_UINT16 ii;
	A_UINT32 field_val = 0;

	//if new 11a mode flag is on, then clear the old 11a flag
	if(CalSetup.enableJapanMode11aNew) {
		CalSetup.Amode = 0;
	}

    common_eeprom_data[0xBF] = ((CalSetup.countryOrDomain & 0x1) << 15) | ((CalSetup.worldWideRoaming & 0x1) << 14) | 
							   (CalSetup.countryOrDomainCode & 0xfff) ;
    common_eeprom_data[0xC1] = (EEPROM_MAJOR_VERSION<<12) | EEPROM_MINOR_VERSION;
	
	common_eeprom_data[0xC2] = (CalSetup.turboDisable << 15) | ((CalSetup.RFSilent & 0x1) << 14) | 
							   ((CalSetup.deviceType & 0x7) << 11) | (((A_UINT16) (2*CalSetup.TurboMaxPower_5G) & 0x7f) << 4) |
							   ((CalSetup.turboDisable_11g & 0x1) << 3) | ((CalSetup.Gmode & 0x1) << 2) |
							   ((CalSetup.Bmode & 0x1) << 1) | (CalSetup.Amode & 0x1) ;
	common_eeprom_data[0xC3] = ((CalSetup.antennaGain5G & 0xff) << 8) | (CalSetup.antennaGain2p5G & 0xff) ;
	common_eeprom_data[0xC4] = ((CalSetup.eeprom_map & 0x3) << 14) | (CalSetup.EARStartAddr & 0xfff) ;
	if(CalSetup.xrDisable) {
		common_eeprom_data[0xc4] |= (0x3 << 12);
	}
	common_eeprom_data[0xC5] = 0x7FFF & (((CalSetup.Enable_32khz & 0x1) << 14) | (CalSetup.TrgtPwrStartAddr & 0xfff));								
	common_eeprom_data[0xC6] = (((configSetup.eepFileVersion & 0xFF) << 8) | ((configSetup.earFileVersion  & 0xFF) << 0)) ;
	//new to 5.4 eeprom version, put the build number in the first 8 bits (other bits are reserved)
	common_eeprom_data[0xC7] = ( ART_BUILD_NUM & 0xFF) << 0;
	common_eeprom_data[0xC8] = ((CalSetup.modeMaskForRadio[1] & 0x3) << 2) | ((CalSetup.modeMaskForRadio[0] & 0x3) << 0); 
		
	if(CalSetup.eeprom_map == CAL_FORMAT_GEN5) {
		common_eeprom_data[0xC8] = common_eeprom_data[0xC8] | ((CalSetup.calStartAddr & 0xfff) << 4);
	}

	for(offset=0xC9; offset < 0xD4; offset++) {
		common_eeprom_data[offset] = 0x0000 ;
	}

	//write the new capability fields
	common_eeprom_data[0xC9] = (CalSetup.compressionDisable << 0) | (CalSetup.aesDisable << 1) | (CalSetup.fastFrameDisable << 2);
	common_eeprom_data[0xc9] = common_eeprom_data[0xc9] | (CalSetup.burstingDisable << 3);
	common_eeprom_data[0xc9] = common_eeprom_data[0xc9] | ((CalSetup.maxNumQCU & 0x1f) <<  4);
	common_eeprom_data[0xc9] = common_eeprom_data[0xc9] | ((CalSetup.enableHeavyClip & 0x1) <<  9);
	common_eeprom_data[0xc9] = common_eeprom_data[0xc9] | ((CalSetup.keyCacheSize & 0xf) <<  12);

	//write the new regulatory flags
	//FALCON flags need to be written here
	common_eeprom_data[0xca] = common_eeprom_data[0xca] | (CalSetup.enableFCCMid << 6);
	common_eeprom_data[0xca] = common_eeprom_data[0xca] | (CalSetup.enableJapanEvenU1 << 7);
	common_eeprom_data[0xca] = common_eeprom_data[0xca] | (CalSetup.enableJapenU2 << 8);
	common_eeprom_data[0xca] = common_eeprom_data[0xca] | (CalSetup.enableJapnMid << 9);
	common_eeprom_data[0xca] = common_eeprom_data[0xca] | (CalSetup.enableJapanOddU1 << 10);
	common_eeprom_data[0xca] = common_eeprom_data[0xca] | (CalSetup.enableJapanMode11aNew << 11);

	// 11a block
	offset = 0xD4;
	common_eeprom_data[offset++] = ((CalSetup.switchSettling & 0x7f) << 8) | ((CalSetup.txrxAtten & 0x3f) << 2) |
									((CalSetup.antennaControl[0] >> 4) & 0x3) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl[0] & 0xf) << 12) | ((CalSetup.antennaControl[1] & 0x3f) << 6) | 
									(CalSetup.antennaControl[2] & 0x3f) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl[3] & 0x3f) << 10) | ((CalSetup.antennaControl[4] & 0x3f) << 4) | 
									((CalSetup.antennaControl[5] & 0x3f) >> 2) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl[5] & 0x3) << 14) | ((CalSetup.antennaControl[6] & 0x3f) << 8) | 
									((CalSetup.antennaControl[7] & 0x3f) << 2) | ((CalSetup.antennaControl[8] & 0x3f) >> 4) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl[8] & 0xf) << 12) | ((CalSetup.antennaControl[9] & 0x3f) << 6) | 
								    (CalSetup.antennaControl[10] & 0x3f) ;
	
	common_eeprom_data[offset++] = ((CalSetup.adcDesiredSize & 0xff) << 8) | ((CalSetup.ob_4 & 0x7) << 5) |
									((CalSetup.db_4 & 0x7) << 2) | ((CalSetup.ob_3 & 0x7) >> 1) ;
	common_eeprom_data[offset++] = ((CalSetup.ob_3 & 0x1) << 15) | ((CalSetup.db_3 & 0x7) << 12) | ((CalSetup.ob_2 & 0x7) << 9) |
									((CalSetup.db_2 & 0x7) << 6) | ((CalSetup.ob_1 & 0x7) << 3) | (CalSetup.db_1 & 0x7) ;
	common_eeprom_data[offset++] = ((CalSetup.txEndToXLNAOn & 0xff) << 8) | (CalSetup.thresh62 & 0xff) ;
	common_eeprom_data[offset++] = ((CalSetup.txEndToXPAOff & 0xff) << 8) | (CalSetup.txFrameToXPAOn & 0xff) ;
	common_eeprom_data[offset++] = ((CalSetup.pgaDesiredSize & 0xff) << 8) | (CalSetup.noisefloor_thresh & 0xff) ;
	common_eeprom_data[offset++] = ((CalSetup.fixed_bias[MODE_11a] & 0x1) << 13) | ((CalSetup.xlnaGain & 0xff) << 5) | 
									((CalSetup.xgain & 0xf) << 1) | (CalSetup.xpd & 0x1) ;
	if((CalSetup.Amode || CalSetup.enableJapanMode11aNew) && (!isGriffin(swDeviceID)) && !isEagle(swDeviceID)) {
		field_val = art_getFieldForMode(devNum, "rf_gain_I", MODE_11A, 0);
	}
	else {
		field_val = 0;
	}
	common_eeprom_data[offset++] = ( ((field_val & 0x7)  << 13) | ((CalSetup.falseDetectBackoff[MODE_11a] & 0x7f) << 6)  |
									 ((A_UINT16)(2*pTargetsSet->pTargetChannel[0].Target24) & 0x3F) ); // 11a XR target power. for the time being same as 6mbps.

	common_eeprom_data[offset++] = ((CalSetup.iqcal_i_corr[MODE_11a] & 0x3f) << 8) | ((CalSetup.iqcal_q_corr[MODE_11a] & 0x1f) << 3) |
								   ((field_val >> 3) & 0x7);
	common_eeprom_data[offset++] = (CalSetup.rxtx_margin[MODE_11a] & 0x3F) | ((CalSetup.switchSettling_Turbo[MODE_11a] & 0x7F) << 6) |
									(( CalSetup.txrxAtten_Turbo[MODE_11a] & 0x7) << 13);
	common_eeprom_data[offset++] = ((CalSetup.txrxAtten_Turbo[MODE_11a] >> 3) & 0x7) | 
								   ((CalSetup.rxtx_margin_Turbo[MODE_11a] & 0x3F) << 3) |
								   ((CalSetup.adcDesiredSize_Turbo[MODE_11a] & 0x7F) << 9);
	common_eeprom_data[offset++] = ((CalSetup.adcDesiredSize_Turbo[MODE_11a] >> 7) & 0x1) |
								   ((CalSetup.pgaDesiredSize_Turbo[MODE_11a] & 0xFF) << 1) |
								   ((CalSetup.pdGainOverlap & 0xf) << 9);

	if (offset != 0xE4) 
	{
		uiPrintf("offset expected to be 0xE4 at this point. [%x]\n", offset);
		exit(0);
	}

	for(offset=0xE4; offset < 0xF2; offset++) {
		common_eeprom_data[offset] = 0x0000 ; //reserved
	}

	// 11b block
	offset = 0xF2;
	common_eeprom_data[offset++] = ((CalSetup.switchSettling_2p4[MODE_11b] & 0x7f) << 8) | ((CalSetup.txrxAtten_2p4[MODE_11b] & 0x3f) << 2) |
								   ((CalSetup.antennaControl_2p4[MODE_11b][0] >> 4) & 0x3) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl_2p4[MODE_11b][0] & 0xf) << 12) | ((CalSetup.antennaControl_2p4[MODE_11b][1] & 0x3f) << 6) | 
								   (CalSetup.antennaControl_2p4[MODE_11b][2] & 0x3f) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl_2p4[MODE_11b][3] & 0x3f) << 10) | ((CalSetup.antennaControl_2p4[MODE_11b][4] & 0x3f) << 4) | 
								   ((CalSetup.antennaControl_2p4[MODE_11b][5] & 0x3f) >> 2) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl_2p4[MODE_11b][5] & 0x3) << 14) | ((CalSetup.antennaControl_2p4[MODE_11b][6] & 0x3f) << 8) | 
								   ((CalSetup.antennaControl_2p4[MODE_11b][7] & 0x3f) << 2) | ((CalSetup.antennaControl_2p4[MODE_11b][8] & 0x3f) >> 4) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl_2p4[MODE_11b][8] & 0xf) << 12) | ((CalSetup.antennaControl_2p4[MODE_11b][9] & 0x3f) << 6) | 
								   (CalSetup.antennaControl_2p4[MODE_11b][10] & 0x3f) ;
	common_eeprom_data[offset++] = ((CalSetup.adcDesiredSize_2p4[MODE_11b] & 0xff) << 8) | ((CalSetup.ob_2p4[MODE_11b] & 0x7) << 4) |
								   (CalSetup.db_2p4[MODE_11b] & 0x7)  ;
	common_eeprom_data[offset++] = ((CalSetup.txEndToXLNAOn_2p4[MODE_11b] & 0xff) << 8) | (CalSetup.thresh62_2p4[MODE_11b] & 0xff) ;
	common_eeprom_data[offset++] = ((CalSetup.txEndToXPAOff_2p4[MODE_11b] & 0xff) << 8) | (CalSetup.txFrameToXPAOn_2p4[MODE_11b] & 0xff) ;
	common_eeprom_data[offset++] = ((CalSetup.pgaDesiredSize_2p4[MODE_11b] & 0xff) << 8) | (CalSetup.noisefloor_thresh_2p4[MODE_11b] & 0xff) ;
	common_eeprom_data[offset++] = ((CalSetup.xlnaGain_2p4[MODE_11b] & 0xff) << 5) | ((CalSetup.xgain_2p4[MODE_11b] & 0xff) << 1) |
									(CalSetup.xpd_2p4[MODE_11b] & 0x1) ;
	
	if(CalSetup.Bmode && (!isGriffin(swDeviceID)) && !isEagle(swDeviceID)) {
		field_val = art_getFieldForMode(devNum, "rf_gain_I", MODE_11B, 0);
	}
	else {
		field_val = 0;
	}
	common_eeprom_data[offset++] = (CalSetup.b_ob_2p4[MODE_11b] & 0x7) | ((CalSetup.b_db_2p4[MODE_11b] & 0x7) << 3) |
									((CalSetup.falseDetectBackoff[MODE_11b] & 0x7f) << 6) | ((field_val & 0x7)  << 13) ;
	common_eeprom_data[offset++] = ((field_val >> 3) & 0x7);
	if ((CalSetup.forcePiers_2p4[MODE_11b]) || (CalSetup.eeprom_map == 1) || (CalSetup.eeprom_map == 2)){
			common_eeprom_data[offset++] = (freq2fbin(CalSetup.piersList_2p4[MODE_11b][0]) & 0xff) |
				(( ((CalSetup.numForcedPiers_2p4[MODE_11b] > 1) && !(CalSetup.eeprom_map == 2)) ? (freq2fbin(CalSetup.piersList_2p4[MODE_11b][1]) & 0xFF) : 0xFF) << 8);
			if ((CalSetup.numForcedPiers_2p4[MODE_11b] > 2) && !(CalSetup.eeprom_map == 2)) {
				common_eeprom_data[offset++] =  ( freq2fbin(CalSetup.piersList_2p4[MODE_11b][2]) & 0xFF) | ((CalSetup.rxtx_margin[MODE_11b] & 0x3F) << 8);		
			} else {
				common_eeprom_data[offset++] =  0xFF | ((CalSetup.rxtx_margin[MODE_11b] & 0x3F) << 8);
			}
	} else {
		common_eeprom_data[offset++] = 0xFFFF;
//		common_eeprom_data[offset++] = 0x00FF;
		common_eeprom_data[offset++] =  0xFF | ((CalSetup.rxtx_margin[MODE_11b] & 0x3F) << 8);
	}

	if (offset != 0x100) 
	{
		uiPrintf("offset expected to be 0x100 at this point. [%x]\n", offset);
		exit(0);
	}
 
	for(offset=0x100; offset < 0x10D; offset++) {
		common_eeprom_data[offset] = 0x0000 ; //reserved
	}

	// 11g block
	offset = 0x10D;
	common_eeprom_data[offset++] = ((CalSetup.switchSettling_2p4[MODE_11g] & 0x7f) << 8) | ((CalSetup.txrxAtten_2p4[MODE_11g] & 0x3f) << 2) |
								   ((CalSetup.antennaControl_2p4[MODE_11g][0] >> 4) & 0x3) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl_2p4[MODE_11g][0] & 0xf) << 12) | ((CalSetup.antennaControl_2p4[MODE_11g][1] & 0x3f) << 6) | 
								   (CalSetup.antennaControl_2p4[MODE_11g][2] & 0x3f) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl_2p4[MODE_11g][3] & 0x3f) << 10) | ((CalSetup.antennaControl_2p4[MODE_11g][4] & 0x3f) << 4) | 
								   ((CalSetup.antennaControl_2p4[MODE_11g][5] & 0x3f) >> 2) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl_2p4[MODE_11g][5] & 0x3) << 14) | ((CalSetup.antennaControl_2p4[MODE_11g][6] & 0x3f) << 8) | 
								   ((CalSetup.antennaControl_2p4[MODE_11g][7] & 0x3f) << 2) | ((CalSetup.antennaControl_2p4[MODE_11g][8] & 0x3f) >> 4) ;
	common_eeprom_data[offset++] = ((CalSetup.antennaControl_2p4[MODE_11g][8] & 0xf) << 12) | ((CalSetup.antennaControl_2p4[MODE_11g][9] & 0x3f) << 6) | 
								   (CalSetup.antennaControl_2p4[MODE_11g][10] & 0x3f) ;
	common_eeprom_data[offset++] = ((CalSetup.adcDesiredSize_2p4[MODE_11g] & 0xff) << 8) | ((CalSetup.ob_2p4[MODE_11g] & 0x7) << 4) |
								   (CalSetup.db_2p4[MODE_11g] & 0x7)  ;
	common_eeprom_data[offset++] = ((CalSetup.txEndToXLNAOn_2p4[MODE_11g] & 0xff) << 8) | (CalSetup.thresh62_2p4[MODE_11g] & 0xff) ;
	common_eeprom_data[offset++] = ((CalSetup.txEndToXPAOff_2p4[MODE_11g] & 0xff) << 8) | (CalSetup.txFrameToXPAOn_2p4[MODE_11g] & 0xff) ;
	common_eeprom_data[offset++] = ((CalSetup.pgaDesiredSize_2p4[MODE_11g] & 0xff) << 8) | (CalSetup.noisefloor_thresh_2p4[MODE_11g] & 0xff) ;
	common_eeprom_data[offset++] = ((CalSetup.fixed_bias[MODE_11g] & 0x1) << 13) | ((CalSetup.xlnaGain_2p4[MODE_11g] & 0xff) << 5) | 
									((CalSetup.xgain_2p4[MODE_11g] & 0xff) << 1) | (CalSetup.xpd_2p4[MODE_11g] & 0x1) ;
	if(CalSetup.Gmode && (!isGriffin(swDeviceID)) && !isEagle(swDeviceID)) {
		field_val = art_getFieldForModeChecked(devNum, "rf_gain_I", MODE_11G, 0);
	}
	else {
		field_val = 0;
	}
	common_eeprom_data[offset++] = (CalSetup.b_ob_2p4[MODE_11g] & 0x7) | ((CalSetup.b_db_2p4[MODE_11g] & 0x7) << 3) |
									((CalSetup.falseDetectBackoff[MODE_11g] & 0x7f) << 6) | ((field_val & 0x7)  << 13) ;
	common_eeprom_data[offset++] = (((field_val >> 3) & 0x7) | (((A_UINT16)(10*CalSetup.cck_ofdm_delta) & 0xFF) << 3) |
									(((A_UINT16)(10*CalSetup.ch14_filter_cck_delta) & 0x1F) << 11) );
	if ((CalSetup.forcePiers_2p4[MODE_11g]) || (CalSetup.eeprom_map == 1) || (CalSetup.eeprom_map == 2)){
		common_eeprom_data[offset++] = (freq2fbin(CalSetup.piersList_2p4[MODE_11g][0]) & 0xff) |
			(( ((CalSetup.numForcedPiers_2p4[MODE_11g] > 1) && !(CalSetup.eeprom_map == 2)) ? ( freq2fbin(CalSetup.piersList_2p4[MODE_11g][1]) & 0xFF) : 0xFF) << 8);
	} else {
		common_eeprom_data[offset++] = 0xFFFF;
	}
	// 11g XR power. for now same as 6mbps.
	common_eeprom_data[offset++] = ((((A_UINT16)(2*(pTargetsSet_2p4[MODE_11g]->pTargetChannel[0].Target24)) & 0x3F) << 7) | 
									 ((A_UINT16) (2*CalSetup.TurboMaxPower_11g) & 0x7F));
	//common_eeprom_data[offset++] =  ((0 & 0x3f) << 7) | ((A_UINT16) (2*CalSetup.TurboMaxPower_11g) & 0x7F);
	if ((CalSetup.forcePiers_2p4[MODE_11g]) || (CalSetup.eeprom_map == 1)) {
		if ((CalSetup.numForcedPiers_2p4[MODE_11g] > 2) && !(CalSetup.eeprom_map == 2)) {
			common_eeprom_data[offset++] =  ( freq2fbin(CalSetup.piersList_2p4[MODE_11g][2]) & 0xFF) | ((CalSetup.rxtx_margin[MODE_11g] & 0x3F) << 8);		
		} else {
			common_eeprom_data[offset++] =  0x00FF | ((CalSetup.rxtx_margin[MODE_11g] & 0x3F) << 8);
		}
	} else {
		common_eeprom_data[offset++] = 0x00FF;
	}
	common_eeprom_data[offset++] =  ((CalSetup.iqcal_i_corr[MODE_11g] & 0x3f) << 5) | (CalSetup.iqcal_q_corr[MODE_11g] & 0x1f) ;
	common_eeprom_data[offset++] =  (((A_INT8)(2*CalSetup.ofdm_cck_gain_delta)) & 0xFF) |
									((CalSetup.switchSettling_Turbo[MODE_11g] & 0x7F) << 8) |
									((CalSetup.txrxAtten_Turbo[MODE_11g] & 0x1) << 15); 
	common_eeprom_data[offset++] = ((CalSetup.txrxAtten_Turbo[MODE_11g] >> 1) & 0x1F) |
								   ((CalSetup.rxtx_margin_Turbo[MODE_11g] & 0x3F) << 5) |
								   ((CalSetup.adcDesiredSize_Turbo[MODE_11g] & 0x1F) << 11) ;
	common_eeprom_data[offset++] = ((CalSetup.adcDesiredSize_Turbo[MODE_11g] >> 5) & 0x7) |
								   ((CalSetup.pgaDesiredSize_Turbo[MODE_11g] & 0xFF) << 3);

	if (offset != 0x120) 
	{
		uiPrintf("offset expected to be 0x120 at this point. [%x]\n", offset);
		exit(0);
	}
 
	for(offset=0x120; offset < 0x128; offset++) {
		common_eeprom_data[offset] = 0x0000 ; // reserved
	}

	offset = 0x128;

	for(ii=0; ii<=(A_UINT16) (pTestGroupSet->numTestGroupsDefined / 2); ii++)
	{
		common_eeprom_data[offset++] = ((pTestGroupSet->pTestGroup[2*ii].TG_Code & 0xff) << 8) | (pTestGroupSet->pTestGroup[2*ii+1].TG_Code & 0xff);
	}
	

	while(offset <= 0x137) {
		common_eeprom_data[offset++] = 0x0000 ; // fill up undeclared CTLs with 0
	}
	
	offset = 0x138;
	while(offset <= 0x14F) {
		common_eeprom_data[offset++] = 0x0000 ;
	}

	write_spur_info(devNum, &common_eeprom_data[0]);

	//	art_eepromWriteBlock(devNum, 0xBF, (0x14F - 0xBF + 1), &(common_eeprom_data[0xBF])) ;
	virtual_eeprom0WriteBlock(devNum, 0xBF, (0x14F - 0xBF + 1), &(common_eeprom_data[0xBF])) ;

}

int write_spur_info_on_demand
(
 A_UINT32 devNum,
 A_UINT32 *common_eeprom_data
)
{
	A_UINT32	  i2, i5, eepromIndex;

	//if eagle or cobra, write spur info for 11g
	//if(isCobra_art(swDeviceID) || isEagle(swDeviceID))
	{
		eepromIndex = EEPROM_SPUR_11G_START;	//0x13d;
		for (i2 = 0; i2 < EEPROM_SPUR_11G_SIZE; i2++)
		{
			if (CalSetup.spur_frequency_2p4g[i2])
			{
				common_eeprom_data[eepromIndex++] = (CalSetup.spur_frequency_2p4g[i2] - 2300) * 10;
			}
			else
			{
				break;
			}
		}
		common_eeprom_data[eepromIndex] = 0x8000;
	}

	// if condor, write spur info for 11a
	//if(isCondor(swDeviceID))
	{
		eepromIndex = EEPROM_SPUR_11A_START;	//0x138;
		for (i5 = 0; i5 < EEPROM_SPUR_11A_SIZE; i5++)
		{
			if (CalSetup.spur_frequency_5g[i5])
			{
				common_eeprom_data[eepromIndex++] = (CalSetup.spur_frequency_5g[i5] - 4900) * 10;
			}
			else
			{
				break;
			}
		}
		common_eeprom_data[eepromIndex] = 0x8000;
	}

	return (i2 || i5);
}

void write_spur_info
(
 A_UINT32 devNum,
 A_UINT32 *common_eeprom_data
)
{
	A_UINT32      spurChans11G[] = {2464, 2420};
	A_UINT32      spurChans11A[] = {5800};
	A_UINT32	  i, eepromIndex;

	if (write_spur_info_on_demand(devNum, common_eeprom_data)) return;

	//there are no 11a spurs so set to no spurs
	common_eeprom_data[EEPROM_SPUR_11A_START] = 0x8000;

	//default to no 11g spurs
	common_eeprom_data[EEPROM_SPUR_11G_START] = 0x8000;

	//if eagle or cobra, write spur info for 11g
	if(isCobra(swDeviceID) || swDeviceID == SW_DEVICE_ID_EAGLE)
		
	{
		eepromIndex = EEPROM_SPUR_11G_START;
		for (i = 0; i < (sizeof(spurChans11G)/sizeof(A_UINT32)); i++) {
			common_eeprom_data[eepromIndex++] = (spurChans11G[i] - 2300) * 10;
		}
		common_eeprom_data[eepromIndex] = 0x8000;
	}
	// if condor, write spur info for 11a
	if(isCondor(swDeviceID))
	{
		eepromIndex = EEPROM_SPUR_11A_START;
		for (i = 0; i < (sizeof(spurChans11A)/sizeof(A_UINT32)); i++)
		{
			common_eeprom_data[eepromIndex++] = (spurChans11A[i] - 4900) * 10;
		}
		common_eeprom_data[eepromIndex] = 0x8000;
	}
	// if swan, write spur info for 11G
	if(isSwan(swDeviceID))
	{
		A_UINT32 spurChans11G_swan[5] = {2400,2420,2440, 2464,2480};
		eepromIndex = EEPROM_SPUR_11G_START;
		for (i = 0; i < (sizeof(spurChans11G_swan)/sizeof(A_UINT32)); i++)
		{
			common_eeprom_data[eepromIndex++] = (spurChans11G_swan[i] - 2300) * 10;
		}
		common_eeprom_data[eepromIndex] = 0x8000;
	}
	if(isNala(swDeviceID))
	{
		A_UINT32 spurChans11G_nala[5] = {2420, 2440, 2464, 2480};
		eepromIndex = EEPROM_SPUR_11G_START;
		for (i = 0; i < (sizeof(spurChans11G_nala)/sizeof(A_UINT32)); i++)
		{
			common_eeprom_data[eepromIndex++] = (spurChans11G_nala[i] - 2300) * 10;
		}
		common_eeprom_data[eepromIndex] = 0x8000;
	}

	return;
}

A_UINT32 parseTestChannels(FILE *fStream, char *pLine, A_UINT32 mode)
{
#define     BUF_SIZE  200
	char lineBuf[BUF_SIZE];
    A_UINT32 testVal;
	div_t res;
	A_UINT16	ii ;
	char		delimiters[] = " \t\n\r" ;
	char		endMarker[3][BUF_SIZE] = {"#END_11g_TEST_CHANNEL_MATRIX", 
									 "#END_11b_TEST_CHANNEL_MATRIX", 
									 "#END_11a_TEST_CHANNEL_MATRIX"};
	double tempChannel;

	ii=0;

	pTestSet[mode]->maxTestChannels = NUM_TEST_CHANNELS;
	pTestSet[mode]->pTestChannel	=  (TEST_CHANNEL_MASKS *)malloc(sizeof(TEST_CHANNEL_MASKS) * pTestSet[mode]->maxTestChannels) ;
	if (pTestSet[mode]->pTestChannel == NULL) {
		uiPrintf("Unable to allocate TestSet for mode %s\n", modeName[mode]);
		return(0);
	}

	while(fgets(lineBuf, BUF_SIZE - 1, fStream) != NULL) {

		pLine = lineBuf;
		pLine = strtok(pLine, delimiters);

		if(pLine == NULL) continue;

		if(strnicmp(endMarker[mode], pLine, strlen(endMarker[mode])) == 0)  {
			pTestSet[mode]->numTestChannelsDefined = ii ;
			return(1); //successfully read data for mode
		}

		if (pLine[0] == '#') {
			continue ;
		}

//		if (!sscanf(pLine, "%d", &(pTestSet[mode]->pTestChannel[ii].channelValue))) {
		if (!sscanf(pLine, "%lf", &(tempChannel))) {
			uiPrintf("Could not read the test frequency %d from token %s\n", ii, pLine);
		}
		else {
			pTestSet[mode]->pTestChannel[ii].channelValueTimes10 = (A_UINT32)(tempChannel * 10);
			if((mode == MODE_11a) && ((pTestSet[mode]->pTestChannel[ii].channelValueTimes10 % 25) != 0)) {
				uiPrintf("Illegal channel value %8.2f in token %s\n", tempChannel, pLine);
			}
		}

		pLine = strtok(NULL, delimiters);
		if (!sscanf(pLine, "%d", &(testVal))) {
			uiPrintf("Could not read the TEST_PER for test frequency %d from token %s\n", ii, pLine);
		} else
		{
			pTestSet[mode]->pTestChannel[ii].TEST_PER = (testVal > 0) ? TRUE : FALSE ;
		}

		pLine = strtok(NULL, delimiters);
		if (!sscanf(pLine, "%d", &(pTestSet[mode]->pTestChannel[ii].targetSensitivity))) {
			uiPrintf("Could not read the target sensitivity %d from token %s\n", ii, pLine);
		}

		pLine = strtok(NULL, delimiters);
		if (!sscanf(pLine, "%d", &(testVal))) {
			uiPrintf("Could not read the LO_RATE_SEN for test frequency %d from token %s\n", ii, pLine);
		} else
		{
			pTestSet[mode]->pTestChannel[ii].LO_RATE_SEN = (testVal > 0) ? TRUE : FALSE ;
		}

		pLine = strtok(NULL, delimiters);
		if (!sscanf(pLine, "%d", &(pTestSet[mode]->pTestChannel[ii].targetLoRateSensitivity))) {
			uiPrintf("Could not read the LO_RATE target sensitivity %d from token %s\n", ii, pLine);
		}

		if ((mode == MODE_11a)||(mode == MODE_11g))
		{
			pLine = strtok(NULL, delimiters);
			if (!sscanf(pLine, "%d", &(testVal))) {
				uiPrintf("Could not read the TEST_TURBO for test frequency %d from token %s\n", ii, pLine);
			} else
			{
				pTestSet[mode]->pTestChannel[ii].TEST_TURBO = (testVal > 0) ? TRUE : FALSE ;
			}
		} else
		{
			pTestSet[mode]->pTestChannel[ii].TEST_TURBO = FALSE;
		}
		
		if (mode == MODE_11a)
		{
			pLine = strtok(NULL, delimiters);
			if(pLine == NULL) {
			//if (!sscanf(pLine, "%d", &(testVal))) {
				uiPrintf("Could not read the TEST_HALF_QUART_RATE for test frequency %d from token %s\n", ii, pLine);
			} else
			{
				pTestSet[mode]->pTestChannel[ii].TEST_HALF_QUART_RATE = (A_UINT16) strtoul(pLine, NULL, 2);
				//pTestSet[mode]->pTestChannel[ii].TEST_HALF_QUART_RATE = (testVal > 0) ? TRUE : FALSE ;
			}
		} else
		{
			pTestSet[mode]->pTestChannel[ii].TEST_HALF_QUART_RATE = 0;
		}

		pLine = strtok(NULL, delimiters);
		if(pLine == NULL) {
//		if (!sscanf(pLine, "%d", &(testVal))) {
			uiPrintf("Could not read the TEST_MASK for test frequency %d from token %s\n", ii, pLine);
		} else
		{
			pTestSet[mode]->pTestChannel[ii].TEST_MASK = (A_UINT16) strtoul(pLine, NULL, 2);
			//pTestSet[mode]->pTestChannel[ii].TEST_MASK = (testVal > 0) ? TRUE : FALSE ;
		}

		if (mode == MODE_11a)
		{
			pLine = strtok(NULL, delimiters);
			if (!sscanf(pLine, "%d", &(testVal))) {
				uiPrintf("Could not read the TEST_OBW for test frequency %d from token %s\n", ii, pLine);
			} else
			{
				pTestSet[mode]->pTestChannel[ii].TEST_OBW = (testVal > 0) ? TRUE : FALSE ;
			}
		} else
		{
			pTestSet[mode]->pTestChannel[ii].TEST_OBW = FALSE;
		}

		pLine = strtok(NULL, delimiters);
		if (pLine != NULL)
		{
			pTestSet[mode]->pTestChannel[ii].TEST_TGT_PWR = (A_UINT16) strtoul(pLine, NULL, 2);
		} else
		{
			pTestSet[mode]->pTestChannel[ii].TEST_TGT_PWR = 0;
		}

		if (pLine == NULL)
		{
			uiPrintf("Need to specify TEMP_MARGIN_TEST flags in the test channel matrix\n");
			pTestSet[mode]->pTestChannel[ii].TEST_TEMP_MARGIN = 0;
		} else
		{
			pLine = strtok(NULL, delimiters);
			if (!sscanf(pLine, "%d", &(testVal))) {
				uiPrintf("Could not read the TEST_TEMP_MARGIN for test frequency %d from token %s\n", ii, pLine);
			} else
			{
				pTestSet[mode]->pTestChannel[ii].TEST_TEMP_MARGIN = (testVal > 0) ? TRUE : FALSE ;
			}
		}
		pLine = strtok(NULL, delimiters);
		if (pLine != NULL)
		{
			if (!sscanf(pLine, "%d", &(testVal))) {
				uiPrintf("Could not read the TP TEST for test frequency %d from token %s\n", ii, pLine);
			} else
			{
				pTestSet[mode]->pTestChannel[ii].TEST_THROUGHPUT = (testVal > 0) ? TRUE : FALSE ;

				//Get the throughput threshold next
				pLine = strtok(NULL, delimiters);
				if(pLine == NULL) {
					uiPrintf("Need to specify throughput threshold in the test channel matrix\n");
				} else {
					if(!sscanf(pLine, "%lf", &(pTestSet[mode]->pTestChannel[ii].throughputThreshold) )) {
						uiPrintf("Could not read throughput threshold for test frequency %d from token %s\n", ii, pLine);
					}
				}
			}
		} else
		{
			pTestSet[mode]->pTestChannel[ii].TEST_THROUGHPUT = 0;
		}


		pLine = strtok(NULL, delimiters);
		if (pLine != NULL)
		{
			if (!sscanf(pLine, "%d", &(testVal))) {
				uiPrintf("Could not read the TP RX TEST for test frequency %d from token %s\n", ii, pLine);
			} else
			{
				pTestSet[mode]->pTestChannel[ii].TEST_THROUGHPUT_RX = (testVal > 0) ? TRUE : FALSE ;

				//Get the throughput threshold next
				pLine = strtok(NULL, delimiters);
				if(pLine == NULL) {
					uiPrintf("Need to specify rx throughput threshold in the test channel matrix\n");
				} else {
					if(!sscanf(pLine, "%lf", &(pTestSet[mode]->pTestChannel[ii].throughputRxThreshold) )) {
						uiPrintf("Could not read rx throughput threshold for test frequency %d from token %s\n", ii, pLine);
					}
				}

			}
		} else
		{
			pTestSet[mode]->pTestChannel[ii].TEST_THROUGHPUT_RX = 0;
		}
		
		/*		pLine = strtok(NULL, delimiters);
		if (pLine != NULL)
		{
			if (!sscanf(pLine, "%d", &(testVal))) {
				uiPrintf("Could not read the TP TURBO TEST for test frequency %d from token %s\n", ii, pLine);
			} else
			{
				pTestSet[mode]->pTestChannel[ii].TEST_TURBO_THROUGHPUT = (testVal > 0) ? TRUE : FALSE ;

				//Get the throughput threshold next
				pLine = strtok(NULL, delimiters);
				if(pLine == NULL) {
					uiPrintf("Need to specify throughput turbo threshold in the test channel matrix\n");
				} else {
					if(!sscanf(pLine, "%lf", &(pTestSet[mode]->pTestChannel[ii].throughputTurboThreshold) )) {
						uiPrintf("Could not read throughput threshold for test frequency %d from token %s\n", ii, pLine);
					}
				}

			}
		} else
		{
			pTestSet[mode]->pTestChannel[ii].TEST_TURBO_THROUGHPUT = 0;
		}
*/

		if ((mode == MODE_11a)||(mode == MODE_11g))
		{
			pLine = strtok(NULL, delimiters);
			if (pLine != NULL)
			{
				if (!sscanf(pLine, "%d", &(testVal))) {
					uiPrintf("Could not read the Channel accuracy for test frequency %d from token %s\n", ii, pLine);
				} else
				{
					pTestSet[mode]->pTestChannel[ii].TEST_CHANNEL_ACCURACY = (testVal > 0) ? TRUE : FALSE ;
				}
			} else {
				pTestSet[mode]->pTestChannel[ii].TEST_CHANNEL_ACCURACY = 0;
			}
		} else {
				pTestSet[mode]->pTestChannel[ii].TEST_CHANNEL_ACCURACY = 0;
		}		

		if ((mode == MODE_11a)||(mode == MODE_11g))
		{
			pLine = strtok(NULL, delimiters);
			if (pLine != NULL)
			{
				if (!sscanf(pLine, "%d", &(testVal))) {
					uiPrintf("Could not read the Aux antenna for test frequency %d from token %s\n", ii, pLine);
				} else
				{
					pTestSet[mode]->pTestChannel[ii].TEST_AUX_ANTENNA = (testVal > 0) ? TRUE : FALSE ;
				}
			} else{
				pTestSet[mode]->pTestChannel[ii].TEST_AUX_ANTENNA = 0;
			}
		} else{
			pTestSet[mode]->pTestChannel[ii].TEST_AUX_ANTENNA = 0;
		}
		
		pLine = strtok(NULL, delimiters);
		if (pLine != NULL)
		{
			if(!sscanf(pLine, "%d", &(testVal)))
			{
				uiPrintf("Could not read the Current Consumption for test frequency %d from token %s\n", ii, pLine);
			}
			else
			{				
				res = div(testVal, 2);
				pTestSet[mode]->pTestChannel[ii].TEST_CURR_CNSMPN_RX = res.quot? TRUE: FALSE;	
				pTestSet[mode]->pTestChannel[ii].TEST_CURR_CNSMPN_TX = res.rem? TRUE: FALSE;
			}
		}
		else
		{
			pTestSet[mode]->pTestChannel[ii].TEST_CURR_CNSMPN_TX = 
				pTestSet[mode]->pTestChannel[ii].TEST_CURR_CNSMPN_RX = FALSE;
		}

		pLine = strtok(NULL, delimiters);
		if(pLine != NULL) 
        {
			if(!sscanf(pLine, "%d", &(pTestSet[mode]->pTestChannel[ii].mask_fail_threshold) )) {
				uiPrintf("Could not read fail limit on spec. mask for test frequency %d from token %s\n", ii, pLine);
			}
		}
		else 
		{
			uiPrintf("Need to specify fail point limit in the test channel matrix\n");
		}
		pLine = strtok(NULL, delimiters);
		if (pLine != NULL)
		{
			pTestSet[mode]->pTestChannel[ii].TEST_ZERO_POWER = (A_UCHAR) strtoul(pLine, NULL, 2);;
		} else{
			pTestSet[mode]->pTestChannel[ii].TEST_ZERO_POWER = 0;
		}
		ii++ ;
	}

	return(0); // shouldn't have ended up here.

}

/*A_BOOL eeprom_verify_checksum (A_UINT32 devNum)
{
	A_UINT32	readChecksum, computedChecksum;
	A_UINT32	length;
	
	length = art_eepromRead(devNum, 0x1B);
	if((length == 0x0) ||(length == 0xffff))
		length = 0x400;

//	uiPrintf(" length in eeprom_verify_checkSum = %x\n",length);
	
	readChecksum = art_eepromRead(devNum, 0xC0);

//	uiPrintf(" Read CheckSum in eeprom_verify_checkSum = %x\n",readChecksum);

	length = length - 0xC1;

	if(length == 0xffff)
	{
		uiPrintf(" Invalid Length need calibration \n");
		return FALSE;
	}
	computedChecksum = eeprom_get_checksum(devNum, 0xC1, length);
	return (readChecksum == computedChecksum);
}

A_UINT32 eeprom_get_checksum(A_UINT32 devNum, A_UINT16 startAddr, A_UINT32 numWords ) 
{
	A_UINT32 i,xor_data=0;
	A_UINT16 addr;
//	A_UINT32 block_rd[2048] ;
	A_UINT32 *block_rd;
	
	
	block_rd = (A_UINT32 *)calloc(numWords,sizeof(A_UINT32));
	if(block_rd == NULL)
	{
			printf(" Not able to allocate memory in eeprom_get_checksum\n");
			exit(1);
	}
	for(i=0;i<numWords;i++)
		block_rd[i]=0xffff;

	art_eepromReadBlock(devNum, startAddr, numWords, block_rd);
	for (addr=0;addr<numWords;addr++) {
		xor_data ^= block_rd[addr];
	}
    xor_data = ~xor_data & 0xffff;
		free(block_rd);
	return(xor_data);
}*/

A_UINT32 dataArr_get_checksum(A_UINT32 devNum, A_UINT16 startAddr, A_UINT16 numWords, A_UINT32 *data) 
{
    A_UINT32 xor_data=0;
	A_UINT16 addr;
	
	for (addr=startAddr;addr<(numWords+startAddr);addr++) {
		xor_data ^= (A_UINT16)(data[addr] & 0xffff);
	}
    xor_data = ~xor_data & 0xffff;
	return(xor_data);
}

A_UINT32 virtual_eeprom_get_checksum(A_UINT32 devNum, A_UINT16 startAddr, A_UINT16 numWords, A_UINT32 eep_blk) 
{
	A_UINT32      chksum, i;
	A_UINT16      ii;
	//A_UINT32      data_arr[0x400];
	A_UINT32 *data_arr;

	data_arr = (A_UINT32 *)calloc(eepromSize,sizeof(A_UINT32));
	if(data_arr == NULL)
	{
			printf(" Memory Not Allocated in virtual_eeprom_get_checksum\n");
			exit(1);
	}
	
	for(i=0;i<eepromSize;i++)
		data_arr[i]=0xffff;

	//assert(startAddr < 0x400);
	assert(startAddr <  eepromSize);	

	//	uiPrintf(" Number of words = %x",numWords);
	
	assert((startAddr + numWords - 1) < eepromSize);

	//for (ii=0; ii<0x400; ii++) 
	for (ii=startAddr; ii < (A_UINT16)(startAddr + numWords); ii++) 
	{
		data_arr[ii] = EEPROM_DATA_BUFFER[eep_blk][ii];
	}
	chksum = dataArr_get_checksum(devNum, startAddr, numWords, data_arr);
	
	//	printf("\n check sum = %x \n",chksum);
	
	free(data_arr);
	return(chksum);
}

void checkUserSize(A_UINT32 devNum)
{

	if(userEepromSize == eepromSize) {
		if( userEepromSize < checkSumLength){
			checkSumLength = eepromSize;
			sizeWarning =1;
		}
		else{			
//			uiPrintf(" NO change inthe checkSumLength\n");
		}
	}

	if(userEepromSize < eepromSize)	{
		if(userEepromSize < checkSumLength)	{
			sizeWarning =1;			
			uiPrintf(" !! WARNING  THE EEP SIZE IS LESS THAN EEPSIZE\n");
			uiPrintf(" !!  EEP SIZE IS %x LESS THAN EEPSIZE %x\n",userEepromSize,eepromSize);
			eepromSize = userEepromSize;		
		}
		else {
			if(userEepromSize >= checkSumLength) {
				uiPrintf(" !! WARNING  THE EEP SIZE IS LESS THAN EEPSIZE\n");
				uiPrintf(" !!  EEP SIZE IS %x LESS THAN EEPSIZE %x\n",userEepromSize,eepromSize);
				eepromSize = userEepromSize;			
				uiPrintf(" The condtion will not be met\n");
			}
		}
	}	

	if(userEepromSize > eepromSize)	{
		if(userEepromSize < checkSumLength)	{
			eepromSize = userEepromSize;
			sizeWarning =1;
			uiPrintf( " Condition will not arise\n");
		}
		else {
			eepromSize = userEepromSize;
		}
	}
}

/*A_UINT32 get_eeprom_size(A_UINT32 devNum,A_UINT32 *eepromSize, A_UINT32 *checkSumLength)
{
	A_UINT32 eepromLower,eepromUpper;
	A_UINT32	size = 0 , length =0,checkSumLengthError=0;

	
	eepromLower = art_eepromRead(devNum,0x1B);
	eepromUpper = art_eepromRead(devNum,0x1C);
	

//	printf("eepromLower = %x\n",eepromLower);
//	printf("eepromUpper = %x\n",eepromUpper);


	if((eepromUpper == 0x0000)|| (eepromLower == 0x0000)){
			length = 0x400;
			size = 0x400;
	}
	if((eepromUpper == 0xffff) || (eepromLower == 0xffff))
	{
		length = 0x400;
		size = 0x400;
	}

	if(length!=0x400)
			{
				size = eepromUpper & 0x1f;
				size = 2 << (size + 9);
				size = size/2;
				eepromUpper = (eepromUpper & 0xffe0) >> 5 ;
				length = eepromUpper | eepromLower;	
			}

	if(length > size) 
	{
		printf("|||| WARNING CHECKSUM LENGTH IS GREATER THEAN EEPROM SIZE\n");
		checkSumLengthError =1;
		length = 0x400;
	}
		
	if(!checkSumLengthError) 
	{
		*checkSumLength = length;
//		printf("CheckSumLength in get_eeprom_size() = %x\n",*checkSumLength);
		*eepromSize = size;
//		printf("CheckSumLength in get_eeprom_size() = %x\n",*checkSumLength);
		return 0;
	}
	else
	{
		*checkSumLength = length;
//		printf(" IN ELSE CheckSumLength in get_eeprom_size() = %x\n",*checkSumLength);
		*eepromSize = size;
//		printf("IN ELSE CheckSumLength in get_eeprom_size() = %x\n",*checkSumLength);
		return 1;
	}
}*/
