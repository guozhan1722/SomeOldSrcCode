/* file : msmscom.c */
/* description : control 3G unit through short messages   */ 


#include <time.h>
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
#include <termios.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>													

#include <stdbool.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>

#include "uci.h"
#include "gnokii.h"
#include "atgen.h"
#include "msmslib.h"


#define	LOGOPTS		(LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)
#ifdef MEMWATCH
    #include "memwatch.h"
#endif


typedef struct {
    int location;
    int command;
    int argu;
} msc_t;

#define DISMATCHED 0
#define MATCHED 1

#define MSC_REBOOT 1
#define MSC_EURD 2
#define MSC_GPSR 3
#define MSC_MIOC 4
#define MSC_MIOP 5
#define MSC_COM_NUM 6
#define MSC_WEBCLIENT 7
#define MSC_APN 8



char *scriptFile="/bin/msmscomscript";
char SMS_CMD_Filter_Phone[6][20];
char SMS_CMD_Filter_Set[5];
int SMS_CMD_Phone_valid;
char System_SMS_Commad_En;

char usr_str[32];
char pwd_str[32];
char apn_str[32];

static int log_info_to_flash(char* loginfo)
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

int calculate_key(char *s_Phone,char *s_IMEI, char *buffer_get)
{
    int i,j;
    int i_imei,i_phone;
    char *p;
    i=strlen(s_Phone);
    j=strlen(s_IMEI);
    while(i>0)
    {
        if(s_Phone[i-1]>='0' && s_Phone[i-1]<='9')break;
        s_Phone[i-1]=0;
        i--;
    }
    while(j>0)
    {
        if(s_IMEI[j-1]>='0' && s_IMEI[j-1]<='9')break;
        s_IMEI[j-1]=0;
        j--;
    }
    //printf("check su key:%s,%s,%d,%d\n",s_Phone,s_IMEI,i,j);

    if(i<6 || j<6)return -1;

    //caculated from the phone number-4 and imei-5 with last 4-5 numbers mutiply and add 162534 get the last six number, then reverse.
    p=s_Phone+(i-4);
    i_phone=atoi(p);
    p=s_IMEI+(j-5);
    i_imei=atoi(p);
    if(i_imei<1 || i_phone<1 )return -1;

    //printf("phone:%d,imei:%d\n",i_phone,i_imei);

    i=i_imei*i_phone+162534;
    buffer_get[0]=10;
    buffer_get[1]=10;
    buffer_get[2]=10;
    buffer_get[3]=0;
    sprintf(buffer_get+3,"%09d",i);
    i=strlen(buffer_get);
    if(i!=12)return -1;
    buffer_get[0]=buffer_get[11];
    buffer_get[1]=buffer_get[10];
    buffer_get[2]=buffer_get[9];
    buffer_get[3]=buffer_get[8];
    buffer_get[4]=buffer_get[7];
    buffer_get[5]=buffer_get[6];
    buffer_get[6]=0;

    return 1;
}


int calculate_loc_key()
{
    //read /tmp/run/std_lte_statue  imei=012773002003778  phonenum=+14035602757
    int i,j;

    char buffer_get[256];
    char s_IMEI[30]="N/A";
    char s_Phone[30]="N/A";

    char *p;
    FILE *f;

    if (!(f = fopen("/var/run/std_lte_statue", "r"))) 
    {
        return -1;
        //do nothing.
    }else
    {
        while (fgets(buffer_get, 256, f)) 
        {
            p=NULL;
            p = strstr(buffer_get, "imei=");//imei="012773002002648"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_IMEI,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "phonenum=");//phonenum=+14036050307
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_Phone,"%s",p);
                }
            }
        }
        fclose(f);
    }

    buffer_get[0]=0;
    if(calculate_key(s_Phone,s_IMEI, buffer_get)<=0)return -1;

    printf("Local Phone:%s\n",s_Phone);
    printf("Local IMEI:%s\n",s_IMEI);
    printf("Local key:%s\n",buffer_get);
    return 1;
}

