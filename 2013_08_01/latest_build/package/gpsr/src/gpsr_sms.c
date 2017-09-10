/*
 * gpsreporting
 *
 * a simple program to connect to a gpsd daemon and dump the received data
 * to remote server
 *
 */


#include "gpsrlib.h"
#include "gpsr.h"

#if !defined (INADDR_NONE)
    #define INADDR_NONE   ((in_addr_t)-1)
#endif

struct schedule_source ss_buf[REPORT_NUMBER];
struct socket_source sockbuf[REPORT_NUMBER];
int enabled_report[REPORT_NUMBER];

int item_no;
char port_str[10];

static bool read_devicename()
{
    char proname[BUFF_LEN];
    //host name may be changed online.
    strcpy(proname,"");
    if(get_option_by_section_name(ctx, "system", "@system[0]", "hostname", proname))
    {
        strcpy(DataBase2_Buff[SUB_MENU_SYSTEM_CFG][SYSTEM_CFG_ITEM_DEVICE_NAME],proname);
    }
    else
    {
        return false;
    }

    return true;
}

static bool read_configdata()
{
    int j;
    //gpsr config
    for(j=0;j<ADVANCED_GPS_CONF_REPORT_NUM;j++)
    {
        if(get_option_by_section_name(ctx, "gpsr","gpsr_conf",Item_Keywords[ADVANCED_GPS_CONF_REPORT][j], DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][j])==false)
        {
            //printf("#%s:%s:%d#\n",Item_Keywords[ADVANCED_GPS_CONF_REPORT][j],DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][j],j);            
            return false;
        }
    }


    //smtp config
    for(j=0;j<GPS_SUB_MENU_SMTP_NUM;j++)
    {
        if(get_option_by_section_name(ctx, "gpsr","gpsr_conf",Item_Keywords[GPS_SUB_MENU_SMTP][j], DataBase1_Buff[GPS_SUB_MENU_SMTP][j])==false)
        {
            //printf("#%s:%d#\n",Item_Keywords[GPS_SUB_MENU_SMTP][j],j);            
            return false;
        }
    }

    //for sms
    for(j=0;j<4;j++) 
    {
        if(j==item_no)
        {
            strcpy(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_SEC0+j],"1");
            DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_TRIGGER_CONF][j]='A';
            DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_TIMER][j]='B'; 
        }else
        {
            DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR][j]='A';
        }
    }

    return true;

}


/* 
 * sending by email
 */
static int send_by_email(char *buf, int index)
{
    char my_ip[32]="0.0.0.0";
    char tmpbuf[64];
    FILE *sfp=NULL;
    int item=0;

    if (false==fetch_Local_IP_ADDR("br-lan",my_ip))fetch_Local_IP_ADDR("eth0",my_ip);

    sprintf(tmpbuf,"/tmp/run/gpsr%d_email.dat",index);
    if ((sfp = fopen(tmpbuf, "w")) == 0) {
        syslog(LOGOPTS,"GPSR fopen %s: %s\n", tmpbuf, strerror(errno));
        return 0;
    }

    /* 
     *  Give out Message header. 
     */
    read_devicename();
    fprintf(sfp, "From: %s@%s\r\n", DataBase2_Buff[SUB_MENU_SYSTEM_CFG][SYSTEM_CFG_ITEM_DEVICE_NAME], my_ip);
    item = SMTP_ITEM0_MESSAGE_TITLE+6*index;
    fprintf(sfp, "Subject: %s\r\n", DataBase1_Buff[GPS_SUB_MENU_SMTP][item]);

    fprintf(sfp, "Sender: %s@%s\r\n", DataBase2_Buff[SUB_MENU_SYSTEM_CFG][SYSTEM_CFG_ITEM_DEVICE_NAME], my_ip);
    item=SMTP_ITEM0_RECEIVER+6*index;
    fprintf(sfp, "To: %s\r\n", DataBase1_Buff[GPS_SUB_MENU_SMTP][item]);


    fprintf(sfp, "MIME-Version: 1.0\r\n"); 
    fprintf(sfp, "Content-Type: multipart/mixed; boundary=\"bound123\"\r\n");  
    fprintf(sfp, "--bound123\r\n");

    fprintf(sfp, "Content-Type: text/plain; charset=ISO-8859-1 \r\n");  
    fprintf(sfp, "Content-Transfer-Encoding: binary\r\n");  
    fprintf(sfp, "Content-Disposition: inline\r\n");   
    fprintf(sfp, "\r\n");           
    fprintf(sfp,buf);
    fprintf(sfp, "\r\n"); 
    fprintf(sfp, "--bound123--\r\n");    
    fflush(sfp); 
    fclose(sfp);

    //call shell to send email here
    item = SMTP_ITEM0_RECEIVER+6*index;
    sprintf(tmpbuf,"gpsrsmtp %s %d",DataBase1_Buff[GPS_SUB_MENU_SMTP][item],index);
    if (SubProgram_Start(gpsrscriptFile,tmpbuf))
        return 1;
    else {
        syslog(LOGOPTS,"GPSR report%d send email error\n", index);
        return 0;
    }
}

/*
 * select sort function
 */
