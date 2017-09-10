
#ifndef DATABASE_H
#define DATABASE_H    
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>
#include "unistd.h"
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
#include "uci.h"


#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)   

#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#ifndef bool
    #define bool int
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

#endif

    #ifndef FALSE
        #define FALSE false	/*-1 is bad*/
    #endif
    #ifndef TRUE
        #define TRUE true
    #endif

#define SUB_MENU_COM1    0
#define COM1_ITEM_PORT_STATUS 0
#define COM1_ITEM_CHANNEL_MODE 1 
#define COM1_ITEM_BAUD_RATE 2
#define COM1_ITEM_DATA_FORMAT 3 
#define COM1_ITEM_FLOW_CONTROL 4
#define COM1_ITEM_CTS_BEFORE_FRAME_DELAY 5
#define COM1_ITEM_CTS_AFTER_FRAME_DELAY 6  
#define COM1_ITEM_DATA_MODE 7
#define COM1_ITEM_CHARACTER_TIMEOUT 8      
#define COM1_ITEM_MAXIMUM_PACKET_SIZE 9
#define COM1_ITEM_QOS 10
#define COM1_ITEM_NO_CONNECTION_DATA_INTAKE 11
#define COM1_ITEM_MODBUS_MODE 12
#define COM1_ITEM_IP_PROTOCOL_CONFIG 13

#define SUB_MENU_COM2    1

#ifdef VIP4G

#define COM2_ITEM_PORT_STATUS 0
#define COM2_ITEM_CHANNEL_MODE 1 
#define COM2_ITEM_BAUD_RATE 2
#define COM2_ITEM_DATA_FORMAT 3 
#define COM2_ITEM_FLOW_CONTROL 4       
#define COM2_ITEM_CTS_BEFORE_FRAME_DELAY 5
#define COM2_ITEM_CTS_AFTER_FRAME_DELAY 6  
#define COM2_ITEM_DATA_MODE 7
#define COM2_ITEM_CHARACTER_TIMEOUT 8      
#define COM2_ITEM_MAXIMUM_PACKET_SIZE 9
#define COM2_ITEM_QOS 10
#define COM2_ITEM_NO_CONNECTION_DATA_INTAKE 11
#define COM2_ITEM_MODBUS_MODE 12
#define COM2_ITEM_IP_PROTOCOL_CONFIG 13

#else

#define COM2_ITEM_PORT_STATUS 0
#define COM2_ITEM_BAUD_RATE 1
#define COM2_ITEM_DATA_FORMAT 2
#define COM2_ITEM_DATA_MODE 3
#define COM2_ITEM_CHARACTER_TIMEOUT 4      
#define COM2_ITEM_MAXIMUM_PACKET_SIZE 5
#define COM2_ITEM_QOS 6  
#define COM2_ITEM_NO_CONNECTION_DATA_INTAKE 7
#define COM2_ITEM_MODBUS_MODE 8
#define COM2_ITEM_IP_PROTOCOL_CONFIG 9

#endif


#define SUB_MENU_USB    2


#define COM1_SUB_MENU_NUMBER 8 

#define COM1_SUB_MENU_TCLIENT 3
#define	TCLIENT_ITEM_0 0
#define	TCLIENT_ITEM_1 1
#define	TCLIENT_ITEM_2 2	

#define COM1_SUB_MENU_TSERVER 4

#define	TSERVER_ITEM_PULLING_MODE 0
#define	TSERVER_ITEM_PULLING_TIMEOUT 1 
#define	TSERVER_ITEM_0 2
#define	TSERVER_ITEM_1 3 


#define COM1_SUB_MENU_TCLIENTSERVER 5

#define	TCLIENTSERVER_ITEM_0 0
#define	TCLIENTSERVER_ITEM_1 1
#define	TCLIENTSERVER_ITEM_2 2  
#define	TCLIENTSERVER_PULLING_MODE  3
#define	TCLIENTSERVER_PULLING_TIMEOUT 4 
#define	TCLIENTSERVER_ITEM_3 5
#define	TCLIENTSERVER_ITEM_4 6	

