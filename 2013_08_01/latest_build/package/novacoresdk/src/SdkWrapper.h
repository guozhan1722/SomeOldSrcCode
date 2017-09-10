/**
*
* Copyright 2008 Novatel Wireless Inc.
*
*/

#ifndef _SDK_WRAPPER_H
#define _SDK_WRAPPER_H

#include "platform_inc.h"
#include "NvtlApiDefinitions.h"

#if defined(_NVTL_WINDOWS_)
#include <windows.h>
#include <stdio.h>
#include <wtypes.h>

#define LOAD_OBJECT(x)		LoadLibraryA(x)
#define UNLOAD_OBJECT		FreeLibrary
#define LOAD_PROC			GetProcAddress
#define OBJECT_TYPE			HMODULE

#define BASESDK_OBJECT_NAME		"NvtlSdk.dll"
#define Activation_OBJECT_NAME	"NvtlActv.dll"
#define Connection_OBJECT_NAME	"NvtlConn.dll"
#define Diagnostics_OBJECT_NAME	"NvtlDiag.dll"
#define Gps_OBJECT_NAME	        "NvtlGps.dll"
#define FileManager_OBJECT_NAME	"NvtlFile.dll"
#define SmsEncoder_OBJECT_NAME	"NvtlEnc.dll"
#define Gobi_OBJECT_NAME		"NvtlGobi.dll"

#define LOAD_FUNCTION_PTR( funcPtr, funcType, funcName , handle ) \
    if( ! funcPtr ) { \
        funcPtr = (funcType)GetProcAddress( handle, funcName ); \
        if( ! funcPtr ) { \
            return 1; \
        } \
    }

#elif DEFINED_MAC

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include <time.h>

#define LOAD_OBJECT(x)		dlopen( x, RTLD_LAZY )
#define UNLOAD_OBJECT		dlclose
#define LOAD_PROC			dlsym
#define OBJECT_TYPE			void*

#define BASESDK_OBJECT_NAME         "@executable_path/../Frameworks/NvtlSdk.framework/NvtlSdk"
//#define BASESDK_OBJECT_NAME		"NvtlSdk.dylib"
#define Activation_OBJECT_NAME         "@executable_path/../Frameworks/NvtlActivation.framework/NvtlActivation"
//#define Activation_OBJECT_NAME  "NvtlActivation.dylib"
#define Connection_OBJECT_NAME         "@executable_path/../Frameworks/NvtlConnection.framework/NvtlConnection"
//#define Connection_OBJECT_NAME  "NvtlConnection.dylib"
#define Diagnostics_OBJECT_NAME         "@executable_path/../Frameworks/NvtlDiagnostics.framework/NvtlDiagnostics"
//#define Diagnostics_OBJECT_NAME  "NvtlDiagnostics.dylib"
#define Gps_OBJECT_NAME         "@executable_path/../Frameworks/NvtlGps.framework/NvtlGps"
//#define Gps_OBJECT_NAME         "NvtlGps.dylib"
#define FileManager_OBJECT_NAME         "@executable_path/../Frameworks/NvtlFileManager.framework/NvtlFileManager"
//#define FileManager_OBJECT_NAME  "NvtlFileManager.dylib"
#define SmsEncoder_OBJECT_NAME         "@executable_path/../Frameworks/NvtlSms.framework/NvtlSms"
//#define SmsEncoder_OBJECT_NAME  "NvtlSms.dylib"
#define Gobi_OBJECT_NAME         "@executable_path/../Frameworks/NvtlGobi.framework/NvtlGobi"
//#define Gobi_OBJECT_NAME  "NvtlGobi.dylib"

#ifndef MAX_PATH
#define MAX_PATH 255
#endif

#define LOAD_FUNCTION_PTR( funcPtr, funcType, funcName , handle ) \
    if( ! funcPtr ) { \
        funcPtr = (funcType)dlsym( handle, funcName ); \
        if( ! funcPtr ) { \
            return -1; \
        } \
    }


#else

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include <time.h>

#define LOAD_OBJECT(x)		dlopen( x, RTLD_LAZY)
#define UNLOAD_OBJECT		dlclose
#define LOAD_PROC			dlsym
#define OBJECT_TYPE			void*

#define BASESDK_OBJECT_NAME		"nvtlsdk.so"
#define Activation_OBJECT_NAME  "nvtlactv.so"
#define Connection_OBJECT_NAME  "nvtlconn.so"
#define Diagnostics_OBJECT_NAME "nvtldiag.so"
#define Gps_OBJECT_NAME         "nvtlgps.so"
#define FileManager_OBJECT_NAME "nvtlfile.so"
#define SmsEncoder_OBJECT_NAME  "nvtlenc.so"
#define Gobi_OBJECT_NAME		"nvtlgobi.so"

