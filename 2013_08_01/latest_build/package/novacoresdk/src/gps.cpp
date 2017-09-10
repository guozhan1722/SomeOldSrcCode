/**
*
* Copyright 2008 Novatel Wireless Inc.
*
*/


#include "NvtlApiDefinitions.h"
#include "SdkWrapper.h"
#include "GpsWrapper.h"

#if DEFINED_MAC
#include <unistd.h>
#endif

#if DEFINED_LINUX
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#endif

/*
Set DEVICE_PORT and DEVICE_TYPE as appropriate for your device
*/
//static char*    DEFAULT_DEVICE_PORT  =  "/dev/ttyUSB1";
char    DEFAULT_DEVICE_PORT[50];
static DeviceTechType DEFAULT_DEVICE_TYPE = DEV_EVDO_REVA;
static char*    DEVICE_PORT          =  "";
static DeviceTechType DEVICE_TYPE    = DEV_NONE;

unsigned long   gDeviceState        = NW_DEVICE_STATE_UNKNOWN;
int             gDeviceAvailable    = 0;
int             gGpsFixReceived     = 0;
int				gXTRAFileDownloaded = 0;
int				gXTRAMsgReceived	= 0;
int				gGpsStopped			= 0;
int             gGpsStarted         = 0;
int				gTimeSyncMsgReceived = 0;
int				gTimeSyncCompleted	= 0;

static bool AttachToDevice( SdkWrapper& sdk );
static bool WaitForDeviceReady( SdkWrapper& sdk );

void my_sleep( int num_seconds )
{
//#if DEFINED_UNIX_BASED
		sleep(num_seconds);
//#else
//        Sleep( num_seconds*1000 );
//#endif
}

/**
Callback function to receive SDK events
*/
void EventHandler(  void* user_data, uint32_t type, uint32_t size, void* ev  )
{
	switch( type )
	{
	case NW_EVENT_SIG_STR:
		printf("Signal Strength Received = %ld\n", ((SigStrEvent*)ev)->val );
		break;

	case NW_EVENT_ROAMING:
		printf("Roaming Status Received = %ld\n",  ((RoamingEvent*)ev)->val );
		break;

    case NW_EVENT_SERVER_ERROR:
		printf("EVENT SERVER ERROR = %ld\n", ((ServerErrorEvent*)ev)->val );
		break;

	case NW_EVENT_DEVICE_STATE:
		{
			printf("Device State Recevied = %ld\n", ((DeviceStateEvent*)ev)->val );
            gDeviceState = ((DeviceStateEvent*)ev)->val;
		}
		break;

	case NW_EVENT_NETWORK:
		printf("EVENT NETWORK = %ld\n", ((NetworkEvent*)ev)->val );
		break;

	case NW_EVENT_DEVICE_ADDED:
        {
            printf("A new device was detected\n");
            gDeviceAvailable = 1;
        }
        break;

	case NW_EVENT_DEVICE_REMOVED:
		printf("A device was removed\n");
		break;

    case NW_EVENT_GPS:
        {
			unsigned long  eventType = ((GpsEvent*)ev)->val;
			if(eventType == NW_GPS_STARTED)
			{
                gGpsStarted = 1;
            	printf("GPS Started\n");
            }
			else if(eventType == NW_GPS_STOPPED)
			{
                if( gGpsStarted )
                {
				    gGpsStopped = 1;
                    gGpsStarted = 0;
                }
				printf("GPS Stopped\n");
			}
			else if(eventType == NW_GPS_FIX_RECEIVED)
			{
				printf("GPS Fix Received\n");
				gGpsFixReceived = 1;
			}
			else if(eventType == NW_GPS_MSBASED_FAILURE)
				printf("GPS MS-Based Failed. May fall back to Standalone\n");
			else if(eventType == NW_GPS_SUBSCRIPTION_FAILED)
				printf("30 day subscription check failed. Need to subscribe for the service. If already subscribed, connect to the network.\n");

        }
        break;
	}
    fflush(stdout);
}


