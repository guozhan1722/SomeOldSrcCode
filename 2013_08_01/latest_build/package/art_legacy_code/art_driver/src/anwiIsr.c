/*
 * All the Anwi interrupt handling code are in this file.
 * 
 */

#include <ntddk.h>
// #include <wdm.h>
#include "anwi.h"
#include "anwiclient.h"
#include "anwievent.h"
#include "anwiio.h"

/*
 * The following function is the interrupt handler for all
 * devices that would be controlled by clients. 
 */
BOOLEAN AnwiInterruptHandler
(	
	PKINTERRUPT Interrupt,
	PVOID Context
) 
{
	pAnwiEventStruct pEvent;
	pAnwiEventStruct pEventCopy;
	pAnwiEventStruct pEventToPush;
	pAnwiEventQueue pIsrEventQ;
	pAnwiEventQueue pTrigeredQ;
	ULONG32 priIsr=0;
	ULONG32 secIsr[5];
	ULONG32 i;
	BOOLEAN bValidEvent;
	pAnwiClientInfo	pClient;

	pClient = Context;

		// check whether the client is registered
		if (!BUSY_CLIENT(pClient)) return FALSE;

		// Check the PCIPENDING register
		// This check assumes the reserved part of the int pending 
		// register is all zeros. Since PCI devices share 
		// interrupts, this ISR will be called even if any other
		// device which uses the same IRQ interrupts the processor.
		// Reading the INT pending register should tell whether our
		// device is the cause of the interrupt.  Since the hw 
		// returns a unknown error value even when the card is switched off, 
		// this check should filter most of them. 

	if (pClient->device_class == NETWORK_CLASS) {
		if ((anwiRegRead(0x4008,&(pClient->regMap[0]))) != 0x00000001) return FALSE;	

		priIsr = anwiRegRead(0xc0,&(pClient->regMap[0]));	

		//FJC
		if(!priIsr) {
			//this might be first gen
			priIsr = anwiRegRead(0x1c,&(pClient->regMap[0]));
		}
		KdPrint(("Anwi::Got WMAC interrupt = %x \n",priIsr));

		secIsr[0] = anwiRegRead(0xc4,&pClient->regMap[0]);
		secIsr[1] = anwiRegRead(0xc8,&pClient->regMap[0]);
		secIsr[2] = anwiRegRead(0xcc,&pClient->regMap[0]);
		secIsr[3] = anwiRegRead(0xd0,&pClient->regMap[0]);
		secIsr[4] = anwiRegRead(0xd4,&pClient->regMap[0]);

	}
	if (pClient->device_class == SIMPLE_COMM_CLASS) {
		priIsr = anwiRegRead(0x08, &(pClient->regMap[0]));
		KdPrint(("Anwi::Got uart interrupt = %x \n",priIsr));
	}

	// start to scan the current queue of events
	pIsrEventQ = &pClient->isrEventQ;
	pTrigeredQ = &pClient->trigeredEventQ;
	pEvent = pIsrEventQ->pHead;

	while ( pEvent ) {
		bValidEvent = FALSE;
		switch ( pEvent->type ) {
			case ISR_INTERRUPT:
			if (priIsr & pEvent->param1 ) {
				// return the complete ISR value 
				// not anded with the mask
				pEvent->result[0] = priIsr;
				for (i=0;i<5;i++) {
					pEvent->result[1+i] = secIsr[i];
				}
				bValidEvent    = TRUE;
			}
			break;
		default:
			KdPrint(("Anwi::Ilegal event type found in ISR event queue!\n"));
			break;
		} // switch
        
		if ( !bValidEvent ) {
			pEvent = pEvent->pNext;
			continue;
    }

		// put the event in the event triggered Q, 
		// either move or copy event, depending 
		// on the persistent flag 
		if ( pEvent->persistent ) {
			pEventCopy = copyEvent(pEvent);
			if( !pEventCopy ) {
				KdPrint(("Anwi::Unable to copy event in interrupt\n"));
				break;
			}
			pEventToPush = pEventCopy;
		} else {
			if (!removeEvent(pEvent, pIsrEventQ, FALSE) ) {
				KdPrint(("Anwi::Unable to remove event from ISR queue \n"));
				break;
			}
			pEventToPush = pEvent;
		}

		// push the event onto the triggered queue
		if (!pushEvent(pEventToPush, pTrigeredQ,FALSE) ) {
			KdPrint(("Anwi::Unable to push event onto triggered queue\n"));
			break;
		}

		// increment to next event 
		pEvent = pEvent->pNext;
	} // while ( pEvent )

	// Request a DPC if required
	// IoRequestDpc(pDo, NULL, NULL );
	// Returning True claims the interrupt, no more
	// IoManager shuttling of this interrupt object.

	return TRUE;
}


