/* mDevtbl_rel.c - contians configuration table for all the devices supported by devlib */
/* Copyright (c) 2001 Atheros Communications, Inc., All Rights Reserved */

#if 0
#ident  "ACI $Id: //depot/sw/branches/ART_V53/sw/src/dk/mdk/devlib/mDevtbl_rel.c#14 $, $Header: //depot/sw/branches/ART_V53/sw/src/dk/mdk/devlib/mDevtbl_rel.c#14 $"
#endif


#ifdef VXWORKS
#include "vxworks.h"
#endif

#ifdef __ATH_DJGPPDOS__
#include <unistd.h>
#ifndef EILSEQ  
    #define EILSEQ EIO
#endif	// EILSEQ

#define __int64	long long
typedef unsigned long DWORD;
#define Sleep	delay
#endif	// #ifdef __ATH_DJGPPDOS__

#include "wlantype.h"
#include "athreg.h"
#include "manlib.h"
#include "mDevtbl.h"
#include "mData210.h"
#include "mCfg210.h"
#include "mData211.h"
#include "mCfg211.h"
#include "mData212.h"
#include "mCfg212.h"
#include "mCfg212d.h"
#include "mAni212.h"
#include "mCfg413.h"
#include "mIds.h"


#ifndef __ATH_DJGPPDOS__
//static ATHEROS_REG_FILE boss_0012[] = {	//new version 2 ini file
//#include "dk_boss_0012.ini"
//};
//static MODE_INFO boss_0012_mode[] = {	//new version 2 mode ini file
//#include "dk_boss_0012.mod"
//};

//static ATHEROS_REG_FILE venice[] = {	//new version 2 ini file
//#include "dk_boss_0013.ini"
//};
//static MODE_INFO venice_mode[] = {	//new version 2 mode ini file
//#include "dk_boss_0013.mod"
//};


//static ATHEROS_REG_FILE venice_derby2_1[] = {	//new version 2 ini file
//#include "dk_0016_2_1.ini"
//};
//static MODE_INFO venice_derby2_1_mode[] = {	//new version 2 mode ini file
//#include "dk_0016_2_1.mod"
//};

//	static ATHEROS_REG_FILE venice_derby2_1_ear[] = {	//new version 2 ini file
//	#include "dk_0016_2_1_ear.ini"
//};
//	static MODE_INFO venice_derby2_1_mode_ear[] = {	//new version 2 mode ini file
//	#include "dk_0016_2_1_ear.mod"
//};

	static ATHEROS_REG_FILE hainan_derby2_1[] = {	//new version 2 ini file
	#include "dk_0017_2_1.ini"
};
	static MODE_INFO hainan_derby2_1_mode[] = {	//new version 2 mode ini file
	#include "dk_0017_2_1.mod"
};
static ATHEROS_REG_FILE hainan_derby2_1_ear[] = {	//new version 2 ini file
#include "dk_0017_2_1_ear.ini"
};

static MODE_INFO hainan_derby2_1_mode_ear[] = {	//new version 2 mode ini file
#include "dk_0017_2_1_ear.mod"
};


static ATHEROS_REG_FILE predator_derby2_1[] = {	//new version 2 ini file
#include "dk_00b0_2_1.ini"
};
static MODE_INFO predator_derby2_1_mode[] = {	//new version 2 mode ini file
#include "dk_00b0_2_1.mod"
};
static ATHEROS_REG_FILE griffin2[] = {	//new version 2 ini file
#include "dk_0018_2.ini"
};
static MODE_INFO griffin2_mode[] = {	//new version 2 mode ini file
#include "dk_0018_2.mod"
};

#endif //__ATH_DJGPPDOS__

static ATHEROS_REG_FILE eagle2[] = {	//new version 2 ini file
#include "dk_0019_2.ini"
};
static MODE_INFO eagle2_mode[] = {	//new version 2 mode ini file
#include "dk_0019_2.mod"
};

