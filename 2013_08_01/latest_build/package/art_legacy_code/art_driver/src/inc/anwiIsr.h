/*
 * This is the header file for Interrupt handling of the 
 * Anwi driver
 */
#ifndef __ANWIISR_H__
#define __ANWIISR_H__

#include "ntddk.h"

BOOLEAN AnwiInterruptHandler(PKINTERRUPT Interrupt, PVOID Context);

#endif /* __ANWI_ISR_H__ */
