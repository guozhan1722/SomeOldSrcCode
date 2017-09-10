/*
 * anwiwdm.c
 * This is the main file for the anwi kernel mode driver. 
 *
 * Revisions:
 */
#define INITGUID	// initialze WDM_GUID in this module
#include <initguid.h>
#include <ntddk.h>
// #include <wdm.h>
#include "anwi.h"
#include "anwiwdm.h"
#include "anwiclient.h"
#include "anwi_guid.h"

// Forward decl
static void anwiWDMUnload (PDRIVER_OBJECT pDriverObject);
static NTSTATUS anwiWDMCreate (PDEVICE_OBJECT pDevObj, PIRP pIrp);
static NTSTATUS anwiWDMClose (PDEVICE_OBJECT pDevObj, PIRP pIrp);
static NTSTATUS anwiWDMRead (PDEVICE_OBJECT pDevObj, PIRP pIrp);
static NTSTATUS anwiWDMWrite (PDEVICE_OBJECT pDevObj, PIRP pIrp);
static NTSTATUS anwiWDMPnp (PDEVICE_OBJECT pDevObj, PIRP pIrp);
static NTSTATUS anwiWDMPower(PDEVICE_OBJECT pDevObj, PIRP pIrp);
static NTSTATUS anwiAddDevice(PDRIVER_OBJECT, PDEVICE_OBJECT);
static NTSTATUS anwiRemoveDevice(PDEVICE_OBJECT pDevObj, PIRP pIrp);
static NTSTATUS anwiStartDevice(PDEVICE_OBJECT pDevObj, PIRP pIrp);
static NTSTATUS anwiStopDevice(PDEVICE_OBJECT pDevObj, PIRP pIrp);
static NTSTATUS anwiSurpriseRemoval(PDEVICE_OBJECT pDevObj, PIRP pIrp);
static NTSTATUS passDownPnp(PDEVICE_OBJECT pDevObj, PIRP pIrp);
static NTSTATUS forwardAndWait (PDEVICE_OBJECT pDevObj, PIRP pIrp);

extern NTSTATUS anwiWDMIoctl (PDEVICE_OBJECT pDevObj, PIRP pIrp);

/*
 * The WDM driver loads at windows init time and
 * does not know which hardware it would be controlling
 * until it is associated with a client. 
 *
 * Arguments:
 *		pDriverObject - Driver Object Passed from I/O Manager
 *		pRegistryPath - UNICODE_STRING pointer to
 *				registry info (service key)
 *				for this driver
 *
 * Return value:
 *		NTSTATUS signaling success or failure
 */

NTSTATUS DriverEntry 
(
	PDRIVER_OBJECT pDriverObject,
	PUNICODE_STRING pRegistryPath
) 
{
	NTSTATUS status = STATUS_SUCCESS;
#ifdef ANWI_NO_UNLOAD
	PDEVICE_EXTENSION pDevExt;
	PDEVICE_OBJECT pDevObj;
#endif

	KdPrint(("Anwi %d.%d \n",ANWI_MAJOR_VERSION,ANWI_MINOR_VERSION));

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::DriverEntry \n"));
#endif

#ifdef ANWI_NO_UNLOAD
	// Create a dummy device to prevent unloading the driver 
	// once it is loaded
	status = IoCreateDevice (pDriverObject,
	                         sizeof(DEVICE_EXTENSION),
	                         NULL,
	                         FILE_DEVICE_UNKNOWN,
	                         0,
	                         FALSE,
	                         &pDevObj);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	pDevExt = pDevObj->DeviceExtension;
	pDevExt->dummy = 1;
#endif // ANWI_NO_UNLOAD
	
	pDriverObject->DriverUnload = anwiWDMUnload;
	pDriverObject->DriverExtension->AddDevice = anwiAddDevice;

	status = initClientTable();

	return status;
}


// anwiWDMUnload 
// 
// Arguments:
//		pDriverObject - pointer to driver object
//
//	Description
//		This function is called when the driver is stopped.
//
// Returns:
//		None. (VOID)

static void anwiWDMUnload 
(
	PDRIVER_OBJECT pDriverObject
) 
{
	PDEVICE_OBJECT pDevObj;
#ifdef ANWI_DEBUG
	KdPrint(("Anwi::anwiWDMunload \n"));
#endif
	cleanupClientTable();
	
#ifdef ANWI_NO_UNLOAD
	pDevObj = pDriverObject->DeviceObject;
	if (pDevObj) {
		IoDeleteDevice(pDevObj);
	}
#endif
	
	return;
}


/*
* anwiWDMCreate 
*
* Arguments:
*		PDEVICE_OBJECT - Device object
*		PIrp - Irp with input parameters
* - Create device Dispatch routine.
*
* Return 
* 	NTSTATUS - Operation status.
*
*/

static NTSTATUS anwiWDMCreate 
(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
) 
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pDevExt;

	pDevExt = pDevObj->DeviceExtension;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi:: anwiWDMCreate::devicenumber =  %d\n", pDevExt->deviceNumber));
