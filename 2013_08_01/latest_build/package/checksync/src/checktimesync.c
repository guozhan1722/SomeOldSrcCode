
#include <dirent.h>  
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <sys/sysinfo.h>
#include <assert.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>
#include "unistd.h"
#include <syslog.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <net/if.h>
#include <errno.h>
#include <sys/stat.h>
#include <termios.h>
#include <signal.h>
#include <stdbool.h>
#include <netinet/ip.h>
#include <netdb.h>
#include "uci.h"

#define	LOGOPTS		(LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)

struct uci_context *ctx;
int gpsd_port=0;
char gpsd_status;
int checkmode=0;
char newloc_set;
time_t diffsec;
time_t tt,tt_loc;
struct tm *now;

#ifdef VIP4G_SERVER
char *devportname="/dev/ttyUSB0";
#endif

#ifdef IPn4G_SERVER
char *devportname="/dev/ttyUSB1";
#endif



int netlib_connectsock(char *host, char *service, char *protocol)
{
    struct hostent *phe;
    struct servent *pse;
    struct protoent *ppe;
    struct sockaddr_in sin;
    int s, type, proto, one = 1;

    memset((char *) &sin, 0, sizeof(sin));
    /*@ -type -mustfreefresh @*/
    sin.sin_family = AF_INET;
    if ((pse = getservbyname(service, protocol)))
        sin.sin_port = htons(ntohs((unsigned short) pse->s_port));
    else if ((sin.sin_port = htons((unsigned short) atoi(service))) == 0)
        return -3;

    if ((phe = gethostbyname(host)))
        memcpy((char *) &sin.sin_addr, phe->h_addr, phe->h_length);
    else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
        return -2;

    ppe = getprotobyname(protocol);
    if (strcmp(protocol, "udp") == 0) {
        type = SOCK_DGRAM;
        proto = (ppe) ? ppe->p_proto : IPPROTO_UDP;
    } else {
        type = SOCK_STREAM;
        proto = (ppe) ? ppe->p_proto : IPPROTO_TCP;
    }

    if ((s = socket(PF_INET, type, proto)) < 0)
        return -4;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one))==-1) {
        (void)close(s);
        return -5;
    }

    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        (void)close(s);
        return -6;
    }

#ifdef IPTOS_LOWDELAY
    int opt = IPTOS_LOWDELAY;
    (void)setsockopt(s, IPPROTO_IP, IP_TOS, &opt, sizeof opt);

#endif
#ifdef TCP_NODELAY
    if (type == SOCK_STREAM)
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
#endif
    return s;

}

bool get_option_by_section_name(struct uci_context *ctx,char*package_name,char*section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_element *e= NULL;
    struct uci_ptr ptr;
    struct uci_option *p_option = NULL;

    //syslog(LOGOPTS,"Enter [%s] %s %s\n", __func__, package_name, section_name, option_name);
    
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
             (e->type == UCI_TYPE_OPTION))
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



bool set_option_by_section_name(struct uci_context *ctx,char*package_name, char *section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_ptr ptr;

    //syslog(LOGOPTS,"Enter [%s] %s %s\n", __func__, package_name, section_name, option_name);

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
            uci_commit(ctx, &ptr.p, false);
            }

        free(tuple);
        return true;
        }

    free(tuple);  
    return false;
}



void update_time_record()
{
    char date_buff[30];
    char time_buff[30];
    time_t start;
    start=time(NULL);
    now = localtime(&start);  
    sprintf(date_buff,"%04d.%02d.%02d",(now->tm_year+1900),(now->tm_mon+1),now->tm_mday);
    sprintf(time_buff,"%02d:%02d:%02d",now->tm_hour,now->tm_min,now->tm_sec);

    ctx = uci_alloc_context(); 
    if (!ctx) 
    {
        fprintf(stderr, "Out of memory\n");
        return -1;
    }
    //    config_get sdate "$cfg" date '2012.12.22'
    //    config_get stime "$cfg" time '12:00:00'
    set_option_by_section_name(ctx, "system","datetime","date",date_buff);
    set_option_by_section_name(ctx, "system","datetime","time",time_buff);
    uci_free_context(ctx);
    return;
}

