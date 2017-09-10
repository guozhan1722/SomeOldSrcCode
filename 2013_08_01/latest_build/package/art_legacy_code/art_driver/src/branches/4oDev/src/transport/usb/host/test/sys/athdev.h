/*++

Module Name:

    athdev.h

Abstract:

Environment:

    Kernel mode

Notes:    

--*/

#ifndef _ATHUSB_DEV_H
#define _ATHUSB_DEV_H

NTSTATUS
AthUsb_DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
AthUsb_DispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
AthUsb_DispatchClean(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );


NTSTATUS
AthUsb_DispatchDevCtrl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
AthUsb_ResetPipe(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInfo
    );

NTSTATUS
AthUsb_ResetDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
AthUsb_GetPortStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PULONG PortStatus
    );

NTSTATUS
AthUsb_ResetParentPort(
    IN IN PDEVICE_OBJECT DeviceObject
    );

VOID
CancelSelectSuspend(
    IN PDEVICE_EXTENSION DeviceExtension
    );

VOID
StreamDpcProc(
              IN   PKDPC    Dpc,
              IN   PVOID    DeferredContex,
              IN   PVOID    SystemContex1,
              IN   PVOID    SystemContex2);

VOID
StreamApiDpcProc(
              IN   PKDPC    Dpc,
              IN   PVOID    DeferredContex,
              IN   PVOID    SystemContex1,
              IN   PVOID    SystemContex2);
#endif
