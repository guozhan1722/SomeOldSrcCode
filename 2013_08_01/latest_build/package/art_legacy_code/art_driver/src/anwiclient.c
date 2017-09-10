/*
 * This file contains the functions for the client and client devices 
 * support
 */

#include <ntddk.h>
// #include <wdm.h>
#include "anwiclient.h"
#include "anwi.h"
#include "anwievent.h"
#include "anwiisr.h"
#include "anwiio.h"

// forward declarations
static VOID freeResources(pAnwiClientInfo pClient);
static VOID resetDevice(pAnwiClientInfo pClient);

static pAnwiClientInfo clientTable[NUM_SUPPORTED_CLIENTS];
static PVOID memTable[NUM_SUPPORTED_CLIENTS];

LONG32 initClientTable
(
	VOID
)
{
	ULONG32 i;

	for (i=0;i<NUM_SUPPORTED_CLIENTS;i++) {
		clientTable[i]=(pAnwiClientInfo)ExAllocatePool(NonPagedPool,sizeof(anwiClientInfo));
		if (clientTable[i] == NULL) {
			cleanupClientTable();
			return STATUS_NO_MEMORY;	
		}
		clientTable[i]->cliId = INVALID_CLIENT;
		memTable[i]=NULL;
	}

	return STATUS_SUCCESS;
}

VOID cleanupClientTable
(
	VOID
)
{
	UINT32 i;

	for (i=0;i<NUM_SUPPORTED_CLIENTS;i++) {
		if (clientTable[i] != NULL) {
			if (VALID_CLIENT(clientTable[i])) {
				removeClient(i);
			}
			ExFreePool(clientTable[i]);		
			clientTable[i]=NULL;
		}
		if (memTable[i] != NULL) {
			KdPrint(("Anwi::Freeing contigous memory allocated at KVA: %x \n",(ULONG32)memTable[i]));
			MmFreeContiguousMemorySpecifyCache(memTable[i],MEM_SIZE,MmNonCached);
			memTable[i] = NULL;
		}
	}

	return;
}


LONG32 findNextClientId
(
	LONG32 sIndex
) 
{
	LONG32 i;

	for (i=sIndex;i<NUM_SUPPORTED_CLIENTS;i++) {
		if (!VALID_CLIENT(clientTable[i])) {
				return i;
		}
	}

	return -1;
}

pAnwiClientInfo getClient
(
	LONG32 cliId
)
{
	pAnwiClientInfo pClient;

	pClient = clientTable[cliId];

	if (VALID_CLIENT(pClient)) {
		return pClient;
	}

	return NULL;
}

static void initClientInfo
(
	pAnwiClientInfo pClient
)
{
        ULONG32 iIndex;
	pClient->cliId = INVALID_CLIENT;
	pClient->busy = 0;
	pClient->numBars = 0;
	pClient->device_class = NETWORK_CLASS;
	RtlZeroMemory((PVOID)&pClient->memMap, sizeof(anwiAddrDesc));
    for (iIndex=0; iIndex<MAX_BARS; iIndex++) {
	    RtlZeroMemory((PVOID)&pClient->regMap[iIndex], sizeof(anwiAddrDesc));
    }
	RtlZeroMemory((PVOID)&pClient->resInfo, sizeof(anwiFullResourceInfo));
	RtlZeroMemory((PVOID)&pClient->intrDesc, sizeof(anwiIntrDesc));
	RtlZeroMemory((PVOID)&pClient->isrEventQ, sizeof(anwiEventQueue));
	RtlZeroMemory((PVOID)&pClient->trigeredEventQ, sizeof(anwiEventQueue));
	pClient->pDevObj = NULL;
}

static PVOID mapToUserSpace(PVOID kernelVirAddr,ULONG32 range,PMDL *ppMdl)
{
	PMDL pMdl;
	PVOID userVirAddr;

	pMdl = IoAllocateMdl (kernelVirAddr,
                          range,
                          FALSE,
                          FALSE,
                          NULL);

	if (pMdl == NULL) {
		KdPrint(("Anwi:: Mdl allocation failed\n"));
		return NULL;
	}

	// Now populate the MDL with the pages
	MmBuildMdlForNonPagedPool(pMdl);

	// Now lock the MDL in user mode space so that
	// the client could use it. This assumes that
	// we execute in the context of the thread that
	// called us for a shared memory
	 userVirAddr = MmMapLockedPages(pMdl,UserMode);

	if (userVirAddr == NULL) {
		IoFreeMdl(pMdl);
		return NULL;
	}

	*ppMdl = pMdl;

	return userVirAddr;
}