static void select_sort_ssbuf(int count)
{
    int i,j,temp_sec,temp_usec, pos,temp_item;

    for (i=0;i<count-1;i++) {
        temp_sec=ss_buf[enabled_report[i]].tnex.tv_sec;
        temp_usec=ss_buf[enabled_report[i]].tnex.tv_usec;
        temp_item = enabled_report[i];
        pos=i;
        for (j=i+1;j<count;j++) {
            if ((ss_buf[enabled_report[j]].tnex.tv_sec<temp_sec)||((ss_buf[enabled_report[j]].tnex.tv_sec==temp_sec)&&(ss_buf[enabled_report[j]].tnex.tv_usec<temp_usec))) {
                temp_sec=ss_buf[enabled_report[j]].tnex.tv_sec;
                temp_usec=ss_buf[enabled_report[j]].tnex.tv_usec;
                temp_item = enabled_report[j];
                pos=j;
            }

        }

        enabled_report[pos]=enabled_report[i];
        enabled_report[i]=temp_item;
    }
}

/*
 * sleep function using by select
 */
static void sleep_us(int sec, int usec)
{
    struct timeval tval;
    if (sec<0)return;
    if ((sec==0)&&(usec==0))return;
    tval.tv_sec =sec;
    tval.tv_usec=usec;
    select(0,NULL,NULL,NULL,&tval);
}

/*
 * printf struct schedule_source
 */
static void print_struct(const struct schedule_source sdata){
    printf("struct schedule_source:\n");
    printf("tval_sec=%d tval_usec=%d tnex_sec=%d tnex_usec=%d texp_sec=%d texp_usec=%d d_enable=%d t_enable=%d trigger_conf=%d rnum=%d\n"\
           ,sdata.tval.tv_sec,sdata.tval.tv_usec,sdata.tnex.tv_sec,sdata.tnex.tv_usec,sdata.texp.tv_sec,sdata.texp.tv_usec,sdata.distance_enable,\
           sdata.timer_enable,sdata.trigger_conf,sdata.rnum);

}

/*
 * init schedule_source buffer
 * we will do the restrict double check as well
 */
static int init_schedule_source(int i)
{
    int result=0;
    /*
     * read configuration from DataBase1_Buff
     */
    ss_buf[i].timer_enable=DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_TIMER][i]-'A';
    ss_buf[i].distance_enable=DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_DIS][i]-'A';
    ss_buf[i].trigger_conf=DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_TRIGGER_CONF][i]-'A';

    // for debug, will be deleted
    //printf("databasebuf: timer=%c dis=%c trigger=%c\n",DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_TIMER][i],\
				DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_DIS][i],\
				DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_TRIGGER_CONF][i]);
    // end of debug

    /* the init state is not be scheduled */
    ss_buf[i].scheduled = NO;

    ss_buf[i].rnum=i;

    if (ss_buf[i].distance_enable==ENABLE) {
        /*
         * distance condition enable and timer enable as well
         */
        //result = POSRECORD;
        if (ss_buf[i].timer_enable==ENABLE) {
            /* if trigger condition is AND, time interval equals setting of cxs */
            if (ss_buf[i].trigger_conf==ANDCONF) {
                ss_buf[i].tval.tv_sec=atoi(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_SEC0+i]);
                ss_buf[i].tval.tv_usec=0;
                ss_buf[i].tnex.tv_sec=ss_buf[i].tval.tv_sec;
                ss_buf[i].tnex.tv_usec=0;
                ss_buf[i].texp.tv_sec=ss_buf[i].tval.tv_sec;
                ss_buf[i].texp.tv_usec=0;
                ss_buf[i].tcxs.tv_sec=ss_buf[i].tval.tv_sec;
                ss_buf[i].tcxs.tv_usec=0;
                /* if trigger condition is OR, time interval is 1 second to check if distance condition can be triggered */
            } else if (ss_buf[i].trigger_conf==ORCONF) {
                result=ORDIS;
                ss_buf[i].tval.tv_sec=1;
                ss_buf[i].tval.tv_usec=0;
                ss_buf[i].tnex.tv_sec=ss_buf[i].tval.tv_sec;
                ss_buf[i].tnex.tv_usec=0;
                ss_buf[i].texp.tv_sec=atoi(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_SEC0+i]);
                ss_buf[i].texp.tv_usec=0;
                ss_buf[i].tcxs.tv_sec=atoi(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_SEC0+i]);
                ss_buf[i].tcxs.tv_usec=0;
            } else {
                //it should be recorded into syslog

                syslog(LOGOPTS,"GPSR init_schedule_source error: the trigger condition should be only two choices--AND/OR\n");

                return -1;
            }
        }
        /*
         * distance enable but timer disable, system will treat it same as distance disable
         */
        if (ss_buf[i].timer_enable==DISABLE) {
            ss_buf[i].distance_enable=DISABLE;
            //it should be recorded into syslog

            syslog(LOGOPTS,"GPSR init_schedule_source error: If timer disable, even cx sets distance enable, system will treat it same as distance disable\n");

            return -1;
        }
    }
    if (ss_buf[i].distance_enable==DISABLE) {
        /*
         * distance disable but timer enable, time interval equals setting of cxs
         */
        if (ss_buf[i].timer_enable==ENABLE) {
            ss_buf[i].tval.tv_sec=atoi(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_SEC0+i]);
            ss_buf[i].tval.tv_usec=0;
            ss_buf[i].tnex.tv_sec=ss_buf[i].tval.tv_sec;
            ss_buf[i].tnex.tv_usec=0;
            ss_buf[i].texp.tv_sec=ss_buf[i].tval.tv_sec;
            ss_buf[i].texp.tv_usec=0;
            ss_buf[i].tcxs.tv_sec=ss_buf[i].tval.tv_sec;
            ss_buf[i].tcxs.tv_usec=0;
        }
        /* if timer and distance both disable, but user have setup this report
         * system will treat it as report disable
         */
        if (ss_buf[i].timer_enable==DISABLE) {
            //it should be recorded into syslog

            syslog(LOGOPTS,"GPSR: init_schedule_source error: never go to here because timer and distance both disable\n");

            return -1;
        }
    }
    // for debug, it will be deleted

    //print_struct(ss_buf[i]);

    // end of debug
    return result;

}