#define COM1_SUB_MENU_UPTOP 6
#define	UPTOP_ITEM_0 0
#define	UPTOP_ITEM_1 1
#define	UPTOP_ITEM_2 2	

#define COM1_SUB_MENU_UPTOMP_AS_P 7
#define	UPTOMP_AS_P_ITEM_0 0
#define	UPTOMP_AS_P_ITEM_1 1
#define	UPTOMP_AS_P_ITEM_2 2	
#define	UPTOMP_AS_P_ITEM_3 3

#define COM1_SUB_MENU_UPTOMP_AS_MP 8
#define	UPTOMP_AS_MP_ITEM_0 0
#define	UPTOMP_AS_MP_ITEM_1 1
#define	UPTOMP_AS_MP_ITEM_2 2	
#define	UPTOMP_AS_MP_ITEM_3 3

#define COM1_SUB_MENU_UMPTOMP 9
#define	UMPTOMP_SEND_MULTICAST_IP 0
#define	UMPTOMP_SEND_MULTICAST_PORT 1
#define	UMPTOMP_SEND_MULTICAST_TTL 2  
#define	UMPTOMP_LISTEN_MULTICAST_IP 3
#define	UMPTOMP_LISTEN_MULTICAST_PORT 4

#define COM1_SUB_MENU_SMTP  10
#define SMTP_ITEM_MESSAGE_TITLE 0
#define SMTP_ITEM_HOST 1
#define SMTP_ITEM_USERNAME 2
#define SMTP_ITEM_PASSWORD 3  
#define SMTP_ITEM_RECEIVER 4
#define SMTP_ITEM_BUFF_SIZE 5 
#define SMTP_ITEM_TIME_OUT 6  
#define SMTP_ITEM_TRANSFER_MODE 7  

#define COM1_SUB_MENU_C1222  11
#define C1222_ITEM_REGISTER 0  
#define C1222_ITEM_LOG_NET_COM 1
#define C1222_ITEM_DIFF_SOCKET 2 
#define C1222_ITEM_REASSEMBLY 3  
#define C1222_ITEM_HOST_SERVER_IP 4  
#define C1222_ITEM_HOST_SERVER_PORT 5  
#define C1222_ITEM_LOCAL_PORT 6  


#define COM2_SUB_MENU_NUMBER 7

#define COM2_SUB_MENU_TCLIENT 12

#define COM2_SUB_MENU_TSERVER 13

#define COM2_SUB_MENU_TCLIENTSERVER 14

#define COM2_SUB_MENU_UPTOP 15

#define COM2_SUB_MENU_UPTOMP_AS_P 16

#define COM2_SUB_MENU_UPTOMP_AS_MP 17

#define COM2_SUB_MENU_UMPTOMP 18
 
#define COM2_SUB_MENU_SMTP  30 /* new add com2 smtp, menu number: 30, members of COM2_SUB_MENU_SMTP already defined above*/

#define COM1_SUB_MENU_SMS  31 /* new add com2 smtp, menu number: 31, members of COM2_SUB_MENU_SMS defined below  */
#define COM2_SUB_MENU_SMS  32 /* new add com2 smtp, menu number: 32, members of COM2_SUB_MENU_SMS defined below  */
#define SMS_ITEM_PHONE1     0
#define SMS_ITEM_PHONE2     1
#define SMS_ITEM_PHONE3     2
#define SMS_ITEM_PHONE4     3
#define SMS_ITEM_PHONE5     4
#define SMS_ITEM_MMS        5
#define SMS_ITEM_TIMEOUT    6
#define SMS_ITEM_ACL        7
#define SMS_ITEM_GETSMS_MODE    8
#define SMS_ITEM_SENDER         9
#define SMS_ITEM_SUBJECT        10
#define SMS_ITEM_DT             11
#define SMS_ITEM_LOC            12
#define SMS_ITEM_SM_TOTAL       13
#define SMS_ITEM_SM_USED        14
#define SMS_ITEM_ME_TOTAL       15
#define SMS_ITEM_ME_USED        16


