#include "precomp.h"

ULONG gulSeed;

/************************************************************************
* Cross-platform drivers should use this routine to check the WDM version 
* before performing any operations that vary by platform or are not 
* supported in all versions of WDM. 
**************************************************************************/
VOID
CheckOSVersion(IN PDEVICE_EXTENSION deviceExtension)
{
    if(IoIsWdmVersionAvailable(1, 0x20)) {

        deviceExtension->WdmVersion = WinXpOrBetter;
    }
    else if(IoIsWdmVersionAvailable(1, 0x10)) {

        deviceExtension->WdmVersion = Win2kOrBetter;
    }
    else if(IoIsWdmVersionAvailable(1, 0x5)) {

        deviceExtension->WdmVersion = WinMeOrBetter;
    }
    else if(IoIsWdmVersionAvailable(1, 0x0)) {

        deviceExtension->WdmVersion = Win98OrBetter;
    }
}

/*****************************************************************************
* Routine Description:
*
*    This routine queries the bus interface version
*
* Arguments:
*
*    DeviceExtension
*
* Return Value:
*
*    VOID
*****************************************************************************/
VOID
GetBusInterfaceVersion(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PIRP                       irp;
    KEVENT                     event;
    NTSTATUS                   ntStatus;
    PDEVICE_EXTENSION          deviceExtension;
    PIO_STACK_LOCATION         nextStack;
    USB_BUS_INTERFACE_USBDI_V1 busInterfaceVer1;

    //
    // initialize vars
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    AthUsb_DbgPrint(3, ("GetBusInterfaceVersion - begins\n"));

    irp = IoAllocateIrp(deviceExtension->TopOfStackDeviceObject->StackSize,
                        FALSE);

    if(NULL == irp) {

        AthUsb_DbgPrint(1, ("Failed to alloc irp in GetBusInterfaceVersion\n"));
        return;
    }

    //
    // All pnp Irp's need the status field initialized to
    // STATUS_NOT_SUPPORTED
    //
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(irp,
                           (PIO_COMPLETION_ROUTINE) IrpCompletionRoutine,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack);
    nextStack->MajorFunction = IRP_MJ_PNP;
    nextStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    //
    // Allocate memory for an interface of type
    // USB_BUS_INTERFACE_USBDI_V0 and have the IRP point to it:
    //
    nextStack->Parameters.QueryInterface.Interface = 
                                (PINTERFACE) &busInterfaceVer1;

    //
    // Assign the InterfaceSpecificData member of the IRP to be NULL
    //
    nextStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    //
    // Set the interface type to the appropriate GUID
    //
    nextStack->Parameters.QueryInterface.InterfaceType = 
                                        &USB_BUS_INTERFACE_USBDI_GUID;

    //
    // Set the size and version of interface in the IRP
    // Currently, there is only one valid version of 
    // this interface available to clients.
    //
    nextStack->Parameters.QueryInterface.Size = 
                                    sizeof(USB_BUS_INTERFACE_USBDI_V1);

    nextStack->Parameters.QueryInterface.Version = USB_BUSIF_USBDI_VERSION_1;
    
    AthUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                            irp);

    if(STATUS_PENDING == ntStatus) {

        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        ntStatus = irp->IoStatus.Status;
    }

    if(NT_SUCCESS(ntStatus)) {

        deviceExtension->IsDeviceHighSpeed = 
                busInterfaceVer1.IsDeviceHighSpeed(
                                       busInterfaceVer1.BusContext);

        AthUsb_DbgPrint(1, ("IsDeviceHighSpeed = %x\n", 
                            deviceExtension->IsDeviceHighSpeed));
    }

    IoFreeIrp(irp);

    AthUsb_DbgPrint(3, ("GetBusInterfaceVersion::"));
    AthUsb_IoDecrement(deviceExtension);

    AthUsb_DbgPrint(3, ("GetBusInterfaceVersion - ends\n"));
}

