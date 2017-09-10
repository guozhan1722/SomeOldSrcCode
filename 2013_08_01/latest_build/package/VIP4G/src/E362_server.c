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

static char	*devname="/dev/ttyUSB0";
static char	*alt_devname="/dev/ttyUSB1";
static char *confile="/etc/config/lte";
static unsigned char	ibuf[512];
static char *version = "0.0.1";
static int status_default;
static int gotdevice;
static int connect_data;

static int start_server;
static int disconnect_data;
static int mreset;
static int have_simcard_status;

static int fd;
static int fd_stat;
static int fd_portbusy;

/*
 *	Signal handling.
 */
struct sigaction	sact;

char *VIP4G_status_file ="/var/run/VIP4G_status"; 

#define MAX_NUM_STATUS 64
#define MAX_RT_BUFF_LEN 256

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

static bool send_cmd(int timeout, char *cmd, char *rtbuf);

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

    st_fd = fopen(Status_conf, "rb");

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

static int is_carrier_disable() 
{ 
    FILE * stat_file_fd;
    char buf[256];
    int retval=QUIT_SERVER;
    char *pos;
    char conf_data[512];


    if (ctx)
       {
           uci_free_context(ctx);
           ctx=NULL;
       }
       ctx = uci_alloc_context();
       if (!ctx)
       {
           fprintf(stderr, "Out of memory\n");
           return(retval);
       }

    if (get_option_by_section_name(ctx, confile, "connect", "disabled",  conf_data)==false) {
#ifdef DEBUG_4G
        syslog(LOGOPTS,"\n\nError can not get req state!\n");
#endif
        return(retval);
    }

    retval=atoi(conf_data);
#ifdef DEBUG_4G
    //syslog(LOGOPTS,"DBUG : %s get carrier enabled state from uci = %d \n",__FUNCTION__,retval);
#endif
    return retval;
}

static int is_ip_passthrough() 
{ 
    FILE * stat_file_fd;
    char buf[256];
    int retval=0;
    char *pos;
    char conf_data[512];

    if (ctx)
       {
           uci_free_context(ctx);
           ctx=NULL;
       }
       ctx = uci_alloc_context();
       if (!ctx)
       {
           fprintf(stderr, "Out of memory\n");
           return(retval);
       }

    if (get_option_by_section_name(ctx, confile, "connect", "ippassthrough",  conf_data)==false) {
#ifdef DEBUG_4G
        syslog(LOGOPTS,"\n\nError can not get ip passthough status!\n");
#endif
        return(retval);
    }

    if (strcmp(conf_data,"Ethernet")==0){
        retval=1;
    }
#ifdef DEBUG_4G
        //syslog(LOGOPTS,"\n\nget ip passthough status = %d !\n",retval);
#endif

    return retval;
}

static bool set_req_state(int stat)
{
    char buf[20];
    FILE * stat_file_fd;
    char conf_data[512];



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
        return false;
    }

    sprintf(buf, "%d",stat);
    set_option_by_section_name(ctx, confile, "connect", "req_state", buf);

#ifdef DEBUG_4G
   // syslog(LOGOPTS,"DBUG : %s set req state to uci = %s \n",__FUNCTION__,buf);
#endif
    return true;
#endif

}

static int get_req_state() { 
    FILE * stat_file_fd;
    char buf[256];
    int retval=QUIT_SERVER;
    char *pos;
    char conf_data[512];


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
           return(retval);
       }

    if (get_option_by_section_name(ctx, confile, "connect", "req_state",  conf_data)==false) {
#ifdef DEBUG_4G
        syslog(LOGOPTS,"\n\nError can not get req state!\n");
#endif
        return(retval);
    }

    retval=atoi(conf_data);
#ifdef DEBUG_4G
    //syslog(LOGOPTS,"DBUG : %s get req state from uci = %d \n",__FUNCTION__,retval);
#endif
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
    char atcmd_buff[32];
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
                    syslog(LOGOPTS,"this error is for status \n");
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
        SubProgram_Start("ifdown","wan2");
        write_status_file();
    }

    return ret_val;
}

