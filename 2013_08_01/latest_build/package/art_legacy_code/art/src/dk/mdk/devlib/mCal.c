/* mCal.c - contains functions related to radio calibration */

/* Copyright (c) 2001 Atheros Communications, Inc., All Rights Reserved */

/* 
Revsision history
--------------------
1.0       Created.
*/

#ifdef __ATH_DJGPPDOS__
#define __int64	long long
typedef unsigned long DWORD;
#endif	// #ifdef __ATH_DJGPPDOS__

#include <errno.h>
//#include <conio.h>
#include "wlantype.h"
#include "mConfig.h"
#include "manlib.h"

#if defined(SPIRIT_AP) || defined(FREEDOM_AP)
#include "misclib.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//++JC++
static AR2413_TXGAIN_TBL griffin_tx_gain_tbl[] =
{
#include  "AR2413_tx_gain.tbl"
} ;

//#ifndef __ATH_DJGPPDOS__
static AR2413_TXGAIN_TBL spider_tx_gain_tbl[] =
{
#include  "spider2_0.tbl"
} ;
//#endif

static AR2413_TXGAIN_TBL eagle_tx_gain_tbl_2[] =
{
#include  "AR5413_tx_gain_2.tbl"
//#include  "condor_test.tbl"
} ;
static AR2413_TXGAIN_TBL eagle_tx_gain_tbl_5[] =
{
#include  "AR5413_tx_gain_5.tbl"
} ;
//#define  AR2413_TX_GAIN_TBL_SIZE  26

static AR2413_TXGAIN_TBL nala_tx_gain_tbl[] = 
{
#include "nala.tbl"
};

MANLIB_API void ForceSinglePCDACTableGriffin(A_UINT32 devNum, A_UINT16 pcdac, A_UINT16 offset)
{
	A_UINT16 i;
	A_UINT32 dac_gain = 0;
	AR2413_TXGAIN_TBL *pGainTbl = NULL;
	A_UINT16 sizeTable = 0;
	
   	LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];

	if ( isSpider(pLibDev->swDevID) || isSwan(pLibDev->swDevID) ) {
//#ifndef __ATH_DJGPPDOS__
		pGainTbl = spider_tx_gain_tbl;
		sizeTable = sizeof(spider_tx_gain_tbl)/sizeof(AR2413_TXGAIN_TBL);
//#endif
	}
	else if(isGriffin(pLibDev->swDevID)) {
		if (isNala(pLibDev->swDevID))
		{
			pGainTbl = nala_tx_gain_tbl;
			sizeTable = sizeof(nala_tx_gain_tbl) / sizeof(AR2413_TXGAIN_TBL);
		}
		else
		{
			pGainTbl = griffin_tx_gain_tbl;
			sizeTable = sizeof(griffin_tx_gain_tbl)/sizeof(AR2413_TXGAIN_TBL);
		}
	}
	else if(isEagle(pLibDev->swDevID)) {
		if(pLibDev->mode == MODE_11A) {
			pGainTbl = eagle_tx_gain_tbl_5;
			sizeTable = sizeof(eagle_tx_gain_tbl_5)/sizeof(AR2413_TXGAIN_TBL);
			if(isCondor(pLibDev->swDevID)) {
                offset -= 10;
            } else {
			    offset -= 10;
            }
		} else {
			pGainTbl = eagle_tx_gain_tbl_2;
			if(isCondor(pLibDev->swDevID)) {
				offset += 15;
            } else {
                offset -= 10;
            }
			sizeTable = sizeof(eagle_tx_gain_tbl_2)/sizeof(AR2413_TXGAIN_TBL);
		}
	}

	if (pcdac & RAW_PCDAC_MASK) offset = 0;

	if(pGainTbl == NULL) {
		mError(devNum, EIO, "Error: unable to initialize gainTable in ForceSinglePCDACTableGriffin\n");
	}
	i = 0;
	if(offset != GAIN_OVERRIDE && !(pcdac & RAW_PCDAC_MASK)) {
		if (pLibDev->mode == 1) {
			offset = (A_UINT16)(offset + 10);	// Up the offset for 11b mode
		}
		pcdac = (A_UINT16)(pcdac + offset);		// Offset pcdac to get in a reasonable range
	}

	if(pcdac & RAW_PCDAC_MASK)
		pcdac &= ~RAW_PCDAC_MASK;

	if(pcdac > pGainTbl[sizeTable - 1].desired_gain) {
		i = sizeTable - 1;
	}
	else {
		while ((pcdac > pGainTbl[i].desired_gain) &&
				(i < sizeTable) ) {i++;}  // Find entry closest
	}
	if (pGainTbl[i].desired_gain > pcdac) {
		dac_gain = pGainTbl[i].desired_gain - pcdac;
	}	
	writeField(devNum, "bb_force_dac_gain", 1);
	writeField(devNum, "bb_forced_dac_gain",  dac_gain);
	writeField(devNum, "bb_force_tx_gain", 1);
	writeField(devNum, "bb_forced_txgainbb1", pGainTbl[i].bb1_gain);
	writeField(devNum, "bb_forced_txgainbb2", pGainTbl[i].bb2_gain);
	writeField(devNum, "bb_forced_txgainif", pGainTbl[i].if_gain);
	writeField(devNum, "bb_forced_txgainrf", pGainTbl[i].rf_gain);
