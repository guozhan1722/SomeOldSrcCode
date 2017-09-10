/*
 * include our .h file 
 */
#include "mpci-4g.h"  
#include "mpci-com.h"
#include <syslog.h>
#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)  
#include <ctype.h>

/* state define*/
#define CONNECT     0   
#define NO_SIMCARD  1
#define NEED_RESET  2
#define RECONNECT   3
#define DISCONNECT  4
#define NEED_HARD_RESET 5
#define QUIT_SERVER 6

#define REMOTE_DEBUG 7

#define CONNECTED 7
#define DISCONNECTED 8
static int err_select_count=0;
static int cur_state=8;
static int req_state;

static char *PortBusyFlagFile="/var/run/port_busy";
static char *portFILEBusyFlagFile="/var/run/VIP4G_STAT_busy";
static char *StatFILEBusyFlagFile="/var/run/VSTAT_busy";
static char *StatFILE="/lib/VIP4G_state";
static char *Status_conf="/lib/status_conf";
static char *Status_conf362="/lib/status_conf362";
static char *Hw_Desc="/usr/lib/hardware_desc";

#ifdef VIP4G_SERVER
static char	*devname="/dev/ttyUSB0";
static char	*alt_devname="/dev/ttyUSB1";
#endif

#ifdef IPn4G_SERVER
static char	*devname="/dev/ttyUSB1";
static char	*alt_devname="/dev/ttyUSB2";
#endif

static char *confile="/etc/config/lte";
static unsigned char	ibuf[512];
static char *version = "0.0.1";
static int status_default;
static int gotdevice;
static int connect_data;
static int modem_type=0;

static int start_server;
static int disconnect_data;
static int mreset;
static int have_simcard_status;
static int simcard_locked=0;

static int fd;
static int fd_stat;
static int fd_portbusy;
static char techmode_data[512];
static int tech_step=0;
/*
 *	Signal handling.
 */
struct sigaction	sact;

char *VIP4G_status_file ="/var/run/VIP4G_status"; 

#define MAX_NUM_STATUS 64
#define MAX_RT_BUFF_LEN 1024

struct sstatus{
    char atcmd[64];
    char keyword[64];
    char initval[64];
    int nosim_status;
    char ex_seperator[64];
};

static struct sstatus status_set[MAX_NUM_STATUS];

static char connect_conf[][32] ={
    "tech",
    "call_para",
    "pdns",
    "sdns",
    "pnbns",
    "snbns",
    "apn",
    "ip",
    "authentication",
    "username",
    "passwd",
    "",
};

static char need_save_autoapn_sign;
static char autoapnstr[32];
static char confapnstr[32];
static char apn_trylist[20][32];
static int apnlist_i=0;
static int apn_try_times=0;
static int apnlist_all=0;
static char networklist[10][8];
static char homeorroam_search='h';// h r 
static char lastimsimccmnc[8];
static int search_apn(char *apnstr);
static int add_apnhistory(char *apnstr);

static bool send_cmd(int timeout, char *cmd, char *rtbuf);
#ifdef IPn4G_SERVER
static bool set_dcd_asser(void);
#endif
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

static int fetch_Local_IP_ADDR(char *ifname, char *addr)
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

int getRightPart(const char* inbuf, char* outbuf,int len)
{

    char *pb=NULL;
    char *p;
    int i=0;

    pb=strstr(inbuf,":");
    if (pb==NULL) {
        strcpy(outbuf,inbuf);
        return false;
    }

    for (p=pb+1;(*p!='\n')&&(*p!='\r')&&(i<len);p++) {
        if (*p=='"') {
            *p=' ';
        }
        if (*p=='=') {
            *p=':';
        }
        outbuf[i]=*p;
        i++;
    }
    return true;
}

int getNumberPart(const char* inbuf, char* outbuf,int len)
{

    char *pb=NULL;
    char *p;
    int i=0,l=0;

    pb=inbuf;

    for (p=pb;l<len;p++,l++) {
        if(isdigit(*p)){
            outbuf[i]=*p;
            i++;
        }
    }
    return true;
}

int getHexNumberPart(const char* inbuf, char* outbuf,int len)
{

    char *pb=NULL;
    char *p;
    int i=0,l=0;

    pb=inbuf;

    for (p=pb;l<len;p++,l++) {
        if(isxdigit(*p)){
            outbuf[i]=*p;
            i++;
        }
    }
    return true;
}

static void write_status_file(void)
{
    int f_int;
	FILE* f_fd;
    int i,rc;

    char buf[256];

    f_fd = fopen(VIP4G_status_file, "w+");

    if (f_fd == NULL ) {
            syslog(LOGOPTS,"Error : open %s Error ..... \n",VIP4G_status_file);            
    }
	//while (!(f_fd = fopen(VIP4G_status_file, "w+")))
	//{
		//syslog(LOGOPTS,"%s %s\n",COM12_statistic_file,strerror(errno));            
	//}
	for(i=0;i<MAX_NUM_STATUS;i++) {
        rc=strlen(status_set[i].keyword);
        if(rc<2) break;
        memset(buf,0,sizeof(buf));
        sprintf(buf,"%s=\"%s\"",status_set[i].keyword,status_set[i].initval);
        fputs(buf,f_fd);
		fputc( '\n',f_fd);
    }
	fclose(f_fd);
}

static bool init_status_set(void)
{
    FILE *st_fd = NULL;
    char buf[256];
    int i;
    int ssim;
    char *start, *end, *pos;

    if (modem_type == 0 ) {
        st_fd = fopen(Status_conf, "rb");
    }else if (modem_type == 1) {
        st_fd = fopen(Status_conf362, "rb");
    }

    if (st_fd == NULL)
    {
        syslog(LOGOPTS,"Can not open file %s\n",Status_conf);
        return false;
    } 

    i=0;
    memset(buf,0,sizeof(buf));
    while (fgets(buf,256,st_fd)){

        if ((strlen(buf)>2)&&(buf[0]!='#')) {
            end=start = strchr(buf, ';');
            if (NULL==start) {
                syslog(LOGOPTS,"file %s format not corrct\n",Status_conf);
                break;
            }
            end++;
            start++;
            end = strchr(start, ';');
            if(NULL==end) {
                syslog(LOGOPTS,"file %s format not corrct\n",Status_conf);
                break;
            }
            
            strncpy(status_set[i].atcmd,start,end-start);

            end++;
            start=end;
            end= strchr(start, ';');
            if (NULL==end)
            {
                syslog(LOGOPTS,"file %s format not corrct\n",Status_conf);
                break;
            }
            strncpy(status_set[i].keyword,start,end-start);

            end++;
            start=end;
            end= strchr(start, ';');
            if(NULL==end) {
                syslog(LOGOPTS,"file %s format not corrct\n",Status_conf);
                break;
            }
            strncpy(status_set[i].initval,start,end-start);

            end++;
            start=end;
            end= strchr(start, ';');
            if (NULL==end) {
                syslog(LOGOPTS,"file %s format not corrct\n",Status_conf);
                break;
            }
            ssim=atoi(start);
            status_set[i].nosim_status=ssim;

            end++;
            start=end;
            end=strchr(start, ';');
            if (NULL==end) {
                strcpy(status_set[i].ex_seperator," ");
            }else{
                strncpy(status_set[i].ex_seperator,start,end-start);
            }
            i++;
        }
        memset(buf,0,sizeof(buf)); 
    }
    fclose(st_fd);
    strcpy(status_set[i].keyword," ");
    strcpy(status_set[i].atcmd," ");
     
    return true;
}

static void set_single_status_value(char *keyword, char *value)
{
    int i;
    int rc;

    for(i=0;i<MAX_NUM_STATUS;i++)
    {
        rc=strlen(status_set[i].keyword);
        if (rc<2)
        {
            break;
        }
        if (0 == strncmp(status_set[i].keyword,keyword,rc)) {
            sprintf(status_set[i].initval,"%s",value);
            break;
        }
    }
}

static char * get_single_status_value(char *keyword)
{
    int i;
    int rc;
    char val[256];

    for(i=0;i<MAX_NUM_STATUS;i++)
    {
        rc=strlen(status_set[i].keyword);
        if (rc<2)
        {
            break;
        }
        if (0 == strncmp(status_set[i].keyword,keyword,rc)) {
            memset(val,0,sizeof(val));
            strcpy(val,status_set[i].initval);
            break;
        }
    }
    return val;
}

static int is_carrier_disable() 
{ 
    FILE * stat_file_fd;
    char buf[256];
    int retval=QUIT_SERVER;
    char *pos;
    char conf_data[512];

    fd_stat = open (StatFILEBusyFlagFile, O_WRONLY);
    if (fd_stat<0) {
        syslog(LOGOPTS,"%s:locked fd_stat<0\n",__func__);
        return false;
       }
    LockFile(fd_stat);    

    if (ctx)
       {
           uci_free_context(ctx);
           ctx=NULL;
       }
       ctx = uci_alloc_context();
       if (!ctx)
       {
           fprintf(stderr, "Out of memory\n");
           UnLockFile(fd_stat);
           return(retval);
       }

    if (get_option_by_section_name(ctx, confile, "connect", "disabled",  conf_data)==false) {
#ifdef DEBUG_4G
        syslog(LOGOPTS,"\n\nError can not get req state!\n");
#endif
        UnLockFile(fd_stat);
        return(retval);
    }

    retval=atoi(conf_data);
#ifdef DEBUG_4G
    //syslog(LOGOPTS,"DBUG : %s get carrier enabled state from uci = %d \n",__FUNCTION__,retval);
#endif
    UnLockFile(fd_stat);
    return retval;
}

static int is_ip_passthrough() 
{ 
    FILE * stat_file_fd;
    char buf[256];
    int retval=0;
    char *pos;
    char conf_data[512];

    fd_stat = open (StatFILEBusyFlagFile, O_WRONLY);
    if (fd_stat<0) {
        syslog(LOGOPTS,"%s:locked fd_stat<0\n",__func__);
        return false;
       }
    LockFile(fd_stat);    

    if (ctx)
       {
           uci_free_context(ctx);
           ctx=NULL;
       }
       ctx = uci_alloc_context();
       if (!ctx)
       {
           fprintf(stderr, "Out of memory\n");
           UnLockFile(fd_stat);
           return(retval);
       }

    if (get_option_by_section_name(ctx, confile, "connect", "ippassthrough",  conf_data)==false) {
#ifdef DEBUG_4G
        syslog(LOGOPTS,"\n\nError can not get ip passthough status!\n");
#endif
        UnLockFile(fd_stat);
        return(retval);
    }

    if (strcmp(conf_data,"Ethernet")==0){
        retval=1;
    }
#ifdef DEBUG_4G
        //syslog(LOGOPTS,"\n\nget ip passthough status = %d !\n",retval);
#endif

    UnLockFile(fd_stat);
    return retval;
}

