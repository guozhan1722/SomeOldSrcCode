/*
 * Copyright (c) 2004 Atheros Communications, Inc., All Rights Reserved
 *
 * athusbconfig.c -- Atheros USB Driver configuration file
 * 
 */
#define INITGUID
#include "athusbapi.h"
#include "athusbdrv.h"
#include "athusbconfig.h"
#include "athusbRxTx.h"

#define TIMEOUT_IN_SECOND   5l
/************************************************************************ 
* Routine Description:
*
*    This routine synchronously submits an urb down the stack.
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    urb - USB request block
*
* Return Value:
************************************************************************/
NTSTATUS
athUsbCallUSBD(
    IN PATHUSB_USB_ADAPTER  pUsbAdapter,
    IN PURB                 urb
    )
{
    PIRP               irp;
    KEVENT             event;
    NTSTATUS           ntStatus;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION nextStack; 
    LARGE_INTEGER      timeOut;

    //
    // initialize the variables
    //

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUSbCallUSBD - begin"));

    irp = NULL;    
    
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_SUBMIT_URB, 
                                        pUsbAdapter->pNextDeviceObject,
                                        NULL, 
                                        0, 
                                        NULL, 
                                        0, 
                                        TRUE, 
                                        &event, 
                                        &ioStatus);

    if (!irp) {

        athUsbDbgPrint(ATHUSB_ERROR, ("IoBuildDeviceIoControlRequest failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }   

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);
    nextStack->Parameters.Others.Argument1 = urb;    

    ntStatus = IoCallDriver(pUsbAdapter->pNextDeviceObject, irp);

    if (ntStatus == STATUS_PENDING) {

        timeOut.QuadPart = -(LONGLONG) 10l*1000l*1000l*TIMEOUT_IN_SECOND;
        ntStatus = KeWaitForSingleObject(&event, 
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              &timeOut);

        if (ntStatus == STATUS_TIMEOUT) {
            athUsbDbgPrint(ATHUSB_ERROR, 
                ("Hardware has no response for %d second\n",TIMEOUT_IN_SECOND));
            return STATUS_UNSUCCESSFUL;
        }

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
        if (!(USBD_SUCCESS(urb->UrbHeader.Status)))
         ntStatus = STATUS_UNSUCCESSFUL;
   }

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("ntStatus = %x\n",ntStatus));
    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbCallUSBD - ends\n"));
    
    return ntStatus;
}

