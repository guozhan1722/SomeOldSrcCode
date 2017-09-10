/**
*
* Copyright 2008 Novatel Wireless Inc.
*
*/

#if !defined (_NVTL_API_DEFS_H_)
#define _NVTL_API_DEFS_H_

#if !defined(_NVTL_WINDOWS_)
	#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(_MSC_VER)
		#define _NVTL_WINDOWS_
	#endif
#endif

#if !defined(UINT32_MAX) && defined(_NVTL_WINDOWS_)

    #if !defined(uint8_t)
    
        #define uint8_t  unsigned char
        #define char   char

        #define uint16_t unsigned short
        #define int16_t  short

        #define uint32_t unsigned long
        #define int32_t  long
        
        #define uint64_t unsigned long long
        #define int64_t  long long

    #endif
#else
    #include <stdint.h>
#endif

/** \file
SDK header file that contains most commonly used definitions and enumerations.
*/

//////////////////////////////////////////////////////////////////////////////////////////
//Error Code Definitions
//////////////////////////////////////////////////////////////////////////////////////////
/// \def LR_ERROR_SUCCESS
/// Indicates an operation was successful
#define	LR_ERROR_SUCCESS					0x0000

/// \def LR_ERROR_UNKNOWN
/// Indicates an operation failed for an unknown reason
#define	LR_ERROR_UNKNOWN					0x0001

/// \def LR_ERROR_BAD_CMD
/// Indicates an operation failed because device did not recognize it
#define	LR_ERROR_BAD_CMD					0x0002

/// \def LR_ERROR_BAD_PARAM
/// Indicates an operation failed because a parameter was missing or invalid
#define LR_ERROR_BAD_PARAM					0x0003

/// \def LR_ERROR_BAD_LEN
/// Reserved. Indicates an error in communication with device
#define LR_ERROR_BAD_LEN					0x0004

/// \def LR_ERROR_BAD_SEC_MODE
/// Reserved. Indicates an error in communication with device
#define	LR_ERROR_BAD_SEC_MODE				0x0005

/// \def LR_ERROR_PORT_NOT_OPEN
/// Indicates an operation failed because the COM port was not open or valid
#define LR_ERROR_PORT_NOT_OPEN				0x0006

/// \def LR_ERROR_TIMED_OUT
/// Indicates an operation failed due to a timeout
#define	LR_ERROR_TIMED_OUT					0x0007

/// \def LR_ERROR_INVALID_PARAM
/// Indicates an operation failed because a parameter was missing or invalid
#define LR_ERROR_INVALID_PARAM				0x0008

/// \def LR_ERROR_WRITE_FAILED
/// Indicates an operation failed because a write to the COM port failed
#define LR_ERROR_WRITE_FAILED				0x0009

/// \def LR_ERROR_DATA_CORRUPT
/// Indicates that a response from FW could not be decoded
#define LR_ERROR_DATA_CORRUPT				0x000B

/// \def LR_ERROR_MODE_CHANGE_FAILED
/// Indicates an error changing the current mode of the device.
#define LR_ERROR_MODE_CHANGE_FAILED			0x000C

/// \def LR_ERROR_BUFFER_TOO_SMALL
/// Indicates an operation failed because a supplied buffer was too small to receive the response
#define LR_ERROR_BUFFER_TOO_SMALL			0x000D

/// \def LR_ERROR_PORT_OPEN_FAILED
/// Indicates that the COM port to the deivce could not be opened
#define LR_ERROR_PORT_OPEN_FAILED			0x000F

/// \def LR_ERROR_PORT_ALREADY_OPEN
/// Indicates that the desired COM port is already in use
#define LR_ERROR_PORT_ALREADY_OPEN			0x0011

/// \def LR_ERROR_INTERNAL_ERROR
/// Indicates an internal error trying to issue a command to FW
#define LR_ERROR_INTERNAL_ERROR				0x0013

/// \def LR_ERROR_API_NOT_SUPPORTED
/// Indicates an operation was not supported for the given device.
/// Usually due to an operation not being supported for the technology type of the device in use
#define LR_ERROR_API_NOT_SUPPORTED			0x0014

/// \def LR_ERROR_BAD_SPC_MODE
/// Reserved. Indicates an error in communication with device
#define LR_ERROR_BAD_SPC_MODE				0x0015

/// \def LR_ERROR_DEVICE_NOT_AVAILABLE			
/// Inidicates that no device is available to perform the operation on
#define LR_ERROR_DEVICE_NOT_AVAILABLE		0x001B

/// \def LR_ERROR_FS_ERROR		
/// Inidicates a file system error
#define LR_ERROR_FS_ERROR					0x001C

/// \def LR_ERROR_ACTIVATION_FAILED			
/// Inidicates an failure during activation
#define LR_ERROR_ACTIVATION_FAILED		    0x001D

/// \def LR_ERROR_AT_CMD_ERROR_RESPONSE			
/// Indicates an AT command resulted in an 'ERROR' response
#define LR_ERROR_AT_CMD_ERROR_RESPONSE		0x0030

/// \def LR_ERROR_DEVICE_ALREADY_LOCKED			
/// Indicates that the device was already locked when attempting to lock it.
#define LR_ERROR_DEVICE_ALREADY_LOCKED		0x0031

/// \def LR_ERROR_DEVICE_ALREADY_UNLOCKED		
/// Indicates that the device was already unlocked when attempting to unlock it.
#define LR_ERROR_DEVICE_ALREADY_UNLOCKED	0x0032

/// \def LR_ERROR_INVALID_LOCK_CODE			
/// Indicates that the supplied lock code was invalid.
#define LR_ERROR_INVALID_LOCK_CODE			0x0033

/// \def LR_ERROR_INVALID_ACTIVATION_CODE			
/// Indicates that the supplied activation code was invalid.
#define LR_ERROR_INVALID_ACTIVATION_CODE	0x0034

/// \def LR_ERROR_OPERATION_NOT_ALLOWED			
/// Indicates that the operation was not permitted, most likely due to the current
/// state of the device. 
#define LR_ERROR_OPERATION_NOT_ALLOWED		0x0035

/// \def LR_ERROR_AT_CMD_ERROR_SIM_BUSY	
/// Indicates that an AT command resulted in a 'SIM BUSY' response
#define LR_ERROR_AT_CMD_ERROR_SIM_BUSY		0x0036

/// \def LR_ERROR_INVALID_RESPONSE			
/// Reserved. Indicates an error in communication with device
#define LR_ERROR_INVALID_RESPONSE			0x0037

/// \def LR_ERROR_SMS_SENDING		
/// Indicates a failure to send an sms message
#define LR_ERROR_SMS_SENDING				0x0038

/// \def LR_ERROR_BAD_MODE			
/// Reserved. Indicates an error in communication with device
#define LR_ERROR_BAD_MODE					0x0040

/// \def LR_ERROR_NO_MEMORY				
/// Indicates an error due to insufficient memory
#define LR_ERROR_NO_MEMORY					0x0042

/// \def LR_ERROR_FILE_OPEN
/// Indicates an error opening a file on EFS
#define LR_ERROR_FILE_OPEN                  0x0044

/// \def LR_ERROR_FILE_WRITE
/// Indicates an error writing a file to EFS
#define LR_ERROR_FILE_WRITE                 0x0045

/// \def LR_ERROR_FILE_READ
/// Indicates an error reading a file from EFS
#define LR_ERROR_FILE_READ                  0x0046

/// \def LR_ERROR_FILE_STAT
/// Indicates an error reading a stat information from EFS
#define LR_ERROR_FILE_STAT                  0x0047

/// \def LR_ERROR_SERVER_NOT_AVAILABLE
/// Indicates that the SDK Server component was unavailable.
#define LR_ERROR_SERVER_NOT_AVAILABLE       0x0048

/// \def LR_ERROR_PARAM_TOO_LARGE
/// Indicates that a supplied parameter exceeded the allowed size.
#define LR_ERROR_PARAM_TOO_LARGE            0x0049

/// \def LR_ERROR_MISSING_BUNDLE_ID
/// Mac OS X only, indicates an application bundle identifier could not be found.
/// This can be due to a missing Info.plist file or a missing entry for CFBundlerIdentifier in the Info.plist file.
/// This application is setting is required to establish application priviledges for creating and modifying 3G network services
#define LR_ERROR_MISSING_BUNDLE_ID          0x0050

/// \def LR_ERROR_OEM_SWITCH
/// Indicates that the power mode is being controlled by a hardware switch
#define LR_ERROR_OEM_SWITCH                 0x0051

/// \def LR_ERROR_UNKNOWN_HOST
/// Indicates that an internet host name could not be resolved.
#define LR_ERROR_UNKNOWN_HOST				0x1007		//server host name can not be resolved

/// \def LR_ERROR_HOST_UNREACHABLE
/// Indicates that a connection to an internet host could not be created.
#define LR_ERROR_HOST_UNREACHABLE			0x1008		//server host not available

/// \def LR_ERROR_GPS_XTRA_FILE_DOWNLOAD
/// Indicates a filure to download the XTRA data file for GPS.
#define LR_ERROR_GPS_XTRA_FILE_DOWNLOAD		0x1009		//XTRA File Download Error

/// \def LR_ERROR_GPS_XTRA_FILE_WRITE
/// Indicates a failure to write the XTRA data file to the device.
#define LR_ERROR_GPS_XTRA_FILE_WRITE		0x100A		//XTRA File Write Error

/// \def LR_ERROR_GPS_TIME_SYNC_PENDING
/// Indicates that the GPS engine is waiting for a time synchronization when using XTRA mode gps.
#define LR_ERROR_GPS_TIME_SYNC_PENDING		0x100B		//Time Sync is pending. Used for Internet-Assisted GPS

#define LR_ERROR_SIM_NOT_READY				0x100C

///
#define LR_ERROR_PDP_CONNECT_FAILED			0x100D


//Error Codes - End

//size definitinos
/// \def NW_SIZE_NAME
/// Defines the basic size in characters of name fields
#define NW_SIZE_NAME				64

/// \def NW_SIZE_NUMBER
/// Defines the basic size in characters of number fields
#define NW_SIZE_NUMBER				22

/// \def NW_ACTIVATION_CODE_SIZE
/// Defines the basic size in characters of spc code fields
#define	NW_ACTIVATION_CODE_SIZE		7

/// \def MAX_PDU_LENGTH
/// Defines the max size in bytes of encoded PDU values
#define MAX_PDU_LENGTH				200

/// \def STRING_LEN
/// Defines the basic size in characters of string fields
#define STRING_LEN                  260

/// \def NW_MAX_PATH
/// Defines the basic size in characters of string fields
#define NW_MAX_PATH                 260

/// \def SdkHandle
/// Defines the handle type used to reference sdk sessions
#define SdkHandle	void*

/// \def NVTLAPIDEF_DS_UMTS_MAX_APN_STRING_LEN
/// Defines maximum length for apn_string
#define NVTLAPIDEF_DS_UMTS_MAX_APN_STRING_LEN 100

#pragma pack(push, 1)

/*******************************************************************
Physical Device Types
Types related to physical device technology and platform attributes
********************************************************************/

/// \enum DeviceTechType
/// This enum defines the available device technology types
typedef enum
{
	DEV_NONE		= 0,        /*!< Unkown device type */
	DEV_EVDO		= 4,        /*!< Evdo device */
	DEV_UMTS		= 7,	    /*!< UMTS device */
	DEV_HSDPA		= 8,        /*!< HSPA device */    
	DEV_EVDO_REVA	= 9,        /*!< Evdo REV-A device */
	DEV_MULTI_MODE	= 11,       /*!< Multi-Mode device (any or all of CDMA/WCDMA/LTE)*/
}DeviceTechType;

/// \enum SourceBuildBaseType 
/// This enum defines the base hardware platform
typedef enum 
{
	SRC_BUILD_BASE_UNKNOWN	 = 0,   /*!< Place Holder */
	SRC_BUILD_BASE_BLACKBIRD = 1,   /*!< Place Holder */
	SRC_BUILD_BASE_RAPTOR	 = 2,   /*!< Place Holder */
	SRC_BUILD_BASE_HSDPA	 = 3,   /*!< Place Holder */
	SRC_BUILD_BASE_HSDPA_7_2 = 4,   /*!< Place Holder */
    SRC_BUILD_BASE_HSDPA_3_6 = 5,   /*!< Place Holder */
    SRC_BUILD_BASE_INDIAN    = 6,   /*!< Place Holder */
    SRC_BUILD_BASE_9600      = 7,   /*!< Place Holder */
    SRC_BUILD_BASE_8220      = 8,   /*!< Place Holder */ 
	SRC_BUILD_BASE_9200      = 9,   /*!< Place Holder */
}SourceBuildBaseType;

/// \enum DeviceMobModelType
/// This enum defines the chipset model of the device
typedef enum
{
	DEV_MOB_MODEL_6500		    = 153,  /*!< Place Holder */
	DEV_MOB_MODEL_6800		    = 238,  /*!< Place Holder */
	DEV_MOB_MODEL_6800_65NM	    = 172,  /*!< Place Holder */
	DEV_MOB_MODEL_6085			= 119,  /*!< BLUE Device  */
	DEV_MOB_MODEL_HSDPA_XU870	= 209,  /*!< Place Holder */
	DEV_MOB_MODEL_HSDPA_U730	= 226,  /*!< Place Holder */
    DEV_MOB_MODEL_7627          = 191,  /*!< Place Holder */
    DEV_MOB_MODEL_9600          = 210, /*!< Place Holder */
    DEV_MOB_MODEL_9600_W        = 4050, /*!< Place Holder */
    DEV_MOB_MODEL_8220          = 4052, /*!< Place Holder */
	DEV_MOB_MODEL_9200          = 4051, /*!< ???Place Holder */
} DeviceMobModelType;


/// \enum DeviceFormFactorType
/// This enum defines the physical form factor of the device
typedef enum 
{
	DEV_TYPE_PC_CARD			= 0,    /*!< Place Holder */
	DEV_TYPE_MINI_PCI			= 1,    /*!< Place Holder */
	DEV_TYPE_EXPRESS_CARD		= 2,    /*!< Place Holder */
	DEV_TYPE_SM_BUS_MINI_PCI	= 3,    /*!< Place Holder */
	DEV_TYPE_GOBI_MINI_PCI		= 4,    /*!< Place Holder */
    DEV_TYPE_USB                = 5,    /*!< Place Holder */
    DEV_TYPE_MIFI               = 6,    /*!< Place Holder */
	DEV_TYPE_GOBI2_MINI_PCI		= 7,    /*!< Place Holder */
    DEV_TYPE_VIA_USB            = 8,    /*!< Place Holder */
	DEV_TYPE_GOBI3_MINI_PCI		= 9,    /*!< Place Holder */
} DeviceFormFactorType;

#define IsGobi(ffactor)  ( (ffactor) == DEV_TYPE_GOBI_MINI_PCI || (ffactor) == DEV_TYPE_GOBI2_MINI_PCI || (ffactor) == DEV_TYPE_GOBI3_MINI_PCI )
#define IsGobi1(ffactor) ( (ffactor) == DEV_TYPE_GOBI_MINI_PCI ) 
#define IsGobi2(ffactor) ( (ffactor) == DEV_TYPE_GOBI2_MINI_PCI || (ffactor) == DEV_TYPE_GOBI3_MINI_PCI ) // for now we treat GOBI3 the same as GOBI2.
#define IsGobi3(ffactor) ( (ffactor) == DEV_TYPE_GOBI3_MINI_PCI )
#define IsVia(ffactor) ( (ffactor) == DEV_TYPE_VIA_USB )

/// \enum ServiceProviderType
/// For CDMA device this enum defines the carrier this device was originally provisioned for.
typedef enum
{
	SRV_PROVIDER_UNKNOWN	= 0,        /*!< Unknown provider */
	SRV_PROVIDER_SPRINT		= 1,        /*!< Sprint */
	SRV_PROVIDER_VERIZON	= 2,        /*!< Verizon */
	SRV_PROVIDER_TELUS		= 3,        /*!< Telus */
	SRV_PROVIDER_BELL_MOBILITY = 4,     /*!< Bell Mobility */
	SRV_PROVIDER_RADIO_FREE = 5,        /*!< Radio Free */
	SRV_PROVIDER_MAX,                   /*!< Place Holder */
}ServiceProviderType;
/*******************************************************************
End Physical Device Types
********************************************************************/


/*******************************************************************
State and Network related types
Types related to device and network state
********************************************************************/

//power mode of the device

/// \enum DeviceModeType
/// This enum defines values for the power mode of the device.
typedef enum
{
	DEV_MODE_MIN		= 0,    /*!< Place holder */
	DEV_MODE_POWER_OFF  = 0,    /*!< (only for GOBI devices) */
	DEV_MODE_FTM		= 1,    /*!< Offline Factory Test Mode */
	DEV_MODE_OFFLINE	= 2,    /*!< <tt><b>(not used)</b></tt> */
	DEV_MODE_OFFLINE_A	= 3,    /*!< <tt><b>(not used)</b></tt> Offline Analog Mode for legacy networks */
	DEV_MODE_OFFLINE_D	= 4,	/*!< Offline Digital mode */
	DEV_MODE_ONLINE		= 5,   	/*!< Online mode; in this mode, the mobile can acquire the system and make calls */
	DEV_MODE_LPM		= 6,    /*!< Low Power mode; in this mode, the device is disabled and will be in least power consumption and will neither attempt any acquisitions nor be on a system */
	DEV_MODE_CASL_POWER_OFF	= 7,    /*!< HP ONLY CASL Power OFF mode; ONLY SPECIFIC TO HP. In this mode,The device is removed from the device manager and the State is forced to POWER OFF. This is not same as no card */
	DEV_MODE_CASL_POWER_ON	= 8,    /*!< HP ONLY CASL Power ON  mode; ONLY SPECIFIC TO HP. In this mode,The device is added back to device manager */
	DEV_MODE_MAX,               /*!< Place Holder */
	DEV_MODE_UNKNOWN	= 99,   /*!< Unknown mode */
	DEV_MODE_RESET		= 100,  /*!< Resets the device if currently in offline mode */
} DeviceModeType;

/// \enum DeviceLockStatusType
/// This enum defines values for current lock status of the device
typedef enum 
{
	DEV_UNLOCKED		= 0,	/*!< Device is not locked */
	DEV_LOCKED			= 1,	/*!< <tt><b>\<CDMA/EVDO devices only\></b></tt> The device is locked */
	DEV_NETWORK_LOCKED	= 2,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> The device is network locked. This means that the device disallows use of the particular SIM that is inserted.*/
	DEV_PIN1_LOCKED		= 3,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> The SIM is PIN locked. PIN number is required to unlock the SIM. */
	DEV_PIN2_LOCKED		= 4,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> The SIM is PIN2 locked. PIN2 number is required to unlock the SIM. */
	DEV_PUK1_LOCKED		= 5,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> The SIM is PUK locked. PUK code is required to unlock the SIM */
	DEV_PUK2_LOCKED		= 6,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> The SIM is PUK2 locked. PUK2 code is required to unlock the SIM */
	DEV_SIM_FAILURE		= 7,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> Failure in communicating with the SIM */
	DEV_SERVICE_PROVIDER_LOCKED	= 8,/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> The device is service-provider locked. This means that the device disallows use of the particular SIM that is inserted.*/
	DEV_NETWORK_SUBSET_LOCKED	= 9,/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> The device is network-subset locked. This means that the device disallows use of the particular SIM that is inserted.*/
	DEV_CORPORATE_LOCKED		= 10,/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> The device is corporate locked. This means that the device disallows use of the particular SIM that is inserted.*/
	DEV_AUTOLOCKED		= 0x80 	/*!< <tt><b>\<not used\></b></tt> */
} DeviceLockStatusType;

