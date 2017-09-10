#include "gpsrecorder.h"

//record file structure: /log/gpsrec/0,1,2,3,4,...,10...50
//Date,Time,Lat,N/S,Lon,W/E,Ele,DI,DO,Speed km/h,Direction degree,RSSI
//#Local Date,Time,DI,DO,RSSI	-----if there is no satellite data and IO changed / system restart/reset(wait 1~2 minutes). 
//Seconds passed,Lat changed,Lon changed,alt,speed, Direction
//120413,191735,5108.57096,N,11404.51843,W,1093,0000,0000,21,142
//+300,+221,,-102,,+6,,,

//gpsrecorder readgps
//status file structrue:
//(printf \n)<tr><td width="40%">&nbsp;&nbsp;Altitude:</td><td width="60%">1096</td></tr>
//echo /tmp/run/gpsstatus 

char *gpsstatus_file="/var/run/gpsstatus";
int gpsrecord_file_index=-1;

char save_new_record_state=0;
int i_MAX_RECITEMS_IN_MEM=19;
char save_rec_file_state=15;

char str_iostatus_buff[20];
char last_str_iostatus_buff[20];

#define SPEED_START 10
#define SPEED_CHG_BASE 10
#define MAX_SPEED_CHG_TIME 2
char speed_state_chg_count=0;//when continues to change times 3s .
int last_record_speed=0;
int now_record_speed=0;
int over_speed_multi_sign=0;

#define MAX_DIRECTION_CHG_TIME 3
#define ORIENT_CHK_BASESPEED 10  //at least 10Km/h will be valid.
char direction_state_chg_count=0;//when continues to change times 3s.
int last_record_direction=0;
int now_record_direction=0;

#define MAX_RSSI_CHG_TIME 3
#define LOW_RSSI_LEVEL 90
#define BACKUP_RSSI_LEVEL 80
char rssi_state_chg_count=0;
int now_record_rssi=999;
int last_record_rssi=999;

char last_full_pos_lat[15];
char last_full_pos_lat_NS;
char last_full_pos_lng[15];
char last_full_pos_lng_WE;
char last_full_utc_time[10];
char last_full_utc_date[10];

char now_pos_lat[15];
char now_pos_lat_NS;
char now_pos_lng[15];
char now_pos_lng_WE;

char str_pos_latlng[50];
char str_pos_alt[20];

char valid_c='0';
char utc_time[20];
char utc_date[20];

    int readbytes = 0;
    int sock = 0;
    ssize_t wrote = 0;
    int fd_tcp=0;
    char buf[2048];
    char cbuf[10];

char c_Pos_Record_EN;
int i_Pos_Record_NUMS;  //no meaning now for automatic adjust record
int i_Pos_Record_INTERVAL;
char c_Pos_Record_ALT;
char c_Pos_Record_IO;
char c_Pos_Record_SPEED;
char c_Pos_Record_ORIENT;
char c_Pos_Record_RSSI;
int i_Pos_Record_OVERSPEED;
int i_Pos_Record_ORIENTCHG;


int gpsd_port;
char gpsd_status;