/*
 * init socket_source buffer
 */
static void init_sockbuf(int index){
    int rport;
    /* setup UDP sock to remote server */
    /* Get the Socket file descriptor */
    if ((sockbuf[index].sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        //it should be recorded into syslog

        syslog(LOGOPTS,"GPSR: ERROR: Failed to obtain remote Socket Descriptor for report%d!\n",index);

        sockbuf[index].errorno=FAILED;
    } else
        sockbuf[index].errorno=SUCCESS;

    rport=atoi(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RPORT0+index]);
    /* Fill the socket address struct */
    sockbuf[index].addr_remote.sin_family = AF_INET;                   // Protocol Family
    sockbuf[index].addr_remote.sin_port = htons(rport);                 // Port number
    inet_pton(AF_INET, DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_REIP0+index], &(sockbuf[index].addr_remote.sin_addr)); // Net Address
    bzero(&(sockbuf[index].addr_remote.sin_zero), 8);                  // Flush the rest of struct
}

/*
 * init gpsr and return the number of enabled report
 */
static void init_gpsr(struct return_value_initgpsr *rvi)
{
    int i,pos_flag,count=0;
    read_devicename();
//    UserDB_read(UserDB,Item_Keywords[SUB_MENU_SYSTEM_CFG][SYSTEM_CFG_ITEM_DEVICE_NAME],DataBase1_Buff[SUB_MENU_SYSTEM_CFG][SYSTEM_CFG_ITEM_DEVICE_NAME],32);
    for (i=0;i<REPORT_NUMBER;i++) {
        if ((DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR][i]=='B')||(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR][i]=='C')) {
            /* if timer and distance both disable, but user have enabled this report
             * system will treat it as report disable
             */
            if (atoi(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_SEC0+i])<1) {
                DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_TIMER][i]='A';
                //syslog(LOGOPTS,"GPSR: report#%d sec<1, system will treat it as timer disable\n",i);
            }
            if (atoi(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_METER0+i])<1) {
                DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_DIS][i]='A';
                //syslog(LOGOPTS,"GPSR: report#%d meter<1, system will treat it as dis disable\n",i);
            }
            if ((DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_TIMER][i]=='B')&&(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_DIS][i]=='B')) {
                if (DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_TRIGGER_CONF][i]=='A') {
                    DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_TRIGGER_CONF][i]='B';
                    //syslog(LOGOPTS,"GPSR: report#%d trigger=none, system will treat it as and\n",i);
                }
            }
            if ((DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_TIMER][i]=='A')&&(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_DIS][i]=='A')) {
                syslog(LOGOPTS,"GPSR: if timer and distance both disable, but user have enabled this report, system will treat it as report%d disable\n",i);
                continue;
            }
            /*
             * double check if cx settings follow the restrict rules
             */
            pos_flag=init_schedule_source(i);
            // cx settings be refused because they didn't follow the restrict rules
            if (pos_flag<0) continue;
            if (DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR][i]=='B') init_sockbuf(i);
            if (DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR][i]=='C') {
                FILE *sfp=NULL;
                char fname[32];

                sprintf(fname,"/etc/ssmtp/gpsr%d.conf",i);
                if ((sfp = fopen(fname, "w")) == 0) {
                    syslog(LOGOPTS,"GPSR init fopen %s: %s\n", fname, strerror(errno));
                    continue;
                }
                fprintf(sfp, "hostname=%s\n", DataBase2_Buff[SUB_MENU_SYSTEM_CFG][SYSTEM_CFG_ITEM_DEVICE_NAME]);
                fprintf(sfp, "FromLineOverride=YES\n"); 
                fprintf(sfp, "mailhub=%s\n", DataBase1_Buff[GPS_SUB_MENU_SMTP][SMTP_ITEM0_HOST+6*i]);
                fprintf(sfp, "AuthUser=%s\n", DataBase1_Buff[GPS_SUB_MENU_SMTP][SMTP_ITEM0_USER_NAME+6*i]);
                fprintf(sfp, "AuthPass=%s\n", DataBase1_Buff[GPS_SUB_MENU_SMTP][SMTP_ITEM0_PASSWORD+6*i]);
                fprintf(sfp, "UseTLS=YES\n");  
                fflush(sfp); 
                fclose(sfp);
            }
            enabled_report[count]=i;
            count++;
        }

    }
    //should be added into syslog
    //syslog(LOGOPTS,"GPSR: the number of enabled report: %d\n",count);
    rvi->count = count;
    rvi->pos_flag = pos_flag;

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

static void usage(void)
{
    (void)fprintf(stderr, "Usage: gpsr [server[ port]]\n\n");
}