static bool set_req_state(int stat)
{
    char buf[20];
    FILE * stat_file_fd;
    char conf_data[512];

    fd_stat = open (StatFILEBusyFlagFile, O_WRONLY);
    if (fd_stat<0) {
        syslog(LOGOPTS,"%s:locked fd_stat<0\n",__func__);
        return false;
   }
    LockFile(fd_stat);    


#if 0
    if ((stat_file_fd = fopen(StatFILE,"w+")) < 0) {
        syslog(LOGOPTS, "ERROR: open(%s) failed\n", "stat_file_fd");
        UnLockFile(fd_stat);
        return false;
    }

    sprintf (buf, "req_state=%d",stat);
    fputs(buf,stat_file_fd);
    fclose(stat_file_fd);
    UnLockFile(fd_stat);
    return true;
#else
    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx)
    {
        fprintf(stderr, "Out of memory\n");
        UnLockFile(fd_stat);
        return false;
    }

    sprintf(buf, "%d",stat);
    set_option_by_section_name(ctx, confile, "connect", "req_state", buf);

#ifdef DEBUG_4G
   // syslog(LOGOPTS,"DBUG : %s set req state to uci = %s \n",__FUNCTION__,buf);
#endif
    UnLockFile(fd_stat);
    return true;
#endif

}

static int get_req_state() { 
    FILE * stat_file_fd;
    char buf[256];
    int retval=QUIT_SERVER;
    char *pos;
    char conf_data[512];

    fd_stat = open (StatFILEBusyFlagFile, O_WRONLY);
    if (fd_stat<0) {
        syslog(LOGOPTS,"%s:locked fd_stat<0\n",__func__);
        return false;
       }
    LockFile(fd_stat);    

#if 0
    if ((stat_file_fd = fopen(StatFILE,"rb")) < 0) {
        syslog(LOGOPTS, "ERROR: open(%s) failed\n", "stat_file_fd");
        UnLockFile(fd_stat);
        return retval;
    }

    memset(buf,0,sizeof(buf));
    while (fgets(buf,256,stat_file_fd)) {
        if ((strlen(buf)>2)&&(buf[0]!='#')) {
            pos = strchr(buf, '=');   
            if(pos!=NULL) {
                retval=atoi(pos+1);
            }
        }
    }

    fclose(stat_file_fd);

#ifdef DEBUG_4G
    syslog(LOGOPTS,"\n\nCurrent stat from stat file %s is %s, return val =%d!\n",StatFILE, buf,retval);
#endif

    UnLockFile(fd_stat);
    req_state=retval;
    return retval;
#else
    if (ctx)
       {
           uci_free_context(ctx);
           ctx=NULL;
       }
       ctx = uci_alloc_context();
       if (!ctx)
       {
           fprintf(stderr, "Out of memory\n");
           UnLockFile(fd_stat);
           return(retval);
       }

    if (get_option_by_section_name(ctx, confile, "connect", "req_state",  conf_data)==false) {
#ifdef DEBUG_4G
        syslog(LOGOPTS,"\n\nError can not get req state!\n");
#endif
        UnLockFile(fd_stat);
        return(retval);
    }

    retval=atoi(conf_data);
#ifdef DEBUG_4G
    //syslog(LOGOPTS,"DBUG : %s get req state from uci = %d \n",__FUNCTION__,retval);
#endif
    UnLockFile(fd_stat);
    return retval;

#endif
}

static int get_cur_state()
{ 
    return cur_state;
}

static int set_cur_state(int state) { 
    cur_state=state;
    return true;
}

/* Send atcommand function */
static bool send_cmd(int timeout, char *cmd, char *rtbuf) {
    char atcmd_buff[64];
    char ret_val[20];
    char	*bp;
    fd_set  atset_fds;
    struct timeval tval1={0,0};
    int n,cmdlen,i;

    int read_timeout_count=0, select_rc=0;

    memset(atcmd_buff, 0, sizeof(atcmd_buff));
    strcpy(atcmd_buff,cmd);
    cmdlen=strlen(cmd);

    atcmd_buff[cmdlen]='\r';

    if (write(rfd, atcmd_buff, strlen(atcmd_buff) /*+2 */) < 0) {
        fprintf(stderr, "ERROR: write (rfd=%d) string %s  failed, "
                "errno=%d\n", rfd,atcmd_buff, errno);
        return false;
    }

    strncpy(ret_val, "OK",2);
    ret_val[2]='\0';

#ifdef DEBUG_4G
    atcmd_buff[cmdlen]='\0';
    //syslog(LOGOPTS,"DBUG : %s send command %s ",__FUNCTION__,atcmd_buff);
#endif

    memset(ibuf,0,sizeof(ibuf));

    for(;;) {
        FD_ZERO(&atset_fds);
        FD_SET(rfd, &atset_fds);
        tval1.tv_usec=0;
        tval1.tv_sec=timeout;

        if ((select_rc =select(rfd+1,&atset_fds,NULL, NULL, &tval1)) <= 0) {
        //if (select(rfd+1,&atset_fds,NULL, NULL, NULL) <= 0) {
            fprintf(stderr, "ERROR: select() failed cmd = %s it is invalidate errno=%d\n",
                    atcmd_buff,errno);
            err_select_count++;
            if (select_rc == 0) {
                read_timeout_count ++;
                if (read_timeout_count > 2) {
                    read_timeout_count= 0;
                    return false;
                }else {
                    continue;
                }
            } else {
                return false;
            }
        }
		err_select_count=0;
        if (FD_ISSET(rfd, &atset_fds)) {
            bp = ibuf;
            if ((n = read(rfd, ibuf, sizeof(ibuf))) < 0) {
                fprintf(stderr, "ERROR: read(fd=%d) failed, "
                        "errno=%d\n", rfd, errno);
                break;
            }
            if (n ==0) {
                fprintf(stderr, "read(rfd=%d) zero.\n", rfd);
                break;
            }

            if (strstr(ibuf,"OK")!=(char*)NULL) {
                strcpy(rtbuf,ibuf);
                //syslog(LOGOPTS,"and , returned result is OK \n");
                return true;
            } else if (strstr(ibuf,"ERROR")!=(char*)NULL) {
                //syslog(LOGOPTS,"and  returned result is ERROR  the result is %s\n",ibuf);
                if (strcmp(atcmd_buff,"at$nwqmistatus") == 0) {
                    //syslog(LOGOPTS,"this error is for status \n");
                    strcpy(rtbuf,"QMI State: QMI_WDS_PKT_DATA_DISCONNECTED \n Call Duration: 0 seconds");
                    //syslog(LOGOPTS,"return buffer is %s \n",rtbuf);
                    break;
                }else {
                    strcpy(rtbuf,"Unknown:Unknown");
                    break;
                }
            } 
        }
    }

    return true;
}

static bool is_weak_rssi() 
{
    char rt_b[MAX_RT_BUFF_LEN];
    char *rt_buff ;
    char wr_buff[512];
    char *rssi_status;
    int rssi_value;
    int ret_val=true;

    rt_buff=rt_b;

    char *cmd_rssi="at$nwrssi=1";

    memset(rt_buff,0,sizeof(rt_buff));
    memset(wr_buff,0,sizeof(wr_buff));
    if(send_cmd(3,cmd_rssi,rt_buff)==true) {
#ifdef DEBUG_4G
//        syslog(LOGOPTS,"DBUG : %s get the return rssi is %s \n",__FUNCTION__,rt_buff);
#endif
        if (true==getRightPart(rt_buff,wr_buff,512)) {
            rssi_status=strstr(wr_buff,":");
            rssi_value=atoi(rssi_status+1);
            //syslog(LOGOPTS,"DBUG : %s get the  rssi is %d \n",__FUNCTION__,rssi_value);
            if (rssi_value > 0 && rssi_value <= 109 ) {
                ret_val=false;
            }
        }
    }

    if (ret_val==true) {
        set_cur_state(DISCONNECTED);
        set_single_status_value("connect_status","_DISCONNECTED");
        set_single_status_value("rssi"," NO SERVICE RSSI : 125");
#ifdef VIP4G_SERVER
        SubProgram_Start("ifdown","wan2");
#endif
#ifdef IPn4G_SERVER
        SubProgram_Start("ifdown","wan");
#endif
        write_status_file();
    }

    return ret_val;
}

static bool connect_status_check() {
    char rt_b[MAX_RT_BUFF_LEN];
    char *rt_buff ;
    char wr_buff[512];
    char * temp_status=NULL, *status362connected=NULL, *status362disconnected=NULL;
    int i,rc;
    char *st="_CONNECTED";

    rt_buff=rt_b;

    char *cmd_chk_status="at$nwqmistatus";
    /*loop 3 times for getting status from modules*/
    for(i=0;i<10;i++) {

        memset(rt_buff,0,sizeof(rt_buff));

        if (send_cmd(10,cmd_chk_status,rt_buff)==false) {
            sleep(1);
            continue;
        }

        temp_status=strstr(rt_buff,"_CONNECTED");
        status362connected=strstr(rt_buff,"CONNECTED");
        status362disconnected=strstr(rt_buff,"DISCONNECTED");

        /*get connection*/
        if (temp_status!=NULL) {
            set_single_status_value("connect_status",st);
            write_status_file();
            set_cur_state(CONNECTED);
#ifdef DEBUG_4G
           // syslog(LOGOPTS,"DBUG : %s connect status check success \n",__FUNCTION__);
#endif
            return true;
        } else if((status362connected!=NULL) && (status362disconnected==NULL)){

            set_single_status_value("connect_status",st);
            write_status_file();
            set_cur_state(CONNECTED);
            return true;
        }
        sleep(1);
    }

#ifdef DEBUG_4G
    syslog(LOGOPTS,"DBUG : %s connect status check false (10 times)\n",__FUNCTION__);
#endif
    set_cur_state(DISCONNECTED);
    set_single_status_value("connect_status","_DISCONNECTED");

    return false;
}



