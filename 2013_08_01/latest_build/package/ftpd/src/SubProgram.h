#ifndef SUBPROGRAM_H
#define SUBPROGRAM_H 

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
 #include <string.h>
#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR) 
    
int SubProgram_Start(char *pszPathName, char *pszArguments);  
int fetch_Local_IP_MAC(char *ifname, char *mac);
#endif