int main( int argc, char *argv[])
{
    // for debug, will be deleted
    int delay;
    struct timeval start, end;
    int delay1;
    struct timeval start1, end1;
    // end of debug

    char buf[2048];
    char sbuf[2048];
    char pbuf[1024];
    char cbuf[5];
    int i,count=0;
    struct timeval min_tval;
    struct return_value_initgpsr rvi;
    int try1_count=0;
    int try2_count=0;
    int try3_count=0;

    /* for gpsd */
    int sock = 0;
    ssize_t wrote = 0;

    char *port;
    char *server;
    item_no=-1;

    port=NULL;
    switch (argc) {
    case 2:{
            //port = DEFAULT_GPSD_PORT;
            server = DEFAULT_GPSD_SERVER;
            item_no = atoi(argv[1]);
            break;
        }
    case 3:{
            server = DEFAULT_GPSD_SERVER;
            port = argv[1];
            item_no = atoi(argv[2]);
            break;
        }
    case 4:{
            server = argv[1];
            port=argv[2];
            item_no = atoi(argv[3]);
            break;
        }
    default:
        usage();
        exit(1);
    }

    if(item_no>3 || item_no<0)
    {
        syslog(LOGOPTS,"GPSR exit: argv should be 0/1/2/3\n");
        exit(1);
    }


    signal(SIGPIPE, SIG_IGN);

    gps_text[60]='*';
    gps_text[61]='0';
    gps_text[62]='0';
    gps_text[63]=0;
    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        fprintf(stderr, "Out of memory\n");
        return -1;
    }

    if (read_configdata()==false) {
        syslog(LOGOPTS,"GPSR exit: read config data error\n");
        exit(1);
    }
    if (port==NULL)
    {
        port_str[0]=0;
        if(get_option_by_section_name(ctx, "gpsd", "gpsd_conf", "GPSD_TCP_Port", port_str)==false)
        {
            syslog(LOGOPTS,"GPSR exit: argv port need\n");
            exit(1);
        }
        port=port_str;
    }

    if (read_devicename()==false) {
        syslog(LOGOPTS,"GPSR exit: read devicename error\n");
        exit(1);
    }
    
    if (strcmp(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR],"AAAA")==0) {
        syslog(LOGOPTS,"GPSR exit: exit because of disable all reports\n");
        exit(1);
    }

/*
 * firstly,we have to connect gpsd with command r=1 to get the raw nmea data
 */
    reconnect:
    sock = netlib_connectsock( server, port, "tcp");
    if (sock < 0) {
        //it should be recorded into syslog
        syslog(LOGOPTS,"GPSR exit: could not connect to gpsd %s:%s\n",server, port);
        exit(1);
    }
/*
 * we will init gpsr
 */
    /* init gpsr */
    init_gpsr(&rvi);
    count = rvi.count;
    if (count==0) {
        //it should be recorded into syslog
        syslog(LOGOPTS,"GPSR exit: init_gpsr return enabled report=0\n");
        exit(1);
    }

    // send first message
    int readbytes = 0;
    bzero(buf,sizeof(buf));
    bzero(cbuf,sizeof(cbuf));
    /* ship the r=1 option to gpsd in order to start receiving gps data */
    strcpy(cbuf, "r=1");
    wrote = write(sock, cbuf, strlen(cbuf));
    if ((ssize_t)strlen(cbuf) != wrote) {
        syslog(LOGOPTS,"GPSR send first message: ship r=1: write error\n");

        shutdown(sock,SHUT_RDWR);
        close(sock);
        syslog(LOGOPTS,"GPSR: shutdown sock and reconnect\n");
        goto reconnect;

    }
    /* sleep 1sec and then to read the buffered data from gpsd in this 1sec */
    sleep_us(1,0);
    readbytes = (int)read(sock, buf, sizeof(buf));
    if (readbytes >0) {
        buf[readbytes]='\0';
        bzero(cbuf,sizeof(cbuf));
        /* ship the r=0 option to gpsd in order to stop receiving gps data */
        strcpy(cbuf, "r=0");
        wrote = write(sock, cbuf, strlen(cbuf));
        if ((ssize_t)strlen(cbuf) != wrote) {
            syslog(LOGOPTS,"GPSR send first message: ship r=0: write error\n");

            shutdown(sock,SHUT_RDWR);
            close(sock);
            syslog(LOGOPTS,"GPSR: shutdown sock and reconnect\n");
            goto reconnect;

        }

        if(get_gps_pos(buf,&base_gps_pos)==0)
        {
            for (i=0;i<count;i++) 
            {
                ss_buf[enabled_report[i]].basepos.lat = base_gps_pos.lat;
                ss_buf[enabled_report[i]].basepos.lng = base_gps_pos.lng;
            }
        }
        else
        {
            for (i=0;i<count;i++) 
            {
                ss_buf[enabled_report[i]].basepos.lat =5108.567871;
                ss_buf[enabled_report[i]].basepos.lng =-11404.513672;
            }
        }


        if (fetch_one_pattern(buf,pbuf)!=0) {
            syslog(LOGOPTS,"GPSR send first messages: fetch_one_pattern failed because gpsd can't send available data.\n");

        } else {
            for (i=0;i<count;i++) {

                bzero(sbuf,sizeof(sbuf));
                if (send_select_nmea(pbuf,DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_MSS_TYPE0+enabled_report[i]],sbuf)<0) {
                    syslog(LOGOPTS,"GPSR send first messages: report%d send_select_nmea failed.\n",enabled_report[i]);
                    continue;
                }
                if (DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR][enabled_report[i]]=='B') {

                    sockbuf[enabled_report[i]].num = sendto(sockbuf[enabled_report[i]].sockfd, sbuf, strlen(sbuf), 0, (struct sockaddr *)&(sockbuf[enabled_report[i]].addr_remote), sizeof(struct sockaddr_in));
                    if (sockbuf[enabled_report[i]].num < 0 ) {

                        syslog(LOGOPTS,"GPSR: Failed to send report to %s:%s!\n",DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_REIP0+enabled_report[i]], DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RPORT0+enabled_report[i]]);

                    }
                    syslog(LOGOPTS,"GPSR send first messages: report%d sending report to %s:%s!\n",enabled_report[i],DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_REIP0+enabled_report[i]], DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RPORT0+enabled_report[i]]);
                }
                if (DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR][enabled_report[i]]=='C') {
                    syslog(LOGOPTS,"GPSR send first messages: report%d sending by email\n",enabled_report[i]);
                    send_by_email(sbuf,enabled_report[i]);
                }
            }
        }
    }
    /* sort by next trigger time */
    select_sort_ssbuf(count);
    /* it is first time so the sleep timeval equals to min timeval in enabled reports list */
    min_tval.tv_sec=ss_buf[enabled_report[0]].tval.tv_sec;
    min_tval.tv_usec=ss_buf[enabled_report[0]].tval.tv_usec;


