/* 
********************************************************* 
 package/checksync/src/checksync.c
 
 Daemon for monitoring the following:
 1. rssi
 2. roaming status
 3. voltage (not implementd)
 4. eight ioports status.
 
 Send snmptrap if status changes or over threshold.
 OID defined in package/net-snmp/src/include/net-snmp/net-snmp-config.h
 
*********************************************************
*/




#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <syslog.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "checksync.h"

char * OID_STRING = "1.3.6.1.4.1.21703.6000.100";

// snmptrap for rssi, roaming and 8 ioports 
int current_rssi = DEFAULT_WRONG_RSSI;
int last_rssi = DEFAULT_WRONG_RSSI;
int current_roaming_status = false;
int last_roaming_status = false;

int current_input1_status = 0;
int current_input2_status = 0;
int current_input3_status = 0;
int current_input4_status = 0;
int current_output1_status = 0;
int current_output2_status = 0;
int current_output3_status = 0;
int current_output4_status = 0;

int last_input1_status = 0;
int last_input2_status = 0;
int last_input3_status = 0;
int last_input4_status = 0;
int last_output1_status = 0;
int last_output2_status = 0;
int last_output3_status = 0;
int last_output4_status = 0;

// snmptrap options
char trap_version[512];
char trap_community_name[512];
char trap_manager_host[512];
char v3_user_name[512];
char v3_authen_password[512];
char v3_user_authen_level[512];

// variables to controll resend interval
unsigned int resend_interval_rssi = 0;
unsigned int resend_interval_roaming = 0;
//unsigned int resend_interval_ioports = 0;

char old_ip[32];
char new_ip[32];


static void send_ip_snmptrap();
bool fetch_Local_IP_MAC(char *ifname, char *mac);





/* Create daemon */
void daemonize(void)
{
    pid_t pid;
    /* Become a session leader */
    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    } else if (pid != 0) /* parent */
        exit(0);

    setsid();

    /* Change the current working directory to root. */
    if (chdir("/") < 0) {
        perror("chdir");
        exit(1);
    }

    /* Attach file descriptors 0, 1, and 2 to /dev/null. */
    close(0);
    open("/dev/null", O_RDWR);
    dup2(0, 1);
    dup2(0, 2);
}


/* Get snmptrap options from /etc/config/snmpd file */
bool get_snmptrap_options()
{
    struct uci_context *ctx = NULL;
    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx) {
        DEBUG("Out of memory\n");
        return(false);
    }

    if ( get_option_by_section_name(ctx, confile, "snmpd", "NetWork_SNMP_Trap_Version",  trap_version) == false ) {
        DEBUG("can not get NetWork_SNMP_Trap_Version from %s\n", confile);
        if (ctx) {
            uci_free_context(ctx);
            ctx = NULL;
        }
        return(false);
    } else {
        DEBUG("snmptrap NetWork_SNMP_Trap_Version:%s\n", trap_version);
    }

    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx) {
        DEBUG("Out of memory\n");
        return(false);
    }

    if ( get_option_by_section_name(ctx, confile, "snmpd", "NetWork_SNMP_Trap_Community_Name",  trap_community_name) == false ) {
        DEBUG("can not get snmp NetWork_SNMP_Trap_Community_Name from from %s\n", confile);
        if (ctx) {
            uci_free_context(ctx);
            ctx = NULL;
        }
        return(false);
    } else {
        DEBUG("snmptrap NetWork_SNMP_Trap_Community_Name:%s\n", trap_community_name);
    }

    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx) {
        DEBUG("Out of memory\n");
        return(false);
    }

    if ( get_option_by_section_name(ctx, confile, "snmpd", "NetWork_SNMP_Trap_Manage_Host",  trap_manager_host) == false ) {
        DEBUG("can not get NetWork_SNMP_Trap_Manage_Host from %s\n", confile);
        if (ctx) {
            uci_free_context(ctx);
            ctx = NULL;
        }
        return(false);
    } else {
        DEBUG("snmptrap NetWork_SNMP_Trap_Manage_Host:%s\n", trap_manager_host);
    }

    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx) {
        DEBUG("Out of memory\n");
        return(false);
    }

    if ( get_option_by_section_name(ctx, confile, "snmpd", "NetWork_SNMP_V3_User_Name",  v3_user_name) == false ) {
        DEBUG("can not get snmp NetWork_SNMP_V3_User_Name from %s\n", confile);
        if (ctx) {
            uci_free_context(ctx);
            ctx = NULL;
        }
        return(false);
    } else {
        DEBUG("snmptrap NetWork_SNMP_V3_User_Name:%s\n", v3_user_name);
    }

    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx) {
        DEBUG("Out of memory\n");
        return(false);
    }

    if ( get_option_by_section_name(ctx, confile, "snmpd", "NetWork_SNMP_V3_Auth_Password",  v3_authen_password) == false ) {
        DEBUG("can not get snmp NetWork_SNMP_V3_Auth_Password from %s\n", confile);
        if (ctx) {
            uci_free_context(ctx);
            ctx = NULL;
        }
        return(false);
    } else {
        DEBUG("snmptrap NetWork_SNMP_V3_Auth_Password:%s\n", v3_authen_password);
    }

    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx) {
        DEBUG("Out of memory\n");
        return(false);
    }

    if ( get_option_by_section_name(ctx, confile, "snmpd", "NetWork_SNMP_V3_User_Auth_Level",  v3_user_authen_level) == false ) {
        DEBUG("can not get snmp NetWork_SNMP_V3_User_Auth_Level from %s\n", confile);
        if (ctx) {
            uci_free_context(ctx);
            ctx = NULL;
        }
        return(false);
    } else {
        DEBUG("snmptrap NetWork_SNMP_V3_User_Auth_Level:%s\n", v3_user_authen_level);
    }
    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }

}





