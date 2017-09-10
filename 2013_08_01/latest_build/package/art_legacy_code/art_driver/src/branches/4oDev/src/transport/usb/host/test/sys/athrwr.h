/*++

Module Name:

    athrwr.h

Abstract:

Environment:

    Kernel mode

Notes:    

--*/
#ifndef _ATHUSB_RWR_H
#define _ATHUSB_RWR_H

#define ATHUSB_MAX_IRP                  3
#define ATHUSB_MAX_BUF_NUM              60

typedef struct _FILE_OBJECT_CONTENT {

    PVOID PipeInformation;
    PVOID StreamInformation;

}FILE_OBJECT_CONTENT, *PFILE_OBJECT_CONTENT;

typedef struct _ATHUSB_RW_CONTEXT {

    PURB                   Urb;
    PMDL                   Mdl;
    ULONG                  Length;         // remaining to xfer
    ULONG                  Numxfer;        // cumulate xfer
    ULONG_PTR              VirtualAddress; // va for next segment of xfer.
    PDEVICE_EXTENSION      DeviceExtension;
    PUSBD_PIPE_INFORMATION pipeInformation;
    BOOLEAN                bPadZeroPacket;
    BOOLEAN                bRead;
    ULONG                  StageLength;

} ATHUSB_RW_CONTEXT, * PATHUSB_RW_CONTEXT;

typedef struct _ATHUSB_STREAM_OBJECT {

    // number of pending irps for this stream
    ULONG  ReadPendingIrps;
    ULONG  WritePendingIrps;

    // event signaled when no irps pending
    KEVENT NoPendingReadIrpEvent;
    KEVENT NoPendingWriteIrpEvent;
    
    PDEVICE_OBJECT DeviceObject;

    PUSBD_PIPE_INFORMATION InPipeInformation;
    PUSBD_PIPE_INFORMATION OutPipeInformation;    
    
    ULONG          TotalBufferLen;

    PUCHAR         SavedDataBuffer;
    ULONG          SavedDataBufferLen;

    PUCHAR         TempDataBuffer;
    ULONG          TempReadPos;
    ULONG          TempWritePos;

    KSPIN_LOCK     ReadQueueLock;
    LIST_ENTRY     ReadPendingIrpQueue;
    LIST_ENTRY     ReadIdleIrpQueue;

    KSPIN_LOCK     WriteQueueLock;
    LIST_ENTRY     WritePendingIrpQueue;
    LIST_ENTRY     WriteIdleIrpQueue;

    CCHAR          StackSize;

    ULONG          ReadPos;
    ULONG          WritePos; 

    ULONG          StreamPackSize;
    LARGE_INTEGER  StartTime;

    ULONG          ReadIoElapseTime;
    ULONG          WriteIoElapseTime ;

    ULONG          ReadElapseTime;
    ULONG          WriteElapseTime ;

    ULONG          MaxReadTime;
    ULONG          MaxWriteTime;
    BOOLEAN        bCancel;

    ULONG          WriteSequenceNum;
    ULONG          ReadSequenceNum;

    BOOLEAN        bWriteQueueReset;
    BOOLEAN        bReadQueueReset;

    BOOLEAN        bStop;
    KEVENT         TimeEvent;

} ATHUSB_STREAM_OBJECT, *PATHUSB_STREAM_OBJECT;

typedef struct _ATHUSB_TRANSFER_OBJECT {

    LIST_ENTRY     Link;
    PIRP Irp;

    PURB Urb;

    PUCHAR  DataBuffer;

    ULONG   DataBufferLen;

    BOOLEAN bRead;  

    //
    // statistics.
    //
    ULONG TimesRecycled;

    ULONG TotalPacketsProcessed;

    ULONG TotalBytesProcessed;

    ULONG ErrorPacketCount;

    ULONG ErrorSeqCount;

    PATHUSB_STREAM_OBJECT StreamObject;    

} ATHUSB_TRANSFER_OBJECT, *PATHUSB_TRANSFER_OBJECT;

typedef struct _WORKER_THREAD_CONTEXT{
    PDEVICE_OBJECT                  DeviceObject;    
    BOOLEAN                         bStreaming;    
    PUSBD_PIPE_INFORMATION          pipeInformation;
    PATHUSB_STREAM_OBJECT           StreamObject; 
    PIO_WORKITEM                    WorkItem;
    PIRP                            Irp;
    BOOLEAN                         bRead;
} WORKER_THREAD_CONTEXT, *PWORKER_THREAD_CONTEXT;

