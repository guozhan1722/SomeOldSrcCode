/*
 * traffic.cpp
 *
 *  Created on: May 10, 2012
 *      Author: zguo
 */

using namespace std;

#include <cassert>

#include <stdio.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <string.h>
#include <syslog.h>

#include <sys/wait.h>
#include <ctime>
#include <cstdlib>
#include "traffic.hpp"

#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)
static int wcnt;

class watchdog {
private:
	int count ;
public:
	int get_count(void);
	void set_count(int cnt);
	watchdog();
};

class iface_traffic {
private:
	long unsigned int irx;
	long unsigned int itx;
public:
	char interface[24];
	long unsigned int get_tx();
	long unsigned int get_rx();
	iface_traffic(const char *in);
	iface_traffic();
};

iface_traffic::iface_traffic(const char *i){
	strcpy(interface,i);
	long unsigned int rx, tx;
	if(read_proc_net_dev(interface,&rx,&tx)==true){
		itx=tx;
		irx=rx;
	}else {
		itx=0;
		irx=0;
	}

}

iface_traffic::iface_traffic(){
	strcpy(interface,INTERFACE);
	long unsigned int rx, tx;
	read_proc_net_dev(interface,&rx,&tx);
	itx=tx;
	irx=rx;
}

long unsigned int iface_traffic::get_rx(){
	long unsigned int rx, tx;
	if(read_proc_net_dev(interface,&rx,&tx)==true){
		itx=tx;
		irx=rx;
	}else {
		itx=0;
		irx=0;
	}

	return irx;
}

long unsigned int iface_traffic::get_tx(){
	return itx;
}


watchdog::watchdog() {
	count=0;
}

int watchdog::get_count() {
	return count;
}

void watchdog::set_count(int cnt) {
	count=cnt;
}

int log_info_to_flash(char* loginfo)
{
    FILE *f;
    int fsize=0;
    /*cat last stop time if exist*/
    struct tm *ttime;
    time_t tm = time(0);
    char time_string[6]; // variable to hold the time HH:MM
    int rc;
    char date_time[256];

    ttime = localtime(&tm);

    sprintf(date_time,"%s",asctime(ttime));

    rc=strlen(date_time);
    date_time[rc-1]='\0';
    strcat(date_time," ");
    strcat(date_time,loginfo);
    f =fopen("/etc/system.debug","a");
    if (f==NULL) {
        return false;
    }
    if (-1==fseek(f,0L,SEEK_END)) {
        return false;
    }
    fsize=ftell(f);

    if (fsize >2000) {
        fclose(f);       
        f =fopen("/etc/system.debug","w+");
        if (f==NULL) {
            return false;
        }
    } 
    fputs(date_time,f);      
//    fputs(loginfo,f);      
    fclose(f);   

    return true;
}

/**********************************************************************************
   Function:  int SubProgram_Start(char *pszPathName, char *pszArguments)   
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

int SubProgram_Start(char *pszPathName, char *pszArguments)
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

int read_proc_net_dev(char* interface,unsigned long* rx,unsigned long* tx)
{
    FILE* f;
    char buffer_netstate[280];
    char tmp_buff[280];

    f = fopen("/proc/net/dev", "r");
    while (fgets(buffer_netstate, 280, f))
    {
        if (strstr(buffer_netstate, interface))
            break;  /*get the right string. */
    }
    fclose(f);

    if (strstr(buffer_netstate, interface)) {
    	sscanf(&buffer_netstate[7],"%s%lu%s%s%s%s%s%s%s%lu%s%s%s%s%s%s",
               tmp_buff, rx,
               tmp_buff, tmp_buff,
               tmp_buff, tmp_buff,
               tmp_buff, tmp_buff,
               tmp_buff, tx,
               tmp_buff,tmp_buff,
               tmp_buff,tmp_buff,
               tmp_buff,tmp_buff
    	);
    	return true;
    } else {/* didnt find*/
        //syslog(LOGOPTS,"can not find %s\n",interface);
        return false;
    }

    return true;
}

int get_uptime(void)
{
	FILE* fp;
	double uptime, idle_time;
	int ut;
	/* Read the system uptime and accumulated idle time from /proc/uptime.*/
	fp = fopen ("/proc/uptime", "r");
	fscanf (fp, "%lf %lf\n", &uptime, &idle_time);
	fclose (fp);
	ut=(int)uptime;

	return ut;
}

int keep_watch(int b_time, int rs_time, int rb_time,const char * iface, const char * hostname)
{
	int current_time;
	watchdog my_dog;
	iface_traffic first(iface);
    char buff[256];
    char pingcmd[256];

	long unsigned int old_rx=0;
	long unsigned int new_rx=0;

	old_rx=first.get_rx();
	while(1){
		sleep(1);
		//sprintf(pingcmd,"ping %s -c 1 -s 1 -W 4 2>&1 >/dev/null",hostname);
		//W_DBG("ping command =  %s \n",pingcmd);
		//system(pingcmd);
		new_rx=first.get_rx();
    	wcnt++;
		W_DBG("set count to  %d \n",wcnt);
    	my_dog.set_count(wcnt);

		W_DBG("count= %d max_count=%d new_rx=%d...\n",wcnt,rb_time,new_rx);
		if (wcnt >= rb_time){
            W_DBG("count over max limit, need reboot system ....\n");
            sprintf(buff,"%s: traffic watch timer = %d reboot\n",__func__,wcnt);
            log_info_to_flash(buff);
            SubProgram_Start("reboot"," ");
			return -1;
		}

		current_time=get_uptime();

		W_DBG("current minus beging time = %d  rboot_count =%d....\n",(current_time-b_time),rs_time);

		if((current_time-b_time) >= rs_time){
			W_DBG("old rx= %d  new rx =%d....\n",old_rx,new_rx);
			if (old_rx != new_rx) {
				my_dog.set_count(0);
			}
    		b_time=current_time;
    		old_rx=new_rx;
		}
		wcnt=my_dog.get_count();
	}

	return -1;
}
