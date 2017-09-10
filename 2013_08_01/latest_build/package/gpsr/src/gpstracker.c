#include "gpstracker.h"

int send_data_len=0;
char send_data_buff[1024];
char recv_data_buff[1024];
char last_pos_latlng[50];
char last_pos_alt[20];
char iostatus_buff[120];
int recv_data_len=0;
    int readbytes = 0;
    int sock = 0;
    ssize_t wrote = 0;
    int fd_tcp=0;
    char buf[2048];
    char cbuf[10];


//config data
char c_TRACKER_Status,c_SERVER_Mode,c_TCP_ALIVE_Mode,c_SERVER_PhoneFilt_En,c_Motion_Set,c_SERVER_Setup_Status,c_SERVER_Conn_StartTr_Status,c_SERVER_Conn_TCP_Status,c_SERVER_Conn_MotionSet,c_SERVER_IOStatus,c_Last_Position;
char s_SERVER_Phone1[20],s_SERVER_Phone2[20],s_SERVER_Phone3[20],s_SERVER_Address[50],s_SERVER_Conn_Phone[20],s_SERVER_Conn_Domain[30],s_SERVER_Conn_STime[50];
char s_TRACKER_IMEI[20],s_TRACKER_Type[20],s_TRACKER_Brand[30],s_TRACKER_Version[20];
int i_TCP_ALIVE_Interval,i_Motion_Distance,i_SERVER_Port,i_SERVER_Interval,i_SERVER_Conn_Port,i_SERVER_Conn_TCPTimeFilter,i_SERVER_Conn_SMSTimeFilter;
    int tcp_conn_status=0;
char c_setup_send_status=0;
    int first_time_reset=0;
int tcp_send_no_return=0;

int gpsd_port;
char gpsd_status;

    struct gps_position basepos_old;
    struct gps_position basepos_new;

static void sleep_us(int sec, int usec)
{
    struct timeval tval;
    if (sec<0)return;
    if ((sec==0)&&(usec==0))return;
    tval.tv_sec =sec;
    tval.tv_usec=usec;
    select(0,NULL,NULL,NULL,&tval);
}