//printf("dacg = %d, bb1 = %d, bb2 = %d, if = %d, rf = %d\n", dac_gain,
//		pGainTbl[i].bb1_gain, pGainTbl[i].bb2_gain, pGainTbl[i].if_gain,
//		pGainTbl[i].rf_gain);
//getch();

	return;
}
//++JC++

MANLIB_API void ForceSinglePCDACTable(A_UINT32 devNum, A_UINT16 pcdac)
{
	A_UINT16 temp16, i;
	A_UINT32 temp32;
	A_UINT32 regoffset ;

//++JC++
	LIB_DEV_INFO *pLibDev = gLibInfo.pLibDevArray[devNum];
	if(isGriffin(pLibDev->swDevID) || isEagle(pLibDev->swDevID))  {    // For Griffin
		ForceSinglePCDACTableGriffin(devNum, pcdac, 30);  // By default, offset of 30
		return;
	}
//++JC++
	temp16 = (A_UINT16) (0x0000 | (pcdac << 8) | 0x00ff);
	temp32 = (temp16 << 16) | temp16 ;

	regoffset = 0x9800 + (608 << 2) ;
	for (i=0; i<32; i++)
	{
		REGW(devNum, regoffset, temp32);
		regoffset += 4;
	}

	return;
}

/* Return the mac address of the mac specified 
   wmac = 0 (for ethernet) wmac = 1 (for wireless)
   instance number (used only for ethernet) to get the mac address, if more than one 
   ethernet mac is present.
   macAddr - buffer to return the mac address
 */
MANLIB_API void getMacAddr(A_UINT32 devNum, A_UINT16 wmac, A_UINT16 instNo, A_UINT8 *macAddr)
{
	A_UINT32 readVal;

	//verify some of the arguments
	if (checkDevNum(devNum) == FALSE) {
		mError(devNum, EINVAL, "Device Number %d:getMacAddr \n", devNum);
		return;
	}

	if (wmac > 1) {
		mError(devNum, EINVAL, "Device Number %d:getMacAddr: Invalid wmac argument \n", devNum);
		return;
	}

	if (macAddr == NULL) {
		mError(devNum, EINVAL, "Device Number %d:getMacAddr: Invalid buffer address for returning mac address \n", devNum);
		return;
	}

#ifndef MDK_AP
	// Client card can have only wmac 
/*
#ifndef PREDATOR_BUILD
	if (wmac == 0) {
		mError(devNum, EINVAL, "Device Number %d:getMacAddr: Client card can read only WMAC address \n", devNum);
		return;
	}
#endif
*/
			// Read the mac address from the eeprom
	readVal = eepromRead(devNum,0x1d);
	macAddr[0] = (A_UINT8)(readVal & 0xff);
	macAddr[1] = (A_UINT8)((readVal >> 8) & 0xff);
	readVal = eepromRead(devNum,0x1e);
	macAddr[2] = (A_UINT8)(readVal & 0xff);
	macAddr[3] = (A_UINT8)((readVal >> 8) & 0xff);
	readVal = eepromRead(devNum,0x1f);
	macAddr[4] = (A_UINT8)(readVal & 0xff);
	macAddr[5] = (A_UINT8)((readVal >> 8) & 0xff);

	instNo = 0; // referencing to remove warning
#endif

#ifdef SPIRIT_AP
	if (spiritGetMacAddr(devNum,wmac,instNo,macAddr) < 0) {
		mError(devNum, EIO, "Get mac address failed \n");
		return;
	}	
#endif // SPIRIT

#ifdef AP22_AP
	mError(devNum, EIO,"Get Mac Address not implemented for AP22 \n");
#endif // AP22

#ifdef FREEDOM_AP
	if (freedomGetMacAddr(devNum,wmac,instNo,macAddr) < 0) {
		mError(devNum, EIO, "Get mac address failed \n");
		return;
	}	
#endif // FREEDOM

#ifdef SENAO_AP
	mError(devNum, EIO,"Get Mac Address not implemented for senao \n");
#endif // SENAO


	return;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