#define LOAD_FUNCTION_PTR( funcPtr, funcType, funcName , handle ) \
    if( ! funcPtr ) { \
        funcPtr = (funcType)dlsym( handle, funcName ); \
        if( ! funcPtr ) { \
            return -1; \
        } \
    }

#endif

typedef SdkHandle ( NOVATEL_CALL *Func_NvtlCommon_CreateSession)();
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_ReleaseSession)( SdkHandle );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetSdkVersion)( char* buffer, unsigned short buffer_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_SetSdkMode)( NvtlSdkModeType sdk_mode, const char* szAccessKey );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetDeviceInfo)(SdkHandle, DeviceInfoStruct* pDevInfo);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetNetworkInfo)(SdkHandle, NetworkInfoStruct* pNetworkInfo);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_ReadContact)(SdkHandle, short index, ContactInfoStruct* pContact);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_ReadMultipleContacts)( SdkHandle session, unsigned short start_index, unsigned short num_to_read, ContactInfoStruct* pContact, unsigned short* pRead );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_WriteContact)(SdkHandle, ContactInfoStruct* pContact);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetContactSizeDetails)(SdkHandle, ContactSizeInfoStruct* pContactDetails);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetPowerMode)(SdkHandle session, unsigned short* mode, unsigned char* disableMask );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_SetPowerMode)(SdkHandle session, unsigned short val );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_RadioState)( SdkHandle session,  PropertyAction bPropAction, unsigned short* state );

typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_Reset)(SdkHandle session );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_RegisterEventCallback)( SdkHandle session, NvtlEventCallback cb );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_UnregisterEventCallback)( SdkHandle session, NvtlEventCallback cb );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_AttachDevice)( SdkHandle session, DeviceDetail* pDeviceDetail );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_DetachDevice)( SdkHandle session );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_RefreshDeviceInfo)(SdkHandle session);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetAvailableDevices)( SdkHandle session, DeviceDetail* pDevList, uint32_t* dev_list_size );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_SetDeviceAccessKey)( SdkHandle session, char *accessKey );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_ConfigureLogging)( const char* filename, SdkLogLevelType logLevel, SdkLogOutputType output );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_UnlockDevice)( SdkHandle session, unsigned char lockType, char* lockCode );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_SetAutoLock)( SdkHandle session, unsigned char lockOn, char* lockCode );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetLockStatus)( SdkHandle session, int32_t* lockStatus, unsigned char* isAutoLockOn );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetLockStatusEx)( SdkHandle session, int32_t* lockStatus, unsigned char* isAutoLockOn, int32_t *retries );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_ChangeLockCode)( SdkHandle session, char* szOldCode, char* szNewCode );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_NetworkPreference)( SdkHandle session,  PropertyAction bPropAction, unsigned char* pref );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetRSSI)( SdkHandle session, RSSIStruct *rssi );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_NovaSpeedEnabled)( SdkHandle session, PropertyAction bSet, unsigned char *NSEnabled );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetTemperature)( SdkHandle session, unsigned short* temp );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetLifetimeTimer)( SdkHandle session, uint32_t* count );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetDisplaySpeed)( SdkHandle session, unsigned char nTheoretical, double* speed );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_AutoInstallEnabled)( SdkHandle session, PropertyAction bSet, unsigned char* bEnabled );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_InternalMassStorageEnabled)( SdkHandle session, PropertyAction bSet, unsigned char* bEnabled );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_RestoreToFactoryDefaults)( SdkHandle session, char* spcCode );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_RestoreToFactoryDefaultsBasic)( SdkHandle session );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_IsLPMSupported)( SdkHandle session, unsigned char* supported );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetRefurbInfo)( SdkHandle session, unsigned char* status, uint32_t* date, char* provider, unsigned short provider_len);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_SetSystemPreferredHybridMode)( SdkHandle session, PropertyAction bSet, unsigned short newPrefMode, unsigned short newHybridMode, unsigned short* oldPrefMode, unsigned short* oldHybridMode );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_IsSMSSupported)( SdkHandle hClient, unsigned char* bSupported );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_ReadSmsMsg)( SdkHandle hClient, unsigned short index, SmsMessageStruct* msg );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_ReadSmsMsgEx)( SdkHandle hClient, unsigned short storageType, unsigned short index, SmsMessageStruct* msg );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_SendSmsMsg)( SdkHandle hClient, SmsMessageStruct* msg );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_DeleteSmsMsg)( SdkHandle hClient, unsigned short index );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_DeleteSmsMsgEx)( SdkHandle hClient, unsigned short storageType, unsigned short index );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetSmsList)( SdkHandle hClient, SmsStateType eSmsType, SmsMessageInfo* info_array, unsigned short array_size, unsigned short* msg_count);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetSmsListEx)( SdkHandle hClient, unsigned short storageType, SmsStateType eSmsType, SmsMessageInfo* info_array, unsigned short array_size, unsigned short* msg_count);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetSmsRxTimestamp)( SdkHandle hClient, unsigned short index, NvtlTimeStruct* timestamp );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_ClearSmsRxTimestamps)( SdkHandle hClient );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetSmsFormat)( SdkHandle hClient, SmsMessageType *pSmsFormat );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetDataCounters)( SdkHandle session, uint32_t* dataTx, uint32_t* dataRx );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetBatteryState)( SdkHandle session, unsigned short* state , unsigned short* chargePct );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetManufacturer)( SdkHandle session, char* buffer, unsigned short* buffer_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetMACAddress)( SdkHandle session, char *buffer, unsigned short* buffer_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetSINR) (SdkHandle session, unsigned short *wCurrMode, int32_t * iSINR);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetIMSRegStatus) (SdkHandle session, unsigned char * ucIMSRegStatus);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetCallHistoryIndexList) (SdkHandle session, uint32_t * pIndexList, uint32_t * pIndexcount);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetCallHistoryForItem) (SdkHandle session, uint32_t index ,CallHistoryItemType *pCallHistoryItem);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_ClearCallHistory) (SdkHandle session, uint32_t *pdwError);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_FatalErrorOption) (SdkHandle session, PropertyAction bSet, unsigned short *pOption);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_ConfigureLoggingMaxFileSize) (SdkHandle session, uint64_t maxFileSize);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetSARRFState) (SdkHandle session, unsigned long *pdwError, char *key);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_SetSARRFState) (SdkHandle session, unsigned long *pdwError, char *key);

typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_BandPreference)( SdkHandle session,  PropertyAction bPropAction, unsigned char nam, BandPreferenceType* pref );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_GetDateTime)( SdkHandle session, NvtlTimeStruct* ts );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_GetEriInfo)( SdkHandle session, EriInfoStruct* eriInfo );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_RoamPreference)( SdkHandle session,  PropertyAction bPropAction, unsigned char nam, RoamPreferenceType* pref );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_IntlRoamPreference)( SdkHandle session,  PropertyAction bPropAction, unsigned char* pref );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_MicroSDControl)( SdkHandle session, PropertyAction bPropAction, unsigned char* enabled );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_LedControl)( SdkHandle session, PropertyAction bPropAction, unsigned char* enabled );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_GetMobileId)( SdkHandle session, char* buffer, unsigned short* buffer_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_GetMeid)( SdkHandle session, char* buffer, unsigned short* buffer_len );
//typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_GetMIPError)( SdkHandle session, uint32_t* error );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_GetEsnAsDecimal)( SdkHandle session, char* decESN, unsigned short* decEsn_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_ReadPBEntry)( SdkHandle session,  uint32_t index, unsigned char *pBuff, uint32_t *dwBuffLen, uint32_t *dwErr );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_WritePBEntry)( SdkHandle session,  uint32_t index, unsigned char *pBuff, uint32_t dwBuffLen, uint32_t *dwErr );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_ClearPBEntry)( SdkHandle session,  uint32_t index, uint32_t *dwErr );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_ClearAllPBEntries)( SdkHandle session,  uint32_t *dwErr );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_MultiModeRssiRange)( SdkHandle session,  PropertyAction bSet, unsigned char *u8Var );

//GSM functions
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_SMSC)( SdkHandle hClient, PropertyAction getOrSet, char* buffer, unsigned short* buffer_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_GetImsi)( SdkHandle hClient, char* buffer, unsigned short* buffer_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_GetIccid)( SdkHandle hClient, char* buffer, unsigned short* buffer_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_GetNetworkMCCMNC)( SdkHandle hClient, char* buffer, unsigned short* buffer_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_GetNetworkOperator)( SdkHandle hClient, char* buffer, unsigned short* buffer_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_SetNetworkOperator)( SdkHandle hClient, unsigned short mode, unsigned short format, unsigned short accessTech, char* szOper);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_GetNetworkOperatorList)( SdkHandle hClient, char* buffer, unsigned short* buffer_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_SendATCommand)( SdkHandle hClient, char* command, char* prompt, char* response, uint32_t response_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_Band)( SdkHandle hClient, PropertyAction bPropAction, uint32_t* band );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_SendStoredSms)( SdkHandle hClient, unsigned short index );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_SetSmsStorage)( SdkHandle hClient, const char* name );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_SmsStorageEx)( SdkHandle hClient, PropertyAction bPropAction, unsigned short *storageType );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_GetContactStorage)( SdkHandle hClient, unsigned short* numRecords, unsigned short* maxRecords, char* buffer, unsigned short* buffer_len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_SetContactStorage)( SdkHandle hClient, char* szStorageName );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_WriteSmsMsg)( SdkHandle hClient, SmsMessageStruct* msg, unsigned short* index);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_IsMsgMemoryFull)( SdkHandle hClient, unsigned char* full );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_IsMsgMemoryFullEx)( SdkHandle hClient, unsigned short storagType, unsigned char* full );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_GetEnsInfo)( SdkHandle hClient, uint32_t* ensMode );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_IsManualNetworkSelected)( SdkHandle session, unsigned char* bManual);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_DomainAttach)( SdkHandle session,  PropertyAction bPropAction, unsigned short* state );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlGsm_GetNetworkCapabilities)( SdkHandle session,  uint32_t *current, uint32_t *available, uint32_t *supported);

typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_DeviceStatusNotify4G )( SdkHandle session, unsigned char status);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_LedConfiguration4G )( SdkHandle session, PropertyAction bSet, LedConfig4G *pConfig);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_DualWorkModeSetting4G )( SdkHandle session, PropertyAction bSet, unsigned char* pSetting );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetInterfaceMode )( SdkHandle session, unsigned char* mode );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_Set3GLimitedPower)( SdkHandle session, unsigned char mode );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetLTENetworkType)( SdkHandle session, LTENetworkType *eNetworkType );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetMSISDN)( SdkHandle session, char *buffer, unsigned short *len );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetMultiModeIpAddress) (SdkHandle session, MultiModeIPAddress *buffer);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_LteAPN)(SdkHandle clientId, PropertyAction bSet, unsigned char *apn_class, char *apn_string, unsigned short *apn_strlen );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_LanAutoConnectionMode)(SdkHandle clientId, PropertyAction bSet, unsigned char *pLanConnectionMode);
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_MicroSDControl)( SdkHandle session, PropertyAction bPropAction, unsigned char* enabled );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_GetCurrentChannelData) ( SdkHandle session, ChannelBandStruct *channelData );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlCommon_LanConnectionStatus)(SdkHandle clientId, LanConnectionStatusStruct * pStatus );
typedef unsigned short ( NOVATEL_CALL *Func_NvtlEvdo_GetCurrentSID)( SdkHandle session, int32_t* iSid );


class SdkWrapper
{
	public:
		SdkWrapper();
		virtual ~SdkWrapper();

		bool		IsLoaded();

        unsigned short GetSdkVersion( char* buffer, unsigned short buffer_len );
		unsigned short CreateSession();
        bool           HasValidSession();
        unsigned short SetSdkMode( NvtlSdkModeType sdk_mode, const char* szAccessKey );
		unsigned short ReleaseSession();
		unsigned short RefreshDeviceInfo();
		unsigned short AttachDevice( DeviceDetail* pDeviceDetail );
		unsigned short DetachDevice();
		unsigned short GetAvailableDevices( DeviceDetail* pDevList, uint32_t* dev_list_size );
		unsigned short SetDeviceAccessKey( char *accessKey );
        unsigned short RegisterEventCallback( NvtlEventCallback& cb );
		unsigned short UnregisterEventCallback( NvtlEventCallback& cb );
		unsigned short GetNetworkInfo( NetworkInfoStruct* pNetworkInfo );
		unsigned short GetDeviceInfo( DeviceInfoStruct* pDeviceInfo );
		unsigned short ReadContact(short index, ContactInfoStruct* pContact);
        unsigned short ReadMultipleContacts( unsigned short start_index, unsigned short num_to_read, ContactInfoStruct* pContact, unsigned short* pRead );
		unsigned short WriteContact(ContactInfoStruct* pContact);
		unsigned short GetContactSizeDetails(ContactSizeInfoStruct* pContactDetails);
		unsigned short GetPowerMode(unsigned short* mode, unsigned char* disableMask );
		unsigned short SetPowerMode(unsigned short val );
        unsigned short RadioState( PropertyAction bPropAction, unsigned short* state );
		unsigned short Reset( );
		unsigned short ConfigureLogging( const char* filename, SdkLogLevelType logLevel, SdkLogOutputType output );
        unsigned short UnlockDevice( unsigned char lockType, char* lockCode );
        unsigned short SetAutoLock( unsigned char lockOn, char* lockCode );
        unsigned short GetLockStatus( int32_t* lockStatus, unsigned char* isAutoLockOn );
        unsigned short GetLockStatusEx( int32_t* lockStatus, unsigned char* isAutoLockOn, int32_t *retries );
        unsigned short ChangeLockCode( char* szOldCode, char* szNewCode );
        unsigned short NetworkPreference( PropertyAction bPropAction, unsigned char* pref );
		unsigned short NovaSpeedEnabled( PropertyAction bSet, unsigned char *NSEnabled );
		unsigned short GetRSSI( RSSIStruct *rssi );
        unsigned short AutoInstallEnabled( PropertyAction bSet, unsigned char* bEnabled );
        unsigned short InternalMassStorageEnabled( PropertyAction bSet, unsigned char* bEnabled );
        unsigned short RestoreToFactoryDefaults( char* spcCode );
        unsigned short RestoreToFactoryDefaultsBasic();
		unsigned short IsLPMSupported( unsigned char* supported );
        unsigned short GetRefurbInfo( unsigned char* status, uint32_t* date, char* provider, unsigned short provider_len);
		unsigned short SetSystemPreferredHybridMode( PropertyAction bSet, unsigned short newPrefMode, unsigned short newHybridMode, unsigned short* oldPrefMode, unsigned short* oldHybridMode );
        unsigned short BandPreference( PropertyAction bPropAction, unsigned char nam, BandPreferenceType* pref );
        unsigned short GetTemperature( unsigned short* temp );
        unsigned short GetLifetimeTimer( uint32_t* count );
		unsigned short GetDisplaySpeed( unsigned char nTheoretical, double* speed );
        unsigned short GetDataCounters( uint32_t* dataTx, uint32_t* dataRx );
        unsigned short GetBatteryState( unsigned short* state , unsigned short* chargePct );
        unsigned short GetManufacturer( char* buffer, unsigned short* buffer_len );
		unsigned short GetMACAddress( char *buffer, unsigned short* buffer_len );

