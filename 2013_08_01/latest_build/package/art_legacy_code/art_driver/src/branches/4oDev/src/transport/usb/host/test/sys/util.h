#ifndef ATHUSB_UTIL_H
#define ATHUSB_UTIL_H

#define VENDOR_RESET_FEATURE      0x00
#define VENDOR_RESET_DEVICE       0x0001
#define VENDOR_RESET_ENDPOINT     0x0002

typedef struct _VENDOR_REQUEST {
    UCHAR     bRequest;
    USHORT    wValue;
    USHORT    wIndex;
    USHORT    wLength;
    UCHAR     direction;
    UCHAR     pData[1];
} VENDOR_REQUEST, *PVENDOR_REQUEST;

VOID
CheckOSVersion(IN PDEVICE_EXTENSION deviceExtension);

VOID
GetBusInterfaceVersion(
    IN PDEVICE_OBJECT DeviceObject);

NTSTATUS
IrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

LONG
AthUsb_IoIncrement(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    );

LONG
AthUsb_IoDecrement(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    );

VOID
GetPentiumCycleCounter(PLARGE_INTEGER cycleCount);

NTSTATUS
AthUsb_GetRegistryDword(
    IN     PWCHAR RegPath,
    IN     PWCHAR ValueName,
    IN OUT PULONG Value
    );

NTSTATUS
CallUSBD(
    IN PDEVICE_OBJECT DeviceObject,
    IN PURB           Urb
    );

PATHUSB_PIPE_CONTEXT
AthUsb_PipeWithName(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PUNICODE_STRING FileName
    );

ULONG Rand(void);

void 
GenRandData(CHAR	* pBuf, 
			ULONG	  dwLen);

NTSTATUS
AthUsb_BulkLatencyTest(IN PDEVICE_OBJECT  DeviceObject, LONG * TimeDiff);

VOID Delay(ULONG uMilliSeconds);

NTSTATUS
AthUsb_VendorRequest(IN PDEVICE_OBJECT  DeviceObject,
                     IN PVENDOR_REQUEST pVendorRequest);

#endif