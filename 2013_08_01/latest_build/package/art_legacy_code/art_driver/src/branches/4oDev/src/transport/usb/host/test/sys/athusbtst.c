/*
 * Copyright (c) 2000-2002 Atheros Communications, Inc., All Rights Reserved
 *
 * athusbdrv.h -- Atheros USB Driver header file
 * 
 */

#include "precomp.h"
#include "athusbapi.h"
#include "athusbdrv.h"
#include "athusbconfig.h"
#include "athusbRxTx.h"
#include "athusbtst.h"

#define USEDPC
//#define DELAY

VOID
athUsbRecvIndication(IN  void     *pStubHandle,
                     IN  A_UINT8   epNum,
                     IN  A_UINT8  *buffer,
                     IN  A_UINT32  bytesReceived)
{
    PDEVICE_OBJECT              deviceObject;
    PDEVICE_EXTENSION           deviceExtension;
    PATHUSB_API_STREAM_OBJECT   streamObject;
    PATHUSB_API_TRANSFER_OBJECT transferObject;

    deviceObject = (PDEVICE_OBJECT)pStubHandle;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
    streamObject = deviceExtension->StreamApiObject;

    if (athUsbRemoveHeadList(streamObject,&transferObject,TRUE,
                             &streamObject->readPendingQueue) == FALSE)
    {
        return;
    }

    ASSERT (transferObject);

    transferObject->dataBuffer = buffer;
    transferObject->dataBufferLen = bytesReceived;

    if ((bytesReceived != 0) && (!streamObject->bCancel) && (!streamObject->bStop)) {
        athUsbInsertTailList(streamObject,transferObject,TRUE,
                            &streamObject->readDoneQueue);
#ifdef USEDPC
        if (streamObject->bDelay == FALSE) {
            KeInsertQueueDpc(&streamObject->streamDpc,NULL,NULL);
        }
#else
        if (!streamObject->bQueueThread) {
            streamObject->bQueueThread = TRUE;
            athsUsbQueueTestWorkItem(deviceObject,streamObject);
        }
#endif
    } else {         
        transferObject->dataBufferLen = gepBufSize[0][epNum];
        athUsbInsertTailList(streamObject,transferObject,TRUE,
                            &streamObject->readIdleQueue);
    }

    return;
}

VOID
athUsbSendConfirm(IN  void     *pStubHandle,
                  IN  A_UINT8   epNum,
                  IN  A_UINT8  *buffer,
                  IN  A_UINT32  bytesSent)
{
    PDEVICE_OBJECT              deviceObject;
    PDEVICE_EXTENSION           deviceExtension;
    PATHUSB_API_STREAM_OBJECT   streamObject;
    PATHUSB_API_TRANSFER_OBJECT transferObject;    

    deviceObject = (PDEVICE_OBJECT)pStubHandle;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
    streamObject = deviceExtension->StreamApiObject;    

    if (athUsbRemoveHeadList(streamObject,&transferObject,FALSE,
                             &streamObject->writePendingQueue) == FALSE) 
    {
        return;
    }

    ASSERT (transferObject);

    transferObject->dataBuffer = buffer;
    transferObject->dataBufferLen = bytesSent;

    if ((bytesSent != 0) && (!streamObject->bCancel) && (!streamObject->bStop)) {
        athUsbInsertTailList(streamObject,transferObject,FALSE,
                            &streamObject->writeDoneQueue);
#ifdef USEDPC
        if (streamObject->bDelay == FALSE) {
            KeInsertQueueDpc(&streamObject->streamDpc,NULL,NULL);
        }
#else
        if (!streamObject->bQueueThread) {
            streamObject->bQueueThread = TRUE;
            athsUsbQueueTestWorkItem(deviceObject,streamObject);
        }
#endif
    } else {         
        transferObject->dataBufferLen = gepBufSize[1][epNum];
        athUsbInsertTailList(streamObject,transferObject,FALSE,
                            &streamObject->writeIdleQueue);
    }

    return;
}

VOID
athUsbStatusIndication(IN  void     *pStubHandle,                            
                       IN  A_UINT32  status)
{
    PDEVICE_OBJECT              deviceObject;
    PDEVICE_EXTENSION           deviceExtension;
    PATHUSB_API_STREAM_OBJECT   streamObject;
    PATHUSB_API_TRANSFER_OBJECT transferObject;

    deviceObject = (PDEVICE_OBJECT)pStubHandle;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
    streamObject = deviceExtension->StreamApiObject;

    streamObject->busState = status;

    return;
}