/// \enum DeviceLockType
/// This enum defines values for the type of lock currently applied on the device
typedef enum 
{
	DEV_LOCK		= 0,	/*!< <tt><b>\<CDMA/EVDO devices only\></b></tt> Unlock the device */
	DEV_NET_LOCK	= 1,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> Unlock Network Lock */
	DEV_PIN1_LOCK	= 2,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> Unlock PIN1 Lock */
	DEV_PIN2_LOCK	= 3,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> Unlock PIN2 Lock */
	DEV_PUK1_LOCK	= 4,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> Unlock PUK Lock */
	DEV_PUK2_LOCK	= 5,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> Unlock PUK2 Lock */
	DEV_AUTOLOCK	= 6,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> Unlock Auto-Lock */
	DEV_SP_LOCK		= 7,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> Unlock Service-Provider Lock */
	DEV_NETSUBSET_LOCK= 8,	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> Unlock Network-Subset Lock */
	DEV_CORPORATE_LOCK= 9	/*!< <tt><b>\<UMTS and HSDPA devices only\></b></tt> Unlock Corporate Lock */
} DeviceLockType;

/// \enum DeviceStateType
/// This enum defines values for the current working state of the device
typedef enum
{
    NW_DEVICE_STATE_UNKNOWN         = 0,  /*!< The state of device cannot be determined. Usually sent during startup of the SDK. */
    NW_DEVICE_STATE_NOCARD          = 1,  /*!< No device has been detected */
    NW_DEVICE_STATE_INITIALIZING    = 2,  /*!< The SDK has recognized a device and is currently in the process of connecting to and initializing it for use. 
    									   * Generally the device is not yet ready for normal operation and clients should wait until the SDK leaves this state before proceeding. */
    NW_DEVICE_STATE_DISABLED        = 3,  /*!< Indicates that a valid device is detected but is disabled. This is generally due to a software or hardware control switch being turned off on the host. */
    NW_DEVICE_STATE_LOCKED          = 4,  /*!< Indicates that a device is detected but is currently locked and requires a PIN or PUK to be entered before use. */
    NW_DEVICE_STATE_SEARCHING       = 5,  /*!< Indicates that the device is available and is currently searching for network service. */
    NW_DEVICE_STATE_IDLE            = 6,  /*!< Indicates that the device is available for general use. This state indicates the normal ready state of the device. */
    NW_DEVICE_STATE_CONNECTING      = 7,  /*!< A connection attempt has been made and the device is trying to establish a data connection. */
    NW_DEVICE_STATE_AUTHENTICATING  = 8,  /*!< A connection attempt has been made and the device is authenticating user credentials for the data connection. */
    NW_DEVICE_STATE_CONNECTED       = 9,  /*!< Indicates that the device has an active data connection */
    NW_DEVICE_STATE_RESERVED_2      = 10, /*!< <tt><b>(not used)</b></tt> */
    NW_DEVICE_STATE_ACTIVATION      = 11, /*!< <tt><b>(not used)</b></tt> */
	NW_DEVICE_STATE_CASL_POWERED_OFF = 12, /*!< HP ONLY. Indicates that the CASL SDK POWERED OFF*/
    NW_DEVICE_STATE_NULL            = 0xFF /*!< <tt><b>(not used)</b></tt> */
}DeviceStateType;

/// \enum DeviceRoamStatusType
/// This enum define svalues for the Roaming Stauts of the device.
typedef enum
{
	DEVICE_STATUS_HOME		 = 0,	/*!< The device is currently on the home network */
	DEVICE_STATUS_ROAM		 = 1,	/*!< The device is currently roaming away from the home network */
	DEVICE_STATUS_ROAM_FLASH = 2,	/*!< The device is currently roaming away from the home network and the connection manager should flash the roaming indication icon */
	DEVICE_STATUS_ERI		 = 3	/*!< The device is currently roaming away from the home network and the connection manager should follow the appropriate rules based
										* on the ERI (Extended Roaming Indicator) information. */
} DeviceRoamStatusType;

/// \enum RoamIndicatorType
/// <tt><b>\<CDMA/EVDO devices only\></b></tt>
/// \n This enum defines values for roaming indication icon's index
typedef enum
{
	ROAM_INDICATOR_OFF					= 0,    /*!< Roaming indicator should be off */
	ROAM_INDICATOR_ON					= 1,    /*!< Roaming indicator shoudd be on */
	ROAM_INDICATOR_FLASH				= 2,    /*!< Roaming indicator should be flashing */    
	ROAM_INDICATOR_CUSTOM_IMAGE_ON		= 3,    /*!< Custom roaming indicator image should be on*/
	ROAM_INDICATOR_CUSTOM_IMAGE_FLASH	= 4     /*!< Custom roaming indicator image should be flashing */
} RoamIndicatorType;

/// \enum DeviceServiceType
/// This enum defines the type of Network Service that the device is attached to.
typedef enum
{
	NW_SERVICE_NONE = 0,	/*!< No network */
	NW_SERVICE_AMPS,		/*!< Network Service type AMPS (Advanced Mobile Phone Service) */
	NW_SERVICE_IS95A,		/*!< Network Service type IS95A (Interim Standard A) */
	NW_SERVICE_IS95B,		/*!< Network Service type IS95B (Interim Standard B) */
	NW_SERVICE_GPRS,		/*!< Network Service type GPRS (General Packet Radio Service) */
	NW_SERVICE_1XRTT,		/*!< Network Service type 1XRTT */
	NW_SERVICE_EVDO,		/*!< Network Service type EV-DO (Evolution Data Optimized) */
	NW_SERVICE_UMTS,		/*!< Network Service type UMTS (Universal Mobile Telecommunications System) */
	NW_SERVICE_HSDPA,		/*!< Network Service type HSDPA (High Speed Downlink Packet Access) */
	NW_SERVICE_EDGE,		/*!< Network Service type EDGE (Enhanced Data rates for Global Evolution) */
	NW_SERVICE_EVDO_REVA,	/*!< Network Service type EV-DO REV A */
	NW_SERVICE_HSUPA,		/*!< Network Service type HSUPA (High Speed Uplink Packet Access) */
	NW_SERVICE_HSPA_PLUS,	/*!< Network Service type HSPA+ (Evovled High Speed Packet Access) */
    NW_SERVICE_LTE,     	/*!< Network Service type LTE (Long Term Evolution) */
    NW_SERVICE_HSPA_PLUS_DC,/*!< Network Service type DC HSPA (Dual-Cell HSPA)*/
    NW_SERVICE_EVDO_REV0,   /*!< Network Service type EV-DO REV 0 */
    NW_SERVICE_EVDO_REVB,   /*!< Network Service type EV-DO REV B */
    NW_SERVICE_EVDO_EHRPD,  /*!< Network Service type EV-DO eHRPD (Evolved High Rate Packet Data) */
} DeviceServiceType;

typedef enum 
{
    NW_NETWORK_HOME = 0,           /*!<Icon Indicator Value = 1 */
    NW_NETWORK_EXTENDED,           /*!<Icon Indicator Value = 0 */
    NW_NETWORK_ROAM,               /*!<Icon Indicator Value = 2 */
    NW_NETWORK_SEARCHING_CDMA,     /*!<Searching for CDMA Network */
    NW_NETWORK_SEARCHING_GSM,      /*!<GSM - Searching for GSM Network */
    NW_NETWORK_DENIED,             /*!<GSM - Network Registration Denied */
    NW_NETWORK_NOT_REG,            /*!<GSM - Not Registered to a Network */
    NW_NETWORK_SWITCHING_TECH,     /*!<Switching  search technology between CDMA and GSM */
    NW_NETWORK_SEARCH_EXHAUSTED,   /*!<Network search exhausted, device in deep sleep ans will retry later */
    NW_NETWORK_SEARCHING_LTE,      /*!<LTE (4G GSM) - Searching for Network */
    NW_NETWORK_LTE_RRC_CONNECTING, /*!<Device has started to attach to the LTE network, but has not yet started authenticating.  (Registering) */
    NW_NETWORK_LTE_AUTHENTICATING, /*!<Device has started to attach to the LTE network, but has not yet started authenticating. */
    NW_NETWORK_LTE_RRC_IDLE,       /*!<Device is attached to the LTE network and in the RRC_IDLE state. */
    NW_NETWORK_LTE_RRC_DISCONNECTING,  /*!<Device has started to detach from the LTE network.  The device should then return to NW_NETWORK_NOT_REG. */
} LTENetworkType;

/// \enum GSMNetworkType
/// This enum defines values that identify the different GSM network technologies supported by the device. 
typedef enum 
{
	NW_NETWORK_TECH_GSM		= 0x0001, /*!< reserved */
	NW_NETWORK_TECH_GPRS	= 0x0002, /*!< GPRS (General Packet Radio Service) */
	NW_NETWORK_TECH_EDGE	= 0x0004, /*!< EDGE (Enhanced Data rates for Global Evolution) */
	NW_NETWORK_TECH_UMTS	= 0x0008, /*!< UMTS (Universal Mobile Telecommunications System) */
	NW_NETWORK_TECH_HSDPA	= 0x0010, /*!< HSDPA (High Speed Downlink Packet Access) */
    NW_NETWORK_TECH_HSUPA   = 0x0020, /*!< HSUPA (High Speed Uplink Packet Access) */
    NW_NETWORK_TECH_HSPA_PLUS  = 0x0040, /*!< HSPA+ (Evovled High Speed Packet Access) */
    NW_NETWORK_TECH_HSPA_PLUS_DC = 0x0080, /*!< HSPA+DC (Dual-Cell HSPA) */
    NW_NETWORK_TECH_UNDEFINED = 0x0100, /*!< reserved */
    NW_NETWORK_TECH_LTE    = 0x0200,  /*!< LTE (Long Term Evolution) */
} GSMNetworkType;

/// \enum DeviceDisableActionType
/// This enum defines values that identify the cause for why a device is disabled or in Low Power Mode. 
typedef enum
{
    NW_WD_ACTION_HW_DISABLE			= 0,    /*!< Place hodler */
    NW_WD_ACTION_SW_DISABLE			= 1,    /*!< Place hodler */
    NW_WD_ACTION_TEMPRATURE_DISABLE = 2,    /*!< Place hodler */
} DeviceDisableActionType;

/// \enum DeviceDisableMaskType
/// This enum defines values that identify the cause for why a device is disabled or in Low Power Mode.
typedef enum{
    NW_WD_MASK_HW_DISABLE		  = 0x0001,	/*!< The hardware control switch is turned off */
    NW_WD_MASK_SW_DISABLE		  = 0x0002,	/*!< The device is software disabled */
    NW_WD_MASK_TEMPRATURE_DISABLE = 0x0004,	/*!< The device is disabled because it has reached unsafe temperature levels */
} DeviceDisableMaskType;


/// \enum CASLDeviceDisableMaskType
/// HP ONLY This enum defines values that identify the cause for why a device is disabled or in Low Power Mode.
typedef enum{
    NW_WD_MASK_CASL_HW_DISABLE		  = 0x0001,	/*!< The hardware control switch is turned off */
    NW_WD_MASK_CASL_BIOS_DISABLE	  = 0x0002,	/*!< The device is BIOS disabled */
    NW_WD_MASK_CASL_DEVICE_STATE_DISABLE		  = 0x0004,	/*!< The device is disabled because Wireless button state requested  */
	NW_WD_MASK_CASL_SIM_ACCESS_DISABLE = 0x0008,	/*!< The device is disabled because SIMCardAccess Mask is enabled and viceversa */	
} CASLDeviceDisableMaskType;


/// \enum RoamPreferenceType
/// This enum defines values for the roam preferences for the device.
typedef enum{
    NW_ROAM_PREF_HOME	= 1,	/*!< Home only */
	NW_ROAM_PREF_AFFIL	= 3,	/*!< Affilated networks only */
	NW_ROAM_PREF_ROAM	= 6,	/*!< Roam Only */
	NW_ROAM_PREF_ANY	= 255	/*!< Automatic */
}RoamPreferenceType;

/// \enum ConnectionStatusExMask
/// Only valid for Garter device and Firmware above 1.26. This enum provides mask for the events received for validating ConnectionStatusEX struct for Event Driven SDK.
typedef enum{
    NW_WD_MASK_CONNECT_BYTES_N_DURATION	= 0x0001,	/*!< The BYTES IN/OUT and the Duration event is recieved */
    NW_WD_MASK_CONNECT_STATUS			= 0x0002,	/*!< The Connection Staus event is recieved */
    NW_WD_MASK_CONNECT_IP_ADDRESS		= 0x0004,	/*!< The Connection IP address event is recieved   */
	NW_WD_MASK_CONNECT_MULTIMODE		= 0x0008,	/*!< The Connection is for Multimode device*/	
	NW_WD_MASK_CONNECT_VALID			= 0x000F	/*!< This field should be used for validation.This field indicates all the above values are received.*/	
} ConnectionStatusExMask;

/// \enum NetworkModePreference
/// <tt><b>\<CDMA/EVDO devices only\></b></tt>
/// \n This enum defines values for the preferred network mode
typedef enum
{ 
    NW_MODE_DIGITAL_PREF          = 0,  /*!< CDMA then Analog */
    NW_MODE_DIGITAL_ONLY          = 1,  /*!< CDMA only */
    NW_MODE_ANALOG_PREF           = 2,  /*!< Analog then CDMA */
    NW_MODE_ANALOG_ONLY           = 3,  /*!< Analog only */
    NW_MODE_AUTOMATIC             = 4,  /*!< Mode is determined automatically */
    NW_MODE_E911                  = 5,  /*!< Emergency mode */
    NW_MODE_HOME_ONLY             = 6,  /*!< Restrict to home only */
    NW_MODE_PCS_CDMA_ONLY         = 7,  /*!< Restrict to PCS home only */
    NW_MODE_CELL_CDMA_ONLY        = 8,  /*!< Restrict to cellular home only */
    NW_MODE_CDMA_ONLY             = 9,  /*!< Place hodler */
    NW_MODE_HDR_ONLY              = 10, /*!< Place hodler */
    NW_MODE_CDMA_AMPS_ONLY        = 11, /*!< Place hodler */
    NW_MODE_GPS_ONLY              = 12, /*!< Place hodler */
    NW_MODE_GSM_ONLY              = 13, /*!< Restrict to GSM only */
    NW_MODE_WCDMA_ONLY            = 14, /*!< Restrict to WCDMA only */
}NetworkModePreference;

/// \enum RatModePreference
/// <tt><b>\<UMTS/HSPA devices only\></b></tt>
/// \n This enum defines values for the preferred radio access technology (RAT) mode
typedef enum
{
    NW_RAT_MODE_AUTO    = 0,
    NW_RAT_MODE_GSM     = 1,
    NW_RAT_MODE_WCDMA  = 2,
}RatModePreference;

/// \enum DisableEventSigType
/// This enum is used internally
typedef enum 
{
	W_DISABLE_SIG_LPM			 = 0,   /*!< Place hodler */
	W_DISABLE_SIG_ONLINE_ATTEMPT = 1    /*!< Place hodler */
} DisableEventSigType;

/// \enum SARRFState
/// This enum is used to set and retrieve the SAR(Specific Absorption Rate) RF power level backoff state.
typedef enum
{
	QMI_SAR_RF_STATE_DEFAULT    = 0,
	QMI_SAR_RF_STATE_1,          
	QMI_SAR_RF_STATE_2,          
	QMI_SAR_RF_STATE_3,          
	QMI_SAR_RF_STATE_4,         
	QMI_SAR_RF_STATE_5,          
	QMI_SAR_RF_STATE_6,          
	QMI_SAR_RF_STATE_7,          
	QMI_SAR_RF_STATE_8,         
} SARRFStates;

/*******************************************************************
END State and Network related types
********************************************************************/

/*******************************************************************
EVDO configuration and setting related types
Types related to EVDO settings 
********************************************************************/

/// \enum SysPrevType
/// <tt><b>\<CDMA/EVDO devices only\></b></tt> \n
/// This enum defines values for the Protocol Revision (PREV) preference.
typedef enum
{
	PREV_CDMA_JSTD008		= 1,	/*!< PREV JSTD008 */
	PREV_CDMA_95A			= 2,	/*!< <tt><b>(not used)</b></tt> */
	PREV_CDMA_95A_TSB74		= 3,	/*!< PREV TSB74 */
	PREV_CDMA_95B_PHASE1	= 4,	/*!< <tt><b>(not used)</b></tt> */
	PREV_CDMA_95B_PHASE2	= 5,	/*!< <tt><b>(not used)</b></tt> */
	PREV_CDMA_2000_REL0		= 6,	/*!< PREV REL0 */
	PREV_CDMA_2000_RELA		= 7		/*!< <tt><b>(not used)</b></tt> */
} SysPrevType;

/// \enum PreferredModeType
/// <tt><b>\<CDMA/EVDO devices only\></b></tt> \n
/// This enum defines values for the preferred network mode.
typedef enum
{
	NV_PREF_MODE_DIGITAL_PREF	= 0,	/*!< CDMA then Analog */
	NV_PREF_MODE_DIGITAL_ONLY	= 1,	/*!< CDMA only */
	NV_PREF_MODE_ANALOG_PREF	= 2,	/*!< Analog then CDMA */
	NV_PREF_MODE_ANALOG_ONLY	= 3,	/*!< Analog only */
	NV_PREF_MODE_AUTOMATIC		= 4,	/*!< Mode is determined automatically */
	NV_PREF_MODE_E911			= 5,	/*!< Emergency mode */
	NV_PREF_MODE_HOME_ONLY		= 6,	/*!< Restrict to home only */
	NV_PREF_MODE_PCS_CDMA_ONLY	= 7,    /*!< Place hodler */
	NV_PREF_MODE_CELL_CDMA_ONLY	= 8,    /*!< Place hodler */
	NV_PREF_MODE_CDMA_ONLY		= 9,    /*!< Place hodler */
	NV_PREF_MODE_HDR_ONLY		= 10,   /*!< Place hodler */
	NV_PREF_MODE_CDMA_AMPS_ONLY	= 11,   /*!< Place hodler */
	NV_PREF_MODE_GPS_ONLY		= 12,   /*!< Place hodler */
	NV_PREF_MODE_GSM_ONLY		= 13,   /*!< Place hodler */
	NV_PREF_MODE_WCDMA_ONLY		= 14,   /*!< Place hodler */
	NV_MODE_CDMA_HDR_ONLY		= 19,	/*!< 1xRTT and EVDO only */
	NV_MODE_LTE_ONLY			= 30,   /*!< LTE only */
	NV_MODE_GWL					= 31,   /*!< GSM, WCDMA and LTE only */
	NV_MODE_CDMA_LTE_ONLY		= 32,   /*!< CDMA and LTE only */
	NV_MODE_HDR_LTE_ONLY		= 33,   /*!< HDR-LTE only */
	NV_MODE_GSM_LTE_ONLY		= 34,   /*!< GSM and LTE only */
	NV_MODE_WCDMA_LTE_ONLY		= 35,   /*!< WCDMA and LTE only */
	NV_MODE_CDMA_HDR_LTE_ONLY	= 36,   /*!< CDMA-HDR and LTE only */
	NV_MODE_CDMA_GSM_LTE_ONLY	= 37,   /*!< CDMA, GSM and LTE only */
	NV_MODE_CDMA_WCDMA_LTE_ONLY	= 38,   /*!< CDMA, WCDMA and LTE only */
	NV_MODE_HDR_GSM_LTE_ONLY	= 39,   /*!< HDR, GSM and LTE only */
	NV_MODE_HDR_WCDMA_LTE_ONLY	= 40,   /*!< HDR, WCDMA and LTE only */
	NV_MODE_CDMA_GSM_WCDMA		= 51,	/*!< CDMA, GSM and WCDMA only*/

} PreferredModeType;

/// \enum SystemPreferenceType
/// This enum defines values for system preference
typedef enum
{
	NV_SYS_PREF_A_ONLY		= 0,	/*!< System A only */
	NV_SYS_PREF_B_ONLY		= 1,	/*!< System B only */
    NV_SYS_PREF_HOME_ONLY	= 2,	/*!< Home only */
    NV_SYS_PREF_STANDARD	= 3		/*!< Home preferred */    
}SystemPreferenceType;

