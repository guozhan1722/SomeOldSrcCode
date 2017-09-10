#ifndef MPCI4G_ATCOMMAND_H_
#define MPCI4G_ATCOMMAND_H_

#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/termios.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <sys/sysinfo.h>
#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <pwd.h>
#include <netinet/tcp.h>
struct uci_context *ctx = NULL;

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#ifndef EMBED
#include <sys/select.h>
#endif

#include "uci.h"
#define DEBUG_4G 1 


/* Define the band info for E371*/
#define SYS_SBAND_BC0_A             0
#define SYS_SBAND_BC0_B             1
#define SYS_SBAND_BC1               2
#define SYS_SBAND_BC2               3
#define SYS_SBAND_BC3               4
#define SYS_SBAND_BC4               5
#define SYS_SBAND_BC5               6
#define SYS_SBAND_GSM_DCS_1800      7
#define SYS_SBAND_GSM_EGSM_900      8
#define SYS_SBAND_GSM_PGSM_900      9
#define SYS_SBAND_BC6               10
#define SYS_SBAND_BC7               11
#define SYS_SBAND_BC8               12
#define SYS_SBAND_BC9               13
#define SYS_SBAND_BC10              14
#define SYS_SBAND_BC11              15
#define SYS_SBAND_GSM_450           16
#define SYS_SBAND_GSM_480           17
#define SYS_SBAND_GSM_750           18
#define SYS_SBAND_GSM_850           19
#define SYS_SBAND_GSM_RGSM_900      20
#define SYS_SBAND_GSM_PCS_1900      21
#define SYS_SBAND_WCDMA_I_IMT_2000  22
#define SYS_SBAND_WCDMA_II_PCS_1900 23
#define SYS_SBAND_WCDMA_III_1700    24
#define SYS_SBAND_WCDMA_IV_1700     25
#define SYS_SBAND_WCDMA_V_850       26
#define SYS_SBAND_WCDMA_VI_800      27
#define RESERVED_BC12_BC14_1        28
#define RESERVED_BC12_BC14_2        29
#define RESERVED_1                  30
#define RESERVED_2                  31

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


#endif /*ifndef MPCI4G_H_*/
 

