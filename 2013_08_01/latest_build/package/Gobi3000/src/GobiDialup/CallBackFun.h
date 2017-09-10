#ifndef _CALL_BACK_FUN_H
#define _CALL_BACK_FUN_H
#include "BaseDef.h"
//The session state callback function
//Be called when the state of session has changed
void  SessionStateCallback(ULONG state,ULONG sessionEndReason );

//New short message callback function
//Be called when recieved a new short message
void NewSMSCallback(ULONG storageType,ULONG messageIndex );

void ActivationStatusCallback( ULONG activationStatus );

//Power state callback function
//Be called when the state of radio has changed
void PowerCallback( ULONG operatingMode );


void SignalStrengthCallback(INT8 signalStrength,ULONG radioInterface );

#endif //_GOBI_API_H_