/* Get current rssi value from /var/run/VIP4G_status file */
void get_current_rssi()
{

    FILE* fp;
    char line [ 128 ];
    char *pos;   

    char *pch_1;
    char *pch_2;
    int len;
    char to[128];

    fp = fopen (VIP4G_status_file, "r");
    if (fp != NULL) {

        while ( fgets ( line, sizeof line, fp ) != NULL ) {
            pos = strstr (line,"rssi=");
            if (pos != NULL) {
                //fputs ( line, stdout );
                pch_1 = strchr (line, ':');
                if (pch_1 != NULL) {
                    //DEBUG ("found : at %d\n", pch_1-line+1);
                }

                pch_2 = strrchr (line, '"');
                if (pch_2 != NULL) {
                    //DEBUG ("found \" at %d\n",pch_2-line+1);
                }

                if ((pch_2 != NULL) && (pch_1 != NULL)) {
                    len = (pch_2-line+1) - (pch_1-line+1);
                    //DEBUG ("len: %d\n", len);
                    strncpy(to, pch_1+1, len-1);
                    //DEBUG ("found current rssi value string: %s\n", to);
                    current_rssi = -atoi(to);
                    //DEBUG ("found integer current rssi value:(%d) dbm.\n", current_rssi);
                }
            }
        }         

        fclose (fp);
        fp=NULL;
    }
}


void init_rssi_history(void)
{
#ifdef WRITE_HISTORY
    int i;
    for (i=0;i<HOUR_HISTORY_POINTS;i++) {

        minrssiHistoryH[i] = DEFAULT_WRONG_RSSI;
        maxrssiHistoryH[i] = DEFAULT_WRONG_RSSI;
        averssiHistoryH[i] = DEFAULT_WRONG_RSSI;
    }
    for (i=0;i<MINUTE_HISTORY_POINTS;i++) {

        minrssiHistoryM[i] = DEFAULT_WRONG_RSSI;
        maxrssiHistoryM[i] = DEFAULT_WRONG_RSSI;
        averssiHistoryM[i] = DEFAULT_WRONG_RSSI;
    }
#endif
}



void prepare_rssi_history_data(void)
{
#ifdef WRITE_HISTORY
/*begin get min rssi and max rssi and average rssi*/
    if (minrssiH>current_rssi) {
        minrssiH=current_rssi;
    }
    if (maxrssiH<current_rssi) {
        maxrssiH=current_rssi;
    }

    averssiH=(averssiH+current_rssi)/2;


    if (minrssiM>current_rssi) {
        minrssiM=current_rssi;
    }
    if (maxrssiM<current_rssi) {
        maxrssiM=current_rssi;
    }

    averssiM=(averssiM+current_rssi)/2;

#endif
}



