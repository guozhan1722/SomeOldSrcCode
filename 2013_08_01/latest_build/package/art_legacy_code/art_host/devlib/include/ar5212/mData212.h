/*
 *  Copyright © 2002 Atheros Communications, Inc.,  All Rights Reserved.
 */
/* mdata(5)212.h - Type definitions needed for data transfer functions */


#ifndef	_MDATA212_H
#define	_MDATA212_H

#if 0
#ifdef Linux
#include "../mdata.h"
#else
#include "..\mdata.h"
#endif
#endif
#include "mdata.h"

#define XRMODE_REG	0x80c0
#define BITS_TO_XR_WAIT_FOR_POLL	7

void macAPIInitAr5212(A_UINT32 devNum);

void setDescriptorAr5212
(
 A_UINT32 devNum,
 MDK_ATHEROS_DESC*  localDescPtr,
 A_UINT32 pktSize,
 A_UINT32 antMode,
 A_UINT32 descNum,
 A_UINT32 rateValue,
 A_UINT32 broadcast
);

void setDescriptorEndPacketAr5212
(
 A_UINT32 devNum,
 MDK_ATHEROS_DESC*  localDescPtr,
 A_UINT32 pktSize,
 A_UINT32 antMode,
 A_UINT32 descNum,
 A_UINT32 rateValue,
 A_UINT32 broadcast
);

void setStatsPktDescAr5212
(
 A_UINT32 devNum,
 MDK_ATHEROS_DESC*  localDescPtr,
 A_UINT32 pktSize,
 A_UINT32 rateValue
);

void setContDescriptorAr5212
(
 A_UINT32 devNum,
 MDK_ATHEROS_DESC*  localDescPtr,
 A_UINT32 pktSize,
 A_UINT32 antMode,
 A_UCHAR  dataRate
);

A_UINT32 txGetDescRateAr5212
(
 A_UINT32 devNum,
 A_UINT32 descAddr
);

void rxBeginConfigAr5212
(
 A_UINT32 devNum
);

#endif	/* _MDATA212_H */
