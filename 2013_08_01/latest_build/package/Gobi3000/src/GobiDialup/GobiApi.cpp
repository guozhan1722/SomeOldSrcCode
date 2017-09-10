#include <dlfcn.h>
#include <stdio.h>
#include "GobiApi.h"

static GobiApi * g_pGobiApi    =NULL;
static int             g_iCount        =0;
GobiApi::GobiApi(void)
{
	Initialize();
}

GobiApi::~GobiApi(void)
{
	CleanUp();
}
GobiApi * GobiApi::GetInstance()
{
	if (NULL == g_pGobiApi)
	{
		g_pGobiApi = new GobiApi;
	}
	g_iCount++;
	return g_pGobiApi;
}
void GobiApi::ReleaseInstance()
{
	if(0 == --g_iCount)
	{
		if(NULL == g_pGobiApi)
		{
			delete g_pGobiApi;
			g_pGobiApi = NULL;
		}
	}
}
void GobiApi::CleanUp()
{
	if (dlHandle != NULL)
	{
		dlclose(dlHandle);
		dlHandle = NULL;
	}
	//Device connectivity service
	aFnQCWWANEnumerateDevices = NULL;
	aFnQCWWANConnect = NULL;
	aFnQCWWANDisconnect = NULL;
	aFnQCWWAN2kGetConnectedDeviceID = NULL;
	aFnQCWWANCancel = NULL;

	//Wireless data service
	aFnGetSessionState = NULL;
	aFnStartDataSession = NULL;
	aFnCancelDataSession = NULL;
	aFnStopDataSession = NULL;
	aFnGetIPAddress = NULL;	
    aFnSetDefaultProfile = NULL;
    aFnGetDefaultProfile = NULL;
    aFnSetEnhancedAutoconnect = NULL;
    aFnGetEnhancedAutoconnect = NULL;

	aFnSetSessionStateCallback = NULL;

	/*SMS begin*/
	aFnDeleteSMS = NULL;
	
	aFnGetSMSList = NULL;
	
	aFnGetSMS = NULL;
	
	aFnModifySMSStatus = NULL;

	aFnSaveSMS = NULL;

	aFnSendSMS = NULL;

	aFnGetSMSCAddress = NULL;

	aFnSetSMSCAddress = NULL;

	aFnGetSMSRoutes = NULL;

	aFnSetSMSRoutes = NULL;

	aFnSetNewSMSCallback = NULL;
	/*SMS end*/

	/*Network access service begin*/
	aFnPerformNetworkScan = NULL;

	aFnPerformNetworkRATScan = NULL;

	aFnInitiateNetworkRegistration = NULL;

	aFnGetServingNetwork = NULL;

	aFnGetServingNetworkCapabilities = NULL;

	aFnGetHomeNetwork = NULL;
	aFnGetNetworkPreference = NULL;
	aFnSetNetworkPreference = NULL;
	/*Network access service end*/


	/*Device management service begin*/

	aFnGetDeviceCapabilities = NULL;
	aFnGetFirmwareRevision = NULL;
	aFnGetFirmwareRevisions = NULL;
	aFnGetIMSI = NULL;
	aFnGetSerialNumbers = NULL;
	//获取PRL（Preferred Roaming List）版本
	//Get Preferred Roaming List Version
	aFnGetPRLVersion = NULL;

	//OTA activate
	aFnActivateAutomatic = NULL;
	aFnActivateManual = NULL;
	aFnGetActivationState = NULL;
	
	aFnSetActivationStatusCallback = NULL;
	
	
	//Power mode management
	aFnSetPower = NULL;
	aFnGetPower = NULL;
	
	aFnSetPowerCallback = NULL;
	
	//This function returns reason why the operating mode of the device is currently offline.
	aFnGetOfflineReason = NULL;
	
	//UIM operate
	aFnUIMSetPINProtection = NULL;

	aFnUIMVerifyPIN = NULL;

	aFnUIMUnblockPIN = NULL;

	aFnUIMChangePIN = NULL;

	aFnUIMGetPINStatus = NULL;

	aFnUIMGetICCID = NULL;
        aFnSetSignalStrengthCallback = NULL;
	//Reset to factory defaults
	aFnResetToFactoryDefaults = NULL;

	bIsConnected = false;
	
}

