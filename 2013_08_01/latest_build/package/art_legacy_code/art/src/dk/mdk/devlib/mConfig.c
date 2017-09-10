/* mConfig.c - contians configuration and setup functions for manlib */
/* Copyright (c) 2001 Atheros Communications, Inc., All Rights Reserved */

#if 0
#ident  "ACI $Id: //depot/sw/branches/ART_V53/sw/src/dk/mdk/devlib/mConfig.c#13 $, $Header: //depot/sw/branches/ART_V53/sw/src/dk/mdk/devlib/mConfig.c#13 $"
#endif

/* 
Revsision history
--------------------
1.0       Created.
*/

#ifdef VXWORKS
#include "vxworks.h"
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
#endif	// #ifdef __ATH_DJGPPDOS__


#include "wlantype.h"
#include "ar5210reg.h"

#if defined(SPIRIT_AP) || defined(FREEDOM_AP)
#ifdef COBRA_AP
#include "ar531xPlusreg.h"
#else
#include "ar531xreg.h"
#endif
#endif

#include "athreg.h"
#include "manlib.h"
#include "mData.h"
#include "mEeprom.h"
#include "mConfig.h"
#include "mDevtbl.h"
#ifdef LINUX 
#include "../mdk/dk_ver.h"
#else
#include "..\mdk\dk_ver.h"
#endif
#include "mCfg210.h"
#include "mCfg211.h"
#include "mData211.h"
#include "mEEP210.h"
#include "mEEP211.h"
#include "artEar.h"
#include "art_ani.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include "mIds.h"
#include "rate_constants.h"
//#ifdef LINUX
//#include "linuxdrv.h"
//#else
//#include "ntdrv.h"
//#endif
#if !defined(VXWORKS) && !defined(LINUX)
#include <io.h>
#endif

#if (!CUSTOMER_REL) && (INCLUDE_DRIVER_CODE)
#if (MDK_AP) 
int m_ar5212Reset(LIB_DEV_INFO *pLibDev, A_UCHAR *mac, A_UCHAR *bss, A_UINT32 freq, A_UINT32 turbo) {
        return -1;
}
#else
  #include "m_ar5212.h"
#endif
#endif //CUSTOMER_REL

// ARCH_BIG_ENDIAN is defined some where. Cannot determine where it is defined
// Undefined LINUX for now
//#ifdef LINUX 
//#undef ARCH_BIG_ENDIAN
//#endif

// Mapping _access to access
#if defined (LINUX) || defined (__ATH_DJGPPDOS__ )
#define _access(fileName,mode) access((const char *)(fileName),(int)(mode))
#endif

#ifdef ARCH_BIG_ENDIAN
#include "endian_func.h"
#endif


//###########Temp place for ar5001 definitions
#define F2_EEPROM_ADDR	    0x6000  // EEPROM address register (10 bit)
#define F2_EEPROM_DATA	    0x6004  // EEPROM data register (16 bit)

#define F2_EEPROM_CMD	    0x6008  // EEPROM command register
#define F2_EEPROM_CMD_READ	 0x00000001
#define F2_EEPROM_CMD_WRITE	 0x00000002
#define F2_EEPROM_CMD_RESET	 0x00000004

#define F2_EEPROM_STS	    0x600c  // EEPROM status register
#define F2_EEPROM_STS_READ_ERROR	 0x00000001
#define F2_EEPROM_STS_READ_COMPLETE	 0x00000002
#define F2_EEPROM_STS_WRITE_ERROR	 0x00000004
#define F2_EEPROM_STS_WRITE_COMPLETE 0x00000008


LIB_INFO gLibInfo = {0, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}};
MANLIB_API A_UINT32 enableCal = DO_NF_CAL | DO_OFSET_CAL;

A_UINT32 lib_memory_range;
A_UINT32 cache_line_size;

static A_INT32 tempMDKErrno = 0;
static char tempMDKErrStr[SIZE_ERROR_BUFFER];
#ifndef MDK_AP
static FILE     *logFileHandle;
static A_BOOL	logging = 0;
#endif


PCI_VALUES pciValues[MAX_PCI_ENTRIES];

//MANLIB_API A_UINT32 checkSumLength=0x400; //default to 16k
//MANLIB_API A_UINT32 eepromSize=0;
					
// Local Functions
static void    initializeTransmitPower(A_UINT32 devNum, A_UINT32 freq,	
    A_INT32  override, A_UCHAR  *pwrSettings);
static A_BOOL setChannel(A_UINT32 devNum, A_UINT32 freq);
static void setSpurMitigation(A_UINT32 devNum, A_UINT32 freq);
#if 0
static void setSpurMitigation_Nala(A_UINT32 devNum, A_UINT32 freq);
static void testEEProm(A_UINT32 devNum);
#endif
#ifdef HEADER_LOAD_SCHEME
static A_BOOL setupEEPromHeaderMap(A_UINT32 devNum);
#endif //HEADER_LOAD_SCHEME

static PWR_CTL_PARAMS  pwrParams = { {FALSE, FALSE, FALSE}, 
									{0, 0, 0}, 
									{0, 0, 0}, 
									{0, 0, 0}, 
									{0, 0, 0}, 
									{0, 0, 0}, 
									{0, 0, 0}, 
									{0, 0, 0}, 
									{0, 0, 0}, 
									{0, 0, 0} };

A_UINT32 hwReset ( A_UINT32 devNum, A_UINT32 resetMask) {
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];

    //printf("SNOOP:::hwReset passing to %s\n", (pLibDev->devMap.remoteLib?"remote":"local"));

    if (pLibDev->devMap.remoteLib)
	   return (pLibDev->devMap.r_hwReset(devNum, resetMask));
    else {
	   ar5kInitData[pLibDev->ar5kInitIndex].pMacAPI->hwReset(devNum, resetMask);
	   return(0xDEADBEEF); // it appears hwReset is overloaded to return macRev ONLY for remoteLib.
						  // return a bogus value in other case to suppress warnings. PD 1/04
	}
}

void pllProgram ( A_UINT32 devNum, A_UINT32 turbo) {

    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];


    if (pLibDev->devMap.remoteLib) {
	    if(((ar5kInitData[pLibDev->ar5kInitIndex].deviceID & 0xf000) == 0xa000) && pLibDev->mode == MODE_11B)
           changeField(devNum,"bb_tx_frame_to_xpab_on", 0x7);
	    pLibDev->devMap.r_pllProgram(devNum, turbo, pLibDev->mode);
    }
    else
	    ar5kInitData[pLibDev->ar5kInitIndex].pMacAPI->pllProgram(devNum, turbo);
}

MANLIB_API A_INT32 getNextDevNum() {
    A_UINT32 i;
    for(i = 0; i < LIB_MAX_DEV; i++) {
		if(gLibInfo.pLibDevArray[i] == NULL) {
			return i;
		}
	}
    return -1;
}

/**************************************************************************
* initializeDevice - Setup all structures associated with this device
*
*/
MANLIB_API A_UINT32 initializeDevice
(
 struct DeviceMap map
) 
{
	A_UINT32     i, devNum = LIB_MAX_DEV;
	LIB_DEV_INFO *pLibDev;

	if (map.remoteLib) { // Thin client
		lib_memory_range = 100 * 1024; // 100kB
		cache_line_size	= 0x10;
	}
	else { // library present locally
		lib_memory_range = 1048576; // 1mB
		cache_line_size	= 0x20;
	}

    //printf("SNOOP:: intializeDevice called\n");
	if(gLibInfo.devCount+1 > LIB_MAX_DEV) {
		mError(0xdead, EINVAL, "initializeDevice: Cannot initialize more than %d devices\n", LIB_MAX_DEV);
		return 0xdead;
	}

	// Device sanity checks
	if(map.DEV_CFG_RANGE != LIB_CFG_RANGE) {
		mError(0xdead, EINVAL, "initializeDevice: DEV_CFG_RANGE should be %d, was given %d\n", LIB_CFG_RANGE, map.DEV_CFG_RANGE);
		return 0xdead;
	}
	// Atleast 1 MB of memory is required
	if(map.DEV_MEMORY_RANGE < lib_memory_range) {
		mError(0xdead, EINVAL, "initializeDevice: DEV_MEMORY_RANGE should be more than %d, was given %d\n", lib_memory_range, map.DEV_MEMORY_RANGE);
		return 0xdead;
	}
	if(map.DEV_REG_RANGE != LIB_REG_RANGE) {
		mError(0xdead, EINVAL, "initializeDevice: DEV_REG_RANGE should be %d, was given %d\n", LIB_REG_RANGE, map.DEV_REG_RANGE);
		return 0xdead;
	}

	//check for the allocated memory being cache line aligned.
        if (map.DEV_MEMORY_ADDRESS & (cache_line_size - 1)) {
            mError(0xdead, EINVAL, 
                "initializeDevice: DEV_MEMORY_ADDRESS should be cache line aligned (0x%x), was given 0x%08lx\n", 
                cache_line_size, map.DEV_MEMORY_ADDRESS);
            return 0xdead;
        }

        for(i = 0; i < LIB_MAX_DEV; i++) {
		if(gLibInfo.pLibDevArray[i] == NULL) {
			devNum = i;
			break;
		}
	}

	assert(i < LIB_MAX_DEV);

	// Allocate the structures for the new device
	gLibInfo.pLibDevArray[devNum] = (LIB_DEV_INFO *)malloc(sizeof(LIB_DEV_INFO));
        if  (gLibInfo.pLibDevArray[devNum] == NULL) {
	   mError(devNum, ENOMEM, "initializeDevice: Failed to allocate the library device information structure\n");
	   return 0xdead;
	}

	// Zero out all structures
	pLibDev = gLibInfo.pLibDevArray[devNum];
	memset(pLibDev, 0, sizeof(LIB_DEV_INFO));

	pLibDev->devNum = devNum;
	pLibDev->ar5kInitIndex = UNKNOWN_INIT_INDEX;
	gLibInfo.devCount++;
	memcpy(&(pLibDev->devMap), &map, sizeof(struct DeviceMap));

    /* Initialize the map bytes for tracking memory management: calloc will init to 0 as well*/
	pLibDev->mem.allocMapSize = (pLibDev->devMap.DEV_MEMORY_RANGE / BUFF_BLOCK_SIZE) / 8;
    pLibDev->mem.pAllocMap = (A_UCHAR *) calloc(pLibDev->mem.allocMapSize, sizeof(A_UCHAR));
	if(pLibDev->mem.pAllocMap == NULL) {
		mError(devNum, ENOMEM, "initializeDevice: Failed to allocate the allocation map\n");
		free(gLibInfo.pLibDevArray[devNum]);
		return 0xdead;
	}

	pLibDev->mem.pIndexBlocks = (A_UINT16 *) calloc((pLibDev->mem.allocMapSize * 8), sizeof(A_UINT16));
	if(pLibDev->mem.pIndexBlocks == NULL) {
		mError(devNum, ENOMEM, "initializeDevice: Failed to allocate the indexBlock map\n");
		free(gLibInfo.pLibDevArray[devNum]->mem.pAllocMap);
		free(gLibInfo.pLibDevArray[devNum]);
		return 0xdead;
	}
   	pLibDev->mem.usingExternalMemory = FALSE;

    pLibDev->regFileRead = 0;
    pLibDev->regFilename[0] = '\0'; // empty string

	pLibDev->mdkErrno = 0;
	pLibDev->mdkErrStr[0] = '\0';
	pLibDev->libCfgParams.refClock = REF_CLK_DYNAMIC;
    pLibDev->devState = INIT_STATE;

	pLibDev->eepromStartLoc = 0x00; // default. set to 0x400 for the 2nd eeprom_block if present
	pLibDev->maxLinPwrx4    = 0;    // default init to 0 dBm

#ifndef __ATH_DJGPPDOS__
	// enable ART ANI and reuse by default
	configArtAniSetup(devNum, ART_ANI_ENABLED, ART_ANI_REUSE_ON);
#endif

	return devNum;
}

/**************************************************************************
* useMDKMemory - Repoints the libraries memory map at the one used by
*                MDK so they can share the map
*
*/

MANLIB_API void useMDKMemory
(
 A_UINT32 devNum, 
 A_UCHAR *pExtAllocMap,
 A_UINT16 *pExtIndexBlocks
)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];

   	if(gLibInfo.pLibDevArray[devNum] == NULL) {
		mError(devNum, EINVAL, "Device Number %d:useMDKMemory\n", devNum);
		return;
	}
   	if (gLibInfo.pLibDevArray[devNum]->devState < INIT_STATE) {
		mError(devNum, EILSEQ, "Device Number %d:useMDKMemory: initializeDevice must be called first\n", devNum);
		return;
	}
    if((pExtAllocMap == NULL) || (pExtIndexBlocks == NULL)) {
        mError(devNum, EINVAL, "Device Number %d:useMDKMemory: provided memory pointers are NULL\n", devNum);
        return;
    }
    // Free the current Memory Map
    free(gLibInfo.pLibDevArray[devNum]->mem.pIndexBlocks);
    free(gLibInfo.pLibDevArray[devNum]->mem.pAllocMap);
    gLibInfo.pLibDevArray[devNum]->mem.pIndexBlocks = NULL;
    gLibInfo.pLibDevArray[devNum]->mem.pAllocMap = NULL;

    // Assign to the MDK Memory Map
    pLibDev->mem.pAllocMap = pExtAllocMap;
    pLibDev->mem.pIndexBlocks = pExtIndexBlocks;
    pLibDev->mem.usingExternalMemory = TRUE;
}

MANLIB_API A_UINT16 getDevIndex
(
	A_UINT32 devNum
)
{
	if (gLibInfo.pLibDevArray[devNum] == NULL) {
		mError(devNum, EINVAL, "Device Number %d:getDevIndex \n", devNum);
		return (A_UINT16)-1;
	}

	return gLibInfo.pLibDevArray[devNum]->devMap.devIndex;
}
/**************************************************************************
* closeDevice - Free all structures associated with this device
*
*/
MANLIB_API void closeDevice
(
	A_UINT32 devNum
) 
{
//   	FIELD_VALUES *pFieldValues;

	if(gLibInfo.pLibDevArray[devNum] == NULL) {
		mError(devNum, EINVAL, "Device Number %d:closeDevice\n", devNum);
		return;
	}
	if(gLibInfo.pLibDevArray[devNum]->mem.usingExternalMemory == FALSE) {
		if(gLibInfo.pLibDevArray[devNum]->mem.pIndexBlocks) {
			free(gLibInfo.pLibDevArray[devNum]->mem.pIndexBlocks);
            gLibInfo.pLibDevArray[devNum]->mem.pIndexBlocks = NULL;
		}
		if (gLibInfo.pLibDevArray[devNum]->mem.pAllocMap) {
			free(gLibInfo.pLibDevArray[devNum]->mem.pAllocMap);
			gLibInfo.pLibDevArray[devNum]->mem.pAllocMap = NULL;
		}
	}

	if(gLibInfo.pLibDevArray[devNum]->eepData.infoValid) {
		freeEepStructs(devNum);
		gLibInfo.pLibDevArray[devNum]->eepData.infoValid = FALSE;
	}

	
	if (gLibInfo.pLibDevArray[devNum]->regArray) {
		free(gLibInfo.pLibDevArray[devNum]->regArray);
        gLibInfo.pLibDevArray[devNum]->regArray = NULL;
	}
	if (gLibInfo.pLibDevArray[devNum]->pModeArray) {
		free(gLibInfo.pLibDevArray[devNum]->pModeArray);
		gLibInfo.pLibDevArray[devNum]->pModeArray  = NULL;
	}

	if (gLibInfo.pLibDevArray[devNum]->pciValuesArray) {
		free(gLibInfo.pLibDevArray[devNum]->pciValuesArray);
        gLibInfo.pLibDevArray[devNum]->pciValuesArray = NULL;
	}
  
	if(gLibInfo.pLibDevArray[devNum]->pEarHead) {
		ar5212EarFree(gLibInfo.pLibDevArray[devNum]->pEarHead);
	}

	free(gLibInfo.pLibDevArray[devNum]);
	gLibInfo.pLibDevArray[devNum] = NULL;
	gLibInfo.devCount--;
}

/**************************************************************************
* specifySubSystemID - Tell library what subsystemID to try if can't find
*                      a match 
*
* 
*
*/
MANLIB_API void specifySubSystemID
(
 A_UINT32 devNum,
 A_UINT32	subsystemID
)
{
	mError(devNum, EIO, "Device Number %d:specifySubSystemID is an obsolete library command\n", devNum);
	
	devNum = 0;			//quiet compiler warning.
	subsystemID = 0; 
	return;
}

/**************************************************************************
* setResetParams - Setup params prior to calling resetDevice 
*
* These are new params added for second generation products
*
*/
MANLIB_API void setResetParams
(
 A_UINT32 devNum,
 A_CHAR     *pFilename,
 A_BOOL		eePromLoad,
 A_BOOL		forceCfgLoad,  
 A_UCHAR	mode,
 A_UINT16     initCodeFlag
)
{
   	LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
	A_UINT16      i;


//	printf("\n In SetResetParams pfilename = %s \n",pFilename);
	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:setResetParams\n", devNum);
		return;
	}
    
	pLibDev->eePromLoad = eePromLoad;
//	pLibDev->eePromHeaderLoad = eePromHeaderLoad;
	pLibDev->mode = mode;
	pLibDev->use_init = initCodeFlag;

	if(eePromLoad == 0) {
		//force the eeprom header to re-apply, next time we load eeprom
		for (i = 0; i < 4; i++) {
			pLibDev->eepromHeaderApplied[i] = FALSE;
		}
	}
	if(forceCfgLoad) {
		//force the pci values to be regenerated
		pLibDev->regFileRead = 0;		
        //printf("SNOOP::: regFileRead to zero\n");

		//force the eeprom header to re-apply
		for (i = 0; i < 4; i++) {
			pLibDev->eepromHeaderApplied[i] = FALSE;
		}
	}

    if(( pFilename == '\0' ) || ( pFilename == NULL ) || (pFilename[0] == 0)) {
		return;
   }


