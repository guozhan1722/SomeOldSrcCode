/*
 * Copyright (c) 2004 Atheros Communications, Inc., All Rights Reserved
 *
 * athusbapi.h -- Atheros USB Driver header file
 * 
 */

#ifndef __ATHUSB_API_H__
#define __ATHUSB_API_H__

#include <wdm.h>
#include "wlantype.h" 

#define ATHUSB_BUS_STATE_NORMAL             0
#define ATHUSB_BUS_STATE_ERROR              1
#define ATHUSB_BUS_STATE_SUSPEND            2
#define ATHUSB_BUS_STATE_SURPRISE_REMOVED   3

typedef VOID   ATHUSBDRV_HANDLE;
typedef ATHUSBDRV_HANDLE   *PATHUSBDRV_HANDLE;

//Received data from bulk endpoint
typedef void (*RECV_INDICATION_HANDLER)(
                            IN  void     *pStubHandle,
                            IN  A_UINT8   epNum,
                            IN  A_UINT8  *buffer,
                            IN  A_UINT32  bytesReceived
                        );

//transmit data complete from bulk endpoint
typedef void  (*SEND_CONFIRM_HANDLER)(
                            IN  void     *pStubHandle,
                            IN  A_UINT8   epNum,
                            IN  A_UINT8  *buffer,
                            IN  A_UINT32  bytesSent
                        );

//status notify routine
typedef void (*STATUS_INDICATION_HANDLER)(
                            IN  void     *pStubHandle,                            
                            IN  A_UINT32  status                             
                        );

//Initialize WDM lower edge.
extern A_STATUS
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
    );

//Notify WDM lower edge driver will quit/remove/stop after this point
extern A_STATUS 
athUsbDrvExit( 
    IN  A_UINT8                  usbDeviceNum,        
    IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle   
    );

//Request data from WDM lower edge
extern A_STATUS 
athUsbDrvReceive(IN  ATHUSBDRV_HANDLE     *pUsbDrvHandle,
                 IN  A_UINT8               epNum,
                 IN  A_UINT8              *pBuffer);

//Send data to WDM lower edge
extern A_STATUS 
athUsbDrvSend(IN  ATHUSBDRV_HANDLE  *pUsbDrvHandle,
              IN  A_UINT8            epNum,
              IN  A_UINT8           *pBuffer);

extern A_BOOL
athUsbDetectCardPresent(
                        IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle                        
                        );

extern A_STATUS
athUsbAbortAndReset( 
            IN  A_UINT8                  usbDeviceNum,        
            IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle   
            );

extern A_STATUS
athUsbSuspend( 
             IN  A_UINT8                  usbDeviceNum,        
             IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle   
             );

extern A_STATUS
athUsbResume( 
             IN  A_UINT8                  usbDeviceNum,        
             IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle   
             );

A_STATUS
athUsbSurpriseRemoved( 
                    IN  A_UINT8                  usbDeviceNum,        
                    IN  ATHUSBDRV_HANDLE        *pUsbDrvHandle   
                    );

extern A_UINT32 gepBufSize[2][10];
#endif /* __ATHUSBAPI_H__ */