void check_record_file_index()
{
    gpsrecord_file_index=0;

    char fullfile[30];
    char tmp_buff[256];
    char timestr0[20];
    char timestr1[20];
    int year0=0;
    int mon0=0;
    int day0=0;
    int hour0=0;
    int min0=0;
    int sec0=0;
    int year1=0;
    int mon1=0;
    int day1=0;
    int hour1=0;
    int min1=0;
    int sec1=0;
    int i;
    FILE* f=NULL;
    char *p,*p1;
    sprintf(fullfile,"%s0",gpsrecord_dir);
    f = fopen(fullfile, "r");
    if(f==NULL)
    {
        return;
    }
nextline_read0:
    tmp_buff[0]=0;
    if(fgets(tmp_buff,250,f)==NULL)
    {
        fclose(f);
        return;
    }
    if(tmp_buff[0]=='#' || strlen(tmp_buff)<32)goto nextline_read0;
    if(tmp_buff[6]!=',')goto nextline_read0;

        tmp_buff[50]=0;
        p1=tmp_buff;
        p=NULL;
        p=strchr(p1,',');
        if(p==NULL)
        {
            fclose(f);
            return;
        }
        p++;
        p1=p;
        p=NULL;
        p=strchr(p1,',');
        if(p==NULL) 
        {
            fclose(f);
            return;
        }
        *p=0;
        sscanf(tmp_buff, "%2d%2d%2d,%2d%2d%2d*",&day0, &mon0, &year0, &hour0, &min0, &sec0);
        if(day0==0)
        {
            fclose(f);
            return;
        }
        sprintf(timestr0,"%02d%02d%02d%02d%02d%02d",year0,mon0,day0,hour0,min0,sec0);
    fclose(f);

    i=1;
    while(i<MAX_REC_FILE_NUM)
    {
        sprintf(fullfile,"%s%d",gpsrecord_dir,i);
        f = fopen(fullfile, "r");
        if(f==NULL)
        {
            return;
        }

nextline_read1:
        tmp_buff[0]=0;
        if(fgets(tmp_buff,250,f)==NULL)
        {
            fclose(f);
            return;
        }
        if(tmp_buff[0]=='#' || strlen(tmp_buff)<32)goto nextline_read1;

            tmp_buff[50]=0;
            p1=tmp_buff;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)
            {
                fclose(f);
                return;
            }
            p++;
            p1=p;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL) 
            {
                fclose(f);
                return;
            }
            *p=0;
            sscanf(tmp_buff, "%2d%2d%2d,%2d%2d%2d*",&day1, &mon1, &year1, &hour1, &min1, &sec1);
            if(day1==0)
            {
                fclose(f);
                return;
            }
            sprintf(timestr1,"%02d%02d%02d%02d%02d%02d",year1,mon1,day1,hour1,min1,sec1);
        fclose(f);
        if(strcmp(timestr1,timestr0)>0)
        {
            gpsrecord_file_index=i;
            strcpy(timestr0,timestr1);
        }
        i++;
    }

    sprintf(fullfile,"%s%d",gpsrecord_dir,gpsrecord_file_index);
    f = fopen(fullfile, "r");
    if(f==NULL)
    {
        return;
    }
    if (-1==fseek(f,0L,SEEK_END))return;
    int fsize=ftell(f);
    fclose(f);    
    if(fsize>16280)gpsrecord_file_index++;
    if(gpsrecord_file_index>=MAX_REC_FILE_NUM)gpsrecord_file_index=0;
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


    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsrecorderd","gpsrecorder_conf","Pos_Record_EN",tmp_buff);
    c_Pos_Record_EN=tmp_buff[0];
    
    tmp_buff[0]=0;    
    get_option_by_section_name(ctx, "gpsrecorderd","gpsrecorder_conf","Pos_Record_INTERVAL",tmp_buff);
    i_Pos_Record_INTERVAL=atoi(tmp_buff);
    if(i_Pos_Record_INTERVAL<10)
    {
        i_Pos_Record_INTERVAL=10;
    }

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsrecorderd","gpsrecorder_conf","Pos_Record_ALT",tmp_buff);
    c_Pos_Record_ALT=tmp_buff[0];

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsrecorderd","gpsrecorder_conf","Pos_Record_IO",tmp_buff);
    c_Pos_Record_IO=tmp_buff[0];

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsrecorderd","gpsrecorder_conf","Pos_Record_SPEED",tmp_buff);
    c_Pos_Record_SPEED=tmp_buff[0];

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsrecorderd","gpsrecorder_conf","Pos_Record_ORIENT",tmp_buff);
    c_Pos_Record_ORIENT=tmp_buff[0];

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "gpsrecorderd","gpsrecorder_conf","Pos_Record_RSSI",tmp_buff);
    c_Pos_Record_RSSI=tmp_buff[0];

    tmp_buff[0]=0;    
    get_option_by_section_name(ctx, "gpsrecorderd","gpsrecorder_conf","Pos_Record_OVERSPEED",tmp_buff);
    i_Pos_Record_OVERSPEED=atoi(tmp_buff);
    if(i_Pos_Record_OVERSPEED<30)
    {
        i_Pos_Record_OVERSPEED=30;
    }

    tmp_buff[0]=0;    
    get_option_by_section_name(ctx, "gpsrecorderd","gpsrecorder_conf","Pos_Record_ORIENTCHG",tmp_buff);
    i_Pos_Record_ORIENTCHG=atoi(tmp_buff);
    if(i_Pos_Record_ORIENTCHG<5)
    {
        i_Pos_Record_ORIENTCHG=5;
    }
    if(i_Pos_Record_ORIENTCHG>180)
    {
        i_Pos_Record_ORIENTCHG=180;
    }

    if(c_Pos_Record_IO!='B')i_MAX_RECITEMS_IN_MEM-=2;
    if(c_Pos_Record_RSSI!='B')i_MAX_RECITEMS_IN_MEM-=2;
    if(c_Pos_Record_SPEED!='B')i_MAX_RECITEMS_IN_MEM-=2;
    if(c_Pos_Record_ORIENT!='B')i_MAX_RECITEMS_IN_MEM-=2;
    if(c_Pos_Record_ALT!='B')i_MAX_RECITEMS_IN_MEM-=1;
    if(i_Pos_Record_INTERVAL>=3600)i_MAX_RECITEMS_IN_MEM=2;
    else
    {
        int i=i_Pos_Record_INTERVAL/600;
        i_MAX_RECITEMS_IN_MEM-=i;
        if(i_MAX_RECITEMS_IN_MEM<3)i_MAX_RECITEMS_IN_MEM=3;
    }

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

// now_record_rssi:0-no such file(will not save to file), 999-unknown(no sim card) or no network.
int get_rssi_buff()
{

    char tmp_buff[256];
    FILE *f;
    char *p;
    int i;

    now_record_rssi=0;
    f=fopen("/var/run/std_lte_statue","r");
    if(f==NULL)return 0;
    while(fgets(tmp_buff,255,f))
    {
        p=NULL;
        p=strstr(tmp_buff,"rssi=");
        if(p!=NULL)
        {
            now_record_rssi=999;
            p=p+strlen("rssi=");
            while(*p==' ' || *p=='-')p++;
            if(*p>='0' && *p<='9')
            {
                i=0;
                i=atoi(p);
                //if(i<0)i=-1*i;
                if(i>0)now_record_rssi=i;
            }
            break;
        }
    }
    fclose(f);
}