/*****************************************************************************
* Routine Description:
*
*    This routine queries the bus interface version
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*
* Return Value:
*
*    VOID
*****************************************************************************/
VOID
athUsbGetBusVersion(
    IN PATHUSB_USB_ADAPTER  pUsbAdapter
    )
{
    PIRP                       irp;
    KEVENT                     event;
    NTSTATUS                   ntStatus;    
    PIO_STACK_LOCATION         nextStack;
    USB_BUS_INTERFACE_USBDI_V1 busInterfaceVer1;
    LARGE_INTEGER              timeOut;

    //
    // initialize vars
    //  

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbGetBusVersion - begins\n"));

    irp = IoAllocateIrp(pUsbAdapter->pNextDeviceObject->StackSize,
                        FALSE);

    if (NULL == irp) {

        athUsbDbgPrint(ATHUSB_ERROR, ("Failed to alloc irp in GetBusInterfaceVersion\n"));
        return;
    }

    //
    // All pnp Irp's need the status field initialized to
    // STATUS_NOT_SUPPORTED
    //
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(irp,
                           (PIO_COMPLETION_ROUTINE) irpCompletionRoutine,
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
                                        (LPGUID)&USB_BUS_INTERFACE_USBDI_GUID;

    //
    // Set the size and version of interface in the IRP
    // Currently, there is only one valid version of 
    // this interface available to clients.
    //
    nextStack->Parameters.QueryInterface.Size = 
                                    sizeof(USB_BUS_INTERFACE_USBDI_V1);

    nextStack->Parameters.QueryInterface.Version = USB_BUSIF_USBDI_VERSION_1;

    ntStatus = IoCallDriver(pUsbAdapter->pNextDeviceObject,
                            irp);

    if (STATUS_PENDING == ntStatus) {

        timeOut.QuadPart = -(LONGLONG) 10l*1000l*1000l*TIMEOUT_IN_SECOND;
        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              &timeOut);

        if (ntStatus == STATUS_TIMEOUT) {
            IoFreeIrp(irp);
            athUsbDbgPrint(ATHUSB_ERROR, 
                ("Hardware has no response for %d second\n",TIMEOUT_IN_SECOND));
            return;
        }

        ntStatus = irp->IoStatus.Status;
    }

    if (NT_SUCCESS(ntStatus)) {

        pUsbAdapter->IsDeviceHighSpeed = 
                busInterfaceVer1.IsDeviceHighSpeed(
                                       busInterfaceVer1.BusContext);

        athUsbDbgPrint(ATHUSB_ERROR, ("IsDeviceHighSpeed = %x\n", 
                            pUsbAdapter->IsDeviceHighSpeed));
    }

    IoFreeIrp(irp);

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbGetBusVersion - ends\n"));
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
*    DeviceObject - Pointer to usb adapter handle
*    Irp - I/O request packet
*    Context - 
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
irpCompletionRoutine(
    IN PDEVICE_OBJECT deviceObject,
    IN PIRP           irp,
    IN PVOID          context
    )
{
    PKEVENT event = context;

    if (irp->PendingReturned) {
        KeSetEvent(event, 0, FALSE);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

/*****************************************************************************
* Routine Description:
*
*    Send Vendor Specific request over USB bus
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    pVendorRequest - vendor request
*
* Return Value:
*
*    NT status value
*****************************************************************************/

NTSTATUS
athUsbVendorRequest(IN PATHUSB_USB_ADAPTER      pUsbAdapter,
                    IN PATHUSB_VENDOR_REQUEST   pVendorRequest)
{
    NTSTATUS            ntStatus        = STATUS_SUCCESS;
    PURB                urb             = NULL;
    A_UINT32            length          = 0;
    A_UINT32            i, siz;
    PUCHAR              buffer = NULL;
    
    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbVendorRequest - begins\n"));

    siz = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
    urb = AthExAllocatePool(NonPagedPool,siz);

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

        ntStatus = athUsbCallUSBD(pUsbAdapter, urb);
    } else {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ExFreePool(urb);

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbVendorRequest - ends\n"));
    return ntStatus;
}

/*****************************************************************************
* Routine Description:
*
*    Send Vendor Specific reset over USB bus to tell target device host
*    want to re-synchronize with device
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
athUsbVendorReset(IN PATHUSB_USB_ADAPTER  pUsbAdapter)
{
    ATHUSB_VENDOR_REQUEST         vendorRequest;
    
    //reset the devie to clear all buffer
    vendorRequest.bRequest = ATHUSB_RESET_FEATURE;
    vendorRequest.direction = 0;
    vendorRequest.wValue = ATHUSB_RESET_DEVICE;
    vendorRequest.wIndex = 0;
    vendorRequest.wLength = 0;

    return athUsbVendorRequest(pUsbAdapter, &vendorRequest);
}

/***************************************************************************** 
* Routine Description:
*
*    This routine configures the USB device.
*    In this routines we get the device descriptor, 
*    the configuration descriptor and select the
*    configuration descriptor.
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*
* Return Value:
*
*    NTSTATUS - NT status value.
*****************************************************************************/
NTSTATUS
athUsbReadandSelectDescriptors(
                               IN PATHUSB_USB_ADAPTER  pUsbAdapter)
{
    PURB                   urb;
    A_UINT32               siz;
    NTSTATUS               ntStatus;
    PUSB_DEVICE_DESCRIPTOR deviceDescriptor;    
    
    //
    // initialize variables
    //

    urb = NULL;
    deviceDescriptor = NULL;    

    //
    // 1. Read the device descriptor
    //

    urb = AthExAllocatePool(NonPagedPool, 
                         sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if (urb) {

        siz = sizeof(USB_DEVICE_DESCRIPTOR);
        deviceDescriptor = AthExAllocatePool(NonPagedPool, siz);

        if (deviceDescriptor) {

            UsbBuildGetDescriptorRequest(
                    urb, 
                    (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                    USB_DEVICE_DESCRIPTOR_TYPE, 
                    0, 
                    0, 
                    deviceDescriptor, 
                    NULL, 
                    siz, 
                    NULL);

            ntStatus = athUsbCallUSBD(pUsbAdapter, urb);

            if (NT_SUCCESS(ntStatus)) {

                ASSERT(deviceDescriptor->bNumConfigurations);
                athUsbDbgPrint(ATHUSB_INFO, ("Device Descriptor = %x, len %x\n",
                                deviceDescriptor,
                                urb->UrbControlDescriptorRequest.TransferBufferLength));

                athUsbDbgPrint(ATHUSB_INFO, ("Atheros USB Device Descriptor:\n"));
                athUsbDbgPrint(ATHUSB_INFO, ("-------------------------\n"));
                athUsbDbgPrint(ATHUSB_INFO, ("bLength %d\n", deviceDescriptor->bLength));
                athUsbDbgPrint(ATHUSB_INFO, ("bDescriptorType 0x%x\n", deviceDescriptor->bDescriptorType));
                athUsbDbgPrint(ATHUSB_INFO, ("bcdUSB 0x%x\n", deviceDescriptor->bcdUSB));
                athUsbDbgPrint(ATHUSB_INFO, ("bDeviceClass 0x%x\n", deviceDescriptor->bDeviceClass));
                athUsbDbgPrint(ATHUSB_INFO, ("bDeviceSubClass 0x%x\n", deviceDescriptor->bDeviceSubClass));
                athUsbDbgPrint(ATHUSB_INFO, ("bDeviceProtocol 0x%x\n", deviceDescriptor->bDeviceProtocol));
                athUsbDbgPrint(ATHUSB_INFO, ("bMaxPacketSize0 0x%x\n", deviceDescriptor->bMaxPacketSize0));
                athUsbDbgPrint(ATHUSB_INFO, ("idVendor 0x%x\n", deviceDescriptor->idVendor));
                athUsbDbgPrint(ATHUSB_INFO, ("idProduct 0x%x\n", deviceDescriptor->idProduct));
                athUsbDbgPrint(ATHUSB_INFO, ("bcdDevice 0x%x\n", deviceDescriptor->bcdDevice));
                athUsbDbgPrint(ATHUSB_INFO, ("iManufacturer 0x%x\n", deviceDescriptor->iManufacturer));
                athUsbDbgPrint(ATHUSB_INFO, ("iProduct 0x%x\n", deviceDescriptor->iProduct));
                athUsbDbgPrint(ATHUSB_INFO, ("iSerialNumber 0x%x\n", deviceDescriptor->iSerialNumber));
                athUsbDbgPrint(ATHUSB_INFO, ("bNumConfigurations 0x%x\n", deviceDescriptor->bNumConfigurations));                

                pUsbAdapter->pUsbDeviceDescriptor = deviceDescriptor;

                ntStatus = athUsbConfigureDevice(pUsbAdapter);    
            } else {
                athUsbDbgPrint(ATHUSB_ERROR, ("\n Failed to Get deviceDescriptor !!!\n"));
            }
                            
            ExFreePool(urb);                
        } else {

            athUsbDbgPrint(ATHUSB_ERROR, ("Failed to allocate memory for deviceDescriptor\n"));

            ExFreePool(urb);
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {

        athUsbDbgPrint(ATHUSB_ERROR, ("Failed to allocate memory for urb\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

	//We can start to download firmware from here.
    return ntStatus;
}

/*****************************************************************************
* Routine Description:
*
*    This helper routine reads the configuration descriptor
*    for the device in couple of steps.
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*
* Return Value:
*
*    NTSTATUS - NT status value
*****************************************************************************/
NTSTATUS
athUsbConfigureDevice(
                      IN PATHUSB_USB_ADAPTER  pUsbAdapter)
{
    PURB                          urb;
    A_UINT32                      siz;
    NTSTATUS                      ntStatus;    
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor;

    //
    // initialize the variables
    //

    urb = NULL;
    configurationDescriptor = NULL;    

    //
    // Read the first configuration descriptor
    // This requires two steps:
    // 1. Read the fixed sized configuration desciptor (CD)
    // 2. Read the CD with all embedded interface and endpoint descriptors
    //

    urb = AthExAllocatePool(NonPagedPool, 
                         sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if (urb) {

        siz = sizeof(USB_CONFIGURATION_DESCRIPTOR);
        configurationDescriptor = AthExAllocatePool(NonPagedPool, siz);

        if (configurationDescriptor) {

            UsbBuildGetDescriptorRequest(
                    urb, 
                    (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                    USB_CONFIGURATION_DESCRIPTOR_TYPE, 
                    0, 
                    0, 
                    configurationDescriptor,
                    NULL, 
                    sizeof(USB_CONFIGURATION_DESCRIPTOR), 
                    NULL);

            ntStatus = athUsbCallUSBD(pUsbAdapter, urb);

            if (!NT_SUCCESS(ntStatus)) {

                athUsbDbgPrint(ATHUSB_ERROR, ("\n Failed to Get configurationDescriptor !!!\n"));
                goto ConfigureDevice_Exit;
            }
        } else {

            athUsbDbgPrint(ATHUSB_ERROR, ("Failed to allocate mem for config Descriptor\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto ConfigureDevice_Exit;
        }

        // Determine how much data is in the entire configuration descriptor
        // and add extra room to protect against accidental overrun
        siz = configurationDescriptor->wTotalLength + 16;

        //  Free up the data buffer memory just used
        ExFreePool(configurationDescriptor);

        configurationDescriptor = AthExAllocatePool(NonPagedPool, siz);

        if (configurationDescriptor) {

            UsbBuildGetDescriptorRequest(
                    urb, 
                    (USHORT)sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                    USB_CONFIGURATION_DESCRIPTOR_TYPE,
                    0, 
                    0, 
                    configurationDescriptor, 
                    NULL, 
                    siz, 
                    NULL);

            ntStatus = athUsbCallUSBD(pUsbAdapter, urb);

            if (!NT_SUCCESS(ntStatus)) {

                athUsbDbgPrint(ATHUSB_ERROR, ("\n Failed to Get configurationDescriptor !!!\n"));
                goto ConfigureDevice_Exit;
            }
        } else {

            athUsbDbgPrint(ATHUSB_ERROR, ("Failed to alloc mem for config Descriptor\n"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto ConfigureDevice_Exit;
        }
    } else {

        athUsbDbgPrint(ATHUSB_ERROR, ("Failed to allocate memory for urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto ConfigureDevice_Exit;
    }

    if (configurationDescriptor) {

        //
        // save a copy of configurationDescriptor in deviceExtension
        // remember to free it later.
        //
        pUsbAdapter->pUsbConfigurationDescriptor = configurationDescriptor;        

        if (configurationDescriptor->bmAttributes & ATHUSB_REMOTE_WAKEUP_MASK) {
            //
            // this configuration supports remote wakeup
            //
            pUsbAdapter->bWaitWakeEnable = 1;
        } else {
            pUsbAdapter->bWaitWakeEnable = 0;
        }

        ntStatus = athUsbSelectInterfaces(pUsbAdapter, configurationDescriptor);
    } else {

        pUsbAdapter->pUsbConfigurationDescriptor = NULL;
    }

ConfigureDevice_Exit:

    if (urb) {

        ExFreePool(urb);
    }

    return ntStatus;
}

/***************************************************************************** 
* Routine Description:
*
*    This helper routine selects the configuration
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    ConfigurationDescriptor - pointer to the configuration
*    descriptor for the device
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
athUsbSelectInterfaces(
    IN PATHUSB_USB_ADAPTER           pUsbAdapter,
    IN PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor)
{
    A_INT32                     numberOfInterfaces, 
                                interfaceNumber, 
                                interfaceindex;
    A_UINT32                    i;
    PURB                        urb;
    PUCHAR                      pInf;
    NTSTATUS                    ntStatus;
    PUSB_INTERFACE_DESCRIPTOR   interfaceDescriptor;
    PUSBD_INTERFACE_LIST_ENTRY  interfaceList, 
                                tmp;
    PUSBD_INTERFACE_INFORMATION interfaceInfo;

    //
    // initialize the variables
    //

    urb = NULL;
    interfaceInfo = NULL;
    interfaceDescriptor = NULL;
    numberOfInterfaces = configurationDescriptor->bNumInterfaces;
    interfaceindex = interfaceNumber = 0;

    //
    // Parse the configuration descriptor for the interface;
    //

    tmp = interfaceList =
        AthExAllocatePool(
               NonPagedPool, 
               sizeof(USBD_INTERFACE_LIST_ENTRY) * (numberOfInterfaces + 1));

    if (!tmp) {

        athUsbDbgPrint(ATHUSB_ERROR, ("Failed to allocate mem for interfaceList\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    while (interfaceNumber < numberOfInterfaces) {

        interfaceDescriptor = USBD_ParseConfigurationDescriptorEx(
                                            configurationDescriptor, 
                                            configurationDescriptor,
                                            interfaceindex,
                                            0, -1, -1, -1);

        if (interfaceDescriptor) {

            interfaceList->InterfaceDescriptor = interfaceDescriptor;            

            interfaceList->Interface = NULL;
            interfaceList++;
            interfaceNumber++;
        }

        interfaceindex++;
    }

    interfaceList->InterfaceDescriptor = NULL;
    interfaceList->Interface = NULL;
    urb = USBD_CreateConfigurationRequestEx(configurationDescriptor, tmp);

    if (urb) {

        interfaceInfo = &urb->UrbSelectConfiguration.Interface;

        for (i = 0; i < interfaceInfo->NumberOfPipes; i++) {

            //
            // perform pipe initialization here
            // set the transfer size and any pipe flags we use
            // USBD sets the rest of the Interface struct members
            //

            interfaceInfo->Pipes[i].MaximumTransferSize = 
                                ATHUSB_DEFAULT_MAXIMUM_SIZE;
        }

        ntStatus = athUsbCallUSBD(pUsbAdapter, urb);

        if (NT_SUCCESS(ntStatus)) {

            // Save the configuration handle for this device
            pUsbAdapter->configurationHandle =
                            urb->UrbSelectConfiguration.ConfigurationHandle;

            //
            // save a copy of interface information in the device extension.
            //
            pUsbAdapter->pUsbInterface = AthExAllocatePool(NonPagedPool,
                                                        interfaceInfo->Length);

            if (pUsbAdapter->pUsbInterface) {
                
                RtlCopyMemory(pUsbAdapter->pUsbInterface,
                              interfaceInfo,
                              interfaceInfo->Length);
            } else {

                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                athUsbDbgPrint(ATHUSB_ERROR, ("memory alloc for UsbInterface failed\n"));
            }

            //
            // Dump the interface to the debugger
            //

            interfaceInfo = &urb->UrbSelectConfiguration.Interface;

            athUsbDbgPrint(ATHUSB_INFO, ("---------\n"));
            athUsbDbgPrint(ATHUSB_INFO, ("NumberOfPipes 0x%x\n", 
                                 interfaceInfo->NumberOfPipes));
            athUsbDbgPrint(ATHUSB_INFO, ("Length 0x%x\n", 
                                 interfaceInfo->Length));
            athUsbDbgPrint(ATHUSB_INFO, ("Alt Setting 0x%x\n", 
                                 interfaceInfo->AlternateSetting));
            athUsbDbgPrint(ATHUSB_INFO, ("Interface Number 0x%x\n", 
                                 interfaceInfo->InterfaceNumber));
            athUsbDbgPrint(ATHUSB_INFO, ("Class, subclass, protocol 0x%x 0x%x 0x%x\n",
                                 interfaceInfo->Class,
                                 interfaceInfo->SubClass,
                                 interfaceInfo->Protocol));
            //
            // Initialize the pipeState
            // Dump the pipe info
            //

            if (interfaceInfo->NumberOfPipes) {
                pUsbAdapter->pPipeState = 
                    AthExAllocatePool(NonPagedPool,
                                   interfaceInfo->NumberOfPipes *
                                   sizeof(ATHUSB_PIPE_STATE));

                if (pUsbAdapter->pPipeState) {
                
                    for (i=0; i<interfaceInfo->NumberOfPipes; i++) {

                        pUsbAdapter->pPipeState[i].pipeOpen = FALSE;
                    }
                } else {
                    
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    athUsbDbgPrint(ATHUSB_ERROR, ("memory alloc for UsbInterface failed\n"));
                }
            }

            for (i = 0; i < interfaceInfo->NumberOfPipes; i ++) {

                athUsbDbgPrint(ATHUSB_INFO, ("---------\n"));
                athUsbDbgPrint(ATHUSB_INFO, ("PipeType 0x%x\n", 
                                     interfaceInfo->Pipes[i].PipeType));
                athUsbDbgPrint(ATHUSB_INFO, ("EndpointAddress 0x%x\n", 
                                     interfaceInfo->Pipes[i].EndpointAddress));
                athUsbDbgPrint(ATHUSB_INFO, ("MaxPacketSize 0x%x\n", 
                                    interfaceInfo->Pipes[i].MaximumPacketSize));
                athUsbDbgPrint(ATHUSB_INFO, ("Interval 0x%x\n", 
                                     interfaceInfo->Pipes[i].Interval));
                athUsbDbgPrint(ATHUSB_INFO, ("Handle 0x%x\n", 
                                     interfaceInfo->Pipes[i].PipeHandle));
                athUsbDbgPrint(ATHUSB_INFO, ("MaximumTransferSize 0x%x\n", 
                                    interfaceInfo->Pipes[i].MaximumTransferSize));
            }

            athUsbDbgPrint(ATHUSB_INFO, ("---------\n"));
        } else {

            athUsbDbgPrint(ATHUSB_ERROR, ("Failed to select an interface\n"));
        }
    } else {
        
        athUsbDbgPrint(ATHUSB_ERROR, ("USBD_CreateConfigurationRequestEx failed\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (tmp) {

        ExFreePool(tmp);
    }

    if (urb) {

        ExFreePool(urb);
    }

    return ntStatus;
}

/***************************************************************************** 
* Routine Description:
*
*    This routine is invoked when the device is removed or stopped.
*    This routine de-configures the usb device.
*
*Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*
*Return Value:
*
*    NT status value
*
*****************************************************************************/
NTSTATUS
athUsbDeconfigureDevice(
    IN PATHUSB_USB_ADAPTER           pUsbAdapter)
{
    PURB         urb;
    A_UINT32     siz;
    NTSTATUS     ntStatus;
    
    //
    // initialize variables
    //

    siz = sizeof(struct _URB_SELECT_CONFIGURATION);
    urb = AthExAllocatePool(NonPagedPool, siz);

    if (urb) {

        UsbBuildSelectConfigurationRequest(urb, (USHORT)siz, NULL);

        ntStatus = athUsbCallUSBD(pUsbAdapter, urb);

        if (!NT_SUCCESS(ntStatus)) {

            athUsbDbgPrint(ATHUSB_ERROR, ("Failed to deconfigure device\n"));
        }

        ExFreePool(urb);
    } else {

        athUsbDbgPrint(ATHUSB_ERROR, ("Failed to allocate urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

/***************************************************************************** 
* Routine Description:
*
*    This routine try to get all string descriptor
*
*Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*
*Return Value:
*
*    NT status value
*
*****************************************************************************/
VOID
athUsbGetAllStringDesciptor(
    IN PATHUSB_USB_ADAPTER           pUsbAdapter)
{    
    PUSB_DEVICE_DESCRIPTOR          pDeviceDescriptor;
    PUSB_CONFIGURATION_DESCRIPTOR   pConfigurationDescriptor;
    PUSB_INTERFACE_DESCRIPTOR       pInterfaceDescriptor;
    
    pDeviceDescriptor = pUsbAdapter->pUsbDeviceDescriptor;
    pConfigurationDescriptor = pUsbAdapter->pUsbConfigurationDescriptor; 
    pInterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)
                                ((UCHAR *)pConfigurationDescriptor + 
                                pConfigurationDescriptor->bLength);

    if (pDeviceDescriptor->iManufacturer) {
        athUsbGetString(pUsbAdapter,pDeviceDescriptor->iManufacturer);
    }

    if (pDeviceDescriptor->iProduct) {
        athUsbGetString(pUsbAdapter,pDeviceDescriptor->iProduct);
    }

    if (pDeviceDescriptor->iSerialNumber) {
        athUsbGetString(pUsbAdapter,pDeviceDescriptor->iSerialNumber);
    }

    if (pConfigurationDescriptor->iConfiguration) {
        athUsbGetString(pUsbAdapter,pConfigurationDescriptor->iConfiguration);
    }

    if (pInterfaceDescriptor->iInterface) {
        athUsbGetString(pUsbAdapter,pInterfaceDescriptor->iInterface);
    }
}

/***************************************************************************** 
* Routine Description:
*
*    This routine try to get specific string descriptor
*
*Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    index       - string descriptor index
*
*Return Value:
*
*    NT status value
*
*****************************************************************************/
NTSTATUS
athUsbGetString(
    IN PATHUSB_USB_ADAPTER          pUsbAdapter,
    A_UCHAR                         index)
{    
    UCHAR                 buf[256], *pch;
    A_UINT32              length,i;
    PATHUSB_STRING_DESC   pMyStringDesc;
    BOOLEAN               bFound = FALSE;

    pUsbAdapter->languageNum = 0;
    length = athUsbGetStringDescriptor(pUsbAdapter,0,English,buf, sizeof(buf));
    if (length) {
        pUsbAdapter->languageNum = (length - 2) / 2;
        pch = buf + 2;
        for (i = 0; i < pUsbAdapter->languageNum; i ++) {
            RtlCopyMemory(&pUsbAdapter->languageID[i],pch, 2);
            pch += 2;            
        }
    }

    if (pUsbAdapter->languageNum == 0) return STATUS_UNSUCCESSFUL;    

    pMyStringDesc = &pUsbAdapter->stringDesc[pUsbAdapter->stringDescNum];
    pMyStringDesc->Index = index;

    for (i = 0; i < pUsbAdapter->languageNum; i ++) {
        length = athUsbGetStringDescriptor(pUsbAdapter,
                                           index,
                                           pUsbAdapter->languageID[i],
                                           buf, sizeof(buf));
        if (length == 0) {
            continue;
        }

        bFound = TRUE;

        pMyStringDesc->UnicodeString[i].MaximumLength = (USHORT)length + sizeof(UNICODE_NULL);
        pMyStringDesc->UnicodeString[i].Length = (USHORT)length;
        pMyStringDesc->UnicodeString[i].Buffer = AthExAllocatePool(PagedPool,
                                    pMyStringDesc->UnicodeString[i].MaximumLength);

        if (pMyStringDesc->UnicodeString[i].Buffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(pMyStringDesc->UnicodeString[i].Buffer, 
                      pMyStringDesc->UnicodeString[i].MaximumLength);

        RtlCopyMemory(pMyStringDesc->UnicodeString[i].Buffer, 
                      &buf[2],length - 2);
    }

    if (bFound) {
        pUsbAdapter->stringDescNum ++;
    }

    return STATUS_SUCCESS;
}
                                    

/***************************************************************************** 
* Routine Description:
*    Gets the specified string descriptor from the given device object
*    
* Arguments:
*    DeviceObject    - pointer to the sample device object
*    Index           - Index of the string descriptor
*    LanguageId      - Language ID of the string descriptor
*    pvOutputBuffer  - pointer to the buffer where the data is to be placed
*    ulLength        - length of the buffer
*
* Return Value:
*    Number of valid bytes in data buffer
******************************************************************************/
A_UINT32
athUsbGetStringDescriptor(
    IN PATHUSB_USB_ADAPTER          pUsbAdapter,
    IN A_UCHAR                      index,
    IN USHORT                       languageId,
    IN OUT PVOID                    pvOutputBuffer,
    IN A_UINT32                     ulLength
    )
{
   NTSTATUS            ntStatus        = STATUS_SUCCESS;
   PURB                urb             = NULL;
   A_UINT32            length          = 0;

   athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("Enter AthUsb_GetStringDescriptor\n"));    

   urb = AthExAllocatePool(NonPagedPool, 
                      sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
                         
   if (urb) {
      if (pvOutputBuffer) {    

         UsbBuildGetDescriptorRequest(urb,
                                      (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                      USB_STRING_DESCRIPTOR_TYPE,    //descriptor type
                                      index,                         //index
                                      languageId,                    //language ID
                                      pvOutputBuffer,                //transfer buffer
                                      NULL,                          //MDL
                                      ulLength,                      //buffer length
                                      NULL);                         //link
                                                                  
         ntStatus = athUsbCallUSBD(pUsbAdapter, urb);
         if (!NT_SUCCESS(ntStatus)) {
             athUsbDbgPrint(ATHUSB_ERROR, ("\n Failed to Get stringDescriptor %d!!!\n",index));
         }

      } else {
         ntStatus = STATUS_INVALID_PARAMETER;
      }    

      // If successful, get the length from the Urb, otherwise set length to 0
      if (NT_SUCCESS(ntStatus)) {
         length = urb->UrbControlDescriptorRequest.TransferBufferLength;
      } else {
         length = 0;
      }

      athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("%d bytes of string descriptor received\n",length));

      ExFreePool(urb);        
   } else {
      ntStatus = STATUS_NO_MEMORY;        
   }        
    
   athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("Leaving Ezusb_GetStringDescriptor\n"));    

   return length;
}

/***************************************************************************** 
* Routine Description:
*
*    This routine returns all the memory allocations acquired during
*    device startup. 
*    
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*        
*    
* Return Value:
*
*    STATUS_SUCCESS if the device can be safely removed, an appropriate 
*    NT Status if not.
*******************************************************************************/
NTSTATUS
athUsbReleaseMemory(
    IN PATHUSB_USB_ADAPTER          pUsbAdapter)
{
    //
    // Disconnect from the interrupt and unmap any I/O ports
    //    
    A_UINT32                 i,j;
    PATHUSB_STRING_DESC      pMyStringDesc;        

    if (pUsbAdapter->pUsbDeviceDescriptor) {

        ExFreePool(pUsbAdapter->pUsbDeviceDescriptor);
        pUsbAdapter->pUsbDeviceDescriptor = NULL;
    }

    if (pUsbAdapter->pUsbConfigurationDescriptor) {

        ExFreePool(pUsbAdapter->pUsbConfigurationDescriptor);
        pUsbAdapter->pUsbConfigurationDescriptor = NULL;
    }

    if (pUsbAdapter->pUsbInterface) {
        
        ExFreePool(pUsbAdapter->pUsbInterface);
        pUsbAdapter->pUsbInterface = NULL;
    }

    if (pUsbAdapter->pPipeState) {

        ExFreePool(pUsbAdapter->pPipeState);
        pUsbAdapter->pPipeState = NULL;
    }

    for (i = 0; i < pUsbAdapter->stringDescNum; i ++) {
        pMyStringDesc = &pUsbAdapter->stringDesc[i];
        for (j = 0; j < pUsbAdapter->languageNum; j ++) {
            if (pMyStringDesc->UnicodeString[j].Buffer) {
                ExFreePool(pMyStringDesc->UnicodeString[j].Buffer);
                pMyStringDesc->UnicodeString[j].Buffer = NULL;
            }
        }
    }    

    return STATUS_SUCCESS;
}

/*****************************************************************************  
* Routine Description:
*
*    This routine synchronously submits a URB_FUNCTION_RESET_PIPE
*    request down the stack.
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    PipeInfo - pointer to PipeInformation structure
*               to retrieve the pipe handle
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
athUsbResetPipe(
    IN PATHUSB_USB_ADAPTER          pUsbAdapter,
    IN PUSBD_PIPE_INFORMATION       pipeInfo
    )
{
    PURB              urb;
    NTSTATUS          ntStatus;
    A_UINT32          portStatus;

    //
    // initialize variables
    //

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbResetPipe - success\n"));

    urb = NULL;

    ntStatus = athUsbGetPortStatus(pUsbAdapter, &portStatus);

    if ((NT_SUCCESS(ntStatus))             &&
       (portStatus & USBD_PORT_ENABLED)    &&
       (portStatus & USBD_PORT_CONNECTED)) 
    {
        urb = AthExAllocatePool(NonPagedPool, 
                         sizeof(struct _URB_PIPE_REQUEST));

        if(urb) {

            urb->UrbHeader.Length = (USHORT) sizeof(struct _URB_PIPE_REQUEST);
            urb->UrbHeader.Function = URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL;
            urb->UrbPipeRequest.PipeHandle = pipeInfo->PipeHandle;

            ntStatus = athUsbCallUSBD(pUsbAdapter, urb);

            ExFreePool(urb);
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (NT_SUCCESS(ntStatus)) {    
            athUsbDbgPrint(ATHUSB_INFO, ("athUsbResetPipe - success\n"));
            ntStatus = STATUS_SUCCESS;
        } else {

            athUsbDbgPrint(ATHUSB_ERROR, ("athUsbResetPipe - failed\n"));
        }
    } else {
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    athUsbDbgPrint(ATHUSB_EXTRA_LOUD, ("athUsbResetPipe - success\n"));

    return ntStatus;
}

/*****************************************************************************
* Routine Description
*
*    Reset all pipe
*
* Arguments:
*
*    DeviceObject - Pointer to usb adapter handle
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
athUsbResetAllPipes(IN PATHUSB_USB_ADAPTER      pUsbAdapter)
{
    
    A_UINT32                    i;          
    PUSBD_INTERFACE_INFORMATION pInterfaceInfo;
    PUSBD_PIPE_INFORMATION      pPipeInformation;
    NTSTATUS                    ntStatus;

    //
    // initialize variables
    //        
    pInterfaceInfo = pUsbAdapter->pUsbInterface;
    
    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbResetAllPipes - begins\n"));
    
    if (pInterfaceInfo == NULL) {

        return STATUS_SUCCESS;
    }

    for (i = 0; i < pInterfaceInfo->NumberOfPipes; i ++) {

        pPipeInformation = &(pInterfaceInfo->Pipes[i]);

        if (pPipeInformation) {
            ntStatus = athUsbResetPipe(pUsbAdapter,pPipeInformation);
    
            if (!NT_SUCCESS(ntStatus)) {
            
                athUsbDbgPrint(ATHUSB_ERROR, ("athUsbResetPipe failed\n"));

                ntStatus = athUsbResetDevice(pUsbAdapter);
            }
        }
    }

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbResetAllPipes - ends\n"));

    return STATUS_SUCCESS;
}
/***************************************************************************** 
* Routine Description:
*
*    This routine invokes AthUsb_ResetParentPort to reset the device
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
athUsbResetDevice(
    IN PATHUSB_USB_ADAPTER          pUsbAdapter)
{
    NTSTATUS    ntStatus;
    A_UINT32    portStatus;

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbResetDevice - begins\n"));

    ntStatus = athUsbGetPortStatus(pUsbAdapter, &portStatus);

    if(((NT_SUCCESS(ntStatus))                 &&
       (!(portStatus & USBD_PORT_ENABLED))    &&
       (portStatus & USBD_PORT_CONNECTED))) 
    {

        ntStatus = athUsbResetParentPort(pUsbAdapter);
    }

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbResetDevice - ends\n"));

    return ntStatus;
}

/***************************************************************************** 
* Routine Description:
*
*    This routine retrieves the status value
*
* Arguments:
*
*    pUsbAdapter - Pointer to usb adapter handle
*    PortStatus - port status
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
athUsbGetPortStatus(
    IN PATHUSB_USB_ADAPTER      pUsbAdapter,
    IN OUT PULONG               PortStatus
    )
{
    NTSTATUS           ntStatus;
    KEVENT             event;
    PIRP               irp;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION nextStack;
    LARGE_INTEGER      timeOut;

    //
    // initialize variables
    //    
    *PortStatus = 0;

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbGetPortStatus - begins\n"));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                    IOCTL_INTERNAL_USB_GET_PORT_STATUS,
                    pUsbAdapter->pNextDeviceObject,
                    NULL,
                    0,
                    NULL,
                    0,
                    TRUE,
                    &event,
                    &ioStatus);

    if (NULL == irp) {

        athUsbDbgPrint(ATHUSB_ERROR, ("memory alloc for irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nextStack = IoGetNextIrpStackLocation(irp);

    ASSERT(nextStack != NULL);

    nextStack->Parameters.Others.Argument1 = PortStatus;

    ntStatus = IoCallDriver(pUsbAdapter->pNextDeviceObject, irp);

    if (STATUS_PENDING == ntStatus) {
        timeOut.QuadPart = -(LONGLONG) 10l*1000l*1000l*TIMEOUT_IN_SECOND;
        KeWaitForSingleObject(&event, 
                               Executive, 
                               KernelMode, 
                               FALSE, 
                               &timeOut);
        if (ntStatus == STATUS_TIMEOUT) {
            athUsbDbgPrint(ATHUSB_ERROR, 
                ("Hardware has no response for %d second\n",TIMEOUT_IN_SECOND));
            return STATUS_UNSUCCESSFUL;
        }
    }
    else {

        ioStatus.Status = ntStatus;
    }

    ntStatus = ioStatus.Status;

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbGetPortStatus - ends\n"));

    return ntStatus;
}

/*****************************************************************************  
* Routine Description:
*
*    This routine sends an IOCTL_INTERNAL_USB_RESET_PORT
*    synchronously down the stack.
*
* Arguments:
*
* Return Value:
*****************************************************************************/
NTSTATUS
athUsbResetParentPort(
    IN PATHUSB_USB_ADAPTER      pUsbAdapter
    )
{
    NTSTATUS           ntStatus;
    KEVENT             event;
    PIRP               irp;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION nextStack;
    LARGE_INTEGER      timeOut;

    //
    // initialize variables
    //

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbResetParentPort - begins\n"));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                    IOCTL_INTERNAL_USB_RESET_PORT,
                    pUsbAdapter->pNextDeviceObject,
                    NULL,
                    0,
                    NULL,
                    0,
                    TRUE,
                    &event,
                    &ioStatus);

    if (NULL == irp) {

        athUsbDbgPrint(ATHUSB_ERROR, ("memory alloc for irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nextStack = IoGetNextIrpStackLocation(irp);

    ASSERT(nextStack != NULL);

    ntStatus = IoCallDriver(pUsbAdapter->pNextDeviceObject, irp);

    if (STATUS_PENDING == ntStatus) {
        timeOut.QuadPart = -(LONGLONG) 10l*1000l*1000l*TIMEOUT_IN_SECOND;
        ntStatus = KeWaitForSingleObject(&event, 
                                          Executive, 
                                          KernelMode, 
                                          FALSE, 
                                          &timeOut);
        if (ntStatus == STATUS_TIMEOUT) {
            athUsbDbgPrint(ATHUSB_ERROR, 
                ("Hardware has no response for %d second\n",TIMEOUT_IN_SECOND));
            return STATUS_UNSUCCESSFUL;
        }
    }
    else {

        ioStatus.Status = ntStatus;
    }

    ntStatus = ioStatus.Status;

    athUsbDbgPrint(ATHUSB_LOUD, ("AthUsb_ResetParentPort - ends\n"));

    return ntStatus;
}

/*****************************************************************************
* Routine Description
*
*    sends an abort pipe request for open pipes.
*
* Arguments:
*
*    DeviceObject - Pointer to usb adapter handle
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
athUsbAbortPipes(IN PATHUSB_USB_ADAPTER      pUsbAdapter)
{
    PURB                        urb;
    A_UINT32                    i;
    NTSTATUS                    ntStatus;    
    PATHUSB_PIPE_STATE          pPipeState;
    PUSBD_PIPE_INFORMATION      pipeInformation;
    PUSBD_INTERFACE_INFORMATION interfaceInfo;

    //
    // initialize variables
    //    
    pPipeState = pUsbAdapter->pPipeState;
    interfaceInfo = pUsbAdapter->pUsbInterface;
    
    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbAbortPipes - begins\n"));
    
    if (interfaceInfo == NULL || pPipeState == NULL) {

        return STATUS_SUCCESS;
    }

    for (i = 0; i < interfaceInfo->NumberOfPipes; i ++) {

        if (pPipeState[i].pipeOpen) {

            athUsbDbgPrint(ATHUSB_LOUD, ("Aborting open pipe %d\n", i));
    
            urb = AthExAllocatePool(NonPagedPool,
                                 sizeof(struct _URB_PIPE_REQUEST));

            if (urb) {

                urb->UrbHeader.Length = sizeof(struct _URB_PIPE_REQUEST);
                urb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
                urb->UrbPipeRequest.PipeHandle = 
                                        interfaceInfo->Pipes[i].PipeHandle;

                ntStatus = athUsbCallUSBD(pUsbAdapter, urb);

                ExFreePool(urb);
            } else {

                athUsbDbgPrint(ATHUSB_ERROR, ("Failed to alloc memory for urb\n"));

                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                return ntStatus;
            }

            if (NT_SUCCESS(ntStatus)) {

                pPipeState[i].pipeOpen = FALSE;
            }
        }
    }

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbAbortPipes - ends\n"));

    return STATUS_SUCCESS;
}

/*****************************************************************************
* Routine Description
*
*    Reset pipe state
*
* Arguments:
*
*    DeviceObject - Pointer to usb adapter handle
*
* Return Value:
*
*    NT status value
*****************************************************************************/
NTSTATUS
athUsbResetPipeState(IN PATHUSB_USB_ADAPTER      pUsbAdapter)
{
    
    A_UINT32                    i;      
    PATHUSB_PIPE_STATE          pPipeState;    
    PUSBD_INTERFACE_INFORMATION interfaceInfo;

    //
    // initialize variables
    //    
    pPipeState = pUsbAdapter->pPipeState;
    interfaceInfo = pUsbAdapter->pUsbInterface;
    
    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbResetPipeState - begins\n"));
    
    if (interfaceInfo == NULL || pPipeState == NULL) {

        return STATUS_SUCCESS;
    }

    for (i = 0; i < interfaceInfo->NumberOfPipes; i ++) {

        if (pPipeState[i].pipeOpen) {
            pPipeState[i].pipeOpen = FALSE;            
        }
    }

    athUsbDbgPrint(ATHUSB_LOUD, ("athUsbResetPipeState - ends\n"));

    return STATUS_SUCCESS;
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
getPentiumCycleCounter(PLARGE_INTEGER cycleCount)
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