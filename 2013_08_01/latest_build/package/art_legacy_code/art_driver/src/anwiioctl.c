#include <ntddk.h>
#include "anwi.h"
#include "anwiIoctl.h"
#include "anwiwdm.h"
#include "anwiclient.h"
#include "anwiio.h"
#include "anwievent.h"


//
// anwiGetVersion
//
// Arguments:
// 		pDev - pointer to device object
// 		pIrp - pointer to IRP
// 		nBytes - pointer to number of bytes returned by this function
//
// Description:
//		This functions return the version number of the ANWI Driver
//
// Return:
//		NTSTATUS.
//		

static NTSTATUS anwiGetVersion 
(
	PDEVICE_OBJECT pDev,
	PIRP pIrp,
	PULONG32 nBytes
) 
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack;
	pAnwiVersionInfo pAnwiVer;
	pAnwiReturnContext pRet;
	ULONG32 inSize;
	ULONG32 outSize;
	PUCHAR	outBuffer;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Get version \n"));
#endif
	
	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

	// Input buffer length
	inSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;

	// Output buffer length
	outSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    
	// Output buffer pointer
	outBuffer = MmGetSystemAddressForMdl( pIrp->MdlAddress);

	pRet = (pAnwiReturnContext) outBuffer;
		
	// Validate the input and output buffer 
	if ((outSize < sizeof(anwiReturnContext)) || (inSize)) {
		KdPrint(("Anwi::Parameters Wrong in call to IOCTL_ANWI_GET_VERSION"));
		pRet->returnCode = ANWI_PARAMETER_ERROR;
		pRet->contextLen = 0;
	} else {
		pRet->returnCode = ANWI_OK;
		pRet->contextLen = sizeof(anwiVersionInfo);
	
		pAnwiVer = (pAnwiVersionInfo) pRet->context;
		pAnwiVer->majorVersion = ANWI_MAJOR_VERSION;
		pAnwiVer->minorVersion = ANWI_MINOR_VERSION;
	}

	*nBytes = sizeof(pRet->returnCode) + sizeof(pRet->contextLen) +
			  	pRet->contextLen;

	return status;
}

static NTSTATUS anwiGetClientInfo 
(
	PDEVICE_OBJECT pDev,
	PIRP pIrp,
	PULONG32 nBytes
) 
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack;
	PDEVICE_EXTENSION	pDevExt;
	ULONG32 inSize, outSize;
	PUCHAR inBuffer, outBuffer;
	pAnwiInClientInfo pInCliInfo;
	pAnwiOutClientInfo pOutCliInfo;
	pAnwiReturnContext pRet;
	pAnwiClientInfo pClient;
	LONG32 cliId;
    ULONG32 iIndex;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Get client info \n"));
