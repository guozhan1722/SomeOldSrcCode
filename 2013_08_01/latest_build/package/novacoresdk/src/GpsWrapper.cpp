/**
*
* Copyright 2008 Novatel Wireless Inc.
*
*/

#include "GpsWrapper.h"

GpsWrapper::GpsWrapper()
{
   m_session = 0;
   m_hDll = 0;
   ClearFunctionPointers();
   LoadObject();
}
GpsWrapper::~GpsWrapper()
{
    if( m_hDll != NULL )
	{
		ReleaseSession();
		UnloadObject();
	}
}
bool GpsWrapper::IsLoaded()
{
	return (m_hDll != NULL);
}
void GpsWrapper::LoadObject()
{
    ClearFunctionPointers();
    if( !m_hDll )
        m_hDll = LOAD_OBJECT( Gps_OBJECT_NAME );
}
void GpsWrapper::UnloadObject()
{
    if( m_hDll )
        UNLOAD_OBJECT( m_hDll );
    m_hDll = NULL;
    ClearFunctionPointers();
}
void GpsWrapper::ClearFunctionPointers()
{
    fpCreateSession = 0;
	fpReleaseSession = 0;
	fpRegisterEventCallback = 0;
	fpIsGpsSupported = 0;
	fpGetSupportedModes = 0;
	fpStart = 0;
	fpStop = 0;
	fpGetLastFix = 0;
    fpGetLastFixEx = 0;
	fpGetSatellitesInfo = 0;
	fpSetPdeAddress = 0;
	fpClearSatelliteData = 0;
	fpGetNmeaPort = 0;
	fpNmeaOutputFormat = 0;
	fpXtraDownloadData = 0;
	fpXtraTimeSync = 0;
	fpXtraDownloadDataCDMA = 0;
	fpXtraTimeSyncCDMA = 0;
	fpSmartMode = 0;
	fpAutoStart = 0;
	fpGpsXtraEnabledCDMA = 0;
	fpConfigureLogging = 0;
    fpEnableNmeaPort = 0;
    fpQuerySession = 0;
    fpGpsEngineLock = 0;
	fpSetXtraGPSConfig =0;
}
unsigned short GpsWrapper::CreateSession( SdkHandle hClient )
{
    LoadObject();

    LOAD_FUNCTION_PTR( fpCreateSession, Func_Gps_CreateSession, "CreateSession", m_hDll );

    m_session = fpCreateSession(hClient);

    return ( m_session == NULL );
}

unsigned short GpsWrapper::ReleaseSession( )
{
    unsigned short rval = 0;

    LOAD_FUNCTION_PTR( fpReleaseSession, Func_Gps_ReleaseSession, "ReleaseSession", m_hDll );

    if( m_session != NULL )
    {
        rval = fpReleaseSession(m_session);
        m_session = 0;
    }
    return rval;
}

unsigned short GpsWrapper::RegisterEventCallback( NvtlEventCallback callback )
{
    LOAD_FUNCTION_PTR( fpRegisterEventCallback, Func_Gps_RegisterEventCallback, "RegisterEventCallback", m_hDll );

    return fpRegisterEventCallback(m_session,callback);
}

unsigned short GpsWrapper::IsGpsSupported( unsigned char* bSupported )
{
    LOAD_FUNCTION_PTR( fpIsGpsSupported, Func_Gps_IsGpsSupported, "IsGpsSupported", m_hDll );

    return fpIsGpsSupported(m_session,bSupported);
}

unsigned short GpsWrapper::GetSupportedModes( GpsModeType* modes )
{
    LOAD_FUNCTION_PTR( fpGetSupportedModes, Func_Gps_GetSupportedModes, "GetSupportedModes", m_hDll );

    return fpGetSupportedModes(m_session,modes);
}

unsigned short GpsWrapper::Start( GpsModeType mode, uint32_t fixCount )
{
    LOAD_FUNCTION_PTR( fpStart, Func_Gps_Start, "Start", m_hDll );

    return fpStart(m_session,mode,fixCount);
}

unsigned short GpsWrapper::Stop( )
{
    LOAD_FUNCTION_PTR( fpStop, Func_Gps_Stop, "Stop", m_hDll );

    return fpStop(m_session);
}

unsigned short GpsWrapper::GetLastFix( GpsFixInfoStruct* lastFix )
{
    LOAD_FUNCTION_PTR( fpGetLastFix, Func_Gps_GetLastFix, "GetLastFix", m_hDll );

    return fpGetLastFix(m_session,lastFix);
}

unsigned short GpsWrapper::GetLastFixEx( GpsFixInfoStructEx* lastFix )
{
    LOAD_FUNCTION_PTR( fpGetLastFixEx, Func_Gps_GetLastFixEx, "GetLastFixEx", m_hDll );

    return fpGetLastFixEx(m_session,lastFix);
}

unsigned short GpsWrapper::GetSatellitesInfo( GpsSatelliteConstellationStruct* satellites)
{
    LOAD_FUNCTION_PTR( fpGetSatellitesInfo, Func_Gps_GetSatellitesInfo, "GetSatellitesInfo", m_hDll );

    return fpGetSatellitesInfo(m_session,satellites);
}

