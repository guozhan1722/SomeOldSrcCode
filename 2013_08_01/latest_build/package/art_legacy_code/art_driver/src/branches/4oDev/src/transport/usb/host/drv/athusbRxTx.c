/*
 * Copyright (c) 2004 Atheros Communications, Inc., All Rights Reserved
 *
 * athusbrxtx.c -- Atheros USB Driver send and receive file
 * 
 */
#include "athusbapi.h"
#include "athusbdrv.h"
#include "athusbconfig.h"
#include "athusbRxTx.h"

/************************************************************************
* Routine Description:
*
*    This routine allocate ATHUSB_RXTX_OBJECT pool (as well as Irp/Urb pair).
*
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*
* Return Value:
*
*    NT status value
************************************************************************/
NTSTATUS
athUsbAllocIrpUrbPool(IN PATHUSB_USB_ADAPTER   pUsbAdapter)
{
    A_UINT32    i;
    NTSTATUS    ntStatus;

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbAllocIrpUrbPool - begins\n"));

    for (i = 0; i < ATHUSB_MAX_IRP_NUM; i++) {

        ntStatus = athUsbAllocTransferObj(pUsbAdapter,FALSE);

        if (!NT_SUCCESS(ntStatus)) {
            
            athUsbDbgPrint(ATHUSB_ERROR, ("athUsbAllocIrpUrbPool [%d] - failed\n", i));

            athUsbFreeIrpUrbPool(pUsbAdapter);
            return ntStatus;            
        }

        pUsbAdapter->totalWriteIrp ++;

        ntStatus = athUsbAllocTransferObj(pUsbAdapter,TRUE);

        if (!NT_SUCCESS(ntStatus)) {
            
            athUsbDbgPrint(ATHUSB_ERROR, ("athUsbAllocIrpUrbPool [%d] - failed\n", i));

            athUsbFreeIrpUrbPool(pUsbAdapter);                
            return ntStatus;
        }

        pUsbAdapter->totalReadIrp ++;
    }

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbAllocIrpUrbPool - ends\n"));
    return ntStatus;
}