void update_rssi_history(int historyHp, int historyMp)
{
#ifdef WRITE_HISTORY
    FILE* fp;
    int i, j;
    char buff[1024];
    char tempbuff[32];
//    int historyHp=-1;
//    int historyMp=-1;
    float fcurrentHour=0.1;
    float fcurrentMinute=0.1;

    printf("minrssiM=%d, maxrssiM=%d, averssiM=%d\n", minrssiM, maxrssiM, averssiM);
    printf("minrssiH=%d, maxrssiH=%d, averssiH=%d\n", minrssiH, maxrssiH, averssiH);
    historyHp=historyHp%HOUR_HISTORY_POINTS;
    minrssiHistoryH[historyHp]=minrssiH;
    maxrssiHistoryH[historyHp]=maxrssiH;
    averssiHistoryH[historyHp]=averssiH;

    historyMp=historyMp%MINUTE_HISTORY_POINTS;
    minrssiHistoryM[historyMp]=minrssiM;
    maxrssiHistoryM[historyMp]=maxrssiM;
    averssiHistoryM[historyMp]=averssiM;

    fp=fopen("/var/run/rssiHistory.min","w+");
    if (fp) {

        strcpy(buff,"");
        for (i=historyHp+1,j=0;i<HOUR_HISTORY_POINTS;i++,j++) {

            sprintf(tempbuff,"%0.2f %d\n",fcurrentHour+(float)j/(float)POINTS_IN_HOUR - (float)HISTORY_HOURS_MR,minrssiHistoryH[i]);
            printf("currenttime=%0.2f j=%d   %s ",fcurrentHour,j,tempbuff);
            fputs(tempbuff,fp);

        }
        for (i=0;i<=historyHp;i++,j++) {
            sprintf(tempbuff,"%0.2f %d\n",fcurrentHour+(float)j/(float)POINTS_IN_HOUR - (float)HISTORY_HOURS_MR,minrssiHistoryH[i]);
            printf("currenttime=%0.2f j=%d   %s ",fcurrentHour,j,tempbuff);
            fputs(tempbuff,fp);

        }
        fclose(fp);
    }
    fp=fopen("/var/run/rssiHistory.max","w+");
    if (fp) {
        strcpy(buff,"");
        for (i=historyHp+1,j=0;i<HOUR_HISTORY_POINTS;i++,j++) {
            sprintf(tempbuff,"%0.2f %d\n",fcurrentHour+(float)j/(float)POINTS_IN_HOUR - (float)HISTORY_HOURS_MR, maxrssiHistoryH[i]);
            fputs(tempbuff,fp);

        }
        for (i=0;i<=historyHp;i++,j++) {
            sprintf(tempbuff,"%0.2f %d\n",fcurrentHour+(float)j/(float)POINTS_IN_HOUR - (float)HISTORY_HOURS_MR,maxrssiHistoryH[i]);
            fputs(tempbuff,fp);

        }


        fclose(fp);
    }
    fp=fopen("/var/run/rssiHistory.ave","w+");
    if (fp) {
        strcpy(buff,"");
        for (i=historyHp+1,j=0;i<HOUR_HISTORY_POINTS;i++,j++) {
            sprintf(tempbuff,"%0.2f %d\n",fcurrentHour+(float)j/(float)POINTS_IN_HOUR - (float)HISTORY_HOURS_MR,averssiHistoryH[i]);
            fputs(tempbuff,fp);

        }
        for (i=0;i<=historyHp;i++,j++) {
            sprintf(tempbuff,"%0.2f %d\n",fcurrentHour+(float)j/(float)POINTS_IN_HOUR - (float)HISTORY_HOURS_MR,averssiHistoryH[i]);
            fputs(tempbuff,fp);

        }

        fclose(fp);
    }



/*begin get data for 1 MINUTE*/


    /*then clear min max av*/
    //minrssiH=maxrssiH=averssiH=crssi;
    /*open file*/

    fp=fopen("/var/run/rssiHistory1h.min","w+");
    if (fp) {

        strcpy(buff,"");
        for (i=historyMp+1,j=0;i<MINUTE_HISTORY_POINTS;i++,j++) {

            sprintf(tempbuff,"%0.2f %d\n",fcurrentMinute+(float)j/(float)POINTS_IN_MINUTE - (float)HISTORY_MINUTES_MR,minrssiHistoryM[i]);
            //printf("currenttime=%0.2f j=%d   %s ",fcurrentMinute,j,tempbuff);
            fputs(tempbuff,fp);

        }
        for (i=0;i<historyMp;i++,j++) {
            sprintf(tempbuff,"%0.2f %d\n",fcurrentMinute+(float)j/(float)POINTS_IN_MINUTE - (float)HISTORY_MINUTES_MR,minrssiHistoryM[i]);
            //printf("currenttime=%0.2f j=%d   %s ",fcurrentMinute,j,tempbuff);
            fputs(tempbuff,fp);

        }
        fclose(fp);
    }
    fp=fopen("/var/run/rssiHistory1h.max","w+");
    if (fp) {
        strcpy(buff,"");
        for (i=historyMp+1,j=0;i<MINUTE_HISTORY_POINTS;i++,j++) {
            sprintf(tempbuff,"%0.2f %d\n",fcurrentMinute+(float)j/(float)POINTS_IN_MINUTE - (float)HISTORY_MINUTES_MR,maxrssiHistoryM[i]);
            fputs(tempbuff,fp);

        }
        for (i=0;i<historyMp;i++,j++) {
            sprintf(tempbuff,"%0.2f %d\n",fcurrentMinute+(float)j/(float)POINTS_IN_MINUTE - (float)HISTORY_MINUTES_MR,maxrssiHistoryM[i]);
            fputs(tempbuff,fp);

        }


        fclose(fp);
    }
    fp=fopen("/var/run/rssiHistory1h.ave","w+");
    if (fp) {
        strcpy(buff,"");
        for (i=historyMp+1,j=0;i<MINUTE_HISTORY_POINTS;i++,j++) {
            sprintf(tempbuff,"%0.2f %d\n",fcurrentMinute+(float)j/(float)POINTS_IN_MINUTE - (float)HISTORY_MINUTES_MR,averssiHistoryM[i]);
            fputs(tempbuff,fp);

        }
        for (i=0;i<historyMp;i++,j++) {
            sprintf(tempbuff,"%0.2f %d\n",fcurrentMinute+(float)j/(float)POINTS_IN_MINUTE - (float)HISTORY_MINUTES_MR,averssiHistoryM[i]);
            fputs(tempbuff,fp);

        }

        fclose(fp);
    }



    /*end of get data in 1 hour*/
#endif
}





