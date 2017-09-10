/*
 * include our .h file 
 */
#include "mpci-4g.h"  
#include "mpci-com.h"
#include <syslog.h>
#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)  

#define SENDCMD_STATUS 1
#define SENDCMD_CONNECT 2
#define SENDCMD_DISCONNECT 3
#define SENDCMD_CHECKCONNECT 4
#define UCI_STATUS 0

char VIP4G_status_buff[][64]={
    "temp=Unknown",
    "ati=Unknown",
    "simid=Unknown",
    "tech=Unknown",
    "tech_avail=Unknown",
    "connect_status=Unknown",
    "connect_duration=Unknown",
    "have_simcard=Unknown",
    "rssi=Unknown",
    "imei=Unknown",
    "cell_id=Unknown",
    "lac=Unknown",
    "network=Unknown",
    "simpin=Unknown",
    "roaming=Unknown",
    "rsrp=Unknown",
    "rsrq=Unknown",
    "phonenum=Unknown",
    "sertype=Unknown",
    "freqband=Unknown",
    "channelnum=Unknown",
    "rscp=Unknown",
    "ecno=Unknown",
    "rat=Unknown",
    "pinr=Unknown",
    "gps=Unknown",
    " "
};

static bool send_cmd(int timeout, char *cmd, char *rtbuf);

static char *version = "0.0.1";
static bool have_simcard;

static int status;
static int status_default;
static int gotdevice;
static int connect_data;
static int disconnect_data;
static int mreset;

/*default device /dev/ttyUSB0*/
static char	*devname="/dev/ttyUSB0";
static char *confile="/etc/config/lte";
static unsigned char	ibuf[512];
static unsigned char	obuf[1024];
static int     atset_fd;

static char *PortBusyFlagFile="/var/run/port_busy"; 

#define MAX_NUM_STATUS 64
#define MAX_RT_BUFF_LEN 4096
static char status_from_file[MAX_NUM_STATUS][32];

static char default_status[][32] ={
    "at$cnti=0",
    "at$cnti=1",
    "at$cnti=2",
    "at$nwrssi=1",
    "at$nwrssi=2",
    "at$nwrssi=4",
    "at$nwqmistatus",
    "ati",
    "at$nwrat?",
    "at+iccid?",
    "at+cnum",
    "at+cpin?",
    "at+cereg?",
    "at+cops?",
    "at$nwcid?",
    "at$nwdegc",
    "at$nwpinr?",
    "at$nwgps?",
    " ",
};

static char nosim_status[][32] ={
    "at$nwqmistatus",
    "ati",
    "at$nwrat?",
    "at$nwcid?",
    "at$nwdegc",
    "at$nwgps?",
    " ",
};

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

static char uci_status_item[][32] ={
    "temp",
    "ati",
    "simid",
    "tech",
    "tech_avail",
    "tech_supp",
    "connect_status",
    "connect_duration",
    "rssi",
    "imei",
    "cell_id",
    "lac",
    "network", 
    "simpin",
    "roaming",
    "rsrp",
    "rsrq",
    "phonenum",
    "sertype",
    "freqband",
    "channelnum",
    "rscp",
    "ecno",
    "rat",
    "pinr",
    "gps",
    " ",
};

struct cmd_set
{
    int cmdtype;
    char *result;
    char *atcmd;
    int cmd_len;
    char *uci_iname;
    int alone_pos;
    int multi_pos;
    struct cmd_status *next;
};
/*
 *	Signal handling.
 */
struct sigaction	sact;
static int num_cmd=0;

static struct cmd_set *head=NULL;
char *VIP4G_status_file ="/var/run/VIP4G_status";