int get_iostatus_buff()
{
    char tmp_buff[50];
    FILE* f;
    char *p;
    str_iostatus_buff[0]=0;
    //printf("----io check:%c--",c_SERVER_IOStatus);
    //  /sys/class/button/INPUT1/status

    if ((f = fopen("/sys/class/button/INPUT1/status", "r"))!=NULL) 
    {
        if(fgets(tmp_buff, 10, f))
        {
            str_iostatus_buff[0]=tmp_buff[0];
        }
        fclose(f);
    }
    if ((f = fopen("/sys/class/button/INPUT2/status", "r"))!=NULL) 
    {
        if(fgets(tmp_buff, 10, f))
        {
            str_iostatus_buff[1]=tmp_buff[0];
        }
        fclose(f);
    }
    if ((f = fopen("/sys/class/button/INPUT3/status", "r"))!=NULL) 
    {
        if(fgets(tmp_buff, 10, f))
        {
            str_iostatus_buff[2]=tmp_buff[0];
        }
        fclose(f);
    }
    if ((f = fopen("/sys/class/button/INPUT4/status", "r"))!=NULL) 
    {
        if(fgets(tmp_buff, 10, f))
        {
            str_iostatus_buff[3]=tmp_buff[0];
        }
        fclose(f);
    }
    str_iostatus_buff[4]=',';


    //  /sys/class/leds/OUTPUT1/brightness
    if ((f = fopen("/sys/class/leds/OUTPUT1/brightness", "r"))!=NULL) 
    {
        if(fgets(tmp_buff, 10, f))
        {
            str_iostatus_buff[5]=tmp_buff[0];
        }
        fclose(f);
    }
    if ((f = fopen("/sys/class/leds/OUTPUT2/brightness", "r"))!=NULL) 
    {
        if(fgets(tmp_buff, 10, f))
        {
            str_iostatus_buff[6]=tmp_buff[0];
        }
        fclose(f);
    }
    if ((f = fopen("/sys/class/leds/OUTPUT3/brightness", "r"))!=NULL) 
    {
        if(fgets(tmp_buff, 10, f))
        {
            str_iostatus_buff[7]=tmp_buff[0];
        }
        fclose(f);
    }
    if ((f = fopen("/sys/class/leds/OUTPUT4/brightness", "r"))!=NULL) 
    {
        if(fgets(tmp_buff, 10, f))
        {
            str_iostatus_buff[8]=tmp_buff[0];
        }
        fclose(f);
    }

    str_iostatus_buff[9]=0;

    return 9;
}

//record to last_ for rssi,io,speed and orientation
int save_to_last_record(char *items)
{
    int i=strlen(items);
    if(i<1 || i>6)return 0;
    char *p;
    p=items;
    while(*p!=0)
    {
        if(*p=='i' && c_Pos_Record_IO=='B')
        {
            strcpy(last_str_iostatus_buff,str_iostatus_buff);
        }
        if(*p=='r' && c_Pos_Record_RSSI=='B')
        {
            if(now_record_rssi!=0)
            {
                last_record_rssi=now_record_rssi;
            }
        }
        if(*p=='s' && c_Pos_Record_SPEED=='B')
        {
            if(now_record_speed>=0)
            {
                last_record_speed=now_record_speed;
            }
        }
        if(*p=='d' && c_Pos_Record_ORIENT=='B')
        {
            if(now_record_direction>=0)
            {
                last_record_direction=now_record_direction;
            }
        }
        if(*p=='p')
        {
            if(now_pos_lat[0]!=0 && now_pos_lng[0]!=0 )
            {
                strcpy(last_full_pos_lat,now_pos_lat);
                strcpy(last_full_pos_lng,now_pos_lng);
                last_full_pos_lat_NS=now_pos_lat_NS;
                last_full_pos_lng_WE=now_pos_lng_WE;
            }
        }
        if(*p=='t')
        {
            if(utc_date[0]!=0 && utc_time[0]!=0 )
            {
                strcpy(last_full_utc_time,utc_time);
                strcpy(last_full_utc_date,utc_date);
            }
        }
        p++;
    }
}


int main(int argc, char *argv[])
{
    char tmp_buff[257];
    char tmp_buff1[10];
    char time_diff[6];
    char lat_diff[7];
    char lng_diff[7];
    float tmp_f=0;
    int gpsd_conn_status=0;
    int time_cnt=0;
    int fsize;
    int readgps_return=0;
    int need_re_initsys=0;
    int full_record_sign=1;
    int wait_valid_psdata=0;
    FILE *f;
    FILE *sfp;
    char *p;
    char *p1;
    int i,j;
    time_t localtime_t;
    int save_file_cnt=0;
    (void) setsid();          
    signal(SIGPIPE, SIG_IGN);
    check_record_file_index();


    //cp working gpsrecord_file to gpsrecord_varfile
    //    sprintf(tmpbuff,"gpsrectovar %d",gpsrecord_file_index);
    //    if (SubProgram_Start(gpsrscriptFile,tmpbuff))
    sprintf(tmp_buff,"%s%d %s",gpsrecord_dir,gpsrecord_file_index,gpsrecord_varfile);
    if(SubProgram_Start("/bin/cp",tmp_buff)==false)
    {
        syslog(LOGOPTS,"GPS record%d copy error\n", gpsrecord_file_index);
        return 0;
    }
    

re_initsys:
    if(readgps_return==1)return 0;

    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        fprintf(stderr, "Out of memory\n");
        return -1;
    }

    read_config();//gpsd, gpsgatetr
    if(argc==2)
    {
        if(strcmp(argv[1],"readgps")==0)
        {
            readgps_return=1;
        }
    }

    //check config or exit.
    if(c_Pos_Record_EN!='B')return 0;

    if(gpsd_status!='1')
    {
        uci_free_context(ctx);
        sleep(5);
        goto re_initsys;
    }

    gpsd_conn_status=0;


