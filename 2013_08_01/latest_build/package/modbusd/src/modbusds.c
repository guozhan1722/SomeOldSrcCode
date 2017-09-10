#include "modbusds.h"
#include "datamap.h"

static struct uci_context *ctx = NULL;   //need to initialize in main()
static int ctx_fresh_cnt=0;

char *devCom  = "/dev/ttyS0"; //com2 for VIP4G  
#define COM1 "/dev/ttyUSB0"
#define COM2 "/dev/ttyS0"
//char *devCom1 = "/dev/ttyS1"; //com1  
//char *devCom2  = "/dev/ttyS0"; //com2   
//char *COM12_statistic_file;
//char *COM1_statistic_file ="/var/run/com1_packet";
//char *COM2_statistic_file ="/var/run/com2_packet";


static char Endian_sign;//B/L   big/litter.  
//Modbus: data and address: High byte go first(Big Endian); CRC:Low byte go first(Little Endian)


static int current_tcp_client_num;
static int remote_tcp_client_sockfd[REMOTE_TCP_MAX_NUMBER];
static int remote_tcp_client_idleseconds[REMOTE_TCP_MAX_NUMBER];
static char remote_tcp_client_status[REMOTE_TCP_MAX_NUMBER];//exchange signal between thread and management.
static pthread_t remote_tcp_client_pt[REMOTE_TCP_MAX_NUMBER];
static char* p_read_buff_tcp[REMOTE_TCP_MAX_NUMBER];
static char* p_send_buff_tcp[REMOTE_TCP_MAX_NUMBER];
static int read_buff_len_tcp[REMOTE_TCP_MAX_NUMBER];

static char DataMap_C[MODBUS_DMAP_C_MAX+1];
static char DataMap_I[MODBUS_DMAP_I_MAX+1];
static char DataMap_R[MODBUS_DMAP_R_MAX*2+1];
static char DataMapFresh_C[MODBUS_DMAP_C_MAX];//main: memset->0; thread:if 0 need to update data,if 1 neednot to update and can read it directly 
static char DataMapFresh_I[MODBUS_DMAP_I_MAX];
static char DataMapFresh_R[MODBUS_DMAP_R_MAX];
static char DataMapFreshSet_C[MODBUS_DMAP_C_MAX];//switch to init data and refresh. 
static char DataMapFreshSet_I[MODBUS_DMAP_I_MAX];
static char DataMapFreshSet_R[MODBUS_DMAP_R_MAX];
static char DataLock[MODBUS_DMAP_TYPE_MAX];//to avoid two thread fresh at the same time: 01,02,03
static char DataLockWho[MODBUS_DMAP_TYPE_MAX];// default value =99;

static char tcp_slave_enable;
static int tcp_idletimeout_max;
static int tcp_slave_port;
static char tcp_slave_id;
static int tcp_offset_c;
static int tcp_offset_i;
static int tcp_offset_r;
static char tcp_IPfilter_en;
static in_addr_t tcp_IPFilter[4];

void tcp_slave_function();
void *tcp_connect_thread_function(void *arg);
void *com_slave_thread_function(void *arg);

static pthread_t fresh_data_pt;
static pthread_t com_slave_pt;
static char read_buff_com[RECV_BUFF_LEN_MAX+1];
static char send_buff_com[SEND_BUFF_LEN_MAX+1];
static int send_buff_len_com;
static char *p_read_buff_com_end;//only by read
static char com_slave_enable;
static char com_slave_dmod;
static char com_slave_baudrate;
static char com_slave_fmt;
static char com_slave_ctimeout;
static char com_slave_id;
static int com_offset_c;
static int com_offset_i;
static int com_offset_r;
static int fdcom;
static char com_port_id;//com1:1, com2:2
static char com_modbus_type;//2:RTU;1:ASCII
static int one_master_datapack; //9 or 17
static int fresh_timeout_max_com=0;//max 300seconds, need refresh all data.

static struct ifreq ifr;

int local_as_server_sockfd, remote_as_client_sockfd;
int sin_size;
int on=1, off=0;
struct sockaddr_in local_as_server_addr;         
fd_set my_readfd; 
int select_return;


uint16_t htons_rv16(uint16_t t_i)
{
    union __hton16_struct tmp;
    uint16_t i_tmp;
    char c_tmp;
    tmp.u_i=t_i;
    c_tmp=tmp.u_c[0];
    tmp.u_c[0]=tmp.u_c[1];
    tmp.u_c[1]=c_tmp;
    i_tmp=tmp.u_i;
    return i_tmp;
}

uint32_t htons_rv32(uint32_t t_i)
{
    union __hton32_struct tmp;
    uint32_t i_tmp;
    char c_tmp;
    tmp.u_i=t_i;

    c_tmp=tmp.u_c[0];
    tmp.u_c[0]=tmp.u_c[3];
    tmp.u_c[3]=c_tmp;

    c_tmp=tmp.u_c[1];
    tmp.u_c[1]=tmp.u_c[2];
    tmp.u_c[2]=c_tmp;

    i_tmp=tmp.u_i;
    return i_tmp;
}

int32_t htons_rv32s(int32_t t_i)
{
    union _bsint4 tmp;
    int32_t i_tmp;
    char c_tmp;
    tmp.i=t_i;

    c_tmp=tmp.c[0];
    tmp.c[0]=tmp.c[3];
    tmp.c[3]=c_tmp;

    c_tmp=tmp.c[1];
    tmp.c[1]=tmp.c[2];
    tmp.c[2]=c_tmp;

    i_tmp=tmp.i;
    return i_tmp;
}

uint64_t htons_rv64(uint64_t t_i)
{
    union __hton64_struct tmp;
    uint64_t i_tmp;
    char c_tmp;
    tmp.u_i=t_i;

    c_tmp=tmp.u_c[0];
    tmp.u_c[0]=tmp.u_c[7];
    tmp.u_c[7]=c_tmp;

    c_tmp=tmp.u_c[1];
    tmp.u_c[1]=tmp.u_c[6];
    tmp.u_c[6]=c_tmp;

    c_tmp=tmp.u_c[2];
    tmp.u_c[2]=tmp.u_c[5];
    tmp.u_c[5]=c_tmp;

    c_tmp=tmp.u_c[3];
    tmp.u_c[3]=tmp.u_c[4];
    tmp.u_c[4]=c_tmp;

    i_tmp=tmp.u_i;
    return i_tmp;
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


char getdatabit(char *cdata,unsigned int iaddr) //i:0-...
{
    //from low to high: 0-7
    char d;
    unsigned int i_Byte,i_bit;
    i_Byte=iaddr>>3;
    i_bit=iaddr&(0x07);
    d=*(cdata+i_Byte);
    d=(d>>i_bit)&(0x01);
    if(d>0)return 1;
    else return 0;
}

//for coil and input data pack process
bool setdatabit(char *cdata,unsigned int iaddr,char bvalue)
{
    char d,chbit;
    char i_Byte,i_bit;
    i_Byte=iaddr>>3;
    i_bit=iaddr&(0x07);
    d=*(cdata+i_Byte);
    chbit=0x01;
    chbit=chbit<<i_bit;
    if(bvalue==1)
    {
        d = d | chbit;
    }else
    {
        chbit=~chbit;
        d = d & chbit;
    }
    *(cdata+i_Byte)=d;
//    printf("\nset bit:%d %x => %x",iaddr,bvalue,d);

    return true;
}


//0-open; 1-close
int get_input_status(char *result)
{
    int i;
    char tmp_buff[20];
    char tmp_result[5];
    for(i=0;i<4;i++) 
    {
        tmp_result[0]=0;
        sprintf(tmp_buff,"input%d",i+1);
        get_option_by_section_name(ctx,"ioports","input",tmp_buff,tmp_result);
        if(tmp_result[0]=='1')*(result+i)=1;
        else *(result+i)=0;
    }
    *(result+4)=0;
    *(result+5)=0;
    *(result+6)=0;
    *(result+7)=0;

    return 0;
}


int set_output_status(int o_i,char result)
{
    if(o_i<0 || o_i>3)return -1;
    if(result!=1 && result!=0)return -1;
    o_i=o_i+1;
    char tmp_buff[20];
    sprintf(tmp_buff,"output%d",o_i);
    set_option_by_section_name(ctx,"ioports","output",tmp_buff,result);
    SubProgram_Start("/etc/init.d/ioports","stop");
    SubProgram_Start("/etc/init.d/ioports","start &");
    return 0;
}

int get_output_status(char *result)
{
    int i;
    char tmp_buff[20];
    char tmp_result[5];
    for(i=0;i<4;i++) 
    {
        tmp_result[0]=0;
        sprintf(tmp_buff,"output%d",i+1);
        get_option_by_section_name(ctx,"ioports","output",tmp_buff,tmp_result);
        if(tmp_result[0]=='1')*(result+i)=1;
        else *(result+i)=0;
    }
    *(result+4)=0;
    *(result+5)=0;
    *(result+6)=0;
    *(result+7)=0;

    return 0;

}


static int mdio_read(int skfd, int location)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    mii->reg_num = location;
    if (ioctl(skfd, SIOCGMIIREG, &ifr) >= 0) 
    {
        return mii->val_out;
    }
    return 0;
}

static int get_eth_link_status(char *ifname)
{

    char tmp_buff[50];
    FILE* f;
    sprintf(tmp_buff,"/sys/class/net/%s/carrier",ifname);
    int result=0;

    if (!(f = fopen(tmp_buff, "r"))) 
    {
        return 0;
        //do nothing.
    }else
    {
        if(fgets(tmp_buff, 40, f)>0)
        {
            if(tmp_buff[0]=='1')result=1;
        }
    }
    fclose(f);
    return result;
}

int set_com_status(int comid,char bvalue)
{
    char s_value[5];
    s_value[1]=0;
    if(bvalue==1)s_value[0]='B';
    else if(bvalue==0)s_value[0]='A';
    else return -1;
    if(comid!=2)return -1;

    set_option_by_section_name(ctx,"comport2","com2","COM2_Port_Status",s_value);
    SubProgram_Start("/etc/init.d/soip2","restart &");
    return 0;
}


int set_wifi_status(int wifiid,char bvalue)
{
    char s_value[5];
    s_value[1]=0;
    if(bvalue==1)s_value[0]='0';
    else if(bvalue==0)s_value[0]='1';
    else return -1;
    if(wifiid!=0)return -1;

    set_option_by_section_name(ctx,"wireless","radio0","disabled",s_value);
    SubProgram_Start("/etc/init.d/vlan","stop");
    SubProgram_Start("/etc/init.d/vlan","start &");
    return 0;
}


int set_carr_status(int carrid,char bvalue)
{
    char s_value[5];
    s_value[1]=0;
    if(bvalue==1)s_value[0]='0';
    else if(bvalue==0)s_value[0]='1';
    else return -1;
    if(carrid!=0)return -1;

    set_option_by_section_name(ctx,"lte","connect","disabled",s_value);
    SubProgram_Start("/etc/init.d/lte","stop");
    SubProgram_Start("/etc/init.d/lte","start &");
    return 0;
}


int set_fw_status(char bvalue)
{
    char s_value[5];
    s_value[1]=0;
    if(bvalue==1)s_value[0]='0';
    else if(bvalue==0)s_value[0]='1';
    else return -1;

    set_option_by_section_name(ctx,"firewall","normal","disabled_firewall",s_value);
    SubProgram_Start("/etc/init.d/firewall","stop");
    SubProgram_Start("/etc/init.d/firewall","start &");
    return 0;
}


int set_eurd_status(int eurid,char bvalue)
{
    char c_value;
    if(eurid<0 || eurid>3)return -1; 
    if(bvalue==1)c_value='D';
    else if(bvalue==0)c_value='A';
    else return -1;
    char buff[10];
    get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
    buff[eurid]=c_value;
    set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
    SubProgram_Start("/etc/init.d/eurd","stop");
    SubProgram_Start("/etc/init.d/eurd","start &");
    return 0;
}


int set_gpsd_status(char bvalue)
{
    char s_value[5];
    s_value[1]=0;
    if(bvalue==1)s_value[0]='1';
    else if(bvalue==0)s_value[0]='0';
    else return -1;
    set_option_by_section_name(ctx,"gpsd","gpsd_conf","status",s_value);

    SubProgram_Start("/etc/init.d/gpsd","stop");
    SubProgram_Start("/etc/init.d/gpsd","start");
    SubProgram_Start("/etc/init.d/gpsr","stop");
    SubProgram_Start("/etc/init.d/gpsr","start &");

    return 0;
}

int set_locnet_status(char bvalue)
{

    char s_value[5];
    s_value[1]=0;
    if(bvalue==1)s_value[0]='B';
    else if(bvalue==0)s_value[0]='A';
    else return -1;
    set_option_by_section_name(ctx,"eurd","eur_conf","Cell_Loc_Network",s_value);
    //    SubProgram_Start("/etc/init.d/eurd","stop");
    //    SubProgram_Start("/etc/init.d/eurd","start");
    return 0;
}

int set_web_status(char bvalue)
{
    char s_value[16];
    if(bvalue==1)strcpy(s_value,"enable");
    else if(bvalue==0)strcpy(s_value,"disable");
    else return -1;

    set_option_by_section_name(ctx, "wsclient", "general", "status", s_value);
    SubProgram_Start("/etc/init.d/eurd","wsClient");
    SubProgram_Start("/etc/init.d/eurd","wsClient &");
    return 0;
}