//Must call CleanUp() before call Initialize()
bool GobiApi::Initialize()
{
	CleanUp();
	if (dlHandle != NULL)
	{
        printf("dlHandle!=NULL)\n");
		return false;
	}
	else
	{
		dlHandle = dlopen(THEPAHTOFDYNAMICLIB,RTLD_NOW);
		if (dlHandle == NULL)
		{
            printf("dlHandle==NULL)\n");
            fprintf(stderr, "%s\n", dlerror());
			return false;
		}
	}
	//Device connectivity service
	aFnQCWWANEnumerateDevices =(tFNQCWWANEnumerateDevices)dlsym(dlHandle,QCWWAN_ENUMERATE_DEVICES);

	aFnQCWWANConnect = (tFNQCWWANConnect)dlsym(dlHandle,QCWWAN_CONNECT);
	aFnQCWWANDisconnect = (tFNQCWWANDisconnect)dlsym(dlHandle,QCWWAN_DISCONNECT);
	aFnQCWWAN2kGetConnectedDeviceID = (tFNQCWWAN2kGetConnectedDeviceID)dlsym(dlHandle,QCWWAN_GET_CONNECTED_DEVICEID);
	aFnQCWWANCancel = (tFNQCWWANCancel)dlsym(dlHandle,QCWWAN_CANCEL);

	//Wireless data service
	aFnGetSessionState = (tFNGetSessionState)dlsym(dlHandle,QCWWAN_GET_SESSION_STATE);
	aFnStartDataSession = (tFNStartDataSession)dlsym(dlHandle,QCWWAN_START_DATA_SESSION);
	aFnCancelDataSession = (tFNCancelDataSession)dlsym(dlHandle,QCWWAN_CANCEL_DATA_SESSION);
	aFnStopDataSession = (tFNStopDataSession)dlsym(dlHandle,QCWWAN_STOP_DATA_SESSION);
	aFnGetIPAddress = (tFNGetIPAddress)dlsym(dlHandle,QCWWAN_GET_IP_ADDRESS);	
    aFnSetDefaultProfile = (tFNSetDefaultProfile)dlsym(dlHandle,QCWWAN_SETDEFAULTPROFILE);
    aFnGetDefaultProfile = (tFNGetDefaultProfile)dlsym(dlHandle,QCWWAN_GETDEFAULTPROFILE);
    aFnSetEnhancedAutoconnect = (tFNSetEnhancedAutoconnect)dlsym(dlHandle,QCWWAN_SETENHANCEDAUTOCONNECT);
    aFnGetEnhancedAutoconnect = (tFNGetEnhancedAutoconnect)dlsym(dlHandle,QCWWAN_GETENHANCEDAUTOCONNECT);

	aFnSetSessionStateCallback = (tFNSetSessionStateCallback)dlsym(dlHandle,QCWWAN_SETS_ESSIONSTATE_CALLBACK);

	 
	/*SMS begin*/
	aFnDeleteSMS = (tFNDeleteSMS)dlsym(dlHandle,QCWWAN_DELETESMS);
	
	aFnGetSMSList = (tFNGetSMSList)dlsym(dlHandle,QCWWAN_GETSMSLIST);
	
	aFnGetSMS = (tFNGetSMS)dlsym(dlHandle,QCWWAN_GETSMS);
	
	aFnModifySMSStatus = (tFNModifySMSStatus)dlsym(dlHandle,QCWWAN_MODIFYSMSSTATUS);

	aFnSaveSMS = (tFNSaveSMS)dlsym(dlHandle,QCWWAN_SAVESMS);

	aFnSendSMS = (tFNSendSMS)dlsym(dlHandle,QCWWAN_SENDSMS);

	aFnGetSMSCAddress = (tFNGetSMSCAddress)dlsym(dlHandle,QCWWAN_GETSMSCADDRESS);

	aFnSetSMSCAddress = (tFNSetSMSCAddress)dlsym(dlHandle,QCWWAN_SETSMSCADDRESS);

	aFnGetSMSRoutes = (tFNGetSMSRoutes)dlsym(dlHandle,QCWWAN_GETSMSROUTES);

	aFnSetSMSRoutes = (tFNSetSMSRoutes)dlsym(dlHandle,QCWWAN_SETSMSROUTES);

	
	aFnSetNewSMSCallback = (tFNSetNewSMSCallback)dlsym(dlHandle,QCWWAN_SETNEWSMSCALLBACK);
	/*SMS end*/

	/*Network access service begin*/
	aFnPerformNetworkScan = (tFNPerformNetworkScan)dlsym(dlHandle,QCWWAN_PERFORM_NETWORK_SCAN);

	aFnPerformNetworkRATScan = (tFNPerformNetworkRATScan)dlsym(dlHandle,QCWWAN_PERFORM_NETWORK_RAT_SCAN);

	aFnInitiateNetworkRegistration = (tFNInitiateNetworkRegistration)dlsym(dlHandle,QCWWAN_INITIATE_NETWORK_REGISTRATION);

	aFnGetServingNetwork = (tFNGetServingNetwork)dlsym(dlHandle,QCWWAN_GET_SRV_NETWORK);

	aFnGetServingNetworkCapabilities = (tFNGetServingNetworkCapabilities)dlsym(dlHandle,QCWWAN_GET_SRV_NETWORK_CAP);

	aFnGetHomeNetwork = (tFNGetHomeNetwork)dlsym(dlHandle,QCWWAN_GET_HOME_NET_WORK);
	aFnGetNetworkPreference = (tFNGetNetworkPreference)dlsym(dlHandle,QCWWAN_GET_NETWORK_PREFERENCE);
	aFnSetNetworkPreference = (tFNSetNetworkPreference)dlsym(dlHandle,QCWWAN_SET_NETWORK_PREFERENCE);
	/*Network access service end*/


	/*Device management service begin*/

	aFnGetDeviceCapabilities = (tFNGetDeviceCapabilities)dlsym(dlHandle,QCWWAN_GETDEVICECAPABILITIES);
	aFnGetFirmwareRevision = (tFNGetFirmwareRevision)dlsym(dlHandle,QCWWAN_GETFIRMWAREREVISION);
	aFnGetFirmwareRevisions = (tFNGetFirmwareRevisions)dlsym(dlHandle,QCWWAN_GETFIRMWAREREVISIONS);
	aFnGetIMSI = (tFNGetIMSI)dlsym(dlHandle,QCWWAN_GETIMSI);
	aFnGetSerialNumbers = (tFNGetSerialNumbers)dlsym(dlHandle,QCWWAN_GETSERIALNUMBERS);
	//获取PRL（Preferred Roaming List）版本
	//Get Preferred Roaming List Version
	aFnGetPRLVersion = (tFNGetPRLVersion)dlsym(dlHandle,QCWWAN_GET_PRL_VERSION);

	//OTA activate
	aFnActivateAutomatic = (tFNActivateAutomatic)dlsym(dlHandle,QCWWAN_ACTIVATE_AUTOMATIC);
	aFnActivateManual = (tFNActivateManual)dlsym(dlHandle,QCWWAN_ACTIVATE_MANUAL);
	aFnGetActivationState = (tFNGetActivationState)dlsym(dlHandle,QCWWAN_GET_ACTIVATION_STATE);
	
	aFnSetActivationStatusCallback = (tFNSetActivationStatusCallback)dlsym(dlHandle,QCWWAN_SET_ACTIVATION_STATUS_CALL_BACK);
	
	
	//Power mode management
	aFnSetPower = (tFNSetPower)dlsym(dlHandle,QCWWAN_SET_POWER);
	aFnGetPower = (tFNGetPower)dlsym(dlHandle,QCWWAN_GET_POWER);
	
	aFnSetPowerCallback = (tFNSetPowerCallback)dlsym(dlHandle,QCWWAN_SET_POWER_MODE_CB);
	
	//This function returns reason why the operating mode of the device is currently offline.
	aFnGetOfflineReason = (tFNGetOfflineReason)dlsym(dlHandle,QCWWAN_GET_OFFLINE_REEASON);
	
	//UIM operate
	aFnUIMSetPINProtection = (tFNUIMSetPINProtection)dlsym(dlHandle,QCWWAN_UIM_SET_PIN_PROTECTION);

	aFnUIMVerifyPIN = (tFNUIMVerifyPIN)dlsym(dlHandle,QCWWAN_UIM_VERIFY_PIN);

	aFnUIMUnblockPIN = (tFNUIMUnblockPIN)dlsym(dlHandle,QCWWAN_UIM_UNBLOCK_PIN);

	aFnUIMChangePIN = (tFNUIMChangePIN)dlsym(dlHandle,QCWWAN_UIM_CHANGE_PIN);

	aFnUIMGetPINStatus = (tFNUIMGetPINStatus)dlsym(dlHandle,QCWWAN_UIM_GET_PIN_STATUS);

	aFnUIMGetICCID = (tFNUIMGetICCID)dlsym(dlHandle,QCWWAN_UIM_GET_ICCID);

         aFnSetSignalStrengthCallback = (tFNSetSignalStrengthCallback)dlsym(dlHandle,QCWWAN_SET_SIGNAL_STRENGTH_CALLBACK);
	//Reset to factory defaults
	aFnResetToFactoryDefaults = (tFNResetToFactoryDefaults)dlsym(dlHandle,QCWWAN_RESET_TO_FACTORY_DEF);
	return true;
	
}