#define USB_SUB_MENU_DATA_MODE 19
#define USB_DATA_MODE_PORT_STATUS 0
#define USB_DATA_MODE_BAUD_RATE 1     
#define USB_DATA_MODE_DATA_FORMAT 2
#define USB_DATA_MODE_DATA_MODE 3
#define USB_DATA_MODE_CHARACTER_TIMEOUT 4      
#define USB_DATA_MODE_MAXIMUM_PACKET_SIZE 5
#define USB_DATA_MODE_QOS 6  
#define USB_DATA_MODE_NO_CONNECTION_DATA_INTAKE 7
#define USB_DATA_MODE_MODBUS_MODE 8
#define USB_DATA_MODE_IP_PROTOCOL_CONFIG 9


#define USB_SUB_MENU_NUMBER 7

#define USB_SUB_MENU_TCLIENT 20

#define USB_SUB_MENU_TSERVER 21

#define USB_SUB_MENU_TCLIENTSERVER 22

#define USB_SUB_MENU_UPTOP 23

#define USB_SUB_MENU_UPTOMP_AS_P 24

#define USB_SUB_MENU_UPTOMP_AS_MP 25

#define USB_SUB_MENU_UMPTOMP 26 

#define COM1_MODBUS_CONF 27
#define COM1_MODBUS_ITEM_STATUS 0
#define COM1_MODBUS_ITEM_PROTECTION_STATUS 1
#define COM1_MODBUS_ITEM_PROTECTION_KEY 2

#define COM2_MODBUS_CONF 28
#define COM2_MODBUS_ITEM_STATUS 0
#define COM2_MODBUS_ITEM_PROTECTION_STATUS 1
#define COM2_MODBUS_ITEM_PROTECTION_KEY 2

#define USB_MODBUS_CONF 29
#define USB_MODBUS_ITEM_STATUS 0
#define USB_MODBUS_ITEM_PROTECTION_STATUS 1
#define USB_MODBUS_ITEM_PROTECTION_KEY 2


//#define MENUS_NUMBER 30
/* Add com2 smtp, menu number: 30 */
/* Add com1 sms, menu number:  31 */
/* Add com2 sms, menu number:  32 */
#define MENUS_NUMBER 33 
#define ITEM_MAX_NUMBER 20

#define FIRST_CHARACTOR 0
#define COM_STATISTICS_CLIENT_LINK_NUM 18


#define OPTION_MAX_NUMBER 18
#define BUFF_LEN 31/* set to 31 so that user can input 30 the last one prob is \n*/           
#define DATABASE_LEN    MENUS_NUMBER *ITEM_MAX_NUMBER* BUFF_LEN 

#define UserDB_Buff1_len 40
#define UserDB_Buff2_len 32

extern struct uci_context *ctx;
extern char *Com1StatisticFile;
extern char *Com2StatisticFile;
extern char *COM2EnableFile;
extern char *USBStatisticFile;
extern char *USBEnableFile;
extern char* UserDB;
extern char* FlashUserDB;

extern char *ifname_bridge;

extern char DataBase1_Buff[MENUS_NUMBER][ITEM_MAX_NUMBER][BUFF_LEN];
extern char Item_Keywords[MENUS_NUMBER][ITEM_MAX_NUMBER][40];

extern int SubProgram_Start(char *pszPathName, char *pszArguments);

extern bool get_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value);
extern bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value);

extern bool UserDB_read( char* UserDB, char* key_word, char* read_buff, int read_buff_len);
extern bool UserDB_write( char *UserDB ,char* key_word, int key_word_len, char* write_value, int write_value_len);

extern bool read_Database(int menu);
extern bool write_Database(int menu);
             
extern int fetch_Local_IP_ADDR(char *ifname, char *addr);
#endif
