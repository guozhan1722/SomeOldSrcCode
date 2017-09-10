/*++

Module Name:

    athfmw.h

Abstract:

Environment:

    Kernel mode

Notes:    

--*/

#ifndef _ATHUSB_FMW_H
#define _ATHUSB_FMW_H

#define MESSAGE_PIPE    0
#define DATA_PIPE       1

typedef struct _ATHUSB_FMDL_MSG {
    A_UINT32        msgLength;      /* Message buffer length*/
    A_UINT32        dataLegnth;     /* data buffer length */
    A_UINT32        totalLength;    /* total firmware length */
    A_UINT32        leftLength;     /* left data length */
    A_UINT32        msgBuffer[];    /* message body */
}ATHUSB_FMDL_MSG, *PATHUSB_FMDL_MSG;

#define FMDLMSG_HEADER_LENGTH (A_UINT32)(&((ATHUSB_FMDL_MSG *)0)->msgBuffer)
#define FMDLMSG_MAX_LENGTH       512
#define FMDLDATA_MAX_LENGTH      2048

typedef struct _ATHUSB_FMDL_OBJECT {        
    PDEVICE_OBJECT         deviceObject;
    VOID *                 pUsbAdapt;
    KEVENT                 readEvent;
    KEVENT                 writeEvent;
    KEVENT                 quitEvent;
    A_UINT8                totalInPipe;
    A_UINT8                totalOutPipe;
    A_UINT8                *fmInMsg;
    A_UINT8                *fmOutMsg; 
    A_UINT8                *fmData;
    A_UINT32               bytesSent;
    A_UINT32               bytesReceived;
    A_UINT32               totalLength;
    A_BOOL                 bFirst;
}ATHUSB_FMDL_OBJECT, *PATHUSB_FMDL_OBJECT;

BOOLEAN
athUsbFirmwareRead(IN PATHUSB_FMDL_OBJECT         firmwareObj,
                   IN A_UINT8                     pipeType,
                   IN A_UINT8                     *pBuf);

BOOLEAN
athUsbFirmwareWrite(IN PATHUSB_FMDL_OBJECT         firmwareObj,
                    IN A_UINT8                     pipeType,
                    IN A_UINT8                     *pBuf);
VOID
athUsbDownloadFirmware(IN PDEVICE_OBJECT  DeviceObject);

VOID
athUsbAbortDownload(IN PATHUSB_FMDL_OBJECT         firmwareObj);

VOID
fmwDldRecvIndication(IN  void     *pStubHandle,
                     IN  UCHAR     epNum,
                     IN  UCHAR    *buffer,
                     IN  ULONG     bytesReceived);

VOID
fmwDldSendConfirm(IN  void     *pStubHandle,
                  IN  UCHAR     epNum,
                  IN  UCHAR    *buffer,
                  IN  ULONG     bytesSent);

VOID
fmwDldStatusIndication(IN  void     *pStubHandle,                            
                       IN  ULONG    status);

VOID
fmwSwapHeader(PATHUSB_FMDL_MSG fwmMsg);

VOID
fmwFreeMemory(IN PATHUSB_FMDL_OBJECT  firmwareObj);

#endif