/// \enum HdrProtocolType
/// <tt><b>\<CDMA/EVDO devices only\></b></tt> \n
/// This enum defines values for the type of HDR protocol. 
typedef enum
{
	CFG_HDR_PROTOCOL_UNKNOWN = 0,	/*!< Current HDR protocol could not be defined */
	CFG_HDR_PROTOCOL_REV0	 = 1,	/*!< REV 0 protocol */
	CFG_HDR_PROTOCOL_REVA	 = 2,	/*!< REV A protocol */
} HdrProtocolType;

/// \enum BandPreferenceType
/// <tt><b>\<CDMA/EVDO devices only\></b></tt> \n
/// This enum defines values for the band class preference
typedef enum
{
	NV_BAND_PREF_CELL		= 3,	/*!< Band class CELL */
	NV_BAND_PREF_PCS		= 4,	/*!< Band class PCS */
	NV_BAND_PREF_AUTOMATIC  = 65535	/*!< Determine band class automatically */
} BandPreferenceType;

/// \enum MipQcmipType
/// <tt><b>\<CDMA/EVDO devices only\></b></tt> \n
/// This enum defines modes for Mobile IP (MIP) behaviour.
typedef enum
{
	NV_MIP_QCMIP_MIP_DISABLED	= 0,	/*!< Device uses Simple IP only */
	NV_MIP_QCMIP_MIP_PREFERRED  = 1,	/*!< Device uses Mobile IP with fallback to Simple IP */
	NV_MIP_QCMIP_MIP_ONLY		= 2		/*!< Device uses Mobile IP only */
} MipQcmipType;

/// \enum BATTERY_STATE
///This enum defines values that describe the state of the device's battery.  These values are used with SDK method Phoenix::IPhoenix::GetBatteryInfo.
typedef enum {
	BATTERY_POWERED = 0,	/*!< Battery is present; Powered by battery. */
	BATTERY_EXTERNAL_POWER = 1,	/*!< Battery is present; Powered by external source. */
	BATTERY_UNAVAILABLE = 2,	/*!< Battery is absent. */
} BATTERY_STATE;

/*******************************************************************
END EVDO configuration and setting related types
********************************************************************/

/*******************************************************************
EVDO activation related types
Types related to EVDO activation
********************************************************************/

/// \enum OmaType
/// This enum defnes the type of OMA-DM sessions available for use with OMA-DM methods.
typedef enum
{
	OMA_TYPE_DM             = 0, /*!< OMA-DM session for device activation */
	OMA_TYPE_PRL            = 1, /*!< OMA-DM session for PRL updates */
    OMA_TYPE_HFA            = 2, /*!< Hands Free Activation sesson */
    OMA_TYPE_FUMO_QUERY		= 3, /*!< Query for firmware update */
	OMA_TYPE_FUMO_UPDATE	= 4, /*!< FUMO session for firmware update */
    OMADI_TYPE_HFA          = 5, /*!< GOBI Only. Device Initiated Hands Free Activation sesson */
    OMANI_TYPE_PRL          = 6, /*!< GOBI Only. Network Initiated session for PRL updates */
    OMANI_TYPE_DM           = 7, /*!< GOBI Only. Network Initiated session for device activation */
    OMA_TYPE_FUMO_UPDATE_NOW = 8, /*!< Bell Mobility. Update now */
    OMA_TYPE_FUMO_UPDATE_LATER = 9, /*!< Bell Mobility. Update later, within 24 hours */
    OMA_TYPE_FUMO_UPDATE_OK = 10, /*!< Bell Mobility. Update is okay to happen now */
    OMA_TYPE_LTE_DOWNLOAD_OK = 11, /*!< LTE devices. Approve download of new FW package */
    OMA_TYPE_LTE_DOWNLOAD_REJECT = 12, /*!< LTE devices. Reject download of new FW package */
    OMA_TYPE_LTE_INSTALL_OK = 13,       /*!< LTE devices. Approve installation of new FW package */
    OMA_TYPE_LTE_INSTALL_REJECT = 14,   /*!< LTE devices. Reject installation of new FW package */
    OMA_TYPE_LTE_ENABLE     = 15,       /*!< LTE devices. Enable FOTA */
    OMA_TYPE_LTE_DISABLE     = 16,      /*!< LTE devices. Disable FOTA */
    OMA_TYPE_LTE_START      = 17,       /*!< LTE devices. Initiates FOTA */
    OMA_TYPE_LTE_CANCEL     = 18,       /*!< LTE devices. Cancels FOTA */
    OMA_TYPE_LTE_GET_PREF   = 19,       /*!< LTE devices. Get FOTA Preferences */
    OMA_TYPE_LTE_ENABLE_AUTO_DL = 20,   /*!< LTE devices. Enable auto firmware download */
    OMA_TYPE_LTE_DISABLE_AUTO_DL = 21,  /*!< LTE devices. Disable auto firmware download */
    OMA_TYPE_LTE_ENABLE_AUTO_UPDATE = 22, /*!< LTE devices. Enable auto firmware update */
    OMA_TYPE_LTE_DISABLE_AUTO_UPDATE = 23,/*!< LTE devices. Disable auto firmware update */
    OMA_TYPE_LTE_GET_OTADM_STATE = 24,    /*!< LTE devices. Get OTADM state */
	OMA_TYPE_LTE_PROGRESS_PAUSE = 25,     /*!< LTE devices. Pause OTADM progress */
	OMA_TYPE_LTE_PROGRESS_CANCEL = 26,    /*!< LTE devices. Cancel OTADM progress */
	OMA_TYPE_LTE_PROGRESS_RESUME = 27,    /*!< LTE devices. Resume OTADM progress */
	OMA_TYPE_LTE_UCS_OK = 28,             /*!< LTE devices. Accept UCS. */
	OMA_TYPE_LTE_UCS_CANCEL = 29,         /*!< LTE devices. Cancel UCS. */
	OMA_TYPE_LTE_NETERR_OK = 30,          /*!< LTE devices. */
	OMA_TYPE_LTE_NETERR_CANCEL = 31,      /*!< LTE devices. */
	OMA_TYPE_LTE_DMALERT_OK = 32,         /*!< LTE devices. */
	OMA_TYPE_LTE_DMALERT_CANCEL = 33,     /*!< LTE devices. */
	OMA_TYPE_LTE_NET_SIG_OK = 34,         /*!< LTE devices. */
    OMA_TYPE_LTE_DLDD_DEFER = 35,         /*!< LTE devices. Defer firmware download */
	OMA_TYPE_LTE_UTS_DEFER = 36,          /*!< LTE devices. Defer firmware install */
}OmaType;


/// \enum OmaState
/// This enum defines values for the state of OMADM subsystem.
typedef enum 
{
	OMA_DM_DISABLED 				= 0x00, /*!< OMA-DM based device activation is disabled */
	OMA_PRL_DISABLED				= 0x01, /*!< OMA-DM based PRL update is disabled */
	OMA_PRL_DM_DISABLED 			= 0x02, /*!< <tt><b>(not used)</b></tt> */
	OMA_IDLE						= 0x03, /*!< OMA-DM subsystem is waiting for command from the network or client application */
	HFA_IN_PROGRESS 				= 0x04,	/*!< HFA has been started and is in progress */
	HFA_IN_RETRY					= 0x05,	/*!< HFA is in retry. The current attempt has failed */
	HFA_IDLE						= 0x06,	/*!< HFA session has completed. */
	CIDC_IN_PROGRESS				= 0x07,	/*!< OMA-DM based Device Activation session is in progress- \e Client Initiated */
	NIDC_IN_PROGRESS				= 0x08, /*!< OMA-DM based Device Activation session is in progress- \e Network Initiated */
	CIPRL_IN_PROGRESS				= 0x09, /*!< OMA-DM based PRL update session is in progress- \e Client Initiated */
	NIPRL_IN_PROGRESS				= 0x0A,	/*!< OMA-DM based PRL update session is in progress- \e Network Initiated */
	ABORT_IN_PROGRESS				= 0x0B,	/*!< Current OMA-DM session is being aborted */
	
	OMA_NULL_STATE					= 0x0C,	/*!< Place holder */
	OMA_FUMO_DISABLED				= 0x0D,	/*!< OMA-DM based firmware update is disabled */ 
	FUMO_IN_PROGRESS				= 0x0E, /*!< OMA-DM based firmware update session is in progress*/
	FUMO_IDLE                       = 0x0F, /*!< FUMO session is complete. */
	
	OMA_STATE_ENUM_MAX				= 0x0F,
}OmaState;

/// \enum OmaStatus
///This enum defines values for OMA-DM specific error codes. 
typedef enum
{
	OMA_SUCCESS 					= 0x00, /*!< Operation succeeded */
	OMA_ERR_NO_REASON				= 0x01, /*!< Unknown error code */
	OMA_ERR_PROFILE_UNAVAILABLE 	= 0x02, /*!< There is no data available in Profile 0 */
	OMA_ERR_INVALID_CREDENTIALS 	= 0x03,	/*!< The server rejected the user credentials for this OMA-DM session */
	OMA_ERR_SERVER_UNREACHABLE		= 0x04,	/*!< The device cannot find the server */
	OMA_ERR_NETWORK_UNAVAILABLE 	= 0x05,	/*!< No network is available */
	OMA_ERR_DM_DISABLED 			= 0x06,	/*!< OMA-DM based device activation is disabled */
	OMA_ERR_PRL_DISABLED			= 0x07,	/*!< OMA-DM based PRL update is disabled */
	OMA_ERR_PRL_DM_DISABLED 		= 0x08, /*!< <tt><b>(not used)</b></tt> */
	OMA_ERR_FEATURE_DM_DISABLED 	= 0x09,	/*!< The device is not capable of OMA-DM based activation */
	OMA_ERR_FEATURE_PRL_DISABLED	= 0x0A,	/*!< The device is not capable of OMA-DM based PRL update */
	
	OMA_NULL_STATUS 				= 0x0B,	/*!< Place holder */
	OMA_ERR_FEATURE_FUMO_DISABLED	= 0x0C,	/*!< OMA-DM based firmware update is disabled */
	OMA_ERR_FUMO_UPDATE_FAILED		= 0x0D,	/*!< OMA-DM based firmware update failed */
	OMA_ERR_FUMO_UPDATE_PACKAGE		= 0x0E,	/*!< OMA-DM based firmware update package is corrupt */
	
	OMA_STATUS_ENUM_MAX 			= 0x0E,
}OmaStatus;

/// \enum OmaExtStatus
///This enum defines values for OMA-DM specific extended status codes
typedef enum
{
	OMA_ERR_EXT_PLACE_HOLDER		= 0x0000,	/*!< Place Holder */
	HFA_HEARTBEAT					= 0x0001,	/*!< HFA (Hands Free Activation) is in progress and is in-between retires. This is valid when "state" is HFA_IN_RETRY. "data" field of OmaStatusEvent structure contains the number of secs left for next attempt. */
	HFA_RETRIES_LEFT				= 0x0002,	/*!< HFA (Hands Free Activation) session has failed but there are retries left. This is valid when "state" is HFA_IN_PROGRESS. "data" field of OmaStatusEvent structure contains the number of retries left. */
	HFA_NOT_ALLOWED 				= 0x0003,	/*!< HFA (Hands Free Activation) session has failed and there are no retries left */
	OMA_EVENT_CB_FUNCTION			= 0x0004,	/*!< <tt><b>(reserved internal use)</b></tt> */
	OMA_EVENT_NI_REQUESTED			= 0x0005,	/*!< A network-initiated OMA-DM event has occured */
	OMA_FUMO_UPDL_IN_PROGRESS		= 0x0006,	/*!< Downloading firmware update package from OMADM server */
	OMA_FUMO_UPDL_TOTAL_SIZE		= 0x0007,	/*!< Firmware update package size to be downloaded */
	OMA_FUMO_UPDATE_PACKAGE_AVAILABLE		= 0x0008,	/*!< New firmware update package is available */
	OMA_FUMO_UPDATE_PACKAGE_NOT_AVAILABLE	= 0x0009,	/*!< Firmware update package is not available */
	FUMO_RESET_AFTER_GET_UDP		= 0x000A,	/*!< Resetting modem after firmware update package download complete */
	FUMO_RESET_AFTER_PROGRAMMING	= 0x000B,	/*!< Resetting modem after programming the update package to flash successfully */
	FUMO_NIFUMO_WAP_PUSH			= 0x000C,	/*!< Received NIFUMO ( network initiated FUMO) wap push from server*/
	FUMO_REPORT_UPDATE_STATUS		= 0x000D,	/*!< Reporting firmware update status to server */
	OMA_EVENT_OMADM_ATCMD_CIDC      = 0x000E,   /*!< reserved */
    OMA_EVENT_OMADM_ATCMD_CIPRL     = 0x000F,   /*!< reserved */
    OMA_EVENT_OMADM_ATCMD_CIFUMO    = 0x0010,   /*!< reserved */
    OMA_DD_UPDATE                   = 0x0011,   /*!< OMA descriptor file downloaded */
    OMA_DL_PKG_RECEIVED             = 0x0012,   /*!< OMA package downloaded */
    FUMO_REPORT_DLDD_DEFFER         = 0x000E,
    FUMO_REPORT_UTS_DEFFER          = 0x000F,
    OMA_AUTOMATIC_DOWNLOAD_ENABLE   = 0x0011,
    OMA_AUTOMATIC_DOWNLOAD_DISABLE  = 0x0012, 
    OMA_AUTOMATIC_FW_UPDATE_ENABLE  = 0x0014,
    OMA_AUTOMATIC_FW_UPDATE_DISABLE = 0x0018,
    OMA_FUMO_DLDD	                = 0x0020,
    OMA_FUMO_UTS	                = 0x0021,
    OMA_DMALERT	                    = 0x0022, 
    OMA_RECOVERY_IN_PROGRESS        = 0x0023,
    OMA_FUMO_DLDD_DEFFER            = 0x0024,
    OMA_FUMO_UTS_DEFFER             = 0x0025,
	OMA_ERR_EXT_NULL				= 0xFFFF,	/*!< Place holder */
}OmaExtStatus;

/// \enum OmaEvent
/// This enum defines events returned by OMA-DM subsystem during an activation or PRL update session. 
typedef enum
{
	OMAEVENT_PRLMaxPRLSizeGetFunc,					/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_PRLMaxPRLSizeReplaceFunc,				/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_PRLPrefRoamListGetFunc,				/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_PRLPrefRoamListReplaceFunc,			/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_PRLPRLIDGetFunc,						/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_PRLPRLIDReplaceFunc,					/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_NAMCdmaNamGetFunc,						/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_NAMCdmaNamReplaceFunc,					/*!< The NAM has been replaced and the device shall need a reset once the OMA-DM session is complete */
	OMAEVENT_NAMMobDirNumGetFunc,					/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_NAMMobDirNumReplaceFunc,				/*!< The mobile directory number has been replaced and the device shall need a reset once the OMA-DM session is complete */
	OMAEVENT_Profile1AuthAlgoAAAGetFunc,			/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1AuthAlgoAAAReplaceFunc,		/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1AuthAlgoHAGetFunc,				/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1AuthAlgoHAReplaceFunc,			/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1MobileIpAddressGetFunc,		/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1MobileIpAddressReplaceFunc,	/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1NAIGetFunc,					/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1NAIReplaceFunc,				/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1PasswordAAAGetFunc,			/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1PasswordAAAReplaceFunc,		/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1PasswordHAGetFunc,				/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1PasswordHAReplaceFunc,			/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1PriHAIpGetFunc,				/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1PriHAIpReplaceFunc,			/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1ReverseTunnelingGetFunc,		/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1ReverseTunnelingReplaceFunc, 	/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1SecHAIpGetFunc,				/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1SecHAIpReplaceFunc,			/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1SpiAAAGetFunc,					/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1SpiAAAReplaceFunc,				/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1SpiHAGetFunc,					/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_Profile1SpiHAReplaceFunc,				/*!< <tt><b>(reserved for internal use)</b></tt> */
	OMAEVENT_CBFuntion_MAX							/*!< Place holder */
}OmaEvent;

/// Defines an API structure used to convey OMA-DM status information.
typedef struct
{
    uint8_t    state;     /*!< TODO oma state  */
    uint8_t    status;    /*!< TODO oma status */
    uint16_t   extended;  /*!< TODO oma exteneded value */
    uint32_t    data;      /*!< TODO oma data */
}OmaStatusEvent;

/// \enum LteOmaModule
/// This enum defines the module type that fired an LTE-based OMA related event. 
typedef enum{
    LTE_MODULE_OMA = 0,
    LTE_MODULE_HFA = 1,
    LTE_MODULE_SDM = 2,
}LteOmaModule;

/// \enum LteOmaInitiator
/// This enum defines the source that caused an LTE-based OMA related event to be fired.
typedef enum{
    LTE_INITIATOR_SERVER = 1,
    LTE_INITIATOR_DEVICE = 2,
    LTE_INITIATOR_USER = 3,
    LTE_INITIATOR_DMCANCEL = 4,
    LTE_INITIATOR_RECOVER = 5,
}LteOmaInitiator;

/// \enum LteOmaStatus
/// This enum defines the type of LTE-based OMA event that was fired.
typedef enum{
  LTE_OMA_INITDM = 100, 
  LTE_OMA_DLDD, 
  LTE_OMA_UTS, 
  LTE_OMA_PROGRESS, //103
  LTE_OMA_UCS, 
  LTE_OMA_NETERR, 
  LTE_OMA_DT_RESUME_DT, 
  LTE_OMA_DL_RESUME_DL, 
  LTE_OMA_UT_RESUME_UT, //108
  LTE_OMA_UCS_RESUME_UCS, 
  LTE_OMA_DMALERT,
  LTE_OMA_NET_NOSIG,
  LTE_OMA_NOT_AVAILABLE_PKG,//112
  LTE_OMA_DMALERT_COMPLETE,
  LTE_OMA_NULL,
}LteOmaStatus;


/// Defines an result codes contianed in the payload for LTE-based OMA events.
typedef enum {
    IPTH_FUMO_SUCCESS = 200,
    IPTH_FUMO_CLIENT_ERROR = 400,
    IPTH_FUMO_USER_CANCEL,
    IPTH_FUMO_PACKAGE_CORRUPTED, 
    IPTH_FUMO_PACKAGE_MISMATCH ,
    IPTH_FUMO_PACKAGE_INVALID ,
    IPTH_FUMO_PACKAGE_UNACCEPTABLE ,
    IPTH_FUMO_DOWNLOAD_AUTHENTICATION_FAILED,
    IPTH_FUMO_DOWNLOAD_TIMEOUT,
    IPTH_FUMO_NOT_IMPLEMENTED,
    IPTH_FUMO_UNDEFINED_ERROR,
    IPTH_FUMO_INSTALL_FAILED,
    IPTH_FUMO_BAD_URL,
    IPTH_FUMO_DOWNLOAD_SERVER_UNAVAILABLE,
    IPTH_FUMO_DOWNLOAD_SERVER_ERROR = 500,
    IPTH_FUMO_DOWNLOAD_OUT_OF_MEMORY,
    IPTH_FUMO_INSTALL_OUT_OF_MEMORY,
    IPTH_FUMO_NETWORK_ERR,
    IPTH_FUMO_DOWNLOAD_ERR
}LteOmaFumoResultCodes;


/// Defines an API structure used to convey LTE-based OMA status information.
typedef struct 
{
   uint8_t  module;       /*!< module as defined by LteOmaModule type */
   uint8_t  initiator;    /*!< initiator as defined by LteOmaInitiator */
   uint8_t status;       /*!< type of event as defined by LteOmaStatus */
   uint16_t extended;     /*!< extended status as defined by OmaExtStatus type */
   uint32_t payload;      /*!< possible 32-bit payload of the event */
}LteOmaStatusEvent;




/// Defines an API structure used when querying the OMA capabilities of a device.
typedef struct
{
    uint8_t    dm_capable;        /*!< oma dm capable flag  */
    uint8_t    dm_enabled;        /*!< oma dm enabled flag  */
    uint8_t    prl_capable;       /*!< oma prl capable flag */
    uint8_t    prl_enabled;       /*!< oma prl enabled flag */
}OmaCapabilityStruct;