/* Send rssi snmptrap */
void send_rssi_snmptrap(void)
{

    char buff[1024];
    memset(buff, 0, sizeof(buff));

    sprintf(buff,"%s %s %s %s.1 s %d %s %s %s",            
            trap_version,
            trap_community_name,
            trap_manager_host,
            OID_STRING,
            current_rssi,
            v3_user_name,
            v3_authen_password,
            v3_user_authen_level);
    //syslog(LOG_INFO,"snmptrap cmd string ---> %s\n", buff);
    DEBUG("cmd string ---> snmptrap %s\n", buff);
    //printf("cmd string ---> snmptrap %s\n", buff);

    SubProgram_Start(snmptrap_script, buff);
}


/* Get currnt roaming status from /var/run/VIP4G_status file */
void get_current_roaming_status()
{
    FILE* fp;
    char line [ 128 ];
    char *pos; 
    char *pch_1;
    char *pch_2;
    int len;
    char to[128];
    int roaming_value;

    fp = fopen (VIP4G_status_file, "r");
    if (fp != NULL) {

        while ( fgets ( line, sizeof line, fp ) != NULL ) {
            pos = strstr (line,"roaming=");
            if (pos != NULL) {
                //fputs ( line, stdout );
                pch_1 = strchr (line, ',');
                if (pch_1 != NULL) {
                    //DEBUG ("found , at %d\n", pch_1-line+1);
                }

                pch_2 = strrchr (line, '"');
                if (pch_2 != NULL) {
                    //DEBUG ("found \" at %d\n",pch_2-line+1);
                }

                if ((pch_2 != NULL) && (pch_1 != NULL)) {
                    len = (pch_2-line+1) - (pch_1-line+1);
                    //DEBUG ("len: %d\n", len);
                    strncpy(to, pch_1+1, len-1);
                    //DEBUG ("found roaming staus value string: %s\n", to);
                    roaming_value = atoi(to);
                    DEBUG ("found integer roaming status value:(%d)\n", roaming_value);
                    if (roaming_value == 5 ) {
                        current_roaming_status = true;
                    } else if (roaming_value == 1 ) {
                        current_roaming_status = false;
                    } else {
                        DEBUG ("Current roaming status UNKNOWN !!!\n");
                    } 
                }
            }
        }         

        fclose (fp);
        fp=NULL;
    }
}


