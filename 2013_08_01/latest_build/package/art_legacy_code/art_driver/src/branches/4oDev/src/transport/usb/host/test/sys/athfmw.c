/*++

Module Name:

    athfmw.c

Abstract:

    This file contains firmware download

Environment:

    Kernel mode

Notes:

--*/

#include "precomp.h"
#include "athusbapi.h"
#include "athusbdrv.h"
#include "athusbconfig.h"
#include "athusbRxTx.h"
#include "athfmw.h"

#define FIWMWARE                L"\\SystemRoot\\System32\\drivers\\ar5523.bin"
#define RETRY_LIMIT             3
#define MAX_WAIT_TIME           20

VOID
athUsbDownloadFirmware(IN PDEVICE_OBJECT  DeviceObject)
{
    IO_STATUS_BLOCK             iosb;
    UNICODE_STRING              fileName;
    OBJECT_ATTRIBUTES           objectAttributes;
    NTSTATUS                    ntStatus;
    A_STATUS                    status;
    HANDLE                      fileHandle = NULL;
    FILE_OBJECT                 *fileObj;
    FILE_STANDARD_INFORMATION   standInfo;
    A_UINT32                    totalLength,leftLength,curLength;
    PDEVICE_EXTENSION           deviceExtension;
    PATHUSB_FMDL_OBJECT         firmwareObj;
    PATHUSB_FMDL_MSG            pMsgHeader;
    LARGE_INTEGER               readPos;
    A_UINT32                    retryTime = 0;    
    PVOID                       eventArray[2];
    LARGE_INTEGER               timeOut;
    A_BOOL                      bQuit;

    deviceExtension = DeviceObject->DeviceExtension;

    RtlInitUnicodeString (&fileName, FIWMWARE);

    InitializeObjectAttributes (
        &objectAttributes,
        &fileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    /*
     * Open fimware file
     */
    ntStatus = ZwCreateFile (
        &fileHandle,
        GENERIC_READ | SYNCHRONIZE,
        &objectAttributes,
        &iosb,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
        );

    if (!NT_SUCCESS(ntStatus)) {
        athUsbDbgPrint(ATHUSB_ERROR, 
                       ("Open firmware image file %s failed, status = %x\n",
                       FIWMWARE,
                       ntStatus));
        return;
    }

    /*
     * Get file length
     */
    ntStatus = ZwQueryInformationFile(
            fileHandle,
            &iosb,
            &standInfo,
            sizeof(FILE_STANDARD_INFORMATION),
            FileStandardInformation);

    if (!NT_SUCCESS(ntStatus)) {
        athUsbDbgPrint(ATHUSB_ERROR, 
                       ("ZwQueryInformationFile failed, status = %x\n",
                       ntStatus));
        ZwClose (fileHandle);
        return;
    }

    totalLength = standInfo.EndOfFile.LowPart;

    firmwareObj = ExAllocatePool(NonPagedPool, 
                                 sizeof(ATHUSB_FMDL_OBJECT));

    if(firmwareObj == NULL) {
        athUsbDbgPrint(ATHUSB_ERROR, ("firmwareObj allocation failed, no memory\n"));
        ZwClose (fileHandle);
        return;
    }

    RtlZeroMemory(firmwareObj, sizeof(ATHUSB_FMDL_OBJECT));

    firmwareObj->fmData = ExAllocatePool(NonPagedPool, 
                                         FMDLDATA_MAX_LENGTH);

    if(firmwareObj->fmData == NULL) {
        athUsbDbgPrint(ATHUSB_ERROR, ("fmData allocation failed, no memory\n"));
        ZwClose (fileHandle);
        fmwFreeMemory(firmwareObj);
        return;
    }

    firmwareObj->fmInMsg = ExAllocatePool(NonPagedPool, 
                                          FMDLMSG_MAX_LENGTH);

    if(firmwareObj->fmInMsg == NULL) {
        athUsbDbgPrint(ATHUSB_ERROR, ("fmInMsg allocation failed, no memory\n"));
        ZwClose (fileHandle);
        fmwFreeMemory(firmwareObj);
        return;
    }

    firmwareObj->fmOutMsg = ExAllocatePool(NonPagedPool, 
                                           FMDLMSG_MAX_LENGTH);

    if(firmwareObj->fmOutMsg == NULL) {
        athUsbDbgPrint(ATHUSB_ERROR, ("fmOutMsg allocation failed, no memory\n"));
        ZwClose (fileHandle);
        fmwFreeMemory(firmwareObj);
        return;
    }

    deviceExtension->FirmwareObject = firmwareObj;

    KeInitializeEvent(&firmwareObj->readEvent,NotificationEvent,TRUE);
    KeInitializeEvent(&firmwareObj->writeEvent,NotificationEvent,TRUE);
    KeInitializeEvent(&firmwareObj->quitEvent,NotificationEvent,FALSE);    

    /*
     * Initialize the USB device
     */
    status = athUsbDrvInit(1,
                           deviceExtension->TopOfStackDeviceObject,
                           (void *)DeviceObject,
                           fmwDldRecvIndication,
                           fmwDldSendConfirm,
                           fmwDldStatusIndication,
                           &firmwareObj->pUsbAdapt,
                           &firmwareObj->totalInPipe,
                           &firmwareObj->totalOutPipe);

    if(status != A_OK) {
        athUsbDbgPrint(ATHUSB_ERROR, ("athUsbDrvInit failed\n"));
        ZwClose (fileHandle);
        ExFreePool(firmwareObj);
        return;
    }

    leftLength = totalLength;
    readPos.QuadPart = 0;

    firmwareObj->totalLength = totalLength;
    firmwareObj->bFirst = TRUE;

    while (leftLength) {
        if (leftLength <= FMDLDATA_MAX_LENGTH) {
            curLength = leftLength;
            leftLength = 0;
        } else {
            curLength = FMDLDATA_MAX_LENGTH;
            leftLength -= FMDLDATA_MAX_LENGTH;
        }

        RtlZeroMemory(firmwareObj->fmData, FMDLDATA_MAX_LENGTH);

        ntStatus = ZwReadFile(fileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &iosb,
                            firmwareObj->fmData,
                            curLength,
                            &readPos,
                            NULL);

        if (!NT_SUCCESS(ntStatus)) {
            athUsbDbgPrint(ATHUSB_ERROR, 
                           ("Read firmware image file failed, status = %x\n",
                           ntStatus));
            break;
        }

        RtlZeroMemory(firmwareObj->fmOutMsg, FMDLMSG_MAX_LENGTH);

        pMsgHeader = (PATHUSB_FMDL_MSG) firmwareObj->fmOutMsg;
        pMsgHeader->msgLength = FMDLMSG_HEADER_LENGTH;
        pMsgHeader->dataLegnth = curLength;
        pMsgHeader->totalLength = totalLength;
        pMsgHeader->leftLength = leftLength;
        
        fmwSwapHeader(pMsgHeader);
        /*
         * Send data information over message pipe
         */
        if (athUsbFirmwareWrite(firmwareObj,MESSAGE_PIPE,firmwareObj->fmOutMsg) == FALSE) {
            athUsbDbgPrint(ATHUSB_ERROR,("Write message failed or driver quit\n"));
            break;
        }        

        /*
         * Read data information back from message pipe, there are two case
         * if the image file is too big, this request will complete immediately
         * since target will tell us not to download, otherwise, this request will
         * be pending until the follow image data packet finish transfer.
         */
        RtlZeroMemory(firmwareObj->fmInMsg, FMDLMSG_MAX_LENGTH);
        KeClearEvent(&firmwareObj->readEvent);
        if (athUsbDrvReceive(firmwareObj->pUsbAdapt,
                             MESSAGE_PIPE,
                             firmwareObj->fmInMsg) != A_OK)
        {
            athUsbDbgPrint(ATHUSB_ERROR,("Read message failed\n"));
            break;        
        }
        
        /*
         * Send data over data pipe
         */
        if (athUsbFirmwareWrite(firmwareObj,DATA_PIPE/*MESSAGE_PIPE*/,firmwareObj->fmData) == FALSE) {
            athUsbDbgPrint(ATHUSB_ERROR,("Write data failed or driver quit\n"));
            break;
        }
        
        /* 
         * Check read back message to determine the status
         */
        timeOut.QuadPart = -10000000 * MAX_WAIT_TIME;     // 5 second
        
        eventArray[0] = &firmwareObj->quitEvent;
        eventArray[1] = &firmwareObj->readEvent;

        status = KeWaitForMultipleObjects(2,  // count
                                      eventArray, // DispatcherObjectArray
                                      WaitAny,    // WaitType
                                      Executive,  // WaitReason
                                      KernelMode, // WaitMode
                                      FALSE,      // Alertable
                                      &timeOut,   // Timeout (Optional)
                                      NULL);      // WaitBlockArray (Optional)

        switch (status) {
            case 0:            
                /*
                * quit event signaled
                */
                bQuit = TRUE;
            break;

            case 1:
                /*
                * USB write finished event signaled
                */
                if (firmwareObj->bytesReceived) {
                    bQuit = FALSE;
                } else {
                    bQuit = TRUE;
                }
                break;

            case STATUS_TIMEOUT:
                /*
                * USB write timeout
                */
                bQuit = TRUE;
                break;
        }

        if (bQuit == TRUE) {
            athUsbDbgPrint(ATHUSB_ERROR,("Read message failed\n"));
            break; 
        }

#if 0
        if (athUsbFirmwareRead(firmwareObj,MESSAGE_PIPE,firmwareObj->fmInMsg) == FALSE) {
            athUsbDbgPrint(ATHUSB_ERROR,("Write data failed or driver quit\n"));
            break;
        }
#endif

        pMsgHeader = (PATHUSB_FMDL_MSG) firmwareObj->fmInMsg;
        fmwSwapHeader(pMsgHeader);

        if (pMsgHeader->msgLength < FMDLMSG_HEADER_LENGTH) {
            athUsbDbgPrint(ATHUSB_ERROR,("Read back message length is too short\n"));
            break;
        }        

        if ((pMsgHeader->dataLegnth != curLength) ||
            (pMsgHeader->leftLength != leftLength) ||
            (pMsgHeader->totalLength != totalLength))
        {
            if (retryTime > RETRY_LIMIT) {
                athUsbDbgPrint(ATHUSB_ERROR,("Over retry limit, give up downloading now\n"));
                break;
            }

            retryTime ++;

            leftLength = totalLength;
            readPos.QuadPart = 0;
            continue;
        }

        readPos.QuadPart += curLength;
    }

    ZwClose (fileHandle);
    athUsbDrvExit(1,firmwareObj->pUsbAdapt);    
    fmwFreeMemory(firmwareObj);
    deviceExtension->FirmwareObject = NULL;
}

VOID
fmwFreeMemory(IN PATHUSB_FMDL_OBJECT  firmwareObj)
{
    if (firmwareObj->fmData) {
        ExFreePool(firmwareObj->fmData);
    }

    if (firmwareObj->fmInMsg) {
        ExFreePool(firmwareObj->fmInMsg);
    }

    if (firmwareObj->fmOutMsg) {
        ExFreePool(firmwareObj->fmOutMsg);
    }

    ExFreePool(firmwareObj);
}

VOID
fmwSwapHeader(PATHUSB_FMDL_MSG fwmMsg)
{
    fwmMsg->dataLegnth = A_swab32(fwmMsg->dataLegnth);
    fwmMsg->leftLength = A_swab32(fwmMsg->leftLength);
    fwmMsg->msgLength = A_swab32(fwmMsg->msgLength);
    fwmMsg->totalLength = A_swab32(fwmMsg->totalLength);
}

VOID
athUsbAbortDownload(IN PATHUSB_FMDL_OBJECT         firmwareObj)
{
    KeSetEvent(&firmwareObj->quitEvent,
               1,
               FALSE);
}

BOOLEAN
athUsbFirmwareWrite(IN PATHUSB_FMDL_OBJECT         firmwareObj,
                    IN A_UINT8                     pipeType,
                    IN A_UINT8                     *pBuf)
{
    PVOID                       eventArray[2];
    LARGE_INTEGER               timeOut;
    NTSTATUS                    status;
    A_BOOL                      bRet = FALSE;

    KeClearEvent(&firmwareObj->writeEvent);

    if (athUsbDrvSend(firmwareObj->pUsbAdapt,
                      pipeType,
                      pBuf) != A_OK)
    {
        return bRet;
    }

    timeOut.QuadPart = -10000000 * MAX_WAIT_TIME;     // 20 second
        
    eventArray[0] = &firmwareObj->quitEvent;
    eventArray[1] = &firmwareObj->writeEvent;

    status = KeWaitForMultipleObjects(2,  // count
                                      eventArray, // DispatcherObjectArray
                                      WaitAny,    // WaitType
                                      Executive,  // WaitReason
                                      KernelMode, // WaitMode
                                      FALSE,      // Alertable
                                      &timeOut,   // Timeout (Optional)
                                      NULL);      // WaitBlockArray (Optional)

    switch (status) {
        case 0:
            /*
             * quit event signaled
             */
            bRet = FALSE;
            break;

        case 1:
            /*
             * USB write finished event signaled
             */
            if (firmwareObj->bytesSent) {
                bRet = TRUE;
            } else {
                bRet = FALSE;
            }
            break;

        case STATUS_TIMEOUT:
            /*
             * USB write timeout
             */
            bRet = FALSE;
            break;
    }

    return bRet;
}

BOOLEAN
athUsbFirmwareRead(IN PATHUSB_FMDL_OBJECT         firmwareObj,
                   IN A_UINT8                     pipeType,
                   IN A_UINT8                     *pBuf)
{
    PVOID                       eventArray[2];
    LARGE_INTEGER               timeOut;
    NTSTATUS                    status;
    A_BOOL                      bRet = FALSE;

    KeClearEvent(&firmwareObj->readEvent);

    if (athUsbDrvReceive(firmwareObj->pUsbAdapt,
                         pipeType,
                         pBuf) != A_OK)
    {
        return bRet;
    }

    timeOut.QuadPart = -10000000 * MAX_WAIT_TIME;     // 5 second
        
    eventArray[0] = &firmwareObj->quitEvent;
    eventArray[1] = &firmwareObj->readEvent;

    status = KeWaitForMultipleObjects(2,  // count
                                      eventArray, // DispatcherObjectArray
                                      WaitAny,    // WaitType
                                      Executive,  // WaitReason
                                      KernelMode, // WaitMode
                                      FALSE,      // Alertable
                                      &timeOut,   // Timeout (Optional)
                                      NULL);      // WaitBlockArray (Optional)

    switch (status) {
        case 0:
            /*
             * quit event signaled
             */
            bRet = FALSE;
            break;

        case 1:
            /*
             * USB write finished event signaled
             */
            if (firmwareObj->bytesReceived) {
                bRet = TRUE;
            } else {
                bRet = FALSE;
            }
            break;

        case STATUS_TIMEOUT:
            /*
             * USB write timeout
             */
            bRet = FALSE;
            break;
    }

    return bRet;
}

VOID
fmwDldRecvIndication(IN  void     *pStubHandle,
                     IN  UCHAR     epNum,
                     IN  UCHAR    *buffer,
                     IN  ULONG     bytesReceived)
{
    PDEVICE_OBJECT              deviceObject;
    PDEVICE_EXTENSION           deviceExtension;
    PATHUSB_FMDL_OBJECT         firmwareObj;
    A_UINT32                    maxRamSize;

    deviceObject = (PDEVICE_OBJECT)pStubHandle;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
    firmwareObj = (PATHUSB_FMDL_OBJECT) deviceExtension->FirmwareObject;

    /*
     * Signal the read completion event
     */
    firmwareObj->bytesReceived = bytesReceived;

    if (firmwareObj->bFirst) {
        firmwareObj->bFirst = FALSE;

        if (bytesReceived) {
            maxRamSize = *((A_UINT32 *)(firmwareObj->fmInMsg + FMDLMSG_HEADER_LENGTH));
            maxRamSize = A_swab32(maxRamSize);

            if (maxRamSize < firmwareObj->totalLength) {
                athUsbDbgPrint(ATHUSB_ERROR,("Image file size is too large\n"));
                KeSetEvent(&firmwareObj->quitEvent,
                           1,
                           FALSE);
            }
        } else {
            KeSetEvent(&firmwareObj->quitEvent,
                       1,
                       FALSE);
        }
    }

    KeSetEvent(&firmwareObj->readEvent,
               1,
               FALSE);
}

VOID
fmwDldSendConfirm(IN  void     *pStubHandle,
                  IN  UCHAR     epNum,
                  IN  UCHAR    *buffer,
                  IN  ULONG     bytesSent)
{
    PDEVICE_OBJECT              deviceObject;
    PDEVICE_EXTENSION           deviceExtension;
    PATHUSB_FMDL_OBJECT         firmwareObj;

    deviceObject = (PDEVICE_OBJECT)pStubHandle;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
    firmwareObj = (PATHUSB_FMDL_OBJECT) deviceExtension->FirmwareObject;

    /*
     * Signal the write completion event
     */
    firmwareObj->bytesSent = bytesSent;

    KeSetEvent(&firmwareObj->writeEvent,
               1,
               FALSE);
}

VOID
fmwDldStatusIndication(IN  void     *pStubHandle,                            
                       IN  ULONG    status)
{
}