int check_su_key(char *s_key)
{
    //read /tmp/run/std_lte_statue  imei=012773002003778  phonenum=+14035602757
    int i,j;

    char buffer_get[256];
    char s_IMEI[30]="N/A";
    char s_Phone[30]="N/A";

    char *p;
    FILE *f;

    if (!(f = fopen("/var/run/std_lte_statue", "r"))) 
    {
        return -1;
        //do nothing.
    }else
    {
        while (fgets(buffer_get, 256, f)) 
        {
            p=NULL;
            p = strstr(buffer_get, "imei=");//imei="012773002002648"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_IMEI,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "phonenum=");//phonenum=+14036050307
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_Phone,"%s",p);
                }
            }
        }
        fclose(f);
    }

    buffer_get[0]=0;
    if(calculate_key(s_Phone,s_IMEI, buffer_get)<=0)return -1;

    //printf("key:%c%c%c%c%c%c\n",buffer_get[8],buffer_get[7],buffer_get[6],buffer_get[5],buffer_get[4],buffer_get[3]);
    if(*(s_key)==buffer_get[0])
        if(*(s_key+1)==buffer_get[1])
            if(*(s_key+2)==buffer_get[2])
                if(*(s_key+3)==buffer_get[3])
                    if(*(s_key+4)==buffer_get[4])
                        if(*(s_key+5)==buffer_get[5])
                        {
                            return 1;
                        }
    return -1;
}

static int decode_command(gn_data * mdatap, msc_t * poolp)
{
    int result = MATCHED;
    char num[3];
    char *p;
    char * ptr = mdatap->sms->user_data[0].u.text;

    num[1]=0;
    num[2]=0;

    p=NULL;
    p=strstr(ptr,"MSC#REBOOT");
    if(p==ptr)
    {
        poolp->location=mdatap->sms->number;
        poolp->command=MSC_REBOOT;
        poolp->argu=0;
        return result;
    }
    p=NULL;
    p=strstr(ptr,"MSC#SUREBOOT");
    if(p==ptr)
    {
        p=p+strlen("MSC#SUREBOOT");
        if(check_su_key(p)>0)
        {
            poolp->location=mdatap->sms->number;
            poolp->command=MSC_REBOOT;
            poolp->argu=999;
            SMS_CMD_Phone_valid=1;
            return result;
        }
    }
    p=strstr(ptr,"MSC#MIOC");
    if(p==ptr)
    {
        poolp->location=mdatap->sms->number;
        poolp->command=MSC_MIOC;
        p+=strlen("MSC#MIOC");
        num[0]=*p;
        poolp->argu=atoi(num);
        if(poolp->argu >4 || poolp->argu <1)return DISMATCHED;
        return result;
    }
    p=strstr(ptr,"MSC#MIOP");
    if(p==ptr)
    {
        poolp->location=mdatap->sms->number;
        poolp->command=MSC_MIOP;
        p+=strlen("MSC#MIOP");
        num[0]=*p;
        poolp->argu=atoi(num);
        if(poolp->argu >4 || poolp->argu <1)return DISMATCHED;
        return result;
    }
    p=strstr(ptr,"MSC#EURD");
    if(p==ptr)
    {
        poolp->location=mdatap->sms->number;
        poolp->command=MSC_EURD;
        p+=strlen("MSC#EURD");
        num[0]=*p;
        poolp->argu=atoi(num)-1;
        if(poolp->argu >2 || poolp->argu<0 )return DISMATCHED;
        return result;
    }
    p=strstr(ptr,"MSC#NMS");
    if(p==ptr)
    {
        poolp->location=mdatap->sms->number;
        poolp->command=MSC_EURD;
        poolp->argu=3;
        return result;
    }
    p=strstr(ptr,"MSC#GPSR");
    if(p==ptr)
    {
        poolp->location=mdatap->sms->number;
        poolp->command=MSC_GPSR;
        p+=strlen("MSC#GPSR");
        num[0]=*p;
        poolp->argu=atoi(num)-1;
        if(poolp->argu >3 || poolp->argu<0 )return DISMATCHED;
        return result;
    }
    p=strstr(ptr,"MSC#WEB");
    if(p==ptr)
    {
        poolp->location=mdatap->sms->number;
        poolp->command=MSC_WEBCLIENT;
        poolp->argu=0;
        return result;
    }
    p=strstr(ptr,"MSC#APN=");//MSC#APN=apnstr[,usr=][,pwd=]
    if(p==ptr)
    {
        char *p0;
        usr_str[0]=10;
        pwd_str[0]=10;
        usr_str[1]=0;
        pwd_str[1]=0;
        apn_str[0]=0;
        poolp->argu=0;
        poolp->location=mdatap->sms->number;
        poolp->command=MSC_APN;
        p0=p+strlen("MSC#APN=");
        p=strchr(p0,',');
        if(p==NULL)
        {
            strcpy(apn_str,p0);
            return result;
        }
        int n=p-p0;
        strncpy(apn_str,p0,n);
        apn_str[n]=0;

        p=strstr(ptr,",usr=");
        if(p!=NULL)
        {
            p0=p+strlen(",usr=");
            p=strchr(p0,',');
            if(p==NULL)strcpy(usr_str,p0);
            else
            {
                n=p-p0;
                strncpy(usr_str,p0,n);
                usr_str[n]=0;
            }
        }

        p=strstr(ptr,",pwd=");
        if(p!=NULL)
        {
            p0=p+strlen(",pwd=");
            p=strchr(p0,',');
            if(p==NULL)strcpy(pwd_str,p0);
            else
            {
                n=p-p0;
                strncpy(pwd_str,p0,n);
                pwd_str[n]=0;
            }
        }

        return result;
    }

    result = DISMATCHED;

    return result;

}