#endif
	
	pDevExt = pDev->DeviceExtension;
	cliId = pDevExt->deviceNumber;
	pClient = getClient(cliId);

	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

	// Input buffer length
	inSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength; 
	// Output buffer length
	outSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength; 
	// Input buffer pointer
	inBuffer = pIrp->AssociatedIrp.SystemBuffer; 
	// Output Buffer pointer
	outBuffer = MmGetSystemAddressForMdl( pIrp->MdlAddress); 
	
	pRet = (pAnwiReturnContext) outBuffer;
	pInCliInfo = (pAnwiInClientInfo) inBuffer;
	pOutCliInfo = (pAnwiOutClientInfo)pRet->context;
	
	if ((outSize < sizeof(anwiReturnContext)) || 
		(inSize != sizeof(anwiInClientInfo))) {
		KdPrint(("Anwi::Parameters Wrong in call to IOCTL_ANWI_GET_VERSION"));
		pRet->returnCode = ANWI_PARAMETER_ERROR;
		pRet->contextLen = 0;
	} else {
		if ((!pClient) || (!BUSY_CLIENT(pClient))) {
			pRet->returnCode = ANWI_INVALID_CLIENT_ID;
			pRet->contextLen = 0;
		} else {
            for(iIndex=0; iIndex<pClient->numBars; iIndex++) {
			   pOutCliInfo->aregPhyAddr[iIndex] = (ULONG32)pClient->regMap[iIndex].physAddr;
			   pOutCliInfo->aregVirAddr[iIndex] = (ULONG32)pClient->regMap[iIndex].usrVirAddr;
			   pOutCliInfo->aregRange[iIndex] = (ULONG32)pClient->regMap[iIndex].range;
			   pOutCliInfo->res_type[iIndex] = (ULONG32)pClient->regMap[iIndex].res_type;
            }
            pOutCliInfo->numBars = pClient->numBars;
			pOutCliInfo->memPhyAddr = (ULONG32)pClient->memMap.physAddr;
			pOutCliInfo->memVirAddr = (ULONG32)pClient->memMap.usrVirAddr;
			pOutCliInfo->irqLevel = (ULONG32)pClient->intrDesc.irql;
			pOutCliInfo->memSize = (ULONG32)pClient->memMap.range;

			pRet->returnCode= ANWI_OK;
			pRet->contextLen = sizeof(anwiOutClientInfo); 
		}
	}
	*nBytes = sizeof(pRet->returnCode) + sizeof(pRet->contextLen) +
			  		pRet->contextLen;

	return status;
} 

static LONG32 deviceOperation
( 
	PDEVICE_OBJECT pDev,
	pAnwiDevOpStruct in,
	pAnwiDevOpStruct out
)
{
	LONG32 ret;
	PDEVICE_EXTENSION	pDevExt;
	pAnwiClientInfo pClient;
	LONG32 cliId;

	// Check whether the client is a valid client	
	pDevExt = pDev->DeviceExtension;
	cliId = pDevExt->deviceNumber;

	pClient = getClient(cliId);
	if ((!pClient) || (!BUSY_CLIENT(pClient))) {
		return ANWI_INVALID_CLIENT_ID;
	}
	ret = ANWI_INVALID_DEVOP_TYPE;

	// Clear the output buffer
	RtlZeroMemory(out,sizeof(anwiDevOpStruct));

	// Copy the client id and op type from the input structure
	out->opType = in->opType;

	// Do the appropiate device operation
	switch (in->opType) {
		case ANWI_CFG_READ:
			out->param1=anwiCfgRead(in->param1,in->param2,pClient);	
			ret = ANWI_OK; 
			break;
		case ANWI_CFG_WRITE:
			anwiCfgWrite(in->param1,in->param2,in->param3,pClient);
			ret = ANWI_OK; 
			break;
		case ANWI_IO_READ:
			out->param1=anwiIORead(in->param1,in->param2);	
			ret = ANWI_OK; 
			break;
		case ANWI_IO_WRITE:
			anwiIOWrite(in->param1,in->param2,(USHORT)in->param3);
			ret = ANWI_OK; 
			break;
	}
	
	return ret;
}



NTSTATUS anwiDeviceOp (
	PDEVICE_OBJECT pDev,
	PIRP pIrp,
	PULONG32 nBytes
) 
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack;
	pAnwiReturnContext pRet;
	pAnwiDevOpStruct in;
	pAnwiDevOpStruct out;
	ULONG32 inSize;
	ULONG32 outSize;
	PUCHAR	inBuffer;
	PUCHAR	outBuffer;
	
	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

	// Input buffer length
	inSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;

	// Output buffer length
	outSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    
	// Input buffer pointer
	inBuffer = pIrp->AssociatedIrp.SystemBuffer;

	// Output buffer pointer
	outBuffer = MmGetSystemAddressForMdl( pIrp->MdlAddress);

	pRet = (pAnwiReturnContext) outBuffer;

	// Validate the input and output buffer 
	if ((outSize < sizeof(anwiReturnContext)) &&
		(inSize != sizeof(anwiDevOpStruct))) {
		KdPrint(("Anwi::Parameters Wrong in call to IOCTL_ANWI_DEV_OP"));
		pRet->returnCode = ANWI_PARAMETER_ERROR;
		pRet->contextLen = 0;
	} else {
		in =  (pAnwiDevOpStruct) inBuffer;	
		out = (pAnwiDevOpStruct) pRet->context;

		pRet->returnCode=deviceOperation(pDev,in,out);
		if (pRet->returnCode == ANWI_OK) {
			pRet->contextLen = sizeof(anwiDevOpStruct); 
		} else {
			pRet->contextLen = 0;
		}
	}

	*nBytes = sizeof(pRet->returnCode) + sizeof(pRet->contextLen) +
			  	pRet->contextLen;

	return status;
}