#endif
	
	status = registerClient(pDevExt->deviceNumber);
	
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;

	IoCompleteRequest (pIrp, IO_NO_INCREMENT);

	return status;
}

/*
* anwiWDMClose - Close device Dispatch routine.
*
* There is a self reset mechanism implemented in
* the close IRP call. Here we check if the number
* of open instances are less than the number of
* active clients, if so, a self reset is issued.
* 
*
* Arguments:
*		PDEVICE_OBJECT - Device object
*		PIrp - Irp with input parameters
*
* Return 
* 	NTSTATUS - Operation status.
*
*/

static NTSTATUS anwiWDMClose (PDEVICE_OBJECT pDevObj, PIRP pIrp) 
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pDevExt;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi:: WDMclose \n"));
#endif
	pDevExt = pDevObj->DeviceExtension;
	
	unregisterClient(pDevExt->deviceNumber);
	
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;

	IoCompleteRequest (pIrp, IO_NO_INCREMENT);

	return status;
}

/*
 * anwiWDMRead - Read device Dispatch routine.
 *
 * Arguments:
 *		PDEVICE_OBJECT - Device object
 *		PIrp - Irp with input parameters
 *
 *	Description:
 *		Currently not used
 *
 * Return 
 * 		NTSTATUS - Operation status.
 *
 */

static NTSTATUS anwiWDMRead 
(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
)
{
	NTSTATUS status = STATUS_SUCCESS;

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest (pIrp, IO_NO_INCREMENT);

	return status;
}

/*
 * anwiWDMWrite - Write device Dispatch routine.
 *
 * Arguments:
 *		PDEVICE_OBJECT - Device object
 *		PIrp - Irp with input parameters
 *
 *	Decription:
 *		Currently not used.
 *
 * Return 
 * 	NTSTATUS - Operation status.
 *
 */

static NTSTATUS anwiWDMWrite 
(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
)
{
	NTSTATUS status = STATUS_SUCCESS;

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest (pIrp, IO_NO_INCREMENT);

	return status;
}

// Pass down the request for now.

static NTSTATUS anwiWDMPower
(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pDevExt;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi:: WDMPower \n"));
#endif

	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	PoStartNextPowerIrp(pIrp);
	IoSkipCurrentIrpStackLocation(pIrp);
	status = PoCallDriver(pDevExt->pLowerDevice,pIrp);

	return status;
}

static NTSTATUS anwiWDMPnp 
(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack;
	PDEVICE_EXTENSION pDevExt;
	

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::WDMPNP\n"));
#endif

	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

	switch(pIrpStack->MinorFunction) {
		case IRP_MN_START_DEVICE:
			status = anwiStartDevice(pDevObj,pIrp);
			break;
		case IRP_MN_STOP_DEVICE:
			status = anwiStopDevice(pDevObj,pIrp);
			break;
		case IRP_MN_REMOVE_DEVICE:
			status = anwiRemoveDevice(pDevObj,pIrp);
			break;
		case IRP_MN_SURPRISE_REMOVAL:
			status = anwiSurpriseRemoval(pDevObj,pIrp);
			break;
		default:
			KdPrint(("Anwi::Unsupported PNP minor function %d \n",pIrpStack->MinorFunction));
			status = passDownPnp(pDevObj,pIrp);
			break;
	
	}


	return status;
}

static NTSTATUS anwiAddDevice
(	
	PDRIVER_OBJECT pDriverObject, 
	PDEVICE_OBJECT pDo
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;
	LONG32 deviceNumber;
	WCHAR hwid[256];
	ULONG len;
	ULONG iIndex;
	ULONG uart_client=0;
	ULONG uart_search_index=0;
	WCHAR uart_str[5];
	ULONG uart_str_len = 4;
	
	uart_str[0] = 'U';
	uart_str[1] = 'A';
	uart_str[2] = 'R';
	uart_str[3] = 'T';

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::AddDevice\n"));
#endif
	
	status = IoCreateDevice (pDriverObject,
	                         sizeof(DEVICE_EXTENSION),
				 NULL,
	                         FILE_DEVICE_UNKNOWN,
	                         0,
	                         FALSE,
	                         &pDevObj);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	// Grab our device extension's pointer
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->dummy = 0;

	status = IoGetDeviceProperty(pDo, DevicePropertyDeviceDescription , sizeof(hwid), hwid, &len);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice (pDevObj);
		return status;
	}

	for(iIndex=0; iIndex<len; iIndex++) {
		KdPrint(("%c", hwid[iIndex]));
		if (hwid[iIndex] == uart_str[uart_search_index]) {
		  uart_search_index ++;
		  if (uart_search_index == uart_str_len) break;
		}
		else {
		  uart_search_index = 0;
		}
	}
#ifdef ANWI_DEBUG
	KdPrint(("\nuart_srch_idx=%d\n", uart_search_index ));
