/*++


Module Name:

    athrwr.c

Abstract:

    This file has routines to perform reads and writes.
    The read and writes are for bulk transfers.

Environment:

    Kernel mode

Notes:


--*/

#include "precomp.h"

NTSTATUS
AthUsb_DispatchReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++
 
Routine Description:

    Dispatch routine for read and write.
    This routine creates a ATHUSB_RW_CONTEXT for a read/write.
    This read/write is performed in stages of ATHUSB_MAX_TRANSFER_SIZE.
    once a stage of transfer is complete, then the irp is circulated again, 
    until the requested length of tranfer is performed.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet

Return Value:

    NT status value

--*/
{
    PMDL                   mdl;
    PURB                   urb;
    ULONG                  totalLength;
    ULONG                  stageLength;
    ULONG                  maxTransferLen;
    ULONG                  urbFlags;
    BOOLEAN                read, bPadZeroPacket = FALSE;
    NTSTATUS               ntStatus;
    PVOID                  virtualAddress;
    PFILE_OBJECT           fileObject;
    PDEVICE_EXTENSION      deviceExtension;
    PIO_STACK_LOCATION     irpStack;
    PIO_STACK_LOCATION     nextStack;
    PATHUSB_RW_CONTEXT     rwContext;
    PUSBD_PIPE_INFORMATION pipeInformation;    

    AthUsb_DbgPrint(3, ("AthUsb_DispatchReadWrite - begins\n"));

    //
    // initialize variables
    //
    urb = NULL;
    mdl = NULL;
    rwContext = NULL;
    totalLength = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    read = (irpStack->MajorFunction == IRP_MJ_READ) ? TRUE : FALSE;
    if (DeviceObject) {
        deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;    

        if(deviceExtension->DeviceState != Working) {

            AthUsb_DbgPrint(1, ("Invalid device state\n"));

            ntStatus = STATUS_INVALID_DEVICE_STATE;
            goto AthUsb_DispatchReadWrite_Exit;
        }

        //
        // It is true that the client driver cancelled the selective suspend
        // request in the dispatch routine for create Irps.
        // But there is no guarantee that it has indeed completed.
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

        if(fileObject && fileObject->FsContext) {

            pipeInformation = fileObject->FsContext;

            if((UsbdPipeTypeBulk != pipeInformation->PipeType) &&
               (UsbdPipeTypeInterrupt != pipeInformation->PipeType)) {
            
                AthUsb_DbgPrint(1, ("Usbd pipe type is not bulk or interrupt\n"));

                ntStatus = STATUS_INVALID_HANDLE;
                goto AthUsb_DispatchReadWrite_Exit;
            }
        }
        else {

            AthUsb_DbgPrint(1, ("Invalid handle\n"));

            ntStatus = STATUS_INVALID_HANDLE;
            goto AthUsb_DispatchReadWrite_Exit;
        }

        rwContext = (PATHUSB_RW_CONTEXT)
                    ExAllocatePool(NonPagedPool,
                                   sizeof(ATHUSB_RW_CONTEXT));

        if(rwContext == NULL) {
        
            AthUsb_DbgPrint(1, ("Failed to alloc mem for rwContext\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto AthUsb_DispatchReadWrite_Exit;
        }

        if(Irp->MdlAddress) {

            totalLength = MmGetMdlByteCount(Irp->MdlAddress);
        }

        if(totalLength > ATHUSB_TEST_BOARD_TRANSFER_BUFFER_SIZE) {

            AthUsb_DbgPrint(1, ("Transfer length > circular buffer\n"));

            ntStatus = STATUS_INVALID_PARAMETER;

            ExFreePool(rwContext);

            goto AthUsb_DispatchReadWrite_Exit;
        }

        if(totalLength == 0) {

            AthUsb_DbgPrint(1, ("Transfer data length = 0\n"));

            ntStatus = STATUS_SUCCESS;

            ExFreePool(rwContext);

            goto AthUsb_DispatchReadWrite_Exit;
        }

        urbFlags = USBD_SHORT_TRANSFER_OK;
        virtualAddress = MmGetMdlVirtualAddress(Irp->MdlAddress);    

        if(read) {

            urbFlags |= USBD_TRANSFER_DIRECTION_IN;
            AthUsb_DbgPrint(3, ("Read operation\n"));
        }
        else {

            urbFlags |= USBD_TRANSFER_DIRECTION_OUT;
            AthUsb_DbgPrint(3, ("Write operation\n"));        
        }

        //
        // the transfer request is for totalLength.
        // To be compliant with the Universal Serial Bus Specification 1.1, 
        // driver must only transmit packets of maximum size (wMaxPacketSize) 
        // and then delimit the end of a data transmission by means of a packet 
        // of less than maximum size, or by means of a zero-length packet. 
        // For a client driver to be compliant with the USB specification, 
        // it must send a zero-length delimiting packet to a device to explicitly 
        // terminate a write transfer whenever the transfer's size is an exact 
        // multiple of the maximum. The system USB stack will not generate these 
        // packets automatically. 
        //
    
        //if(totalLength > ATHUSB_MAX_TRANSFER_SIZE) {
        maxTransferLen = (ATHUSB_MAX_TRANSFER_SIZE / pipeInformation->MaximumPacketSize) 
                         * pipeInformation->MaximumPacketSize;

        if(maxTransferLen == 0) maxTransferLen = pipeInformation->MaximumPacketSize;

        if (totalLength > maxTransferLen) {
            stageLength = maxTransferLen;
        }
        else {        
            if (totalLength >= pipeInformation->MaximumPacketSize) {
                stageLength = (totalLength / pipeInformation->MaximumPacketSize) *
                               pipeInformation->MaximumPacketSize;
                if (stageLength == totalLength) {
                    //The buffer size is an exact multiple of maximum packet size, 
                    //we need add zero length packet.
                    if (deviceExtension->bZeroPacket) {
                        bPadZeroPacket = TRUE;
                    }
                }
            }
            else {
                stageLength = totalLength;
            }
        }

        mdl = IoAllocateMdl(virtualAddress,
                            totalLength,
                            FALSE,
                            FALSE,
                            NULL);

        if (mdl == NULL) {
    
            AthUsb_DbgPrint(1, ("Failed to alloc mem for mdl\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            ExFreePool(rwContext);

            goto AthUsb_DispatchReadWrite_Exit;
        }

        //
        // map the portion of user-buffer described by an mdl to another mdl
        //
        IoBuildPartialMdl(Irp->MdlAddress,
                          mdl,
                          virtualAddress,
                          stageLength);

        urb = ExAllocatePool(NonPagedPool,
                             sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));

        if(urb == NULL) {

            AthUsb_DbgPrint(1, ("Failed to alloc mem for urb\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            ExFreePool(rwContext);
            IoFreeMdl(mdl);

            goto AthUsb_DispatchReadWrite_Exit;
        }

        //AthUsb_ResetPipe(DeviceObject, pipeInformation);

        UsbBuildInterruptOrBulkTransferRequest(
                                urb,
                                sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
                                pipeInformation->PipeHandle,
                                NULL,
                                mdl,
                                stageLength,
                                urbFlags,
                                NULL);

        //
        // set ATHUSB_RW_CONTEXT parameters.
        //
    
        rwContext->Urb             = urb;
        rwContext->Mdl             = mdl;
        rwContext->Length          = totalLength - stageLength;
        rwContext->Numxfer         = 0;
        rwContext->VirtualAddress  = (ULONG_PTR)virtualAddress + stageLength;
        rwContext->DeviceExtension = deviceExtension;
        rwContext->pipeInformation = pipeInformation;
        rwContext->bPadZeroPacket = bPadZeroPacket;
        rwContext->bRead = read;
        rwContext->StageLength = stageLength;

        //
        // use the original read/write irp as an internal device control irp
        //

        nextStack = IoGetNextIrpStackLocation(Irp);
        nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        nextStack->Parameters.Others.Argument1 = (PVOID) urb;
        nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                                 IOCTL_INTERNAL_USB_SUBMIT_URB;

        IoSetCompletionRoutine(Irp, 
                               (PIO_COMPLETION_ROUTINE)AthUsb_ReadWriteCompletion,
                               rwContext,
                               TRUE,
                               TRUE,
                               TRUE);

        //
        // since we return STATUS_PENDING call IoMarkIrpPending.
        // This is the boiler plate code.
        // This may cause extra overhead of an APC for the Irp completion
        // but this is the correct thing to do.
        //

        IoMarkIrpPending(Irp);

        AthUsb_DbgPrint(3, ("AthUsb_DispatchReadWrite::"));
        AthUsb_IoIncrement(deviceExtension);

        ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                                Irp);

        if(!NT_SUCCESS(ntStatus)) {

            AthUsb_DbgPrint(1, ("IoCallDriver fails with status %X\n", ntStatus));

            AthUsb_HandleError(ntStatus, DeviceObject, pipeInformation);
        }

        //
        // we return STATUS_PENDING and not the status returned by the lower layer.
        //
        return STATUS_PENDING;
	}
	else {
           ntStatus = STATUS_DEVICE_NOT_CONNECTED;
	}

AthUsb_DispatchReadWrite_Exit:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    AthUsb_DbgPrint(3, ("AthUsb_DispatchReadWrite - ends\n"));

    return ntStatus;
}

NTSTATUS
AthUsb_ReadWriteCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
/*++
 
Routine Description:

    This is the completion routine for reads/writes
    If the irp completes with success, we check if we
    need to recirculate this irp for another stage of
    transfer. In this case return STATUS_MORE_PROCESSING_REQUIRED.
    if the irp completes in error, free all memory allocs and
    return the status.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet
    Context - context passed to the completion routine.

Return Value:

    NT status value

--*/
{
    ULONG                  stageLength;
    NTSTATUS               ntStatus;
    PATHUSB_RW_CONTEXT     rwContext;
    ULONG                  maxTransferLen;
    PUSBD_PIPE_INFORMATION pipeInformation;

    AthUsb_DbgPrint(3, ("AthUsb_ReadWriteCompletion - begins\n"));

    //
    // initialize variables
    //
    rwContext = (PATHUSB_RW_CONTEXT) Context;
    ntStatus = Irp->IoStatus.Status;

    UNREFERENCED_PARAMETER(DeviceObject);    

    //
    // successfully performed a stageLength of transfer.
    // check if we need to recirculate the irp.
    //
    if(NT_SUCCESS(ntStatus)) {

        if(rwContext) {

            pipeInformation = rwContext->pipeInformation;

            rwContext->Numxfer += 
              rwContext->Urb->UrbBulkOrInterruptTransfer.TransferBufferLength;

            /*if (rwContext->Urb->UrbBulkOrInterruptTransfer.TransferBufferLength < 
                rwContext->StageLength)
            {
                rwContext->VirtualAddress = rwContext->VirtualAddress -
                                            rwContext->StageLength +
                                            rwContext->Urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
                rwContext->Length = rwContext->Length + rwContext->StageLength -
                    rwContext->Urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
            }*/
        
            if(rwContext->Length) {

                //
                // another stage transfer
                //
                AthUsb_DbgPrint(3, ("Another stage transfer...\n"));

                //if(totalLength > ATHUSB_MAX_TRANSFER_SIZE) {
                maxTransferLen = (ATHUSB_MAX_TRANSFER_SIZE / pipeInformation->MaximumPacketSize) 
                                 * pipeInformation->MaximumPacketSize;

                if(maxTransferLen == 0) maxTransferLen = pipeInformation->MaximumPacketSize;

                if (rwContext->Length > maxTransferLen) {
                    stageLength = maxTransferLen;
                }
                else {        
                    if (rwContext->Length >= pipeInformation->MaximumPacketSize) {

                        stageLength = (rwContext->Length / 
                                       pipeInformation->MaximumPacketSize) *
                                       pipeInformation->MaximumPacketSize;

                        if (stageLength == rwContext->Length) {
                            if (rwContext->DeviceExtension->bZeroPacket) {
                                rwContext->bPadZeroPacket = TRUE;
                            }
                        }
                    }
                    else {
                        stageLength = rwContext->Length;
                    }
                }

                ntStatus = AthUsb_ResumitIrp(rwContext, Irp, stageLength);
                if(!NT_SUCCESS(ntStatus)) {

                    AthUsb_DbgPrint(1, ("AthUsb_ResumitIrp fails with status %X\n", ntStatus));

                    if ((ntStatus != STATUS_CANCELLED) &&
                        (ntStatus != STATUS_DEVICE_NOT_CONNECTED)) 
                    {
                        AthsUsb_QueueWorkItem(DeviceObject,FALSE,pipeInformation,NULL,FALSE);
                    }
                    //return ntStatus;
                }
                
                return STATUS_MORE_PROCESSING_REQUIRED;
            }
            else {

                //
                // this is the last transfer
                //
                if ((!rwContext->bRead) && rwContext->bPadZeroPacket) {

                    //Send zero length packet to delimit the transfer.
                    rwContext->bPadZeroPacket = FALSE;
                    ntStatus = AthUsb_ResumitIrp(rwContext, Irp, 0);
                    if(!NT_SUCCESS(ntStatus)) {

                        if ((ntStatus != STATUS_CANCELLED) &&
                        (ntStatus != STATUS_DEVICE_NOT_CONNECTED)) 
                        {
                            AthUsb_DbgPrint(1, ("AthUsb_ResumitIrp fails with status %X\n", ntStatus));
                            AthsUsb_QueueWorkItem(DeviceObject,FALSE,pipeInformation,NULL,FALSE);
                        }
                        //return ntStatus;
                    }

                    return STATUS_MORE_PROCESSING_REQUIRED;
                }

                Irp->IoStatus.Information = rwContext->Numxfer;
            }
        }
    }
    else {

        AthUsb_DbgPrint(1, ("ReadWriteCompletion - failed with status = %X\n", ntStatus));
    }
    
    if(rwContext) {

        //
        // dump rwContext
        //
        AthUsb_DbgPrint(3, ("rwContext->Urb             = %X\n", 
                             rwContext->Urb));
        AthUsb_DbgPrint(3, ("rwContext->Mdl             = %X\n", 
                             rwContext->Mdl));
        AthUsb_DbgPrint(3, ("rwContext->Length          = %d\n", 
                             rwContext->Length));
        AthUsb_DbgPrint(3, ("rwContext->Numxfer         = %d\n", 
                             rwContext->Numxfer));
        AthUsb_DbgPrint(3, ("rwContext->VirtualAddress  = %X\n", 
                             rwContext->VirtualAddress));
        AthUsb_DbgPrint(3, ("rwContext->DeviceExtension = %X\n", 
                             rwContext->DeviceExtension));

        AthUsb_DbgPrint(3, ("AthUsb_ReadWriteCompletion::"));
        AthUsb_IoDecrement(rwContext->DeviceExtension);

        ExFreePool(rwContext->Urb);
        IoFreeMdl(rwContext->Mdl);
        ExFreePool(rwContext);
    }

    AthUsb_DbgPrint(3, ("AthUsb_ReadWriteCompletion - ends\n"));

    return ntStatus;
}

VOID 
AthUsb_HandleError(
                   IN NTSTATUS            ntStatus,
                   IN PDEVICE_OBJECT      DeviceObject,
                   PUSBD_PIPE_INFORMATION pipeInformation
                   )
{
    //
    // if the device was yanked out, then the pipeInformation 
    // field is invalid.
    // similarly if the request was cancelled, then we need not
    // invoked reset pipe/device.
    //
    if ((ntStatus != STATUS_CANCELLED) && 
        (ntStatus != STATUS_DEVICE_NOT_CONNECTED)) {

        ntStatus = AthUsb_ResetPipe(DeviceObject,
                                    pipeInformation);
    
        if (!NT_SUCCESS(ntStatus)) {
            
            AthUsb_DbgPrint(1, ("AthUsb_ResetPipe failed\n"));

            ntStatus = AthUsb_ResetDevice(DeviceObject);
        }
    }
    else {

        AthUsb_DbgPrint(3, ("ntStatus is STATUS_CANCELLED or "
                            "STATUS_DEVICE_NOT_CONNECTED\n"));
    }
}

NTSTATUS
AthUsb_ResumitIrp(
           IN PATHUSB_RW_CONTEXT  rwContext,
           IN PIRP                Irp,
           IN ULONG               stageLength)
{
    PIO_STACK_LOCATION  nextStack;
    NTSTATUS            ntStatus;    

    AthUsb_DbgPrint(3, ("AthUsb_ResumitIrp - begins\n"));
    if (stageLength != 0) {
        // the source MDL is not mapped and so when the lower driver
        // calls MmGetSystemAddressForMdl(Safe) on Urb->Mdl (target Mdl), 
        // system PTEs are used.
        // IoFreeMdl calls MmPrepareMdlForReuse to release PTEs (unlock
        // VA address before freeing any Mdl
        // Rather than calling IoFreeMdl and IoAllocateMdl each time,
        // just call MmPrepareMdlForReuse
        // Not calling MmPrepareMdlForReuse will leak system PTEs
        // 
        MmPrepareMdlForReuse(rwContext->Mdl);

        IoBuildPartialMdl(Irp->MdlAddress,
                      rwContext->Mdl,
                      (PVOID) rwContext->VirtualAddress,
                      stageLength);
            
        //
        // reinitialize the urb
        //
    
        rwContext->Urb->UrbBulkOrInterruptTransfer.TransferBufferMDL = rwContext->Mdl;
    } else {
        rwContext->Urb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    }

    rwContext->Urb->UrbBulkOrInterruptTransfer.TransferBufferLength = stageLength;
    rwContext->VirtualAddress += stageLength;
    rwContext->Length -= stageLength;
    rwContext->StageLength = stageLength;

    nextStack = IoGetNextIrpStackLocation(Irp);
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.Others.Argument1 = rwContext->Urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                       IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine(Irp,
                           AthUsb_ReadWriteCompletion,
                           rwContext,
                           TRUE,
                           TRUE,
                           TRUE);

    ntStatus = IoCallDriver(rwContext->DeviceExtension->TopOfStackDeviceObject,
                            Irp);

    AthUsb_DbgPrint(3, ("AthUsb_ResumitIrp - ends\n"));
    return ntStatus;    
}

NTSTATUS
AthsUsb_QueueWorkItem(
                 IN PDEVICE_OBJECT            DeviceObject,
                 IN BOOLEAN                   bStreaming,
                 IN PUSBD_PIPE_INFORMATION    pipeInformation,
                 IN PATHUSB_STREAM_OBJECT     StreamObject,                 
                 IN BOOLEAN                   bRead)
{
    PIO_WORKITEM            item;
    PWORKER_THREAD_CONTEXT  context;
    ULONG                   portStatus;
    NTSTATUS                ntStatus;
    LARGE_INTEGER           timeout;
    
    if (bStreaming && StreamObject->bStop) {
        return STATUS_SUCCESS;        
    }    

    AthUsb_DbgPrint(3, ("AthsUsb_QueueWorkItem - begins\n"));

    context = ExAllocatePool(NonPagedPool, sizeof(WORKER_THREAD_CONTEXT));
    if (NULL == context) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Note - Windows 98 doesn't support IoWorkItem's, it only supports the
    //        ExWorkItem version. We use the Io versions here because the
    //        Ex versions can cause system crashes during unload (ie the driver
    //        can unload in the middle of an Ex work item callback)
    //
    item = IoAllocateWorkItem(DeviceObject);

    if (NULL == item) {

        ExFreePool(context);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    context->DeviceObject = DeviceObject;
    context->pipeInformation = pipeInformation;
    context->StreamObject = StreamObject;
    context->WorkItem = item;
    context->bStreaming = bStreaming;
    context->bRead = bRead;

    IoQueueWorkItem(
        item,
        AthUsb_WorkerItemCallback,
        DelayedWorkQueue,
        context
        );

    AthUsb_DbgPrint(3, ("AthsUsb_QueueWorkItem - ends\n"));

    return STATUS_SUCCESS;
}

void
AthUsb_InitStreamObject(IN PATHUSB_STREAM_OBJECT StreamObject)
{
    StreamObject->bCancel = FALSE;
    StreamObject->bReadQueueReset = FALSE;
    StreamObject->bWriteQueueReset = FALSE;
    StreamObject->MaxReadTime = 0;
    StreamObject->MaxWriteTime = 0;
    StreamObject->ReadElapseTime = 0;
    StreamObject->ReadIoElapseTime = 0;
    StreamObject->ReadPendingIrps = 0;
    StreamObject->ReadPos = 0;
    StreamObject->ReadSequenceNum = 0;
    StreamObject->TempWritePos = 0;
    StreamObject->TempWritePos = 0;
    StreamObject->WriteElapseTime = 0;
    StreamObject->WritePendingIrps = 0;
    StreamObject->WritePos = 0;
    StreamObject->WriteSequenceNum = 0;
}


VOID
AthUsb_WorkerItemCallback(
     IN  PDEVICE_OBJECT  DeviceObject,
     IN  PVOID           Context)
{
    PWORKER_THREAD_CONTEXT  context = (PWORKER_THREAD_CONTEXT) Context;
    PUSBD_PIPE_INFORMATION  pipeInformation = context->pipeInformation;
    PDEVICE_EXTENSION       deviceExtension;
    NTSTATUS                ntStatus;
    PATHUSB_STREAM_OBJECT   streamObject;
    LARGE_INTEGER           dueTime;        
    ULONG                   portStatus;

    AthUsb_DbgPrint(3, ("AthUsb_WorkerItemCallback - begins\n"));

    ntStatus = AthUsb_GetPortStatus(context->DeviceObject, &portStatus);
    if (!(NT_SUCCESS(ntStatus)) || 
        ((portStatus & USBD_PORT_ENABLED) != USBD_PORT_ENABLED) ||
        ((portStatus & USBD_PORT_CONNECTED) != USBD_PORT_CONNECTED))
    {
        //Device is gone!
        IoFreeWorkItem(context->WorkItem);
        ExFreePool((PVOID)context);
        return;
    }

    deviceExtension = context->DeviceObject->DeviceExtension;
    streamObject = context->StreamObject;

    if (context->bStreaming == TRUE) {
        if (streamObject->bStop == FALSE) {
            //cancel and free all revious IRP, you can't send the same IRP down if this
            //IRP failed before.
            AthUsb_CancelStreamIrp(streamObject);
            //
            // wait for the transfer objects irps to complete.
            //
            KeWaitForSingleObject(&streamObject->NoPendingReadIrpEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);        

            KeWaitForSingleObject(&streamObject->NoPendingWriteIrpEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);        
    
            AthUsb_FreeStreamResource(streamObject); 
        
            AthUsb_InitStreamObject(streamObject);
        
            AthUsb_VendorReset(DeviceObject);

            if (streamObject->InPipeInformation) {
                ntStatus = AthUsb_ResetPipe(context->DeviceObject,
                                    streamObject->InPipeInformation);
    
                if (!NT_SUCCESS(ntStatus)) {
            
                    AthUsb_DbgPrint(1, ("AthUsb_ResetPipe failed\n"));

                    ntStatus = AthUsb_ResetDevice(context->DeviceObject);
                }
            }

            if (streamObject->OutPipeInformation) {
                ntStatus = AthUsb_ResetPipe(context->DeviceObject,
                                    streamObject->OutPipeInformation);
    
                if (!NT_SUCCESS(ntStatus)) {
            
                    AthUsb_DbgPrint(1, ("AthUsb_ResetPipe failed\n"));

                    ntStatus = AthUsb_ResetDevice(context->DeviceObject);
                }
            }            

            /*AthUsb_SetupFullDuplexXerf(context->DeviceObject,
                                   context->StreamObject);*/
            dueTime.QuadPart = -100;
            KeSetTimer(&deviceExtension->StreamTimer, dueTime,&deviceExtension->StreamDpc);
        }
    } else {

        if (pipeInformation) {
            ntStatus = AthUsb_ResetPipe(context->DeviceObject,
                                    pipeInformation);
    
            if (!NT_SUCCESS(ntStatus)) {
            
                AthUsb_DbgPrint(1, ("AthUsb_ResetPipe failed\n"));

                ntStatus = AthUsb_ResetDevice(context->DeviceObject);
            }
        }
    }

    //
    // Cleanup before exiting from the worker thread.
    //
    IoFreeWorkItem(context->WorkItem);
    ExFreePool((PVOID)context);

    AthUsb_DbgPrint(3, ("AthUsb_WorkerItemCallback - ends\n"));
}
  
VOID
StreamDpcProc(
              IN   PKDPC    Dpc,
              IN   PVOID    DeferredContex,
              IN   PVOID    SystemContex1,
              IN   PVOID    SystemContex2)
{
    PDEVICE_EXTENSION       deviceExtension;  
    PDEVICE_OBJECT          deviceObject;
    PATHUSB_STREAM_OBJECT   streamObject;

    AthUsb_DbgPrint(3, ("DpcRoutine - begins\n"));
    
    deviceObject = (PDEVICE_OBJECT)DeferredContex;
    deviceExtension = (PDEVICE_EXTENSION)deviceObject->DeviceExtension;

    //
    // Clear this event since a DPC has been fired!
    //
    KeClearEvent(&deviceExtension->StreamDpcEvent);

    streamObject = (PATHUSB_STREAM_OBJECT)deviceExtension->StreamObject;
    if (streamObject->bReadQueueReset) {
        AthsUsb_QueueWorkItem(deviceObject,TRUE,streamObject->InPipeInformation,streamObject,TRUE);
    } 
    else if (streamObject->bWriteQueueReset) {
        AthsUsb_QueueWorkItem(deviceObject,TRUE,streamObject->OutPipeInformation,streamObject,TRUE);
    }
    else {
        AthUsb_SetupFullDuplexXerf(deviceObject,streamObject);
    }

    KeSetEvent(&deviceExtension->StreamDpcEvent,
               IO_NO_INCREMENT,
               FALSE);

    AthUsb_DbgPrint(3, ("DpcRoutine - ends\n"));
}

NTSTATUS
AthUsb_StartUsbStream(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,    
    IN ULONG          InPipeNum,
    IN ULONG          OutPipeNum,
    IN ULONG          PacketSize
    )
/*++
 
Routine Description:

    This routine create a single stream object and
    invokes StartTransfer for ATHUSB_MAX_IRP number of 
    times.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet

Return Value:

    NT status value

--*/
{
    ULONG                  i, bufferLength;
    ULONG                  info;
    ULONG                  inputBufferLength;
    ULONG                  outputBufferLength;
    NTSTATUS               ntStatus;
    PFILE_OBJECT           fileObject;
    PDEVICE_EXTENSION      deviceExtension;
    PIO_STACK_LOCATION     irpStack;
    PATHUSB_STREAM_OBJECT  streamObject;
    PUSBD_PIPE_INFORMATION pipeInformation;
    PVOID                  ioBuffer;
    LARGE_INTEGER          dueTime;    

    AthUsb_DbgPrint(3, ("AthUsb_StartUsbStream - begins\n"));

    info = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    streamObject = NULL;
    pipeInformation = NULL;
    deviceExtension = DeviceObject->DeviceExtension;
    inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;

    streamObject = ExAllocatePool(NonPagedPool, 
                                  sizeof(struct _ATHUSB_STREAM_OBJECT));

    if(streamObject == NULL) {

        AthUsb_DbgPrint(1, ("failed to alloc mem for streamObject\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto AthUsb_StartUsbStream_Exit;
    }

    RtlZeroMemory(streamObject, sizeof(ATHUSB_STREAM_OBJECT));
    RtlZeroMemory(&deviceExtension->XferStreamInfo, sizeof(ATHUSB_STREAM_INFO));
    RtlZeroMemory(&deviceExtension->RecStreamInfo, sizeof(ATHUSB_STREAM_INFO));

    pipeInformation = &(deviceExtension->UsbInterface->Pipes[deviceExtension->RecvEndPoints[InPipeNum]]);
    if ((UsbdPipeTypeBulk != pipeInformation->PipeType) &&
         (UsbdPipeTypeInterrupt != pipeInformation->PipeType)) 
    {
        AthUsb_DbgPrint(1, ("Usbd pipe type is not bulk or interrupt\n"));
        ExFreePool(streamObject);            
        ntStatus = STATUS_INVALID_HANDLE;
        goto AthUsb_StartUsbStream_Exit;
    }

    AthUsb_VendorReset(DeviceObject);

    // reset the pipe
    //
    AthUsb_ResetPipe(DeviceObject, pipeInformation);
    streamObject->InPipeInformation = pipeInformation;
    
    pipeInformation = &(deviceExtension->UsbInterface->Pipes[deviceExtension->SendEndPoints[OutPipeNum]]);
    if ((UsbdPipeTypeBulk != pipeInformation->PipeType) &&
         (UsbdPipeTypeInterrupt != pipeInformation->PipeType)) 
    {
        AthUsb_DbgPrint(1, ("Usbd pipe type is not bulk or interrupt\n"));
        ExFreePool(streamObject);
        ntStatus = STATUS_INVALID_HANDLE;
        goto AthUsb_StartUsbStream_Exit;
    }

    // reset the pipe
    //
    AthUsb_ResetPipe(DeviceObject, pipeInformation);
    streamObject->OutPipeInformation = pipeInformation;

    streamObject->DeviceObject = DeviceObject;

    KeQuerySystemTime(&streamObject->StartTime);

    KeInitializeSpinLock(&streamObject->ReadQueueLock);
    InitializeListHead(&streamObject->ReadIdleIrpQueue);
    InitializeListHead(&streamObject->ReadPendingIrpQueue);

    KeInitializeEvent(&streamObject->NoPendingReadIrpEvent,
                      NotificationEvent,
                      TRUE);    

    KeInitializeSpinLock(&streamObject->WriteQueueLock);
    InitializeListHead(&streamObject->WriteIdleIrpQueue);
    InitializeListHead(&streamObject->WritePendingIrpQueue);

    KeInitializeEvent(&streamObject->NoPendingWriteIrpEvent,
                      NotificationEvent,
                      TRUE);    

    KeInitializeEvent(&streamObject->TimeEvent,
                      SynchronizationEvent,
                      FALSE); 
    
    streamObject->StreamPackSize = PacketSize;
    streamObject->SavedDataBuffer = ExAllocatePool(NonPagedPool,PacketSize * ATHUSB_MAX_BUF_NUM);
    streamObject->TempDataBuffer = ExAllocatePool(NonPagedPool,PacketSize * ATHUSB_MAX_BUF_NUM);

    streamObject->StackSize = (CCHAR) (deviceExtension->TopOfStackDeviceObject->StackSize + 1);

    if (streamObject->SavedDataBuffer == NULL || streamObject->TempDataBuffer == NULL) {

        AthUsb_DbgPrint(1, ("failed to alloc mem for SavedDataBuffer\n"));    
        
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        if (streamObject->SavedDataBuffer) {
            ExFreePool(streamObject->SavedDataBuffer);
        }
        if (streamObject->TempDataBuffer) {
            ExFreePool(streamObject->TempDataBuffer);
        }

        ExFreePool(streamObject);
        goto AthUsb_StartUsbStream_Exit;
    }

    deviceExtension->StreamObject = (PVOID)streamObject;
    dueTime.QuadPart = -100;
    KeSetTimer(&deviceExtension->StreamTimer, dueTime,&deviceExtension->StreamDpc);

    /*ntStatus = AthUsb_SetupFullDuplexXerf(DeviceObject,streamObject);
    if(ntStatus == STATUS_INSUFFICIENT_RESOURCES) {
        goto AthUsb_StartUsbStream_Exit;
    } */      

    if (fileObject && fileObject->FsContext) {
        
        if(streamObject->ReadPendingIrps) {

            ((PFILE_OBJECT_CONTENT)fileObject->FsContext)->StreamInformation 
                                                                = streamObject;
        }
        else {

            AthUsb_DbgPrint(1, ("no transfer object irp sent..abort..\n"));
            ExFreePool(streamObject);
            ((PFILE_OBJECT_CONTENT)fileObject->FsContext)->StreamInformation = NULL;
        }
    }
    else {
        *(ULONG *)ioBuffer = (ULONG) streamObject;
        info = sizeof(ULONG);
    }

AthUsb_StartUsbStream_Exit:

    Irp->IoStatus.Information = info;
    Irp->IoStatus.Status = ntStatus;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    AthUsb_DbgPrint(3, ("AthUsb_StartUsbStream::"));
    AthUsb_IoDecrement(deviceExtension);

    AthUsb_DbgPrint(3, ("AthUsb_StartUsbStream - ends\n"));

    return ntStatus;
}

NTSTATUS
AthUsb_SetupFullDuplexXerf(
                           IN PDEVICE_OBJECT         DeviceObject,
                           IN PATHUSB_STREAM_OBJECT  StreamObject)
{
    ULONG i;
    PDEVICE_EXTENSION      deviceExtension;
    NTSTATUS               ntStatus;

    AthUsb_DbgPrint(3, ("AthUsb_SetupFullDuplexXerf - begins\n"));

    if (StreamObject == NULL) {
        AthUsb_DbgPrint(3, ("AthUsb_SetupFullDuplexXerf - ends, StreamObject is NULL\n"));
        return STATUS_UNSUCCESSFUL;
    }
	
    deviceExtension = DeviceObject->DeviceExtension;

    for (i = 0; i < ATHUSB_MAX_IRP; i++) {

        ntStatus = AthUsb_StartTransfer(DeviceObject,
                                        StreamObject,                                                                     
                                        i, FALSE);

        if(!NT_SUCCESS(ntStatus)) {         
            
            AthUsb_DbgPrint(1, ("AthUsb_StartTransfer [%d] - failed\n", i));

            if(ntStatus == STATUS_INSUFFICIENT_RESOURCES) {

                AthUsb_StreamObjectCleanup(StreamObject,deviceExtension,FALSE);                
                return ntStatus;
            }
        }

        ntStatus = AthUsb_StartTransfer(DeviceObject,
                                        StreamObject,                                                                     
                                        i, TRUE);

        if(!NT_SUCCESS(ntStatus)) {         
            
            AthUsb_DbgPrint(1, ("AthUsb_StartTransfer [%d] - failed\n", i));

            if(ntStatus == STATUS_INSUFFICIENT_RESOURCES) {                

                AthUsb_StreamObjectCleanup(StreamObject,deviceExtension,FALSE);                
                return ntStatus;
            }
        }
    }	

    AthUsb_DbgPrint(3, ("AthUsb_SetupFullDuplexXerf - ends\n"));
    return ntStatus;
}

NTSTATUS
AthUsb_StartTransfer(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PATHUSB_STREAM_OBJECT StreamObject,     
    IN ULONG                 Index,
    IN BOOLEAN               bRead
    )
/*++
 
Routine Description:

    This routine creates a transfer object for each irp/urb pair.
    After initializing the pair, it sends the irp down the stack.

Arguments:

    DeviceObject - pointer to device object.
    StreamObject - pointer to stream object
    Index - index into the transfer object table in stream object

Return Value:

    NT status value

--*/
{
    PIRP                    irp;
    CCHAR                   stackSize;
    ULONG                   packetSize;
    ULONG                   maxXferSize;
    ULONG                   numPackets;
    NTSTATUS                ntStatus;
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      nextStack;
    PATHUSB_TRANSFER_OBJECT transferObject;
    PUSBD_PIPE_INFORMATION  pipeInformation;
    KIRQL                   oldIrql;

    AthUsb_DbgPrint(3, ("AthUsb_StartTransfer - begins\n"));

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    if (bRead) {
        pipeInformation = StreamObject->InPipeInformation; 
        AthUsb_DbgPrint(2, ("Setup read IRP\n"));
    }
    else {
        AthUsb_DbgPrint(2, ("Setup write IRP\n"));
        pipeInformation = StreamObject->OutPipeInformation;
    }
    
    //maxXferSize = pipeInformation->MaximumTransferSize;
    maxXferSize = StreamObject->StreamPackSize;
    packetSize = pipeInformation->MaximumPacketSize;
    numPackets = maxXferSize / packetSize;

    if (StreamObject->SavedDataBufferLen < maxXferSize) {
        StreamObject->SavedDataBufferLen = maxXferSize;
    }    

    transferObject = ExAllocatePool(NonPagedPool,
                                    sizeof(struct _ATHUSB_TRANSFER_OBJECT));

    if (transferObject == NULL) {

        AthUsb_DbgPrint(1, ("failed to alloc mem for transferObject\n"));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(transferObject,
                  sizeof(struct _ATHUSB_TRANSFER_OBJECT));


    InitializeListHead(&transferObject->Link);

    transferObject->StreamObject = StreamObject;
    

    stackSize = StreamObject->StackSize;

    irp = IoAllocateIrp(stackSize, FALSE);

    if (irp == NULL) {

        AthUsb_DbgPrint(1, ("failed to alloc mem for irp\n"));

        ExFreePool(transferObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    transferObject->Irp = irp;

    transferObject->DataBuffer = ExAllocatePool(NonPagedPool,
                                                PAGE_SIZE        
                                                /*maxXferSize*/);

    
    if (transferObject->DataBuffer == NULL) {
        AthUsb_DbgPrint(1, ("failed to alloc mem for DataBuffer\n"));

        ExFreePool(transferObject);
        IoFreeIrp(irp);        
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(transferObject->DataBuffer, PAGE_SIZE);

    transferObject->DataBufferLen = maxXferSize;
    transferObject->bRead = bRead;

    if (bRead == FALSE) {
        AthUsb_GenerateTxData(StreamObject,transferObject,TRUE);        
    }

    transferObject->Urb = ExAllocatePool(NonPagedPool,
                                         sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));

    if (transferObject->Urb == NULL) {

        AthUsb_DbgPrint(1, ("failed to alloc mem for Urb\n"));

        ExFreePool(transferObject->DataBuffer);
        IoFreeIrp(irp);
        ExFreePool(transferObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }   

    AthUsb_DbgPrint(2, ("Irp = %x,transferObject= %x,DataBuffer= %x, Urb = %x\n",
                        irp,transferObject,transferObject->DataBuffer,transferObject->Urb));

    if (!bRead) {
        AthUsb_DbgPrint(3, ("DataBuffer = %x, %x, %x, %x,%x, %x, %x, %x\n",
                        transferObject->DataBuffer[0],transferObject->DataBuffer[1],
                        transferObject->DataBuffer[2],transferObject->DataBuffer[3],
                        transferObject->DataBuffer[4],transferObject->DataBuffer[5],
                        transferObject->DataBuffer[6],transferObject->DataBuffer[7]));
    }

    AthUsb_InitializeStreamUrb(DeviceObject, transferObject);    

    AthUsb_QueuePendingIrp(StreamObject, transferObject,bRead);    

    if (bRead) {
        InterlockedIncrement(&StreamObject->ReadPendingIrps);
    } else {
        InterlockedIncrement(&StreamObject->WritePendingIrps);
    }

    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->Parameters.Others.Argument1 = transferObject->Urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                   IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    if (bRead) {
        IoSetCompletionRoutine(irp, 
                           AthUsb_UsbReadIrp_Complete,
                           transferObject,
                           TRUE,
                           TRUE,
                           TRUE);
    } else {
        IoSetCompletionRoutine(irp, 
                           AthUsb_UsbWriteIrp_Complete,
                           transferObject,
                           TRUE,
                           TRUE,
                           TRUE);
    }


    AthUsb_DbgPrint(3, ("AthUsb_StartTransfer::"));
    AthUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                            irp);

    if(NT_SUCCESS(ntStatus)) {

        ntStatus = STATUS_SUCCESS;
    }
    else {
        AthUsb_DbgPrint(1, ("IoCallDriver fails with status %X\n", ntStatus));

        AthUsb_HandleError(ntStatus, DeviceObject, pipeInformation);
    }

    AthUsb_DbgPrint(3, ("AthUsb_StartTransfer - ends\n"));

    return ntStatus;
}


NTSTATUS
AthUsb_InitializeStreamUrb(
    IN PDEVICE_OBJECT          DeviceObject,
    IN PATHUSB_TRANSFER_OBJECT TransferObject
    )
/*++
 
Routine Description:

    This routine initializes the irp/urb pair in the transfer object.

Arguments:

    DeviceObject - pointer to device object
    TransferObject - pointer to transfer object

Return Value:

    NT status value

--*/
{
    PURB                  urb;
    ULONG                 i;    
    ULONG                 packetSize;
    ULONG                 numPackets;
    ULONG                 maxXferSize;
    PATHUSB_STREAM_OBJECT streamObject;
    PUSBD_PIPE_INFORMATION  pipeInformation;

    AthUsb_DbgPrint(3, ("AthUsb_InitializeStreamUrb - begins\n"));

    urb = TransferObject->Urb;
    streamObject = TransferObject->StreamObject;

    if (TransferObject->bRead) {
        pipeInformation = streamObject->InPipeInformation;
    }
    else {
        pipeInformation = streamObject->OutPipeInformation;
    }

    //maxXferSize = pipeInformation->MaximumTransferSize;
    maxXferSize = streamObject->StreamPackSize;
    packetSize = pipeInformation->MaximumPacketSize;
    numPackets = maxXferSize / packetSize;    

    if(numPackets > 255) {

        numPackets = 255;
        maxXferSize = packetSize * numPackets;
    }

    TransferObject->DataBufferLen = maxXferSize;

    (urb)->UrbHeader.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER; 
    (urb)->UrbHeader.Length = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
    (urb)->UrbBulkOrInterruptTransfer.PipeHandle = 
                                      pipeInformation->PipeHandle;

    (urb)->UrbBulkOrInterruptTransfer.TransferBufferLength = 
                                      TransferObject->DataBufferLen;

    (urb)->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    (urb)->UrbBulkOrInterruptTransfer.TransferBuffer = 
                                      TransferObject->DataBuffer;

    if (TransferObject->bRead) {
        (urb)->UrbBulkOrInterruptTransfer.TransferFlags = 
                      USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
    }
    else {
        (urb)->UrbBulkOrInterruptTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_OUT;
    }

    (urb)->UrbBulkOrInterruptTransfer.UrbLink = NULL;

    AthUsb_DbgPrint(3, ("AthUsb_InitializeStreamUrb - ends\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
AthUsb_UsbReadIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
/*++
 
Routine Description:

    This is the completion routine of the irp in the irp/urb pair
    passed down the stack for stream transfers.

    If the transfer was cancelled or the device yanked out, then we
    release resources, dump the statistics and return 
    STATUS_MORE_PROCESSING_REQUIRED, so that the cleanup module can
    free the irp.

    otherwise, we reinitialize the transfers and continue recirculaiton 
    of the irps.

Arguments:

    DeviceObject - pointer to device object below us.
    Irp - I/O completion routine.
    Context - context passed to the completion routine

Return Value:

--*/
{
    NTSTATUS                ntStatus;
    PDEVICE_OBJECT          deviceObject;
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      nextStack;
    PATHUSB_STREAM_OBJECT   streamObject;
    PATHUSB_TRANSFER_OBJECT transferObject;
    PUSBD_PIPE_INFORMATION  pipeInformation;    
    PATHUSB_STREAM_INFO     pStreamInfo;
    BOOLEAN                 bRead;
    PIRP                    irp;
    KIRQL                   oldIrql;
    LARGE_INTEGER           newTime,startTime,beginTime;
    ULONG                   elapseTime, seqNum;
    BOOLEAN                 bTooLong = FALSE;
    USBD_STATUS             usbdStatus;
    LARGE_INTEGER           dueTime;        

    AthUsb_DbgPrint(3, ("AthUsb_UsbReadIrp_Complete - begins\n"));     

    GetPentiumCycleCounter(&beginTime);

    transferObject = (PATHUSB_TRANSFER_OBJECT) Context;
    streamObject = transferObject->StreamObject;
    deviceObject = streamObject->DeviceObject;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;

    bRead = transferObject->bRead;
    pipeInformation = streamObject->InPipeInformation;
    pStreamInfo = &deviceExtension->RecStreamInfo;    

    /*AthUsb_DbgPrint(3, ("Irp = %x,transferObject= %x,DataBuffer= %x, Urb = %x\n",
                        transferObject->Irp,transferObject,transferObject->DataBuffer,transferObject->Urb));*/

    ntStatus = AthUsb_ProcessTransfer(transferObject,&usbdStatus);    

    if ((streamObject->bCancel == TRUE) || (!NT_SUCCESS(ntStatus))) {       

        /*AthUsb_DbgPrint(2, ("Bulk Read Irp %x, obj->Irp = %x ,urb = %x, databuf = %x cancelled/device removed\n",
            Irp, transferObject->Irp,transferObject->Urb,transferObject->DataBuffer));*/

        AthUsb_RecycleIrp(streamObject,transferObject,bRead);

        if (!NT_SUCCESS(ntStatus)) { 
            if ((ntStatus != STATUS_CANCELLED) && 
                (ntStatus != STATUS_DEVICE_NOT_CONNECTED) &&
                (usbdStatus != USBD_STATUS_DEVICE_GONE) && 
                (usbdStatus != USBD_STATUS_CANCELED) &&
                (!streamObject->bStop))
            {
                if (!streamObject->bReadQueueReset) {
                    streamObject->bReadQueueReset = streamObject->bWriteQueueReset = TRUE;
                    dueTime.QuadPart = -1000000;
                    KeSetTimer(&deviceExtension->StreamTimer, dueTime,&deviceExtension->StreamDpc);
                    //AthsUsb_QueueWorkItem(deviceObject,TRUE,pipeInformation,streamObject,TRUE);
                }
            } else {
                streamObject->bStop = TRUE;
            }
        }

        //
        // this is the last irp to complete with this erroneous value
        // signal an event and return STATUS_MORE_PROCESSING_REQUIRED
        //
        if (InterlockedDecrement(&streamObject->ReadPendingIrps) == 0) {

            AthUsb_DbgPrint(2, ("Set NoPendingReadIrpEvent\n"));

            KeSetEvent(&streamObject->NoPendingReadIrpEvent,
                       1,
                       FALSE);

            
	        KeQuerySystemTime(&newTime);

            elapseTime = (ULONG)((newTime.QuadPart - streamObject->StartTime.QuadPart) / 10000);
            AthUsb_DbgPrint(2, ("Total time spend on stream        = %d\n",elapseTime));

            AthUsb_DbgPrint(2, ("TotalPacketsProcessed   = %d\n", 
                             pStreamInfo->TotalPacketsProcessed));            

            AthUsb_DbgPrint(2, ("Total Cost Time        = %d\n",streamObject->ReadElapseTime));

            AthUsb_DbgPrint(2, ("Total Io Cost Time        = %d\n",streamObject->ReadIoElapseTime));

            AthUsb_DbgPrint(2, ("Max Cost Time        = %d\n",streamObject->MaxReadTime));
                             
            AthUsb_DbgPrint(3, ("-----------------------------\n"));
        }

        AthUsb_DbgPrint(2, ("AthUsb_UsbReadIrp_Complete - ends\n"));
        AthUsb_IoDecrement(deviceExtension);        

        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    
    //
    // otherwise circulate the irps.
    //
    transferObject->TotalBytesProcessed += transferObject->DataBufferLen;
    transferObject->TotalPacketsProcessed ++;   

    AthUsb_CompareRxData(streamObject,transferObject);

    /*AthUsb_DbgPrint(3, ("Read DataBuffer = %x, %x, %x, %x,%x, %x, %x, %x\n",
                        transferObject->DataBuffer[0],transferObject->DataBuffer[1],
                        transferObject->DataBuffer[2],transferObject->DataBuffer[3],
                        transferObject->DataBuffer[4],transferObject->DataBuffer[5],
                        transferObject->DataBuffer[6],transferObject->DataBuffer[7]));*/

    AthUsb_CalcStreamInfo(pStreamInfo,transferObject);
    AthUsb_RecycleIrp(streamObject,transferObject,bRead);
    AthUsb_ReInitXerfObject(transferObject);         
    
    //We don't need check failure case because we just put one free irp into queue
    AthUsb_GetNewTransferObj(streamObject,&transferObject,bRead); 
    irp = transferObject->Irp;

    RtlZeroMemory(transferObject->DataBuffer, transferObject->DataBufferLen);

    AthUsb_QueuePendingIrp(streamObject, transferObject,bRead);

    transferObject->bRead = bRead;

    AthUsb_InitializeStreamUrb(deviceObject, transferObject);    

    nextStack = IoGetNextIrpStackLocation(irp);
    nextStack->Parameters.Others.Argument1 = transferObject->Urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                                IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    GetPentiumCycleCounter(&startTime);

    IoSetCompletionRoutine(irp,
                           AthUsb_UsbReadIrp_Complete,
                           transferObject,
                           TRUE,
                           TRUE,
                           TRUE);

    transferObject->TimesRecycled++;    

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,Irp);

    if (!NT_SUCCESS(ntStatus)) {

        AthUsb_DbgPrint(1, ("IoCallDriver fails with status %X\n", ntStatus));

        if ((ntStatus != STATUS_CANCELLED) &&
            (ntStatus != STATUS_DEVICE_NOT_CONNECTED))
        {
            AthsUsb_QueueWorkItem(deviceObject,TRUE,pipeInformation,streamObject,TRUE);
        }
    }
    
    GetPentiumCycleCounter(&newTime);
    elapseTime = (ULONG)((newTime.QuadPart - startTime.QuadPart) / 1331);
    streamObject->ReadIoElapseTime += elapseTime;        

    if (streamObject->MaxReadTime < elapseTime) {
        streamObject->MaxReadTime = elapseTime;
    }

    elapseTime = (ULONG)((newTime.QuadPart - beginTime.QuadPart) / 1331);
    streamObject->ReadElapseTime += elapseTime;        
    

    /*if (bTooLong == FALSE) {
        if (elapseTime > 40000) {
            bTooLong = TRUE;
            AthUsb_DbgPrint(1, ("Too long time r4, elapse = %d\n",elapseTime));
        }
    }*/

    AthUsb_DbgPrint(3, ("AthUsb_UsbReadIrp_Complete - ends, elapseTime =%d\n",elapseTime));
    
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
AthUsb_UsbWriteIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
/*++
 
Routine Description:

    This is the completion routine of the irp in the irp/urb pair
    passed down the stack for stream transfers.

    If the transfer was cancelled or the device yanked out, then we
    release resources, dump the statistics and return 
    STATUS_MORE_PROCESSING_REQUIRED, so that the cleanup module can
    free the irp.

    otherwise, we reinitialize the transfers and continue recirculaiton 
    of the irps.

Arguments:

    DeviceObject - pointer to device object below us.
    Irp - I/O completion routine.
    Context - context passed to the completion routine

Return Value:

--*/
{
    NTSTATUS                ntStatus;
    PDEVICE_OBJECT          deviceObject;
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      nextStack;
    PATHUSB_STREAM_OBJECT   streamObject;
    PATHUSB_TRANSFER_OBJECT transferObject;
    PUSBD_PIPE_INFORMATION  pipeInformation;    
    PATHUSB_STREAM_INFO     pStreamInfo;
    BOOLEAN                 bRead;
    PIRP                    irp;
    KIRQL                   oldIrql;
    LARGE_INTEGER           newTime,startTime,beginTime;
    ULONG                   elapseTime;
    BOOLEAN                 bTooLong = FALSE;
    USBD_STATUS             usbdStatus;
    LARGE_INTEGER           dueTime;        

    AthUsb_DbgPrint(3, ("AthUsb_UsbWriteIrp_Complete - begins\n"));     

    GetPentiumCycleCounter(&beginTime);

    transferObject = (PATHUSB_TRANSFER_OBJECT) Context;
    streamObject = transferObject->StreamObject;
    deviceObject = streamObject->DeviceObject;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;

    bRead = transferObject->bRead;
    pipeInformation = streamObject->OutPipeInformation;
    pStreamInfo = &deviceExtension->XferStreamInfo;    
 
    /*AthUsb_DbgPrint(3, ("Irp = %x,transferObject= %x,DataBuffer= %x, Urb = %x\n",
                        transferObject->Irp,transferObject,transferObject->DataBuffer,transferObject->Urb));*/

    ntStatus = AthUsb_ProcessTransfer(transferObject,&usbdStatus);    

    if ((!NT_SUCCESS(ntStatus)) || (streamObject->bCancel == TRUE)) {          

        /*AthUsb_DbgPrint(2, ("Bulk Write Irp %x, obj->Irp = %x ,urb = %x, databuf = %x cancelled/device removed\n",
            Irp, transferObject->Irp,transferObject->Urb,transferObject->DataBuffer));*/

        if (!NT_SUCCESS(ntStatus)) {
            if ((ntStatus != STATUS_CANCELLED) && 
                (ntStatus != STATUS_DEVICE_NOT_CONNECTED) &&
                (usbdStatus != USBD_STATUS_DEVICE_GONE) && 
                (usbdStatus != USBD_STATUS_CANCELED) &&
                (!streamObject->bStop))
            {
                if (!streamObject->bWriteQueueReset) {
                    streamObject->bReadQueueReset = streamObject->bWriteQueueReset = TRUE;
                    dueTime.QuadPart = -1000000;
                    KeSetTimer(&deviceExtension->StreamTimer, dueTime,&deviceExtension->StreamDpc);
                    //AthsUsb_QueueWorkItem(deviceObject,TRUE,pipeInformation,streamObject,FALSE);
                }
            } else {
                streamObject->bStop = TRUE;
            }
        }

        AthUsb_RecycleIrp(streamObject,transferObject,bRead);

        //
        // this is the last irp to complete with this erroneous value
        // signal an event and return STATUS_MORE_PROCESSING_REQUIRED
        //
        if (InterlockedDecrement(&streamObject->WritePendingIrps) == 0) {

            AthUsb_DbgPrint(2, ("Set NoPendingWriteIrpEvent\n"));            

            KeSetEvent(&streamObject->NoPendingWriteIrpEvent,
                       1,
                       FALSE);

            
	        KeQuerySystemTime(&newTime);

            elapseTime = (ULONG)((newTime.QuadPart - streamObject->StartTime.QuadPart) / 10000);
            AthUsb_DbgPrint(2, ("Total time spend on stream        = %d\n",elapseTime));

            AthUsb_DbgPrint(2, ("TotalPacketsProcessed   = %d\n", 
                             pStreamInfo->TotalPacketsProcessed));            

            AthUsb_DbgPrint(2, ("Total Cost Time        = %d\n",streamObject->WriteElapseTime));
            AthUsb_DbgPrint(2, ("Total Io Cost Time        = %d\n",streamObject->WriteIoElapseTime));

            AthUsb_DbgPrint(2, ("Max Cost Time        = %d\n",streamObject->MaxWriteTime));
                             
            AthUsb_DbgPrint(3, ("-----------------------------\n"));
        }

        AthUsb_DbgPrint(2, ("AthUsb_UsbWriteIrp_Complete - ends\n"));
        AthUsb_IoDecrement(deviceExtension);        

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    //
    // otherwise circulate the irps.
    //
    transferObject->TotalBytesProcessed += transferObject->DataBufferLen;
    transferObject->TotalPacketsProcessed ++;   

    RtlCopyMemory(&streamObject->SavedDataBuffer[streamObject->WritePos * transferObject->DataBufferLen],
                   transferObject->DataBuffer,
                   transferObject->DataBufferLen);        

    streamObject->WritePos ++;
    if (streamObject->WritePos >= ATHUSB_MAX_BUF_NUM) {
        streamObject->WritePos = 0;
    }

    /*AthUsb_DbgPrint(3, ("Write DataBuffer = %x, %x, %x, %x,%x, %x, %x, %x\n",
                    transferObject->DataBuffer[0],transferObject->DataBuffer[1],
                    transferObject->DataBuffer[2],transferObject->DataBuffer[3],
                    transferObject->DataBuffer[4],transferObject->DataBuffer[5],
                    transferObject->DataBuffer[6],transferObject->DataBuffer[7]));*/
    
    AthUsb_CalcStreamInfo(pStreamInfo,transferObject);
    AthUsb_RecycleIrp(streamObject,transferObject,bRead);
    AthUsb_ReInitXerfObject(transferObject);       

    //We don't need check failure case because we just put one free irp into queue
    AthUsb_GetNewTransferObj(streamObject,&transferObject,bRead); 
    irp = transferObject->Irp;

    AthUsb_QueuePendingIrp(streamObject, transferObject,bRead);

    transferObject->bRead = bRead;

    AthUsb_GenerateTxData(streamObject,transferObject,FALSE);

    /*AthUsb_DbgPrint(3, ("New Data DataBuffer = %x, %x, %x, %x,%x, %x, %x, %x\n",
                        transferObject->DataBuffer[0],transferObject->DataBuffer[1],
                        transferObject->DataBuffer[2],transferObject->DataBuffer[3],
                        transferObject->DataBuffer[4],transferObject->DataBuffer[5],
                        transferObject->DataBuffer[6],transferObject->DataBuffer[7]));*/
    
    AthUsb_InitializeStreamUrb(deviceObject, transferObject);

    nextStack = IoGetNextIrpStackLocation(irp);
    nextStack->Parameters.Others.Argument1 = transferObject->Urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                                IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    GetPentiumCycleCounter(&startTime);

    IoSetCompletionRoutine(irp,
                           AthUsb_UsbWriteIrp_Complete,
                           transferObject,
                           TRUE,
                           TRUE,
                           TRUE);

    transferObject->TimesRecycled++;

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,Irp);

    if (!NT_SUCCESS(ntStatus)) {

        AthUsb_DbgPrint(1, ("IoCallDriver fails with status %X\n", ntStatus));

        if ((ntStatus != STATUS_CANCELLED) &&
            (ntStatus != STATUS_DEVICE_NOT_CONNECTED))
        {
            AthsUsb_QueueWorkItem(deviceObject,TRUE,pipeInformation,streamObject,FALSE);
        }
    }
    
    GetPentiumCycleCounter(&newTime);
    elapseTime = (ULONG)((newTime.QuadPart - startTime.QuadPart) / 1331);
    streamObject->WriteIoElapseTime += elapseTime;

    if (streamObject->MaxWriteTime < elapseTime) {
        streamObject->MaxWriteTime = elapseTime;
    }

    elapseTime = (ULONG)((newTime.QuadPart - beginTime.QuadPart) / 1331);
    streamObject->WriteElapseTime += elapseTime;    

    /*if (bTooLong == FALSE) {
        if (elapseTime > 40000) {
            bTooLong = TRUE;
            AthUsb_DbgPrint(1, ("Too long time w4, elapse = %d\n",elapseTime));
        }
    }*/

    AthUsb_DbgPrint(3, ("AthUsb_UsbWriteIrp_Complete - ends, elapseTime = %d\n",elapseTime));
    
    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID AthUsb_RecycleIrp(IN PATHUSB_STREAM_OBJECT   StreamObject,
                       IN PATHUSB_TRANSFER_OBJECT TransferObject,
                       IN BOOLEAN                 bRead)
{
    KIRQL                   oldIrql;

    AthUsb_DbgPrint(3, ("AthUsb_RecycleIrp - begins\n"));

    if (bRead) {
        KeAcquireSpinLock(&StreamObject->ReadQueueLock, &oldIrql);        
        RemoveEntryList(&TransferObject->Link);
        InsertTailList(&StreamObject->ReadIdleIrpQueue,&TransferObject->Link);    
        KeReleaseSpinLock(&StreamObject->ReadQueueLock, oldIrql);
    } else {
        KeAcquireSpinLock(&StreamObject->WriteQueueLock, &oldIrql);        
        RemoveEntryList(&TransferObject->Link);
        InsertTailList(&StreamObject->WriteIdleIrpQueue,&TransferObject->Link);    
        KeReleaseSpinLock(&StreamObject->WriteQueueLock, oldIrql);
    }

    AthUsb_DbgPrint(3, ("AthUsb_RecycleIrp - ends\n"));
}

VOID AthUsb_ReInitXerfObject(IN PATHUSB_TRANSFER_OBJECT TransferObject)
{
    AthUsb_DbgPrint(3, ("AthUsb_ReInitXerfObject - begins\n"));
    TransferObject->TimesRecycled = 0;
    TransferObject->ErrorPacketCount = 0;
    TransferObject->TotalBytesProcessed = 0;
    TransferObject->TotalPacketsProcessed = 0;
    TransferObject->ErrorSeqCount = 0;

    //RtlZeroMemory(TransferObject->DataBuffer, TransferObject->DataBufferLen);

    AthUsb_DbgPrint(3, ("AthUsb_ReInitXerfObject - ends\n"));
}

BOOLEAN 
AthUsb_GetNewTransferObj(IN PATHUSB_STREAM_OBJECT       StreamObject,                 
                       IN OUT PATHUSB_TRANSFER_OBJECT *pTransferObject,
                       IN BOOLEAN bRead)
{
    KIRQL                   oldIrql;
    PATHUSB_TRANSFER_OBJECT transferObject;    
    PLIST_ENTRY             listEntry;

    AthUsb_DbgPrint(3, ("AthUsb_GetNewTransferObj - begins\n"));

    if (bRead) {
        KeAcquireSpinLock(&StreamObject->ReadQueueLock, &oldIrql);        
        if (IsListEmpty(&StreamObject->ReadIdleIrpQueue)) {
            KeReleaseSpinLock(&StreamObject->ReadQueueLock, oldIrql);
            return FALSE;
        }
        else {
            listEntry = RemoveHeadList(&StreamObject->ReadIdleIrpQueue);        
            transferObject = CONTAINING_RECORD(listEntry, ATHUSB_TRANSFER_OBJECT, Link);   
            KeReleaseSpinLock(&StreamObject->ReadQueueLock, oldIrql);
        }
    } else {
        KeAcquireSpinLock(&StreamObject->WriteQueueLock, &oldIrql);        
        if (IsListEmpty(&StreamObject->WriteIdleIrpQueue)) {
            KeReleaseSpinLock(&StreamObject->WriteQueueLock, oldIrql);
            return FALSE;
        }
        else {
            listEntry = RemoveHeadList(&StreamObject->WriteIdleIrpQueue);        
            transferObject = CONTAINING_RECORD(listEntry, ATHUSB_TRANSFER_OBJECT, Link);   
            KeReleaseSpinLock(&StreamObject->WriteQueueLock, oldIrql);
        }
    }

    
    (*pTransferObject) = transferObject;

    AthUsb_DbgPrint(3, ("AthUsb_GetNewTransferObj - ends\n"));

    return TRUE;
}

AthUsb_QueuePendingIrp(IN PATHUSB_STREAM_OBJECT   StreamObject,
                       IN PATHUSB_TRANSFER_OBJECT TransferObject,
                       IN BOOLEAN bRead)
{
    KIRQL                   oldIrql;

    AthUsb_DbgPrint(3, ("AthUsb_QueuePendingIrp - begins\n"));

    if (bRead) {
        KeAcquireSpinLock(&StreamObject->ReadQueueLock, &oldIrql);    
        InsertTailList(&StreamObject->ReadPendingIrpQueue,&TransferObject->Link);
        KeReleaseSpinLock(&StreamObject->ReadQueueLock, oldIrql);
    } else {
        KeAcquireSpinLock(&StreamObject->WriteQueueLock, &oldIrql);    
        InsertTailList(&StreamObject->WritePendingIrpQueue,&TransferObject->Link);
        KeReleaseSpinLock(&StreamObject->WriteQueueLock, oldIrql);
    }

    AthUsb_DbgPrint(3, ("AthUsb_QueuePendingIrp - ends\n"));
}

NTSTATUS
AthUsb_ProcessTransfer(
    IN PATHUSB_TRANSFER_OBJECT TransferObject,
    USBD_STATUS                *pUsbdStatus
    )
/*++
 
Routine Description:

    This routine is invoked from the completion routine to check the status
    of the irp, urb and the bulk packets.

    updates statistics

Arguments:

    TranferObject - pointer to transfer object for the irp/urb pair which completed.

Return Value:

    NT status value

--*/
{
    PIRP        irp;
    PURB        urb;
    ULONG       i;
    NTSTATUS    ntStatus;
    USBD_STATUS usbdStatus;

    AthUsb_DbgPrint(3, ("AthUsb_ProcessTransfer - begins\n"));

    irp = TransferObject->Irp;
    urb = TransferObject->Urb;
    ntStatus = irp->IoStatus.Status;

    if(!NT_SUCCESS(ntStatus)) {

        AthUsb_DbgPrint(3, ("Bulk irp failed with status = %X\n", ntStatus));    
//        ASSERT (0);
    }

    usbdStatus = urb->UrbHeader.Status;

    if(!USBD_SUCCESS(usbdStatus)) {

        AthUsb_DbgPrint(3, ("urb failed with status = %X\n", usbdStatus));
//        ASSERT (0);
        if (NT_SUCCESS(ntStatus)) {
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }

    *pUsbdStatus = usbdStatus;
    
    AthUsb_DbgPrint(3, ("AthUsb_ProcessTransfer - ends\n"));

    return ntStatus;
}

NTSTATUS
AthUsb_StopUsbStream(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PATHUSB_STREAM_OBJECT StreamObject,
    IN PIRP                  Irp
    )
/*++
 
Routine Description:

    This routine is invoked from the IOCTL to stop the stream transfers.

Arguments:

    DeviceObject - pointer to device object
    StreamObject - pointer to stream object
    Irp - pointer to Irp

Return Value:

    NT status value

--*/
{
    ULONG              i;
    KIRQL              oldIrql;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;

    AthUsb_DbgPrint(2, ("AthUsb_StopUsbStream - begins\n"));
    //
    // initialize vars
    //
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;    

    if((StreamObject == NULL) ||
       (StreamObject->DeviceObject != DeviceObject)) {

        AthUsb_DbgPrint(1, ("invalid streamObject\n"));
        return STATUS_INVALID_PARAMETER;
    }

    AthUsb_StreamObjectCleanup(StreamObject, deviceExtension, TRUE);

    AthUsb_DbgPrint(2, ("AthUsb_StopUSbStream - ends\n"));

    return STATUS_SUCCESS;
}

void
AthUsb_CancelStreamIrp(IN PATHUSB_STREAM_OBJECT StreamObject)
{
    KIRQL                   oldIrql;
    PATHUSB_TRANSFER_OBJECT transferObject;    
    PLIST_ENTRY             listEntry;    

    AthUsb_DbgPrint(2, ("AthUsb_CancelStreamIrp - begins\n"));    

    StreamObject->bCancel = TRUE;

    KeAcquireSpinLock(&StreamObject->ReadQueueLock, &oldIrql);

    if(StreamObject->ReadPendingIrps != 0) {
        KeClearEvent(&StreamObject->NoPendingReadIrpEvent);
    }

    while (!IsListEmpty(&StreamObject->ReadPendingIrpQueue)) {        
        listEntry = RemoveHeadList(&StreamObject->ReadPendingIrpQueue);         
        transferObject = CONTAINING_RECORD(listEntry, ATHUSB_TRANSFER_OBJECT, Link);   
        
        //
        // cancel transferobject irps/urb pair
        // safe to touch these irps because the 
        // completion routine always returns 
        // STATUS_MORE_PRCESSING_REQUIRED
        // 
        //
        if(transferObject && transferObject->Irp) 
        {
            AthUsb_DbgPrint(2, ("Cancel Read Irp %x\n", transferObject->Irp));
            IoCancelIrp(transferObject->Irp);        
        }

        listEntry->Flink = listEntry->Blink = listEntry;
    }   

    KeReleaseSpinLock(&StreamObject->ReadQueueLock, oldIrql);        

    KeAcquireSpinLock(&StreamObject->WriteQueueLock, &oldIrql);

    if(StreamObject->WritePendingIrps != 0) {
        KeClearEvent(&StreamObject->NoPendingWriteIrpEvent);
    }

    while (!IsListEmpty(&StreamObject->WritePendingIrpQueue)) {        
        listEntry = RemoveHeadList(&StreamObject->WritePendingIrpQueue);         
        transferObject = CONTAINING_RECORD(listEntry, ATHUSB_TRANSFER_OBJECT, Link);   
        
        //
        // cancel transferobject irps/urb pair
        // safe to touch these irps because the 
        // completion routine always returns 
        // STATUS_MORE_PRCESSING_REQUIRED
        // 
        //
        if(transferObject && transferObject->Irp) 
        {
            AthUsb_DbgPrint(2, ("Cancel Write Irp %x\n", transferObject->Irp));
            IoCancelIrp(transferObject->Irp);        
        }

        listEntry->Flink = listEntry->Blink = listEntry;
    }   

    KeReleaseSpinLock(&StreamObject->WriteQueueLock, oldIrql);      

    AthUsb_DbgPrint(2, ("AthUsb_CancelStreamIrp - ends\n"));
}

void
AthUsb_FreeStreamResource(IN PATHUSB_STREAM_OBJECT StreamObject)
{
    KIRQL                   oldIrql;
    PATHUSB_TRANSFER_OBJECT xferObject;    
    PLIST_ENTRY             listEntry;

    AthUsb_DbgPrint(3, ("AthUsb_FreeStreamResource - begins\n"));

    KeAcquireSpinLock(&StreamObject->ReadQueueLock, &oldIrql);        
    while(!IsListEmpty(&StreamObject->ReadIdleIrpQueue)) {        
        listEntry = RemoveHeadList(&StreamObject->ReadIdleIrpQueue);        
        xferObject = CONTAINING_RECORD(listEntry, ATHUSB_TRANSFER_OBJECT, Link);   
        
        AthUsb_DbgPrint(3, ("Free Read Irp = %x, urb = %x, dataBuf = %x\n",
            xferObject->Irp,xferObject->Urb,xferObject->DataBuffer));

        if (xferObject) { 
            
            if (xferObject->Urb) {

                ExFreePool(xferObject->Urb);
                xferObject->Urb = NULL;
            }

            if (xferObject->DataBuffer) {                
                ExFreePool(xferObject->DataBuffer);
                xferObject->DataBuffer = NULL;
            }

            if (xferObject->Irp) {
                IoFreeIrp(xferObject->Irp);
            }
                
            ExFreePool(xferObject);
        }
    }
    
    KeReleaseSpinLock(&StreamObject->ReadQueueLock, oldIrql);    

    KeAcquireSpinLock(&StreamObject->WriteQueueLock, &oldIrql);        
    while(!IsListEmpty(&StreamObject->WriteIdleIrpQueue)) {        
        listEntry = RemoveHeadList(&StreamObject->WriteIdleIrpQueue);        
        xferObject = CONTAINING_RECORD(listEntry, ATHUSB_TRANSFER_OBJECT, Link);   
        
        AthUsb_DbgPrint(3, ("Free Write Irp = %x, urb = %x, dataBuf = %x\n",
            xferObject->Irp,xferObject->Urb,xferObject->DataBuffer));

        if (xferObject) { 
            
            if (xferObject->Urb) {

                ExFreePool(xferObject->Urb);
                xferObject->Urb = NULL;
            }

            if (xferObject->DataBuffer) {                
                ExFreePool(xferObject->DataBuffer);
                xferObject->DataBuffer = NULL;
            }

            if (xferObject->Irp) {
                IoFreeIrp(xferObject->Irp);
            }
                
            ExFreePool(xferObject);
        }
    }
    
    KeReleaseSpinLock(&StreamObject->WriteQueueLock, oldIrql);    

    AthUsb_DbgPrint(3, ("AthUsb_FreeStreamResource - ends\n"));
}

void
AthUsb_CalcStreamInfo(IN PATHUSB_STREAM_INFO     StreamInfo,
                      IN PATHUSB_TRANSFER_OBJECT TransferObject)
{   
    AthUsb_DbgPrint(3, ("AthUsb_CalcStreamInfo - begins\n"));
    //
    // dump the statistics
    //
    StreamInfo->TimesRecycled         += TransferObject->TimesRecycled;
    StreamInfo->TotalPacketsProcessed += TransferObject->TotalPacketsProcessed;    
    StreamInfo->TotalBytesProcessed   += (LONGLONG)TransferObject->TotalBytesProcessed;
    StreamInfo->ErrorPacketCount      += TransferObject->ErrorPacketCount;
    StreamInfo->ErrorSeqCount         += TransferObject->ErrorSeqCount;    

    AthUsb_DbgPrint(3, ("AthUsb_CalcStreamInfo - ends\n"));
}

NTSTATUS
AthUsb_StreamObjectCleanup(
    IN PATHUSB_STREAM_OBJECT StreamObject,
    IN PDEVICE_EXTENSION     DeviceExtension,
    IN BOOLEAN               bStop
    )
/*++
 
Routine Description:

    This routine is invoked either when the user-mode app passes an IOCTL to
    abort stream transfers or when the the cleanup dispatch routine is run.
    It is guaranteed to run only once for every stream transfer.

Arguments:

    StreamObject - StreamObject corresponding to stream transfer which
    needs to be aborted.

    DeviceExtension - pointer to device extension

Return Value:

    NT status value

--*/
{
    ULONG                   i;    
    PATHUSB_TRANSFER_OBJECT xferObject;
    NTSTATUS    ntStatus;
    LARGE_INTEGER timeout;
    
    AthUsb_DbgPrint(2, ("AthUsb_StreamObjectCleanup - begins\n"));

    StreamObject->bStop = TRUE;

    if (KeCancelTimer(&DeviceExtension->StreamTimer) == FALSE) {
        KeWaitForSingleObject(&DeviceExtension->StreamDpcEvent,
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);
    }

    AthUsb_CancelStreamIrp(StreamObject);    

    //
    // wait for the transfer objects irps to complete.
    //
    KeWaitForSingleObject(&StreamObject->NoPendingReadIrpEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    
    KeWaitForSingleObject(&StreamObject->NoPendingWriteIrpEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    AthUsb_FreeStreamResource(StreamObject);    
    
    /*if (bStop) {		

        ntStatus = AthUsb_StartTransfer(StreamObject->DeviceObject,
                                    StreamObject,                                                                     
                                    0, TRUE);

        if (NT_SUCCESS(ntStatus)) {
            KeClearEvent(&StreamObject->NoPendingIrpEvent);
            timeout.QuadPart = -4000000;
            KeWaitForSingleObject(&StreamObject->NoPendingIrpEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          &timeout);

            AthUsb_CancelStreamIrp(StreamObject);    

            KeWaitForSingleObject(&StreamObject->NoPendingIrpEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
        } 
    }*/

    if (StreamObject->SavedDataBuffer) {
        ExFreePool(StreamObject->SavedDataBuffer);
    }

    if (StreamObject->TempDataBuffer) {
        ExFreePool(StreamObject->TempDataBuffer);
    }

    ExFreePool(StreamObject);

    DeviceExtension->StreamObject = NULL;
//    AthUsb_ResetParentPort(DeviceObject);

    AthUsb_DbgPrint(2, ("AthUsb_StreamObjectCleanup - ends\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
AthUsb_VendorReset(IN PDEVICE_OBJECT        DeviceObject)
{
    VENDOR_REQUEST         vendorRequest;
    
    //reset the devie to clear all buffer
    vendorRequest.bRequest = VENDOR_RESET_FEATURE;
    vendorRequest.direction = 0;
    vendorRequest.wValue = VENDOR_RESET_DEVICE;
    vendorRequest.wIndex = 0;
    vendorRequest.wLength = 0;

    return AthUsb_VendorRequest(DeviceObject, &vendorRequest);
}

VOID
AthUsb_GenerateTxData(IN PATHUSB_STREAM_OBJECT   StreamObject,
                      IN PATHUSB_TRANSFER_OBJECT TransferObject,
                      IN BOOLEAN                  bRand)
{    
    ULONG    packetSize,i,loop, *pdwTemp;
    PUCHAR   pch;
    PULONG   pdwPtr;

    packetSize = (StreamObject->OutPipeInformation)->MaximumPacketSize;

    loop = TransferObject->DataBufferLen / packetSize;

    pch = TransferObject->DataBuffer;
    for (i = 0; i < loop; i ++) {        
        *((ULONG *)pch) = StreamObject->WriteSequenceNum;        
        pch[4] = (UCHAR)i;
        pdwTemp = (ULONG *)&pch[5];
        *pdwTemp = (ULONG) TransferObject->DataBuffer;
        StreamObject->WriteSequenceNum ++;

        if (bRand) {
            GenRandData(&pch[9],packetSize - 9);
        }

        pch += packetSize;
    }
    
    /*loop = packetSize / 4;
    pdwPtr = (PULONG)TransferObject->DataBuffer;
    for (i = 0 ; i < loop; i ++) {
         (*pdwPtr)= 0x2020202;
         pdwPtr ++;
    }

    loop = TransferObject->DataBufferLen / packetSize - 1;
    pch = TransferObject->DataBuffer + packetSize;
    for (i = 0; i < loop; i ++) {
        RtlCopyMemory(pch ,TransferObject->DataBuffer, packetSize);
        pch += packetSize;
    }*/
}

VOID
AthUsb_CompareRxData(IN PATHUSB_STREAM_OBJECT StreamObject,
                     IN PATHUSB_TRANSFER_OBJECT TransferObject)
{
    ULONG    packetSize,i,loop, seqNum,dataNum;
    PUCHAR   pch,pTemp;
    BOOLEAN  bWrongSeq = FALSE;

    packetSize = (StreamObject->InPipeInformation)->MaximumPacketSize;
    loop = TransferObject->DataBufferLen / packetSize;

    pch = TransferObject->DataBuffer;
    //for (i = 0; i < loop; i ++) {
    seqNum = *((ULONG *)pch);
    dataNum = pch[4];
    
    if (seqNum != StreamObject->ReadSequenceNum) {
         AthUsb_DbgPrint(3, ("Receive SeqNum = %d,    PacketNum = %d, Current SeqNum = %d\n",
                seqNum,dataNum, StreamObject->ReadSequenceNum));
//       bWrongSeq = TRUE;
         TransferObject->ErrorSeqCount ++;
//         ASSERT(0);
        
    }
    
    StreamObject->ReadSequenceNum += loop;
//  pch += packetSize;    

    if (StreamObject->ReadPos != StreamObject->WritePos) {        
        pch = &StreamObject->SavedDataBuffer[StreamObject->ReadPos * TransferObject->DataBufferLen];
        pTemp = &StreamObject->TempDataBuffer[StreamObject->TempReadPos * TransferObject->DataBufferLen];
        
        while (StreamObject->TempReadPos != StreamObject->TempWritePos) {
            if (RtlEqualMemory(pch,pTemp,TransferObject->DataBufferLen) == FALSE) {
                TransferObject->ErrorPacketCount ++;
                AthUsb_DbgPrint(3, ("Looptest failed\n"));
//                ASSERT (0);
            }

            StreamObject->TempReadPos ++;
            pTemp += TransferObject->DataBufferLen;
            if (StreamObject->TempReadPos >= ATHUSB_MAX_BUF_NUM) {
                StreamObject->TempReadPos = 0;        
                pTemp = StreamObject->TempDataBuffer;
            }

            StreamObject->ReadPos ++;
            pch += TransferObject->DataBufferLen;
            if (StreamObject->ReadPos >= ATHUSB_MAX_BUF_NUM) {
                StreamObject->ReadPos = 0;       
                pch = StreamObject->SavedDataBuffer;
            }

            if (StreamObject->ReadPos == StreamObject->WritePos) {
                break;
            }           
            
        }

        if (StreamObject->ReadPos == StreamObject->WritePos) {
            RtlCopyMemory (&StreamObject->TempDataBuffer[StreamObject->TempWritePos * TransferObject->DataBufferLen],
                           TransferObject->DataBuffer,
                           TransferObject->DataBufferLen);

            StreamObject->TempWritePos ++;
            if (StreamObject->TempWritePos >= ATHUSB_MAX_BUF_NUM) {
                StreamObject->TempWritePos = 0;        
            }
        } else {
            if (RtlEqualMemory(pch,
                               TransferObject->DataBuffer,
                               TransferObject->DataBufferLen) == FALSE) 
            {
                TransferObject->ErrorPacketCount ++;
                AthUsb_DbgPrint(3, ("Looptest failed\n"));
//                ASSERT (0);
            }
    
            StreamObject->ReadPos ++;
            if (StreamObject->ReadPos >= ATHUSB_MAX_BUF_NUM) {
                StreamObject->ReadPos = 0;        
            }
        }
    } else {
        RtlCopyMemory (&StreamObject->TempDataBuffer[StreamObject->TempWritePos * TransferObject->DataBufferLen],
                        TransferObject->DataBuffer,
                        TransferObject->DataBufferLen);

        StreamObject->TempWritePos ++;
        if (StreamObject->TempWritePos >= ATHUSB_MAX_BUF_NUM) {
            StreamObject->TempWritePos = 0;        
        }
    }
}