static void write_status_file(void)
{
#ifdef DEBUG_4G
    syslog(LOGOPTS,"DBUG : %s writing status to file = %s \n",__FUNCTION__,VIP4G_status_file);
#endif

    int f_int;
	FILE* f_fd;
	int i,rc;
	while (!(f_fd = fopen(VIP4G_status_file, "w+")))
		{
		//syslog(LOGOPTS,"%s %s\n",COM12_statistic_file,strerror(errno));            
		}
	for (i=0;i<64;i++)
		{
        rc=strlen(VIP4G_status_buff[i]);
        if(rc<2) break;

        fputs( VIP4G_status_buff[i],f_fd);
		fputc( '\n',f_fd);
		}
	fclose(f_fd);
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
       // if ((*p==' ')||(*p=='\"')) {
       //     continue;
       // }
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

static bool connect_status_check()
{
    char rt_b[MAX_RT_BUFF_LEN];
    char *rt_buff ;
    char wr_buff[512];
    char * temp_status;
    int i,rc;
    char *st="_CONNECTED";

    rt_buff=rt_b;

    char *cmd_chk_status="at$nwqmistatus";

    /*loop 3 times for getting status from modules*/
    for(i=0;i<3;i++) {

        memset(rt_buff,0,sizeof(rt_buff));

        if(send_cmd(3,cmd_chk_status,rt_buff)==false) {
            return false;
        }

        temp_status=strstr(rt_buff,"_CONNECTED");

        /*get connection*/
        if(temp_status!=NULL) {
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

            set_option_by_section_name(ctx, confile, "status", "connect_status", "_CONNECTED");

            return true;
        } 
    
        sleep(1);
    }
    return false;
}

static bool get_connect_conf_uci()
{
    char conf_data[512];
    char argstr[256] = "at$nwqmiconnect=";
    int i,rc;
    char rt_b[MAX_RT_BUFF_LEN];
    char *rt_buff ;

    rt_buff=rt_b;

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

    for(i=0;i<64;i++) {
        rc=strlen(connect_conf[i]);
        if(rc<2) {
            break;
        }

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

        memset(conf_data,0,sizeof(conf_data));
        if(get_option_by_section_name(ctx, confile, "connect", connect_conf[i], conf_data)==false){
            fprintf(stderr, "can not get conf from uci\n");
            return(false);
        }

        strcat(argstr,conf_data);
        strcat(argstr,",");
    }
    rc=strlen(argstr);
    argstr[rc-1]='\0';

#ifdef DEBUG_4G
    printf("DBUG : %s get uci config argstr = %s \n",__FUNCTION__,argstr);
#endif

    if(send_cmd(3, argstr,rt_buff)==false)
        return false;

    if(strstr(rt_buff,"OK")!=NULL) {
        sleep(2);
        if(connect_status_check()==false) {
            return false;
        }
        return true;
    }
    return false;
}

void init_status_uci_conf()
{
#if 0
    int i;
    int rc;

    for(i=0;i<100;i++) {

        rc=strlen(uci_status_item[i]);
        if(rc<2) {
            break;
        }

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
        set_option_by_section_name(ctx, confile, "status", uci_status_item[i],"Unknown");
    }
#endif
}

static bool setconf_uci(char *ibufc, char *cmd)
{
    char atcmd_cmd[32];
    char atcmd_buf[512];
    int rc,cmd_len,buf_len,i;
    char *pb=NULL;
    char wr_buff[512];
    char item_buff[512];
    char *temp_imei,*temp_status;

    init_status_uci_conf();

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


    buf_len=strlen(ibufc);
    strncpy(atcmd_buf,ibufc,buf_len);
    cmd_len=strlen(cmd);
    strncpy(atcmd_cmd,cmd,cmd_len);

    atcmd_cmd[cmd_len]='\0';
    atcmd_buf[buf_len]='\0';

#ifdef DEBUG_4G
    printf("DBUG : %s set cmd=%s \n",__FUNCTION__,atcmd_cmd);
    printf("DBUG : %s set ibuf=%s \n",__FUNCTION__,atcmd_buf);
#endif


    memset(wr_buff,0,sizeof(wr_buff));
    if ( strcmp(atcmd_cmd,"at$cnti=0")==0 ) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "tech", wr_buff);
       }else
           return false;
   } 
   else if (strcmp(atcmd_cmd,"at$cnti=1")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "tech_avail", wr_buff);
       } else 
           return false;
   }

   else if (strcmp(atcmd_cmd,"at$cnti=2")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "tech_supp", wr_buff);
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwqmistatus")==0) {
       printf("DBUG : %s entering status branch\n",__FUNCTION__);
       temp_status=strstr(atcmd_buf,"QMI State:");
       if(temp_status!=NULL) {
              if (true==getRightPart(temp_status,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "connect_status", wr_buff);
              }else
                  return false;
       }

       memset(wr_buff,0,sizeof(wr_buff));
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
       temp_status=strstr(atcmd_buf,"Call Duration:");
       if(temp_status!=NULL) {
              if (true==getRightPart(temp_status,wr_buff,512)) {
                  set_option_by_section_name(ctx, confile, "status", "connect_duration", wr_buff);
              }else
                  return false;
       }

   }
   else if (strcmp(atcmd_cmd,"at$nwrssi=1")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "rssi", wr_buff);
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwrssi=2")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "rsrp", wr_buff);
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwrssi=4")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "rsrq", wr_buff);
       }else
           return false;
   }

   else if (strcmp(atcmd_cmd,"ati")==0) {
       temp_imei=strstr(atcmd_buf, "IMEI:"); 
       if(temp_imei !=NULL) {
           if (true==getRightPart(temp_imei,wr_buff,512)) {
               set_option_by_section_name(ctx, confile, "status", "imei",wr_buff);
           }
       }
   }

   else if (strcmp(atcmd_cmd,"at+iccid?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "simid", wr_buff);
       }else
           return false;
   }

   else if (strcmp(atcmd_cmd,"at$nwrat?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "rat", wr_buff);
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at+cnum")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "phonenum", wr_buff);
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at+cpin?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "simpin", wr_buff);
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at+cereg?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "roaming", wr_buff);
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at+cops?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "network", wr_buff);
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwcid?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "cell_id", wr_buff);
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwdegc")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "temp", wr_buff);
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwpinr?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "pinr", wr_buff);
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwgps?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
                set_option_by_section_name(ctx, confile, "status", "gps", wr_buff);
       }else
           return false;
   }

   return true;
}

