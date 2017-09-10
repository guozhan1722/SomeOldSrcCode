/**
*@file GobiDef.h
*
*@brief 定义Gobi SDK使用相关的参数和函数
*
*@note
*
* Copyright (C) 2010 Huawei Technologies Co.,Ltd
*
* 版权所有 (C) 2010 华为技术有限公司
*
*
*
*@version 1.0
*
*
*
*/
/* =====================================================================
TackID		Name                      Date                         	Version                 Command
====================================================================== */
#ifndef __GOBI_DEF_H__
#define __GOBI_DEF_H__

#include "BaseDef.h"
static char  THEPAHTOFDYNAMICLIB[] = "/usr/lib/GobiConnectionMgmt.so";
//Device connectivity service
static char   QCWWAN_ENUMERATE_DEVICES[] = "QCWWANEnumerateDevices";
static char   QCWWAN_CONNECT[]   = "QCWWANConnect";
static char   QCWWAN_DISCONNECT[]  = "QCWWANDisconnect";
static char   QCWWAN_GET_CONNECTED_DEVICEID[]  = "QCWWANGetConnectedDeviceID";
static char   QCWWAN_CANCEL[]  = "QCWWANCancel";

//Wireless data service
static char   QCWWAN_GET_SESSION_STATE[]  = "GetSessionState";
static char   QCWWAN_START_DATA_SESSION[]  = "StartDataSession";
static char   QCWWAN_CANCEL_DATA_SESSION[]  = "CancelDataSession";
static char   QCWWAN_STOP_DATA_SESSION[] = "StopDataSession";
static char   QCWWAN_GET_IP_ADDRESS[]  = "GetIPAddress";
static char   QCWWAN_SETDEFAULTPROFILE[]  = "SetDefaultProfile";
static char   QCWWAN_GETDEFAULTPROFILE[]  = "GetDefaultProfile";
static char   QCWWAN_SETENHANCEDAUTOCONNECT[]  = "SetEnhancedAutoconnect";
static char   QCWWAN_GETENHANCEDAUTOCONNECT[]  = "GetEnhancedAutoconnect";

static char   QCWWAN_SETS_ESSIONSTATE_CALLBACK[]  = "SetSessionStateCallback";


//need modify ,above this line 



/*SMS  begin*/
static char   QCWWAN_DELETESMS[]  = "DeleteSMS";
static char   QCWWAN_GETSMSLIST[]  = "GetSMSList";
static char   QCWWAN_GETSMS[]  = "GetSMS";
static char   QCWWAN_MODIFYSMSSTATUS[]  = "ModifySMSStatus";
static char   QCWWAN_SAVESMS[]  = "SaveSMS";
static char   QCWWAN_SENDSMS[]  = "SendSMS";
static char   QCWWAN_GETSMSCADDRESS[]  = "GetSMSCAddress";
static char   QCWWAN_SETSMSCADDRESS[]  = "SetSMSCAddress";
static char   QCWWAN_GETSMSROUTES[]  = "GetSMSRoutes";
static char   QCWWAN_SETSMSROUTES[]  = "SetSMSRoutes";
static char   QCWWAN_SETNEWSMSCALLBACK[]  = "SetNewSMSCallback";
/*SMS end*/

/*Network access service begin*/
static char   QCWWAN_PERFORM_NETWORK_SCAN[]  = "PerformNetworkScan";
static char   QCWWAN_PERFORM_NETWORK_RAT_SCAN[]  = "PerformNetworkRATScan";
static char   QCWWAN_INITIATE_NETWORK_REGISTRATION[]  = "InitiateNetworkRegistration";
static char   QCWWAN_GET_SRV_NETWORK[]  = "GetServingNetwork";
static char   QCWWAN_GET_SRV_NETWORK_CAP[]  = "GetServingNetworkCapabilities";
static char   QCWWAN_GET_HOME_NET_WORK[]  = "GetHomeNetwork";
static char   QCWWAN_GET_NETWORK_PREFERENCE[]  = "GetNetworkPreference";
static char   QCWWAN_SET_NETWORK_PREFERENCE[]  = "SetNetworkPreference";
/*Network access service end*/

/*Device management service begin*/

