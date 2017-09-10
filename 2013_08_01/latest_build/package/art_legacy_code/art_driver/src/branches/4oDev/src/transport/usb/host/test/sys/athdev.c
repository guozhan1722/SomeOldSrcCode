/*++

Module Name:

    athdev.c

Abstract:

    This file contains dispatch routines for create, 
    close and selective suspend. 
    The selective suspend feature is enabled if
    the SSRegistryEnable key in the registry is set to 1.

Environment:

    Kernel mode

Notes:

--*/

#include "precomp.h"
#include "athusbtst.h"

#define APITEST
/*****************************************************************************
* Routine Description:
*
*    Dispatch routine for create.
*
* Arguments:
*
*    DeviceObject - pointer to device object
*    Irp - I/O request packet.
*
* Return Value:
*
*    NT status value
*******************************************************************************/
NTSTATUS
AthUsb_DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{
    ULONG                       i;
    NTSTATUS                    ntStatus;
    PFILE_OBJECT                fileObject;
    PDEVICE_EXTENSION           deviceExtension;
    PIO_STACK_LOCATION          irpStack;
    PATHUSB_PIPE_CONTEXT       pipeContext;
    PUSBD_INTERFACE_INFORMATION interfaceInfo;

    PAGED_CODE();

    AthUsb_DbgPrint(3, ("AthUsb_DispatchCreate - begins\n"));

    //
    // initialize variables
    //
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if(deviceExtension->DeviceState != Working) {

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto AthUsb_DispatchCreate_Exit;
    }

    if(deviceExtension->UsbInterface) {

        interfaceInfo = deviceExtension->UsbInterface;
    }
    else {

        AthUsb_DbgPrint(1, ("UsbInterface not found\n"));

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto AthUsb_DispatchCreate_Exit;
    }

    //
    // FsContext is Null for the device
    //
    if(fileObject) {
        
        fileObject->FsContext = NULL; 
    }
    else {

        ntStatus = STATUS_INVALID_PARAMETER;
        goto AthUsb_DispatchCreate_Exit;
    }

    if(0 == fileObject->FileName.Length) {

        //
        // opening a device as opposed to pipe.
        //
        ntStatus = STATUS_SUCCESS;

        InterlockedIncrement(&deviceExtension->OpenHandleCount);

        //
        // the device is idle if it has no open handles or pending PnP Irps
        // since we just received an open handle request, cancel idle req.
        //
        if(deviceExtension->SSEnable) {
        
            CancelSelectSuspend(deviceExtension);
        }

        goto AthUsb_DispatchCreate_Exit;
    }
    
    pipeContext = AthUsb_PipeWithName(DeviceObject, &fileObject->FileName);

    if(pipeContext == NULL) {

        ntStatus = STATUS_INVALID_PARAMETER;
        goto AthUsb_DispatchCreate_Exit;
    }

    ntStatus = STATUS_INVALID_PARAMETER;

    for(i=0; i<interfaceInfo->NumberOfPipes; i++) {

        if(pipeContext == &deviceExtension->PipeContext[i]) {

            //
            // found a match
            //
            AthUsb_DbgPrint(3, ("open pipe %d\n", i));

            fileObject->FsContext = &interfaceInfo->Pipes[i];
            
            ASSERT(fileObject->FsContext);

            pipeContext->PipeOpen = TRUE;

            ntStatus = STATUS_SUCCESS;

            //
            // increment OpenHandleCounts
            //
            InterlockedIncrement(&deviceExtension->OpenHandleCount);

            //
            // the device is idle if it has no open handles or pending PnP Irps
            // since we just received an open handle request, cancel idle req.
            //
            if(deviceExtension->SSEnable) {

                CancelSelectSuspend(deviceExtension);
            }

            break;
        }
    }

AthUsb_DispatchCreate_Exit:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    AthUsb_DbgPrint(3, ("AthUsb_DispatchCreate - ends\n"));
    
    return ntStatus;
}