static bool set_status_tmpfile(char *ibufc, char *cmd)
{
    char atcmd_cmd[32];
    char atcmd_buf[512];
    int rc,cmd_len,buf_len,i;
    char *pb=NULL;
    char wr_buff[512];
    char item_buff[512];
    char *temp_imei,*temp_status;

    buf_len=strlen(ibufc);
    strncpy(atcmd_buf,ibufc,buf_len);
    cmd_len=strlen(cmd);
    strncpy(atcmd_cmd,cmd,cmd_len);

    atcmd_cmd[cmd_len]='\0';
    atcmd_buf[buf_len]='\0';

    memset(wr_buff,0,sizeof(wr_buff));
#ifdef DEBUG_4G
    syslog(LOGOPTS,"DBUG : %s cmd = %s setting status to buff VIP4G_status_buff \n",__FUNCTION__,atcmd_cmd);
#endif

    if ( strcmp(atcmd_cmd,"at$cnti=0")==0 ) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"tech=")) {
                   sprintf(VIP4G_status_buff[i],"tech=\"%s\"",wr_buff);
                   break;
               } 
           }

       }else
           return false;
   } 
   else if (strcmp(atcmd_cmd,"at$cnti=1")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"tech_avail=")) {
                   sprintf(VIP4G_status_buff[i],"tech_avail=\"%s\"",wr_buff);
                   break;
               }
           }

       } else 
           return false;
   }

   else if (strcmp(atcmd_cmd,"at$cnti=2")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"tech_supp=")) {
                   sprintf(VIP4G_status_buff[i],"tech_supp=\"%s\"",wr_buff);
                   break;
               }
           }
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwqmistatus")==0) {

       temp_status=strstr(atcmd_buf,"QMI State:");
       if(temp_status!=NULL) {
              if (true==getRightPart(temp_status,wr_buff,512)) {
                  for(i=0;i<64;i++) {
                      rc=strlen(VIP4G_status_buff[i]);
                      if(rc<2) {
                          break;
                      }
                      if (NULL!=strstr(VIP4G_status_buff[i],"connect_status=")) {
                          sprintf(VIP4G_status_buff[i],"connect_status=\"%s\"",wr_buff);
                          break;
                      }
                  }
              }else
                  return false;
       }

       temp_status=strstr(atcmd_buf,"Call Duration:");
       if(temp_status!=NULL) {
              if (true==getRightPart(temp_status,wr_buff,512)) {
                  for(i=0;i<64;i++) {
                      rc=strlen(VIP4G_status_buff[i]);
                      if(rc<2) {
                          break;
                      }
                      if (NULL!=strstr(VIP4G_status_buff[i],"connect_duration=")) {
                          sprintf(VIP4G_status_buff[i],"connect_duration=\"%s\"",wr_buff);
                          break;
                      }
                  }

              }else
                  return false;
       }

   }
   else if (strcmp(atcmd_cmd,"at$nwrssi=1")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"rssi=")) {
                   sprintf(VIP4G_status_buff[i],"rssi=\"%s\"",wr_buff);
                   break;
               }
           }

       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwrssi=2")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"rsrp=")) {
                   sprintf(VIP4G_status_buff[i],"rsrp=\"%s\"",wr_buff);
                   break;
               }
           }

       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwrssi=4")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"rsrq=")) {
                   sprintf(VIP4G_status_buff[i],"rsrq=\"%s\"",wr_buff);
                   break;
               }
           }

       }else
           return false;
   }

   else if (strcmp(atcmd_cmd,"ati")==0) {
       temp_imei=strstr(atcmd_buf, "IMEI:"); 
       if(temp_imei !=NULL) {
           if (true==getRightPart(temp_imei,wr_buff,512)) {
               for(i=0;i<64;i++) {
                   rc=strlen(VIP4G_status_buff[i]);
                   if(rc<2) {
                       break;
                   }
                   if (NULL!=strstr(VIP4G_status_buff[i],"imei=")) {
                       sprintf(VIP4G_status_buff[i],"imei=\"%s\"",wr_buff);
                       break;
                   }
               }
           }
       }
   }

   else if (strcmp(atcmd_cmd,"at+iccid?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"simid=")) {
                   sprintf(VIP4G_status_buff[i],"simid=\"%s\"",wr_buff);
                   break;
               }
           }
       }else
           return false;
   }

   else if (strcmp(atcmd_cmd,"at$nwrat?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"rat=")) {
                   sprintf(VIP4G_status_buff[i],"rat=\"%s\"",wr_buff);
                   break;
               }
           }
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at+cnum")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"phonenum=")) {
                   sprintf(VIP4G_status_buff[i],"phonenum=\"%s\"",wr_buff);
                   break;
               }
           }
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at+cpin?")==0) {
       if (true==getRightPart(atcmd_buf, wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"simpin=")) {
                   sprintf(VIP4G_status_buff[i],"simpin=\"%s\"",wr_buff);
                   break;
               }
           }
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at+cereg?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"roaming=")) {
                   sprintf(VIP4G_status_buff[i],"roaming=\"%s\"",wr_buff);
                   break;
               }
           }
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at+cops?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"network=")) {
                   sprintf(VIP4G_status_buff[i],"network=\"%s\"",wr_buff);
                   break;
               }
           }
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwcid?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"cell_id=")) {
                   sprintf(VIP4G_status_buff[i],"cell_id=\"%s\"",wr_buff);
                   break;
               }
           }
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwdegc")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"temp=")) {
                   sprintf(VIP4G_status_buff[i],"temp=\"%s\"",wr_buff);
                   break;
               }
           }
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwpinr?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"pinr=")) {
                   sprintf(VIP4G_status_buff[i],"pinr=\"%s\"",wr_buff);
                   break;
               }
           }
       }else
           return false;
   }
   else if (strcmp(atcmd_cmd,"at$nwgps?")==0) {
       if (true==getRightPart(atcmd_buf,wr_buff,512)) {
           for(i=0;i<64;i++) {
               rc=strlen(VIP4G_status_buff[i]);
               if(rc<2) {
                   break;
               }
               if (NULL!=strstr(VIP4G_status_buff[i],"gps=")) {
                   sprintf(VIP4G_status_buff[i],"gps=\"%s\"",wr_buff);
                   break;
               }
           }
       }else
           return false;
   }

   return true;
}