static char   QCWWAN_GETDEVICECAPABILITIES[]  = "GetDeviceCapabilities";
static char   QCWWAN_GETFIRMWAREREVISION[]  = "GetFirmwareRevision";
static char   QCWWAN_GETFIRMWAREREVISIONS[]  = "GetFirmwareRevisions";
static char   QCWWAN_GETIMSI[]  = "GetIMSI";
static char   QCWWAN_GETSERIALNUMBERS[]  = "GetSerialNumbers";

//获取PRL（Preferred Roaming List）版本
//Get Preferred Roaming List Version
static char   QCWWAN_GET_PRL_VERSION[]  = "GetPRLVersion";

//OTA activate
static char   QCWWAN_ACTIVATE_AUTOMATIC[]  = "ActivateAutomatic";
static char   QCWWAN_ACTIVATE_MANUAL[]  = "ActivateManual";
static char   QCWWAN_GET_ACTIVATION_STATE[]  = "GetActivationState";
static char   QCWWAN_SET_ACTIVATION_STATUS_CALL_BACK[]  = "SetActivationStatusCallback";

//电源模式管理
//Power Mode Management
static char   QCWWAN_SET_POWER[]  = "SetPower";
static char   QCWWAN_GET_POWER[]  = "GetPower";
//Power mode callback
static char   QCWWAN_SET_POWER_MODE_CB[]  = "SetPowerCallback";


//This function returns reason why the operating mode of the device is currently offline.
static char   QCWWAN_GET_OFFLINE_REEASON[]  = "GetOfflineReason";

//UIM operate
static char   QCWWAN_UIM_SET_PIN_PROTECTION[]  = "UIMSetPINProtection";
static char   QCWWAN_UIM_VERIFY_PIN[]  = "UIMVerifyPIN";
static char   QCWWAN_UIM_UNBLOCK_PIN[]  = "UIMUnblockPIN";
static char   QCWWAN_UIM_CHANGE_PIN[]  = "UIMChangePIN";
static char   QCWWAN_UIM_GET_PIN_STATUS[]  = "UIMGetPINStatus";
static char   QCWWAN_UIM_GET_ICCID[]  = "UIMGetICCID";

//Signal strength callback 
static char   QCWWAN_SET_SIGNAL_STRENGTH_CALLBACK[] = "SetSignalStrengthCallback";

//恢复出厂设置
//Reset to factory default
static char   QCWWAN_RESET_TO_FACTORY_DEF[]  = "ResetToFactoryDefaults";
/*Device management service end*/