#ifndef MDK_AP    
    //do a check for file exist and is readable
    if(_access(pFilename, 04) != 0) {
        mError(devNum, EINVAL, "Device Number %d:setRegFilename: either file [%s] does not exist or is not readable\n", devNum, pFilename);
        return;
    }

	//if there is a config file change then reset the config file reading
	if(strcmp(pLibDev->regFilename, pFilename) != 0) {
		pLibDev->regFileRead = 0;
        //printf("SNOOP::: regFileRead to zero\n");
	}

    strcpy(pLibDev->regFilename, pFilename);
#endif
}

/**************************************************************************
* configureLibParams - Allow certain library params to be configured.
*                      Similar to setResetParmams.  This function will
*                      use a strcuture to pass params as this will be device
*                      specific params that will grow through time.
*
*/
MANLIB_API void configureLibParams
(
 A_UINT32 devNum,
 LIB_PARAMS *pLibParamsInfo
)
{
   	LIB_DEV_INFO *pLibDev;

    //printf("SNOOP::::::::::::::::::: configureLibParams::devNum=%d\n", devNum);
    //fflush(stdout);
   	pLibDev = gLibInfo.pLibDevArray[devNum];
	if (pLibDev->libCfgParams.loadEar != pLibParamsInfo->loadEar) {
		pLibDev->regFileRead = 0;
        //printf("SNOOP::: regFileRead to zero\n");
	} 

#ifndef __ATH_DJGPPDOS__
	if (pLibDev->artAniSetup.Enabled != pLibParamsInfo->artAniEnable) {
		if (pLibParamsInfo->artAniEnable == ART_ANI_ENABLED) {
			enableArtAniSetup(devNum);
		} else {
			disableArtAniSetup(devNum);
		}
	}

	pLibDev->artAniSetup.Reuse = pLibParamsInfo->artAniReuse;
	pLibDev->artAniSetup.currNILevel = pLibParamsInfo->artAniNILevel;
	pLibDev->artAniSetup.currBILevel = pLibParamsInfo->artAniBILevel;
	pLibDev->artAniSetup.currSILevel = pLibParamsInfo->artAniSILevel;
#else
	pLibParamsInfo->artAniEnable = ART_ANI_DISABLED;
#endif

	memcpy(&(pLibDev->libCfgParams), pLibParamsInfo, sizeof(LIB_PARAMS));
	return;
}

/**************************************************************************
* getDeviceInfo - get a subset of the device info for display
	*
*/
MANLIB_API void getDeviceInfo
(
 A_UINT32 devNum, 
 SUB_DEV_INFO *pInfoStruct		//pointer to caller assigned struct
)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
	A_UINT32	strLength;

	if(pLibDev->ar5kInitIndex == UNKNOWN_INIT_INDEX) {
		mError(devNum, EIO, "Device Number %d:getDeviceInfo fail, run resetDevice first\n", devNum);
		return;
	}
	pInfoStruct->aRevID = pLibDev->aRevID;
	pInfoStruct->hwDevID = pLibDev->hwDevID;
	pInfoStruct->swDevID = ar5kInitData[pLibDev->ar5kInitIndex].swDeviceID;
	pInfoStruct->bbRevID = pLibDev->bbRevID;
	pInfoStruct->macRev = pLibDev->macRev;
	pInfoStruct->subSystemID = pLibDev->subSystemID;
	strLength = strlen(pLibDev->regFilename);
	if (strLength) {
		strcpy(pInfoStruct->regFilename, pLibDev->regFilename);
		pInfoStruct->defaultConfig = 0;
	}
	else {
		//copy in the default
		if (ar5kInitData[pLibDev->ar5kInitIndex].pCfgFileVersion) {
			strcpy(pInfoStruct->regFilename, ar5kInitData[pLibDev->ar5kInitIndex].pCfgFileVersion);
		} else {
			pInfoStruct->regFilename[0]='\0';
		}
		pInfoStruct->defaultConfig = 1;
	}
	sprintf(pInfoStruct->libRevStr, DEVLIB_VER1); 

}

static A_BOOL analogRevMatch
(
	A_UINT32 index,
	A_UINT32 aRevID
)
{
	A_UINT16 i;
	A_UCHAR  aProduct = (A_UCHAR)(aRevID & 0xf);
	A_UCHAR  aRevision = (A_UCHAR)((aRevID >> 4) & 0xf);

	//tmp ym
	//printf("Entered analogRevMatch\n");
	

	//printf("SNOOP::aProduct=%x:aRev=%x\n", aProduct, aRevision);
	if(ar5kInitData[index].pAnalogRevs == NULL) {
		return(TRUE);
	}
	//printf("SNOOP::numAnalogRevs = %d\n", ar5kInitData[index].numAnalogRevs);
	for (i = 0; i < ar5kInitData[index].numAnalogRevs; i++) {
		//printf("SNOOP::index=%d:aProduct=%x:aRev=%x\n", i, ar5kInitData[index].pAnalogRevs[i].productID, ar5kInitData[index].pAnalogRevs[i].revID);
		
		if(aProduct == ar5kInitData[index].pAnalogRevs[i].productID)
		//	printf("aProduct matches\n");
		if (aRevision == ar5kInitData[index].pAnalogRevs[i].revID)
		//	printf("aRev mathces\n");
			

		if((aProduct == ar5kInitData[index].pAnalogRevs[i].productID) &&
			(aRevision == ar5kInitData[index].pAnalogRevs[i].revID)) {
			return (TRUE);
		}
	}
	return(FALSE);
}

static A_BOOL macRevMatch
(
	A_UINT32 index,
	A_UINT32 macRev
)
{
	A_UINT16 i;

	//printf("Entered macRevMatch\n");
	
	if(ar5kInitData[index].pMacRevs == NULL) {
		return (TRUE);
	}

	for(i = 0; i < ar5kInitData[index].numMacRevs; i++) {

		//tmp YM
		//printf("ar5kInitData[index].pMacRevs[i] = %d\n",ar5kInitData[index].pMacRevs[i]);
		if(macRev == ar5kInitData[index].pMacRevs[i]) {
			return(TRUE);
		}
	}
	return(FALSE);
}

A_INT32 findDevTableEntry
(
	A_UINT32 devNum
)
{
	A_UINT32 i;
	LIB_DEV_INFO *pLibDev;

	pLibDev = gLibInfo.pLibDevArray[devNum];

	//FJC: 07/24/03 this function can now be called from outside resetDevice
	//so it needs to read the hw info incase it has not been read.
	//have copied this from resetDevice rather than move it to minimize
	//inpact.  Should readly optimize this so don't do the same stuff twice.
	pLibDev->hwDevID = (pLibDev->devMap.OScfgRead(devNum, 0) >> 16) & 0xffff;

	 /* Reset the device. */
	 /* Masking the board specific part of the device id */
	 switch (pLibDev->hwDevID & 0xff)	{
		case 0x0007:
#ifndef __ATH_DJGPPDOS__
            if (pLibDev->devMap.remoteLib)
            {
			    pLibDev->macRev = hwReset(devNum, BUS_RESET | BB_RESET | MAC_RESET);
            }
            else
            {
			    hwResetAr5210(devNum, BUS_RESET | BB_RESET | MAC_RESET);
#if defined(SPIRIT_AP) || defined(FREEDOM_AP)
#if !defined(COBRA_AP)
                pLibDev->macRev = (sysRegRead(AR531X_REV) >> REV_WMAC_MIN_S) & 0xff;
#endif
#else
                pLibDev->macRev = REGR(devNum, F2_SREV) & F2_SREV_ID_M;
#endif
            }
#endif
			break;
		case 0x0011:
		case 0x0012:
		case 0x0013:
		case 0x0014:
		case 0x0015:
        case 0x0016: 
        case 0x0017: 
		case 0x001a:  //  nala
		case 0x001b:
		case 0x001c:
		case 0x001d:  //  nala
		case 0x001f:
        case 0x00b0: 
		case 0xa014:
		case 0xa016:
		case 0x0019:
		case (DEVICE_ID_COBRA&0xff):
            if (pLibDev->devMap.remoteLib)
			    pLibDev->macRev = hwReset(devNum, BUS_RESET | BB_RESET | MAC_RESET);
            else
            {
		        hwResetAr5211(devNum, BUS_RESET | BB_RESET | MAC_RESET);
#if defined(SPIRIT_AP) || defined(FREEDOM_AP)
#if defined(COBRA_AP)
		    if((pLibDev->hwDevID & 0xff) == (DEVICE_ID_COBRA&0xff))
		        pLibDev->macRev = (sysRegRead(AR531XPLUS_PCI+F2_SREV) & F2_SREV_ID_M);
		    else {
		        pLibDev->macRev = REGR(devNum, F2_SREV) & F2_SREV_ID_M;
			}
#else
                pLibDev->macRev = (sysRegRead(AR531X_REV) >> REV_WMAC_MIN_S) & 0xff;
#endif
#else
                pLibDev->macRev = REGR(devNum, F2_SREV) & F2_SREV_ID_M;
#endif
				if (isGriffin_1_0(pLibDev->macRev) || isGriffin_1_1(pLibDev->macRev)) {
					// to override corrupted 0x9808 in griffin 1.0 and 1.1. 2.0 should be fixed.
//					printf("SNOOP: setting reg 0x9808 to 0 for griffin 1.0 and 1.1 \n");
					REGW(devNum, 0x9808, 0);
				}
            }
			break;
	}

	// Needed for venice FPGA 
	if ((pLibDev->hwDevID == 0xf013) || (pLibDev->hwDevID == 0xfb13) 
		|| (pLibDev->hwDevID == 0xf016) || (pLibDev->hwDevID == 0xff16)) {
		REGW(devNum, 0x4014, 0x3);
		REGW(devNum, 0x4018, 0x0);
		REGW(devNum, 0xd87c, 0x16);
		mSleep(100);
	}

	// set it to ofdm mode for reading the analog rev id for 11b fpga
	if (pLibDev->hwDevID == 0xf11b) {	
		REGW(devNum, 0xa200, 0);
		REGW(devNum, 0x987c, 0x19) ;
		mSleep(20);
	}

   	//read the baseband revision 
	pLibDev->bbRevID = REGR(devNum, PHY_CHIP_ID);
	//printf("bb rev id = %x\n", pLibDev->bbRevID);

	//Needed so that analog revID write will work.
	if((pLibDev->hwDevID & 0xff) >= 0x0012) {
		REGW(devNum, 0x9800, 0x07);
	}
	else {
		REGW(devNum, 0x9800, 0x47);
	}

	//read the analog revIDs
	REGW(devNum, (PHY_BASE+(0x34<<2)), 0x00001c16);
    for (i=0; i<8; i++) {   
       REGW(devNum, (PHY_BASE+(0x20<<2)), 0x00010000);
    }	
    pLibDev->aRevID = (REGR(devNum, PHY_BASE + (256<<2)) >> 24) & 0xff;   
    pLibDev->aRevID = reverseBits(pLibDev->aRevID, 8);

	if ((0x0012 == pLibDev->hwDevID) || (0xff12 == pLibDev->hwDevID)
		||((pLibDev->hwDevID == 0x0013) && ((pLibDev->aRevID & 0xF) != 0x5)) || // exclude griffin
		((pLibDev->hwDevID == 0xff13) && ((pLibDev->aRevID & 0xf) < 3))) {
		// Only the 2 mac of freedom has A/B 
		#if defined(FREEDOM_AP)&&!defined(COBRA_AP)
			if (pLibDev->devMap.DEV_REG_ADDRESS == AR531X_WLAN1) 
		#endif
		{	
			//read the beanie rev ID
			REGW(devNum, 0x9800, 0x00004007);
			mSleep(2);
			REGW(devNum, (PHY_BASE+(0x34<<2)), 0x00001c16);
			for (i=0; i<8; i++) {   
				REGW(devNum, (PHY_BASE+(0x20<<2)), 0x00010000);
			}
			pLibDev->aBeanieRevID = (REGR(devNum, PHY_BASE + (256<<2)) >> 24) & 0xff;   
			pLibDev->aBeanieRevID = reverseBits(pLibDev->aBeanieRevID, 8);
			REGW(devNum, 0x9800, 0x07);
		}
	}
	

	//tmp YM
	//	printf("numDeviceIDS = %d\n", numDeviceIDs);
	for (i = 0; i < numDeviceIDs; i++)	{

         	
		if (((pLibDev->hwDevID == ar5kInitData[i].deviceID) || (ar5kInitData[i].deviceID == DONT_MATCH) ) && 
            analogRevMatch(i, pLibDev->aRevID) && macRevMatch(i, pLibDev->macRev) )
		{
			patch_specific_idata(&ar5kInitData[i], pLibDev->registerSpecificPatch);
			break;
		}
	}


    if(i == numDeviceIDs) {
       	mError(devNum, EIO, "Device Number %d:unable to find device init details for deviceID 0x%04lx, analog rev 0x%02lx, mac rev 0x%04lx, bb rev 0x%04lx\n", devNum, 
		pLibDev->hwDevID, pLibDev->aRevID, pLibDev->macRev, pLibDev->bbRevID);
       	printf("Device Number %d:unable to find device init details for deviceID 0x%04x, analog rev 0x%04x, mac rev 0x%04x, bb rev 0x%04x\n", devNum, 
		pLibDev->hwDevID, pLibDev->aRevID, pLibDev->macRev, pLibDev->bbRevID);
		return -1;
    }

	//read the subsystemID, this has been moved from above, so can read 
	//this from eeprom 
	//force the eeprom size incase blank card
	if(ar5kInitData[i].swDeviceID >= 0x0012) {
		REGW(devNum, 0x6010, REGR(devNum, 0x6010) | 0x3);
	}
    if (pLibDev->devMap.remoteLib) {
        pLibDev->subSystemID =  pLibDev->devMap.r_eepromRead(devNum, 0x7);
	}
    else {
	    pLibDev->subSystemID = ar5kInitData[i].pMacAPI->eepromRead(devNum, 0x7);
    }

#ifndef NO_LIB_PRINT		
   	if (!pLibDev->regFileRead) {
		printf(DEVLIB_VER1);
		printf("\n\n                     ===============================================\n");
		printf("                     |               ");                                   
		switch(pLibDev->subSystemID) {
			case 0x1021  :  printf("AR5001a_cb                    "); break;
			case 0x1022  :  printf("AR5001x_cb                    "); break;
			case 0x2022  :  printf("AR5001x_mb                    "); break;
			case 0x2023  :  printf("AR5001a_mb                    "); break;
			case 0xa021  :  printf("AR5001ap_ap                   "); break;

			case 0x1025  :  printf("AR5001g_cb21g                 "); break;
			case 0x1026  :  printf("AR5001x2_cb22ag               "); break;
			case 0x2025  :  printf("AR5001g_mb21g                 "); break;
			case 0x2026  :  printf("AR5001x2_mb22ag               "); break;

			case  0x2030 :	printf("AR5002g_mb31g (de-stuffed)    "); break;        
			case  0x2031 :  printf("AR5002x_mb32ag                "); break;               
			case  0x2027 :	printf("AR5001g_mb22g (de-stuffed)    "); break;        
			case  0x2029 :	printf("AR5001x_mb22ag (single-sided) "); break;        
			case  0x2024 :  printf("AR5001x_mb23j                 "); break;                 
			case  0x1030 :	printf("AR5002g_cb31g (de-stuffed)    "); break;        
			case  0x1031 :  printf("AR5002x_cb32ag                "); break;               
			case  0x1027 :	printf("AR5001g_cb22g (de-stuffed)    "); break;        
			case  0x1029 :	printf("AR5001x_cb22ag (single-sided) "); break;        
			case  0xa032 :  printf("AR5002a_ap30                  "); break;                 
			case  0xa034 :  printf("AR5002a_ap30  (040)           "); break;                 
		    case  0xa041 :	printf("AR5004ap_ap41g (g)            "); break;                 
		    case  0xa043 :	printf("AR5004ap_ap43g (g)            "); break;                 
			case  0xa048 :	printf("AR5004ap_ap48ag (a/g)         "); break;                 
			case  0x1041 :	printf("AR5004g_cb41g (g)             "); break;                 
			case  0x1042 :	printf("AR5004x_cb42ag (a/g)          "); break;                 
			case  0x1043 :	printf("AR5004g_cb43g (g)             "); break;                 
			case  0x2041 :	printf("AR5004g_mb41g (g)             "); break;                 
			case  0x2042 :	printf("AR5004x_mb42ag (a/g)          "); break;                 
			case  0x2043 :	printf("AR5004g_mb43g (g)             "); break;                 
			case  0x2044 :	printf("AR5004x_mb44ag (a/g)          "); break;                 
			case  0x1051 :	printf("AR5005gs_cb51g (g super)      "); break;                 
			case  0x1052 :	printf("AR5005g_cb51g (g)             "); break;                 
			case  0x2051 :	printf("AR5005gs_mb51g (g super)      "); break;                 
			case  0x2052 :	printf("AR5005g_mb51g (g)             "); break;                 
			case  0x1062 :	printf("AR5006xs_cb62ag (a/g super)   "); break;                 
			case  0x1063 :	printf("AR5006x_cb62ag (a/g)          "); break;                 
			case  0x2062 :	printf("AR5006xs_mb62ag (a/g super)   "); break;                 
			case  0x2063 :	printf("AR5006x_mb62g (a/g)           "); break;                 
			case  0x2055 :  printf("AR5007g_mb55g (g)             "); break;
			case  0x1055 :  printf("AR5007g_cb55g (g)             "); break;
			case  UB51_SUBSYSTEM_ID:  printf("AR5005ug_UB51 (g)            "); break;                 
			case  UB52_SUBSYSTEM_ID:  printf("AR5005ux_UB52 (a/g)            "); break;                 
			case  AP51_FULL_SUBSYSTEM_ID:  printf("AR5006apgs_AP51 (g)            "); break;                 
			case  AP51_LITE_SUBSYSTEM_ID:  printf("AR5006apg_AP51 (g)            "); break;         
			case  AP53_FULL_SUBSYSTEM_ID:  printf("AR5006apgs_AP53 (a/g)            "); break;                 
			case  AP53_LITE_SUBSYSTEM_ID:  printf("AR5006apg_AP53 (a/g)            "); break;         

			default: printf("UNKNOWN TYPE"); break;
		}
		printf("|\n");                                   
		printf("                     ===============================================\n\n");
		printf("Devices detected:\n");
		printf("   PCI deviceID     : 0x%04x\t", pLibDev->hwDevID);
		printf("Sub systemID     : 0x%04x\n", pLibDev->subSystemID);
		printf("   MAC revisionID   : 0x%02x\t", pLibDev->macRev & 0xff);
		printf("BB  revisionID   : 0x%02x\n", pLibDev->bbRevID & 0xff);
		printf("   RF  productID    : 0x%01x\t", pLibDev->aRevID & 0xf);
		printf("RF  revisionID   : 0x%01x\n", (pLibDev->aRevID >> 4) & 0xf);
		if((0x0012 == ar5kInitData[i].swDeviceID) || (ar5kInitData[i].swDeviceID == 0x0013) ) {
			printf("   Beanie productID : 0x%01x\t", pLibDev->aBeanieRevID & 0xf);
			printf("Beanie revisionID: 0x%01x\n\n", (pLibDev->aBeanieRevID >> 4) & 0xf);
		} else {
			printf("\n");
		}
	}
#endif //NO_LIB_PRINT
	return i;
}