//Device connectivity service
ULONG GobiApi:: EnumerateDevices(BYTE * pDevicesSize, BYTE * pDevices )
{
	ULONG ulerror = 1;
	if (aFnQCWWANEnumerateDevices !=  NULL)
	{
		ulerror = aFnQCWWANEnumerateDevices(pDevicesSize,pDevices);
	}
    else
    {
        printf("aFnQCWWANEnumerateDevices ==  NULL\n");
    }
	return ulerror;
}

ULONG GobiApi::Connect(CHAR * pDeviceNode,CHAR * pDeviceKey )
{
	ULONG ulerror = 1;
	if (aFnQCWWANConnect != NULL)
	{
		ulerror = aFnQCWWANConnect(pDeviceNode,pDeviceKey);
	}
	return ulerror;
}

ULONG GobiApi::Disconnect()
{
	ULONG ulerror = 1;
	if (aFnQCWWANDisconnect != NULL)
	{
		ulerror = aFnQCWWANDisconnect();
	}
	return ulerror;
}

ULONG GobiApi::GetConnectedDeviceID(ULONG deviceNodeSize,
                                                                                 CHAR * pDeviceNode,
                                                                                 ULONG deviceKeySize,
                                                                                 CHAR * pDeviceKey )
{
	ULONG ulerror = 1;
	if(aFnQCWWAN2kGetConnectedDeviceID != NULL)
	{
		ulerror = aFnQCWWAN2kGetConnectedDeviceID(deviceNodeSize,
												pDeviceNode,
												deviceKeySize,
												pDeviceKey);
	}
	return ulerror;
}

