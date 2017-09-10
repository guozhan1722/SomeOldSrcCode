#include <ntddk.h>
#include "anwi.h"
#include "anwiclient.h"
#include "anwiwdm.h"

#define MAX_CFG_OFFSET 256

ULONG32 anwiRegRead
(
	ULONG32 offset,
	pAnwiAddrDesc pRegMap
)
{
	ULONG32 *address;
	ULONG32 data;

	address = (ULONG32 *)((ULONG32)pRegMap->kerVirAddr + offset);

	data = *address;
#ifdef ANWI_DEBUG
	KdPrint(("Anwi::RegRead @ %x : %x \n",(ULONG32)address,data));
#endif
	return data;
}

VOID anwiRegWrite
(
	ULONG32 offset,
	ULONG32 data,
	pAnwiAddrDesc pRegMap
)
{
	ULONG32 *address;

	address = (ULONG32 *)((ULONG32)pRegMap->kerVirAddr + offset);
	*address = data;
#ifdef ANWI_DEBUG
	KdPrint(("Anwi::RegWrite @ %x : %x \n",(ULONG32)address,data));
#endif
	
	return;
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

static NTSTATUS busReadWriteConfig 
(
	PDEVICE_OBJECT pDevObj,
	ULONG32 whichSpace,
	PVOID buffer,
	ULONG32 offset,
	ULONG32 length,
	BOOLEAN readConfig
)
{
	PIRP pIrp;
	KEVENT event;
	PDEVICE_EXTENSION pDevExt;
	PIO_STACK_LOCATION ioStack;
	NTSTATUS status;

	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pIrp=IoAllocateIrp(pDevExt->pLowerDevice->StackSize,FALSE);

	if (pIrp == NULL) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	IoSetCompletionRoutine(pIrp,(PIO_COMPLETION_ROUTINE)onRequestComplete,
                        (PVOID) &event,TRUE,TRUE, TRUE);

	ioStack = IoGetNextIrpStackLocation(pIrp);
	
	ioStack->MajorFunction=IRP_MJ_PNP;
	if (readConfig) {
		ioStack->MinorFunction=IRP_MN_READ_CONFIG;
	}
	else {
		ioStack->MinorFunction=IRP_MN_WRITE_CONFIG;
	}
	
	ioStack->Parameters.ReadWriteConfig.Buffer = buffer;
	ioStack->Parameters.ReadWriteConfig.Offset = offset;
	ioStack->Parameters.ReadWriteConfig.Length = length;
	
	if (readConfig) {
		RtlZeroMemory(buffer,length);
	}

        status = IoCallDriver(pDevExt->pLowerDevice,pIrp);
		
	if (status == STATUS_PENDING) {
		KeWaitForSingleObject(&event,Executive, KernelMode, FALSE, NULL);	}

        status = pIrp->IoStatus.Status;

	IoFreeIrp(pIrp);

	return status;
}


ULONG32 anwiCfgRead
(
	ULONG32 offset,
	ULONG32 length,
	pAnwiClientInfo pClient
) 
{
	ULONG32 data;
	NTSTATUS status;

	if (offset > MAX_CFG_OFFSET) {
		KdPrint(("Anwi::CfgRead:Invalid offset \n"));
		return 0xdeafbeaf;
	}

	status = busReadWriteConfig(pClient->pDevObj,
	                            PCI_WHICHSPACE_CONFIG,
	                            (PVOID) &data,
	                            offset,
				    length, 
				    TRUE);

	if (!NT_SUCCESS(status)) {
		KdPrint(("Anwi::CfgRead @ %x failed \n",offset));
		data = 0xdeadbeef;
	} else {
#ifdef ANWI_DEBUG
		KdPrint(("Anwi::CfgRead @ %x : %x \n",offset,data));
#endif
	}

	return data;
}



VOID anwiCfgWrite
(
	ULONG32 offset,
	ULONG32 length,
	ULONG32 data,
	pAnwiClientInfo pClient
) 
{
	NTSTATUS status;

	if (offset > MAX_CFG_OFFSET) {
		KdPrint(("Anwi::CfgWrite:Invalid offset \n"));
		return;
	}
	status = busReadWriteConfig(pClient->pDevObj,
	                            PCI_WHICHSPACE_CONFIG,
	                            (PVOID) &data,
	                            offset,
				    length,
				    FALSE);

	if (!NT_SUCCESS(status)) {
		KdPrint(("Anwi::Cfgwrite @ %x failed \n",offset));
	} else {
#ifdef ANWI_DEBUG
		KdPrint(("Anwi::CfgWrite @ %x : %x \n",offset,data));
#endif
	}

	return;
}



USHORT anwiIORead
(
	ULONG32 offset,
	ULONG32 length
) 
{
	USHORT data;
	NTSTATUS status;

	data = READ_PORT_USHORT((USHORT *)offset);

#ifdef ANWI_DEBUG
		KdPrint(("Anwi::IORead @ %x : %x\n",offset,data));
#endif

	return data;
}



VOID anwiIOWrite
(
	ULONG32 offset,
	ULONG32 length,
	USHORT data
) 
{
	NTSTATUS status;

    WRITE_PORT_USHORT((USHORT *)offset, data);
#ifdef ANWI_DEBUG
		KdPrint(("Anwi::IOWrite @ %x : %x\n",offset,data));
#endif
}




