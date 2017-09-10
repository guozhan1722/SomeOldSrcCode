
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <sys/sysinfo.h>
#include <assert.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdarg.h>
#ifndef __GLIBC__
    #include <linux/if_arp.h>
    #include <linux/if_ether.h>
#endif

#include "msmslib.h"
#include "mii.h"

#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR) 

char *readoutfile="/var/run/datausagestat";
char *tmp_daystat="/var/run/daystat";
char *etc_daystat="/etc/carrierstat/daystat";
char *etc_savedir="/etc/carrierstat/mon";

char hostname[50];
char ifname_carrier[30];

char c_DataUseMonitor_Set='0';
char c_M_Notice_Set,c_D_Notice_Set,c_M_DataUnit_Type,c_D_DataUnit_Type;
uint64_t il_M_Notice_Limit,il_D_Notice_Limit;
unsigned int i_M_Period_Day;
char s_M_SMS_Phone[30],s_D_SMS_Phone[30];
char s_M_SMTP_Mail_Subject[100],s_M_SMTP_Server[50],s_M_SMTP_User_Name[30],s_M_SMTP_Password[30],s_M_SMTP_Recipient[50];
char s_D_SMTP_Mail_Subject[100],s_D_SMTP_Server[50],s_D_SMTP_User_Name[30],s_D_SMTP_Password[30],s_D_SMTP_Recipient[50];
uint64_t limit_to_save;

typedef struct month_st
{
    int year_id;
    uint64_t day_all[32];
}month_st;
typedef struct cur_month_st
{
    int year_id;
    int mon_id;
    uint64_t day_rx[32];
    uint64_t day_tx[32];
}cur_month_st;
typedef struct cur_day_st
{
    int year_id;
    int mon_id;
    int day_id;
    uint64_t day_rx;
    uint64_t day_tx;
    uint64_t day_all;
    uint64_t rx_rec;
    uint64_t tx_rec;
    uint64_t all_rec;
}cur_day_st;
struct month_st month_stat[13];
//for internel use, only when need wirte save mon file, it will update.
struct cur_month_st cur_month_stat;
struct cur_day_st cur_day_stat;
uint64_t now_rx_rec;
uint64_t now_tx_rec;
uint64_t now_all_rec;
int now_year;
int now_mon;
int now_day;

char last_alert_m[12]="1900-00";
char last_alert_d[10]="00-00";
uint64_t cur_month_usage_all=0;
uint64_t last_cur_month_usage_all=0;
uint64_t last_cur_day_usage_all=0;