static VOID unmapFromUserSpace(PVOID usrVirAddr,PMDL pMdl)
{
	MmUnmapLockedPages(usrVirAddr,pMdl);
	IoFreeMdl(pMdl);
	return;
}


static LONG32 claimResources
(
	pAnwiClientInfo pClient,
	PCM_RESOURCE_LIST pResList,
	PCM_RESOURCE_LIST pResListOrig
)
{
	PCM_FULL_RESOURCE_DESCRIPTOR pFrd;
	PCM_PARTIAL_RESOURCE_LIST pPrl;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR pPrd;
	PCM_FULL_RESOURCE_DESCRIPTOR pFrdOrig;
	PCM_PARTIAL_RESOURCE_LIST pPrlOrig;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR pPrdOrig;
	ULONG32 numPrd;
	pAnwiFullResourceInfo pAnwiResInfo;
	ULONG32 i;
	ULONG32 numRes;
	PVOID address;
	ULONG32 Error;
	pAnwiAddrDesc pMemMap;
	pAnwiAddrDesc pRegMap;
	PHYSICAL_ADDRESS loLimit,hiLimit,boundAddr;
	PHYSICAL_ADDRESS phyAddr;
	ULONG32 cliId;
	NTSTATUS status;
	ULONG32 device_class_code;

	cliId = pClient->cliId;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Claim resources for client %d \n",cliId));
#endif
	
	pClient->sLock = ExAllocatePool(NonPagedPool, sizeof(KSPIN_LOCK));
	if (pClient->sLock == NULL) {
		KdPrint(("Anwi:: Memory allocation for spin lock failed\n"));
		return -1;
	}
	KeInitializeSpinLock(pClient->sLock);



#ifdef ANWI_DEBUG
	KdPrint(("Anwi::CM res list count = %d \n",pResList->Count));
#endif
	pFrd = &(pResList->List[0]);
	pPrl = &(pFrd->PartialResourceList);
	pFrdOrig = &(pResListOrig->List[0]);
	pPrlOrig = &(pFrdOrig->PartialResourceList);
	numPrd = pPrl->Count;
#ifdef ANWI_DEBUG
	KdPrint(("Anwi::CM partial res list count = %d \n",numPrd));
#endif

	pAnwiResInfo = &(pClient->resInfo);
	pAnwiResInfo->numItems = 0;
	Error = 0;
	for (i=0;i<numPrd;i++) {
		if (Error) {
			break;
		}
		numRes=pAnwiResInfo->numItems;

		pPrd = &(pPrl->PartialDescriptors[i]);
		pPrdOrig = &(pPrlOrig->PartialDescriptors[i]);
#ifdef ANWI_DEBUG
		KdPrint(("Anwi::Res type = %d \n",pPrd->Type));
#endif
		switch (pPrd->Type) {
			case CmResourceTypeInterrupt:
				pAnwiResInfo->Item[numRes].res_type = RES_INTERRUPT;
				pAnwiResInfo->Item[numRes].I.Int.irql = pPrd->u.Interrupt.Level;
				pAnwiResInfo->Item[numRes].I.Int.vector = pPrd->u.Interrupt.Vector;

				pClient->intrDesc.irql = (KIRQL)pPrd->u.Interrupt.Level;
				pClient->intrDesc.syncIrql = pClient->intrDesc.irql;
				pClient->intrDesc.vector = pPrd->u.Interrupt.Vector;
				pClient->intrDesc.affinity = pPrd->u.Interrupt.Affinity;
				if (pPrd->Flags & CM_RESOURCE_INTERRUPT_LATCHED) {
					pClient->intrDesc.intMode = Latched;
				} else {
					pClient->intrDesc.intMode = LevelSensitive;
				}

				pAnwiResInfo->numItems++; 
			break;
			case CmResourceTypeMemory:
				address = (PVOID) MmMapIoSpace(pPrd->u.Memory.Start, pPrd->u.Memory.Length, MmNonCached);

				if (address != NULL) {
					pAnwiResInfo->Item[numRes].res_type = RES_MEMORY;
					pAnwiResInfo->Item[numRes].I.Mem.mappedAddress = (PULONG32)address;
					pAnwiResInfo->Item[numRes].I.Mem.nBytes = pPrd->u.Memory.Length;

					pRegMap = &(pClient->regMap[pClient->numBars]);
                    pRegMap->res_type = RES_MEMORY;
					pRegMap->kerVirAddr = address;
					pRegMap->physAddr=(PVOID)pPrd->u.Memory.Start.LowPart;
					pRegMap->range=pPrd->u.Memory.Length;
        //            			if (pRegMap->range > (128 * 1024) ) {
        //                   			pRegMap->range = 128 * 1024;
        //            			}
					  if (pRegMap->range > (1024 * 1024) ) {
					    pRegMap->range = 1024 * 1024;
					  }
#ifdef ANWI_DEBUG
					KdPrint(("Anwi::Device Memory (%x) Mapped at KVA=%x length %d \n",pRegMap->physAddr,pRegMap->kerVirAddr,pRegMap->range));
#endif
			                pClient->numBars++;
					pAnwiResInfo->numItems++; 
                    
				} else {
					KdPrint(("Anwi::Mapping device memory to kernel virtual address space failed \n"));
					Error = 1;
                		}
			break;
			case CmResourceTypePort:
				address = (PVOID) MmMapIoSpace(pPrd->u.Port.Start, pPrd->u.Port.Length, MmNonCached);

				if (address != NULL) {
					pAnwiResInfo->Item[numRes].res_type = RES_IO;
					pAnwiResInfo->Item[numRes].I.IO.bAddr =  (PULONG32)address;
					pAnwiResInfo->Item[numRes].I.IO.nBytes = pPrd->u.Port.Length;

					pRegMap = &(pClient->regMap[pClient->numBars]);
                    pRegMap->res_type = RES_IO;
					pRegMap->kerVirAddr = address;
//					pRegMap->physAddr=(PVOID)pPrd->u.Port.Start.LowPart;
					pRegMap->physAddr=(PVOID)pPrd->u.Port.Start.LowPart;
					pRegMap->range=pPrd->u.Port.Length;
					//                    			if (pRegMap->range > (128 * 1024) ) {
					//                            			pRegMap->range = 128 * 1024;
					  if (pRegMap->range > (1024 * 1024) ) {
					    pRegMap->range = 1024 * 1024;
					  }
#ifdef ANWI_DEBUG
					KdPrint(("Anwi::Device I/O port at (%x) Mapped to KVA=%x length %d \n",pRegMap->physAddr,pRegMap->kerVirAddr,pRegMap->range));
#endif
			                pClient->numBars++;
					pAnwiResInfo->numItems++; 
                    
				} else {
					KdPrint(("Anwi::Mapping device memory to kernel virtual address space failed \n"));
					Error = 1;
                		}

			break;
			default:
				break;
		}
	}

	if (Error == 1) {
		freeResources(pClient);
		return -1;
	}

	if (memTable[cliId] == NULL) {
		loLimit.LowPart  = 0x0;
		loLimit.HighPart = 0x0;
		hiLimit.LowPart  = 0x0;
		hiLimit.HighPart = 0xffffffff;
		boundAddr.LowPart  = 0x0;
		boundAddr.HighPart = 0x0;
		address = MmAllocateContiguousMemorySpecifyCache(MEM_SIZE,
		                                                loLimit,
                                                        hiLimit,
                                                        boundAddr,
                                                        MmNonCached);
		KdPrint(("Anwi::Contigous memory allocated at KVA: %x \n",(ULONG32)address));
		memTable[cliId] = address;
	} else {
     	KdPrint(("Anwi::Using Contigous memory already allocated at KVA: %x \n",(ULONG32)memTable[cliId]));
	}
	address = memTable[cliId];

	if (address == NULL) {
		KdPrint(("Anwi::Allocate contigous memory failed \n"));
		freeResources(pClient);
		return -1;
	} else {
		pMemMap = &(pClient->memMap);
		pMemMap->kerVirAddr = address;
		phyAddr=MmGetPhysicalAddress(address);
		pMemMap->physAddr=(PVOID)phyAddr.LowPart;
		pMemMap->range=MEM_SIZE;
		KdPrint(("Anwi::Memory (%x) Mapped at KVA=%x length %d \n",pMemMap->physAddr,pMemMap->kerVirAddr,pMemMap->range));
    }

	KdPrint(("Anwi::Irql = %x vector = %x aff=%x intMode=%x \n", pClient->intrDesc.irql, pClient->intrDesc.vector, pClient->intrDesc.affinity, pClient->intrDesc.intMode));
	device_class_code = anwiCfgRead(0x08, 0x4, pClient);
	pClient->device_class = (device_class_code >> 24);
	KdPrint(("Anwi::Device class read = %x\n", pClient->device_class));
	// Now go ahead and connect to the interrupt.
	status = IoConnectInterrupt(&(pClient->intrDesc.pIntObj),
		                      AnwiInterruptHandler,
		                      pClient,
							  pClient->sLock,
							  pClient->intrDesc.vector,
							  pClient->intrDesc.irql,
							  pClient->intrDesc.irql,
							  pClient->intrDesc.intMode,
							  TRUE,
							  pClient->intrDesc.affinity,
							  FALSE);

	if (!NT_SUCCESS(status)) {
		KdPrint(("Anwi::Error connecting to interrupt \n"));
		freeResources(pClient);
		return -1;
	} 

	pClient->valid = 1;

	return 0;
}