A_BOOL loadCfgData(A_UINT32 devNum, A_UINT32 freq) {

LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
A_UINT32 devTableIndex;
A_BOOL   earHere = FALSE;
A_UINT32 jj; 

//printf("SNOOP:::loadCfgData called\n");
    if (!pLibDev->regFileRead) {
    //printf("SNOOP:::loadCfgData regFileRead not set\n");
        devTableIndex = findDevTableEntry(devNum);
        if (devTableIndex == -1) {
            mError(devNum, EIO, "Invalid Device table number %d:\n", devTableIndex);
            return 0;
        }

        pLibDev->ar5kInitIndex = devTableIndex;
        //try to read the eeprom to see if there is an ear
	if (pLibDev->eePromLoad) {
		if(setupEEPromMap(devNum)) 
		{
//			printf("pLibDev->eePromLoad setupEEpromMap\n");
			if(pLibDev->p16kEepHeader->majorVersion >= 4) 
			{
				earHere = ar5212IsEarEngaged(devNum, pLibDev->pEarHead, freq);
			//	printf("In pLibDev->p16kEepHeader->majorVersion  loadcfgdata ar5212IsEarEngaged\n");
			}
			//If we are loading the EAR, then move onto the next structure
			//so we point to the "frozen" config file contents, then load ear on that
			if(earHere && (pLibDev->libCfgParams.loadEar)) 
			{
				//	printf("earHere\n");
				if(((ar5kInitData[devTableIndex].swDeviceID & 0xff) == 0x0016) || ((ar5kInitData[devTableIndex].swDeviceID & 0xff) == 0x0017) ||
					((ar5kInitData[devTableIndex].swDeviceID & 0xff) == 0x0018))
				{
				//		printf("ar5kInitData[devTableIndex].swDeviceID & 0xff) == 0x0016) || ((ar5kInitData[devTableIndex].swDeviceID & 0xff)\n");
						devTableIndex++;
				}
			}
		}
	}

        //set devTableIndex again incase it moved
        pLibDev->ar5kInitIndex = devTableIndex;
        pLibDev->sizeRegArray = ar5kInitData[devTableIndex].sizeArray;

        if(pLibDev->regArray) {
            free(pLibDev->regArray);
            pLibDev->regArray = NULL;
        }
        pLibDev->regArray = (ATHEROS_REG_FILE *)malloc(sizeof(ATHEROS_REG_FILE) *  pLibDev->sizeRegArray);
        if (!pLibDev->regArray) {
            mError(devNum, ENOMEM, "Device Number %d:resetDevice Failed to allocate memory for regArray \n", devNum);
            return 0;
        }

        memcpy(pLibDev->regArray, ar5kInitData[devTableIndex].pInitRegs, (sizeof(ATHEROS_REG_FILE) *  pLibDev->sizeRegArray));

        pLibDev->sizeModeArray = ar5kInitData[devTableIndex].sizeModeArray;

        if (pLibDev->pModeArray) {
            free(pLibDev->pModeArray);
            pLibDev->pModeArray = NULL;
        }
        pLibDev->pModeArray = (MODE_INFO *)malloc(sizeof(MODE_INFO)*pLibDev->sizeModeArray);
        if (!pLibDev->pModeArray) {
            mError(devNum, ENOMEM, "Device Number %d:resetDevice Failed to allocate memory for modeArray \n", devNum);
            return 0;
        }
        memcpy(pLibDev->pModeArray, ar5kInitData[devTableIndex].pModeValues,sizeof(MODE_INFO)*pLibDev->sizeModeArray);


        pLibDev->swDevID = ar5kInitData[devTableIndex].swDeviceID;
//				printf("pLibDev->regFilename = %s\n",pLibDev->regFilename);
        if (!parseAtherosRegFile(pLibDev, pLibDev->regFilename)) 
				{
					if (pLibDev->mdkErrno) 
					{
						printf("SNOOP: ERROR : %s\n", pLibDev->mdkErrStr);
						pLibDev->mdkErrno = 0;
					}
          mError(devNum, EIO, "Device Number %d:resetDevice: Unable to open atheros register file : %s\n", devNum, pLibDev->regFilename);
          return 0;
        }

		if (!createPciRegWrites(devNum)) 
				{
        
					//	printf("Not of createPciRegWrites \n");
						mError(devNum, EIO, "Device Number %d:resetDevice: Unable to convert atheros register file to pci write values\n", devNum);
            return 0;
        }
        pLibDev->regFileRead = 1;

        //initialize the API
        ar5kInitData[pLibDev->ar5kInitIndex].pMacAPI->macAPIInit(devNum);

	if (pLibDev->devMap.remoteLib) {
	}
	else {
          //zero out the key cache
          for(jj = 0x8800; jj < 0x97ff; jj+=4) {
            REGW(devNum, jj, 0);
          }
	}
    }
//printf("Load cfgdata end\n");
	return 1;
}

//++JC++
void sleep_cal( clock_t wait ) {
   clock_t goal;
   goal = wait + clock();
   while( goal > clock() );
}

/**************************************************************************
* griffin_cl_cal - Do carrier leak cal for griffin
***************************************************************************/
void griffin_cl_cal(A_UINT32 devNum)
{

   A_INT32 add_I_first[9], add_Q_first[9], add_I_second[9], add_Q_second[9];
   A_INT32 add_I_avg[9], add_Q_avg[9];
   A_INT32 add_I_diff, add_Q_diff, kk;
   A_UINT32 offset, data, cl_cal_done, i;
   A_UINT32 cl_cal_reg[9] = {0xa35c,0xa360,0xa364,0xa368,0xa36c,0xa370,0xa374,0xa378,0xa37c};


   data = REGR(devNum,0xa358);
   data = data | 0x3;
   REGW(devNum,0xa358, data);

   // Get first set of I and Q data
   data = REGR(devNum,0x9860);
   data = data | 0x1;
   REGW(devNum,0x9860, data);

   sleep_cal (10);

   // Check for cal done
   for (i = 0; i < 1000; i++) {
      if ((REGR(devNum, PHY_AGC_CONTROL) & (enableCal)) == 0 ) {
         break;
      }
      mSleep(1);
   }
   if(i >= 1000) {
      printf("griffin_cl_cal : Device Number %d didn't complete cal but keep going anyway\n", devNum);
   }

    for (kk=0; kk<9; kk++) {
	offset = cl_cal_reg[kk];
        data = REGR(devNum, offset);

        add_I_first[kk] = (data >> 16) & 0x7ff;
        add_Q_first[kk] = (data >>  5) & 0x7ff;

        if (((add_I_first[kk]>>10) & 0x1) == 1) {
            add_I_first[kk] = add_I_first[kk] | 0xfffff800;
        }
        if (((add_Q_first[kk]>>10) & 0x1) == 1) {
            add_Q_first[kk] = add_Q_first[kk] | 0xfffff800;
        }

        //printf ("Debug: 0x%04x: I_first:%d Q_first:%d\n", offset, add_I_first[kk], add_Q_first[kk]);
    }
    // Finished getting the first set of data

   // Get second set of data - in the while loop till the second set is obtained
   cl_cal_done = 0;
   while (cl_cal_done == 0) {
   
      data = REGR(devNum,0x9860);
      data = data | 0x1;
      REGW(devNum,0x9860, data);
   
      // Check for cal done
      for (i = 0; i < 1000; i++) {
         if ((REGR(devNum, PHY_AGC_CONTROL) & (enableCal)) == 0 ) {
            break;
         }
         mSleep(1);
      }
      if(i >= 1000) {
         printf("Device Number %d:Didn't complete cal but keep going anyway\n", devNum);
      }

      offset = cl_cal_reg[0];
      data = REGR(devNum, offset);
      add_I_second[0] = (data >> 16) & 0x7ff;
      add_Q_second[0] = (data >>  5) & 0x7ff;
   
      if (((add_I_second[0]>>10) & 0x1) == 1) {
          add_I_second[0] = add_I_second[0] | 0xfffff800;
      }
      if (((add_Q_second[0]>>10) & 0x1) == 1) {
          add_Q_second[0] = add_Q_second[0] | 0xfffff800;
      }
   
      // Get the difference between the first set and the current set
      add_I_diff = abs(add_I_first[0] - add_I_second[0]);
      add_Q_diff = abs(add_Q_first[0] - add_Q_second[0]);

      // Recognize the current set as the second set if diff > 8
      if ((add_I_diff > 8) || (add_Q_diff > 8)) {
          for (kk=0; kk<9; kk++) {             // Get the full second set
   	      offset = cl_cal_reg[kk];
              data = REGR(devNum, offset);
   
              add_I_second[kk] = (data >> 16) & 0x7ff;
              add_Q_second[kk] = (data >>  5) & 0x7ff;
   
              if (((add_I_second[kk]>>10) & 0x1) == 1) {
                  add_I_second[kk] = add_I_second[kk] | 0xfffff800;
              }
              if (((add_Q_second[kk]>>10) & 0x1) == 1) {
                  add_Q_second[kk] = add_Q_second[kk] | 0xfffff800;
              }
          }
          cl_cal_done = 1;
      }
   }

   // Form the average of the two values
   for (kk=0; kk<9; kk++) {             
      add_I_avg[kk] = (add_I_first[kk] + add_I_second[kk])/2;
      add_Q_avg[kk] = (add_Q_first[kk] + add_Q_second[kk])/2;

      add_I_avg[kk] = add_I_avg[kk] & 0x7FF;  // To take care of signed values
      add_Q_avg[kk] = add_Q_avg[kk] & 0x7FF;  // To take care of signed values
   }

   // Write back the average values into the registers
   for (kk=0; kk<9; kk++) {             
      offset = cl_cal_reg[kk];
      data = REGR(devNum, offset);
      data = data & 0xF800001F;               // Clear the bits where the avg is to be written
      data = data | (add_I_avg[kk] << 16) | (add_Q_avg[kk] << 5);     // Or in the avg data
      REGW(devNum,cl_cal_reg[kk], data);
   }

   // Enable Carrier Leak Correction
   data = REGR(devNum,0xa358);
   data = data | 0x40000000;
   REGW(devNum,0xa358, data);
   //printf("Done Carrier Leak cal workaround\n\n");

}
//++JC++


void setRegisterPatch(int devNum, int patch_no)
{
	gLibInfo.pLibDevArray[devNum]->registerSpecificPatch = patch_no;
}

void setSpurFrequency(int devNum, A_UINT32 *_5g, A_UINT32 *_2p4g)
{
	memcpy(gLibInfo.pLibDevArray[devNum]->spur_frequency_5g, _5g, sizeof(gLibInfo.pLibDevArray[devNum]->spur_frequency_5g));
	memcpy(gLibInfo.pLibDevArray[devNum]->spur_frequency_2p4g, _2p4g, sizeof(gLibInfo.pLibDevArray[devNum]->spur_frequency_2p4g));
}

/**************************************************************************
* resetDevice - reset the device and initialize all registers
*
*/
MANLIB_API void resetDevice
(
 A_UINT32 devNum, 
 A_UCHAR *mac, 
 A_UCHAR *bss, 
 A_UINT32 freq, 
 A_UINT32 turbo
)
{
   	LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
	A_INT32 i;
	A_UINT32  *pValue = NULL;
	A_UINT32  newThreshold, newThresholdTurbo;
	A_UINT32 temp1, temp2, temp3, temp4, temp5, temp6;
	A_UINT32 modifier;
	A_BOOL   earHere = FALSE;
	A_UINT32 prev_freq;
	A_UINT32 gpioData;
	A_UINT32 timeout = 0;
    A_UINT32 temp;

//++JC++
A_UINT32 bank6_data[63];
A_INT32 zz = 0;
A_INT32 bank6_over = 0;
A_UINT32 di_31_00, di_63_32, di_67_64, do_31_00, do_63_32, do_67_64;
A_UINT32 os0, os1, dreg0, dreg1, da0, da1, da2, tmp, offset, data;
A_INT32 kk, shift_good, num_tries;
//++JC++

#ifndef CUSTOMER_REL
    A_UINT32 resetDevice_start, resetDevice_end;
//    printf("SNOOP::calling resetDevice:with turbo=%d:freq=%d\n", turbo, freq);
	resetDevice_start=milliTime();
#endif
	//strip off the channelMask flags, not using this any more, leave for now just 
	//in case somewhere in the code we set the mask.
	pLibDev->channelMasks = turbo & 0xffff0000;
	turbo = turbo & 0xffff;

	//look for the other alternative way of setting the 2.5Mhz channel centers
	//look for channel *10 
//printf("SNOOP: freq at 1= %d\n", freq);
	if((freq >= 49000) && (freq < 60000)) {
		if((freq % 10) == 0) {
			freq = freq / 10;
//printf("SNOOP: freq at 2= %d\n", freq);
		}
		else {
			//must be a 2.5 MHz channel - check
//printf("SNOOP: freq at 3= %d\n", freq);
			if((freq % 25) != 0) {
				mError(devNum, EINVAL, "Illegal channel value %d:resetDevice\n", freq);
				return;
			}
			pLibDev->channelMasks |= QUARTER_CHANNEL_MASK;
			freq = (freq - 25) / 10;
		}
	}
	if((freq >= 24120) && (freq <= 24840)) {
		if((freq % 10) == 0) {
			freq = freq / 10;
		}
		else {
			mError(devNum, EINVAL, "Illegal channel value %d:resetDevice\n", freq);
			return;
		}
	}
	// temporarily disable eeprom load for griffin SNOOP: remove asap
	if (isGriffin(pLibDev->swDevID)) {
//		printf("Disabling EEPROM load for griffin\n");
//		pLibDev->eePromLoad = FALSE;
	}

	prev_freq = pLibDev->freqForResetDevice;

	pLibDev->turbo = 0;
	pLibDev->mdkErrno = 0;
	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:resetDevice\n", devNum);
		return;
	}
   	if (gLibInfo.pLibDevArray[devNum]->devState < INIT_STATE) {
		mError(devNum, EILSEQ, "Device Number %d:resetDevice: Device should be initialized before calling reset\n", devNum);
		return;
	}
// Dont call the driver init code for now
#if (MDK_AP) || (CUSTOMER_REL) || (!INCLUDE_DRIVER_CODE)
	pLibDev->use_init = !DRIVER_INIT_CODE;
