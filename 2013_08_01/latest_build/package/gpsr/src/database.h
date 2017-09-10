
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
#include <dirent.h>  

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



#define MENUS_NUMBER 2
#define ITEM_MAX_NUMBER 29
#define BUFF_LEN 64


#define ADVANCED_GPS_CONF_REPORT 0
#define ADVANCED_GPS_CONF_REPORT_NUM 28
enum
  {
      AGCR_ITEM_RR=0,            //remote reporting off or on, cx can enable at most four reportings
      AGCR_ITEM_REIP0,           //a remote ip address for each remote reporting
      AGCR_ITEM_REIP1,
      AGCR_ITEM_REIP2,
      AGCR_ITEM_REIP3,
      AGCR_ITEM_RPORT0,           //a remote port for each remote reporting
      AGCR_ITEM_RPORT1,
      AGCR_ITEM_RPORT2,
      AGCR_ITEM_RPORT3,
      /*trigger timer, a char represents to turn on or off a timer which is used by a remote reporting, right now we occupy 4 chars for four reporting */
      AGCR_ITEM_TIMER,
      AGCR_ITEM_SEC0,            //trigger timer, unit sec, for each remote report
      AGCR_ITEM_SEC1,
      AGCR_ITEM_SEC2,
      AGCR_ITEM_SEC3,
      /*distance trigger, a char represents to turn on or off a distance which is used by a remote reporting, right now we occupy 4 chars for four reporting */
      AGCR_ITEM_DIS,
      AGCR_ITEM_METER0,      //trigger distance, unit meter, for each remote report
      AGCR_ITEM_METER1,
      AGCR_ITEM_METER2,
      AGCR_ITEM_METER3,
      AGCR_ITEM_TRIGGER_CONF,    //triggered by a combination of time AND/OR distance, a char represents a condition for a remote reporting
      /* cx can select at most four gps message types in one reporting, a char represents a type, right now we occupy 4 chars for one reporting */
      AGCR_ITEM_MSS_TYPE0,
      AGCR_ITEM_MSS_TYPE1,
      AGCR_ITEM_MSS_TYPE2,
      AGCR_ITEM_MSS_TYPE3,
      AGCR_Local_Stream_Set0,
      AGCR_Local_Stream_Set1,
      AGCR_Local_Stream_Set2,
      AGCR_Local_Stream_Set3,
};


#define GPS_SUB_MENU_SMTP  1
#define GPS_SUB_MENU_SMTP_NUM 24
enum{
    SMTP_ITEM0_MESSAGE_TITLE=0,
    SMTP_ITEM0_HOST,
    SMTP_ITEM0_USER_NAME,
    SMTP_ITEM0_PASSWORD,
    SMTP_ITEM0_RECEIVER,
    SMTP_ITEM0_BUFF_SIZE,
    SMTP_ITEM1_MESSAGE_TITLE,
    SMTP_ITEM1_HOST,
    SMTP_ITEM1_USER_NAME,
    SMTP_ITEM1_PASSWORD,
    SMTP_ITEM1_RECEIVER,
    SMTP_ITEM1_BUFF_SIZE,
    SMTP_ITEM2_MESSAGE_TITLE,
    SMTP_ITEM2_HOST,
    SMTP_ITEM2_USER_NAME,
    SMTP_ITEM2_PASSWORD,
    SMTP_ITEM2_RECEIVER,
    SMTP_ITEM2_BUFF_SIZE,
    SMTP_ITEM3_MESSAGE_TITLE,
    SMTP_ITEM3_HOST,
    SMTP_ITEM3_USER_NAME,
    SMTP_ITEM3_PASSWORD,
    SMTP_ITEM3_RECEIVER,
    SMTP_ITEM3_BUFF_SIZE,
};



#define SUB_MENU_SYSTEM_CFG    0
#define SYSTEM_CFG_ITEM_DEVICE_NAME 0  




extern struct uci_context *ctx;

extern char *ConfigDir;
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

extern char DataBase2_Buff[1][1][BUFF_LEN];
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

extern int gpsd_pid_check();

#endif











/////////////////////////////////END////////////////////////////////////////////////








