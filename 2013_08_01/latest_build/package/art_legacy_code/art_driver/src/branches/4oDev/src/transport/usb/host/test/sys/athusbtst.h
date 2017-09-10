/*
 * Copyright (c) 2000-2002 Atheros Communications, Inc., All Rights Reserved
 *
 * athusbdrv.h -- Atheros USB Driver header file
 * 
 */

#ifndef __ATHUSB_TEST_H__
#define __ATHUSB_TEST_H__

typedef struct _ATHUSB_API_STREAM_OBJECT {    
    
    PDEVICE_OBJECT         deviceObject;
    VOID *                 pUsbAdapt;

    UCHAR                   inPipeNum;
    UCHAR                   outPipeNum;    

    UCHAR                   totalInPipe;
    UCHAR                   totalOutPipe;

    PUCHAR                 savedDataBuffer;
    ULONG                  savedDataBufferLen;

    ULONG                  streamPackSize;

    PUCHAR                 tempDataBuffer;
    ULONG                  tempReadPos;
    ULONG                  tempWritePos;

    ATHUSB_SPIN_LOCK       readQueueLock;
    LIST_ENTRY             readPendingQueue;
    LIST_ENTRY             readIdleQueue;
    LIST_ENTRY             readDoneQueue;

    ATHUSB_SPIN_LOCK       writeQueueLock;
    LIST_ENTRY             writePendingQueue;
    LIST_ENTRY             writeIdleQueue;
    LIST_ENTRY             writeDoneQueue;

    KSPIN_LOCK             dpcLock;

    ULONG                  readPos;
    ULONG                  writePos;     

    ULONG                  readIoElapseTime;
    ULONG                  writeIoElapseTime ;

    ULONG                  readElapseTime;
    ULONG                  writeElapseTime ;

    ULONG                  maxReadTime;
    ULONG                  maxWriteTime;
    BOOLEAN                bCancel;

    ULONG                  writeSequenceNum;
    ULONG                  readSequenceNum;

    BOOLEAN                bWriteQueueReset;
    BOOLEAN                bReadQueueReset;

    BOOLEAN                bStop;
    KEVENT                 timeEvent;

    BOOLEAN                bDelay;

    PUSBD_PIPE_INFORMATION inPipeInformation;
    PUSBD_PIPE_INFORMATION outPipeInformation;    

    KDPC                    streamDpc;
    KTIMER                  streamTimer;
    KEVENT                  streamDpcEvent;
    BOOLEAN                 bQueueThread;

    ULONG                   busState;

    LARGE_INTEGER           oldTime;
    ULONG                   minTime;
    ULONG                   maxTime;
    ULONG                   totalTime;
    ULONG                   totalCycle;

} ATHUSB_API_STREAM_OBJECT, *PATHUSB_API_STREAM_OBJECT;

typedef struct _ATHUSB_API_TRANSFER_OBJECT {

    LIST_ENTRY                  link;    

    PUCHAR                      dataBuffer;

    ULONG                       dataBufferLen;

    BOOLEAN                     bRead;    

    PATHUSB_API_STREAM_OBJECT   streamObject;

} ATHUSB_API_TRANSFER_OBJECT, *PATHUSB_API_TRANSFER_OBJECT;

typedef struct _TEST_THREAD_CONTEXT{
    PDEVICE_OBJECT                  deviceObject;            
    PATHUSB_API_STREAM_OBJECT       streamObject; 
    PIO_WORKITEM                    workItem;    
} TEST_THREAD_CONTEXT, *PTEST_THREAD_CONTEXT;

VOID
athUsbRecvIndication(IN  void     *pStubHandle,
                     IN  UCHAR     epNum,
                     IN  UCHAR    *buffer,
                     IN  ULONG     bytesReceived);

VOID
athUsbSendConfirm(IN  void     *pStubHandle,
                  IN  UCHAR     epNum,
                  IN  UCHAR    *buffer,
                  IN  ULONG     bytesSent);

VOID
athUsbStatusIndication(IN  void     *pStubHandle,                            
                       IN  ULONG    status);

VOID
StreamApiDpcProc(
              IN   PKDPC    Dpc,
              IN   PVOID    DeferredContex,
              IN   PVOID    SystemContex1,
              IN   PVOID    SystemContex2);

NTSTATUS
athUsbStartUsbStream(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,    
    IN ULONG          InPipeNum,
    IN ULONG          OutPipeNum,
    IN ULONG          PacketSize
    );

NTSTATUS
athUsbSetupBufferQueue(IN PDEVICE_OBJECT             DeviceObject,
                       IN PATHUSB_API_STREAM_OBJECT  StreamObject);

NTSTATUS
athUsbAllocBuffer(
    IN PDEVICE_OBJECT            DeviceObject,
    IN PATHUSB_API_STREAM_OBJECT StreamObject,         
    IN BOOLEAN                   bRead);

VOID
athUsbGenerateTxData(IN PATHUSB_API_STREAM_OBJECT   StreamObject,
                     IN PATHUSB_API_TRANSFER_OBJECT TransferObject,
                     IN BOOLEAN                     bRand);

VOID
athUsbCompareRxData(IN PATHUSB_API_STREAM_OBJECT StreamObject,
                    IN PATHUSB_API_TRANSFER_OBJECT TransferObject,
                    IN PATHUSB_STREAM_INFO         StreamInfo);

void
athUsbFreeBufferQueue(IN PATHUSB_API_STREAM_OBJECT StreamObject);


NTSTATUS
athUsbStopUsbStream(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PATHUSB_API_STREAM_OBJECT StreamObject,
    IN PIRP                  Irp
    );

BOOLEAN 
athUsbRemoveHeadList(IN PATHUSB_API_STREAM_OBJECT       StreamObject,                 
                     IN OUT PATHUSB_API_TRANSFER_OBJECT *pTransferObject,
                     IN BOOLEAN                         bRead,
                     IN PLIST_ENTRY                     pQueue);

VOID
athUsbInsertTailList(IN PATHUSB_API_STREAM_OBJECT   StreamObject,
                     IN PATHUSB_API_TRANSFER_OBJECT TransferObject,
                     IN BOOLEAN                     bRead,
                     IN PLIST_ENTRY                 pQueue);

NTSTATUS
athUsbStreamApiObjectCleanup(
    IN PATHUSB_API_STREAM_OBJECT StreamObject,
    IN PDEVICE_EXTENSION         DeviceExtension
    );

VOID
athUsbTestWorkerItemCallback(
     IN  PDEVICE_OBJECT  DeviceObject,
     IN  PVOID           Context);

NTSTATUS
athsUsbQueueTestWorkItem(
                 IN PDEVICE_OBJECT              DeviceObject,                
                 IN PATHUSB_API_STREAM_OBJECT   StreamObject);

#endif /* __ATHUSBDRV_H__ */