#endif

	if (pLibDev->use_init == DRIVER_INIT_CODE) {
#if (!CUSTOMER_REL) && (INCLUDE_DRIVER_CODE)
#ifndef NO_LIB_PRINT
       printf("resetDevice::Using Driver init code\n"); 
#endif
	   if (m_ar5212Reset(pLibDev, mac, bss, freq, turbo) != 0) {

       }
	   if (!loadCfgData(devNum, freq))
		 {
				 return;
		 }
		if(!setupEEPromMap(devNum)) 
		{
			mError(devNum, EIO, "Error: unable to load EEPROM\n");
			return;
		}
#endif //CUSTOMER_REL
	}
	else {
    // load config data, if return value = 0 return from resetDevice
	if (!loadCfgData(devNum, freq))
	{
			return;
	}
//++JC++
	if((((pLibDev->swDevID & 0xff) == 0x0018) || isNala(pLibDev->swDevID)) && (pLibDev->mode == MODE_11A)) {
		pLibDev->mode = MODE_11G;
	}
//++JC++
	hwReset(devNum, BUS_RESET | BB_RESET | MAC_RESET);

	//Check pll control register, added by Dudi for RMA7147 & 7230
    temp = REGR(devNum, 0x987c);
	if(!temp){
	
		mError(devNum, EIO, "Device Number %d:ResetDevice: PLL Control reg is inaccessible\n", devNum);
		return;
	}
    
    // program the pll, check to see if we want to override it
	if(pLibDev->libCfgParams.applyPLLOverride) {
		REGW(devNum, 0xa200, 0);
		REGW(devNum, 0x987c, pLibDev->libCfgParams.pllValue);
		mSleep(2);
	}
	else {
		pllProgram(devNum, turbo);
	}
        
    // Put baseband into base/TURBO mode 
	pLibDev->turbo = turbo;  
	if(turbo == TURBO_ENABLE) {

		REGW(devNum, PHY_FRAME_CONTROL, REGR(devNum, PHY_FRAME_CONTROL) | PHY_FC_TURBO_MODE);

		//check the turbo bit is set
		//FJC, increased this timeout from 10 to 20, sometimes predator fails here.
		for (i = 0; i < 20; i++) {
			if (REGR(devNum, PHY_FRAME_CONTROL) & PHY_FC_TURBO_MODE) {
				break;
			}
			mSleep(1);
		}

		if(i == 10) {
			mError(devNum, EIO, "Device Number %d:ResetDevice: Unable to put the device into tubo mode\n", devNum);
			return;
		}

		// Reset the baseband 
		hwReset(devNum, BB_RESET);

	}
	else {
		// Base mode
		REGW(devNum, PHY_FRAME_CONTROL, REGR(devNum, PHY_FRAME_CONTROL) & ~PHY_FC_TURBO_MODE);
		//put these back, in case they were written for 11g turbo
	}

    /* Hack for AR6000 to stop reg defaulting so INI can be written */
	if (pLibDev->swDevID == 0x0022) {
        REGW(devNum, 0x99dc, 0xFFFFFFFF);
    }

	/* New section to handle mode switching */
	if(ar5kInitData[pLibDev->ar5kInitIndex].cfgVersion < 2) {
		switch(pLibDev->mode) {
		case MODE_11A:	//11a
			if ((pLibDev->swDevID == 0xe011) || (pLibDev->swDevID == 0xf11b)||(pLibDev->swDevID == 0x0012)) {
				changeField(devNum, "bb_enable_xpaa", 1);
				changeField(devNum, "bb_enable_xpab", 0);
				changeField(devNum, "rf_b_B_mode", 0);
			}
			break;

		case MODE_11G: //11g
			if ((pLibDev->swDevID == 0xe011) || (pLibDev->swDevID == 0xf11b)||(pLibDev->swDevID == 0x0012)) {
				changeField(devNum, "bb_enable_xpaa", 0);
				changeField(devNum, "bb_enable_xpab", 1);
				changeField(devNum, "rf_b_B_mode", 1);
			}

			break;

		case MODE_11B: //11b

			break;
		}
	}
	else {
		//Do the new handling from the ini files
		switch(pLibDev->mode) {
		case MODE_11A:	//11a
			if(pLibDev->turbo != TURBO_ENABLE) {	//11a base
				pValue = &(pLibDev->pModeArray->value11a);
			}
			else {			//11a turbo
				pValue = &(pLibDev->pModeArray->value11aTurbo);
			}

			break;

		case MODE_11G: //11g
		case MODE_11O: //ofdm@2.4
			if((pLibDev->turbo != TURBO_ENABLE) || (ar5kInitData[pLibDev->ar5kInitIndex].cfgVersion < 3)) {	//11g base
				pValue = &(pLibDev->pModeArray->value11g);
			}
			else {
				pValue = &(pLibDev->pModeArray->value11gTurbo);
			}
			break;

		case MODE_11B: //11b
			pValue = &(pLibDev->pModeArray->value11b);
			break;
		} //end switch
		//do all the mode change fields
		for(i = 0; i < pLibDev->sizeModeArray; i++) {
//			changeField(devNum, pLibDev->regArray[pLibDev->pModeArray[i].indexToMainArray].fieldName, 
//				*pValue);
			updateField(devNum, &pLibDev->regArray[pLibDev->pModeArray[i].indexToMainArray], 
				*pValue, 0);
			//increment value pointer by size of an array element
			pValue = (A_UINT32 *)((A_CHAR *)pValue + sizeof(MODE_INFO));
		}
	}

#ifdef LINUX
	{
		static int is_hydra = -1;
		if (is_hydra == 1 || (is_hydra && system("cat /proc/cpuinfo | grep -q '^system type.*Atheros AR7'") == 0)) {
			is_hydra = 1;
			changeField(devNum, "mc_max_dma_size_rd", 0);
			changeField(devNum, "mc_max_dma_size_wr", 0);
		} else is_hydra = 0;
	}
#endif

	//workaround needed for first rev of Oahu
	if(pLibDev->macRev == 0x40) {
		changeField(devNum, "mc_disable_dynamic_clock", 1);
	}

	//see if need to perform any tx threshold adjusting
	if(pLibDev->adjustTxThresh) {
		getField(devNum, "mc_trig_level", &newThreshold, &newThresholdTurbo);
		if((pLibDev->turbo == TURBO_ENABLE) && (ar5kInitData[pLibDev->ar5kInitIndex].cfgVersion < 2)) {
			newThreshold = newThresholdTurbo * 2;
		}
		else {
			newThreshold = newThreshold * 2;
		}
		if (newThreshold > 0x3f) {
			newThreshold = 0x3f;
		}
		changeField(devNum, "mc_trig_level", newThreshold);
		pLibDev->adjustTxThresh  = 0;
	}

	if((pLibDev->swDevID & 0xff) >= 0x13) {
		if(pLibDev->libCfgParams.enableXR) {
			changeField(devNum, "bb_enable_xr", 1);
		}
		else {
			changeField(devNum, "bb_enable_xr", 0);
		}
	}

	
// phyonly reset. Used in the emulation board
#if defined(SPIRIT_AP) || defined(FREEDOM_AP)
#ifdef EMULATION
	REGW(devNum,0x9928,0);
	mSleep(10);
	REGW(devNum,0x9928,1);
#endif
#endif


#ifdef FREEDOM_AP
//	changeField(devNum, "rf_ovr", 1);
//	changeField(devNum, "rf_gain_I", 0x10);
#endif
	// Setup Values from EEPROM - only touches EEPROM the first time
	//FJC: 06/09/03 moved reading eeprom to here, since it reads the EAR
	if (pLibDev->eePromLoad) {
		if(pLibDev->swDevID >= 0x0012) {
			REGW(devNum, 0x6010, REGR(devNum, 0x6010) | 0x3);
		}
		if(!setupEEPromMap(devNum)) {
			mError(devNum, EIO, "Error: unable to load EEPROM\n");
			return;
		}
		if(pLibDev->p16kEepHeader->majorVersion >= 4) 
		{
			earHere = ar5212IsEarEngaged(devNum, pLibDev->pEarHead, freq);
		}
	}
	//do a check for EAR changes to rf registers
	if (earHere) {
           ar5212EarModify(devNum, pLibDev->pEarHead, EAR_LC_RF_WRITE, freq, &modifier);
        }

	  /* Initialize chips with values from register file */
	  for(i = 0; i < pLibDev->sizePciValuesArray; i++ ) {
#if defined(SPIRIT_AP) || defined(FREEDOM_AP)
#ifdef EMULATION
	    if (pLibDev->pciValuesArray[i].offset == 0x9928) continue;
#endif
#endif
		/*if dealing with a version 2 config file, 
		  then all values are found in the base array */
		pciValues[i].offset = pLibDev->pciValuesArray[i].offset;
		if((pLibDev->turbo == TURBO_ENABLE) && (ar5kInitData[pLibDev->ar5kInitIndex].cfgVersion < 2)) {
		   if (pLibDev->devMap.remoteLib) {
		     pciValues[i].baseValue = pLibDev->pciValuesArray[i].turboValue;
		   }
 		   else {
		        temp1 = pLibDev->pciValuesArray[i].turboValue;
		   }
		}
		else {
		   if (pLibDev->devMap.remoteLib) {
		     pciValues[i].baseValue = pLibDev->pciValuesArray[i].baseValue;
		   }
		   else {
		         temp1 = pLibDev->pciValuesArray[i].baseValue;
		   }
		}

		if (!pLibDev->devMap.remoteLib) {
			/* Preserve register byteswap */
			if (pLibDev->pciValuesArray[i].offset == F2_CFG) {
				temp1 |= REGR(devNum, F2_CFG) & F2_CFG_SWRG;
			}

#if defined(COBRA_AP)
			  if(pLibDev->pciValuesArray[i].offset < 0x14000)
#endif
				REGW(devNum, pLibDev->pciValuesArray[i].offset, temp1);
		}

//++JC++ For Griffin bank6 problem - temporary - remove after the problem is fixed
// Capture the data that is written to bank6
if((pLibDev->swDevID & 0xff) == 0x0018) {
	if (isGriffin_1_0(pLibDev->macRev) || isGriffin_1_1(pLibDev->macRev)) {
		if((pLibDev->pciValuesArray[i].offset == 0x989c) || (pLibDev->pciValuesArray[i].offset == 0x98d8)) { 
   
			if ((pLibDev->pciValuesArray[i].offset == 0x989c) && (bank6_over == 0)) {
				bank6_data[zz] = pLibDev->pciValuesArray[i].baseValue;
				zz++; 
			} else if ((pLibDev->pciValuesArray[i].offset == 0x98d8) && (zz >= 25)) {
				bank6_data[zz] = pLibDev->pciValuesArray[i].baseValue;
				bank6_over = 1;
			}
		}
	}
}
//++JC++
	} // end of for
	if (pLibDev->devMap.remoteLib) 
		sendPciWrites(devNum, pciValues, pLibDev->sizePciValuesArray);
//++ALEX
// For 2464 Tx spur workaround
	if(isNala(pLibDev->swDevID) &&  (pLibDev->libCfgParams.ctlToApply & 0xf0) != 0x30)
	{
		writeField(devNum, "bb_dac_clk_select", 2);
	}

//++JC++
// Form the data that needs to be compared with data read out of bank6
if(isGriffin(pLibDev->swDevID)) {
	if (isGriffin_1_0(pLibDev->macRev) || isGriffin_1_1(pLibDev->macRev)) {
		di_31_00 = (bank6_data[zz - 8] & 0xff) | ((bank6_data[zz - 7] & 0xff) << 8) |
			((bank6_data[zz - 6] & 0xff) << 16) | ((bank6_data[zz - 5] & 0xff) << 24);

		di_63_32 = (bank6_data[zz - 4] & 0xff) | ((bank6_data[zz - 3] & 0xff) << 8) |
			((bank6_data[zz - 2] & 0xff) << 16) | ((bank6_data[zz - 1] & 0xff) << 24);

		di_67_64 = (bank6_data[zz] & 0xf);

		// Program reg5 to read out tx chain
		REGW(devNum, (0x9800+(0x34<<2)), 0x0);
		os1 = 1;
		os0 = 0;

		dreg1 = 0;
		dreg0 = 0;

		da2 = 1;
		da1 = 1;
		da0 = 0;

		tmp = da0<<20 | da1<<19 | da2<<18 | dreg0<<17 | dreg1<<16
         		| 0x5<<2 | os0<<1 | os1;

		REGW(devNum, (0x9800+(0x34<<2)), tmp);
		
		//------------------------------------------------------------------------
		//loop to do write and check
		//------------------------------------------------------------------------

		shift_good = 0;
		num_tries  = 0;
		
		while ((shift_good == 0) && (num_tries<1000)) {

			//
			// shift in data
			//
			for (kk=0;kk<27;kk++) {
				if (kk==26) {
					offset = 0x36;
				} else {
					offset = 0x27;
				}
				data = bank6_data[zz-26+kk];
		
				REGW(devNum,0x9800+(offset<<2), data);
			}
			//
			// shift out data
			//
			for (kk=0; kk<32; kk++) {
	        		REGW(devNum,(0x9800+(0x20<<2)), 0x00010000);
			}
			do_31_00 = REGR(devNum,0x9800+(256<<2));
		
			for (kk=0; kk<32; kk++) {
				REGW(devNum,(0x9800+(0x20<<2)), 0x00010000);
	    		}
			do_63_32 = REGR(devNum,0x9800+(256<<2));
		
			for (kk=0; kk<4; kk++) {
				REGW(devNum,(0x9800+(0x20<<2)), 0x00010000);
			}
			do_67_64 = (REGR(devNum,0x9800+(256<<2)))>>28;
		
			//printf ("\n");
			//printf (" di: 0x%01x  0x%08x  0x%08x\n", di_67_64, di_63_32, di_31_00);
			//printf (" do: 0x%01x  0x%08x  0x%08x\n", do_67_64, do_63_32, do_31_00);
			if ((((di_31_00)&0xffffffff) != ((do_31_00)&0xffffffff)) 
				|| (((di_63_32)&0xffffffff) != ((do_63_32)&0xffffffff)) 
				|| ((di_67_64 & 0xf) != (do_67_64 & 0xf))) {
		
				//printf ("\n");
				//printf (" di: 0x%01x  0x%08x  0x%08x\n", di_67_64, di_63_32, di_31_00);
				//printf (" do: 0x%01x  0x%08x  0x%08x\n", do_67_64, do_63_32, do_31_00);
				shift_good = 0;
			} else {
				shift_good = 1;
			}
			num_tries++;
		}
	}
}
// End of workaround for griffin bank6 problem
//++JC++
	if((turbo == TURBO_ENABLE) && (ar5kInitData[pLibDev->ar5kInitIndex].cfgVersion < 3)) {
		//quick enable to venice 11g turbo.  Force minimum fields in here
		//version 3 config files have a 11g turbo column
		if(((pLibDev->swDevID & 0xff) >= 0x0013) && ((pLibDev->mode == MODE_11G)||(pLibDev->mode == MODE_11O))) {
			changeRegValueField(devNum, "bb_short20", 1);
			changeRegValueField(devNum, "rf_turbo", 1);
			changeRegValueField(devNum, "bb_turbo", 1);
			changeRegValueField(devNum, "mc_turbo_mode", 1);
			changeRegValueField(devNum, "bb_dyn_ofdm_cck_mode", 0);
			changeRegValueField(devNum, "bb_agc_settling", 0x25);
			changeRegValueField(devNum, "mc_sifs_dcu", 480);
			changeRegValueField(devNum, "mc_slot_dcu", 480);
			changeRegValueField(devNum, "mc_eifs_dcu", 4480);
			changeRegValueField(devNum, "mc_usec_duration", 80);
			changeRegValueField(devNum, "mc_sifs_duration_usec", 6);
			changeRegValueField(devNum, "mc_cts_timeout", 0x12c0);
			changeRegValueField(devNum, "mc_ack_timeout", 0x12c0);
			changeRegValueField(devNum, "mc_usec", 79);
		}
	}
	if(((pLibDev->swDevID & 0xff) >= 0x0013) && (pLibDev->mode == MODE_11O)) {
		changeRegValueField(devNum, "bb_dyn_ofdm_cck_mode", 0);
	}
/* Remove this after moving this value to config file  */


#ifdef FREEDOM_AP
/*
	REGW(devNum,0x9848,0x0018d410);
	REGW(devNum,0x996c, 0x1301); // sigma-delta control
	REGW(devNum, 0x982c, 0x0002effe);
*/
#endif



	/* delta_slope_coeff_exp and delta_slope_coeff_man for venice */
	if ((pLibDev->swDevID & 0x00ff) >= 0x0013) {
		double fclk,coeff;
		A_UINT32 coeffExp,coeffMan;
		A_UINT32 deltaSlopeCoeffMan, deltaSlopeCoeffExp;
		A_UINT32 progCoeff;

		switch (pLibDev->mode) {
			case MODE_11A:
				if(pLibDev->swDevID == 0xf013) {
					fclk = 16.0;
				}
				else {
					switch (turbo) {
					case QUARTER_SPEED_MODE:
						fclk = 10;
						if(isEagle(pLibDev->swDevID)) {
							writeField(devNum, "bb_quarter_rate_mode", 1);
							writeField(devNum, "bb_half_rate_mode", 0);
							writeField(devNum, "bb_window_length", 3);
						}
						break;
					case HALF_SPEED_MODE:
						fclk = 20;
						if(isEagle(pLibDev->swDevID)) {
							writeField(devNum, "bb_quarter_rate_mode", 0);
							writeField(devNum, "bb_half_rate_mode", 1);
							writeField(devNum, "bb_window_length", 3);
						}
						break;
					case TURBO_ENABLE:
						fclk = 80;
						if(isEagle(pLibDev->swDevID)) {
							writeField(devNum, "bb_quarter_rate_mode", 0);
							writeField(devNum, "bb_half_rate_mode", 0);
							writeField(devNum, "bb_window_length", 0);
						}
						break;
					default :   // base 
						fclk = 40;
						if(isEagle(pLibDev->swDevID)) {
							writeField(devNum, "bb_quarter_rate_mode", 0);
							writeField(devNum, "bb_half_rate_mode", 0);
							writeField(devNum, "bb_window_length", 0);
						}
						break;
					}
				}
				progCoeff = 1;
				break;
			case MODE_11G:
			case MODE_11O:
				if(pLibDev->swDevID == 0xf013) {
					fclk = (16.0 * 10.0) / 11.0;
				}
				else {
					switch (turbo) {
					case QUARTER_SPEED_MODE:
						fclk = 10;
						if(isEagle(pLibDev->swDevID)) {
							writeField(devNum, "bb_quarter_rate_mode", 1);
							writeField(devNum, "bb_half_rate_mode", 0);
							writeField(devNum, "bb_window_length", 3);
						}
						break;
					case HALF_SPEED_MODE:
						fclk = 20;
						if(isEagle(pLibDev->swDevID)) {
							writeField(devNum, "bb_quarter_rate_mode", 0);
							writeField(devNum, "bb_half_rate_mode", 1);
							writeField(devNum, "bb_window_length", 3);
						}
						break;
					case TURBO_ENABLE:
						fclk = 80;
						if(isEagle(pLibDev->swDevID)) {
							writeField(devNum, "bb_quarter_rate_mode", 0);
							writeField(devNum, "bb_half_rate_mode", 0);
							writeField(devNum, "bb_window_length", 0);
						}
						break;
					default :   // base 
						fclk = 40;
						if(isEagle(pLibDev->swDevID)) {
							writeField(devNum, "bb_quarter_rate_mode", 0);
							writeField(devNum, "bb_half_rate_mode", 0);
							writeField(devNum, "bb_window_length", 0);
						}
						break;
					}
				}
				progCoeff = 1;
				break;
			default:
				fclk = 0;
				progCoeff = 0;
				break;
		}
		if (progCoeff) {
			coeff = (2.5 * fclk) / ((double)freq);
			coeffExp = 14 -  (int)(floor(log10((double)coeff)/log10((double)2)));
			coeffMan = (int)(floor((coeff*(pow((double)2,(double)coeffExp))) + 0.5));
			deltaSlopeCoeffExp = coeffExp - 16;
			deltaSlopeCoeffMan = coeffMan;
			REGW(devNum, 0x9814, (REGR(devNum,0x9814) & 0x00001fff) | 
								 (deltaSlopeCoeffExp << 13) |
								      (deltaSlopeCoeffMan << 17));


		}
	}

	//Disable all the mac queue clocks
	if((pLibDev->swDevID != 0x0007) && (pLibDev->swDevID != 0x0010)) {
		disable5211QueueClocks(devNum);
	}
// Byteswap Tx and Rx descriptors for Big Endian systems
#if defined(ARCH_BIG_ENDIAN)
	REGW(devNum, F2_CFG, REGR(devNum, F2_CFG) | (F2_CFG_SWTD | F2_CFG_SWRD));
#endif


#ifdef HEADER_LOAD_SCHEME
	else if (pLibDev->eePromHeaderLoad) {
		setupEEPromHeaderMap(devNum);
	}
#endif //HEADER_LOAD_SCHEME

    // Setup the macAddr in the chip
	memcpy(pLibDev->macAddr.octets, mac, WLAN_MAC_ADDR_SIZE);
#ifndef ARCH_BIG_ENDIAN
	REGW(devNum, F2_STA_ID0, pLibDev->macAddr.st.word);
	temp1 = REGR(devNum, F2_STA_ID1); 
	temp2 = (temp1 & 0xffff0000) | F2_STA_ID1_AD_HOC | pLibDev->macAddr.st.half | F2_STA_ID1_DESC_ANT;
	REGW(devNum, F2_STA_ID1, temp2); 
#else
	{
		A_UINT32 addr;
		
		addr = swap_l(pLibDev->macAddr.st.word);
		REGW(devNum, F2_STA_ID0, addr);
		addr = (A_UINT32)swap_s(pLibDev->macAddr.st.half);
		REGW(devNum, F2_STA_ID1, (REGR(devNum, F2_STA_ID1) & 0xffff0000) | F2_STA_ID1_AD_HOC |
				addr | F2_STA_ID1_DESC_ANT);
	}
#endif

	// then our BSSID
	memcpy(pLibDev->bssAddr.octets, bss, WLAN_MAC_ADDR_SIZE);
#ifndef ARCH_BIG_ENDIAN
	REGW(devNum, F2_BSS_ID0, pLibDev->bssAddr.st.word);
	REGW(devNum, F2_BSS_ID1, pLibDev->bssAddr.st.half);
#else
	{
		A_UINT32 addr;

		addr = swap_l(pLibDev->bssAddr.st.word);		
		REGW(devNum, F2_BSS_ID0, addr);
		addr = (A_UINT32)swap_s(pLibDev->bssAddr.st.half);
		REGW(devNum, F2_BSS_ID1, addr);
	}
#endif

	REGW(devNum, F2_BCR, F2_BCR_STAMODE);
	REGR(devNum, F2_BSR); // cleared on read


	// enable activity leds and clock run enable
#if defined(COBRA_AP)
	sysRegWrite(AR531XPLUS_PCI + F2_PCICFG, sysRegRead(AR531XPLUS_PCI + F2_PCICFG) | F2_PCICFG_CLKRUNEN | F2_PCICFG_LED_ACT);
#else
//	REGW(devNum, F2_PCICFG, REGR(devNum, F2_PCICFG) | F2_PCICFG_CLKRUNEN | F2_PCICFG_LED_ACT);
#endif

	// Set the RX Disable to stop even packets outside of RX_FILTER (ProbeRequest)
	// Note that scripts must turn off RX Disable to receive packets inc. ACKs.
//	REGW(devNum, F2_DIAG_SW, F2_DIAG_RX_DIS);
	ar5kInitData[pLibDev->ar5kInitIndex].pMacAPI->disableRx(devNum);

	// Writing to F2_BEACON will start timers. Hence it should be the last
	// register to be written.
	REGW(devNum, F2_BEACON, F2_BEACON_RESET_TSF | 0xffff);

    // Setup board specific options
    if (pLibDev->eePromLoad) {
		assert(pLibDev->eepData.eepromChecked);
		if((((pLibDev->eepData.version >> 12) & 0xF) == 1) ||
			(((pLibDev->eepData.version >> 12) & 0xF) == 2)){
			REGW(devNum, PHY_BASE+(10<<2), (REGR(devNum, PHY_BASE+(10<<2)) & 0xFFFF00FF) |
				(pLibDev->eepData.xlnaOn << 8));
			REGW(devNum, PHY_BASE+(13<<2), (pLibDev->eepData.xpaOff << 24) |
				(pLibDev->eepData.xpaOff << 16) | (pLibDev->eepData.xpaOn << 8) |
				pLibDev->eepData.xpaOn);
			REGW(devNum, PHY_BASE+(17<<2), (REGR(devNum, PHY_BASE+(17<<2)) & 0xFFFFC07F) |
				((pLibDev->eepData.antenna >> 1) & 0x3F80));
			REGW(devNum, PHY_BASE+(18<<2), (REGR(devNum, PHY_BASE+(18<<2)) & 0xFFFC0FFF) |
				((pLibDev->eepData.antenna << 10) & 0x3F000));
			REGW(devNum, PHY_BASE+(25<<2), (REGR(devNum, PHY_BASE+(25<<2)) & 0xFFF80FFF) |
				((pLibDev->eepData.thresh62 << 12) & 0x7F000));
			REGW(devNum, PHY_BASE+(68<<2), (REGR(devNum, PHY_BASE+(68<<2)) & 0xFFFFFFFC) |
				(pLibDev->eepData.antenna & 0x3));
		}
	}
	// Setup the transmit power values for cards since 0x0[0-2]05
	pLibDev->freqForResetDevice = freq;
	initializeTransmitPower(devNum, freq, 0, NULL);
	setChannel(devNum, freq);
	
	/* set spur mitigation for COBRA and Eagle */

	if(pLibDev->mode == MODE_11G || pLibDev->mode == MODE_11A) {
		//if(isCobra(pLibDev->swDevID) || isEagle(pLibDev->swDevID)) //Cobra, Spider1_0, Eagle or Condor or swan
			setSpurMitigation(devNum, freq);
		//else if(pLibDev->subSystemID == 0x2055)  //NALA  0x2055 = mb55
		//	setSpurMitigation_Nala(devNum, freq);
	}

    //check for any EAR modifications needed before PHY_ACTIVATE
	if (earHere) {
        ar5212EarModify(devNum, pLibDev->pEarHead, EAR_LC_PHY_ENABLE, freq, &modifier);
    }

 /* overwrite for quarter rate mode -- start */ 
   if ((pLibDev->swDevID & 0xff) == 0x0013) {
			 /* tx power control measure time */
	 		 /* wait after reset */
		   if (!pwrParams.initialized[pLibDev->mode]) {
			   pwrParams.initialized[pLibDev->mode] = TRUE;
				
				 //pwrParams.bb_active_to_receive[pLibDev->mode] = getFieldForMode(devNum, "bb_active_to_receive", pLibDev->mode, 0); // base mode vals
				 pwrParams.bb_active_to_receive[pLibDev->mode] = (REGR(devNum, 0x9914) & 0x3FFF);

    			 pwrParams.rf_wait_I[pLibDev->mode] = getFieldForMode(devNum, "rf_wait_I", pLibDev->mode, 0);
				 pwrParams.rf_wait_S[pLibDev->mode] = getFieldForMode(devNum, "rf_wait_S", pLibDev->mode, 0);
				 pwrParams.rf_max_time[pLibDev->mode] = getFieldForMode(devNum, "rf_max_time", pLibDev->mode, 0);
		   }

		   temp1 = pwrParams.bb_active_to_receive[pLibDev->mode];
		   temp2 = pwrParams.rf_wait_I[pLibDev->mode];
		   temp3 = pwrParams.rf_wait_S[pLibDev->mode];
		   temp4 = pwrParams.rf_max_time[pLibDev->mode];

		   if (turbo == HALF_SPEED_MODE) {
				 temp1 = temp1 >> 1; 
				 temp2 = temp2 << 1;
				 temp3 = temp3 << 1;
				 temp4 = temp4 + 1;
			 } else if (turbo == QUARTER_SPEED_MODE) {
				 temp1 = temp1 >> 2;
				 temp2 = temp2 << 2;
				 temp3 = temp3 << 2;
				 temp4 = temp4 + 2;
			 } else if (turbo == TURBO_ENABLE) {
				 temp1 = temp1 << 1; 
				 temp2 = temp2 >> 1;
				 temp3 = temp3 >> 1;				 
		 	 } 
		 
		 	 temp1 = (temp1 > 0x3FFF) ? 0x3FFF : temp1;  // limit to max field size of bb_active_to_receive
			 temp2 = (temp2 > 31) ? 31 : temp2;  // limit to max field size of rf_wait_I
			 temp3 = (temp3 > 31) ? 31 : temp3;  // limit to max field size of rf_wait_S
			 temp4 = (temp4 > 3)  ?  3 : temp4;  // limit to max field size of rf_max_time
			 
			 writeField(devNum, "bb_active_to_receive", temp1);   
			 writeField(devNum, "rf_wait_I", temp2); 
			 writeField(devNum, "rf_wait_S", temp3);
			 writeField(devNum, "rf_max_time", temp4);
  }
   if (pLibDev->swDevID == SW_DEVICE_ID_CONDOR || pLibDev->swDevID == 0x0018 || pLibDev->swDevID == 0x0019 || isCobra(pLibDev->swDevID)
	   || pLibDev->swDevID == SW_DEVICE_ID_SWAN || isNala(pLibDev->swDevID)) {
	   // add code for griffin here
   } else 
    if(((pLibDev->swDevID & 0xff) == 0x0014)||((pLibDev->swDevID & 0xff) >= 0x0016)) { /* venice derby	   if (turbo == QUARTER_SPEED_MODE)  */
			 /* tx power control measure time */
	 		 /* wait after reset */
	 		   if (!pwrParams.initialized[pLibDev->mode]) {
				   pwrParams.initialized[pLibDev->mode] = TRUE;				

//				   pwrParams.bb_active_to_receive[pLibDev->mode] = getFieldForMode(devNum, "bb_active_to_receive", pLibDev->mode, 0); // base mode vals
//				   pwrParams.bb_tx_frame_to_tx_d_start[pLibDev->mode] = getFieldForMode(devNum, "bb_tx_frame_to_tx_d_start", pLibDev->mode, 0); // base mode vals
					
				   // do a regread instead of getFieldForMode to capture EAR changes. 
				   pwrParams.bb_active_to_receive[pLibDev->mode] = (REGR(devNum, 0x9914) & 0x3FFF);
				   pwrParams.bb_tx_frame_to_tx_d_start[pLibDev->mode] = (REGR(devNum, 0x9824) & 0xFF);
				   
				   pwrParams.rf_pd_period_a[pLibDev->mode] = getFieldForMode(devNum, "rf_pd_period_a", pLibDev->mode, 0); // base mode vals
				   pwrParams.rf_pd_delay_a[pLibDev->mode] = getFieldForMode(devNum, "rf_pd_delay_a", pLibDev->mode, 0); // base mode vals
				   pwrParams.rf_pd_period_xr[pLibDev->mode] = getFieldForMode(devNum, "rf_pd_period_xr", pLibDev->mode, 0); // base mode vals
				   pwrParams.rf_pd_delay_xr[pLibDev->mode] = getFieldForMode(devNum, "rf_pd_delay_xr", pLibDev->mode, 0); // base mode vals
				   
			   }			   


			   temp1 = pwrParams.bb_active_to_receive[pLibDev->mode];
			   temp2 = pwrParams.rf_pd_period_a[pLibDev->mode];
			   temp3 = pwrParams.rf_pd_delay_a[pLibDev->mode];
			   temp4 = pwrParams.rf_pd_period_xr[pLibDev->mode];
     		   temp5 = pwrParams.rf_pd_delay_xr[pLibDev->mode];
			   temp6 = pwrParams.bb_tx_frame_to_tx_d_start[pLibDev->mode];

			 if (turbo == TURBO_ENABLE) {
				 temp1 = temp1 << 1; 
				 temp2 = temp2 >> 1;
				 temp3 = temp3 >> 1;
				 temp4 = temp4 >> 1;
				 temp5 = temp5 >> 1;
			 } else if (turbo == HALF_SPEED_MODE) {
				 temp1 = temp1 >> 1; 
				 temp2 = temp2 << 1;
				 temp3 = temp3 << 1;
				 temp4 = temp4 << 1;
				 temp5 = temp5 << 1;
			 } else if (turbo == QUARTER_SPEED_MODE) {
				 temp1 = temp1 >> 2;
				 temp2 = temp2 << 2;
				 temp3 = temp3 << 2;
				 temp4 = temp4 << 2;
				 temp5 = temp5 << 2;
				 temp6 -= ((temp6 > 5) ? 5 : 0);
			 } 

			 temp1 = (temp1 > 0x3FFF) ? 0x3FFF : temp1;  // limit to max field size of bb_active_to_receive
			 temp2 = (temp2 > 15) ? 15 : temp2;  // limit to max field size of rf_wait_I
			 temp3 = (temp3 > 15) ? 15 : temp3;  // limit to max field size of rf_wait_S
			 temp4 = (temp4 > 15) ? 15 : temp4;  // limit to max field size of rf_wait_I
			 temp5 = (temp5 > 15) ? 15 : temp5;  // limit to max field size of rf_wait_S
			 writeField(devNum, "bb_active_to_receive", temp1);   
			 writeField(devNum, "rf_pd_period_a", temp2); 
			 writeField(devNum, "rf_pd_delay_a", temp3);
			 writeField(devNum, "rf_pd_period_xr", temp4); 
			 writeField(devNum, "rf_pd_delay_xr", temp5);
			 writeField(devNum, "bb_tx_frame_to_tx_d_start", temp6);
   }
   
   /* overwrite for quarter rate mode -- end */ 
	// activate D2
	REGW(devNum, PHY_ACTIVE, PHY_ACTIVE_EN);
	mSleep(3);

	if( (pLibDev->swDevID == 0xf11b) && (pLibDev->mode == MODE_11B)) {
		REGW(devNum, 0xd87c,0x19);
		mSleep(4);
	}
	if( (pLibDev->swDevID == 0xf11b) && ((pLibDev->mode == MODE_11B) || (pLibDev->mode == MODE_11G))) {
		REGW(devNum, 0xd808,0x502); 
		mSleep(2);
	}

	if (isGriffin_1_1(pLibDev->macRev)) {
		REGW(devNum,0xa358, (REGR(devNum,0xa358) | 0x3));
	}
	//enableCal flag should have the bits set for CAL
	// calibrate it and poll the bit going to 0 for completion
	if (enableCal) {
		REGW(devNum, PHY_AGC_CONTROL,
			REGR(devNum, PHY_AGC_CONTROL) | enableCal);

		// noise cal takes a long time to complete in simulation
		// need not wait for the noise cal to complete
#ifdef SIM
			enableCal = DO_OFSET_CAL;
#endif


		// Do remote check
		if (pLibDev->devMap.remoteLib) {
			timeout = 32000;
	   	  i = pLibDev->devMap.r_calCheck(devNum, enableCal, timeout);
		}
		else {
		  timeout = 1000;
		  for (i = 0; i < timeout; i++)
		  {
			if ((REGR(devNum, PHY_AGC_CONTROL) & (enableCal)) == 0 )
			{
				break;
			}
			mSleep(1);
		  }
		}
		if(i >= timeout) {
			//mError(EIO, "Device Number %d:resetDevice: device failed to finish offset and/or noisefloor calibration in 10 ms\n");
			printf("resetDevice : Device Number %d:Didn't complete cal after timeout(%d) but keep going anyway\n", devNum, timeout);
		}

//++JC++
		// For Griffin alone - To work around Carrier Leak Problem
		if(isGriffin(pLibDev->swDevID) || isEagle(pLibDev->swDevID)) {
			if (isGriffin_1_0(pLibDev->macRev)) {
    			griffin_cl_cal(devNum);
			} else {
				REGW(devNum,0xa358, (REGR(devNum,0xa358) | 0x3));
			}
		} 
		// End of workaround for griffing Carrier Leak Problem
//++JC++

	}

	else {
	}

	//check for any post reset EAR modificatons
	if (earHere) {
        ar5212EarModify(devNum, pLibDev->pEarHead, EAR_LC_POST_RESET, freq, &modifier);
    }

    
	} // end of else (use_init...

	if((isGriffin(pLibDev->swDevID) || isEagle(pLibDev->swDevID)) && (!pLibDev->libCfgParams.noUnlockEeprom)) {
		if (!isNala(pLibDev->swDevID))
		{
			if(isCondor(pLibDev->swDevID) && ! isSwan(pLibDev->swDevID)){
			//need to remove the write protection from eeprom
				REGW(devNum, F2_GPIOCR, REGR(devNum, F2_GPIOCR) | F2_GPIOCR_3_CR_A);
				gpioData = 0x1 << 3;
				REGW(devNum, F2_GPIODO, REGR(devNum, F2_GPIODO) & ~gpioData);
			}
			else if (isSwan(pLibDev->swDevID))
			{
				//REGW(devNum, F2_GPIOCR, REGR(devNum, F2_GPIOCR) | F2_GPIOCR_4_CR_A);
				REGW(devNum, F2_GPIOCR, REGR(devNum, F2_GPIOCR) | 0xC);
				//gpioData = 0x11 << 2;
				gpioData = 0x02;
				REGW(devNum, F2_GPIODO, REGR(devNum, F2_GPIODO) & ~gpioData); 
			}
			//need to remove the write protection from eeprom
			REGW(devNum, F2_GPIOCR, REGR(devNum, F2_GPIOCR) | F2_GPIOCR_4_CR_A);
			gpioData = 0x1 << 4;
			REGW(devNum, F2_GPIODO, REGR(devNum, F2_GPIODO) & ~gpioData);
		}
	}
	
	pLibDev->devState = RESET_STATE;