/*
 * check if gps data is available
 */
#if 0
    for (;;) {
        int readbytes = 0;
        bzero(buf,sizeof(buf));
        bzero(cbuf,sizeof(cbuf));
        /* ship the r=1 option to gpsd in order to start receiving gps data */
        strcpy(cbuf, "r=1");
        wrote = write(sock, cbuf, strlen(cbuf));
        if ((ssize_t)strlen(cbuf) != wrote) {
            //it should be recorded into syslog
            syslog(LOGOPTS,"GPSR exit: ship assembled options: write error\n");
            if (wrote == -1) {
                shutdown(sock,SHUT_RDWR);
                close(sock);
                syslog(LOGOPTS,"GPSR: ship assembled options: write error\n");
            } else
                exit(1);
        }
        /* sleep 1sec and then to read the buffered data from gpsd in this 1sec */
        sleep_us(1,0);
        readbytes = (int)read(sock, buf, sizeof(buf));
        //debug("read gpsd:\n%s",buf);

        /* until success read data from gpsd */
        if (readbytes == 0 ) {
            if (try1_count<30) {
                try1_count++;
                continue;
            } else {
                syslog(LOGOPTS,"GPSR exit: read gpsd return 0 byte data\n");
                exit(1);
            }
        }

        if (readbytes >0) {
            buf[readbytes]='\0';

            // try max 60 times to wait gps data available
            if (check_gps_data(buf,&base_gps_pos)==WAITING) {
                if (try2_count<60) {
                    try2_count++;
                    continue;
                } else {
                    syslog(LOGOPTS,"GPSR exit: gpsd cann't provide available gps data\n");
                    exit(1);
                }

            }

            //syslog(LOGOPTS,"GPSR: gps_pos:%9.4f %9.4f\n",base_gps_pos.lat,base_gps_pos.lng);
            bzero(cbuf,sizeof(cbuf));
            /* ship the r=0 option to gpsd in order to stop receiving gps data */
            strcpy(cbuf, "r=0");
            wrote = write(sock, cbuf, strlen(cbuf));
            if ((ssize_t)strlen(cbuf) != wrote) {
                //it should be recorded into syslog
                syslog(LOGOPTS,"GPSR exit: ship assembled options: write error\n");
                exit(1);
            }
            close(sock);
            //send first messages

            if (fetch_one_pattern(buf,pbuf)!=0) {
                syslog(LOGOPTS,"GPSR send first messages: fetch_one_pattern failed because gpsd can't send available data.\n");
                break;
            }


            for (i=0;i<count;i++) {
                int item=enabled_report[i];
                bzero(sbuf,sizeof(sbuf));
                if (send_select_nmea(pbuf,DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_MSS_TYPE0+item],sbuf)<0) {
                    syslog(LOGOPTS,"GPSR send first messages: report%d send_select_nmea failed.\n",item);
                    continue;
                }
                if (DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR][item]=='B') {

                    sockbuf[item].num = sendto(sockbuf[item].sockfd, sbuf, strlen(sbuf), 0, (struct sockaddr *)&(sockbuf[item].addr_remote), sizeof(struct sockaddr_in));
                    if (sockbuf[item].num < 0 ) {

                        syslog(LOGOPTS,"GPSR: Failed to send report to %s:%s!\n",DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_REIP0+item], DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RPORT0+item]);

                    }
                    syslog(LOGOPTS,"GPSR send first messages: report%d sending report to %s:%s!\n",item,DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_REIP0+item], DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RPORT0+item]);
                }
                if (DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR][item]=='C') {
                    syslog(LOGOPTS,"GPSR send first messages: report%d sending by email\n",item);
                    send_by_email(sbuf,enabled_report[i]);
                }
            }

            break;
            // try max 30 times to read data from gpsd if read error
        } else {
            if (try3_count<30) {
                try3_count++;
                continue;
            } else {
                //should be recorded into syslog
                syslog(LOGOPTS,"GPSR exit: gpsr cann't read data from gpsd\n");
                exit(1);
            }
        }

    }//end of for check if gps data is available
#endif
    int try_count=0;
    /* get current time */
    gettimeofday(&start,0);
    addeq_oper_timeval(&start,&min_tval);

    int init_flag = ENABLE;

    /*
 * right now, gpsr starts running
 */                   
    time_t test_time;