//int i_Pos_Record_NUMS;
//int i_Pos_Record_INTERVAL;

    need_re_initsys=0;
    while(1)
    {
        if(need_re_initsys==1)goto re_initsys;

        //time_cnt++;
        //sleep(1);
        valid_c='0';

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
                time_cnt+=5;
                need_re_initsys=1;
                goto check_IORSSI;
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
                time_cnt+=5;
                need_re_initsys=1;
                goto check_IORSSI;
            }

            sleep(1);
            time_cnt++;

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
                    time_cnt+=5;
                    need_re_initsys=1;
                    goto check_IORSSI;
                }

                if(readgps_return==1)
                {
                    get_gps_pos(buf,&base_gps_pos);
                        char s_tr_num[10];
                        char s_view_num[10];
                        str_pos_alt[0]=0;
                        p=NULL;
                        p=strstr(buf,"$GPGGA");
                        if(p!=NULL)
                        {
                            while(*p!=',')p++;//1
                            p++;
                            p1=p;
                            while(*p!=',')p++;//2
                            strncpy(utc_time,p1,p-p1);
                            utc_time[p-p1]=0;
                            p++;
                            while(*p!=',')p++;//3
                            p++;
                            while(*p!=',')p++;//4
                            p++;
                            while(*p!=',')p++;//5
                            p++;
                            while(*p!=',')p++;//6
                            p++;
                            if(*p=='1' || *p=='2')valid_c='1';
                            while(*p!=',')p++;//7
                            p++;
                            p1=p;
                            while(*p!=',')p++;//8
                            strncpy(s_tr_num,p1,p-p1);//tracked satellite 1~20
                            s_tr_num[p-p1]=0;
                            p++;
                            while(*p!=',')p++;//9
                            p++;
                            p1=p;
                            while(*p!=',')p++;//5
                            strncpy(str_pos_alt,p1,p-p1);//alt: HHH.h,
                            str_pos_alt[p-p1]=0;
                        }
                        p=NULL;
                        p=strstr(buf,"$GPRMC");
                        if(p!=NULL)
                        {
                            while(*p!=',')p++;//1
                            p++;
                            p1=p;
                            while(*p!=',')p++;//2
                            strncpy(utc_time,p1,p-p1);
                            utc_time[p-p1]=0;
                            p++;
                            if(*p=='A')valid_c='1';
                            while(*p!=',')p++;//3
                            p++;
                            while(*p!=',')p++;//4
                            p++;
                            while(*p!=',')p++;//5
                            p++;
                            while(*p!=',')p++;//6
                            p++;
                            while(*p!=',')p++;//7
                            p++;
                            p1=p;
                            while(*p!=',')p++;//8
                            //speed here.
                            strncpy(tmp_buff,p1,p-p1);
                            tmp_buff[p-p1]=0;
                            float tmp_f=0;
                            sscanf(tmp_buff, "%f", &(tmp_f));
                            tmp_f=1.852*tmp_f+0.5;
                            last_record_speed=(int)tmp_f;
                            p++;
                            p1=p;
                            while(*p!=',')p++;//9
                            //orientation here.
                            strncpy(tmp_buff,p1,p-p1);
                            tmp_buff[p-p1]=0;
                            sscanf(tmp_buff, "%f", &(tmp_f));
                            tmp_f=tmp_f+0.5;
                            last_record_direction=(int)tmp_f;
                            if(last_record_speed==0)last_record_direction=0;
                            p++;
                            p1=p;
                            while(*p!=',')p++;//10
                            strncpy(utc_date,p1,p-p1);
                            utc_date[p-p1]=0;

                            

                        }
                        p=NULL;
                        p=strstr(buf,"$GPGSV"); //$GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75
                        if(p!=NULL)
                        {
                            while(*p!=',')p++;//1
                            p++;
                            while(*p!=',')p++;//2
                            p++;
                            while(*p!=',')p++;//3
                            p++;
                            p1=p;
                            while(*p!=',')p++;//4
                            strncpy(s_view_num,p1,p-p1);
                            s_view_num[p-p1]=0;
                        }

                        if(valid_c=='1')
                        {

                            char c_n=' ';
                            char c_e=' ';
                            float lat1=0;
                            float lng1=0;
                            int i;
                            i=base_gps_pos.lat/100;
                            lat1=i+(base_gps_pos.lat-i*100)/60+0.000005;
                            i=base_gps_pos.lng/100;
                            lng1=i+(base_gps_pos.lng-i*100)/60+0.000005;

                            if(base_gps_pos.c_north=='S')
                            {
                                c_n='-';
                            }
                            if(base_gps_pos.c_east=='W')
                            {
                                c_e='-';
                            }

                            utc_date[10]=0;
                            utc_date[9]=utc_date[5];
                            utc_date[8]=utc_date[4];
                            utc_date[7]='0';
                            utc_date[6]='2';
                            utc_date[5]='/';
                            utc_date[4]=utc_date[3];
                            utc_date[3]=utc_date[2];
                            utc_date[2]='/';
                            utc_time[8]=0;
                            utc_time[7]=utc_time[5];
                            utc_time[6]=utc_time[4];
                            utc_time[5]=':';
                            utc_time[4]=utc_time[3];
                            utc_time[3]=utc_time[2];
                            utc_time[2]=':';
                            utc_time[8]=0;


                            sfp=NULL;
                            sfp =fopen(gpsstatus_file,"w");
                            if(sfp!=NULL) 
                            {
                                fprintf(sfp,"<tr><td>Satellites In View:</td><td>%s</td></tr>\n",s_view_num);
                                fprintf(sfp,"<tr><td>Satellites tracked:</td><td>%s</td></tr>\n",s_tr_num);
                                fprintf(sfp,"<tr><td>Latitude:</td><td>%c%.6f,%c</td></tr>\n",c_n,lat1,base_gps_pos.c_north);
                                fprintf(sfp,"<tr><td>Longitude:</td><td>%c%.6f,%c</td></tr>\n",c_e,lng1,base_gps_pos.c_east);
                                fprintf(sfp,"<tr><td>Altitude:</td><td>%s</td></tr>\n",str_pos_alt);
                                fprintf(sfp,"<tr><td>Speed:</td><td>%d(Km/h)</td></tr>\n",last_record_speed);
                                fprintf(sfp,"<tr><td>Orientation:</td><td>%d(Degree to North)</td></tr>\n",last_record_direction);
                                fprintf(sfp,"<tr><td>NMEA UTC Time:</td><td>%s &nbsp;%s</td></tr>\n",utc_date,utc_time);
                                fflush(sfp);
                                fclose(sfp);
                            }
                        }
                    shutdown(sock,SHUT_RDWR);
                    close(sock);
                    return 0;
                }
            }else
            {
                shutdown(sock,SHUT_RDWR);
                close(sock);
                sock=0;
                uci_free_context(ctx);
                sleep(5);
                time_cnt+=5;
                need_re_initsys=1;
                goto check_IORSSI;
            }
            //OK
            gpsd_conn_status=1;
        }


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
            time_cnt+=5;
            goto re_initsys;
        }
        sleep(1);
        time_cnt++;
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
                time_cnt+=5;
                goto re_initsys;
            }
        }else
        {
            shutdown(sock,SHUT_RDWR);
            close(sock);
            sock=0;
            uci_free_context(ctx);
            sleep(5);
            time_cnt+=5;
            goto re_initsys;
        }


        //Date,Time,Lat,N/S,Lon,W/E,Alt,DI,DO,Speed km/h,Direction degree,RSSI
        //Time difference replace,Lat difference replace,Lon difference replace,alt,speed, Direction
        //#Local Date,Time,DI,DO,RSSI	-----if there is no satellite data and IO changed / system restart/reset(wait 1~2 minutes). 

        utc_time[0]=0;
        utc_date[0]=0;
        now_pos_lat[0]=0;
        now_pos_lat_NS=0;
        now_pos_lng[0]=0;
        now_pos_lng_WE=0;
        str_pos_alt[0]=0;
        now_record_speed=-1;
        now_record_direction=-361;

        if(c_Pos_Record_ALT=='B')
        {
            p=NULL;
            p=strstr(buf,"$GPGGA");
            if(p!=NULL)
            {
                while(*p!=',')p++;//1
                p++;
                p1=p;
                while(*p!=',')p++;//2
                i=p-p1;
                if(i>5 && i<11)i=6;
                else i=0;
                strncpy(utc_time,p1,i);
                utc_time[i]=0;
                p++;
                p1=p;
                while(*p!=',')p++;//3
                i=p-p1;
                strncpy(now_pos_lat,p1,i);
                now_pos_lat[i]=0;
                p++;
                p1=p;
                while(*p!=',')p++;//4
                i=p-p1;
                if(i==1)now_pos_lat_NS=*p1;
                p++;
                p1=p;
                while(*p!=',')p++;//5
                i=p-p1;
                strncpy(now_pos_lng,p1,i);
                now_pos_lng[i]=0;
                p++;
                p1=p;
                while(*p!=',')p++;//6
                i=p-p1;
                if(i==1)now_pos_lng_WE=*p1;
    
                p++;
                if(*p=='1' || *p=='2')valid_c='1';
                while(*p!=',')p++;//7: 1,
                p++;
                while(*p!=',')p++;//8
                p++;
                while(*p!=',')p++;//9
                p++;
                p1=p;
                while(*p!=',')p++;//10
                i=p-p1;
                strncpy(str_pos_alt,p1,i);//alt: HHH.h,
                str_pos_alt[i]=0;
                if(i>1)
                {
                    p=NULL;
                    p=strchr(str_pos_alt,'.');
                    if(p!=NULL)*p=0;
                }
            }
        }

        p=NULL;
        p=strstr(buf,"$GPRMC");
        if(p!=NULL)
        {
            while(*p!=',')p++;//1
            p++;
            p1=p;
            while(*p!=',')p++;//2
            i=p-p1;
            if(i>5 && i<11)i=6;
            else i=0;
            strncpy(utc_time,p1,i);
            utc_time[i]=0;
            p++;
            if(*p=='A')valid_c='1';
            while(*p!=',')p++;//3 ,
            p++;
            p1=p;
            while(*p!=',')p++;//4
            i=p-p1;
            strncpy(now_pos_lat,p1,i);
            now_pos_lat[i]=0;
            p++;
            p1=p;
            while(*p!=',')p++;//5
            i=p-p1;
            if(i==1)now_pos_lat_NS=*p1;
            p++;
            p1=p;
            while(*p!=',')p++;//6
            i=p-p1;
            strncpy(now_pos_lng,p1,i);
            now_pos_lng[i]=0;
            p++;
            p1=p;
            while(*p!=',')p++;//7
            i=p-p1;
            if(i==1)now_pos_lng_WE=*p1;
            p++;
            p1=p;
            while(*p!=',')p++;//8
            i=p-p1;
            if(i>0)
            {
                strncpy(tmp_buff,p1,i);
                tmp_buff[i]=0;
                sscanf(tmp_buff, "%f", &(tmp_f));
                tmp_f=1.852*tmp_f+0.5;
                now_record_speed=(int)tmp_f;
            }
            p++;
            p1=p;
            while(*p!=',')p++;//9
            i=p-p1;
            if(i>0)
            {
                strncpy(tmp_buff,p1,i);
                tmp_buff[i]=0;
                sscanf(tmp_buff, "%f", &(tmp_f));
                tmp_f=tmp_f+0.5;
                now_record_direction=(int)tmp_f;
                if(now_record_direction<0)now_record_direction+=360;
            }
            p++;
            p1=p;
            while(*p!=',')p++;//10
            i=p-p1;
            if(i==6)
            {
                strncpy(utc_date,p1,i);
                utc_date[i]=0;
            }
        }


        if(wait_valid_psdata==1 && full_record_sign==1)
        {
            if(utc_date[0]!=0 && valid_c=='1') 
            {
                if(strlen(now_pos_lat)>2) {
                    save_new_record_state='1';
                }
            }
        }

