/*
 * Copyright (c) 2004 Atheros Communications, Inc., All Rights Reserved
 *
 * athusrxtx.h -- Atheros USB Driver header file
 * 
 */
#ifndef ATHUSB_RXTX_H
#define ATHUSB_RXTX_H

#define ATHUSB_MAX_IRP_NUM                  50

#define athAcquireSpinLock(pAthLock, pOldIrql)         do {         \
    InterlockedIncrement(&((pAthLock)->num));                       \
    athUsbDbgPrint(ATHUSB_LOCK,                                     \
         ("==>"__FUNCTION__"() line%d: Acquire lock%d, num = %d\n", \
                     __LINE__,(pAthLock)->label,(pAthLock)->num));  \
    if ((*pOldIrql) < DISPATCH_LEVEL) {                             \
        KeAcquireSpinLock(&((pAthLock)->spinLock), pOldIrql);       \
    } else {                                                        \
        KeAcquireSpinLockAtDpcLevel(&((pAthLock)->spinLock));       \
    }                                                               \
} while(0)

#define athReleaseSpinLock(pAthLock, newIrql)         do {          \
    InterlockedDecrement(&((pAthLock)->num));                       \
    athUsbDbgPrint(ATHUSB_LOCK,                                     \
         ("==>"__FUNCTION__"() line%d: Release lock%d, num = %d\n", \
                   __LINE__,(pAthLock)->label,(pAthLock)->num));    \
    if (newIrql < DISPATCH_LEVEL) {                                 \
        KeReleaseSpinLock(&((pAthLock)->spinLock), newIrql);        \
    } else {                                                        \
        KeReleaseSpinLockFromDpcLevel(&((pAthLock)->spinLock));     \
    }                                                               \
} while(0)

typedef struct _ATHUSB_WORKER_THREAD_CONTEXT{
    PATHUSB_USB_ADAPTER             pUsbAdapter;    
    PIO_WORKITEM                    workItem;
    PUSBD_PIPE_INFORMATION          pipeInformation;
    A_BOOL                          bResetDevice;
} ATHUSB_WORKER_THREAD_CONTEXT, *PATHUSB_WORKER_THREAD_CONTEXT;

NTSTATUS
athUsbAllocTransferObj(IN PATHUSB_USB_ADAPTER   pUsbAdapter,
                       IN BOOLEAN               bRead);

VOID 
athUsbQueueIdleIrp(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                   IN PATHUSB_RXTX_OBJECT     pTransferObject,
                   IN BOOLEAN                 bRead);

VOID
athUsbInsertPendingIrp(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                      IN PATHUSB_RXTX_OBJECT     pTransferObject,
                      IN BOOLEAN                 bRead);

NTSTATUS
athUsbRemoveIdleIrp(IN PATHUSB_USB_ADAPTER      pUsbAdapter,
                    IN OUT PATHUSB_RXTX_OBJECT  *ppTransferObject,
                    IN BOOLEAN                  bRead);

NTSTATUS
athUsbAllocIrpUrbPool(IN PATHUSB_USB_ADAPTER   pUsbAdapter);

VOID
athUsbFreeIrpUrbPool(IN PATHUSB_USB_ADAPTER     pUsbAdapter);

VOID
athUsbCancelAllIrp(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                   IN A_BOOL                  bAbort);

VOID
athUsbCancelIrp(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                IN A_UINT8                 epNum,
                IN A_BOOL                  bRead);

NTSTATUS
athUsbInitTransferUrb(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                      IN PATHUSB_RXTX_OBJECT     pTransferObject);

NTSTATUS
athUsbInitTransferIrp(IN PATHUSB_USB_ADAPTER     pUsbAdapter,
                      IN PATHUSB_RXTX_OBJECT     pTransferObject);

NTSTATUS
athUsbProcessTransfer(IN PATHUSB_RXTX_OBJECT     pTransferObject,
                      IN OUT USBD_STATUS         *pUsbdStatus);

VOID
athUsbDrainWaitQueue(IN PATHUSB_USB_ADAPTER     pUsbAdapter);

NTSTATUS
athUsbQueueWorkItem(IN PATHUSB_USB_ADAPTER     pUsbAdapter);

VOID
athUsbWorkerItemCallback(IN  PDEVICE_OBJECT  DeviceObject,
                         IN  PVOID           Context);


#endif