static VOID freeResources
(
	pAnwiClientInfo pClient
)
{
	pAnwiFullResourceInfo pAnwiResInfo;
	ULONG32 i;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Free resources for client %d \n",pClient->cliId));
#endif

	pClient->valid = 0;

	if (pClient->intrDesc.pIntObj) {
			KdPrint(("Anwi::Freeing Irql = %x \n",pClient->intrDesc.irql));
				IoDisconnectInterrupt(pClient->intrDesc.pIntObj);
			pClient->intrDesc.pIntObj = NULL;
	}

	pAnwiResInfo = &(pClient->resInfo);

	for (i=0;i<(pAnwiResInfo->numItems);i++) {
		switch (pAnwiResInfo->Item[i].res_type) {
			case RES_INTERRUPT:
				break;
			case RES_MEMORY:
				KdPrint(("Anwi::Free Device Memory Mapped at KVA=%x length %d \n",pAnwiResInfo->Item[i].I.Mem.mappedAddress,pAnwiResInfo->Item[i].I.Mem.nBytes));
				MmUnmapIoSpace (pAnwiResInfo->Item[i].I.Mem.mappedAddress,
				                pAnwiResInfo->Item[i].I.Mem.nBytes);
				break;
			case RES_IO:
				KdPrint(("Anwi::Free Device IO Memory Mapped at KVA=%x length %d \n",pAnwiResInfo->Item[i].I.IO.bAddr,pAnwiResInfo->Item[i].I.IO.nBytes));
				MmUnmapIoSpace (pAnwiResInfo->Item[i].I.IO.bAddr, pAnwiResInfo->Item[i].I.IO.nBytes);
				break;
			default:
				break;
		}
	}
	pAnwiResInfo->numItems = 0;

	if (pClient->sLock != NULL) {
		ExFreePool(pClient->sLock);
		pClient->sLock = NULL;
	}


	return;
}



