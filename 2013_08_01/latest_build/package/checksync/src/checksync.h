#ifndef CHECKSYNC_H_
#define CHECKSYNC_H_

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/termios.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <sys/sysinfo.h>
#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <pwd.h>
#include <netinet/tcp.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include "uci.h"


//#define DEBUG_PRINT_ENABLED 1  // uncomment to enable DEBUG statements

#if DEBUG_PRINT_ENABLED
#define DEBUG printf
#else
#define DEBUG(format, args...) ((void)0)
#endif



char *devRadioS4 ="/dev/ttyS4";
char *devRadioS5 ="/dev/ttyS5";
char *radioavparamfile="/var/run/radioavparam";
char *sysrebootlogfile="/etc/rebootlog";

#define CELL_CONSOLE_COMMON_BAUD 115200
#define CELL_CONSOLE_DEFAULT_BAUD 19200
#define max_return_len  4096

#define BROKEN_CONNECTION_RESET_LIMIT 120
#define INIT_IDLE_LIMIT               120

#define SNMP_TRAP_RESEND_INTERVAL 30
#define SNMP_TRAP_RSSI_THRESHOLD_LOW -99
//#define SNMP_TRAP_RSSI_THRESHOLD_LOW -90
#define SNMP_TRAP_VOLTAGE_THREASHOLD_LOW 7
#define SNMP_TRAP_VOLTAGE_THREASHOLD_HIGH 36
#define DEFAULT_FORCE_SWITCH_TIME 30
#define HISTORY_HOURS 72
#define HISTORY_HOURS_24 24
#define HISTORY_HOURS_M0 (HISTORY_HOURS)
#define HISTORY_HOURS_M1 (HISTORY_HOURS-1)
#define HISTORY_HOURS_MR (HISTORY_HOURS)

#define HISTORY_MINUTES_60 60
#define HISTORY_MINUTES_MR (HISTORY_MINUTES_60)
#define MINUTES_INTERVAL 15 
#define POINTS_IN_HOUR (60/MINUTES_INTERVAL)
#define HOUR_HISTORY_POINTS (HISTORY_HOURS*POINTS_IN_HOUR) /*less than 1024*/
#define SECONDS_INTERVAL 15 
#define POINTS_IN_MINUTE (60/SECONDS_INTERVAL)
#define MINUTE_HISTORY_POINTS (60*POINTS_IN_MINUTE) /*less than 1024*/

#define ONE_HOUR_SECONDS 3600
#define UPDATE_OFF_SET_SECOND 100
#define REPORT_INTERVAL 3600 /*1 HOUR=3600s*/
#define UPDATE_INTERVAL 10 /*update every 10 seconds*/
#define REBOOT_REPORT_DELAY 300 /*5 Minutes=300s*/

#define MIN_RSSI (-113)
#define DEFAULT_WRONG_RSSI (-120)

#define MIN_ECNO (-64)
#define DEFAULT_WRONG_ECNO (-64)

#define DEFAULT_TEMPER (-40.0)
#define DEFAULT_WRONG_TEMPER (-40.0)
#define DEFAULT_VOLTAGE (0.0)
#define DEFAULT_WRONG_VOLTAGE (0.0)


static char *confile="/etc/config/snmpd";
static char *ioport_confile="/etc/config/ioports";
char *VIP4G_status_file="/var/run/VIP4G_status";


char *snmptrap_script = "/bin/snmptrap.sh";


#ifdef WRITE_HISTORY
    int minrssiH = -20;
    int maxrssiH = -110;
    int averssiH = -80;
    int minrssiHistoryH[HOUR_HISTORY_POINTS+1];/*5 days History every perhour*/
    int maxrssiHistoryH[HOUR_HISTORY_POINTS+1];/*5 days History every perhour*/
    int averssiHistoryH[HOUR_HISTORY_POINTS+1];/*5 days History every perhour*/

    int minrssiM = DEFAULT_WRONG_RSSI;
    int maxrssiM = DEFAULT_WRONG_RSSI;
    int averssiM = DEFAULT_WRONG_RSSI;
    int minrssiHistoryM[MINUTE_HISTORY_POINTS+1];/*every minute*/
    int maxrssiHistoryM[MINUTE_HISTORY_POINTS+1];/*every minute*/
    int averssiHistoryM[MINUTE_HISTORY_POINTS+1];/*every minute*/
