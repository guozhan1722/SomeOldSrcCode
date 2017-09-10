#ifndef PROJECT_H
#define PROJECT_H

#define CUSTOMER_REL
//#define __ATH_DJGPPDOS__
#define LINUX
//#define linux
//#define __GNUC__

//#if 0
//#ifdef LINUX
typedef char                    A_CHAR;
typedef A_CHAR                  A_INT8;
typedef short                   A_INT16;
typedef long                    A_INT32;

typedef unsigned char           A_UCHAR;
typedef A_UCHAR                 A_UINT8;
typedef unsigned short          A_UINT16;
typedef unsigned long           A_UINT32;
typedef unsigned long           A_UINT;
typedef unsigned long           UINT;

typedef int                     A_BOOL;

typedef long long               A_INT64;
typedef unsigned long long      A_UINT64;
typedef unsigned long           ULONGLONG;
#define A_LL                    "ll"

typedef A_UINT64                A_LONGSTATS;

typedef A_INT8                  A_RSSI;
typedef A_INT32                 A_RSSI32;

//#endif /* Linux */
//#endif



//#include "wlantype.h"

//#define uiPrintf printf
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

#include "common_defs.h"

#define A_MACADDR_COMP(m1, m2) ( memcmp((char *)&((m1)->octets[0]),\
	(char *)&((m2)->octets[0]),\
	WLAN_MAC_ADDR_SIZE))
#define A_MACADDR_COPY(from, to) ((void *) memcpy(&((to)->octets[0]),&((from)->octets[0]),WLAN_MAC_ADDR_SIZE))

extern A_UINT32 swDeviceID;

#endif
