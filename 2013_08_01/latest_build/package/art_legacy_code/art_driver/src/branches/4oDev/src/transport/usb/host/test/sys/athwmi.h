/*++


Module Name:

    athwmi.h

Abstract:

Environment:

    Kernel mode

Notes:


--*/

#ifndef _ATHUSB_WMI_H
#define _ATHUSB_WMI_H

NTSTATUS
AthUsb_WmiRegistration(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
AthUsb_WmiDeRegistration(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
AthUsb_DispatchSysCtrl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
AthUsb_QueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo	    
    );

NTSTATUS
AthUsb_SetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN ULONG          GuidIndex,
    IN ULONG          InstanceIndex,
    IN ULONG          DataItemId,
    IN ULONG          BufferSize,
    IN PUCHAR         Buffer
    );

NTSTATUS
AthUsb_SetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN ULONG          GuidIndex,
    IN ULONG          InstanceIndex,
    IN ULONG          BufferSize,
    IN PUCHAR         Buffer
    );

NTSTATUS
AthUsb_QueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN ULONG          GuidIndex,
    IN ULONG          InstanceIndex,
    IN ULONG          InstanceCount,
    IN OUT PULONG     InstanceLengthArray,
    IN ULONG          OutBufferSize,
    OUT PUCHAR        Buffer
    );

PCHAR
WMIMinorFunctionString (
    UCHAR MinorFunction
    );

#endif