#endif


/*
    Function prototype
*/



/**********************************************************************************
   Function:    int main(void) 
   Input:       void
   Output:      None
   Return:      int   
   Note:	read database and config com port and according to selected IP protocol 
        to process.        
**********************************************************************************/

static bool get_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_element *e= NULL;
    struct uci_ptr ptr;
    struct uci_option *p_option = NULL;

    //printf("%s option_name: %s\n",__func__,option_name);
    strcpy(value, "");

    tuple = malloc( strlen(package_name) + strlen(section_name) + strlen(option_name)+ 3 ); /* "." and trailing \0 */

    if (!tuple) return false;

    strcpy(tuple, package_name);
    strcat(tuple, ".");
    strcat(tuple, section_name);
    strcat(tuple, ".");
    strcat(tuple, option_name);

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK)
        {

        e = ptr.last;

        if ( (ptr.flags & UCI_LOOKUP_COMPLETE) &&
             (e->type == UCI_TYPE_OPTION) )
            {
            p_option = ptr.o;
            strcpy(value, p_option->v.string);
            //printf("%s find %s\n",__func__, value);
            free(tuple);
            return true;
            }
        }

    free(tuple);

    return false;
}


/**********************************************************************************
   Function:  bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

static bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_ptr ptr;

    tuple = malloc( strlen(package_name) + strlen(section_name) + strlen(option_name) + strlen(value) + 4 );    /* "." "=" and trailing \0 */

    if (!tuple) return false;

    strcpy(tuple, package_name);
    strcat(tuple, ".");
    strcat(tuple, section_name);
    strcat(tuple, ".");
    strcat(tuple, option_name);
    strcat(tuple, "=");
    strcat(tuple, value);

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK)
        {
        if ( UCI_OK == uci_set(ctx, &ptr) )
            {
            uci_save(ctx, ptr.p);
         //   uci_commit(ctx, &ptr.p, false);
            }

        free(tuple);
        return true;
        }

    free(tuple);

    return false;
}

/**********************************************************************************
   Function:  int SubProgram_Start(char *pszPathName, char *pszArguments)   
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

static int SubProgram_Start(char *pszPathName, char *pszArguments)
{
    int  sub_status; 
    int MAX_ARGS=32;
    char * pArgs[MAX_ARGS];
    unsigned long ulNumArgs = 1;
    char tmpArgs[256];
    char *pArg = NULL;
    int pid = 0;    

    pArgs[0]=pszPathName;
    if (pszArguments != NULL)
        {
        strcpy(tmpArgs, pszArguments);    
        /* 'execl' return -1 on error. */
        pArg = strtok(tmpArgs, " ");
        while ((pArg != NULL) && (ulNumArgs < MAX_ARGS))
            {
            pArgs[ulNumArgs] = pArg;
            ulNumArgs++;          
            pArg = strtok(NULL, " ");
            }

        /*
        pArgs[0]=pszPathName;  
        pArgs[1]=pszArguments;  
        pArgs[2] = NULL; */   
        }
    pArgs[ulNumArgs] = NULL; 
    if (!(pid = vfork()))
        {
        seteuid((uid_t)0);
        /* We are the child! */            
        if (execvp(pszPathName, pArgs) == -1)
            {
            //syslog(LOGOPTS,"SubProgram_Start false\n"); 
            return false;
            }
        //syslog(LOGOPTS,"Child PID after = %d\n",pid);          
        }
    else
        {
        waitpid(pid,&sub_status,0);
        return true;        
        }    
    return true; 
}

static void LockFile(int fd)
{
    struct flock lock;       
    memset (&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl (fd, F_SETLKW, &lock);
}

static void UnLockFile(int fd)
{
    struct flock lock;  
    lock.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &lock);
    close (fd);     
}


#endif /*ifndef CHECKSYSC_H_*/