static int decode_new_sms_list(msc_t * poolp)
{
    gn_raw_data grawdata;
    gn_sms message;
    gn_data mdata;

    gn_error    error = GN_ERR_NONE;
    char *pos;
    char *tmp;
    char *buffp = NULL;
    char *buff;
    char *dp;
    int length;
    int result = DISMATCHED;

    gn_data_clear(data);
    memset(&message, 0, sizeof(gn_sms));
    message.memory_type = gn_str2memory_type("SM");
    message.number = 0;  //unread
    data->sms = &message;
    data->raw_data = &grawdata;
    state->config.connect_script[0]=0;  //standard
    error = gn_smslist_get(data,state);
    if (error == GN_ERR_NONE) {

        tmp = data->raw_data->data;
        dp = tmp;
        pos = tmp;
        int i=0;
        tmp = strstr(pos,"+CMGL: ");
        if (!tmp) {
            goto rout;
        }

        while (pos) {

            pos = strstr(tmp+7, "+CMGL: ");
            if (pos)
                length = pos-tmp;
            else
                length = data->raw_data->length - (tmp-dp);
            if (length<=0) {
                syslog(LOGOPTS,"MSMSCOMMAND: error raw_data length=%d\n",length);
                break;
            }
            buff = (char*)realloc(buffp, length*sizeof(char));
            if (buff) {
                gn_sms_raw rawsms;
                buffp=buff;

                memcpy(buff, tmp, length);
                gn_data_clear(&mdata);
                memset(&message, 0, sizeof(gn_sms));
                message.memory_type = gn_str2memory_type("SM");
                memset(&rawsms, 0, sizeof(gn_sms_raw));
                mdata.raw_sms = &rawsms;
                mdata.sms = &message;

                error = decodesms(buff,length,&mdata,state);
                if (error != GN_ERR_NONE) {
                    syslog(LOGOPTS,"MSMSCOMMAND: decodesms return error=%d\n",error);
                    free(buffp);
                    buffp=NULL;
                    buff=NULL;
                    goto rout;
                }

                error = gn_sms_parse(&mdata);
                if (error != GN_ERR_NONE) {
                    syslog(LOGOPTS,"MSMSCOMMAND: sms_parse return error=%d\n",error);
                    free(buffp);
                    buffp=NULL;
                    buff=NULL;
                    goto rout;
                }
                if (decode_command(&mdata,poolp)) {
                    result = MATCHED;
                    if(SMS_CMD_Filter_Set[0]=='B')
                    {
                        if(strcmp(mdata.sms->remote.number,SMS_CMD_Filter_Phone[0])==0)SMS_CMD_Phone_valid=1;
                        else if(strcmp(mdata.sms->remote.number,SMS_CMD_Filter_Phone[1])==0)SMS_CMD_Phone_valid=1;
                        else if(strcmp(mdata.sms->remote.number,SMS_CMD_Filter_Phone[2])==0)SMS_CMD_Phone_valid=1;
                        else if(strcmp(mdata.sms->remote.number,SMS_CMD_Filter_Phone[3])==0)SMS_CMD_Phone_valid=1;
                        else if(strcmp(mdata.sms->remote.number,SMS_CMD_Filter_Phone[4])==0)SMS_CMD_Phone_valid=1;
                        else if(strcmp(mdata.sms->remote.number,SMS_CMD_Filter_Phone[5])==0)SMS_CMD_Phone_valid=1;
                    }else SMS_CMD_Phone_valid=1;

                    char sms_time[40];
                    get_local_time(mdata.sms, sms_time);
                    sprintf(strOut, "<tr><td>%s</td><td>%s</td><td>%s</td><td>", mdata.sms->remote.number,sms_time,mdata.sms->user_data[0].u.text);
                    break;  
                }
            } else {
                free(buffp);
                buffp=NULL;
                buff=NULL;
                goto rout;
            }

            tmp=pos;
        }

        free(buffp);
        buffp=NULL;
        buff=NULL;
    }
    rout:
    if (grawdata.data) free(grawdata.data);
    grawdata.data=NULL;
    return result;
}