static bool set_status_tmpfile(char *ibufc, char *cmd,char *keyword)
{
    char atcmd_cmd[32];
    char atcmd_buf[512];
    int rc,cmd_len,buf_len,i;
    char *pb=NULL,*end=NULL;
    char wr_buff[512];
    char item_buff[512];
    char *temp_imei=NULL,*temp_status=NULL,*status362connected=NULL, *status362disconnected=NULL;

    buf_len=strlen(ibufc);
    if (buf_len == 0) return false;
    strncpy(atcmd_buf,ibufc,buf_len);
    cmd_len=strlen(cmd);
    strncpy(atcmd_cmd,cmd,cmd_len);

    atcmd_cmd[cmd_len]='\0';
    atcmd_buf[buf_len]='\0';

    memset(wr_buff,0,sizeof(wr_buff));

    /*special handle for status command */
    if ( strcmp(atcmd_cmd,"at$nwqmistatus")==0) {
        temp_status=strstr(atcmd_buf,"QMI State:");
        if (temp_status!=NULL) {
            getRightPart(temp_status,wr_buff,512);

            status362disconnected=strstr(wr_buff,"DISCONNECTED");

            if(status362disconnected!=NULL){
                set_single_status_value("connect_status","_DISCONNECTED");
            }else
                set_single_status_value("connect_status","_CONNECTED");
        } else {
            set_single_status_value("connect_status","_DISCONNECTED");
        }

        memset(wr_buff,0,sizeof(wr_buff));
        temp_status=strstr(atcmd_buf,"Call Duration:");
        if (temp_status!=NULL) {
            getRightPart(temp_status,wr_buff,512);
            set_single_status_value("connect_duration",wr_buff);
        }
    } else if ( strcmp(atcmd_cmd,"at+cimi")==0) {
        memset(wr_buff,0,sizeof(wr_buff));
        getNumberPart(atcmd_buf,wr_buff,buf_len);
        set_single_status_value("imsi",wr_buff);
    } else if ( strcmp(atcmd_cmd,"at$nwnn")==0) {
        memset(wr_buff,0,sizeof(wr_buff));
        pb=strchr(atcmd_buf,'\n');
        pb++;
        pb=strchr(pb,'\n');
        pb++;
        end=strchr(pb,'\r');
        strncpy(wr_buff,pb,end-pb);
        set_single_status_value("network",wr_buff);
    }else {
        for(i=0;i<MAX_NUM_STATUS;i++) {
            rc=strlen(status_set[i].atcmd);
            if(rc<2) break;
    
            if (0==strncmp(status_set[i].keyword ,keyword,rc)) {
               
                if (strlen(status_set[i].ex_seperator)>2) {
                    temp_status=strstr(atcmd_buf,status_set[i].ex_seperator);
                    if (temp_status!=NULL)   
                        strcpy(atcmd_buf,temp_status);
                }
        
                if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                    //sprintf(status_set[i].initval,"%s",wr_buff);
                    strcpy(status_set[i].initval,wr_buff);
                   break;
                }
            }
        }
    }
    return true;
}


static bool send_status_cmd() {
    int i,rc;
    char atcmd_buff[32];
    char rt_b[MAX_RT_BUFF_LEN];
    char *rt_buff ;
    
    rt_buff=rt_b;

    if(status_default <= 0)
        return false;

    usleep(500*1000);

    for(i=0;i<MAX_NUM_STATUS;i++) {
        rc=strlen(status_set[i].atcmd);
        if(rc<2) {
            break;
        }
        if (have_simcard_status ==0) {
            if(status_set[i].nosim_status == 0) {
                set_single_status_value(status_set[i].keyword,"No SIM Card");
                continue ;
            }
        }else {
            if (simcard_locked == 1) {
            if(status_set[i].nosim_status == 0) {
                set_single_status_value(status_set[i].keyword,"SIM Card Locked");
                continue ;
            }
        }
        }
        strncpy(atcmd_buff,status_set[i].atcmd,rc);
        atcmd_buff[rc]='\0';

        usleep(200*1000);
        
        memset(rt_b,0,sizeof(rt_b));
        if (send_cmd(3,atcmd_buff,rt_buff)==false) {
#ifdef DEBUG_4G
            syslog(LOGOPTS,"DBUG : %s ERROR: send cmd(%s) failed\n",__FUNCTION__,atcmd_buff);
#endif
            return false;
        }

        if (set_status_tmpfile(rt_buff,atcmd_buff,status_set[i].keyword)==false) {
#ifdef DEBUG_4G
            syslog(LOGOPTS,"DBUG : %s ERROR: set_status_tempfile cmd(%s) failed\n",__FUNCTION__,atcmd_buff);
#endif
            return false;
        }
    }

    write_status_file();
    return true;
}
static bool check_autoapn(char *apnstr)
{
    char *p;
    int i=0;
    i=strlen(apnstr);
    if(i<2)return true;
    if(i>20)return false;
    p=apnstr;
    while(*p==' ')p++;
    if(*p!='a' && *p!='A')return false;
    p++;
    if(*p!='u' && *p!='U')return false;
    p++;
    if(*p!='t' && *p!='T')return false;
    p++;
    if(*p!='o' && *p!='O')return false;
    p++;
    while(*p==' ')p++;
    if(*p!=0)return false;
    return true;
}

static bool get_connect_conf_uci() {
    char conf_data[512];
    char argstr[256] = "at$nwqmiconnect=";
    char cgdcnt[256] = "At+cgdcont=1,\"IP\",\"";
    int i,rc;
    char rt_b[MAX_RT_BUFF_LEN];
    char *rt_buff ;

    rt_buff=rt_b;

    //this part added for E362 connecting to internet
    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx) {
        fprintf(stderr, "Out of memory\n");
        return(false);
    }

    memset(conf_data,0,sizeof(conf_data));
    if (get_option_by_section_name(ctx, confile, "connect", "apn", conf_data)==false) {
        fprintf(stderr, "can not get conf from uci\n");
        return(false);
    }


    unlink("/var/run/autoapn");
    need_save_autoapn_sign=1;
    autoapnstr[0]=0;
    strcpy(confapnstr,conf_data);
    if(check_autoapn(conf_data)==true)
    {
        i=search_apn(autoapnstr);
        if(i<0)return false;
        if(i==0)search_apn(autoapnstr);
        if(autoapnstr[0]!=0)
        {
            strcpy(conf_data,autoapnstr);
        }else
        {
            fprintf(stderr, "can not get apn from uci and apntable\n");
            return false;
        }
    }

    //printf("*********get apn ok:%s\n",conf_data);

    strcat(cgdcnt,conf_data);
    strcat(cgdcnt,"\"");

    //LL, novatel said that do not touch cgdcont on 1.41 E362
    if (modem_type == 0) {
        if (send_cmd(3, cgdcnt,rt_buff)==false)
        return false;
    }
#if 0
    if (ctx)
    {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx)
    {
        fprintf(stderr, "Out of memory\n");
        return(false);
    }

    if(get_option_by_section_name(ctx, confile, "connect", "disabled",  conf_data)==false){
        fprintf(stderr, "can not get disabled status from uci\n");
        return(false);
    }

    if(strcmp(conf_data,"1")==0) {
        fprintf(stderr, "module status is disabled mode\n");
        return false;
    }

#endif
    for(i=0;i<64;i++) {
        rc=strlen(connect_conf[i]);
        if(rc<2) {
            break;
        }
        if (ctx) {
            uci_free_context(ctx);
            ctx=NULL;
        }
        ctx = uci_alloc_context();
        if (!ctx) {
            fprintf(stderr, "Out of memory\n");
            return(false);
        }

        memset(conf_data,0,sizeof(conf_data));
        if (get_option_by_section_name(ctx, confile, "connect", connect_conf[i], conf_data)==false) {
            fprintf(stderr, "can not get conf from uci\n");
            return(false);
        }

        if(strcmp(connect_conf[i],"apn")==0)
        {
            strcpy(confapnstr,conf_data);
            if(check_autoapn(conf_data)==true)
            {
                if(autoapnstr[0]!=0)
                {
                    strcpy(conf_data,autoapnstr);
                }else
                {
                    fprintf(stderr, "can not get apn from uci and apntable\n");
                    return false;
                }
            }
        }

        strcat(argstr,conf_data);
        strcat(argstr,",");
    }
    rc=strlen(argstr);
    argstr[rc-1]='\0';

#ifdef DEBUG_4G
    syslog(LOGOPTS,"DBUG : %s get uci config argstr = %s \n",__FUNCTION__,argstr);
#endif

    if (send_cmd(10, argstr,rt_buff)==false)
        return false;
    if(strstr(rt_buff,"OK")!=NULL) {
        sleep(3);
        if(connect_status_check()==false) {
            return false;
        }

        if(check_autoapn(confapnstr)==true)
        {
            add_apnhistory(autoapnstr);
        }else add_apnhistory(confapnstr);

        return true;
    }
    return false;
}

static bool set_connect_tech_mode()
{
    char argstr[256] = "AT$NWRAT=";
    int i,rc;
    char rt_b[MAX_RT_BUFF_LEN];
    char *rt_buff ;
    static int flag = 0;

    rt_buff=rt_b;

    if(ctx) {
        uci_free_context(ctx);
        ctx=NULL;
        }
    ctx = uci_alloc_context();
    if (!ctx)
    {
        fprintf(stderr, "Out of memory\n");
        return(false);
    }

    if (flag == 0) {
        flag = 1;
        if(get_option_by_section_name(ctx, confile, "connect", "tech_mode",  techmode_data)==false){
            fprintf(stderr, "can not get disabled status from uci\n");
            return(false);
        }
        if (strcmp(techmode_data,"4g")==0){
            strcat(argstr,"3,2");
        } else if (strcmp(techmode_data,"3g")==0){
            strcat(argstr,"2,2");
        } else if (strcmp(techmode_data,"2g")==0) {
            strcat(argstr,"1,2");
        } else if (strcmp(techmode_data,"1x")==0) {
            strcat(argstr,"4,2");
        } else if (strcmp(techmode_data,"hdr")==0) {
            strcat(argstr,"5,2");
        } else 
            strcat(argstr,"0,2");

#ifdef DEBUG_4G
       // syslog(LOGOPTS,"DBUG : %s set connect tech mode = %s \n",__FUNCTION__,argstr);
#endif
        if(send_cmd(3, argstr,rt_buff)==false)
            return false;
    }
        return true;
}

static bool set_sim_pin()
{
    char rt_b[MAX_RT_BUFF_LEN];
    char *cmd_pin="at+cpin?";
    char conf_data[512];
    char set_simpin[256] = "at+cpin=";
    static int flag = 0;

    if(send_cmd(3,cmd_pin,rt_b)==false) {
        return false;
    }

    if(NULL!=strstr(rt_b,"READY")) {
        have_simcard_status=1;
        simcard_locked=0;
        return true;
    }  else if(NULL!=strstr(rt_b,"SIM PIN")) { /*sim is locked*/
        simcard_locked=1;
        have_simcard_status=1;
        //just do once when sim card is locked until user set another pin num.
        if (flag == 0) {
            flag=1;
            if (ctx) {
            uci_free_context(ctx);
            ctx=NULL;
        }
        ctx = uci_alloc_context();
        if (!ctx)
        {
            fprintf(stderr, "Out of memory\n");
            return(false);
        }

        if(get_option_by_section_name(ctx, confile, "connect", "simcardpin",  conf_data)==false){
            fprintf(stderr, "can not get simcardpin status from uci\n");
            return(false);
        }

#ifdef DEBUG_4G
        //syslog(LOGOPTS,"DBUG : %s get sim pin from uci file = %s \n",__FUNCTION__,conf_data);
#endif

        if(strlen(conf_data)<1) {
            fprintf(stderr, "simcardpin is empty\n");
            return false;
        }

        strcat(set_simpin,conf_data);
        
        memset(rt_b,0,sizeof(rt_b));
    
            syslog(LOGOPTS,"DBUG : %s just send once cmd for unlock sim pin = %s \n",__FUNCTION__,set_simpin);
            if(send_cmd(3,set_simpin,rt_b)==false) {
            return false;
            }
        }
    }else
        have_simcard_status=0;
    return true;
}