ULONG GobiApi::Cancel()
{
	ULONG ulerror = 1;
	if (aFnQCWWANCancel != NULL)
	{
		ulerror = aFnQCWWANCancel();
	}
	return ulerror;
}

ULONG GobiApi::SetDefaultProfile(
                                  ULONG  profileType,
                                  ULONG *pPDPType, 
                                  ULONG *pIPAddress, 
                                  ULONG *pPrimaryDNS, 
                                  ULONG *pSecondaryDNS, 
                                  ULONG *pAuthentication, 
                                  CHAR  *pName, 
                                  CHAR  *pAPNName, 
                                  CHAR  *pUsername,
                                  CHAR  *pPassword )
{
    ULONG ulerror = 1;
    if (aFnSetDefaultProfile != NULL)
    {
        printf("SetDefaultProfile");
        ulerror = aFnSetDefaultProfile(profileType,
                                             pPDPType,       
                                             pIPAddress,     
                                             pPrimaryDNS,    
                                             pSecondaryDNS,  
                                             pAuthentication,
                                             pName,          
                                             pAPNName,       
                                             pUsername,      
                                             pPassword );    
         
    }
    return ulerror;

}

ULONG GobiApi::GetDefaultProfile(
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
                            CHAR  *pUsername)
{
    ULONG ulerror = 1;
    if (aFnGetDefaultProfile != NULL)
    {
        printf("GetDefaultProfile");
        ulerror = aFnGetDefaultProfile(
                                        profileType,
                                        pPDPType,       
                                        pIPAddress,     
                                        pPrimaryDNS,    
                                        pSecondaryDNS,  
                                        pAuthentication,
                                        nameSize,
                                        pName,
                                        apnSize,       
                                        pAPNName,       
                                        userSize,
                                        pUsername);
    }
    return ulerror;

}


ULONG GobiApi::SetEnhancedAutoconnect(ULONG setting,
                                      ULONG *pRoamSetting)
{
    ULONG ulerror = 1;
    if (aFnSetEnhancedAutoconnect != NULL)
    {
        ulerror = aFnSetEnhancedAutoconnect(setting, pRoamSetting);
    }
    return ulerror;
}

ULONG GobiApi::GetEnhancedAutoconnect(ULONG *pSetting,
                                      ULONG *pRoamSetting)
{
    ULONG ulerror = 1;
    if (aFnGetEnhancedAutoconnect != NULL)
    {
        ulerror = aFnGetEnhancedAutoconnect(pSetting, pRoamSetting);
    }
    return ulerror;
}

//Wireless data service
ULONG GobiApi::GetSessionState( ULONG * pState )
{
	ULONG ulerror = 1;
	if (aFnGetSessionState != NULL)
	{
		ulerror = aFnGetSessionState(pState);
	}
	return ulerror;
}