/*****************************************************************************
* Routine Description:
*
*    This routine is a completion routine.
*    In this routine we set an event.
*
*    Since the completion routine returns 
*    STATUS_MORE_PROCESSING_REQUIRED, the Irps,
*    which set this routine as the completion routine,
*    should be marked pending.
*
* Arguments:
*
*    DeviceObject - pointer to device object
*    Irp - I/O request packet
*    Context - 
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
IrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
{
    PKEVENT event = Context;

    if (Irp->PendingReturned) {
        KeSetEvent(event, 0, FALSE);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

/***************************************************************************** 
* Routine Description:
*
*    This routine bumps up the I/O count.
*    This routine is typically invoked when any of the
*    dispatch routines handle new irps for the driver.
*
* Arguments:
*
*    DeviceExtension - pointer to device extension
*
* Return Value:
*
*    new value
*****************************************************************************/
LONG
AthUsb_IoIncrement(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    )
{
    LONG  result = 0;
    KIRQL oldIrql;

    KeAcquireSpinLock(&DeviceExtension->IOCountLock, &oldIrql);

    result = InterlockedIncrement(&DeviceExtension->OutStandingIO);

    //
    // when OutStandingIO bumps from 1 to 2, clear the StopEvent
    //

    if(result == 2) {

        KeClearEvent(&DeviceExtension->StopEvent);
    }

    KeReleaseSpinLock(&DeviceExtension->IOCountLock, oldIrql);

    AthUsb_DbgPrint(3, ("AthUsb_IoIncrement::%d\n", result));

    return result;
}

/***************************************************************************** 
* Routine Description:
*
*    This routine decrements the outstanding I/O count
*    This is typically invoked after the dispatch routine
*    has finished processing the irp.
*
* Arguments:
*
*    DeviceExtension - pointer to device extension
*
* Return Value:
*
*    new value
*****************************************************************************/
LONG
AthUsb_IoDecrement(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    )
{
    LONG  result = 0;
    KIRQL oldIrql;

    KeAcquireSpinLock(&DeviceExtension->IOCountLock, &oldIrql);

    result = InterlockedDecrement(&DeviceExtension->OutStandingIO);

    if(result == 1) {

        KeSetEvent(&DeviceExtension->StopEvent, IO_NO_INCREMENT, FALSE);
    }

    if(result == 0) {

        ASSERT(Removed == DeviceExtension->DeviceState);

        KeSetEvent(&DeviceExtension->RemoveEvent, IO_NO_INCREMENT, FALSE);
    }

    KeReleaseSpinLock(&DeviceExtension->IOCountLock, oldIrql);

    AthUsb_DbgPrint(3, ("AthUsb_IoDecrement::%d\n", result));

    return result;
}

/************************************************************************
*																		
* Routine Description:													
*																		
*  Uses pentium RDTSC instruction to get current cycle count since		
*  chip was powered. This records a staring cycle count, and then		
*  call the stop routine to get a cycle count.							
*																		
*																		
* Arguments:															
*																		
*  cycleCount - this is where we will store the starting cycle count.	
*																		
* Return Value:														
*																		
*	VOID																
*																		
************************************************************************/
VOID
GetPentiumCycleCounter(PLARGE_INTEGER cycleCount)
{
    LARGE_INTEGER   startCount;

    _asm {
        // the RDTSC instruction is not really documented,
        // so assemblers may not support it, use _emit to
        // create instruction
        
        _emit	0Fh
        _emit	31h
        mov		startCount.HighPart, edx
        mov		startCount.LowPart,  eax
    }

    // here is our starting count
    *cycleCount = startCount;
} 