int main(int argc, char* argv[])
{

    SdkWrapper	        *sdk;
    GpsWrapper          *gpsSdk;
    NvtlEventCallback	cb;
    DeviceInfoStruct	device_info;
    int                 retries = 0;
    unsigned short      rval = 0;
    unsigned char       bSupported = 0;
    unsigned char       NmeaFormat = 0;
    char                szNmeaPortName[512] = {0};
    GpsFixInfoStruct    gpsFix;
    GpsModeType			modes = NW_GPS_NOGPS;
    GpsModeType         gpsMode = NW_GPS_NOGPS;
    time_t record_time;

    //if(argc!=2)exit(0);

    strcpy(DEFAULT_DEVICE_PORT,"/dev/ttyUSB2");
    int i_port=2;

restart_gps:
    if(retries>0)sleep(5);
    if(retries>20)_exit(-1);
    retries++;

    if(i_port==2)
    {
        strcpy(DEFAULT_DEVICE_PORT,"/dev/ttyUSB1");
        i_port=1;
    }else
    {
        strcpy(DEFAULT_DEVICE_PORT,"/dev/ttyUSB2");
        i_port=2;
    }
    gGpsStopped=0;
    sdk=new SdkWrapper;
    gpsSdk=new GpsWrapper;
    if( !sdk->IsLoaded() || !gpsSdk->IsLoaded() )
    {
        delete sdk;
        delete gpsSdk;
        goto restart_gps;
    }

    sdk->SetSdkMode( SdkModeLocal, "" );
    rval = sdk->CreateSession();
    if( !AttachToDevice( *sdk ) )
    {
        sdk->ReleaseSession();
        delete sdk;
        delete gpsSdk;
        goto restart_gps;
    }
    rval = gpsSdk->CreateSession( sdk->m_session );
    bSupported = 0;
    rval = gpsSdk->IsGpsSupported( &bSupported );
    if( rval )
    {
        gpsSdk->ReleaseSession();
        sdk->DetachDevice();
        sdk->ReleaseSession();
        delete sdk;
        delete gpsSdk;
        goto restart_gps;
    }
    else
    {
        if( !bSupported )
        {
            gpsSdk->ReleaseSession();
            sdk->DetachDevice();
            sdk->ReleaseSession();
            delete sdk;
            delete gpsSdk;
            goto restart_gps;
        }
        else
        {
            rval = gpsSdk->GetSupportedModes(&modes);
            rval = gpsSdk->GetNmeaPort( szNmeaPortName, 512 );
            rval = gpsSdk->NmeaOutputFormat( PropGet, &NmeaFormat );
            gpsMode = NW_GPS_STANDALONE;
            rval = gpsSdk->Start( gpsMode, 0xFFFFFFFF );
            if( rval )
            {
                gpsSdk->ReleaseSession();
                sdk->DetachDevice();
                sdk->ReleaseSession();
                delete sdk;
                delete gpsSdk;
                goto restart_gps;
            }
            sleep(10);
            _exit(-1);
            return -1;
        }
    }
    _exit(-1);
return -1;









    //release the gps sdk session
    gpsSdk->ReleaseSession();

    //Stop using this device
    sdk->DetachDevice();

    //Notify the SDK that is is okay to clean up now
    sdk->ReleaseSession();

	return 0;
}

bool AttachToDevice( SdkWrapper& sdk )
{
    bool            bRet = false;
    unsigned short  rval = 0;
    DeviceDetail    dev_detail;

    // Check to see if port information has been supplied:
    if( !strlen(DEVICE_PORT) || DEV_NONE == DEVICE_TYPE  )
    {
        //printf( "Configuration not set in sample code. Defaulting to DEV_EVDO_REVA on /dev/ttyUSB?\n");
    	// Default to an EVDO RevA device using diag port on /dev/ttyUSB1
    	memset( &dev_detail, 0, sizeof(DeviceDetail) );
        dev_detail.eTechnology = DEFAULT_DEVICE_TYPE;
        strncpy( dev_detail.szPort, DEFAULT_DEVICE_PORT, sizeof(dev_detail.szPort) );

    	printf( "Attempting to open default port: %s\n", dev_detail.szPort );
    }
    else
    {
        memset( &dev_detail, 0, sizeof(DeviceDetail) );
        dev_detail.eTechnology = DEFAULT_DEVICE_TYPE;
        strncpy( dev_detail.szPort, DEFAULT_DEVICE_PORT, sizeof(dev_detail.szPort) );
    	
        // Use the data supplied in gDeviceDetail
        //printf( "Attempting to open port: %s \n", dev_detail.szPort );
    }

	// Attempt to attach to the device
	rval = sdk.AttachDevice( &dev_detail );
	if( rval )
	{
		//printf( "Failed to attach to port: %s\n", dev_detail.szPort );
        ;
	}
	else
	{
		//device is available and attached, return success
		//printf( "Attached to port: %s\n", dev_detail.szPort );
		bRet = true;
	}

    return bRet;
}

bool WaitForDeviceReady( SdkWrapper& sdk )
{
    bool bRet = false;
    int retries = 60;
    unsigned short rval = 0;
    DeviceInfoStruct device_info;

    //Check the state of the device to see if it is ready for general use
    memset(&device_info, 0, sizeof(DeviceInfoStruct));
    rval = sdk.GetDeviceInfo( &device_info );
    if( LR_ERROR_SUCCESS == rval )
    {
        //Pause here until the device is ready.  We are waiting for the
        //event callback to notify us of state changes.
        // We are not considering here the cases where the device is Locked, Disabled, or unable to find service.
//        while( gDeviceState < NW_DEVICE_STATE_IDLE && --retries )
        {
            my_sleep(1);
        }

//        if( gDeviceState >= NW_DEVICE_STATE_IDLE )
        {
            printf("Device is now ready\n");
            bRet = true;
        }
//        else
        {
//            printf("Device failed to reach ready state\n");
        }
    }
    return bRet;
}