ULONG GobiApi::StartDataSession(ULONG * pTechnology,
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
								ULONG * pFailureReason )
{
	ULONG ulerror = 1;
	if (aFnStartDataSession != NULL)
	{
		ulerror = aFnStartDataSession(pTechnology,
								pPrimaryDNS,
								pSecondaryDNS,
								pPrimaryNBNS,
								pSecondaryNBNS,
								pAPNName,
								pIPAddress,
								pAuthentication,
								pUsername,
								pPassword,
								pSessionId,
								pFailureReason);
	}
	return ulerror;
}

ULONG GobiApi::CancelDataSession()
{
	ULONG ulerror = 1;
	if (aFnCancelDataSession != NULL)
	{
		ulerror = aFnCancelDataSession();
	}
	return ulerror;
}

ULONG GobiApi::StopDataSession( ULONG sessionId )
{
	ULONG ulerror = 1;
	if (aFnStopDataSession != NULL)
	{
		ulerror = aFnStopDataSession(sessionId);
	}
	return ulerror;
}
	
	
ULONG GobiApi::GetIPAddress( ULONG * pIPAddress )
{
	ULONG ulerror = 1;
	if (aFnGetIPAddress != NULL)
	{
		ulerror = aFnGetIPAddress(pIPAddress);
	}
	return ulerror;
}

ULONG GobiApi::SetSessionStateCallback(tFNSessionState pCallback )
{
	ULONG ulerror = 1;
	if (aFnSetSessionStateCallback != NULL)
	{
		ulerror = aFnSetSessionStateCallback(pCallback);
	}
	return ulerror;
}

/*SMS begin*/
ULONG GobiApi::DeleteSMS(ULONG storageType,
						ULONG * pMessageIndex,
						ULONG * pMessageTag )
{
	ULONG ulerror = 1;
	if (aFnDeleteSMS != NULL)
	{
		ulerror = aFnDeleteSMS(storageType,
							pMessageIndex,
							pMessageTag);
	}
	return ulerror;
}

ULONG GobiApi::GetSMSList(ULONG storageType,
						ULONG * pRequestedTag,
						ULONG * pMessageListSize,
						BYTE * pMessageList )
{
	ULONG ulerror = 1;
	if (aFnGetSMSList != NULL)
	{
		ulerror =aFnGetSMSList(storageType,
							pRequestedTag,
							pMessageListSize,
							pMessageList);
	}
	return ulerror;
}

ULONG GobiApi::GetSMS(ULONG storageType,
					ULONG messageIndex,
					ULONG * pMessageTag,
					ULONG * pMessageFormat,
					ULONG * pMessageSize,
					BYTE * pMessage )
{
	ULONG ulerror = 1;
	if (aFnGetSMS != NULL)
	{
		ulerror = aFnGetSMS(storageType,
						messageIndex,
						pMessageTag,
						pMessageFormat,
						pMessageSize,
						pMessage);
	}
	return ulerror;

}
	
	
ULONG GobiApi::ModifySMSStatus(ULONG storageType,
								ULONG messageIndex,
								ULONG messageTag )
{
	ULONG ulerror = 1;
	if (aFnModifySMSStatus != NULL)
	{
		ulerror =aFnModifySMSStatus(storageType,
								messageIndex,
								messageTag);
	}
	return ulerror;
}
	
	
ULONG GobiApi::SaveSMS(ULONG storageType,
						ULONG messageFormat,
						ULONG messageSize,
						BYTE * pMessage,
						ULONG * pMessageIndex )
{
	ULONG ulerror = 1;
	if (aFnSaveSMS != NULL)
	{
		ulerror = aFnSaveSMS(storageType,
							messageFormat,
							messageSize,
							pMessage,
							pMessageIndex);
	}
	return ulerror;
}


ULONG GobiApi::SendSMS(ULONG messageFormat,
						ULONG messageSize,
						BYTE * pMessage,
						ULONG * pMessageFailureCode )
{
	ULONG ulerror = 1;
	if (aFnSendSMS != NULL)
	{
		ulerror = aFnSendSMS(messageFormat,
							messageSize,
							pMessage,
							pMessageFailureCode);
	}
	return ulerror;
}


ULONG GobiApi::GetSMSCAddress(BYTE addressSize,
							CHAR * pSMSCAddress,
							BYTE typeSize,
							CHAR * pSMSCType )
{
	ULONG ulerror = 1;
	if (aFnGetSMSCAddress != NULL)
	{
		ulerror =aFnGetSMSCAddress(addressSize,
								pSMSCAddress,
								typeSize,
								pSMSCType);
	}
	return ulerror;
}
								