VOID
StreamApiDpcProc(
              IN   PKDPC    Dpc,
              IN   PVOID    DeferredContex,
              IN   PVOID    SystemContex1,
              IN   PVOID    SystemContex2)
{ 
    PATHUSB_API_STREAM_OBJECT   streamObject;
    PATHUSB_API_TRANSFER_OBJECT transferObject;
    PDEVICE_OBJECT              deviceObject;
    PDEVICE_EXTENSION           deviceExtension;
    UCHAR                       *pch,epNum;
    LARGE_INTEGER               dueTime,newTime;
    ULONG                       timeDiff;

    AthUsb_DbgPrint(3, ("DpcRoutine - begins\n"));
    
    streamObject = (PATHUSB_API_STREAM_OBJECT)DeferredContex;   
    deviceObject = streamObject->deviceObject;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;

    if (streamObject->bStop || streamObject->bCancel) {
        return;
    }

    //
    // Clear this event since a DPC has been fired!
    //
    KeClearEvent(&streamObject->streamDpcEvent);
#ifdef DELAY
    if (streamObject->bDelay == FALSE) {
        dueTime.LowPart = -200000;
        dueTime.HighPart = -1;
        KeSetTimer(&streamObject->streamTimer, 
                    dueTime,&streamObject->streamDpc);
        streamObject->bDelay = TRUE;

        KeSetEvent(&streamObject->streamDpcEvent,
               IO_NO_INCREMENT,
               FALSE);
        GetPentiumCycleCounter(&streamObject->oldTime);        

        return;
    } else {
        streamObject->bDelay = FALSE;
        GetPentiumCycleCounter(&newTime);
        timeDiff = (ULONG)((newTime.QuadPart - streamObject->oldTime.QuadPart) /1331);

        if (streamObject->minTime > timeDiff) {
            streamObject->minTime = timeDiff;
        }

        if (streamObject->maxTime < timeDiff) {
            streamObject->maxTime = timeDiff;
        }
        streamObject->totalCycle ++;
        streamObject->totalTime += timeDiff;        
    }
#endif

    epNum = streamObject->outPipeNum;    
    pch = &streamObject->savedDataBuffer[streamObject->writePos * 
                                                  gepBufSize[1][epNum]];
    while (!IsListEmpty(&streamObject->writeDoneQueue)) {
        if (athUsbRemoveHeadList(streamObject,&transferObject,FALSE,
                            &streamObject->writeDoneQueue) == FALSE)
        {
            break;
        }

        ASSERT (transferObject);        
     
        KeAcquireSpinLockAtDpcLevel(&streamObject->dpcLock);
        (deviceExtension->XferStreamInfo).TimesRecycled++;
        (deviceExtension->XferStreamInfo).TotalBytesProcessed += 
                                                 transferObject->dataBufferLen;
        (deviceExtension->XferStreamInfo).TotalPacketsProcessed ++;

        RtlCopyMemory(pch,
                      transferObject->dataBuffer,
                      transferObject->dataBufferLen);        
        streamObject->writePos ++;
        pch += transferObject->dataBufferLen;
        if (streamObject->writePos >= ATHUSB_MAX_BUF_NUM) {
            streamObject->writePos = 0;
            pch = streamObject->savedDataBuffer;
        }
        
        transferObject->dataBufferLen = gepBufSize[1][epNum];
        athUsbGenerateTxData(streamObject,transferObject,TRUE);

        KeReleaseSpinLockFromDpcLevel(&streamObject->dpcLock);
        athUsbInsertTailList(streamObject,transferObject,FALSE,
                            &streamObject->writeIdleQueue);
    }

    epNum = streamObject->inPipeNum;    
    while (!IsListEmpty(&streamObject->readDoneQueue)) {
        if (athUsbRemoveHeadList(streamObject,&transferObject,TRUE,
            &streamObject->readDoneQueue) == FALSE) 
        {
            break;
        }
        
        ASSERT (transferObject);

        KeAcquireSpinLockAtDpcLevel(&streamObject->dpcLock);
        (deviceExtension->RecStreamInfo).TimesRecycled++;
        (deviceExtension->RecStreamInfo).TotalBytesProcessed += transferObject->dataBufferLen;
        (deviceExtension->RecStreamInfo).TotalPacketsProcessed ++;

        athUsbCompareRxData(streamObject,transferObject,&deviceExtension->RecStreamInfo);        
        transferObject->dataBufferLen = gepBufSize[0][epNum];

        KeReleaseSpinLockFromDpcLevel(&streamObject->dpcLock);

        athUsbInsertTailList(streamObject,transferObject,TRUE,
                            &streamObject->readIdleQueue);
    }

    if ((!streamObject->bCancel) && (!streamObject->bStop)) {
        if (streamObject->busState == ATHUSB_BUS_STATE_NORMAL) {
            while (!IsListEmpty(&streamObject->writeIdleQueue)) {
                if (athUsbRemoveHeadList(streamObject,&transferObject,FALSE,
                                     &streamObject->writeIdleQueue) == FALSE)
                {
                    break;
                }

                ASSERT (transferObject);

                pch = transferObject->dataBuffer;
                athUsbInsertTailList(streamObject,transferObject,FALSE,
                                     &streamObject->writePendingQueue);

                if (athUsbDrvSend(streamObject->pUsbAdapt,
                                  streamObject->outPipeNum,
                                  pch) != STATUS_SUCCESS)
                {
                    KeSetEvent(&streamObject->streamDpcEvent,
                               IO_NO_INCREMENT,
                               FALSE);
                    return;
                }
        
            }

            while (!IsListEmpty(&streamObject->readIdleQueue)) {
                if (athUsbRemoveHeadList(streamObject,&transferObject,TRUE,
                                     &streamObject->readIdleQueue) == FALSE)
                {
                    break;
                }

                ASSERT (transferObject);

                pch = transferObject->dataBuffer;
                athUsbInsertTailList(streamObject,transferObject,TRUE,
                                    &streamObject->readPendingQueue);

                if (athUsbDrvReceive(streamObject->pUsbAdapt,
                                     streamObject->inPipeNum,
                                     pch) != STATUS_SUCCESS)
                {
                    KeSetEvent(&streamObject->streamDpcEvent,
                               IO_NO_INCREMENT,
                               FALSE);
                    return;
                }
        
            }
        } else {
            dueTime.LowPart = -200;
            dueTime.HighPart = -1;
            KeSetTimer(&streamObject->streamTimer, 
                    dueTime,&streamObject->streamDpc);
        }
    }

    KeSetEvent(&streamObject->streamDpcEvent,
               IO_NO_INCREMENT,
               FALSE);

    AthUsb_DbgPrint(3, ("DpcRoutine - ends\n"));

    return;
}