static ATHEROS_REG_FILE condor[] = {	//new version 2 ini file
#include "dk_0020.ini"
};
static MODE_INFO condor_mode[] = {	//new version 2 mode ini file
#include "dk_0020.mod"
};

static ATHEROS_REG_FILE swan[] = {	//new version 2 ini file
#include "dk_0025.ini"
};
static MODE_INFO swan_mode[] = {	//new version 2 mode ini file
#include "dk_0025.mod"
};

static ATHEROS_REG_FILE swan_hp63040[] = {	//new version 2 ini file
#include "dk_0025_hb63040.ini"
};
static MODE_INFO swan_mode_hp63040[] = {	//new version 2 mode ini file
#include "dk_0025_hb63040.mod"
};
DEVICE_PATCH_IDATA swan_hp63_patch[] =
{
	{swan, swan_mode},
	{swan_hp63040, swan_mode_hp63040}
};
#define SWAN_HP63_PATCH_SIZE	(sizeof(swan_hp63_patch)/sizeof(DEVICE_PATCH_IDATA))


static ATHEROS_REG_FILE nala[] =
{
#include "dk_0026.ini"
};
static MODE_INFO nala_mode[] = 
{
#include "dk_0026.mod"
};


#ifdef DOS_CLIENT
static ATHEROS_REG_FILE hainan_derby2_1[] = {	//new version 2 ini file
#include "dk_0017_2_1.ini"
};
static MODE_INFO hainan_derby2_1_mode[] = {	//new version 2 mode ini file
#include "dk_0017_2_1.mod"
};

static ATHEROS_REG_FILE griffin2[] = {	//new version 2 ini file
#include "dk_0018_2.ini"
};
static MODE_INFO griffin2_mode[] = {	//new version 2 mode ini file
#include "dk_0018_2.mod"
};
#endif

#ifndef __ATH_DJGPPDOS__
#if 0
static MAC_API_TABLE maui2API = {
	macAPIInitAr5211,
	eepromReadAr5211,		
	eepromWriteAr5211,
	
	hwResetAr5211,
	pllProgramAr5211,
	
	setRetryLimitAllAr5211,
	setupAntennaAr5211,
	sendTxEndPacketAr5211,
	setDescriptorAr5211,
	setStatsPktDescAr5211,
	setContDescriptorAr5211,
	txBeginConfigAr5211,
	txBeginContDataAr5211,
	txBeginContFramedDataAr5211,
	txEndContFramedDataAr5211,
	beginSendStatsPktAr5211,
	writeRxDescriptorAr5211,
	rxBeginConfigAr5211,
	rxCleanupConfigAr5211,
	txCleanupConfigAr5211,
	txGetDescRateAr5211,
	setPPM5211,
	isTxdescEvent5211,
	isRxdescEvent5211,
	isTxComplete5211,
	enableRx5211,
	disableRx5211,
	setQueueAr5211,
	mapQueueAr5211,
	clearKeyCacheAr5211,
	AGCDeafAr5211,
	AGCUnDeafAr5211
};
#endif

static MAC_API_TABLE veniceAPI = {
	macAPIInitAr5212,
	eepromReadAr5211,		
	eepromWriteAr5211,
	
	hwResetAr5211,
	pllProgramAr5212,
	
	setRetryLimitAllAr5211,
	setupAntennaAr5211,
	sendTxEndPacketAr5211,
	setDescriptorAr5212,
	setStatsPktDescAr5212,
	setContDescriptorAr5212,
	txBeginConfigAr5211,
	txBeginContDataAr5211,
	txBeginContFramedDataAr5211,
	txEndContFramedDataAr5211,
	beginSendStatsPktAr5211,
	writeRxDescriptorAr5211,
	rxBeginConfigAr5212,
	rxCleanupConfigAr5211,
	txCleanupConfigAr5211,
	txGetDescRateAr5212,
	setPPM5211,
	isTxdescEvent5211,
	isRxdescEvent5211,
	isTxComplete5211,
	enableRx5211,
	disableRx5211,
	setQueueAr5211,
	mapQueueAr5211,
	clearKeyCacheAr5211,
	AGCDeafAr5211,
	AGCUnDeafAr5211
};
#endif