LONG32 eventOperation
( 
	PDEVICE_OBJECT pDev,
	pAnwiEventOpStruct in,
	pAnwiEventOpStruct out
)
{
	LONG32 ret;
	PDEVICE_EXTENSION	pDevExt;
	pAnwiClientInfo pClient;
	pAnwiEventStruct pEvent;
	anwiEventHandle eventHandle;	
	ULONG32 i;
	LONG32 cliId;

	// Check whether the client is a valid client	
	pDevExt = pDev->DeviceExtension;
	cliId = pDevExt->deviceNumber;

	pClient = getClient(cliId);
	if ((!pClient) || (!BUSY_CLIENT(pClient))) {
		return ANWI_INVALID_CLIENT_ID;
	}

	ret = ANWI_INVALID_EVENTOP_TYPE;

	// Copy the client id and op type from the input structure
	out->opType = in->opType;
	out->valid = 0;

	// Do the appropiate device operation
	switch (in->opType) {
		case ANWI_CREATE_EVENT:
			ret = ANWI_CREATE_EVENT_ERROR;
			if (in->valid == 1) {
				eventHandle.eventID=in->param[5] & 0xffff;
				eventHandle.f2Handle=(in->param[5] >> 16) & 0xffff;
				pEvent = createEvent(in->param[0], //type
				                     in->param[1], //persistent
				                     in->param[2], //param1
				                     in->param[3], //param2
				                     in->param[4], //param3
				                     eventHandle);
		
				if (pEvent != NULL) {
   					// need to look at the event type to see which queue 
    				switch (pEvent->type ) {
    					case ISR_INTERRUPT:
							// if param1 is zero, we, by default
							// set the "ISR IMR" to pass everything
							if ( 0 == pEvent->param1 ) {
								pEvent->param1 = 0xffffffff;
							}
							if (pushEvent(pEvent, &pClient->isrEventQ,TRUE) ) {
								ret = ANWI_OK; 
							} else {
								KdPrint(("Anwi::Push Event Failed \n"));
								ExFreePool(pEvent);
							}
							break;
						default:
							KdPrint(("Anwi::Event Type %d not supported \n",pEvent->type));
							ExFreePool(pEvent);
							break;
    				}
				}
			}
			break;
		case ANWI_GET_NEXT_EVENT:
			ret = ANWI_OK; 
			out->valid = 0;
			if (pClient->trigeredEventQ.queueSize) {
				if (checkForEvents(&pClient->trigeredEventQ,TRUE)) {
					pEvent = popEvent(&pClient->trigeredEventQ,TRUE);
					out->valid = 1;
					out->param[0] = pEvent->type;
					out->param[1] = pEvent->persistent;
					out->param[2] = pEvent->param1;
					out->param[3] = pEvent->param2;
					out->param[4] = pEvent->param3;
					out->param[5] = (pEvent->eventHandle.f2Handle << 16) | pEvent->eventHandle.eventID;
					for (i=0;i<6;i++) {
						out->param[6+i] = pEvent->result[i];
					}
#ifdef ANWI_DEBUG
					KdPrint(("Anwi:: Pop event %x \n",(ULONG32)pEvent));
#endif
					ExFreePool(pEvent);
				}
			}	
			break;
	}

	return ret;
}
NTSTATUS anwiEventOp (
	PDEVICE_OBJECT pDev,
	PIRP pIrp,
	PULONG32 nBytes
) 
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack;
	pAnwiReturnContext pRet;
	ULONG32 inSize;
	ULONG32 outSize;
	pAnwiEventOpStruct in;
	pAnwiEventOpStruct out;
	PUCHAR	inBuffer;
	PUCHAR	outBuffer;
	
	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

	// Input buffer length
	inSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;

	// Output buffer length
	outSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    
	// Input buffer pointer
	inBuffer = pIrp->AssociatedIrp.SystemBuffer;

	// Output buffer pointer
	outBuffer = MmGetSystemAddressForMdl( pIrp->MdlAddress);

	pRet = (pAnwiReturnContext) outBuffer;

	// Validate the input and output buffer 
	if ((outSize < sizeof(anwiReturnContext)) &&
		(inSize != sizeof(anwiEventOpStruct))) {
		KdPrint(("Anwi::Parameters Wrong in call to IOCTL_ANWI_EVENT_OP"));
		pRet->returnCode = ANWI_PARAMETER_ERROR;
		pRet->contextLen = 0;
	} else {
		in =  (pAnwiEventOpStruct) inBuffer;	
		out = (pAnwiEventOpStruct) pRet->context;

		pRet->returnCode=eventOperation(pDev,in,out);
		if (pRet->returnCode == ANWI_OK) {
			pRet->contextLen = sizeof(anwiEventOpStruct); 
		} else {
			pRet->contextLen = 0;
		}
	}

	*nBytes = sizeof(pRet->returnCode) + sizeof(pRet->contextLen) +
			  	pRet->contextLen;

	return status;
}