int fetch_Local_IP_ADDR(char *ifname, char *addr)
{
    struct ifreq ifr;
    int sock;
    char *p;
    char temp[32]="0.0.0.0";

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if (-1==sock)
    {
        syslog(LOGOPTS,"fetch_Local_IP_ADDR: socket=-1");  
        perror("fetch_Local_IP_ADDR ");
        strcpy(addr, temp); 
        return false;
    }
    strncpy(ifr.ifr_name,ifname,sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';   
    if (-1==ioctl(sock, SIOCGIFADDR, &ifr))
    {
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

int send_email_notice(char type_md,char *msg)
{
    char my_ip[32]="0.0.0.0";
    char tmpbuf[256];
    FILE *sfp=NULL;
    int item=0;

     if ((sfp = fopen("/tmp/run/datausage.conf", "w")) == 0) 
     {
         syslog(LOGOPTS,"Data usage send email init conf fopen: %s\n",  strerror(errno));
         return -1;
     }
     fprintf(sfp, "hostname=%s\n", hostname);
     fprintf(sfp, "FromLineOverride=YES\n"); 
     if(type_md=='m')
     {
         fprintf(sfp, "mailhub=%s\n", s_M_SMTP_Server);
         fprintf(sfp, "AuthUser=%s\n", s_M_SMTP_User_Name);
         fprintf(sfp, "AuthPass=%s\n", s_M_SMTP_Password);
     }
     if(type_md=='d')
     {
         fprintf(sfp, "mailhub=%s\n", s_D_SMTP_Server);
         fprintf(sfp, "AuthUser=%s\n", s_D_SMTP_User_Name);
         fprintf(sfp, "AuthPass=%s\n", s_D_SMTP_Password);
     }
     fprintf(sfp, "UseTLS=YES\n");  
     fflush(sfp); 
     fclose(sfp);

     struct tm *ttime;
     time_t tm0 = time(0);
     ttime = localtime(&tm0);

    if ((sfp = fopen("/tmp/run/datause_email.dat", "w")) == 0) {
        syslog(LOGOPTS,"Data usage send email content file fopen: %s\n", strerror(errno));
        return -1;
    }

    fetch_Local_IP_ADDR(ifname_carrier,my_ip);
    fprintf(sfp, "From: %s@%s\r\n", hostname, my_ip);
    fprintf(sfp, "Sender: %s@%s\r\n", hostname, my_ip);
    if(type_md=='m')
    {
        fprintf(sfp, "Subject: %s\r\n", s_M_SMTP_Mail_Subject);
        fprintf(sfp, "To: %s\r\n", s_M_SMTP_Recipient);
        sprintf(tmpbuf,"noticesmtp %s",s_M_SMTP_Recipient);
    }
    if(type_md=='d')
    {
        fprintf(sfp, "Subject: %s\r\n", s_D_SMTP_Mail_Subject);
        fprintf(sfp, "To: %s\r\n", s_D_SMTP_Recipient);
        sprintf(tmpbuf,"noticesmtp %s",s_D_SMTP_Recipient);
    }

    fprintf(sfp, "MIME-Version: 1.0\r\n"); 
    fprintf(sfp, "Content-Type: multipart/mixed; boundary=\"bound123\"\r\n");  
    fprintf(sfp, "--bound123\r\n");
    fprintf(sfp, "Content-Type: text/plain; charset=ISO-8859-1 \r\n");  
    fprintf(sfp, "Content-Transfer-Encoding: binary\r\n");  
    fprintf(sfp, "Content-Disposition: inline\r\n");   
    fprintf(sfp, "\r\n");           
    fprintf(sfp,msg);
    fprintf(sfp, "\r\n"); 
    fprintf(sfp, "From:%s@%s Record Time:%s\r\n",hostname,my_ip,asctime(ttime));           
    fprintf(sfp, "\r\n"); 
    fprintf(sfp, "----Microhard Systems Inc.----\r\n"); 
    fprintf(sfp, "--bound123--\r\n");    
    fflush(sfp); 
    fclose(sfp);

    //sprintf(tmpbuf,"-C/tmp/run/datausage.conf %s < /tmp/run/datause_email.dat &",);
    if (SubProgram_Start("/bin/datausagescript",tmpbuf))
        return 1;
    else {
        syslog(LOGOPTS,"Data Usage send mail send notice email error\n");
        return -1;
    }
}

int send_sms_notice(char *phonenum,char *msg)
{
    if(strlen(phonenum)<4||*msg==0)return 0;

    int fd;
    fd = open ( RadioBusyFlagFile, O_WRONLY);
    if (fd<0) {
        syslog(LOGOPTS,"%s send sms notice:fd<0\n",__func__);
        return -1;
    }
    LockFile(fd); 
    businit();
    send_sms(phonenum, msg, strlen(msg));
    busterminate();
    UnLockFile(fd);

    return 1;
}

void read_config()
{
    char tmpbuff[20];
    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","DataUseMonitor_Set",tmpbuff);
    c_DataUseMonitor_Set=tmpbuff[0];
    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","M_Notice_Set",tmpbuff);
    c_M_Notice_Set=tmpbuff[0];
    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","D_Notice_Set",tmpbuff);
    c_D_Notice_Set=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","M_DataUnit_Type",tmpbuff);
    c_M_DataUnit_Type=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","D_DataUnit_Type",tmpbuff);
    c_D_DataUnit_Type=tmpbuff[0];
    
    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","M_Period_Day",tmpbuff);
    i_M_Period_Day=atoi(tmpbuff);
    if(i_M_Period_Day<1||i_M_Period_Day>31)
    {
       i_M_Period_Day=1;   
    }

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","M_Notice_Limit",tmpbuff);
    il_M_Notice_Limit=atoi(tmpbuff);
    if(c_M_DataUnit_Type=='M')
    {
        il_M_Notice_Limit=il_M_Notice_Limit<<20;
    }else if(c_M_DataUnit_Type=='G')
    {
        il_M_Notice_Limit=il_M_Notice_Limit<<30;
    }else if(c_M_DataUnit_Type=='K')
    {
        il_M_Notice_Limit=il_M_Notice_Limit<<10;
    }

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","D_Notice_Limit",tmpbuff);
    il_D_Notice_Limit=atoi(tmpbuff);
    if(c_D_DataUnit_Type=='M')
    {
        il_D_Notice_Limit=il_D_Notice_Limit<<20;
    }else if(c_D_DataUnit_Type=='G')
    {
        il_D_Notice_Limit=il_D_Notice_Limit<<30;
    }else if(c_D_DataUnit_Type=='K')
    {
        il_D_Notice_Limit=il_D_Notice_Limit<<10;
    }


    s_M_SMS_Phone[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","M_SMS_Phone",s_M_SMS_Phone);
    s_D_SMS_Phone[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","D_SMS_Phone",s_D_SMS_Phone);
    s_M_SMTP_Mail_Subject[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","M_SMTP_Mail_Subject",s_M_SMTP_Mail_Subject);
    s_M_SMTP_Server[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","M_SMTP_Server",s_M_SMTP_Server);
    s_M_SMTP_User_Name[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","M_SMTP_User_Name",s_M_SMTP_User_Name);
    s_M_SMTP_Password[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","M_SMTP_Password",s_M_SMTP_Password);
    s_M_SMTP_Recipient[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","M_SMTP_Recipient",s_M_SMTP_Recipient);
    s_D_SMTP_Mail_Subject[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","D_SMTP_Mail_Subject",s_D_SMTP_Mail_Subject);
    s_D_SMTP_Server[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","D_SMTP_Server",s_D_SMTP_Server);
    s_D_SMTP_User_Name[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","D_SMTP_User_Name",s_D_SMTP_User_Name);
    s_D_SMTP_Password[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","D_SMTP_Password",s_D_SMTP_Password);
    s_D_SMTP_Recipient[0]=0;
    get_option_by_section_name(ctx, "datausemonitor","datausage_conf","D_SMTP_Recipient",s_D_SMTP_Recipient);

    strcpy(hostname,"Microhard");
    get_option_by_section_name(ctx, "system", "@system[0]", "hostname", hostname);
}



void read_curday_data(int need_change)
{
    //uint64_t  atoll()
    FILE *f;
    char tmp_buff[256];
    char *p;
    char *p1;
    int i,j;
    int need_save=0;
    bzero(&cur_day_stat,sizeof(cur_day_stat));
    now_rx_rec=0;
    now_tx_rec=0;
    now_all_rec=0;

    j=-1;
    f=NULL;
    f = fopen(tmp_daystat, "r");
    if(f==NULL)f = fopen(etc_daystat, "r");
    else need_save=1;
    if(f==NULL)
    {
        time_t tt;
        tt=time(NULL);
        struct tm *now0;
        now0 = localtime(&tt); 
        cur_day_stat.year_id=now0->tm_year+1900;
        cur_day_stat.mon_id=now0->tm_mon+1;
        cur_day_stat.day_id=now0->tm_mday;

    }else
    {
        tmp_buff[0]=0;
        while(fgets(tmp_buff, 255, f))
        {
            p=NULL;
            p=strchr(tmp_buff,':');
            if(p==NULL)continue;
            p++;

            p1=strstr(tmp_buff,"RX bytes");
            if(p1!=NULL)
            {
                //RX bytes:996964 (973.5 KiB)  TX bytes:634096 (619.2 KiB)
                while(*p<'0'||*p>'9')p++;
                p1=p;
                while(*p>='0'&&*p<='9')p++;
                *p=0;
                sscanf(p1,"%llu",&now_rx_rec);
                while(*p!=':' && *p!=0 )p++;
                if(*p==0)
                {
                    now_rx_rec=0;
                    continue;
                }
                p++;
                p1=p;
                while(*p>='0'&&*p<='9')p++;
                *p=0;
                sscanf(p1,"%llu",&now_tx_rec);
                now_all_rec=now_rx_rec+now_tx_rec;
                //printf("*******304:%llu,%llu,%llu,%llu\n",now_rx_rec,now_tx_rec,now_all_rec,cur_day_stat.rx_rec,cur_day_stat.day_rx);
                continue;
            }

            if(tmp_buff[0]=='d'&&tmp_buff[1]=='a'&&tmp_buff[2]=='t'&&tmp_buff[3]=='e')
            {
                sscanf(p, "%4d-%2d-%2d*",&cur_day_stat.year_id, &cur_day_stat.mon_id, &cur_day_stat.day_id);
                //printf("###1:%s:%d,%d,%d\n",p,cur_day_stat.year_id, cur_day_stat.mon_id, cur_day_stat.day_id);
                continue;
            }


            if(tmp_buff[0]>='0' && tmp_buff[0]<='9')
            {
                if(tmp_buff[0]=='0' && tmp_buff[1]==':')
                {
                    sscanf(p, "%llu,%llu;%llu",&cur_day_stat.rx_rec, &cur_day_stat.tx_rec, &cur_day_stat.all_rec);
                    cur_day_stat.all_rec=cur_day_stat.rx_rec+cur_day_stat.tx_rec;
                }else
                {
                    sscanf(tmp_buff, "%d:%llu,%llu;%llu",&j,&cur_day_stat.day_rx, &cur_day_stat.day_tx, &cur_day_stat.day_all);
                    cur_day_stat.day_all=cur_day_stat.day_rx+cur_day_stat.day_tx;
                }
            }
        }
        fclose(f);
    }

    if(now_rx_rec>cur_day_stat.rx_rec && now_tx_rec>cur_day_stat.tx_rec)
    {
        cur_day_stat.day_rx+=now_rx_rec-cur_day_stat.rx_rec;
        cur_day_stat.day_tx+=now_tx_rec-cur_day_stat.tx_rec;
        cur_day_stat.day_all=cur_day_stat.rx_rec+cur_day_stat.tx_rec;
        cur_day_stat.rx_rec=now_rx_rec;
        cur_day_stat.tx_rec=now_tx_rec;
        cur_day_stat.all_rec=now_rx_rec+now_tx_rec;
    }

    if(need_save==1 && need_change==1)
    {
        save_day_file();
        unlink(tmp_daystat);
    }
}

void read_history_data()
{
    //uint64_t  atoll()
    FILE *f;
    char tmp_buff[256];
    char *p;
    char *p1;
    int i,j;
    bzero(month_stat,sizeof(month_stat));
    bzero(&cur_month_stat,sizeof(cur_month_stat));

    //if(cur_day_stat.mon_id!=0 && cur_day_stat.year_id!=0)
    {
        //it is sure
        cur_month_stat.mon_id=cur_day_stat.mon_id;
        cur_month_stat.year_id=cur_day_stat.year_id;
    }

    for(i=1;i<13;i++) 
    {
        f=NULL;
        sprintf(tmp_buff,"%s%d",etc_savedir,i);
        if (f = fopen(tmp_buff, "r"))
        {
            tmp_buff[0]=0;

            while(fgets(tmp_buff, 255, f))
            {
                p=NULL;
                p=strchr(tmp_buff,':');
                if(p!=NULL)
                {
                    *p=0;
                    if(p-tmp_buff>2)
                    {
                        if(strcmp(tmp_buff,"year")==0)
                        {
                            p++;
                            month_stat[i].year_id=atoi(p);
                            continue;
                        }
                        break;
                    }

                    j=atoi(tmp_buff);
                    if(j<0||j>31)break;
                    p++;
                    p1=p;
                    p=NULL;
                    p=strchr(p1,';');
                    if(p==NULL)continue;
                    p++;
                    sscanf(p,"%llu",&month_stat[i].day_all[j]);
                    
                    if(i==cur_month_stat.mon_id && month_stat[i].year_id==cur_month_stat.year_id)
                    {
                        *p=0;
                        sscanf(p1,"%llu,%llu*",&cur_month_stat.day_rx[j],&cur_month_stat.day_tx[j]);
                    }
                }
            }
            fclose(f);
        }
    }

    if(cur_month_stat.year_id!=month_stat[cur_month_stat.mon_id].year_id)//no cur day's month data record, clear and trans cur day data.
    {
        for(i=0;i<=31;i++)
        {
            month_stat[cur_month_stat.mon_id].day_all[i]=0;
        }
        cur_month_stat.day_rx[cur_day_stat.day_id]=cur_day_stat.day_rx;
        cur_month_stat.day_tx[cur_day_stat.day_id]=cur_day_stat.day_tx;

        month_stat[cur_month_stat.mon_id].year_id=cur_month_stat.year_id;
        month_stat[cur_month_stat.mon_id].day_all[cur_day_stat.day_id]=cur_day_stat.day_rx+cur_day_stat.day_tx;
        month_stat[cur_month_stat.mon_id].day_all[0]=cur_day_stat.day_rx+cur_day_stat.day_tx;
    }

}


void save_day_tmp_file()
{
    FILE *sfp;
    char tmp_buff[512];
    int i;
    sprintf(tmp_buff,"date:%d-%02d-%02d\n%d:%llu,%llu;%llu\n0:%llu,%llu;%llu\n",cur_day_stat.year_id,cur_day_stat.mon_id,cur_day_stat.day_id, \
            cur_day_stat.day_id,cur_day_stat.day_rx,cur_day_stat.day_tx,cur_day_stat.day_all, \
            cur_day_stat.rx_rec,cur_day_stat.tx_rec,cur_day_stat.all_rec);

    sfp=NULL;
    sfp =fopen(tmp_daystat,"w");
    if(sfp!=NULL) 
    {
        fprintf(sfp,"%s",tmp_buff);
        fflush(sfp);
        fclose(sfp);
    }
}

void save_day_file()
{
    FILE *sfp;
    char tmp_buff[512];
    int i;
    sprintf(tmp_buff,"date:%d-%02d-%02d\n%d:%llu,%llu;%llu\n0:%llu,%llu;%llu\n",cur_day_stat.year_id,cur_day_stat.mon_id,cur_day_stat.day_id, \
            cur_day_stat.day_id,cur_day_stat.day_rx,cur_day_stat.day_tx,cur_day_stat.day_all, \
            cur_day_stat.rx_rec,cur_day_stat.tx_rec,cur_day_stat.all_rec);
    sfp=NULL;
    sfp =fopen(etc_daystat,"w");
    if(sfp!=NULL) 
    {
        fprintf(sfp,"%s",tmp_buff);
        fflush(sfp);
        fclose(sfp);
    }
    sfp=NULL;
    sfp =fopen(tmp_daystat,"w");
    if(sfp!=NULL) 
    {
        fprintf(sfp,"%s",tmp_buff);
        fflush(sfp);
        fclose(sfp);
    }
}

void save_mon_file(int mon_id)
{
    FILE *sfp;
    char tmp_buff[100];
    int i;

    sfp=NULL;
    sprintf(tmp_buff,"%s%d",etc_savedir,mon_id);
    sfp =fopen(tmp_buff,"w");
    if(sfp!=NULL) 
    {
        fprintf(sfp,"year:%d\n",cur_month_stat.year_id);
        for(i=0;i<=31;i++)
        {
            fprintf(sfp,"%d:%llu,%llu;%llu\n",i,cur_month_stat.day_rx[i],cur_month_stat.day_tx[i],month_stat[mon_id].day_all[i]);
        }
        fflush(sfp);
        fclose(sfp);
    }
}

void get_stat_data()
{
    FILE *f;
    char tmp_buff[256];
    char *p;
    char *p1;
    int i,j;
    uint64_t ui_diff;

    now_rx_rec=0;
    now_tx_rec=0;
    f=NULL;
    f = fopen("/proc/net/dev", "r");
    if(f==NULL)return;
    tmp_buff[0]=0;
    while(fgets(tmp_buff, 255, f))
    {
        p=NULL;
        p = strstr(tmp_buff, ifname_carrier);
        if(p==NULL)continue;

        p+=strlen(ifname_carrier);
        for(i=0;i<9;i++)
        {
            while(*p<'0'||*p>'9')p++;
            p1=p;
            while(*p>='0'&&*p<='9')p++;
            if(i==0)
            {
                *p=0;
                sscanf(p1,"%llu",&now_rx_rec);
            }
            if(i==8)
            {
                *p=0;
                sscanf(p1,"%llu",&now_tx_rec);
            }
        }
        break;
    }
    fclose(f);
    now_all_rec=now_tx_rec+now_rx_rec;

    if(now_rx_rec<100)return; //interface just no valid for some time. 

    time_t tt;
    tt=time(NULL);
    struct tm *now0;
    now0 = localtime(&tt); 
    now_year=now0->tm_year+1900;
    now_mon=now0->tm_mon+1;
    now_day=now0->tm_mday;

    //date changed.
    if(cur_day_stat.year_id!=now_year || cur_day_stat.mon_id!=now_mon || cur_day_stat.day_id!=now_day) //
    {
        //deal current date:
        //1)trans cur day to cur month and month_stat  -----be sured cur day and cur mon and month[cur] with same date.
        //2)save mon date file
        //3)get now data to cur day.
        //4)read history data to mon buffer.

        cur_month_stat.day_rx[cur_day_stat.day_id]=cur_day_stat.day_rx;
        cur_month_stat.day_tx[cur_day_stat.day_id]=cur_day_stat.day_tx;

        month_stat[cur_month_stat.mon_id].day_all[0]=0;
        cur_month_stat.day_rx[0]=0;
        cur_month_stat.day_tx[0]=0;
        for(i=1;i<=cur_day_stat.day_id;i++)
        {
            cur_month_stat.day_rx[0]+=cur_month_stat.day_rx[i];
            cur_month_stat.day_tx[0]+=cur_month_stat.day_tx[i];
        }
        month_stat[cur_month_stat.mon_id].day_all[0]=cur_month_stat.day_rx[0]+cur_month_stat.day_tx[0];
        month_stat[cur_month_stat.mon_id].day_all[cur_day_stat.day_id]=cur_month_stat.day_rx[cur_day_stat.day_id]+cur_month_stat.day_tx[cur_day_stat.day_id];

        save_mon_file(cur_month_stat.mon_id);

        cur_day_stat.year_id=now_year;
        cur_day_stat.mon_id=now_mon;
        cur_day_stat.day_id=now_day;
        cur_day_stat.day_rx=0;
        cur_day_stat.day_tx=0;
        cur_day_stat.rx_rec=now_rx_rec;
        cur_day_stat.tx_rec=now_tx_rec;
        cur_day_stat.all_rec=now_all_rec;

        read_history_data();
        save_day_file();

        return;
    }

    int need_save_day=0;
    if(cur_day_stat.day_rx < cur_month_stat.day_rx[cur_day_stat.day_id])//lost day stat file
    {
        cur_day_stat.day_rx=cur_month_stat.day_rx[cur_day_stat.day_id];
        cur_day_stat.day_tx=cur_month_stat.day_tx[cur_day_stat.day_id];
        cur_day_stat.day_all=cur_day_stat.day_rx+cur_day_stat.day_tx;
        need_save_day=1;//        save_day_file();
    }

    //when carrier restart or system restart
    if(now_rx_rec < cur_day_stat.rx_rec || now_all_rec < cur_day_stat.all_rec || now_tx_rec < cur_day_stat.tx_rec)
    {
        cur_day_stat.rx_rec=now_rx_rec;
        cur_day_stat.tx_rec=now_tx_rec;
        cur_day_stat.all_rec=now_all_rec;
        need_save_day=1;//        save_day_file();
    }

    if(now_rx_rec>cur_day_stat.rx_rec)
    {
        ui_diff=now_rx_rec - cur_day_stat.rx_rec;
        cur_day_stat.day_rx += ui_diff;

        if(now_tx_rec>cur_day_stat.tx_rec)
        {
            ui_diff=now_tx_rec - cur_day_stat.tx_rec;
            cur_day_stat.day_tx += ui_diff;
        }

        cur_day_stat.day_all = cur_day_stat.day_rx+cur_day_stat.day_tx;

        cur_day_stat.rx_rec=now_rx_rec;
        cur_day_stat.tx_rec=now_tx_rec;
        cur_day_stat.all_rec=now_all_rec;
    }


    //cur_month will be keep until save day file; month stat can be update anytime.
    //if(cur_day_stat.day_all>month_stat[cur_day_stat.mon_id].day_all[cur_day_stat.day_id] + limit_to_save)
    if(cur_day_stat.day_all>cur_month_stat.day_rx[cur_day_stat.day_id] + cur_month_stat.day_tx[cur_day_stat.day_id] + limit_to_save)
    {

        cur_month_stat.day_rx[cur_day_stat.day_id]=cur_day_stat.day_rx;
        cur_month_stat.day_tx[cur_day_stat.day_id]=cur_day_stat.day_tx;
        month_stat[cur_day_stat.mon_id].day_all[cur_day_stat.day_id]=cur_day_stat.day_all;

        cur_month_stat.day_rx[0]=0;
        cur_month_stat.day_tx[0]=0;
        for(i=1;i<=cur_day_stat.day_id;i++)
        {
            cur_month_stat.day_rx[0]+=cur_month_stat.day_rx[i];
            cur_month_stat.day_tx[0]+=cur_month_stat.day_tx[i];
        }
        month_stat[cur_day_stat.mon_id].day_all[0]=cur_month_stat.day_rx[0]+cur_month_stat.day_tx[0];

        need_save_day=1;//        save_day_file();
        //save_mon_file(cur_month_stat.mon_id);
    }

    if(need_save_day==1)save_day_file();

    return;
}

void get_lastday_usage(uint64_t *p_usage)
{
    int i,j;
    uint64_t usage_cnt;
    struct month_st *p_month_stat=NULL;
    i=cur_day_stat.day_id-1;
    if(i>0)
    {
        *p_usage=month_stat[cur_day_stat.mon_id].day_all[i];
        return;
    }

    //else
    time_t tt;
    tt=time(NULL);
    struct tm *yestoday0;
    tt=tt-3600*24;
    yestoday0 = localtime(&tt); 
    int yt_year=yestoday0->tm_year+1900;
    int yt_mon=yestoday0->tm_mon+1;
    int yt_day=yestoday0->tm_mday;

    usage_cnt=0;
    if(month_stat[yt_mon].year_id==yt_year)
    {
        usage_cnt=month_stat[yt_mon].day_all[yt_day];
    }
    *p_usage=usage_cnt;
}

void get_lastmonth_usage(uint64_t *p_usage)
{
    int i,j;
    uint64_t usage_cnt;
    struct month_st *p_last_month_stat=NULL;
    struct month_st *p_lastlast_month_stat=NULL;

    month_stat[cur_day_stat.mon_id].day_all[cur_day_stat.day_id]=cur_day_stat.day_all;

    month_stat[cur_day_stat.mon_id].day_all[0]=0;
    for(i=1;i<=cur_day_stat.day_id;i++)
    {
        month_stat[cur_day_stat.mon_id].day_all[0]+=month_stat[cur_day_stat.mon_id].day_all[i];
    }

    i=cur_day_stat.mon_id-1;
    if(i<1)
    {
        i+=12;
        j=month_stat[i].year_id+1;
    }else j=month_stat[i].year_id;
    if(j==cur_day_stat.year_id)
    {
        p_last_month_stat=&month_stat[i];
        i=cur_day_stat.mon_id-2;
        if(i<1)
        {
            i+=12;
            j=month_stat[i].year_id+1;
        }else j=month_stat[i].year_id;
        if(j==cur_day_stat.year_id)
        {
            p_lastlast_month_stat=&month_stat[i];
        }
    }

    if(i_M_Period_Day==1) 
    {
        *p_usage=0;
        if(p_last_month_stat!=NULL)
        {
            *p_usage=p_last_month_stat->day_all[0];
        }
        return;
    }

    //if(c_M_Notice_Set=='B'||c_M_Notice_Set=='C') if(i_M_Period_Day!=1)
    *p_usage=0;
    usage_cnt=0;
    if(i_M_Period_Day<=cur_day_stat.day_id)
    {
        for(i=1;i<i_M_Period_Day;i++)
        {
            usage_cnt+=month_stat[cur_day_stat.mon_id].day_all[i];
        }
        if(p_last_month_stat!=NULL)
        {
            for(i=i_M_Period_Day;i<=31;i++)
            {
                usage_cnt+=p_last_month_stat->day_all[i];
            }
        }
    }else
    {
        if(p_last_month_stat!=NULL)
        {
            for(i=1;i<i_M_Period_Day;i++)
            {
                usage_cnt+=p_last_month_stat->day_all[i];
            }
            if(p_lastlast_month_stat!=NULL)
            {
                for(i=i_M_Period_Day;i<=31;i++)
                {
                    usage_cnt+=p_lastlast_month_stat->day_all[i];
                }
            }
        }
    }
    *p_usage=usage_cnt;
}


void cur_month_usage_calulate()
{
    int i,j;

    month_stat[cur_day_stat.mon_id].day_all[cur_day_stat.day_id]=cur_day_stat.day_rx+cur_day_stat.day_tx;

    month_stat[cur_day_stat.mon_id].day_all[0]=0;
    for(i=1;i<=cur_day_stat.day_id;i++)
    {
        month_stat[cur_day_stat.mon_id].day_all[0]+=month_stat[cur_day_stat.mon_id].day_all[i];
    }

    if(i_M_Period_Day==1)
    {
        cur_month_usage_all=month_stat[cur_day_stat.mon_id].day_all[0];
        return;
    }

    //else
    cur_month_usage_all=0;
    if(i_M_Period_Day<=cur_day_stat.day_id)
    {
        for(i=i_M_Period_Day;i<=cur_day_stat.day_id;i++)
        {
            cur_month_usage_all+=month_stat[cur_day_stat.mon_id].day_all[i];
        }
    }else
    {
        for(i=1;i<=cur_day_stat.day_id;i++)
        {
            cur_month_usage_all+=month_stat[cur_day_stat.mon_id].day_all[i];
        }
        j=cur_day_stat.mon_id-1;
        int tmp_year;
        if(j<1)
        {
            j+=12;
            tmp_year=month_stat[j].year_id+1;
        }else tmp_year=month_stat[j].year_id;
        if(tmp_year==cur_day_stat.year_id);
        {
            for(i=i_M_Period_Day;i<=31;i++)
            {
                cur_month_usage_all+=month_stat[j].day_all[i];
            }
        }
    }

}


int check_overlimit()
{
    //char last_alert_m[12]="1900-00";
    //char last_alert_d[10]="00-00";
    char tmp_buff[256];
    uint64_t ui_tmp;
    int send_sms_m=0;
    int return_value=0;

    if(c_M_Notice_Set=='B'||c_M_Notice_Set=='C')
    {
        //special month data
        cur_month_usage_calulate();
        if(last_cur_month_usage_all!=0)
        {
            sprintf(tmp_buff,"%d-%02d",cur_day_stat.year_id,cur_day_stat.mon_id);
            if(strcmp(last_alert_m,tmp_buff)!=0 && cur_month_usage_all>il_M_Notice_Limit && last_cur_month_usage_all<=il_M_Notice_Limit)
            {
                strcpy(last_alert_m,tmp_buff);
                ui_tmp=cur_month_usage_all;
                ui_tmp=ui_tmp>>20;
                sprintf(tmp_buff,"Data Usage Notice:Monthly Over %lluMB %d-%d-%d",ui_tmp,cur_day_stat.year_id,cur_day_stat.mon_id,cur_day_stat.day_id);
                if(c_M_Notice_Set=='B')send_sms_notice(s_M_SMS_Phone,tmp_buff);
                else 
                {
                    send_email_notice('m',tmp_buff);
                    send_sms_m=1;
                }
                return_value=1;
            }
        }
        last_cur_month_usage_all=cur_month_usage_all;
    }

    if(c_D_Notice_Set=='B'||c_D_Notice_Set=='C')
    {
        if(last_cur_day_usage_all!=0)
        {
            sprintf(tmp_buff,"%02d-%02d",cur_day_stat.mon_id,cur_day_stat.day_id);
            if(strcmp(last_alert_d,tmp_buff)!=0 && cur_day_stat.day_all>il_D_Notice_Limit && cur_day_stat.day_all<=il_D_Notice_Limit)
            {
                strcpy(last_alert_d,tmp_buff);
                ui_tmp=cur_day_stat.day_all;
                ui_tmp=ui_tmp>>20;
                sprintf(tmp_buff,"Data Usage Notice:Daily Over %lluMB %d-%d-%d",ui_tmp,cur_day_stat.year_id,cur_day_stat.mon_id,cur_day_stat.day_id);
                if(c_D_Notice_Set=='B')send_sms_notice(s_D_SMS_Phone,tmp_buff);
                else 
                {
                    if(send_sms_m==1)sleep(5);
                    send_email_notice('d',tmp_buff);
                }
                return_value=1;
            }
        }
        last_cur_day_usage_all=cur_day_stat.day_all;
    }
    return return_value;
}

//max 30 len
void trans_bytes_unit(uint64_t num,char *s_result)
{
    float tmp_f;
    uint64_t tmp;
    tmp=1;
    tmp=tmp<<10;
    if(num<tmp)
    {
        sprintf(s_result,"%llu Bytes",num);
        return;
    }

    tmp=tmp<<10;
    if(num<tmp)
    {
        tmp_f=num;
        tmp_f=tmp_f/1024;
        sprintf(s_result,"%.3f KB",tmp_f);
        return;
    }

    tmp=tmp<<10;
    if(num<tmp)
    {
        num=num>>10;
        tmp_f=num;
        tmp_f=tmp_f/1024;
        sprintf(s_result,"%.3f MB",tmp_f);
        return;
    }

    num=num>>20;
    tmp_f=num;
    tmp_f=tmp_f/1024;
    sprintf(s_result,"%.3f GB",tmp_f);
    return;
}

void save_webinfo_file()
{
    FILE *sfp;
    char tmp_buff[256];
    char tmp_font1[50];
    char tmp_font2[30];
    char s_num[40];
    int i;
    uint64_t last_usage;

    month_stat[cur_day_stat.mon_id].day_all[cur_day_stat.day_id]=cur_day_stat.day_all;
    cur_month_stat.day_rx[cur_day_stat.day_id]=cur_day_stat.day_rx;
    cur_month_stat.day_tx[cur_day_stat.day_id]=cur_day_stat.day_tx;

    cur_month_stat.day_rx[0]=0;
    cur_month_stat.day_tx[0]=0;
    for(i=1;i<=cur_day_stat.day_id;i++)
    {
        cur_month_stat.day_rx[0]+=cur_month_stat.day_rx[i];
        cur_month_stat.day_tx[0]+=cur_month_stat.day_tx[i];
    }
    month_stat[cur_day_stat.mon_id].day_all[0]=cur_month_stat.day_rx[0]+cur_month_stat.day_tx[0];

    cur_month_usage_calulate();

    sfp=NULL;
    sfp =fopen(readoutfile,"w");
    if(sfp!=NULL) 
    {
        //<td></td><td></td>
        tmp_font1[0]=0;
        tmp_font2[0]=0;
        if(cur_day_stat.day_all>il_D_Notice_Limit && c_D_Notice_Set!='A')
        {
            strcpy(tmp_font1,"<font color=#FF0000>");
            strcpy(tmp_font2,"</font>");
        }
        trans_bytes_unit(cur_day_stat.day_all,s_num);
        fprintf(sfp,"<tr><td>Today's Usage:</td><td>%s %s %s</td></tr>\n",tmp_font1,s_num,tmp_font2);

        last_usage=0;
        get_lastday_usage(&last_usage);
        tmp_font1[0]=0;
        tmp_font2[0]=0;
        if(last_usage>il_D_Notice_Limit && c_D_Notice_Set!='A')
        {
            strcpy(tmp_font1,"<font color=#FF0000>");
            strcpy(tmp_font2,"</font>");
        }
        trans_bytes_unit(last_usage,s_num);
        fprintf(sfp,"<tr><td>Yesterday's Usage:</td><td>%s %s %s</td></tr>\n",tmp_font1,s_num,tmp_font2);


        tmp_font1[0]=0;
        tmp_font2[0]=0;
        if(cur_month_usage_all>il_M_Notice_Limit && c_M_Notice_Set!='A')
        {
            strcpy(tmp_font1,"<font color=#FF0000>");
            strcpy(tmp_font2,"</font>");
        }
        trans_bytes_unit(cur_month_usage_all,s_num);
        fprintf(sfp,"<tr><td>Current Monthly Usage:</td><td>%s %s %s</td></tr>\n",tmp_font1,s_num,tmp_font2);

        last_usage=0;
        get_lastmonth_usage(&last_usage);
        tmp_font1[0]=0;
        tmp_font2[0]=0;
        if(last_usage>il_M_Notice_Limit && c_M_Notice_Set!='A')
        {
            strcpy(tmp_font1,"<font color=#FF0000>");
            strcpy(tmp_font2,"</font>");
        }
        trans_bytes_unit(last_usage,s_num);
        fprintf(sfp,"<tr><td>Last Monthly Usage:</td><td>%s %s %s</td></tr>\n",tmp_font1,s_num,tmp_font2);


        fflush(sfp);
        fclose(sfp);
    }
}

//usage: br-wan [readout]
//ifname_carrier = "br-wan";     //IPn4G
//ifname_carrier = "br-wan2";     //VIP4G
int main(int argc, char ** argv)
{
    if(argc<2)return 0;

    ifname_carrier[0]=0;
    strcpy(ifname_carrier,argv[1]);
    if(strlen(ifname_carrier)<2)return 0;


    (void) setsid();                                  
    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        syslog(LOGOPTS,"Salertd EXIT:Out of memory\n");
        return -1;
    }


    //argv:
    //"readout": write statistic data in /var/run/datausagestat with <tr><td></td><td></td></tr>
        //includes:before last month, last month, current month, before yesterday, yesterday, today.


    read_config();

    uint64_t u_i=0;
    limit_to_save=0;//Monthly/256 ? Dayly/16 : which less. with Bytes.
    if(c_D_Notice_Set=='B'||c_D_Notice_Set=='C')limit_to_save=il_D_Notice_Limit>>4;
    if(c_M_Notice_Set=='B'||c_M_Notice_Set=='C')
    {
        u_i=il_M_Notice_Limit>>8;
        if(limit_to_save==0 || u_i<limit_to_save)
        {
            limit_to_save=u_i;
        }
    }
    if(limit_to_save==0)limit_to_save=4194304;

    if(argc==3)
    {
        if(strcmp(argv[2],"readout")==0)
        {
            read_curday_data(0);
            read_history_data();
            save_webinfo_file();
        }
        return 0;
    }

    if(argc==4)
    {
        if(strcmp(argv[2],"clear")==0 && strcmp(argv[3],"tozero")==0)
        {
            read_curday_data(0);
            read_history_data();
            get_stat_data();
            get_stat_data();
            get_stat_data();
            cur_day_stat.day_rx=0;
            cur_day_stat.day_tx=0;
            cur_day_stat.day_all=0;
            save_day_file();
        }
        return 0;
    }

    if(c_DataUseMonitor_Set!='B')return 0;

    //read all /etc/carrierstat/mon1,mon2,mon3,...mon12.
    //check /tmp/run/daystat and /etc/carrierstat/daystat and deal it
    read_curday_data(1);
    read_history_data();

    while(1) 
    {
        //read /proc/net/dev and do cycle thing:sleep 5
        get_stat_data();

        //if >check M/D ammount, deal with alert notice
        if(check_overlimit()>0)save_day_file();
        else save_day_tmp_file();
        //printf("%d-%d-%d:%llu,%llu; %llu : %llu, %llu, %llu\n",cur_day_stat.year_id,cur_day_stat.mon_id,cur_day_stat.day_id,cur_day_stat.day_rx,cur_day_stat.day_tx,cur_day_stat.day_all, cur_day_stat.rx_rec,cur_day_stat.tx_rec,cur_day_stat.all_rec);   
           
        sleep(5);   
    }

    return 0;
}


