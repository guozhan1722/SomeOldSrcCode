/*
 * Copyright (c) 2004 Atheros Communications, Inc., All Rights Reserved
 *
 * athusbdrv.c -- Atheros USB Driver API file
 * 
 */

#include "athusbapi.h"
#include "athusbdrv.h"
#include "athusbconfig.h"
#include "athusbRxTx.h"

//#define RESET_PIPE_ONLY_FOR_ERROR

A_UINT32   usbDebugLevel = ATHUSB_INFO;
A_UINT32   gepBufSize[2][10] = {512,2048,0,0,0,0,0,0,0,0,
                             512,2048,0,0,0,0,0,0,0,0};
/*****************************************************************************
* Routine Description:
*
*    This routine will initialize WDM lower edge. It must call at PASSIVE_LEVEL
*    
* Arguments:
*
*   usbDeviceNum   - usb device number. We may support mulitple usb device
*   pMiniportAdaptHandle - Specifies the NDIS handle that identifies the miniport 
*                    driver's NIC. This handle was originally passed to 
*                    MiniportInitialize. 
*   pStubHandle    - stub handle
*   recvIndHandler - receive call back function handle
*   sendCfmHandler - transmit complete notify function handle
*   statIndHandler - status notify function handle
*   ppUsbDrvHandle - Pointer to a caller-allocated buffer, this buffer receive
*                    usb adpater handle.
*   pSendEndPoints - Pointer to a caller-allocated buffer, this buffer receive
*                    all send endpoint number,
*   pRecvEndPoints - Pointer to a caller-allocated buffer, this buffer receive
*                    all send endpoint number
*        
*    
* Return Value:
*
*    STATUS_SUCCESS if the device can be safely removed, an appropriate 
*    NT Status if not.
*******************************************************************************/
A_STATUS 
athUsbDrvInit( 
    IN  A_UINT8                    usbDeviceNum,
    IN  void                      *pMiniportAdaptHandle,
    IN  void                      *pStubHandle,
    IN  RECV_INDICATION_HANDLER    recvIndHandler,
    IN  SEND_CONFIRM_HANDLER       sendCfmHandler,
    IN  STATUS_INDICATION_HANDLER  statIndHandler,
    OUT PATHUSBDRV_HANDLE         *ppUsbDrvHandle,
    OUT A_UINT8                   *pRecvEndPoints,
    OUT A_UINT8                   *pSendEndPoints    
    )
{
    PATHUSB_USB_ADAPTER             pUsbAdapter;
    PUSBD_INTERFACE_INFORMATION     pInterfaceInfo;
    PUSBD_PIPE_INFORMATION          pPipeInformation;
    A_UINT8                         i, sendEPNum,recvEPNum;
    NTSTATUS                        ntStatus;    

    PAGED_CODE();

    ASSERT(pMiniportAdaptHandle);
    ASSERT(pStubHandle);
    ASSERT(recvIndHandler);
    ASSERT(sendCfmHandler);
    ASSERT(statIndHandler);

    *ppUsbDrvHandle = NULL;

    pUsbAdapter = (PATHUSB_USB_ADAPTER)
                   AthExAllocatePool(NonPagedPool,sizeof(ATHUSB_USB_ADAPTER));

    if (pUsbAdapter == NULL) {
        return A_NO_MEMORY;
    }

    RtlZeroMemory(pUsbAdapter,sizeof(ATHUSB_USB_ADAPTER));

    pUsbAdapter->usbDeviceNum = usbDeviceNum;

#ifdef CHIP_TEST
    //The following two lines are only for testing purpose    
    pUsbAdapter->pNextDeviceObject = (PDEVICE_OBJECT)pMiniportAdaptHandle;
    pUsbAdapter->pFunctionalDeviceObject =  (PDEVICE_OBJECT)pStubHandle;
#else
    //Retrieves device objects required to set up communication 
    //with a miniport driver through a bus driver
    NdisMGetDeviceProperty(
        (NDIS_HANDLE)pMiniportAdaptHandle,
		&pUsbAdapter->pPhysicalDeviceObject,
		&pUsbAdapter->pFunctionalDeviceObject,
		&pUsbAdapter->pNextDeviceObject,
		NULL,
		NULL
	);
#endif

    pUsbAdapter->pStubHandle = (VOID *)pStubHandle;
    
    //Save all callback handle
    pUsbAdapter->sendCfmHandler = sendCfmHandler;
    pUsbAdapter->recvIndHandler = recvIndHandler;
    pUsbAdapter->statIndHandler = statIndHandler;
    
    //
    // Read the device descriptor, configuration descriptor 
    // and select the interface descriptors
    //

    ntStatus = athUsbReadandSelectDescriptors(pUsbAdapter);

    if (!NT_SUCCESS(ntStatus)) {

        athUsbDbgPrint(ATHUSB_ERROR, ("athUsbReadandSelectDescriptors failed\n"));
        ExFreePool(pUsbAdapter);
        return A_DEVICE_NOT_FOUND;
    }

    //Get stack size
    pUsbAdapter->stackSize = (CCHAR) 
                             (pUsbAdapter->pNextDeviceObject->StackSize + 1);

    //Initialize variable
    KeInitializeSpinLock(&(pUsbAdapter->readQueueLock.spinLock));
    pUsbAdapter->readQueueLock.num = 0;
    pUsbAdapter->readQueueLock.label = 0;
    InitializeListHead(&pUsbAdapter->readIdleIrpQueue);
    InitializeListHead(&pUsbAdapter->readWaitingQueue);
    InitializeListHead(&pUsbAdapter->readPendingIrpQueue);

    KeInitializeEvent(&pUsbAdapter->noPendingReadIrpEvent,
                      NotificationEvent,
                      TRUE);

    //
    // Initialize the noWorkItemPendingEvent to signaled state.
    // This event is cleared when we try to schedule work item
    // and signaled on completion of the work-item.
    //
    KeInitializeEvent(&pUsbAdapter->noWorkItemPendingEvent,
                      NotificationEvent,
                      TRUE);

    KeInitializeSpinLock(&(pUsbAdapter->writeQueueLock.spinLock));
    pUsbAdapter->writeQueueLock.label = 1;
    pUsbAdapter->writeQueueLock.num = 0;
    InitializeListHead(&pUsbAdapter->writeWaitingQueue);
    InitializeListHead(&pUsbAdapter->writeIdleIrpQueue);
    InitializeListHead(&pUsbAdapter->writePendingIrpQueue);

    KeInitializeEvent(&pUsbAdapter->noPendingWriteIrpEvent,
                      NotificationEvent,
                      TRUE);    
    
    for (i = 0; i < MAX_PIPE_NUM; i ++) {
        KeInitializeEvent(&pUsbAdapter->noPendingPipeReadEvent[i],
                          NotificationEvent,
                          TRUE);

        KeInitializeEvent(&pUsbAdapter->noPendingPipeWriteEvent[i],
                          NotificationEvent,
                          TRUE);
    }

    KeInitializeDpc(&pUsbAdapter->usbAdptDpc,athUsbDpcProc,pUsbAdapter);
    KeInitializeTimer(&pUsbAdapter->usbAdptTimer);
    KeInitializeEvent(&pUsbAdapter->usbAdptDpcEvent,NotificationEvent,TRUE);

    athUsbGetAllStringDesciptor(pUsbAdapter);

    pInterfaceInfo = pUsbAdapter->pUsbInterface;

    ntStatus = athUsbAllocIrpUrbPool(pUsbAdapter);
    if (!NT_SUCCESS(ntStatus)) {

        athUsbDbgPrint(ATHUSB_ERROR, ("athUsbAllocIrpUrbPool failed\n"));
        athUsbDrvExit(usbDeviceNum,pUsbAdapter);
        return A_NO_MEMORY;
    }

    sendEPNum = 0;
    recvEPNum = 0;

    //We should do vendor reset first to allow device switch back to data0
    athUsbVendorReset(pUsbAdapter);
    athUsbResetAllPipes(pUsbAdapter);

    for (i = 0; i < pInterfaceInfo->NumberOfPipes; i ++) {
        pPipeInformation = &pInterfaceInfo->Pipes[i];
        if (UsbdPipeTypeBulk == pPipeInformation->PipeType) {
            if (USB_ENDPOINT_DIRECTION_IN(pPipeInformation->EndpointAddress)) {
                pUsbAdapter->recvEndPoints[recvEPNum] = i;
                recvEPNum ++;
            } else {
                pUsbAdapter->sendEndPoints[sendEPNum] = i;
                sendEPNum ++;
            }
        }
    }

    *pSendEndPoints = sendEPNum;
    *pRecvEndPoints = recvEPNum;
    *ppUsbDrvHandle = (PATHUSBDRV_HANDLE)pUsbAdapter;

    pUsbAdapter->bStop = FALSE;
    pUsbAdapter->bUseTimer = FALSE;
    //pUsbAdapter->minTime = 0xFFFFFFFF;

    return A_OK;
}