        unsigned short GetDateTime( NvtlTimeStruct* ts );
		unsigned short GetEriInfo( EriInfoStruct* eriInfo );
        unsigned short RoamPreference( PropertyAction bPropAction, unsigned char nam, RoamPreferenceType* pref );
        unsigned short IntlRoamPreference( PropertyAction bPropAction, unsigned char* pref );
        unsigned short MicroSDControl( PropertyAction bPropAction, unsigned char* enabled );
        unsigned short LedControl( PropertyAction bPropAction, unsigned char* enabled );

        unsigned short IsSMSSupported( unsigned char *bSupported );
        unsigned short ReadSmsMsg( unsigned short index, SmsMessageStruct* msg );
        unsigned short ReadSmsMsgEx( unsigned short storageType, unsigned short index, SmsMessageStruct* msg );
        unsigned short SendSmsMsg( SmsMessageStruct* msg );
        unsigned short DeleteSmsMsg( unsigned short index );
        unsigned short DeleteSmsMsgEx( unsigned short storageType, unsigned short index );
        unsigned short GetSmsList( SmsStateType eSmsType, SmsMessageInfo* info_array, unsigned short array_size, unsigned short* msg_count );
        unsigned short GetSmsListEx( unsigned short storageType, SmsStateType eSmsType, SmsMessageInfo* info_array, unsigned short array_size, unsigned short* msg_count );
        unsigned short GetSmsRxTimestamp( unsigned short index, NvtlTimeStruct* timestamp );
        unsigned short ClearSmsRxTimestamps();
        unsigned short GetSmsFormat( SmsMessageType *pSmsFormat);


        unsigned short SMSC( PropertyAction bPropSet, char* buffer, unsigned short* buffer_len );
        unsigned short GetImsi( char* buffer, unsigned short* buffer_len );
        unsigned short GetIccid( char* buffer, unsigned short* buffer_len );
        unsigned short GetNetworkMCCMNC( char* buffer, unsigned short* buffer_len );
        unsigned short GetNetworkOperator( char* buffer, unsigned short* buffer_len );
        unsigned short SetNetworkOperator( unsigned short mode, unsigned short format, unsigned short accessTech, char* szOper);
        unsigned short GetNetworkOperatorList( char* buffer, unsigned short* buffer_len );
        unsigned short SendATCommand( char* command, char* prompt, char* response, uint32_t response_len );
        unsigned short Band( PropertyAction bPropAction, uint32_t* band );
        unsigned short SendStoredSms( unsigned short index );
        unsigned short SetSmsStorage( const char* name );
        unsigned short SmsStorageEx( PropertyAction bPropAction, unsigned short *storageType );
        unsigned short GetContactStorage( unsigned short* numRecords, unsigned short* maxRecords, char* buffer, unsigned short* buffer_len );
        unsigned short SetContactStorage( char* szStorageName );
        unsigned short WriteSmsMsg( SmsMessageStruct* msg, unsigned short* index);
        unsigned short IsMsgMemoryFull( unsigned char* full );
        unsigned short IsMsgMemoryFullEx( unsigned short storageType, unsigned char* full );
        unsigned short GetEnsInfo( uint32_t* ensMode );
        unsigned short GetMobileId( char* buffer, unsigned short* buffer_len );
        unsigned short GetMeid( char* buffer, unsigned short* buffer_len );
        unsigned short IsManualNetworkSelected( unsigned char* bManual );
        unsigned short DomainAttach( PropertyAction bPropAction, unsigned short* state );
        unsigned short GetNetworkCapabilities( uint32_t *current, uint32_t *available, uint32_t *supported );

        //unsigned short GetMIPError( uint32_t* error );
        unsigned short GetEsnAsDecimal( char* decESN, unsigned short* decEsn_len );