static bool send_cmd(int timeout, char *cmd, char *rtbuf)
{
    char atcmd_buff[32];
    char ret_val[20];
    char	*bp;
    fd_set  atset_fds;
    struct timeval tval1={0,0};
    int n,cmdlen;
    
    strcpy(atcmd_buff,cmd);
    cmdlen=strlen(cmd);
#ifdef DEBUG_4G
    atcmd_buff[cmdlen]='\0';
    printf("DBUG : %s before write send command %s \n",__FUNCTION__,atcmd_buff);
#endif

    atcmd_buff[cmdlen]='\r';

    if (write(rfd, atcmd_buff, strlen(atcmd_buff)+2) < 0) {
        fprintf(stderr, "ERROR: write (rfd=%d) string %s  failed, "
                "errno=%d\n", rfd,atcmd_buff, errno);
        return false;
    }

    strncpy(ret_val, "OK",2);
    ret_val[2]='\0';

#ifdef DEBUG_4G
    atcmd_buff[cmdlen]='\0';
    printf("DBUG : %s send command %s \n",__FUNCTION__,atcmd_buff);
#endif
    memset(ibuf,0,sizeof(ibuf));

    for(;;) {
        FD_ZERO(&atset_fds);
        FD_SET(rfd, &atset_fds);
        tval1.tv_usec=0;
        tval1.tv_sec=timeout;

        if (select(rfd+1,&atset_fds,NULL, NULL, &tval1) <= 0) {
        //if (select(rfd+1,&atset_fds,NULL, NULL, NULL) <= 0) {
            fprintf(stderr, "ERROR: select() failed cmd is invalidate errno=%d\n",
                    errno);
            break;
        }

        if (FD_ISSET(rfd, &atset_fds)) {
            bp = ibuf;
            if ((n = read(rfd, ibuf, sizeof(ibuf))) < 0) {
                fprintf(stderr, "ERROR: read(fd=%d) failed, "
                        "errno=%d\n", rfd, errno);
                break;
            }
            if (n == 0) {
                fprintf(stderr, "read(rfd=%d) zero.\n", rfd);
                break;
            }

            if (strstr(ibuf,"OK")!=(char*)NULL)
            {
              //  if(status_default>0||status>0) {
                    strcpy(rtbuf,ibuf);
               // }
                return true;
            }
        }
    }
    return true;
}