check_IORSSI:
        if(time_cnt>=i_Pos_Record_INTERVAL)
        {
            save_new_record_state='1';
            time_cnt=0;
        }

        if(c_Pos_Record_IO=='B')
        {
            get_iostatus_buff();
            if(strcmp(last_str_iostatus_buff,str_iostatus_buff)!=0)
            {
                save_new_record_state='1';
                save_rec_file_state='1';
                full_record_sign=1;
            }
        }
        if(c_Pos_Record_RSSI=='B')
        {
            get_rssi_buff();
            if(now_record_rssi!=0) 
            {
                i=0;
                if(last_record_rssi>LOW_RSSI_LEVEL && now_record_rssi<BACKUP_RSSI_LEVEL)i=1;
                if(last_record_rssi<BACKUP_RSSI_LEVEL && now_record_rssi>LOW_RSSI_LEVEL)i=1;
                if(i==1)
                {
                    rssi_state_chg_count++;
                    if(save_new_record_state=='1')  //could be confirm one time.
                    {
                        save_rec_file_state='1';
                        full_record_sign=1;
                        rssi_state_chg_count=0;
                    }
                }
                else rssi_state_chg_count=0;

                if(rssi_state_chg_count>MAX_RSSI_CHG_TIME)
                {
                    save_new_record_state='1';
                    save_rec_file_state='1';
                    full_record_sign=1;
                    rssi_state_chg_count=0;
                }
            }
        }

        if(c_Pos_Record_SPEED=='B')
        {
            if(valid_c=='1' && now_record_speed>=0)
            {
                i=0;
                if(over_speed_multi_sign==0)
                {
                    if(now_record_speed>i_Pos_Record_OVERSPEED)i=1;
                    if(last_record_speed<SPEED_START && now_record_speed>(SPEED_START+SPEED_CHG_BASE))i=1;
                    if(last_record_speed>(SPEED_START+SPEED_CHG_BASE) && now_record_speed<SPEED_START)i=1;
                }else
                {
                    if(now_record_speed<i_Pos_Record_OVERSPEED-SPEED_CHG_BASE && last_record_speed>=i_Pos_Record_OVERSPEED)i=1;
                    if(now_record_speed>=i_Pos_Record_OVERSPEED+over_speed_multi_sign*SPEED_CHG_BASE)i=1;
                }

                if(i==1)
                {
                    speed_state_chg_count++;
                }
                else speed_state_chg_count=0;

                if(speed_state_chg_count>MAX_SPEED_CHG_TIME)
                {
                    save_new_record_state='1';
                    speed_state_chg_count=0;
                    save_rec_file_state='1';
                    if(over_speed_multi_sign==0)
                    {
                        if(now_record_speed>i_Pos_Record_OVERSPEED)over_speed_multi_sign=1;
                    }else
                    {
                        if(now_record_speed<i_Pos_Record_OVERSPEED-SPEED_CHG_BASE)over_speed_multi_sign=0;
                        if(now_record_speed>=i_Pos_Record_OVERSPEED+SPEED_CHG_BASE) 
                        {
                            int i_muti=(now_record_speed-i_Pos_Record_OVERSPEED)/SPEED_CHG_BASE+1;
                            if(i_muti>over_speed_multi_sign)over_speed_multi_sign=i_muti;
                        }
                    }
                }
            }
        }

        if(c_Pos_Record_ORIENT=='B') 
        {
            if(i_Pos_Record_ORIENTCHG<180)  //180:disable
            {
                if(valid_c=='1' && now_record_direction>=0)
                {
                    if(now_record_speed>=ORIENT_CHK_BASESPEED)
                    {
                        i=now_record_direction-last_record_direction;
                        if(i<0)i=(-1)*i;
                        if(i>180)i=360-i;
                        if(i>=i_Pos_Record_ORIENTCHG)
                        {
                            direction_state_chg_count++;
                        }else direction_state_chg_count=0;
    
                        if(direction_state_chg_count>=MAX_DIRECTION_CHG_TIME)
                        {
                            save_new_record_state='1';
                            direction_state_chg_count=0;
                        }
                    }else direction_state_chg_count=0;
                }
            }
        }

        //save record.
        tmp_buff[0]=0;
        fsize=0;
        if(save_new_record_state=='1')
        {
            time_diff[5]=0;
            lat_diff[6]=0;
            lng_diff[6]=0;
            time_diff[0]=0;
            lat_diff[0]=0;
            lng_diff[0]=0;

            if(valid_c=='1') 
            {
                if( now_pos_lat[0]==0 || now_pos_lng[0]==0 || utc_date[0]==0 || utc_time[0]==0)continue; 

                if(full_record_sign==0)
                {
                    if(now_pos_lat_NS!=last_full_pos_lat_NS || now_pos_lng_WE!=last_full_pos_lng_WE)
                    {
                        full_record_sign=1;
                        goto save_next_step;
                    }
                    if(utc_time[0]!=last_full_utc_time[0]) 
                    {
                        full_record_sign=1;
                        goto save_next_step;
                    }
                    if(strcmp(utc_date,last_full_utc_date)!=0)
                    {
                        full_record_sign=1;
                        goto save_next_step;
                    }
                    p=NULL;
                    p=strchr(now_pos_lat,'.');
                    if(p==NULL)continue;
                    if(p!=NULL)
                    {
                        i=(p-now_pos_lat)-1;
                        if(i>0)
                        {
                            if(strncmp(last_full_pos_lat,now_pos_lat,i)!=0)
                            {
                                full_record_sign=1;
                                goto save_next_step;
                            }else
                            {
                                if(*(last_full_pos_lat+i)!=*(now_pos_lat+i))
                                {
                                    lat_diff[0]=*(now_pos_lat+i);
                                    strncpy(lat_diff+1,now_pos_lat+i+2,5);
                                    lat_diff[6]=0;
                                }else
                                {
                                    i+=2;
                                    j=i+5;
                                    while(*(last_full_pos_lat+i)==*(now_pos_lat+i) && i<j)i++;
                                    j=0;
                                    while(now_pos_lat[i]!=0)
                                    {
                                        lat_diff[j]=now_pos_lat[i];
                                        i++;
                                        j++;
                                    }
                                    lat_diff[j]=0;
                                }
                            }
                        }
                    }

                    p=NULL;
                    p=strchr(now_pos_lng,'.');
                    if(p==NULL)continue;
                    if(p!=NULL)
                    {
                        i=(p-now_pos_lng)-1;
                        if(i>0)
                        {
                            if(strncmp(last_full_pos_lng,now_pos_lng,i)!=0)
                            {
                                full_record_sign=1;
                                goto save_next_step;
                            }else
                            {
                                if(*(last_full_pos_lng+i)!=*(now_pos_lng+i))
                                {
                                    lng_diff[0]=*(now_pos_lng+i);
                                    strncpy(lng_diff+1,now_pos_lng+i+2,5);
                                    lng_diff[6]=0;
                                }else
                                {
                                    i+=2;
                                    j=i+5;
                                    while(*(last_full_pos_lng+i)==*(now_pos_lng+i) && i<j)i++;
                                    j=0;
                                    while(now_pos_lng[i]!=0)
                                    {
                                        lng_diff[j]=now_pos_lng[i];
                                        i++;
                                        j++;
                                    }
                                    lng_diff[j]=0;
                                }
                            }
                        }
                    }
                }
save_next_step:
        //Date,Time,Lat,N/S,Lon,W/E,Alt,DI,DO,Speed km/h,Direction degree,RSSI
        //Time difference replace,Lat difference replace,Lon difference replace,alt,speed, Direction
        //#Local Date,Time,DI,DO,RSSI	-----if there is no satellite data and IO changed / system restart/reset(wait 1~2 minutes). 

                if(full_record_sign==1)//save a full record
                {
                    sprintf(tmp_buff,"%s,%s,%s,%c,%s,%c,",utc_date,utc_time,now_pos_lat,now_pos_lat_NS,now_pos_lng,now_pos_lng_WE);
                    if(c_Pos_Record_ALT=='B' && str_pos_alt[0]!=0)strcat(tmp_buff,str_pos_alt);
                    strcat(tmp_buff,",");
                    if(c_Pos_Record_IO=='B')strcat(tmp_buff,str_iostatus_buff);
                    strcat(tmp_buff,",");
                    if(c_Pos_Record_SPEED=='B' && now_record_speed>=0)
                    {
                        sprintf(tmp_buff1,"%d",now_record_speed);
                        strcat(tmp_buff,tmp_buff1);
                    }
                    strcat(tmp_buff,",");

                    if(c_Pos_Record_ORIENT=='B' && now_record_direction>=0 && now_record_speed>0)
                    {
                        sprintf(tmp_buff1,"%d",now_record_direction);
                        strcat(tmp_buff,tmp_buff1);
                    }
                    strcat(tmp_buff,",");

                    if(c_Pos_Record_RSSI=='B' && now_record_rssi!=0)
                    {
                        i=now_record_rssi;
                        sprintf(tmp_buff1,"%d",i);
                        strcat(tmp_buff,tmp_buff1);
                    }
                    strcat(tmp_buff,"\n");
                    sfp=NULL;
                    sfp =fopen(gpsrecord_varfile,"a");  
                    if(sfp==NULL)continue;
                    fprintf(sfp,tmp_buff);      
                    save_to_last_record("tpisdr");
                    full_record_sign=0;
                    rssi_state_chg_count=0;
                    speed_state_chg_count=0;
                    direction_state_chg_count=0;
                }else
                {
                    if(strcmp(utc_date,last_full_utc_date)!=0)continue;
                    if(utc_time[0]!=last_full_utc_time[0])continue;

                    i=1;
                    while(last_full_utc_time[i]==utc_time[i])i++;
                    j=0;
                    while(i<6)
                    {
                        time_diff[j]=utc_time[i];
                        j++;
                        i++;
                    }
                    time_diff[j]=0;
                    if(j==0)continue;

                    //Time difference replace,Lat difference replace,Lon difference replace,alt,speed, Direction
                    sprintf(tmp_buff,"%s,%s,%s,",time_diff,lat_diff,lng_diff);
                    if(c_Pos_Record_ALT=='B' && str_pos_alt[0]!=0)strcat(tmp_buff,str_pos_alt);
                    strcat(tmp_buff,",");
                    if(c_Pos_Record_SPEED=='B' && now_record_speed>=0)
                    {
                        sprintf(tmp_buff1,"%d",now_record_speed);
                        strcat(tmp_buff,tmp_buff1);
                    }
                    strcat(tmp_buff,",");
                    if(c_Pos_Record_ORIENT=='B' && now_record_direction>=0 && now_record_speed>0)
                    {
                        sprintf(tmp_buff1,"%d",now_record_direction);
                        strcat(tmp_buff,tmp_buff1);
                    }
                    strcat(tmp_buff,"\n");
                    sfp=NULL;
                    sfp =fopen(gpsrecord_varfile,"a");  
                    if(sfp==NULL)continue;
                    fprintf(sfp,tmp_buff);      
                    save_to_last_record("psd");
                    speed_state_chg_count=0;
                    direction_state_chg_count=0;
                }
                wait_valid_psdata=0;
            }else
            {
                //#Local Date,Time,DI,DO,RSSI	-----if there is no satellite data and IO changed / system restart/reset(wait 1~2 minutes). 
                //only rssi and io and local time record
                if(c_Pos_Record_IO=='B' || c_Pos_Record_RSSI=='B')
                {
                    localtime_t = time (NULL);
                    sprintf(tmp_buff,"#%ld,",localtime_t);
                    if(c_Pos_Record_IO=='B')strcat(tmp_buff,str_iostatus_buff);
                    strcat(tmp_buff,",");
                    if(c_Pos_Record_RSSI=='B' && now_record_rssi!=0)
                    {
                        i=now_record_rssi;
                        sprintf(tmp_buff1,"%d",i);
                        strcat(tmp_buff,tmp_buff1);
                    }
                    strcat(tmp_buff,"\n");
                    sfp=NULL;
                    sfp =fopen(gpsrecord_varfile,"a");  
                    if(sfp==NULL)continue;
                    fprintf(sfp,tmp_buff);      
                    save_to_last_record("ir");
                }
                full_record_sign=1; //notice next time to full record
                wait_valid_psdata=1;
            }

            if (-1==fseek(sfp,0L,SEEK_END))return false;
            fsize=ftell(sfp);
            if(fsize>16280)save_rec_file_state='1';
            fclose(sfp);    
              
            save_file_cnt++;
            if(save_file_cnt>i_MAX_RECITEMS_IN_MEM)save_rec_file_state='1';
            save_new_record_state='0';
            time_cnt=0;
        }

        if(save_rec_file_state=='1')
        {
            save_file_cnt=0;
            save_rec_file_state='0';
            sprintf(tmp_buff,"%s %s%d",gpsrecord_varfile,gpsrecord_dir,gpsrecord_file_index);
            if(SubProgram_Start("/bin/cp",tmp_buff)==false)
            {
                syslog(LOGOPTS,"GPS record%d save error\n", gpsrecord_file_index);
                return 0;
            }

            if(fsize>16280)
            {
                gpsrecord_file_index++;
                if(gpsrecord_file_index>=MAX_REC_FILE_NUM)gpsrecord_file_index=0;

                if(SubProgram_Start("/bin/rm",gpsrecord_varfile)==false)
                {
                    syslog(LOGOPTS,"GPS record%d new error\n", gpsrecord_file_index);
                    return 0;
                }

                full_record_sign=1;
                fsize=0;
            }
        }
    }
    
}