extern "C" 
{
	//Device connectivity service
	
	//Enumerates the QC WWAN devices currently attached to the system
	typedef ULONG (*tFNQCWWANEnumerateDevices)(BYTE * pDevicesSize, BYTE * pDevices );
	
	//Connects CM API to the specified QC WWAN device
	typedef ULONG (*tFNQCWWANConnect)(CHAR * pDeviceNode,CHAR * pDeviceKey );
	
	typedef ULONG (*tFNQCWWANDisconnect)();
	
	//Returns device ID and device key of currently connected QC WWAN device
	typedef ULONG (*tFNQCWWAN2kGetConnectedDeviceID)(ULONG deviceNodeSize,
                                                                                 CHAR * pDeviceNode,
                                                                                 ULONG deviceKeySize,
                                                                                 CHAR * pDeviceKey );
	typedef ULONG (*tFNQCWWANCancel)();

    typedef ULONG (*tFNSetDefaultProfile)(
                                                   ULONG  profileType,
                                                   ULONG *pPDPType, 
                                                   ULONG *pIPAddress, 
                                                   ULONG *pPrimaryDNS, 
                                                   ULONG *pSecondaryDNS, 
                                                   ULONG *pAuthentication, 
                                                   CHAR  *pName, 
                                                   CHAR  *pAPNName, 
                                                   CHAR  *pUsername,
                                                   CHAR  *pPassword );
    typedef ULONG (*tFNGetDefaultProfile)(
                                                   ULONG  profileType,
                                                   ULONG *pPDPType, 
                                                   ULONG *pIPAddress, 
                                                   ULONG *pPrimaryDNS, 
                                                   ULONG *pSecondaryDNS, 
                                                   ULONG *pAuthentication, 
                                                   BYTE   nameSize,
                                                   CHAR  *pName, 
                                                   BYTE   apnSize,
                                                   CHAR  *pAPNName, 
                                                   BYTE   userSize,
                                                   CHAR  *pUsername);


    typedef ULONG (*tFNSetEnhancedAutoconnect)(ULONG setting, ULONG *pRoamSetting);
    typedef ULONG (*tFNGetEnhancedAutoconnect)(ULONG *pSetting, ULONG *pRoamSetting);

	//Wireless data service
	//Return the state of session
	typedef ULONG (*tFNGetSessionState)( ULONG * pState );

	//Start the data session
	typedef ULONG (*tFNStartDataSession)(ULONG * pTechnology,
									ULONG * pPrimaryDNS,
									ULONG * pSecondaryDNS,
									ULONG * pPrimaryNBNS,
									ULONG * pSecondaryNBNS,
									CHAR * pAPNName,
									ULONG * pIPAddress,
									ULONG * pAuthentication,
									CHAR * pUsername,
									CHAR * pPassword,
									ULONG * pSessionId,
									ULONG * pFailureReason );
	
	typedef ULONG (*tFNCancelDataSession)();
	
	typedef ULONG (*tFNStopDataSession)( ULONG sessionId );
	typedef ULONG (*tFNGetIPAddress)( ULONG * pIPAddress );	
	
	// Session state callback function
	typedef void (*tFNSessionState)(ULONG state,
									ULONG sessionEndReason );
	// QCWWAN set session state callback function pointer
	typedef ULONG (*tFNSetSessionStateCallback)(tFNSessionState pCallback );

	 
	

	/*SMS begin*/
	typedef ULONG (*tFNDeleteSMS)(ULONG storageType,
								ULONG * pMessageIndex,
								ULONG * pMessageTag );
	
	typedef ULONG (*tFNGetSMSList)(ULONG storageType,
								ULONG * pRequestedTag,
								ULONG * pMessageListSize,
								BYTE * pMessageList );
	
	typedef ULONG (*tFNGetSMS)(ULONG storageType,
								ULONG messageIndex,
								ULONG * pMessageTag,
								ULONG * pMessageFormat,
								ULONG * pMessageSize,
								BYTE * pMessage );
	
	typedef ULONG (*tFNModifySMSStatus)(ULONG storageType,
								ULONG messageIndex,
								ULONG messageTag );

	typedef ULONG (*tFNSaveSMS)(ULONG storageType,
								ULONG messageFormat,
								ULONG messageSize,
								BYTE * pMessage,
								ULONG * pMessageIndex );

	typedef ULONG (*tFNSendSMS)(ULONG messageFormat,
								ULONG messageSize,
								BYTE * pMessage,
								ULONG * pMessageFailureCode );

	typedef ULONG (*tFNGetSMSCAddress)(BYTE addressSize,
								CHAR * pSMSCAddress,
								BYTE typeSize,
								CHAR * pSMSCType );

	typedef ULONG (*tFNSetSMSCAddress)(CHAR * pSMSCAddress,
								CHAR * pSMSCType );

	typedef ULONG (*tFNGetSMSRoutes)(BYTE * pRouteSize,
								BYTE * pRoutes );

	typedef ULONG (*tFNSetSMSRoutes)(BYTE * pRouteSize,
								BYTE * pRoutes );

	typedef void (*tFNNewSMS)(ULONG storageType,
								ULONG messageIndex );

	typedef ULONG (*tFNSetNewSMSCallback)( tFNNewSMS pCallback );
	/*SMS end*/

	/*Network access service begin*/
	//搜索可用网络
	typedef ULONG (*tFNPerformNetworkScan)(BYTE * pInstanceSize,
								BYTE * pInstances );

	typedef ULONG (*tFNPerformNetworkRATScan)(BYTE * pInstanceSize,
								BYTE * pInstances,
								BYTE * pRATSize,
								BYTE * pRATInstances );

	typedef ULONG (*tFNInitiateNetworkRegistration)(ULONG regType,
								WORD mcc,
								WORD mnc,
								ULONG rat );

	typedef ULONG (*tFNGetServingNetwork)(ULONG * pRegistrationState,
								ULONG * pCSDomain,
								ULONG * pPSDomain,
								ULONG * pRAN,
								BYTE * pRadioIfacesSize,
								BYTE * pRadioIfaces,
								ULONG * pRoaming,
								WORD * pMCC,
								WORD * pMNC,
								BYTE nameSize,
								CHAR * pName );

	typedef ULONG (*tFNGetServingNetworkCapabilities)(BYTE * pDataCapsSize,
								BYTE * pDataCaps );

	typedef ULONG (*tFNGetHomeNetwork)(WORD * pMCC,
								WORD * pMNC,
								BYTE nameSize,
								CHAR * pName,
								WORD * pSID,
								WORD * pNID );
	//获得优先注册网络信息
	typedef ULONG (*tFNGetNetworkPreference)(ULONG * pTechnologyPref,
								ULONG * pDuration,
								ULONG * pPersistentTechnologyPref );
	
	typedef ULONG (*tFNSetNetworkPreference)(ULONG technologyPref,
								ULONG duration );
	/*Network access service end*/


	/*Device management service begin*/

	typedef ULONG (*tFNGetDeviceCapabilities)(ULONG * pMaxTXChannelRate,
								ULONG * pMaxRXChannelRate,
								ULONG * pDataServiceCapability,
								ULONG * pSimCapability,
								ULONG * pRadioIfacesSize,
								BYTE * pRadioIfaces );
	typedef ULONG (*tFNGetFirmwareRevision)(BYTE stringSize,
								CHAR * pString );
	typedef ULONG (*tFNGetFirmwareRevisions)(BYTE amssSize,
								CHAR * pAMSSString,
								BYTE bootSize,
								CHAR * pBootString,
								BYTE priSize,
								CHAR * pPRIString );
	typedef ULONG (*tFNGetIMSI)(BYTE stringSize,
								CHAR * pString );
	typedef ULONG (*tFNGetSerialNumbers)(BYTE esnSize,
								CHAR * pESNString,
								BYTE imeiSize,
								CHAR * pIMEIString,
								BYTE meidSize,
								CHAR * pMEIDString );
	//获取PRL（Preferred Roaming List）版本
	//Get Preferred Roaming List Version
	typedef ULONG (*tFNGetPRLVersion)( WORD * pPRLVersion );

	//OTA activate
	typedef ULONG (*tFNActivateAutomatic)( CHAR * pActivationCode );
	typedef ULONG (*tFNActivateManual)(CHAR * pSPC,
								WORD sid,
								CHAR * pMDN,
								CHAR * pMIN,
								ULONG prlSize,
								BYTE * pPRL,
								CHAR * pMNHA,
								CHAR * pMNAAA );
	typedef ULONG (*tFNGetActivationState)( ULONG * pActivationState );
	typedef void (*tFNActivationStatus)(ULONG activationStatus );
	typedef ULONG (*tFNSetActivationStatusCallback)( tFNActivationStatus pCallback );
	
	
	//Power mode management
	typedef ULONG (*tFNSetPower)( ULONG powerMode );
	typedef ULONG (*tFNGetPower)( ULONG * pPowerMode );
	typedef void (*tFNPower)(ULONG operatingMode);
	typedef ULONG (*tFNSetPowerCallback)( tFNPower pCallback );
	
	//This function returns reason why the operating mode of the device is currently offline.
	typedef ULONG (*tFNGetOfflineReason)(ULONG * pReasonMask,
								ULONG * pbPlatform );
	
	//UIM operate
	typedef ULONG (*tFNUIMSetPINProtection)(ULONG id,
								ULONG bEnable,
								CHAR * pValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft );

	typedef ULONG (*tFNUIMVerifyPIN)(ULONG id,
								CHAR * pValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft );

	typedef ULONG (*tFNUIMUnblockPIN)(ULONG id,
								CHAR * pPUKValue,
								CHAR * pNewValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft );

	typedef ULONG (*tFNUIMChangePIN)(ULONG id,
								CHAR * pOldValue,
								CHAR * pNewValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft );

	typedef ULONG (*tFNUIMGetPINStatus)(ULONG id,
								ULONG * pStatus,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft );

	typedef ULONG (*tFNUIMGetICCID)(BYTE stringSize,
								CHAR * pString );

	//Signal strength callback function

        typedef void (*tFNSignalStrength)(INT8 signalStrength,
								ULONG radioInterface );
	
	typedef ULONG (*tFNSetSignalStrengthCallback)(tFNSignalStrength pCallback,
								BYTE thresholdsSize,
								INT8 * pThresholds );

	//Reset to factory defaults
	typedef ULONG (*tFNResetToFactoryDefaults)( CHAR * pSPC );	
}


#endif //__GOBI_DEF_H__
