
#ifndef DATABASE_H
#define DATABASE_H    
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>
#include "unistd.h"

#include <syslog.h>

#include <sys/wait.h>
#include <sys/time.h>
#define _XOPEN_SOURCE /* glibc2 needs this */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>													

#include "uci.h"


#define	LOGOPTS		(LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)
#ifdef MEMWATCH
    #include "memwatch.h"
#endif

#ifndef bool
    #define bool int
#endif
#ifndef true
    #define true 1
#endif
#ifndef false
   #define false 0/*-1 is bad*/
#endif   
#ifndef false2
   #define false2 -2
#endif     
#ifndef false3
   #define false3 -3
#endif    
#ifndef false4
   #define false4 -4    
#endif
#ifndef FALSE
   #define FALSE false	/*-1 is bad*/
#endif
#ifndef TRUE
   #define TRUE true
#endif

#define SDP_DISCOVER_STATUS_DISCOVABLE 0x01
#define SDP_CMD_INQUIRY	 0x00
#define SDP_RESPONSE_INQUIRY	 (SDP_CMD_INQUIRY+1)


#define ARG_NUMBER 16
#define UserDB_Buff1_len 40
#define UserDB_Buff2_len 62



#define MENUS_NUMBER 1
#define ITEM_MAX_NUMBER 22
#define BUFF_LEN 30
#define USERDB_NAME_NUMBER 10

#define ADVANCED_EUR_CONF 0
    enum
    {
        AERC_ITEM_RR=0,            //event reporting off or on, cx can enable at most four reportings
        AERC_ITEM_REIP0,           //a remote ip address for each remote reporting
        AERC_ITEM_REIP1,
        AERC_ITEM_REIP2,
        AERC_ITEM_REIP3,

        AERC_ITEM_RPORT0,           //a remote port for each remote reporting
        AERC_ITEM_RPORT1,
        AERC_ITEM_RPORT2,
        AERC_ITEM_RPORT3,
        AERC_ITEM_SEC0,            //trigger timer, unit sec, for each remote report

        AERC_ITEM_SEC1,
        AERC_ITEM_SEC2,
        AERC_ITEM_SEC3,
        /* cx can select at most three message types in one reporting, a char represents a type, right now we occupy 4 chars for one reporting */
        AERC_ITEM_MSS_TYPE0,
        AERC_ITEM_MSS_TYPE1,

        AERC_ITEM_MSS_TYPE2,
        AERC_ITEM_MAS0,
        AERC_ITEM_MAS1,
        AERC_ITEM_MAS2,
        AERC_ITEM_MAS3,
    };


#define RADIO_STATUS_NUM 22
enum RADIO_SYMBOL {
    APN_POS=0,
    ACTIVITY_STATUS_POS,
    OPERATOR_POS,
    NETWORK_ROAMING_POS,
    CELLID_POS,
    SERVICE_STATUS,
    CHANNEL_NUMBER,
    FREQUENCY_BAND,
    ECNR,
    RSSI_POS,
    RSCP_POS,
    RADIO_TEMPERATURE,
    RADIO_VOLTAGE,

    IMEI_POS,
    IMSI_POS,
    SIM_STATUS_POS,
    ICCID_POS,
    PHONENUMBER,
    LOCAL_IP_POS,
    DNS1,
    DNS2,
};



extern struct uci_context *ctx;

extern char *EURDStatisticFile;
extern char *ConfigDir;
extern char *EURDConfigFileTemp;
extern char *EURDConfigFile;

extern char *VersionFile;
extern char *BuilttimeFile;
extern char *dnsdefFile;



extern char *ifname_bridge;
extern char *ifname_eth0;
extern char *ifname_wan;
extern char *ifname_eth1;
extern char *ifname_carrier;
extern char *ifname_eth2;
extern char *ifname_radio;
extern char *ifname_radio_mon;


extern char DataBase1_Buff[MENUS_NUMBER][ITEM_MAX_NUMBER][BUFF_LEN];
extern char Item_Keywords[MENUS_NUMBER][ITEM_MAX_NUMBER][40];

extern int SubProgram_Start(char *pszPathName, char *pszArguments);
extern bool get_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value);
extern bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char*section_name, char*option_name, char *value);


extern bool UserDB_read_file(char *UserDB ,char* key_word,char* read_buff,int read_buff_len);

//following is from UCI connection.
extern bool UserDB_read(unsigned int *UserDB ,char* key_word,char* read_buff,int read_buff_len);
extern bool UserDB_write(unsigned int *UserDB ,char* key_word,int key_word_len,char* write_value,int write_value_len);

extern bool read_Database(int menu);
extern bool write_Database(int menu);

extern int fetch_Local_IP_ADDR(char*ifname,char*addr);
extern int fetch_Local_IP_MASK(char *ifname, char *mask);
extern bool fetch_Local_IP_MAC(char *ifname, char *mac);


extern int sendLocationRequest(unsigned int MNC,unsigned int MCC, unsigned int CID, unsigned int LAC);


#endif











/////////////////////////////////END////////////////////////////////////////////////