static MAC_API_TABLE eagleAPI = {
	macAPIInitAr5212,
	eepromReadAr5211,		
	eepromWriteAr5211,

	hwResetAr5211,
	pllProgramAr5413,
	
	setRetryLimitAllAr5211,
	setupAntennaAr5211,
	sendTxEndPacketAr5211,
	setDescriptorAr5212,
	setStatsPktDescAr5212,
	setContDescriptorAr5212,
	txBeginConfigAr5211,
	txBeginContDataAr5211,
	txBeginContFramedDataAr5211,
	txEndContFramedDataAr5211,
	beginSendStatsPktAr5211,
	writeRxDescriptorAr5211,
	rxBeginConfigAr5212,
	rxCleanupConfigAr5211,
	txCleanupConfigAr5211,
	txGetDescRateAr5212,
	setPPM5211,
	isTxdescEvent5211,
	isRxdescEvent5211,
	isTxComplete5211,
	enableRx5211,
	disableRx5211,
	setQueueAr5211,
	mapQueueAr5211,
	clearKeyCacheAr5211,
	AGCDeafAr5211,
	AGCUnDeafAr5211
};

#ifndef __ATH_DJGPPDOS__
#if 0
static RF_API_TABLE sombreroAPI = {
	initPowerAr5211,
	setSinglePowerAr5211,
	setChannelAr5211
};

static RF_API_TABLE sombrero_beanieAPI = {
	initPowerAr5211,
	setSinglePowerAr5211,
	setChannelAr5211_beanie
};
#endif

static RF_API_TABLE derbyAPI = {
	initPowerAr5212,
	setSinglePowerAr5211,
	setChannelAr5212
};
#endif

static RF_API_TABLE griffinRfAPI = {
	initPowerAr2413,
	setSinglePowerAr5211,
	setChannelAr2413
};


static ART_ANI_API_TABLE veniceArtAniAPI = {
	configArtAniLadderAr5212,
	enableArtAniAr5212,
	disableArtAniAr5212,
	setArtAniLevelAr5212,
	setArtAniLevelMaxAr5212,
	setArtAniLevelMinAr5212,
	incrementArtAniLevelAr5212,
	decrementArtAniLevelAr5212,
	getArtAniLevelAr5212,
	measArtAniFalseDetectsAr5212,
	isArtAniOptimizedAr5212,
	getArtAniFalseDetectsAr5212,
	setArtAniFalseDetectIntervalAr5212,
	programCurrArtAniLevelAr5212
};

#ifndef __ATH_DJGPPDOS__
ANALOG_REV sombreroRevs[] = {
	{1, 0x5},
	{1, 0x6},
	{1, 0x7}
};
const A_UINT16 numSombreroRevs = sizeof(sombreroRevs)/sizeof(ANALOG_REV);

ANALOG_REV derby2_1Revs[] = {
	{3, 0x6},
	{4, 0x6},
	{0, 0}   // dummy to handle AP31 with no 5G derby
};
#define numderby2_1Revs (sizeof(derby2_1Revs)/sizeof(ANALOG_REV))

const A_UINT16 veniceRevs[] = {
	0x50, 0x51, 0x53, 0x56
};

const A_UINT16 numVeniceRevs = sizeof(veniceRevs)/sizeof(A_UINT16);

const A_UINT16 hainanRevs[] = {
	0x55,
	0x59
};

#define numHainanRevs (sizeof(hainanRevs)/sizeof(A_UINT16))