unsigned short GpsWrapper::SetPdeAddress( char* pdeAddress, unsigned short port )
{
    LOAD_FUNCTION_PTR( fpSetPdeAddress, Func_Gps_SetPdeAddress, "SetPdeAddress", m_hDll );

    return fpSetPdeAddress(m_session,pdeAddress,port);
}

unsigned short GpsWrapper::ClearSatelliteData( unsigned char bDeleteAlmanac, unsigned char bDeleteEphemeris )
{
    LOAD_FUNCTION_PTR( fpClearSatelliteData, Func_Gps_ClearSatelliteData, "ClearSatelliteData", m_hDll );

    return fpClearSatelliteData(m_session,bDeleteAlmanac,bDeleteEphemeris);
}

unsigned short GpsWrapper::GetNmeaPort( char* szPortName, unsigned short buffer_length  )
{
    LOAD_FUNCTION_PTR( fpGetNmeaPort, Func_Gps_GetNmeaPort, "GetNmeaPort", m_hDll );

    return fpGetNmeaPort(m_session, szPortName, buffer_length );
}

unsigned short GpsWrapper::NmeaOutputFormat( PropertyAction bPropAction, unsigned char* format )
{
    LOAD_FUNCTION_PTR( fpNmeaOutputFormat, Func_Gps_NmeaOutputFormat, "NmeaOutputFormat", m_hDll );

    return fpNmeaOutputFormat(m_session,bPropAction,format);
}

unsigned short GpsWrapper::EnableNmeaPort( unsigned char enabled)
{
    LOAD_FUNCTION_PTR( fpEnableNmeaPort, Func_Gps_EnableNmeaPort, "EnableNmeaPort", m_hDll );

    return fpEnableNmeaPort(m_session,enabled);
}

unsigned short GpsWrapper::XtraDownloadData( char* server )
{
    LOAD_FUNCTION_PTR( fpXtraDownloadData, Func_Gps_XtraDownloadData, "XtraDownloadData", m_hDll );

    return fpXtraDownloadData(m_session,server);
}

unsigned short GpsWrapper::XtraTimeSync( char* server )
{
    LOAD_FUNCTION_PTR( fpXtraTimeSync, Func_Gps_XtraTimeSync, "XtraTimeSync", m_hDll );

    return fpXtraTimeSync(m_session,server);
}

unsigned short GpsWrapper::XtraDownloadDataCDMA( GPSEventXTRADownloadStruct* servers )
{
    LOAD_FUNCTION_PTR( fpXtraDownloadDataCDMA, Func_Gps_XtraDownloadDataCDMA, "XtraDownloadDataCDMA", m_hDll );

    return fpXtraDownloadDataCDMA(m_session,servers);
}

unsigned short GpsWrapper::XtraTimeSyncCDMA( GPSEventXTRATimeinfoStruct* servers )
{
    LOAD_FUNCTION_PTR( fpXtraTimeSyncCDMA, Func_Gps_XtraTimeSyncCDMA, "XtraTimeSyncCDMA", m_hDll );

    return fpXtraTimeSyncCDMA(m_session,servers);
}
unsigned short GpsWrapper::SmartMode( PropertyAction bPropAction, unsigned char* smartMode )
{
    LOAD_FUNCTION_PTR( fpSmartMode, Func_Gps_SmartMode, "SmartMode", m_hDll );

    return fpSmartMode(m_session,bPropAction,smartMode);
}

unsigned short GpsWrapper::AutoStart( PropertyAction bPropAction, unsigned char* autoStart )
{
    LOAD_FUNCTION_PTR( fpAutoStart, Func_Gps_AutoStart, "AutoStart", m_hDll );

    return fpAutoStart(m_session,bPropAction,autoStart);
}
unsigned short GpsWrapper::GpsXtraEnabledCDMA(PropertyAction bPropAction, unsigned char* xtraEnabled )
{
    LOAD_FUNCTION_PTR( fpGpsXtraEnabledCDMA, Func_Gps_GpsXtraEnabledCDMA, "GpsXtraEnabledCDMA", m_hDll );

    return fpGpsXtraEnabledCDMA(m_session,bPropAction,xtraEnabled);
}
unsigned short GpsWrapper::QuerySession( PDSMSessionInfo* info)
{
    LOAD_FUNCTION_PTR( fpQuerySession, Func_Gps_QuerySession, "QuerySession", m_hDll );

    return fpQuerySession(m_session, info);
}
unsigned short GpsWrapper::GpsEngineLock( PropertyAction bPropAction, unsigned char* bLocked )
{
    LOAD_FUNCTION_PTR( fpGpsEngineLock, Func_Gps_GpsEngineLock, "GpsEngineLock", m_hDll );

    return fpGpsEngineLock(m_session, bPropAction, bLocked );
}

unsigned short GpsWrapper::ConfigureLogging( const char* filename, SdkLogLevelType logLevel, SdkLogOutputType output )
{
    LOAD_FUNCTION_PTR( fpConfigureLogging, Func_Gps_ConfigureLogging, "ConfigureLogging", m_hDll );

    return fpConfigureLogging(filename,logLevel,output);
}

unsigned short GpsWrapper::SetXtraGPSConfig( unsigned char val)
{
    LOAD_FUNCTION_PTR( fpSetXtraGPSConfig, Func_Gps_SetXtraGPSConfig, "SetXtraGPSConfig", m_hDll );

    return fpSetXtraGPSConfig(m_session,val);
}