/* set current network operator COPS= */
static bool set_carrier_id()
{
    char conf_selection[8];
    char conf_id[32];
    char set_carrierid[256] = "at+cops=";
    char rt_b[MAX_RT_BUFF_LEN];

    // get user carrier_selection setting from uci
    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx) {
        fprintf(stderr, "Out of memory\n");
        return(false);
    }
    if(get_option_by_section_name(ctx, confile, "connect", "carrier_selection",  conf_selection)==false){
        fprintf(stderr, "can not get carrier_selection from uci\n");
        return(false);
    }
    //syslog(LOGOPTS,"DBUG:[%s] get carrier_selection=%s from uci file\n",__FUNCTION__,conf_selection);

    // get user carrier_id setting from uci
    if (0 == strcmp(conf_selection,"2") || 0 == strcmp(conf_selection,"3")) { //manual or fixed         
        if (ctx) {
            uci_free_context(ctx);
            ctx=NULL;
        }
        ctx = uci_alloc_context();
        if (!ctx) {
            fprintf(stderr, "Out of memory\n");
            return(false);
        }
        if(get_option_by_section_name(ctx, confile, "connect", "carrier_id",  conf_id)==false){
            fprintf(stderr, "can not get carrier_id status from uci\n");
            return(false);
        }
        //syslog(LOGOPTS,"DBUG:[%s] get carrier_id=%s from uci file\n",__FUNCTION__,conf_id);
    }

    // set current network operator according to user 
    if (0 == strcmp(conf_selection,"0")) { //auto mode
        //syslog(LOGOPTS,"DBUG:[%s] will set auto mode\n",__FUNCTION__);
        strcat(set_carrierid,"0");
    }
    if (0 == strcmp(conf_selection,"1")) { //based on SIM mode
        char simnum[32];
        char get_num[32];
        char*p;
        memset(simnum,0,sizeof(simnum));
        strcpy(simnum,get_single_status_value("imsi"));
        p=simnum;

        //if get Unknown or others string will return false
        if (strlen(simnum)<10) return false;
        
        //get special position number 
        memset(get_num,0,sizeof(get_num));
        strncpy(get_num,p,6);
        //syslog(LOGOPTS,"DBUG:[%s] get carrier id=%s based on SIM\n",__FUNCTION__,get_num);
        strcat(set_carrierid,"1,2,");
        strcat(set_carrierid,get_num);
    }
    if (0 == strcmp(conf_selection,"2") || 0 == strcmp(conf_selection,"3")) { //manual or fixed carrier_id mode
        if( 0 == strcmp(conf_id,"0") ) { //fixed carrier_id mode, but user set id: 0
            //syslog(LOGOPTS,"DBUG:[%s] fixed id=%s, will set auto mode\n",__FUNCTION__, conf_id);
            strcat(set_carrierid,"0");
        } else {
            //syslog(LOGOPTS,"DBUG:[%s] will set carrier_id=%s from uci file\n",__FUNCTION__,conf_id);
            strcat(set_carrierid,"1,2,");
            strcat(set_carrierid,conf_id);  
        }
    }
    
    syslog(LOGOPTS,">>>>>DBUG:[%s] send cmd:%s to set current network operator\n",__FUNCTION__,set_carrierid);
    memset(rt_b,0,sizeof(rt_b));
    if(send_cmd(3, set_carrierid, rt_b)==false) {
        return false;
    }

    return true;
}

static bool disconnectdata()
{
    char rt_b[MAX_RT_BUFF_LEN];
    char wr_buff[512];
    int i,rc;
    char *rb="_DISCONNECTED";

#ifdef DEBUG_4G
    //syslog(LOGOPTS,"DBUG : %s send disconnect \n",__FUNCTION__);
#endif

    char *cmd_disconnect="at$nwqmidisconnect";

    //if(send_cmd(3,cmd_disconnect,rt_b)==false) {
    //    return false;
    //}
    send_cmd(3,cmd_disconnect,rt_b);
    set_single_status_value("connect_status",rb);
    set_cur_state(DISCONNECTED);

    write_status_file();
    return true;
}

static bool connectdata()
    {
    disconnectdata();
    sleep(2);
    if(set_sim_pin()==false) {
#ifdef DEBUG_4G
        syslog(LOGOPTS,"DBUG : %s set sim pin return false \n",__FUNCTION__);
#endif
//        return false;
    }
    set_connect_tech_mode();

    // set current network operator
    set_carrier_id();

    if (get_connect_conf_uci()==false)
        return false;
    return true;
}


static bool reset_dhcp_client() {
#ifdef DEBUG_4G
    syslog(LOGOPTS,"Reset dhcp client");
#endif

    SubProgram_Start("/etc/init.d/dhcp_client","stop");
    sleep(3);
    //SubProgram_Start("ifconfig","eth2 up");
    //sleep(1);
    SubProgram_Start("/etc/init.d/dhcp_client","start");
    return true;
}

static bool mresetmodule()
{
    char rt_b[MAX_RT_BUFF_LEN];
    char wr_buff[512];
    int i;

    char *cmd_reset="at+cfun=7";

    if(send_cmd(3,cmd_reset,rt_b)==false) {
        return false;
    }

    sleep(2);
    char *cmd_reset1="at+cfun=6";

    if(send_cmd(3,cmd_reset1,rt_b)==false) {
        return false;
    }
#ifdef DEBUG_4G
    printf("DBUG : %s get returen = %s \n",__FUNCTION__,rt_b);
#endif
    //sleep(5);
    return true;
}

static void check_modem_type(void)
{
    FILE *st_fd = NULL;
    char buf[256];
    char *start, *end;
    char keyword[32],val[64];

    st_fd = fopen(Hw_Desc, "rb");

    if (st_fd == NULL)
    {
        syslog(LOGOPTS,"Can not open file %s\n",Hw_Desc);
        return false;
    } 

    memset(buf,0,sizeof(buf));
    while (fgets(buf,sizeof(buf),st_fd)){
        if ((strlen(buf)>2)&&(buf[0]!='#')) {
            start = buf;
            end = strchr(start, '=');
            if(NULL==end) {
                syslog(LOGOPTS,"file %s format not corrct\n",Hw_Desc);
                break;
            }
            strncpy(keyword,start,end-start);
            if (strcmp(keyword,"modem_type")==0) {
                end++;
                strcpy(val,end);
                if(strstr(val,"E362") != NULL){
                    modem_type=1;
                    syslog(LOGOPTS,"System using E362 as LTE modem modem_type set to %d\n",modem_type);
                } else {
                    modem_type =0;
                    syslog(LOGOPTS,"System using E371 as LTE modem modem_type set to %d\n",modem_type);
                }
                break;
            }
        }
        memset(buf,0,sizeof(buf));
    }
    fclose(st_fd);
}

#ifdef VIP4G_SERVER
static bool check_sim_card()
{
    FILE * simfd;
    int i,rc;
    char rt[2];
    char *rb="_DISCONNECTED";
    if ((simfd = fopen("/sys/class/button/SIM_STAT/status","r")) < 0) {
        fprintf(stderr, "ERROR: open(%s) failed, errno=%d\n", 
            "simfd", errno);
        return false;
    }

    memset(rt,0,sizeof(rt));
    fgets(rt,2,simfd);
    fclose(simfd);

    rt[1]='\0';

    if ( strcmp(rt,"0")==0) {
#ifdef DEBUG_4G
        //syslog(LOGOPTS,"\n\nSim card status is inserted!\n", rt);
#endif
//        set_single_status_value("have_simcard","1");
        have_simcard_status=1;

        if (set_sim_pin()==false){
            syslog(LOGOPTS,"\n\nSim card read cpin ?return false!\n");
            }
    } else {
#ifdef DEBUG_4G
        //syslog(LOGOPTS,"\n\nSim card status is removed and return false!\n", rt);
#endif
//        set_single_status_value("have_simcard","0");
        have_simcard_status=0;

        /*if no simcard set connect status to _DISCONNECTED*/
        set_single_status_value("connect_status",rb);
    }
    write_status_file();

    return have_simcard_status;
}
#endif

#ifdef IPn4G_SERVER
static bool check_sim_card()
{
    FILE * simfd;
    int i,rc;
    char rt[512];
    char *rb="_DISCONNECTED";
    char get_simid[256] = "at+cpin?";
    char rt_b[MAX_RT_BUFF_LEN];
    char wr_buff[512];

    if (set_sim_pin()==false){
        syslog(LOGOPTS,"\n\nSim card read cpin ?return false!\n");
            }
    if(have_simcard_status==0) {
        set_single_status_value("connect_status",rb);
        //syslog(LOGOPTS,"\n\nSim card not inserted!\n");
        set_single_status_value("simpin","No SIM Card");
    } else {
        if (simcard_locked ==1) {
            set_single_status_value("simpin","SIM Card Locked");
        }
    }
    write_status_file();

    return have_simcard_status;
}

#endif
static void do_hard_reset()
{
    char log_buff[254];
    sprintf(log_buff,"%s: 4G module is taking hard reset\n","mpci-4g_server");
    log_info_to_flash(log_buff);
    
#ifdef VIP4G_SERVER
    SubProgram_Start("ifconfig","eth2 down");
    SubProgram_Start("ifdown","wan2");
#endif
#ifdef IPn4G_SERVER
    SubProgram_Start("ifdown","wan");
    SubProgram_Start("/etc/init.d/dhcp_client","stop");
#endif
    SubProgram_Start("/etc/init.d/dhcp_client","stop");
    SubProgram_Start("killall","-9 udhcpc");
    UnLockFile(fd_portbusy);
    close(rfd);
    sleep(3);
    SubProgram_Start("mpci","-gpio set 8 0");
    sleep(1);
    SubProgram_Start("mpci","-gpio set 8 1");
    //sleep(1);
    //sleep(3);
    //SubProgram_Start("ifconfig","eth2 up");
    //set_req_state(RECONNECT);
    sleep(3);
}