static void exec_command(msc_t * poolp)
{
    struct tm *ttime;
    time_t tm = time(0);
    char time_string[100]; 
    int rc;
    ttime = localtime(&tm);

    if(SMS_CMD_Phone_valid!=1)
    {
        sprintf(time_string,"Not valid phone. None to do. @%s</td></tr>\r\n",asctime(ttime));
        strcat(strOut,time_string);
        return;
    }

    int command=poolp->command;
    int argu=poolp->argu;
    char sp[16];
    switch (command) {
    case MSC_REBOOT:
        if(System_SMS_Commad_En!='B' && poolp->argu!=999)
        {
            sprintf(time_string,"Disabled:reboot @%s</td></tr>\r\n",asctime(ttime));
            strcat(strOut,time_string);
            sms_history_to_flash(strOut,CmdHistoryFile);
            strOut[0]=0;
            time_string[0]=0;
            return;
        }

        syslog(LOGOPTS,"MSMSCOMMAND:reboot\n");
        //Menu_after_show_process(SYSTEM_TOOLS_SUB_MENU_REBOOT_SYSTEM, BUTTON_SUBMIT);
        sprintf(time_string,"Run:reboot @%s</td></tr>\r\n",asctime(ttime));
        strcat(strOut,time_string);
        if(poolp->argu!=999)sms_history_to_flash(strOut,CmdHistoryFile);
        if(poolp->argu!=999)log_info_to_flash("SMS COMMAND Runing:reboot\n");
        else log_info_to_flash("SMS COMMAND Runing:reboot for test\n");
        strOut[0]=0;
        time_string[0]=0;
        SubProgram_Start(scriptFile,"reboot");
        return;

    case MSC_EURD:
        if(System_SMS_Commad_En!='B')
        {
            if(poolp->argu==3)
                sprintf(time_string,"Disabled:Send NMS report @%s</td></tr>\r\n",asctime(ttime));
            else
                sprintf(time_string,"Disabled:Send event report No.%d @%s</td></tr>\r\n",(poolp->argu+1),asctime(ttime));
            break;
        }

        syslog(LOGOPTS,"MSMSCOMMAND:eurd%d\n",poolp->argu);
        sprintf(sp,"msc_eurd %d",poolp->argu);
        SubProgram_Start(scriptFile, sp);
        if(poolp->argu==3)
            sprintf(time_string,"Send NMS report @%s</td></tr>\r\n",asctime(ttime));
        else
            sprintf(time_string,"Send event report No.%d @%s</td></tr>\r\n",(poolp->argu+1),asctime(ttime));
        break;
    case MSC_GPSR:
        if(System_SMS_Commad_En!='B')
        {
            sprintf(time_string,"Disabled:Send gps report No.%d @%s</td></tr>\r\n",(poolp->argu+1),asctime(ttime));
            break;
        }
        syslog(LOGOPTS,"MSMSCOMMAND:gpsr%d\n",poolp->argu);
        sprintf(sp,"msc_gpsr %d",poolp->argu);
        SubProgram_Start(scriptFile, sp);
        sprintf(time_string,"Send gps report No.%d @%s</td></tr>\r\n",(poolp->argu+1),asctime(ttime));
        break;
    case MSC_WEBCLIENT:
        if(System_SMS_Commad_En!='B')
        {
            sprintf(time_string,"Disabled:Send webclient request @%s</td></tr>\r\n",asctime(ttime));
            break;
        }
        syslog(LOGOPTS,"MSMSCOMMAND:webclient request\n");
        sprintf(sp,"msc_webclient");
        SubProgram_Start(scriptFile, sp);
        sprintf(time_string,"Send webclient request @%s</td></tr>\r\n",asctime(ttime));
        break;
    case MSC_APN:
        if(System_SMS_Commad_En!='B')
        {
            sprintf(time_string,"Disabled:Set apn:%s @%s</td></tr>\r\n",apn_str,asctime(ttime));
            break;
        }

        syslog(LOGOPTS,"MSMSCOMMAND:apn setting:%s @%s\n",apn_str,asctime(ttime));
        sprintf(sp,"msc_apn_set %s",apn_str);
        SubProgram_Start(scriptFile, sp);
        if(usr_str[0]!=10)
        {
            sprintf(sp,"msc_apn_usr %s",usr_str);
            SubProgram_Start(scriptFile, sp);
            syslog(LOGOPTS,"MSMSCOMMAND:apn set user name:%s \n",usr_str);
        }
        if(pwd_str[0]!=10)
        {
            sprintf(sp,"msc_apn_pwd %s",pwd_str);
            SubProgram_Start(scriptFile, sp);
            syslog(LOGOPTS,"MSMSCOMMAND:apn set password:%s \n",pwd_str);
        }
        sprintf(sp,"msc_apn_enable");
        SubProgram_Start(scriptFile, sp);
        sprintf(time_string,"Set apn:%s @%s</td></tr>\r\n",apn_str,asctime(ttime));
        break;
    case MSC_MIOC:
        if(System_SMS_Commad_En!='B')
        {
            sprintf(time_string,"Disabled:Set output %d closed @%s</td></tr>\r\n",(poolp->argu),asctime(ttime));
            break;
        }

        syslog(LOGOPTS,"MSMSCOMMAND:io %d close\n",poolp->argu);
        sprintf(sp,"msc_ioclose %d",poolp->argu);
        SubProgram_Start(scriptFile,sp);
        sprintf(time_string,"Set output %d closed @%s</td></tr>\r\n",(poolp->argu),asctime(ttime));
        break;
    case MSC_MIOP:
        if(System_SMS_Commad_En!='B')
        {
            sprintf(time_string,"Disabled:Set output %d opened @%s</td></tr>\r\n",(poolp->argu),asctime(ttime));
            break;
        }
        syslog(LOGOPTS,"MSMSCOMMAND:io %d open\n",poolp->argu);
        sprintf(sp,"msc_ioopen %d",poolp->argu);
        SubProgram_Start(scriptFile,sp);
        sprintf(time_string,"Set output %d opened @%s</td></tr>\r\n",(poolp->argu),asctime(ttime));
        break;
    default:
        syslog(LOGOPTS,"MSMSCOMMAND:default command\n");
        sprintf(time_string,"Not valid Command @%s</td></tr>\r\n",(poolp->argu),asctime(ttime));
        break;
    }
    strcat(strOut,time_string);

}


