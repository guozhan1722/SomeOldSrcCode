#ifndef _GOBI_API_H_
#define _GOBI_API_H_
#include "GobiDef.h"


class GobiApi
{
public:
	static GobiApi * GetInstance();
	static void ReleaseInstance();
	
private:
	GobiApi(void);
	~GobiApi(void);
	void CleanUp();
	bool Initialize();
public:
	//Device connectivity service
	ULONG EnumerateDevices(BYTE * pDevicesSize, BYTE * pDevices );
	
	ULONG Connect(CHAR * pDeviceNode,CHAR * pDeviceKey );
	ULONG Disconnect();
	ULONG GetConnectedDeviceID(ULONG deviceNodeSize,
                                                                                 CHAR * pDeviceNode,
                                                                                 ULONG deviceKeySize,
                                                                                 CHAR * pDeviceKey );
    ULONG Cancel();


	//Wireless data service
	ULONG GetSessionState( ULONG * pState );
	ULONG StartDataSession(ULONG * pTechnology,
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
	ULONG CancelDataSession();
	ULONG StopDataSession( ULONG sessionId );
	ULONG GetIPAddress( ULONG * pIPAddress );	
    ULONG SetDefaultProfile(
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
    ULONG GetDefaultProfile(
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


    ULONG SetEnhancedAutoconnect(ULONG setting, ULONG *pRoamSetting);
    ULONG GetEnhancedAutoconnect(ULONG *pSetting, ULONG *pRoamSetting);
	 

	ULONG SetSessionStateCallback(tFNSessionState pCallback );

	 
	

	/*SMS begin*/
	
	ULONG DeleteSMS(ULONG storageType,
								ULONG * pMessageIndex,
								ULONG * pMessageTag );
	
	ULONG GetSMSList(ULONG storageType,
								ULONG * pRequestedTag,
								ULONG * pMessageListSize,
								BYTE * pMessageList );
	
	ULONG GetSMS(ULONG storageType,
								ULONG messageIndex,
								ULONG * pMessageTag,
								ULONG * pMessageFormat,
								ULONG * pMessageSize,
								BYTE * pMessage );
	
	ULONG ModifySMSStatus(ULONG storageType,
								ULONG messageIndex,
								ULONG messageTag );

	ULONG SaveSMS(ULONG storageType,
								ULONG messageFormat,
								ULONG messageSize,
								BYTE * pMessage,
								ULONG * pMessageIndex );

	ULONG SendSMS(ULONG messageFormat,
								ULONG messageSize,
								BYTE * pMessage,
								ULONG * pMessageFailureCode );
	/**
	*	获得短信中心地址
	*@param1[IN]	[BYTE *]addressSize:	地址大小
	*@param2[OUT]	[char *]pSMSCAddress:	输出短信中心地址
	*@param3[IN]	[BYTE] typeSize:		类型大小
	*@param4[OUT]	[char *]pSMSTyoe:		输出类型
	*	
	*@return ulong
	*	
	*/
	ULONG GetSMSCAddress(BYTE addressSize,
								CHAR * pSMSCAddress,
								BYTE typeSize,
								CHAR * pSMSCType );

	ULONG SetSMSCAddress(CHAR * pSMSCAddress,
								CHAR * pSMSCType );

	ULONG GetSMSRoutes(BYTE * pRouteSize,
								BYTE * pRoutes );

	ULONG SetSMSRoutes(BYTE * pRouteSize,
								BYTE * pRoutes );

	
	ULONG SetNewSMSCallback( tFNNewSMS pCallback );
	/*SMS end*/

	/*Network access service begin*/
	ULONG PerformNetworkScan(BYTE * pInstanceSize,
								BYTE * pInstances );

	ULONG PerformNetworkRATScan(BYTE * pInstanceSize,
								BYTE * pInstances,
								BYTE * pRATSize,
								BYTE * pRATInstances );

	ULONG InitiateNetworkRegistration(ULONG regType,
								WORD mcc,
								WORD mnc,
								ULONG rat );

	ULONG GetServingNetwork(ULONG * pRegistrationState,
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

	ULONG GetServingNetworkCapabilities(BYTE * pDataCapsSize,
								BYTE * pDataCaps );

	ULONG GetHomeNetwork(WORD * pMCC,
								WORD * pMNC,
								BYTE nameSize,
								CHAR * pName,
								WORD * pSID,
								WORD * pNID );
	ULONG GetNetworkPreference(ULONG * pTechnologyPref,
								ULONG * pDuration,
								ULONG * pPersistentTechnologyPref );

	ULONG SetNetworkPreference(ULONG technologyPref,
								ULONG duration );
	/*Network access service end*/


	/*Device management service begin*/

	ULONG GetDeviceCapabilities(ULONG * pMaxTXChannelRate,
								ULONG * pMaxRXChannelRate,
								ULONG * pDataServiceCapability,
								ULONG * pSimCapability,
								ULONG * pRadioIfacesSize,
								BYTE * pRadioIfaces );
	ULONG GetFirmwareRevision(BYTE stringSize,
								CHAR * pString );
	ULONG GetFirmwareRevisions(BYTE amssSize,
								CHAR * pAMSSString,
								BYTE bootSize,
								CHAR * pBootString,
								BYTE priSize,
								CHAR * pPRIString );
	ULONG GetIMSI(BYTE stringSize,
								CHAR * pString );
	ULONG GetSerialNumbers(BYTE esnSize,
								CHAR * pESNString,
								BYTE imeiSize,
								CHAR * pIMEIString,
								BYTE meidSize,
								CHAR * pMEIDString );
	//获取PRL（Preferred Roaming List）版本
	//Get Preferred Roaming List Version
	ULONG GetPRLVersion( WORD * pPRLVersion );

	//OTA activate
	ULONG ActivateAutomatic( CHAR * pActivationCode );
	ULONG ActivateManual(CHAR * pSPC,
								WORD sid,
								CHAR * pMDN,
								CHAR * pMIN,
								ULONG prlSize,
								BYTE * pPRL,
								CHAR * pMNHA,
								CHAR * pMNAAA );
	ULONG GetActivationState( ULONG * pActivationState );
	
	ULONG SetActivationStatusCallback( tFNActivationStatus pCallback );
	
	
	//Power mode management
	ULONG SetPower( ULONG powerMode );
	ULONG GetPower( ULONG * pPowerMode );
	
	ULONG SetPowerCallback( tFNPower pCallback );
	
	//This function returns reason why the operating mode of the device is currently offline.
	ULONG GetOfflineReason(ULONG * pReasonMask,
								ULONG * pbPlatform );
	
	//UIM operate
	ULONG UIMSetPINProtection(ULONG id,
								ULONG bEnable,
								CHAR * pValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft );

	ULONG UIMVerifyPIN(ULONG id,
								CHAR * pValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft );

	ULONG UIMUnblockPIN(ULONG id,
								CHAR * pPUKValue,
								CHAR * pNewValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft );

	ULONG UIMChangePIN(ULONG id,
								CHAR * pOldValue,
								CHAR * pNewValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft );

	ULONG UIMGetPINStatus(ULONG id,
								ULONG * pStatus,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft );

	ULONG UIMGetICCID(BYTE stringSize,
								CHAR * pString );
	//Signal strength callback function
	ULONG SetSignalStrengthCallback(tFNSignalStrength pCallback,
								BYTE thresholdsSize,
								INT8 * pThresholds );
	//Reset to factory defaults
	ULONG ResetToFactoryDefaults( CHAR * pSPC );	
private:
	//Device connectivity service
	tFNQCWWANEnumerateDevices aFnQCWWANEnumerateDevices;
	tFNQCWWANConnect aFnQCWWANConnect;
	tFNQCWWANDisconnect aFnQCWWANDisconnect;
	tFNQCWWAN2kGetConnectedDeviceID aFnQCWWAN2kGetConnectedDeviceID;
	tFNQCWWANCancel aFnQCWWANCancel;

	//Wireless data service
	tFNGetSessionState aFnGetSessionState;
	tFNStartDataSession aFnStartDataSession;
	tFNCancelDataSession aFnCancelDataSession;
	tFNStopDataSession aFnStopDataSession;
	tFNGetIPAddress aFnGetIPAddress;	

    tFNSetDefaultProfile aFnSetDefaultProfile;
    tFNGetDefaultProfile aFnGetDefaultProfile;
    tFNSetEnhancedAutoconnect aFnSetEnhancedAutoconnect;
    tFNGetEnhancedAutoconnect aFnGetEnhancedAutoconnect;


	tFNSetSessionStateCallback  aFnSetSessionStateCallback;

	 
	

	/*SMS begin*/
	tFNDeleteSMS aFnDeleteSMS;
	
	tFNGetSMSList aFnGetSMSList;
	
	tFNGetSMS aFnGetSMS;
	
	tFNModifySMSStatus aFnModifySMSStatus;

	tFNSaveSMS aFnSaveSMS;

	tFNSendSMS aFnSendSMS;

	tFNGetSMSCAddress aFnGetSMSCAddress;

	tFNSetSMSCAddress aFnSetSMSCAddress;

	tFNGetSMSRoutes aFnGetSMSRoutes;

	tFNSetSMSRoutes aFnSetSMSRoutes;

	

	tFNSetNewSMSCallback aFnSetNewSMSCallback;
	/*SMS end*/

	/*Network access service begin*/
	tFNPerformNetworkScan aFnPerformNetworkScan;

	tFNPerformNetworkRATScan aFnPerformNetworkRATScan;

	tFNInitiateNetworkRegistration aFnInitiateNetworkRegistration;

	tFNGetServingNetwork aFnGetServingNetwork;

	tFNGetServingNetworkCapabilities aFnGetServingNetworkCapabilities;

	tFNGetHomeNetwork aFnGetHomeNetwork;
	tFNGetNetworkPreference aFnGetNetworkPreference;
	tFNSetNetworkPreference aFnSetNetworkPreference;
	/*Network access service end*/


	/*Device management service begin*/

	tFNGetDeviceCapabilities aFnGetDeviceCapabilities;
	tFNGetFirmwareRevision  aFnGetFirmwareRevision;
	tFNGetFirmwareRevisions aFnGetFirmwareRevisions;
	tFNGetIMSI aFnGetIMSI;
	tFNGetSerialNumbers aFnGetSerialNumbers;
	//获取PRL（Preferred Roaming List）版本
	//Get Preferred Roaming List Version
	tFNGetPRLVersion aFnGetPRLVersion;

	//OTA activate
	tFNActivateAutomatic aFnActivateAutomatic;
	tFNActivateManual aFnActivateManual;
	tFNGetActivationState aFnGetActivationState;
	
	tFNSetActivationStatusCallback aFnSetActivationStatusCallback;
	
	
	//Power mode management
	tFNSetPower aFnSetPower;
	tFNGetPower aFnGetPower;
	
	tFNSetPowerCallback aFnSetPowerCallback;
	
	//This function returns reason why the operating mode of the device is currently offline.
	tFNGetOfflineReason aFnGetOfflineReason;
	
	//UIM operate
	tFNUIMSetPINProtection aFnUIMSetPINProtection;

	tFNUIMVerifyPIN aFnUIMVerifyPIN;

	tFNUIMUnblockPIN aFnUIMUnblockPIN;

	tFNUIMChangePIN aFnUIMChangePIN;

	tFNUIMGetPINStatus aFnUIMGetPINStatus;

	tFNUIMGetICCID aFnUIMGetICCID;

	//Signal strength callback function	
	tFNSetSignalStrengthCallback aFnSetSignalStrengthCallback;
	

	//Reset to factory defaults
	tFNResetToFactoryDefaults aFnResetToFactoryDefaults;

	void * dlHandle;
	bool    bIsConnected;
};

#endif //_GOBI_API_H_
