#pragma pack(push, 4)
//
//The following definitions are not packed.
//


/// Defines an enum used to define OMA/FOTA update severity
typedef enum FOTASeverityEnum
{
	FOTASeverity_unknown, 
	FOTASeverity_Critical, 
	FOTASeverity_High, 
	FOTASeverity_Medium, 
	FOTASeverity_Low
} FOTASeverityEnum;

#define NW_OMA_MAX_URI_SIZE     128
#define NW_OMA_SHORT_STR_SZ		40
#define NW_OMA_VERY_LONG_STR_SZ	255

/// Defines the structure that describes an available OMA firmware update
typedef struct {
   char objectURI[NW_OMA_MAX_URI_SIZE+1];        /* Identifies object location */
   int32_t  size;					/* Identifies object size */
   char type[NW_OMA_SHORT_STR_SZ+1];
   char name[NW_OMA_SHORT_STR_SZ+1];
   char vendor[NW_OMA_SHORT_STR_SZ+1];		/* Identifies vendor name */
   char description[NW_OMA_VERY_LONG_STR_SZ+1];	/* Identifies object description */
   char installNotifyURL[NW_OMA_MAX_URI_SIZE+1]; /* Identifies notifying location */
   char nextURL[NW_OMA_MAX_URI_SIZE+1];          /* Identifies next URL */
   char infoURL[NW_OMA_MAX_URI_SIZE+1];          /* Identifies info location */
   char iconURL[NW_OMA_MAX_URI_SIZE+1];          /* Identifies icon location  */
   char installParam[NW_OMA_VERY_LONG_STR_SZ+1];	/* Install parameters */
   char crc[32];				/* Update package CRC */
   int32_t update_time;	/* Time takes to update the firmware ,count in seconds*/
   int32_t download_time;	/* Time takes to download update pkg ,count in seconds*/
   FOTASeverityEnum eSeverity;
} OmaDdStructType;

/// Defines the structure that describes an available LTE-Based OMA firmware update
typedef struct
{
	char name[32];          /* Download descriptor name */
	char ddVersion[32];     /* Download descriptor version */
	char objectURI[128];    /* URI of the object */
	char mediaType[32];     /* Media type */
	char vendor[32];        /* Vendor*/
	char description[128];  /* Informative description */
	char installNotifyURI[128]; /* URI to be notified when installed */
	char nextURI[128];      /* Next URI */
	char infoURI[128];      /* Info URI */
	char iconURI[128];      /* URI for retrieving icon */
	char installParam[128]; /* Install parameters */
	int32_t  objectSize;        /* Size (in bytes) of the object to be downloaded */
	int32_t	 downloadTime;       /* Estimatd download time in seconds*/
	int32_t  updateTime;         /* Estimated install time in seconds*/
	int32_t  severity;           /* The severity of the package to be downloaded */
} OmaLteDownloadDescriptor;


/// Defines a structure used to provide WIMAX network frequency channel plan information
struct ChannelPlan
{
    int32_t freq_min_mhz;              /*!< Min frequency */
    int32_t freq_max_mhz;              /*!< Max frequency */
    int32_t freq_step_khz;             /*!< Step increments in khz */
};

/// Defines a structure used to provide WIMAX specific activation information
typedef struct 
{
    int32_t version;               /*!< Version of the WimaxOMAParameters struct */
    char userIdentity[128];     /*!< User id */
    char password[128];         /*!< Password */
    int32_t workmode;              /*!< Work mode indicator */
    int32_t nap_id;                /*!< Network Access Provider identifier */
    struct ChannelPlan cp;      /*!< Channel plan details */
    int32_t session_continuity;     /*!< Session Continuity */
   	int32_t scan_attempt_timeout;   /*!< Scan Attempt Timeout */
   	int32_t scan_retries;           /*!< Retry count */
   	int32_t idle_sleep;             /*!< Idle Sleep */
   	int32_t entry_rx;               /*!< Entry Rx */
   	int32_t entry_cinr;             /*!< Entry CINR */
   	int32_t entry_delay;            /*!< Entry Delay */
   	int32_t exit_cinr;              /*!< Exit CINR */
   	int32_t exit_delay;             /*!< Exit Delay */
} WimaxOMAParameters;
#pragma pack(pop)


/// \enum ActivationType
/// <tt><b>\<CDMA/EVDO devices only\></b></tt> \n
/// This enum defines the possible activation types that can be performed.
typedef enum
{
	NW_ACTIVATE_AUTOMATIC		= 0,    /*!< Activation type Automatic. Attempts to determine the appropriate activation type*/
	NW_ACTIVATE_MANUAL			= 1,    /*!< Activation type manual. Device is reset upon successfull activation */
	NW_ACTIVATE_MANUAL_NORESET	= 2,    /*!< Activation type manual, but device is not reset. This is useful when just a single use activation code needs to be validated 
											    or when other methods need to be called before the device is reset. */
	NW_ACTIVATE_MANUAL_NOACTIVATION_CODE = 3, /*!< Just activation information is being provided without the activation code. The activation code was provided earlier in the process. */
	NW_ACTIVATE_OVERRIDE_IOTA	= 4,	/*!< Activation overridden to perform an IOTA activation. Used to force an IOTA activation. No other activation-type checks will be executed. */
	NW_ACTIVATE_OVERRIDE_OTASP	= 5,	/*!< Activation overridden to perform an OTASP activation. Used to force an OTASP activation. No other activation-type checks will be executed. */
} ActivationType;


/// \enum ActivationStateType
/// This enum defines values for the status of the activation process.
typedef enum
{
	NW_ACT_STATUS_UNKNOWN				= 0,	/*!< Status unknown */
	NW_ACT_STATUS_PRGMING_IN_PROGRESS	= 1,	/*!< Activation has begun and device programming is in progress */
	NW_ACT_STATUS_SPL_UNLOCK_OK			= 2,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_SPL_UNLOCK_FAILED		= 3,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_DLOAD_NAM_OK			= 4,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_DLOAD_NAM_FAILED		= 5,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_DLOAD_MDN_OK			= 6,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_DLOAD_MDN_FAILED		= 7,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_DLOAD_IMSI_OK			= 8,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_DLOAD_IMSI_FAILED		= 9,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_DLOAD_PRL_OK			= 10,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_DLOAD_PRL_FAILED		= 11,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_COMMIT_SUCCESSFUL		= 12,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_COMMIT_FAILED			= 13,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_ACTIVATION_SUCCESSFUL	= 14,	/*!< Activation was successfull */
	NW_ACT_STATUS_ACTIVATION_FAILED		= 15,	/*!< Activation Failed */
	NW_ACT_STATUS_WAP_PUSH_IN_PROGRESS	= 16,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_RETRY_ATTEMPT			= 17,	/*!< <tt><b>(reserved)</b></tt> */
	NW_ACT_STATUS_COUNT							/*!< Place Holder */
} ActivationStateType;

/// Defines a structure used to provide activation information
typedef struct
{
    uint32_t           dwSize;                     /*!< size of the ActivationInfoStruct */
    ActivationType          eType;                      /*!< Type of activation to perform */
    uint8_t           bAsynchronousActivation;    /*!< Asynchronous flag */
    char                    szMDN[NW_SIZE_NUMBER];      /*!< The MDN to activate with (manual only) */	
    char                    szMIN[NW_SIZE_NUMBER];      /*!< The MIN to activate with (manual only) */	
    char                    szActivationCode[ NW_ACTIVATION_CODE_SIZE ]; /*!< The service programming code for activation */
    uint16_t          dwHomeSID;                  /*!< The Home SID to set (manual only) */
    uint32_t           dwPRLDataSize;              /*!< The size of the PRL data being provided (manual only)*/
    uint8_t           *pPRLData;                  /*!< Pointer to a buffer containing the PRL data to apply */
} ActivationInfoStruct;

/*******************************************************************
END EVDO activation related types
********************************************************************/

/*******************************************************************
GSM related types
********************************************************************/
/// \enum CopsModeType
/// <tt><b><UMTS and HSDPA devices only\></b></tt>\n
///This enum defines values for the mode parameter as used with #NvtlGsm_SetNetworkOperator method.
///It denotes the mode for the command AT+COPS that is used internally.
typedef enum
{
	COPS_MODE_AUTOMATIC	= 0,	/*!< Automatically select network operator */
	COPS_MODE_MANUAL	= 1,	/*!< Manually select network operator */
	COPS_MODE_DEREGISTER = 2,	/*!< Deregistration from the network */
	COPS_MODE_SET_FORMAT = 3,	/*!< Set \ref _at_cops_format_e_type "format" for read command */
}CopsModeType;

/// \enum CopsFormatType
/// <tt><b><UMTS and HSDPA devices only\></b></tt>\n
///This enum defines values for the format parameter as used with #NvtlGsm_SetNetworkOperator method.
///It denotes the format for the command AT+COPS that is used internally.
typedef enum
{
	COPS_FORMAT_LONG_ALPHANUMERIC	= 0, /*!< Long alphanumeric format. For e.g. "Orange F" */
	COPS_FORMAT_SHORT_ALPHANUMERIC	= 1, /*!< Short alphanumeric format. For e.g. "Cingular" */
	COPS_FORMAT_NUMERIC				= 2, /*!< Numeric format. For e.g. "310380", for Cingular */
}CopsFormatType;

/// \enum AccessTechType
/// <tt><b><UMTS and HSDPA devices only\></b></tt>\n
///This enum defines values for the access technology parameter as used with the NvtlGsm_SetNetworkOperator method.
///It is used internally with AT+COPS command.
typedef enum
{
	ACCESS_TECH_GSM			= 0,	/*!< GSM */
	ACCESS_TECH_GSM_COMPACT	= 1,	/*!< GSM Compact */
	ACCESS_TECH_UTRAN		= 2,	/*!< UMTS Terrestrial Radio Access Network */
	ACCESS_TECH_AUTOMATIC	= 3,	/*!< Automatic selection */
	ACCESS_TECH_NA			= 4,	/*!< Place Holder */
}AccessTechType;

/// \enum CopsNetworkStatusType
/// <tt><b><UMTS and HSDPA devices only\></b></tt>\n
///This enum defines values for the network status types as returned from the AT+COPS data returned from the GetNetworkOperatorList method.
typedef enum 
{
	COPS_NETWORK_STATUS_UNKNOWN		= 0,
	COPS_NETWORK_STATUS_AVAIAILABLE	= 1,
	COPS_NETWORK_STATUS_CURRENT		= 2,
	COPS_NETWORK_STATUS_FORBIDDEN	= 3
} CopsNetworkStatusType;
/*******************************************************************
END GSM related types
*******************************************************************/
/*******************************************************************
Address Book related types
********************************************************************/

/// Defines an API structure used when setting or retrieving a contact from the address book.
typedef struct
{   
	uint32_t        dwIndex;                            /*!< Index of the contact in the address book */
	char	        szContactName	[ NW_SIZE_NAME ];   /*!< Name of the contact */
	char          szContactDetails[ NW_SIZE_NAME ];   /*!< Phone number of the contact */
} ContactInfoStruct;

/// Defines an API structure used when querying statistics about the address book.
typedef struct
{
	uint32_t	dwMinIndex;             /*!< The mininum valid index in the address book */
	uint32_t	dwMaxIndex;             /*!< The maximum valid index in the address book */
	uint32_t	dwContactNameMax;       /*!< The maximum allowed characters for the name of a contact */
	uint32_t	dwContactDetailsMax;    /*!< The maximum allowed characters for the detail of a contact */
} ContactSizeInfoStruct;
/*******************************************************************
END Address Book related types
********************************************************************/


/*******************************************************************
SMS related types
********************************************************************/
/// \enum SMSMessageState
/// defines general sms message states
typedef enum
{	
    SMS_STATE_EMPTY	= 60000,        /*!< NULL sms state */
	SMS_STATE_UNREAD,               /*!< sms is unread */
	SMS_STATE_UNREAD_PRIORITY,      /*!< sms is an unread urgent message */
	SMS_STATE_READ,                 /*!< sms is read */
	SMS_STATE_FORWARDED,            /*!< sms is forwarded */
	SMS_STATE_REPLIED,              /*!< sms is replied */
	SMS_STATE_SENDING,              /*!< sms is sending */
	SMS_STATE_SENT,                 /*!< sms is sent */
	SMS_STATE_DELIVERED,            /*!< sms is delivered */    
	SMS_STATE_FAILED_SEND           /*!< sms failed to send*/
}SMSMessageState;

/// \enum SMSBoxEnum
/// Not Used.
typedef enum
{
    SMSInbox	= 0,        /*!< sms inbox   */
	SMSOutbox,              /*!< sms outbox  */
	SMSSentbox              /*!< sms sentbox */
}SMSBoxEnum;

/// \enum SmsStorageType
/// Not used
typedef enum 
{
    SmsStorageHOST	= 0,    /*!< sms messages are stored on the host */
	SmsStorageSM,           /*!< sms messages are stored on the SIM (HSPA only) */
}SmsStorageType;


/// \enum SmsStorageTypeEx
/// Defines the available sms storage types.  Available only for multimode devices
typedef enum 
{
    SmsStorageUnknown       = 0,    /*!< Unknown storage area */
    SmsStorageSim3GPP       = 2,    /*!< SIM based message store for 3GPP (WCDMA) messages */
    SmsStorageDevice3GPP    = 3,    /*!< Device based message store for 3GPP (WCDMA) messages */
    SmsStorageDevice3GPP2   = 6,    /*!< Device based message store for 3GPP2 (CDMA) messages */
}SmsStorageTypeEx;


/// \enum SmsStateType
/// This enum defines values for the possible states of an sms message.  Used when querying for a list of sms messages
typedef enum
{	
	NW_SMS_RECEIVED_UNREAD		= 0,  /*!< New unread message */
	NW_SMS_RECEIVED_READ		= 1,  /*!< Previously read message */
	NW_SMS_SENT_NOT_DELIVERED	= 2,  /*!< Sent but unconfirmed message */
	NW_SMS_SENT					= 3,  /*!< Set messsage */
	NW_SMS_ALL					= 4,  /*!< ANY message stat*/ 
	NW_SMS_STATUS_UNKNOWN		= 5   /*!< Unknown sms message state*/
}SmsStateType;

/// \enum SmsSendStatusType
/// This enum defines the possible status values that can be received when sending an sms mesage.=
typedef enum
{
    NW_SMS_SEND_SUCCESS     = 0,    /*!< Sms sent okay */
    NW_SMS_SEND_NO_SERVICE,         /*!< Sms send failed due to lack of service */
    NW_SMS_SEND_INTERNAL_ERROR,     /*!< Sms send failed due to some other error */
    NW_SMS_MO_DISABLED = 200,        /*!< Sms send is disabled on the device */
}SmsSendStatusType;

/// Provides information about the voice mail indicator
typedef struct
{
	uint8_t   bVoiceMailActive;   /*!< Voicemail indicator is active */
	uint32_t   dwVoiceMailIndex;   /*!< sms index of voicemail indicator */
} VoiceMailInfoStruct;

/// \enum SmsMessageType 
/// Indicates the type of message that is encoded/present in the sms message struct.
typedef enum
{
	NW_SMS_TYPE_UNKNOWN		= 0,    /*!< Unknown encoding */
	NW_SMS_TYPE_WMS			= 1,    /*!< WMS encoding */
	NW_SMS_TYPE_IS683		= 2,    /*!< IS683 encoding - (Deprecated, use NW_SMS_TYPE_IS637)*/
	NW_SMS_TYPE_IS637		= 2,    /*!< IS637 encoding */
	NW_SMS_TYPE_PDU			= 3,    /*!< GSM PDU encoding */
} SmsMessageType;

/// Defines a structure used when retrieving or sending an Sms message
typedef struct
{
	uint16_t      eSMSType;   /*!< The Sms encoding type*/
	uint16_t      eState;     /*!< The state of the sms message */
	uint16_t      index;      /*!< The index of the sms message in the sms list*/
	uint16_t      data_len;   /*!< The length of the encoded data*/
	uint8_t       data[500];  /*!< A pointer to a buffer that contains/receives the encoded data */
} SmsMessageStruct;

/// Defines the state of an sms msg at the corresponding index of the list of sms messages
typedef struct
{
	uint16_t	index;    /*!< The index of the sms message in the sms list */
	uint16_t	eState;   /*!< The state of the sms message at the index */
} SmsMessageInfo;

/*******************************************************************
END SMS related types
********************************************************************/


/*******************************************************************
GPS related types
********************************************************************/

/// \enum GpsModeType
/// This enum defines the possible modes of the GPS engine
typedef enum
{
	NW_GPS_NOGPS				= 0x00,		/*!< No GPS Support */
	NW_GPS_STANDALONE			= 0x01,	    /*!< Standalone */
	NW_GPS_MS_BASED				= 0x02,	    /*!< MS Based */
	NW_GPS_MS_ASSISTED			= 0x04,	    /*!< MS Assisted */
	NW_GPS_INTERNET_ASSISTED	= 0x08,		/*!< Standalone mode with internet assistance for retrieving satellite info quickly*/
} GpsModeType;

/// \enum GpsSessionType
/// This enum defines the types of sessions that can be used when receiving fixes.
typedef enum
{
	NW_GPS_LAST_FIX                 = 0,    /*!< Last fix */
	NW_GPS_NEW_FIX                  = 1,    /*!< 1 new fix */
	NW_CGPS_TRACK_INDEPENDENT_FIX	= 2,		
	NW_GPS_CONTINUOUS_FIX           = 3,    /*!< Continuous fix */
	NW_GPS_CONSTANT_CONTINUOUS_FIX  = 4,    /*!< Continuous fix*/
} GpsSessionType;

//\enum GpsXtraConfigType
//XTRA MODE ENUM
typedef enum
{
	NW_GPS_XTRA_OFF = 0,
	NW_GPS_XTRA_CLIENT_BASED = 1,
	NW_GPS_XTRA_EMBEDDED_CLIENT=2,
	NW_GPS_XTRA_MODE_MAX=3
}GpsXtraConfigType;

/// \enum GpsEventType
/// This enum defines the possible types of GPS Event
typedef enum
{
    NW_GPS_INVALID_FIX          = 0x00,     /*!< Too many invalid fixes received consecutively */
	NW_GPS_FIX_RECEIVED         = 0x01,     /*!< GPS Fix Received */ 
    NW_GPS_STARTED				= 0x02,		/*!< GPS Engine Started */
	NW_GPS_STOPPED				= 0x03,	    /*!< GPS Engine Stopped */
	NW_GPS_MSBASED_FAILURE		= 0x04,		/*!< MS-Based GPS Failed. May fallback to Standalone mode (available only in smart mode)*/
	NW_GPS_SUBSCRIPTION_FAILED	= 0x05,		/*!< 30 day subscription requirement failed. Valid only for Standalone mode */
	NW_GPS_STANDALONE_FALLBACK	= 0x06,		/*!< Fallback to Standalone mode (available only in smart mode)*/
    NW_GPS_SESSION_END          = 0x07,     /*!< A Gps pd session ended */
    NW_GPS_SESSION_TERMINATED   = 0x08,     /*!< Gps session terminated by user */
} GpsEventType;

/// \def MAX_PRN
/// Defines the maximum number of satellite
#define MAX_PRN 37
#define MAX_ACTIVE_PRN 12

/// Defines an API structure used to obtain information about a single GPS fix
typedef struct
{
	double	        latitude;                           /*!< latitude */
	double	        longitude;                          /*!< longitude */
	int32_t         altitude;                           /*!< altitude */
	double	        horizontal_velocity;                /*!< horizontal velocity */
	double	        vertical_velocity;                  /*!< vertical velocity */
	double	        heading;                            /*!< heading */
	double	        angle_uncertainty;                  /*!< angle_uncertainty */
	double	        std_dev_uncertainty;                /*!< standard deviation uncertainty */
	double	        perpendicular_std_dev_uncertainty;  /*!< perpendicular standard deviation uncertainty */
	double	        vertical_std_dev_uncertainty;       /*!< vertical standard deviation uncertainty */
	int32_t         fix_type;                           /*!< type of fix */
	uint32_t        timestamp;                          /*!< timestamp of fix */
} GpsFixInfoStruct;