static void restoreBusData
(
	pAnwiClientInfo pClient
)
{
    ULONG iIndex;

	if (pClient->valid) {
        for(iIndex=0; iIndex<pClient->numBars; iIndex++) {
		    anwiCfgWrite(0x10 + (iIndex*4),4,(ULONG32)pClient->regMap[iIndex].physAddr,pClient);
        }
		anwiCfgWrite(0x3c,1,pClient->intrDesc.irql,pClient);
	}

	return;
}

LONG32 addClient
(
	ULONG32 clientId,
	PDEVICE_OBJECT pDevObj,
	PCM_RESOURCE_LIST pResourceList,
	PCM_RESOURCE_LIST pResourceListOrig
)
{
	pAnwiClientInfo pClient;

	pClient = clientTable[clientId];

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Add client with id = %d \n",clientId));
#endif

	if (VALID_CLIENT(pClient)) {
		KdPrint(("Anwi::Cannot add a valid client \n"));
		return STATUS_INVALID_PARAMETER;
	}

	initClientInfo(pClient);

	pClient->cliId = clientId;
	pClient->pDevObj = pDevObj;

	if (claimResources(pClient,pResourceList,pResourceListOrig) < 0) {
		KdPrint(("Anwi::Cannot claim resources \n"));
		pClient->cliId = INVALID_CLIENT;
		pClient->pDevObj = NULL;
		return STATUS_INSUFFICIENT_RESOURCES;
	}


	resetDevice(pClient);

	return STATUS_SUCCESS;
}