        unsigned short ReadPBEntry( uint32_t index, unsigned char *pBuff, uint32_t *dwBuffLen, uint32_t *dwErr );
        unsigned short WritePBEntry( uint32_t index, unsigned char *pBuff, uint32_t dwBuffLen, uint32_t *dwErr );
        unsigned short ClearPBEntry( uint32_t index, uint32_t *dwErr );
        unsigned short ClearAllPBEntries( uint32_t *dwErr );

        
        unsigned short DeviceStatusNotify4G( unsigned char status );
        unsigned short LedConfiguration4G( PropertyAction bSet, LedConfig4G *pConfig );
        unsigned short DualWorkModeSetting4G( PropertyAction bSet, unsigned char* pSetting );
        unsigned short GetInterfaceMode( unsigned char* mode );
        unsigned short Set3GLimitedPower( unsigned char mode );      
        unsigned short GetLTENetworkType( LTENetworkType *eNetworkType );
        unsigned short GetMSISDN( char *buffer, unsigned short *len );
		unsigned short GetMultiModeIpAddress (MultiModeIPAddress *buffer);
        unsigned short LteAPN( PropertyAction bSet, unsigned char *apn_class, char *apn_string, unsigned short *apn_strlen );
        unsigned short LanAutoConnectionMode( PropertyAction bSet, unsigned char *pLanConnectionMode );
        unsigned short CmnMicroSDControl( PropertyAction bPropAction, unsigned char* enabled );
        unsigned short GetCurrentChannelData( ChannelBandStruct *channelData );
		unsigned short GetSINR(unsigned short *wCurrMode, int32_t *iSINR);
		unsigned short GetIMSRegStatus(unsigned char *ucIMSRegStatus);
		unsigned short GetCallHistoryIndexList(uint32_t* pIndexList, uint32_t* pIndexcount);
		unsigned short GetCallHistoryForItem(uint32_t index ,CallHistoryItemType *pCallHistoryItem);
		unsigned short ClearCallHistory(uint32_t *pdwError);
		unsigned short GetSARRFState(unsigned long *pdwError, char *key);
		unsigned short SetSARRFState(unsigned long *pdwError, char *key);

        unsigned short FatalErrorOption(PropertyAction bSet, unsigned short *pOption);
		unsigned short GetCurrentSID(int32_t* iSID);
		unsigned short ConfigureLoggingMaxFileSize(uint64_t maxFileSize);
		unsigned short MultiModeRssiRange(PropertyAction bSet, unsigned char *u8var);

		unsigned short LanConnectionStatus( LanConnectionStatusStruct * pStatus );
	
        SdkHandle	m_session;
	private:
		OBJECT_TYPE	m_hDll;
		

		void		ClearFunctionPointers();

