/*
 * $Id: //depot/sw/branches/ART_V53/sw/branches/4oDev/src/include/ntdefs.h#1 $
 *
 * Copyright © 2000-2003 Atheros Communications, Inc.,  All Rights Reserved.
 *
 * NDIS-specific declarations for some data structs used at application level
 */

#ifndef	_NTDEFS_H_
#define	_NTDEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KERPLUG
#include <windows.h>
#endif

#include "wlantype.h"

/* Driver specific data types */
#define A_SEM_TYPE	A_UINT32		/* @@@ temporary definition */

#ifdef __cplusplus
}
#endif

#endif /* _NTDEFS_H_ */
