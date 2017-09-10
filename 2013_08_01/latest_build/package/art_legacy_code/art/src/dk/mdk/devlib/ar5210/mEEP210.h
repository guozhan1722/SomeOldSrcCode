/*
 *  Copyright © 2002 Atheros Communications, Inc.,  All Rights Reserved.
 *
 *  mcfg(5)210.h - Type definitions needed for device specific functions 
 */

#ifndef	_MEEP210_H
#define	_MEEP210_H

#if 0
#ident  "ACI $Id: //depot/sw/branches/ART_V53/sw/src/dk/mdk/devlib/ar5210/mEEP210.h#1 $, $Header: //depot/sw/branches/ART_V53/sw/src/dk/mdk/devlib/ar5210/mEEP210.h#1 $"
#endif


A_BOOL read5210eepData
(
 A_UINT32	devNum
);

A_UINT32 check5210Prom
(
 A_UINT32 devNum,
 A_UINT32 enablePrint
);



#endif //_MEEP210_H

