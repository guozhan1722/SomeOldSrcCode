/*
 * dhcpcd - DHCP client daemon -
 * Copyright (C) 1996 - 1997 Yoichi Hariguchi <yoichi@fore.com>
 * Copyright (C) January, 1998 Sergei Viznyuk <sv@phystech.com>
 * 
 * dhcpcd is an RFC2131 and RFC1541 compliant DHCP client daemon.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <setjmp.h>
#include "dhcpcd.h"
#include "client.h"
#include "pathnames.h"

extern char     *ProgramName;
extern char     *IfName;
extern jmp_buf      env;
extern void     *(*currState)();
int my_timeout_flag=1;
/*****************************************************************************/
void killPid(sig)
int sig;
{
	FILE *fp;
	pid_t pid;
	char pidfile[48];
	sprintf(pidfile,PID_FILE_PATH,IfName);
	fp=fopen(pidfile,"r");
	if ( fp == NULL ) goto ntrn;
	fscanf(fp,"%u",&pid);
	fclose(fp);
	if ( kill(pid,sig) )
		{
		unlink(pidfile);
		ntrn: if ( sig == SIGALRM )	return;
		fprintf(stderr,"****  %s: not running\n",ProgramName);
		exit(1);
		}
	exit(0);
}
/*****************************************************************************/
void writePidFile(pid_t pid)
	{
	FILE *fp;
	char pidfile[48];
	sprintf(pidfile,PID_FILE_PATH,IfName);
	fp=fopen(pidfile,"w");
	if ( fp == NULL )
		{
		syslog(LOG_ERR,"writePidFile: fopen: %m\n");
		exit(1);
		}
	fprintf(fp,"%u\n",pid);
	fclose (fp);
	}
/*****************************************************************************/
void deletePidFile()
	{
	char pidfile[48];
	sprintf(pidfile,PID_FILE_PATH,IfName);
	unlink(pidfile);
	}
/*****************************************************************************/

int SubProgram_Start(char *pszPathName, char *pszArguments)
	{
	int  sub_status; 
	int MAX_ARGS=8;
	char * pArgs[MAX_ARGS];
	unsigned long ulNumArgs = 0;
	char tmpArgs[512];
	char *pArg = NULL;
	int pid = 0;    
	pArgs[0] = NULL;

	if (pszArguments != NULL)
		{
		strcpy(tmpArgs, pszArguments);

		pArg = (char *)strtok(tmpArgs, " ");
		while ((pArg != NULL) && (ulNumArgs < MAX_ARGS))
			{
			pArgs[ulNumArgs] = pArg;
			ulNumArgs++;

			pArg = (char *)strtok(NULL, " ");
			}

		pArgs[ulNumArgs] = NULL; 
		}
	if (!(pid = vfork()))
		{
		seteuid((uid_t)0);            
		/* We are the child! */
		if (execvp(pszPathName, pArgs) == -1)
			{
			syslog(LOG_ERR,"SubProgram_Start false\n"); 
			return 0;
			}
		//syslog(LOGOPTS,"Child PID after = %d\n",pid);          
		}
	else
		{
		//waitpid(pid,&sub_status,0);
		//syslog(LOG_INFO,"Child returned.\n"); 
		return 1;        
		}    
	return 1; 
	}


void sigHandler(sig)
int sig;
{
  char buff[256];

  sprintf(buff,"/bin/zcip -f -q %s /etc/init.d/zcip_script",IfName);
	if ( sig == SIGCHLD )
		{
		waitpid(-1,NULL,WNOHANG);
		return;
		}
	if ( sig == SIGALRM )
		{
		if ( currState == &dhcpBound )
			siglongjmp(env,1); /* this timeout is T1 */
		else
			{
			if ( currState == &dhcpRenew )
				siglongjmp(env,2); /* this timeout is T2 */
			else
				{
				if ( currState == &dhcpRebind )
					siglongjmp(env,3);	/* this timeout is dhcpIpLeaseTime */
				else
					{
					if ( currState == &dhcpReboot )
						siglongjmp(env,4);	/* failed to acquire the same IP address */
					else
						{
//              syslog(LOG_ERR,"timed out waiting for a valid DHCP server response\n");
						if (my_timeout_flag)
							{
							SubProgram_Start("/bin/zcip", buff); 
							}
						my_timeout_flag = 0;
						siglongjmp(env,4);
						}
					}
				}
			}
		}
	else
		{
		if ( sig == SIGHUP )
			{
			dhcpRelease();
			/* allow time for final packets to be transmitted before shutting down     */
			/* otherwise 2.0 drops unsent packets. fixme: find a better way than sleep */
			sleep(1);
			}
		syslog(LOG_ERR,"terminating on signal %d\n",sig);
		}
	dhcpStop();
	deletePidFile();
	exit(sig);
}
/*****************************************************************************/
void signalSetup()
	{
	int i;
	struct sigaction action;
	sigaction(SIGHUP,NULL,&action);
	action.sa_handler= &sigHandler;
	action.sa_flags = 0;
	for (i=1;i<16;i++) sigaction(i,&action,NULL);
	sigaction(SIGCHLD,&action,NULL);
	}