static bool connect_status_check() {
    char rt_b[1024];
    char *rt_buff ;
    char wr_buff[512];
    char * temp_status=NULL, *status362connected=NULL, *status362disconnected=NULL;
    int i,rc;
    char *st="_CONNECTED";

    rt_buff=rt_b;

    char *cmd_chk_status="at$nwqmistatus";
    /*loop 3 times for getting status from modules*/
    for(i=0;i<3;i++) {
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
    syslog(LOGOPTS,"DBUG : %s connect status check false (3 times)\n",__FUNCTION__);
#endif
    set_cur_state(DISCONNECTED);
    set_single_status_value("connect_status","_DISCONNECTED");
    //SubProgram_Start("ifdown","wan2");
    return false;
}

static bool set_status_tmpfile(char *ibufc, char *cmd,char *keyword)
{
    char atcmd_cmd[32];
    char atcmd_buf[512];
    int rc,cmd_len,buf_len,i;
    char *pb=NULL;
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

static bool get_connect_conf_uci() {
    char conf_data[512];
    char argstr[256] = "at$nwqmiconnect=";
    char cgdcnt[256] = "At+cgdcont=1,\"IP\",\"";
    int i,rc;
    char rt_b[512];
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
    strcat(cgdcnt,conf_data);
    strcat(cgdcnt,"\"");

    //LL, novatel said that do not touch cgdcont on 1.41
    //if (send_cmd(3, cgdcnt,rt_buff)==false)
    //    return false;

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

        strcat(argstr,conf_data);
        strcat(argstr,",");
    }
    rc=strlen(argstr);
    argstr[rc-1]='\0';

#ifdef DEBUG_4G
    syslog(LOGOPTS,"DBUG : %s get uci config argstr = %s \n",__FUNCTION__,argstr);
#endif

    if (send_cmd(3, argstr,rt_buff)==false)
        return false;
    if(strstr(rt_buff,"OK")!=NULL) {
        sleep(3);
        if(connect_status_check()==false) {
            return false;
        }
        return true;
    }
    return false;
}

static bool set_connect_tech_mode()
{
    char conf_data[512];
    char argstr[256] = "AT$NWRAT=";
    int i,rc;
    char rt_b[MAX_RT_BUFF_LEN];
    char *rt_buff ;

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

    if(get_option_by_section_name(ctx, confile, "connect", "tech_mode",  conf_data)==false){
        fprintf(stderr, "can not get disabled status from uci\n");
            return(false);
        }

    if (strcmp(conf_data,"4g")==0){
        strcat(argstr,"3,2");
    } else if (strcmp(conf_data,"3g")==0){
        strcat(argstr,"2,2");
    } else if (strcmp(conf_data,"2g")==0) {
        strcat(argstr,"1,2");
    } else 
        strcat(argstr,"0,2");
    
#ifdef DEBUG_4G
       // syslog(LOGOPTS,"DBUG : %s set connect tech mode = %s \n",__FUNCTION__,argstr);
#endif
        if(send_cmd(3, argstr,rt_buff)==false)
            return false;
        return true;
}

static bool set_sim_pin()
{
    char rt_b[MAX_RT_BUFF_LEN];
    char *cmd_pin="at+cpin?";
    char conf_data[512];
    char set_simpin[256] = "at+cpin=";

    if(send_cmd(3,cmd_pin,rt_b)==false) {
        return false;
    }

    if(NULL!=strstr(rt_b,"READY")) 
        return true;
    else if(NULL!=strstr(rt_b,"SIM PIN")) { /*sim is locked*/
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

#ifdef DEBUG_4G
        //syslog(LOGOPTS,"DBUG : %s send cmd for unlock sim pin = %s \n",__FUNCTION__,set_simpin);
#endif
        if(send_cmd(3,set_simpin,rt_b)==false) {
            return false;
        }
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

static void do_hard_reset()
{
    char log_buff[254];
    sprintf(log_buff,"%s: 4G module is taking hard reset\n","mpci-4g_server");
    log_info_to_flash(log_buff);
    
    SubProgram_Start("ifconfig","eth2 down");
    SubProgram_Start("ifdown","wan2");
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
    
    SubProgram_Start("ifconfig","eth2 down");
    SubProgram_Start("ifdown","wan2");
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

static bool server()
{
    int reqstate;
    int curstate;
    char wanip[256];
    char log_buff[256];
    int need_soft_reset=0;
    int ippthrough;

    int i,j;
    while(true) { 
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
                continue ;
            }
        }

        if(need_soft_reset == 1) {
            need_soft_reset=0;
            do_soft_reset();
            continue ;
        }

        //syslog(LOGOPTS,"%s err seletc count =%d \n",__FUNCTION__,err_select_count);

        if (err_select_count > 10) {
            err_select_count=0;
            do_hard_reset();
            continue ;
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
        }
            //continue;

        if (have_simcard_status == 0) {
            break;
        }

        if(is_weak_rssi()==true) {
            syslog(LOGOPTS,"%s rssi is too low \n",__FUNCTION__);
            disconnectdata();
            break;
        }

        ippthrough=is_ip_passthrough();

        if(connect_status_check()==true){
            fetch_Local_IP_ADDR("br-wan2",wanip);
            //syslog(LOGOPTS," br-wan ipaddr = %s\n",wanip);
            if(ippthrough == 0) {
                if(strstr(wanip,"0.0.0.0")!=NULL){
                    syslog(LOGOPTS," module connected but br-wan2 ipaddr = %s need reconnect it .....\n",wanip);
                    disconnectdata();
                }
            }
        }

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
                    break;
                }
                reset_dhcp_client();
            }
            break;
        case DISCONNECT:
            //syslog(LOGOPTS," current stat is disconnect  %d\n",DISCONNECT);
            disconnectdata();
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
    SubProgram_Start("ifconfig","eth2 down");
    SubProgram_Start("ifdown","wan2");
    sleep(1);
    close(rfd);
    UnLockFile(fd_portbusy);
    sleep(1);
//    UnLockFile(fd);

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
    syslog(LOGOPTS,"DBUG : +++++++ start E363_server +++++++++++\n");
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

    init_status_set();

    if(start_server >0)
    {
        if(is_carrier_disable()==1){
            syslog(LOGOPTS,"\n\nEnd server for setting is disabled !\n");
        }else {
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
    SubProgram_Start("ifconfig","eth2 down");
    SubProgram_Start("ifdown","wan2");
    SubProgram_Start("/etc/init.d/dhcp_client","stop");
    //sleep(1);

    close(rfd);
    UnLockFile(fd_portbusy);
    //restorelocaltermios();
    UnLockFile(fd);
    //SubProgram_Start("ifup","wan");

    return true;

fail_process:
    disconnectdata();
    sleep(2);

    restoreremotetermios();
    SubProgram_Start("ifconfig","eth2 down");
    SubProgram_Start("ifdown","wan2");
    SubProgram_Start("/etc/init.d/dhcp_client","stop");
    sleep(1);

    close(rfd);
    UnLockFile(fd_portbusy);
    sleep(1);
    UnLockFile(fd);

    return false;
}


