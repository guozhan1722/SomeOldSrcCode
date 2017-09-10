/*
 * Copyright (c) 2004 Atheros Communications, Inc., All Rights Reserved
 *
 * athusbdrv.h -- Atheros USB Driver header file
 * 
 */

#ifndef __ATHUSB_DRV_H__
#define __ATHUSB_DRV_H__

#include "usbdrivr.h"

#define MAX_PIPE_NUM            10

typedef struct _ATHUSB_PIPE_STATE {
    A_BOOL       pipeOpen;
} ATHUSB_PIPE_STATE, *PATHUSB_PIPE_STATE;

typedef struct _ATHUSB_STRING_DESC {
    ULONG           Index;
    UNICODE_STRING  UnicodeString[256];
}ATHUSB_STRING_DESC, *PATHUSB_STRING_DESC;

typedef struct _ATHUSB_SPIN_LOCK{
    A_UINT32        num;
    A_UINT32        label;
    KSPIN_LOCK      spinLock;
} ATHUSB_SPIN_LOCK, *PATHUSB_SPIN_LOCK;

typedef struct _ATHUSB_USB_ADAPTER {
    A_UINT8                         usbDeviceNum;
    VOID                            *pStubHandle;

    //Represents the physical device for the miniport driver
    PDEVICE_OBJECT                  pPhysicalDeviceObject;
    //Represents the functional device object that NDIS creates 
    //for the physical device
	PDEVICE_OBJECT                  pFunctionalDeviceObject;
    //Represents the next device object. This next device object is 
    //preceded in the chain by the functional device object that 
    //belongs to the miniport driver.
	PDEVICE_OBJECT                  pNextDeviceObject;

    // Device Descriptor
    PUSB_DEVICE_DESCRIPTOR          pUsbDeviceDescriptor;

    // Configuration Descriptor
    PUSB_CONFIGURATION_DESCRIPTOR   pUsbConfigurationDescriptor;

    // Interface Information structure
    PUSBD_INTERFACE_INFORMATION     pUsbInterface;

    // Pipe context for the bulkusb driver
    PATHUSB_PIPE_STATE              pPipeState;

    // configuration handle for the configuration the
    // device is currently in
    USBD_CONFIGURATION_HANDLE       configurationHandle;

    A_UINT32                        languageNum;
    A_UINT32                        stringDescNum;
    A_UINT16                        languageID[256];
    ATHUSB_STRING_DESC              stringDesc[256];

    // number of read pending irps for this stream
    A_UINT32                        readPendingIrps;
    // number of write pending irps for this stream
    A_UINT32                        writePendingIrps;

    // event signaled when no read irps pending
    KEVENT                          noPendingReadIrpEvent;
    // event signaled when no write irps pending
    KEVENT                          noPendingWriteIrpEvent;
    
    // number of read pending irps for the particular pipe
    A_UINT32                        pipeReadPending[MAX_PIPE_NUM];
    // number of write pending irps for the particular pipe
    A_UINT32                        pipeWritePending[MAX_PIPE_NUM];

    // event signaled when no read irps pending
    KEVENT                          noPendingPipeReadEvent[MAX_PIPE_NUM];
    // event signaled when no write irps pending
    KEVENT                          noPendingPipeWriteEvent[MAX_PIPE_NUM];

    PDEVICE_OBJECT                  deviceObject;        

    //Spin lock to access IN packet queue
    ATHUSB_SPIN_LOCK                readQueueLock;
    //If we use timer to send IN packet, this queue will maintain all waiting IRP
    LIST_ENTRY                      readWaitingQueue;
    //Link list for pending IN packet IRP
    LIST_ENTRY                      readPendingIrpQueue;
    //Link list for Idle IRP
    LIST_ENTRY                      readIdleIrpQueue;

    //Spin lock to access OUT packet queue
    ATHUSB_SPIN_LOCK                writeQueueLock;
    //If we use timer to send out packet, this queue will maintain all waiting IRP
    LIST_ENTRY                      writeWaitingQueue;
    //Link list for pending OUT packet IRP
    LIST_ENTRY                      writePendingIrpQueue;
    //Link list for Idle IRP
    LIST_ENTRY                      writeIdleIrpQueue;    

    A_INT8                          stackSize;
    A_UINT8                         recvEndPoints[MAX_PIPE_NUM];
    A_UINT8                         sendEndPoints[MAX_PIPE_NUM];
    //
    // statistics.
    //
    A_UINT32                        timesRecycled;
    A_UINT32                        totalPacketsProcessed;
    A_UINT32                        totalBytesProcessed;
    A_UINT32                        errorPacketCount;
    A_UINT32                        totalReadIrp;
    A_UINT32                        totalWriteIrp;

    RECV_INDICATION_HANDLER         recvIndHandler; 
    SEND_CONFIRM_HANDLER            sendCfmHandler;
    STATUS_INDICATION_HANDLER       statIndHandler;

    KDPC                            usbAdptDpc;
    KTIMER                          usbAdptTimer;
    KEVENT                          usbAdptDpcEvent;
    KEVENT                          noWorkItemPendingEvent;

    //This flag reflect to cancle pending IRP
    A_BOOL                          bCancel;
    //This flag reflect to stop/remove device
    A_BOOL                          bStop;
    A_BOOL                          IsDeviceHighSpeed;
    A_BOOL                          bWaitWakeEnable;
    
    A_BOOL                          bDeviceReset;

    A_BOOL                          bResetPipe[2][MAX_PIPE_NUM];
    A_BOOL                          bQueueWorkItem;

    A_BOOL                          bUseTimer;
    A_BOOL                          bQueueTimer;

    A_BOOL                          bReset;
    A_BOOL                          bSuspend;

    A_UINT8                         busState;

    //For testing purpose
    LARGE_INTEGER                   oldTime;
    A_UINT32                        minTime;
    A_UINT32                        maxTime;
    A_UINT32                        totalTime;
    A_UINT32                        totalCycle;
    A_BOOL                          bCount;
} ATHUSB_USB_ADAPTER, *PATHUSB_USB_ADAPTER;

