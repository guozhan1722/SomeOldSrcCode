/**
*
* Copyright 2008 Novatel Wireless Inc.
*
*/

#ifndef _Gps_WRAPPER_H_
#define _Gps_WRAPPER_H_

#include "SdkWrapper.h"

typedef void* ( NOVATEL_CALL *Func_Gps_CreateSession)( SdkHandle hClient );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_ReleaseSession)( SdkHandle gpsSession );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_RegisterEventCallback)( SdkHandle gpsSession, NvtlEventCallback callback );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_IsGpsSupported)( SdkHandle gpsSession, unsigned char* bSupported );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_GetSupportedModes)( SdkHandle gpsSession, GpsModeType* modes );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_Start)( SdkHandle gpsSession, GpsModeType mode, uint32_t fixCount );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_Stop)( SdkHandle gpsSession );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_GetLastFix)( SdkHandle gpsSession, GpsFixInfoStruct* lastFix );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_GetLastFixEx)( SdkHandle gpsSession, GpsFixInfoStructEx* lastFix );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_GetSatellitesInfo)( SdkHandle gpsSession, GpsSatelliteConstellationStruct* satellites);
typedef unsigned short ( NOVATEL_CALL *Func_Gps_SetPdeAddress)( SdkHandle gpsSession, char* pdeAddress, unsigned short port );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_ClearSatelliteData)( SdkHandle gpsSession, unsigned char bDeleteAlmanac, unsigned char bDeleteEphemeris );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_GetNmeaPort)( SdkHandle gpsSession, char* szPortName, unsigned short buffer_length );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_NmeaOutputFormat)( SdkHandle gpsSession, PropertyAction bPropAction, unsigned char* format );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_XtraDownloadData)( SdkHandle gpsSession, char* server );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_XtraTimeSync)( SdkHandle gpsSession, char* server );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_XtraDownloadDataCDMA)( SdkHandle gpsSession, GPSEventXTRADownloadStruct* servers );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_XtraTimeSyncCDMA)( SdkHandle gpsSession, GPSEventXTRATimeinfoStruct* servers );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_SmartMode)( SdkHandle gpsSession, PropertyAction bPropAction, unsigned char* smartMode );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_AutoStart)( SdkHandle gpsSession, PropertyAction bPropAction, unsigned char* autoStart );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_GpsXtraEnabledCDMA)( SdkHandle gpsSession, PropertyAction bPropAction, unsigned char* xtraEnabled );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_ConfigureLogging)( const char* filename, SdkLogLevelType logLevel, SdkLogOutputType output );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_EnableNmeaPort)( SdkHandle gpsSession, unsigned char enable );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_QuerySession)( SdkHandle gpsSession, PDSMSessionInfo* info );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_GpsEngineLock)( SdkHandle gpsSession, PropertyAction bPropAction, unsigned char* bLocked );
typedef unsigned short ( NOVATEL_CALL *Func_Gps_SetXtraGPSConfig)(SdkHandle gpsSession,  unsigned char val);


class GpsWrapper
{
public:
    GpsWrapper();
    ~GpsWrapper();
    bool IsLoaded();
    void LoadObject();
    void UnloadObject();

    unsigned short CreateSession( SdkHandle hClient );
	unsigned short ReleaseSession( );
	unsigned short RegisterEventCallback( NvtlEventCallback callback );
	unsigned short IsGpsSupported( unsigned char* bSupported );
	unsigned short GetSupportedModes( GpsModeType* modes );
	unsigned short Start( GpsModeType mode, uint32_t fixCount );
	unsigned short Stop( );
	unsigned short GetLastFix( GpsFixInfoStruct* lastFix );
    unsigned short GetLastFixEx( GpsFixInfoStructEx* lastFix );
	unsigned short GetSatellitesInfo( GpsSatelliteConstellationStruct* satellites);
	unsigned short SetPdeAddress( char* pdeAddress, unsigned short port );
	unsigned short ClearSatelliteData( unsigned char bDeleteAlmanac, unsigned char bDeleteEphemeris );
	unsigned short GetNmeaPort( char* szPortName, unsigned short buffer_length );
	unsigned short NmeaOutputFormat( PropertyAction bPropAction, unsigned char* format );
	unsigned short XtraDownloadData( char* server );
	unsigned short XtraTimeSync( char* server );
	unsigned short XtraDownloadDataCDMA( GPSEventXTRADownloadStruct* servers );
	unsigned short XtraTimeSyncCDMA( GPSEventXTRATimeinfoStruct* servers );
	unsigned short SmartMode( PropertyAction bPropAction, unsigned char* smartMode );
	unsigned short AutoStart( PropertyAction bPropAction, unsigned char* autoStart );
	unsigned short GpsXtraEnabledCDMA( PropertyAction bPropAction, unsigned char* xtraEnabled );
	unsigned short ConfigureLogging( const char* filename, SdkLogLevelType logLevel, SdkLogOutputType output );
    unsigned short EnableNmeaPort( unsigned char enable );
    unsigned short QuerySession( PDSMSessionInfo *info );
    unsigned short GpsEngineLock( PropertyAction bPropAction, unsigned char* bLocked );
	unsigned short SetXtraGPSConfig( unsigned char val );

private:
    OBJECT_TYPE m_hDll;
    SdkHandle   m_session;
    void        ClearFunctionPointers();

    Func_Gps_CreateSession 			fpCreateSession;
	Func_Gps_ReleaseSession 		fpReleaseSession;
	Func_Gps_RegisterEventCallback 	fpRegisterEventCallback;
	Func_Gps_IsGpsSupported 		fpIsGpsSupported;
	Func_Gps_GetSupportedModes 		fpGetSupportedModes;
	Func_Gps_Start 			        fpStart;
	Func_Gps_Stop 			        fpStop;
	Func_Gps_GetLastFix 			fpGetLastFix;
    Func_Gps_GetLastFixEx 			fpGetLastFixEx;
	Func_Gps_GetSatellitesInfo 		fpGetSatellitesInfo;
	Func_Gps_SetPdeAddress 			fpSetPdeAddress;
	Func_Gps_ClearSatelliteData 	fpClearSatelliteData;
	Func_Gps_GetNmeaPort 			fpGetNmeaPort;
	Func_Gps_NmeaOutputFormat 		fpNmeaOutputFormat;
	Func_Gps_XtraDownloadData 		fpXtraDownloadData;
	Func_Gps_XtraTimeSync 			fpXtraTimeSync;
	Func_Gps_XtraDownloadDataCDMA 	fpXtraDownloadDataCDMA;
	Func_Gps_XtraTimeSyncCDMA 		fpXtraTimeSyncCDMA;
	Func_Gps_SmartMode 			    fpSmartMode;
	Func_Gps_AutoStart 			    fpAutoStart;
	Func_Gps_GpsXtraEnabledCDMA	    fpGpsXtraEnabledCDMA;
	Func_Gps_ConfigureLogging 		fpConfigureLogging;
    Func_Gps_EnableNmeaPort         fpEnableNmeaPort;
    Func_Gps_QuerySession           fpQuerySession;
    Func_Gps_GpsEngineLock          fpGpsEngineLock;
	Func_Gps_SetXtraGPSConfig       fpSetXtraGPSConfig;
};

#endif // _Gps_WRAPPER_H_