/// Defines an API structure used to obtain information about a single GPS fix along with active satellite information
typedef struct
{
   double           latitude;       /*!< latitude */
   double           longitude;      /*!< longitude */
   uint8_t          fix_quality;    /*!< Fix quality UNKNOWN=0, GPS=1, DGPS=2 */
   uint16_t         alt_ellipsoid;  /*!< height above WGS84 ellipsoid. unit is in x10 meters*/
   uint32_t         timestamp;      /*!< timestamp of fix */
   uint8_t          num_locked_prn;  /*!< number of satellites to to obtain fix */
   uint8_t          locked_prn[MAX_ACTIVE_PRN]; /*!< PRN num used for position */ 
   uint32_t         h_dop;          /*!< [x10], range is 1 to 50(lowest accuracy)*/
   uint32_t         p_dop;          /*!< [x10], range is 1 to 50(lowest accuracy)*/
   uint32_t         v_dop;          /*!< [x10], range is 1 to 50(lowest accuracy)*/
   uint8_t          fix_type;       /*!< type of Fix UKNOWN = 0, 2D = 1, 3D = 2 */
   uint32_t         speed_knot;     /*!< unit is in x10 knot*/
   int64_t          magnetic_var; /*!< unit is in x10 degrees */
   double	        heading;        /*!< heading */
} GpsFixInfoStructEx;

/// Defines an API structure that contains information about a single GPS satellite
typedef struct
{
	uint8_t     prn;              /*!< Pseudo Random Noise (PRN) number. This is used to identify the satellite */  
	uint8_t     elevation;        /*!< satellite elevation */  
	uint16_t    azimuth;         /*!< satellite azimuth*/    
	uint16_t    snr;             /*!< satellite signal to noise ratio (SNR) */
	uint8_t     location_valid;   /*!< satellite location is valid */
	uint8_t     snr_valid;        /*!< SNR value is valid */
}GpsSatelliteInfoStruct;

/// Defines an API structure that contains information about the GPS satellites
typedef struct
{
	int32_t num_svs;								/*!< Number of SVs (space vehicles) used to calculate position */
	int32_t num_svs_detected;						/*!< Total number of SVs detected */
	GpsSatelliteInfoStruct sv_info[MAX_PRN];	/*!< Information about each SV.  Indexed by Pseudo Random Noise (PRN) number */
}GpsSatelliteConstellationStruct;

/// \enum GpsSmartModeType
/// This enum defines the modes of GPS smart mode
typedef enum
{
	NW_GPS_SMARTMODE_OFF			= 0,	/*!< No Smart Mode */
	NW_GPS_SMARTMODE_MSBASED		= 1,	/*!< MS-based smart mode (falls back to standalone if MS-Based fails) */
	NW_GPS_SMARTMODE_STANDALONE		= 2,	/*!< Standalone smart mode (falls back to MS-based if Standalone fails) */
}GpsSmartModeType;

/// \enum NMEAOutputType
/// This enum defines the possible Formats of NMEA Output string
typedef enum
{
	NW_NMEA_OUTPUT_GPGGA	=	0x0001,     /*!< NMEA OUTPUT TYPE GPGGA */
	NW_NMEA_OUTPUT_GPGSA	=	0x0002,     /*!< NMEA OUTPUT TYPE GPGSA */
	NW_NMEA_OUTPUT_GPGSV	=	0x0004,     /*!< NMEA OUTPUT TYPE GPGSV */
	NW_NMEA_OUTPUT_GPVTG	=	0x0008,     /*!< NMEA OUTPUT TYPE GPVTG */
	NW_NMEA_OUTPUT_GPRMC	=	0x0010,     /*!< NMEA OUTPUT TYPE GPRMC */
} NMEAOutputType;

/// \enum PDSMSessionType
/// This enum defines the possible session types for the GPS engine
typedef enum
{
    NW_PDSM_SESSION_OPERATION_MIN = 0,         /*!< <tt><b>(reserved for internal use)</b><tt>*/
    NW_PDSM_SESSION_OPERATION_STANDALONE_ONLY, /*!< Standalone only mode */
    NW_PDSM_SESSION_OPERATION_MSBASED,         /*!< MS-based mode */
    NW_PDSM_SESSION_OPERATION_MSASSISTED,      /*!< MS-assisted mode */
    NW_PDSM_SESSION_OPERATION_OPTIMAL_SPEED,   /*!< Assisted optimal speed mode */
    NW_PDSM_SESSION_OPERATION_OPTIMAL_ACCURACY,/*!< Assisted optimal accuracy mode */
    NW_PDSM_SESSION_OPERATION_OPTIMAL_DATA,    /*!< Assisted optimal data mode */
    NW_PDSM_SESSION_OPERATION_REF_POSITION,    /*!< Assisted reference position mode */
}  PDSMSessionType;

/// Defines an API structure that contains information about the current GPS session
typedef struct
{
    PDSMSessionType mode;                      /*!< Operating Mode (set to 0xff if there is no ongoing GPS session or the session has been completed) */
    uint8_t         qosSessionTimeout;         /*!< Qos Session Timeout (set to 1 if there is no ongoing GPS session or the session has been completed) */
    uint32_t        qosAccuracyThreshold;      /*!< Qos Accuracy Threshold (set to 0xffffffff if there is no ongoing GPS session or the session has been completed) */
} PDSMSessionInfo;

/*******************************************************************
END GPS related types
********************************************************************/

/// Defines an API structure that contains basic information about an available WWAN device
typedef struct
{
	DeviceTechType            eTechnology;		         /*!< technology type of the device */
	DeviceFormFactorType      eFormFactor;				 /*!< physical form factor */
	char                      szDescription[NW_MAX_PATH];	 /*!< general name of the device */
    	char                      szPort[NW_MAX_PATH];         /*!< name of the modem port */
    	char                      szFriendlyName[NW_MAX_PATH]; /*!< friendly name of the modem port */
}DeviceDetail;

typedef struct
{
    DeviceTechType          eTechnology;	    		//technology type of the device
	DeviceFormFactorType    eFormFactor;
    uint16_t                vid;
    uint16_t                pid;
    uint64_t                sessionId;					// Session ID

    char                    szDescription[NW_MAX_PATH];
    char                    szModemPort[NW_MAX_PATH];
    char                    szStatusPort[NW_MAX_PATH];
    char                    szDiagPort[NW_MAX_PATH];
    char                    szGpsPort[NW_MAX_PATH];

#if defined(_NVTL_WINDOWS_)
    char                    szModemPortFriendlyName[NW_MAX_PATH];
    char                    szStatusPortFriendlyName[NW_MAX_PATH];
    char                    szDiagPortFriendlyName[NW_MAX_PATH];
    char                    szGpsPortFriendlyName[NW_MAX_PATH];
    
    int32_t                   iNdisIndex;
    char                    szPriHardwareId[NW_MAX_PATH];
    char                    szSecHardwareId[NW_MAX_PATH];
    char                    szNdisHardwareId[NW_MAX_PATH];
    char                    szDriverVersion[NW_MAX_PATH];
    char                    szStatusPortInstanceId[NW_MAX_PATH];

#elif DEFINED_MAC
    
    char                    szModemDialIn[NW_MAX_PATH];
    char                    szStatusDialIn[NW_MAX_PATH];
    char                    szDiagDialIn[NW_MAX_PATH];
    char                    szGpsDialIn[NW_MAX_PATH];
    uint8_t                   bRestricted;

#elif DEFINED_LINUX
	uint16_t			bus_num;
	uint16_t			dev_num;
    uint8_t             bRestricted;
#endif

}OSDeviceDetail;
/*******************************************************************
Basic SDK API related types
********************************************************************/

/// \enum NvtlSdkModeType
/// This enum defines the mode in which the SDK is being used
typedef enum 
{
	SdkModeLocal = 0,   /*!< The SDK is accessing the device COM port directlly */
	SdkModeShared,      /*!< The SDK is accessing the device through the SDK server to allow concurrent access */
}NvtlSdkModeType;

/// \enum PropertyAction
/// This enum defines the action types used when accessing and SDK property
typedef enum
{
	PropGet = 0,    /*!< The property is being read */
	PropSet = 1,    /*!< The property is being set */
    PROP_GET = 0,   /*!< The property is being read */
    PROP_SET = 1,   /*!< The property is being set */
}PropertyAction;

//Enumerates the possible events that can be fired by the SDK
/// \enum NvtlEventType
/// This enum defines the types of SDK events that can be received in an SDK callback handler
typedef enum
{
	NW_EVENT_SIG_STR = 0,           /*!< The signal strength has changed.  The handler event receives a pointer to a SigStrEvent struct */
	NW_EVENT_ROAMING,               /*!< The roaming status has changed.   The handler event receives a pointer to a RoamingEvent struct */
	NW_EVENT_DEVICE_STATE,          /*!< The device state has changed.   The handler event receives a pointer to a DeviceStateEvent struct */
    NW_EVENT_DORMANT,               /*!< The dormancy status has changed.  The handler event receives a pointer to a DormantEvent struct */
	NW_EVENT_NETWORK,               /*!< The network status has changed.   The handler event receives a pointer to a NetworkEvent struct */
	NW_EVENT_SERVER_ERROR,          /*!< The SDK received a device error.  The handler event receives a pointer to a ServerErrorEvent struct */
	NW_EVENT_POWER_SAVE_NOTIFY,     /*!< The device is entering low power mode.  */
	NW_EVENT_LOG_PACKET,            /*!< not for general use */
	NW_EVENT_DIAG_PACKET,           /*!< not for general use */
	NW_EVENT_UNSOLICITED_AT,        /*!< An unsolicited AT response was received.  The handler event receives a pointer to a UnsolicitedATEvent struct */
	NW_EVENT_SMS,                   /*!< A new sms message was received.  The handler event receives a pointer to a SmsEvent struct */
	NW_EVENT_INCOMING_CALL,         /*!< not used */
	NW_EVENT_DEVICE_ADDED,          /*!< A new device was recognized by the host.  The handler event receives a pointer to a DeviceAddedEvent struct */
	NW_EVENT_DEVICE_REMOVED,        /*!< A device was removed from the host.  The handler event receives a pointer to a DeviceRemovedEvent struct.
                                        This event is fired when ANY device removal is detected and may not pertain to the active device in use if multiple
                                        devices are recognized in the system.  To detect the removal of a device currently in use handle the NW_EVENT_DEVICE_ERROR event.*/
	NW_EVENT_ACTIVATION,            /*!< The SDK received an activation status update.  The handler event receives a pointer to a ActivationEvent struct */
    NW_EVENT_OMADM,                 /*!< The SDK received an OMA-DM status update. The handler event receives a pointer to a OmaStatusEvent struct */
    NW_EVENT_SMS_SEND_STATUS,       /*!< The SDK received an Sms send status update. The handler event receives a pointer to a SmsSentEvent struct */
    NW_EVENT_GPS,	                /*!< The SDK received a GPS event.   The handler event receives a pointer to a GpsEvent struct */
    NW_EVENT_DEVICE_ERROR,          /*!< Indicates that there was an error with the device and communication with the device has stopped. 
                                        For example the deivce was removed while in use.  The handler event receives a pointer to a DeviceErrorEvent struct.
                                        Use this event as an indication of a plug-n-play removal of a device that is currently being used by the SDK.*/ 
	NW_EVENT_GPS_XTRA_STATUS,		/*!< Reports the status of the XTRA file Download and Injection into the device. The handler event receives a pointer to a XtraStatuEvent struct */
	NW_EVENT_TIME_SYNC_STATUS,		/*!< Reports the status of the Time sync from NTP server. The handler event receives a pointer to a XtraTimeSyncEvent struct */
    NW_EVENT_DEVICE_ATTACHED,       /*!< The current session has succesfully attached to a device   */
    NW_EVENT_DEVICE_DETACHED,       /*!< The current session has succesfully detached from a device */
    NW_EVENT_MIP_ERROR,             /*!< A Mobile IP error indication has been received from the device */
    NW_EVENT_GPS_XTRA_DOWNLOAD_REQ, /*!< Requests the XTRA file to be downloaded and injected into the device. The handler event receives a pointer to a XtraStatuEvent struct */
    NW_EVENT_GPS_XTRA_TIMEINFO_REQ, /*!< Requests the XTRA Time sync from the NTP server to be injected into the device. The handler event receives a pointer to a XtraStatuEvent struct */
    NW_EVENT_GPS_XTRA_CMD_ERR,      /*!< Indicates that the XTRA download or Time sync command was sent to the GPS engine incorrectly. */
    NW_EVENT_GPS_XTRA_DOWNLOAD_STATUS, /*!< Indicates that the status of XTRA download or Time sync command as reported by the firmware. */
    NW_EVENT_GPS_SMS_FILTER,        /*!< Reports a specialized GPS directed SMS was received.  Only for VZW LBS services */
    NW_EVENT_GPS_SECURITY,          /*!< Reports that gps security is initialized.  Only for VZW LBS services */
    NW_EVENT_GPS_CMD_ERROR,         /*!< Reports that a gps command failed. Only for VZW LBS services. */
    NW_EVENT_GPS_FIX_STATUS,        /*!< Reports the error code when a gps fix attempt has ended. Only for VZW LBS services. */
    NW_EVENT_NETWORK_ERROR,         /*!< Reports EVDO network error codes. Only for EVDO devices. */
    NW_EVENT_SMS_EX,                /*!< The handler event receives a pointer to a SmsExEvent struct. */
    NW_EVENT_HDR_CONNECTION_ATTEMPT,/*!< A 1xEVDO connection attempt event has been received from the device */
    NW_EVENT_HDR_CONNECTION_RELEASE,/*!< A 1xEVDO connection release event has been received from the device */
    NW_EVENT_CDMA_CONNECTION_RELEASE,/*!< A CDMA connection was terminated by the device. */
    NW_EVENT_OMADM_LTE,             /*!< LTE devices.  An LTE related OMA firmware update event was received. The handler event receives a pointer to a LteOmaStatusEvent struct */
	NW_EVENT_IMS_REG_STATUS,		/*!< IMS reg. event */
	NW_EVENT_CONNECTION_STATUS,		/*!< Connection status event containing ConnectionStatusStructEx as payload (MULTI-MODE devices only) */
	NW_EVENT_SIM_STATE,				/*!< Generated when there  is a change in the SIM state.*/
	NW_EVENT_NW_IP_ADDR,			/*!< Generated when there  is a change in the IP ADDRESS.*/
	NW_EVENT_NW_CHANNEL_BAND,		/*!< Reports the Channel and the Band Class.*/
	NW_EVENT_NW_LTE_BW,				/*!< Reports the LTE Bandwidth.*/

}NvtlEventType;                     

/// Defines a structure sent to registered event callbacks containing information about the event.
typedef struct
{
	uint32_t	type;       /*!< Contains the type of the event (NvtlEventType) */
	uint32_t	size;       /*!< Contains the total size of the event buffer */
	uint8_t*	buffer;     /*!< A buffer containing the data or the pointer to the appropriate structure or type */
}NvtlEventStruct;

typedef struct {
    uint32_t val;  /*!< The value of the event */
}StandardEvent;

/// Defines the strength event received in the event callback when
/// the signal strength changes.  The value is the current signal strength of the device
typedef StandardEvent SigStrEvent;         

/// Defines the romaing event struct receieved in the event callback when
/// the roaming status changes.  The value is the current roaming status of the device as defined by the #DeviceRoamStatusType enum
typedef StandardEvent RoamingEvent;

/// Defines the device state event received in the event callback when
/// the device state changes.  The value is the current state of the device as defined by the #DeviceStateType enum
typedef StandardEvent DeviceStateEvent;

/// Defines the dormancy event received in the event callback when
/// the dormancy status changes.  The value is the current dormancy status of the device
typedef StandardEvent DormantEvent;

/// Defines the network event received in the event callback when
/// the network status changes.  The value is the current network state of the device as defined by the #DeviceServiceType enum
typedef StandardEvent NetworkEvent;

/// Defines the server error event received in the event callback when
/// an error is received from the device relating to connections.  The value is an
/// error received during a connection or connection attempt.
typedef StandardEvent ServerErrorEvent;

/// Defines the sms event received in the event callback when 
/// new sms messages are available.  The value is an indicator that new sms messages are waiting.
typedef StandardEvent SmsEvent;

/// Defines the sms sent event received in the event callback when
/// an sms message is either sent or failed during transmission. The value is an indicator of the 
/// success or failure of an attempt at sending an sms message
typedef StandardEvent SmsSentEvent;

/// Defines the device added event received in the event callback when
/// a new device is added to the system. 
typedef StandardEvent DeviceAddedEvent;

/// Defines the device removed event received in the event callback when
/// a device is removed from the system
typedef StandardEvent DeviceRemovedEvent;

/// Defines the activation event received in the event callback when
/// the activation status changes.  The value is an indication of the current state of the activation process
/// as defined by the #ActivationStateType enum
typedef StandardEvent ActivationEvent;

/// Defines the gps event received in the event callback when
/// gps status changes or fixes are available.
typedef StandardEvent GpsEvent;

/// Defines the device erorr event received in the event callback when
/// an error occurs with the communication with a device.
typedef StandardEvent DeviceErrorEvent;

/// Defines the xtra status event received in the event callback when
/// the xtra status of the gps engine changes.
typedef StandardEvent XtraStatuEvent;

/// Defines the xtra time synce event received in the event callback when
/// the gps engine synchronizes time with an internet server.
typedef StandardEvent XtraTimeSyncEvent;

/// Defines the device erorr event received in the event callback when
/// a session has succesfully attached to a device.
typedef StandardEvent DeviceAttachedEvent;

/// Defines the device erorr event received in the event callback when
/// a session has succesfully detached from a device.
typedef StandardEvent DeviceDetachedEvent;

/// Defines the error event received in the event callback when
/// a mobile ip error has been received.
typedef StandardEvent MipErrorEvent;

/// Defines the error event received in the event callback when
/// a 1xEVDO error has been received.
typedef StandardEvent NetworkErrorEvent;

/// Defines the error event received in the event callback when
/// a 1xEVDO error has been received.
typedef StandardEvent HdrConnectReleaseEvent;

/// Defines the unsolicited event received in the event callback when
/// an unsolicited AT repsonse is received.
typedef char* UnsolicitedATEvent;

/// Defines the event received when a specialized GPS routed SMS is received. Select VZW devices only.
typedef struct{
    char    val[255];
}GpsSmsFilterEvent;

/// Defines the event received when an MT SMS is received.
typedef struct{
    int32_t     storage_type;
    int32_t     index;
}SmsExEvent;

/// Defines the erorr event received in the event callback when
/// a gps command has failed.
typedef StandardEvent GpsCommandErrorEvent;


/// Defines the callback function used to propogate SDK events.
typedef void (*NvtlSdkEventFunc)( void* user_data, uint32_t type, uint32_t size, void* ev );

typedef struct 
{
    uint32_t                flags;
    uint32_t                status;
    uint16_t                command;
    uint16_t                startGpsWeek;
    uint16_t                startGpsMinutes;
    uint16_t                validDurationHours;
    char                  xtra_server_primary[128];
    char                  xtra_server_secondary[128];
    char                  xtra_server_tertiary[128];
    uint32_t                maxFilePartSize;
    uint32_t                maxFileSize;
} GPSEventXTRADownloadStruct;

typedef struct 
{
    uint32_t                command;
    uint32_t                oneway_delay_failover_thresh;
    char                  xtra_server_primary[128];
    char                  xtra_server_secondary[128];
    char                  xtra_server_tertiary[128];
} GPSEventXTRATimeinfoStruct;


/// Callback structure used to receive events from the SDK
typedef struct
{
	void*               user_data;      /*!< A user defined pointer that is returned during the callback */
	NvtlSdkEventFunc    event_func;     /*!< A pointer to the NvtlSdkEventFunc that should be called for events */
}NvtlEventCallback;