typedef struct _ATHUSB_RXTX_OBJECT {

    LIST_ENTRY              link;
    A_BOOL                  bRead;

    PIRP                    irp;
    PURB                    urb;

    PUCHAR                  pDataBuffer;
    A_UINT32                dataBufferLen;

    A_UINT8                 pipeNum;
    PUSBD_PIPE_INFORMATION  pPipeInformation;        
    
    PATHUSB_USB_ADAPTER     pUsbAdapter;

} ATHUSB_RXTX_OBJECT, *PATHUSB_RXTX_OBJECT;

extern A_UINT32 usbDebugLevel;

//
// Debug levels: lower values indicate higher urgency
//
#define ATHUSB_EXTRA_LOUD       6
#define ATHUSB_LOUD             5
#define ATHUSB_LOCK             4
#define ATHUSB_INFO             3
#define ATHUSB_WARN             2
#define ATHUSB_ERROR            1

#if DBG

#define athUsbDbgPrint(level, _x_) \
            if((level) <= usbDebugLevel) { \
                DbgPrint _x_; \
            }

#else

#define athUsbDbgPrint(level, _x_)

#endif

#define ATHUSBTAG (ULONG) 'AthU'

#define AthExAllocatePool(type, size) \
    ExAllocatePoolWithTag(type, size, ATHUSBTAG);

NTSTATUS
athUsbTxIrpComplete(IN PDEVICE_OBJECT deviceObject,
                    IN PIRP           irp,
                    IN PVOID          context);

NTSTATUS
athUsbRxIrpComplete(IN PDEVICE_OBJECT deviceObject,
                    IN PIRP           irp,
                    IN PVOID          context);

VOID
athUsbDpcProc(
              IN   PKDPC    dpc,
              IN   PVOID    deferredContex,
              IN   PVOID    systemContex1,
              IN   PVOID    systemContex2);

#endif /* __ATHUSBDRV_H__ */