        Func_NvtlCommon_GetSdkVersion               fpNvtlCommon_GetSdkVersion;
		Func_NvtlCommon_CreateSession				fpNvtlCommon_CreateSession;
        Func_NvtlCommon_SetSdkMode                  fpNvtlCommon_SetSdkMode;
		Func_NvtlCommon_ReleaseSession				fpNvtlCommon_ReleaseSession;
		Func_NvtlCommon_AttachDevice				fpNvtlCommon_AttachDevice;
		Func_NvtlCommon_DetachDevice				fpNvtlCommon_DetachDevice;
		Func_NvtlCommon_GetDeviceInfo				fpNvtlCommon_GetDeviceInfo;
		Func_NvtlCommon_GetNetworkInfo				fpNvtlCommon_GetNetworkInfo;
		Func_NvtlCommon_ReadContact					fpNvtlCommon_ReadContact;
        Func_NvtlCommon_ReadMultipleContacts        fpNvtlCommon_ReadMultipleContacts;
		Func_NvtlCommon_WriteContact				fpNvtlCommon_WriteContact;
		Func_NvtlCommon_GetContactSizeDetails		fpNvtlCommon_GetContactSizeDetails;
		Func_NvtlCommon_GetPowerMode				fpNvtlCommon_GetPowerMode;
		Func_NvtlCommon_SetPowerMode				fpNvtlCommon_SetPowerMode;
        Func_NvtlCommon_RadioState                  fpNvtlCommon_RadioState;
		Func_NvtlCommon_Reset						fpNvtlCommon_Reset;
		Func_NvtlCommon_RegisterEventCallback		fpNvtlCommon_RegisterEventCallback;
		Func_NvtlCommon_UnregisterEventCallback		fpNvtlCommon_UnregisterEventCallback;
		Func_NvtlCommon_GetAvailableDevices			fpNvtlCommon_GetAvailableDevices;
		Func_NvtlCommon_SetDeviceAccessKey			fpNvtlCommon_SetDeviceAccessKey;
		Func_NvtlCommon_RefreshDeviceInfo			fpNvtlCommon_RefreshDeviceInfo;
		Func_NvtlCommon_ConfigureLogging			fpNvtlCommon_ConfigureLogging;
        Func_NvtlCommon_UnlockDevice                fpNvtlCommon_UnlockDevice;
        Func_NvtlCommon_SetAutoLock                 fpNvtlCommon_SetAutoLock;
        Func_NvtlCommon_GetLockStatus               fpNvtlCommon_GetLockStatus;
        Func_NvtlCommon_GetLockStatusEx             fpNvtlCommon_GetLockStatusEx;
        Func_NvtlCommon_ChangeLockCode              fpNvtlCommon_ChangeLockCode;
        Func_NvtlCommon_NetworkPreference           fpNvtlCommon_NetworkPreference;
		Func_NvtlCommon_GetRSSI                     fpNvtlCommon_GetRSSI;
		Func_NvtlCommon_NovaSpeedEnabled            fpNvtlCommon_NovaSpeedEnabled;
        Func_NvtlCommon_AutoInstallEnabled          fpNvtlCommon_AutoInstallEnabled;
        Func_NvtlCommon_InternalMassStorageEnabled  fpNvtlCommon_InternalMassStorageEnabled;
        Func_NvtlCommon_GetTemperature              fpNvtlCommon_GetTemperature;
        Func_NvtlCommon_GetLifetimeTimer            fpNvtlCommon_GetLifetimeTimer;
		Func_NvtlCommon_GetDisplaySpeed				fpNvtlCommon_GetDisplaySpeed;
		Func_NvtlCommon_GetDataCounters             fpNvtlCommon_GetDataCounters;
        Func_NvtlCommon_RestoreToFactoryDefaults    fpNvtlCommon_RestoreToFactoryDefaults;
        Func_NvtlCommon_RestoreToFactoryDefaultsBasic    fpNvtlCommon_RestoreToFactoryDefaultsBasic;
		Func_NvtlCommon_IsLPMSupported				fpNvtlCommon_IsLPMSupported;
        Func_NvtlCommon_GetRefurbInfo               fpNvtlCommon_GetRefurbInfo;
		Func_NvtlCommon_SetSystemPreferredHybridMode	fpNvtlCommon_SetSystemPreferredHybridMode;
		Func_NvtlCommon_IsSMSSupported              fpNvtlCommon_IsSMSSupported;
		Func_NvtlCommon_ReadSmsMsg                  fpNvtlCommon_ReadSmsMsg;
        Func_NvtlCommon_ReadSmsMsgEx                  fpNvtlCommon_ReadSmsMsgEx;
        Func_NvtlCommon_SendSmsMsg                  fpNvtlCommon_SendSmsMsg;
        Func_NvtlCommon_DeleteSmsMsg                fpNvtlCommon_DeleteSmsMsg;
        Func_NvtlCommon_DeleteSmsMsgEx              fpNvtlCommon_DeleteSmsMsgEx;
        Func_NvtlCommon_GetSmsList                  fpNvtlCommon_GetSmsList;
        Func_NvtlCommon_GetSmsListEx                fpNvtlCommon_GetSmsListEx;
        Func_NvtlCommon_GetSmsRxTimestamp           fpNvtlCommon_GetSmsRxTimestamp;
        Func_NvtlCommon_ClearSmsRxTimestamps        fpNvtlCommon_ClearSmsRxTimestamps;
        Func_NvtlCommon_GetSmsFormat                fpNvtlCommon_GetSmsFormat;

        Func_NvtlCommon_GetBatteryState             fpNvtlCommon_GetBatteryState;
		Func_NvtlCommon_GetManufacturer             fpNvtlCommon_GetManufacturer;
		Func_NvtlCommon_GetMACAddress			    fpNvtlCommon_GetMACAddress;

        Func_NvtlEvdo_BandPreference                fpNvtlEvdo_BandPreference;
        Func_NvtlEvdo_GetDateTime                   fpNvtlEvdo_GetDateTime;
		Func_NvtlEvdo_GetEriInfo					fpNvtlEvdo_GetEriInfo;
        Func_NvtlEvdo_RoamPreference                fpNvtlEvdo_RoamPreference;
        Func_NvtlEvdo_IntlRoamPreference            fpNvtlEvdo_IntlRoamPreference;
        Func_NvtlEvdo_MicroSDControl                fpNvtlEvdo_MicroSDControl;
        Func_NvtlEvdo_LedControl                    fpNvtlEvdo_LedControl;
        Func_NvtlEvdo_GetMobileId                   fpNvtlEvdo_GetMobileId;
        Func_NvtlEvdo_GetMeid                       fpNvtlEvdo_GetMeid;
        //Func_NvtlEvdo_GetMIPError                   fpNvtlEvdo_GetMIPError;
        Func_NvtlEvdo_GetEsnAsDecimal               fpNvtlEvdo_GetEsnAsDecimal;
        Func_NvtlEvdo_ReadPBEntry                   fpNvtlEvdo_ReadPBEntry;
        Func_NvtlEvdo_WritePBEntry                  fpNvtlEvdo_WritePBEntry;
        Func_NvtlEvdo_ClearPBEntry                  fpNvtlEvdo_ClearPBEntry;
        Func_NvtlEvdo_ClearAllPBEntries             fpNvtlEvdo_ClearAllPBEntries;
        Func_NvtlEvdo_MultiModeRssiRange            fpNvtlEvdo_MultiModeRssiRange;