#ifndef __ATH_DJGPPDOS__
	// ART ANI routine
	tweakArtAni(devNum, prev_freq, pLibDev->freqForResetDevice);
#endif
#ifndef CUSTOMER_REL
	resetDevice_end=milliTime();
//printf("SNOOP::exit resetDevice:Time taken = %dms\n", (resetDevice_end-resetDevice_start));
#endif
}


/**************************************************************************
 * testLib -  test function  that check for software interrupt (added to 
 *			  create dktest functionality).  Returns TRUE if pass, FALSE 
 *			  otherwise
 *
 */
MANLIB_API A_BOOL testLib
(
 A_UINT32 devNum,
 A_UINT32 timeout
)
{
	ISR_EVENT	event;
	LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
	A_UINT32	i;

	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:testLib\n", devNum);
		printf("Device Number %d:testLib\n", devNum);
		return(FALSE);
	}
	if (pLibDev->devState < RESET_STATE) {
		mError(devNum, EILSEQ, "Device Number %d:testLib: Device should be out of Reset before testing library\n", devNum);
		printf("Device Number %d:testLib: Device should be out of Reset before testing library\n", devNum);
		return(FALSE);
	}

	if (isGriffin(pLibDev->swDevID)) {
		if (isGriffin_1_0(pLibDev->macRev) || isGriffin_1_1(pLibDev->macRev)) {
			printf("******************\n\n");
			printf(" register test disabled for griffin on mb51. \n");
			printf("    enable it when griffin is fixed\n");
			printf("******************\n\n");
			return(TRUE);
		}
	}