VOID removeClient
(
	ULONG32 clientId
)
{
	pAnwiClientInfo pClient;
	pClient = clientTable[clientId];

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Remove client with id = %d \n",clientId));
#endif

	if (!VALID_CLIENT(pClient)) {
		KdPrint(("Anwi::Cannot free a invalid client \n"));
		return;
	}

	resetDevice(pClient);

	freeResources(pClient);

	initClientInfo(pClient);

	return;
}


static LONG32 mapResourcesToUserSpace
(
	pAnwiClientInfo pClient
)
{
	pAnwiAddrDesc pRegMap;
	pAnwiAddrDesc pMemMap;
    ULONG32 iIndex;

#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Map resources to user space \n"));
#endif
     KdPrint(("Anwi::Num Bars = %d\n", pClient->numBars));
     for (iIndex=0; iIndex<pClient->numBars; iIndex++) {
		pRegMap = &(pClient->regMap[iIndex]);
		pRegMap->usrVirAddr = mapToUserSpace(pRegMap->kerVirAddr, pRegMap->range,&(pRegMap->pMdl));
        KdPrint(("Anwi::KVA=%x:range=%d:UVA=%x\n", pRegMap->kerVirAddr, pRegMap->range, pRegMap->usrVirAddr));
		if (pRegMap->usrVirAddr != NULL) {
			pRegMap->valid = 1;
			KdPrint(("Anwi::Device Memory %d mapped at KVA=%x UVA=%x length %d \n", iIndex, pRegMap->kerVirAddr,pRegMap->usrVirAddr,pRegMap->range));
		} else {
			KdPrint(("Anwi::Mapping Kernel to User Address for Registers failed \n"));
			return -1;
		}
     } // end of for

		pMemMap = &(pClient->memMap);
		pMemMap->usrVirAddr = mapToUserSpace(pMemMap->kerVirAddr,pMemMap->range,&(pMemMap->pMdl));
		if (pMemMap->usrVirAddr != NULL) {
			pMemMap->valid = 1;
			KdPrint(("Anwi::Memory Mapped at KVA=%x UVA=%x length %d \n",pMemMap->kerVirAddr,pMemMap->usrVirAddr,pMemMap->range));
		} else {
			KdPrint(("Anwi::Mapping Kernel to User Address for Memory failed \n"));
			return -1;
        }
		return 0;
}

static VOID unmapResourcesFromUserSpace
(
	pAnwiClientInfo pClient
)
{
	pAnwiAddrDesc pRegMap;
	pAnwiAddrDesc pMemMap;
    ULONG32 iIndex;
	
#ifdef ANWI_DEBUG
	KdPrint(("Anwi::Unmap resources from user space \n"));
#endif

  for (iIndex=0; iIndex<pClient->numBars; iIndex++) {
	pRegMap = &(pClient->regMap[iIndex]);
	if (pRegMap->valid) {
		KdPrint(("Anwi::Free device Memory Mapped at KVA=%x UVA=%x length %d \n",pRegMap->kerVirAddr,pRegMap->usrVirAddr,pRegMap->range));
		unmapFromUserSpace(pRegMap->usrVirAddr,pRegMap->pMdl);
		pRegMap->valid = 0;
	}

	pMemMap = &(pClient->memMap);
	if (pMemMap->valid) {
		KdPrint(("Anwi::Free Memory Mapped at KVA=%x UVA=%x length %d \n",pMemMap->kerVirAddr,pMemMap->usrVirAddr,pMemMap->range));
		unmapFromUserSpace(pMemMap->usrVirAddr,pMemMap->pMdl);
		pMemMap->valid = 0;
	}
  }

	return;
}

