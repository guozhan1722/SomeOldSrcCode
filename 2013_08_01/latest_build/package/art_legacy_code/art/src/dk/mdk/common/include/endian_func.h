// endian_func.h - contians endian conversion functions 

// Copyright (c) 2000 Atheros Communications, Inc., All Rights Reserved */


// modification history
// --------------------
// 21Jan02	sharmat		Created.

// description
// -----------
// Contains functions which converts short and long integers from big endian
// to little endian and vice versa. 


#ifndef __INCendian_funch
#define __INCendian_funch

#ifdef WIN32
#include <windows.h>
#endif
#include "dk_common.h"
#include "wlantype.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus 

DLL_EXPORT A_UINT32 ltob_l
(
	A_UINT32 num
);

DLL_EXPORT A_UINT16 ltob_s
(
	A_UINT16 num
);

DLL_EXPORT A_UINT32 btol_l
(
	A_UINT32 num
);

DLL_EXPORT A_UINT16 btol_s
(
	A_UINT16 num
);

DLL_EXPORT A_UINT32 swap_l
(
	A_UINT32 num
);

DLL_EXPORT A_UINT16 swap_s
(
	A_UINT16 num
);

DLL_EXPORT void swapAndCopyBlock_l
(	
	void *dest,
	void *src,
	A_UINT32 size
);

DLL_EXPORT void swapBlock_l
(
	void *src,
	A_UINT32 size
);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __INCendian_funch