#ifndef SIM
	//enable software interrupt
	REGW(devNum, F2_IMR, REGR(devNum, F2_IMR) | F2_IMR_SWI);
#endif

	//#################Hard code maui2 for now
	REGW(devNum, 0xA0, REGR(devNum, 0xA0) | F2_IMR_SWI);

#ifndef SIM
	//clear ISR before enable
	REGR(devNum, F2_ISR);
#endif

	REGR(devNum, 0x80);
	REGW(devNum, F2_IER, F2_IER_ENABLE);

	//create the software interrupt
	REGW(devNum, F2_CR, REGR(devNum, F2_CR) | F2_CR_SWI);

	//wait for software interrupt
	for (i = 0; i < timeout; i++)
	{
		event = pLibDev->devMap.getISREvent(devNum);
		if (event.valid)
		{
            //see if it is the SW interrupt
			if (event.ISRValue & F2_ISR_SWI) {
				//This is the event we are waiting for
				break;
			}
		}
		mSleep(1);		
	}
	if (i == timeout) {
		mError(devNum, 21, "Device Number %d:Error: Software interrupt not received\n", devNum);
		return(FALSE);
	}
/*
	//perform a simple register test
	saveRegValue = REGR(devNum, F2_STA_ID0);
	for(i = 0; i < sizeof(testValues)/sizeof(A_UINT32); i++) {
		REGW(devNum, F2_STA_ID0, testValues[i]);
		testRegValue = REGR(devNum, F2_STA_ID0);
		if(testRegValue != testValues[i]) {
			mError(devNum, 71, "Device Number %d:Failed simple register test, wrote %x, read %x\n", devNum, testValues[i], testRegValue);
			REGW(devNum, F2_STA_ID0, saveRegValue);
			return(FALSE);
		}
	}
	//do a walking 1 pattern test
	mask = 0x01;
	for(i = 0; i < 32; i++) {
		REGW(devNum, F2_STA_ID0, mask);
		testRegValue = REGR(devNum, F2_STA_ID0);
		if(testRegValue != mask) {
			mError(devNum, 71, "Failed simple register test, wrote %x, read %x\n", mask, testRegValue);
			REGW(devNum, F2_STA_ID0, saveRegValue);
			return(FALSE);
		}
		mask = mask << 1;
	}

	REGW(devNum, F2_STA_ID0, saveRegValue); */ /*commented by Dudi*/

    /* Self test PHY & MAC registers */  /*added by Dudi*/
{
    A_UINT32  regAddr[2] = {F2_STA_ID0, PHY_BASE+(8 << 2)};
    A_UINT32  regHold[2];
    A_UINT32  patternData[4] = {0x55555555, 0xaaaaaaaa, 0x66666666, 0x99999999};
    A_UINT32  addr, loop, wrData, rdData, pattern;
	
    for (i = 0; i < 2; i++) {
        addr       = regAddr[i];
        regHold[i] = REGR(devNum, addr);

        for (loop = 0; loop < 0x100; loop++) {

            wrData = (loop << 16) | loop;
            REGW(devNum, addr, wrData);
            rdData = REGR(devNum, addr);
            if (rdData != wrData) {
		  mError(devNum, 71, "Failed simple register test, addr: 0x%08x -wrote %x, read %x\n", addr, wrData, rdData);
                return FALSE;
            }
        }
        for (pattern = 0; pattern < 4; pattern++) {
            wrData = patternData[pattern];
            REGW(devNum, addr, wrData);
            rdData = REGR(devNum, addr);
            if (wrData != rdData) {
		  mError(devNum, 71, "Failed simple register test, addr: 0x%08x -wrote %x, read %x\n", addr, wrData, rdData);
                return FALSE;
            }
        }
        REGW(devNum, regAddr[i], regHold[i]);
    	}
}	
	return(TRUE);
}




/**************************************************************************
 * checkRegs - Perform register tests to various domains of the AR5K
 *
 */
MANLIB_API A_UINT32 checkRegs
(
 A_UINT32 devNum
)
{
	LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
    A_UINT32 failures = 0;
    A_UINT32 addr, i, loop, wrData, rdData, pattern;
    A_UINT32 regAddr[2] = {F2_STA_ID0, PHY_BASE+(8 << 2)};
    A_UINT32 regHold[2];
    A_UINT32 patternData[4] = {0x55555555, 0xaaaaaaaa, 0x66666666, 0x99999999};

	if(pLibDev->swDevID == 0x0007) {
		// Test PHY & MAC registers
		for(i = 0; i < 2; i++) {
			addr = regAddr[i];
			regHold[i] = REGR(devNum, addr);
			for (loop=0; loop < 0x10000; loop++) {

				wrData =  (loop << 16) | loop;
				REGW(devNum, addr, wrData);
				rdData = REGR(devNum, addr);
				if (rdData != wrData) {
					failures++;
				}
			}

			for (pattern=0; pattern<4; pattern++) {

				wrData = patternData[pattern];

				REGW(devNum, addr, wrData);
				rdData = REGR(devNum, addr);

				if (wrData != rdData) {
					failures++;
				}
			}
		}    

   		// Disable AGC to A2 traffic
		REGW(devNum, 0x9808, REGR(devNum, 0x9808) | 0x08000000);

		// Test Radio Register
		for (loop=0; loop < 10000; loop++) {
			wrData = loop & 0x3f;

			// ------ DAC 1 Write -------
			REGW(devNum, (PHY_BASE+(0x35<<2)), (reverseBits(wrData, 6) << 16) | (reverseBits(wrData, 6) << 8) | 0x24);      
			REGW(devNum, (PHY_BASE+(0x34<<2)), 0x0);
			REGW(devNum, (PHY_BASE+(0x34<<2)), 0x00120017);      
    
			for (i=0; i<18; i++) {
				REGW(devNum, PHY_BASE+(0x20<<2), 0x00010000);
			}
    
			rdData = reverseBits((REGR(devNum, PHY_BASE+(256<<2)) >> 26) & 0x3f, 6);
    
			if (rdData != wrData) {
					failures++;
			}
    
			REGW(devNum, (PHY_BASE+(0x34<<2)), 0x0);
			REGW(devNum, (PHY_BASE+(0x34<<2)), 0x00110017);      
    
			for (i=0; i<18; i++) {
				REGW(devNum, PHY_BASE+(0x20<<2), 0x00010000);
			}
    
			rdData = reverseBits((REGR(devNum, PHY_BASE+(256<<2)) >> 26) & 0x3f, 6);
    
			if (rdData != wrData) {
					failures++;
			}
		}
		REGW(devNum, (PHY_BASE+(0x34<<2)), 0x14);
		for(i = 0; i < 2; i++) {
			REGW(devNum, regAddr[i], regHold[i]);
		}

		// Re-enable AGC to A2 traffic
		REGW(devNum, 0x9808, REGR(devNum, 0x9808) & (~0x08000000));
		gLibInfo.pLibDevArray[devNum]->devState = INIT_STATE;
	}
	else {
		mError(devNum, EIO, "Device Number %d:CheckRegs not implemented for this deviceID\n", devNum);
	}

    return failures;
}

/**************************************************************************
 * changeChannel - Change the channel of the given device
 *
 */
MANLIB_API void changeChannel
(
 A_UINT32 devNum,
 A_UINT32 freq		// New channel
)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];

	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:changeChannel\n", devNum);
		return;
	}
	if (pLibDev->devState < RESET_STATE) {
		mError(devNum, EILSEQ, "Device Number %d:changeChannel: Device should be out of Reset before changing channel\n", devNum);
		return;
	}

	if((freq >= 49000) && (freq < 60000)) {
		if((freq % 10) == 0) {
			freq = freq / 10;
//printf("SNOOP: freq at 2= %d\n", freq);
		}
		else {
			//must be a 2.5 MHz channel - check
//printf("SNOOP: freq at 3= %d\n", freq);
			if((freq % 25) != 0) {
				mError(devNum, EINVAL, "Illegal channel value %d:changeChannel\n", freq);
				return;
			}
			pLibDev->channelMasks |= QUARTER_CHANNEL_MASK;
			freq = (freq - 25) / 10;
		}
	}
	if((freq >= 24120) && (freq <= 24840)) {
		if((freq % 10) == 0) {
			freq = freq / 10;
		}
		else {
			mError(devNum, EINVAL, "Illegal channel value %d:resetDevice\n", freq);
			return;
		}
	}

	// Disable AGC to A2 traffic
//	REGW(devNum, 0x9808, REGR(devNum, 0x9808) | 0x08000000);
	//AGC disable may not always work, this should
	REGW(devNum, PHY_ACTIVE, PHY_ACTIVE_DIS);
	mSleep(1);

	setChannel(devNum, freq);
	//set falseDetectBackoff.  May get overwritten by eeprom value
	if((pLibDev->swDevID & 0xff) >= 0x0012) {
		applyFalseDetectBackoff(devNum, freq, pLibDev->suppliedFalseDetBackoff[pLibDev->mode]);
	}


	//set the transmit power
	//#############Comment out for now, until fix the calibration sequencing issue.
	initializeTransmitPower(devNum, freq, 0, NULL);

	// Re-enable AGC to A2 traffic
//	REGW(devNum, 0x9808, REGR(devNum, 0x9808) & (~0x08000000));
	//AGC disable may not always work, this should
	REGW(devNum, PHY_ACTIVE, PHY_ACTIVE_EN);
	mSleep(1);

	// Set the noise floor after setting the channel.
	REGW(devNum, PHY_AGC_CONTROL, REGR(devNum, PHY_AGC_CONTROL) | PHY_AGC_CONTROL_NF);
	mSleep(2);
}

/**************************************************************************
 * setChannel - Perform the algorithm to change the channel
 *
 */
A_BOOL setChannel
(
 A_UINT32 devNum,
 A_UINT32 freq		// New channel
)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
	A_UINT32 i = pLibDev->ar5kInitIndex;

	if (ar5kInitData[i].pRfAPI->setChannel == NULL)
	{
		return(0);
	}
	return (ar5kInitData[i].pRfAPI->setChannel(devNum, freq));

}

/**************************************************************************
 * getSpurChans - get spur frequencies
 * return a pointer to spur array (and numSpurs), or NULL for nothing
 */
A_UINT32 *getSpurChans(LIB_DEV_INFO *pLibDev, A_UINT32 *numSpurs)
{
	static A_UINT32	spurChans11G_CobraEagle[5] = {2400,2420,2440, 2464,2480};	//involve Spider1_0, Condor and Swan
	static A_UINT32	spurChans11G_Nala[4] = {2420,2440,2464,2480};
	static A_UINT32	spurChans11A[1] = {5800};
	A_UINT32	ssize=0, i;
	A_UINT32	*spurFreq;

	//check spur on demand first, then compatible legacy code
	if (pLibDev->mode == MODE_11A)
	{
		spurFreq = pLibDev->spur_frequency_5g;
		ssize = EEPROM_SPUR_11A_SIZE;
	}
	else if (pLibDev->mode == MODE_11G)
	{
		spurFreq = pLibDev->spur_frequency_2p4g;
		ssize = EEPROM_SPUR_11G_SIZE;
	}
	for (i=0; i<ssize; i++)
	{
		if (spurFreq[i] == 0)
		{
			if (i)
			{
				*numSpurs = i;
				return spurFreq;
			}
			break;
		}
	}

	//do legacy code here
	*numSpurs = 0;
	spurFreq = NULL;
	if(isCobra(pLibDev->swDevID) || isEagle(pLibDev->swDevID))
	{
		if (pLibDev->mode == MODE_11A)
		{
			spurFreq = spurChans11A;
			*numSpurs = sizeof(spurChans11A)/sizeof(A_UINT32);
		}
		else if (pLibDev->mode == MODE_11G)
		{
			spurFreq = spurChans11G_CobraEagle;
			*numSpurs = sizeof(spurChans11G_CobraEagle)/sizeof(A_UINT32);
		}
	}
	else if(isNala(pLibDev->swDevID))
	{
		if (pLibDev->mode == MODE_11A)		//nala without 11A!!
		{
			spurFreq = spurChans11A;
			*numSpurs = sizeof(spurChans11A)/sizeof(A_UINT32);
		}
		else if (pLibDev->mode == MODE_11G)
		{
			spurFreq = spurChans11G_Nala;
			*numSpurs = sizeof(spurChans11G_Nala)/sizeof(A_UINT32);
		}
	}
	return spurFreq;
}

#define SPUR_CHAN_WIDTH       87
#define BIN_WIDTH_BASE_100HZ  3125
#define BIN_WIDTH_TURBO_100HZ 6250
/**************************************************************
 * setSpurMitigation
 *
 * Apply Spur Immunity to Boards that require it.
 * Applies only to OFDM RX operation.
 */