ANALOG_REV griffinAnalogRevs[] = {
	{5, 0x1},
	{5, 0x2},
};
const A_UINT16 numGriffinAnalogRevs = sizeof(griffinAnalogRevs)/sizeof(ANALOG_REV);

A_UINT16 griffinRevs[] = {
	0x74,  // griffin 1.0
    0x75,  // griffin 1.1
    0x76,  // griffin 2.0
	0x78,  //griffin lite
	0x79   //griffin 2.1
};
#define numGriffinRevs (sizeof(griffinRevs)/sizeof(A_UINT16))

const ANALOG_REV griffin2_AnalogRevs[] = {
	{5, 0x5},
	{5, 0x6},
};
#define numGriffin2_AnalogRevs (sizeof(griffin2_AnalogRevs)/sizeof(ANALOG_REV))

const A_UINT16 predatorRevs[] = {
	0x80,  // Predator 1.0
	0x81,  // Predator 1.1
};

#define numPredatorRevs (sizeof(predatorRevs)/sizeof(A_UINT16))
#endif //__ATH_DJGPPDOS__

const A_UINT16 eagle2Revs[] = {
	0xa2,  // eagle 2.0
	0xa3,  // eagle 2.0
	0xa4,  // eagle 2.1 lite
	0xa5,  // eagle 2.1 super
};

#define numEagle2Revs (sizeof(eagle2Revs)/sizeof(A_UINT16))

A_UINT16 nalaRevs[] =
{
	0xf0
};

#define numNalaRevs (sizeof(nalaRevs) / sizeof(A_UINT16))

ANALOG_REV eagle2AnalogRevs[] = {
	{6, 0x2},
	{6, 0x3},
	{0xb, 0},
};
#define numEagle2AnalogRevs (sizeof(eagle2AnalogRevs)/sizeof(ANALOG_REV))

A_UINT16 condorRevs[] = {
	0x9a,  //condor 2.0 lite
	0x9b,  //condor 2.0 full
	0x98,  //condor 2.0 lite 2424
	0x99,  //condor 2.0 full 2424
	0x9c,  //future proof?
	0x9d,  //future proof?
	0x9e,  //future proof?
	0x9f,  //future proof?
	0xa0,  //hawk 3.2 lite g only
	0xa1,  //hawk 3.2 full g only
	0xa2,  //condor 3.2 lite
	0xa3,  //condor 3.2 full
};

#define numCondorRevs (sizeof(condorRevs)/sizeof(A_UINT16))

ANALOG_REV condorAnalogRevs[] = {
	{7, 0x1},
	{0xa, 0x2},
};
#define numCondorAnalogRevs (sizeof(condorAnalogRevs)/sizeof(ANALOG_REV))

A_UINT16 swanRevs[] = {
    0xa0, //swan 1.1
    0xe0, //swan 1.2
	0xe1, //swan 1.3
	0xe2, //swan 1.4
};

#define numSwanRevs (sizeof(swanRevs)/sizeof(A_UINT16))

ANALOG_REV swanAnalogRevs[] = {	
    {0, 0},
};

#define numSwanAnalogRevs (sizeof(swanAnalogRevs)/sizeof(ANALOG_REV))