///Contains information about a file in the EFS of the device
typedef struct
{
	uint32_t	type;       /*!< File type */
	uint32_t	mode;       /*!< file mode */
	uint32_t	size;       /*!< file size */
	uint32_t	nLink;      /*!< number of links */
	uint32_t	aTime;      /*!< access time */
	uint32_t    mTime;      /*!< modified time */
	uint32_t	cTime;      /*!< creation time */
	char name[NW_MAX_PATH];     /*!< filename */
}EfsFileInfoStruct;

typedef enum
{
    ROUTER_CONFIG_ID_SSID_CURRENT       = 1,    /*!< The current SSID in use */
    ROUTER_CONFIG_ID_SSID_OPEN          = 2,    /*!< The SSID for the open profile */
    ROUTER_CONFIG_ID_SSID_SECURE        = 3,    /*!< The SSID for the secure profile */
    ROUTER_CONFIG_ID_SSID_TEMPORARY     = 4,    /*!< The SSID for the temporary profie */
    ROUTER_CONFIG_ID_PROFILE_CURRENT    = 5,    /*!< The current wireless profile in use as defined by 0 = Open, 1 = Secure, 2 = Temporary */
    ROUTER_CONFIG_ID_KEY_CURRENT     = 6,       /*!< Returns the current active key in use according to the current wireless profile in use */
}RouterConfigItemIdType;

/// \enum CallStatusType
/// Defines the possible values of the current call state of the device
typedef enum 
{
	CALL_STATUS_IDLE,            /*!< The device is idle */
	CALL_STATUS_CONNECTING,      /*!< The device is establishing a call */
	CALL_STATUS_AUTHENTICATING,  /*!< The device is authenticating a call */
	CALL_STATUS_CONNECTED,       /*!< The device in in a call */
	CALL_STATUS_DORMANT,         /*!< The device is in a call but dormant */
	CALL_STATUS_DISCONNECTING,   /*!< The device is terminating a call */
}CallStatusType;

typedef enum 
{
    //// 1x Specific Network Errors
    NETWORK_ERROR_1X_CO_NO_SERVICE = 1000,      /*!< Call origination – No service */
    NETWORK_ERROR_1X_CO_ACCESS_FAILURE,         /*!< Call origination – Access Failure */
    NETWORK_ERROR_1X_CO_CANNOT_ORIGINATE,       /*!< Call origination – Cannot Originate */
    NETWORK_ERROR_1X_CO_REDIRECTION,            /*!< Call origination – Redirection */
    NETWORK_ERROR_1X_CO_HANDOFF,                /*!< Call origination – Handoff to another mode */
    NETWORK_ERROR_1X_CO_IN_PROGRESS,            /*!< Call origination – Access already in progress */
    NETWORK_ERROR_1X_CO_PRIORITY_INBOUND,       /*!< Call origination – Priority given to inbound */
    NETWORK_ERROR_1X_CO_LOCKED,                 /*!< Call origination – Locked */
    NETWORK_ERROR_1X_CO_INCOMPATIBLE_SERVICES,  /*!< Call origination – Incompatible services */
    NETWORK_ERROR_1X_CO_CONCURRENT_NOT_SUPPORTED,/*!< Call origination – Concurrent not supported */
    NETWORK_ERROR_1X_CO_NO_RESPONSE,            /*!< Call origination – Timeout */
    NETWORK_ERROR_1X_CO_REJECT,                 /*!< Call origination – Reject */
    NETWORK_ERROR_1X_CO_SO_NOT_SUPPORTED,       /*!< Call origination – SO not supported */
    NETWORK_ERROR_1X_CO_CHANNEL_OPEN,           /*!< Call origination – Channel already open */
    NETWORK_ERROR_1X_CO_ALERT_STOP,             /*!< Call origination – Alert stop */
    NETWORK_ERROR_1X_CO_MAX_ACCESS,             /*!< Call origination – Max access */
    NETWORK_ERROR_1X_CO_ACTIVATION_NOT_AVAILABLE,/*!< Call origination – Activation not available */
    NETWORK_ERROR_1X_CO_INTERCEPT,              /*!< Call origination – Call intercepted */
    NETWORK_ERROR_1X_CO_REORDER,                /*!< Call origination – Reorder */      
    NETWORK_ERROR_1X_CO_OTHER,                  /*!< Call origination – Other */
    NETWORK_ERROR_1X_RELEASE_FADE,              /*!< Release – Fade */
    NETWORK_ERROR_1X_RELEASE_NO_REASON,         /*!< Release - No reason specified */
    NETWORK_ERROR_1X_RELEASE_SO_NOT_SUPPORTED,  /*!< Release - SO not supported */
    NETWORK_ERROR_1X_PROTOCOL_FAILURE,          /*!< Protocol Failure */
    NETWORK_ERROR_1X_REDIRECT_TO_EVDO,          /*!< Redirect to EVDO */
    NETWORK_ERROR_1X_FADE,                      /*!< Fade */
    NETWORK_ERROR_1X_USER_DISCONNECTED,         /*!< User disconnected */
    NETWORK_ERROR_1X_OTASP_ENDED,               /*!< OTASP ended */
    NETWORK_ERROR_1X_ENDED_FOR_VOICE,           /*!< Ended for voice*/
    NETWORK_ERROR_1X_E911_CALL_ENDED,           /*!< 911 call ended */
    NETWORK_ERROR_1X_E911_EMERGENCY_CALL,       /*!< Call ended in favor of 911 emergency call */
    NETWORK_ERROR_1X_E911_GPS_FIX,              /*!< Call ended in favor of 911 GPS fix */
      


    /// GSM/WCDMA Specific Network Errors
    NETWORK_ERROR_WCDMA_NO_SERVICE = 2000,      /*!< No Service */
    NETWORK_ERROR_WCDMA_PROTOCOL_FAILURE,       /*!< Protocol failure */
    NETWORK_ERROR_WCDMA_ORIGINATION_FAILURE,    /*!< Origination failure */
    NETWORK_ERROR_WCDMA_INCOMING_REJECTED,      /*!< Incoming rejected */
    NETWORK_ERROR_WCDMA_NETWORK_DISCONNECTED,   /*!< Network disconnected */
    NETWORK_ERROR_WCDMA_USER_DISCONNECTED,      /*!< User disconnected */
     


    /// EVDO Specific Network Errors
    NETWORK_ERROR_EVDO_CO_NO_SERVICE = 3000,    /*!< Call Origination - No service */
    NETWORK_ERROR_EVDO_CO_ACCESS_FAILURE,       /*!< Call Origination - Access failure */
    NETWORK_ERROR_EVDO_CO_REDIRECTION,          /*!< Call Origination - Redirection */
    NETWORK_ERROR_EVDO_CO_NOT_PREFERRED,        /*!< Call Origination - Not in preferred PRL list */
    NETWORK_ERROR_EVDO_CO_MODE_HANDOFF,         /*!< Call Origination - Mode handoff  */
    NETWORK_ERROR_EVDO_CO_IN_PROGRESS,          /*!< Call Origination - Call already in progress */
    NETWORK_ERROR_EVDO_CO_SETUP_TIMEOUT,        /*!< Call Origination - Call setup timed out */
    NETWORK_ERROR_EVDO_CO_SESSION_NOT_OPEN,     /*!< Call Origination - Session not open */
    NETWORK_ERROR_EVDO_RELEASE_NO_REASON,       /*!< Release - No reason specified */
    NETWORK_ERROR_EVDO_PROTOCOL_FAILURE,        /*!< Protocol failure */
    NETWORK_ERROR_EVDO_DENY_NO_REASON,          /*!< Denied - No reason specified */
    NETWORK_ERROR_EVDO_DENY_NETWORK_BUSY,       /*!< Denied - Network busy */
    NETWORK_ERROR_EVDO_DENY_AUTHENTICATION,     /*!< Denied - Authentication or billing failure */
    NETWORK_ERROR_EVDO_REDIRECT_TO_1X,          /*!< Redirect to 1X */
    NETWORK_ERROR_EVDO_FADE,                    /*!< Fade */
    NETWORK_ERROR_EVDO_USER_DISCONNECTED,       /*!< User disconnected */
    NETWORK_ERROR_EVDO_GPS_FIX,                 /*!< EV-DO call ended in favor of GPS fix */

      


    /// LTE Specific Network Errors
    NETWORK_ERROR_LTE_NO_SERVICE = 4000,        /*!< No service */
    NETWORK_ERROR_LTE_PROTOCOL_FAILURE,         /*!< Protocol failure */
    NETWORK_ERROR_LTE_NETWORK_DISCONNECTED,     /*!< Network disconnected */
    NETWORK_ERROR_LTE_USER_DISCONNECTED,        /*!< User disconnected */
     
} NetworkErrorType;

/// Defines a structure containing generally relevant device information
typedef struct
{
	uint32_t                dwSize;			/*!< Reserved. The size of the struct. */
    uint32_t                dwCurFWVersion;	/*!< Reserved */
	SourceBuildBaseType     eBuildBase;		/*!< base chipset if known */
	ServiceProviderType     eSrvProvider;	/*!< The intended service provider, if known */
	DeviceTechType          eTech;			/*!< The device technology of the device */
	DeviceFormFactorType    eFormFactor;	/*!< The form factor of the device */
	DeviceStateType	        eState;         /*!< A general state of the device taking into account various pieces of volatile info.
                                             * This state is a primary indication of the readiness or current actions of the device */
    DeviceModeType          modeDevice;		/*!< The Current power mode of the device */
	CallStatusType          connStatus;		/*!< Indicates the current call state of the device */
	
    uint8_t                 bDeviceLocked;  /*!< Indicates if the device is currently locked and requires a PIN or PUK */
	uint8_t                 maskWDisable;   /*!< Indicates if the device is currently disabled */
	uint8_t                 subsysID;		/*!< reserved */

	//module related static info
	char                  szSoftwareVersion[ NW_SIZE_NAME * 2];	/*!< Firmware version string */
	char                  szDeviceModel[ NW_SIZE_NAME ];			/*!< Device model string */ 
	char                  szHardwareVersion[ NW_SIZE_NAME ];		/*!< Hardware version string */
	char                  szDriverVersion[ NW_SIZE_NAME ];		/*!< Driver version string */
	char                  szMDN[ NW_SIZE_NUMBER ];				/*!< Mobile Number */
	char                  szFID[ NW_SIZE_NAME ];					/*!< Factory ID */

    //UMTS/HSPA specific values
    char                  szIMEI[ NW_SIZE_NUMBER ];		/*!< UMTS/HSPA. IMEI string */
    char                  szSMSC[ NW_SIZE_NAME ];			/*!< UMTS/HSPA. SMSC string */
	char                  szIMSI[ NW_SIZE_NUMBER ];		/*!< UMTS/HSPA. IMSI string */
	char                  szICCID[ NW_SIZE_NUMBER ];		/*!< UMTS/HSPA. ICCID string */

    //EVDO specific values
    char                  szMIN [ NW_SIZE_NUMBER ];		/*!< EVDO. Mobile Identification Number (MIN) */
    uint32_t                dwESN;							/*!< EVDO. Electronic Serial Number (ESN) */
	uint32_t                dwPRLVersion;					/*!< EVDO. PRL verison */
	uint32_t                dwERIVersion;                   /*!< EVDO.  ERI version */
	uint32_t                dwHomeSID;						/*!< EVDO. HOME SID */

} DeviceInfoStruct;

/// Defines a structure that contains relevant information about the current network state of the device
typedef struct
{
    uint32_t                dwSize;         /*!< reserved. the size of the stuct */
	uint32_t                dwSigStr;       /*!< The current signal strength of the device in range 0 - 5 */
	uint32_t                dwRSSI;	        /*!< The current rssi of the device */
	uint32_t	            dwEnsStatus;    /*!< HSPA ENS Status */    
    float                   fdBmSignal;     /*!< The dbm signal of the device */
    DeviceServiceType       eService;       /*!< The current network service type of the device */
	DeviceRoamStatusType    eRoam;          /*!< The current roaming status of the device */
    RoamIndicatorType	    eIndID;         /*!< EVDO. ERI Roaming indicator type */
    uint8_t                 bDormant;       /*!< The current dormancy state of the device */
	uint8_t                 bNewSMS;        /*!< A flag indicating new sms status */ //should remove?
} NetworkInfoStruct;

/// EVDO Only - Defines a structure containing an rssi and signal strengh pair.
typedef struct
{
	uint16_t	rssi;       /*!< An rssi retrieved from the device */
	uint16_t	rssidBm;    /*!< The dbm signal of the device */
} CdmaRSSIValue;

/// EVDO Only - Defines a structure containing both 1x and EVDO rssi values.
typedef struct
{
    CdmaRSSIValue cdma; /*!< Rssi and signal strengh for the 1xRTT signal */
    CdmaRSSIValue evdo; /*!< Rssi and signal strengh for the EVDO signal */
} RSSIStruct;

/// \enum SdkLogLevelType
/// This enum defines the possible log levels
typedef enum
{
	SdkLogLevelNone	= 0,    /*!< No logging */
	SdkLogLevelError,       /*!< Log only error statements */
	SdkLogLevelWarning,     /*!< Log any warning or more serious statement */
	SdkLogLevelInfo,        /*!< Log general usage or more serious statements */
	SdkLogLevelDebug,       /*!< Log more detailed debugging statement */
	SdkLogLevelAll          /*!< Enable all possible statement */
}SdkLogLevelType;

/// \enum SdkLogOutputType
/// This enum defines the output method used for logging statement
typedef enum
{
	SdkOutputNone	= 0x00,     /*!< No logging output */
	SdkOutputStdOut	= 0x01,     /*!< Log to stdout */
	SdkOutputFile	= 0x02,     /*!< Log to a file */
    SdkOutputBoth   = 0x03,     /*!< Log to both a file and std out */
    SdkOutputTrace  = 0x04,     /*!< Log to Windows Debug Log using OutputDebugString */
}SdkLogOutputType;

/// \enum MipDetailMask
/// Defines mask values related to the MIP settings
typedef enum{
	MIP_DETAIL_HOME_ADDRESS	    = 0x0001,	/*!< \e MIP_Home_Addr value is being specified */
	MIP_DETAIL_PRI_HA_ADDRESS	= 0x0002,	/*!< \e MIP_PRI_HA_Addr value is being specified */
	MIP_DETAIL_SEC_HA_ADDRESS	= 0x0004,	/*!< \e MIP_SEC_HA_Addr value is being specified */
	MIP_DETAIL_HA_SPI			= 0x0008,	/*!< \e MIP_HA_SPI value is being specified */
	MIP_DETAIL_AAA_SPI			= 0x0010,	/*!< \e MIP_AAA_SPI value is being specified */
	MIP_DETAIL_REV_TUNNEL		= 0x0020,	/*!< \e MIP_REV_TUN value is being specified */
    MIP_DETAIL_HA_KEY           = 0x0040,	/*!< \e ha_key value is being specified */
    MIP_DETAIL_AAA_KEY          = 0x0080,	/*!< \e aaa_key value is being specified */
    MIP_DETAIL_NAI              = 0x0100,   /*!< \e NAI value is being specified */
    MIP_DETAIL_ENABLED          = 0x0200,   /*!< \e Enabled value is being specified */
} MipDetailMask;

/// Defines a structure representing a date and time.
typedef struct
{
	uint16_t		wYear;      /*!< year */
	uint16_t		wMonth;     /*!< month */
	uint16_t		wDay;       /*!< day */
	uint16_t		wHour;      /*!< hour */
	uint16_t		wMinute;    /*!< minute */
	uint16_t		wSeconds;   /*!< seconds */
	uint16_t		wMilSecs;   /*!< milliseconds */
	uint16_t		wUTCDiff;   /*!< utc offset */
} NvtlTimeStruct;

/// Defines a structure used for setting MIP detail information.
typedef struct
{
	uint8_t		    index;                  /*!< TODO mip detail index */
	uint8_t		    mn_ha_spi_set;          /*!< TODO flag indicating ha spi is set*/
	uint32_t		mn_ha_spi;              /*!< TODO ha spi value*/
	uint8_t		    mn_aaa_spi_set;         /*!< TODO flag indicating aaa spi is set */
	uint32_t		mn_aaa_spi;             /*!< TODO aaa spi value */
	uint8_t		    rev_tun_pref;           /*!< TODO rev tun pref */
	uint32_t		home_addr;              /*!< TODO home address */
	uint32_t		primary_ha_addr;        /*!< TODO primary ha address */
	uint32_t		secondary_ha_addr;      /*!< TODO secondary ha addrress */
} MipDetailsStruct;

/// Defines a structure used for setting MIP detail information.  Supported only on select devices
typedef struct
{
    uint32_t        mask;                   /*!< When setting, specifies which of the fields contain data to be written */
	uint8_t		    index;                  /*!< mip detail index */
	uint32_t		mn_ha_spi;              /*!< ha spi value*/
	uint32_t		mn_aaa_spi;             /*!< aaa spi value */
	uint8_t		    rev_tun_pref;           /*!< rev tun pref */
	uint32_t		home_addr;              /*!< home address */
	uint32_t		primary_ha_addr;        /*!< primary ha address */
	uint32_t		secondary_ha_addr;      /*!< secondary ha addrress */
    uint8_t         enabled;                /*!< enabled state */
    char          ha_key[25];             /*!< ha_key */
    char          aaa_key[25];            /*!< aaa_key */
    char          nai[72];                /*!< nai    */
} MipDetailsStructEx;


/// \enum MipParametersMask
/// Defines mask values related to the fields in the MipParametersStruct
typedef enum{
	MIP_PARAM_MODE              = 0x0001,	/*!< \e mode value is being specified */
	MIP_PARAM_RETRY_LIMIT       = 0x0002,	/*!< \e retry limit value is being specified */
	MIP_PARAM_RETRY_INTERVAL    = 0x0004,	/*!< \e retry interval value is being specified */
	MIP_PARAM_RE_REG_PERIOD     = 0x0008,	/*!< \e re-registration period value is being specified */
	MIP_PARAM_RE_REG_TRAFFIC    = 0x0010,	/*!< \e re-registration traffic value is being specified */
	MIP_PARAM_HA_AUTHENTICATOR  = 0x0020,	/*!< \e HA Authenticator value is being specified */
    MIP_PARAM_HA2002BIS         = 0x0040,	/*!< \e HA2002BIS value is being specified */
} MipParametersMask;

/// Defines a structure used for setting or retreiving MIP parameter information.  Supported only on select devices
typedef struct
{
    uint32_t  mask;                   /*!< When setting, specifies which of the fields contain data to be written */
    uint8_t   mode;                   /*!< MIP mode 0-off 1-preferred 2-MIP only */
    uint8_t   retryLimit;             /*!< Registration retry attempt limit */
    uint8_t   retryInterval;          /*!< Registration retry attempt interval */
    uint8_t   reRegPeriod;            /*!< Period to attempt re-registration before current registration expires (in minutes) */
    uint8_t   reRegTraffic;           /*!< Re-registration only if traffic since the last attempt? */     
    uint8_t   HAAuthenticator;        /*!< MH-HA authenticator calculator? */ 
    uint8_t   HA2002bis;              /*!< MH-HA RFC 2002bis authentication instead of RFC2002 */
}MipParametersStruct;

/// Defines a structure used for setting MIP key information
typedef struct 
{
    uint8_t      index;                           /*!< TODO mip keys index */
    uint8_t      mn_ha_shared_secret_length;      /*!< TODO length of ha key */
    uint8_t      mn_ha_shared_secret	[16];       /*!< TODO ha key*/
    uint8_t      mn_aaa_shared_secret_length;     /*!< TODO aaa key length */
    uint8_t      mn_aaa_shared_secret[16];        /*!< TODO aaa key */
} MipKeysStruct;