/* Send roaming status snmptrap */
void send_roaming_snmptrap()
{    

    char buff[1024];
    memset(buff, 0, sizeof(buff));

    sprintf(buff,"%s %s %s %s.3 s %d %s %s %s",
            trap_version,
            trap_community_name,
            trap_manager_host,
            OID_STRING,
            current_roaming_status,
            v3_user_name,
            v3_authen_password,
            v3_user_authen_level);
    //syslog(LOG_INFO,"snmptrap cmd string ---> %s\n", buff);
    DEBUG("cmd string ---> snmptrap %s\n", buff);
    //printf("cmd string ---> snmptrap %s\n", buff);

    SubProgram_Start(snmptrap_script, buff);

}


/* Get currnt input port status from:
   cat /sys/class/button/INPUT1/status
*/
int new_get_current_input_status(int num)
{
    FILE* fp;
    char buff[1024];
    char rt[2];
    int status_value=0;
    char ioport[512];   

    memset(rt,0,sizeof(rt));
    memset(buff, 0, sizeof(buff));
    sprintf(ioport, "input%d", num);
    sprintf(buff,"/sys/class/button/INPUT%d/status", num);

    fp=fopen(buff,"r");
    if (fp) {
        fgets(rt,2,fp);
        fclose(fp);
        status_value = atoi(rt);
        //printf ("found integer INPUT%d status value:(%d)\n", num, status_value);

        return status_value;

    } else {
        fprintf(stderr, "ERROR: open(%s) failed, errno=%d\n", buff, errno);
        return false;
    }
}


/* Since hotplug is not very reliable, this function deprecated */
/* Get currnt input port status from /etc/config/ioports file */
#if 0
int get_current_input_status(int num)
{
    char ioport[512];
    char input[512];
    struct uci_context *ctx = NULL;

    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx) {
        DEBUG("Out of memory\n");
        return(false);
    }

    sprintf(ioport, "input%d", num); 
    if ( get_option_by_section_name(ctx, ioport_confile, "input", ioport, input) == false ) {
        DEBUG("can not get %s from %s\n", ioport, ioport_confile);
        if (ctx) {
            uci_free_context(ctx);
            ctx = NULL;
        }
        return(false);
    } else {
        DEBUG("snmptrap %s status: %s\n", ioport, input);
        return atoi(input);
    }
    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
}
#endif


/* Get currnt output port status from:
   cat /sys/class/leds/OUTPUT1/brightness
*/
int new_get_current_output_status(int num)
{
    FILE* fp;
    char buff[1024];
    char rt[2];
    int status_value=0;
    char ioport[512];

    memset(rt,0,sizeof(rt));
    memset(buff, 0, sizeof(buff));
    sprintf(ioport, "output%d", num);
    sprintf(buff,"/sys/class/leds/OUTPUT%d/brightness", num);

    fp=fopen(buff,"r");
    if (fp) {
        fgets(rt,2,fp);
        fclose(fp);
        status_value = atoi(rt);
        //printf ("found integer OUTPUT%d status value:(%d)\n", num, status_value);

        return status_value;

    } else {
        fprintf(stderr, "ERROR: open(%s) failed, errno=%d\n", buff, errno);
        return false;
    }
}