static bool send_status_cmd()
{
    int i,rc;
    char atcmd_buff[32];
    char rt_b[MAX_RT_BUFF_LEN];
    char *rt_buff ;

    rt_buff=rt_b;
    for(i=0;i<MAX_NUM_STATUS;i++) {
        if(status > 0) {
            rc=strlen(status_from_file[i]);
            strncpy(atcmd_buff,status_from_file[i],rc);
        } else if (status_default>0) {
            if(have_simcard==1) {
                rc=strlen(default_status[i]);
                strncpy(atcmd_buff,default_status[i],rc);
            } else {
                rc=strlen(nosim_status[i]);
                strncpy(atcmd_buff,nosim_status[i],rc);
            }
        } else
            return false;

        if(rc<2) break;

        atcmd_buff[rc]='\0';
        
#ifdef DEBUG_4G
    printf("DBUG : %s send command %s \n",__FUNCTION__,atcmd_buff);
#endif
     //   memset(rt_buff,0,sizeof(rt_buff));
        if(send_cmd(5,atcmd_buff,rt_buff)==false){
            fprintf(stderr, "ERROR: send cmd(%s) failed\n", 
                atcmd_buff);
            return false;
        }
#if UCI_STATUS

        if (setconf_uci(rt_buff,atcmd_buff)==false){
            fprintf(stderr, "ERROR: send cmd(%s) failed\n", 
                atcmd_buff);
            return false;
        }
#else
        if (set_status_tmpfile(rt_buff,atcmd_buff)==false){
            fprintf(stderr, "ERROR: send cmd(%s) failed\n", 
                atcmd_buff);
            return false;
        }
#endif
    }
    write_status_file();
    return true;
}