void check_expire_sms_list(void)
{
    gn_raw_data grawdata;
    gn_sms message;
    gn_data mdata;

    gn_error    error = GN_ERR_NONE;
    char *pos;
    char *tmp;
    char *buffp = NULL;
    char *buff;
    char *dp;
    int length;

    struct tm *ttime;
    time_t tm = time(0);
    char time_string[100]; 
    int rc;

    int del_id_list[100];
    int i_del_id;
    for(i_del_id=0;i_del_id<100;i_del_id++)del_id_list[i_del_id]=-1;
    i_del_id=0;

    FILE *sfp=NULL;
    if ((sfp = fopen(ReadsmsFile, "w")) == 0) {
        syslog(LOGOPTS,"SMS fopen %s error\n", ReadsmsFile);
        return 0;
    }

    gn_data_clear(data);
    memset(&message, 0, sizeof(gn_sms));
    message.memory_type = gn_str2memory_type("SM");//GN_MT_ME/SM
     if(smsread_state!=0&&smsread_state!=1&&smsread_state!=4) 
     {
         smsread_state=1;
     }
    message.number = smsread_state;  //number: 0/1/4 at AT_GetSMSList  atgen.c   <stat>: Status = 0  unread, 1  read, 4  all
    data->sms = &message;
    data->raw_data = &grawdata;
    state->config.connect_script[0]=0;  //set as standard; 1 for motorola
    used = 0;
    error = gn_smslist_get(data,state);
    if (error == GN_ERR_NONE) {
        tmp = data->raw_data->data;
        dp = tmp;
        pos = tmp;
        int i=0;
        tmp = strstr(pos,"+CMGL: ");
        if (!tmp) {
            goto rout;
        }
        while (pos) {

            pos = strstr(tmp+7, "+CMGL: ");
            if (pos)
                length = pos-tmp;
            else
                length = data->raw_data->length - (tmp-dp);
            if (length<=0) {
                syslog(LOGOPTS,"advanced_sms length=%d\n",length);
                break;
            }
            buff = (char*)realloc(buffp, length*sizeof(char));
            if (buff) {
                gn_sms_raw rawsms;
                buffp=buff;
                memcpy(buff, tmp, length);
                gn_data_clear(&mdata);
                memset(&message, 0, sizeof(gn_sms));
                message.memory_type = gn_str2memory_type("SM");
                memset(&rawsms, 0, sizeof(gn_sms_raw));
                mdata.raw_sms = &rawsms;
                mdata.sms = &message;
                error = decodesms(buff,length,&mdata,state);
                if (error != GN_ERR_NONE) {
                    syslog(LOGOPTS,"advanced_sms, decodesms return error=%d\n",error);
                    free(buffp);
                    buffp=NULL;
                    buff=NULL;
                    goto rout;
                }
                error = gn_sms_parse(&mdata);
                if (error != GN_ERR_NONE) {
                    syslog(LOGOPTS,"advanced_sms, sms_parse return error=%d\n",error);
                    free(buffp);
                    buffp=NULL;
                    buff=NULL;
                    goto rout;
                }
                  strOut[0]=0;
                  char sms_time[50];
                  sms_time[0]=0;
                  get_local_time(mdata.sms, sms_time);
                  int is_cmd=0;
                if (strlen(mdata.sms->user_data[0].u.text)<30) 
                {
                    //deal with command.
                    msc_t smspool;
                    smspool.location=0;
                    smspool.command=0;
                    smspool.argu=0;
                    if (decode_command(&mdata,&smspool)) 
                    {
                        ttime = localtime(&tm);
                        sprintf(strOut, "<tr><td>%s</td><td>%s</td><td>%s</td><td>Expired, no running. @%s</td></tr>", mdata.sms->remote.number,sms_time,mdata.sms->user_data[0].u.text,asctime(ttime));
                        sms_history_to_flash(strOut,CmdHistoryFile);
                        is_cmd=1;
                        del_id_list[i_del_id]=mdata.sms->number;
                        i_del_id++;
                    }

                }
                if(is_cmd==0) {
                    char tmp_phone_q[20];
                    strcpy(tmp_phone_q,mdata.sms->remote.number);
                    if(tmp_phone_q[0]=='+')
                    {
                        tmp_phone_q[0]='Q';
                    }
                    fprintf(sfp, "<tr><td>%d</td><td>%s</td><td>%s</td><td>%s <a href=carrier-sms.sh?deleteid=%dP%s>Delete</a> <a href=carrier-sms.sh?replyid=%dP%s>Reply</a></td></tr>\r\n", (used+1),mdata.sms->remote.number,sms_time,mdata.sms->user_data[0].u.text,mdata.sms->number,tmp_phone_q,mdata.sms->number,tmp_phone_q);
                    used++;
                }
                //syslog(LOGOPTS,"advanced_sms used=%d\n",used);
            } else {
                free(buffp);
                buffp=NULL;
                buff=NULL;
                goto rout;
            }


            tmp=pos;
        }

        free(buffp);
        buffp=NULL;
        buff=NULL;
    }


    rout:
    while(i_del_id>0)
    {
        i_del_id--;
        if(del_id_list[i_del_id]>-1)delete_one_sms(del_id_list[i_del_id]);
    }
    if(del_id_list[0]>=0)sleep(1);


    if(used == 0)
    {
        fprintf(sfp, "<tr><td colspan=4>There is no message untreated in sim card.</td></tr>\r\n");
    }
    fflush(sfp);
    fclose(sfp);
    if (grawdata.data) free(grawdata.data);
    grawdata.data=NULL;

    return;
}


