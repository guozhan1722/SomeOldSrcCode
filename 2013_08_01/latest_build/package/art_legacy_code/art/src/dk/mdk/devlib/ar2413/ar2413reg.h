/*
 *  Copyright � 2004 Atheros Communications, Inc.,  All Rights Reserved.
 *
 *  ar2413reg.h - Register definitions needed for device specific handling
 */

#ifndef	_ar2413reg_H
#define	_ar2413reg_H

#if 0
#ident  "ACI $Id: //depot/sw/branches/ART_V53/sw/src/dk/mdk/devlib/ar2413/ar2413reg.h#1 $, $Header: //depot/sw/branches/ART_V53/sw/src/dk/mdk/devlib/ar2413/ar2413reg.h#1 $"
#endif

#define TPCRG1_REG 0xa258
#define BB_FORCE_DAC_GAIN_SET         0x00000001
#define BB_NUM_PD_GAIN_CLEAR          0xffff3fff
#define BB_PD_GAIN_SETTING1_CLEAR     0xfffcffff
#define BB_PD_GAIN_SETTING2_CLEAR     0xfff3ffff
#define BB_PD_GAIN_SETTING3_CLEAR     0xffcfffff
#define BB_NUM_PD_GAIN_SHIFT          14
#define BB_PD_GAIN_SETTING1_SHIFT     16
#define BB_PD_GAIN_SETTING2_SHIFT     18
#define BB_PD_GAIN_SETTING3_SHIFT     20

#define TPCRG7_REG 0xa274
#define BB_FORCE_TX_GAIN_SET 0x00000001

#define TPCRG5_REG 0xa26c
#define BB_PD_GAIN_OVERLAP_MASK       0x0000000f

#endif //_ar2413reg_H