static bool get_atcmd_from_user(FILE *at_file)
{
    int rc,n,i;
    char atcmd_buff[32];

	if (at_file != NULL) {
        if ((atset_fd = fopen(at_file,"r")) < 0) {
            fprintf(stderr, "ERROR: open(%s) failed, errno=%d\n", 
                at_file, errno);
            return false;
        }

        i=0;
        memset(status_from_file,0,sizeof(status_from_file));
        while (fgets(status_from_file[i],32,atset_fd)) {
            if ((strlen(status_from_file[i])>2)&&(status_from_file[i][0]!='#')) {
                rc=strlen(status_from_file[i]);
                status_from_file[i][rc-1]='\0';
                i++;
            }
        }
        fclose(atset_fd);
    } 
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

        if(get_option_by_section_name(ctx, confile, "connect", "simcardpin",  conf_data)==false){
            fprintf(stderr, "can not get simcardpin status from uci\n");
            return(false);
        }

#ifdef DEBUG_4G
        printf("DBUG : %s get sim pin from uci file = %s \n",__FUNCTION__,conf_data);
#endif

        if(strlen(conf_data)<1) {
            fprintf(stderr, "simcardpin is empty\n");
            return false;
        }

        strcat(set_simpin,conf_data);
        
        memset(rt_b,0,sizeof(rt_b));

#ifdef DEBUG_4G
        printf("DBUG : %s send cmd for unlock sim pin = %s \n",__FUNCTION__,set_simpin);
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
    int i;

    char *cmd_disconnect="at$nwqmidisconnect";

    if(send_cmd(3,cmd_disconnect,rt_b)==false) {
        return false;
    }
#ifdef DEBUG_4G
    printf("DBUG : %s get returen = %s \n",__FUNCTION__,rt_b);
#endif

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

    set_option_by_section_name(ctx, confile, "status", "connect_status", "_DISCONNECTED");

    return true;
}

static bool connectdata()
{
    disconnectdata();
    sleep(2);
    if(set_sim_pin()==false) 
        return false;
    if(get_connect_conf_uci()==false)
        return false;
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

void sighandler(int signal)
{
    printf("\n\nGot signal %d!\n", signal);
    printf("Cleaning up...");
    restorelocaltermios();
    restoreremotetermios();
    printf("Done\n");
	close(rfd);
	exit(1);
}

void usage(FILE *fp, int rc)
{
    fprintf(fp, "Usage: mpci-4g [-?h][-v] [-s speed] [-r atset_file] [-u] [-l device]\n\n"
        "\t-h?\tthis help\n"
        "\t-s\tbaud rate (default 115200)\n"
        "\t-l\tdevice to use\n"
        "\t-v\tshow version information\n"
        "\t-r\tget 4g module status from atset_file and set them to uci file /etc/config/lte\n"
        "\t-u\tget 4g module status from default status and set them to uci file /etc/config/lte\n"
        "\t-c\tStart a package data session\n"
        "\t-d\tStop a package data session\n"
        "\t-n\tReset modules\n");
    exit(rc);
}

static bool check_sim_card()
{

#if 0
    char rt_b[MAX_RT_BUFF_LEN];
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
        return(false);
    }

    if(get_option_by_section_name(ctx, confile, "status", "have_simcard",  conf_data)==false){
        fprintf(stderr, "can not get sim card status from uci\n");
        return(false);
    }

#ifdef DEBUG_4G
        syslog(LOGOPTS,"DBUG : %s get have_sim pin from uci file = %s \n",__FUNCTION__,conf_data);
#endif

        if(strcmp(conf_data,"0")==0) {
            fprintf(stderr, "simcard is removed\n");
            return false;
        } else if (strcmp(conf_data,"1")==0) {
#ifdef DEBUG_4G
            syslog(LOGOPTS,"DBUG : %s sim card is insert\n",__FUNCTION__);
#endif
            return true;
        } else {
#ifdef DEBUG_4G
            syslog(LOGOPTS,"DBUG : %s sim card Unknow\n",__FUNCTION__);
#endif
             return false;
        }
    return false;
#else
    int simfd;
    int i,rc;
    char rt[2];

    if((simfd = fopen("/sys/class/button/SIM_STAT/status","r")) < 0) {
        fprintf(stderr, "ERROR: open(%s) failed, errno=%d\n", 
            "simfd", errno);
        return false;
}

    memset(rt,0,sizeof(rt));
    fgets(rt,2,simfd);
    fclose(simfd);

    rt[1]='\0';
#ifdef DEBUG_4G
    syslog(LOGOPTS,"\n\ngot sim card status is  %s!\n", rt);
#endif

#if UCI_STATUS
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

    if( strcmp(rt,"0")==0) {
        set_option_by_section_name(ctx, confile, "status", "have_simcard", "1");
        return true;
    }else {
        set_option_by_section_name(ctx, confile, "status", "have_simcard", "0");
        return false;
    }
#else
    for(i=0;i<64;i++){
        rc=strlen(VIP4G_status_buff[i]);
        if(rc<2) {
            break;
        }
        if (NULL!=strstr(VIP4G_status_buff[i],"have_simcard=")) {
            if ( strcmp(rt,"0")==0) {
                sprintf(VIP4G_status_buff[i],"have_simcard=1");
                have_simcard=1;
                write_status_file();
                return true;
            }else {
                sprintf(VIP4G_status_buff[i],"have_simcard=0");
                have_simcard=0;
                write_status_file();
                return false;
            }
        }
    }

#endif
    return false;    
#endif
}