/*
 * anwiWDMIoctl - the io control entry point
 *
 * This is where we are called when a client makes a 
 * DeviceIoControl call. This dispatch is special because
 * we could take an input buffer and an output buffer.
 * 
 * The buffer passing policy is decided by the transfer_type
 * flag in the IOCTL definition. For all anwi IOCTLs we have
 * this flag set to METHOD_IN_DIRECT. Where,
 *
 * The input buffer passed by the client is wrapped with an
 * MDL and the address of this MDL is in pIrp->MdlAddress.
 *
 * The Output buffer is handled with a temporary nonpaged
 * system buffer whose address is in 
 * pIrp->AssociatedIrp.SystemBuffer, and is copied on the
 * the user buffer when the IRP completes.
 *
 * The list of IOCTL's handled are..
 *
 * 1. IOCTL_ANWI_GET_VERSION
 * 
 * 2. IOCTL_ANWI_DEVICE_INIT
 *
 * Arguments:
 * 	PDEVICE_OBJECT
 * 	PIRP
 *
 * Return:
 * 	NTSTATUS
 */

NTSTATUS anwiWDMIoctl (PDEVICE_OBJECT pDev, PIRP pIrp) 
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack;
	ULONG32 nBytes = 0;

	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

	switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_ANWI_GET_VERSION: 
			status = anwiGetVersion(pDev,
			                        pIrp,
			                        &nBytes);
			break;
    
		case IOCTL_ANWI_GET_CLIENT_INFO:
			status = anwiGetClientInfo(pDev, 
			                        pIrp, 
			                        &nBytes);
			break;

		case IOCTL_ANWI_DEV_OP:
			status = anwiDeviceOp(pDev,
			                      pIrp,
			                      &nBytes);
			break;

		case IOCTL_ANWI_EVENT_OP:
			status = anwiEventOp(pDev,
			                     pIrp,
		                         &nBytes);
			break;
		default:
	    		status = STATUS_INVALID_DEVICE_REQUEST;
			break;
    }
    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = nBytes;

    IoCompleteRequest (pIrp, IO_NO_INCREMENT);
    
    return status;
}


		