ULONG GobiApi::SetSMSCAddress(CHAR * pSMSCAddress,
								CHAR * pSMSCType )
{
	ULONG ulerror = 1;
	if (aFnGetSMSCAddress != NULL)
	{
		ulerror =aFnSetSMSCAddress(pSMSCAddress,
								pSMSCType);
	}
	return ulerror;
}

ULONG GobiApi::GetSMSRoutes(BYTE * pRouteSize,
								BYTE * pRoutes )
{
	ULONG ulerror = 1;
	if (aFnSetSMSCAddress != NULL)
	{
		ulerror = aFnGetSMSRoutes(pRouteSize,
								pRoutes);
	}
	return ulerror;
}


ULONG GobiApi::SetSMSRoutes(BYTE * pRouteSize,
							BYTE * pRoutes )
{
	ULONG ulerror = 1;
	if (aFnSetSMSRoutes != NULL)
	{
		ulerror = aFnSetSMSRoutes(pRouteSize,
								pRoutes);
	}
	return ulerror;
}

ULONG GobiApi::SetNewSMSCallback( tFNNewSMS pCallback )
{
	ULONG ulerror  = 1;
	if (aFnSetNewSMSCallback != NULL)
	{
		ulerror = aFnSetNewSMSCallback(pCallback);
	}
	return ulerror;
}
/*SMS end*/


/*Network access service begin*/
ULONG GobiApi::PerformNetworkScan(BYTE * pInstanceSize,
								BYTE * pInstances )
{
	ULONG ulerror = 1;
	if (aFnPerformNetworkScan != NULL)
	{
		ulerror = aFnPerformNetworkScan(pInstanceSize,pInstances);
	}
	return ulerror;
}

ULONG GobiApi::PerformNetworkRATScan(BYTE * pInstanceSize,
								BYTE * pInstances,
								BYTE * pRATSize,
								BYTE * pRATInstances )
{
	ULONG ulerror = 1;
	if (aFnPerformNetworkRATScan != NULL)
	{
		ulerror = aFnPerformNetworkRATScan(pInstanceSize,
										pInstances,
										pRATSize,
										pRATInstances);
	}
	return ulerror;
}


ULONG GobiApi:: InitiateNetworkRegistration(ULONG regType,
											WORD mcc,
											WORD mnc,
											ULONG rat )
{
	ULONG ulerror = 1;
	if (aFnInitiateNetworkRegistration != NULL)
	{
		ulerror = aFnInitiateNetworkRegistration(regType,
											mcc,
											mnc,
											rat);
	}
	return ulerror;
}


ULONG GobiApi::GetServingNetwork(ULONG * pRegistrationState,
								ULONG * pCSDomain,
								ULONG * pPSDomain,
								ULONG * pRAN,
								BYTE * pRadioIfacesSize,
								BYTE * pRadioIfaces,
								ULONG * pRoaming,
								WORD * pMCC,
								WORD * pMNC,
								BYTE nameSize,
								CHAR * pName )
{
	ULONG ulerror = 1;
	if (aFnGetServingNetwork != NULL)
	{
		ulerror = aFnGetServingNetwork(pRegistrationState,
									pCSDomain,
									pPSDomain,
									pRAN,
									pRadioIfacesSize,
									pRadioIfaces,
									pRoaming,
									pMCC,
									pMNC,
									nameSize,
									pName);
	}
	return ulerror;
}


ULONG GobiApi::GetServingNetworkCapabilities(BYTE * pDataCapsSize,
											BYTE * pDataCaps )
{
	ULONG ulerror = 1;
	if (aFnGetServingNetworkCapabilities != NULL)
	{
		ulerror =aFnGetServingNetworkCapabilities(pDataCapsSize,
											pDataCaps);
	}
	return ulerror;
}

ULONG GobiApi::GetHomeNetwork(WORD * pMCC,
								WORD * pMNC,
								BYTE nameSize,
								CHAR * pName,
								WORD * pSID,
								WORD * pNID )
{
	ULONG ulerror = 1;
	if (aFnGetHomeNetwork != NULL)
	{
		ulerror = aFnGetHomeNetwork(pMCC,
								pMNC,
								nameSize,
								pName,
								pSID,
								pNID);
	}
	return ulerror;
}


ULONG GobiApi::GetNetworkPreference(ULONG * pTechnologyPref,
								ULONG * pDuration,
								ULONG * pPersistentTechnologyPref )
{
	ULONG ulerror = 1;
	if (aFnGetNetworkPreference != NULL)
	{
		ulerror = aFnGetNetworkPreference(pTechnologyPref,
									pDuration,
									pPersistentTechnologyPref);
	}
	return ulerror;
}