//    for (;;) {
        /*
         * select the report or reports have earliest timer
         */
        struct timeval cur_earliest_time;
        cur_earliest_time.tv_sec=ss_buf[enabled_report[0]].tnex.tv_sec;
        cur_earliest_time.tv_usec=ss_buf[enabled_report[0]].tnex.tv_usec;

        //syslog(LOGOPTS,"GPSR: current trigger timer is %d.%d\n",cur_earliest_time.tv_sec,cur_earliest_time.tv_usec);

        for (i=0;i<count;i++) {
            ss_buf[enabled_report[i]].scheduled = NO;
            if ((cur_earliest_time.tv_sec==ss_buf[enabled_report[i]].tnex.tv_sec)\
                &&(cur_earliest_time.tv_usec==ss_buf[enabled_report[i]].tnex.tv_usec)) {
                /*
                 * this report will be scheduled to trigger at this time,
                 * we have to estimate the next scheduled time only for this report,
                 * but for other reports, it is not necessary
                 */
                /* only timer enable */
                if (ss_buf[enabled_report[i]].distance_enable==DISABLE) {
                    ss_buf[enabled_report[i]].scheduled = TIMER;
                    addeq_oper_timeval(&(ss_buf[enabled_report[i]].tnex),&(ss_buf[enabled_report[i]].tval));
                    // for debug,will be delete
                    //syslog(LOGOPTS,"GPSR: timer trigger scheduled report is report%d\n",enabled_report[i]);
                    // end of debug
                    continue;
                }
                /* check if distance enable */
                if (ss_buf[enabled_report[i]].distance_enable==ENABLE) {
                    /* store init time get gps postion as basepos */
                    if (init_flag==ENABLE) {
                        ss_buf[enabled_report[i]].basepos.lat = base_gps_pos.lat;
                        ss_buf[enabled_report[i]].basepos.lng = base_gps_pos.lng;
                        init_flag=DISABLE;
//printf("##Update pos1: %d##\n",i);

                    }
                    if (ss_buf[enabled_report[i]].trigger_conf == ANDCONF) {
                        if ((cur_earliest_time.tv_sec==ss_buf[enabled_report[i]].texp.tv_sec)\
                            &&(cur_earliest_time.tv_usec==ss_buf[enabled_report[i]].texp.tv_usec)) {
                            ss_buf[enabled_report[i]].scheduled = TIMERANDDIS;
                            // tnex == texp; tval == tcxs
                            addeq_oper_timeval(&(ss_buf[enabled_report[i]].tnex),&(ss_buf[enabled_report[i]].tval));
                            addeq_oper_timeval(&(ss_buf[enabled_report[i]].texp),&(ss_buf[enabled_report[i]].tcxs));
                            //ss_buf[enabled_report[i]].basepos.lat = base_gps_pos.lat;
                            //ss_buf[enabled_report[i]].basepos.lng = base_gps_pos.lng;
                            // for debug,will be delete
                            //syslog(LOGOPTS,"GPSR: timer and distance trigger scheduled report is report%d\n",enabled_report[i]);
                            // end of debug
                            continue;
                        }
                    } else if (ss_buf[enabled_report[i]].trigger_conf == ORCONF) {
                        ss_buf[enabled_report[i]].scheduled = ORDIS;
                        addeq_oper_timeval(&(ss_buf[enabled_report[i]].tnex),&(ss_buf[enabled_report[i]].tval));

                        //syslog(LOGOPTS,"GPSR: or distance trigger scheduled report is report%d\n",enabled_report[i]);

                        if ((cur_earliest_time.tv_sec==ss_buf[enabled_report[i]].texp.tv_sec)\
                            &&(cur_earliest_time.tv_usec==ss_buf[enabled_report[i]].texp.tv_usec)) {
                            addeq_oper_timeval(&(ss_buf[enabled_report[i]].texp),&(ss_buf[enabled_report[i]].tcxs));
                            //ss_buf[enabled_report[i]].basepos.lat = base_gps_pos.lat;
                            //ss_buf[enabled_report[i]].basepos.lng = base_gps_pos.lng;
                            ss_buf[enabled_report[i]].scheduled = TIMERORDIS;

                            //syslog(LOGOPTS,"GPSR: timer or distance trigger and timer expired scheduled report is report%d\n",enabled_report[i]);

                        }

                    }

                }
            }// end of exteroriest if


        }// end of for
        /*
         * send message
         *
         */

        readbytes = 0;


        //gettimeofday(&end1,0);
        //debug("select report end time %d.%d\n",end1.tv_sec,end1.tv_usec);
        //delay1 = (end1.tv_sec-start1.tv_sec)*1000+(end1.tv_usec-start1.tv_usec)/1000;
        //debug("select report delay:%d\n",delay1);
        gettimeofday(&end,0);
        start1.tv_sec = start.tv_sec;
        start1.tv_usec = start.tv_usec;
        //syslog(LOGOPTS,"GPSR: end timer is %d.%d\n",end.tv_sec,end.tv_usec);
        //syslog(LOGOPTS,"GPSR: start timer is %d.%d\n",start.tv_sec,start.tv_usec);
        minuseq_oper_timeval(&start1,&end);

        resending:
        //syslog(LOGOPTS,"GPSR: sleep timer is %d.%d\n",start1.tv_sec,start1.tv_usec);
        bzero(buf,sizeof(buf));
        if (start1.tv_sec>2) {
//sms            sleep_us(start1.tv_sec-2,start1.tv_usec);
            /* ship the r=1 option in order to start receiving gps data from gpsd */
            bzero(cbuf,sizeof(cbuf));
            strcpy(cbuf, "r=1");
            wrote = write(sock, cbuf, strlen(cbuf));
            if ((ssize_t)strlen(cbuf) != wrote) {
                //it should be recorded into syslog
                syslog(LOGOPTS,"GPSR error: ship r=1 option: write error\n");

                shutdown(sock,SHUT_RDWR);
                close(sock);
                //sleep_us(2,0);
                sock = netlib_connectsock(server,port,"tcp");
                if (sock<0) {
                    syslog(LOGOPTS,"GPSR exit: reconnect gpsd failed\n");
                    exit(1);
                }
                start1.tv_sec = 1;
                start1.tv_usec = 0;
                goto resending;

            }
            sleep_us(2,0);

        } else {
            /* ship the r=1 option in order to start receiving gps data from gpsd */
            bzero(cbuf,sizeof(cbuf));
            strcpy(cbuf, "r=1");
            wrote = write(sock, cbuf, strlen(cbuf));
            if ((ssize_t)strlen(cbuf) != wrote) {
                //it should be recorded into syslog
                syslog(LOGOPTS,"GPSR error: ship r=1 option: write error\n");

                shutdown(sock,SHUT_RDWR);
                close(sock);
                //sleep_us(start1.tv_sec,start1.tv_usec);
                sock = netlib_connectsock(server,port,"tcp");
                if (sock<0) {
                    syslog(LOGOPTS,"GPSR exit: reconnect gpsd failed\n");
                    exit(1);
                }
                start1.tv_sec = 1;
                start1.tv_usec = 0;
                goto resending;

            }
//sms            sleep_us(start1.tv_sec,start1.tv_usec);
        }

        readbytes = (int)read(sock, buf, sizeof(buf));

        bzero(cbuf,sizeof(cbuf));
        /* ship the r=0 option to gpsd in order to stop receiving gps data */
        strcpy(cbuf, "r=0");
        wrote = write(sock, cbuf, strlen(cbuf));
        if ((ssize_t)strlen(cbuf) != wrote) {
            //it should be recorded into syslog
            syslog(LOGOPTS,"GPSR error: ship r=0 option: write error\n");

            shutdown(sock,SHUT_RDWR);
            close(sock);
            sock = netlib_connectsock(server,port,"tcp");
            if (sock<0) {
                syslog(LOGOPTS,"GPSR exit: reconnect gpsd failed\n");
                exit(1);
            }
            start1.tv_sec = 1;
            start1.tv_usec = 0;
            goto resending;

        }

        // for debug, will be deleted
        //syslog(LOGOPTS,"read gpsd:\n%s",buf);
        // end of debug
        //gettimeofday(&start1,0);
        //syslog(LOGOPTS,"select setence and send data start time %d.%d\n",start1.tv_sec,start1.tv_usec);
        if (readbytes >0) {
            buf[readbytes]='\0';
            /* get one pattern */
            bzero(pbuf,sizeof(pbuf));
            //int gop = get_one_pattern(buf,pbuf);
            int fop=fetch_one_pattern(buf,pbuf);
            if (fop!=0) {
                syslog(LOGOPTS,"GPSR exit: fetch_one_pattern failed because gpsd can't send available data.\n");
                start1.tv_sec = 1;
                start1.tv_usec = 0;
                goto resending;
            }
            int ggp_flag = 0;
            for (i=0;i<count;i++) {
                int item=enabled_report[i];
                int ssn=-1;

                if (ss_buf[enabled_report[i]].scheduled == NO) continue;

                bzero(sbuf,sizeof(sbuf));
                if ((ss_buf[enabled_report[i]].scheduled == TIMER)||(ss_buf[enabled_report[i]].scheduled == TIMERORDIS)) {
                    if (ss_buf[enabled_report[i]].scheduled == TIMERORDIS) {
                        int ggp=0;
                        if (ggp_flag==0) {
                            ggp=get_gps_pos(buf,&base_gps_pos);
                            ggp_flag=1;
                        }
                        if (ggp<0) {
                            //should be recorded into syslog
                            syslog(LOGOPTS,"GPSR: report#%d trigger condition is timerORdistance && timer is expired, get_gps_pos error\n",item);
                            continue;
                        }
                        /* get_gps_pos success, break for */
                        if (ggp==0) {
                            ss_buf[enabled_report[i]].basepos.lat = base_gps_pos.lat;
                            ss_buf[enabled_report[i]].basepos.lng = base_gps_pos.lng;

                            //syslog(LOGOPTS,"GPSR report#%d trigger condition is timerORdistance && timer is expired: base gps_pos:%9.4f %9.4f\n ",item,ss_buf[item].basepos.lat,ss_buf[item].basepos.lng);
                            //syslog(LOGOPTS,"GPSR report#%d trigger condition is timerORdistance && timer is expired: current gps_pos:%9.4f %9.4f\n",item,base_gps_pos.lat,base_gps_pos.lng);

                        }
                    }
                    ssn=send_select_nmea(pbuf,DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_MSS_TYPE0+item],sbuf);

                } else {
                    if ((ss_buf[enabled_report[i]].scheduled == ORDIS)||(ss_buf[enabled_report[i]].scheduled == TIMERANDDIS)) {
                        /* get current gps postion, to compute the distance */
                        int ggp=0;
                        if (ggp_flag==0) {
                            ggp=get_gps_pos(buf,&base_gps_pos);
//printf("**AND Updategps%d #%f#%f**\n",item,base_gps_pos.lat,base_gps_pos.lng);
                            ggp_flag=1;
                        }
                        /* get_gps_pos return error */
                        if (ggp<0) {
                            //should be recorded into syslog
                            syslog(LOGOPTS,"GPSR: report#%d trigger condition is timerORdistance || timerANDdistance, get_gps_pos error\n",item);
                            continue;
                        }
                        /* get_gps_pos success, break for */
                        if (ggp==0) {

                            //syslog(LOGOPTS,"GPSR report#%d trigger condition is timerORdistance || timerANDdistance: base gps_pos:%9.4f %9.4f\n ",item,ss_buf[item].basepos.lat,ss_buf[item].basepos.lng);
                            //syslog(LOGOPTS,"GPSR report#%d trigger condition is timerORdistance || timerANDdistance: current gps_pos:%9.4f %9.4f\n",item,base_gps_pos.lat,base_gps_pos.lng);

                            double distance = compute_distance(&(ss_buf[item].basepos),&base_gps_pos);
                            int int_dis=(int)distance;
//                           float tmp_1=ss_buf[enabled_report[i]].basepos.lat;
//                           float tmp_2=ss_buf[enabled_report[i]].basepos.lng;
                            //syslog(LOGOPTS,"GPSR trigger condition is timerORdistance || timerANDdistance: distance=%f int_dis=%d\n",distance,int_dis);
                            if (ss_buf[enabled_report[i]].scheduled == TIMERANDDIS) {
                                ss_buf[enabled_report[i]].basepos.lat = base_gps_pos.lat;
                                ss_buf[enabled_report[i]].basepos.lng = base_gps_pos.lng;
//printf("##Update pos3: %d##\n",i);
                            }

                            if (int_dis>atoi(DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_METER0+item]))
                            {
                                ssn=send_select_nmea(pbuf,DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_MSS_TYPE0+item],sbuf);
//time(&test_time);
//printf("**dis %d trigger%d %s*%f,%f--%f,%f\n",int_dis,item,ctime(&test_time),tmp_1,tmp_2,base_gps_pos.lat,base_gps_pos.lng);
                            }
                            else {
                                //syslog(LOGOPTS,"GPSR report#%d trigger condition is timerORdistance || timerANDdistance: trigger condition is satisfied but the distance dismatched\n",item);
                                continue;
                            }
                        }
                    }
                }

                if (ssn<0) {
                    //syslog(LOGOPTS,"GPSR: report#%d trigger condition is satisfied but the message type dismatched\n",item);
                    continue;
                }
                if (DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR][item]=='B') {

                    sockbuf[item].num = sendto(sockbuf[item].sockfd, sbuf, strlen(sbuf), 0, (struct sockaddr *)&(sockbuf[item].addr_remote), sizeof(struct sockaddr_in));
//time(&test_time);
//printf("**send report%d %d : %s",item,sockbuf[item].num,ctime(&test_time));

                    //add by dwj 2012-9-11, to reset after send.
                    if(get_gps_pos(buf,&base_gps_pos)==0)
                    {
                                    ss_buf[enabled_report[i]].basepos.lat = base_gps_pos.lat;
                                    ss_buf[enabled_report[i]].basepos.lng = base_gps_pos.lng;
                    }
//printf("**dis%d N%f W%f \n",enabled_report[i],base_gps_pos.lat,base_gps_pos.lng);


                            
                    if (sockbuf[item].num < 0 ) {

                        syslog(LOGOPTS,"GPSR: Failed to send report to %s:%s!\n",DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_REIP0+item], DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RPORT0+item]);

                    } else {

                        //syslog(LOGOPTS,"OK: Sent to %s total %d bytes !\n", DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RPORT0+item], sockbuf[item].num);

                    }
                }
                if (DataBase1_Buff[ADVANCED_GPS_CONF_REPORT][AGCR_ITEM_RR][item]=='C') {
                    //syslog(LOGOPTS,"GPSR : report%d sending by email\n",item);
                    send_by_email(sbuf,enabled_report[i]);

                    //add by dwj 2012-9-11, to directly reset after send.
                    if(get_gps_pos(buf,&base_gps_pos)==0)
                    {
                        ss_buf[enabled_report[i]].basepos.lat = base_gps_pos.lat;
                        ss_buf[enabled_report[i]].basepos.lng = base_gps_pos.lng;
                    }
                    //

                }
                /*scheduled already so that we have to reset scheduled flag to NO*/
                ss_buf[enabled_report[i]].scheduled = NO;
            }
        } else {
            //should be recorded into syslog
            syslog(LOGOPTS,"GPSR exit: gpsr cann't read data from gpsd\n");
            exit(1);

        }