int get_gps_data_ll(char *lat, char *lng , char *alt)
{

    char tmpbuf[256];
    FILE* f;
    char* p;
    int j;

    *lat=0;
    *lng=0;
    *alt=0;

    if (!(f = fopen("/tmp/run/GPS_position", "r"))) 
    {
        return 0;
        //do nothing.
    }else
    {
        while (fgets(tmpbuf, 256, f)) 
        {
            p=NULL;
            p = strstr(tmpbuf, "latitude=");//latitude="51.142962"
            if (p != NULL)
            {
                p+=strlen("latitude=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        lat[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0) 
                    {
                        lat[j]=0;
                    }
            }//if p

            p=NULL;
            p = strstr(tmpbuf, "longitude=");//longitude="-114.075094"
            if (p != NULL)
            {
                p+=strlen("longitude=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        lng[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0) 
                    {
                        lng[j]=0;
                    }
            }//if p

            p=NULL;
            p = strstr(tmpbuf, "elevation=");
            if (p != NULL)
            {
                p+=strlen("elevation=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        alt[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0) 
                    {
                        alt[j]=0;
                    }
            }//if p

        }//while
    }//else
    if(f)fclose(f);
    if(strlen(lat)<7 || strlen(lng)<7)
    {
        *lat=0;
        *lng=0;
        *alt=0;
        return 0;
    }

    p=NULL;
    p=strchr(lat,'.');
    if(p!=NULL)
    {
        for(j=0;j<6;j++)
        {
            if(*(p+1)>'9' || *(p+1)<'0')*p='0';
            else *p=*(p+1);
            p++;
        }
        *p=0;
    }

    p=NULL;
    p=strchr(lng,'.');
    if(p!=NULL)
    {
        for(j=0;j<6;j++)
        {
            if(*(p+1)>'9' || *(p+1)<'0')*p='0';
            else *p=*(p+1);
            p++;
        }
        *p=0;
    }

    p=NULL;
    p=strchr(alt,'.');
    if(p!=NULL)
    {
        for(j=0;j<6;j++)
        {
            *p=0;
            p++;
        }
        *p=0;
    }

    return 1;
}


bool Fresh_Datamap_C(char mode,unsigned int iaddr,char *bvalue)//mode:0 only read; 1 write and read.
{
    if(iaddr<0 || iaddr>=MODBUS_DMAP_C_MAX)return false;
    char c_value;
    char s_value[10];
    int i;
    if(mode==2 || mode==3)
    {
        *bvalue=DataMap_C[iaddr];
        if(DataMapFresh_C[iaddr]==0&&mode==2)DataMapFreshSet_C[iaddr]=3;
        return true;
    }

    if(mode==1)
    {
        c_value=*bvalue;
        if(c_value!=0 && c_value!=1)return false;
        DataMapFreshSet_C[iaddr]=3;

        switch(iaddr)
        {
        case MODBUS_DMAP_C_O1:
        case MODBUS_DMAP_C_O2:
        case MODBUS_DMAP_C_O3:
        case MODBUS_DMAP_C_O4:
            set_output_status(iaddr-MODBUS_DMAP_C_O1,c_value);
            ctx_fresh_cnt+=2;
            DataMapFresh_C[iaddr]=0;
            return true;
        case MODBUS_DMAP_C_O5:
        case MODBUS_DMAP_C_O6:
        case MODBUS_DMAP_C_O7:
        case MODBUS_DMAP_C_O8:
            //do nothing
            return true;
        case MODBUS_DMAP_C_COM1:
            //set_com_status(1,c_value);
            //DataMapFresh_C[iaddr]=0;
            return true;
        case MODBUS_DMAP_C_COM2:
            set_com_status(2,c_value);
            DataMapFresh_C[iaddr]=0;
            return true;
        case MODBUS_DMAP_C_COM3:
        case MODBUS_DMAP_C_COM4:
            return true;
        case MODBUS_DMAP_C_ETH0:
        case MODBUS_DMAP_C_ETH1:
        case MODBUS_DMAP_C_ETH2:
        case MODBUS_DMAP_C_ETH3:
            return true;
        case MODBUS_DMAP_C_CARR0:
            set_carr_status(iaddr-MODBUS_DMAP_C_CARR0,c_value);
            DataMapFresh_C[iaddr]=0;
            return true;
        case MODBUS_DMAP_C_CARR1:
            return true;
        case MODBUS_DMAP_C_WIFI0:
            set_wifi_status(iaddr-MODBUS_DMAP_C_WIFI0,c_value);
            DataMapFresh_C[iaddr]=0;
            return true;
        case MODBUS_DMAP_C_WIFI1:
            return true;
        case MODBUS_DMAP_C_USB0:
        case MODBUS_DMAP_C_USB1:
            return true;
        case MODBUS_DMAP_C_GPSD:
            set_gpsd_status(c_value);
            DataMapFresh_C[iaddr]=0;
            return true;
        case MODBUS_DMAP_C_LOCNET:
            set_locnet_status(c_value);
            DataMapFresh_C[iaddr]=0;
            return true;
        case MODBUS_DMAP_C_EUR1:
        case MODBUS_DMAP_C_EUR2:
        case MODBUS_DMAP_C_EUR3:
        case MODBUS_DMAP_C_NMS:
            set_eurd_status(iaddr-MODBUS_DMAP_C_EUR1,c_value);
            DataMapFresh_C[MODBUS_DMAP_C_EUR1]=0;
            DataMapFresh_C[MODBUS_DMAP_C_EUR2]=0;
            DataMapFresh_C[MODBUS_DMAP_C_EUR3]=0;
            DataMapFresh_C[MODBUS_DMAP_C_NMS]=0;
            return true;
        case MODBUS_DMAP_C_WEB:
            set_web_status(c_value);
            DataMapFresh_C[iaddr]=0;
            return true;
        case MODBUS_DMAP_C_FWL:
            set_fw_status(c_value);
            DataMapFresh_C[iaddr]=0;
            return true;
        case MODBUS_DMAP_C_REBT:
            if(c_value==1)
            {
                SubProgram_Start("/sbin/reboot"," &");
            }
            return true;
        }

        return true;
    }


    //mode=0:initial->2 data OK
    DataMapFreshSet_C[iaddr]=2;
    switch(iaddr)
    {
    case MODBUS_DMAP_C_O1:
    case MODBUS_DMAP_C_O2:
    case MODBUS_DMAP_C_O3:
    case MODBUS_DMAP_C_O4:
        if(DataMapFresh_C[iaddr]==0)
        {
            get_output_status(s_value);
            DataMap_C[MODBUS_DMAP_C_O1]=s_value[0];
            DataMap_C[MODBUS_DMAP_C_O2]=s_value[1];
            DataMap_C[MODBUS_DMAP_C_O3]=s_value[2];
            DataMap_C[MODBUS_DMAP_C_O4]=s_value[3];
            DataMapFresh_C[MODBUS_DMAP_C_O1]=2;
            DataMapFresh_C[MODBUS_DMAP_C_O2]=2;
            DataMapFresh_C[MODBUS_DMAP_C_O3]=2;
            DataMapFresh_C[MODBUS_DMAP_C_O4]=2;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_C_O5:
    case MODBUS_DMAP_C_O6:
    case MODBUS_DMAP_C_O7:
    case MODBUS_DMAP_C_O8:
        DataMap_C[iaddr]=0;
        *bvalue=DataMap_C[iaddr];
        DataMapFresh_C[iaddr]=99;
        return true;
    case MODBUS_DMAP_C_COM1:
        DataMap_C[iaddr]=0;
        *bvalue=DataMap_C[iaddr];
        DataMapFresh_C[iaddr]=99;
        return true;
    case MODBUS_DMAP_C_COM2:
        if(DataMapFresh_C[iaddr]==0)
        {
            get_option_by_section_name(ctx,"comport2","com2","COM2_Port_Status",s_value);
            if(s_value[0]=='B')DataMap_C[iaddr]=1;
            else DataMap_C[iaddr]=0;
            DataMapFresh_C[iaddr]=5;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_C_COM3:
        DataMap_C[iaddr]=0;
        *bvalue=DataMap_C[iaddr];
        DataMapFresh_C[iaddr]=99;
        return true;
    case MODBUS_DMAP_C_COM4:
        DataMap_C[iaddr]=0;
        *bvalue=DataMap_C[iaddr];
        DataMapFresh_C[iaddr]=99;
        return true;
    case MODBUS_DMAP_C_ETH0:
        if(DataMapFresh_C[iaddr]==0)
        {
           int link=get_eth_link_status("eth0");
           if(link>0)c_value=1;
           else c_value=0;
           DataMap_C[iaddr]=c_value;
           DataMapFresh_C[iaddr]=5;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_C_ETH1:
        if(DataMapFresh_C[iaddr]==0)
        {
           int link=get_eth_link_status("eth1");
           if(link>0)c_value=1;
           else c_value=0;
           DataMap_C[iaddr]=c_value;
           DataMapFresh_C[iaddr]=5;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_C_ETH2:
        DataMap_C[iaddr]=0;
        *bvalue=DataMap_C[iaddr];
        DataMapFresh_C[iaddr]=99;
        return true;
    case MODBUS_DMAP_C_ETH3:
        DataMap_C[iaddr]=0;
        *bvalue=DataMap_C[iaddr];
        DataMapFresh_C[iaddr]=99;
        return true;
    case MODBUS_DMAP_C_CARR0:
        if(DataMapFresh_C[iaddr]==0)
        {
            get_option_by_section_name(ctx,"lte","connect","disabled",s_value);
            if(s_value[0]=='0')DataMap_C[iaddr]=1;
            else DataMap_C[iaddr]=0;
            DataMapFresh_C[iaddr]=3;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_C_CARR1:
        DataMap_C[iaddr]=0;
        *bvalue=DataMap_C[iaddr];
        DataMapFresh_C[iaddr]=99;
        return true;
    case MODBUS_DMAP_C_WIFI0:
        if(DataMapFresh_C[iaddr]==0)
        {
            get_option_by_section_name(ctx,"wireless","radio0","disabled",s_value);
            if(s_value[0]=='0')DataMap_C[iaddr]=1;
            else DataMap_C[iaddr]=0;
            DataMapFresh_C[iaddr]=3;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_C_WIFI1:
        DataMap_C[iaddr]=0;
        *bvalue=DataMap_C[iaddr];
        DataMapFresh_C[iaddr]=99;
        return true;
    case MODBUS_DMAP_C_USB0:
        DataMap_C[iaddr]=0;
        *bvalue=DataMap_C[iaddr];
        DataMapFresh_C[iaddr]=99;
        return true;
    case MODBUS_DMAP_C_USB1:
        DataMap_C[iaddr]=0;
        *bvalue=DataMap_C[iaddr];
        DataMapFresh_C[iaddr]=99;
        return true;
    case MODBUS_DMAP_C_GPSD:
        if(DataMapFresh_C[iaddr]==0)
        {
            get_option_by_section_name(ctx,"gpsd","gpsd_conf","status",s_value);
            if(s_value[0]=='1')DataMap_C[iaddr]=1;
            else DataMap_C[iaddr]=0;
            DataMapFresh_C[iaddr]=3;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_C_LOCNET:
        if(DataMapFresh_C[iaddr]==0)
        {
            get_option_by_section_name(ctx,"eurd","eur_conf","Cell_Loc_Network",s_value);
            if(s_value[0]=='B')DataMap_C[iaddr]=1;
            else DataMap_C[iaddr]=0;
            DataMapFresh_C[iaddr]=3;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_C_EUR1:
    case MODBUS_DMAP_C_EUR2:
    case MODBUS_DMAP_C_EUR3:
    case MODBUS_DMAP_C_NMS:
        if(DataMapFresh_C[iaddr]==0)
        {
            get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", s_value);
            if(s_value[0]=='A')DataMap_C[MODBUS_DMAP_C_EUR1]=0;
            else DataMap_C[MODBUS_DMAP_C_EUR1]=1;
            if(s_value[1]=='A')DataMap_C[MODBUS_DMAP_C_EUR2]=0;
            else DataMap_C[MODBUS_DMAP_C_EUR2]=1;
            if(s_value[2]=='A')DataMap_C[MODBUS_DMAP_C_EUR3]=0;
            else DataMap_C[MODBUS_DMAP_C_EUR3]=1;
            if(s_value[3]=='A')DataMap_C[MODBUS_DMAP_C_NMS]=0;
            else DataMap_C[MODBUS_DMAP_C_NMS]=1;
            DataMapFresh_C[iaddr]=3;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_C_WEB:
        if(DataMapFresh_C[iaddr]==0)
        {
            get_option_by_section_name(ctx, "wsclient", "general", "status", s_value);
            if ( strcmp(s_value,"enable")==0 )DataMap_C[iaddr]=1;
            else DataMap_C[iaddr]=0;
            DataMapFresh_C[iaddr]=3;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_C_FWL:
        if(DataMapFresh_C[iaddr]==0)
        {
            get_option_by_section_name(ctx,"firewall","normal","disabled_firewall",s_value);
            if(s_value[0]=='0')DataMap_C[iaddr]=1;
            else DataMap_C[iaddr]=0;
            DataMapFresh_C[iaddr]=3;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_C_REBT:
        DataMap_C[iaddr]=0;
        *bvalue=DataMap_C[iaddr];
        DataMapFresh_C[iaddr]=99;
        return true;
    default:
        break;
    }
 
    return false;
}


bool Fresh_Datamap_I(char mode, unsigned int iaddr,char *bvalue)
{
    if(iaddr<0 || iaddr>=MODBUS_DMAP_I_MAX)return false;
    char s_value[10];
    int i;
    if(mode==2 || mode==3)
    {
        *bvalue=DataMap_I[iaddr];
        if(DataMapFresh_I[iaddr]==0&&mode==2)DataMapFreshSet_I[iaddr]=3;
        return true;
    }

    DataMapFreshSet_I[iaddr]=2;

    switch(iaddr)
    {
    case MODBUS_DMAP_I_O1:
    case MODBUS_DMAP_I_O2:
    case MODBUS_DMAP_I_O3:
    case MODBUS_DMAP_I_O4:
        if(DataMapFresh_C[iaddr]==0)
        {
            get_input_status(s_value);
            DataMap_C[MODBUS_DMAP_I_O1]=s_value[0];
            DataMap_C[MODBUS_DMAP_I_O2]=s_value[1];
            DataMap_C[MODBUS_DMAP_I_O3]=s_value[2];
            DataMap_C[MODBUS_DMAP_I_O4]=s_value[3];
            DataMapFresh_C[MODBUS_DMAP_I_O1]=2;
            DataMapFresh_C[MODBUS_DMAP_I_O2]=2;
            DataMapFresh_C[MODBUS_DMAP_I_O3]=2;
            DataMapFresh_C[MODBUS_DMAP_I_O4]=2;
        }
        *bvalue=DataMap_C[iaddr];
        return true;
    case MODBUS_DMAP_I_O5:
    case MODBUS_DMAP_I_O6:
    case MODBUS_DMAP_I_O7:
    case MODBUS_DMAP_I_O8:
        DataMap_I[iaddr]=0;
        *bvalue=DataMap_I[iaddr];
        DataMapFresh_I[iaddr]=99;
        return true;
    default:
        break;
    }

    return false;
}


bool Fresh_Datamap_R(char mode,unsigned int iaddr,char *r_value)//mode:0 only read; 1 write and read. r_value[2]
{
    if(iaddr<0 || iaddr>=MODBUS_DMAP_R_MAX)return false;
    int iaddr_p0=2*iaddr;
    int i,j,i_tmp;
    char *p;
    FILE* f;
    float f_tmp=0;
    char c_value[2],c_tmp;
    char buff_tmp[256];
    char lat[20],lng[20],alt[10];
    union _bint2 bi2_value;
    union _bsint2 bi2s_value;
    union _bint4 bi4_value;
    union _bsint4 bi4s_value;
    union _bint8 bi8_value;
    if(mode==2 || mode==3)
    {
        *r_value=*(DataMap_R+iaddr_p0);
        *(r_value+1)=*(DataMap_R+iaddr_p0+1);
        if(DataMapFresh_R[iaddr]==0&&mode==2)DataMapFreshSet_R[iaddr]=3;
        return true;
    }

    if(mode==1)
    {
        DataMapFreshSet_R[iaddr]=3;
        c_value[0]=*r_value;
        c_value[1]=*(r_value+1);
        switch(iaddr)
        {
        case MODBUS_DMAP_R_COM2_RATE:
            if(Endian_sign=='L')
            {
                bi2s_value.c[1]=*r_value;
                bi2s_value.c[0]=*(r_value+1);
            }else
            {
                bi2s_value.c[0]=*r_value;
                bi2s_value.c[1]=*(r_value+1);
            }
            int com_baudrate[18]={3,6,12,24,36,48,72,96,144,192,288,384,576,1152,2304,4608,9216};
            for(i=0;i<18;i++)
            {
                if(bi2s_value.i==com_baudrate[i])break;
            }
            if(i==18)return false;
            buff_tmp[1]=0;
            buff_tmp[0]=i+'A';
            set_option_by_section_name(ctx,"comport2","com2","COM2_Data_Baud_Rate",buff_tmp);
            SubProgram_Start("/etc/init.d/soip2","restart &");
            DataMapFresh_R[iaddr]=0;
            return true;
        case MODBUS_DMAP_R_COM2_FMT:
            c_tmp=*(r_value+1);
            c_tmp=c_tmp-1+'A';
            buff_tmp[1]=0;
            buff_tmp[0]=c_tmp;
            set_option_by_section_name(ctx,"comport2","com2","COM2_Data_Format",buff_tmp);
            SubProgram_Start("/etc/init.d/soip2","restart &");
            DataMapFresh_R[iaddr]=0;
            return true;

        default:

            return true;
            ;
        }
        return true;
     }


    char imei[30];
    char rssi[10];
    char temp[10];
    char buildno[10];
    int trans_sign=1;
    char packet_statistics_buff[13];
    char rm[20],tm[20];

    DataMapFreshSet_R[iaddr]=2;
    if(DataMapFresh_R[iaddr]==0)
    {
        c_value[0]=0;
        c_value[1]=0;
        switch(iaddr)
        {
        case MODBUS_DMAP_R_MODEL:
             DataMapFresh_R[iaddr]=99;
             *(DataMap_R+iaddr_p0)=0;
             *(DataMap_R+iaddr_p0+1)=MODBUS_MODEL_VIP4G;
            break;
        case MODBUS_DMAP_R_BUILD:
            *(DataMap_R+iaddr_p0)=0xFF;
            *(DataMap_R+iaddr_p0+1)=0xFF;
            buildno[0]=0;
            if (!(f = fopen("/etc/version", "r"))) 
            {
                //do nothing.
            }else
            {
                while (fgets(buff_tmp, 100, f)) 
                {
                    p=NULL;
                    p = strstr(buff_tmp, "BUILD=");//imei=" 012773002002648"
                    if (p != NULL)
                    {
                        p+=strlen("BUILD=")+1;
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<10)) 
                            {
                                buildno[j]=*p;
                                p++;
                                j++;
                             }
                        buildno[j]=0;
                        break;
                    }//if p
                }
            }
            if(f)fclose(f);
            j=strlen(buildno);
            if(j>1)
            {
                bi2_value.i=atoi(buildno);
                if(bi2_value.i>0)
                {
                    if(Endian_sign=='L')bi2_value.i=htons_rv16(bi2_value.i);
                    *(DataMap_R+iaddr_p0)=bi2_value.c[0];
                    *(DataMap_R+iaddr_p0+1)=bi2_value.c[1];
                    DataMapFresh_R[iaddr]=99;
                }
            }
            break;
        case MODBUS_DMAP_R_MID0:
        case MODBUS_DMAP_R_MID1:
        case MODBUS_DMAP_R_MID2:
        case MODBUS_DMAP_R_MID3:
        case MODBUS_DMAP_R_RSSI:
        case MODBUS_DMAP_R_TEMP:
            imei[0]=0;
            rssi[0]=0;
            temp[0]=0;
            if (!(f = fopen("/var/run/VIP4G_status", "r"))) 
            {
                //do nothing.
            }else
            {
                while (fgets(buff_tmp, 100, f)) 
                {
                    p=NULL;
                    p = strstr(buff_tmp, "rssi=");//rssi=" GSM RSSI : 72"
                    if (p != NULL)
                    {
                        while ((*p != ':') && (*p != '\n') && (*p!=0))p++; //key space
                        if (*p==':')
                        {
                            p++;
                            while (*p == ' ')p++; //first space
                            j=0;
                            while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<20)) 
                            {
                                j++;
                                rssi[j]=*p;
                                p++;
                             }
                            if(j>0)
                            {
                                j++;
                                rssi[j]=0;
                                rssi[0]='-';
                            }
                        }//if :
                    }//if p

                    p=NULL;
                    p = strstr(buff_tmp, "imei=");//imei=" 012773002002648"
                    if (p != NULL)
                    {
                        p+=strlen("imei=")+1;
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                            {
                                imei[j]=*p;
                                p++;
                                j++;
                             }
                        if(j>0)imei[j]=0;
                    }//if p

                    p=NULL;
                    p = strstr(buff_tmp, "temp=");//temp=" 39 degC"
                    if (p != NULL)
                    {
                        p+=strlen("temp=")+1;
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != '\"')&& (*p!=' ') && (*p!='\0') && (*p!='\n') &&(j<20)) 
                            {
                                temp[j]=*p;
                                p++;
                                j++;
                             }
                        if(j>0)temp[j]=0;
                    }//if p
                }
            }
            if(f)fclose(f);

            bi8_value.i=0;
            if (sscanf(imei,"%llu",&(bi8_value.i))>0)
            {
                if(bi8_value.i>1)
                {
                    if(Endian_sign=='L')bi8_value.i=htons_rv64(bi8_value.i);
                    DataMapFresh_R[MODBUS_DMAP_R_MID0]=99;
                    DataMapFresh_R[MODBUS_DMAP_R_MID1]=99;
                    DataMapFresh_R[MODBUS_DMAP_R_MID2]=99;
                    DataMapFresh_R[MODBUS_DMAP_R_MID3]=99;
                    DataMapFreshSet_R[MODBUS_DMAP_R_MID0]=2;
                    DataMapFreshSet_R[MODBUS_DMAP_R_MID1]=2;
                    DataMapFreshSet_R[MODBUS_DMAP_R_MID2]=2;
                    DataMapFreshSet_R[MODBUS_DMAP_R_MID3]=2;

                }else bi8_value.i=0;
            }
            for(i=0;i<8;i++)
            {
                DataMap_R[2*MODBUS_DMAP_R_MID0+i]=bi8_value.c[i];
            }

            bi2s_value.i=atoi(rssi);
            if(Endian_sign=='L')
            {
                c_tmp=bi2s_value.c[1];
                bi2s_value.c[1]=bi2s_value.c[0];
                bi2s_value.c[0]=c_tmp;
            }
            DataMap_R[2*MODBUS_DMAP_R_RSSI]=bi2s_value.c[0];
            DataMap_R[2*MODBUS_DMAP_R_RSSI+1]=bi2s_value.c[1];
            DataMapFresh_R[MODBUS_DMAP_R_RSSI]=3;
            DataMapFreshSet_R[MODBUS_DMAP_R_RSSI]=2;

            bi2s_value.i=atoi(temp);
            if(Endian_sign=='L')
            {
                c_tmp=bi2s_value.c[1];
                bi2s_value.c[1]=bi2s_value.c[0];
                bi2s_value.c[0]=c_tmp;
            }
            DataMap_R[2*MODBUS_DMAP_R_TEMP]=bi2s_value.c[0];
            DataMap_R[2*MODBUS_DMAP_R_TEMP+1]=bi2s_value.c[1];
            DataMapFresh_R[MODBUS_DMAP_R_TEMP]=3;
            DataMapFreshSet_R[MODBUS_DMAP_R_TEMP]=2;

            break;
        case MODBUS_DMAP_R_VDC:
            DataMap_R[2*MODBUS_DMAP_R_VDC]=0;
            DataMap_R[2*MODBUS_DMAP_R_VDC+1]=0;
            DataMapFresh_R[iaddr]=99;
            break;
        case MODBUS_DMAP_R_CARR_RM:
        case MODBUS_DMAP_R_CARR_TM:
            p=NULL;
            if (!(f = fopen("/proc/net/dev", "r"))) {
                trans_sign=0;
            }else
            {
                while (fgets(buff_tmp, 256, f)) {
                p = strstr(buff_tmp, "br-wan2");
                if (p != NULL)
                {
                    trans_sign=2;
                    break;  
                }
                }
            }
            if(f)fclose(f);

            if(trans_sign==2 && p!=NULL) 
            {
                p += strlen("br-wan2");        
                p++;
                for (i_tmp=0;i_tmp<16;i_tmp++) 
                    {
                        while (*p == ' ')p++;
                        j=0;
                        while ((*p != ' ')&&(j<11)) 
                        {
                        packet_statistics_buff[j] = *p;
                        p++;
                        j++;
                        }
                        packet_statistics_buff[j]= 0;                  
                        if(j>0)
                        {
                            if (i_tmp==0)sprintf(rm,"%s",packet_statistics_buff);
                            if (i_tmp==8)sprintf(tm,"%s",packet_statistics_buff);
                        }
                    }
            }else
            {
                strcpy(rm,"0");
                strcpy(tm,"0");
            }

            strcpy(buff_tmp,rm);
            j=strlen(buff_tmp);
            bi2_value.i=0;
            if(j==6 && buff_tmp[0]>'4' && buff_tmp[0]<='9')bi2_value.i=1;
            if(j>6)
            {
                i_tmp=buff_tmp[j-6]-'0';
                for(i=0;i<6;i++)
                {
                    buff_tmp[j-6+i]=0;
                }
                bi2_value.i=atoi(buff_tmp);
                if(i_tmp>4)bi2_value.i+=1;
            }
            if(Endian_sign=='L')bi2_value.i=htons_rv16(bi2_value.i);
            *(DataMap_R+2*MODBUS_DMAP_R_CARR_RM)=bi2_value.c[0];
            *(DataMap_R+2*MODBUS_DMAP_R_CARR_RM+1)=bi2_value.c[1];
            DataMapFresh_R[MODBUS_DMAP_R_CARR_RM]=5;
            DataMapFreshSet_R[MODBUS_DMAP_R_CARR_RM]=2;

            strcpy(buff_tmp,tm);
            j=strlen(buff_tmp);
            bi2_value.i=0;
            if(j==6 && buff_tmp[0]>'4' && buff_tmp[0]<='9')bi2_value.i=1;
            if(j>6)
            {
                i_tmp=buff_tmp[j-6]-'0';
                for(i=0;i<6;i++)
                {
                    buff_tmp[j-6+i]=0;
                }
                bi2_value.i=atoi(buff_tmp);
                if(i_tmp>4)bi2_value.i+=1;
            }
            if(Endian_sign=='L')bi2_value.i=htons_rv16(bi2_value.i);
            *(DataMap_R+2*MODBUS_DMAP_R_CARR_TM)=bi2_value.c[0];
            *(DataMap_R+2*MODBUS_DMAP_R_CARR_TM+1)=bi2_value.c[1];
            DataMapFresh_R[MODBUS_DMAP_R_CARR_TM]=5;
            DataMapFreshSet_R[MODBUS_DMAP_R_CARR_TM]=2;
            break;
        case MODBUS_DMAP_R_GPS_LAT0:
        case MODBUS_DMAP_R_GPS_LAT1:
        case MODBUS_DMAP_R_GPS_LON0:
        case MODBUS_DMAP_R_GPS_LON1:
        case MODBUS_DMAP_R_GPS_ALT:
            get_gps_data_ll(lat,lng,alt);
            bi4s_value.i=atoi(lat);
            if(Endian_sign=='L')bi4s_value.i=htons_rv32s(bi4s_value.i);
            for(i=0;i<4;i++)
            {
                DataMap_R[2*MODBUS_DMAP_R_GPS_LAT0+i]=bi4s_value.c[i];
            }
            DataMapFresh_R[MODBUS_DMAP_R_GPS_LAT0]=3;
            DataMapFresh_R[MODBUS_DMAP_R_GPS_LAT1]=3;
            DataMapFreshSet_R[MODBUS_DMAP_R_GPS_LAT0]=2;
            DataMapFreshSet_R[MODBUS_DMAP_R_GPS_LAT1]=2;

            bi4s_value.i=atoi(lng);
            if(Endian_sign=='L')bi4s_value.i=htons_rv32s(bi4s_value.i);
            for(i=0;i<4;i++)
            {
                DataMap_R[2*MODBUS_DMAP_R_GPS_LON0+i]=bi4s_value.c[i];
            }
            DataMapFresh_R[MODBUS_DMAP_R_GPS_LON0]=3;
            DataMapFresh_R[MODBUS_DMAP_R_GPS_LON1]=3;
            DataMapFreshSet_R[MODBUS_DMAP_R_GPS_LON0]=2;
            DataMapFreshSet_R[MODBUS_DMAP_R_GPS_LON1]=2;

            bi2s_value.i=atoi(alt);
            if(Endian_sign=='L')
            {
                c_tmp=bi2s_value.c[1];
                bi2s_value.c[1]=bi2s_value.c[0];
                bi2s_value.c[0]=c_tmp;
            }
            DataMap_R[2*MODBUS_DMAP_R_GPS_ALT]=bi2s_value.c[0];
            DataMap_R[2*MODBUS_DMAP_R_GPS_ALT+1]=bi2s_value.c[1];
            DataMapFresh_R[MODBUS_DMAP_R_GPS_ALT]=3;
            DataMapFreshSet_R[MODBUS_DMAP_R_GPS_ALT]=2;
            break;
        case MODBUS_DMAP_R_COM1_RATE:
            DataMap_R[2*MODBUS_DMAP_R_COM1_RATE]=0;
            DataMap_R[2*MODBUS_DMAP_R_COM1_RATE+1]=0;
            DataMapFresh_R[iaddr]=99;
            break;
        case MODBUS_DMAP_R_COM1_FMT:
            DataMap_R[2*MODBUS_DMAP_R_COM1_FMT]=0;
            DataMap_R[2*MODBUS_DMAP_R_COM1_FMT+1]=0;
            DataMapFresh_R[iaddr]=99;
            break;
        case MODBUS_DMAP_R_COM2_RATE:
            get_option_by_section_name(ctx,"comport2","com2","COM2_Data_Baud_Rate",buff_tmp);
            int com_baudrate[18]={3,6,12,24,36,48,72,96,144,192,288,384,576,1152,2304,4608,9216};
            bi2s_value.i=com_baudrate[buff_tmp[0]-'A'];
            if(Endian_sign=='L')
            {
                c_tmp=bi2s_value.c[1];
                bi2s_value.c[1]=bi2s_value.c[0];
                bi2s_value.c[0]=c_tmp;
            }
            DataMap_R[2*MODBUS_DMAP_R_COM2_RATE]=bi2s_value.c[0];
            DataMap_R[2*MODBUS_DMAP_R_COM2_RATE+1]=bi2s_value.c[1];
            DataMapFresh_R[MODBUS_DMAP_R_COM2_RATE]=5;
            break;
        case MODBUS_DMAP_R_COM2_FMT:
            get_option_by_section_name(ctx,"comport2","com2","COM2_Data_Format",buff_tmp);
            i=buff_tmp[0]-'A'+1;
            if(i<0)i=0;
            DataMap_R[2*MODBUS_DMAP_R_COM2_FMT]=0;
            DataMap_R[2*MODBUS_DMAP_R_COM2_FMT+1]=i;
            DataMapFresh_R[MODBUS_DMAP_R_COM2_FMT]=5;
            break;
        }
    }
    *r_value=*(DataMap_R+iaddr_p0);
    *(r_value+1)=*(DataMap_R+iaddr_p0+1);
    //printf("\naddr:%d, %x %x   \n",iaddr, *r_value, *(r_value+1));
    return true;
}


int Pack_Data_C(char *buff_start,int iaddr_begin,int data_num)
{
    if(iaddr_begin<0 || iaddr_begin>=MODBUS_DMAP_C_MAX)return 0;
    if(data_num<1 || data_num>255)return 0;

    //    memset(p_send_buff_tcp[connect_id], 0, SEND_BUFF_LEN_MAX*sizeof(int));
    //buff_start[0]=0;
    //buff_start[1]=0;
    buff_start[2]=0;
    buff_start[3]=0;
    buff_start[4]=0;
    buff_start[6]=tcp_slave_id;
    buff_start[7]=0x01;
    char *p=buff_start+9;
    char c_value;
    int i;
    c_value=data_num-1;
    c_value=c_value>>3;
    unsigned int len=c_value+1;
    *(p+c_value)=0;
    for(i=0;i<data_num;i++)
    {
        c_value=0;
        Fresh_Datamap_C(DataMapFreshSet_C[iaddr_begin+i],iaddr_begin+i,&c_value);
        setdatabit(p,i,c_value);
    }
    buff_start[8]=len;
    buff_start[5]=len+3;

    len=len+9;
/*
printf("\n*****pack data:%d bytes:",len);
int k;
for(k=0;k<len;k++)
{
    printf("%x ",buff_start[k]);
}
printf("\n");
*/
    return len;
}


int Pack_Data_C_rtu(char *buff_start,int iaddr_begin,int data_num)
{
    if(iaddr_begin<0 || iaddr_begin>=MODBUS_DMAP_C_MAX)return 0;
    if(data_num<1 || data_num>255)return 0;

    //    memset(p_send_buff_tcp[connect_id], 0, SEND_BUFF_LEN_MAX*sizeof(int));
    buff_start[0]=com_slave_id;
    buff_start[1]=0x01;

    char *p=buff_start+3;
    char c_value;
    int i;
    c_value=data_num-1;
    c_value=c_value>>3;
    *(p+c_value)=0;
    unsigned int len=c_value+1;
    for(i=0;i<data_num;i++)
    {
        c_value=0;
        Fresh_Datamap_C(DataMapFreshSet_C[iaddr_begin+i],iaddr_begin+i,&c_value);
        setdatabit(p,i,c_value);
    }
    buff_start[2]=len;

    len=len+5;

    return len;
}

int Pack_Data_I(char *buff_start,int iaddr_begin,int data_num)
{
    if(iaddr_begin<0 || iaddr_begin>=MODBUS_DMAP_I_MAX)return 0;
    if(data_num<1 || data_num>255)return 0;
    //buff_start[0]=0;
    //buff_start[1]=0;
    buff_start[2]=0;
    buff_start[3]=0;
    buff_start[4]=0;
    buff_start[6]=tcp_slave_id;
    buff_start[7]=0x02;
    char *p=buff_start+9;
    char c_value;
    int i;
    c_value=data_num-1;
    c_value=c_value>>3;
    unsigned int len=c_value+1;
    *(p+c_value)=0;
    for(i=0;i<data_num;i++)
    {
        c_value=0;
        Fresh_Datamap_I(DataMapFreshSet_I[iaddr_begin+i],iaddr_begin+i,&c_value);
        setdatabit(p,i,c_value);
    }
    buff_start[8]=len;
    buff_start[5]=len+3;

    len=len+9;
    return len;
}


int Pack_Data_I_rtu(char *buff_start,int iaddr_begin,int data_num)
{
    if(iaddr_begin<0 || iaddr_begin>=MODBUS_DMAP_C_MAX)return 0;
    if(data_num<1 || data_num>255)return 0;

    //    memset(p_send_buff_tcp[connect_id], 0, SEND_BUFF_LEN_MAX*sizeof(int));
    buff_start[0]=com_slave_id;
    buff_start[1]=0x02;

    char *p=buff_start+3;
    char c_value;
    int i;
    c_value=data_num-1;
    c_value=c_value>>3;
    *(p+c_value)=0;
    unsigned int len=c_value+1;
    for(i=0;i<data_num;i++)
    {
        c_value=0;
        Fresh_Datamap_I(DataMapFreshSet_I[iaddr_begin+i],iaddr_begin+i,&c_value);
        setdatabit(p,i,c_value);
    }
    buff_start[2]=len;

    len=len+5;

    return len;
}

int Pack_Data_R(char *buff_start,int iaddr_begin,int data_num)
{

    if(iaddr_begin<0 || iaddr_begin>=MODBUS_DMAP_R_MAX)return 0;
    if(data_num<1 || data_num>122)return 0;
    //buff_start[0]=0;
    //buff_start[1]=0;
    buff_start[2]=0;
    buff_start[3]=0;
    buff_start[4]=0;
    buff_start[6]=tcp_slave_id;
    buff_start[7]=0x03;
    char *p;
    p=buff_start+9;
    char c_value[2];
    int i;
    for(i=0;i<data_num;i++)
    {
        c_value[0]=0x00;
        c_value[1]=0x00;
        if(Fresh_Datamap_R(DataMapFreshSet_R[iaddr_begin+i],iaddr_begin+i,c_value)==false)break;
        *(p+2*i)=c_value[0];
        *(p+2*i+1)=c_value[1];
    }
    unsigned int len=2*i;
    buff_start[8]=len;
    buff_start[5]=len+3;
    len=len+9;
    return len;
}


int Pack_Data_R_rtu(char *buff_start,int iaddr_begin,int data_num)
{

    if(iaddr_begin<0 || iaddr_begin>=MODBUS_DMAP_R_MAX)return 0;
    if(data_num<1 || data_num>122)return 0;
    buff_start[0]=com_slave_id;
    buff_start[1]=0x03;

    char *p;
    p=buff_start+3;
    char c_value[2];
    int i;
    for(i=0;i<data_num;i++)
    {
        c_value[0]=0x00;
        c_value[1]=0x00;
        if(Fresh_Datamap_R(DataMapFreshSet_R[iaddr_begin+i],iaddr_begin+i,c_value)==false)break;
        *(p+2*i)=c_value[0];
        *(p+2*i+1)=c_value[1];
    }
    unsigned int len=2*i;
    buff_start[2]=len;
    len=len+5;
    return len;
}



//if >0 need to pack_modbus_tcp
//find the last cmd to deal with.
int unpack_modbus_tcp(unsigned int connect_id)
{
    int i;
    char *p;
    int begin_addr;
    int data_num;
    union _bint2 bi2_tmp,bi2_tmp2;

    if(connect_id>=REMOTE_TCP_MAX_NUMBER)return 0;
    if(read_buff_len_tcp[connect_id]<11)return 0;
    p=p_read_buff_tcp[connect_id];
    i=read_buff_len_tcp[connect_id]-11;
    while(i>0)
    {
//        if(*p==0 && *(p+1)==0 && *(p+2)==0 && *(p+3)==0 && *(p+4)==0 && *(p+5)==6)
        if(*(p+2)==0 && *(p+3)==0 && *(p+4)==0 && *(p+5)==6)  //can't confirm MBAP's value
        {
            if(*(p+6)==tcp_slave_id)  //can't accept broadcast cmd
            {
                p_send_buff_tcp[connect_id][0]=*p;
                p_send_buff_tcp[connect_id][1]=*(p+1);
                if(Endian_sign=='B')
                {
                    bi2_tmp.c[0]=*(p+8);
                    bi2_tmp.c[1]=*(p+9);
                    bi2_tmp2.c[0]=*(p+10);
                    bi2_tmp2.c[1]=*(p+11);
                }else
                {
                    bi2_tmp.c[1]=*(p+8);
                    bi2_tmp.c[0]=*(p+9);
                    bi2_tmp2.c[1]=*(p+10);
                    bi2_tmp2.c[0]=*(p+11);
                }
                data_num=bi2_tmp2.i;
                if(*(p+7)==1)
                {
                    begin_addr=bi2_tmp.i-tcp_offset_c;
                    return Pack_Data_C(p_send_buff_tcp[connect_id],begin_addr,data_num);
                }
                if(*(p+7)==2)
                {
                    begin_addr=bi2_tmp.i-tcp_offset_i;
                    return Pack_Data_I(p_send_buff_tcp[connect_id],begin_addr,data_num);
                }
                if(*(p+7)==3)
                {
                    begin_addr=bi2_tmp.i-tcp_offset_r;
                    return Pack_Data_R(p_send_buff_tcp[connect_id],begin_addr,data_num);
                }
                if(*(p+7)==5)
                {
                    //printf("\n****setting coil:%x %x:",*(p+10),*(p+11));
                    begin_addr=bi2_tmp.i-tcp_offset_c;
                    char c_tmp;
                    if(*(p+10)==0xFF && *(p+11)==0x00 )//close
                    {
                        //printf("close \n");
                        c_tmp=1;
                        Fresh_Datamap_C(1,begin_addr,&c_tmp);
                        for(i=0;i<12;i++)
                        {
                            p_send_buff_tcp[connect_id][i]=*(p+i);
                        }
                        return 12;
                    }
                    if(*(p+10)==0x00 && *(p+11)==0x00)//open
                    {
                        //printf("open \n");
                        c_tmp=0;
                        Fresh_Datamap_C(1,begin_addr,&c_tmp);
                        for(i=0;i<12;i++)
                        {
                            p_send_buff_tcp[connect_id][i]=*(p+i);
                        }
                        return 12;
                    }

                }
                if(*(p+7)==6)
                {
                    begin_addr=bi2_tmp.i-tcp_offset_r;
                    Fresh_Datamap_R(1,begin_addr,(p+10));
                    for(i=0;i<12;i++)
                    {
                        p_send_buff_tcp[connect_id][i]=*(p+i);
                    }
                    return 12;
                }
            }
        }

        i--;
        p++;
        
    }
    return 0;
}


int setTcpSocketAlive(int s)
{

    int optval;
    socklen_t optlen = sizeof(optval);

    /* Check the status for the keepalive option */
    if (getsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) < 0) {
        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
    }
    //syslog( LOGOPTS,"SO_KEEPALIVE is %s\n", (optval ? "ON" : "OFF"));

    /* Set the option active */
    optval = 1;
    optlen = sizeof(optval);
    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        syslog( LOGOPTS,"%s setsockopt %s\n", __func__,strerror(errno));
    }
    //syslog( LOGOPTS,"SO_KEEPALIVE set on socket\n");

    /* Check the status again */
    if (getsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) < 0) {
        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
    }

    if (getsockopt(s, SOL_TCP, TCP_KEEPCNT, &optval, &optlen) < 0) {
        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
    }

    optval = KEEP_COUNT;
    if (setsockopt(s, SOL_TCP, TCP_KEEPCNT, &optval, optlen) < 0) {
        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
    }
    if (getsockopt(s, SOL_TCP, TCP_KEEPCNT, &optval, &optlen) < 0) {
        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
    }


    if (getsockopt(s, SOL_TCP, TCP_KEEPIDLE, &optval, &optlen) < 0) {
        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
    }

    optval = KEEP_IDLE;
    if (setsockopt(s, SOL_TCP, TCP_KEEPIDLE, &optval, optlen) < 0) {
        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
    }

    if (getsockopt(s, SOL_TCP, TCP_KEEPIDLE, &optval, &optlen) < 0) {
        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
    }


    if (getsockopt(s, SOL_TCP, TCP_KEEPINTVL, &optval, &optlen) < 0) {
        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
    }


    optval = KEEP_INTVL;
    if (setsockopt(s, SOL_TCP, TCP_KEEPINTVL, &optval, optlen) < 0) {
        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));

        if (getsockopt(s, SOL_TCP, TCP_KEEPINTVL, &optval, &optlen) < 0) {
            syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
        }
    }

    return 0;
}


void Clear_tcp_connect_thread_s(int i)
{
    if(i>=0 && i<REMOTE_TCP_MAX_NUMBER)
    {
        if(remote_tcp_client_sockfd[i]>0)
        {
            shutdown(remote_tcp_client_sockfd[i],SHUT_RDWR);  
            close(remote_tcp_client_sockfd[i]);
            remote_tcp_client_sockfd[i]=0;
        }
        if(remote_tcp_client_pt[i]!=0)
        {
            pthread_cancel(remote_tcp_client_pt[i]);
            void* state; 
            pthread_join(remote_tcp_client_pt[i], &state);
        }
        remote_tcp_client_idleseconds[i]=0;
        remote_tcp_client_status[i]=0;
        if(p_read_buff_tcp[i]!=NULL)
        {
            //free must before pthread cancel.
            free(p_read_buff_tcp[i]);//malloc(send_buff_size * sizeof(char));
            p_read_buff_tcp[i]=NULL;
        }
        if(p_send_buff_tcp[i]!=NULL)
        {
            free(p_send_buff_tcp[i]);
            p_send_buff_tcp[i]=NULL;
        }
        int j;
        for(j=0;j<MODBUS_DMAP_TYPE_MAX;j++)
        {
            if(DataLockWho[j]==i)
            {
                DataLockWho[j]=99;
                if(DataLock[j]!=0)DataLock[j]=0;
            }
        }
    }
}

void Clear_tcp_connect_thread()
{
    int i;
    for(i=0;i<REMOTE_TCP_MAX_NUMBER;i++)
    {
        Clear_tcp_connect_thread_s(i);
    }
    current_tcp_client_num=0;
}

void *tcp_connect_thread_function(void *connect_id)
{
    //init
    int i;
    int pt_id=atoi(connect_id);
    remote_tcp_client_idleseconds[pt_id]=0;
    remote_tcp_client_status[pt_id]=0;
    memset(p_read_buff_tcp[pt_id], 0, RECV_BUFF_LEN_MAX*sizeof(char));
    memset(p_send_buff_tcp[pt_id], 0, SEND_BUFF_LEN_MAX*sizeof(char));

    //printf("\n########thread:%d running#####\n",pt_id);
    int i_tmp=0;
    int send_buff_len=0;
    int tcp_error_send_cnt=0;
    char *p_send_begin=p_send_buff_tcp[pt_id];
while(1) 
{
    if(remote_tcp_client_sockfd[pt_id]<=0)break;

    read_buff_len_tcp[pt_id]=0;
    i_tmp = recv(remote_tcp_client_sockfd[pt_id], p_read_buff_tcp[pt_id], (RECV_BUFF_LEN_MAX-1), 0);//block
    if(i_tmp<0)
    {
        if(errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)
        {
            remote_tcp_client_status[pt_id]=9;
            break;
        }
    }
    if(i_tmp==0)//may be closed by client, need to re accept
    {
        remote_tcp_client_status[pt_id]=9;
        break;
    }
    if(i_tmp>0)//only consider one time get one full data pack
    {
//printf("\n*****recv 1: %d bytes :",i_tmp);
//int k;
//for(k=0;k<i_tmp;k++)
//{
//    printf("%x ",p_read_buff_tcp[pt_id][k]);
//}
//printf("\n");
        read_buff_len_tcp[pt_id]=i_tmp;
        p_send_begin=p_send_buff_tcp[pt_id];
        send_buff_len=0;
        send_buff_len=unpack_modbus_tcp(pt_id);// if there is new request, give up old send data
        remote_tcp_client_idleseconds[pt_id]=0;
    }


    send_continue:

    if(remote_tcp_client_status[pt_id]==5)
    {
        remote_tcp_client_status[pt_id]=0;
        for(i_tmp=0;i_tmp<MODBUS_DMAP_TYPE_MAX;i_tmp++) 
        {
            if(DataLockWho[i_tmp]==pt_id)
            {
                DataLockWho[i_tmp]==99;
                DataLock[i_tmp]=0;
            }
        }
    }

    if(send_buff_len>0)
    {
        i=0;
try_more_send:
        i_tmp = send(remote_tcp_client_sockfd[pt_id],p_send_begin,send_buff_len,0);
        if(i_tmp<0)
        {
            if(errno==EINTR || errno==ENOSPC || errno==EAGAIN || errno==EWOULDBLOCK)
            {
                i++;
                tcp_error_send_cnt++;
                sleep(1);
                if(i<3)goto try_more_send;
            }else
            {
                remote_tcp_client_status[pt_id]=9;
                break;
            }
        }
        if(i_tmp==0)//may be closed by client, need to re accept
        {
            sleep(1);
        }
        if(i_tmp>0)
        {
//printf("\n******send ok:%d\n",i_tmp);
            tcp_error_send_cnt=0;
            if(i_tmp<send_buff_len)
            {
                p_send_begin+=i_tmp;
                send_buff_len-=i_tmp;
                goto send_continue;
            }else
            {
                send_buff_len=0;
            }
        }

        if(tcp_error_send_cnt>20)
        {
            remote_tcp_client_status[pt_id]=9;
            break;
        }
    }


}//while 1, normal no sleep 

    free(p_read_buff_tcp[pt_id]);
    free(p_send_buff_tcp[pt_id]);
    p_read_buff_tcp[pt_id]=NULL;
    p_send_buff_tcp[pt_id]=NULL;
}


void tcp_slave_function()
{


    if(tcp_slave_port<1 || tcp_slave_id<1 || tcp_slave_id>254)
    {
        syslog(LOGOPTS,"Modbus slave parameter is wrong. Terminated.\n");
        return;
    }
    if(tcp_offset_c<0 || tcp_offset_c>65535 || tcp_offset_i<0 || tcp_offset_i>65535 || tcp_offset_r<0 || tcp_offset_r>65535)
    {
        syslog(LOGOPTS,"Modbus slave offset parameter is wrong. Terminated.\n");
        return;
    }
    if(tcp_idletimeout_max<1)tcp_idletimeout_max=300;


    int tcp_listen_ok=0;
    int i_tmp,i,j;

    current_tcp_client_num=0;
    memset(&remote_tcp_client_sockfd, 0, REMOTE_TCP_MAX_NUMBER*sizeof(int));
    memset(&remote_tcp_client_idleseconds, 0, REMOTE_TCP_MAX_NUMBER*sizeof(int));
    memset(&remote_tcp_client_status, 0, REMOTE_TCP_MAX_NUMBER*sizeof(char));
    memset(&remote_tcp_client_pt, 0, REMOTE_TCP_MAX_NUMBER*sizeof(pthread_t));
    for(i=0;i<REMOTE_TCP_MAX_NUMBER;i++)p_read_buff_tcp[i]=NULL;
    for(i=0;i<REMOTE_TCP_MAX_NUMBER;i++)p_send_buff_tcp[i]=NULL;

    memset(&DataMap_C, 0, MODBUS_DMAP_C_MAX*sizeof(char));
    memset(&DataMap_I, 0, MODBUS_DMAP_I_MAX*sizeof(char));
    memset(&DataMap_R, 0, MODBUS_DMAP_R_MAX*2*sizeof(char));
    memset(&DataMapFresh_C, 0,MODBUS_DMAP_C_MAX *sizeof(char));
    memset(&DataMapFresh_I, 0, MODBUS_DMAP_I_MAX*sizeof(char));
    memset(&DataMapFresh_R, 0, MODBUS_DMAP_R_MAX*sizeof(char));
    memset(&DataMapFreshSet_C, 0,MODBUS_DMAP_C_MAX *sizeof(char));
    memset(&DataMapFreshSet_I, 0, MODBUS_DMAP_I_MAX*sizeof(char));
    memset(&DataMapFreshSet_R, 0, MODBUS_DMAP_R_MAX*sizeof(char));
    memset(&DataLock, 0, MODBUS_DMAP_TYPE_MAX*sizeof(char));
    for(i=0;i<MODBUS_DMAP_TYPE_MAX;i++)DataLockWho[i]=99;

    local_as_server_sockfd=0;

while(1)
{


    if(tcp_listen_ok!=1)//init tcp server
    {

        tcp_listen_ok=0;
        Clear_tcp_connect_thread();
        if(local_as_server_sockfd>0)
        {
            shutdown(local_as_server_sockfd,SHUT_RDWR);  
            close(local_as_server_sockfd);
            local_as_server_sockfd=0;
        }

        //tcp servser setup
        if ((local_as_server_sockfd = socket(AF_INET, SOCK_STREAM,0))==-1) /*build a socket    IPPROTO_TCP*/
        {
            syslog(LOGOPTS,"tcp_process Socket error %s exit\n",strerror(errno));
            tcp_listen_ok=0;
            sleep(1);
            goto tcp_init_fail;
        }
        /* reuse address*/
        if (setsockopt(local_as_server_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))< 0) 
        {
            syslog(LOGOPTS,"tcp_process reuse address error %s exit\n",strerror(errno));
            tcp_listen_ok=0;
            sleep(1);
            goto tcp_init_fail;
        }
        setTcpSocketAlive(local_as_server_sockfd);
        bzero(&local_as_server_addr,sizeof(struct sockaddr_in));
        /*set the server_address to 0*/
        local_as_server_addr.sin_family = AF_INET;/*internet*/
        local_as_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        /*any address can connect*/
        local_as_server_addr.sin_port = htons(tcp_slave_port);
        /*put the remote_server_port_int from host to internet frame*/
        if (bind(local_as_server_sockfd,(struct sockaddr*)(&local_as_server_addr),sizeof(struct sockaddr))==-1)
        /*bined the setting with the socket*/
        {
            syslog(LOGOPTS,"tcp_process Bind error %s exit\n",strerror(errno));
            tcp_listen_ok=0;
            sleep(1);
            goto tcp_init_fail;
        }
        if (listen(local_as_server_sockfd,REMOTE_TCP_MAX_NUMBER)==-1) {      /*can not block here.*/
            syslog(LOGOPTS,"tcp_process Listen error %s exit\n",strerror(errno));
            tcp_listen_ok=0;
            sleep(1);
            goto tcp_init_fail;
        }

        tcp_listen_ok=1;
        fcntl (local_as_server_sockfd, F_SETFL, fcntl(local_as_server_sockfd, F_GETFL) | O_NONBLOCK);

    }//end init setup tcp server and listen
    tcp_init_fail:

    if(tcp_listen_ok==1 && current_tcp_client_num<REMOTE_TCP_MAX_NUMBER) 
    {

        sin_size=sizeof(struct sockaddr_in);
        remote_as_client_sockfd=0;
        remote_as_client_sockfd=accept(local_as_server_sockfd,(struct sockaddr*)(&local_as_server_addr),&sin_size);
        if (remote_as_client_sockfd>0)  //non block
        {

        char client_IP[20];
        strcpy(client_IP,inet_ntoa(local_as_server_addr.sin_addr));
        printf("\n*******get new connect:%d:%s*******\n",remote_as_client_sockfd,client_IP);

            //check IP here
            if(tcp_IPfilter_en=='B')
            {
                in_addr_t newaddr_t=local_as_server_addr.sin_addr.s_addr;
                if(newaddr_t!=tcp_IPFilter[0] && newaddr_t!=tcp_IPFilter[1] && newaddr_t!=tcp_IPFilter[2] && newaddr_t!=tcp_IPFilter[3] ) 
                {
                    shutdown(remote_as_client_sockfd,SHUT_RDWR);  
                    close(remote_as_client_sockfd);
                    remote_as_client_sockfd=0;
                }
            }

            if(remote_as_client_sockfd>0)
            {
                for (i=0;i<REMOTE_TCP_MAX_NUMBER;i++)
                {
                    if (remote_tcp_client_sockfd[i]<=0)
                    {
                        remote_tcp_client_sockfd[i]=remote_as_client_sockfd;
                        remote_tcp_client_pt[i]=0;

                        p_read_buff_tcp[i]=(char *)malloc(RECV_BUFF_LEN_MAX * sizeof(char));
                        p_send_buff_tcp[i]=(char *)malloc(SEND_BUFF_LEN_MAX * sizeof(char));

                        char parameter[5];
                        sprintf(parameter,"%d",i);
                        pthread_create(&remote_tcp_client_pt[i],NULL,tcp_connect_thread_function,parameter);
                        if(remote_tcp_client_pt[i]>0)
                        {
                            current_tcp_client_num++;
                        }else
                        {
                            shutdown(remote_tcp_client_sockfd[i],SHUT_RDWR);  
                            close(remote_tcp_client_sockfd[i]);
                            remote_tcp_client_sockfd[i]=0;
                            remote_tcp_client_pt[i]=0;
                            free(p_read_buff_tcp[i]);
                            p_read_buff_tcp[i]=NULL;
                            free(p_send_buff_tcp[i]);
                            p_send_buff_tcp[i]=NULL;
                        }
                        remote_as_client_sockfd=0;
                        break;
                    }
                }

            }

        }else if(remote_as_client_sockfd==-1)
        {
            if(errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)
            {
                tcp_listen_ok=0;
            }
        }
    }

    if(tcp_listen_ok==1 && current_tcp_client_num>0) 
    {
        //printf("***All phread %d:%d:",current_tcp_client_num,tcp_idletimeout_max);
        for(i=0;i<REMOTE_TCP_MAX_NUMBER;i++)
        {
            if(remote_tcp_client_sockfd[i]>0)
            {
                if(tcp_idletimeout_max>0)
                {
                    if(remote_tcp_client_idleseconds[i]<tcp_idletimeout_max)remote_tcp_client_idleseconds[i]++;
                    else 
                    {
                        //printf("***Stop phread %d:%d\n",current_tcp_client_num,tcp_idletimeout_max);
                        Clear_tcp_connect_thread_s(i);
                        current_tcp_client_num--;
                    }
                }
                if(remote_tcp_client_status[i]==9)
                {
                    Clear_tcp_connect_thread_s(i);
                    current_tcp_client_num--;
                }
            }
        }
    }

    sleep(1);
    /*
    if(DataLock[MODBUS_DMAP_TYPE_C]==1)
    {
        if(DataLockWho[MODBUS_DMAP_TYPE_C]<REMOTE_TCP_MAX_NUMBER)
        {
            remote_tcp_client_status[DataLockWho[MODBUS_DMAP_TYPE_C]]=5;
        }
            DataLock[MODBUS_DMAP_TYPE_C]=0;
    }
    if(DataLock[MODBUS_DMAP_TYPE_I]==1)
    {
        if(DataLockWho[MODBUS_DMAP_TYPE_I]<REMOTE_TCP_MAX_NUMBER)
        {
            remote_tcp_client_status[DataLockWho[MODBUS_DMAP_TYPE_I]]=5;
        }
            DataLock[MODBUS_DMAP_TYPE_I]=0;
    } 
    if(DataLock[MODBUS_DMAP_TYPE_R]==1)
    {
        if(DataLockWho[MODBUS_DMAP_TYPE_R]<REMOTE_TCP_MAX_NUMBER)
        {
            remote_tcp_client_status[DataLockWho[MODBUS_DMAP_TYPE_R]]=5;
            remote_tcp_client_idleseconds[DataLockWho[MODBUS_DMAP_TYPE_R]]=tcp_idletimeout_max-3;
        }else  DataLock[MODBUS_DMAP_TYPE_R]=0;
    }
    */ 
}//while 1

}

int read_serial(int fd_, void * buf_, int size_)
{
    unsigned int Timing_out_frame_delay=com_slave_ctimeout;
    char * writePtr;
    int bytesRead;
    struct timeval frameDelay;
    fd_set fds;
    int rt;

    writePtr = (char *)buf_;
    bytesRead = 0;

    while ( bytesRead < size_ ) {
        frameDelay.tv_sec = 0;
        frameDelay.tv_usec = ( Timing_out_frame_delay > 1000 ) ?
                             Timing_out_frame_delay : 1000;
        FD_ZERO( &fds );
        FD_SET( fd_, &fds );

        rt = select( fd_ + 1, &fds, NULL, NULL, &frameDelay );
        if ( rt < 0 ) {
            syslog( LOGOPTS, "Failed to select in read_serial [%d,%d]\n", rt, errno );
            return rt;
        }

        if ( FD_ISSET( fd_, &fds ) ) {
            rt = read( fd_, writePtr + bytesRead, size_ - bytesRead );
            if ( rt < 0 ) {
                syslog( LOGOPTS, "Failed to read in read_serial [%d,%d]\n", rt, errno );
                return rt;
            }

            bytesRead += rt;
        } else {
            /* A frame timeout occurred. Break out of the loop */
            break;
        }
    }

    return bytesRead;
}


int COM_Set_Parameters(char dev )
{
    /*************************************Open com speed with  ************************8********/
    int com_speed[18]={300,600,1200,2400,3600,4800,7200,9600,14400,19200,28800,38400,57600,115200,230400,460800,921600};
    char com_parity[15][4]={"8N1","8N2","8E1","8O1","7N1","7N2","7E1","7O1","7E2","7O2"}; //12
    int fd,device_fd;
    int status;
    //printf("%s:set parameters\n",__func__);
    //sleep(5);
    char flow_control='A';

    if (dev==1) {    //COM1
        /* Open the BSP Device */
        //printf("%s:open COM1\n");
        //sleep(5);

        fd = OpenDev(COM1);//open com1

        if (fd<0) {
            syslog(LOGOPTS,"open com Error %s\n",strerror(errno));  
            return -1;
        }


        //syslog(LOGOPTS,"open com set ok com1 fd= %d\n",fd);
        Set_Speed(fd,com_speed[com_slave_baudrate-'A']);   


        if ('C'==flow_control) {
            if (-1==Set_DataFrame(fd,com_parity[com_slave_fmt-'A'][0]-'0',
                                  com_parity[com_slave_fmt-'A'][2]-'0',\
                                  com_parity[com_slave_fmt-'A'][1],0 )) {//set Databit:8,stop bit:1,parity:FLOW CONTROL
                syslog(LOGOPTS,"Set Parity Error %s\n",strerror(errno));  
                return -1;       
            } else {
                ioctl(fd,TIOCMGET, &status);
                status &=~TIOCM_RTS ;
                ioctl(fd, TIOCMSET,&status);

                return fd;
            }

        } else {
            if (-1==Set_DataFrame(fd,com_parity[com_slave_fmt-'A'][0]-'0',
                                  com_parity[com_slave_fmt-'A'][2]-'0',\
                                  com_parity[com_slave_fmt-'A'][1],
                                  flow_control-'A')) {//set Databit:8,stop bit:1,parity:FLOW CONTROL
                syslog(LOGOPTS,"Set Parity Error2 %s\n",strerror(errno));  
                return -1;
            } else {
                return fd;
            }


        }   
    } else {

        fd = OpenDev(COM2);//open com2

        //syslog(LOGOPTS,"open com  com2 fd= %d\n",fd);

        if (fd<0) {
            syslog(LOGOPTS,"open com2 Error %s\n",strerror(errno));  
            return -1;
        }
        Set_Speed(fd,com_speed[com_slave_baudrate-'A']);

        //DBG_log("[%s] COM_Set_Parameters: speed: %d\n", __func__, com_speed[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_BAUD_RATE][0]-'A']);
        //DBG_log("[%s] COM_Set_Parameters: databits: %d\n",  __func__, com_parity[flow_control-'A'][0]-'0' );
        //DBG_log("[%s] COM_Set_Parameters: parity: %c\n", __func__, com_parity[flow_control-'A'][1] );
        //DBG_log("[%s] COM_Set_Parameters: stop bit: %d\n", __func__, com_parity[flow_control-'A'][2]-'0' );
        //DBG_log("[%s] COM_Set_Parameters: FLOW CONTROL: %c\n", __func__, DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_FLOW_CONTROL][0]);


        if ( 'C' ==flow_control ) {

            if (-1==Set_DataFrame(fd,com_parity[flow_control-'A'][0]-'0',
                                  com_parity[flow_control-'A'][2]-'0',\
                                  com_parity[flow_control-'A'][1],0 )) {//set Databit:8,stop bit:1,parity:FLOW CONTROL
                syslog(LOGOPTS,"Set Parity Error %s\n",strerror(errno));  
                return -1;       
            } else {
                ioctl(fd,TIOCMGET, &status);
                status &=~TIOCM_RTS ;
                ioctl(fd, TIOCMSET,&status);

                return fd;
            }
        } else {

            if (-1==Set_DataFrame(fd,com_parity[flow_control-'A'][0]-'0',
                                  com_parity[flow_control-'A'][2]-'0',\
                                  com_parity[flow_control-'A'][1],
                                  flow_control-'A')) {//set Databit:8,stop bit:1,parity:FLOW CONTROL
                syslog(LOGOPTS,"Set Parity Error2 %s\n",strerror(errno));  
                return -1;
            } else {
                return fd;
            }
        }

        return fd;
    }

}

int COM_Set_Parameters1(char dev )
{
	/*************************************Open com speed with  ************************8********/
    int com_baudrate[18]={300,600,1200,2400,3600,4800,7200,9600,14400,19200,28800,38400,57600,115200,230400,460800,921600};
	char com_parity[15][4]={"8N1","8N2","8E1","8O1","7N1","7N2","7E1","7O1","7E2","7O2"}; //12
	int fd,device_fd;
	int status;
    struct gpio_ioctl info;

/*
NanoCPU         PA23        PA24        PA3
    RS232       H           L           L
    RS422       L           H           H
    RS485_TX    H           H           H
    RS485_TX    L           L           H
    
    these are encapsulated in BSP, still using old IOCTLs
*/    
/*    
    if (com_slave_dmod=='B')
    {
        SubProgram_Start("/bin/mpci","-gpio set 0 1");
        SubProgram_Start("/bin/mpci","-gpio set 1 0");
    }
    else if(com_slave_dmod=='C')
    {
        SubProgram_Start("/bin/mpci","-gpio set 0 0");
        SubProgram_Start("/bin/mpci","-gpio set 1 0");
    }
    else
    {
        SubProgram_Start("/bin/mpci","-gpio set 0 0");
        SubProgram_Start("/bin/mpci","-gpio set 1 1");
    }
*/
	if (dev==1)
        {	 //COM1
    
        //devCom=devCom1;
    
        /* Open the BSP Device */
        if ((device_fd = open(DEVICE_NAME,O_RDONLY)) >= 0 )
            {
    /* 
            //done in script 
            if (com_slave_dmod=='A')
                {
                ioctl(device_fd,IOCTL_GPIO11_HIGH,NULL);
                ioctl(device_fd,IOCTL_GPIO12_HIGH,NULL);
                info.gpio = 35;         //PA3
                info.value = 0;
                ioctl(device_fd,IOCTL_GPIO_SET,&info);
                }
            else if (com_slave_dmod=='B')
                {
                ioctl(device_fd,IOCTL_GPIO11_LOW,NULL);
                ioctl(device_fd,IOCTL_GPIO12_HIGH,NULL);     
                info.gpio = 35;         //PA3
                info.value = 1;
                ioctl(device_fd,IOCTL_GPIO_SET,&info);
                }
    
            else if (com_slave_dmod=='C')
                {
                ioctl(device_fd,IOCTL_GPIO11_LOW,NULL);
                ioctl(device_fd,IOCTL_GPIO12_LOW,NULL);     
                info.gpio = 35;         //PA3
                info.value = 1;
                ioctl(device_fd,IOCTL_GPIO_SET,&info);
                }
            else
                {
                ioctl(device_fd,IOCTL_GPIO11_HIGH,NULL);
                ioctl(device_fd,IOCTL_GPIO12_HIGH,NULL);     
                info.gpio = 35;         //PA3
                info.value = 0;
                ioctl(device_fd,IOCTL_GPIO_SET,&info);
                }
    */ 
            close(device_fd);
            }
    
        fd = OpenDev(devCom);//open com1
    
        if (fd<0)
            {
            syslog(LOGOPTS,"open com Error %s\n",strerror(errno));  
            return -1;
            }
    }else if(dev==2)
    {

        //devCom=devCom2;
        fd = OpenDev(devCom);//open com2
        if (fd<0)
            {
            syslog(LOGOPTS,"open com2 Error %s\n",strerror(errno));  
            return -1;
            }
    }else return -1;

    printf("\ncombang:%c,%c,%d:%d\n",com_slave_baudrate,com_slave_fmt,fd,com_baudrate[com_slave_baudrate-'A']);
    Set_Speed(fd,com_baudrate[com_slave_baudrate-'A']);   
    if (Set_DataFrame(fd,com_parity[com_slave_fmt-'A'][0]-'0',com_parity[com_slave_fmt-'A'][2]-'0',com_parity[com_slave_fmt-'A'][1],0) == -1)
		{//set Databit:8,stop bit:1,parity:FLOW CONTROL
				syslog(LOGOPTS,"Set Parity Error2 %s\n",strerror(errno));  
                return -1;
		}

    return fd;
}


void calculate_crc(char *buff_begin, int cnt, char *result)
{
    int i, j;
    unsigned short temp, temp2, flag;
    unsigned char tempc;

    temp = 0xffff;

    for ( i = 0; i < cnt; i++ ) 
    {
        temp2=temp & 0xFF00;
        flag=temp & 0x00FF;
        tempc=flag;
        tempc=tempc ^ buff_begin[i];
        temp=temp2+tempc;
        //temp = ( temp ^ buff_begin[ i ] );
        for ( j = 0; j < 8; j++ ) 
        {
            flag = ( temp & 0x0001 );
            temp = ( temp >> 1 );
            if ( flag ) 
            {
                temp = temp ^ 0xa001;
            }
        }
    }

    /* Reverse byte order. */
    temp2 = temp >> 8;
    temp = ( ( temp << 8 ) | temp2 );

    flag=temp;
    flag=flag>>8;
    *(result)=flag;

    flag=temp;
    temp2=255;
    flag=flag&temp2;
    *(result+1)=flag;
}

char ascii_to(char *buff_ascii)
{
    char cs[3];
    unsigned int j;
    cs[0]=*buff_ascii;
    cs[1]=*(buff_ascii+1);
    cs[2]=0;
    sscanf(cs,"%X",&j);

    return j;
}
void to_ascii(unsigned char c, char *buff_ascii)
{
    unsigned char cs[3];
    cs[0]=c>>4;
    cs[1]=c&0x0F;
    if(cs[0]>9)cs[0]=cs[0]+0x37;
    else cs[0]=cs[0]+0x30; 
    if(cs[1]>9)cs[1]=cs[1]+0x37;
    else cs[1]=cs[1]+0x30; 
    *buff_ascii=cs[0];
    *(buff_ascii+1)=cs[1];
}

void calculate_lrc_ascii(char *buff_begin, int cnt, char *result)//char * puchMsg, unsigned char usDataLen )
{
    unsigned char lrcData = 0; 
    int i=0;
    while( i<cnt )
    {
        lrcData += ascii_to(buff_begin+i);
        i+=2;
    }
    lrcData  = ( unsigned char ) ( -( ( char ) lrcData  ) );
    to_ascii(lrcData,result);
}

char calculate_lrc(char *buff_begin, int cnt)
{
    unsigned char lrcData = 0; 
    while( cnt-- )
    {
        lrcData += *buff_begin++;
    }
    lrcData  = ( unsigned char ) ( -( ( char ) lrcData  ) );
    return(lrcData );
}

void trans_to_ascii(char *buff_begin,int len)
{
    char *pw;
    char *pr;
    pw=buff_begin+2*len-2;
    pr=buff_begin+len-1;
    while(pr>=buff_begin)
    {
        to_ascii(*pr, pw);
        pr--;
        pw--;
        pw--;
    }
}


//from p_begin to end
//if ok, return packed data lenght in send_buff_com[]; without ":" and CRLF
int unpack_modbus_com_ascii(int len)
{
    int i;
    char c;
//    char tmplog[300],tmp[80];
/*
    sprintf(tmplog,"\n********Recv ASXII Code:%d bytes\n",len);
    for(i=0;i<len;i++)
    {
        c=*(p_read_buff_com_end+i);
        sprintf(tmp,"%c",c);
        strcat(tmplog,tmp);
    }
    syslog(LOGOPTS,"%s\n",tmplog);
*/

    p_read_buff_com_end+=len;
    if( p_read_buff_com_end < read_buff_com+one_master_datapack )return 0;
    int begin_addr;
    int data_num;
    union _bint2 bi2_tmp,bi2_tmp2;
    char *p_next_begin;
    char *p;
    char lrc_data[3];
    int sendpack_len=0;
    p=p_read_buff_com_end-one_master_datapack;
    p_next_begin=read_buff_com;

    char ZERO=0x30;
    char HEAD=0x3A;
    char ID_COM[2];
    char value_c[2];
    to_ascii(com_slave_id,ID_COM);
    //sprintf(tmplog,"***ID %c%c  packlen %d  ZERO %x",ID_COM[0],ID_COM[1],one_master_datapack,ZERO);
    //syslog(LOGOPTS,"%s\n",tmplog);
    while(p>=read_buff_com)
    {
        if(*p==HEAD && *(p+1)==ID_COM[0]  && *(p+2)==ID_COM[1] &&  *(p+3)==ZERO &&  *(p+one_master_datapack-2)==0x0D &&  *(p+one_master_datapack-1)==0x0A )
        {
            c=*(p+4);
            c=c-ZERO;
            if(c==1 || c==2  || c==3  || c==5  || c==6)
            {
                calculate_lrc_ascii(p+1,one_master_datapack-5,lrc_data);
                //sprintf(tmplog,"\n******check lrc:%c%c : %c%c****\n",*(p+one_master_datapack-4),*(p+one_master_datapack-3),lrc_data[0],lrc_data[1]);
                //syslog(LOGOPTS,"%s\n",tmplog);
                if(lrc_data[0]==*(p+one_master_datapack-4) && lrc_data[1]==*(p+one_master_datapack-3))
                {
                    //now packing valid data
                    if(Endian_sign=='B')
                    {
                        bi2_tmp.c[0]=ascii_to(p+5);
                        bi2_tmp.c[1]=ascii_to(p+7);
                        bi2_tmp2.c[0]=ascii_to(p+9);
                        bi2_tmp2.c[1]=ascii_to(p+11);
                        value_c[0]=bi2_tmp2.c[0];
                        value_c[1]=bi2_tmp2.c[1];

                    }else
                    {
                        bi2_tmp.c[1]=ascii_to(p+5);
                        bi2_tmp.c[0]=ascii_to(p+7);
                        bi2_tmp2.c[1]=ascii_to(p+9);
                        bi2_tmp2.c[0]=ascii_to(p+11);
                        value_c[0]=bi2_tmp2.c[1];
                        value_c[1]=bi2_tmp2.c[0];
                    }
                    data_num=bi2_tmp2.i;
                    if(c==1)
                    {
                        begin_addr=bi2_tmp.i-com_offset_c;
                        sendpack_len=Pack_Data_C_rtu(send_buff_com+1,begin_addr,data_num);//include ';' and one LRC byte.
                        if(sendpack_len>2)
                        {
                            send_buff_com[sendpack_len-1]=calculate_lrc(send_buff_com+1,sendpack_len-2);
                            trans_to_ascii(send_buff_com+1,sendpack_len-1);
                            sendpack_len=2*sendpack_len+1;
                            send_buff_com[0]=HEAD;
                            send_buff_com[sendpack_len-2]=0x0D;
                            send_buff_com[sendpack_len-1]=0x0A;
                        }
                    }
                    if(c==2)
                    {
                        begin_addr=bi2_tmp.i-com_offset_i;
                        sendpack_len=Pack_Data_I_rtu(send_buff_com+1,begin_addr,data_num);
                        if(sendpack_len>2)
                        {
                            send_buff_com[sendpack_len-1]=calculate_lrc(send_buff_com+1,sendpack_len-2);
                            trans_to_ascii(send_buff_com+1,sendpack_len-1);
                            sendpack_len=2*sendpack_len+1;
                            send_buff_com[0]=HEAD;
                            send_buff_com[sendpack_len-2]=0x0D;
                            send_buff_com[sendpack_len-1]=0x0A;
                        }
                    }
                    if(c==3)
                    {
                        begin_addr=bi2_tmp.i-com_offset_r;
                        sendpack_len=Pack_Data_R_rtu(send_buff_com+1,begin_addr,data_num);
                        if(sendpack_len>2)
                        {
                            send_buff_com[sendpack_len-1]=calculate_lrc(send_buff_com+1,sendpack_len-2);
                            trans_to_ascii(send_buff_com+1,sendpack_len-1);
                            sendpack_len=2*sendpack_len+1;
                            send_buff_com[0]=HEAD;
                            send_buff_com[sendpack_len-2]=0x0D;
                            send_buff_com[sendpack_len-1]=0x0A;
                        }
                    }
                    if(c==5)
                    {
                        //printf("\n****setting coil:%x %x:",*(p+10),*(p+11));
                        begin_addr=bi2_tmp.i-com_offset_c;
                        char c_tmp;
                        if(*(p+9)=='F' && *(p+10)=='F' && *(p+11)=='0' && *(p+12)=='0')//close
                        {
                            //printf("close \n");
                            c_tmp=1;
                            Fresh_Datamap_C(1,begin_addr,&c_tmp);
                            for(i=0;i<17;i++)
                            {
                                send_buff_com[i]=*(p+i);
                            }
                            sendpack_len= 17;
                        }
                        if(*(p+9)=='0' && *(p+10)=='0' && *(p+11)=='0' && *(p+12)=='0')//open
                        {
                            //printf("open \n");
                            c_tmp=0;
                            Fresh_Datamap_C(1,begin_addr,&c_tmp);
                            for(i=0;i<17;i++)
                            {
                                send_buff_com[i]=*(p+i);
                            }
                            sendpack_len= 17;
                        }

                    }
                    if(c==6)
                    {
                        begin_addr=bi2_tmp.i-com_offset_r;
                        Fresh_Datamap_R(1,begin_addr,value_c);
                        for(i=0;i<17;i++)
                        {
                            send_buff_com[i]=*(p+i);
                        }
                        sendpack_len= 17;
                    }

                    if(sendpack_len>0)
                    {
                        p_next_begin=p+one_master_datapack;
                        break;
                    }

                }//crc
            }//type c
        }//id 
        p--;
    }//while

    p=p_read_buff_com_end-one_master_datapack+1;
    if(p_next_begin<p)p_next_begin=p;

    //move data to the buffer's beginning.
    p=p_next_begin;
    i=0;
    while(p<p_read_buff_com_end)
    {
        read_buff_com[i]=*p;
        p++;
        i++;
    }
    p_read_buff_com_end=read_buff_com+i;

    //printf("\n-------pack ascii len:%d------\n",sendpack_len);
    return sendpack_len;

}

int unpack_modbus_com_rtu(int len)
{
    int i;
    char c;
    /*
    printf("\nRecv RTU Code:%d bytes: ",len);
    for(i=0;i<len;i++)
    {
        c=*(p_read_buff_com_end+i);
        printf("%02x ",c);
        send_buff_com[i]=c;
    }
    printf("\n"); 
    */ 

    p_read_buff_com_end+=len;
    if( p_read_buff_com_end < read_buff_com+one_master_datapack )return 0;

    int begin_addr;
    int data_num;
    union _bint2 bi2_tmp,bi2_tmp2;
    char *p_next_begin;
    char *p;
    char crc_data[3];
    int sendpack_len=0;
    p=p_read_buff_com_end-one_master_datapack;
    p_next_begin=read_buff_com;
    while(p>=read_buff_com)
    {
        if(*p==com_slave_id)
        {
            c=*(p+1);
            if(c==1 || c==2  || c==3  || c==5  || c==6)
            {
                calculate_crc(p,one_master_datapack-2,crc_data);
                //printf("\n******check crc:%x %x : %x %x****\n",*(p+one_master_datapack-2),*(p+one_master_datapack-1),crc_data[0],crc_data[1]);
                if(crc_data[0]==*(p+one_master_datapack-2) && crc_data[1]==*(p+one_master_datapack-1))
                {
                    //now packing valid data
                    if(Endian_sign=='B')
                    {
                        bi2_tmp.c[0]=*(p+2);
                        bi2_tmp.c[1]=*(p+3);
                        bi2_tmp2.c[0]=*(p+4);
                        bi2_tmp2.c[1]=*(p+5);
                    }else
                    {
                        bi2_tmp.c[1]=*(p+2);
                        bi2_tmp.c[0]=*(p+3);
                        bi2_tmp2.c[1]=*(p+4);
                        bi2_tmp2.c[0]=*(p+5);
                    }
                    data_num=bi2_tmp2.i;
                    if(c==1)
                    {
                        begin_addr=bi2_tmp.i-com_offset_c;
                        sendpack_len=Pack_Data_C_rtu(send_buff_com,begin_addr,data_num);
                        if(sendpack_len>2)calculate_crc(send_buff_com,sendpack_len-2,send_buff_com+sendpack_len-2);
                    }
                    if(c==2)
                    {
                        begin_addr=bi2_tmp.i-com_offset_i;
                        sendpack_len=Pack_Data_I_rtu(send_buff_com,begin_addr,data_num);
                        if(sendpack_len>2)calculate_crc(send_buff_com,sendpack_len-2,send_buff_com+sendpack_len-2);
                    }
                    if(c==3)
                    {
                        begin_addr=bi2_tmp.i-com_offset_r;
                        sendpack_len=Pack_Data_R_rtu(send_buff_com,begin_addr,data_num);
                        if(sendpack_len>2)calculate_crc(send_buff_com,sendpack_len-2,send_buff_com+sendpack_len-2);
                    }
                    if(c==5)
                    {
                        //printf("\n****setting coil:%x %x:",*(p+10),*(p+11));
                        begin_addr=bi2_tmp.i-com_offset_c;
                        char c_tmp;
                        if(*(p+4)==0xFF && *(p+5)==0x00 )//close
                        {
                            //printf("close \n");
                            c_tmp=1;
                            Fresh_Datamap_C(1,begin_addr,&c_tmp);
                            for(i=0;i<8;i++)
                            {
                                send_buff_com[i]=*(p+i);
                            }
                            sendpack_len= 8;
                        }
                        if(*(p+4)==0x00 && *(p+5)==0x00)//open
                        {
                            //printf("open \n");
                            c_tmp=0;
                            Fresh_Datamap_C(1,begin_addr,&c_tmp);
                            for(i=0;i<8;i++)
                            {
                                send_buff_com[i]=*(p+i);
                            }
                            sendpack_len= 8;
                        }

                    }
                    if(c==6)
                    {
                        begin_addr=bi2_tmp.i-com_offset_r;
                        Fresh_Datamap_R(1,begin_addr,(p+4));
                        for(i=0;i<8;i++)
                        {
                            send_buff_com[i]=*(p+i);
                        }
                        sendpack_len= 8;
                    }

                    if(sendpack_len>0)
                    {
                        p_next_begin=p+one_master_datapack;
                        //recv_count++;
                        //printf("=======Recv:%d=====\n",recv_count);
                        break;
                    }

                }//crc
            }//type c
        }//id 
        p--;
    }//while

    p=p_read_buff_com_end-one_master_datapack+1;
    if(p_next_begin<p)p_next_begin=p;

    //move data to the buffer's beginning.
    p=p_next_begin;
    i=0;
    while(p<p_read_buff_com_end)
    {
        read_buff_com[i]=*p;
        p++;
        i++;
    }
    p_read_buff_com_end=read_buff_com+i;

    //printf("\n-------pack len:%d------\n",sendpack_len);
    return sendpack_len;
}

//send all one time or not return
//if ASCII, first send ":"(0x3A) and end with CR/LF(0x0D 0x0A)
int send_modbus_com()
{
    int com_error_send_cnt=0;
    //char tmplog[300],tmp[80];
    //sprintf(tmplog,"=======Send %d:",send_buff_len_com);

    int i=0;
    int j=0;
    //for(i=0;i<send_buff_len_com;i++) {sprintf(tmp,"%02X ",*(send_buff_com+i));
    //strcat(tmplog,tmp);
    //}

    i=0;
    j=0;
    while(i<send_buff_len_com)
    {
        j=write(fdcom, (send_buff_com+i), send_buff_len_com-i);
        //j=write(fdcom, (send_buff_com+i), 1);
        if ( j< 0)
        {
            //sprintf(tmp,"send error:%s\n",strerror(errno));
            //strcat(tmplog,tmp);
            if(errno==EINTR || errno==ENOSPC || errno==EAGAIN || errno==EWOULDBLOCK)
            {
                com_error_send_cnt++;
                if(com_error_send_cnt==20)sleep(1);
                if(com_error_send_cnt==40)sleep(1);
                if(com_error_send_cnt<60)continue;
                else  return -1;
            }else return -1;//error
        }
        com_error_send_cnt=0;
        i+=j;
    }
    //sprintf(tmp,"+%d ",j);
    //strcat(tmplog,tmp);
    //syslog(LOGOPTS,"%s\n",tmplog);

    return i;
}

void print_ascii_test()
{
    //printf ascii code of: 1~255 
    //then translate bak to num.
    int i,j;
    char c1,cs[10];
    printf("\n**********Here for test: int->ascii->int*********\n");
    for (i=0;i<255;i++)
    {
        printf("%d  :",i);
        sprintf(cs,"%02x",i);
        printf("%s  :",cs);
        sscanf(cs,"%x",&j);
        printf("%d\n",j);
    }
    printf("\n***********End*********\n");
}

int Fresh_UCI_ctx()
{
    uci_free_context(ctx);
    ctx_fresh_cnt=0;

    ctx=NULL;
    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        fprintf(stderr, "Out of memory\n");
        return -1;
    }
    return 0;
}


void *com_slave_thread_function(void *arg)
{
    char tmp_buff[10];
    //print_ascii_test();
    int com_state_ok;
    int i_tmp,i;

    if(com_slave_id<1 || com_slave_id>254)
    {
        syslog(LOGOPTS,"Modbus com slave parameter is wrong. Terminated.\n");
        return NULL;
    }
    if(com_offset_c<0 || com_offset_c>65535 || com_offset_i<0 || com_offset_i>65535 || com_offset_r<0 || com_offset_r>65535)
    {
        syslog(LOGOPTS,"Modbus com slave offset parameter is wrong. Terminated.\n");
        return NULL;
    }
    if(com_slave_ctimeout<1)com_slave_ctimeout=5;

    com_state_ok=0;
    fdcom=0;

while(1)
{
    if(com_state_ok==0)
    {
        if(fdcom>0)
        {
            close(fdcom);
            fdcom=0;
        }
        if(com_slave_enable=='B' || com_slave_enable=='C')
        {
            tmp_buff[0]=0;
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", tmp_buff);
            if(tmp_buff[0]=='B')
            {
                sleep(5);
                Fresh_UCI_ctx();
            }else
            {
                com_port_id=2;
                com_modbus_type=com_slave_enable-'A';
                fdcom=0;
                if ((fdcom=COM_Set_Parameters(com_port_id))==-1)
                {    /*notice this function return -1 */
                     syslog(LOGOPTS,"Modbus COM_Set_Parameters ==-1 exit\n");
                     return NULL;
                }
                com_state_ok=1;
            }
        }
        if(com_modbus_type==2)one_master_datapack=8;
        else one_master_datapack=17;
        if(com_state_ok==1)
        {
            send_buff_len_com=0;
            p_read_buff_com_end=read_buff_com;
        }
    }
    if(com_state_ok!=1)continue;

    //com state ok.
    
    send_buff_len_com=0;
    i_tmp=read( fdcom, p_read_buff_com_end, 2*one_master_datapack);
    if(i_tmp<0)
    {
        if(errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)
        {
            com_state_ok=0;
            continue;
        }
    }
    if(i_tmp==0)//may be closed by client, need to re accept
    {
        ;
    }
    if(i_tmp>0)// need consider max time out
    {
        send_buff_len_com=0;
        if(com_modbus_type==1)
        {
            send_buff_len_com=unpack_modbus_com_ascii(i_tmp);
        }else 
        send_buff_len_com=unpack_modbus_com_rtu(i_tmp);
        //printf("\n*******recv:%d, pack send to:%d*******\n",i_tmp,send_buff_len_com);
    }

    if(send_buff_len_com>0)
    {
        fresh_timeout_max_com=0;
        if(send_modbus_com()<0)com_state_ok=0;
        send_buff_len_com=0;
    }

    sleep(1);

}//while 1

}


void *Fresh_Datamap_Check(void *arg)
{
    int i;
    char tmp_buff[20];
    fresh_timeout_max_com=0;
    ctx_fresh_cnt=0;
printf("FRESH DATA Thread.........\n");


    while(1)
    {
        if(fresh_timeout_max_com>300)//5 minutes if there is no valid data required.
        {
            fresh_timeout_max_com=0;
            if(Fresh_UCI_ctx()<0)exit(0);
            for(i=0;i<MODBUS_DMAP_C_MAX;i++)DataMapFreshSet_C[i]=0;
            for(i=0;i<MODBUS_DMAP_I_MAX;i++)DataMapFreshSet_I[i]=0;
            for(i=0;i<MODBUS_DMAP_R_MAX;i++)DataMapFreshSet_R[i]=0;
        }
        fresh_timeout_max_com++;
        sleep(1);
        ctx_fresh_cnt++;
        if(ctx_fresh_cnt>30)ctx_fresh_cnt=0;

        for(i=0;i<MODBUS_DMAP_C_MAX;i++)
        {
            if(DataMapFresh_C[i]==0 && DataMapFreshSet_C[i]==3)
            {
                if(ctx_fresh_cnt>2)
                {
                    ctx_fresh_cnt=0;
                    if(Fresh_UCI_ctx()<0)exit(0);
                }
                Fresh_Datamap_C(0,i,tmp_buff);
                DataMapFreshSet_C[i]=2;
            }
            if(DataMapFresh_C[i]>0 && DataMapFresh_C[i]<99) 
            {
                DataMapFresh_C[i]=DataMapFresh_C[i]-1;
            }
        }
        for(i=0;i<MODBUS_DMAP_I_MAX;i++)
        {
            if(DataMapFresh_I[i]==0 && DataMapFreshSet_I[i]==3)
            {
                if(ctx_fresh_cnt>2)
                {
                    ctx_fresh_cnt=0;
                    if(Fresh_UCI_ctx()<0)exit(0);
                }
                Fresh_Datamap_I(0,i,tmp_buff);
                DataMapFreshSet_I[i]=2;
            }
            if(DataMapFresh_I[i]>0 && DataMapFresh_I[i]<99) 
            {
                DataMapFresh_I[i]=DataMapFresh_I[i]-1;
            }
        }
        for(i=0;i<MODBUS_DMAP_R_MAX;i++)
        {
            if(DataMapFresh_R[i]==0 && DataMapFreshSet_R[i]==3)
            {
                if(ctx_fresh_cnt>2)
                {
                    ctx_fresh_cnt=0;
                    if(Fresh_UCI_ctx()<0)exit(0);
                }
                Fresh_Datamap_R(0,i,tmp_buff);
                DataMapFreshSet_R[i]=2;
            }
            if(DataMapFresh_R[i]>0 && DataMapFresh_R[i]<99) 
            {
                DataMapFresh_R[i]=DataMapFresh_R[i]-1;
            }
        }

    }


}

void check_big_endian(void)
{
    union _bint2 bint2;
    bint2.i = 0x0102;
    if(bint2.c[0] == 1)
    {
        Endian_sign='B';
//        printf("***big_2bytes 0x0102: %d:%d,%d\n",bint2.i,bint2.c[0],bint2.c[1]);
    }else 
        Endian_sign='L';
}

int read_conf()
{
    char tmp_buff[35];
    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_Enable", tmp_buff);
    if(tmp_buff[0]!='B')return -1;

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_en", tmp_buff);
    if(tmp_buff[0]=='B')tcp_slave_enable='B';
    else tcp_slave_enable='A';

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_port", tmp_buff);
    tcp_slave_port=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_timeout", tmp_buff);
    tcp_idletimeout_max=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_id", tmp_buff);
    tcp_slave_id=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_C", tmp_buff);
    tcp_offset_c=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_I", tmp_buff);
    tcp_offset_i=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_R", tmp_buff);
    tcp_offset_r=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_IPF_EN", tmp_buff);
    if(tmp_buff[0]=='B')tcp_IPfilter_en='B';
    else tcp_IPfilter_en='A';

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_IPF1", tmp_buff);
    tcp_IPFilter[0]=inet_addr(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_IPF2", tmp_buff);
    tcp_IPFilter[1]=inet_addr(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_IPF3", tmp_buff);
    tcp_IPFilter[2]=inet_addr(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_TCP_IPF4", tmp_buff);
    tcp_IPFilter[3]=inet_addr(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_COM_en", tmp_buff);
    if(tmp_buff[0]=='B')com_slave_enable='B';
    else if(tmp_buff[0]=='C')com_slave_enable='C';
    else com_slave_enable='A';

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_COM_rate", tmp_buff);
    com_slave_baudrate=tmp_buff[0];

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_COM_format", tmp_buff);
    com_slave_fmt=tmp_buff[0];

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_COM_chmode", tmp_buff);
    com_slave_dmod=tmp_buff[0];

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_COM_timeout", tmp_buff);
    com_slave_ctimeout=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_COM_id", tmp_buff);
    com_slave_id=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_COM_C", tmp_buff);
    com_offset_c=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_COM_I", tmp_buff);
    com_offset_i=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "modbusd", "modbus_conf", "Modbus_S_COM_R", tmp_buff);
    com_offset_r=atoi(tmp_buff);

    return 0;
}

int main( int argc, char *argv[])
{
    if(argc==2)
    {
        if(strcmp(argv[1],"dmap")==0)
        {
            int i;
            FILE *sfp=NULL;
            char buff[250];
            if ((sfp = fopen("/var/run/modbusdmap", "w")) == 0) 
            {
                syslog(LOGOPTS,"SMS fopen web out put file error\n");
                return 0;
            }
            strcpy(buff,"<strong>Coil Bits (Output and Internal Status):</strong><table width=60% cellpadding=3><tr><td width=15%>Bit Address</td><td width=20%>Hex Format</td><td>Definition</td></tr>\r\n");
            fprintf(sfp,"%s",buff); 
            for(i=0;i<MODBUS_DMAP_C_MAX;i++)
            {
                if(strlen(Modbus_DMAP_C_name[i])<2)continue;
                if(Modbus_DMAP_C_name[i][0]!='*')
                    fprintf(sfp, "<tr><td>%d</td><td>0x%04x</td><td>%s</td></tr>\r\n",i,i,Modbus_DMAP_C_name[i]);
            }
            fprintf(sfp, "</table><br>\r\n");

            strcpy(buff,"<strong>Input Bits:</strong><table width=60% cellpadding=3><tr><td width=15%>Bit Address</td><td width=20%>Hex Format</td><td>Definition</td></tr>\r\n");
            fprintf(sfp,"%s",buff); 
            for(i=0;i<MODBUS_DMAP_I_MAX;i++)
            {
                if(strlen(Modbus_DMAP_I_name[i])<2)continue;
                if(Modbus_DMAP_I_name[i][0]!='*')
                    fprintf(sfp, "<tr><td>%d</td><td>0x%04x</td><td>%s</td></tr>\r\n",i,i,Modbus_DMAP_I_name[i]);
            }
            fprintf(sfp, "</table><br>\r\n");

            strcpy(buff,"<strong>Registers:</strong><table width=60% cellpadding=3><tr><td width=15%>16 Bits Address</td><td width=20%>Hex Format</td><td>Definition</td></tr>\r\n");
            fprintf(sfp,"%s",buff); 
            for(i=0;i<MODBUS_DMAP_R_MAX;i++)
            {
                if(strlen(Modbus_DMAP_R_name[i])<2)continue;
                if(Modbus_DMAP_R_name[i][0]!='*')
                    fprintf(sfp, "<tr><td>%d</td><td>0x%04x</td><td>%s</td></tr>\r\n",i,i,Modbus_DMAP_R_name[i]);
            }
            fprintf(sfp, "</table><br>\r\n");

            strcpy(buff,"<strong>Modem Model Types:</strong><table width=60% cellpadding=2><tr><td width=15%>Type ID</td><td>Definition</td></tr>\r\n");
            fprintf(sfp,"%s",buff); 
            for(i=0;i<MODBUS_DODEM_MODEL_MAX;i++)
            {
                if(strlen(Modbus_Dodem_Model_type[i])<2 || strcmp(Modbus_Dodem_Model_type[i],"To be defined")==0)continue;
                fprintf(sfp, "<tr><td>%d</td><td>%s</td></tr>\n",i,Modbus_Dodem_Model_type[i]);
            }
            fprintf(sfp, "</table><br>\n");

            strcpy(buff,"<strong>Com Data Format Definition:</strong><table width=60% cellpadding=2><tr><td width=15%>Type ID</td><td>Definition</td></tr>\r\n");
            fprintf(sfp,"%s",buff); 
            for(i=0;i<MODBUS_COM_DATAFMT_MAX;i++)
            {
                if(strlen(Modbus_Com_Data_Format[i])<2)continue;
                fprintf(sfp, "<tr><td>%d</td><td>%s</td></tr>\n",i,Modbus_Com_Data_Format[i]);
            }
            fprintf(sfp, "</table><br>\n");

            fflush(sfp);
            fclose(sfp);
            return 0;
        }
    }


    //system initial
    check_big_endian();

    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        fprintf(stderr, "Out of memory\n");
        return -1;
    }
    if(read_conf()<0)
    {
        syslog(LOGOPTS,"Modbus slave service is disabled and terminated.\n");
        return 0;
    }
    if(tcp_slave_enable=='A' && com_slave_enable=='A') 
    {
        syslog(LOGOPTS,"Modbus slave service is not active and terminated.\n");
        return 0;
    }


    syslog(LOGOPTS,"modbusds main loop begin...\n");
    pthread_create(&fresh_data_pt,NULL,Fresh_Datamap_Check,NULL);//(void*)(&fdcom);
    if(tcp_slave_enable=='B')
    {
        if(com_slave_enable>'A' && com_slave_enable<'F')
        {
            //read config from db1[] to local
            pthread_create(&com_slave_pt,NULL,com_slave_thread_function,NULL);//(void*)(&fdcom);
        }

        tcp_slave_function();// while running

    }else 
    {
        if(com_slave_enable>'A' && com_slave_enable<'F')
        {
            com_slave_thread_function(NULL);// while running
        }
    }


    //...
    //end of program
    if(tcp_slave_enable=='B' && com_slave_enable!='A')
        pthread_cancel(com_slave_pt);
    pthread_cancel(fresh_data_pt);

}