static void do_soft_reset()
{
    char log_buff[254];
    sprintf(log_buff,"%s: 4G module is taking soft reset\n","mpci-4g_server");
    log_info_to_flash(log_buff);
    
#ifdef VIP4G_SERVER
    SubProgram_Start("ifconfig","eth2 down");
    SubProgram_Start("ifdown","wan2");
#endif
#ifdef IPn4G_SERVER
    SubProgram_Start("ifdown","wan");
    SubProgram_Start("/etc/init.d/dhcp_client","stop");
#endif
    SubProgram_Start("/etc/init.d/dhcp_client","stop");
    SubProgram_Start("killall","-9 udhcpc");
    sleep(3);
    mresetmodule();
    //sleep(1);
    //restoreremotetermios();
    UnLockFile(fd_portbusy);
    close(rfd);
    sleep(3);
    //SubProgram_Start("ifconfig","eth2 up");
}





 
// check current tech, if tech going up from EDGE--->HSPA---->LTE, save it
void save_last_known_state()
{  

    static int last_tech = 2;
    int current_tech = 0;
    char buf[64];
    int buf_len=strlen(status_set[0].initval);
    char rt_b[MAX_RT_BUFF_LEN];
    char *rt_buff ;
    rt_buff=rt_b;

    strcpy(buf, status_set[0].initval);                    
    //syslog(LOGOPTS,">>>>>>>>>DBUG : %s i=%d [%s]\n",__FUNCTION__, i, buf);        
    
    //E362 support: GSM, GPRS, EDGE, UMTS, HSDPA, HSUPA, HSPA+, HSPA+DC, LTE, GW, GWL, 1xRTT, EvDO, EvDO Rel0, EvDO
    //E371 support: GSM, GPRS, EDGE, UMTS, HSDPA, HSUPA, HSPA+, HSPA+DC, LTE"
    if (strstr(buf, "GSM") != NULL){
        current_tech = 2;                
    }
    if (strstr(buf, "GPRS") != NULL){
        current_tech = 2;                
    }
    if (strstr(buf, "EDGE") != NULL){
        current_tech = 2;                
    }
    if (strstr(buf, "UMTS") != NULL){
        current_tech = 3;                
    }
    if (strstr(buf, "HSDPA") != NULL){
        current_tech = 3;                
    }
    if (strstr(buf, "HSUPA") != NULL){
        current_tech = 3;                
    }
    if (strstr(buf, "HSPA+") != NULL){
        current_tech = 3;                
    }
    if (strstr(buf, "HSPA+DC") != NULL){
        current_tech = 3;                
    }
    if (strstr(buf, "LTE") != NULL){
        current_tech = 4;                
    }

    //syslog(LOGOPTS,">>>>>DBUG : %s i=%d [%s] last_tech=%d, current_tech=%d\n",__FUNCTION__,  i, buf, last_tech, current_tech);

    // if LTE, do at+nwpdn to save
    if ( current_tech > last_tech) { 
         syslog(LOGOPTS,">>>>>DBUG : Save Good Known State. [%s] [%s] last_tech=%d, current_tech=%d, \n", status_set[6].initval, buf, last_tech, current_tech);
         // send at+nwpdn to save
       if (send_cmd(3, "at$NWPDN=0", rt_buff)==false) {
#ifdef DEBUG_4G
          syslog(LOGOPTS,"DBUG : %s ERROR: send cmd(at$NWPDN=0) failed\n", __FUNCTION__);
#endif
       } 
         last_tech = current_tech;
     } // carrier tech going up           
    
}

static bool force4G()
{
    char rt_b[MAX_RT_BUFF_LEN];
    syslog(LOGOPTS,"DBUG : %s \n", __FUNCTION__);
    return send_cmd(3, "AT$NWRAT=3,2",rt_b);

}
static bool force3G()
{
    syslog(LOGOPTS,"DBUG : %s \n", __FUNCTION__);
    char rt_b[MAX_RT_BUFF_LEN];
    return send_cmd(3, "AT$NWRAT=2,2",rt_b);

}

static bool force2G()
{
    syslog(LOGOPTS,"DBUG : %s \n", __FUNCTION__);
    char rt_b[MAX_RT_BUFF_LEN];
    return send_cmd(3, "AT$NWRAT=1,2",rt_b);

}

static bool forceAuto()
{
    syslog(LOGOPTS,"DBUG : %s \n", __FUNCTION__);
    char rt_b[MAX_RT_BUFF_LEN];
    return send_cmd(3, "AT$NWRAT=0,2",rt_b);

}

static int get_duration()
{
    char val[256];
    syslog(LOGOPTS,"DBUG : %s \n", __FUNCTION__);
    char timer_c[10];
    int timer;

    strcpy(val,get_single_status_value("connect_duration"));
    syslog(LOGOPTS,"DBUG : %s get duration from struct %s\n", __FUNCTION__,val);
    sprintf(timer_c," %s seconds",val);
    timer=atoi(timer_c);
    syslog(LOGOPTS,"DBUG : %s get timer from %s is %d\n", __FUNCTION__,timer_c,timer);
    return timer;
}

static int get_curr_techmode()
{
    char val[256];
    syslog(LOGOPTS,"DBUG : %s \n", __FUNCTION__);
    char techmode_c[10];
    char *start, *end;
    int tmode=88;

    strcpy(val,get_single_status_value("rat"));
    syslog(LOGOPTS,"DBUG : %s get current tech mode from struct %s\n", __FUNCTION__,val);
    end=strchr(val,',');
    if (end !=NULL) {
        memset(techmode_c,0,sizeof(techmode_c));
        strncpy(techmode_c,val,end-val);
    }
    syslog(LOGOPTS,"DBUG : %s get current tech mode first value = %s\n", __FUNCTION__,techmode_c);
    
    tmode=atoi(techmode_c);
    syslog(LOGOPTS,"DBUG : %s get current tech mode first value = %s number = %d\n", __FUNCTION__,techmode_c, tmode);
    return tmode;

}
#if 0
static void auto_techmode(void)
{
    char buf[64];
    int timer;
    static int step=0;
    int cur_mode=88;

    // if customer setting is auto, we do this algorithsm
    if (step == 3) return;
    
    if (strcmp(techmode_data,"auto")==0){

        strcpy(buf, status_set[0].initval);
        if (strstr(buf, "LTE") == NULL){
            timer=get_duration();
            if (step == 0) {
                if (timer >300) {
                    syslog(LOGOPTS,"DBUG : %s Enter step %d\n", __FUNCTION__,step);
                    force4G();
                    disconnectdata();
                    step++;
                }
            }else if (step == 1){
                if (timer >120) {
                    syslog(LOGOPTS,"DBUG : %s Enter step %d\n", __FUNCTION__,step);
                    force3G();
                    disconnectdata();
                    step++;
                }
            } else if (step == 2){
                syslog(LOGOPTS,"DBUG : %s Enter step %d\n", __FUNCTION__,step);
                forceAuto();
                disconnectdata();
                step++;
            }
        } else {
            cur_mode=get_curr_techmode();
            if (cur_mode != 0) {
                forceAuto();
            }

        }
    }
}
#else
static void auto_techmode(void)
{
    char buf[64];
    int timer;
    int cur_mode=88;

    struct timeval tv;
    struct timezone tz;
    static unsigned int us_begin=0, s_begin=0;
    unsigned int us_elaped_time=0;

    // if customer setting is auto, we do this algorithsm
    if (strcmp(techmode_data,"auto")==0){
        if (tech_step == 0) {
            if(cur_state == CONNECTED ) {
                strcpy(buf, status_set[0].initval);
                if (strstr(buf, "LTE") == NULL){
                    disconnectdata();
                    force4G();
                    tech_step =1;
                    if (0==gettimeofday (&tv, &tz)) { /*get starttime again*/
                        s_begin=tv.tv_sec;
                        //us_begin=tv.tv_usec;
                    }else{
                        syslog(LOGOPTS,"%s get timeofday error\n", __FUNCTION__);              
                    }
                }
            }
       } else if(tech_step ==1){
             if(cur_state == CONNECTED){
                 forceAuto();
                 tech_step = 9;
             }else{
                if (-1==gettimeofday (&tv, &tz)) {
                    syslog(LOGOPTS,"%s get timeofday error\n", __FUNCTION__);              
                }
                us_elaped_time= (tv.tv_sec-s_begin);
                
                if ( us_elaped_time > 150) {
                     disconnectdata();
                     force3G();
                     tech_step=2;
                     if (0==gettimeofday (&tv, &tz)) { /*get starttime again*/
                         s_begin=tv.tv_sec;
                         //us_begin=tv.tv_usec;
                     }else{
                         syslog(LOGOPTS,"%s get timeofday error\n", __FUNCTION__);              
                     }
                }
             }
       }else if(tech_step ==2 ){
            if (-1==gettimeofday (&tv, &tz)) {
                syslog(LOGOPTS,"%s get timeofday error\n", __FUNCTION__);              
            }
            us_elaped_time= (tv.tv_sec-s_begin);

            if (us_elaped_time >150) {
                forceAuto();
                tech_step = 10;
            }
       }
    }
}


#endif

/* iccid
 * Telecommunication 89 
 * ROGERS       302720 
 * Bell         302610 
 * TELUS        302220 
 * WIND         302490 
 * Mobilicity   302320 
 * Videotron1   302500 
 * Videotron2   302510 
 * Fido         302370 
 * SaskTel      302780 
 */
static char * get_line_from_file(char * filename, char *keyword)
{
	char tmpbuff[256];
	FILE * f=NULL;
	char *p;

	if ((f = fopen(filename, "r")))
    {
		memset(tmpbuff,0, sizeof(tmpbuff));
        while (fgets(tmpbuff, sizeof(tmpbuff), f))
        {
        	p=tmpbuff;
        	if (strstr(tmpbuff,keyword)!=NULL) {
        		break;
        	}else {
        		memset(tmpbuff,0, sizeof(tmpbuff));
        	}
        }
        fclose(f);
    }
	return p;
}
static char *parse_iccid()
{
    char simnum[32];
    int i,j;
    int step=0;
    char *p,*cp;
    char get_num[22];
    char carrier[32];
    char line_from_file[256];
    char *spos,*epos;

    memset(simnum,0,sizeof(simnum));
    strcpy(simnum,get_single_status_value("simid"));
    p=simnum;

    //if get Unknown or others string will return false
    if (strlen(simnum)<10) return false;
   
    //get special position number 
    memset(get_num,0,sizeof(get_num));
    strncpy(get_num,p+3,6);


#if 0
    if(strcmp(get_num,"302720")==0){
        strcpy(carrier,"ROGERS");
    } else if(strcmp(get_num,"302610")==0){
        strcpy(carrier,"Bell");
    } else if(strcmp(get_num,"302220")==0){
        strcpy(carrier,"TELUS");
    } else if(strcmp(get_num,"302490")==0){
        strcpy(carrier,"WIND");
    } else if(strcmp(get_num,"302320")==0){
        strcpy(carrier,"Mobilicity");
    } else if(strcmp(get_num,"302500")==0){
        strcpy(carrier,"Videotron1");
    } else if(strcmp(get_num,"302510")==0){
        strcpy(carrier,"Videotron2");
    } else {
        strcpy(carrier,"Unknown");
    }
    return carrier;
#else
    //find carrier in /etc/apntable
    cp=carrier;
    strcpy(line_from_file,get_line_from_file("/etc/apntable", get_num) );
    spos=strchr(line_from_file,',');
    memset(carrier,0,sizeof(carrier));
    if(spos!=NULL){
    	spos++;
    	epos=strchr(spos,',');
    	if(epos!=NULL){
            strncpy(carrier,spos,epos-spos);

    	}else {
            strcpy(carrier,"Unknown");
    	}
    }else{
        strcpy(carrier,"Unknown");
    }

    return cp;
#endif
}

static unsigned int get_nwband()
{
    char rt_b[MAX_RT_BUFF_LEN];
    char wr_buff[512];
    char temp1[512];
    unsigned int bandval;
    int i;

    //syslog(LOGOPTS,"DBUG : %s \n", __FUNCTION__);
    //read the band value first 
    memset(rt_b,0,sizeof(rt_b));
    memset(wr_buff,0,sizeof(wr_buff));

    if(send_cmd(3, "AT$NWBAND?",rt_b)==false){
        syslog(LOGOPTS,"DBUG : %s send at$nwband? false return %d \n", __FUNCTION__,false);
        return false;
    } else {
        memset(temp1,0,sizeof(temp1));
        getRightPart(rt_b,temp1,sizeof(temp1));
        getHexNumberPart(temp1,wr_buff,sizeof(temp1));
        sscanf(wr_buff,"%x",&bandval);
        if (bandval == 0) {
            bandval++;
        }
        return bandval;
    }
}