        Func_NvtlGsm_SMSC                           fpNvtlGsm_SMSC;
        Func_NvtlGsm_GetImsi                        fpNvtlGsm_GetImsi;
        Func_NvtlGsm_GetIccid                       fpNvtlGsm_GetIccid;
        Func_NvtlGsm_GetNetworkMCCMNC               fpNvtlGsm_GetNetworkMCCMNC;
        Func_NvtlGsm_GetNetworkOperator             fpNvtlGsm_GetNetworkOperator;
        Func_NvtlGsm_SetNetworkOperator             fpNvtlGsm_SetNetworkOperator;
        Func_NvtlGsm_GetNetworkOperatorList         fpNvtlGsm_GetNetworkOperatorList;
        Func_NvtlGsm_SendATCommand                  fpNvtlGsm_SendATCommand;
        Func_NvtlGsm_Band                           fpNvtlGsm_Band;
        Func_NvtlGsm_SendStoredSms                  fpNvtlGsm_SendStoredSms;
        Func_NvtlGsm_SetSmsStorage                  fpNvtlGsm_SetSmsStorage;
        Func_NvtlCommon_SmsStorageEx                fpNvtlCommon_SmsStorageEx;
        Func_NvtlGsm_GetContactStorage              fpNvtlGsm_GetContactStorage;
        Func_NvtlGsm_SetContactStorage              fpNvtlGsm_SetContactStorage;
        Func_NvtlGsm_WriteSmsMsg                    fpNvtlGsm_WriteSmsMsg;
        Func_NvtlGsm_IsMsgMemoryFull                fpNvtlGsm_IsMsgMemoryFull;
        Func_NvtlGsm_IsMsgMemoryFullEx              fpNvtlGsm_IsMsgMemoryFullEx;
        Func_NvtlGsm_GetEnsInfo                     fpNvtlGsm_GetEnsInfo;
        Func_NvtlGsm_IsManualNetworkSelected        fpNvtlGsm_IsManualNetworkSelected;
        Func_NvtlGsm_DomainAttach                   fpNvtlGsm_DomainAttach;
		Func_NvtlGsm_GetNetworkCapabilities			fpNvtlGsm_GetNetworkCapabilities;

        Func_NvtlCommon_DeviceStatusNotify4G        fpNvtlCommon_DeviceStatusNotify4G;
        Func_NvtlCommon_LedConfiguration4G          fpNvtlCommon_LedConfiguration4G;
        Func_NvtlCommon_DualWorkModeSetting4G       fpNvtlCommon_DualWorkModeSetting4G;
        Func_NvtlCommon_GetInterfaceMode            fpNvtlCommon_GetInterfaceMode;
        Func_NvtlCommon_Set3GLimitedPower           fpNvtlCommon_Set3GLimitedPower;
        Func_NvtlCommon_GetLTENetworkType           fpNvtlCommon_GetLTENetworkType;
        Func_NvtlCommon_GetMSISDN                   fpNvtlCommon_GetMSISDN;
		Func_NvtlCommon_GetMultiModeIpAddress		fpNvtlCommon_GetMultiModeIpAddress;
        Func_NvtlCommon_LteAPN                      fpNvtlCommon_LteAPN;
        Func_NvtlCommon_LanAutoConnectionMode       fpNvtlCommon_LanAutoConnectionMode;
		Func_NvtlCommon_MicroSDControl              fpNvtlCommon_MicroSDControl;
		Func_NvtlCommon_GetCurrentChannelData       fpNvtlCommon_GetCurrentChannelData;
		Func_NvtlCommon_GetSINR						fpNvtlCommon_GetSINR;
		Func_NvtlCommon_GetIMSRegStatus				fpNvtlCommon_GetIMSRegStatus;
		Func_NvtlCommon_GetCallHistoryIndexList		fpNvtlCommon_GetCallHistoryIndexList;
		Func_NvtlCommon_GetCallHistoryForItem		fpNvtlCommon_GetCallHistoryForItem;
		Func_NvtlCommon_ClearCallHistory			fpNvtlCommon_ClearCallHistory;
		Func_NvtlCommon_LanConnectionStatus			fpNvtlCommon_LanConnectionStatus;
        Func_NvtlCommon_FatalErrorOption            fpNvtlCommon_FatalErrorOption;
        Func_NvtlCommon_ConfigureLoggingMaxFileSize fpNvtlCommon_ConfigureLoggingMaxFileSize;
		Func_NvtlEvdo_GetCurrentSID                 fpNvtlEvdo_GetCurrentSID;      
		Func_NvtlCommon_GetSARRFState				fpNvtlCommon_GetSARRFState;
		Func_NvtlCommon_SetSARRFState				fpNvtlCommon_SetSARRFState;
};

#endif