NTSTATUS
AthUsb_DispatchReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
AthUsb_ReadWriteCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

VOID 
AthUsb_HandleError(
                   IN NTSTATUS            ntStatus,
                   IN PDEVICE_OBJECT      DeviceObject,
                   PUSBD_PIPE_INFORMATION pipeInformation
                   );

NTSTATUS
AthUsb_ResumitIrp(
           IN PATHUSB_RW_CONTEXT  rwContext,
           IN PIRP                Irp,
           IN ULONG               stageLength);


NTSTATUS
AthsUsb_QueueWorkItem(
                 IN PDEVICE_OBJECT            DeviceObject,
                 IN BOOLEAN                   bStreaming,
                 IN PUSBD_PIPE_INFORMATION    pipeInformation,
                 IN PATHUSB_STREAM_OBJECT     StreamObject,                 
                 IN BOOLEAN                   bRead);

VOID
AthUsb_WorkerItemCallback(
     IN  PDEVICE_OBJECT  DeviceObject,
     IN  PVOID           Context);

NTSTATUS
AthUsb_StartUsbStream(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,    
    IN ULONG          InPipeNum,
    IN ULONG          OutPipeNum,
    IN ULONG          PacketSize
    );

NTSTATUS
AthUsb_StartTransfer(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PATHUSB_STREAM_OBJECT StreamObject,     
    IN ULONG                 Index,
    IN BOOLEAN               bRead
    );

NTSTATUS
AthUsb_SetupFullDuplexXerf(
                           IN PDEVICE_OBJECT         DeviceObject,
                           IN PATHUSB_STREAM_OBJECT  StreamObject);

NTSTATUS
AthUsb_InitializeStreamUrb(
    IN PDEVICE_OBJECT          DeviceObject,
    IN PATHUSB_TRANSFER_OBJECT TransferObject
    );

NTSTATUS
AthUsb_UsbReadIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
AthUsb_UsbWriteIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
AthUsb_ProcessTransfer(
    IN PATHUSB_TRANSFER_OBJECT TransferObject,
    USBD_STATUS                *pUsbdStatus
    );

NTSTATUS
AthUsb_StopUsbStream(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PATHUSB_STREAM_OBJECT StreamObject,
    IN PIRP                  Irp
    );

void
AthUsb_CancelStreamIrp(IN PATHUSB_STREAM_OBJECT StreamObject);

void
AthUsb_FreeStreamResource(IN PATHUSB_STREAM_OBJECT StreamObject);

void
AthUsb_CalcStreamInfo(IN PATHUSB_STREAM_INFO     StreamInfo,
                      IN PATHUSB_TRANSFER_OBJECT TransferObject);

NTSTATUS
AthUsb_StreamObjectCleanup(
    IN PATHUSB_STREAM_OBJECT StreamObject,
    IN PDEVICE_EXTENSION     DeviceExtension,
    IN BOOLEAN               bStop
    );

VOID AthUsb_RecycleIrp(IN PATHUSB_STREAM_OBJECT   StreamObject,
                       IN PATHUSB_TRANSFER_OBJECT TransferObject,
                       IN BOOLEAN bRead);

VOID AthUsb_ReInitXerfObject(IN PATHUSB_TRANSFER_OBJECT TransferObject);

BOOLEAN 
AthUsb_GetNewTransferObj(IN PATHUSB_STREAM_OBJECT       StreamObject,                          
                       IN OUT PATHUSB_TRANSFER_OBJECT *pTransferObject,
                       IN BOOLEAN bRead);

AthUsb_QueuePendingIrp(IN PATHUSB_STREAM_OBJECT   StreamObject,
                       IN PATHUSB_TRANSFER_OBJECT TransferObject,
                       IN BOOLEAN bRead);

NTSTATUS
AthUsb_VendorReset(IN PDEVICE_OBJECT        DeviceObject);

VOID
AthUsb_GenerateTxData(IN PATHUSB_STREAM_OBJECT StreamObject,
                      IN PATHUSB_TRANSFER_OBJECT TransferObject,
                      IN BOOLEAN                  bRand);

VOID
AthUsb_CompareRxData(IN PATHUSB_STREAM_OBJECT StreamObject,
                     IN PATHUSB_TRANSFER_OBJECT TransferObject);

void
AthUsb_InitStreamObject(IN PATHUSB_STREAM_OBJECT StreamObject);

#endif