/* This function deprecated */
/* Get currnt output port status from /etc/config/ioports file */
#if 0
int get_current_output_status(int num)
{
    char ioport[512];
    char output[512];
    struct uci_context *ctx = NULL;

    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx) {
        DEBUG("Out of memory\n");
        return(false);
    }

    sprintf(ioport, "output%d", num); 
    if ( get_option_by_section_name(ctx, ioport_confile, "output", ioport,  output) == false ) {
        DEBUG("can not get %s from %s\n", ioport, ioport_confile);
        if (ctx) {
            uci_free_context(ctx);
            ctx = NULL;
        }
        return(false);
    } else {
        DEBUG("snmptrap %s status: %s\n", ioport, output);
        return atoi(output);
    }
    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
}
#endif


void get_ioports_status()
{
    current_input1_status = new_get_current_input_status(1);    
    current_input2_status = new_get_current_input_status(2); 
    current_input3_status = new_get_current_input_status(3); 
    current_input4_status = new_get_current_input_status(4);

    current_output1_status = new_get_current_output_status(1);
    current_output2_status = new_get_current_output_status(2);
    current_output3_status = new_get_current_output_status(3);
    current_output4_status = new_get_current_output_status(4); 
}


void assign_last_ioports_status()
{    
    last_input1_status = current_input1_status;
    last_input2_status = current_input2_status;
    last_input3_status = current_input3_status;
    last_input4_status = current_input4_status;

    last_output1_status = current_output1_status;
    last_output2_status = current_output2_status; 
    last_output3_status = current_output3_status; 
    last_output4_status = current_output4_status; 
}


/* Send eight ioports status snmptrap */
void send_ioports_snmptrap()
{
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    int current_ioports_status = 0;

    /*
    ******************************************************* 
       current_ioports_status ----  0: open;    1: close; 
        bit 0:  OUTPUT1 
        bit 1:  OUTPUT2 
        bit 2:  OUTPUT3
        bit 3:  OUTPUT4 
         
        bit 4:  INPUT1 
        bit 5:  INPUT2  
        bit 6:  INPUT3  
        bit 7:  INPUT4  
    ******************************************************** 
    */ 
    if ( current_output1_status == 1 ) {
        current_ioports_status |= 0x1;
    }
    if ( current_output2_status == 1 ) {
        current_ioports_status |= 0x1<<1;
    }
    if ( current_output3_status == 1 ) {
        current_ioports_status |= 0x1<<2;
    }
    if ( current_output4_status == 1 ) {
        current_ioports_status |= 0x1<<3;
    }
    if ( current_input1_status == 1 ) {
        current_ioports_status |= 0x1<<4;
    }
    if ( current_input2_status == 1 ) {
        current_ioports_status |= 0x1<<5;
    }
    if ( current_input3_status == 1 ) {
        current_ioports_status |= 0x1<<6;
    }
    if ( current_input4_status == 1 ) {
        current_ioports_status |= 0x1<<7;
    }
    DEBUG("current_ioports_status=0x%x\n", current_ioports_status);    

    sprintf(buff,"%s %s %s %s.4 s %d %s %s %s",
            trap_version,
            trap_community_name,
            trap_manager_host,
            OID_STRING,
            current_ioports_status,
            v3_user_name,
            v3_authen_password,
            v3_user_authen_level);
    //syslog(LOG_INFO,"snmptrap cmd string ---> %s\n", buff);
    DEBUG("cmd string ---> snmptrap %s\n", buff);
    //printf("cmd string ---> snmptrap %s\n", buff);

    SubProgram_Start(snmptrap_script, buff);

}


unsigned int get_sys_uptime()
{
    unsigned int uptimes;
    FILE* fp = fopen ("/proc/uptime", "r");
    if (fp != NULL) {
        char buf[128];
        int res;
        float upsecs;
        char *b = fgets (buf, 128, fp);
        if (b == buf) {
            /* The following sscanf must use the C locale.  */
            // setlocale (LC_NUMERIC, "C");
            res = sscanf (buf, "%f", &upsecs);
            // setlocale (LC_NUMERIC, "");
            if (res == 1) {
                uptimes = (unsigned int) upsecs;
                //printf("uptime=%d\n", uptimes);
            } else {
                //printf("uptime=%d\n", uptimes);
            }
        }

        fclose (fp);
        fp=NULL;
    }
    return uptimes;
}