/*****************************************************************************
* Routine Description:
*
*    Dispatch routine for close.
*
* Arguments:
*
*    DeviceObject - pointer to device object
*    Irp - I/O request packet
*
* Return Value:
*
*    NT status value
*******************************************************************************/
NTSTATUS
AthUsb_DispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{
    NTSTATUS               ntStatus;
    PFILE_OBJECT           fileObject;
    PDEVICE_EXTENSION      deviceExtension;
    PIO_STACK_LOCATION     irpStack;
    PATHUSB_PIPE_CONTEXT  pipeContext;
    PUSBD_PIPE_INFORMATION pipeInformation;
    
    PAGED_CODE();

    //
    // initialize variables
    //
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    pipeContext = NULL;
    pipeInformation = NULL;

    if (DeviceObject) {
       deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
          AthUsb_DbgPrint(3, ("AthUsb_DispatchClose - begins\n"));

          if(fileObject && fileObject->FsContext) {

             pipeInformation = fileObject->FsContext;

             if(0 != fileObject->FileName.Length) {

               pipeContext = AthUsb_PipeWithName(DeviceObject, 
                                               &fileObject->FileName);
             }

             if(pipeContext && pipeContext->PipeOpen) {
            
               pipeContext->PipeOpen = FALSE;
             }
          }
       //
       // set ntStatus to STATUS_SUCCESS 
       //
       ntStatus = STATUS_SUCCESS;
	}
	else 
       ntStatus = STATUS_DEVICE_NOT_CONNECTED;


    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    InterlockedDecrement(&deviceExtension->OpenHandleCount);

    AthUsb_DbgPrint(3, ("AthUsb_DispatchClose - ends\n"));

    return ntStatus;
}