void setSpurMitigation(A_UINT32 devNum, A_UINT32 freq)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
    A_UINT32      pilotMask[2] = {0, 0}, binMagMask[4] = {0, 0, 0 , 0};
	A_UINT32      *spurChannels;
    A_UINT16      i;
    A_UINT16      finalSpur, curChanAsSpur, binWidth = 0, spurDetectWidth, spurChan;
    A_INT32       spurDeltaPhase = 0, spurFreqSd = 0, spurOffset, binOffsetNumT16, curBinOffset;
    A_INT16       numBinOffsets;
    A_UINT16      magMapFor4[4] = {1, 2, 2, 1};
    A_UINT16      magMapFor3[3] = {1, 2, 1};
    A_UINT16      *pMagMap;
	A_UINT32	  numSpurs, offset;
    
	if ((spurChannels=getSpurChans(pLibDev, &numSpurs)) == NULL) return;

    if(pLibDev->mode == MODE_11G)
	{
		offset = 2300;
	}
    else
	{
		offset = 4900;
	}
	
    curChanAsSpur = (freq - offset) * 10;
    /* 
     * Check if spur immunity should be performed for this channel
     * Should only be performed once per channel and then saved
     */
    finalSpur = 0;
    spurDetectWidth = SPUR_CHAN_WIDTH;
    if (pLibDev->turbo == TURBO_ENABLE) {
        spurDetectWidth *= 2;
    }

    /* Decide if any spur affects the current channel */
    for (i = 0; i < numSpurs; i++)
	{
		spurChan = (spurChannels[i] - offset) * 10;

		if ((curChanAsSpur - spurDetectWidth <= spurChan) &&
			(curChanAsSpur + spurDetectWidth >= spurChan))
		{
			finalSpur = spurChan;
			break;
		}
	}
    
    /* Write spur immunity data */
    if (finalSpur == 0) {
		/* Disable spur mitigation */
        writeField(devNum, "bb_enable_spur_filter", 0);
        writeField(devNum, "bb_enable_chan_mask", 0);
        writeField(devNum, "bb_enable_pilot_mask", 0);
        writeField(devNum, "bb_use_spur_filter_in_agc", 0);
        writeField(devNum, "bb_use_spur_filter_in_selfcor", 0);
        writeField(devNum, "bb_enable_spur_rssi", 0);
		return;
    } else {
        spurOffset = finalSpur - curChanAsSpur;
        /*
         * Spur calculations:
         * spurDeltaPhase is (spurOffsetIn100KHz / chipFrequencyIn100KHz) << 21
         * spurFreqSd is (spurOffsetIn100KHz / sampleFrequencyIn100KHz) << 11
         */
        switch (pLibDev->mode) {
            case MODE_11A:
				if(pLibDev->turbo != TURBO_ENABLE) {
					/* Chip Frequency & sampleFrequency are 40 MHz */
					spurDeltaPhase = (spurOffset << 17) / 25;
					spurFreqSd = spurDeltaPhase >> 10;
					binWidth = BIN_WIDTH_BASE_100HZ;
				}
				else {  
  		            /* Chip Frequency & sampleFrequency are 80 MHz */
		            spurDeltaPhase = (spurOffset << 16) / 25;
                    spurFreqSd = spurDeltaPhase >> 10;
                    binWidth = BIN_WIDTH_TURBO_100HZ;
				}
                break;
            case MODE_11G: 
		        if(pLibDev->turbo != TURBO_ENABLE) { 
                    /* Chip Frequency is 44MHz, sampleFrequency is 40 MHz */
                    spurFreqSd = (spurOffset << 8) / 55;
                    spurDeltaPhase = (spurOffset << 17) / 25;
                    binWidth = BIN_WIDTH_BASE_100HZ;
                }
                else {  
  		            /* Chip Frequency & sampleFrequency are 80 MHz */
		            spurDeltaPhase = (spurOffset << 16) / 25;
                    spurFreqSd = spurDeltaPhase >> 10;
                    binWidth = BIN_WIDTH_TURBO_100HZ;
				}
                break;
        }
        
	    spurDeltaPhase &= 0xfffff;
        spurFreqSd &= 0x3ff;

        /* Compute Pilot Mask */
        binOffsetNumT16 = ((spurOffset * 1000) << 4) / binWidth;
        /* The spur is on a bin if it's remainder at times 16 is 0 */
        if (binOffsetNumT16 & 0xF) { 
            numBinOffsets = 4; 
            pMagMap = magMapFor4;
        } else {
            numBinOffsets = 3;
            pMagMap = magMapFor3;
        }
        for (i = 0; i < numBinOffsets; i++) {
            /* Get Pilot Mask values */
            curBinOffset = (binOffsetNumT16 >> 4) + i + 25;
            if ((curBinOffset >= 0) && (curBinOffset <= 32)) {
                if (curBinOffset <= 25) {
                    pilotMask[0] |= 1 << curBinOffset;
                } else if (curBinOffset >= 27){
                    pilotMask[0] |= 1 << (curBinOffset - 1);
                }
            }
            else if ((curBinOffset >= 33) && (curBinOffset <= 52)) {
                pilotMask[1] |= 1 << (curBinOffset - 33);
            }
            
            /* Get viterbi values */
            if ((curBinOffset >= -1) && (curBinOffset <= 14)) {
                binMagMask[0] |= pMagMap[i] << (curBinOffset + 1) * 2;
            } else if ((curBinOffset >= 15) && (curBinOffset <= 30)) {
                binMagMask[1] |= pMagMap[i] << (curBinOffset - 15) * 2;
            } else if ((curBinOffset >= 31) && (curBinOffset <= 46)) {
                binMagMask[2] |= pMagMap[i] << (curBinOffset -31) * 2;
            } else if((curBinOffset >= 47) && (curBinOffset <= 53)) {
                binMagMask[3] |= pMagMap[i] << (curBinOffset -47) * 2;
            }
        }

#if 0
        printf("||=== Spur Mitigation Debug =====\n");
        printf("|| For channel %d MHz a spur was found at freq %d KHz\n", freq, spurChan + 23000);
        printf("|| Offset found to be %d (100 KHz)\n", spurOffset);
        printf("|| spurDeltaPhase %d, spurFreqSd %d\n", spurDeltaPhase, spurFreqSd);
        printf("|| Pilot Mask 0 0X%08X 1 0X%08X\n", pilotMask[0], pilotMask[1]);
        printf("|| Viterbi Mask 0 0X%08X 1 0X%08X 2 0X%08X 3 0X%08X\n", binMagMask[0], binMagMask[1], binMagMask[2], binMagMask[3]);
        printf("||\n");
        printf("||===");

        for (i = 0; i <= 25; i++) {
            if ((pilotMask[0] >> i) & 1) {
                printf(" X ");
            } else {
                printf("   ");
            }
        }
        printf("===||\n");

        printf("||");
        for (i = 0; i <= 31; i+=2) {
            printf(" %d ", (binMagMask[0] >> i) & 0x3);
        }
        for (i = 0; i <= 23; i+=2) {
            printf(" %d ", (binMagMask[1] >> i) & 0x3);
        }
        printf("||\n");

        printf("||-27-26-25-24-23-22-21-20-19-18-17-16-15-14-13-12-11-10-9 -8 -7 -6 -5 -4 -3 -2 -1  0 ||\n");
        printf("||------------------------------------------------------------------------------------||\n");

        printf("||===");
        for (i = 26; i <= 31; i++) {
            if ((pilotMask[0] >> i) & 1) {
                printf(" X ");
            } else {
                printf("   ");
            }
        }
        for (i = 0; i <= 19; i++) {
            if ((pilotMask[1] >> i) & 1) {
                printf(" X ");
            } else {
                printf("   ");
            }
        }
        printf("===||\n");

        printf("||");
        for (i = 22; i <= 31; i+=2) {
            printf(" %d ", (binMagMask[1] >> i) & 0x3);
        }
        for (i = 0; i <= 31; i+=2) {
            printf(" %d ", (binMagMask[2] >> i) & 0x3);
        }
        for (i = 0; i <= 13; i+=2) {
            printf(" %d ", (binMagMask[3] >> i) & 0x3);
        }
        printf("||\n");

        printf("|| 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27||\n");
        printf("||------------------------------------------------------------------------------------||\n");
#endif

		/* Write Spur Delta Phase, Spur Freq, and enable bits */
        writeField(devNum, "bb_enable_spur_filter", 1);
        writeField(devNum, "bb_enable_chan_mask", 1);
        writeField(devNum, "bb_enable_pilot_mask", 1);
        writeField(devNum, "bb_use_spur_filter_in_agc", 1);
//        writeField(devNum, "bb_use_spur_filter_in_selfcor", 1);
        writeField(devNum, "bb_spur_rssi_thresh", 40);
        writeField(devNum, "bb_enable_spur_rssi", 1);
        writeField(devNum, "bb_mask_rate_cntl", 255);
        writeField(devNum, "bb_mask_select", 240);
		writeField(devNum, "bb_spur_delta_phase", spurDeltaPhase);
		writeField(devNum, "bb_spur_freq_sd", spurFreqSd);
		/* Write pilot masks */
        writeField(devNum, "bb_pilot_mask_1", pilotMask[0]);
        writeField(devNum, "bb_pilot_mask_2", pilotMask[1]);
        writeField(devNum, "bb_chan_mask_1", pilotMask[0]);
        writeField(devNum, "bb_chan_mask_2", pilotMask[1]);
        /* Write magnitude masks */
        REGW(devNum, 0x9900, binMagMask[0]);
        REGW(devNum, 0x9904, binMagMask[1]);
        REGW(devNum, 0x9908, binMagMask[2]);
        REGW(devNum, 0x990c, (REGR(devNum, 0x990c) & 0xffffc000) | binMagMask[3]);
        REGW(devNum, 0x9988, binMagMask[0]);
        REGW(devNum, 0x998c, binMagMask[1]);
        REGW(devNum, 0x9990, binMagMask[2]);
        REGW(devNum, 0x9994, (REGR(devNum, 0x9994) & 0xffffc000) | binMagMask[3]);
    }
	return;
}

#if 0
void setSpurMitigation_Nala(A_UINT32 devNum, A_UINT32 freq)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
    A_UINT32      pilotMask[2] = {0, 0}, binMagMask[4] = {0, 0, 0 , 0};
	A_UINT32      spurChans11G[4] = {2420, 2440, 2464, 2480};//2464
	A_UINT32      spurChans11A[1] = {5800};
    A_UINT16      i;
    A_UINT16      finalSpur, curChanAsSpur, binWidth = 0, spurDetectWidth, spurChan;
    A_INT32       spurDeltaPhase = 0, spurFreqSd = 0, spurOffset, binOffsetNumT16, curBinOffset;
    A_INT16       numBinOffsets;
    A_UINT16      magMapFor4[4] = {1, 2, 2, 1};
    A_UINT16      magMapFor3[3] = {1, 2, 1};
    A_UINT16      *pMagMap;
	A_BOOL		  is2GHz;
	A_UINT32	  numSpurs, offset;
    
    if(pLibDev->mode == MODE_11G)
	{
		is2GHz = TRUE;
		offset = 2300;
		numSpurs = 4;
		
	}
    else
	{
		is2GHz = FALSE;
		offset = 4900;
		numSpurs = 1;
	}
	
    curChanAsSpur = (freq - offset) * 10;
    /* 
     * Check if spur immunity should be performed for this channel
     * Should only be performed once per channel and then saved
     */
    finalSpur = 0;
    spurDetectWidth = SPUR_CHAN_WIDTH;
    if (pLibDev->turbo == TURBO_ENABLE) {
        spurDetectWidth *= 2;
    }

    /* Decide if any spur affects the current channel */
    for (i = 0; i < numSpurs; i++)
	{
		if(is2GHz)
		{
			spurChan = (spurChans11G[i] - offset) * 10;
		}
		else
		{
			spurChan = (spurChans11A[i] - offset) * 10;
		}

		if ((curChanAsSpur - spurDetectWidth <= spurChan) &&
			(curChanAsSpur + spurDetectWidth >= spurChan))
		{
			finalSpur = spurChan;
			break;
		}
	}
    
    /* Write spur immunity data */
    if (finalSpur == 0) {
		/* Disable spur mitigation */
        writeField(devNum, "bb_enable_spur_filter", 0);
        writeField(devNum, "bb_enable_chan_mask", 0);
        writeField(devNum, "bb_enable_pilot_mask", 0);
        writeField(devNum, "bb_use_spur_filter_in_agc", 0);
        writeField(devNum, "bb_use_spur_filter_in_selfcor", 0);
        writeField(devNum, "bb_enable_spur_rssi", 0);
		return;
    } else {
        spurOffset = finalSpur - curChanAsSpur;
        /*
         * Spur calculations:
         * spurDeltaPhase is (spurOffsetIn100KHz / chipFrequencyIn100KHz) << 21
         * spurFreqSd is (spurOffsetIn100KHz / sampleFrequencyIn100KHz) << 11
         */
        switch (pLibDev->mode) {
            case MODE_11A:
				if(pLibDev->turbo != TURBO_ENABLE) {
					/* Chip Frequency & sampleFrequency are 40 MHz */
					spurDeltaPhase = (spurOffset << 17) / 25;
					spurFreqSd = spurDeltaPhase >> 10;
					binWidth = BIN_WIDTH_BASE_100HZ;
				}
				else {  
  		            /* Chip Frequency & sampleFrequency are 80 MHz */
		            spurDeltaPhase = (spurOffset << 16) / 25;
                    spurFreqSd = spurDeltaPhase >> 10;
                    binWidth = BIN_WIDTH_TURBO_100HZ;
				}
                break;
            case MODE_11G: 
		        if(pLibDev->turbo != TURBO_ENABLE) { 
                    /* Chip Frequency is 44MHz, sampleFrequency is 40 MHz */
                    spurFreqSd = (spurOffset << 8) / 55;
                    spurDeltaPhase = (spurOffset << 17) / 25;
                    binWidth = BIN_WIDTH_BASE_100HZ;
                }
                else {  
  		            /* Chip Frequency & sampleFrequency are 80 MHz */
		            spurDeltaPhase = (spurOffset << 16) / 25;
                    spurFreqSd = spurDeltaPhase >> 10;
                    binWidth = BIN_WIDTH_TURBO_100HZ;
				}
                break;
        }
        
	    spurDeltaPhase &= 0xfffff;
        spurFreqSd &= 0x3ff;

        /* Compute Pilot Mask */
        binOffsetNumT16 = ((spurOffset * 1000) << 4) / binWidth;
        /* The spur is on a bin if it's remainder at times 16 is 0 */
        if (binOffsetNumT16 & 0xF) { 
            numBinOffsets = 4; 
            pMagMap = magMapFor4;
        } else {
            numBinOffsets = 3;
            pMagMap = magMapFor3;
        }
        for (i = 0; i < numBinOffsets; i++) {
            /* Get Pilot Mask values */
            curBinOffset = (binOffsetNumT16 >> 4) + i + 25;
            if ((curBinOffset >= 0) && (curBinOffset <= 32)) {
                if (curBinOffset <= 25) {
                    pilotMask[0] |= 1 << curBinOffset;
                } else if (curBinOffset >= 27){
                    pilotMask[0] |= 1 << (curBinOffset - 1);
                }
            }
            else if ((curBinOffset >= 33) && (curBinOffset <= 52)) {
                pilotMask[1] |= 1 << (curBinOffset - 33);
            }
            
            /* Get viterbi values */
            if ((curBinOffset >= -1) && (curBinOffset <= 14)) {
                binMagMask[0] |= pMagMap[i] << (curBinOffset + 1) * 2;
            } else if ((curBinOffset >= 15) && (curBinOffset <= 30)) {
                binMagMask[1] |= pMagMap[i] << (curBinOffset - 15) * 2;
            } else if ((curBinOffset >= 31) && (curBinOffset <= 46)) {
                binMagMask[2] |= pMagMap[i] << (curBinOffset -31) * 2;
            } else if((curBinOffset >= 47) && (curBinOffset <= 53)) {
                binMagMask[3] |= pMagMap[i] << (curBinOffset -47) * 2;
            }
        }

#if 0
        printf("||=== Spur Mitigation Debug =====\n");
        printf("|| For channel %d MHz a spur was found at freq %d KHz\n", freq, spurChan + 23000);
        printf("|| Offset found to be %d (100 KHz)\n", spurOffset);
        printf("|| spurDeltaPhase %d, spurFreqSd %d\n", spurDeltaPhase, spurFreqSd);
        printf("|| Pilot Mask 0 0X%08X 1 0X%08X\n", pilotMask[0], pilotMask[1]);
        printf("|| Viterbi Mask 0 0X%08X 1 0X%08X 2 0X%08X 3 0X%08X\n", binMagMask[0], binMagMask[1], binMagMask[2], binMagMask[3]);
        printf("||\n");
        printf("||===");

        for (i = 0; i <= 25; i++) {
            if ((pilotMask[0] >> i) & 1) {
                printf(" X ");
            } else {
                printf("   ");
            }
        }
        printf("===||\n");

        printf("||");
        for (i = 0; i <= 31; i+=2) {
            printf(" %d ", (binMagMask[0] >> i) & 0x3);
        }
        for (i = 0; i <= 23; i+=2) {
            printf(" %d ", (binMagMask[1] >> i) & 0x3);
        }
        printf("||\n");

        printf("||-27-26-25-24-23-22-21-20-19-18-17-16-15-14-13-12-11-10-9 -8 -7 -6 -5 -4 -3 -2 -1  0 ||\n");
        printf("||------------------------------------------------------------------------------------||\n");

        printf("||===");
        for (i = 26; i <= 31; i++) {
            if ((pilotMask[0] >> i) & 1) {
                printf(" X ");
            } else {
                printf("   ");
            }
        }
        for (i = 0; i <= 19; i++) {
            if ((pilotMask[1] >> i) & 1) {
                printf(" X ");
            } else {
                printf("   ");
            }
        }
        printf("===||\n");

        printf("||");
        for (i = 22; i <= 31; i+=2) {
            printf(" %d ", (binMagMask[1] >> i) & 0x3);
        }
        for (i = 0; i <= 31; i+=2) {
            printf(" %d ", (binMagMask[2] >> i) & 0x3);
        }
        for (i = 0; i <= 13; i+=2) {
            printf(" %d ", (binMagMask[3] >> i) & 0x3);
        }
        printf("||\n");

        printf("|| 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27||\n");
        printf("||------------------------------------------------------------------------------------||\n");
#endif

		/* Write Spur Delta Phase, Spur Freq, and enable bits */
        writeField(devNum, "bb_enable_spur_filter", 1);
        writeField(devNum, "bb_enable_chan_mask", 1);
        writeField(devNum, "bb_enable_pilot_mask", 1);
        writeField(devNum, "bb_use_spur_filter_in_agc", 1);
//        writeField(devNum, "bb_use_spur_filter_in_selfcor", 1);
        writeField(devNum, "bb_spur_rssi_thresh", 40);
        writeField(devNum, "bb_enable_spur_rssi", 1);
        writeField(devNum, "bb_mask_rate_cntl", 255);
        writeField(devNum, "bb_mask_select", 240);
		writeField(devNum, "bb_spur_delta_phase", spurDeltaPhase);
		writeField(devNum, "bb_spur_freq_sd", spurFreqSd);
		/* Write pilot masks */
        writeField(devNum, "bb_pilot_mask_1", pilotMask[0]);
        writeField(devNum, "bb_pilot_mask_2", pilotMask[1]);
        writeField(devNum, "bb_chan_mask_1", pilotMask[0]);
        writeField(devNum, "bb_chan_mask_2", pilotMask[1]);
        /* Write magnitude masks */
        REGW(devNum, 0x9900, binMagMask[0]);
        REGW(devNum, 0x9904, binMagMask[1]);
        REGW(devNum, 0x9908, binMagMask[2]);
        REGW(devNum, 0x990c, (REGR(devNum, 0x990c) & 0xffffc000) | binMagMask[3]);
        REGW(devNum, 0x9988, binMagMask[0]);
        REGW(devNum, 0x998c, binMagMask[1]);
        REGW(devNum, 0x9990, binMagMask[2]);
        REGW(devNum, 0x9994, (REGR(devNum, 0x9994) & 0xffffc000) | binMagMask[3]);
    }
	return;
}
#endif


/**************************************************************************
* rereadProm - reset the EEPROM information
*
*/
MANLIB_API void rereadProm
(
 A_UINT32 devNum
)
{
	A_UINT16 i;
	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:rereadProm\n", devNum);
		return;
	}
    gLibInfo.pLibDevArray[devNum]->eepData.eepromChecked = FALSE;
	for (i = 0; i < 4; i++) {
		gLibInfo.pLibDevArray[devNum]->eepromHeaderApplied[i] = FALSE;
	}

	//free up the 16K eeprom struct if needed
	freeEepStructs(devNum);
//	printf("After rereadProm \n");
}

/**************************************************************************
* eepromRead - call correct eepromRead Function
*
* RETURNS: 16 bit value from given offset (in a 32-bit value)
*/
MANLIB_API A_UINT32 eepromRead
(
 A_UINT32 devNum, 
 A_UINT32 eepromOffset
)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
	A_UINT32 i = pLibDev->ar5kInitIndex;
//	printf("\n In eepromRead = %d \n",eepromOffset);
	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:eepromRead\n", devNum);
		return 0xdeadbeef;
	}

	if (ar5kInitData[i].pMacAPI->eepromRead == NULL)
	{
		printf(" In ar5kInitData\n");
		return(0xdeadbeef);
	}

    if (pLibDev->devMap.remoteLib)
		{
			//	printf("\n In IF eepromRead = %d \n",eepromOffset);
				return (pLibDev->devMap.r_eepromRead(devNum, eepromOffset));
		 
		}
    else
		{
			//printf("\n In ELSE eepromRead = %d \n",eepromOffset);
				return (ar5kInitData[i].pMacAPI->eepromRead(devNum, eepromOffset));
		 
		}
}

/**************************************************************************
* eepromWrite - Call correct eepromWrite function
*
*/
MANLIB_API void eepromWrite
(
 A_UINT32 devNum, 
 A_UINT32 eepromOffset, 
 A_UINT32 eepromValue
)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
	A_UINT32 i = pLibDev->ar5kInitIndex;

	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:eepromWrite\n", devNum);
		return;
	}

	if (ar5kInitData[i].pMacAPI->eepromWrite == NULL)
	{
		return;
	}
    if (pLibDev->devMap.remoteLib)
	   pLibDev->devMap.r_eepromWrite(devNum, eepromOffset, eepromValue);
    else
	    ar5kInitData[i].pMacAPI->eepromWrite(devNum, eepromOffset, eepromValue);
	return;
}




