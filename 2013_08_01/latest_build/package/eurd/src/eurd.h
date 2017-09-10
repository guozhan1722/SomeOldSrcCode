#ifndef EURD_H
#define EURD_H

#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <netinet/ether.h>
#include "database.h"

#define REPORT_NUMBER 4
#define EUR_MSS_TYPE_NUMBER 3

#define MODEMINFO 1
#define CARRINFO 4
#define WANINFO 16
#define SDPINFO 64
#define JSONINFO 32768

//ETHINFO, CARRINFO, RADINFO/USBINFO(3G), COM0INFO, IOINFO/COM2INFO(3G) 
#define ETHINFO 1
#define CARINFO 2
//#define USBINFO 4
#define RADINFO 4
#define COM0INFO 8
#define IOINFO 16
//#define COM1INFO 16
#define VPNINFO 32

#define EURD_OK 0
#define EURD_ERROR -1

#define EURD_SCHEDULED 1
#define EURD_WAITTING 2
#define EURD_NONE 0

#define EURD_MAS_CONF_CHANGED 1
#define EURD_MAS_CONF_NONE 0

#define MEP_PACKAGE_HEADER 20
#define MEP_PACKAGE_LENGTH 18
#define EURD_TAILINFO_HEADER 2
#define EURD_CARRINFO_HEADER 2
#define EURD_WANINFO_HEADER 2
#define EURD_MODEMINFO_HEADER 2
#define IPn3G_FLAGE 1035
#define VIP4G_FLAGE 1036
#define MAN_VERSION 1
#define MAN_PACKAGE_HEADER 50
#define MAN_PACKAGE_LENGTH 39
#define MAN_PLAYLOAD_HEADER 3
#define TRAFFIC_LENGTH 32

#define RADIO_POS 0
#define ETH_POS 1
#define CARRIER_POS 2
#define USB_POS 3
#define COM_POS 4

#define RADIO_3G 0x0
#define ETH_3G 0x1
#define CARRIER_3G 0x1
#define USB_3G 0x1
#define COM_3G 0x2

union __hton16_struct
{
    char u_c[2];
    uint16_t u_i;
};
union __hton32_struct
{
    char u_c[4];
    uint32_t u_i;
};
union __hton64_struct
{
    char u_c[8];
    uint64_t u_i;
};

union __hexint_struct
{
    char c[4];
    int i;
};

typedef struct mod_list
{
    uint8_t eth;
    uint8_t carrier;
    uint8_t radio;
    uint8_t com;
    uint8_t ioport;
    char vpn[200];
}mod_list;

typedef struct eur_def
{
    uint64_t modem_id;
    uint16_t mess_type_mask;
    uint8_t mac[6];
    uint16_t product_flage;
    uint16_t pack_len;

    	int sockfd;
    	struct sockaddr_in addr_remote;    // remote host address information
    	int state;
    	int scheduled;
    	uint16_t interval_tm;
    	uint16_t min_interval;
    	time_t last_action;
}eur_def;

typedef struct _EUR_radios_data_t
{
    char mid[5];
    char ipAddr[30];  //ipv6?
    char macAddr[40];   //long 
    char networkName[30];
    char operationMode[30];
    char searchMode[30];
    char authenticationKey[30];
    char encryptionType[30];
    char rssi[40];
    char childRssi[30];
    char coreTemperature[10];
    char supplyVoltage[10];
    char vswr[30];
    char linkRate[20];
    char channelNumber[20];
    char txRate[20];
    char channelBandwidth[20];
    char receivedBytes[20];
    char receivedPackets[20];
    char transmittedBytes[20];
    char transmittedPackets[20];
}_EUR_radios_data_t;

typedef struct _EUR_eths_data_t
{
    char mid[5];
    char ipAddr[30];
    char ipMask[30];
    char macAddr[30];
    char receivedBytes[20];
    char receivedPackets[20];
    char transmittedBytes[20];
    char transmittedPackets[20];
}_EUR_eths_data_t;

typedef struct _EUR_carrs_data_t
{
    char mid[5];
    char ipAddr[30];
    char dns1[30];
    char dns2[30];
    char rssi[20];
    char ecNo[20];
    char rscp[20];
    char frequencyBand[20];
    char threeGNetwork[20];
    char serviceType[30];
    char channelNumber[20];
    char simCardNumber[30];
    char phoneNumber[20];
    char coreTemperature[20];
    char supplyVoltage[20];
    char apn[30];
    char mnc[10];
    char mcc[10];
    char lac[10];
    char cellId[20];
    char activityStatus[20];
    char homeRoaming[20];
    char imei[30];
    char imsi[30];
    char latitude[20];
    char longitude[20];
    char radius[30];
    char RSRP[20];//if LTE 4G
    char RSRQ[20];//if LTE 4G
    char receivedBytes[20];
    char receivedPackets[20];
    char transmittedBytes[20];
    char transmittedPackets[20];
}_EUR_carrs_data_t;
char s_carrs_geodata;