NTSTATUS
athsUsbQueueTestWorkItem(
                 IN PDEVICE_OBJECT              DeviceObject,                
                 IN PATHUSB_API_STREAM_OBJECT   StreamObject)
{
    PIO_WORKITEM            item;
    PTEST_THREAD_CONTEXT    context;    
    NTSTATUS                ntStatus;   
    
    context = ExAllocatePool(NonPagedPool, sizeof(TEST_THREAD_CONTEXT));
    if (NULL == context) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    item = IoAllocateWorkItem(DeviceObject);

    if (NULL == item) {

        ExFreePool(context);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    context->deviceObject = DeviceObject;    
    context->streamObject = StreamObject;
    context->workItem = item;    

    IoQueueWorkItem(
        item,
        athUsbTestWorkerItemCallback,
        DelayedWorkQueue,
        context
        );

    AthUsb_DbgPrint(3, ("AthsUsb_QueueWorkItem - ends\n"));

    return STATUS_SUCCESS;
}

VOID
athUsbTestWorkerItemCallback(
     IN  PDEVICE_OBJECT  DeviceObject,
     IN  PVOID           Context)
{
    PTEST_THREAD_CONTEXT        context = (PTEST_THREAD_CONTEXT)Context;
    PATHUSB_API_STREAM_OBJECT   streamObject;
    PATHUSB_API_TRANSFER_OBJECT transferObject;
    PDEVICE_OBJECT              deviceObject;
    PDEVICE_EXTENSION           deviceExtension;
    UCHAR                       *pch,epNum;
    
    streamObject = context->streamObject;   
    deviceObject = context->deviceObject;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;

    if (streamObject->bStop || streamObject->bCancel) {
        IoFreeWorkItem(context->workItem);
        ExFreePool((PVOID)context);
        return;
    }

    epNum = streamObject->outPipeNum;    
    pch = &streamObject->savedDataBuffer[streamObject->writePos * 
                                                  gepBufSize[1][epNum]];
    while (!IsListEmpty(&streamObject->writeDoneQueue)) {
        athUsbRemoveHeadList(streamObject,&transferObject,FALSE,
                            &streamObject->writeDoneQueue);
        (deviceExtension->XferStreamInfo).TimesRecycled++;
        (deviceExtension->XferStreamInfo).TotalBytesProcessed += 
                                                 transferObject->dataBufferLen;
        (deviceExtension->XferStreamInfo).TotalPacketsProcessed ++;

        RtlCopyMemory(pch,
                      transferObject->dataBuffer,
                      transferObject->dataBufferLen);        
        streamObject->writePos ++;
        pch += transferObject->dataBufferLen;
        if (streamObject->writePos >= ATHUSB_MAX_BUF_NUM) {
            streamObject->writePos = 0;
            pch = streamObject->savedDataBuffer;
        }
        
        transferObject->dataBufferLen = gepBufSize[1][epNum];
        athUsbGenerateTxData(streamObject,transferObject,TRUE);
        athUsbInsertTailList(streamObject,transferObject,FALSE,
                            &streamObject->writeIdleQueue);
    }

    epNum = streamObject->inPipeNum;    
    while (!IsListEmpty(&streamObject->readDoneQueue)) {
        athUsbRemoveHeadList(streamObject,&transferObject,TRUE,&streamObject->readDoneQueue);
        (deviceExtension->RecStreamInfo).TimesRecycled++;
        (deviceExtension->RecStreamInfo).TotalBytesProcessed += transferObject->dataBufferLen;
        (deviceExtension->RecStreamInfo).TotalPacketsProcessed ++;

        athUsbCompareRxData(streamObject,transferObject,&deviceExtension->RecStreamInfo);        
        transferObject->dataBufferLen = gepBufSize[0][epNum];
        athUsbInsertTailList(streamObject,transferObject,TRUE,
                            &streamObject->readIdleQueue);
    }

    if ((!streamObject->bCancel) && (!streamObject->bStop)) {
        streamObject->bQueueThread = FALSE;
        if (streamObject->busState == ATHUSB_BUS_STATE_NORMAL) {
            while (!IsListEmpty(&streamObject->writeIdleQueue)) {
                athUsbRemoveHeadList(streamObject,&transferObject,FALSE,
                                     &streamObject->writeIdleQueue);

                pch = transferObject->dataBuffer;
                athUsbInsertTailList(streamObject,transferObject,FALSE,
                                     &streamObject->writePendingQueue);

                if (athUsbDrvSend(streamObject->pUsbAdapt,
                                  streamObject->outPipeNum,
                                  pch) != STATUS_SUCCESS)
                {    
                    IoFreeWorkItem(context->workItem);
                    ExFreePool((PVOID)context);
                    return;
                }            
            }

            while (!IsListEmpty(&streamObject->readIdleQueue)) {
                athUsbRemoveHeadList(streamObject,&transferObject,TRUE,
                                     &streamObject->readIdleQueue);
                pch = transferObject->dataBuffer;
                athUsbInsertTailList(streamObject,transferObject,TRUE,
                                    &streamObject->readPendingQueue);

                if (athUsbDrvReceive(streamObject->pUsbAdapt,
                                     streamObject->inPipeNum,
                                     pch) != STATUS_SUCCESS)
                {   
                    IoFreeWorkItem(context->workItem);
                    ExFreePool((PVOID)context);
                    return;
                }            
            }
        } else {
            athsUsbQueueTestWorkItem(deviceObject,streamObject);
        }
    }  

    IoFreeWorkItem(context->workItem);
    ExFreePool((PVOID)context);
}

NTSTATUS
athUsbStartUsbStream(
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
    ULONG                       i, bufferLength;
    ULONG                       info;
    ULONG                       inputBufferLength;
    ULONG                       outputBufferLength;
    NTSTATUS                    ntStatus;
    PFILE_OBJECT                fileObject;
    PDEVICE_EXTENSION           deviceExtension;
    PIO_STACK_LOCATION          irpStack;
    PATHUSB_API_STREAM_OBJECT   streamObject;
    PUSBD_PIPE_INFORMATION      pipeInformation;
    PVOID                       ioBuffer;
    LARGE_INTEGER               dueTime;
    ULONG                       allocSize;
    A_STATUS                    status;

    AthUsb_DbgPrint(3, ("AthUsb_StartUsbStream - begins\n"));

    info = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    streamObject = NULL;

    deviceExtension = DeviceObject->DeviceExtension;
    inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;

    streamObject = ExAllocatePool(NonPagedPool, 
                                  sizeof(struct _ATHUSB_API_STREAM_OBJECT));

    if(streamObject == NULL) {

        AthUsb_DbgPrint(1, ("failed to alloc mem for streamObject\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto athUsbStartUsbStream_Exit;
    }

    RtlZeroMemory(streamObject, sizeof(ATHUSB_API_STREAM_OBJECT));
    RtlZeroMemory(&deviceExtension->XferStreamInfo, sizeof(ATHUSB_STREAM_INFO));
    RtlZeroMemory(&deviceExtension->RecStreamInfo, sizeof(ATHUSB_STREAM_INFO));

    streamObject->deviceObject = DeviceObject;

    status = athUsbDrvInit(0,
                            deviceExtension->TopOfStackDeviceObject,
                            (void *)DeviceObject,
                            athUsbRecvIndication,
                            athUsbSendConfirm,
                            athUsbStatusIndication,
                            &streamObject->pUsbAdapt,&streamObject->totalInPipe,
                            &streamObject->totalOutPipe);

    if (status != A_OK) {

        AthUsb_DbgPrint(1, ("athUsbDrvInit failed\n"));
        ExFreePool(streamObject);            
        ntStatus = STATUS_INVALID_HANDLE;
        goto athUsbStartUsbStream_Exit;
    }

    if ((InPipeNum >= streamObject->totalInPipe) || 
        (OutPipeNum >= streamObject->totalOutPipe))
    {
        AthUsb_DbgPrint(1, ("Invalid pipe Number\n"));
        athUsbDrvExit(0,streamObject->pUsbAdapt);
        ExFreePool(streamObject);            
        ntStatus = STATUS_INVALID_HANDLE;
        goto athUsbStartUsbStream_Exit;
    }

    streamObject->inPipeNum = (UCHAR)InPipeNum;
    streamObject->outPipeNum = (UCHAR)OutPipeNum;        

    pipeInformation = &(deviceExtension->UsbInterface->Pipes[deviceExtension->RecvEndPoints[InPipeNum]]);
    streamObject->inPipeInformation = pipeInformation;
    pipeInformation = &(deviceExtension->UsbInterface->Pipes[deviceExtension->SendEndPoints[OutPipeNum]]);
    streamObject->outPipeInformation = pipeInformation;

    KeInitializeSpinLock(&streamObject->readQueueLock.spinLock);
    streamObject->readQueueLock.num = 0;
    streamObject->readQueueLock.label = 0;
    InitializeListHead(&streamObject->readIdleQueue);
    InitializeListHead(&streamObject->readPendingQueue);
    InitializeListHead(&streamObject->readDoneQueue);

    KeInitializeSpinLock(&streamObject->writeQueueLock.spinLock);
    streamObject->writeQueueLock.num = 0;
    streamObject->writeQueueLock.label = 1;
    InitializeListHead(&streamObject->writeIdleQueue);
    InitializeListHead(&streamObject->writePendingQueue);
    InitializeListHead(&streamObject->writeDoneQueue);
    
    KeInitializeSpinLock(&streamObject->dpcLock);

    KeInitializeEvent(&streamObject->timeEvent,
                      SynchronizationEvent,
                      FALSE); 
    
    KeInitializeDpc(&streamObject->streamDpc,StreamApiDpcProc,streamObject);    
    KeInitializeTimer(&streamObject->streamTimer);
    KeInitializeEvent(&streamObject->streamDpcEvent,NotificationEvent,TRUE);

    allocSize = PacketSize;
    if (allocSize < gepBufSize[0][InPipeNum]) {
        allocSize = gepBufSize[0][InPipeNum];
    }
    if (allocSize < gepBufSize[1][OutPipeNum]) {
        allocSize = gepBufSize[1][OutPipeNum];
    }

    streamObject->streamPackSize = allocSize;
    streamObject->savedDataBuffer = ExAllocatePool(NonPagedPool,allocSize * ATHUSB_MAX_BUF_NUM);
    streamObject->tempDataBuffer = ExAllocatePool(NonPagedPool,allocSize * ATHUSB_MAX_BUF_NUM);    

    streamObject->minTime = 0xFFFFFFFF;

    if (streamObject->savedDataBuffer == NULL || streamObject->tempDataBuffer == NULL) {

        AthUsb_DbgPrint(1, ("failed to alloc mem for SavedDataBuffer\n"));  
        athUsbDrvExit(0,streamObject->pUsbAdapt);
        
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        if (streamObject->savedDataBuffer) {
            ExFreePool(streamObject->savedDataBuffer);
        }
        if (streamObject->tempDataBuffer) {
            ExFreePool(streamObject->tempDataBuffer);
        }

        ExFreePool(streamObject);
        goto athUsbStartUsbStream_Exit;
    }

    streamObject->savedDataBufferLen = allocSize * ATHUSB_MAX_BUF_NUM;
    deviceExtension->StreamApiObject = (PVOID)streamObject;    

    ntStatus = athUsbSetupBufferQueue(DeviceObject,streamObject);
    if(ntStatus == STATUS_INSUFFICIENT_RESOURCES) {
        athUsbDrvExit(0,streamObject->pUsbAdapt);        
        
        ExFreePool(streamObject->savedDataBuffer);
        ExFreePool(streamObject->tempDataBuffer);
        ExFreePool(streamObject);

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

        goto athUsbStartUsbStream_Exit;
    }       

    *(ULONG *)ioBuffer = (ULONG) streamObject;
    info = sizeof(ULONG);    

    KeInsertQueueDpc(&streamObject->streamDpc,NULL,NULL);

athUsbStartUsbStream_Exit:

    Irp->IoStatus.Information = info;
    Irp->IoStatus.Status = ntStatus;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    AthUsb_DbgPrint(3, ("AthUsb_StartUsbStream::"));
    AthUsb_IoDecrement(deviceExtension);

    AthUsb_DbgPrint(3, ("AthUsb_StartUsbStream - ends\n"));

    return ntStatus;
}

NTSTATUS
athUsbSetupBufferQueue(IN PDEVICE_OBJECT             DeviceObject,
                       IN PATHUSB_API_STREAM_OBJECT  StreamObject)
{
    ULONG i;
    PDEVICE_EXTENSION      deviceExtension;
    NTSTATUS               ntStatus;    

    AthUsb_DbgPrint(3, ("athUsbSetupBufferQueue - begins\n"));

    if (StreamObject == NULL) {
        AthUsb_DbgPrint(3, ("athUsbSetupBufferQueue - ends, StreamObject is NULL\n"));
        return STATUS_UNSUCCESSFUL;
    }
	
    deviceExtension = DeviceObject->DeviceExtension;

    for (i = 0; i < ATHUSB_MAX_IRP; i++) {      

        ntStatus = athUsbAllocBuffer(DeviceObject, StreamObject,FALSE);

        if(!NT_SUCCESS(ntStatus)) {         
            
            AthUsb_DbgPrint(1, ("athUsbAllocBuffer [%d] - failed\n", i));

            if(ntStatus == STATUS_INSUFFICIENT_RESOURCES) {

                athUsbFreeBufferQueue(StreamObject);                
                return ntStatus;
            }
        }

        ntStatus = athUsbAllocBuffer(DeviceObject, StreamObject,TRUE);

        if(!NT_SUCCESS(ntStatus)) {         
            
            AthUsb_DbgPrint(1, ("athUsbAllocBuffer [%d] - failed\n", i));

            if(ntStatus == STATUS_INSUFFICIENT_RESOURCES) {                

                athUsbFreeBufferQueue(StreamObject);                
                return ntStatus;
            }
        }
    }	

    AthUsb_DbgPrint(3, ("athUsbSetupBufferQueue - ends\n"));
    return ntStatus;
}

NTSTATUS
athUsbAllocBuffer(
    IN PDEVICE_OBJECT            DeviceObject,
    IN PATHUSB_API_STREAM_OBJECT StreamObject,         
    IN BOOLEAN                   bRead
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
    NTSTATUS                        ntStatus;
    PDEVICE_EXTENSION               deviceExtension;
    PIO_STACK_LOCATION              nextStack;
    PATHUSB_API_TRANSFER_OBJECT     transferObject;    
    KIRQL                           oldIrql;
    A_UINT8                         epNum;
    ULONG                           allocSize;

    AthUsb_DbgPrint(3, ("AthUsb_StartTransfer - begins\n"));

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;    

    transferObject = ExAllocatePool(NonPagedPool,
                                    sizeof(struct _ATHUSB_TRANSFER_OBJECT));

    if (transferObject == NULL) {

        AthUsb_DbgPrint(1, ("failed to alloc mem for transferObject\n"));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(transferObject,
                  sizeof(struct _ATHUSB_TRANSFER_OBJECT));


    InitializeListHead(&transferObject->link);

    transferObject->streamObject = StreamObject; 
    
    if (bRead) {
        epNum = StreamObject->inPipeNum;        
        transferObject->dataBufferLen = gepBufSize[0][epNum];
        athUsbInsertTailList(StreamObject, transferObject,bRead,&StreamObject->readIdleQueue);
    } else {
        epNum = StreamObject->outPipeNum;        
        transferObject->dataBufferLen = gepBufSize[1][epNum];        
        athUsbInsertTailList(StreamObject, transferObject,bRead,&StreamObject->writeIdleQueue);
    }    
    
    allocSize = (transferObject->dataBufferLen / PAGE_SIZE + 1) * PAGE_SIZE;
    transferObject->dataBuffer = ExAllocatePool(NonPagedPool,
                                                allocSize);

    
    if (transferObject->dataBuffer == NULL) {
        AthUsb_DbgPrint(1, ("failed to alloc mem for DataBuffer\n"));
        ExFreePool(transferObject);              
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(transferObject->dataBuffer, allocSize);

    if (!bRead) {
        athUsbGenerateTxData(StreamObject,transferObject,TRUE);
    }

    transferObject->bRead = bRead;
    AthUsb_DbgPrint(3, ("AthUsb_StartTransfer - ends\n"));

    return STATUS_SUCCESS;
}

VOID
athUsbGenerateTxData(IN PATHUSB_API_STREAM_OBJECT   StreamObject,
                     IN PATHUSB_API_TRANSFER_OBJECT TransferObject,
                     IN BOOLEAN                     bRand)
{    
    ULONG    packetSize,i,loop, *pdwTemp;
    PUCHAR   pch;
    PULONG   pdwPtr;    

    packetSize = (StreamObject->outPipeInformation)->MaximumPacketSize;

    loop = TransferObject->dataBufferLen / packetSize;

    pch = TransferObject->dataBuffer;
    for (i = 0; i < loop; i ++) {        
        *((ULONG *)pch) = StreamObject->writeSequenceNum;        
        pch[4] = (UCHAR)i;
        pdwTemp = (ULONG *)&pch[5];
        *pdwTemp = (ULONG) TransferObject->dataBuffer;
        StreamObject->writeSequenceNum ++;

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
athUsbCompareRxData(IN PATHUSB_API_STREAM_OBJECT   StreamObject,
                    IN PATHUSB_API_TRANSFER_OBJECT TransferObject,
                    IN PATHUSB_STREAM_INFO         StreamInfo)
{
    ULONG    packetSize,i,loop, seqNum,dataNum;
    PUCHAR   pch,pTemp;
    BOOLEAN  bWrongSeq = FALSE;    

    packetSize = (StreamObject->inPipeInformation)->MaximumPacketSize;
    loop = TransferObject->dataBufferLen / packetSize;

    pch = TransferObject->dataBuffer;
    //for (i = 0; i < loop; i ++) {
    seqNum = *((ULONG *)pch);
    dataNum = pch[4];
    
    if (seqNum != StreamObject->readSequenceNum) {
         AthUsb_DbgPrint(3, ("Receive SeqNum = %d,    PacketNum = %d, Current SeqNum = %d\n",
                seqNum,dataNum, StreamObject->readSequenceNum));
//       bWrongSeq = TRUE;
         StreamInfo->ErrorSeqCount ++;
//         ASSERT(0);
        
    }
    
    StreamObject->readSequenceNum += loop;
//  pch += packetSize;    

    if (StreamObject->readPos != StreamObject->writePos) {        
        pch = &StreamObject->savedDataBuffer[StreamObject->readPos * TransferObject->dataBufferLen];
        pTemp = &StreamObject->tempDataBuffer[StreamObject->tempReadPos * TransferObject->dataBufferLen];
        
        while (StreamObject->tempReadPos != StreamObject->tempWritePos) {
            if (RtlEqualMemory(pch,pTemp,TransferObject->dataBufferLen) == FALSE) {
                StreamInfo->ErrorPacketCount ++;
                AthUsb_DbgPrint(3, ("Looptest failed\n"));
//                ASSERT (0);
            }

            StreamObject->tempReadPos ++;
            pTemp += TransferObject->dataBufferLen;
            if (StreamObject->tempReadPos >= ATHUSB_MAX_BUF_NUM) {
                StreamObject->tempReadPos = 0;        
                pTemp = StreamObject->tempDataBuffer;
            }

            StreamObject->readPos ++;
            pch += TransferObject->dataBufferLen;
            if (StreamObject->readPos >= ATHUSB_MAX_BUF_NUM) {
                StreamObject->readPos = 0;       
                pch = StreamObject->savedDataBuffer;
            }

            if (StreamObject->readPos == StreamObject->writePos) {
                break;
            }           
            
        }

        if (StreamObject->readPos == StreamObject->writePos) {
            RtlCopyMemory (&StreamObject->tempDataBuffer[StreamObject->tempWritePos * TransferObject->dataBufferLen],
                           TransferObject->dataBuffer,
                           TransferObject->dataBufferLen);

            StreamObject->tempWritePos ++;
            if (StreamObject->tempWritePos >= ATHUSB_MAX_BUF_NUM) {
                StreamObject->tempWritePos = 0;        
            }
        } else {
            if (RtlEqualMemory(pch,
                               TransferObject->dataBuffer,
                               TransferObject->dataBufferLen) == FALSE) 
            {
                StreamInfo->ErrorPacketCount ++;
                AthUsb_DbgPrint(3, ("Looptest failed\n"));
//                ASSERT (0);
            }
    
            StreamObject->readPos ++;
            if (StreamObject->readPos >= ATHUSB_MAX_BUF_NUM) {
                StreamObject->readPos = 0;        
            }
        }
    } else {
        RtlCopyMemory (&StreamObject->tempDataBuffer[StreamObject->tempWritePos * TransferObject->dataBufferLen],
                        TransferObject->dataBuffer,
                        TransferObject->dataBufferLen);

        StreamObject->tempWritePos ++;
        if (StreamObject->tempWritePos >= ATHUSB_MAX_BUF_NUM) {
            StreamObject->tempWritePos = 0;        
        }
    }
}

VOID
athUsbInsertTailList(IN PATHUSB_API_STREAM_OBJECT   StreamObject,
                     IN PATHUSB_API_TRANSFER_OBJECT TransferObject,
                     IN BOOLEAN                     bRead,
                     IN PLIST_ENTRY                 pQueue)
{
    KIRQL                   oldIrql;

    AthUsb_DbgPrint(3, ("AthUsb_QueuePendingIrp - begins\n"));

    oldIrql = KeGetCurrentIrql();

    if (bRead) {
        athAcquireSpinLock(&StreamObject->readQueueLock, &oldIrql);    
        InsertTailList(pQueue,&TransferObject->link);
        athReleaseSpinLock(&StreamObject->readQueueLock, oldIrql);
    } else {
        athAcquireSpinLock(&StreamObject->writeQueueLock, &oldIrql);    
        InsertTailList(pQueue,&TransferObject->link);
        athReleaseSpinLock(&StreamObject->writeQueueLock, oldIrql);
    }

    AthUsb_DbgPrint(3, ("AthUsb_QueuePendingIrp - ends\n"));
}

BOOLEAN 
athUsbRemoveHeadList(IN PATHUSB_API_STREAM_OBJECT       StreamObject,                 
                     IN OUT PATHUSB_API_TRANSFER_OBJECT *pTransferObject,
                     IN BOOLEAN                         bRead,
                     IN PLIST_ENTRY                     pQueue)
{
    KIRQL                       oldIrql;
    PATHUSB_API_TRANSFER_OBJECT transferObject;    
    PLIST_ENTRY                 listEntry;

    AthUsb_DbgPrint(3, ("AthUsb_GetNewTransferObj - begins\n"));

    (*pTransferObject) = NULL;

    oldIrql = KeGetCurrentIrql();

    if (bRead) {
        athAcquireSpinLock(&StreamObject->readQueueLock, &oldIrql);        
        if (IsListEmpty(pQueue)) {
            athReleaseSpinLock(&StreamObject->readQueueLock, oldIrql);
            return FALSE;
        }
        else {
            listEntry = RemoveHeadList(pQueue);        
            transferObject = CONTAINING_RECORD(listEntry, 
                                               ATHUSB_API_TRANSFER_OBJECT, link);   
            (*pTransferObject) = transferObject;
            athReleaseSpinLock(&StreamObject->readQueueLock, oldIrql);
        }
    } else {
        athAcquireSpinLock(&StreamObject->writeQueueLock, &oldIrql);        
        if (IsListEmpty(pQueue)) {
            athReleaseSpinLock(&StreamObject->writeQueueLock, oldIrql);
            return FALSE;
        }
        else {
            listEntry = RemoveHeadList(pQueue);        
            transferObject = CONTAINING_RECORD(listEntry, 
                                               ATHUSB_API_TRANSFER_OBJECT, link);
            (*pTransferObject) = transferObject;
            athReleaseSpinLock(&StreamObject->writeQueueLock, oldIrql);
        }
    }

    AthUsb_DbgPrint(3, ("AthUsb_GetNewTransferObj - ends\n"));

    return TRUE;
}

NTSTATUS
athUsbStopUsbStream(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PATHUSB_API_STREAM_OBJECT StreamObject,
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
       (StreamObject->deviceObject != DeviceObject)) {

        AthUsb_DbgPrint(1, ("invalid streamObject\n"));
        return STATUS_INVALID_PARAMETER;
    }

    StreamObject->bStop = TRUE;

    AthUsb_DbgPrint(2, ("totalTime = %d minTime = %d, maxTime = %d, totalCycle = %d\n",
                        StreamObject->totalTime,StreamObject->minTime,StreamObject->maxTime,
                        StreamObject->totalCycle));
    athUsbStreamApiObjectCleanup(StreamObject, deviceExtension);
    
    AthUsb_DbgPrint(2, ("AthUsb_StopUSbStream - ends\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
athUsbStreamApiObjectCleanup(
    IN PATHUSB_API_STREAM_OBJECT StreamObject,
    IN PDEVICE_EXTENSION         DeviceExtension
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
    AthUsb_DbgPrint(2, ("athUsbStopUsbStream - begins\n"));
    
    StreamObject->bCancel = TRUE;    

    if (KeRemoveQueueDpc(&StreamObject->streamDpc) == FALSE ||
        KeCancelTimer(&StreamObject->streamTimer) == FALSE) {
        KeWaitForSingleObject(&StreamObject->streamDpcEvent,
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);
    }

    athUsbDrvExit(0,StreamObject->pUsbAdapt);
    athUsbFreeBufferQueue(StreamObject);

    if (StreamObject->savedDataBuffer) {
        ExFreePool(StreamObject->savedDataBuffer);
    }

    if (StreamObject->tempDataBuffer) {
        ExFreePool(StreamObject->tempDataBuffer);
    }

    ExFreePool(StreamObject);

    DeviceExtension->StreamApiObject = NULL;

    AthUsb_DbgPrint(2, ("athUsbStopUsbStream - ends\n"));

    return STATUS_SUCCESS;
}

void
athUsbFreeBufferQueue(IN PATHUSB_API_STREAM_OBJECT StreamObject)
{
    KIRQL                       oldIrql;
    PATHUSB_API_TRANSFER_OBJECT xferObject;    
    PLIST_ENTRY                 listEntry;

    AthUsb_DbgPrint(3, ("athUsbFreeBufferQueue - begins\n"));

    oldIrql = KeGetCurrentIrql();

    athAcquireSpinLock(&StreamObject->readQueueLock, &oldIrql);        
    while(!IsListEmpty(&StreamObject->readIdleQueue)) {        
        listEntry = RemoveHeadList(&StreamObject->readIdleQueue);        
        xferObject = CONTAINING_RECORD(listEntry, ATHUSB_API_TRANSFER_OBJECT, link);           
        
        if (xferObject) {             

            if (xferObject->dataBuffer) {                
                ExFreePool(xferObject->dataBuffer);
                xferObject->dataBuffer = NULL;
            }            
                
            ExFreePool(xferObject);
        }
    }
    
    athReleaseSpinLock(&StreamObject->readQueueLock, oldIrql);    

    athAcquireSpinLock(&StreamObject->writeQueueLock, &oldIrql);        
    while(!IsListEmpty(&StreamObject->writeIdleQueue)) {        
        listEntry = RemoveHeadList(&StreamObject->writeIdleQueue);        
        xferObject = CONTAINING_RECORD(listEntry, ATHUSB_API_TRANSFER_OBJECT, link);   
       
        if (xferObject) {             
       
            if (xferObject->dataBuffer) {                
                ExFreePool(xferObject->dataBuffer);
                xferObject->dataBuffer = NULL;
            }            
                
            ExFreePool(xferObject);
        }
    }
    
    athReleaseSpinLock(&StreamObject->writeQueueLock, oldIrql);    

    AthUsb_DbgPrint(3, ("athUsbFreeBufferQueue - ends\n"));
}