void read_config(int check_type)
{
    char tmp_buff[20];

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsd","gpsd_conf","GPSD_TCP_Port",tmp_buff);
    gpsd_port=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsd","gpsd_conf","status",tmp_buff);
    gpsd_status=tmp_buff[0];

    tmp_buff[0]=0;
    checkmode=0;
    get_option_by_section_name(ctx, "system","datetime","mode",tmp_buff);
    if(strcmp("sync",tmp_buff)==0)checkmode=1;
    else checkmode=2;
    if(check_type!=checkmode)exit(0);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "system","datetime","newloc",tmp_buff);
    newloc_set=tmp_buff[0];

    tmp_buff[0]=0;
    diffsec=0;
    get_option_by_section_name(ctx, "system","datetime","diffsec",tmp_buff);
    if(tmp_buff[0]!=0)sscanf(tmp_buff,"%ld",&diffsec);

}

int SubProgram_Start(char *pszPathName, char *pszArguments)
{
    int  sub_status; 
    int MAX_ARGS=8;
    char * pArgs[MAX_ARGS];
    unsigned long ulNumArgs = 1;
    char tmpArgs[256];
    char *pArg = NULL;
    int pid =0; 
    //syslog(LOGOPTS,"Enter [%s]\n", __func__);   

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

int set_with_gps_time()
{
    char tmp_buff[256];
    char utc_time[8];
    char utc_date[8];
    int readbytes = 0;
    int sock = 0;
    ssize_t wrote = 0;
    int fd_tcp=0;
    char buf[2048];
    char cbuf[10];
    int i_cnt=0;
    int i;
    char *p;
    char *p1;
    char valid_c=0;

    if(gpsd_status!='1')return 0;

    sprintf(tmp_buff,"%d",gpsd_port);
    sock = netlib_connectsock( "127.0.0.1", tmp_buff, "tcp");
    if (sock < 0)return -1;

next_get_gps:
    i_cnt++;
    if(i_cnt>3)return -2;

    strcpy(cbuf, "r=1");
    wrote = write(sock, cbuf, strlen(cbuf));
    if ((ssize_t)strlen(cbuf) != wrote)goto next_get_gps;

    sleep(1);
    bzero(buf,sizeof(buf));

    readbytes = 0;
    readbytes = (int)read(sock, buf, sizeof(buf));
    if (readbytes <=20)goto next_get_gps;
    buf[readbytes]='\0';

    strcpy(cbuf, "r=0");
    wrote = write(sock, cbuf, strlen(cbuf));
    if ((ssize_t)strlen(cbuf) != wrote)goto next_get_gps;

    utc_time[0]=0;
    utc_date[0]=0;

    p=NULL;
    p=strstr(buf,"$GPRMC,");
    if(p==NULL)goto next_get_gps;
    //$GPRMC,204727.00,A,5108.56973,N,11404.51895,W,0.304,19.15,090513,,,D*4A

    p1=NULL;
    p1=strchr(p,'*');
    if(p1==NULL)goto next_get_gps;

    *p1=0;
    p=p+strlen("$GPRMC,");//1,

    p1=p;
    while(*p!=',')p++;//2,
    i=p-p1;
    if(i>5 && i<11)i=6;
    else goto next_get_gps;
    strncpy(utc_time,p1,6);
    utc_time[6]=0;

    p++;
    if(*p=='A')valid_c='1';
    while(*p!=',' && *p!=0)p++;//3
    if(*p==0)goto next_get_gps;
    p++;
    while(*p!=',' && *p!=0)p++;//4
    if(*p==0)goto next_get_gps;
    p++;
    while(*p!=',' && *p!=0)p++;//5
    if(*p==0)goto next_get_gps;
    p++;
    while(*p!=',' && *p!=0)p++;//6
    if(*p==0)goto next_get_gps;
    p++;
    while(*p!=',' && *p!=0)p++;//7
    if(*p==0)goto next_get_gps;
    p++;
    while(*p!=',' && *p!=0)p++;//8
    if(*p==0)goto next_get_gps;
    p++;
    while(*p!=',' && *p!=0)p++;//9
    if(*p==0)goto next_get_gps;
    p++;
    p1=p;
    while(*p!=',' && *p!=0)p++;//10
    if(*p==0)goto next_get_gps;
    if(p-p1!=6)goto next_get_gps;

    strncpy(utc_date,p1,6);
    utc_date[6]=0;


    tt=time(NULL);
    now = localtime(&tt);  

    tt_loc=time(NULL);
    if(daylight==1)tt_loc=tt_loc-now->tm_isdst*3600; //now daylight addition
    tt_loc=tt_loc+timezone;  //now tt_loc as local time to utc
    //printf("++++++++local utc:%ld,timezone:%d, daylight:%d,%d\n",tt_loc,timezone,daylight,now->tm_isdst);

    int day0,mon0,year0,hour0,min0,sec0;
    sscanf(utc_date, "%2d%2d%2d",&day0, &mon0, &year0);
    sscanf(utc_time, "%2d%2d%2d",&hour0, &min0, &sec0);
    sprintf(tmp_buff,"20%02d-%02d-%02d,%02d:%02d:%02d",year0,mon0,day0,hour0,min0,sec0);
    tt=0;
    strptime(tmp_buff,"%Y-%m-%d,%H:%M:%S", now);
    tt=mktime(now);
    strptime(tmp_buff,"%Y-%m-%d,%H:%M:%S", now);
    tt=mktime(now);
    tt=tt+1;
    //printf("*******UTC:%ld ***%s\n",tt,tmp_buff);

    //local time and new set.
    if(checkmode==2 && newloc_set=='1')  //only to count diffsec and save.
    {

        diffsec=tt_loc-tt;
        //printf("*******local to utc diff:%ld\n",diffsec);
        sprintf(tmp_buff,"%ld",diffsec);

        ctx = uci_alloc_context(); 
        if (!ctx) 
        {
            fprintf(stderr, "Out of memory\n");
            return -1;
        }
        read_config(checkmode);//gpsd, system.datetime
        
        if(newloc_set!='1')exit(0);

        set_option_by_section_name(ctx, "system","datetime","diffsec",tmp_buff);
        set_option_by_section_name(ctx, "system","datetime","newloc","0");
        uci_free_context(ctx);

        update_time_record();
        return 2;
    }


    if(checkmode==1)diffsec=0;

    tt_loc=tt+diffsec;//should be local utc
    tt_loc=tt_loc-timezone; //should be local time  
    if(daylight==1)tt_loc=tt_loc+now->tm_isdst*3600;//with utc's daylight saving.
    stime(&tt_loc); 

    tt_loc=time(NULL);
    now = localtime(&tt_loc);  //update daylight status

    tt_loc=tt+diffsec;//should be local utc
    tt_loc=tt_loc-timezone; //should be local time  
    if(daylight==1)tt_loc=tt_loc+now->tm_isdst*3600;//with update daylight saving.
    stime(&tt_loc); 
    struct timeval tv_set;
    tv_set.tv_sec  = tt_loc;
    tv_set.tv_usec = 0;
    settimeofday(&tv_set,NULL);//two ways confirm to set time

    shutdown(sock,SHUT_RDWR);
    close(sock);
    update_time_record();
    return 1;
}



//1:connect ok.
int check_carrier_satus(void)
{
    char tmp_buff[256];
    FILE* f;
    char *p;
    int result=-1;

//    connect_status="_CONNECTED"

    if (!(f = fopen("/var/run/VIP4G_status", "r"))) 
    {
        //do nothing.
    }else
    {
        while (fgets(tmp_buff, 256, f)) 
        {
            p=NULL;
            p = strstr(tmp_buff, "connect_status=");  //connect_status="_CONNECTED"
            if (p != NULL)
            {
                p=NULL;
                p = strstr(tmp_buff,"_CONNECTED");
                if(p!=NULL)
                {
                    result=1;
                }
                break;
            }//if p
        }
    }
    if(f)fclose(f);

    return result;
}

char *RadioBusyFlagFile="/var/run/VIP4G_STAT_busy";
void LockFile(int fd)
{
    struct flock lock;       
    memset (&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl (fd, F_SETLKW, &lock);
}

void UnLockFile(int fd)
{
    struct flock lock;  
    memset (&lock, 0, sizeof(lock));
    lock.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &lock);
    close (fd);     
}


int set_with_wirelessnet_time()
{
    if(check_carrier_satus()!=1)return -1;

    int rfd=0;
    char cmd_buff[30];
    char recv_buff[512];
    char tmp_buff[100];
    fd_set  atset_fds;
    struct timeval tval1={0,0};
    int i,n;
    int timeout_times;
    char *p;
    char *p1;
    int fd;

    strcpy(cmd_buff,"AT$NWNITZ?\r\n");

    fd = open ( RadioBusyFlagFile, O_WRONLY);
    if (fd<0) return -1;
    LockFile(fd); 
    if ((rfd = open(devportname, (O_RDWR | O_NDELAY))) < 0)
    {
        UnLockFile(fd);
        return -1;
    }
    read(rfd, recv_buff, sizeof(recv_buff));
    memset(recv_buff,0,sizeof(recv_buff));
    if (write(rfd, cmd_buff, strlen(cmd_buff) ) < 0)
    {
       close(rfd);
       UnLockFile(fd);
       return -1;
    }

    i=0;
    timeout_times=0;
    for(;;) 
    {
        FD_ZERO(&atset_fds);
        FD_SET(rfd, &atset_fds);
        tval1.tv_usec=0;
        tval1.tv_sec=1;
    
        n=0;
        n =select(rfd+1,&atset_fds,NULL, NULL, &tval1);
        if (n < 0)break;
        if(n==0)
        {
            timeout_times++;
            if(timeout_times>2)break;
            else continue;
        }
        if (FD_ISSET(rfd, &atset_fds))
        {
            n = read(rfd, recv_buff+i, sizeof(recv_buff)-i);
            if(n<=0)break;
            i+=n;
            recv_buff[i]=0;
            if(i>400)break;
        }
    }

    close(rfd);
    UnLockFile(fd);

    //printf("*******%d:%s\n",i,recv_buff);

    if (i <=0)return -2;
    recv_buff[i]=0;
    p=strstr(recv_buff,"OK");
    if(p==NULL)return -2;
    p=strstr(recv_buff,"Available");
    if(p!=NULL)return -2;

    p=strstr(recv_buff,"$NWNITZ:");
    if(p==NULL)return -2;
    p=p+strlen("$NWNITZ:");
    while(*p==' ')p++;
    if(*p<'0'||*p>'9')return -2;
    p1=p;
    while(*p!='\r' && *p!='\n' && *p!='\0')p++;
    if(p-p1<24)return -2;
    *p=0;

    p=strchr(p1,':');
    if(p==NULL)return -2;

    //09:43:25 05-10-2013 UTZ-6:00 DST+0
    int day0,mon0,year0,hour0,min0,sec0,zone_i,dst_i;
    char zero_sign;
    day0=0;mon0=0;year0=0;zone_i=0;n=0;dst_i=0;
    sscanf(p1, "%2d:%2d:%2d %2d-%2d-%4d UTZ%c%d:%d*",&hour0, &min0, &sec0, &mon0, &day0, &year0, &zero_sign, &zone_i, &n);
    if(day0==0||mon0==0)return -2;

    zone_i=zone_i*3600;
    zone_i=zone_i+n*60;
    if(zero_sign=='-')zone_i=zone_i*(-1);

    zone_i=zone_i*(-1);

    p=strstr(p1,"DST");
    if(p!=NULL)
    {
        n=0;
        sscanf(p,"DST%c%d*",&zero_sign,&n);
        dst_i=n*3600;
        if(zero_sign=='-')dst_i=dst_i*(-1);
    }


    tt=time(NULL);
    now = localtime(&tt);  
    tt_loc=time(NULL);
    //printf("+++++%ld,%d\n",tt_loc,now->tm_hour);
    if(daylight==1)tt_loc=tt_loc-now->tm_isdst*3600; //now daylight addition
    tt_loc=tt_loc+timezone;  //now tt_loc as local time to utc
    //printf("++++++++local utc:%ld,timezone:%d, daylight:%d,%d\n",tt_loc,timezone,daylight,now->tm_isdst);

    sprintf(tmp_buff,"%04d-%02d-%02d,%02d:%02d:%02d",year0,mon0,day0,hour0,min0,sec0);
    tt=0;
    strptime(tmp_buff,"%Y-%m-%d,%H:%M:%S", now);
    tt=mktime(now);
    strptime(tmp_buff,"%Y-%m-%d,%H:%M:%S", now);
    tt=mktime(now);
    //printf("*******UTC0:%ld ***   %d, %d\n",tt,dst_i,zone_i);
    tt=tt+1;
    tt=tt-dst_i+zone_i;  //UTC time
    //printf("*******UTC:%ld ***%s   %d\n",tt,tmp_buff,hour0);
    //printf("++++++2++timezone:%d, daylight:%d,%d\n",timezone,daylight,now->tm_isdst);

    //local time and new set.
    if(checkmode==2 && newloc_set=='1')  //only to count diffsec and save.
    {

        diffsec=tt_loc-tt;
        //printf("*******local to utc diff:%ld\n",diffsec);
        sprintf(tmp_buff,"%ld",diffsec);

        ctx = uci_alloc_context(); 
        if (!ctx) 
        {
            fprintf(stderr, "Out of memory\n");
            return -1;
        }
        read_config(checkmode);//gpsd, system.datetime

        if(newloc_set!='1')exit(0);

        set_option_by_section_name(ctx, "system","datetime","diffsec",tmp_buff);
        set_option_by_section_name(ctx, "system","datetime","newloc","0");
        uci_free_context(ctx);

        update_time_record();
        return 2;
    }

    if(checkmode==1)diffsec=0;

    tt_loc=tt+diffsec;//should be local utc
    tt_loc=tt_loc-timezone; //should be local time  
    if(daylight==1)tt_loc=tt_loc+now->tm_isdst*3600;//with utc's daylight saving.
    //printf("++++++20++%ld,timezone:%d, daylight:%d,%d\n",tt_loc,timezone,daylight,now->tm_isdst);
    stime(&tt_loc); 
    //printf("++++++2++%ld,timezone:%d, daylight:%d,%d\n",tt_loc,timezone,daylight,now->tm_isdst);

    tt_loc=time(NULL);
    now = localtime(&tt_loc);  //update daylight status
    //printf("++++++3++%ld,timezone:%d, daylight:%d,%d, hour:%d\n",tt_loc,timezone,daylight,now->tm_isdst,now->tm_hour);

    tt_loc=tt+diffsec;//should be local utc
    tt_loc=tt_loc-timezone; //should be local time  
    if(daylight==1)tt_loc=tt_loc+now->tm_isdst*3600;//with update daylight saving.
    stime(&tt_loc); 

    struct timeval tv_set;
    tv_set.tv_sec  = tt_loc;
    tv_set.tv_usec = 0;
    settimeofday(&tv_set,NULL);//two ways confirm to set time
    update_time_record();

    return 1;
}

int read_carrier_time(char *p_time_str)
{
    char tmp_buff[256];
    FILE* f;
    char *p;
    char *p1;
    int result=-1;

//    connect_status="_CONNECTED"

    if (!(f = fopen("/var/run/VIP4G_status", "r"))) 
    {
        //do nothing.
    }else
    {
        while (fgets(tmp_buff, 256, f)) 
        {
            p=NULL;
            p = strstr(tmp_buff, "connect_status=");  //connect_status="_CONNECTED"
            if (p != NULL)
            {
                p=NULL;
                p = strstr(tmp_buff,"_CONNECTED");
                if(p!=NULL)
                {
                    result=1;
                }
            }


            p=NULL;
            p = strstr(tmp_buff, "nettime=");  //nettime=" 10:01:04 07-05-2013 UTZ-6:00 DST+0"
            if (p != NULL)
            {
                p=tmp_buff + strlen("nettime=");
                while(*p==' ' || *p=='"')p++;

                p1 = strchr(p,'"');
                if(p1!=NULL)*p1=0;
                strcpy(p_time_str,p);
            }
        }
    }
    if(f)fclose(f);

    return result;
}

int set_with_wirelessnet_time_status()
{
    char tmp_buff[100];
    char last_time_str[100];
    int i,n;
    char *p;

    tmp_buff[0]=0;
    if(read_carrier_time(tmp_buff)!=1)return -1;

    if(strlen(tmp_buff)<15)return -1;
    strcpy(last_time_str,tmp_buff);
    i=20;
    while(i>0)
    {
        i--;
        if(i<=0)return -1;

        sleep(1);

        tmp_buff[0]=0;
        if(read_carrier_time(tmp_buff)!=1)return -1;
        if(strlen(tmp_buff)<15)return -1;

        if(strcmp(tmp_buff,last_time_str)!=0)break;
    }
    
    //09:43:25 05-10-2013 UTZ-6:00 DST+0
    int day0,mon0,year0,hour0,min0,sec0,zone_i,dst_i;
    char zero_sign;
    day0=0;mon0=0;year0=0;zone_i=0;n=0;dst_i=0;
    sscanf(tmp_buff, "%2d:%2d:%2d %2d-%2d-%4d UTZ%c%d:%d*",&hour0, &min0, &sec0, &mon0, &day0, &year0, &zero_sign, &zone_i, &n);
    if(day0==0||mon0==0)return -2;

    zone_i=zone_i*3600;
    zone_i=zone_i+n*60;
    if(zero_sign=='-')zone_i=zone_i*(-1);

    zone_i=zone_i*(-1);

    p=NULL;
    p=strstr(tmp_buff,"DST");
    if(p!=NULL)
    {
        n=0;
        sscanf(p,"DST%c%d*",&zero_sign,&n);
        dst_i=n*3600;
        if(zero_sign=='-')dst_i=dst_i*(-1);
    }


    tt=time(NULL);
    now = localtime(&tt);  
    tt_loc=time(NULL);
    //printf("+++++%ld,%d\n",tt_loc,now->tm_hour);
    if(daylight==1)tt_loc=tt_loc-now->tm_isdst*3600; //now daylight addition
    tt_loc=tt_loc+timezone;  //now tt_loc as local time to utc
    //printf("++++++++local utc:%ld,timezone:%d, daylight:%d,%d\n",tt_loc,timezone,daylight,now->tm_isdst);

    sprintf(tmp_buff,"%04d-%02d-%02d,%02d:%02d:%02d",year0,mon0,day0,hour0,min0,sec0);
    tt=0;
    strptime(tmp_buff,"%Y-%m-%d,%H:%M:%S", now);
    tt=mktime(now);
    strptime(tmp_buff,"%Y-%m-%d,%H:%M:%S", now);
    tt=mktime(now);
    //printf("*******UTC0:%ld ***   %d, %d\n",tt,dst_i,zone_i);
    tt=tt+1;
    tt=tt-dst_i+zone_i;  //UTC time
    //printf("*******UTC:%ld ***%s   %d\n",tt,tmp_buff,hour0);
    //printf("++++++2++timezone:%d, daylight:%d,%d\n",timezone,daylight,now->tm_isdst);

    //local time and new set.
    if(checkmode==2 && newloc_set=='1')  //only to count diffsec and save.
    {

        diffsec=tt_loc-tt;
        //printf("*******local to utc diff:%ld\n",diffsec);
        sprintf(tmp_buff,"%ld",diffsec);

        ctx = uci_alloc_context(); 
        if (!ctx) 
        {
            fprintf(stderr, "Out of memory\n");
            return -1;
        }
        read_config(checkmode);//gpsd, system.datetime

        if(newloc_set!='1')exit(0);

        set_option_by_section_name(ctx, "system","datetime","diffsec",tmp_buff);
        set_option_by_section_name(ctx, "system","datetime","newloc","0");
        uci_free_context(ctx);

        update_time_record();
        return 2;
    }

    if(checkmode==1)diffsec=0;

    tt_loc=tt+diffsec;//should be local utc
    tt_loc=tt_loc-timezone; //should be local time  
    if(daylight==1)tt_loc=tt_loc+now->tm_isdst*3600;//with utc's daylight saving.
    //printf("++++++20++%ld,timezone:%d, daylight:%d,%d\n",tt_loc,timezone,daylight,now->tm_isdst);
    stime(&tt_loc); 
    //printf("++++++2++%ld,timezone:%d, daylight:%d,%d\n",tt_loc,timezone,daylight,now->tm_isdst);

    tt_loc=time(NULL);
    now = localtime(&tt_loc);  //update daylight status
    //printf("++++++3++%ld,timezone:%d, daylight:%d,%d, hour:%d\n",tt_loc,timezone,daylight,now->tm_isdst,now->tm_hour);

    tt_loc=tt+diffsec;//should be local utc
    tt_loc=tt_loc-timezone; //should be local time  
    if(daylight==1)tt_loc=tt_loc+now->tm_isdst*3600;//with update daylight saving.
    stime(&tt_loc); 

    struct timeval tv_set;
    tv_set.tv_sec  = tt_loc;
    tv_set.tv_usec = 0;
    settimeofday(&tv_set,NULL);//two ways confirm to set time
    update_time_record();

    return 1;

}

char process_sign[50];
int check_process_exit()
{
    char tmp_buff[50];
    FILE* sfp=NULL;
    sfp =fopen("/var/run/check_time_sync","r");
    if(sfp==NULL)return 0;
    tmp_buff[0]=0;
    fgets(tmp_buff,50,sfp); 
    fclose(sfp);

    if(strcmp(tmp_buff,process_sign)!=0)return 1;
    return 0;
}

void set_process_sign()
{
    time_t localtime_t = time (NULL);
    sprintf(process_sign,"%ld",localtime_t);
    FILE* sfp=NULL;
    sfp =fopen("/var/run/check_time_sync","w");
    if(sfp!=NULL) 
    {
        fprintf(sfp,"%s",process_sign);
        fflush(sfp);
        fclose(sfp);
    }
    return;
}

int main(int argc, char *argv[])
{
    int check_type=0;

    if(argc!=2)
    {
        printf("no arguments, end\n");
        return 0;
    }

    if(strcmp(argv[1],"sync")==0)check_type=1;
    else if(strcmp(argv[1],"loc")==0)check_type=2;

    if(check_type==0)return 0;

    set_process_sign();

    (void) setsid();          
    signal(SIGPIPE, SIG_IGN);


    tt=time(NULL);
    now = localtime(&tt);  
    if (now==NULL)
    {
        syslog(LOGOPTS,"%s:error to get system time for time sychronizing\n",__func__);
        return -1;
    }


    ctx = uci_alloc_context(); 
    if (!ctx) 
    {
        fprintf(stderr, "Out of memory\n");
        return -1;
    }

    read_config(check_type);//gpsd, system.datetime
    uci_free_context(ctx);

    int i_cnt=100;
    while(i_cnt>0)
    {
        i_cnt--;

        if(check_process_exit()>0)return 0;

        if(set_with_gps_time()>0)return 1;
        if(set_with_wirelessnet_time_status()>0)return 1;

        sleep(5);
        ctx = uci_alloc_context(); 
        if (!ctx) 
        {
            fprintf(stderr, "Out of memory\n");
            return -1;
        }
        read_config(check_type);//gpsd, system.datetime
        uci_free_context(ctx);

    }

    return 0;
}