/* return value:
 *  0 : return false, set error
 *  1 : return true, set sucess
 *  2 : dont need set
 */
static int set_nwband_preference(unsigned int mask)
{
    char argstr[64]="AT$NWBAND=";
    char rt_b[MAX_RT_BUFF_LEN];
    char str_band[32];
    int rtval;
    static int flag = 0;
    unsigned int ori_band=0, s_band=0,b_band=0;
    int i, need_set=0;

    if(flag ==1) return 2;
    syslog(LOGOPTS,"DBUG : %s will set band for special iccid using mask 0x%x\n", __FUNCTION__,mask);
    //read the band value first 
    ori_band=get_nwband();
    if (ori_band == false) {
        syslog(LOGOPTS,"DBUG : %s The band read return false \n", __FUNCTION__);
        return false;
    }

    //if the band value already set to correct value ,just return 2
    //s_band=ori_band;
    need_set=0;
#if 0
    //looking for evevry bit to find out which band will be set
    for(i=0;i<32;i++) {
        if(mask & (1<<i)) {
            //syslog(LOGOPTS,"DBUG : %s get mask = %d s_band = 0x%x and the num =%d \n", __FUNCTION__,i,s_band,i);
            b_band = ori_band & (1<< i);
            if (b_band == 0) {
                //here need set a new band for bit i
                need_set =1 ;
                s_band |= (1<<i);  
                syslog(LOGOPTS,"DBUG : %s need set new band to 0x%x for bit %d\n", __FUNCTION__,s_band,i);
            }
            
        } else
            continue ;
    }
#else
    //syslog(LOGOPTS,"DBUG : %s  mask = 0x%x ori = 0x%x \n", __FUNCTION__,mask,ori_band);
    if(ori_band != mask) {
        need_set =1;
        s_band=mask;
    }
    
#endif
    if (need_set == 0) {
        flag=1;
        return 2;
    } 

    //convert hex number to string
    memset(str_band,0,sizeof(str_band));
    sprintf(str_band,"%x",s_band);
    //syslog(LOGOPTS,"DBUG : %s The band read int ori = 0x%x  s_band = 0x%x and sband str = %s \n", __FUNCTION__,ori_band,s_band,str_band);

    //send the at command to change band value return true(1) is success.
    strcat(argstr,str_band);
    if (send_cmd(3, argstr,rt_b)==false){
        syslog(LOGOPTS,"DBUG : %s send %s false \n", __FUNCTION__,argstr);
        return false;
    }else {
        //syslog(LOGOPTS,"DBUG : %s send cmd = %s success\n", __FUNCTION__,argstr);
        flag=1;
        return true;
    }
}

#ifdef IPn4G_SERVER
static bool set_dcd_asser(void)
{
    char dcd_data[8];

    if(ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx)
    {
        fprintf(stderr, "Out of memory\n");
        return(false);
    }

    if(get_option_by_section_name(ctx, "comport", "com1", "COM1_DCD_Assert", dcd_data)==false){
            fprintf(stderr, "can not get dcd from uci\n");
            return(false);
    }
    if (strcmp(dcd_data,"C")==0){
        if(get_cur_state()== CONNECTED)
        {
            //syslog(LOGOPTS,"DBUG : DCD get connect status and assert\n");
            SubProgram_Start("/bin/mpci","-gpio set 18 0");
        }else {
            //syslog(LOGOPTS,"DBUG : DCD get disconnect status and disassert\n");
            SubProgram_Start("/bin/mpci","-gpio set 18 1");
        }
    } 
    return true;
}
#endif

static bool server()
{
    int reqstate;
    int curstate;
    char wanip[256];
    char log_buff[256];
    int need_soft_reset=0;
    int ippthrough=0;
    FILE * nvfd;
    char rt[512];
    unsigned int band_mask=0 , default_band_mask=0;
    char carri_get[32];
    int i,j;
    static int flag=0;
    while(true) { 
#ifdef IPn4G_SERVER
        set_dcd_asser();
#endif
        sleep(1);
        fd_portbusy = open (portFILEBusyFlagFile, O_WRONLY);
        if (fd_portbusy<0) {
            syslog(LOGOPTS,"%s:locked fd_portbusy<0\n",__func__);
            return false;
        }
        LockFile(fd_portbusy);    

        for(i=0;i<10;i++) {
            if ((rfd = open(devname, (O_RDWR | O_NDELAY))) < 0) {
                syslog(LOGOPTS,"ERROR: failed to open %s, errno=%d\n",
                    devname, errno);
                set_single_status_value("connect_status","_DISCONNECTED");
                set_cur_state(DISCONNECTED);
                write_status_file();
                sleep(3);
                continue ;
              //  goto fail_process;
            }else 
                break;
        }

        if(i==10) {
            for(j=0;j<3;j++) {
                if ((rfd = open(alt_devname, (O_RDWR | O_NDELAY))) < 0) {
                    syslog(LOGOPTS,"ERROR: failed to open %s, errno=%d\n",
                           alt_devname, errno);
                    sleep(3);
                    continue ;
                  // goto fail_process;
                }else {
                    need_soft_reset=1;
                    //set_req_state(NEED_RESET);
                    break;
                }
            }

            if(j==3) {
                do_hard_reset();
                //continue ;
            }
        }

        if(need_soft_reset == 1) {
            need_soft_reset=0;
            do_soft_reset();
            //continue ;
        }

        //try to get NV set result on E371
        if (modem_type == 0) {
            if ((nvfd = fopen("/var/run/nvset_result","r")) < 0) {
                fprintf(stderr, "ERROR: open(%s) failed, errno=%d\n", 
                        "nvfd", errno);
                return false;
        }

        memset(rt,0,sizeof(rt));

        fgets(rt,sizeof(rt),nvfd);
        fclose(nvfd);
        if (strstr(rt,"0000")!=NULL) {
            do_hard_reset();
            }
        }

        //syslog(LOGOPTS,"%s err seletc count =%d \n",__FUNCTION__,err_select_count);

        if (err_select_count > 10) {
            err_select_count=0;
            do_hard_reset();
            //continue ;
        }

        saveremotetermios();
        setremotetermios();

        reqstate=get_req_state();
        curstate=get_cur_state();

        if(reqstate == QUIT_SERVER){
            disconnectdata();
            return false;
        }

       //read in case something stuck in serial channel.
       read(rfd, ibuf, sizeof(ibuf));
        memset(ibuf,0,sizeof(ibuf));

        check_sim_card();

        status_default++;
        if (send_status_cmd()==false) {
          //  syslog(LOGOPTS,"%s send status cmd return false will close file and re open it \n",__FUNCTION__);
            break;
        }else {
            err_select_count=0;
            SubProgram_Start("/bin/sh","/bin/parse_lte_status.sh");
        }
            //continue;

        if (have_simcard_status == 0) {
            break;
        }else {
            if (simcard_locked == 1) {
                break;
            }
        }

        set_connect_tech_mode();
        
        if (modem_type == 0 && curstate == DISCONNECTED) {
            default_band_mask =  (1 << SYS_SBAND_WCDMA_V_850 |
                                  1 << SYS_SBAND_WCDMA_II_PCS_1900 |
                                  1 << SYS_SBAND_WCDMA_I_IMT_2000 |
                                  1 << SYS_SBAND_GSM_PCS_1900 |
                                  1 << SYS_SBAND_GSM_850 |
                                  1 << SYS_SBAND_GSM_PGSM_900 |
                                  1 << SYS_SBAND_GSM_EGSM_900 |
                                  1 << SYS_SBAND_GSM_DCS_1800 );
    
            memset(carri_get,0,sizeof(carri_get));
            strcpy(carri_get,parse_iccid());
            //syslog(LOGOPTS,"%s According to ICCID, The carrier is %s \n",__FUNCTION__,carri_get);
            if((strcmp(carri_get,"Videotron1")==0) || (strcmp(carri_get,"Videotron2")==0) || strstr(carri_get,"T-Mobile US") !=NULL ){
            //if( get_special_simnum() ) {
                band_mask = default_band_mask | (1 << SYS_SBAND_WCDMA_IV_1700);
            } else 
                band_mask = default_band_mask ;
    
            if(set_nwband_preference(band_mask)==false) break;
        }

        if(is_weak_rssi()==true) {
            syslog(LOGOPTS,"%s rssi is too low \n",__FUNCTION__);
            disconnectdata();
            if (modem_type == 0) {
                #ifdef ADVANCE
                auto_techmode();
                #endif
            }
            break;
        }

        ippthrough=is_ip_passthrough();
        if(flag == 1) {
        if(connect_status_check()==true){

            if(need_save_autoapn_sign==1) 
            {
                if(check_autoapn(confapnstr)==true)
                {
                    add_apnhistory(autoapnstr);
                }else add_apnhistory(confapnstr);
            }
            

#ifdef VIP4G_SERVER
            fetch_Local_IP_ADDR("br-wan2",wanip);
#endif
#ifdef IPn4G_SERVER
            fetch_Local_IP_ADDR("br-wan",wanip);
#endif
                 
            if (strstr(wanip,"0.0.0.0")!=NULL) {
                syslog(LOGOPTS," module connected but br-wan ipaddr = %s we restart dhcp_client .....\n",wanip);
                reset_dhcp_client();
                for (i=1; i<10;i++) {
                    sleep(1);
                    if (strstr(wanip,"0.0.0.0")!=NULL) {
#ifdef VIP4G_SERVER
                        fetch_Local_IP_ADDR("br-wan2",wanip);
#endif
#ifdef IPn4G_SERVER
                        fetch_Local_IP_ADDR("br-wan",wanip);
#endif
                    } else
                        break;
                }

                if (strstr(wanip,"0.0.0.0")!=NULL) {
                    syslog(LOGOPTS," module connected but br-wan ipaddr = %s need reconnect it .....\n",wanip);
                    disconnectdata();
                }
            }
            //syslog(LOGOPTS," br-wan ipaddr = %s\n",wanip);
            if(ippthrough == 1) {
                if(strstr(wanip,"1.1.1.1")==NULL) {
                    syslog(LOGOPTS," module connected in IP Passthrough mode ipaddr = %s need reconnect it .....\n",wanip);
                    disconnectdata();
                }
            }

        }
    
        }
        flag=1;
        reqstate=get_req_state();
        curstate=get_cur_state();

        //syslog(LOGOPTS," req stat = %d cur state =  %d\n",reqstate, curstate);
        switch (reqstate)
        {

        case QUIT_SERVER:
            disconnectdata();
            return false;
        case RECONNECT:
            //syslog(LOGOPTS," current stat is reconnect  %d\n",RECONNECT);
            if(connectdata()==false){
                break;
            }
            reset_dhcp_client(); 
            set_req_state(CONNECT);
            break;

        case CONNECT:
            //syslog(LOGOPTS," current stat is connect  %d\n",CONNECT);
            if(curstate == DISCONNECTED) {
                if(connectdata()==false) {
                    if (modem_type == 0) {
                        #ifdef ADVANCE
                        auto_techmode();
                        #endif
                    }

                    break;
                }
                reset_dhcp_client();
            }else{
                if (modem_type == 0) {
                    //check if current tech is good as LTE, save it 
                    #ifdef ADVANCE
                    save_last_known_state();
                    auto_techmode();
                    #endif
                }
            }
            break;
        case DISCONNECT:
            //syslog(LOGOPTS," current stat is disconnect  %d\n",DISCONNECT);
            disconnectdata();
            break;
        case REMOTE_DEBUG:
            syslog(LOGOPTS," LTE enter remote debug mode..  \n");
            restoreremotetermios();
            UnLockFile(fd_portbusy);
            UnLockFile(fd);
            UnLockFile(fd_stat);
            close(rfd);
            exit(0);
            break;
        default:
            syslog(LOGOPTS," current stat is unknown  \n");
            break;
        }

#if 1
        restoreremotetermios();
        close(rfd);
        UnLockFile(fd_portbusy);
        //sleep(1);
#endif
    }

    disconnectdata();
    restoreremotetermios();
    UnLockFile(fd_portbusy);
    close(rfd);
    return true;
}