int check_carrier_imei(char *imeibuff)
{
    char tmp_buff[256];
    FILE* f;
    char *p;
    int result=-1;
    int j;
    imeibuff[0]=0;

    if (!(f = fopen("/var/run/VIP4G_status", "r"))) 
    {
        //do nothing.
    }else
    {
        while (fgets(tmp_buff, 256, f)) 
        {
            p=NULL;
            p = strstr(tmp_buff, "imei=");//imei=" 012773002002648"
            if (p != NULL)
            {
                p+=strlen("imei=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        imeibuff[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0)
                {
                    imeibuff[j]=0;
                    result=1;
                }
                break;
            }//if p
        }
    }
    if(f)fclose(f);

    return result;
}

//    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf",s_TRACKER_Type,s_TRACKER_Type);
//    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf",s_TRACKER_Brand,s_TRACKER_Brand);
//    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf",s_TRACKER_Version,s_TRACKER_Version);
int check_system_version()
{
    char tmp_buff[256];
    FILE* f;
    char *p;
    int result=-1;
    int j;
    s_TRACKER_Type[0]=0;
    s_TRACKER_Version[0]=0;
    strcpy(s_TRACKER_Brand,"Microhard Systems");

    if (!(f = fopen("/etc/version", "r"))) 
    {
        //do nothing.
    }else
    {
        while (fgets(tmp_buff, 256, f)) 
        {
            p=NULL;
            p = strstr(tmp_buff, "PRODUCT=");
            if (p != NULL)
            {
                p+=strlen("PRODUCT=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        s_TRACKER_Type[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0)
                {
                    s_TRACKER_Type[j]=0;
                    result=1;
                }
            }

            p=NULL;
            p = strstr(tmp_buff, "HARDWARE=");
            if (p != NULL)
            {
                p+=strlen("HARDWARE=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        s_TRACKER_Version[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0)
                {
                    s_TRACKER_Version[j]=0;
                    result=1;
                }
            }


        }
    }
    if(f)fclose(f);

    return result;
}


void read_config()
{
    char tmp_buff[20];

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsd","gpsd_conf","GPSD_TCP_Port",tmp_buff);
    gpsd_port=atoi(tmp_buff);
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsd","gpsd_conf","status",tmp_buff);
    gpsd_status=tmp_buff[0];

    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Phone1",s_SERVER_Phone1);
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Phone2",s_SERVER_Phone2);
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Phone3",s_SERVER_Phone3);
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Address",s_SERVER_Address);
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_Phone",s_SERVER_Conn_Phone);
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_Domain",s_SERVER_Conn_Domain);
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_STime",s_SERVER_Conn_STime);
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","TRACKER_IMEI",s_TRACKER_IMEI);
//    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf",s_TRACKER_Type,s_TRACKER_Type);
//    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf",s_TRACKER_Brand,s_TRACKER_Brand);
//    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf",s_TRACKER_Version,s_TRACKER_Version);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","TRACKER_Status",tmp_buff);
    c_TRACKER_Status=tmp_buff[0];
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Mode",tmp_buff);
    c_SERVER_Mode=tmp_buff[0];
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","TCP_ALIVE_Mode",tmp_buff);
    c_TCP_ALIVE_Mode=tmp_buff[0];
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_PhoneFilt_En",tmp_buff);
    c_SERVER_PhoneFilt_En=tmp_buff[0];
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","Motion_Set",tmp_buff);
    c_Motion_Set=tmp_buff[0];
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Setup_Status",tmp_buff);
    c_SERVER_Setup_Status=tmp_buff[0];
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_MotionSet",tmp_buff);
    c_SERVER_Conn_MotionSet=tmp_buff[0];
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_StartTr_Status",tmp_buff);
    c_SERVER_Conn_StartTr_Status=tmp_buff[0];
    tmp_buff[0]=0;    
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_IOStatus",tmp_buff);
    c_SERVER_IOStatus=tmp_buff[0];
    tmp_buff[0]=0;    
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","Last_Position",tmp_buff);
    c_Last_Position=tmp_buff[0];
    
    tmp_buff[0]=0;    
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","TCP_ALIVE_Interval",tmp_buff);
    i_TCP_ALIVE_Interval=atoi(tmp_buff);
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","Motion_Distance",tmp_buff);
    i_Motion_Distance=atoi(tmp_buff);
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Port",tmp_buff);
    i_SERVER_Port=atoi(tmp_buff);
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Interval",tmp_buff);
    i_SERVER_Interval=atoi(tmp_buff);
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_Port",tmp_buff);
    i_SERVER_Conn_Port=atoi(tmp_buff);
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_TCPTimeFilter",tmp_buff);
    i_SERVER_Conn_TCPTimeFilter=atoi(tmp_buff);
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_SMSTimeFilter",tmp_buff);
    i_SERVER_Conn_SMSTimeFilter=atoi(tmp_buff);

    last_pos_latlng[0]=0;
    last_pos_alt[0]=0;
}


/*
 * connect to gpsd with socket
 */
static int netlib_connectsock(const char *host, const char *service, const char *protocol)
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

int get_iostatus_buff()
{
    int length=0;
    char tmp_buff[50];
    FILE* f;
    char *p;
    iostatus_buff[0]=0;
    //printf("----io check:%c--",c_SERVER_IOStatus);
    //  /sys/class/button/INPUT1/status
    if(c_SERVER_IOStatus!='B' && c_SERVER_IOStatus!='C' && c_SERVER_IOStatus!='D')
    {
        return 0;
    }
    if(c_SERVER_IOStatus=='B' || c_SERVER_IOStatus=='D') //input
    {
        if ((f = fopen("/sys/class/button/INPUT1/status", "r"))!=NULL) 
        {
            if(fgets(tmp_buff, 10, f))
            {
                sprintf(tmp_buff,",Button1=%c",tmp_buff[0]);
                strcat(iostatus_buff,tmp_buff);
            }
            fclose(f);
        }
        if ((f = fopen("/sys/class/button/INPUT2/status", "r"))!=NULL) 
        {
            if(fgets(tmp_buff, 10, f))
            {
                sprintf(tmp_buff,",Button2=%c",tmp_buff[0]);
                strcat(iostatus_buff,tmp_buff);
            }
            fclose(f);
        }
        if ((f = fopen("/sys/class/button/INPUT3/status", "r"))!=NULL) 
        {
            if(fgets(tmp_buff, 10, f))
            {
                sprintf(tmp_buff,",Button3=%c",tmp_buff[0]);
                strcat(iostatus_buff,tmp_buff);
            }
            fclose(f);
        }
        if ((f = fopen("/sys/class/button/INPUT4/status", "r"))!=NULL) 
        {
            if(fgets(tmp_buff, 10, f))
            {
                sprintf(tmp_buff,",Button4=%c",tmp_buff[0]);
                strcat(iostatus_buff,tmp_buff);
            }
            fclose(f);
        }
    }
    //  /sys/class/leds/OUTPUT1/brightness
    if(c_SERVER_IOStatus=='C' || c_SERVER_IOStatus=='D') //input
    {
        if ((f = fopen("/sys/class/leds/OUTPUT1/brightness", "r"))!=NULL) 
        {
            if(fgets(tmp_buff, 10, f))
            {
                sprintf(tmp_buff,",Switch1=%c",tmp_buff[0]);
                strcat(iostatus_buff,tmp_buff);
            }
            fclose(f);
        }
        if ((f = fopen("/sys/class/leds/OUTPUT2/brightness", "r"))!=NULL) 
        {
            if(fgets(tmp_buff, 10, f))
            {
                sprintf(tmp_buff,",Switch2=%c",tmp_buff[0]);
                strcat(iostatus_buff,tmp_buff);
            }
            fclose(f);
        }
        if ((f = fopen("/sys/class/leds/OUTPUT3/brightness", "r"))!=NULL) 
        {
            if(fgets(tmp_buff, 10, f))
            {
                sprintf(tmp_buff,",Switch3=%c",tmp_buff[0]);
                strcat(iostatus_buff,tmp_buff);
            }
            fclose(f);
        }
        if ((f = fopen("/sys/class/leds/OUTPUT4/brightness", "r"))!=NULL) 
        {
            if(fgets(tmp_buff, 10, f))
            {
                sprintf(tmp_buff,",Switch4=%c",tmp_buff[0]);
                strcat(iostatus_buff,tmp_buff);
            }
            fclose(f);
        }
    }

    length=strlen(iostatus_buff);
    //printf("=====%d:%s====\n",length,iostatus_buff);
    return length;
}

//type:1-sms, 2-tcp.
//change send_data_len and send-data_buff.
int Decode_One_Command(char type, char *phoneip, char *cmdstr)
{
    //check phone if setup ok.
    //check IMEI

    char *p=cmdstr;
    char *p1;
    char tmp_buff[50];
    char tmp_c;
    int i;

    //check phone list:
    if(c_SERVER_PhoneFilt_En=='B' && type!=2)
    {
        if(strcmp(phoneip,s_SERVER_Phone1)!=0)
            if(strcmp(phoneip,s_SERVER_Phone2)!=0)
                if(strcmp(phoneip,s_SERVER_Phone3)!=0)
            {
                    return 0;
            }
    }

    //checksum,
    p1=NULL;
    p1=strchr(p,'*');
    if(p1==NULL)return 0;//checksum
    tmp_buff[0]=*(p1+1);
    tmp_buff[1]=*(p1+2);
    set_nmea_checksum(cmdstr,p1-p);
    if(tmp_buff[0]!=*(p1+1) || tmp_buff[1]!=*(p1+2))return 0;
    p+=7;
    p1=NULL;
    p1=strchr(p,',');
    if(p1==NULL)return 0;//IMEI
    strncpy(tmp_buff,p,p1-p);
    tmp_buff[p1-p]=0;
    if(strcmp(tmp_buff,s_TRACKER_IMEI)!=0)return 0;
    p=p1;
    p++;
    p1=NULL;
    p1=strchr(p,',');
    if(p1==NULL)
    {
        p1=strchr(p,'*');
        if(p1==NULL)return 0;//command
    }
    strncpy(tmp_buff,p,p1-p);
    tmp_buff[p1-p]=0;
    if(type!=2) 
    {
        if(strcmp(phoneip,s_SERVER_Conn_Phone)!=0)
        {
            if(strcmp(tmp_buff,"_GprsSettings")!=0)return 0;
        }
    }

    //_GprsSettings,_StartTracking,_StopTracking,_PollPosition,_Ping,_DeviceReset
    tmp_c=tmp_buff[3];
    if(tmp_c=='r')
    if(strcmp(tmp_buff,"_GprsSettings")==0)
    {
        char new_hostname[30];
        char new_port[10];
        int i_port;
        p=p1;
        p++;
        p1=NULL;
        p1=strchr(p,',');//inline
        if(p1==NULL)return 0;
        p=p1;
        p++;
        p1=NULL;
        p1=strchr(p,',');//apn
        if(p1==NULL)return 0;
        p=p1;
        p++;
        p1=NULL;
        p1=strchr(p,',');//apnpw
        if(p1==NULL)return 0;
        p=p1;
        p++;
        p1=NULL;
        p1=strchr(p,',');//hostname
        if(p1==NULL)return 0;
        strncpy(new_hostname,p,p1-p);
        new_hostname[p1-p]=0;
        p=p1;
        p++;
        p1=NULL;
        p1=strchr(p,'*');//port
        if(p1==NULL)return 0;
        strncpy(new_port,p,p1-p);
        new_port[p1-p]=0;
        tmp_c=0;
        if(strcmp(new_hostname,s_SERVER_Conn_Domain)!=0)
        {
            tmp_c=1;
            strcpy(s_SERVER_Conn_Domain,new_hostname);
            set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_Domain",s_SERVER_Conn_Domain);
        }
        i_port=atoi(new_port);
        if(i_port!=i_SERVER_Conn_Port)
        {
            tmp_c=1;
            i_SERVER_Conn_Port=i_port;
            set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_Port",new_port);
        }
        if(tmp_c==1)
        {
            tcp_conn_status=0;
            first_time_reset=0;
            c_SERVER_Conn_TCP_Status='A';
            set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_TCP_Status","A");
        }
        c_SERVER_Setup_Status='B';
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Setup_Status","B");
        strcpy(s_SERVER_Conn_Phone,phoneip);
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_Phone",s_SERVER_Conn_Phone);
        c_SERVER_Conn_StartTr_Status='A';
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_StartTr_Status","A");

        c_SERVER_Conn_MotionSet='0';
        i_SERVER_Conn_TCPTimeFilter=0;
        i_SERVER_Conn_SMSTimeFilter=0;
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_MotionSet","0");
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_TCPTimeFilter","0");
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_SMSTimeFilter","0");
        struct tm *ttime;
        time_t tm = time(0);
        ttime = localtime(&tm);
        sprintf(tmp_buff,"%s",asctime(ttime));
        i=strlen(tmp_buff);
        while(i>0)
        {
            i--;
            if(tmp_buff[i]=='\r' || tmp_buff[i]=='\n') {
                tmp_buff[i]=0;
            }
        }
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_STime",tmp_buff);


        p=send_data_buff+send_data_len;
        if(c_SERVER_Mode=='B')
        {
            sprintf(p,"$FRRET,%s,_GprsSettings,,%s,%s,%s,tcp*00",s_TRACKER_IMEI,s_TRACKER_Type,s_TRACKER_Brand,s_TRACKER_Version);
        }else if(c_SERVER_Mode=='C')
        {
            sprintf(p,"$FRRET,%s,_GprsSettings,,%s,%s,%s,sms*00",s_TRACKER_IMEI,s_TRACKER_Type,s_TRACKER_Brand,s_TRACKER_Version);
        }else
            sprintf(p,"$FRRET,%s,_GprsSettings,,%s,%s,%s*00",s_TRACKER_IMEI,s_TRACKER_Type,s_TRACKER_Brand,s_TRACKER_Version);

        i=strlen(p);
        set_nmea_checksum(p,(i-3));
        p[i]=0;
        strcat(p,"\r\n");
        i=strlen(p);
        send_data_len+=i;

        c_setup_send_status=1;

        return 1;
    }

    if(tmp_c=='a')
    if(strcmp(tmp_buff,"_StartTracking")==0)
    {
        char setstr[30];
        char setval[20];
        char *pe;
        p=p1;
        p++;
        p1=NULL;
        p1=strchr(p,',');//none
        if(p1==NULL)return 0;
        
        c_SERVER_Conn_MotionSet='0';
        i_SERVER_Conn_TCPTimeFilter=0;
        i_SERVER_Conn_SMSTimeFilter=0;


next_rule_check:
        p=p1;
        p++;
        p1=NULL;
        p1=strchr(p,',');//rule1
        if(p1==NULL)
        {
            p1=strchr(p,'*');
        }
        if(p1!=NULL)
        {
            strncpy(tmp_buff,p,(p1-p));
            tmp_buff[p1-p]=0;
            pe=NULL;
            pe=strchr(tmp_buff,'=');
            if(pe!=NULL)
            {
                *pe=0;
                pe++;
                strcpy(setstr,tmp_buff);
                strcpy(setval,pe);
                if(strcmp(setstr,"TimeFilter")==0)
                {
                    i_SERVER_Conn_TCPTimeFilter=atoi(setval);
                }else if(strcmp(setstr,"SmsTimeFilter")==0)
                {
                    i_SERVER_Conn_SMSTimeFilter=atoi(setval);
                }else if(strcmp(setstr,"Motion")==0)
                {
                    c_SERVER_Conn_MotionSet=setval[0];
                }
            }
        }
        if(p1!=NULL)
            if(*p1==',')goto next_rule_check;

        sprintf(tmp_buff,"%c",c_SERVER_Conn_MotionSet);
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_MotionSet",tmp_buff);
        sprintf(tmp_buff,"%d",i_SERVER_Conn_TCPTimeFilter);
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_TCPTimeFilter",tmp_buff);
        sprintf(tmp_buff,"%d",i_SERVER_Conn_SMSTimeFilter);
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_SMSTimeFilter",tmp_buff);
        c_SERVER_Conn_StartTr_Status='B';
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_StartTr_Status","B");


        p=send_data_buff+send_data_len;
        sprintf(p,"$FRRET,%s,_StartTracking*00",s_TRACKER_IMEI);
        i=strlen(p);
        set_nmea_checksum(p,(i-3));
        p[i]=0;
        strcat(p,"\r\n");
        i=strlen(p);
        send_data_len+=i;

        return 1;
    }

    if(tmp_c=='o')
    if(strcmp(tmp_buff,"_StopTracking")==0)
    {
        //$FRRET,IMEI,_StopTracking*00
        c_SERVER_Conn_StartTr_Status='A';
        strcpy(tmp_buff,"A");
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_StartTr_Status",tmp_buff);

        p=send_data_buff+send_data_len;
        sprintf(p,"$FRRET,%s,_StopTracking*00",s_TRACKER_IMEI);
        i=strlen(p);
        set_nmea_checksum(p,(i-3));
        p[i]=0;
        strcat(p,"\r\n");
        i=strlen(p);
        send_data_len+=i;


        return 1;
    }

    if(tmp_c=='l')
    if(strcmp(tmp_buff,"_PollPosition")==0)
    {

                char valid_c='0';
                char alt[10];
                char speed[10];
                char degree[10];
                char utc_time[12];
                char utc_date[10];
                strcpy(utc_time,",");
                strcpy(utc_date,",");
                strcpy(alt,",");
                strcpy(speed,",");
                strcpy(degree,",");
                strcpy(tmp_buff,",,,,");
                p=NULL;
                p=strstr(buf,"$GPGGA");
                if(p!=NULL)
                {
                    while(*p!=',')p++;//1
                    p++;
                    p1=p;
                    while(*p!=',')p++;//2
                    p++;
                    strncpy(utc_time,p1,p-p1);
                    utc_time[p-p1]=0;
                    p1=p;
                    while(*p!=',')p++;//3
                    p++;
                    while(*p!=',')p++;//4
                    p++;
                    while(*p!=',')p++;//5
                    p++;
                    while(*p!=',')p++;//6
                    p++;
                    strncpy(tmp_buff,p1,p-p1);//lat&lng: DDMM.mmmm,N,DDMM.mmm,W,
                    tmp_buff[p-p1]=0;
                    if(*p=='1' || *p=='2')valid_c='1';
                    while(*p!=',')p++;//7
                    p++;
                    while(*p!=',')p++;//8
                    p++;
                    while(*p!=',')p++;//9
                    p++;
                    p1=p;
                    while(*p!=',')p++;//5
                    p++;
                    strncpy(alt,p1,p-p1);//alt: HHH.h,
                    alt[p-p1]=0;
                }
                p=NULL;
                p=strstr(buf,"$GPRMC");
                if(p!=NULL)
                {
                    while(*p!=',')p++;//1
                    p++;
                    p1=p;
                    while(*p!=',')p++;//2
                    p++;
                    strncpy(utc_time,p1,p-p1);
                    utc_time[p-p1]=0;
                    if(*p=='A')valid_c='1';
                    while(*p!=',')p++;//3
                    p++;
                    p1=p;
                    while(*p!=',')p++;//4
                    p++;
                    while(*p!=',')p++;//5
                    p++;
                    while(*p!=',')p++;//6
                    p++;
                    while(*p!=',')p++;//7
                    p++;
                    strncpy(tmp_buff,p1,p-p1);//lat&lng: DDMM.mmmm,N,DDMM.mmm,W,
                    tmp_buff[p-p1]=0;
                    p1=p;
                    while(*p!=',')p++;//8
                    p++;
                    strncpy(speed,p1,p-p1);
                    speed[p-p1]=0;
                    p1=p;
                    while(*p!=',')p++;//9
                    p++;
                    strncpy(degree,p1,p-p1);
                    degree[p-p1]=0;
                    p1=p;
                    while(*p!=',')p++;//10
                    p++;
                    strncpy(utc_date,p1,p-p1);
                    utc_date[p-p1]=0;
                }
                /*
                printf("***pos:%s\n",tmp_buff);
                printf("***alt:%s\n",alt);
                printf("***speed:%s\n",speed);
                printf("***degree:%s\n",degree);
                printf("***date:%s\n",utc_date);
                printf("***time:%s\n",utc_time);
                */
                get_iostatus_buff();
                if(c_Last_Position=='B') 
                {
                    if(valid_c=='1' && strlen(tmp_buff)>10)
                    {
                        strcpy(last_pos_latlng,tmp_buff);
                        strcpy(last_pos_alt,alt);
                    }else
                    {
                        if(strlen(tmp_buff)<10 && strlen(last_pos_latlng)>10)
                        {
                            strcpy(speed,",");
                            strcpy(degree,",");
                            valid_c='1';
                            strcpy(tmp_buff,last_pos_latlng);
                            strcpy(alt,last_pos_alt);
                        }
                    }
                }

                p=send_data_buff+send_data_len;
                sprintf(p,"$FRRET,%s,_PollPosition,,%s%s%s%s%s%s%c%s*00",s_TRACKER_IMEI,tmp_buff,alt,speed,degree,utc_date,utc_time,valid_c,iostatus_buff);
                i=strlen(p);
                set_nmea_checksum(p,(i-3));
                p[i]=0;
                strcat(p,"\r\n");
                i=strlen(p);
                send_data_len+=i;
                return 1;
    }
/*
    if(tmp_c=='n')
    if(strcmp(tmp_buff,"_Ping")==0)
    {
        //do none
        ;
    }

    if(tmp_c=='v')
    if(strcmp(tmp_buff,"_DeviceReset")==0)
    {
        //do none
        ;
    }
*/
return 0;
}

//  /var/run/tcp_send_waiting0,waiting1
//don't change send_data_len
//if size > 200K, then next file. 
void save_tcpsend_later()
{
    ;
}

//  /var/run/tcp_send_waiting
// when finished send_data_len=0 and unlink.
// if it is succed, unlink file.
void send_previous_data()
{
    ;
}

int send_data_sms()
{
    if(send_data_len<1)return 0;
    //printf("*****send sms:");

    FILE *sfp=NULL;
    if ((sfp = fopen("/var/run/trackcmdsmssend", "a+")) == 0)return -1;
    int fsize=ftell(sfp);
    if (fsize >800) 
    {
        fclose(sfp);      
        unlink("/var/run/trackcmdsmssend");
        sfp =fopen("/var/run/trackcmdsmssend","w+");
        if(sfp==NULL) 
        {
            return -1;
        }
    } 


    send_data_buff[send_data_len]=0;
    fprintf(sfp,"#%s:%s\r\n",s_SERVER_Conn_Phone,send_data_buff);
    fflush(sfp);
    fclose(sfp);
    //printf("*****send sms OK:%d\n",send_data_len);
    send_data_len=0;
    return 0;
}

int send_data_tcp(int fd_tcp)
{
    int tcp_send_err_cnt=0;
    int i=0;
    int i_tmp;

    while(i<send_data_len)
    {
      i_tmp= send(fd_tcp, send_data_buff+i, send_data_len-i,0);
      if(i_tmp<=0)
      {
            if(errno==EINTR || errno==ENOSPC || errno==EAGAIN || errno==EWOULDBLOCK)
            {
                tcp_send_err_cnt++;
                //printf("****send -1 wait:%d**\n",tcp_send_err_cnt);
            }else
            {
                //printf("****send -1 erro:%d**\n",tcp_send_err_cnt);
                return -1;
            }
      }else 
      {
          tcp_send_err_cnt=0;
          i+=i_tmp;
      }
      if(tcp_send_err_cnt>10)return -1; 
    }
    send_data_len=0;
    tcp_send_no_return++;
    return 0;
}


int main(int argc, char *argv[])
{
    char tmp_buff[257];
    char tmp_buff1[10];
    int gpsd_conn_status=0;
    int send_tcp_ok=0;
    int time_cnt=0;
    int tcp_err_cnt=0;
    int tcp_recv_err_cnt=0;
    int alive_cnt=0;
    FILE *f;
    char *p;
    char *p1;
    char *p_phone;
    double distance_m;
    int int_dis;
    int check_send_pos;
    int i;
    (void) setsid();          
    signal(SIGPIPE, SIG_IGN);

                            

    check_system_version();

re_initsys:
    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        fprintf(stderr, "Out of memory\n");
        return -1;
    }

    read_config();//gpsd, gpsgatetr

    //check config or exit.
    if(c_TRACKER_Status!='B' && c_TRACKER_Status!='C')return 0;

    if(c_SERVER_Conn_TCP_Status!='A')
    {
        c_SERVER_Conn_TCP_Status='A';
        strcpy(tmp_buff,"A");
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_TCP_Status",tmp_buff);
    }

    tmp_buff[0]=0;
    check_carrier_imei(tmp_buff);
    if(strlen(tmp_buff)<8)
    {
        uci_free_context(ctx);
        sleep(5);
        goto re_initsys;
    }
    //printf("****imei:%s****\n",tmp_buff);
    if(strcmp(tmp_buff,s_TRACKER_IMEI)!=0)
    {
        strcpy(s_TRACKER_IMEI,tmp_buff);
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","TRACKER_IMEI",s_TRACKER_IMEI);
        c_SERVER_Setup_Status='A';
        strcpy(tmp_buff,"A");
        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Setup_Status",tmp_buff);

    }


    if(gpsd_status!='1')
    {
        uci_free_context(ctx);
        sleep(5);
        goto re_initsys;
    }

    gpsd_conn_status=0;
    tcp_conn_status=0;
    time_cnt=0;
    send_data_len=0;
    tcp_err_cnt=0;
    tcp_recv_err_cnt=0;
    tcp_send_no_return=0;

    if(c_TRACKER_Status=='B')
    while(1)
    {
        alive_cnt++;
        alive_cnt++;
        time_cnt++;
        time_cnt++;

//test:
//c_SERVER_Conn_StartTr_Status='B';

        //1.gpsd connect,//1-2:get one time data
        //2.sms read and send(check tcp status 2 continues times)
        //3.setup check or loop---uci status  ->ok and first time send _reset
        //4.tcp connect-->uci status  <-config set valid
        //5.tcp send data(from sms)
        //6.tcp recv
        //
        //7.c_SERVER_Conn_StartTr_Status=='B' and time interval  
        //   or A:time interval tcp alive
        //8.tcp send.
//1
        if(gpsd_conn_status!=1)
        {
            //printf("****connect gpsd***\n");
            if(sock>0)
            {
                shutdown(sock,SHUT_RDWR);
                close(sock);
                sock=0;
            }

            gpsd_pid_check();
            sprintf(tmp_buff,"%d",gpsd_port);
            sock = netlib_connectsock( "127.0.0.1", tmp_buff, "tcp");
            if (sock < 0)
            {
                uci_free_context(ctx);
                sleep(5);
                goto re_initsys;
            }
            //get one time data
            bzero(buf,sizeof(buf));
            bzero(cbuf,sizeof(cbuf));
            strcpy(cbuf, "r=1");
            wrote = write(sock, cbuf, strlen(cbuf));
            if ((ssize_t)strlen(cbuf) != wrote) 
            {
                shutdown(sock,SHUT_RDWR);
                close(sock);
                sock=0;
                uci_free_context(ctx);
                sleep(5);
                goto re_initsys;
            }

            sleep(1);//sleep_us(1,0);
            readbytes = 0;
            readbytes = (int)read(sock, buf, sizeof(buf));
            if (readbytes >0) 
            {
                buf[readbytes]='\0';
                bzero(cbuf,sizeof(cbuf));
                strcpy(cbuf, "r=0");
                wrote = write(sock, cbuf, strlen(cbuf));
                if ((ssize_t)strlen(cbuf) != wrote) 
                {
                    shutdown(sock,SHUT_RDWR);
                    close(sock);
                    sock=0;
                    uci_free_context(ctx);
                    sleep(5);
                    goto re_initsys;
                }
                if(get_gps_pos(buf,&base_gps_pos)==0)
                {
                    basepos_old.lat = base_gps_pos.lat;
                    basepos_old.lng = base_gps_pos.lng;
                }

            }else
            {
                shutdown(sock,SHUT_RDWR);
                close(sock);
                sock=0;
                uci_free_context(ctx);
                sleep(5);
                goto re_initsys;
            }
            //OK
            gpsd_conn_status=1;
        }

//1-2:get one time data
        bzero(buf,sizeof(buf));
        bzero(cbuf,sizeof(cbuf));
        strcpy(cbuf, "r=1");
        wrote = write(sock, cbuf, strlen(cbuf));
        if ((ssize_t)strlen(cbuf) != wrote) 
        {
            shutdown(sock,SHUT_RDWR);
            close(sock);
            sock=0;
            uci_free_context(ctx);
            sleep(5);
            goto re_initsys;
        }
        sleep(1);//sleep_us(1,0);
        readbytes = 0;
        readbytes = (int)read(sock, buf, sizeof(buf));
        if (readbytes >0) 
        {
            buf[readbytes]='\0';
            bzero(cbuf,sizeof(cbuf));
            strcpy(cbuf, "r=0");
            wrote = write(sock, cbuf, strlen(cbuf));
            if ((ssize_t)strlen(cbuf) != wrote) 
            {
                shutdown(sock,SHUT_RDWR);
                close(sock);
                sock=0;
                uci_free_context(ctx);
                sleep(5);
                goto re_initsys;
            }
            if(get_gps_pos(buf,&base_gps_pos)==0)
            {
                basepos_new.lat = base_gps_pos.lat;
                basepos_new.lng = base_gps_pos.lng;
            }
        }else
        {
            shutdown(sock,SHUT_RDWR);
            close(sock);
            sock=0;
            uci_free_context(ctx);
            sleep(5);
            goto re_initsys;
        }

//2
        if (!(f = fopen("/var/run/trackcmdsmsread", "r"))) 
        {
            //do nothing.
        }else
        {
            while (fgets(tmp_buff, 256, f)) 
            {
                if(tmp_buff[0]!='#')continue;

                p=NULL;
                p = strchr(tmp_buff, ':');
                if(p==NULL)continue;

                *p=0;
                p++;
                p_phone=tmp_buff+1;
                if(*p!='$' || *(p+1)!='F' || *(p+2)!='R' || *(p+3)!='C' || *(p+4)!='M' || *(p+5)!='D' || *(p+6)!=',')continue;
                Decode_One_Command(1,p_phone,p);
            }
            fclose(f);
            unlink("/var/run/trackcmdsmsread");
        }


//3,4
        if(c_SERVER_Setup_Status!='B')
        {
            if(send_data_len>0)send_data_sms();
            sleep(3);
            continue;
        }
        if(tcp_conn_status!=1)
        {
            if(fd_tcp>0)
            {
                shutdown(fd_tcp,SHUT_RDWR);
                close(fd_tcp);
                fd_tcp=0;
            }

            sprintf(tmp_buff,"%d",i_SERVER_Conn_Port);
            //printf("*****set up tcp :%s,%s******\n",s_SERVER_Conn_Domain,tmp_buff);
            fd_tcp = netlib_connectsock( s_SERVER_Conn_Domain, tmp_buff, "tcp");
            if (fd_tcp < 0)
            {
                if(tcp_err_cnt>5)
                {
                    if(tcp_err_cnt<8)
                    {
                        tcp_err_cnt++;
                        c_SERVER_Conn_TCP_Status='A';
                        strcpy(tmp_buff,"A");
                        set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_TCP_Status",tmp_buff);
                    }
                }else
                {
                    tcp_err_cnt++;
                }
            }
            if(fd_tcp>0)
            {
                tcp_err_cnt=0;
                tcp_recv_err_cnt=0;
                tcp_send_no_return=0;
                tcp_conn_status=1;
                c_SERVER_Conn_TCP_Status='B';
                strcpy(tmp_buff,"B");
                set_option_by_section_name(ctx, "gpsgatetr","gpsgatetr_conf","SERVER_Conn_TCP_Status",tmp_buff);
                fcntl (fd_tcp, F_SETFL, fcntl(fd_tcp, F_GETFL) | O_NONBLOCK);
                unlink("/var/run/trackcmdsmssend");
                send_previous_data();
            }
        }

//5
        if(send_data_len>0)
        {
            if(tcp_conn_status!=1)send_data_sms();
            else 
            {
                if(send_data_tcp(fd_tcp)<0)
                {
                    tcp_err_cnt++;
                    //printf("****send -1:%d**\n",tcp_err_cnt);
                } else
                {
                    tcp_err_cnt=0;
                    alive_cnt=0;
                }
                if(tcp_err_cnt>10)
                {
                    tcp_conn_status=0;
                }
            }
        }

//6
        if(tcp_conn_status==1)
        {
            i=(int)recv(fd_tcp, recv_data_buff+recv_data_len, 200,0);
            if(i>0)
            {
                recv_data_len+=i;
                tcp_recv_err_cnt=0;
                tcp_send_no_return=0;
            }
            if(i==0)
            {
                tcp_recv_err_cnt++;//may be error.
                //printf("****recv0:%d**\n",tcp_recv_err_cnt);
            }
            if(i<0)
            {
                if(errno!=EINTR && errno!=ENOSPC && errno!=EAGAIN && errno!=EWOULDBLOCK)tcp_conn_status=0;
                //printf("****recv-1:%d,%d,%d,%d**\n",tcp_err_cnt,tcp_recv_err_cnt,tcp_send_no_return,tcp_conn_status);
            }

            if(tcp_recv_err_cnt>10)tcp_conn_status=0;
            if(tcp_send_no_return>5)tcp_conn_status=0;

            if(recv_data_len>15)
            {
                recv_data_buff[recv_data_len]=0;
                p=recv_data_buff;
                p1=NULL;
                p1=strstr(p,"$FRCMD");
                i=-1;
                while(p1!=NULL)
                {
                    i=Decode_One_Command(2,"tcp",p1);
                    p=p1;
                    p1=NULL;
                    p1=strstr(p+1,"$FRCMD");
                    //if(i>0)printf("*****decode send:%s",send_data_buff);
                }
                if(i>0)recv_data_len=0;
                else
                {
                    recv_data_len=recv_data_len-(p-recv_data_buff);
                    for(i=0;i<recv_data_len;i++)
                    {
                        recv_data_buff[i]=*(p+i);
                    }
                }
                if(recv_data_len>200)recv_data_len=0;
            }
        }

//4-2
        if(first_time_reset==0)
        {
            first_time_reset=1;
            sprintf(tmp_buff,"$FRCMD,%s,_DeviceReset*00",s_TRACKER_IMEI);
            i=strlen(tmp_buff);
            set_nmea_checksum(tmp_buff,(i-3));
            tmp_buff[i]=0;
            strcat(tmp_buff,"\r\n");
            i=strlen(tmp_buff);
            strcat(send_data_buff+send_data_len,tmp_buff);
            send_data_len+=i;
            c_setup_send_status=1;
        }

//7.c_SERVER_Conn_StartTr_Status=='B' and time interval  
        //printf("*****timecnt:%d,%d**\n",time_cnt,i_SERVER_Conn_SMSTimeFilter);
        if(c_SERVER_Conn_StartTr_Status=='B' || c_setup_send_status==1)
        {
            check_send_pos=0;
            if(c_SERVER_Conn_StartTr_Status=='B') 
            {
                if(tcp_conn_status==1 && time_cnt>=i_SERVER_Conn_TCPTimeFilter && i_SERVER_Conn_TCPTimeFilter>0)check_send_pos=1;
                if(tcp_conn_status!=1 && time_cnt>=i_SERVER_Conn_SMSTimeFilter && i_SERVER_Conn_SMSTimeFilter>0)
                {
                    //printf("*****time **sms:%d,%d**\n",time_cnt,i_SERVER_Conn_SMSTimeFilter);
                    check_send_pos=1;
                }
                if(check_send_pos!=1 && c_SERVER_Conn_MotionSet=='1')
                {
                    distance_m = compute_distance(&basepos_new,&basepos_old);
                    if(distance_m<0)distance_m=-1*distance_m;
                    int_dis=(int)distance_m;
                    if(int_dis>i_Motion_Distance)check_send_pos=1;
                }
            }
            if(check_send_pos==1 || c_setup_send_status==1)
            {
                c_setup_send_status=0;
                time_cnt=0;
                basepos_old.lat=basepos_new.lat;
                basepos_old.lng=basepos_new.lng;

                //$GPGGA,221654.00,5108.54654,N,11404.52720,W,2,06,2.82,1128.0,M,-17.5,M,,*59
                //$GPRMC,170549.00,A,5108.56540,N,11404.51207,W,0.404,93.02,051212,,,D*45
                char valid_c='0';
                char alt[10];
                char speed[10];
                char degree[10];
                char utc_time[12];
                char utc_date[10];
                strcpy(utc_time,",");
                strcpy(utc_date,",");
                strcpy(alt,",");
                strcpy(speed,",");
                strcpy(degree,",");
                strcpy(tmp_buff,",,,,");
                p=NULL;
                p=strstr(buf,"$GPGGA");
                if(p!=NULL)
                {
                    while(*p!=',')p++;//1
                    p++;
                    p1=p;
                    while(*p!=',')p++;//2
                    p++;
                    strncpy(utc_time,p1,p-p1);
                    utc_time[p-p1]=0;
                    p1=p;
                    while(*p!=',')p++;//3
                    p++;
                    while(*p!=',')p++;//4
                    p++;
                    while(*p!=',')p++;//5
                    p++;
                    while(*p!=',')p++;//6
                    p++;
                    strncpy(tmp_buff,p1,p-p1);//lat&lng: DDMM.mmmm,N,DDMM.mmm,W,
                    tmp_buff[p-p1]=0;
                    if(*p=='1' || *p=='2')valid_c='1';
                    while(*p!=',')p++;//7
                    p++;
                    while(*p!=',')p++;//8
                    p++;
                    while(*p!=',')p++;//9
                    p++;
                    p1=p;
                    while(*p!=',')p++;//5
                    p++;
                    strncpy(alt,p1,p-p1);//alt: HHH.h,
                    alt[p-p1]=0;
                }
                p=NULL;
                p=strstr(buf,"$GPRMC");
                if(p!=NULL)
                {
                    while(*p!=',')p++;//1
                    p++;
                    p1=p;
                    while(*p!=',')p++;//2
                    p++;
                    strncpy(utc_time,p1,p-p1);
                    utc_time[p-p1]=0;
                    if(*p=='A')valid_c='1';
                    while(*p!=',')p++;//3
                    p++;
                    p1=p;
                    while(*p!=',')p++;//4
                    p++;
                    while(*p!=',')p++;//5
                    p++;
                    while(*p!=',')p++;//6
                    p++;
                    while(*p!=',')p++;//7
                    p++;
                    strncpy(tmp_buff,p1,p-p1);//lat&lng: DDMM.mmmm,N,DDMM.mmm,W,
                    tmp_buff[p-p1]=0;
                    p1=p;
                    while(*p!=',')p++;//8
                    p++;
                    strncpy(speed,p1,p-p1);
                    speed[p-p1]=0;
                    p1=p;
                    while(*p!=',')p++;//9
                    p++;
                    strncpy(degree,p1,p-p1);
                    degree[p-p1]=0;
                    p1=p;
                    while(*p!=',')p++;//10
                    p++;
                    strncpy(utc_date,p1,p-p1);
                    utc_date[p-p1]=0;
                }
                /*
                printf("***pos:%s\n",tmp_buff);
                printf("***alt:%s\n",alt);
                printf("***speed:%s\n",speed);
                printf("***degree:%s\n",degree);
                printf("***date:%s\n",utc_date);
                printf("***time:%s\n",utc_time);
                */
                get_iostatus_buff();
                if(c_Last_Position=='B') 
                {
                    if(valid_c=='1' && strlen(tmp_buff)>10)
                    {
                        strcpy(last_pos_latlng,tmp_buff);
                        strcpy(last_pos_alt,alt);
                    }else
                    {
                        if(strlen(tmp_buff)<10 && strlen(last_pos_latlng)>10)
                        {
                            strcpy(speed,",");
                            strcpy(degree,",");
                            valid_c='1';
                            strcpy(tmp_buff,last_pos_latlng);
                            strcpy(alt,last_pos_alt);
                        }
                    }
                }

                p=send_data_buff+send_data_len;
                sprintf(p,"$FRCMD,%s,_SendMessage,,%s%s%s%s%s%s%c%s*00",s_TRACKER_IMEI,tmp_buff,alt,speed,degree,utc_date,utc_time,valid_c,iostatus_buff);
                i=strlen(p);
                set_nmea_checksum(p,(i-3));
                p[i]=0;
                strcat(p,"\r\n");
                i=strlen(p);
                send_data_len+=i;
                check_send_pos=0;
            }
        }

//   or A:tcp alive time interval 
        if(send_data_len>0)alive_cnt=0;
        if(tcp_conn_status==1 && alive_cnt>i_TCP_ALIVE_Interval)
        {
            //pack _PING
            alive_cnt=0;
            sprintf(tmp_buff,"$FRCMD,%s,_Ping*00",s_TRACKER_IMEI);
            i=strlen(tmp_buff);
            set_nmea_checksum(tmp_buff,(i-3));
            tmp_buff[i]=0;
            strcat(tmp_buff,"\r\n");
            i=strlen(tmp_buff);
            strcpy(send_data_buff,tmp_buff);
            send_data_len=i;
        }


//8.only tcp send.
        if(send_data_len>0)
        {
            if(tcp_conn_status!=1 && i_SERVER_Conn_SMSTimeFilter>0)
            {
                save_tcpsend_later();
                send_data_sms();
            }
            else 
            {
                if(send_data_tcp(fd_tcp)<0)
                {
                    save_tcpsend_later();
                    send_data_len=0;
                    tcp_err_cnt++;
                } else tcp_err_cnt=0;
                if(tcp_err_cnt>10)
                {
                    tcp_conn_status=0;
                    continue;
                }
                if(send_data_len>400)//exception avoid dump
                {
                    send_data_len=0;
                }
            }
        }

        sleep(1);
    }
    



    if(c_TRACKER_Status=='C')
    while(1)
    {
        time_cnt++;
        time_cnt++;


        if(tcp_conn_status!=1)
        {
            if(fd_tcp>0)
            {
                shutdown(fd_tcp,SHUT_RDWR);
                close(fd_tcp);
                fd_tcp=0;
            }

            sprintf(tmp_buff,"%d",i_SERVER_Port);
            fd_tcp = netlib_connectsock( s_SERVER_Address, tmp_buff, "tcp");
            if (fd_tcp < 0)
            {
                uci_free_context(ctx);
                sleep(5);
                goto re_initsys;
            }
            tcp_conn_status=1;
            tcp_err_cnt=0;
            tcp_send_no_return=0;
            tcp_recv_err_cnt=0;
            send_previous_data();
            fcntl (fd_tcp, F_SETFL, fcntl(fd_tcp, F_GETFL) | O_NONBLOCK);
        }

        if(gpsd_conn_status!=1)
        {
            if(sock>0)
            {
                shutdown(sock,SHUT_RDWR);
                close(sock);
                sock=0;
            }

            gpsd_pid_check();
            sprintf(tmp_buff,"%d",gpsd_port);
            sock = netlib_connectsock( "127.0.0.1", tmp_buff, "tcp");
            if (sock < 0)
            {
                uci_free_context(ctx);
                sleep(5);
                goto re_initsys;
            }
            //get one time data
            bzero(buf,sizeof(buf));
            bzero(cbuf,sizeof(cbuf));
            strcpy(cbuf, "r=1");
            wrote = write(sock, cbuf, strlen(cbuf));
            if ((ssize_t)strlen(cbuf) != wrote) 
            {
                shutdown(sock,SHUT_RDWR);
                close(sock);
                sock=0;
                uci_free_context(ctx);
                sleep(5);
                goto re_initsys;
            }

            sleep(1);//sleep_us(1,0);
            readbytes = 0;
            readbytes = (int)read(sock, buf, sizeof(buf));
            if (readbytes >0) 
            {
                buf[readbytes]='\0';
                bzero(cbuf,sizeof(cbuf));
                strcpy(cbuf, "r=0");
                wrote = write(sock, cbuf, strlen(cbuf));
                if ((ssize_t)strlen(cbuf) != wrote) 
                {
                    shutdown(sock,SHUT_RDWR);
                    close(sock);
                    sock=0;
                    uci_free_context(ctx);
                    sleep(5);
                    goto re_initsys;
                }
                if(get_gps_pos(buf,&base_gps_pos)==0)
                {
                    basepos_old.lat = base_gps_pos.lat;
                    basepos_old.lng = base_gps_pos.lng;
                }

            }else
            {
                shutdown(sock,SHUT_RDWR);
                close(sock);
                sock=0;
                uci_free_context(ctx);
                sleep(5);
                goto re_initsys;
            }
            //OK
            gpsd_conn_status=1;
        }


        //read data and send with interval and check
        //get one time data
        bzero(buf,sizeof(buf));
        bzero(cbuf,sizeof(cbuf));
        strcpy(cbuf, "r=1");
        wrote = write(sock, cbuf, strlen(cbuf));
        if ((ssize_t)strlen(cbuf) != wrote) 
        {
            shutdown(sock,SHUT_RDWR);
            close(sock);
            sock=0;
            uci_free_context(ctx);
            sleep(5);
            goto re_initsys;
        }
        sleep(1);//sleep_us(1,0);
        readbytes = 0;
        readbytes = (int)read(sock, buf, sizeof(buf));
        if (readbytes >0) 
        {
            buf[readbytes]='\0';
            bzero(cbuf,sizeof(cbuf));
            strcpy(cbuf, "r=0");
            wrote = write(sock, cbuf, strlen(cbuf));
            if ((ssize_t)strlen(cbuf) != wrote) 
            {
                shutdown(sock,SHUT_RDWR);
                close(sock);
                sock=0;
                uci_free_context(ctx);
                sleep(5);
                goto re_initsys;
            }
            if(get_gps_pos(buf,&base_gps_pos)==0)
            {
                basepos_new.lat = base_gps_pos.lat;
                basepos_new.lng = base_gps_pos.lng;
            }

        }else
        {
            shutdown(sock,SHUT_RDWR);
            close(sock);
            sock=0;
            uci_free_context(ctx);
            sleep(5);
            goto re_initsys;
        }

        check_send_pos=0;
        if(time_cnt>=i_SERVER_Interval)
        {
            time_cnt=0;
            check_send_pos=1;
        }

        if(c_Motion_Set=='1' && check_send_pos!=1) 
        {
            distance_m = compute_distance(&basepos_new,&basepos_old);
            if(distance_m<0)distance_m=-1*distance_m;
            int_dis=(int)distance_m;
            if(int_dis>i_Motion_Distance)check_send_pos=1;
        }

        if(check_send_pos==1)
        {                
            time_cnt=0;
            basepos_old.lat=basepos_new.lat;
            basepos_old.lng=basepos_new.lng;

            //$GPGGA,221654.00,5108.54654,N,11404.52720,W,2,06,2.82,1128.0,M,-17.5,M,,*59
            //$GPRMC,170549.00,A,5108.56540,N,11404.51207,W,0.404,93.02,051212,,,D*45
            char valid_c='0';
            char alt[10];
            char speed[10];
            char degree[10];
            char utc_time[12];
            char utc_date[10];
            strcpy(utc_time,",");
            strcpy(utc_date,",");
            strcpy(alt,",");
            strcpy(speed,",");
            strcpy(degree,",");
            strcpy(tmp_buff,",,,,");
            p=NULL;
            p=strstr(buf,"$GPGGA");
            if(p!=NULL)
            {
                while(*p!=',')p++;//1
                p++;
                p1=p;
                while(*p!=',')p++;//2
                p++;
                strncpy(utc_time,p1,p-p1);
                utc_time[p-p1]=0;
                p1=p;
                while(*p!=',')p++;//3
                p++;
                while(*p!=',')p++;//4
                p++;
                while(*p!=',')p++;//5
                p++;
                while(*p!=',')p++;//6
                p++;
                strncpy(tmp_buff,p1,p-p1);//lat&lng: DDMM.mmmm,N,DDMM.mmm,W,
                tmp_buff[p-p1]=0;
                if(*p=='1' || *p=='2')valid_c='1';
                while(*p!=',')p++;//7
                p++;
                while(*p!=',')p++;//8
                p++;
                while(*p!=',')p++;//9
                p++;
                p1=p;
                while(*p!=',')p++;//5
                p++;
                strncpy(alt,p1,p-p1);//alt: HHH.h,
                alt[p-p1]=0;
            }
            p=NULL;
            p=strstr(buf,"$GPRMC");
            if(p!=NULL)
            {
                while(*p!=',')p++;//1
                p++;
                p1=p;
                while(*p!=',')p++;//2
                p++;
                strncpy(utc_time,p1,p-p1);
                utc_time[p-p1]=0;
                if(*p=='A')valid_c='1';
                while(*p!=',')p++;//3
                p++;
                p1=p;
                while(*p!=',')p++;//4
                p++;
                while(*p!=',')p++;//5
                p++;
                while(*p!=',')p++;//6
                p++;
                while(*p!=',')p++;//7
                p++;
                strncpy(tmp_buff,p1,p-p1);//lat&lng: DDMM.mmmm,N,DDMM.mmm,W,
                tmp_buff[p-p1]=0;
                p1=p;
                while(*p!=',')p++;//8
                p++;
                strncpy(speed,p1,p-p1);
                speed[p-p1]=0;
                p1=p;
                while(*p!=',')p++;//9
                p++;
                strncpy(degree,p1,p-p1);
                degree[p-p1]=0;
                p1=p;
                while(*p!=',')p++;//10
                p++;
                strncpy(utc_date,p1,p-p1);
                utc_date[p-p1]=0;
            }
            /*
            printf("***pos:%s\n",tmp_buff);
            printf("***alt:%s\n",alt);
            printf("***speed:%s\n",speed);
            printf("***degree:%s\n",degree);
            printf("***date:%s\n",utc_date);
            printf("***time:%s\n",utc_time);
            */
            get_iostatus_buff();
            if(c_Last_Position=='B') 
            {
                if(valid_c=='1'  && strlen(tmp_buff)>10 )
                {
                    strcpy(last_pos_latlng,tmp_buff);
                    strcpy(last_pos_alt,alt);
                }else
                {
                    if(strlen(tmp_buff)<10 && strlen(last_pos_latlng)>10)
                    {
                        strcpy(speed,",");
                        strcpy(degree,",");
                        valid_c='1';
                        strcpy(tmp_buff,last_pos_latlng);
                        strcpy(alt,last_pos_alt);
                    }
                }
            }

            sprintf(send_data_buff,"$FRCMD,%s,_SendMessage,,%s%s%s%s%s%s%c%s*00",s_TRACKER_IMEI,tmp_buff,alt,speed,degree,utc_date,utc_time,valid_c,iostatus_buff);
            i=strlen(send_data_buff);
            set_nmea_checksum(send_data_buff,(i-3));
            send_data_buff[i]=0;
            strcat(send_data_buff,"\r\n");
            send_data_len=i+2;
            check_send_pos=0;
        }

        if(send_data_len>0)
        {
            i=send(fd_tcp, send_data_buff, send_data_len, 0);
            if(i==send_data_len)
            {
                //printf("****send %d OK:%s********\n",send_data_len,send_data_buff);
                send_data_len=0;
                tcp_err_cnt=0;
                tcp_send_no_return++;
            }else tcp_err_cnt++;
            if(tcp_err_cnt>20)
            {
                save_tcpsend_later();
                tcp_conn_status=0;
                tcp_err_cnt=0;
                send_data_len=0;
            }
        }


        i=(int)recv(fd_tcp, recv_data_buff, 200,0);
        if(i>0)
        {
            //printf("*****recv:%s*****,%d,%d\n",recv_data_buff,tcp_recv_err_cnt,tcp_send_no_return);
            tcp_recv_err_cnt=0;
            tcp_send_no_return=0;
        }
        if(i==0)
        {
            tcp_recv_err_cnt++;//may be error.
            //printf("*****recv error---0 to reconnect*****\n");
        }
        if(i<0)
        {
            if(errno!=EINTR && errno!=ENOSPC && errno!=EAGAIN && errno!=EWOULDBLOCK)tcp_conn_status=0;
            //printf("****recv-1:%d,%d**\n",tcp_recv_err_cnt,tcp_conn_status);
        }
        if(tcp_recv_err_cnt>20)
        {
            tcp_conn_status=0;
            //printf("*****recv error to reconnect*****\n");
        }
        if(tcp_send_no_return>20)
        {
            tcp_conn_status=0;
            //printf("*****send no return error to reconnect*****\n");
        }

        sleep(1);
    }




}