#endif

	if (uart_search_index == uart_str_len) {
		KdPrint(("Registering anwi uart\n"));
		deviceNumber = findNextClientId(UART_FN_DEV_START_NUM);
		status = IoRegisterDeviceInterface(pDo, &ANWI_UART_GUID, NULL, &(pDevExt->symLinkName));
	}
	else {
		KdPrint(("Registering anwi wmac\n"));
		deviceNumber = findNextClientId(0);
		status = IoRegisterDeviceInterface(pDo, &ANWI_GUID, NULL, &(pDevExt->symLinkName));
	}

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Device number = %d \n",deviceNumber));
#endif

	if (deviceNumber < 0) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	if (!NT_SUCCESS(status)) {
	KdPrint(("IoRegister failure\n"));
		IoDeleteDevice (pDevObj);
		return status;
	}

	// Say to the IO manager we want only buffered IRPs
	pDevObj->Flags |= DO_BUFFERED_IO;

	// Now initialize our device's extension
	pDevExt->pDevice = pDevObj;
	pDevExt->pDriver = pDriverObject;
	pDevExt->deviceNumber = deviceNumber; 

	pDevExt->pLowerDevice = IoAttachDeviceToDeviceStack(pDevObj,pDo);

	pDevObj->StackSize = pDevExt->pLowerDevice->StackSize + 1;

	KdPrint(("Anwi::Stack size = %d \n",pDevObj->StackSize));

	

	// Lets declare where we like to be called for each code
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = anwiWDMCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = anwiWDMClose;
	pDriverObject->MajorFunction[IRP_MJ_READ] = anwiWDMRead;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = anwiWDMWrite;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = anwiWDMIoctl;
	pDriverObject->MajorFunction[IRP_MJ_PNP]=anwiWDMPnp;
	pDriverObject->MajorFunction[IRP_MJ_POWER]=anwiWDMPower;

	return status;
}


static NTSTATUS anwiRemoveDevice
(	
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pDevExt;
	LONG32 deviceNumber;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Remove Device\n"));
#endif
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	
	if (pDevExt->dummy) {
		KdPrint(("Anwi::Cannot remove dummy device in removeDevice \n"));
		return status;
	} 
	deviceNumber = pDevExt->deviceNumber; 

	IoSetDeviceInterfaceState(&(pDevExt->symLinkName),FALSE);
	RtlFreeUnicodeString(&pDevExt->symLinkName);

	removeClient(deviceNumber);

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	status=passDownPnp(pDevObj,pIrp);

	IoDetachDevice(pDevExt->pLowerDevice);

	// Delete the device object
	IoDeleteDevice(pDevObj);

	return status;
}

static NTSTATUS anwiStartDevice
(	
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pDevExt;
	PIO_STACK_LOCATION pIrpStack;
	PCM_RESOURCE_LIST pResourceList;
	PCM_RESOURCE_LIST pResourceListOrig;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Start Device\n"));
#endif
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

	status = forwardAndWait(pDevObj,pIrp);
	
	if (!NT_SUCCESS(status)) {
		pIrp->IoStatus.Status = status;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest (pIrp, IO_NO_INCREMENT);
		return status;
	}

	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	pResourceListOrig = pIrpStack->Parameters.StartDevice.AllocatedResources;
	pResourceList = pIrpStack->Parameters.StartDevice.AllocatedResourcesTranslated;

	status = addClient(pDevExt->deviceNumber,pDevObj,pResourceList,pResourceListOrig);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	IoSetDeviceInterfaceState(&(pDevExt->symLinkName),TRUE);

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest (pIrp, IO_NO_INCREMENT);

	return status;
}


static NTSTATUS anwiStopDevice
(	
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pDevExt;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Remove Device\n"));
#endif
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

//	removeClient(pDevExt->deviceNumber);

	pIrp->IoStatus.Status = STATUS_SUCCESS;

	return passDownPnp(pDevObj,pIrp);
}



static NTSTATUS anwiSurpriseRemoval
(	
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pDevExt;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Surprise Removal\n"));
#endif
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;


//	removeClient(pDevExt->deviceNumber);
	
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	return passDownPnp(pDevObj,pIrp);
}

static NTSTATUS passDownPnp
(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
)
{
	PDEVICE_EXTENSION pDevExt;

	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(pDevExt->pLowerDevice,pIrp);
}

static NTSTATUS onRequestComplete 
(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp,
	PKEVENT pev
)
{
	KeSetEvent(pev,0,FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}


static NTSTATUS forwardAndWait
(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
)
{
	KEVENT event;
	PDEVICE_EXTENSION pDevExt;

	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	IoCopyCurrentIrpStackLocationToNext(pIrp);
	IoSetCompletionRoutine(pIrp,(PIO_COMPLETION_ROUTINE)onRequestComplete,
			(PVOID) &event,TRUE,TRUE, TRUE);
	IoCallDriver(pDevExt->pLowerDevice,pIrp);
	KeWaitForSingleObject(&event,Executive, KernelMode, FALSE, NULL);
	
	return pIrp->IoStatus.Status;
}