#if 0
        gettimeofday(&end1,0);
        syslog(LOGOPTS,"select sentence end time %d.%d\n",end1.tv_sec,end1.tv_usec);
        delay1 = (end1.tv_sec-start1.tv_sec)*1000+(end1.tv_usec-start1.tv_usec)/1000;
        syslog(LOGOPTS,"select sentences and send report delay:%d\n",delay1);
#endif
        /*
         * after send messgaes
         * we have to calculate sleep timeval
         */
        //gettimeofday(&start1,0);
        //syslog(LOGOPTS,"sort start time %d.%d\n",start1.tv_sec,start1.tv_usec);
        reschedule:
        select_sort_ssbuf(count);
        minus_oper_timeval(&min_tval,&(ss_buf[enabled_report[0]].tnex),&(cur_earliest_time));

        //gettimeofday(&end1,0);
        //syslog(LOGOPTS,"sort end time %d.%d\n",end1.tv_sec,end1.tv_usec);
        //delay1 = (end1.tv_sec-start1.tv_sec)*1000+(end1.tv_usec-start1.tv_usec)/1000;
        //syslog(LOGOPTS,"sort delay:%d\n",delay1);
        /* get current time */
        gettimeofday(&start,0);
        addeq_oper_timeval(&start,&min_tval);

        // for debug, will be deleted
        //syslog(LOGOPTS,"min interval time is %d.%d\n",min_tval.tv_sec,min_tval.tv_usec);

        // end of debug

//    }//end of for

    return 0;

}//end of main