ULONG GobiApi::SetNetworkPreference(ULONG technologyPref,
								ULONG duration )
{
	ULONG ulerror = 1;
	if (aFnSetNetworkPreference != NULL)
	{
		ulerror = aFnSetNetworkPreference(technologyPref,
									duration);
	}
	return ulerror;
}
/*Network access service end*/


/*Device management service begin*/
ULONG GobiApi::GetDeviceCapabilities(ULONG * pMaxTXChannelRate,
								ULONG * pMaxRXChannelRate,
								ULONG * pDataServiceCapability,
								ULONG * pSimCapability,
								ULONG * pRadioIfacesSize,
								BYTE * pRadioIfaces )
{
	ULONG ulerror = 1;
	if (aFnGetDeviceCapabilities != NULL)
	{
		ulerror = aFnGetDeviceCapabilities(pMaxTXChannelRate,
									pMaxRXChannelRate,
									pDataServiceCapability,
									pSimCapability,
									pRadioIfacesSize,
									pRadioIfaces);
		
	}
	return ulerror;
}

ULONG GobiApi::GetFirmwareRevision(BYTE stringSize,
								CHAR * pString )
{
	ULONG ulerror = 1;
	if(aFnGetFirmwareRevision != NULL)
	{
		ulerror = aFnGetFirmwareRevision(stringSize,pString);
	}
	return ulerror;
}

ULONG GobiApi::GetFirmwareRevisions(BYTE amssSize,
								CHAR * pAMSSString,
								BYTE bootSize,
								CHAR * pBootString,
								BYTE priSize,
								CHAR * pPRIString )
{
	ULONG ulerror = 1;
	if (aFnGetFirmwareRevisions != NULL)
	{
		ulerror = aFnGetFirmwareRevisions(amssSize,
									pAMSSString,
									bootSize,
									pBootString,
									priSize,
									pPRIString);
	}
	return ulerror;
}

ULONG GobiApi::GetIMSI(BYTE stringSize,
					CHAR * pString )
{
	ULONG ulerror = 1;
	if (aFnGetIMSI != NULL)
	{
		ulerror =aFnGetIMSI(stringSize,pString);
	}
	return ulerror;
}

ULONG GobiApi::GetSerialNumbers(BYTE esnSize,
								CHAR * pESNString,
								BYTE imeiSize,
								CHAR * pIMEIString,
								BYTE meidSize,
								CHAR * pMEIDString )
{
	ULONG ulerror = 1;
	if (aFnGetSerialNumbers != NULL)
	{
		ulerror = aFnGetSerialNumbers(esnSize,
								pESNString,
								imeiSize,
								pIMEIString,
								meidSize,
								pMEIDString);
	}
	return ulerror;
}
	

//Get Preferred Roaming List Version
ULONG GobiApi::GetPRLVersion( WORD * pPRLVersion )
{
	ULONG ulerror = 1;
	if(aFnGetPRLVersion != NULL)
	{
		ulerror = aFnGetPRLVersion(pPRLVersion);
	}
	return ulerror;
}


//OTA activate
ULONG GobiApi::ActivateAutomatic( CHAR * pActivationCode )
{
	ULONG ulerror = 1;
	if(aFnActivateAutomatic != NULL)
	{
		ulerror = aFnActivateAutomatic(pActivationCode);
	}
	return ulerror;
}

ULONG GobiApi::ActivateManual(CHAR * pSPC,
								WORD sid,
								CHAR * pMDN,
								CHAR * pMIN,
								ULONG prlSize,
								BYTE * pPRL,
								CHAR * pMNHA,
								CHAR * pMNAAA )
{
	ULONG ulerror = 1;
	if(aFnActivateManual != NULL)
	{
		ulerror = aFnActivateManual(pSPC,
								sid,
								pMDN,
								pMIN,
								prlSize,
								pPRL,
								pMNHA,
								pMNAAA);
	}
	return ulerror;
}
	
ULONG GobiApi::GetActivationState( ULONG * pActivationState )
{
	ULONG ulerror = 1;
	if (aFnGetActivationState != NULL)
	{
		ulerror = aFnGetActivationState(pActivationState);
	}
	return ulerror;
}

ULONG GobiApi::SetActivationStatusCallback( tFNActivationStatus pCallback )
{
	ULONG ulerror = 1;
	if(aFnSetActivationStatusCallback != NULL)
	{
		ulerror = aFnSetActivationStatusCallback(pCallback);
	}
	return ulerror;
}
	

	
	