void sighandler(int signal)
{

    char log_buff[256];
#ifdef DEBUG_4G
    syslog(LOGOPTS,"\n\nGot signal %d Reset module !\n", signal);
    syslog(LOGOPTS,"Cleaning up...");
#endif

    sprintf(log_buff,"%s: 4G module recieved a signal %d to stop server\n","mpci-4g_server",signal);
    log_info_to_flash(log_buff);

    disconnectdata();
    restoreremotetermios();
    SubProgram_Start("/etc/init.d/dhcp_client","stop");
#ifdef VIP4G_SERVER
    SubProgram_Start("ifconfig","eth2 down");
    SubProgram_Start("ifdown","wan2");
#endif
#ifdef IPn4G_SERVER
    SubProgram_Start("ifconfig","eth1 down");
    SubProgram_Start("ifdown","wan");
#endif

    sleep(1);
    close(rfd);
    UnLockFile(fd_portbusy);
    sleep(1);
//    UnLockFile(fd);
    UnLockFile(fd_stat);

#ifdef DEBUG_4G
    syslog(LOGOPTS,"done\n");
#endif
    //return;

//    exit(1);
}

void usage(FILE *fp, int rc)
{
    fprintf(fp, "Usage: mpci-4g_server [-?h][-v] [-s speed] [-r atset_file] [-u] [-l device]\n\n"
        "\t-h?\tthis help\n"
        "\t-s\tbaud rate (default 115200)\n"
        "\t-l\tdevice to use\n"
        "\t-v\tshow version information\n"
        "\t-u\tget 4g module status from default status and set them to file /var/run/VIP4G_status\n"
        "\t-c\tStart a package data session\n"
        "\t-d\tStop a package data session\n"
        "\t-b\tStart Server running\n"
        "\t-n\tReset modules\n");
        exit(rc);
}

int main(int argc, char *argv[])
{
	int	c;
    FILE *f;
    FILE *f_stat;
    FILE *f_portbusy;

    int i;
    bool end_s;

    char log_buff[254];

#ifdef DEBUG_4G
    syslog(LOGOPTS,"DBUG : +++++++ start mpci-4g_server +++++++++++\n");
#endif

    /*create the busy file*/
    f=fopen( PortBusyFlagFile,"w+");
    if (f==NULL) {
        syslog(LOGOPTS,"ERROR: %s lock file can not open\n",PortBusyFlagFile);
        sprintf(log_buff,"%s: %s lock file can not open\n","mpci-4g_server",PortBusyFlagFile);
        log_info_to_flash(log_buff);
        return false;
    }
    fclose(f);     

     /*create the stat busy file*/
    f_stat=fopen(StatFILEBusyFlagFile,"w+");
    if (f_stat==NULL) {
        syslog(LOGOPTS,"ERROR: %s lock file can not open\n",StatFILEBusyFlagFile);
        sprintf(log_buff,"%s: %s lock file can not open\n","mpci-4g_server",StatFILEBusyFlagFile);
        log_info_to_flash(log_buff);
        return false;
    }
    fclose(f_stat);     

    /*create the port busy file*/
    f_portbusy=fopen(portFILEBusyFlagFile,"w+");
    if (f_portbusy==NULL) {
        syslog(LOGOPTS,"ERROR: %s lock file can not open\n",f_portbusy);
        sprintf(log_buff,"%s: %s lock file can not open\n","mpci-4g_server",f_portbusy);
        log_info_to_flash(log_buff);
        return false;
    }
    fclose(f_portbusy);     
	gotdevice = 0;

    while ((c = getopt(argc, argv, "?hvcdnubs:l:")) > 0) {
        switch (c) {
        case 'l':
            gotdevice++;
            devname = optarg;
            break;
        case 's':
            baud = atoi(optarg);
            if (baud2flag(baud) < 0) {
                fprintf(stderr,
                    "ERROR: baud speed specified %d\n",
                    baud);
                exit(false);
            }
            break;
        case 'v':
            printf("%s: version %s\n", argv[0], version);
            exit(true);
        case 'b':
            start_server++;
            break;
        case 'u':
            status_default++;
            break;
        case 'c':
            connect_data++;
            break;
        case 'd':
            disconnect_data++;
            break;
        case 'n':
            mreset++;
            break;
        case 'h':
        case '?':
            usage(stdout, true);
            break;
        default:
            fprintf(stderr, "ERROR: unkown option '%c'\n", c);
            usage(stderr, false);
            break;
        }
    }

    if (argc<2) {
        fprintf(stderr, "Need Some argument\n");
        usage(stderr, false);
    }

    (void) setsid();

    /*
	 *	Set the signal handler to restore the old termios .
	 */
	sact.sa_handler = sighandler;
	sigaction(SIGHUP, &sact, NULL);
	sigaction(SIGINT, &sact, NULL);
    sigaction(SIGQUIT, &sact, NULL);
    sigaction(SIGPIPE, &sact, NULL);
    sigaction(SIGTERM, &sact, NULL);

#if 1
    fd = open (PortBusyFlagFile, O_WRONLY);
    if (fd<0) {
        syslog(LOGOPTS,"%s:locked fd<0\n",__func__);
    }
    LockFile(fd);    
#endif
    /*set init stat */
    set_cur_state(DISCONNECTED);

    if(start_server >0)
    {
        if(is_carrier_disable()==1){
            syslog(LOGOPTS,"\n\nEnd server for setting is disabled !\n");
        }else {
            check_modem_type();
            init_status_set();

            do {
                end_s=server();
            } while(end_s==true);
        }
    }
    syslog(LOGOPTS,"\n\n mpci-4g_server is stopped !\n");

//    disconnectdata();
    set_single_status_value("connect_status","_DISCONNECTED");
    set_cur_state(DISCONNECTED);
    write_status_file();

    restoreremotetermios();
#ifdef VIP4G_SERVER
    SubProgram_Start("ifconfig","eth2 down");
    SubProgram_Start("ifdown","wan2");
#endif
#ifdef IPn4G_SERVER
    SubProgram_Start("ifconfig","eth1 down");
    SubProgram_Start("ifdown","wan");
#endif
    SubProgram_Start("/etc/init.d/dhcp_client","stop");
    //sleep(1);

    close(rfd);
    UnLockFile(fd_portbusy);
    //restorelocaltermios();
    UnLockFile(fd);
    UnLockFile(fd_stat);
    //SubProgram_Start("ifup","wan");

    return true;

fail_process:
    disconnectdata();
    sleep(2);

    restoreremotetermios();
#ifdef VIP4G_SERVER
    SubProgram_Start("ifconfig","eth2 down");
    SubProgram_Start("ifdown","wan2");
#endif
#ifdef IPn4G_SERVER
    SubProgram_Start("ifconfig","eth1 down");
    SubProgram_Start("ifdown","wan");
#endif
    SubProgram_Start("/etc/init.d/dhcp_client","stop");
    sleep(1);

    close(rfd);
    UnLockFile(fd_portbusy);
    sleep(1);
    UnLockFile(fd);
    UnLockFile(fd_stat);

    return false;
}






static int parse_add_apnlist(char *rec_apnstr)
{
    char *p;
    char *p1;
    int i,j;
    char apnstr[32];
    p=rec_apnstr;
    while(*p!=',' && *p!=0)p++;
    if(*p!=',')return 0;
    p++;
    while(*p!=',' && *p!=0)p++;
    if(*p!=',')return 0;
    p++;
next_parse:
    p1=p;
    while(*p!=',' && *p!=0 && *p!='\r' && *p!='\n')p++;
    i=p-p1;
    if(apnlist_all<20 && i>1)
    {
        if(i<32)
        {
            strncpy(apnstr,p1,i);
            apnstr[i]=0;

            j=0;
            while(j<apnlist_all)
            {
                if(strcmp(apnstr,apn_trylist[j])==0)break;
                j++;
            }
            if(j==apnlist_all)
            {
                strcpy(apn_trylist[apnlist_all],apnstr);
                apnlist_all++;
            }
        }
        if(*p==',')
        {
            p++;
            goto next_parse;
        }
    }

return 0;
}


static int search_apn_infile_tolist(char *imsi6c)
{
    int i,j;
    FILE *f;
    char *p;
    char *p1;
    char tmpbuff[256];
    char imsistr[8];
    i=strlen(imsi6c);
    if(i<4 || i>6)return -1;
    strcpy(imsistr,imsi6c);

    //find in /etc/apnhistory to apn_trylist with imsi
    if ((f = fopen("/etc/apnhistory", "r"))) 
    {
        while (fgets(tmpbuff, 256, f)) 
        {
            p=tmpbuff;
            while(*p!=',' && *p!=0)p++;
            if(*p!=',')continue;

            i=p-tmpbuff;
            j=0;
            while(j<i)
            {
                if(tmpbuff[j]!=imsistr[j])break;
                j++;
            }
            if(j==i)
            {
                parse_add_apnlist(tmpbuff);
//printf("+++++++apn from history:%s\n",tmpbuff);
            }
        }
        fclose(f);
    }

    //find in /etc/apntable to apn_trylist with imsi
    f=NULL;
    if ((f = fopen("/etc/apntable", "r"))) 
    {
        while (fgets(tmpbuff, 256, f)) 
        {
            p=tmpbuff;
            while(*p!=',' && *p!=0)p++;
            if(*p!=',')continue;

            i=p-tmpbuff;
            j=0;
            while(j<i)
            {
                if(tmpbuff[j]!=imsistr[j])break;
                j++;
            }
            if(j==i)
            {
                parse_add_apnlist(tmpbuff);
//printf("+++++++apn from table:%s\n",tmpbuff);
            }
        }
        fclose(f);
    }
    return apnlist_all;
}