int main(int argc, char *argv[])
{
	int	c;
    FILE *atset_file=NULL;
    FILE *f;
    int fd;

    /*create the busy file*/
    f=fopen( PortBusyFlagFile,"w+");
    if (f==NULL) {
        return false;
    }
    fclose(f);     

    have_simcard=check_sim_card();
	gotdevice = 0;

    while ((c = getopt(argc, argv, "?hvcdnur:s:l:")) > 0) {
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
		case 'r':
			status++;
            atset_file = optarg;
			break;
		case 'v':
			printf("%s: version %s\n", argv[0], version);
			exit(true);
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

    if (optind < argc) {
		fprintf(stderr, "ERROR: too many arguments\n");
		usage(stderr, false);
	}

    /*lock file here */
    fd = open (PortBusyFlagFile, O_WRONLY);
    if (fd<0) {
        fprintf(stderr,"%s:fd<0\n",__func__);
        return false;
    }
    LockFile(fd);    

    if ((rfd = open(devname, (O_RDWR | O_NDELAY))) < 0) {
		fprintf(stderr, "ERROR: failed to open %s, errno=%d\n",
			devname, errno);

        UnLockFile(fd);
		return(false);
    }


    (void) setsid();

    //savelocaltermios();
	//setlocaltermios();

    saveremotetermios();
    setremotetermios();

   	/*
	 *	Set the signal handler to restore the old termios .
	 */
	sact.sa_handler = sighandler;
	sigaction(SIGHUP, &sact, NULL);
	sigaction(SIGINT, &sact, NULL);
	sigaction(SIGQUIT, &sact, NULL);
	sigaction(SIGPIPE, &sact, NULL);
	sigaction(SIGTERM, &sact, NULL);

    if (get_atcmd_from_user(atset_file)==false) {
        goto fail_process;
    }

    if(status>0 ||status_default>0) {
        if(send_status_cmd()==false)
            goto fail_process;
    }

    if(connect_data>0) {
        if (connectdata()==false)
            goto fail_process;
    }

    if(disconnect_data>0) {
        if (disconnectdata()==false)
            goto fail_process;
    }

    if(mreset>0) {
        if (mresetmodule()==false)
            goto fail_process;
    }
    //loopit();

    restoreremotetermios();
    //restorelocaltermios();

    close(rfd);
    UnLockFile(fd);
    return true;

fail_process:

    restoreremotetermios();
    //restorelocaltermios();

    close(rfd);
    UnLockFile(fd);

    return false;
}