/************************************************************************ 
* Routine Description:
*
*    This routine creates a transfer object for each irp/urb pair.
*
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    StreamObject - pointer to stream object
*
* Return Value:
*
*    NT status value
************************************************************************/
NTSTATUS
athUsbAllocTransferObj(IN PATHUSB_USB_ADAPTER   pUsbAdapter,
                       IN BOOLEAN               bRead)
{
    PIRP                    irp;
    CCHAR                   stackSize;
    A_UINT32                numPackets;
    PATHUSB_RXTX_OBJECT     pTransferObject;

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbAllocTransferObj - begins\n"));

    pTransferObject = AthExAllocatePool(NonPagedPool,
                                    sizeof(ATHUSB_RXTX_OBJECT));

    if (pTransferObject == NULL) {

        athUsbDbgPrint(ATHUSB_ERROR, ("Failed to alloc mem for transferObject\n"));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pTransferObject,sizeof(ATHUSB_RXTX_OBJECT));

    InitializeListHead(&pTransferObject->link);

    pTransferObject->pUsbAdapter = pUsbAdapter;    

    stackSize = pUsbAdapter->stackSize;
    irp = IoAllocateIrp(stackSize, FALSE);
    if (irp == NULL) {

        athUsbDbgPrint(ATHUSB_ERROR, ("failed to alloc mem for irp\n"));
        ExFreePool(pTransferObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pTransferObject->irp = irp;        
    
    pTransferObject->bRead = bRead;
    
    pTransferObject->urb = AthExAllocatePool(NonPagedPool,
                                   sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));

    if (pTransferObject->urb == NULL) {

        athUsbDbgPrint(ATHUSB_ERROR, ("failed to alloc mem for Urb\n"));        
        IoFreeIrp(irp);
        ExFreePool(pTransferObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }    

    if (bRead) {
        InsertTailList(&pUsbAdapter->readIdleIrpQueue,
                       &pTransferObject->link);
    } else {
        InsertTailList(&pUsbAdapter->writeIdleIrpQueue,
                                    &pTransferObject->link);
    }
    
    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbAllocTransferObj - ends\n"));

    return STATUS_SUCCESS;
}

/************************************************************************ 
* Routine Description:
*
*    Put the idle IRP into the queue.
*
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    pTransferObject - pointer to transfer object
*    bRead - Read/Write flag
*
* Return Value:
*
*    None
************************************************************************/
VOID
athUsbQueueIdleIrp(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                   IN PATHUSB_RXTX_OBJECT     pTransferObject,
                   IN BOOLEAN                 bRead)
{
    KIRQL                   oldIrql;

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbQueueIdleIrp - begins\n"));

    IoReuseIrp(pTransferObject->irp,STATUS_SUCCESS);

    oldIrql = KeGetCurrentIrql();

    if (bRead) {
        athAcquireSpinLock(&pUsbAdapter->readQueueLock, &oldIrql);        
        RemoveEntryList(&pTransferObject->link);
        InsertTailList(&pUsbAdapter->readIdleIrpQueue,&pTransferObject->link);
        athReleaseSpinLock(&pUsbAdapter->readQueueLock, oldIrql);
    } else {
        athAcquireSpinLock(&pUsbAdapter->writeQueueLock, &oldIrql);        
        RemoveEntryList(&pTransferObject->link);
        InsertTailList(&pUsbAdapter->writeIdleIrpQueue,&pTransferObject->link);
        athReleaseSpinLock(&pUsbAdapter->writeQueueLock, oldIrql);
    }

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbQueueIdleIrp - ends\n"));
}

/************************************************************************
* Routine Description:
*
*    Put the pending IRP into the queue.
*
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    pTransferObject - pointer to transfer object
*    bRead - Read/Write flag
*
* Return Value:
*
*    None
************************************************************************/
VOID
athUsbInsertPendingIrp(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                      IN PATHUSB_RXTX_OBJECT     pTransferObject,
                      IN BOOLEAN                 bRead)
{
    KIRQL                   oldIrql;

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbInsertPendingIrp - begins\n"));

    oldIrql = KeGetCurrentIrql();

    if (bRead) {
        athAcquireSpinLock(&pUsbAdapter->readQueueLock, &oldIrql);    
        InsertTailList(&pUsbAdapter->readPendingIrpQueue,&pTransferObject->link);
        athReleaseSpinLock(&pUsbAdapter->readQueueLock, oldIrql);
        InterlockedIncrement(&pUsbAdapter->readPendingIrps);
        InterlockedIncrement(&pUsbAdapter->pipeReadPending[pTransferObject->pipeNum]);
    } else {
        athAcquireSpinLock(&pUsbAdapter->writeQueueLock, &oldIrql);    
        InsertTailList(&pUsbAdapter->writePendingIrpQueue,&pTransferObject->link);
        athReleaseSpinLock(&pUsbAdapter->writeQueueLock, oldIrql);
        InterlockedIncrement(&pUsbAdapter->writePendingIrps);
        InterlockedIncrement(&pUsbAdapter->pipeWritePending[pTransferObject->pipeNum]);
    }

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbInsertPendingIrp - ends\n"));
}

/************************************************************************
* Routine Description:
*
*    Remove the Idle IRP from the queue.
*
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    pTransferObject - pointer to transfer object
*    bRead - Read/Write flag
*
* Return Value:
*
*    None
************************************************************************/
NTSTATUS
athUsbRemoveIdleIrp(IN PATHUSB_USB_ADAPTER      pUsbAdapter,
                    IN OUT PATHUSB_RXTX_OBJECT  *ppTransferObject,
                    IN BOOLEAN                  bRead)
{
    KIRQL                   oldIrql;
    PATHUSB_RXTX_OBJECT     pTransferObject;
    PLIST_ENTRY             listEntry;
    NTSTATUS                ntStatus = STATUS_SUCCESS;

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbRemoveIdleIrp - begins\n"));

    oldIrql = KeGetCurrentIrql();

    if (bRead) {
        athAcquireSpinLock(&pUsbAdapter->readQueueLock, &oldIrql); 
        
        if (IsListEmpty(&pUsbAdapter->readIdleIrpQueue)) {
            ntStatus = athUsbAllocTransferObj(pUsbAdapter,TRUE);
            if (!NT_SUCCESS(ntStatus)) {
                athReleaseSpinLock(&pUsbAdapter->readQueueLock, oldIrql);
                athUsbDbgPrint(ATHUSB_ERROR, ("athUsbAllocTransferObj - failed\n"));
                return ntStatus;
            }
            pUsbAdapter->totalReadIrp ++;
            athUsbDbgPrint(ATHUSB_INFO,("total Read IRP = %d\n",pUsbAdapter->totalReadIrp));
        }
        
        listEntry = RemoveHeadList(&pUsbAdapter->readIdleIrpQueue);        
        pTransferObject = CONTAINING_RECORD(listEntry, ATHUSB_RXTX_OBJECT, link);   

        (*ppTransferObject) = pTransferObject;

        athReleaseSpinLock(&pUsbAdapter->readQueueLock, oldIrql);
        
    } else {
        athAcquireSpinLock(&pUsbAdapter->writeQueueLock, &oldIrql);        
        
        if (IsListEmpty(&pUsbAdapter->writeIdleIrpQueue)) {
            ntStatus = athUsbAllocTransferObj(pUsbAdapter,FALSE);
            if (!NT_SUCCESS(ntStatus)) {
                athReleaseSpinLock(&pUsbAdapter->writeQueueLock, oldIrql);
                athUsbDbgPrint(ATHUSB_ERROR, ("athUsbAllocTransferObj - failed\n"));
                return ntStatus;
            }

            pUsbAdapter->totalWriteIrp ++;
            athUsbDbgPrint(ATHUSB_INFO,("total Write IRP = %d\n",pUsbAdapter->totalWriteIrp));
        }
        
        listEntry = RemoveHeadList(&pUsbAdapter->writeIdleIrpQueue);        
        pTransferObject = CONTAINING_RECORD(listEntry, ATHUSB_RXTX_OBJECT, link);   
        
        (*ppTransferObject) = pTransferObject;

        athReleaseSpinLock(&pUsbAdapter->writeQueueLock, oldIrql);        
    }

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbRemoveIdleIrp - ends\n"));

    return ntStatus;
}

/************************************************************************ 
* Routine Description:
*
*    This routine initializes the urb in the transfer object.
*
* Arguments:
*
*    DeviceObject - Pointer to usb adapter handle
*    pTransferObject - pointer to transfer object
*
* Return Value:
*
*    NT status value
*
************************************************************************/
NTSTATUS
athUsbInitTransferUrb(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                      IN PATHUSB_RXTX_OBJECT     pTransferObject)

{
    PURB                    urb;        
    PUSBD_PIPE_INFORMATION  pipeInformation;

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbInitTransferUrb - begins\n"));

    urb = pTransferObject->urb;
    pipeInformation = pTransferObject->pPipeInformation;

    (urb)->UrbHeader.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER; 
    (urb)->UrbHeader.Length = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
    (urb)->UrbBulkOrInterruptTransfer.PipeHandle = 
                                      pipeInformation->PipeHandle;

    (urb)->UrbBulkOrInterruptTransfer.TransferBufferLength = 
                                      pTransferObject->dataBufferLen;

    (urb)->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    (urb)->UrbBulkOrInterruptTransfer.TransferBuffer = 
                                      pTransferObject->pDataBuffer;

    if (pTransferObject->bRead) {
        (urb)->UrbBulkOrInterruptTransfer.TransferFlags = 
                      USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
    } else {
        (urb)->UrbBulkOrInterruptTransfer.TransferFlags = 
                                                    USBD_TRANSFER_DIRECTION_OUT;
    }

    (urb)->UrbBulkOrInterruptTransfer.UrbLink = NULL;

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbInitTransferUrb - ends\n"));

    return STATUS_SUCCESS;
}

/************************************************************************ 
* Routine Description:
*
*    This routine initializes the Irp in the transfer object.
*
* Arguments:
*
*    DeviceObject - Pointer to usb adapter handle
*    pTransferObject - pointer to transfer object
*
* Return Value:
*
*    NT status value
*
************************************************************************/
NTSTATUS
athUsbInitTransferIrp(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                      IN PATHUSB_RXTX_OBJECT     pTransferObject)
{
    PIRP                    irp;
    PIO_STACK_LOCATION      nextStack;

    irp = pTransferObject->irp;

    nextStack = IoGetNextIrpStackLocation(irp);
    nextStack->Parameters.Others.Argument1 = pTransferObject->urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                   IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    if (pTransferObject->bRead) {
        IoSetCompletionRoutine(irp, 
                           athUsbRxIrpComplete,
                           pTransferObject,
                           TRUE,
                           TRUE,
                           TRUE);
    } else {
        IoSetCompletionRoutine(irp, 
                           athUsbTxIrpComplete,
                           pTransferObject,
                           TRUE,
                           TRUE,
                           TRUE);
    }

    return STATUS_SUCCESS;
}

/************************************************************************
* Routine Description:
*
*    This routine free ATHUSB_RXTX_OBJECT pool (as well as Irp/Urb pair).
*
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*
* Return Value:
*
*    none
************************************************************************/
VOID
athUsbFreeIrpUrbPool(IN PATHUSB_USB_ADAPTER     pUsbAdapter)
{
    KIRQL                   oldIrql;
    PATHUSB_RXTX_OBJECT     xferObject;    
    PLIST_ENTRY             listEntry;

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbFreeIrpUrbPool - begins\n"));

    oldIrql = KeGetCurrentIrql();

    athAcquireSpinLock(&pUsbAdapter->readQueueLock, &oldIrql);
    
    while (!IsListEmpty(&pUsbAdapter->readIdleIrpQueue)) {        
        listEntry = RemoveHeadList(&pUsbAdapter->readIdleIrpQueue);        
        xferObject = CONTAINING_RECORD(listEntry, ATHUSB_RXTX_OBJECT, link);           
      
        if (xferObject) { 
            
            if (xferObject->urb) {
                ExFreePool(xferObject->urb);
                xferObject->urb = NULL;
            }
            
            if (xferObject->irp) {
                athReleaseSpinLock(&pUsbAdapter->readQueueLock, oldIrql);
                IoFreeIrp(xferObject->irp);
                athAcquireSpinLock(&pUsbAdapter->readQueueLock, &oldIrql);
            }
                
            ExFreePool(xferObject);
        }
    }
    
    athReleaseSpinLock(&pUsbAdapter->readQueueLock, oldIrql);

    athAcquireSpinLock(&pUsbAdapter->writeQueueLock, &oldIrql);

    while (!IsListEmpty(&pUsbAdapter->writeIdleIrpQueue)) {        
        listEntry = RemoveHeadList(&pUsbAdapter->writeIdleIrpQueue);        
        xferObject = CONTAINING_RECORD(listEntry, ATHUSB_RXTX_OBJECT, link);           
    
        if (xferObject) { 
            
            if (xferObject->urb) {
                ExFreePool(xferObject->urb);
                xferObject->urb = NULL;
            }            

            if (xferObject->irp) {
                athReleaseSpinLock(&pUsbAdapter->writeQueueLock, oldIrql);
                IoFreeIrp(xferObject->irp);
                athAcquireSpinLock(&pUsbAdapter->writeQueueLock, &oldIrql);
            }
                
            ExFreePool(xferObject);
        }
    }
    
    athReleaseSpinLock(&pUsbAdapter->writeQueueLock, oldIrql);

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbFreeIrpUrbPool - ends\n"));
}

/************************************************************************
* Routine Description:
*
*    This routine cancel all transaction pending IRP.
*
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    bAbort      - abort pipe or not
*
* Return Value:
*
*    none
************************************************************************/
VOID
athUsbCancelAllIrp(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                IN A_BOOL                     bAbort)
{
    KIRQL                   oldIrql;
    PATHUSB_RXTX_OBJECT     pTransferObject;    
    PLIST_ENTRY             listEntry;
    ULONG                   numIrps;

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbCancelAllIrp - begins\n"));

    pUsbAdapter->bCancel = TRUE;

    oldIrql = KeGetCurrentIrql();

    athAcquireSpinLock(&pUsbAdapter->readQueueLock, &oldIrql);

    if (pUsbAdapter->readPendingIrps != 0) {
        KeClearEvent(&pUsbAdapter->noPendingReadIrpEvent);
    }

    numIrps = 0;
    if (bAbort) {
        while (!IsListEmpty(&pUsbAdapter->readPendingIrpQueue)) {
            listEntry = RemoveHeadList(&pUsbAdapter->readPendingIrpQueue);
            pTransferObject = CONTAINING_RECORD(listEntry, ATHUSB_RXTX_OBJECT, link);
            listEntry->Flink = listEntry->Blink = listEntry;

            //
            // cancel transferobject irps/urb pair
            // safe to touch these irps because the 
            // completion routine always returns 
            // STATUS_MORE_PRCESSING_REQUIRED
            // 
            //
            if (pTransferObject && pTransferObject->irp) {
                athReleaseSpinLock(&pUsbAdapter->readQueueLock, oldIrql);
                IoCancelIrp(pTransferObject->irp);
                athAcquireSpinLock(&pUsbAdapter->readQueueLock, &oldIrql);
                athUsbDbgPrint(ATHUSB_LOUD, ("Cancel Read IRP %x\n",pTransferObject->irp));
            }
            numIrps ++;
            
        }   
    }

    athUsbDbgPrint(ATHUSB_LOUD, ("Total %d Read IRPs cancel\n",numIrps));

    athReleaseSpinLock(&pUsbAdapter->readQueueLock, oldIrql);

    athAcquireSpinLock(&pUsbAdapter->writeQueueLock, &oldIrql);

    if (pUsbAdapter->writePendingIrps != 0) {
        KeClearEvent(&pUsbAdapter->noPendingWriteIrpEvent);
    }

    numIrps = 0;
    if (bAbort) {
        while (!IsListEmpty(&pUsbAdapter->writePendingIrpQueue)) {        
            listEntry = RemoveHeadList(&pUsbAdapter->writePendingIrpQueue);         
            pTransferObject = CONTAINING_RECORD(listEntry, ATHUSB_RXTX_OBJECT, link);
            listEntry->Flink = listEntry->Blink = listEntry;
        
            //
            // cancel transferobject irps/urb pair
            // safe to touch these irps because the 
            // completion routine always returns 
            // STATUS_MORE_PRCESSING_REQUIRED
            // 
            //
            if (pTransferObject && pTransferObject->irp) {
                athReleaseSpinLock(&pUsbAdapter->writeQueueLock, oldIrql);
                IoCancelIrp(pTransferObject->irp);
                athAcquireSpinLock(&pUsbAdapter->writeQueueLock, &oldIrql);
                athUsbDbgPrint(ATHUSB_LOUD, ("Cancel Write IRP %x\n",pTransferObject->irp));
            }
            
            numIrps ++;
        }   
    }

    athUsbDbgPrint(ATHUSB_LOUD, ("Total %d Write IRPs cancel\n",numIrps));
    athReleaseSpinLock(&pUsbAdapter->writeQueueLock, oldIrql);

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbCancelAllIrp - ends\n"));
}

/************************************************************************
* Routine Description:
*
*    This routine cancel all transaction pending IRP.
*
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    bAbort      - abort pipe or not
*
* Return Value:
*
*    none
************************************************************************/
VOID
athUsbCancelIrp(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                IN A_UINT8                 epNum,
                IN A_BOOL                  bRead)
{
    KIRQL                   oldIrql;
    PATHUSB_RXTX_OBJECT     pTransferObject;    
    PLIST_ENTRY             listEntry,tempEntry,pQueue;
    PATHUSB_SPIN_LOCK       pAthLock;

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbCancelIrp - begins\n"));    

    if (bRead) {
        if (pUsbAdapter->pipeReadPending[epNum]) {
            KeClearEvent(&pUsbAdapter->noPendingPipeReadEvent[epNum]);
        }

        pQueue = &pUsbAdapter->readPendingIrpQueue;
        pAthLock = &pUsbAdapter->readQueueLock;
    } else {
        if (pUsbAdapter->pipeWritePending[epNum]) {
            KeClearEvent(&pUsbAdapter->noPendingPipeWriteEvent[epNum]);
        }

        pQueue = &pUsbAdapter->writePendingIrpQueue;
        pAthLock = &pUsbAdapter->writeQueueLock;
    }

    oldIrql = KeGetCurrentIrql();
    athAcquireSpinLock(pAthLock ,&oldIrql);

    listEntry = pQueue->Flink;
    while (listEntry != pQueue) {

        pTransferObject = CONTAINING_RECORD(listEntry, ATHUSB_RXTX_OBJECT, link);

        if (pTransferObject && (pTransferObject->bRead == bRead)
            && (pTransferObject->pipeNum == epNum))
        {
            tempEntry = listEntry->Flink;  

            RemoveEntryList(listEntry);

            listEntry->Flink = listEntry->Blink = listEntry;
            listEntry = tempEntry;

            //
            // cancel transferobject irps/urb pair
            // safe to touch these irps because the
            // completion routine always returns
            // STATUS_MORE_PRCESSING_REQUIRED
            // 
            //
            if (pTransferObject->irp) {
                athReleaseSpinLock(pAthLock, oldIrql);
                IoCancelIrp(pTransferObject->irp);
                athAcquireSpinLock(pAthLock ,&oldIrql);
            }
        } else {
            listEntry = listEntry->Flink;
        }
    }

    athReleaseSpinLock(pAthLock, oldIrql);

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbCancelIrp - ends\n"));
}
/************************************************************************ 
* Routine Description:
*
*    This routine is invoked from the completion routine to check the status
*    of the irp, urb and the bulk packets.
*
*    updates statistics
*
* Arguments:
*
*    TranferObject - pointer to transfer object for the irp/urb pair 
*                    which completed.
*
* Return Value:
*
*    NT status value
************************************************************************/
NTSTATUS
athUsbProcessTransfer(IN PATHUSB_RXTX_OBJECT     pTransferObject,
                      IN OUT USBD_STATUS         *pUsbdStatus)

{
    PIRP        irp;
    PURB        urb;    
    NTSTATUS    ntStatus;
    USBD_STATUS usbdStatus;

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbProcessTransfer - begins\n"));

    irp = pTransferObject->irp;
    urb = pTransferObject->urb;
    ntStatus = irp->IoStatus.Status;

    if (!NT_SUCCESS(ntStatus)) {
        athUsbDbgPrint(ATHUSB_ERROR, 
                      ("Bulk irp failed with status = %X, irp =%x\n", 
                      ntStatus,irp))
//        ASSERT (0);
    }

    usbdStatus = urb->UrbHeader.Status;

    if (!USBD_SUCCESS(usbdStatus)) {
        athUsbDbgPrint(ATHUSB_ERROR, 
                       ("urb failed with status = %X, pData = %x\n", 
                       usbdStatus,pTransferObject->pDataBuffer));
//        ASSERT (0);
        if (NT_SUCCESS(ntStatus)) {
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }

    *pUsbdStatus = usbdStatus;
    
    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbProcessTransfer - ends\n"));

    return ntStatus;
}

/************************************************************************
* Routine Description:
*
*    This routine will queue a work item to do pipe reset to clear errors
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*
* Return Value:
*
*    NT status value
************************************************************************/
NTSTATUS
athUsbQueueWorkItem(IN PATHUSB_USB_ADAPTER     pUsbAdapter)
{
    PIO_WORKITEM                    item;
    PATHUSB_WORKER_THREAD_CONTEXT   context;
    ULONG                           portStatus;
    NTSTATUS                        ntStatus;
    LARGE_INTEGER                   timeout;
    
    if (!pUsbAdapter->bReset && pUsbAdapter->bStop) {
        //device already stop/remove, so just return
        return STATUS_SUCCESS;
    }    

    if (pUsbAdapter->bQueueWorkItem) {
        //already queue one work item, so just return
        return STATUS_SUCCESS;
    }

    athUsbDbgPrint(ATHUSB_INFO, ("athsUsbQueueWorkItem - begins\n"));

    context = AthExAllocatePool(NonPagedPool, sizeof(ATHUSB_WORKER_THREAD_CONTEXT));
    if (NULL == context) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Note - Windows 98 doesn't support IoWorkItem's, it only supports the
    //        ExWorkItem version. We use the Io versions here because the
    //        Ex versions can cause system crashes during unload (ie the driver
    //        can unload in the middle of an Ex work item callback)
    //
    item = IoAllocateWorkItem(pUsbAdapter->pFunctionalDeviceObject);

    if (NULL == item) {

        ExFreePool(context);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    context->pUsbAdapter = pUsbAdapter;    
    context->workItem = item;    

    pUsbAdapter->bQueueWorkItem = TRUE;

    KeClearEvent(&pUsbAdapter->noWorkItemPendingEvent);

    IoQueueWorkItem(
        item,
        athUsbWorkerItemCallback,
        DelayedWorkQueue,
        context
        );

    athUsbDbgPrint(ATHUSB_INFO, ("athsUsbQueueWorkItem - ends\n"));

    return STATUS_SUCCESS;
}

/************************************************************************
* Routine Description:
*
*    Work item callback routine, this routine will be called in the 
*    context of a system thread.
*
* Arguments:
*
*    DeviceObject - pointer to device object
*    Context      - work item context
*
* Return Value:
*
*    NT status value
************************************************************************/
VOID
athUsbWorkerItemCallback(IN  PDEVICE_OBJECT  DeviceObject,
                         IN  PVOID           Context)
{
    PATHUSB_WORKER_THREAD_CONTEXT  context = 
                                        (PATHUSB_WORKER_THREAD_CONTEXT)Context;    
    PATHUSB_USB_ADAPTER            pUsbAdapter = context->pUsbAdapter;
    NTSTATUS                       ntStatus;   
    ULONG                          portStatus;
    A_UINT8                        i;

    athUsbDbgPrint(ATHUSB_INFO, ("athUsbWorkerItemCallback - begins\n"));

    pUsbAdapter->bQueueWorkItem = FALSE;

    if (!pUsbAdapter->bReset && pUsbAdapter->bStop) {
        //Device is gone!
        IoFreeWorkItem(context->workItem);
        ExFreePool((PVOID)context);
        KeSetEvent(&pUsbAdapter->noWorkItemPendingEvent,
                    IO_NO_INCREMENT,
                    FALSE);
        athUsbDbgPrint(ATHUSB_INFO, ("athUsbWorkerItemCallback - ends\n"));
        return;
    }

    ntStatus = athUsbGetPortStatus(pUsbAdapter, &portStatus);
    if (!(NT_SUCCESS(ntStatus)) || 
        ((portStatus & USBD_PORT_ENABLED) != USBD_PORT_ENABLED) ||
        ((portStatus & USBD_PORT_CONNECTED) != USBD_PORT_CONNECTED))
    {
        //Device is gone!
        IoFreeWorkItem(context->workItem);
        ExFreePool((PVOID)context);
        athUsbDbgPrint(ATHUSB_INFO, ("athUsbWorkerItemCallback - ends\n"));
        KeSetEvent(&pUsbAdapter->noWorkItemPendingEvent,
                    IO_NO_INCREMENT,
                    FALSE);
        pUsbAdapter->bReset = FALSE;
        return;
    }

    if (pUsbAdapter->bReset) {
        ntStatus = athUsbResetDevice(pUsbAdapter);
        pUsbAdapter->bReset = FALSE;
    } else if (pUsbAdapter->bDeviceReset) {
        //cancel and free all revious IRP, you can't send the same IRP down
        //if this IRP failed before.
        athUsbCancelAllIrp(pUsbAdapter, TRUE);
        //
        // wait for receive irps to complete.
        //
        KeWaitForSingleObject(&pUsbAdapter->noPendingReadIrpEvent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        //
        // wait for transmit irps to complete.
        //
        KeWaitForSingleObject(&pUsbAdapter->noPendingWriteIrpEvent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);    
         
        ntStatus = athUsbVendorReset(pUsbAdapter);

        if (NT_SUCCESS(ntStatus)) {
            ntStatus = athUsbResetAllPipes(pUsbAdapter);
        }

        pUsbAdapter->bDeviceReset = FALSE;
    } else {
        PUSBD_PIPE_INFORMATION       pipeInfo;
        A_UINT8                      pipeNum;
        NTSTATUS                     ntStatus;

        for (i = 0; i < MAX_PIPE_NUM; i ++) {

            if (pUsbAdapter->bResetPipe[0][i]) {

                athUsbCancelIrp(pUsbAdapter,i,TRUE);

                //
                // wait for receive irps to complete.
                //
                KeWaitForSingleObject(&pUsbAdapter->noPendingPipeReadEvent[i],
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);

                pipeNum = pUsbAdapter->recvEndPoints[i];
                pipeInfo = &((pUsbAdapter->pUsbInterface)->Pipes[pipeNum]);

                ntStatus = athUsbResetPipe(pUsbAdapter,pipeInfo);
    
                if (!NT_SUCCESS(ntStatus)) {
            
                    athUsbDbgPrint(ATHUSB_ERROR, ("athUsbResetPipe failed\n"));

                    ntStatus = athUsbResetDevice(pUsbAdapter);
                }

                pUsbAdapter->bResetPipe[0][i] = FALSE;
            }

            if (pUsbAdapter->bResetPipe[1][i]) {
                athUsbCancelIrp(pUsbAdapter,i,FALSE);

                //
                // wait for receive irps to complete.
                //
                KeWaitForSingleObject(&pUsbAdapter->noPendingPipeWriteEvent[i],
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
                pipeNum = pUsbAdapter->sendEndPoints[i];
                pipeInfo = &((pUsbAdapter->pUsbInterface)->Pipes[pipeNum]);

                ntStatus = athUsbResetPipe(pUsbAdapter,pipeInfo);
    
                if (!NT_SUCCESS(ntStatus)) {
            
                    athUsbDbgPrint(ATHUSB_ERROR, ("athUsbResetPipe failed\n"));

                    ntStatus = athUsbResetDevice(pUsbAdapter);
                }

                pUsbAdapter->bResetPipe[1][i] = FALSE;
            }
        }
    }

    if (NT_SUCCESS(ntStatus)) {
        pUsbAdapter->statIndHandler((VOID *)pUsbAdapter->pStubHandle,ATHUSB_BUS_STATE_NORMAL);
    }

    pUsbAdapter->bStop = FALSE;
    pUsbAdapter->bCancel = FALSE;
    pUsbAdapter->busState = ATHUSB_BUS_STATE_NORMAL;
    //
    // Cleanup before exiting from the worker thread.
    //
    IoFreeWorkItem(context->workItem);
    ExFreePool((PVOID)context);

    KeSetEvent(&pUsbAdapter->noWorkItemPendingEvent,
               IO_NO_INCREMENT,
               FALSE);
    athUsbDbgPrint(ATHUSB_INFO, ("athUsbWorkerItemCallback - ends\n"));
}

/************************************************************************ 
*Routine Description:
*
*    Send all waiting IRP to the bus driver
*
*Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*
*Return Value:
*
*    None
************************************************************************/
VOID
athUsbDrainWaitQueue(IN PATHUSB_USB_ADAPTER     pUsbAdapter)
{    
    PATHUSB_RXTX_OBJECT             pTransferObject;
    PLIST_ENTRY                     listEntry;
    NTSTATUS                        ntStatus;

    while (!IsListEmpty(&pUsbAdapter->writeWaitingQueue)) {
        listEntry = ExInterlockedRemoveHeadList(&pUsbAdapter->writeWaitingQueue,
                                                &(pUsbAdapter->writeQueueLock.spinLock));
        pTransferObject = CONTAINING_RECORD(listEntry,ATHUSB_RXTX_OBJECT, link);
        athUsbInsertPendingIrp(pUsbAdapter,pTransferObject,FALSE);
        ntStatus = IoCallDriver(pUsbAdapter->pNextDeviceObject,pTransferObject->irp);
        if(!NT_SUCCESS(ntStatus)) {
            return;
        }
    }

    while (!IsListEmpty(&pUsbAdapter->readWaitingQueue)) {
        listEntry = ExInterlockedRemoveHeadList(&pUsbAdapter->readWaitingQueue,
                                                &(pUsbAdapter->readQueueLock.spinLock));
        pTransferObject = CONTAINING_RECORD(listEntry,ATHUSB_RXTX_OBJECT, link);
        athUsbInsertPendingIrp(pUsbAdapter,pTransferObject,TRUE);
        ntStatus = IoCallDriver(pUsbAdapter->pNextDeviceObject,pTransferObject->irp);
        if(!NT_SUCCESS(ntStatus)) {            
            return;
        }
    }
}

/************************************************************************ 
* Routine Description:
*
*    DPC routine triggered by the timer to schdule work item 
*    which will do pipe reset
*
* Arguments:
*
*    DeferredContext - context for the dpc routine.
*                      ATHUSB_USB_ADAPTER in our case.
*
* Return Value:
*
*    None
************************************************************************/
VOID
athUsbDpcProc(
              IN   PKDPC    dpc,
              IN   PVOID    deferredContex,
              IN   PVOID    systemContex1,
              IN   PVOID    systemContex2)
{
    PATHUSB_USB_ADAPTER             pUsbAdapter;
    LARGE_INTEGER                   newTime;
    ULONG                           timeDiff;

    athUsbDbgPrint(ATHUSB_INFO, ("DpcRoutine - begins\n"));

    pUsbAdapter = (PATHUSB_USB_ADAPTER)deferredContex;

    if (pUsbAdapter->bStop || pUsbAdapter->bCancel) {
        athUsbDbgPrint(ATHUSB_INFO, ("DpcRoutine - ends\n"));
        return;
    }

    KeClearEvent(&pUsbAdapter->usbAdptDpcEvent);

    pUsbAdapter->bQueueTimer = FALSE;    

    /*if (!pUsbAdapter->bCount) {
        getPentiumCycleCounter(&pUsbAdapter->oldTime);
        pUsbAdapter->bCount = TRUE;
    } else {
        getPentiumCycleCounter(&newTime);
        timeDiff = (ULONG)((newTime.QuadPart - pUsbAdapter->oldTime.QuadPart) /1331);

        if (pUsbAdapter->minTime > timeDiff) {
            pUsbAdapter->minTime = timeDiff;
        }       

        if (pUsbAdapter->maxTime < timeDiff) {
            pUsbAdapter->maxTime = timeDiff;
        }
        pUsbAdapter->totalCycle ++;
        pUsbAdapter->totalTime += timeDiff;
        pUsbAdapter->bCount = FALSE;
    }*/
#ifdef DEBUG
    getPentiumCycleCounter(&pUsbAdapter->oldTime);
#endif

    athUsbDrainWaitQueue(pUsbAdapter);

#ifdef DEBUG    
    getPentiumCycleCounter(&newTime);
    timeDiff = (ULONG)((newTime.QuadPart - pUsbAdapter->oldTime.QuadPart) /1331);

    if (pUsbAdapter->minTime > timeDiff) {
        pUsbAdapter->minTime = timeDiff;
    }       

    if (pUsbAdapter->maxTime < timeDiff) {
        pUsbAdapter->maxTime = timeDiff;
    }
    pUsbAdapter->totalCycle ++;
    pUsbAdapter->totalTime += timeDiff;
#endif

    KeSetEvent(&pUsbAdapter->usbAdptDpcEvent,
               IO_NO_INCREMENT,
               FALSE);

    athUsbDbgPrint(ATHUSB_INFO, ("DpcRoutine - ends\n"));
}