static int search_apn_infile_tolist_roam()
{
    //static char networklist[10][8];
    int i,j;
    i=0;
    while(i<10)
    {
        if(strlen(networklist[i])<4)break;
        search_apn_infile_tolist(networklist[i]);
        i++;
    }

    return apnlist_all;
}

static int get_imsi_mccmnc(char *imsi6c)
{
    char imsi_buff[40];
    char rt_b[256];

    memset(rt_b,0,sizeof(rt_b));
    memset(imsi_buff,0,sizeof(imsi_buff));
    if (send_cmd(3,"at+cimi",rt_b)==false)return -1;
    getNumberPart(rt_b,imsi_buff,strlen(rt_b));
    if(strlen(imsi_buff)>6)
    {
        *imsi6c=imsi_buff[0];
        *(imsi6c+1)=imsi_buff[1];
        *(imsi6c+2)=imsi_buff[2];
        *(imsi6c+3)=imsi_buff[3];
        *(imsi6c+4)=imsi_buff[4];
        *(imsi6c+5)=imsi_buff[5];
        *(imsi6c+6)=0;
        return 6;
    }else return -1;
}

//networklist[10][8]
static int search_network()
{
    int i,j,net_num;
    char mccmnc[16];
    char cmphomeimsi[8];
    int cmphomeimsi_i;
    char rt_b[2048];
    char network_act[10];
    char network_actc0=0;
    int network_acti=0;
    char network_actc[10];
    int need_sort_insert;
    char *p;
    char *p1;
    char *p2;
    

    memset(network_act,0,sizeof(network_act));
    memset(rt_b,0,sizeof(rt_b));
    memset(mccmnc,0,sizeof(mccmnc));
    for(i=0;i<10;i++)networklist[i][0]=0;

    if (send_cmd(200,"at+cops=?",rt_b)==false)return -1;

//printf("+++++++at+cops:%s\n",rt_b);

    net_num=0;
    p=NULL;
    p=strstr(rt_b,"+COPS:");
    //+COPS: (0,"ROGERS","ROGERS","302720",7)
    while(p!=NULL && net_num<9)
    {
        need_sort_insert=0;
        network_acti=-1;
        mccmnc[0]=0;
        p1=NULL;
        p1=strchr(p,')');
        if(p1!=NULL)
        {
            p2=p1;
            while(p2>p && *p2!=',')p2--;
            if(*p2==',') 
            {
                network_acti=0;
                i=p1-p2-1;
                if(i>0 && i<10)
                {
                    strncpy(network_actc,p2+1,i);
                    network_actc[i]=0;
                    network_acti=atoi(network_actc);
                } 
            }
            p2--;
            if(*p2=='\"')p2--;
            p1=p2;
            while(p2>p && *p2!=',' && *p2!='\"')p2--;
            if(*p2==',' || *p2=='\"')
            {
                i=p1-p2;
                if(i>3 && i<16)
                {
                    strncpy(mccmnc,p2+1,i);
                    mccmnc[i]=0;
                    j=atoi(mccmnc);
                    if(j<1000)mccmnc[0]=0;
                    else sprintf(mccmnc,"%d",j);
                }
            }
            
            cmphomeimsi_i=0;
            if(network_acti>=0 && mccmnc[0]!=0) 
            {
                i=strlen(mccmnc);
                strncpy(cmphomeimsi,lastimsimccmnc,i);
                cmphomeimsi[i]=0;
                if(strcmp(mccmnc,cmphomeimsi)==0)cmphomeimsi_i=1;
            }

            if(network_acti>=0 && mccmnc[0]!=0 && cmphomeimsi_i==0)  //valid data to insert networklist[10]
            {
                network_actc0=network_acti;
                if(net_num==0)
                {
                    network_act[0]=network_actc0;
                    strcpy(networklist[0],mccmnc);
                    net_num++;
                }else
                {
                    i=0;
                    while(i<net_num)
                    {
                        if(strcmp(mccmnc,networklist[i])==0)
                        {
                            if(network_actc0>network_act[i])
                            {
                                need_sort_insert=1;
                                j=i;
                                while(j<net_num-1)
                                {
                                    network_act[j]=network_act[j+1];
                                    strcpy(networklist[j],networklist[j+1]);
                                    j++;
                                }
                                networklist[j][0]=0;
                                network_act[j]=0;
                                net_num--;
                            }
                            break;
                        }
                        i++;
                    }
                    if(need_sort_insert!=1)
                    {
                        if(i==net_num)need_sort_insert=1;
                    }

                    if(need_sort_insert==1)
                    {
                        i=0;
                        while(i<net_num)
                        {
                            if(network_actc0>network_act[i])break;
                            i++;
                        }
                        j=net_num-1;
                        while(j>=i)
                        {
                            network_act[j+1]=network_act[j];
                            strcpy(networklist[j+1],networklist[j]);
                            j--;
                        }
                        network_act[i]=network_actc0;
                        strcpy(networklist[i],mccmnc);
                        net_num++;
                    }
                }
            }
        }

        p2=p+4;
        p=NULL;
        p=strstr(p2,"+COPS:");
    }
//printf("+++++++roaming network num:%d\n",net_num);

    return net_num;
}

static int search_apn(char *apnstr)
{
    char imsi6c[8];
    int i;
    imsi6c[0]=0;
    if(get_imsi_mccmnc(imsi6c)<0)return -1;
//printf("+++++++mccmnc from imsi:%s\n",imsi6c);

    lastimsimccmnc[7]=0;
    if(strcmp(lastimsimccmnc,imsi6c)==0)
    {
        if(apnlist_i<apnlist_all && strlen(apn_trylist[apnlist_i])>2)
        {
            strcpy(apnstr,apn_trylist[apnlist_i]);
            if(apn_try_times<3)apn_try_times++;
            else 
            {
                apn_try_times=0;
                apnlist_i++;
            }
            return 1;
        }
    }else 
    {
        strcpy(lastimsimccmnc,imsi6c);
        homeorroam_search='h';
    }

    apn_try_times=0;
    apnlist_all=0;
    apnlist_i=0;
    for(i=0;i<20;i++)apn_trylist[i][0]=0;

    if(homeorroam_search=='r')
    {
        homeorroam_search='h';
        if(search_network()<1)return 0;
        if(search_apn_infile_tolist_roam()<1)return 0;
        if(apnlist_all==0)return 0;
//printf("+++++++apn roam num all:%d\n",apnlist_all);

        strcpy(apnstr,apn_trylist[apnlist_i]);
        apn_try_times++;
                
    }else  //search home
    {
        homeorroam_search='r';
        if(search_apn_infile_tolist(imsi6c)<1)return 0;
        if(apnlist_all==0)return 0;
//printf("+++++++apn home num all:%d\n",apnlist_all);

        strcpy(apnstr,apn_trylist[apnlist_i]);
        apn_try_times++;
    }
    return 1;
}

//  for /etc/apnhistory  networkmccmnc,name,apnstr
static int add_apnhistory(char *apnstr)
{
    lastimsimccmnc[0]=0;//mark it as new search when try next time.

    char mccmnc_buff[10];
    char network_buff[32];
    char apnstr_buff[32];
    char tmpbuff[256];
    char *p;
    char *p1;
    int i,j;

    if(strlen(apnstr)<2)return -1;
    strcpy(apnstr_buff,apnstr);

    send_status_cmd();

    strcpy(tmpbuff,get_single_status_value("mccmnc"));
    if(strlen(tmpbuff)<10) return -1;
    p=tmpbuff;
    while(*p!=',' && *p!=0 )p++;
    if(*p!=',')return -1;
    p++;
    while(*p!=',' && *p!=0 )p++;
    if(*p!=',')return -1;
    p++;
    while(*p==' ')p++; 
    if(*p<'0' || *p>'9')return -1;
    p1=p;
    while(*p1!=',' && *p1!=' ' && *p1!=0)p1++;
    i=p1-p;
    if(i<4 || i>6)return -1;
    strncpy(mccmnc_buff,p,i);
    mccmnc_buff[i]=0;

    strcpy(tmpbuff,get_single_status_value("network"));
    if(strlen(tmpbuff)<3) return -1;
    p=tmpbuff;
    while(*p!=',' && *p!=0 )p++;
    if(*p!=',')
    {
        strcpy(network_buff,tmpbuff);
    }else
    {
        strcpy(network_buff,"NONAME");
        p++;
        while(*p!=',' && *p!=0 )p++;
        if(*p==',')
        {
            p++;
            while(*p==' ')p++; 
            if(*p!=',' && *p!=0 )
            {
                p1=p;
                while(*p1!=',' && *p1!=' ' && *p1!=0)p1++;
                i=p1-p;
                if(i>30)i=30;
                strncpy(network_buff,p,i);
                network_buff[i]=0;
            }
        }
    }

    //search previous same record
    int need_add=1;
    int need_resort=0;
    FILE *f;
    if (!(f = fopen("/etc/apnhistory", "r"))) 
    {
        ;
    }else
    {
        i=strlen(mccmnc_buff);
        while (fgets(tmpbuff, 256, f)) 
        {
            j=0;
            while(j<i)
            {
                if(tmpbuff[j]!=mccmnc_buff[j])break;
                j++;
            }
            if(j==i)
            {
                need_resort=1;
                p=NULL;
                p=strstr(tmpbuff,apnstr_buff);
                if(p!=NULL)
                {
                    need_add=0;
                    break;
                }
            }
        }
        fclose(f);
    }


    //add new record. at head? with "fopen r+"
    sprintf(tmpbuff,"%s,%s,%s\n",mccmnc_buff,network_buff,apnstr_buff);
    if(need_add==1 && need_resort==0)
    {
        f=NULL;
        if (!(f = fopen("/etc/apnhistory", "a"))) 
        {
            return -1;
        }else
        {
            fputs(tmpbuff,f);
            fclose(f);
        }
    }

    if(need_add==1 && need_resort==1)
    {
        char allbuff[4096];
        sprintf(allbuff,"%s,%s,%s\n",mccmnc_buff,network_buff,apnstr_buff);
        f=NULL;
        if (!(f = fopen("/etc/apnhistory", "r"))) 
        {
            ;
        }else
        {
            i=strlen(mccmnc_buff);
            while (fgets(tmpbuff, 256, f)) 
            {
                strcat(allbuff,tmpbuff);
            }
            fclose(f);
        }

        if (!(f = fopen("/etc/apnhistory", "w"))) 
        {
            return -1;
        }else
        {
            fputs(allbuff,f);
            fclose(f);
        }
    }


    sprintf(tmpbuff,"%s %s %s %s*\n",apnstr_buff,mccmnc_buff,network_buff,lastimsimccmnc);
    f=NULL;
    if (!(f = fopen("/var/run/autoapn", "w"))) 
    {
        return -1;
    }else
    {
        fputs(tmpbuff,f);
        fclose(f);
    }

    need_save_autoapn_sign=0;
    return 1;
}