typedef struct _EUR_usbs_data_t
{
    char mid[5];
    char receivedBytes[20];
    char receivedPackets[20];
    char transmittedBytes[20];
    char transmittedPackets[20];
}_EUR_usbs_data_t;


typedef struct _EUR_coms_data_t
{
    char mid[5];
    char receivedBytes[20];
    char receivedPackets[20];
    char transmittedBytes[20];
    char transmittedPackets[20];
}_EUR_coms_data_t;

typedef struct _EUR_io_data_t
{
    char InputStatus[10];
    char OutputStatus[10];
}_EUR_io_data_t;

typedef struct _EUR_gps_data_t
{
    uint8_t gps_flag;
    char latitude[20];
    char longitude[20];
}_EUR_gps_data_t;

typedef struct _EUR_vpncls_data_t
{
    char vpn_key[3];//s_ c_

    char mid[5];
    char name[30];
    char protocol[20];
    char inf[20];
    char connected[20];
    char ipAddr[30];
    char gateway[30];
    char receivedBytes[20];
    char receivedPackets[20];
    char transmittedBytes[20];
    char transmittedPackets[20];
}_EUR_vpncls_data_t;


typedef struct _EUR_device_data_t
{
    char deviceName[256];
    char productName[30];
    char imageName[30];
    char macAddr[30];
    char hardwareVersion[10];
    char softwareVersion[10];
    char repInterval[10];
    char uptime[15];
    char webUIRepId[5];
    char httpPort[6];
    char modulesList[20];
    char vpnClientList[160];
}_EUR_device_data_t;
int num_radio=0;

typedef struct _EUR_conf_data_t
{
    char repServer[30];
    char repPort[10];
    char repInterval[10];
//    char encryption[];//{"enabled":true,"algorithm":"DES","secretKey":[0xF1,0x78,0x22,0x23,0x09,0xAD,0x9C,0x53]}
//    char compression[];//{"enabled":true,"algorithm":"DEFLATE"}
    char webUIRepId[5];
}_EUR_conf_data_t;




char *eurdscriptFile="/bin/eurdscript";
char *iw_statefile="/tmp/run/eurd_iw_state";


struct eur_def eur_buff;
int enabled_report;
char network_name[32];
#define MAX_ETHS_NUMBER 2
#define EUR_SET_ETH1_ONOROFF 1
#define MAX_CARRS_NUMBER 1
#define MAX_USBS_NUMBER 1
#define MAX_RADIOS_NUMBER 1
#define MAX_COMS_NUMBER 1
#define MAX_IO_NUMBER 1
#define MAX_VPNCLS_NUMBER 16
struct _EUR_eths_data_t *p_eur_eths_buff[MAX_ETHS_NUMBER];
struct _EUR_carrs_data_t *p_eur_carrs_buff[MAX_CARRS_NUMBER];
struct _EUR_radios_data_t *p_eur_radios_buff[MAX_RADIOS_NUMBER];
struct _EUR_usbs_data_t *p_eur_usbs_buff[MAX_USBS_NUMBER];  //no use in 4G
struct _EUR_coms_data_t *p_eur_coms_buff[MAX_COMS_NUMBER];
struct _EUR_io_data_t *p_eur_io_buff[MAX_IO_NUMBER];
struct _EUR_vpncls_data_t *p_eur_vpncls_buff[MAX_VPNCLS_NUMBER];
struct _EUR_device_data_t eur_device_buff;
struct _EUR_gps_data_t eur_gps_buff;
struct _EUR_conf_data_t eur_conf_buff;
#define EUR_SET_ON 1
#define EUR_SET_OFF 0
char set_eur_radios_valid[MAX_RADIOS_NUMBER];//each position has its special name definition
char set_eur_eths_valid[MAX_ETHS_NUMBER];
char set_eur_carrs_valid[MAX_CARRS_NUMBER];
char set_eur_usbs_valid[MAX_USBS_NUMBER];  //no use in 4G
char set_eur_coms_valid[MAX_COMS_NUMBER];
char set_eur_io_valid[MAX_IO_NUMBER];
char set_eur_vpncls_valid[MAX_VPNCLS_NUMBER];

char last_carrierIP_str[32]="N/A";
int IP_changed_sign=2;

#endif