/**************************************************************************
* setAntenna - Change antenna to the given antenna A or B
*
*/
MANLIB_API void setAntenna
(
 A_UINT32 devNum, 
 A_UINT32 antenna
)
{    
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
	A_UINT32	antMode = 0;

	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:setAntenna\n", devNum);
		return;
	}
	if (!ar5kInitData[pLibDev->ar5kInitIndex].pMacAPI->setupAntenna(devNum, antenna, &antMode))
	{
		return;
	}

}

/**************************************************************************
* setTransmitPower - Adjusts output power to a percentage of maximum.
*   resetDevice or changeChannel must be called before the setting takes affect.
*   powerScale is ignored if the EEPROM is not at least rev 1
*
*/
MANLIB_API void setPowerScale
(
 A_UINT32 devNum,
 A_UINT32 powerScale
)
{
	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:setTransmitPower\n", devNum);
		return;
	}
    if((powerScale < TP_SCALE_LOWEST) || (powerScale > TP_SCALE_HIGHEST)) {
		mError(devNum, EINVAL, "Device Number %d:setPowerScale: Invalid powerScale option: %d\n", devNum, powerScale);
		return;
    }
    gLibInfo.pLibDevArray[devNum]->txPowerData.tpScale = (A_UINT16)powerScale;
    return;
}

/**************************************************************************
* setTransmitPower - Calls the correct setTransmitPower function based on 
*		deviceID
*   
*
*/
MANLIB_API void setTransmitPower
(
 A_UINT32 devNum, 
 A_UCHAR txPowerArray[17]
)
{

	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:setTransmitPower\n", devNum);
		return;
	}
	if (gLibInfo.pLibDevArray[devNum]->devState < RESET_STATE) {
		mError(devNum, EILSEQ, "Device Number %d:setTransmitPower: Device should be out of Reset before changing transmit power\n", devNum);
		return;
	}

	// Disable AGC to A2 traffic during A2 writes
	REGW(devNum, 0x9808, REGR(devNum, 0x9808) | 0x08000000);

	initializeTransmitPower(devNum, 0, 1, txPowerArray);
	mSleep(1);

	// Re-enable AGC to A2 traffic
	REGW(devNum, 0x9808, REGR(devNum, 0x9808) & (~0x08000000));
	mSleep(1);
}


/**************************************************************************
* setSingleTransmitPower - Set one pcdac value to be copied to all rates
*   with zero gain_delta.
*   Ignores power scaling but will be reset by resetDevice or changeChannel.
*
*/
MANLIB_API void setSingleTransmitPower
(
 A_UINT32 devNum, 
 A_UCHAR pcdac
)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];

	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:setSingleTransmitPower\n", devNum);
		return;
	}
	if (gLibInfo.pLibDevArray[devNum]->devState < RESET_STATE) {
		mError(devNum, EILSEQ, "Device Number %d:setSingleTransmitPower: Device should be out of Reset before changing transmit power\n", devNum);
		return;
	}

    ar5kInitData[pLibDev->ar5kInitIndex].pRfAPI->setSinglePower(devNum, pcdac);

}


/**************************************************************************
* devSleep - Put the device to sleep for sleep power measurements
*
*/
MANLIB_API void devSleep
(
 A_UINT32 devNum
)
{

	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:devSleep\n", devNum);
		return;
	}
	if (gLibInfo.pLibDevArray[devNum]->devState < RESET_STATE) {
		mError(devNum, EILSEQ, "Device Number %d:devSleep: Device should be out of Reset before entering sleep\n", devNum);
		return;
	}

#if defined(MDK_AP) 
	mError(devNum, EINVAL, "Device Number %d:devSleep: AP cannot go to sleep \n", devNum);
#else
	REGW(devNum, F2_SFR, F2_SFR_SLEEP);
#endif
}

MANLIB_API A_UINT32 checkProm
(
 A_UINT32 devNum,
 A_UINT32 enablePrint
)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];

	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:printProm\n", devNum);
		return 1;
	}

	//	printf("After CheckDenNUm in checkprom \n");
	if (gLibInfo.pLibDevArray[devNum]->devState < RESET_STATE) {
		mError(devNum, EILSEQ, "Device Number %d:printProm: Device should be out of Reset before checking the EEPROM\n", devNum);
		return 1;
	}

	//check that the info fromt the eeprom is valid
	if(!pLibDev->eepData.infoValid) {
		mError(devNum,EILSEQ, "Device Number %d:checkProm: eeprom info is not valid\n", devNum);
					return 1;
	}
    if(((pLibDev->eepData.version >> 12) & 0xF) == 1) {
#ifndef __ATH_DJGPPDOS__
		check5210Prom(devNum, enablePrint);
	//	printf("After Check5210Prom \n");
#endif
	}
    else if(((pLibDev->eepData.version >> 12) & 0xF) >= 3) {
		
	}
	else{	
		mError(devNum, EIO, "Device Number %d:checkProm: Wrong PROM Version to print\n", devNum);
        return 1;
    }
	//	printf("checkProm\n");
	return 0;
}


/**************************************************************************
* setupEEPromMap - Read the EEPROM and setup the structure
*
* Returns: TRUE if the EEPROM is calibrated, else FALSE
*/
A_BOOL setupEEPromMap
(
 A_UINT32 devNum
)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
    A_UINT32     pciReg;
    // Only check the EEPROM once
    if(pLibDev->eepData.eepromChecked == TRUE) {
        return pLibDev->eepData.infoValid;
    }
    else  // This code will only be run the first time this function is called
    {
		// Setup the PCI Config space for correct card access on the first reset
		// Setup memory window, bus mastering, & SERR
		pciReg = pLibDev->devMap.OScfgRead(devNum, F2_PCI_CMD);
		pciReg |= (MEM_ACCESS_ENABLE | MASTER_ENABLE | SYSTEMERROR_ENABLE); 
		pciReg &= ~MEM_WRITE_INVALIDATE; // Disable write & invalidate for our device
		pLibDev->devMap.OScfgWrite(devNum, F2_PCI_CMD, pciReg);

		pciReg = pLibDev->devMap.OScfgRead(devNum, F2_PCI_CACHELINESIZE);
		//changed this to not write the cacheline size, only want to write the latency timer
		pciReg = (pciReg & 0xffff00ff) | (0x40 << 8);
		pLibDev->devMap.OScfgWrite(devNum, F2_PCI_CACHELINESIZE, pciReg);

        pLibDev->eepData.version = (A_UINT16) eepromRead(devNum, (ATHEROS_EEPROM_OFFSET + 1));
        pLibDev->eepData.protect = (A_UINT16) eepromRead(devNum, (EEPROM_PROTECT_OFFSET));
        if(((pLibDev->eepData.version >> 12) & 0xF) < 2) {

#ifndef __ATH_DJGPPDOS__
			read5210eepData(devNum);			
#endif
		}

        else if(((pLibDev->eepData.version >> 12) & 0xF) == 2) {
			mError(devNum, EIO, "Device Number %d:Version 2 EEPROM not supported \n", devNum);
		} else if ((((pLibDev->eepData.version >> 12) & 0xF) == 3) || (((pLibDev->eepData.version >> 12) & 0xF) >= 4)) {
			if(!readEEPData_16K(devNum)) {
				mError(devNum, EIO, "Device Number %d:Unable to read 16K eeprom info\n", devNum);
				pLibDev->eepData.eepromChecked = TRUE;
				pLibDev->eepData.infoValid = FALSE;
				return FALSE;
			}
//			printf("Middle setupEEPromMap\n");
			if(((pLibDev->eepData.version >> 12) & 0xF) >= 4) {
				//read the EAR
				if(!readEar(devNum, pLibDev->p16kEepHeader->earStartLocation)) {
					mError(devNum, EIO, "Unable to read EAR information from EEPROM\n");
				//	printf(" In IF read Ear\n");
					pLibDev->eepData.eepromChecked = TRUE;
					pLibDev->eepData.infoValid = FALSE;
					return FALSE;
				}
			}
			pLibDev->eepromHeaderChecked = TRUE;
		} 

		else {
			mError(devNum, EIO, "Device Number %d:setupEEPromMap: Invalid version found: 0x%04X\n", devNum, pLibDev->eepData.version);
			pLibDev->eepData.eepromChecked = TRUE;
			pLibDev->eepData.infoValid = FALSE;
		//	printf(" In else read Ear\n");
			return FALSE;
		}		
        pLibDev->eepData.infoValid = TRUE;
        pLibDev->eepData.eepromChecked = TRUE;
			//	printf(" In read Ear\n");
        return TRUE;
    }
	//	printf("End setupEEPromMap\n");
}

#ifdef HEADER_LOAD_SCHEME
A_BOOL setupEEPromHeaderMap
(
 A_UINT32 devNum
)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];

    // Only check the EEPROM once
    if(pLibDev->eepromHeaderChecked == TRUE) {
        return TRUE;
    }

    allocateEepStructs(devNum);
	pLibDev->eepData.version = (A_UINT16) eepromRead(devNum, (ATHEROS_EEPROM_OFFSET + 1));
	if (((pLibDev->eepData.version >> 12) & 0xF) == 3) {
		readHeaderInfo(devNum, pLibDev->p16kEepHeader);
		pLibDev->eepromHeaderChecked = TRUE;
		return TRUE;
	}
	return FALSE;
}
#endif //HEADER_LOAD_SCHEME

void initializeTransmitPower
(
	A_UINT32 devNum,
	A_UINT32 freq,
	A_INT32  override,
	A_UCHAR  *pwrSettings
)
{
    LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
	A_UINT32 i = pLibDev->ar5kInitIndex;


	if (ar5kInitData[i].pRfAPI->setPower == NULL)
	{
		return;
	}

	ar5kInitData[i].pRfAPI->setPower(devNum, freq, override, pwrSettings);
	return;
}




/**************************************************************************
* checkDevNum - Makes sure the device is initialized before using
*
* RETURNS: FALSE if device not yet initialized - else TRUE
*/
A_BOOL checkDevNum
(
 A_UINT32 devNum
) 
{
	if(gLibInfo.pLibDevArray[devNum] == NULL) {
#ifndef NO_LIB_PRINT
		printf("Device Number %d:DevNum not initialized for function: ", devNum);
#endif
		return FALSE;
	}
	return TRUE;
}

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

/**************************************************************************
* mError - output error messages
*
* This routine is the equivalent of printf.  It is used such that logging
* capabilities can be added.
*
* RETURNS: same as printf.  Number of characters printed
*/
int mError
(
	A_UINT32 devNum,
	A_UINT32 error,
    const char * format,
    ...
)
{

    LIB_DEV_INFO *pLibDev;
    va_list argList;
    int retval = 0;
#ifndef NO_LIB_PRINT
    char    buffer[256];
#endif

    /*if have logging turned on then can also write to a file if needed */
    /* get the arguement list */
    va_start(argList, format);

    /* using vprintf to perform the printing it is the same is printf, only
     * it takes a va_list or arguments
     */
#ifndef NO_LIB_PRINT
    printf("%s : ", strerror(error));
    retval = vprintf(format, argList);
    fflush(stdout);

#ifndef MDK_AP
#ifndef __ATH_DJGPPDOS__
    if (logging) {
		vsprintf(buffer, format, argList);
        fputs(buffer, logFileHandle);
		fflush(logFileHandle);
    }
#endif //__ATH_DJGPPDOS__
#endif //MDK_AP
#endif //NO_LIB_PRINT


	if(devNum != 0xdead) {
	    //take a copy of the error buffer
		pLibDev = gLibInfo.pLibDevArray[devNum];
		vsprintf(pLibDev->mdkErrStr, format, argList);
		pLibDev->mdkErrno = error;
	}
	else {
		//this is a special case where initializeDevice failed,
		//before a devNum could be assigned, store these in static variables
		vsprintf(tempMDKErrStr, format, argList);
		tempMDKErrno = error;

	}
    va_end(argList);    /* cleanup arg list */

    return(retval);
}

/**************************************************************************
* getMdkErrStr - Get last error string
*
* This routine should only be called if mdkErrno is set.  It will copy
* the last error string into the callers buffer.  Callers buffer should
* be at least SIZE_ERROR_BUFFER bytes
*
* RETURNS: same as printf.  Number of characters printed
*/
MANLIB_API void getMdkErrStr
(	
  	A_UINT32 devNum,
    A_CHAR *pBuffer		//pointer to called allocated memory 
)
{
    LIB_DEV_INFO *pLibDev;
	char *pErrString;

	if(devNum == 0xdead) {
		pErrString = tempMDKErrStr;
	}
	else {
		pLibDev = gLibInfo.pLibDevArray[devNum];
		pErrString = pLibDev->mdkErrStr;
	}
	strncpy(pBuffer, pErrString, SIZE_ERROR_BUFFER-1);
	return;
}

/**************************************************************************
* getLastErrorNo - get the last error number
*
* Returns : mdkErrno
*/
MANLIB_API A_INT32 getMdkErrNo
(
	A_UINT32 devNum
)
{
    LIB_DEV_INFO *pLibDev;
	A_INT32 returnValue;

	if(devNum == 0xdead) {
		returnValue = tempMDKErrno;
		tempMDKErrno = 0;
	}
	else {
		pLibDev = gLibInfo.pLibDevArray[devNum];
		returnValue = pLibDev->mdkErrno;
		pLibDev->mdkErrno = 0;
	}
	return returnValue;
}

#ifndef MDK_AP
/**************************************************************************
* enableLogging - enable logging of any lib messages to file 
*
*  
*
*/
MANLIB_API void enableLogging
(
 A_CHAR *pFilename
)
{
	logging = 1;

	logFileHandle = fopen(pFilename, "a+");

    if (logFileHandle == NULL) {
        printf( "Unable to open file for logging within library%s\n", pFilename);
		logging = 0;
    }
}

/**************************************************************************
* disableLogging - turn off logging flag
*
*
*/
MANLIB_API void disableLogging(void)
{
	logging = 0;
	fclose(logFileHandle);
	logFileHandle = NULL;
}
#endif

MANLIB_API void devlibCleanup()
{
	A_UINT32 i;

    //printf("SNOOP:::: devlibCleanup called \n");
	// cleanup all the devInfo structures
    	for ( i = 0; i < LIB_MAX_DEV; i++ ) {
        	if ( gLibInfo.pLibDevArray[i] ) {
				closeDevice(i);
	    		gLibInfo.pLibDevArray[i] = NULL;        
        	}
	}
   
}

MANLIB_API A_BOOL isPredator(A_UINT32 swDeviceID)
{
	if (swDeviceID == SW_DEVICE_ID_PREDATOR) {
		return TRUE;
	} else {
		return FALSE;
	}
}

MANLIB_API A_BOOL isGriffin(A_UINT32 swDeviceID)
{
	if ((swDeviceID == 0x0018) || 
		(swDeviceID == SW_DEVICE_ID_AP51) ||
		(swDeviceID == DEVICE_ID_SPIDER1_0) || 
		(swDeviceID == SW_DEVICE_ID_NALA)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

MANLIB_API A_BOOL isSpider(A_UINT32 swDeviceID)
{
	if (swDeviceID == DEVICE_ID_SPIDER1_0) {
printf("Snoop: Is spider returned true\n");
		return TRUE;
	} else {
		return FALSE;
	}
}

MANLIB_API A_BOOL isCobra(A_UINT32 swDeviceID)
{
	if (swDeviceID == SW_DEVICE_ID_AP51 || swDeviceID == DEVICE_ID_SPIDER1_0) {
		return TRUE;
	} else {
		return FALSE;
	}
}
MANLIB_API A_BOOL isGriffin_1_0(A_UINT32 macRev)
{
	if (macRev == 0x74) {
		return TRUE;
	} else {
		return FALSE;
	}
}

MANLIB_API A_BOOL isGriffin_1_1(A_UINT32 macRev)
{
	if (macRev == 0x75) {
		return TRUE;
	} else {
		return FALSE;
	}
}

MANLIB_API A_BOOL isGriffin_2_0(A_UINT32 macRev)
{
	if (macRev == 0x76) {
		return TRUE;
	} else {
		return FALSE;
	}
}

MANLIB_API A_BOOL isGriffin_2_1(A_UINT32 macRev)
{
	if (macRev == 0x79 || macRev == 0xb0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

MANLIB_API A_BOOL isGriffin_lite(A_UINT32 macRev)
{
	if (macRev == 0x78) {
		return TRUE;
	} else {
		return FALSE;
	}
}

MANLIB_API A_BOOL isEagle_lite(A_UINT32 macRev)
{
	if (macRev == 0xa4) {
		return TRUE;
	} else {
		return FALSE;
	}
}

MANLIB_API A_BOOL needsUartPciCfg(A_UINT32 swDeviceID)
{
	if (isGriffin(swDeviceID) && !isCobra(swDeviceID)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void memDisplay(A_UINT32 devNum, A_UINT32 address, A_UINT32 nWords) {
  A_UINT32 iIndex, memValue;
	LIB_DEV_INFO *pLibDev;

	pLibDev = gLibInfo.pLibDevArray[devNum];

  for(iIndex=0; iIndex<nWords; iIndex++) {
    pLibDev->devMap.OSmemRead(devNum, address+(iIndex*4), (A_UCHAR *)&memValue, sizeof(memValue));
	printf("Word %d (addr %x)=%x\n", iIndex, (address+ (iIndex*4)), memValue);
  } 

}

MANLIB_API A_BOOL isEagle(A_UINT32 swDeviceID)
{
	if (swDeviceID == SW_DEVICE_ID_EAGLE) {
		return TRUE;
	} else if (isCondor(swDeviceID)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

MANLIB_API A_BOOL isEagle_1_0(A_UINT32 macRev)
{
	if (macRev == 0xa1) {
		return TRUE;
	} else {
		return FALSE;
	}
}


MANLIB_API A_BOOL isCondor(A_UINT32 swDeviceID)
{
    if (swDeviceID == SW_DEVICE_ID_CONDOR) {
        return TRUE;
	}
	if (swDeviceID == SW_DEVICE_ID_SWAN) {
		return TRUE;
	}		
	return FALSE;
}

MANLIB_API A_BOOL isSwan(A_UINT32 swDeviceID)
{
	if (swDeviceID == SW_DEVICE_ID_SWAN) {
		return TRUE;
	} else {
		return FALSE;
	}
}

