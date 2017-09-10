/*
 * Copyright (c) 2004 Atheros Communications, Inc., All Rights Reserved
 *
 * athusbconfig.h -- Atheros USB Driver header file
 * 
 */

#ifndef __ATHUSB_CONFIG_H__
#define __ATHUSB_CONFIG_H__

#define ATHUSB_RESET_FEATURE      0x00
#define ATHUSB_RESET_DEVICE       0x0001
#define ATHUSB_RESET_ENDPOINT     0x0002

#define ATHUSB_REMOTE_WAKEUP_MASK 0x20

#define ATHUSB_DEFAULT_MAXIMUM_SIZE   4096

#define English             0x0409
#define French              0x040c
#define Spanish             0x0c0a 
#define Italian             0x0410 
#define Swedish             0x041D 
#define Dutch               0x0413 
#define Brazilian           0x0416 
#define Finnish             0x040b 
#define Norwegian           0x0414 
#define Danish              0x0406 
#define Hungarian           0x040e 
#define Polish              0x0415 
#define Russian             0x0419 
#define Czech               0x0405 
#define Greek               0x0408 
#define Portuguese          0x0816 
#define Turkish             0x041f 
#define Japanese            0x0411 
#define Korean              0x0412 
#define German              0x0407 
#define Chinese_Simplified  0x0804 
#define Chinese_Traditional 0x0404 
#define Arabic              0x0401 
#define Hebrew              0x040d 

typedef struct _ATHUSB_VENDOR_REQUEST {
    UCHAR     bRequest;
    USHORT    wValue;
    USHORT    wIndex;
    USHORT    wLength;
    UCHAR     direction;
    UCHAR     pData[1];
} ATHUSB_VENDOR_REQUEST, *PATHUSB_VENDOR_REQUEST;

NTSTATUS
athUsbCallUSBD(
    IN PATHUSB_USB_ADAPTER  pUsbAdapter,
    IN PURB                 urb
    );

VOID
athUsbGetBusVersion(
    IN PATHUSB_USB_ADAPTER  pUsbAdapter
    );

NTSTATUS
irpCompletionRoutine(
    IN PDEVICE_OBJECT deviceObject,
    IN PIRP           irp,
    IN PVOID          context
    );

NTSTATUS
athUsbVendorRequest(IN PATHUSB_USB_ADAPTER      pUsbAdapter,
                    IN PATHUSB_VENDOR_REQUEST   pVendorRequest);

NTSTATUS
athUsbVendorReset(IN PATHUSB_USB_ADAPTER  pUsbAdapter);

NTSTATUS
athUsbReadandSelectDescriptors(
                               IN PATHUSB_USB_ADAPTER  pUsbAdapter);

NTSTATUS
athUsbConfigureDevice(
                      IN PATHUSB_USB_ADAPTER  pUsbAdapter);

NTSTATUS
athUsbSelectInterfaces(
    IN PATHUSB_USB_ADAPTER           pUsbAdapter,
    IN PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor);

NTSTATUS
athUsbDeconfigureDevice(
    IN PATHUSB_USB_ADAPTER           pUsbAdapter);

VOID
athUsbGetAllStringDesciptor(
    IN PATHUSB_USB_ADAPTER           pUsbAdapter);

NTSTATUS
athUsbGetString(
    IN PATHUSB_USB_ADAPTER          pUsbAdapter,
    A_UCHAR                         index);

A_UINT32
athUsbGetStringDescriptor(
    IN PATHUSB_USB_ADAPTER          pUsbAdapter,
    IN A_UCHAR                      index,
    IN USHORT                       languageId,
    IN OUT PVOID                    pvOutputBuffer,
    IN A_UINT32                     ulLength);

NTSTATUS
athUsbReleaseMemory(
    IN PATHUSB_USB_ADAPTER          pUsbAdapter);

NTSTATUS
athUsbResetPipe(
    IN PATHUSB_USB_ADAPTER          pUsbAdapter,
    IN PUSBD_PIPE_INFORMATION       PipeInfo);

NTSTATUS
athUsbResetAllPipes(IN PATHUSB_USB_ADAPTER      pUsbAdapter);

NTSTATUS
athUsbResetDevice(
    IN PATHUSB_USB_ADAPTER          pUsbAdapter);

NTSTATUS
athUsbGetPortStatus(
    IN PATHUSB_USB_ADAPTER      pUsbAdapter,
    IN OUT PULONG               PortStatus);

NTSTATUS
athUsbResetParentPort(
    IN PATHUSB_USB_ADAPTER      pUsbAdapter);

NTSTATUS
athUsbAbortPipes(IN PATHUSB_USB_ADAPTER      pUsbAdapter);

NTSTATUS
athUsbResetPipeState(IN PATHUSB_USB_ADAPTER      pUsbAdapter);

VOID
getPentiumCycleCounter(PLARGE_INTEGER cycleCount);

#endif /* __ATHUSBCONFIG_H__ */