NTSTATUS
AthUsb_DispatchClean(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++
 
Routine Description:

    Dispatch routine for IRP_MJ_CLEANUP

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the pnp manager

Return Value:

    NT status value

--*/
{
    PDEVICE_EXTENSION  deviceExtension;
    KIRQL              oldIrql;
    LIST_ENTRY         cleanupList;
    PLIST_ENTRY        thisEntry, 
                       nextEntry, 
                       listHead;
    PIRP               pendingIrp;
    PIO_STACK_LOCATION pendingIrpStack, 
                       irpStack;
    NTSTATUS           ntStatus;

    //
    // initialize variables
    //

    if (DeviceObject) {
        deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
        irpStack = IoGetCurrentIrpStackLocation(Irp);
        InitializeListHead(&cleanupList);

        AthUsb_DbgPrint(3, ("AthUsb_DispatchClean::"));
        AthUsb_IoIncrement(deviceExtension);

        //
        // acquire queue lock
        //
        KeAcquireSpinLock(&deviceExtension->QueueLock, &oldIrql);

        //
        // remove all Irp's that belong to input Irp's fileobject
        //

        listHead = &deviceExtension->NewRequestsQueue;

        for(thisEntry = listHead->Flink, nextEntry = thisEntry->Flink;
           thisEntry != listHead;
           thisEntry = nextEntry, nextEntry = thisEntry->Flink) {

            pendingIrp = CONTAINING_RECORD(thisEntry, IRP, Tail.Overlay.ListEntry);

            pendingIrpStack = IoGetCurrentIrpStackLocation(pendingIrp);

            if(irpStack->FileObject == pendingIrpStack->FileObject) {

                RemoveEntryList(thisEntry);

                //
                // set the cancel routine to NULL
                //
                if(NULL == IoSetCancelRoutine(pendingIrp, NULL)) {

                    InitializeListHead(thisEntry);
                }
                else {

                    InsertTailList(&cleanupList, thisEntry);
                }
            }
        }

        //
        // Release the spin lock
        //

        KeReleaseSpinLock(&deviceExtension->QueueLock, oldIrql);

        //
        // walk thru the cleanup list and cancel all the Irps
        //

        while(!IsListEmpty(&cleanupList)) {

            //
            // complete the Irp
            //
            thisEntry = RemoveHeadList(&cleanupList);

            pendingIrp = CONTAINING_RECORD(thisEntry, IRP, Tail.Overlay.ListEntry);

            pendingIrp->IoStatus.Information = 0;
            pendingIrp->IoStatus.Status = STATUS_CANCELLED;

            IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
        }
	}

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    AthUsb_DbgPrint(3, ("AthUsb_DispatchClean::"));
    AthUsb_IoDecrement(deviceExtension);

    return STATUS_SUCCESS;
}

NTSTATUS
AthUsb_DispatchDevCtrl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++
 
Routine Description:

    Dispatch routine for IRP_MJ_DEVICE_CONTROL

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet

Return Value:

    NT status value

--*/
{
    ULONG              code;
    PVOID              ioBuffer;
    ULONG              inputBufferLength;
    ULONG              outputBufferLength;
    ULONG              info;
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;
    PFILE_OBJECT       fileObject;

    //
    // initialize variables
    //
    info = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    code = irpStack->Parameters.DeviceIoControl.IoControlCode;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if(deviceExtension->DeviceState != Working) {

        AthUsb_DbgPrint(1, ("Invalid device state\n"));

        Irp->IoStatus.Status = ntStatus = STATUS_INVALID_DEVICE_STATE;
        Irp->IoStatus.Information = info;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return ntStatus;
    }

    AthUsb_DbgPrint(3, ("AthUsb_DispatchDevCtrl::"));
    AthUsb_IoIncrement(deviceExtension);

    //
    // It is true that the client driver cancelled the selective suspend
    // request in the dispatch routine for create.
    // But there is no guarantee that it has indeed been completed.
    // so wait on the NoIdleReqPendEvent and proceed only if this event
    // is signalled.
    //
    AthUsb_DbgPrint(3, ("Waiting on the IdleReqPendEvent\n"));
    
    //
    // make sure that the selective suspend request has been completed.
    //

    if(deviceExtension->SSEnable) {

        KeWaitForSingleObject(&deviceExtension->NoIdleReqPendEvent, 
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);
    }

    switch(code) {

    case IOCTL_ATHUSB_RESET_PIPE:
    {
        PFILE_OBJECT           fileObject;
        PUSBD_PIPE_INFORMATION pipe;

        pipe = NULL;
        fileObject = NULL;

        //
        // FileObject is the address of the kernel file object to
        // which the IRP is directed. Drivers use the FileObject
        // to correlate IRPs in a queue.
        //
        fileObject = irpStack->FileObject;

        if(fileObject == NULL) {

            ntStatus = STATUS_INVALID_PARAMETER;

            break;
        }

        pipe = (PUSBD_PIPE_INFORMATION) fileObject->FsContext;

        if(pipe == NULL) {

            ntStatus = STATUS_INVALID_PARAMETER;
        }
        else {
            
            ntStatus = AthUsb_ResetPipe(DeviceObject, pipe);
        }

        break;
    }

    case IOCTL_ATHUSB_GET_DEVICE_DESCRIPTOR:
    {
        ULONG length;

        if(deviceExtension->UsbDeviceDescriptor) {

            length = sizeof(USB_DEVICE_DESCRIPTOR);

            if(outputBufferLength >= length) {

                RtlCopyMemory(ioBuffer,
                              deviceExtension->UsbDeviceDescriptor,
                              length);

                info = length;

                ntStatus = STATUS_SUCCESS;
            }
            else {
                
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            }
        }
        else {
            
            ntStatus = STATUS_UNSUCCESSFUL;
        }

        break;
    }

    case IOCTL_ATHUSB_GET_CONFIG_DESCRIPTOR:
    {
        ULONG length;

        if(deviceExtension->UsbConfigurationDescriptor) {

            length = deviceExtension->UsbConfigurationDescriptor->wTotalLength;

            if(outputBufferLength >= length) {

                RtlCopyMemory(ioBuffer,
                              deviceExtension->UsbConfigurationDescriptor,
                              length);

                info = length;

                ntStatus = STATUS_SUCCESS;
            }
            else {
                
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            }
        }
        else {
            
            ntStatus = STATUS_UNSUCCESSFUL;
        }

        break;
    }

    case IOCTL_ATHUSB_GET_LANGUAGE_ID:
    {
        PULONG  pNum;
        PSHORT  pId;
        ULONG   totalLen,i;

        totalLen = sizeof(ULONG) + deviceExtension->LanguageNum * sizeof(USHORT);
        if (outputBufferLength < totalLen) 
        {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        pNum = (ULONG *)ioBuffer;
        (*pNum) = deviceExtension->LanguageNum;
        pNum ++;
        pId = (PUSHORT)pNum;

        for (i = 0; i < deviceExtension->LanguageNum; i ++) {
            (*pId) = deviceExtension->LanguageID[i];
            pId ++;
        }

        info = totalLen;
        ntStatus = STATUS_SUCCESS;
        break;
    }

    case IOCTL_ATHUSB_GET_STRING:
    {
        ULONG             index,i, dwLen;
        PQUERY_STRING     queryString;
        PMY_STRING_DESC   pMyStringDesc = NULL;
        UCHAR             *pch;
        ANSI_STRING       ansiString;
        
        if (inputBufferLength < sizeof(ULONG)) {
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
        }

        index = *((ULONG *)ioBuffer);

        queryString = (PQUERY_STRING) ioBuffer;
        dwLen = sizeof(ULONG);

        for (i = 0; i < deviceExtension->StringDescNum; i ++) {
            if (deviceExtension->StringDesc[i].Index == index) {
                pMyStringDesc = &deviceExtension->StringDesc[i];
                break;
            }
        }

        if (pMyStringDesc == NULL) {
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
        }

        pch = queryString->StringBuf;
        for (i = 0; i < deviceExtension->LanguageNum; i ++) {
            if (pMyStringDesc->UnicodeString[i].Buffer) {
                RtlUnicodeStringToAnsiString(&ansiString,
                                             &pMyStringDesc->UnicodeString[i],
                                             TRUE);

                dwLen += ansiString.Length;
                if (dwLen <= outputBufferLength) {
                    RtlCopyMemory(pch,
                        ansiString.Buffer,
                        ansiString.Length);

                    pch += ansiString.Length;
                    queryString->DescNum++;
                }
                else {
                    ntStatus = STATUS_BUFFER_TOO_SMALL;
                    break;
                }

                RtlFreeAnsiString(&ansiString);
            }
        }

        info = dwLen;
        ntStatus = STATUS_SUCCESS;
        break;
    }

    case IOCTL_ATHUSB_RESET_DEVICE:
        
        ntStatus = AthUsb_ResetDevice(DeviceObject);

        if(NT_SUCCESS(ntStatus)) {
            ntStatus = AthUsb_VendorReset(DeviceObject);
        }

        break;

    case IOCTL_ATHUSB_SET_PACKET_DELIMIT:
        if (inputBufferLength < sizeof(BOOLEAN)) {
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
        }

        deviceExtension->bZeroPacket = (BOOLEAN)(*((ULONG *)ioBuffer));
        info = 0;
        ntStatus = STATUS_SUCCESS;
        break;
        
    case IOCTL_ATHUSB_START_BULK_STREAM:
    {
        ULONG * pdwParam;
        if (inputBufferLength < 3 * sizeof(ULONG)) {
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
        }

        pdwParam = (ULONG *)ioBuffer;
#ifdef APITEST
        ntStatus = athUsbStartUsbStream(DeviceObject, Irp, pdwParam[0], pdwParam[1],pdwParam[2]);
#else
        ntStatus = AthUsb_StartUsbStream(DeviceObject, Irp, pdwParam[0], pdwParam[1],pdwParam[2]);
#endif

        return STATUS_SUCCESS;
    }
    case IOCTL_ATHUSB_GET_STREAM_INFO:
    {
        if (outputBufferLength >= 2 * sizeof(ATHUSB_STREAM_INFO)) {
            RtlCopyMemory(ioBuffer,
                          &deviceExtension->RecStreamInfo,
                          sizeof(ATHUSB_STREAM_INFO));

            RtlCopyMemory((char *)ioBuffer + sizeof(ATHUSB_STREAM_INFO),
                          &deviceExtension->XferStreamInfo,
                          sizeof(ATHUSB_STREAM_INFO));

            info = 2 * sizeof(ATHUSB_STREAM_INFO);

            ntStatus = STATUS_SUCCESS;
        }
        else {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }

        break;
    }
    case IOCTL_ATHUSB_STOP_BULK_STREAM:
    {           
        PFILE_OBJECT_CONTENT fileObjectContent;
        
#ifdef APITEST    
        PATHUSB_API_STREAM_OBJECT   streamObject;

        streamObject = deviceExtension->StreamApiObject;

        athUsbStopUsbStream(DeviceObject,
                            streamObject,                            
                            Irp);
#else
        PATHUSB_STREAM_OBJECT streamObject;

        if(fileObject && fileObject->FsContext) {

            fileObjectContent = (PFILE_OBJECT_CONTENT)
                                fileObject->FsContext;

            ntStatus = AthUsb_StopUsbStream(
                            DeviceObject,
                            InterlockedExchangePointer(
                                &fileObjectContent->StreamInformation,
                                NULL),
                            Irp);
        }
        else {

            streamObject = (PATHUSB_STREAM_OBJECT)deviceExtension->StreamObject;
            ntStatus = AthUsb_StopUsbStream(
                            DeviceObject,
                            streamObject,                            
                            Irp);
        }
#endif
        if (outputBufferLength >= 2 * sizeof(ATHUSB_STREAM_INFO)) {
            RtlCopyMemory(ioBuffer,
                          &deviceExtension->RecStreamInfo,
                          sizeof(ATHUSB_STREAM_INFO));

            RtlCopyMemory((char *)ioBuffer + sizeof(ATHUSB_STREAM_INFO),
                          &deviceExtension->XferStreamInfo,
                          sizeof(ATHUSB_STREAM_INFO));

            info = 2 * sizeof(ATHUSB_STREAM_INFO);

            ntStatus = STATUS_SUCCESS;
        }
        else {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }

        break;
    }

    default :

        ntStatus = STATUS_INVALID_DEVICE_REQUEST;

        break;
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = info;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    AthUsb_DbgPrint(3, ("AthUsb_DispatchDevCtrl::"));
    AthUsb_IoDecrement(deviceExtension);

    return ntStatus;
}

NTSTATUS
AthUsb_ResetPipe(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInfo
    )
/*++
 
Routine Description:

    This routine synchronously submits a URB_FUNCTION_RESET_PIPE
    request down the stack.

Arguments:

    DeviceObject - pointer to device object
    PipeInfo - pointer to PipeInformation structure
               to retrieve the pipe handle

Return Value:

    NT status value

--*/
{
    PURB              urb;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;
    ULONG             portStatus;

    //
    // initialize variables
    //

    urb = NULL;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;


    ntStatus = AthUsb_GetPortStatus(DeviceObject, &portStatus);

    if((NT_SUCCESS(ntStatus))                 &&
       (portStatus & USBD_PORT_ENABLED)    &&
       (portStatus & USBD_PORT_CONNECTED)) 
    {
        urb = ExAllocatePool(NonPagedPool, 
                         sizeof(struct _URB_PIPE_REQUEST));

        if(urb) {

            urb->UrbHeader.Length = (USHORT) sizeof(struct _URB_PIPE_REQUEST);
            urb->UrbHeader.Function = URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL;
            urb->UrbPipeRequest.PipeHandle = PipeInfo->PipeHandle;

            ntStatus = CallUSBD(DeviceObject, urb);

            ExFreePool(urb);
        }
        else {

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        if(NT_SUCCESS(ntStatus)) {
    
            AthUsb_DbgPrint(3, ("AthUsb_ResetPipe - success\n"));
            ntStatus = STATUS_SUCCESS;
        }
        else {

            AthUsb_DbgPrint(1, ("AthUsb_ResetPipe - failed\n"));
        }
    }
    else {
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    return ntStatus;
}

NTSTATUS
AthUsb_ResetDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
/*++
 
Routine Description:

    This routine invokes AthUsb_ResetParentPort to reset the device

Arguments:

    DeviceObject - pointer to device object

Return Value:

    NT status value

--*/
{
    NTSTATUS ntStatus;
    ULONG    portStatus;

    AthUsb_DbgPrint(3, ("AthUsb_ResetDevice - begins\n"));

    ntStatus = AthUsb_GetPortStatus(DeviceObject, &portStatus);

    if((NT_SUCCESS(ntStatus))                 &&
       (!(portStatus & USBD_PORT_ENABLED))    &&
       (portStatus & USBD_PORT_CONNECTED)) 
    {

        ntStatus = AthUsb_ResetParentPort(DeviceObject);
    }

    AthUsb_DbgPrint(3, ("AthUsb_ResetDevice - ends\n"));

    return ntStatus;
}

NTSTATUS
AthUsb_GetPortStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PULONG     PortStatus
    )
/*++
 
Routine Description:

    This routine retrieves the status value

Arguments:

    DeviceObject - pointer to device object
    PortStatus - port status

Return Value:

    NT status value

--*/
{
    NTSTATUS           ntStatus;
    KEVENT             event;
    PIRP               irp;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION  deviceExtension;

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    *PortStatus = 0;

    AthUsb_DbgPrint(3, ("AthUsb_GetPortStatus - begins\n"));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                    IOCTL_INTERNAL_USB_GET_PORT_STATUS,
                    deviceExtension->TopOfStackDeviceObject,
                    NULL,
                    0,
                    NULL,
                    0,
                    TRUE,
                    &event,
                    &ioStatus);

    if(NULL == irp) {

        AthUsb_DbgPrint(1, ("memory alloc for irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nextStack = IoGetNextIrpStackLocation(irp);

    ASSERT(nextStack != NULL);

    nextStack->Parameters.Others.Argument1 = PortStatus;

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, irp);

    if(STATUS_PENDING == ntStatus) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    }
    else {

        ioStatus.Status = ntStatus;
    }

    ntStatus = ioStatus.Status;

    AthUsb_DbgPrint(3, ("AthUsb_GetPortStatus - ends\n"));

    return ntStatus;
}

NTSTATUS
AthUsb_ResetParentPort(
    IN PDEVICE_OBJECT DeviceObject
    )
/*++
 
Routine Description:

    This routine sends an IOCTL_INTERNAL_USB_RESET_PORT
    synchronously down the stack.

Arguments:

Return Value:

--*/
{
    NTSTATUS           ntStatus;
    KEVENT             event;
    PIRP               irp;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION  deviceExtension;

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    AthUsb_DbgPrint(3, ("AthUsb_ResetParentPort - begins\n"));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                    IOCTL_INTERNAL_USB_RESET_PORT,
                    deviceExtension->TopOfStackDeviceObject,
                    NULL,
                    0,
                    NULL,
                    0,
                    TRUE,
                    &event,
                    &ioStatus);

    if(NULL == irp) {

        AthUsb_DbgPrint(1, ("memory alloc for irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nextStack = IoGetNextIrpStackLocation(irp);

    ASSERT(nextStack != NULL);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, irp);

    if(STATUS_PENDING == ntStatus) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    }
    else {

        ioStatus.Status = ntStatus;
    }

    ntStatus = ioStatus.Status;

    AthUsb_DbgPrint(3, ("AthUsb_ResetParentPort - ends\n"));

    return ntStatus;
}

VOID
CancelSelectSuspend(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++
 
Routine Description:

    This routine is invoked to cancel selective suspend request.

Arguments:

    DeviceExtension - pointer to device extension

Return Value:

    None.

--*/
{
    PIRP  irp;
    KIRQL oldIrql;

    irp = NULL;

    AthUsb_DbgPrint(3, ("CancelSelectSuspend - begins\n"));

    KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

    if(!CanDeviceSuspend(DeviceExtension))
    {
        AthUsb_DbgPrint(3, ("Device is not idle\n"));
    
        irp = (PIRP) InterlockedExchangePointer(
                            &DeviceExtension->PendingIdleIrp, 
                            NULL);
    }

    KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

    //
    // since we have a valid Irp ptr,
    // we can call IoCancelIrp on it,
    // without the fear of the irp 
    // being freed underneath us.
    //
    if(irp) {

        //
        // This routine has the irp pointer.
        // It is safe to call IoCancelIrp because we know that
        // the compleiton routine will not free this irp unless...
        // 
        //        
        if(IoCancelIrp(irp)) {

            AthUsb_DbgPrint(3, ("IoCancelIrp returns TRUE\n"));
        }
        else {
            AthUsb_DbgPrint(3, ("IoCancelIrp returns FALSE\n"));
        }

        //
        // ....we decrement the FreeIdleIrpCount from 2 to 1.
        // if completion routine runs ahead of us, then this routine 
        // decrements the FreeIdleIrpCount from 1 to 0 and hence shall
        // free the irp.
        //
        if(0 == InterlockedDecrement(&DeviceExtension->FreeIdleIrpCount)) {

            AthUsb_DbgPrint(3, ("CancelSelectSuspend frees the irp\n"));
            IoFreeIrp(irp);

            KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                       IO_NO_INCREMENT,
                       FALSE);
        }
    }

    AthUsb_DbgPrint(3, ("CancelSelectSuspend - ends\n"));

    return;
}