/************************************************************************
* Routine Description:
*
*    This routine reads the specified reqistry value.
*
* Arguments:
*
*    RegPath - registry path
*    ValueName - property to be fetched from the registry
*    Value - corresponding value read from the registry.
*
* Return Value:
*
*    NT status value
************************************************************************/
NTSTATUS
AthUsb_GetRegistryDword(
    IN     PWCHAR RegPath,
    IN     PWCHAR ValueName,
    IN OUT PULONG Value
    )
{
    ULONG                    defaultData = 0;
    WCHAR                    buffer[MAXIMUM_FILENAME_LENGTH];
    NTSTATUS                 ntStatus;
    UNICODE_STRING           regPath;
    RTL_QUERY_REGISTRY_TABLE paramTable[2];

    AthUsb_DbgPrint(3, ("AthUsb_GetRegistryDword - begins\n"));

    regPath.Length = 0;
    regPath.MaximumLength = MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);
    regPath.Buffer = buffer;

    RtlZeroMemory(regPath.Buffer, regPath.MaximumLength);
    RtlMoveMemory(regPath.Buffer,
                  RegPath,
                  wcslen(RegPath) * sizeof(WCHAR));

    RtlZeroMemory(paramTable, sizeof(paramTable));

    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = ValueName;
    paramTable[0].EntryContext = Value;
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &defaultData;
    paramTable[0].DefaultLength = sizeof(ULONG);

    ntStatus = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE |
                                      RTL_REGISTRY_OPTIONAL,
                                      regPath.Buffer,
                                      paramTable,
                                      NULL,
                                      NULL);

    if(NT_SUCCESS(ntStatus)) {

        AthUsb_DbgPrint(3, ("success Value = %X\n", *Value));
        return STATUS_SUCCESS;
    }
    else {

        *Value = 0;
        return STATUS_UNSUCCESSFUL;
    }
}
/************************************************************************ 
* Routine Description:
*
*    This routine synchronously submits an urb down the stack.
*
* Arguments:
*
*    DeviceObject - pointer to device object
*    Urb - USB request block
*
* Return Value:
************************************************************************/
NTSTATUS
CallUSBD(
    IN PDEVICE_OBJECT DeviceObject,
    IN PURB           Urb
    )
{
    PIRP               irp;
    KEVENT             event;
    NTSTATUS           ntStatus;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION  deviceExtension;

    //
    // initialize the variables
    //

    irp = NULL;
    deviceExtension = DeviceObject->DeviceExtension;
    
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_SUBMIT_URB, 
                                        deviceExtension->TopOfStackDeviceObject,
                                        NULL, 
                                        0, 
                                        NULL, 
                                        0, 
                                        TRUE, 
                                        &event, 
                                        &ioStatus);

    if(!irp) {

        AthUsb_DbgPrint(1, ("IoBuildDeviceIoControlRequest failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);
    nextStack->Parameters.Others.Argument1 = Urb;

    AthUsb_DbgPrint(3, ("CallUSBD::"));
    AthUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, irp);

    if(ntStatus == STATUS_PENDING) {

        KeWaitForSingleObject(&event, 
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);

        ntStatus = ioStatus.Status;
    }

    //
    // if ioStatus.Status indicates an error (ie. the IRP failed) then return that.
    // If ioStatus.Status indicates success, it is still possible that what we were
    // trying to do failed.  For example, if the IRP is cancelled, the status returned
    // by the I/O manager for the IRP will not indicate an error.  In that case, we
    // should check the URB status.  If it indicates anything other than
    // USBD_SUCCESS, then we should return STATUS_UNSUCCESSFUL.
    //
    if (NT_SUCCESS(ntStatus)) {
        if (!(USBD_SUCCESS(Urb->UrbHeader.Status)))
         ntStatus = STATUS_UNSUCCESSFUL;
   }

    AthUsb_DbgPrint(3, ("ntStatus = %x\n",ntStatus));
    AthUsb_DbgPrint(3, ("CallUSBD::\n"));

    AthUsb_IoDecrement(deviceExtension);
    return ntStatus;
}

PATHUSB_PIPE_CONTEXT
AthUsb_PipeWithName(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PUNICODE_STRING FileName
    )
/*++
 
Routine Description:

    This routine will pass the string pipe name and
    fetch the pipe number.

Arguments:

    DeviceObject - pointer to DeviceObject
    FileName - string pipe name

Return Value:

    The device extension maintains a pipe context for 
    the pipes on 82930 board.
    This routine returns the pointer to this context in
    the device extension for the "FileName" pipe.

--*/
{
    LONG                  ix;
    ULONG                 uval; 
    ULONG                 nameLength;
    ULONG                 umultiplier;
    PDEVICE_EXTENSION     deviceExtension;
    PATHUSB_PIPE_CONTEXT pipeContext;

    //
    // initialize variables
    //
    pipeContext = NULL;
    //
    // typedef WCHAR *PWSTR;
    //
    nameLength = (FileName->Length / sizeof(WCHAR));
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    AthUsb_DbgPrint(3, ("AthUsb_PipeWithName - begins\n"));

    if(nameLength != 0) {
    
        AthUsb_DbgPrint(3, ("Filename = %ws nameLength = %d\n", FileName->Buffer, nameLength));

        //
        // Parse the pipe#
        //
        ix = nameLength - 1;

        // if last char isn't digit, decrement it.
        while((ix > -1) &&
              ((FileName->Buffer[ix] < (WCHAR) '0')  ||
               (FileName->Buffer[ix] > (WCHAR) '9')))             {

            ix--;
        }

        if(ix > -1) {

            uval = 0;
            umultiplier = 1;

            // traversing least to most significant digits.

            while((ix > -1) &&
                  (FileName->Buffer[ix] >= (WCHAR) '0') &&
                  (FileName->Buffer[ix] <= (WCHAR) '9'))          {
        
                uval += (umultiplier *
                         (ULONG) (FileName->Buffer[ix] - (WCHAR) '0'));

                ix--;
                umultiplier *= 10;
            }

            if (deviceExtension->UsbInterface) {
                if(uval < (deviceExtension->UsbInterface)->NumberOfPipes
                    && deviceExtension->PipeContext) 
                {        
                    pipeContext = &deviceExtension->PipeContext[uval];
                }
            }
        }
    }

    AthUsb_DbgPrint(3, ("AthUsb_PipeWithName - ends\n"));

    return pipeContext;
}

ULONG
Rand(void)
{
	gulSeed = gulSeed*0x41c4e6d + 12345;
	return gulSeed & 0x7fffffff;
}

void 
GenRandData(CHAR	* pBuf, 
			ULONG	  dwLen)
{
    ULONG	i,dwData,dwDivd,dwCount,dwRand;

    dwData = dwLen / 4;
    dwDivd = dwLen % 4;

    dwCount = 0;
    if (dwData >= 1) {	
        for (i = 0; i < dwData; i ++) {
            dwRand = Rand();
            memcpy(&pBuf[dwCount], &dwRand,4);
            dwCount += 4;
        }
    }

    if (dwDivd > 0) {
        dwData = 1;
        for (i = 0; i < dwDivd; i ++) {
            dwData *= 0xFF;
        }

        dwRand = Rand() % dwData;
        dwDivd = dwLen - dwCount;
        memcpy(&pBuf[dwCount], &dwRand,dwDivd);
    }
}

NTSTATUS
AthUsb_BulkLatencyTest(IN PDEVICE_OBJECT  DeviceObject, LONG * TimeDiff)
/*++

  schedule an interrupt IN, when that goes off, schedule a bulk IN.
  measure the elapsed time from the time the interrupt completes and
  the time the bulk completes.

--*/
{
    PDEVICE_EXTENSION           deviceExtension;
    NTSTATUS                    ntStatus;
    PURB                        bulkUrb,intUrb;
    ULONG                       urbSize = 0 ,i;
    ULONG                       transferFlags = 0;
    PUSBD_INTERFACE_INFORMATION interfaceInfo;
    PUSBD_PIPE_INFORMATION      bulkPipeInfo = NULL;   
    PUSBD_PIPE_INFORMATION      intPipeInfo = NULL;      
    ULONG                       bulkBufferLength, intBufferLength;
    UCHAR                     * pBuffer;
    LARGE_INTEGER               cycleCount;
    LARGE_INTEGER               cycleTotal;
    PIRP                        bulkIrp,intIrp;
    KEVENT                      bulkEvent, intEvent;
    PIO_STACK_LOCATION          nextStack;
    IO_STATUS_BLOCK             ioStatus;
    LARGE_INTEGER               startCount, stopCount;
    LONG                        bulkTime,intTime;

    AthUsb_DbgPrint(3, ("AthUsb_BulkLatencyTest\n"));   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if(deviceExtension->UsbInterface) {
        interfaceInfo = deviceExtension->UsbInterface;
    }
    else {
        AthUsb_DbgPrint(1, ("UsbInterface not found\n"));
        return STATUS_UNSUCCESSFUL;
    }
      
    for (i = 0; i< interfaceInfo->NumberOfPipes; i ++) {
        if (interfaceInfo->Pipes[i].PipeType == UsbdPipeTypeBulk) {
            if (USB_ENDPOINT_DIRECTION_IN(interfaceInfo->Pipes[i].EndpointAddress)) {

                //find bulk in pipe!
                bulkPipeInfo = &interfaceInfo->Pipes[i];
                break;
            }
        }
        else if (interfaceInfo->Pipes[i].PipeType == UsbdPipeTypeInterrupt) {
            //find interrupt pipe
            intPipeInfo = &interfaceInfo->Pipes[i];
        }
    }
                
    if ((bulkPipeInfo == NULL) || (intPipeInfo == NULL)) {
        AthUsb_DbgPrint(1, ("Either we don't find bulk in pipe or interrupt pipe\n"));
        return STATUS_UNSUCCESSFUL;
    }
   
    bulkBufferLength = bulkPipeInfo->MaximumPacketSize;   
    intBufferLength = intPipeInfo->MaximumPacketSize;
    if (bulkBufferLength <= intBufferLength) {
        intBufferLength = bulkBufferLength;
    }
    else {
        bulkBufferLength = intBufferLength;
    }   
    
    pBuffer = ExAllocatePool(NonPagedPool, intBufferLength + 16);
    if (pBuffer == NULL) {

        AthUsb_DbgPrint(1, ("failed to alloc mem for receive bufer\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
   
    urbSize = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);

    //allocate urb for interrupt pipe
    intUrb = ExAllocatePool(NonPagedPool,urbSize);
    if (intUrb == NULL) {
        AthUsb_DbgPrint(1, ("failed to alloc mem for interrupt urb\n"));
        ExFreePool(pBuffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //allocate urb for bulk pipe
    bulkUrb = ExAllocatePool(NonPagedPool,urbSize);
    if (bulkUrb == NULL) {
        AthUsb_DbgPrint(1, ("failed to alloc mem for bulk urb\n"));
        ExFreePool(pBuffer);
        ExFreePool(intUrb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    transferFlags = USBD_SHORT_TRANSFER_OK | USBD_TRANSFER_DIRECTION_IN;

    //setup interrupt urb
    UsbBuildInterruptOrBulkTransferRequest(intUrb,        //ptr to urb
                               (USHORT) urbSize,          //size of urb
							   intPipeInfo->PipeHandle,   //usbd pipe handle
							   pBuffer,                   //TransferBuffer
							   NULL,                      //mdl
							   intBufferLength,           //bufferlength
							   transferFlags,             //flags
							   NULL);                     //link

    //Allocate interrupt IRP
    KeInitializeEvent(&intEvent, NotificationEvent, FALSE);

    intIrp = IoBuildDeviceIoControlRequest(
             IOCTL_INTERNAL_USB_SUBMIT_URB,
             deviceExtension->TopOfStackDeviceObject,
             NULL,
             0,
             NULL,
             0,
             TRUE, /* INTERNAL */
             &intEvent,
             &ioStatus);

    nextStack = IoGetNextIrpStackLocation(intIrp);
    nextStack->Parameters.Others.Argument1 = intUrb;   
    
    transferFlags = USBD_SHORT_TRANSFER_OK | USBD_TRANSFER_DIRECTION_IN;

    //setup bulk urb
    UsbBuildInterruptOrBulkTransferRequest(bulkUrb,        //ptr to urb
                              (USHORT) urbSize,            //size of urb
							   bulkPipeInfo->PipeHandle,   //usbd pipe handle
							   pBuffer,                    //TransferBuffer
							   NULL,                       //mdl
							   bulkBufferLength,           //bufferlength
							   transferFlags,              //flags
							   NULL);                      //link

    KeInitializeEvent(&bulkEvent, NotificationEvent, FALSE);

    //Allocate bulk IRP
    bulkIrp = IoBuildDeviceIoControlRequest(
              IOCTL_INTERNAL_USB_SUBMIT_URB,
              deviceExtension->TopOfStackDeviceObject,
              NULL,
              0,
              NULL,
              0,
              TRUE, /* INTERNAL */
              &bulkEvent,
              &ioStatus);

    nextStack = IoGetNextIrpStackLocation(bulkIrp);
    nextStack->Parameters.Others.Argument1 = bulkUrb;
   
    
    //start the performance counter
    GetPentiumCycleCounter(&startCount);

    // do the int transfer   
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,intIrp);
    if (ntStatus == STATUS_PENDING) {
        KeWaitForSingleObject(&intEvent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
    }

    //stop the performance counter
    GetPentiumCycleCounter(&stopCount);
    intTime = (LONG)(stopCount.QuadPart - startCount.QuadPart);
   
    // start the performance counter
    GetPentiumCycleCounter(&startCount);
   
    // do the bulk transfer   
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,bulkIrp);

    if (ntStatus == STATUS_PENDING) {
        KeWaitForSingleObject(&bulkEvent,
                            Executive,
                            KernelMode,
                            FALSE,
                            NULL);
    } 

    //stop the performance counter
    GetPentiumCycleCounter(&stopCount);

    bulkTime = (LONG)(stopCount.QuadPart - startCount.QuadPart);
    (*TimeDiff) = bulkTime - intTime;

    AthUsb_DbgPrint(3, ("Latency between bulk and interrupt is %d\n", (*TimeDiff)));

    ExFreePool(pBuffer);
    ExFreePool(intUrb);
    ExFreePool(bulkUrb);

    return STATUS_SUCCESS;
}

VOID Delay(ULONG uMilliSeconds)
{
	LARGE_INTEGER	CurTime,InitTime;	

	KeQuerySystemTime(&InitTime);

	while(1)
	{
		KeQuerySystemTime(&CurTime);
		if((CurTime.QuadPart - InitTime.QuadPart) > uMilliSeconds * 10000) break;
	}
}

NTSTATUS
AthUsb_VendorRequest(IN PDEVICE_OBJECT  DeviceObject,
                     IN PVENDOR_REQUEST pVendorRequest)
{
    NTSTATUS            ntStatus        = STATUS_SUCCESS;
    PURB                urb             = NULL;
    ULONG               length          = 0;
    ULONG               i, siz;
    PUCHAR              buffer = NULL;
    
    AthUsb_DbgPrint(3, ("AthUsb_VendorRequest - begins\n"));

    siz = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
    urb = ExAllocatePool(NonPagedPool,siz);

    if (pVendorRequest->wLength) {
        buffer = pVendorRequest->pData;
    }

    if (urb) {
        RtlZeroMemory(urb,sizeof(struct  _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));

        UsbBuildVendorRequest(urb,               //ptr to urb
				    URB_FUNCTION_VENDOR_DEVICE,  //vendor-defined request for a USB device 
					(USHORT)siz,                 //size of urb
                    pVendorRequest->direction,   //transferFlags
					0,                           //RequestTypeReservedBits         
					pVendorRequest->bRequest,    //request
					pVendorRequest->wValue,      //Value
					pVendorRequest->wIndex,      //index
					buffer,                      //transferBuffer
					NULL,                        //mdl(unused)
					pVendorRequest->wLength,     //transferBufferLength
					NULL);                       //link

        ntStatus = CallUSBD(DeviceObject, urb);
    } else {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ExFreePool(urb);

    AthUsb_DbgPrint(3, ("AthUsb_VendorRequest - ends\n"));
    return ntStatus;
}
                     
