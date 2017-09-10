/*++

Module Name:

    athpnp.h

Abstract:

Environment:

    Kernel mode

Notes:

--*/

#ifndef _ATHUSB_PNP_H
#define _ATHUSB_PNP_H

#define REMOTE_WAKEUP_MASK 0x20
#define ATHUSB_DEFAULT_MAXIMUM_TRANSFER_SIZE   40960

NTSTATUS
AthUsb_DispatchPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleQueryStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleQueryRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleCancelRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleSurpriseRemoval(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleCancelStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleQueryCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
ReadandSelectDescriptors(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ConfigureDevice(
	IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SelectInterfaces(
	IN PDEVICE_OBJECT                DeviceObject,
	IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
    );

NTSTATUS
DeconfigureDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
ProcessQueuedRequests(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
AthUsb_AbortPipes(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
CanStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
CanRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
ReleaseMemory(
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
CanDeviceSuspend(
    IN PDEVICE_EXTENSION DeviceExtension
    );

PCHAR
PnPMinorFunctionString (
    IN UCHAR MinorFunction
    );

ULONG
AthUsb_GetStringDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR             Index,
    IN USHORT            LanguageId,
    IN OUT PVOID         pvOutputBuffer,
    IN ULONG             ulLength
    );

NTSTATUS
AthUsb_GetString(
    IN PDEVICE_OBJECT DeviceObject,
    UCHAR             Index);

VOID
AthUsb_GetAllStringDesciptor(
    IN PDEVICE_OBJECT DeviceObject
    );
#endif