LONG32 registerClient
(
	ULONG32 clientId
)
{
	pAnwiClientInfo pClient;
	ULONG32 vendorId;

	pClient = clientTable[clientId];

	KdPrint(("Anwi::Register client with id = %d \n",clientId));

	if (!VALID_CLIENT(pClient)) {
		KdPrint(("Anwi::Cannot register a invalid client \n"));
		return STATUS_INVALID_PARAMETER;
	}

	if (BUSY_CLIENT(pClient)) {
		KdPrint(("Anwi::Cannot register a busy client \n"));
		return STATUS_DEVICE_BUSY;
	}

	vendorId = anwiCfgRead(0x0,4,pClient) & 0xffff;
	if (vendorId != ATHEROS_VENDOR_ID) {
		KdPrint(("Anwi::Client not present \n"));
		return STATUS_DEVICE_OFF_LINE;
	}

	initEventQueue(&pClient->isrEventQ,pClient->intrDesc.syncIrql);
	initEventQueue(&pClient->trigeredEventQ,pClient->intrDesc.syncIrql);

	if (mapResourcesToUserSpace(pClient) < 0) {
		KdPrint(("Anwi::Cannot map resources to user space \n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	restoreBusData(pClient);

	resetDevice(pClient);

	pClient->busy = 1;

	return STATUS_SUCCESS;
}

VOID unregisterClient
(
	ULONG32 clientId
)
{
	pAnwiClientInfo pClient;
	pClient = clientTable[clientId];

	KdPrint(("Anwi::Unregister client with id = %d \n",clientId));

	if (!VALID_CLIENT(pClient)) {
		KdPrint(("Anwi::Cannot register a invalid client \n"));
		return;
	}

	if (!BUSY_CLIENT(pClient)) {
		KdPrint(("Anwi::Cannot unregister a non-busy client \n"));
		return;
	}

	resetDevice(pClient);

	unmapResourcesFromUserSpace(pClient);

	deleteEventQueue(&pClient->isrEventQ);
	deleteEventQueue(&pClient->trigeredEventQ);

	pClient->busy = 0;

	return;
}

static VOID resetDevice
(
	pAnwiClientInfo pClient
)
{
	ULONG32 macRev;

	if (pClient->device_class == SIMPLE_COMM_CLASS) {
	   anwiRegWrite(0x4,0x0,&pClient->regMap[1]); // IER
	   anwiRegWrite(0x114,0x0,&pClient->regMap[1]); // Ext IER 
	   anwiRegWrite(0x104,0x1,&pClient->regMap[1]); // RC
	}
	if (pClient->device_class == NETWORK_CLASS) {
	   macRev = anwiRegRead(0x4020,&pClient->regMap[0]) & 0xff;
	   if( ((macRev >= 0x90) && (macRev <= 0x9f)) ||
		   ((macRev >= 0xa0) && (macRev <= 0xa3)) ||
		   ((macRev >= 0xe0) && (macRev <= 0xef)) ) {
	       //ie its condor, needs mac out of sleep
		   if(!(anwiRegRead(0x4010, &pClient->regMap[0]) & 0x18)) {
			   //double check its not eagle
			   anwiRegWrite(0x4000,0x0,&pClient->regMap[0]);
		   }
	   }
	   // disable interrupts
	   anwiRegWrite(0x0024,0x0,&pClient->regMap[0]);
	   // put the device in reset state
	   if( ((macRev >= 0x90) && (macRev <= 0x9f)) ||
		   ((macRev >= 0xa0) && (macRev <= 0xa3)) ||
		   ((macRev >= 0xe0) && (macRev <= 0xef))) {
		   if(anwiRegRead(0x4010, &pClient->regMap[0]) & 0x18) {
			   //ie its eagle
			   anwiRegWrite(0x4000,0x1f,&pClient->regMap[0]);
		   }
		   else {
				//ie its condor, no pci reset
				anwiRegWrite(0x4000,0x3,&pClient->regMap[0]);
		   }
	   }
	   else {
		   anwiRegWrite(0x4000,0x1f,&pClient->regMap[0]);
	   }
	}

}
