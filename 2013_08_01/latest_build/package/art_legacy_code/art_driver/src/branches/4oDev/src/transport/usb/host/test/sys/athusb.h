/*++

Module Name:

    athusb.h

Abstract:

Environment:

    Kernel mode

Notes:

    
--*/

#include <initguid.h>
#include <wdm.h>
#include <wmilib.h>
#include <wmistr.h>
#include "usbdrivr.h"

#ifndef _ATHUSB_H
#define _ATHUSB_H

#define ATHTAG (ULONG) 'AthU'

#undef ExAllocatePool
#define ExAllocatePool(type, size) \
    ExAllocatePoolWithTag(type, size, ATHTAG);

#if DBG

#define AthUsb_DbgPrint(level, _x_) \
            if((level) <= DebugLevel) { \
                DbgPrint _x_; \
            }

#else

#define AthUsb_DbgPrint(level, _x_)

#endif

typedef struct _GLOBALS {

    UNICODE_STRING AthUsb_RegistryPath;

} GLOBALS;

#define IDLE_INTERVAL 5000

typedef enum _DEVSTATE {

    NotStarted,         // not started
    Stopped,            // device stopped
    Working,            // started and working
    PendingStop,        // stop pending
    PendingRemove,      // remove pending
    SurpriseRemoved,    // removed by surprise
    Removed             // removed

} DEVSTATE;

typedef enum _QUEUE_STATE {

    HoldRequests,       // device is not started yet
    AllowRequests,      // device is ready to process
    FailRequests        // fail both existing and queued up requests

} QUEUE_STATE;

typedef enum _WDM_VERSION {

    WinXpOrBetter,
    Win2kOrBetter,
    WinMeOrBetter,
    Win98OrBetter

} WDM_VERSION;

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DeviceState =  NotStarted;\
        (_Data_)->PrevDevState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PrevDevState =  (_Data_)->DeviceState;\
        (_Data_)->DeviceState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DeviceState =   (_Data_)->PrevDevState;


#define ATHUSB_MAX_TRANSFER_SIZE   4096
#define ATHUSB_TEST_BOARD_TRANSFER_BUFFER_SIZE (64 *1024 )

//
// registry path used for parameters 
// global to all instances of the driver
//

#define ATHUSB_REGISTRY_PARAMETERS_PATH  \
	L"\\REGISTRY\\Machine\\System\\CurrentControlSet\\SERVICES\\ATHUSB\\Parameters"


typedef struct _ATHUSB_PIPE_CONTEXT {

    BOOLEAN PipeOpen;

} ATHUSB_PIPE_CONTEXT, *PATHUSB_PIPE_CONTEXT;

typedef struct _ATHUSB_STREAM_INFO {
    ULONG TimesRecycled;

    ULONG TotalPacketsProcessed;

    LONGLONG TotalBytesProcessed;

    ULONG ErrorPacketCount;
    ULONG ErrorSeqCount;
} ATHUSB_STREAM_INFO, *PATHUSB_STREAM_INFO;

typedef struct _MY_STRING_DESC {
    ULONG           Index;
    UNICODE_STRING  UnicodeString[256];
}MY_STRING_DESC, *PMY_STRING_DESC;

//
// A structure representing the instance information associated with
// this particular device.
//

typedef struct _DEVICE_EXTENSION {

    // Functional Device Object
    PDEVICE_OBJECT FunctionalDeviceObject;

    // Device object we call when submitting Urbs
    PDEVICE_OBJECT TopOfStackDeviceObject;

    // The bus driver object
    PDEVICE_OBJECT PhysicalDeviceObject;

    // Name buffer for our named Functional device object link
    // The name is generated based on the driver's class GUID
    UNICODE_STRING InterfaceName;

    // Bus drivers set the appropriate values in this structure in response
    // to an IRP_MN_QUERY_CAPABILITIES IRP. Function and filter drivers might
    // alter the capabilities set by the bus driver.
    DEVICE_CAPABILITIES DeviceCapabilities;

    // Device Descriptor
    PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor;

    // Configuration Descriptor
    PUSB_CONFIGURATION_DESCRIPTOR UsbConfigurationDescriptor;

    // Interface Information structure
    PUSBD_INTERFACE_INFORMATION UsbInterface;

    // Pipe context for the bulkusb driver
    PATHUSB_PIPE_CONTEXT PipeContext;

    // configuration handle for the configuration the
    // device is currently in
    USBD_CONFIGURATION_HANDLE ConfigurationHandle;

    // current state of device
    DEVSTATE DeviceState;

    // state prior to removal query
    DEVSTATE PrevDevState;

    // obtain and hold this lock while changing the device state,
    // the queue state and while processing the queue.
    KSPIN_LOCK DevStateLock;

    // current system power state
    SYSTEM_POWER_STATE SysPower;

    // current device power state
    DEVICE_POWER_STATE DevPower;

    // Pending I/O queue state
    QUEUE_STATE QueueState;

    // Pending I/O queue
    LIST_ENTRY NewRequestsQueue;

    // I/O Queue Lock
    KSPIN_LOCK QueueLock;

    KEVENT RemoveEvent;

    KEVENT StopEvent;
    
    ULONG OutStandingIO;

    KSPIN_LOCK IOCountLock;

    // selective suspend variables

    LONG SSEnable;

    LONG SSRegistryEnable;

    PUSB_IDLE_CALLBACK_INFO IdleCallbackInfo;
	
    PIRP PendingIdleIrp;
	
    LONG IdleReqPend;

    LONG FreeIdleIrpCount;

    KSPIN_LOCK IdleReqStateLock;

    KEVENT NoIdleReqPendEvent;

    // default power state to power down to on self-susped
    ULONG PowerDownLevel;
    
    // remote wakeup variables
    PIRP WaitWakeIrp;

    LONG FlagWWCancel;

    LONG FlagWWOutstanding;

    LONG WaitWakeEnable;

    // open handle count
    LONG OpenHandleCount;

    // selective suspend model uses timers, dpcs and work item.
    KTIMER Timer;

    KDPC DeferredProcCall;

    // This event is cleared when a DPC/Work Item is queued.
    // and signaled when the work-item completes.
    // This is essential to prevent the driver from unloading
    // while we have DPC or work-item queued up.
    KEVENT NoDpcWorkItemPendingEvent;

    // WMI information
    WMILIB_CONTEXT WmiLibInfo;

    // WDM version
    WDM_VERSION WdmVersion;

    // High speed
    ULONG IsDeviceHighSpeed;    

    ULONG              LanguageNum;
    ULONG              StringDescNum;
    USHORT             LanguageID[256];
    MY_STRING_DESC     StringDesc[256];

    ATHUSB_STREAM_INFO      RecStreamInfo;
    ATHUSB_STREAM_INFO      XferStreamInfo;
    BOOLEAN                 bZeroPacket;
    KDPC                    StreamDpc;  
    KTIMER                  StreamTimer;
    KEVENT                  StreamDpcEvent; 
    PVOID                   StreamObject;    
    PVOID                   StreamApiObject;    

    UCHAR                   RecvEndPoints[10];
    UCHAR                   SendEndPoints[10];

    PVOID                   FirmwareObject;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _IRP_COMPLETION_CONTEXT {

    PDEVICE_EXTENSION DeviceExtension;

    PKEVENT Event;

} IRP_COMPLETION_CONTEXT, *PIRP_COMPLETION_CONTEXT;

extern GLOBALS Globals;
extern ULONG DebugLevel;

#endif