void write_reboot_log()
{
    char buff[1024];
    struct timeval tv1;
    gettimeofday (&tv1, NULL);
    unsigned int uptimes = get_sys_uptime();
    tv1.tv_sec -= uptimes;
    printf("uptime=%d\n", uptimes);

    struct tm *time_tm = localtime(&tv1.tv_sec); 

    if (time_tm!=NULL) {
        strftime(buff,60,"%Y-%m-%d  %H:%M:%S", time_tm);        
    }
    FILE* fp = fopen(sysrebootlogfile, "a+");
    if (fp) {
        fputs("\nSystem rebooted at ",fp);
        fputs(buff,fp);
        fclose(fp);
    }
}


int main(int argc, char ** argv)
{
#ifdef WRITE_HISTORY
    int timer2=0;
    int historyHp=-1;
    int historyMp=-1;
    struct timeval tv1;
    long new_diff=0;
    long old_diff=0;
    struct sysinfo sinfo;
    unsigned int uptimes;



    uptimes = get_sys_uptime();
    write_reboot_log();

    init_rssi_history();

    new_diff =old_diff= tv1.tv_sec - sinfo.uptime;
#endif
    daemonize();
    get_snmptrap_options();

    get_current_rssi();
    last_rssi = current_rssi ;
    get_ioports_status();
    assign_last_ioports_status();            
    fetch_Local_IP_ADDR("br-wan2",old_ip);
    while (1) {
        sleep(3); 

        //syslog(LOG_INFO,"checksync\n");
        //printf("checksync\n");

#ifdef WRITE_HISTORY
        uptimes = get_sys_uptime();
        write_reboot_log();

        // reboot log
//            uptimes = get_sys_uptime();
//            printf("uptime=%d\n", uptimes);
//            if ( uptimes< REBOOT_REPORT_DELAY ) {
//                sysinfo(&sinfo);
//                new_diff = tv1.tv_sec - sinfo.uptime;
//                if ((new_diff - 5 > old_diff) || (new_diff + 5 < old_diff)) {
//                    write_reboot_log();
//                }
//            }
        timer2++;
        timer2=timer2%(REPORT_INTERVAL+1);
#endif
        resend_interval_rssi++;
        resend_interval_roaming++;
        //resend_interval_ioports++;


        /* ********** 1. rssi snmptrap******* */
        get_current_rssi();                       
        DEBUG("last_rssi=%d, current_rssi=%d, resend_interval_rssi=%d\n", last_rssi,current_rssi, resend_interval_rssi);

        if ((current_rssi < SNMP_TRAP_RSSI_THRESHOLD_LOW) && (last_rssi < SNMP_TRAP_RSSI_THRESHOLD_LOW)) {
            if ( resend_interval_rssi > SNMP_TRAP_RESEND_INTERVAL ) {
                send_rssi_snmptrap();
                resend_interval_rssi = 0;
            }
        } else if ((current_rssi < SNMP_TRAP_RSSI_THRESHOLD_LOW) && (last_rssi >= SNMP_TRAP_RSSI_THRESHOLD_LOW)) {
            send_rssi_snmptrap();
            resend_interval_rssi = 0;
        }
        last_rssi = current_rssi ;

#ifdef WRITE_HISTORY
        prepare_rssi_history_data();
        //printf("minrssiM=%d, maxrssiM=%d, averssiM=%d\n", minrssiM, maxrssiM, averssiM);
        //printf("minrssiH=%d, maxrssiH=%d, averssiH=%d\n", minrssiH, maxrssiH, averssiH);
        if ( !(timer2 % UPDATE_INTERVAL) ) {
            update_rssi_history(historyHp, historyMp);
        }
#endif

        /* ************* 2. voltage snmptrap ************ */


        /* ************* 3. roaming snmptrap ************ */
        get_current_roaming_status();
        if ( current_roaming_status == true ) {
            if ( resend_interval_roaming > SNMP_TRAP_RESEND_INTERVAL ) {
                send_roaming_snmptrap();
                resend_interval_roaming = 0;
            }
        }

        /* ************* 4. ioports snmptrap ************ */
        get_ioports_status();

        DEBUG("current_output1_status=%d last_output1_status=%d\n", current_output1_status, last_output1_status);

        //Check if ANY ioport status change 
        if ( ( current_output1_status != last_output1_status) || ( current_output2_status != last_output2_status) || 
             ( current_output3_status != last_output3_status) || ( current_output4_status != last_output4_status) ||
             ( current_input1_status != last_input1_status) || ( current_input2_status != last_input2_status) ||
             ( current_input3_status != last_input3_status) || ( current_input4_status != last_input4_status) ) {
            //if ( resend_interval_ioports > SNMP_TRAP_RESEND_INTERVAL ) {
            send_ioports_snmptrap();
            //resend_interval_ioports = 0;
            //}
        }

        assign_last_ioports_status(); 

        /* *************step 5. ip address changed snmptrap ************ */
        fetch_Local_IP_ADDR("br-wan2",new_ip);

        //Check if ip change 
        if (strcmp(old_ip,new_ip)) {
            send_ip_snmptrap();
            strcpy(old_ip, new_ip);
        }
        else
        {
            //printf("oldip=%s, new_ip=%s",old_ip,new_ip);
        }
        /* ****************************************** */


    } //end while loop

    return 0;
}



