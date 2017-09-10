
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


#define SDP_SERVER_DEFAULT_LISTON_PORT 20087
#define SDP_DISCOVER_STATUS_DISABLE 0x00
#define SDP_DISCOVER_STATUS_DISCOVABLE 0x01
#define SDP_DISCOVER_STATUS_CHANGABLE 0x02

#define SDP_CMD_INQUIRY	 0x00
#define SDP_CMD_CHANGE_IP 	0x02
#define SDP_CMD_REBOOT 	0x04
#define SDP_CMD_TX_CTRL	 0x06
#define SDP_CMD_TX_STATUS 	0x08
#define SDP_CMD_PROSOFT_INQUIRY 	0x20

static char *ifname_bridge_oldvip="br0";											   
static char *ifname_bridge="br-lan";											   
static char *ifname_wireless = "wlan0";
static char *ifname_ethernet = "eth0";

static char *network_conffile="network";
static char *network_lan_section="lan";
static char *WirelessNetworkConnectionStatusFile="/var/run/netdiscovery";

extern struct uci_context *ctx;
extern int SubProgram_Start(char *pszPathName, char *pszArguments);

extern bool get_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value);
extern bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value);

extern bool uci_section_read( char*conf_fname, char* sect_name, char* key_word, char* read_buff);

extern bool uci_section_write( char*conf_fname, char* sect_name, char* key_word, char* write_value);

extern int fetch_Local_IP_ADDR(char *ifname, char *addr);
extern bool fetch_Local_IP_MAC(char *ifname, char *mac);
#endif  /* DATABASE_H*/