/** \enum MipKeyStatusType
<tt><b>\<CDMA/EVDO devices only\></b></tt>\n
This enum defines values for the status of shared secret keys. 
*/
typedef enum
{
	MIP_KEY_STATUS_SET          = 0,	/*!< 0 - Both HA and AAA are set */
	MIP_KEY_STATUS_NOT_SET      = 3,    /*!< 3 - Either HA or AAA or both are not set */
	MIP_KEY_STATUS_MASS_NOT_SET = 4,	/*!< 4 - When only HA is set */
	MIP_KEY_STATUS_MHSS_NOT_SET = 5		/*!< 5 - When only AAA is set */
}MipKeyStatusType;

/** \enum HdrConnectResultType
<tt><b>\<CDMA/EVDO devices only\></b></tt>\n
This enum defines values for connect status when a connection attempt event is received from the device. 
*/
typedef enum
{
    HDR_CONNECT_DENY_GENERAL = 0,    /*!< 0 - Connection denied with reason = General */
    HDR_CONNECT_DENY_NETWORK,        /*!< 1 - Connection denied with reason = Network Busy */
    HDR_CONNECT_DENY_AUTHENTICATION, /*!< 2 - Connection denied with reason = Authentication or billing failure */
    HDR_CONNECT_MAX,                 /*!< 3 - Maximum access probes */
    HDR_CONNECT_SYSTEM_LOST,         /*!< 4 - System lost (supervision failures) */
    HDR_CONNECT_NOT_PREFERRED,       /*!< 5 - Not Preferred (SD told OHVD to switch systems, QC redirect, Access network ID) */
    HDR_CONNECT_REDIRECT,            /*!< 6 - Redirect (ALMP received a redirect message) */
    HDR_CONNECT_TIMEOUT,             /*!< 7 - Connection setup timeout */
    HDR_CONNECT_POWER,               /*!< 8 - Power down received */
    HDR_CONNECT_OFFLINE,             /*!< 9 - Offline received */
    HDR_CONNECT_NAM,                 /*!< 10 - NAM change received */
    HDR_CONNECT_USER,                /*!< 11 - User abort */
    HDR_CONNECT_HANDOFF,             /*!< 12 - Access handoff */
    HDR_CONNECT_RESERVED1,           /*!< 13 - Reserved */
    HDR_CONNECT_RESERVED2,           /*!< 14 - Reserved */
    HDR_CONNECT_SUCCESS,             /*!< 15 - Success */
}HdrConnectResultType;

/** \enum HdrConnectTCAType
<tt><b>\<CDMA/EVDO devices only\></b></tt>\n
This enum defines values for the disconnect reason if a connection is terminated by the device. 
*/
typedef enum
{
    HDR_TCA_RTCACK_FAIL = 0,         /*!< 0 - Neither TCA nor RTCACK messages received */
    HDR_TCA_ONLY,                    /*!< 1 - TCA message received but RTCACK not received */
    HDR_TCA_RTCACK_SUCCESS,          /*!< 2 - Both TCA and RTCACK received */
} HdrConnectTCAType;

/// Defines a structure to hold the connection attempt result codes, used by NW_EVENT_HDR_CONNECTION_ATTEMPT.
typedef struct 
{
    HdrConnectResultType result;
    HdrConnectTCAType    tca;
} HdrConnectResultEvent;

/** \enum HdrConnectReleaseType
<tt><b>\<CDMA/EVDO devices only\></b></tt>\n
This enum defines values for the disconnect reason if a connection is terminated by the device, used by NW_EVENT_HDR_CONNECTION_RELEASE. 
*/
typedef enum
{
    HDR_RELEASE_AN = 0,                 /*!< 0 - AN connection close */
    HDR_RELEASE_AT,                     /*!< 1 - AT connection close */
    HDR_RELEASE_SYSTEM,                 /*!< 2 - System lost (supervision failures, TCA message rejected) */
    HDR_RELEASE_NOT_PREFERRED,          /*!< 3 - Not Preferred (SD told OHVD to switch systems, QC redirect, Access network ID) */
    HDR_RELEASE_REDIRECT,               /*!< 4 - Redirect (ALMP received a redirect message) */
    HDR_RELEASE_POWER,                  /*!< 5 - Power down received */
    HDR_RELEASE_OFFLINE,                /*!< 6 - Offline received */
    HDR_RELEASE_NAM,                    /*!< 7 - NAM change received */
    HDR_RELEASE_PAGE,                   /*!< 8 - Page message received */
    HDR_RELEASE_UNSPECIFIED             /*!< 9 - Unspecified (ALMP rude close) */
}HdrConnectReleaseType;

/// Defines a structure for obtaining ERI information.
typedef struct
{
    int32_t     roam;          /*!< TODO ERI roaming value */
	int32_t     indicatorId;   /*!< TODO ERI indicator id*/
	int32_t     iconId;        /*!< ERI icon id*/
	int32_t     alertId;       /*!< ERI alert id*/
	char      version[15];   /*!< ERI version string */
	char      data[33];      /*!< ERI data */
}EriInfoStruct;

/// Defines a structure for accessing Sid an Nid information
typedef struct
{
    uint16_t sid_count;   /*!< TODO the number of SIDs in the sids array */
    uint16_t nid_count;   /*!< TODO the number of NIDs in the nids array */
    uint16_t sids[20];    /*!< TODO the array of SID values */
	uint16_t nids[20];    /*!< TODO the array of NID valus*/
}SidNidInfoStruct;

/// Defines a structure for accessing channel information
typedef struct
{
    uint16_t primary_a;       /*!< TODO Primay channel a*/
    uint16_t primary_b;       /*!< TODO Primary channel b*/
    uint16_t secondary_a;     /*!< TODO Secondary channel a*/
    uint16_t secondary_b;     /*!< TODO Secondary channel b*/
}ChannelParametersStruct;

/// Defines a structure containing the current channel and band class
typedef struct
{
    uint16_t channel;         /*!< Current channel*/
    uint16_t band_class;      /*!< Current band class*/
    uint16_t mode;            /*!< Current duplex mode*/
}ChannelBandStruct;

/// For the MultiMode Device with Firmware greater than 1.26. Defines a structure containing the current channel and band class
typedef struct
{
    unsigned short channel;         /*!< Current channel*/
    unsigned short band_class;      /*!< Current band class*/    
}ChannelBandStructEx;

typedef struct
{
	uint32_t err_no;
	uint32_t last_index;
	uint32_t num_entries;
	uint8_t  data[1];
} CallHistoryListType;


typedef struct 
{
  uint16_t    uYear;                  // i.e. 2003
  uint16_t    uMonth;                 // 1-12
  uint16_t    uDay;                   // 1-31
  uint16_t    uHour;                  // 0-23
  uint16_t    uMinute;                // 0-59
  uint16_t    uSecond;                // 0-59
  uint16_t    uMillisecond;           // 0-999
  int32_t     nMinutesFromUTC;        // Used for time zone specification (can be positive or negative)
} DATETIME;

typedef enum _WMC_CALL_TYPE
{
      WMC_CALL_ALL = 0,
      WMC_CALL_VOICE_INBOUND,
      WMC_CALL_VOICE_OUTBOUND,
      WMC_CALL_DATA_OUTBOUND,
} WMC_CALL_TYPE;


typedef struct
{
    uint32_t    err_no;
    uint32_t    data_len;
    uint32_t    index;
    uint32_t    num_length;
    char		number[24];
    uint32_t    total_time;
    uint32_t    active_time;
    uint32_t    tx_ip_bytes;
    uint32_t    rx_ip_bytes;
    WMC_CALL_TYPE  call_type;
    uint16_t	call_err_no; 
    uint8_t     reserved_byte1;
	
    DATETIME	start_date_time;
    uint8_t     reserved_byte2;
    uint8_t     reserved_byte3;
    uint32_t    ip_address;
    uint8_t     reserved[2];
} CallHistoryItemType;




/*******************************************************************
END Basic SDK API related types
********************************************************************/

/*******************************************************************
Diagnostics and Field Test related types
********************************************************************/
/** \enum field_test_field_evdo_e_type
This enum defines values that can be used to parse data obtained from method GetFieldTestInfo for a CDMA/EVDO device.
*/
typedef enum 
{
	FT_MIN_EVDO					= 0,	/*!< Place Holder */
	FT_WARRANTY_DATE			= 0,	/*!< Warranty Date */
	FT_SLOT_CYCLE_INDEX			= 1,	/*!< Slot Cycle Index */
	FT_CURRENT_NAM				= 2,	/*!< Current NAM */
	FT_AUTO_NAM					= 3,	/*!< Auto NAM */
	FT_SPC_CHANGE_ENABLED		= 4,	/*!< SPC Change Enabled */
	FT_NAM_NAME					= 5,	/*!< NAM Name */
	FT_DIRECTORY_NUMBER			= 6,	/*!< Directory # */
	FT_ACCOLC					= 7,	/*!< Access Overload Class */
	FT_MCC						= 8,	/*!< MCC */
	FT_MNC						= 9,	/*!< MNC */
	FT_CHANNEL_PRI_A			= 10,	/*!< Channel Primary A */
	FT_CHANNEL_PRI_B			= 11,	/*!< Channel Primary B */
	FT_CHANNEL_SEC_A			= 12,	/*!< Channel Secondary A */
	FT_CHANNEL_SEC_B			= 13,	/*!< Channel Secondary B */
	FT_HOME_SID_TABLE			= 14,	/*!< Home SID Table */
	FT_TERM_REG_HOME_SID		= 15,	/*!< Terminated Reg Home SID */
	FT_TERM_REG_FOREIGN_SID		= 16,	/*!< Terminated Reg Foreign SID */
	FT_TERM_REG_FOREIGN_NID		= 17,	/*!< Terminated Reg Foreign NID */
	FT_SYS_PREF_MODE			= 18,	/*!< System Preffered Mode */
	FT_PRL_VER					= 19,	/*!< PRL Version Number */
	FT_DNS_PRI					= 20,	/*!< DNS Primary */
	FT_DNS_SEC					= 21,	/*!< DNS Secondary */
	FT_PACKET_DIAL_STR			= 22,	/*!< Packet Dial String */
	FT_MDR_MODE					= 23,	/*!< MDR Mode */
	FT_DATA_SCRM				= 24,	/*!< Data SCRM */
	FT_MIP_HA_SPI				= 25,	/*!< MIP HA SPI Value */
	FT_MIP_REV_TUN				= 26,	/*!< MIP Reverse Tunneling */
	FT_MIP_HOME					= 27,	/*!< MIP Home */
	FT_MIP_PRI_HA				= 28,	/*!< MIP Primary HA Address */
	FT_MIP_SEC_HA				= 29,	/*!< MIP Secondary HA Address */
	FT_MIP_BEHAVIOR				= 30,	/*!< MIP Behavior */
	FT_MIP_PRE_REG_TIMEOUT		= 31,	/*!< MIP Pre-Registration Timeout */
	FT_MIP_REG_RETRIES			= 32,	/*!< MIP Registration Retries */
	FT_DMU_KEY_EXHG_INDICATOR	= 33,	/*!< DMU Key Exchange Indicator */
	FT_NID						= 34,	/*!< NID */
	FT_FER						= 35,	/*!< FER */
	FT_RSSI						= 36,	/*!< RSSI */
	FT_ECIO						= 37,	/*!< EC/IO */
	FT_CHANNEL					= 38,	/*!< Channel */
	FT_LATITUDE					= 39,	/*!< Latitude */
	FT_LONGITUDE				= 40,	/*!< Longitude */
	FT_TX_POWER					= 41,	/*!< Tx Power */
	FT_RX_POWER					= 42,	/*!< Rx Power */
	FT_BAND_CLASS				= 43,	/*!< Band Class */
	FT_PREV						= 44,	/*!< P Rev */
	FT_PKT_ZONE_ID				= 45,	/*!< Packet Zone ID */
	FT_LAST_CALL_ERR			= 46,	/*!< Last Call Error */
	FT_SO_IN_USE				= 47,	/*!< Service Option in Use */
	FT_CALL_STATE				= 48,	/*!< Call State. Possible values are:
												- 0 Idle (or no air link)
												- 1 Connecting
												- 2 Authenticating
												- 3 Connected and NOT DORMANT
												- 4 Connected and DORMANT
												- 5 Disconnecting
												.
												*/
	FT_DORMANT_STATE			= 49,	/*!< Dormant State */
	FT_MAC_INDEX				= 50,	/*!< MAC Index */
	FT_SUBNET_MASK				= 51,	/*!< Subnet Mask */
	FT_COL_CODE					= 52,	/*!< Color Code */
	FT_UATI024					= 53,	/*!< UATI024 */
	FT_1X_FIN_INFO_PN_OFFSET	= 54,	/*!< 1X Finger Info PN Offsets */
	FT_1X_FIN_INFO_WALSH_CODES	= 55,	/*!< 1X Finger Info Walsh Codes */
	FT_1X_FIN_INFO_RSSI			= 56,	/*!< 1X Finger Info RSSI */
	FT_1X_ACT_SET_COUNT			= 57,	/*!< Number of pilots in 1X Active Set */
	FT_1X_ACT_SET_PN_OFFSET		= 58,	/*!< 1X Active Set PN Offsets */
	FT_1X_ACT_SET_ECIO			= 59,	/*!< 1X Active Set Ec/Io (dBm). These are floating point values. */
	FT_1X_NEB_SET_COUNT			= 60,	/*!< Number of pilots in 1X Neighbor Set */
	FT_1X_NEB_SET_PN_OFFSET		= 61,	/*!< 1X Neighbor Set PN Offsets */
	FT_1X_NEB_SET_ECIO			= 62,	/*!< 1X Neighbor Set Ec/Io (dBm). These are floating point values. */
	FT_1X_CAND_SET_COUNT		= 63,	/*!< Number of pilots in 1X Candidate Set */
	FT_1X_CAND_SET_PN_OFFSET	= 64,	/*!< 1X Candidate Set PN Offsets */
	FT_1X_CAND_SET_ECIO			= 65,	/*!< 1X Candidate Set Ec/Io (dBm). These are floating point values. */
	FT_HDR_FIN_INFO_PN_OFFSET	= 66,	/*!< HDR Finger Info PN Offsets */
	FT_HDR_FIN_INFO_WALSH_CODES	= 67,	/*!< HDR Finger Info Walsh Codes */
	FT_HDR_FIN_INFO_RSSI		= 68,	/*!< HDR Finger Info RSSI */
	FT_HDR_ACT_SET_COUNT		= 69,	/*!< Number of pilots in EVDO Active Set */
	FT_HDR_ACT_SET_PN_OFFSET	= 70,	/*!< HDR Active Set PN Offsets */
	FT_HDR_ACT_SET_ECIO			= 71,	/*!< HDR Active Set Ec/Io (dBm). These are floating point values. */
	FT_HDR_ACT_SET_CHANNEL		= 72,	/*!< HDR Active Set Channel */
	FT_HDR_ACT_SET_BAND_CLASS	= 73,	/*!< HDR Active Set Band Class */
	FT_HDR_NEB_SET_COUNT		= 74,	/*!< Number of pilots in EVDO Neighbor Set */
	FT_HDR_NEB_SET_PN_OFFSET	= 75,	/*!< HDR Neighbor Set PN Offsets */
	FT_HDR_NEB_SET_ECIO			= 76,	/*!< HDR Neighbor Set Ec/Io (dBm). These are floating point values. */
	FT_HDR_NEB_SET_CHANNEL		= 77,	/*!< HDR Neighbor Set Channel */
	FT_HDR_NEB_SET_BAND_CLASS	= 78,	/*!< HDR Neighbor Set Band Class */
	FT_HDR_CAND_SET_COUNT		= 79,	/*!< Number of pilots in EVDO Candidate Set */
	FT_HDR_CAND_SET_PN_OFFSET	= 80,	/*!< HDR Candidate Set PN Offsets */
	FT_HDR_CAND_SET_ECIO		= 81,	/*!< HDR Candidate Set Ec/Io (dBm). These are floating point values. */
	FT_HDR_CAND_SET_CHANNEL		= 82,	/*!< HDR Candidate Set Channel */
	FT_HDR_CAND_SET_BAND_CLASS	= 83,	/*!< HDR Candidate Set Band Class */
	FT_SECTOR_ID				= 84,	/*!< Sector ID */
	FT_EVDO_TEMPERATURE			= 85,	/*!< Temperature */
	FT_EVDO_PRI_INFO			= 86,	/*!< PRI Info */
	FT_EVDO_AN_AAA_STATUS		= 87,	/*!< AN-AAA Status. Possible values are:
											- 0 Not Authenticated
											- 1 Authenticated
											.
											*/
	FT_HDR_SESSION_STATE		= 88,	/*!< HDR Session State. Possible values are:
											- 0 Inactive State
											- 1 AMP Setup State
											- 2 AT initiated State
											- 3 AN initiated State
											- 4 Open state
											.
											*/
	FT_MIP_AAA_SPI				= 89,	/*!< MIP AAA SPI Value */
	FT_ACQ_SID					= 90,	/*!< Acquired SID. SID of system in use.*/
	FT_MAX_EVDO	
} field_test_field_evdo_e_type;

/** \enum field_test_field_umts_e_type
This enum defines values that can be used to parse data obtained from method GetFieldTestInfo for a UMTS/HSDPA device.
*/
typedef enum 
{
	FT_MIN_UMTS					= 0,	/*!< Place Holder */
	FT_CALL_MANAGER_STATE		= 0,	/*!< Call Manager State */		
	FT_SYSTEM_MODE				= 1,	/*!< System Mode */
	FT_NAS_GMM_STATE			= 2,	/*!< NAS GMM State */
	FT_NAS_GMM_SUBSTATE			= 3,	/*!< NAS GMM Sub State */
	FT_NAS_MM_STATE				= 4,	/*!< NAS MM State */
	FT_NAS_MM_SUBSTATE			= 5,	/*!< NAS MM SubState */
	FT_NAS_MM_UPDATE			= 6,	/*!< NAS MM Update */
	FT_NAS_REGISTRATION_STATE	= 7,	/*!< NAS Registration State */
	FT_PLMN_SELECT_MODE			= 8,	/*!< PLMN Select Mode */
	FT_UE_OPERATION_MODE		= 9,	/*!< UE Operation Mode */
	FT_PS_CALL_ID				= 10,	/*!< PS Call ID */				
	FT_NSAPI_VALUE				= 11,	/*!< NSAPI Value */
	FT_NETWORK_OPERATION_MODE	= 12,	/*!< Network Operation Mode */
	FT_SERVICE_TYPE				= 13,	/*!< Service Type */
	FT_SERVING_CELL_PLMN		= 14,	/*!< Serving Cell PLMN */
	FT_SERVING_CELL_LAI			= 15,	/*!< Serving Cell LAI */
	FT_SERVING_CELL_RAI			= 16,	/*!< Serving Cell RAI */
	FT_NUM_OF_AVAILABLE_PLMN	= 17,	/*!< Number of Available PLMN */
	FT_PLMN_1_TO_10 			= 18,	/*!< PLMN 1..10 */
	FT_P_TMSI					= 19,	/*!< P_TMSI */
	FT_IMSI						= 20,	/*!< IMSI */						
	FT_IMEI						= 21,	/*!< IMEI */
	FT_IMEI_SOFTWARE_VERSION	= 22,	/*!< IMEI Software Version */
	FT_QOS_CONNECTION_ID		= 23,	/*!< QoS Connection ID */
	FT_DELAY_CLASS				= 24,	/*!< Delay Class */
	FT_RELIABILITY_CLASS		= 25,	/*!< Reliability Class */
	FT_PEAK_THROUGHPUT			= 26,	/*!< Peak Throughput */
	FT_PRECEDENCE_CLASS			= 27,	/*!< Precedence Class */
	FT_MEAN_THROUGHPUT			= 28,	/*!< Mean Throughput */
	FT_TRAFFIC_CLASS			= 29,	/*!< Traffic Class */
	FT_DELIVERY_ORDER			= 30,	/*!< Delivery Order */			
	FT_DELIVERY_ERROR_SDU		= 31,	/*!< Delivery of Error SDU */	
	FT_MAX_SDU_SIZE				= 32,	/*!< Max SDU Size */
	FT_MIN_BIT_RATE_UL			= 33,	/*!< Min Bit Rate UL */
	FT_MIN_BIT_RATE_DL			= 34,	/*!< Min Bit Rate DL */
	FT_RESIDUAL_ERROR_RATIO		= 35,	/*!< Residual Error Ratio */
	FT_TRANSFER_DELAY			= 36,	/*!< Transfer Delay */
	FT_TRAFFIC_HANDLING_PRIORITY= 37,	/*!< Traffic Handling Priority */
	FT_MAX_BIT_RATE_UL			= 38,	/*!< Max Bit Rate for UL */
	FT_MAX_BIT_RATE_DL			= 39,	/*!< Max Bit Rate for DL */
	FT_FDD_ACTIVE_SET_CELLS		= 40,	/*!< FDD Active Set Cells */		
	FT_FDD_ACTIVE_SET_FREQ		= 41,	/*!< FDD Active Set Frequency */
	FT_FDD_NEIGHBOR_SET_FREQ	= 42,	/*!< FDD Neighbor Set Frequency */
	FT_DOWNLINK_FREQ_1_TO_3		= 43,	/*!< Downlink Frequency 1..3 */
	FT_NEIGHBORS_PER_FREQ_1_TO_3= 44,	/*!< Neighbors per Freq 1..3 */
	FT_RRC_STATE				= 45,	/*!< RRC State */				
	FT_CURRENT_RRC_PROCEDURE	= 46,	/*!< Current RRC Procedure */
	FT_RRC_FAILURE_CAUSE		= 47,	/*!< RRC Failure Cause */
	FT_PROTOCOL_ERROR_CAUSE		= 48,	/*!< Protocol Error Cause */		
	FT_HSDPA_TEMPERATURE        = 49,	/*!< Temperature */
    FT_SERVING_CELL_ID          = 50,   /*!< Serving cell id */
    FT_CELL_LAC                 = 51,   /*!< Cell LAC */
	FT_RLC_BYTES_RX				= 52,   /*!< RLC Data Bearer Channel */
	FT_MAX_UMTS	
} field_test_field_umts_e_type;