//localsukey
//calsukey phone imei 
int main(int argc, char ** argv)
{
    if(argc==2)
    {
        if(strcmp(argv[1],"localsukey")==0)
        {
            if(calculate_loc_key()<=0)printf("Error,please try again.\n");
            return 0;
        }

        return 0;
    }

    if(argc==4) 
    {
        if(strcmp(argv[1],"calsukey")==0)
        {
            char keybuff[30];
            if(calculate_key(argv[2],argv[3],keybuff)<=0)printf("Error,please try again with:calsukey phone imei.\n");
            printf("Phone:%s\n",argv[2]);
            printf("IMEI:%s\n",argv[3]);
            printf("Key:%s\n",keybuff);
            return 0;
        }

        return 0;
    }

    int fd;
    int sbuf_account=0;
    char sendbuf[160];
    msc_t smspool;
    int i, first=0;
    int res = DISMATCHED;
    FILE* fp;
    char set_System_SMS_Commad[5];
    int read_all_sms=0;

    (void) setsid();                                  


    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        syslog(LOGOPTS,"MSMSCOMD EXIT:Out of memory\n");
        return -1;
    }

    set_System_SMS_Commad[0]=0;
    if(get_option_by_section_name(ctx, "msmscomd","msms_conf","System_SMS_Commad",set_System_SMS_Commad)==false)return -2;

    System_SMS_Commad_En=set_System_SMS_Commad[0];
    //if(set_System_SMS_Commad[0]!='B')
    //{
    //    syslog(LOGOPTS,"MSMSCOMMAND:disable, so sms command exits\n");
    //    exit(0);
    //}

    SMS_CMD_Filter_Set[0]=0;
    get_option_by_section_name(ctx, "msmscomd","msms_conf","SMS_CMD_Filter_Set",SMS_CMD_Filter_Set);
    if(SMS_CMD_Filter_Set[0]=='B')
    {
        SMS_CMD_Filter_Phone[0][0]=0;
        SMS_CMD_Filter_Phone[1][0]=0;
        SMS_CMD_Filter_Phone[2][0]=0;
        SMS_CMD_Filter_Phone[3][0]=0;
        SMS_CMD_Filter_Phone[4][0]=0;
        SMS_CMD_Filter_Phone[5][0]=0;
        get_option_by_section_name(ctx, "msmscomd","msms_conf","SMS_CMD_Filter_Phone1",SMS_CMD_Filter_Phone[0]);
        get_option_by_section_name(ctx, "msmscomd","msms_conf","SMS_CMD_Filter_Phone2",SMS_CMD_Filter_Phone[1]);
        get_option_by_section_name(ctx, "msmscomd","msms_conf","SMS_CMD_Filter_Phone3",SMS_CMD_Filter_Phone[2]);
        get_option_by_section_name(ctx, "msmscomd","msms_conf","SMS_CMD_Filter_Phone4",SMS_CMD_Filter_Phone[3]);
        get_option_by_section_name(ctx, "msmscomd","msms_conf","SMS_CMD_Filter_Phone5",SMS_CMD_Filter_Phone[4]);
        get_option_by_section_name(ctx, "msmscomd","msms_conf","SMS_CMD_Filter_Phone6",SMS_CMD_Filter_Phone[5]);
        if(strlen(SMS_CMD_Filter_Phone[0])<3)SMS_CMD_Filter_Phone[0][0]=0;
        if(strlen(SMS_CMD_Filter_Phone[1])<3)SMS_CMD_Filter_Phone[1][0]=0;
        if(strlen(SMS_CMD_Filter_Phone[2])<3)SMS_CMD_Filter_Phone[2][0]=0;
        if(strlen(SMS_CMD_Filter_Phone[3])<3)SMS_CMD_Filter_Phone[3][0]=0;
        if(strlen(SMS_CMD_Filter_Phone[4])<3)SMS_CMD_Filter_Phone[4][0]=0;
        if(strlen(SMS_CMD_Filter_Phone[5])<3)SMS_CMD_Filter_Phone[5][0]=0;
    }



    for (;;) {

        if(check_carrier_satus()!=1)
        {
            sleep(2);
            continue;
        }


        strOut[0]=0;
        SMS_CMD_Phone_valid=0;
        fd = open ( RadioBusyFlagFile, O_WRONLY);
        if (fd<0) {
            syslog(LOGOPTS,"MSMSCOMMAND: error %s:fd<0\n",__func__);
            goto read_diag_fail_process;
        }
        LockFile(fd); 
        gn_lib_businit(&data, &state);
        if(read_all_sms==0)
        {
            smsread_state=4;
            check_expire_sms_list();
            read_all_sms=1;
            res=DISMATCHED;
        }else
        {
            smspool.location=0;
            smspool.command=0;
            smspool.argu=0;
            res = decode_new_sms_list(&smspool);
        }
        gn_lib_busterminate(&state);


        if (res == MATCHED) {
            syslog(LOGOPTS,"MSMSCOMMAND: delete sms%d\n",smspool.location);
            gn_lib_businit(&data, &state);
            delete_one_sms(smspool.location);
            gn_lib_busterminate(&state);
            
        }

        gn_lib_businit(&data, &state);
        smsread_state=1;
        get_new_sms_list();
        gn_lib_busterminate(&state); 

        read_diag_fail_process:
        UnLockFile(fd);
        if (res == MATCHED)
        {
           exec_command(&smspool);
           res = DISMATCHED;
        }

        if(strlen(strOut)>10)
        {
            //printf("\n*****log:%s****\n",strOut);
            sms_history_to_flash(strOut,CmdHistoryFile);
        }



        sleep(5);
    }


    UnLockFile(fd);
    return 0;
}
/* end of source code */