/*****************************************************************************
* Routine Description:
*
*    Notify WDM lower edge that device will quit/remove/stop after this point,
*    It must call at PASSIVE_LEVEL
*    
* Arguments:
*
*   usbDeviceNum   - usb device number. We may support mulitple usb device
*   pAppHandle     - Specifies the NDIS handle that identifies the miniport 
*                driver's NIC. This handle was originally passed to 
*                MiniportInitialize. 
*   pUsbDrvHandle  - Pointer to usb adapter handle
*        
*    
* Return Value:
*
*    STATUS_SUCCESS if the device can be safely removed, an appropriate 
*    NT Status if not.
*******************************************************************************/
A_STATUS 
athUsbDrvExit( 
    IN  A_UINT8                  usbDeviceNum,        
    IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle   
    )
{
    PATHUSB_USB_ADAPTER             pUsbAdapter;
    NTSTATUS                        ntStatus;
    LARGE_INTEGER                   timeOut;

    PAGED_CODE();

    pUsbAdapter = (PATHUSB_USB_ADAPTER)pUsbDrvHandle;
    
    ASSERT(pUsbAdapter);
    ASSERT(pUsbAdapter->usbDeviceNum == usbDeviceNum);

    pUsbAdapter->bStop = TRUE;

    if (pUsbAdapter->bUseTimer) {
        athUsbDrainWaitQueue(pUsbAdapter);
    }

    athUsbCancelAllIrp(pUsbAdapter, TRUE);

    // KeCancelTimer returns false, it means the DPC has been either
    // delivered or in the process of being delivered 

    if (KeCancelTimer(&pUsbAdapter->usbAdptTimer) == FALSE) {
        KeWaitForSingleObject(&pUsbAdapter->usbAdptDpcEvent,
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);
    }

    //
    // make sure that if a reset work item already schedule,
    // then the work-time have run to their completion
    //
    KeWaitForSingleObject(&pUsbAdapter->noWorkItemPendingEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    //
    // wait for receive irps to complete.
    //
    timeOut.QuadPart = -100000000;
    if (KeWaitForSingleObject(&pUsbAdapter->noPendingReadIrpEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          &timeOut) == STATUS_TIMEOUT)
    {
        athUsbDbgPrint(ATHUSB_ERROR,("Wait for pendingReadIrpEvent timeOut, %d\n",pUsbAdapter->readPendingIrps));
    }
    
    //
    // wait for transmit irps to complete.
    //
    timeOut.QuadPart = -100000000;
    if (KeWaitForSingleObject(&pUsbAdapter->noPendingWriteIrpEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          &timeOut) == STATUS_TIMEOUT)
    {
        athUsbDbgPrint(ATHUSB_ERROR,("Wait for pendingWriteIrpEvent timeOut, %d\n",pUsbAdapter->writePendingIrps));
    }

    athUsbFreeIrpUrbPool(pUsbAdapter);    
   
#ifndef CHIP_TEST
    if (pUsbAdapter->busState == ATHUSB_BUS_STATE_NORMAL) {
        athUsbDbgPrint(ATHUSB_INFO, ("Device state is normal, deconfig it\n"));
        ntStatus = athUsbDeconfigureDevice(pUsbAdapter);
        if (!NT_SUCCESS(ntStatus)) {
            athUsbDbgPrint(ATHUSB_ERROR, ("athUsbDeconfigureDevice failed\n"));
        }
    }
#endif

    athUsbReleaseMemory(pUsbAdapter);
    ExFreePool(pUsbAdapter);

    return A_OK;
}

A_STATUS
athUsbAbortAndReset( 
            IN  A_UINT8                  usbDeviceNum,        
            IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle   
            )
{    
    PATHUSB_USB_ADAPTER             pUsbAdapter;

    pUsbAdapter = (PATHUSB_USB_ADAPTER)pUsbDrvHandle;

    ASSERT(pUsbAdapter);
    ASSERT(pUsbAdapter->usbDeviceNum == usbDeviceNum);

    athUsbCancelAllIrp(pUsbAdapter, TRUE);
    pUsbAdapter->bReset = TRUE;

    return A_OK;
}

A_STATUS
athUsbSuspend( 
             IN  A_UINT8                  usbDeviceNum,        
             IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle   
             )
{    
    PATHUSB_USB_ADAPTER             pUsbAdapter;
    LARGE_INTEGER                   timeOut;

    PAGED_CODE();

    pUsbAdapter = (PATHUSB_USB_ADAPTER)pUsbDrvHandle;

    ASSERT(pUsbAdapter);
    ASSERT(pUsbAdapter->usbDeviceNum == usbDeviceNum);

    pUsbAdapter->bSuspend = TRUE;
    athUsbCancelAllIrp(pUsbAdapter, TRUE);

    KeWaitForSingleObject(&pUsbAdapter->noWorkItemPendingEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    //
    // wait for receive irps to complete.
    //
    timeOut.QuadPart = -100000000;
    if (KeWaitForSingleObject(&pUsbAdapter->noPendingReadIrpEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          &timeOut) == STATUS_TIMEOUT)
    {
        athUsbDbgPrint(ATHUSB_ERROR,("Wait for pendingReadIrpEvent timeOut, %d\n",pUsbAdapter->readPendingIrps));
    }
    
    //
    // wait for transmit irps to complete.
    //
    timeOut.QuadPart = -100000000;
    if (KeWaitForSingleObject(&pUsbAdapter->noPendingWriteIrpEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          &timeOut) == STATUS_TIMEOUT)
    {
        athUsbDbgPrint(ATHUSB_ERROR,("Wait for pendingWriteIrpEvent timeOut, %d\n",pUsbAdapter->writePendingIrps));
    }

    athUsbFreeIrpUrbPool(pUsbAdapter);
    athUsbDeconfigureDevice(pUsbAdapter);
    athUsbReleaseMemory(pUsbAdapter);

    pUsbAdapter->busState = ATHUSB_BUS_STATE_SUSPEND;
    return A_OK;
}

A_STATUS
athUsbResume( 
             IN  A_UINT8                  usbDeviceNum,        
             IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle   
             )
{    
    PATHUSB_USB_ADAPTER             pUsbAdapter;
    A_UINT8                         i, sendEPNum,recvEPNum;
    NTSTATUS                        ntStatus;
    PUSBD_INTERFACE_INFORMATION     pInterfaceInfo;
    PUSBD_PIPE_INFORMATION          pPipeInformation;

    PAGED_CODE();

    pUsbAdapter = (PATHUSB_USB_ADAPTER)pUsbDrvHandle;

    ASSERT(pUsbAdapter);
    ASSERT(pUsbAdapter->usbDeviceNum == usbDeviceNum);

    if (pUsbAdapter->busState == ATHUSB_BUS_STATE_SURPRISE_REMOVED) {
        return A_DEVICE_NOT_FOUND;
    }

    ntStatus = athUsbReadandSelectDescriptors(pUsbAdapter);

    if (!NT_SUCCESS(ntStatus)) {
        athUsbDbgPrint(ATHUSB_ERROR, ("athUsbReadandSelectDescriptors failed\n"));
        return A_DEVICE_NOT_FOUND;
    }

    athUsbGetAllStringDesciptor(pUsbAdapter);

    sendEPNum = 0;
    recvEPNum = 0;

    athUsbVendorReset(pUsbAdapter);
    athUsbResetAllPipes(pUsbAdapter);

    pInterfaceInfo = pUsbAdapter->pUsbInterface;

    for (i = 0; i < pInterfaceInfo->NumberOfPipes; i ++) {
        pPipeInformation = &pInterfaceInfo->Pipes[i];
        if (UsbdPipeTypeBulk == pPipeInformation->PipeType) {
            if (USB_ENDPOINT_DIRECTION_IN(pPipeInformation->EndpointAddress)) {
                pUsbAdapter->recvEndPoints[recvEPNum] = i;
                recvEPNum ++;
            } else {
                pUsbAdapter->sendEndPoints[sendEPNum] = i;
                sendEPNum ++;
            }
        }
    }

    ntStatus = athUsbAllocIrpUrbPool(pUsbAdapter);
    if (!NT_SUCCESS(ntStatus)) {
        athUsbDbgPrint(ATHUSB_ERROR, ("athUsbAllocIrpUrbPool failed\n"));
        return A_NO_MEMORY;
    }

    pUsbAdapter->busState = ATHUSB_BUS_STATE_NORMAL;
    pUsbAdapter->bSuspend = FALSE;
    pUsbAdapter->bStop = FALSE;

    return A_OK;
}

A_STATUS
athUsbSurpriseRemoved( 
                    IN  A_UINT8                  usbDeviceNum,        
                    IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle   
                    )
{
    PATHUSB_USB_ADAPTER             pUsbAdapter;

    pUsbAdapter = (PATHUSB_USB_ADAPTER)pUsbDrvHandle;

    ASSERT(pUsbAdapter);
    ASSERT(pUsbAdapter->usbDeviceNum == usbDeviceNum);

    pUsbAdapter->busState = ATHUSB_BUS_STATE_SURPRISE_REMOVED;
    return A_OK;
}

/*****************************************************************************
* Routine Description:
*
*    Retrieve data from WDM lower edge
*    
* Arguments:
*
*   pUsbDrvHandle - Pointer to usb adapter handle
*   epNum         - endpoint number      
*   buffer        - read buffer
*    
* Return Value:
*
*    STATUS_SUCCESS if the device can be safely removed, an appropriate 
*    NT Status if not.
*******************************************************************************/
A_STATUS 
athUsbDrvReceive(IN  ATHUSBDRV_HANDLE     *pUsbDrvHandle,
                 IN  A_UINT8               epNum,
                 IN  A_UINT8              *pBuffer)
{
    PATHUSB_USB_ADAPTER             pUsbAdapter;
    NTSTATUS                        ntStatus = STATUS_SUCCESS;
    PATHUSB_RXTX_OBJECT             pTransferObject;
    A_UINT8                         endpoint;
    PATHUSB_PIPE_STATE              pPipeState;
    LARGE_INTEGER                   dueTime;
    A_STATUS                        retStatus = A_OK;

    pUsbAdapter = (PATHUSB_USB_ADAPTER)pUsbDrvHandle;

    ASSERT(pUsbAdapter);

    if (pUsbAdapter->busState != ATHUSB_BUS_STATE_NORMAL || pUsbAdapter->bStop) {
        athUsbDbgPrint(ATHUSB_ERROR, ("Device is gone, so we can't receive anything\n"));
        return A_ERROR;
    }

    ntStatus = athUsbRemoveIdleIrp(pUsbAdapter,&pTransferObject,TRUE);
    if (!NT_SUCCESS(ntStatus)) {
        athUsbDbgPrint(ATHUSB_ERROR, ("athUsbRemoveIdleIrp failed\n"));
        return A_NO_MEMORY;
    }

    endpoint = pUsbAdapter->recvEndPoints[epNum];
    pTransferObject->pDataBuffer = pBuffer;
    pTransferObject->dataBufferLen = gepBufSize[0][epNum];
    pTransferObject->pPipeInformation = &(pUsbAdapter->pUsbInterface->Pipes[endpoint]);
    pTransferObject->pipeNum = epNum;
    pTransferObject->pUsbAdapter = pUsbAdapter;
    pPipeState = &(pUsbAdapter->pPipeState[endpoint]);

    if (pPipeState->pipeOpen == FALSE) {        
        pPipeState->pipeOpen = TRUE;
    }
    
    athUsbInitTransferUrb(pUsbAdapter,pTransferObject);
    athUsbInitTransferIrp(pUsbAdapter,pTransferObject);
    
    if (pUsbAdapter->bUseTimer) {
        ExInterlockedInsertTailList(&pUsbAdapter->readWaitingQueue,
                                    &pTransferObject->link,
                                    &(pUsbAdapter->readQueueLock.spinLock));

        if (!pUsbAdapter->bQueueTimer) {
            dueTime.LowPart = -50000;
            dueTime.HighPart = -1;
            KeSetTimer(&pUsbAdapter->usbAdptTimer, 
                        dueTime,&pUsbAdapter->usbAdptDpc);
            pUsbAdapter->bQueueTimer = TRUE;
        }
    } else {
        athUsbInsertPendingIrp(pUsbAdapter,pTransferObject,TRUE);
        ntStatus = IoCallDriver(pUsbAdapter->pNextDeviceObject,
                                pTransferObject->irp);    

        if(NT_SUCCESS(ntStatus)) {

            ntStatus = STATUS_SUCCESS;
        }
        else {
            athUsbDbgPrint(ATHUSB_ERROR, ("IoCallDriver fails with status %X\n", ntStatus));
            retStatus = A_ERROR;
        }
    }

    return retStatus;
}

/*****************************************************************************
* Routine Description:
*
*    Send data to WDM lower edge
*    
* Arguments:
*
*   pUsbDrvHandle - Pointer to usb adapter handle
*   epNum         - endpoint number      
*   buffer        - send buffer
*    
* Return Value:
*
*    STATUS_SUCCESS if the device can be safely removed, an appropriate 
*    NT Status if not.
*******************************************************************************/
A_STATUS
athUsbDrvSend(IN  ATHUSBDRV_HANDLE  *pUsbDrvHandle,
              IN  A_UINT8            epNum,
              IN  A_UINT8           *pBuffer)
{
    PATHUSB_USB_ADAPTER             pUsbAdapter;
    NTSTATUS                        ntStatus = STATUS_SUCCESS;
    PATHUSB_RXTX_OBJECT             pTransferObject;
    A_UINT8                         endpoint;
    PATHUSB_PIPE_STATE              pPipeState;
    LARGE_INTEGER                   dueTime;
    A_STATUS                        retStatus = A_OK;

    pUsbAdapter = (PATHUSB_USB_ADAPTER)pUsbDrvHandle;

    ASSERT(pUsbAdapter);

    if (pUsbAdapter->busState != ATHUSB_BUS_STATE_NORMAL || pUsbAdapter->bStop) {
        athUsbDbgPrint(ATHUSB_ERROR, ("Device is gone, so we can't transmit anything\n"));
        return A_ERROR;
    }

    ntStatus = athUsbRemoveIdleIrp(pUsbAdapter,&pTransferObject,FALSE);
    if (!NT_SUCCESS(ntStatus)) {
        athUsbDbgPrint(ATHUSB_ERROR, ("athUsbRemoveIdleIrp failed\n"));
        return A_NO_MEMORY;
    }

    endpoint = pUsbAdapter->sendEndPoints[epNum];
    pTransferObject->pDataBuffer = pBuffer;
    pTransferObject->dataBufferLen = gepBufSize[1][epNum];
    pTransferObject->pPipeInformation = &(pUsbAdapter->pUsbInterface->Pipes[endpoint]);
    pTransferObject->pipeNum = epNum;
    pTransferObject->pUsbAdapter = pUsbAdapter;
    pPipeState = &(pUsbAdapter->pPipeState[endpoint]);

    if (pPipeState->pipeOpen == FALSE) {        
        pPipeState->pipeOpen = TRUE;
    }
    
    athUsbInitTransferUrb(pUsbAdapter,pTransferObject);
    athUsbInitTransferIrp(pUsbAdapter,pTransferObject);
    
    if (pUsbAdapter->bUseTimer) {
        ExInterlockedInsertTailList(&pUsbAdapter->writeWaitingQueue,
                                    &pTransferObject->link,
                                    &(pUsbAdapter->writeQueueLock.spinLock));

        if (!pUsbAdapter->bQueueTimer) {
            dueTime.LowPart = -50000;
            dueTime.HighPart = -1;
            KeSetTimer(&pUsbAdapter->usbAdptTimer, 
                        dueTime,&pUsbAdapter->usbAdptDpc);
            pUsbAdapter->bQueueTimer = TRUE;
        }

    } else {
        athUsbInsertPendingIrp(pUsbAdapter,pTransferObject,FALSE);
        ntStatus = IoCallDriver(pUsbAdapter->pNextDeviceObject,
                                pTransferObject->irp);    

        if(NT_SUCCESS(ntStatus)) {

            ntStatus = STATUS_SUCCESS;
        }
        else {
            athUsbDbgPrint(ATHUSB_ERROR, ("IoCallDriver fails with status %X\n", ntStatus));
            retStatus = A_ERROR;
        }
    }

    return retStatus;
}

/***************************************************************************** 
* Routine Description:
*
*    This is the completion routine of the write irp in the irp/urb pair
*    passed down the stack for stream transfers.
*
*    If the transfer was cancelled or the device yanked out, then we
*    release resources, dump the statistics and return 
*    STATUS_MORE_PROCESSING_REQUIRED, so that the cleanup module can
*    free the irp.
*
*    otherwise, we reinitialize the transfers and continue recirculaiton 
*    of the irps.
*
* Arguments:
*
*    deviceObject - pointer to device object below us.
*    irp - I/O completion routine.
*    context - context passed to the completion routine
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
athUsbRxIrpComplete(IN PDEVICE_OBJECT deviceObject,
                    IN PIRP           irp,
                    IN PVOID          context
    )
{
    PATHUSB_USB_ADAPTER             pUsbAdapter;
    NTSTATUS                        ntStatus;
    PATHUSB_RXTX_OBJECT             pTransferObject;
    USBD_STATUS                     usbdStatus;    
    A_UINT32                        length;

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbRxIrpComplete - begin\n"));

    pTransferObject = (PATHUSB_RXTX_OBJECT)context;
    pUsbAdapter = pTransferObject->pUsbAdapter;

    length = pTransferObject->urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
    ntStatus = athUsbProcessTransfer(pTransferObject,&usbdStatus);

    if (!NT_SUCCESS(ntStatus)) {                

        if ((((ntStatus != STATUS_CANCELLED) && 
            (ntStatus != STATUS_DEVICE_NOT_CONNECTED) &&
            (usbdStatus != USBD_STATUS_DEVICE_GONE) && 
            (usbdStatus != USBD_STATUS_CANCELED))  || pUsbAdapter->bReset) &&
            (!pUsbAdapter->bStop))
        {
#ifdef RESET_PIPE_ONLY_FOR_ERROR
            if (!pUsbAdapter->bResetPipe[0][pTransferObject->pipeNum]) {
                pUsbAdapter->bResetPipe[0][pTransferObject->pipeNum] = TRUE;        
                pUsbAdapter->busState = ATHUSB_BUS_STATE_ERROR;
                if (pUsbAdapter->bSuspend == FALSE) {
                    athUsbQueueWorkItem(pUsbAdapter);
                }
            }
#else
            if (!pUsbAdapter->bDeviceReset) {
                pUsbAdapter->bDeviceReset = TRUE;                
                pUsbAdapter->busState = ATHUSB_BUS_STATE_ERROR;
                if ((pUsbAdapter->bSuspend == FALSE) &&
                    (pUsbAdapter->busState != ATHUSB_BUS_STATE_SURPRISE_REMOVED))
                {
                    athUsbQueueWorkItem(pUsbAdapter);
                }
            }
#endif
        } else {
            pUsbAdapter->bStop = TRUE;
        } 

        if (pUsbAdapter->bSuspend == FALSE) {
            pUsbAdapter->statIndHandler((VOID *)pUsbAdapter->pStubHandle,ATHUSB_BUS_STATE_ERROR);
        }

        //
        // this is the last irp to complete with this erroneous value
        // signal an event and return STATUS_MORE_PROCESSING_REQUIRED
        //
        if (InterlockedDecrement(&pUsbAdapter->readPendingIrps) == 0) {     

            KeSetEvent(&pUsbAdapter->noPendingReadIrpEvent,
                       1,
                       FALSE);
        }        

        pUsbAdapter->recvIndHandler((VOID *)pUsbAdapter->pStubHandle,
                                    pTransferObject->pipeNum,
                                    pTransferObject->pDataBuffer,
                                    0);
        athUsbDbgPrint(ATHUSB_ERROR, ("Receive Irp failed\n"));

    } else {

        if (InterlockedDecrement(&pUsbAdapter->readPendingIrps) == 0) {     
            if (pUsbAdapter->bCancel == TRUE) {
                KeSetEvent(&pUsbAdapter->noPendingReadIrpEvent,
                           1,
                           FALSE);
            }
        }
        
        pUsbAdapter->recvIndHandler((VOID *)pUsbAdapter->pStubHandle,
                                    pTransferObject->pipeNum,
                                    pTransferObject->pDataBuffer,
                                    length);
    }

    if (InterlockedDecrement(&pUsbAdapter->pipeReadPending[pTransferObject->pipeNum]) == 0) {     

        if (pUsbAdapter->bResetPipe[0][pTransferObject->pipeNum]) {
            KeSetEvent(&pUsbAdapter->noPendingPipeReadEvent[pTransferObject->pipeNum],
                       1,
                       FALSE);
        }
    }

    athUsbQueueIdleIrp(pUsbAdapter,pTransferObject,TRUE);

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbRxIrpComplete - ends\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}

/***************************************************************************** 
* Routine Description:
*
*    This is the completion routine of the read irp in the irp/urb pair
*    passed down the stack for stream transfers.
*
*    If the transfer was cancelled or the device yanked out, then we
*    release resources, dump the statistics and return 
*    STATUS_MORE_PROCESSING_REQUIRED, so that the cleanup module can
*    free the irp.
*
*    otherwise, we reinitialize the transfers and continue recirculaiton 
*    of the irps.
*
* Arguments:
*
*    deviceObject - pointer to device object below us.
*    irp - I/O completion routine.
*    context - context passed to the completion routine
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
athUsbTxIrpComplete(IN PDEVICE_OBJECT deviceObject,
                    IN PIRP           irp,
                    IN PVOID          context)
{
    PATHUSB_USB_ADAPTER             pUsbAdapter;
    NTSTATUS                        ntStatus;
    PATHUSB_RXTX_OBJECT             pTransferObject;
    USBD_STATUS                     usbdStatus; 

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbTxIrpComplete - begin\n"));

    pTransferObject = (PATHUSB_RXTX_OBJECT)context;
    pUsbAdapter = pTransferObject->pUsbAdapter;
    
    ntStatus = athUsbProcessTransfer(pTransferObject,&usbdStatus);    

    if (!NT_SUCCESS(ntStatus)) {                
        
        if ((((ntStatus != STATUS_CANCELLED) &&
            (ntStatus != STATUS_DEVICE_NOT_CONNECTED) &&
            (usbdStatus != USBD_STATUS_DEVICE_GONE) && 
            (usbdStatus != USBD_STATUS_CANCELED)) || pUsbAdapter->bReset) &&
            (!pUsbAdapter->bStop))
        {
#ifdef RESET_PIPE_ONLY_FOR_ERROR
            if (!pUsbAdapter->bResetPipe[1][pTransferObject->pipeNum]) {
                pUsbAdapter->bResetPipe[1][pTransferObject->pipeNum] = TRUE;                
                pUsbAdapter->busState = ATHUSB_BUS_STATE_ERROR;
                if (pUsbAdapter->bSuspend == FALSE) {
                    athUsbQueueWorkItem(pUsbAdapter);
                }
            }
#else
            if (!pUsbAdapter->bDeviceReset) {
                pUsbAdapter->bDeviceReset = TRUE;                
                pUsbAdapter->busState = ATHUSB_BUS_STATE_ERROR;
                if ((pUsbAdapter->bSuspend == FALSE) && 
                    (pUsbAdapter->busState != ATHUSB_BUS_STATE_SURPRISE_REMOVED))
                {
                    athUsbQueueWorkItem(pUsbAdapter);
                }
            }
#endif
        } else {
            pUsbAdapter->bStop = TRUE;
        }

        if (pUsbAdapter->bSuspend == FALSE) {
            pUsbAdapter->statIndHandler((VOID *)pUsbAdapter->pStubHandle,ATHUSB_BUS_STATE_ERROR);
        }

        //
        // this is the last irp to complete with this erroneous value
        // signal an event and return STATUS_MORE_PROCESSING_REQUIRED
        //
        if (InterlockedDecrement(&pUsbAdapter->writePendingIrps) == 0) {           

            KeSetEvent(&pUsbAdapter->noPendingWriteIrpEvent,
                       1,
                       FALSE);       
        }

        pUsbAdapter->sendCfmHandler((VOID *)pUsbAdapter->pStubHandle,
                                    pTransferObject->pipeNum,
                                    pTransferObject->pDataBuffer,
                                    0);

        athUsbDbgPrint(ATHUSB_ERROR, ("Send Irp failed\n"));

    } else {
        if (InterlockedDecrement(&pUsbAdapter->writePendingIrps) == 0) {           
            if (pUsbAdapter->bCancel == TRUE) {
                KeSetEvent(&pUsbAdapter->noPendingWriteIrpEvent,
                           1,
                           FALSE);       
            }
        }
        
        pUsbAdapter->sendCfmHandler((VOID *)pUsbAdapter->pStubHandle,
                                    pTransferObject->pipeNum,
                                    pTransferObject->pDataBuffer,
                                    pTransferObject->dataBufferLen);
    }

    if (InterlockedDecrement(&pUsbAdapter->pipeWritePending[pTransferObject->pipeNum]) == 0) {     

        if (pUsbAdapter->bResetPipe[1][pTransferObject->pipeNum]) {
            KeSetEvent(&pUsbAdapter->noPendingPipeWriteEvent[pTransferObject->pipeNum],
                       1,
                       FALSE);
        }
    }

    athUsbQueueIdleIrp(pUsbAdapter,pTransferObject,FALSE);

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbTxIrpComplete - ends\n"));    

    return STATUS_MORE_PROCESSING_REQUIRED;
}

/***************************************************************************** 
* Routine Description:
*
*    This routine detect card still present or not.
*
* Arguments:
*
*    deviceObject - pointer to device object below us.
*    irp - I/O completion routine.
*    context - context passed to the completion routine
*
* Return Value:
*
*    NT status value
*****************************************************************************/
A_BOOL
athUsbDetectCardPresent(
                        IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle                        
                        )
{
    PATHUSB_USB_ADAPTER             pUsbAdapter;
    NTSTATUS                        ntStatus;   
    ULONG                           portStatus;

    pUsbAdapter = (PATHUSB_USB_ADAPTER)pUsbDrvHandle;

    ASSERT(pUsbAdapter);

    ntStatus = athUsbGetPortStatus(pUsbAdapter, &portStatus);
    if (!(NT_SUCCESS(ntStatus)) || 
        ((portStatus & USBD_PORT_ENABLED) != USBD_PORT_ENABLED) ||
        ((portStatus & USBD_PORT_CONNECTED) != USBD_PORT_CONNECTED))
    {
        //Device is gone!        
        return FALSE;
    }

    return TRUE;
}