typedef struct{
    char    str[STRING_LEN];    /*!< The string value*/
}StringStruct;

#define FT_MAX_MULTIMODE FT_MAX_EVDO+FT_MAX_UMTS
typedef struct{
    StringStruct    labels[FT_MAX_MULTIMODE];    /*!< Array of field test info names */
    StringStruct    values[FT_MAX_MULTIMODE];    /*!< Array of field test info values */
	uint32_t	    size;                        /*!< Number of entries inthe labels and values field */
}FieldTestInfoStruct;
/*******************************************************************
End Diagnostics and Field Test related types
********************************************************************/


/*******************************************************************
Connectivity related types
********************************************************************/
// Defines an API structure used to hold the Quality of Service settings to use for GSM data connections.
/// \n /// <tt><b>\<UMTS and HSDPA devices only\></b></tt>
typedef struct
{
    int32_t    MaxSduSize;             /*!< A numeric parameter (1,2,3,etc) that indicates the maximum allowed SDU size in octets. 
                                        \n If the parameter is set to '0' the subscribed value will be requested. */

    int32_t    DeliveryOfSDUError;     /*!< A numeric parameter that indicates whether SDUs detected as erroneous shall be delivered or not. 
                                        \n Possible values are as follows:
		                                \n \b 0 - no
		                                \n \b 1 - yes
		                                \n \b 2 - no detect
		                                \n \b 3 - subscribed value */

    int32_t    DeliveryOrder;          /*!< A numeric parameter that indicates whether the UMTS bearer shall provide in-sequence SDU delivery or not.
		                                \n Possible values are as follows:
		                                \n \b 0 - no
		                                \n \b 1 - yes
		                                \n \b 2 - subscribed value */

    int32_t    GuarBitDL;              /*!< A numeric parameter that indicates the guaranteed number of kbits/s delivered by UMTS  (down-link traffic) at a SAP (provided that there is data to deliver).
		                                \n If the parameter is set to '0' the subscribed value will be requested.
		                                \n This parameter should be provided if the Traffic class is specified as conversational or streaming. */

    int32_t    GuarBitUL;              /*!< A numeric parameter that indicates the guaranteed number of kbits/s delivered to UMTS (up-link traffic) at a SAP (provided that there is data to deliver).
		                                \n If the parameter is set to '0' the subscribed value will be requested.
		                                \n This parameter should be provided if the Traffic class is specified as conversational or streaming. */

    int32_t    MaxBitDL;               /*!< A numeric parameter that indicates the maximum number of kbits/s delivered by UMTS (down-link traffic) at a SAP.
		                                \n This parameter should be provided if the Traffic class is specified as conversational or streaming.
                                        \n If the parameter is set to '0' the subscribed value will be requested. */

    int32_t    MaxBitUL;               /*!< A numeric parameter that indicates the maximum number of kbits/s delivered to UMTS (up-link traffic) at a SAP.
		                                \n This parameter should be provided if the Traffic class is specified as conversational or streaming.
                                        \n 0 means default subscribed value */

    int32_t    TrafficClass;           /*!< A numeric parameter that indicates the type of application for which the UMTS bearer service is optimised.
		                                \n 0 - conversational
		                                \n 1 - streaming
		                                \n 2 - interactive
		                                \n 3 - background
		                                \n 4 - subscribed value */

    int32_t    TransferDelay;          /*!< A numeric parameter (0,1,2,...) that indicates the targeted time between request to transfer an SDU at one SAP to its delivery at the other SAP, in milliseconds.
		                                \n If the parameter is set to '0' the subscribed value will be requested. */

    int32_t    TrafficHandling;        /*!< A numeric parameter (1,2,3,etc) that specifies the relative importance for handling of all SDUs belonging to the UMTS bearer compared to the SDUs of other bearers.
		                                \n If the parameter is set to '0' the subscribed value will be requested. */

    char    ResErrorRatio[5];       /*!< A string parameter that indicates the target value for the undetected bit error ratio in the delivered SDUs.
                                         \n If no error detection is requested, Residual bit error ratio indicates the bit error ratio in the delivered SDUs.
		                                 \n The value is specified as 'mEe'. As an example a target residual bit error ratio of 5\b .10<sup>-3 </sup> would be specified as '5E3'. 
                                         \n '0E0' means subscribed value. */

    char    SDUErrorRatio[5];       /*!< A string parameter that indicates the target value for the fraction of SDUs lost or detected as erroneous. 
                                        \n SDU error ratio is defined only for conforming traffic.
                                		\n  The value is specified as 'mEe'. As an example a target SDU error ratio of 5\b .10<sup>-3 </sup> would be specified as "5E3". 
                                        \n "0E0" means subscribed value. */
}QosSettingsStruct;

/// Defines an API structure used to hold the APN settings for GSM data connections.
/// \n /// <tt><b>\<UMTS and HSDPA devices only\></b></tt>
typedef struct
{
    char     APN[STRING_LEN];    /*!< The APN string to use for the connecton */
    int32_t    PDPAddress;         /*!< The PDP address to use for the connection. 0 means dynamic address */
    int32_t    PDPType;            /*!< The PDPType to use for the connection. 0 means PPP, 1 means IPV4, 2 means IPV6, 3 means IPV4 and IPV6 */
}ApnSettingsStruct;

/// Defines a structure used to hold the general data connection settings.
typedef struct
{
    char     ConnectionName[STRING_LEN];   /*!< The name of the connection interface (Ras or Ndis name)*/
    char     Username[STRING_LEN];         /*!< The user name to use for the connection */
    char     Password[STRING_LEN];         /*!< The password to use for the connection */
    char     DialString[STRING_LEN];       /*!< The dial string to use for the connection */    
    char     VpnProfile[STRING_LEN];       /*!< A VPN profile to launch when a connection is created.  (RAS connections on Windows only)*/
    int32_t    UseVpn;                     /*!< Flag specifying that a VPN should be used. (RAS connections on Windows only)*/
    int32_t    IPAddress;                  /*!< Specifies a fixed IP address that should be used for the connection */
    int32_t    PrimaryDNSAddress;          /*!< Specifies a primary DNS that should be used for the connection (Windows only)*/
    int32_t    SecondaryDNSAddress;        /*!< Specifies a secondary DNS that should be used for the connection (Windows only)*/
    int32_t    PrimaryWINSAddress;         /*!< Specifies a primary WINS that should be used for the connection (Windows only)*/
    int32_t    SecondaryWINSAddress;       /*!< Specifies a secondary WINS that should be used for the connection (Windows only)*/
    int32_t    AuthenticationType;         /*!< The authentication type that should be used for the connection.  0 = Automatic, 1 = Password Authentication, 2 = Challenge and Response (Windows Only) */
}ConnectionSettingsStruct;

/// Defines a structure used to hold the data connection settings for devices that support LAN mode via RmNet/QMI.
typedef struct
{
	uint32_t  Mask;						/*!< bit mask that indicates which fields are to be set */
	uint32_t  TechPreference;			/*!< Mask Bit 0 - The Technology preference to use */
	uint32_t  ProfileId;				/*!< Mask Bit 1 - The profile index to use */
	uint32_t  PrimaryDNSAddress;        /*!< Mask Bit 2 - Specifies a primary DNS that should be used for the connection (Windows only)*/
	uint32_t  SecondaryDNSAddress;      /*!< Mask Bit 3 - Specifies a secondary DNS that should be used for the connection (Windows only)*/
	uint32_t  PrimaryWINSAddress;       /*!< Mask Bit 4 - Specifies a primary WINS that should be used for the connection (Windows only)*/
	uint32_t  SecondaryWINSAddress;     /*!< Mask Bit 5 - Specifies a secondary WINS that should be used for the connection (Windows only)*/
	char	  APN[STRING_LEN];          /*!< Mask Bit 6 - The APN to use for the connection */
	uint32_t  IPAddress;                /*!< Mask Bit 7 - Specifies a fixed IP address that should be used for the connection */
	uint32_t  AuthenticationType;       /*!< Mask Bit 8 - The authentication type that should be used for the connection.  0 = Automatic, 1 = Password Authentication, 2 = Challenge and Response */
	char      Username[STRING_LEN];     /*!< Mask Bit 9 -The user name to use for the connection */
	char      Password[STRING_LEN];     /*!< Mask Bit 10 -The password to use for the connection */
	uint32_t  mtu;					    /*!< OSX Only, can be used to adjust the MTU size for the ethernet connection */
}LanConnectionSettingsStruct;

/// Defines a structure used to hold the data connection settings for devices that support LAN mode via RmNet/QMI.
typedef struct
{
	uint32_t  Mask;						/*!< bit mask that indicates which fields are to be set */
	uint32_t  TechPreference;			/*!< Mask Bit 0 - The Technology preference to use */
	uint32_t  ProfileId;				/*!< Mask Bit 1 - The profile index to use */
	uint32_t  PrimaryDNSAddress;        /*!< Mask Bit 2 - Specifies a primary DNS that should be used for the connection (Windows only)*/
	uint32_t  SecondaryDNSAddress;      /*!< Mask Bit 3 - Specifies a secondary DNS that should be used for the connection (Windows only)*/
	uint32_t  PrimaryWINSAddress;       /*!< Mask Bit 4 - Specifies a primary WINS that should be used for the connection (Windows only)*/
	uint32_t  SecondaryWINSAddress;     /*!< Mask Bit 5 - Specifies a secondary WINS that should be used for the connection (Windows only)*/
	char	  APN[STRING_LEN];          /*!< Mask Bit 6 - The APN to use for the connection */
	uint32_t  IPAddress;                /*!< Mask Bit 7 - Specifies a fixed IP address that should be used for the connection */
	uint32_t  AuthenticationType;       /*!< Mask Bit 8 - The authentication type that should be used for the connection.  0 = Automatic, 1 = Password Authentication, 2 = Challenge and Response */
	char      Username[STRING_LEN];     /*!< Mask Bit 9 -The user name to use for the connection */
	char      Password[STRING_LEN];     /*!< Mask Bit 10 -The password to use for the connection */
	uint32_t  mtu;					    /*!< OSX Only, can be used to adjust the MTU size for the ethernet connection */
	char      IpPref;					/*!< used to set IP preference for interface.  Valid values are 0 for both IPv4 and IPv6, 4 for IPv4, 6 for IPv6 */
}LanConnectionSettingsStructEx;

/** \struct LanConnectionStatus
 This structure is used to hold data connection information retrieved directlly from the device, bypassing any operating system status.
 It is used with method LanConnectionStatus().
 NOTE that FW only supports 32 bit values for duration and byte counters
 This is available only on LTE multimode devices.
 */
typedef struct
{
	uint32_t version;
	uint32_t totalDuration;
	uint32_t activeDuration;
	uint32_t bytesIn;
	uint32_t bytesOut;
	uint32_t status;
}LanConnectionStatusStruct;

/// \enum NDISErrorType
/// This enum defines the possible NDIS related call end reasons returned in the error member of ConnectionStatusStruct.
typedef enum
{
    NDIS_ERROR_SUCCESS           = 0,           /*!< No Error */
    NDIS_ERROR_INTERNAL_ERROR    = 0x2000,      /*!< An unexpected error occurred on the device */
    NDIS_ERROR_INVALID_PROFILE,                 /*!< The specified configuration profile index does not exist */
    NDIS_ERROR_NO_EFFECT,                       /*!< The connection is already in progress */
    NDIS_ERROR_INVALID_TECH_PREF,               /*!< Invalid technology preference */
    NDIS_ERROR_INVALID_PDP_TYPE,                /*!< Invalid PDP type */
    NDIS_ERROR_UNKNOWN           = 0x2099,      /*!< An unexpected error code was returned */
    NDIS_ERROR_CALL_ENDED_UNSPECIFIED = 0x2100, /*!< The call failed with reason unknown */
    NDIS_ERROR_CALL_ENDED_USER_DISCONNECT,      /*!< User disconnected */
    NDIS_ERROR_CALL_ENDED_NO_SERVICE,           /*!< No Service */
    NDIS_ERROR_CALL_ENDED_FADE,                 /*!< Signal fade, call has ended abnormally */
    NDIS_ERROR_CALL_ENDED_REL_NORMAL,           /*!< Received release from Base Station – no reason given */
    NDIS_ERROR_CALL_ENDED_ACC_IN_PROG,          /*!< Access attempt already in progress */
    NDIS_ERROR_CALL_ENDED_ACC_FAIL,             /*!< Access failure for reason other than the above */
    NDIS_ERROR_CALL_ENDED_REDIR_HANDOFF,        /*!< Call rejected because of redirection or handoff */
    NDIS_ERROR_CALL_ENDED_CLOSE_IN_PROGRESS,    /*!< Call failed because close is in progress */
    NDIS_ERROR_CALL_ENDED_AUTH_FAILED           /*!< Authentication failed */
} NDISErrorType; 


/** \struct ConnectionStatusStruct
This structure is used to hold data connection information.
It is used with method GetConnectionStatus().
This structure is deprecated, ConnectionStatusEx should be used
*/
typedef struct
{
    int32_t    status;             /*!< The status of the connection as defined by the ConnectionState enum */
    uint32_t   bytesIn;   /*!< The number of bytes received during a connection */
    uint32_t   bytesOut;  /*!< The number of bytes sent during a connection */
    uint32_t   duration;  /*!< The elapsed time in seconds of a connection */
    int32_t    error;              /*!< A connection related error code, IE a RAS error code on Windows */
    char     ipAddress[18];      /*!< The IP Address of a connection */
}ConnectionStatusStruct;

/** \struct ConnectionStatusStructEx
This structure is used to hold data connection information.
This version has 64 bit types to avoid overflow issues after 4GB
It is used with method GetConnectionStatusEx().
*/
typedef struct
{
    int32_t    status;             /*!< The status of the connection as defined by the ConnectionState enum */
    uint64_t   bytesIn;   /*!< The number of bytes received during a connection */
    uint64_t   bytesOut;  /*!< The number of bytes sent during a connection */
    uint64_t   duration;  /*!< The elapsed time in seconds of a connection */
    int32_t    error;              /*!< A connection related error code, IE a RAS error code on Windows */
    char     ipAddress[18];      /*!< The IP Address of a connection */
}ConnectionStatusStructEx;

/** \struct ConnectionStatusStructEx
This structure is used for MultiMode devices and Firmware > 1.26.
This structure is used to hold data connection information.
This version has 64 bit types to avoid overflow issues after 4GB
It is used to store the bit mask for the event driven sdk

*/
typedef struct
{
	ConnectionStatusStructEx connstatus;	 /*!< Stores the struct ConnectionStatusStructEx */
	unsigned char	event_driven;			/*!< This is the mask for values defined in enum ConnectionStatusExMask.*/
}ConnectionStatusStructExWrapper;

/// \enum ConnectionModeType
/// This enum defines the possible connection mode values.
typedef enum
{
    CONNECTION_MODE_ETHERNET_MANUAL  = 0x00,     /*!< Manual ethernet mode. Connection is seen as a standard NIC and connection is controlled manually by the user (NDIS on Windows ) */   
    CONNECTION_MODE_ETHERNET_AUTO    = 0x01,     /*!< Automatic ethernet mode. Connection is seen as a standard NIC and connection is started automatically
                                                    whenever a device is inserted. (NDIS on Windows ) */
    CONNECTION_MODE_DIAL_UP          = 0x02,     /*!< Dial up networking mode (For example...RAS on Windows )*/ 
	CONNECTION_MODE_ETHERNET_AUTO_HOME	= 0x03, /* !< ethernet automatic home(no-roaming) mode */
    CONNECTION_MODE_MAX              = 0x04,     /*!< Dial up networking mode (For example...RAS on Windows )*/    
}ConnectionModeType;

/// \enum ConnectionState
/// This enum defines values for the state of the connection owned by the device.
typedef enum 
{
	CONN_STATE_DISCONNECTED    = 0,	/*!< The device is disconnected */
	CONN_STATE_CONNECTING      = 1,	/*!< A connection attempt has been made and the device is trying to connect   */
	CONN_STATE_AUTHENTICATING  = 2,	/*!< A connection attempt has been made and the device is authenticating user credentials */
	CONN_STATE_CONNECTED       = 3,	/*!< The device is connected   */
	CONN_STATE_DISABLED        = 4,	/*!< The device is disabled   */
	CONN_STATE_DISCONNECTING   = 5,	/*!< The device is disabled   */
	CONN_STATE_INITIALIZING    = 6,	/*!< The device is disabled   */
} ConnectionState;

/** \struct MultiModeIpAddress
This structure is used to hold the current internet protocol adresses for 4G capable multi-mode devices.
*/
typedef struct
{
    char             ipV4Address[18];   /*!< Standard IPv4 address */
    char			   ipV6Address[128];  /*!< The IPv6 address */
} MultiModeIPAddress;

/// \enum LanConnectionMode
/// This enum defines possible values for LAN connection mode of the device
typedef enum 
{
	LAN_CONN_MODE_MANUAL,
	LAN_CONN_MODE_AUTOMATIC,
	LAN_CONN_MODE_AUTOMATIC_HOME_ONLY
} LanConnectionMode;
/*******************************************************************
END Connectivity related types
********************************************************************/

typedef struct
{
    uint8_t   led_pattern;
    uint8_t   blink1_color;
    uint16_t  blink1_on_time;
    uint16_t  blink1_off_time;
    uint8_t   blink2_color;
    uint16_t  blink2_on_time;
    uint16_t  blink2_off_time;
}LedConfig4G;

#if !defined (_NVTL_GOBI_FW_DEFS_)
#define _NVTL_GOBI_FW_DEFS_
/*******************************************************************
Firmware related types
********************************************************************/
typedef struct
{
    char		*path; 
    uint32_t	firmwareID;
    uint32_t	technology;
    uint32_t	carrier;
    uint32_t	region;
    uint32_t	GPSCapability;
} FirmwareImageInfoStruct;
/*******************************************************************
END Firmware related types
********************************************************************/

#endif //_NVTL_GOBI_FW_DEFS_

#pragma pack(pop)


#endif //_NVTL_API_DEFS_H_