/* Send eight ioports status snmptrap */
static void send_ip_snmptrap()
{
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    int current_ioports_status = 0;
    char smac[32]="0.0.0.0";
    char hostname[128]="N/A";

    gethostname(hostname, sizeof(hostname));

    //Device name, Mac address, old IP, new IP
    fetch_Local_IP_MAC("br-wan2", smac);

    sprintf(buff,"%s %s %s %s.5 s %s\t%s\t%s\t%s %s %s %s",
            trap_version,
            trap_community_name,
            trap_manager_host,
            OID_STRING,
            smac,old_ip,new_ip,hostname,
            v3_user_name,
            v3_authen_password,
            v3_user_authen_level);
    //syslog(LOG_INFO,"snmptrap cmd string ---> %s\n", buff);
    DEBUG("cmd string ---> snmptrap %s\n", buff);
    //printf("cmd string ---> snmptrap %s\n", buff);

    //SubProgram_Start(snmptrap_script, buff);

    SubProgram_Start("/bin/snmptrap.sh",buff);     


}





int fetch_Local_IP_ADDR(char *ifname, char *addr)
{
    struct ifreq ifr;
    int sock;
    char *p;
    char temp[32]="0.0.0.0";

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if (-1==sock) {
        //syslog(LOGOPTS,"fetch_Local_IP_ADDR: socket=-1");  
        perror("fetch_Local_IP_ADDR ");
        strcpy(addr, temp); 
        return false;
    }
    strncpy(ifr.ifr_name,ifname,sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';   
    if (-1==ioctl(sock, SIOCGIFADDR, &ifr)) {
        //perror("ioctl(SIOCGIFADDR) "); /*dont show it, sometimes no ip*/
        //syslog(LOGOPTS,"fetch_Local_IP_ADDR:%s ioctl(SIOCGIFADDR)",ifname);       
        strcpy(addr, temp); 
        close(sock);
        return false;
    }
    p=inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr);
    strncpy(temp,p,sizeof(temp)-1);
    temp[sizeof(temp)-1]='\0';
    strcpy(addr, temp);  
    close(sock);  
    return true;
}


/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/
bool fetch_Local_IP_MAC(char *ifname, char *mac)
{
    struct ifreq ifr;
    int sock, j, k;
    char temp[32]="00:00:00:00:00:00";

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if (-1==sock) {
        //syslog(LOGOPTS,"fetch_Local_IP_MAC:%s socket() ",ifname);      
        perror("socket() ");
        strcpy(mac, temp);        
        return false;
    }
    strncpy(ifr.ifr_name,ifname,sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';

    if (-1==ioctl(sock, SIOCGIFHWADDR, &ifr)) {
        //syslog(LOGOPTS,"fetch_Local_IP_MAC:%s ioctl(SIOCGIFHWADDR)",ifname);         
        //perror("ioctl(SIOCGIFHWADDR) ");
        strcpy(mac, temp);    
        close(sock);
        return false;
    }

    for (j=0, k=0; j<6; j++) {
        k+=snprintf(temp+k, sizeof(temp)-k-1, j ? ":%02X" : "%02X",
                    (int)(unsigned int)(unsigned char)ifr.ifr_hwaddr.sa_data[j]);
    }
    temp[sizeof(temp)-1]='\0';
    strcpy(mac, temp);          
    close(sock);  
    return true;
}