//Power mode management
ULONG GobiApi::SetPower( ULONG powerMode )
{
	ULONG ulerror = 1;
	if (aFnSetPower != NULL)
	{
		ulerror = aFnSetPower(powerMode);
	}
	return ulerror;
}

ULONG GobiApi::GetPower( ULONG * pPowerMode )
{
	ULONG ulerror = 1;
	if (aFnGetPower != NULL)
	{
		ulerror = aFnGetPower(pPowerMode);
	}
	return ulerror;
}
	
ULONG GobiApi::SetPowerCallback( tFNPower pCallback )
{
	ULONG ulerror = 1;
	if (aFnSetPowerCallback != NULL)
	{
		ulerror = aFnSetPowerCallback(pCallback);
	}
	return ulerror;
}
	
	
//This function returns reason why the operating mode of the device is currently offline.
ULONG GobiApi::GetOfflineReason(ULONG * pReasonMask,
								ULONG * pbPlatform )
{
	ULONG ulerror = 1;
	if(aFnGetOfflineReason != NULL)
	{
		ulerror = aFnGetOfflineReason(pReasonMask,pbPlatform);
	}
	return ulerror;
}

	
//UIM operate
ULONG GobiApi::UIMSetPINProtection(ULONG id,
								ULONG bEnable,
								CHAR * pValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft )
{
	ULONG ulerror = 1;
	if (aFnUIMSetPINProtection != NULL)
	{
		ulerror = aFnUIMSetPINProtection(id,
								bEnable,
								pValue,
								pVerifyRetriesLeft,
								pUnblockRetriesLeft);
	}
	return ulerror;
}

ULONG GobiApi::UIMVerifyPIN(ULONG id,
								CHAR * pValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft )
{
	ULONG ulerror = 1;
	if (aFnUIMVerifyPIN != NULL)
	{
		ulerror = aFnUIMVerifyPIN(id,
							pValue,
							pVerifyRetriesLeft,
							pUnblockRetriesLeft);
	}
	return ulerror;
}

ULONG GobiApi::UIMUnblockPIN(ULONG id,
								CHAR * pPUKValue,
								CHAR * pNewValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft )
{
	ULONG ulerror = 1;
	if (aFnUIMUnblockPIN != NULL)
	{
		ulerror = aFnUIMUnblockPIN(id,
								pPUKValue,
								pNewValue,
								pVerifyRetriesLeft,
								pUnblockRetriesLeft);
	}
	return ulerror;
}

ULONG GobiApi::UIMChangePIN(ULONG id,
								CHAR * pOldValue,
								CHAR * pNewValue,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft )
{
	ULONG ulerror = 1;
	if(aFnUIMChangePIN != NULL)
	{
		ulerror = aFnUIMChangePIN(id,
								pOldValue,
								pNewValue,
								pVerifyRetriesLeft,
								pUnblockRetriesLeft);
	}
	return ulerror;
}

ULONG GobiApi::UIMGetPINStatus(ULONG id,
								ULONG * pStatus,
								ULONG * pVerifyRetriesLeft,
								ULONG * pUnblockRetriesLeft )
{
	ULONG ulerror = 1;
	if (aFnUIMGetPINStatus != NULL)
	{
		ulerror = aFnUIMGetPINStatus(id,
								pStatus,
								pVerifyRetriesLeft,
								pUnblockRetriesLeft);
	}
	return ulerror;
}


ULONG GobiApi::UIMGetICCID(BYTE stringSize,
								CHAR * pString )
{
	ULONG ulerror = 1;
	if (aFnUIMGetICCID != NULL)
	{
		ulerror = aFnUIMGetICCID(stringSize,
								pString);
	}
	return ulerror;
}

//Signal strength callback function
ULONG GobiApi::SetSignalStrengthCallback(tFNSignalStrength pCallback,
								BYTE thresholdsSize,
								INT8 * pThresholds )
{
	ULONG ulerror = 1;
	if (aFnSetSignalStrengthCallback != NULL)
	{
		ulerror =aFnSetSignalStrengthCallback(pCallback,
										thresholdsSize,
										pThresholds);
	}
	return ulerror;
}

//Reset to factory defaults
ULONG GobiApi::ResetToFactoryDefaults( CHAR * pSPC )
{
	ULONG ulerror = 1;
	if (aFnResetToFactoryDefaults != NULL)
	{
		ulerror = aFnResetToFactoryDefaults(pSPC);
	}
	return ulerror;
}





































