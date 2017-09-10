/*++

Module Name:

    athusr.h

Abstract:

Environment:

    Kernel mode

Notes:


--*/

#ifndef _ATHUSB_USER_H
#define _ATHUSB_USER_H

#include <initguid.h>

// {73FBE916-40E8-485f-AB11-DB6986B0E2D2}
DEFINE_GUID(GUID_CLASS_ATH_USB, 
0x73fbe916, 0x40e8, 0x485f, 0xab, 0x11, 0xdb, 0x69, 0x86, 0xb0, 0xe2, 0xd2);

#define ATHUSB_IOCTL_INDEX             0x0000


#define IOCTL_ATHUSB_GET_CONFIG_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     ATHUSB_IOCTL_INDEX,     \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)

#define IOCTL_ATHUSB_GET_DEVICE_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     ATHUSB_IOCTL_INDEX + 1, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)
                                                   
#define IOCTL_ATHUSB_RESET_DEVICE          CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     ATHUSB_IOCTL_INDEX + 2, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)

#define IOCTL_ATHUSB_RESET_PIPE            CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     ATHUSB_IOCTL_INDEX + 3, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)

#define IOCTL_ATHUSB_START_BULK_STREAM      CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     ATHUSB_IOCTL_INDEX + 4, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)

#define IOCTL_ATHUSB_STOP_BULK_STREAM       CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     ATHUSB_IOCTL_INDEX + 5, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)

#define IOCTL_ATHUSB_GET_STRING             CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     ATHUSB_IOCTL_INDEX + 6, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)

#define IOCTL_ATHUSB_GET_LANGUAGE_ID        CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     ATHUSB_IOCTL_INDEX + 7, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)

#define IOCTL_ATHUSB_SET_PACKET_DELIMIT     CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     ATHUSB_IOCTL_INDEX + 8, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)

#define IOCTL_ATHUSB_GET_STREAM_INFO        CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     ATHUSB_IOCTL_INDEX + 9, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)

typedef struct _QUERY_STRING {
    ULONG   DescNum;
    UCHAR   StringBuf[1];
}QUERY_STRING, *PQUERY_STRING;

#endif