#ifndef __ATH_DJGPPDOS__
DEVICE_INIT_DATA ar5kInitData[] = {
//	{0x0012, NULL, 0, NULL, 0, 0x0012,								//Identifiers
//	 boss_0012, sizeof(boss_0012)/sizeof(ATHEROS_REG_FILE),			//register file
//	 &maui2API, &sombrero_beanieAPI, &veniceArtAniAPI,				//APIs
//	 2, boss_0012_mode, sizeof(boss_0012_mode)/sizeof(MODE_INFO),   //Mode file
//	 CFG_VERSION_STRING_0012 },										//configuraton string

//	{0xff12, NULL, 0, NULL, 0, 0x0012,						    	//Identifiers
//	 boss_0012, sizeof(boss_0012)/sizeof(ATHEROS_REG_FILE),         //Register file
//	 &maui2API, &sombrero_beanieAPI, &veniceArtAniAPI,              //APIs
//	 2, boss_0012_mode, sizeof(boss_0012_mode)/sizeof(MODE_INFO),   //Mode file
//	 CFG_VERSION_STRING_0012 },                                     //configuration string


//			{DONT_MATCH, derby2_1Revs, numderby2_1Revs, veniceRevs, numVeniceRevs, SW_DEVICE_ID_VENICE_DERBY2, //Identifiers
//	 venice_derby2_1,  sizeof(venice_derby2_1)/sizeof(ATHEROS_REG_FILE),		//Register file
//	 &veniceAPI, &derbyAPI,	&veniceArtAniAPI,							//APIs
//	 3, venice_derby2_1_mode, sizeof(venice_derby2_1_mode)/sizeof(MODE_INFO), //Mode file
//	 CFG_VERSION_STRING_d016 },											//configuration string

	//will only point to this structure if want to load EAR from EEPROM
//			{DONT_MATCH, derby2_1Revs, numderby2_1Revs, veniceRevs, numVeniceRevs, SW_DEVICE_ID_VENICE_DERBY2, //Identifiers
//			 venice_derby2_1_ear,  sizeof(venice_derby2_1_ear)/sizeof(ATHEROS_REG_FILE),		//Register file
//	 &veniceAPI, &derbyAPI,	&veniceArtAniAPI,								//APIs
//			 3, venice_derby2_1_mode_ear, sizeof(venice_derby2_1_mode_ear)/sizeof(MODE_INFO), //Mode file
//			 CFG_VERSION_STRING_d016_EAR },											//configuration string

			{DONT_MATCH, derby2_1Revs, numderby2_1Revs, hainanRevs, numHainanRevs, SW_DEVICE_ID_HAINAN_DERBY, //Identifiers
	 hainan_derby2_1,  sizeof(hainan_derby2_1)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &veniceAPI, &derbyAPI,	&veniceArtAniAPI,								//APIs
	 3, hainan_derby2_1_mode, sizeof(hainan_derby2_1_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_d017 },												//configuration string

	//will only point to this structure if want to load EAR from EEPROM
			{DONT_MATCH, derby2_1Revs, numderby2_1Revs, hainanRevs, numHainanRevs, SW_DEVICE_ID_HAINAN_DERBY, //Identifiers
			 hainan_derby2_1_ear,  sizeof(hainan_derby2_1_ear)/sizeof(ATHEROS_REG_FILE),		//Register file
	 &veniceAPI, &derbyAPI,	&veniceArtAniAPI,							//APIs
			 3, hainan_derby2_1_mode_ear, sizeof(hainan_derby2_1_mode_ear)/sizeof(MODE_INFO), //Mode file
			 CFG_VERSION_STRING_d017_EAR },											//configuration string

	{DONT_MATCH, griffin2_AnalogRevs, numGriffin2_AnalogRevs, griffinRevs, numGriffinRevs, 0x0018, //Identifiers
	 griffin2,  sizeof(griffin2)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &veniceAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
	 3, griffin2_mode, sizeof(griffin2_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_c018 },												//configuration string

	//will only point to this structure if want to load EAR from EEPROM
	//This is to support hainan ear, which must use same frozen venice_derby config file
			{DONT_MATCH, griffin2_AnalogRevs, numGriffin2_AnalogRevs, griffinRevs, numGriffinRevs, SW_DEVICE_ID_GRIFFIN, //Identifiers
			 hainan_derby2_1_ear,  sizeof(hainan_derby2_1_ear)/sizeof(ATHEROS_REG_FILE),		//Register file
	 &veniceAPI, &griffinRfAPI,	&veniceArtAniAPI,							//APIs
			 3, hainan_derby2_1_mode_ear, sizeof(hainan_derby2_1_mode_ear)/sizeof(MODE_INFO), //Mode file
			 CFG_VERSION_STRING_d017_EAR },											//configuration string

			{DONT_MATCH, derby2_1Revs, numderby2_1Revs, predatorRevs, numPredatorRevs, SW_DEVICE_ID_PREDATOR, //Identifiers
	 predator_derby2_1,  sizeof(predator_derby2_1)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &veniceAPI, &derbyAPI,	&veniceArtAniAPI,								//APIs
	 3, predator_derby2_1_mode, sizeof(predator_derby2_1_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_00b0 },												//configuration string

	{DONT_MATCH, eagle2AnalogRevs, numEagle2AnalogRevs, eagle2Revs, numEagle2Revs, SW_DEVICE_ID_EAGLE, //Identifiers
	 eagle2,  sizeof(eagle2)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &eagleAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
	 3, eagle2_mode, sizeof(eagle2_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_d019 },												//configuration string

	{DONT_MATCH, condorAnalogRevs, numCondorAnalogRevs, condorRevs, numCondorRevs, SW_DEVICE_ID_CONDOR, //Identifiers
	 condor,  sizeof(condor)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &eagleAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
	 3, condor_mode, sizeof(condor_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_0020 },												//configuration string

	 {DONT_MATCH, swanAnalogRevs, numSwanAnalogRevs, swanRevs, numSwanRevs, SW_DEVICE_ID_SWAN, //Identifiers
	 swan,  sizeof(swan)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &eagleAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
	 3, swan_mode, sizeof(swan_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_0025 },			//configuration string
	//  nala
	{
		DONT_MATCH,
		swanAnalogRevs,
		numSwanAnalogRevs,
		nalaRevs,
		numNalaRevs,
		SW_DEVICE_ID_NALA,
		nala,
		sizeof(nala) / sizeof(ATHEROS_REG_FILE),
		&eagleAPI,
		&griffinRfAPI,
		&veniceArtAniAPI,
		3,
		nala_mode,
		sizeof(nala_mode) / sizeof(MODE_INFO),
		CFG_VERSION_STRING_0026
	}

//	{0xff19, NULL, 0, NULL, 0, SW_DEVICE_ID_CONDOR, //Identifiers
//	 condor,  sizeof(condor)/sizeof(ATHEROS_REG_FILE),	//Register file
//	 &eagleAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
//	 3, condor_mode, sizeof(condor_mode)/sizeof(MODE_INFO), //Mode file
//	 CFG_VERSION_STRING_0020 },												//configuration string

//	{0x001c, NULL, 0, NULL, 0, SW_DEVICE_ID_CONDOR, //Identifiers
//	 condor,  sizeof(condor)/sizeof(ATHEROS_REG_FILE),	//Register file
//	 &eagleAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
//	 3, condor_mode, sizeof(condor_mode)/sizeof(MODE_INFO), //Mode file
//	 CFG_VERSION_STRING_0020 },												//configuration string

};
#else
DEVICE_INIT_DATA ar5kInitData[] = {
//Special set for dos Keep the duplicate set
//to be compatible with code 
#ifndef DOS_CLIENT
//	{DONT_MATCH, griffin2_AnalogRevs, numGriffin2_AnalogRevs, griffinRevs, numGriffinRevs, 0x0018, //Identifiers
//	 griffin2,  sizeof(griffin2)/sizeof(ATHEROS_REG_FILE),	//Register file
//	 &veniceAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
//	 3, griffin2_mode, sizeof(griffin2_mode)/sizeof(MODE_INFO), //Mode file
//	 CFG_VERSION_STRING_c018 },												//configuration string

	//will only point to this structure if want to load EAR from EEPROM
//	{DONT_MATCH, griffin2_AnalogRevs, numGriffin2_AnalogRevs, griffinRevs, numGriffinRevs, 0x0018, //Identifiers
//	 griffin2,  sizeof(griffin2)/sizeof(ATHEROS_REG_FILE),	//Register file
//	 &veniceAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
//	 3, griffin2_mode, sizeof(griffin2_mode)/sizeof(MODE_INFO), //Mode file
//	 CFG_VERSION_STRING_c018 },												//configuration string

	{DONT_MATCH, eagle2AnalogRevs, numEagle2AnalogRevs, eagle2Revs, numEagle2Revs, SW_DEVICE_ID_EAGLE, //Identifiers
	 eagle2,  sizeof(eagle2)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &eagleAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
	 3, eagle2_mode, sizeof(eagle2_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_d019 },												//configuration string

	{DONT_MATCH, condorAnalogRevs, numCondorAnalogRevs, condorRevs, numCondorRevs, SW_DEVICE_ID_CONDOR, //Identifiers
	 condor,  sizeof(condor)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &eagleAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
	 3, condor_mode, sizeof(condor_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_0020 },												//configuration string

	{DONT_MATCH, swanAnalogRevs, numSwanAnalogRevs, swanRevs, numSwanRevs, SW_DEVICE_ID_SWAN, //Identifiers
	 swan,  sizeof(swan)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &eagleAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
	 3, swan_mode, sizeof(swan_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_0025 },			//configuration string

#else
	{DONT_MATCH, derby2_1Revs, numderby2_1Revs, hainanRevs, numHainanRevs, SW_DEVICE_ID_HAINAN_DERBY, //Identifiers
	 hainan_derby2_1,  sizeof(hainan_derby2_1)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &veniceAPI, &derbyAPI,	&veniceArtAniAPI,								//APIs
	 3, hainan_derby2_1_mode, sizeof(hainan_derby2_1_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_d017 },												//configuration string

	{DONT_MATCH, derby2_1Revs, numderby2_1Revs, hainanRevs, numHainanRevs, SW_DEVICE_ID_HAINAN_DERBY, //Identifiers
	 hainan_derby2_1,  sizeof(hainan_derby2_1)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &veniceAPI, &derbyAPI,	&veniceArtAniAPI,								//APIs
	 3, hainan_derby2_1_mode, sizeof(hainan_derby2_1_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_d017 },												//configuration string

	{DONT_MATCH, griffin2_AnalogRevs, numGriffin2_AnalogRevs, griffinRevs, numGriffinRevs, 0x0018, //Identifiers
	 griffin2,  sizeof(griffin2)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &veniceAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
	 3, griffin2_mode, sizeof(griffin2_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_c018 },												//configuration string

	{DONT_MATCH, griffin2_AnalogRevs, numGriffin2_AnalogRevs, griffinRevs, numGriffinRevs, 0x0018, //Identifiers
	 griffin2,  sizeof(griffin2)/sizeof(ATHEROS_REG_FILE),	//Register file
	 &veniceAPI, &griffinRfAPI,	&veniceArtAniAPI,								//APIs
	 3, griffin2_mode, sizeof(griffin2_mode)/sizeof(MODE_INFO), //Mode file
	 CFG_VERSION_STRING_c018 },												//configuration string

#endif
};
#endif

A_UINT32 numDeviceIDs = (sizeof(ar5kInitData)/sizeof(DEVICE_INIT_DATA));

void patch_specific_idata(DEVICE_INIT_DATA *dev_idata, int patch_no)
{
	if (dev_idata)
	{
		if (isSwan(dev_idata->swDeviceID))
		{
			if (patch_no && (patch_no<SWAN_HP63_PATCH_SIZE))
			{
				dev_idata->pInitRegs = swan_hp63_patch[patch_no].pInitRegs;
				dev_idata->pModeValues = swan_hp63_patch[patch_no].pModeValues;
				dev_idata->pCfgFileVersion = CFG_VERSION_STRING_b025;
			}
		}
	}
}
