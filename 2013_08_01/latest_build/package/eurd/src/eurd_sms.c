//#define DEBUG_PRINT 
/*
 * event udp reporting for VIP4G
 *
 * a simple program to collect data base different events happened and send them
 * to remote server
 *
 */
#include "eurd.h"
#include <zlib.h>

#if !defined (INADDR_NONE)
    #define INADDR_NONE   ((in_addr_t)-1)
#endif

int AERC_REPORT_NUM;

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

union _bint4{
        uint32_t i;
        char c[4];
    };
    union _bint2{
        uint16_t i;
        char c[2];
    };
    union _bint8{
        uint64_t i;
        char c[8];
    };
void check_big_endian(void)
{
    union _bint2 bint2;
    union _bint4 bint4;
    union _bint8 bint8;
    bint2.i = 0x0102;
    bint4.i = 0x01020304;
    bint8.i = UINT64_C(0x0102030405060708);
    if(bint2.c[0] == 1)
    {
        printf("***big_2bytes 0x0102: %d:%d,%d\n",bint2.i,bint2.c[0],bint2.c[1]);
    }else
    {
        printf("***little_2bytes 0x0102: %d:%d,%d\n",bint2.i,bint2.c[0],bint2.c[1]);
    }

    if(bint4.c[0] == 1)
    {
        printf("***big_4bytes 0x01020304: %d:%d,%d,%d,%d\n",bint4.i,bint4.c[0],bint4.c[1],bint4.c[2],bint4.c[3]);
    }else
    {
        printf("***little_4bytes 0x01020304: %d:%d,%d,%d,%d\n",bint4.i,bint4.c[0],bint4.c[1],bint4.c[2],bint4.c[3]);
    }

    if(bint8.c[0] == 1)
    {
        printf("***big_8bytes 0x0102030405060708: %llu:%d,%d,%d,%d,%d,%d,%d,%d\n",bint8.i,bint8.c[0],bint8.c[1],bint8.c[2],bint8.c[3],bint8.c[4],bint8.c[5],bint8.c[6],bint8.c[7]);
    }else
    {
        printf("***little_8bytes 0x0102030405060708: %llu:%d,%d,%d,%d,%d,%d,%d,%d\n",bint8.i,bint8.c[0],bint8.c[1],bint8.c[2],bint8.c[3],bint8.c[4],bint8.c[5],bint8.c[6],bint8.c[7]);
    }

}

static int scpytrim(char *dst, const char *src, int len)
{
    char *d = dst;
    const char *s = src;
    int i, j = 0;

    for (i=0; i<len; i++) {
//        if (s[i]=='\r' || s[i]=='\n' || s[i]=='\0' || s[i]==' ') {
        if (s[i]=='\r' || s[i]=='\n' || s[i]=='\0' ) {
            continue;
        }
        d[j] = s[i];
        j++;
    }
    return j;
}


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

static void check_current_apn(char *apnstr)
{
    char buff[100];
    FILE *f;
    char *p;
    char *p1;
    int i;
    f=fopen("/var/run/autoapn","r");
    if(f==NULL)return;

    if(fgets(buff,98,f))
    {
        p=buff;
        while(*p==' ')p++;
        p1=p;
        while(*p!=' ' && *p!=0 )p++;
        if(*p==' ')
        {
            i=p-p1;
            if(i>=30)i=29;
            if(i>=2)
            {
                strncpy(apnstr,p1,i);
                apnstr[i]=0;
            }
        }
    }
    fclose(f);
    return;
}


static void refresh_eventinfo_buff()
{
    char buff[30];
    char buffer_get[256];
    FILE* f;
    char *p;
    int j;
    if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RR][AERC_REPORT_NUM]=='B') 
    {

        strcpy(buff,"");
        if (!(f = fopen("/var/run/std_lte_statue", "r"))) 
        {
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
                        sprintf(buff,"%s",p);
                    }
                }

#if 0
                p=NULL;
                p = strstr(buffer_get, "imei=");//imei=" 012773002002648"
                j=0;
                if (p != NULL)
                {
                    p+=strlen("imei=")+1;
                    while (*p == ' ')p++; //first space
                    while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                        {
                            buff[j]=*p;
                            p++;
                            j++;
                         }
                    buff[j]=0;
                }//if p
                if(j>1)break; 
 #endif
            }//while
        }//else
        if(f)fclose(f);


       if (sscanf(buff,"%llu",&(eur_buff.modem_id))<1) {
            syslog(LOGOPTS,"EURD: %s Failed to convert IMEI string to uint64_t for report%d mep_package!\n",__FUNCTION__,AERC_REPORT_NUM);
            eur_buff.modem_id=0;
        }
        int j=0,h=0,l=0;
        for (j=0;j<6;j++) {
            if (eur_device_buff.macAddr[2*j]>='0'&&eur_device_buff.macAddr[2*j]<='9') {
                h=eur_device_buff.macAddr[2*j]-'0';
            } else if (eur_device_buff.macAddr[2*j]>='a'&&eur_device_buff.macAddr[2*j]<='f') {
                h=eur_device_buff.macAddr[2*j]-'a'+10;
            } else if (eur_device_buff.macAddr[2*j]>='A'&&eur_device_buff.macAddr[2*j]<='F') {
                h=eur_device_buff.macAddr[2*j]-'A'+10;
            }
            if (eur_device_buff.macAddr[2*j+1]>='0'&&eur_device_buff.macAddr[2*j+1]<='9') {
                l=eur_device_buff.macAddr[2*j+1]-'0';
            } else if (eur_device_buff.macAddr[2*j+1]>='a'&&eur_device_buff.macAddr[2*j+1]<='f') {
                l=eur_device_buff.macAddr[2*j+1]-'a'+10;
            } else if (eur_device_buff.macAddr[2*j+1]>='A'&&eur_device_buff.macAddr[2*j+1]<='F') {
                l=eur_device_buff.macAddr[2*j+1]-'A'+10;
            }
            eur_buff.mac[j]=h*16+l;
        }
    }//if 'B'
}


static void refresh_device_buff()
{    
    char proname[256];
    int j;

    //host name may be changed online.
    strcpy(proname,"");
    if(get_option_by_section_name(ctx, "system", "@system[0]", "hostname", proname))
    {
        strcpy(eur_device_buff.deviceName,proname);
    }

    if (false==fetch_Local_IP_MAC(ifname_bridge,proname))fetch_Local_IP_MAC(ifname_eth0,proname);
    bzero(eur_device_buff.macAddr,sizeof(eur_device_buff.macAddr));
    for (j=0;j<6;j++) 
    {
        eur_device_buff.macAddr[2*j] = proname[3*j];
        eur_device_buff.macAddr[2*j+1] = proname[3*j+1];
    }

    if(get_option_by_section_name(ctx, "httpd", "@httpd[0]", "port", proname))//httpd.@httpd[0].port=80
    {
        strcpy(eur_device_buff.httpPort,proname);
    }
}

static void refresh_json_network_buff()
{
    char proname[32];
    int i,j,col;
    FILE* f;
    char buffer_netstate[256];
    char packet_statistics_buff[13];
    char *p;
    int trans_sign;


    if(p_eur_eths_buff[0]!=NULL) //lan
    {

        strcpy(p_eur_eths_buff[0]->receivedBytes,"0");
        strcpy(p_eur_eths_buff[0]->receivedPackets,"0");
        strcpy(p_eur_eths_buff[0]->transmittedBytes,"0");
        strcpy(p_eur_eths_buff[0]->transmittedPackets,"0");

        strcpy(p_eur_eths_buff[0]->macAddr,eur_device_buff.macAddr);
        strcpy(p_eur_eths_buff[0]->ipAddr,"N/A");
        if (false==fetch_Local_IP_ADDR(ifname_bridge,p_eur_eths_buff[0]->ipAddr))fetch_Local_IP_ADDR(ifname_eth0,p_eur_eths_buff[0]->ipAddr);
        strcpy(p_eur_eths_buff[0]->ipMask,"255.255.255.0");
        if (false==fetch_Local_IP_MASK(ifname_bridge,p_eur_eths_buff[0]->ipMask))fetch_Local_IP_MASK(ifname_eth0,p_eur_eths_buff[0]->ipMask);

        trans_sign=1;
        p=NULL;
        if (!(f = fopen("/proc/net/dev", "r"))) {
            trans_sign=0;
        }else
        {
            while (fgets(buffer_netstate, 256, f)) {
            p = strstr(buffer_netstate, ifname_bridge);
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
            p += strlen(ifname_bridge);       
            p++;
            for (col=0;col<16;col++) {
                while (*p == ' ')
                    p++;

                j=0;
                while ((*p != ' ')&&(j<11)) {
                    packet_statistics_buff[j] = *p;
                    p++;
                    j++;
                }

                packet_statistics_buff[j]= 0;                  
                if(j>0){
                    if (col==0) {
                        sprintf(p_eur_eths_buff[0]->receivedBytes,"%s",packet_statistics_buff);
                    }
                    if (col==1) {
                        sprintf(p_eur_eths_buff[0]->receivedPackets,"%s",packet_statistics_buff);
                    }
                    if (col==8) {
                        sprintf(p_eur_eths_buff[0]->transmittedBytes,"%s",packet_statistics_buff);
                    }
                    if (col==9) {
                        sprintf(p_eur_eths_buff[0]->transmittedPackets,"%s",packet_statistics_buff);
                    }
                }
            }
        }
    }

    if(p_eur_eths_buff[1]!=NULL) //wan
    {
        strcpy(p_eur_eths_buff[1]->receivedBytes,"0");
        strcpy(p_eur_eths_buff[1]->receivedPackets,"0");
        strcpy(p_eur_eths_buff[1]->transmittedBytes,"0");
        strcpy(p_eur_eths_buff[1]->transmittedPackets,"0");

        if (false==fetch_Local_IP_MAC(ifname_wan,proname))fetch_Local_IP_MAC(ifname_eth1,proname);
        bzero(p_eur_eths_buff[1]->macAddr,sizeof(p_eur_eths_buff[1]->macAddr));
        for (j=0;j<6;j++) 
        {
            p_eur_eths_buff[1]->macAddr[2*j] = proname[3*j];
            p_eur_eths_buff[1]->macAddr[2*j+1] = proname[3*j+1];
        }
        strcpy(p_eur_eths_buff[1]->ipAddr,"N/A");
        if (false==fetch_Local_IP_ADDR(ifname_wan,p_eur_eths_buff[1]->ipAddr))fetch_Local_IP_ADDR(ifname_eth1,p_eur_eths_buff[1]->ipAddr);
        strcpy(p_eur_eths_buff[1]->ipMask,"255.255.255.0");
        if (false==fetch_Local_IP_MASK(ifname_wan,p_eur_eths_buff[1]->ipMask))fetch_Local_IP_MASK(ifname_eth1,p_eur_eths_buff[1]->ipMask);

        trans_sign=1;
        p=NULL;
        if (!(f = fopen("/proc/net/dev", "r"))) {
            trans_sign=0;
        }else
        {
            while (fgets(buffer_netstate, 256, f)) {
            p = strstr(buffer_netstate, "br-wan:");
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
            p += strlen(ifname_wan);       
            p++;
            for (col=0;col<16;col++) 
            {
                while (*p == ' ')
                    p++;

                j=0;
                while ((*p != ' ')&&(j<11)) {
                    packet_statistics_buff[j] = *p;
                    p++;
                    j++;
                }
 
                packet_statistics_buff[j]= 0;
                if(j>0) 
                {
                    if (col==0) {
                        sprintf(p_eur_eths_buff[1]->receivedBytes,"%s",packet_statistics_buff);
                    }
                    if (col==1) {
                        sprintf(p_eur_eths_buff[1]->receivedPackets,"%s",packet_statistics_buff);
                    }
                    if (col==8) {
                        sprintf(p_eur_eths_buff[1]->transmittedBytes,"%s",packet_statistics_buff);
                    }
                    if (col==9) {
                        sprintf(p_eur_eths_buff[1]->transmittedPackets,"%s",packet_statistics_buff);
                    }
                }
            }
        }
    }
    if(p_eur_radios_buff[0]!=NULL) //wifi
    {
        if (false==fetch_Local_IP_MAC(ifname_radio,proname))fetch_Local_IP_MAC(ifname_radio_mon,proname);
        bzero(p_eur_radios_buff[0]->macAddr,sizeof(p_eur_radios_buff[0]->macAddr));
        for (j=0;j<6;j++) 
        {
            p_eur_radios_buff[0]->macAddr[2*j] = proname[3*j];
            p_eur_radios_buff[0]->macAddr[2*j+1] = proname[3*j+1];
        }
        strcpy(p_eur_radios_buff[0]->ipAddr,"N/A");
        if (false==fetch_Local_IP_ADDR(ifname_radio,p_eur_radios_buff[0]->ipAddr))fetch_Local_IP_ADDR(ifname_radio_mon,p_eur_radios_buff[0]->ipAddr);


        trans_sign=1;
        p=NULL;
        if (!(f = fopen("/proc/net/dev", "r"))) {
            trans_sign=0;
        }else
        {
            while (fgets(buffer_netstate, 256, f)) {
            p = strstr(buffer_netstate, ifname_radio);
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
            p += strlen(ifname_radio);       
            p++;
            for (col=0;col<16;col++) {
                while (*p == ' ')
                    p++;

                j=0;
                while ((*p != ' ')&&(j<11)) {
                    packet_statistics_buff[j] = *p;
                    p++;
                    j++;
                }

                packet_statistics_buff[j]= 0;                  
                if (col==0) {
                    sprintf(p_eur_radios_buff[0]->receivedBytes,"%s",packet_statistics_buff);
                }
                if (col==1) {
                    sprintf(p_eur_radios_buff[0]->receivedPackets,"%s",packet_statistics_buff);
                }
                if (col==8) {
                    sprintf(p_eur_radios_buff[0]->transmittedBytes,"%s",packet_statistics_buff);
                }
                if (col==9) {
                    sprintf(p_eur_radios_buff[0]->transmittedPackets,"%s",packet_statistics_buff);
                }
            }
        }else
        {
            strcpy(p_eur_radios_buff[0]->receivedBytes,"0");
            strcpy(p_eur_radios_buff[0]->receivedPackets,"0");
            strcpy(p_eur_radios_buff[0]->transmittedBytes,"0");
            strcpy(p_eur_radios_buff[0]->transmittedPackets,"0");
        }



    }
    if(p_eur_carrs_buff[0]!=NULL) //carrier
    {
        strcpy(p_eur_carrs_buff[0]->ipAddr,"N/A");
        char ippass_buff[20];
        ippass_buff[0]=0;
        get_option_by_section_name(ctx, "lte", "connect", "ippassthrough", ippass_buff);
        if(strcmp(ippass_buff,"Ethernet")==0)
        {
            if (!(f = fopen("/var/run/ippth_4g", "r")))
            {
                ;
            }else
            {
                buffer_netstate[0]=0;
                if(fgets(buffer_netstate, 256, f))
                {
                    if(strlen(buffer_netstate)>10)
                    {
                        p=NULL;
                        p = strstr(buffer_netstate, "on");
                        if(p!=NULL)
                        {
                            *p=0;
                            if(*(p-1)==' ')
                            {
                                p=p-1;
                                *p=0;
                            }
                            strcpy(p_eur_carrs_buff[0]->ipAddr,buffer_netstate);
                        }
                    }
                }
            }

        }else
        {
            if (false==fetch_Local_IP_ADDR(ifname_carrier,p_eur_carrs_buff[0]->ipAddr))fetch_Local_IP_ADDR(ifname_eth2,p_eur_carrs_buff[0]->ipAddr);
        }

        strcpy(p_eur_carrs_buff[0]->apn,"N/A");
        get_option_by_section_name(ctx, "lte", "connect", "apn", p_eur_carrs_buff[0]->apn);
        check_current_apn(p_eur_carrs_buff[0]->apn);

        trans_sign=1;
        p=NULL;
        if (!(f = fopen("/proc/net/dev", "r"))) {
            trans_sign=0;
        }else
        {
            while (fgets(buffer_netstate, 256, f)) {
            p = strstr(buffer_netstate, ifname_carrier);
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
            p += strlen(ifname_carrier);        
            p++;
            for (col=0;col<16;col++) {
                while (*p == ' ')
                    p++;

                j=0;
                while ((*p != ' ')&&(j<11)) {
                    packet_statistics_buff[j] = *p;
                    p++;
                    j++;
                }

                packet_statistics_buff[j]= 0;                   
                if (col==0) {
                    sprintf(p_eur_carrs_buff[0]->receivedBytes,"%s",packet_statistics_buff);
                }
                if (col==1) {
                    sprintf(p_eur_carrs_buff[0]->receivedPackets,"%s",packet_statistics_buff);
                }
                if (col==8) {
                    sprintf(p_eur_carrs_buff[0]->transmittedBytes,"%s",packet_statistics_buff);
                }
                if (col==9) {
                    sprintf(p_eur_carrs_buff[0]->transmittedPackets,"%s",packet_statistics_buff);
                }
            }
        }else
        {
            strcpy(p_eur_carrs_buff[0]->receivedBytes,"0");
            strcpy(p_eur_carrs_buff[0]->receivedPackets,"0");
            strcpy(p_eur_carrs_buff[0]->transmittedBytes,"0");
            strcpy(p_eur_carrs_buff[0]->transmittedPackets,"0");
        }
    }

}



static void refresh_json_com_buff()
{
///var/run/com2_packet---4G
    char proname[32];
    char tx_buff[20],rx_buff[20];
    int i,j,col;
    FILE* f;
    char buffer_netstate[256];
    char packet_statistics_buff[13];
    char *p;
    int trans_sign;
    if(p_eur_coms_buff[0]==NULL)return;

    //cat /proc/tty/driver/serial                      
    //serinfo:1.0 driver revision:                                                    
    //0: uart:16550A mmio:0x18020000 irq:11 tx:118628 rx:3287 RTS|DTR|DSR|CD   
/*
    strcpy(tx_buff,"0");
    strcpy(rx_buff,"0");
    p=NULL;
    if ((f = fopen("/proc/tty/driver/serial", "r"))) 
    {
        while(fgets(buffer_netstate, 256, f))
        {

            p = strstr(buffer_netstate, "tx:");
            if (p != NULL)
            {
                p=p+3;
                j=0;
                while ((*p != ' ')&&(j<20)) 
                {
                    tx_buff[j] = *p;
                    p++;
                    j++;
                }
                tx_buff[j]=0;
            }

            p = strstr(buffer_netstate, "rx:");
            if (p != NULL)
            {
                p=p+3;
                j=0;
                while ((*p != ' ')&&(j<20)) 
                {
                    rx_buff[j] = *p;
                    p++;
                    j++;
                }
                rx_buff[j]=0;
            }


        }
    }
    if(f)fclose(f);
    strcpy(p_eur_coms_buff[0]->receivedBytes,rx_buff);
    strcpy(p_eur_coms_buff[0]->receivedPackets,"0");
    strcpy(p_eur_coms_buff[0]->transmittedBytes,tx_buff);
    strcpy(p_eur_coms_buff[0]->transmittedPackets,"0");
*/
    strcpy(p_eur_coms_buff[0]->receivedBytes,"0");
    strcpy(p_eur_coms_buff[0]->receivedPackets,"0");
    strcpy(p_eur_coms_buff[0]->transmittedBytes,"0");
    strcpy(p_eur_coms_buff[0]->transmittedPackets,"0");

    proname[0]=0;
    get_option_by_section_name(ctx,"comport2","com2","COM2_Port_Status",proname);  //disable
    if(proname[0]=='A') 
    {
        //no change as default values
        return;
    }

    p=NULL;
    col=0;
    if (!(f = fopen("/var/run/com2_packet", "r"))) {/* the file must be there */
        trans_sign=0;
    }else
    {
        while(fgets(buffer_netstate, 256, f)) 
        {
            col++;
            j=strlen(buffer_netstate);
            if(col==1 && j<20)
            {
                while(j>0)
                {
                    j--;
                    if(buffer_netstate[j]=='\r'||buffer_netstate[j]=='\n')buffer_netstate[j]=0;
                }
                strcpy(p_eur_coms_buff[0]->receivedBytes,buffer_netstate);
            }
            if(col==2 && j<20)
            {
                while(j>0)
                {
                    j--;
                    if(buffer_netstate[j]=='\r'||buffer_netstate[j]=='\n')buffer_netstate[j]=0;
                }
                strcpy(p_eur_coms_buff[0]->receivedPackets,buffer_netstate);
            }
            if(col==9 && j<20)
            {
                while(j>0)
                {
                    j--;
                    if(buffer_netstate[j]=='\r'||buffer_netstate[j]=='\n')buffer_netstate[j]=0;
                }
                strcpy(p_eur_coms_buff[0]->transmittedBytes,buffer_netstate);
            }
            if(col==10 && j<20)
            {
                while(j>0)
                {
                    j--;
                    if(buffer_netstate[j]=='\r'||buffer_netstate[j]=='\n')buffer_netstate[j]=0;
                }
                strcpy(p_eur_coms_buff[0]->transmittedPackets,buffer_netstate);
            }
        }
    }
    if(f)fclose(f);

}



static void refresh_json_ioport_buff()
{
    FILE* f;
    char tmp_buf[10];

    if(p_eur_io_buff[0]==NULL)return;
 
 
    strcpy(p_eur_io_buff[0]->InputStatus,"NNNN");
    if(get_option_by_section_name(ctx, "ioports", "input", "input1", tmp_buf))
    {
        p_eur_io_buff[0]->InputStatus[0]=tmp_buf[0];
    }
    else
    {
        if ((f = fopen("/sys/class/button/INPUT1/status", "r"))) 
        {
            if(fgets(tmp_buf, 10, f)) 
            {
                p_eur_io_buff[0]->InputStatus[0]=tmp_buf[0];
            }
        }
        if(f)fclose(f);
    }


    if(get_option_by_section_name(ctx, "ioports", "input", "input2", tmp_buf))
    {
        p_eur_io_buff[0]->InputStatus[1]=tmp_buf[0];
    }
    else
    {
        if ((f = fopen("/sys/class/button/INPUT2/status", "r"))) 
        {
            if(fgets(tmp_buf, 10, f)) 
            {
                p_eur_io_buff[0]->InputStatus[1]=tmp_buf[0];
            }
        }
        if(f)fclose(f);
    }


    if(get_option_by_section_name(ctx, "ioports", "input", "input3", tmp_buf))
    {
        p_eur_io_buff[0]->InputStatus[2]=tmp_buf[0];
    }
    else
    {
        if ((f = fopen("/sys/class/button/INPUT3/status", "r"))) 
        {
            if(fgets(tmp_buf, 10, f)) 
            {
                p_eur_io_buff[0]->InputStatus[2]=tmp_buf[0];
            }
        }
        if(f)fclose(f);
    }


    if(get_option_by_section_name(ctx, "ioports", "input", "input4", tmp_buf))
    {
        p_eur_io_buff[0]->InputStatus[3]=tmp_buf[0];
    }
    else
    {
        if ((f = fopen("/sys/class/button/INPUT4/status", "r"))) 
        {
            if(fgets(tmp_buf, 10, f)) 
            {
                p_eur_io_buff[0]->InputStatus[3]=tmp_buf[0];
            }
        }
        if(f)fclose(f);
    }

    strcpy(p_eur_io_buff[0]->OutputStatus,"NNNN");

    if(get_option_by_section_name(ctx, "ioports", "output", "output1", tmp_buf))
    {
        p_eur_io_buff[0]->OutputStatus[0]=tmp_buf[0];
    }
    else
    {
        if ((f = fopen("/sys/class/leds/OUTPUT1/brightness", "r"))) 
        {
            if(fgets(tmp_buf, 10, f)) 
            {
                p_eur_io_buff[0]->OutputStatus[0]=tmp_buf[0];
            }
        }
        if(f)fclose(f);
    }

    if(get_option_by_section_name(ctx, "ioports", "output", "output2", tmp_buf))
    {
        p_eur_io_buff[0]->OutputStatus[1]=tmp_buf[0];
    }
    else
    {
        if ((f = fopen("/sys/class/leds/OUTPUT2/brightness", "r"))) 
        {
            if(fgets(tmp_buf, 10, f)) 
            {
                p_eur_io_buff[0]->OutputStatus[1]=tmp_buf[0];
            }
        }
        if(f)fclose(f);
    }

    if(get_option_by_section_name(ctx, "ioports", "output", "output3", tmp_buf))
    {
        p_eur_io_buff[0]->OutputStatus[2]=tmp_buf[0];
    }
    else
    {
        if ((f = fopen("/sys/class/leds/OUTPUT3/brightness", "r"))) 
        {
            if(fgets(tmp_buf, 10, f)) 
            {
                p_eur_io_buff[0]->OutputStatus[2]=tmp_buf[0];
            }
        }
        if(f)fclose(f);
    }

    if(get_option_by_section_name(ctx, "ioports", "output", "output4", tmp_buf))
    {
        p_eur_io_buff[0]->OutputStatus[3]=tmp_buf[0];
    }
    else
    {
        if ((f = fopen("/sys/class/leds/OUTPUT4/brightness", "r"))) 
        {
            if(fgets(tmp_buf, 10, f)) 
            {
                p_eur_io_buff[0]->OutputStatus[3]=tmp_buf[0];
            }
        }
        if(f)fclose(f);
    }
  

}


static void refresh_json_rads_buff()
{
    FILE* f;
    char *p;
    int j;
    char crssi[40];
    char tmp[10];
    int rssi,seq_no;
    char buffer_get[300];
    if(p_eur_radios_buff[0]==NULL)return;

    strcpy(p_eur_radios_buff[0]->authenticationKey,"N/A");
    strcpy(p_eur_radios_buff[0]->childRssi,"N/A");
    strcpy(p_eur_radios_buff[0]->coreTemperature,"N/A");
    strcpy(p_eur_radios_buff[0]->supplyVoltage,"N/A");
    strcpy(p_eur_radios_buff[0]->vswr,"N/A");
    strcpy(p_eur_radios_buff[0]->txRate,"N/A");

    strcpy(p_eur_radios_buff[0]->linkRate,"N/A");
    get_option_by_section_name(ctx, "wireless", "@wifi-iface[0]", "rate", p_eur_radios_buff[0]->linkRate);
    strcpy(p_eur_radios_buff[0]->encryptionType,"N/A");
    get_option_by_section_name(ctx, "wireless", "@wifi-iface[0]", "encryption", p_eur_radios_buff[0]->encryptionType);
    strcpy(p_eur_radios_buff[0]->operationMode,"N/A");
    get_option_by_section_name(ctx, "wireless", "@wifi-iface[0]", "mode", p_eur_radios_buff[0]->operationMode);
    strcpy(p_eur_radios_buff[0]->searchMode,"N/A");
    get_option_by_section_name(ctx, "wireless", "radio0", "type", p_eur_radios_buff[0]->searchMode);
    strcpy(p_eur_radios_buff[0]->networkName,"N/A");
    get_option_by_section_name(ctx, "wireless", "@wifi-iface[0]", "ssid", p_eur_radios_buff[0]->networkName);
    strcpy(p_eur_radios_buff[0]->channelNumber,"N/A");
    get_option_by_section_name(ctx, "wireless", "radio0", "channel", p_eur_radios_buff[0]->channelNumber);
    strcpy(p_eur_radios_buff[0]->channelBandwidth,"N/A");
    get_option_by_section_name(ctx, "wireless", "radio0", "bwmode", p_eur_radios_buff[0]->channelBandwidth);


    strcpy(p_eur_radios_buff[0]->rssi,"[\"N/A\"]"); //"rssi":["-45","-98","-87"],		
    j=AERC_REPORT_NUM;
    sprintf(crssi,"eurd_iw 0 %d",j);
    if (SubProgram_Start(eurdscriptFile,crssi))
    {
        ;
    }else {
        syslog(LOGOPTS,"GPSR report%d send email error\n", index);
        return;
        }

    sprintf(crssi,"%s%d",iw_statefile,j);
    if (!(f = fopen(crssi, "r"))) {
        //do nothing.   resolv.conf.auto
    }else
    {
        sprintf(crssi,"[");
        seq_no=0;
        while (fgets(buffer_get, 300, f)) 
        {
            if(strlen(buffer_get)<5)continue;

            p=NULL;
            p = strstr(buffer_get, "rssi:");
            if (p != NULL)
            {
                p+=strlen("rssi:");
                while(*p==' ')p++;
                j=0;
                while ((*p != ' ')&& (*p!='\r') && (*p!='\0') && (*p!='t') &&(j<5)) 
                {
                    tmp[j]=*p;
                    j++;
                    p++;
                 }
                if(j>0)
                {
                    tmp[j]=0;
                    rssi=atoi(tmp)-95;
                    if(seq_no==0)
                    {
                        sprintf(tmp,"\"%d\"",rssi);
                        strcat(crssi,tmp);
                    }else
                    {
                        sprintf(tmp,",\"%d\"",rssi);
                        strcat(crssi,tmp);
                    }
                    seq_no=1;
                }
            }
        }
        if(seq_no==1)
        {
            strcat(crssi,"]");
            strcpy(p_eur_radios_buff[0]->rssi,crssi); 
        }
    }
    if(f)fclose(f);

}
static void refresh_json_carrs_buff()
{
    char proname[32];
    int i,j,col;
    FILE* f;
    char buffer_tmp[20];
    char buffer_get[256];
    char *p;
    char *p1;
    int trans_sign=0;
    char buff[80];
    int pos = 0;
    int connet_sign;
    char lac[20];
    char mccmnc[10];
    char mcc[10];
    char mnc[10];
    char cid[20];
    char cell_loc_network[10];
    if(p_eur_carrs_buff[0]==NULL)return;

        lac[0]=0;
        mccmnc[0]=0;
        p=NULL;
        strcpy(p_eur_carrs_buff[0]->dns1,"N/A");
        strcpy(p_eur_carrs_buff[0]->dns2,"N/A");
        strcpy(p_eur_carrs_buff[0]->ecNo,"N/A");
        strcpy(p_eur_carrs_buff[0]->rscp,"N/A");
        strcpy(p_eur_carrs_buff[0]->frequencyBand,"N/A");
        strcpy(p_eur_carrs_buff[0]->channelNumber,"N/A");
        strcpy(p_eur_carrs_buff[0]->supplyVoltage,"N/A");
        strcpy(p_eur_carrs_buff[0]->imsi,"N/A");
        strcpy(p_eur_carrs_buff[0]->latitude,"N/A");
        strcpy(p_eur_carrs_buff[0]->longitude,"N/A");
        strcpy(p_eur_carrs_buff[0]->radius,"N/A");

        //done
        strcpy(p_eur_carrs_buff[0]->simCardNumber,"N/A");
        strcpy(p_eur_carrs_buff[0]->phoneNumber,"N/A");
        strcpy(p_eur_carrs_buff[0]->rssi,"N/A");
        strcpy(p_eur_carrs_buff[0]->RSRP,"N/A");
        strcpy(p_eur_carrs_buff[0]->RSRQ,"N/A");
        strcpy(p_eur_carrs_buff[0]->threeGNetwork,"NOT REGISTERED");
        strcpy(p_eur_carrs_buff[0]->cellId,"N/A");
        strcpy(p_eur_carrs_buff[0]->mcc,"N/A");
        strcpy(p_eur_carrs_buff[0]->mnc,"N/A");
        strcpy(p_eur_carrs_buff[0]->lac,"N/A");
        strcpy(p_eur_carrs_buff[0]->imei,"N/A");
        strcpy(p_eur_carrs_buff[0]->activityStatus,"N/A");
        strcpy(p_eur_carrs_buff[0]->homeRoaming,"N/A");
        strcpy(p_eur_carrs_buff[0]->serviceType,"No Service");
        strcpy(p_eur_carrs_buff[0]->coreTemperature,"N/A");

        if (!(f = fopen("/tmp/resolv.conf.auto", "r"))) {
            //do nothing.   resolv.conf.auto
        }else
        {
            while (fgets(buffer_get, 256, f)) 
            {
                p=NULL;
                p = strstr(buffer_get, "nameserver");
                if (p != NULL)
                {
                    if(trans_sign==1)
                    {

                        while (*p == ' ')p++; //first space
                        while (*p != ' ')p++; //nameserver
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != ' ')&& (*p!='\r') && (*p!='\0') && (*p!='\n')&&(j<20)) 
                        {
                            buffer_tmp[j] = *p;
                            p++;
                            j++;
                         }                   
                         buffer_tmp[j]=0;
                         if(j>5) 
                         {
                             strcpy(p_eur_carrs_buff[0]->dns2,buffer_tmp);
                             trans_sign++;
                         }
                    }//if 1
    
                   if(trans_sign==0) 
                    {
                       while (*p == ' ')p++; //first space
                       while (*p != ' ')p++; //nameserver
                       while (*p == ' ')p++; //first space
                       j=0;
                       while ((*p != ' ')&& (*p!='\r') && (*p!='\0') &&(j<20)) 
                       {
                           buffer_tmp[j] = *p;
                           p++;
                           j++;
                        }                   
                        buffer_tmp[j]=0;
                        if(j>5) 
                        {
                            strcpy(p_eur_carrs_buff[0]->dns1,buffer_tmp);
                            trans_sign++;
                        }
                    }//if 0
                    if (trans_sign>1) 
                    {
                        break; 
                    }
                }//if p
            }//while
        }//else
        if(f)fclose(f);


     //SIM
        p=NULL;
        int sim_ok=0;
        if (!(f = fopen("/sys/class/button/SIM_STAT/status", "r"))) 
        {/* the file must be there */
            if(f)fclose(f);
        }else
        {
            if (fgets(buffer_get, 10, f))
            {
                sim_ok=0;
                if(buffer_get[0]=='0')sim_ok=1;
            }
            if(f)fclose(f);
        }


//        if(sim_ok==1)   // if sim card not valid, also get such data.
            //        {
            strcpy(buff,"");
            if (!(f = fopen("/var/run/std_lte_statue", "r"))) 
            {
                //do nothing.
            }else
            {
                while (fgets(buffer_get, 256, f)) 
                {
                    p=NULL;
                    p = strstr(buffer_get, "rssi=");//rssi="-72"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->rssi,"%s",p);
                        }
                    }
                    
                    p=NULL;
                    p = strstr(buffer_get, "rsrp=");//rsrp="-20"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->RSRP,"%s",p);
                        }
                    }

                    p=NULL;
                    p = strstr(buffer_get, "rsrq=");//rsrq="-69"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->RSRQ,"%s",p);
                        }
                    }

                    p=NULL;
                    p = strstr(buffer_get, "imei=");//imei="012773002002648"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->imei,"%s",p);
                        }
                    }

                    p=NULL;
                    p = strstr(buffer_get, "imsi=");//imsi="302720500395176"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->imsi,"%s",p);
                        }
                    }

                    p=NULL;
                    p = strstr(buffer_get, "phonenum=");//phonenum="+14036050307"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->phoneNumber,"%s",p);
                        }
                    }


                    p=NULL;
                    p = strstr(buffer_get, "simid=");//simid="89302720403007563710"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->simCardNumber,"%s",p);
                        }
                    }

                    p=NULL;
                    p = strstr(buffer_get, "cellid=");//cellid="29441"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->cellId,"%s",p);
                        }
                    }

                    p=NULL;
                    p = strstr(buffer_get, "lac=");//lac="65534"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->lac,"%s",p);
                        }
                    }
                    
                    p=NULL;
                    p = strstr(buffer_get, "mccmnc=");//mccmnc=" 0,2, 302220 ,2"
                    if (p != NULL)
                    {
                        while((*p != ',')&& (*p!='\n'))p++;
                        if(*p == ',') 
                        {
                            p++;
                            while((*p != ',') && (*p!='\n'))p++;
                            if(*p == ',') 
                            {
                                p++;
                                while(*p == ' ')p++;
                                j=0;
                                while ((*p != ',')&&(*p != '\"')&& (*p!=' ') && (*p!='\n') &&(j<20)) 
                                    {
                                        mccmnc[j]=*p;
                                        p++;
                                        j++;
                                     }
                                 if(j>3)
                                 {
                                     mccmnc[j]=0;
                                     mcc[0]=mccmnc[0];
                                     mcc[1]=mccmnc[1];
                                     mcc[2]=mccmnc[2];
                                     mcc[3]=0;
                                     mnc[0]=mccmnc[3];
                                     if(j>4)mnc[1]=mccmnc[4];
                                     else  mnc[1]=0;
                                     if(j>5)mnc[2]=mccmnc[5]; 
                                     else  mnc[2]=0;
                                     mnc[3]=0;
                                     unsigned int MCC=atoi(mcc);
                                     unsigned int MNC=atoi(mnc);
                                     sprintf(p_eur_carrs_buff[0]->mcc,"%d",MCC);
                                     sprintf(p_eur_carrs_buff[0]->mnc,"%d",MNC);
                                 } else mccmnc[0]=0;

                            }
                        }

                    }//if p

                    p=NULL;
                    connet_sign=0;
                    p = strstr(buffer_get, "connect_status=");//connect_status="Connected"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->activityStatus,"%s",p);
                        }
                    }

                    p=NULL;
                    p = strstr(buffer_get, "roaming=");//roaming="HOME"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->homeRoaming,"%s",p);
                        }
                    }
                    
                    p=NULL;
                    p = strstr(buffer_get, "state=");//state="LTE PS (Packet Switched)"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->serviceType,"%s",p);
                        }
                    }
                  
                    p=NULL;
                    p = strstr(buffer_get, "network=");//network="Verizon"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->threeGNetwork,"%s",p);
                        }
                    }
                   
                    p=NULL;
                    p = strstr(buffer_get, "temp=");//temp=" 39 degC"
                    if (p != NULL)
                    {
                        p = strchr(buffer_get, '=');
                        if ( p!= NULL)
                        {
                            p++;
                            sprintf(p_eur_carrs_buff[0]->coreTemperature,"%s",p);
                        }
                    }
      
#if 0 /* remove here*/

                    p = strstr(buffer_get, "rssi=");//rssi=" GSM RSSI : 72"
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
                                p_eur_carrs_buff[0]->rssi[j]=*p;
                                p++;
                             }
                            if(j>0)
                            {
                                j++;
                                p_eur_carrs_buff[0]->rssi[j]=0;
                                p_eur_carrs_buff[0]->rssi[0]='-';
                            }
                        }//if :
                    }//if p



                    p=NULL;
                    p = strstr(buffer_get, "rsrp=");//rsrp=" RSRP : 0"
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
                                p_eur_carrs_buff[0]->RSRP[j]=*p;
                                j++;
                                p++;
                             }
                            if(j>0)p_eur_carrs_buff[0]->RSRP[j]=0;
                        }//if :
                    }//if p

                    p=NULL;
                    p = strstr(buffer_get, "rsrq=");//rsrq=" RSRQ : 0"  rsrp=" RSRP : 0"   rsrq=" RSRQ : 0"

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
                                p_eur_carrs_buff[0]->RSRQ[j]=*p;
                                j++;
                                p++;
                             }
                            if(j>0)p_eur_carrs_buff[0]->RSRQ[j]=0;
                        }//if :
                    }//if p


                    p=NULL;
                    p = strstr(buffer_get, "imei=");//imei=" 012773002002648"
                    if (p != NULL)
                    {
                        p+=strlen("imei=")+1;
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                            {
                                p_eur_carrs_buff[0]->imei[j]=*p;
                                p++;
                                j++;
                             }
                        if(j>0)p_eur_carrs_buff[0]->imei[j]=0;
                    }//if p


                    p=NULL;
                    p = strstr(buffer_get, "imsi=");//imsi="302720500395176"
                    if (p != NULL)
                    {
                        p+=strlen("imsi=")+1;
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                            {
                                p_eur_carrs_buff[0]->imsi[j]=*p;
                                p++;
                                j++;
                             }
                        if(j>0)p_eur_carrs_buff[0]->imsi[j]=0;
                    }//if p



                    p=NULL;
                    p = strstr(buffer_get, "phonenum=");//phonenum="  TELEPHONE , +14036050307 ,145"
                    if (p != NULL)
                    {
                        while ((*p != ',') && (*p != '\n') )p++; //key space
                        if (*p==',')
                        {
                            p++;
                            while(*p == ' ')p++;
                            j=0;
                            while ((*p != ' ')&& (*p!=',') && (*p!='\0') && (*p!='\n') &&(j<20)) 
                            {
                                p_eur_carrs_buff[0]->phoneNumber[j]=*p;
                                p++;
                                j++;
                             }
                            if(j>0)p_eur_carrs_buff[0]->phoneNumber[j]=0;
                        }//if :
                    }//if p


                    p=NULL;
                    p = strstr(buffer_get, "simid=");//simid=" 89302720403007563710"
                    if (p != NULL)
                    {
                        p+=strlen("simid=")+1;
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                            {
                                p_eur_carrs_buff[0]->simCardNumber[j]=*p;
                                p++;
                                j++;
                             }
                        if(j>0)p_eur_carrs_buff[0]->simCardNumber[j]=0;
                    }//if p

                    p=NULL;
                    p = strstr(buffer_get, "cell_id=");//cell_id=" 2,1, 2BC4, 4BA5A63, 2"
                    if (p != NULL)
                    {
                        while (*p != ','  && (*p!='\n') )p++; //first space
                        if(*p == ',') 
                        {
                            p++;
                            while (*p != ','  && (*p!='\"') )p++; //first space
                            if(*p!='\"')p++;
                            while (*p == ' ')p++; //first space
                            j=0;
                            while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') && (*p!=',')  &&(j<20)) //lac
                                {
                                    lac[j]=*p;
                                    p++;
                                    j++;
                                 }
                            if(j>0)
                            {
                                lac[j]=0;
                                unsigned int lac0;
                                sscanf(lac,"%x",&lac0);
                                sprintf(p_eur_carrs_buff[0]->lac,"%d",lac0);
                            }
                            while (*p != ','  && (*p!='\"') )p++; //first space
                            if(*p!='\"')p++;
                            while (*p == ' ')p++; //first space
                            j=0;
                            while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') && (*p!=',')  &&(j<20)) //cellid
                                {
                                    cid[j]=*p;
                                    p++;
                                    j++;
                                 }
                            if(j>0)
                            {
                                cid[j]=0;
                                unsigned int cid0;
                                sscanf(cid,"%x",&cid0);
                                sprintf(p_eur_carrs_buff[0]->cellId,"%d",cid0);

                            }

                        }

                    }//if p

                    p = strstr(buffer_get, "mccmnc=");//mccmnc=" 0,2, 302220 ,2"
                    if (p != NULL)
                    {
                        while((*p != ',')&& (*p!='\n'))p++;
                        if(*p == ',') 
                        {
                            p++;
                            while((*p != ',') && (*p!='\n'))p++;
                            if(*p == ',') 
                            {
                                p++;
                                while(*p == ' ')p++;
                                j=0;
                                while ((*p != ',')&&(*p != '\"')&& (*p!=' ') && (*p!='\n') &&(j<20)) 
                                    {
                                        mccmnc[j]=*p;
                                        p++;
                                        j++;
                                     }
                                 if(j>3)
                                 {
                                     mccmnc[j]=0;
                                     mcc[0]=mccmnc[0];
                                     mcc[1]=mccmnc[1];
                                     mcc[2]=mccmnc[2];
                                     mcc[3]=0;
                                     mnc[0]=mccmnc[3];
                                     if(j>4)mnc[1]=mccmnc[4];
                                     else  mnc[1]=0;
                                     if(j>5)mnc[2]=mccmnc[5]; 
                                     else  mnc[2]=0;
                                     mnc[3]=0;
                                     unsigned int MCC=atoi(mcc);
                                     unsigned int MNC=atoi(mnc);
                                     sprintf(p_eur_carrs_buff[0]->mcc,"%d",MCC);
                                     sprintf(p_eur_carrs_buff[0]->mnc,"%d",MNC);
                                 } else mccmnc[0]=0;

                            }
                        }

                    }//if p


                    p=NULL;
                    connet_sign=0;
                    p = strstr(buffer_get, "connect_status=");//connect_status=" QMI_WDS_PKT_DATA_DISCONNECTED"
                    if (p != NULL)
                    {
                        p1=NULL;
                        p1 = strstr(buffer_get, "_CONNECTED");
                        if (p1 != NULL)
                        {
                            connet_sign=1;
                            strcpy( p_eur_carrs_buff[0]->activityStatus,"Connected");
                        }
                        else
                            strcpy( p_eur_carrs_buff[0]->activityStatus,"Disconnected");                    
                    }//if p

                    p=NULL;
                    p = strstr(buffer_get, "roaming=");//roaming=" 0,0"
                    if (p != NULL)
                    {
                        while((*p != ',') && (*p!='\r') && (*p!='\0') && (*p!='\n'))p++;
                        if(*p == ',') 
                        {
                            p++;
                            while(*p == ' ')p++;
                            if(*p=='0')strcpy(p_eur_carrs_buff[0]->homeRoaming,"Not Searching");
                            if(*p=='1')strcpy(p_eur_carrs_buff[0]->homeRoaming,"Home");
                            if(*p=='2')strcpy(p_eur_carrs_buff[0]->homeRoaming,"Searching");
                            if(*p=='3')strcpy(p_eur_carrs_buff[0]->homeRoaming,"Denied");
                            if(*p=='4')strcpy(p_eur_carrs_buff[0]->homeRoaming,"Unknown");
                            if(*p=='5')strcpy(p_eur_carrs_buff[0]->homeRoaming,"Roaming");
                        }
                    }//if p


/*
                    p=NULL;
                    p = strstr(buffer_get, "rat=");//rat=" 0,2,0"
                    if (p != NULL)
                    {
                        while((*p != ',')&& (*p!='\r') && (*p!='\0') && (*p!='\n'))p++;
                        if(*p == ',') 
                        {
                            p--;
                            strcpy(p_eur_carrs_buff[0]->serviceType,"Unknown");
                            if(*p=='0')strcpy(p_eur_carrs_buff[0]->serviceType,"Automatic");
                            if(*p=='1')strcpy(p_eur_carrs_buff[0]->serviceType,"GSM Only");
                            if(*p=='2')strcpy(p_eur_carrs_buff[0]->serviceType,"WCDMA Only");
                        }
                    }//if p
*/
                    p=NULL;
                    p = strstr(buffer_get, "rat=");//rat=" 0,2,0"
                    if (p != NULL)
                    {
                        strcpy(p_eur_carrs_buff[0]->serviceType,"Unknown");
                        p=NULL;
                        p=strrchr(buffer_get,',');
                        if(p!=NULL) 
                        {
                            if(*p == ',') 
                            {
                                p++;
                                j=0;
                                while((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n'))
                                {
                                    buffer_tmp[j]=*p;
                                    p++;
                                    j++;
                                }
                                buffer_tmp[j]=0;
                                j=atoi(buffer_tmp);
                                switch (j) 
                                {
                                case 0:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"Searching for Service");
                                    break;
                                case 1:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"WCDMA CS (Circuit Switched)");
                                    break;
                                case 2:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"WCDMA PS (Packet Switched)");
                                    break;
                                case 3:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"WCDMA CS and PS");
                                    break;
                                case 4:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"GSM CS (Circuit Switched)");
                                    break;
                                case 5:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"GSM PS (Packet Switched)");
                                    break;
                                case 6:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"GSM CS and PS");
                                    break;
                                case 7:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"LTE CS (Circuit Switched)");
                                    break;
                                case 8:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"LTE PS (Packet Switched)");
                                    break;
                                case 9:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"LTE CS and PS");
                                    break;
                                case 10:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"CDMA CS (Circuit Switched)");
                                    break;
                                case 11:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"CDMA PS (Packet Switched)");
                                    break;
                                case 12:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"CDMA CS and PS");
                                    break;
                                case 13:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"HDR CS (Circuit Switched)");
                                    break;
                                case 14:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"HDR PS (Packet Switched)");
                                    break;
                                case 15:
                                    strcpy(p_eur_carrs_buff[0]->serviceType,"HDR CS and PS");
                                    break;
                                default:
                                    break;
                                }
                            }
                        }
                    }//if p


                    p=NULL;
//                    strcpy(p_eur_carrs_buff[0]->threeGNetwork,"Unknown");
                    p = strstr(buffer_get, "network=");//network=" 0,0, Bell ,2"
                    if (p != NULL)
                    {
                        while((*p != ',')&& (*p!='\n'))p++;
                        if(*p == ',') 
                        {
                            p++;
                            while((*p != ',') && (*p!='\n'))p++;
                            if(*p == ',') 
                            {
                                p++;
                                while(*p == ' ')p++;
                                j=0;
                                while ((*p != ',')&&(*p != '\"')&& (*p!=' ') && (*p!='\n') &&(j<20)) 
                                    {
                                        p_eur_carrs_buff[0]->threeGNetwork[j]=*p;
                                        p++;
                                        j++;
                                     }
                                 if(j>0)p_eur_carrs_buff[0]->threeGNetwork[j]=0;
                            }
                        }

                    }//if p

                    p=NULL;
                    p = strstr(buffer_get, "temp=");//temp=" 39 degC"
                    if (p != NULL)
                    {
                        p+=strlen("temp=")+1;
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != '\"')&& (*p!=' ') && (*p!='\0') && (*p!='\n') &&(j<20)) 
                            {
                                p_eur_carrs_buff[0]->coreTemperature[j]=*p;
                                p++;
                                j++;
                             }
                        if(j>0)p_eur_carrs_buff[0]->coreTemperature[j]=0;
                    }//if p
#endif
                }//while
            }//else
            if(f)fclose(f);


            if(strlen(eur_gps_buff.latitude)>4)
            {
                strcpy(p_eur_carrs_buff[0]->latitude,eur_gps_buff.latitude);
                strcpy(p_eur_carrs_buff[0]->longitude,eur_gps_buff.longitude);
                strcpy(p_eur_carrs_buff[0]->radius,"5");
                return;
            }

            if(p_eur_carrs_buff[0]->lac[0]=='N' || p_eur_carrs_buff[0]->mnc[0]=='N')return;

            cell_loc_network[0]=0;
            get_option_by_section_name(ctx, "eurd", "eur_conf", "Cell_Loc_Network", cell_loc_network);
            if(cell_loc_network[0]=='B')
            {
                if (SubProgram_Start("/bin/gps_webget","B")==false)return;
            }else if(cell_loc_network[0]=='A')
            {
                if (SubProgram_Start("/bin/gps_webget","A")==false)return;
            }else return;

            char gpssource[10];
            gpssource[0]=0;
            if (!(f = fopen("/var/run/GPS_position_carr", "r"))) 
            {
                //do nothing.
            }else
            {
                while (fgets(buffer_get, 256, f)) 
                {
                    p=NULL;
                    p = strstr(buffer_get, "latitude=");//latitude="51.142962"
                    if (p != NULL)
                    {
                        p+=strlen("latitude=")+1;
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                            {
                                p_eur_carrs_buff[0]->latitude[j]=*p;
                                p++;
                                j++;
                             }
                        if(j>0) 
                            {
                                p_eur_carrs_buff[0]->latitude[j]=0;
                            }
                    }//if p

                    p=NULL;
                    p = strstr(buffer_get, "longitude=");//longitude="-114.075094"
                    if (p != NULL)
                    {
                        p+=strlen("longitude=")+1;
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                            {
                                p_eur_carrs_buff[0]->longitude[j]=*p;
                                p++;
                                j++;
                             }
                        if(j>0) 
                            {
                                p_eur_carrs_buff[0]->longitude[j]=0;
                            }
                    }//if p

                    p=NULL;
                    p = strstr(buffer_get, "source=");//
                    if (p != NULL)
                    {
                        p+=strlen("source=")+1;
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<10)) 
                            {
                                gpssource[j]=*p;
                                p++;
                                j++;
                             }
                        if(j>0) 
                            {
                                gpssource[j]=0;
                            }
                    }//if p

                    p=NULL;
                    p = strstr(buffer_get, "radius=");//longitude="-114.075094"
                    if (p != NULL)
                    {
                        p+=strlen("radius=")+1;
                        while (*p == ' ')p++; //first space
                        j=0;
                        while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<10)) 
                            {
                                p_eur_carrs_buff[0]->radius[j]=*p;
                                p++;
                                j++;
                             }
                        if(j>0) 
                            {
                                p_eur_carrs_buff[0]->radius[j]=0;
                            }
                    }//if p

                }//while
            }//else
            if(f)fclose(f);
            if(gpssource[0]=='G' && gpssource[1]=='P') {
                strcpy(p_eur_carrs_buff[0]->radius,"2000");
            }



/*
            //geo data comes from cell id and lac
            s_carrs_geodata='0';
                char mcc[4];
                unsigned int cid0,lac0;
                mcc[0]=mccmnc[0];
                mcc[1]=mccmnc[1];
                mcc[2]=mccmnc[2];
                mcc[3]=0;
                sscanf(p_eur_carrs_buff[0]->cellId,"%x",&cid0);
                sscanf(lac,"%x",&lac0);
//                            cid0=atoi(p_eur_carrs_buff[0]->cellId);
//                            lac0=atoi(lac);
                unsigned int MCC=atoi(mcc);
                unsigned int MNC=atoi(mccmnc+3);
                if(sendLocationRequest(MNC,MCC,cid0,lac0)==true)
                {
                    s_carrs_geodata='1';
                }
*/

//        }//sim ok

}


static void refresh_json_gps_buff()
{
    FILE* f;
    char *p;
    int j;
    char buffer_get[256];
    char check_status[5],check_location[5];
    check_status[0]=0;
    check_location[0]=0;

    eur_gps_buff.gps_flag=0;
    strcpy(eur_gps_buff.latitude,"N/A");
    strcpy(eur_gps_buff.longitude,"N/A");

    get_option_by_section_name(ctx, "gpsd", "gpsd_conf", "status", check_status);
    get_option_by_section_name(ctx, "gpsd", "gpsd_conf", "mylocation", check_location);
    if(check_status[0]=='1' && check_location[0]=='1')
    {
        eur_gps_buff.gps_flag=1;
    }else
    {
        return;
    }

    if (!(f = fopen("/var/run/GPS_position", "r"))) 
    {
        //do nothing.
    }else
    {
        while (fgets(buffer_get, 256, f)) 
        {
            p=NULL;
            p = strstr(buffer_get, "latitude=");//latitude="51.142962"
            if (p != NULL)
            {
                p+=strlen("latitude=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        eur_gps_buff.latitude[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0) 
                    {
                        eur_gps_buff.latitude[j]=0;
                    }
            }//if p

            p=NULL;
            p = strstr(buffer_get, "longitude=");//longitude="-114.075094"
            if (p != NULL)
            {
                p+=strlen("longitude=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        eur_gps_buff.longitude[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0) 
                    {
                        eur_gps_buff.longitude[j]=0;
                    }
            }//if p

        }//while
    }//else
    if(f)fclose(f);

}

static void refresh_json_vpncls_buff()
{
    int i,j,vpn_i,col;
    FILE* f;
    char *p;
    char buffer_tmp[40];
    char buffer_get[256];
    int set_statistic;
    int trans_sign;



    //update vpn string for device_buff.
    vpn_i=1;
    eur_device_buff.vpnClientList[0]='\"';
    if (!(f = fopen("/etc/config/ipsec", "r"))) 
    {/* the file must be there */
     //do nothing. then all p_vpn_buff[] is NULL.
    }else
    {
        while (fgets(buffer_get, 256, f)) 
        {
            p=NULL;
            p = strstr(buffer_get, "s2stunnel");
            if(p != NULL)
            {
                //config 's2stunnel' 'g2g1'
                while(*p != ' ') p++;
                while((*p == ' ') || (*p == '\'')) p++;
                j=0;
                while ((*p != ' ')&&(*p != '\'')&&(j<19)&&(vpn_i<158)) 
                {
                    eur_device_buff.vpnClientList[vpn_i]=*p;
                    p++;
                    vpn_i++;
                    j++;
                }
                eur_device_buff.vpnClientList[vpn_i]='\"';
                vpn_i++;
                eur_device_buff.vpnClientList[vpn_i]=',';
                vpn_i++;
                eur_device_buff.vpnClientList[vpn_i]='\"';
                vpn_i++;
            }
            p=NULL;
            p = strstr(buffer_get, "x2ctunnel");
            if(p != NULL)
            {
                //config 'x2ctunnel' 'c2g1'
                while(*p != ' ') p++;
                while((*p == ' ') || (*p == '\'')) p++;
                j=0;
                while ((*p != ' ')&&(*p != '\'')&&(j<19)&&(vpn_i<158)) 
                {
                    eur_device_buff.vpnClientList[vpn_i]=*p;
                    p++;
                    vpn_i++;
                    j++;
                }
                eur_device_buff.vpnClientList[vpn_i]='\"';
                vpn_i++;
                eur_device_buff.vpnClientList[vpn_i]=',';
                vpn_i++;
                eur_device_buff.vpnClientList[vpn_i]='\"';
                vpn_i++;

            }

        }//while
        eur_device_buff.vpnClientList[vpn_i-1]=0;
        if(vpn_i>2)eur_device_buff.vpnClientList[vpn_i-2]=0;
    }
    if(f)fclose(f);



    //2.fresh uci ipsec to check is OK by config vpn list.--------vpn_fresh
    for (i=0;((i<MAX_VPNCLS_NUMBER) && (p_eur_vpncls_buff[i]!=NULL));i++) // all p_ had been set by 'B'
    {

        set_statistic=0;
        strcpy(p_eur_vpncls_buff[i]->protocol,"L2TP");
        strcpy(p_eur_vpncls_buff[i]->receivedBytes,"0");
        strcpy(p_eur_vpncls_buff[i]->receivedPackets,"0");
        strcpy(p_eur_vpncls_buff[i]->transmittedBytes,"0");
        strcpy(p_eur_vpncls_buff[i]->transmittedPackets,"0");

        strcpy(p_eur_vpncls_buff[i]->connected,"N/A");//true  constat=N/A  /var/run/mipsec/$x2c
        strcpy(p_eur_vpncls_buff[i]->ipAddr,"N/A");  //'localip=' /var/run/mipsec/$x2c    s:leftsubnet
        strcpy(p_eur_vpncls_buff[i]->gateway,"N/A");  //right-uci
        strcpy(p_eur_vpncls_buff[i]->inf,"N/A");//status=interface down
//        buffer_tmp;
        get_option_by_section_name(ctx, "ipsec", p_eur_vpncls_buff[i]->name, "right", p_eur_vpncls_buff[i]->gateway);
        if(p_eur_vpncls_buff[i]->vpn_key[0]=='s') 
        {
            get_option_by_section_name(ctx, "ipsec", p_eur_vpncls_buff[i]->name, "leftsubnet", p_eur_vpncls_buff[i]->ipAddr);
            sprintf(buffer_tmp,"/var/run/mipsec/%s.status",p_eur_vpncls_buff[i]->name);
            if (!(f = fopen(buffer_tmp, "r"))) 
            {/* the file must be there */
             //do nothing. then all p_vpn_buff[] is NULL.
            }else
            {
                while (fgets(buffer_get, 256, f)) 
                {
                    p=NULL;
                    p = strstr(buffer_get, "constat="); 
                    if(p!=NULL) 
                    {
                        while( *p != '=')p++;
                        p++;
                        j=0;
                        while ((*p != ' ')&&(*p != '\'') && (*p!='\r') && (*p!='\0') && (*p!='\n')&&(j<9)) 
                        {
                            p_eur_vpncls_buff[i]->connected[j]=*p;
                            p++;
                            j++;
                        }
                        p_eur_vpncls_buff[i]->connected[j]=0;

                    }

                    p=NULL;
                    p = strstr(buffer_get, "status="); 
                    if(p!=NULL) 
                    {
                        while( *p != '=')p++;
                        p++;
                        j=0;
                        while ((*p != '\'')&& (*p!='\r') && (*p!='\0') && (*p!='\n')&&(j<19)) 
                        {
                            p_eur_vpncls_buff[i]->inf[j]=*p;
                            p++;
                            j++;
                        }
                        p_eur_vpncls_buff[i]->inf[j]=0;

                    }//if p

                }//while
                
            }//else
            if(f)fclose(f);
        }//s

        if(p_eur_vpncls_buff[i]->vpn_key[0]=='c') 
        {
            sprintf(buffer_tmp,"/var/run/mipsec/%s",p_eur_vpncls_buff[i]->name);
            if (!(f = fopen(buffer_tmp, "r"))) 
            {/* the file must be there */
             //do nothing. then all p_vpn_buff[] is NULL.
            }else
            {
                while (fgets(buffer_get, 256, f)) 
                {
                    p=NULL;
                    p = strstr(buffer_get, "constat="); 
                    if(p!=NULL) 
                    {
                        while( *p != '=')p++;
                        p++;
                        j=0;
                        while ((*p != ' ')&&(*p != '\'') && (*p!='\r') && (*p!='\0') && (*p!='\n')&&(j<19)) 
                        {
                            p_eur_vpncls_buff[i]->connected[j]=*p;
                            p++;
                            j++;
                        }
                        p_eur_vpncls_buff[i]->connected[j]=0;

                    }

                    p=NULL;
                    p = strstr(buffer_get, "status="); 
                    if(p!=NULL) 
                    {
                        while( *p != '=')p++;
                        p++;
                        j=0;
                        while ((*p != '\'')&& (*p!='\r') && (*p!='\0') && (*p!='\n')&&(j<19)) 
                        {
                            p_eur_vpncls_buff[i]->inf[j]=*p;
                            p++;
                            j++;
                        }
                        p_eur_vpncls_buff[i]->inf[j]=0;

                    }

                    p=NULL;
                    p = strstr(buffer_get, "inf="); 
                    if(p!=NULL) 
                    {
                        while( *p != '=')p++;
                        p++;
                        j=0;
                        while ((*p != '\'')&& (*p!='\r') && (*p!='\0') && (*p!='\n')&&(j<19)) 
                        {
                            p_eur_vpncls_buff[i]->inf[j]=*p;
                            p++;
                            j++;
                        }
                        p_eur_vpncls_buff[i]->inf[j]=0;
                        if(j>0)set_statistic=1;
                    }

                    p=NULL;
                    p = strstr(buffer_get, "localip="); 
                    if(p!=NULL) 
                    {
                        while( *p != '=')p++;
                        p++;
                        j=0;
                        while ((*p != ' ')&&(*p != '\'')&& (*p!='\r') && (*p!='\0') && (*p!='\n')&&(j<19)) 
                        {
                            p_eur_vpncls_buff[i]->ipAddr[j]=*p;
                            p++;
                            j++;
                        }
                        p_eur_vpncls_buff[i]->ipAddr[j]=0;

                    }//if p

                }//while
                
            }//else
            if(f)fclose(f);

            if(set_statistic==1)
            {
                trans_sign=1;
                p=NULL;
                if (!(f = fopen("/proc/net/dev", "r"))) {
                    trans_sign=0;
                }else
                {
                    while (fgets(buffer_get, 256, f)) {
                    p = strstr(buffer_get, p_eur_vpncls_buff[i]->inf);
                    if (p != NULL)
                    {
                        trans_sign=2;
                        break; 
                    }
                    }
                }
                if(f)fclose(f);
                p += strlen(p_eur_vpncls_buff[i]->inf);       
                p++;
                for (col=0;col<16;col++) {
                    while (*p == ' ')
                        p++;
    
                    j=0;
                    while ((*p != ' ')&&(j<11)) {
                        buffer_tmp[j] = *p;
                        p++;
                        j++;
                    }
    
                    buffer_tmp[j]= 0;                  
                    if (col==0) {
                        sprintf(p_eur_vpncls_buff[i]->receivedBytes,"%s",buffer_tmp);
                    }
                    if (col==1) {
                        sprintf(p_eur_vpncls_buff[i]->receivedPackets,"%s",buffer_tmp);
                    }
                    if (col==8) {
                        sprintf(p_eur_vpncls_buff[i]->transmittedBytes,"%s",buffer_tmp);
                    }
                    if (col==9) {
                        sprintf(p_eur_vpncls_buff[i]->transmittedPackets,"%s",buffer_tmp);
                    }
                }

            }

        }//c


    }//for i

}


static void refresh_buff()
{
    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        fprintf(stderr, "Out of memory\n");
        return;
    }

    refresh_device_buff();

    if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RR][AERC_REPORT_NUM]=='D') 
    {
        refresh_json_network_buff();
        refresh_json_gps_buff();
        if (((eur_buff.mess_type_mask) & RADINFO) == RADINFO)refresh_json_rads_buff();
        if (((eur_buff.mess_type_mask) & CARINFO) == CARINFO)refresh_json_carrs_buff();
        if (((eur_buff.mess_type_mask) & IOINFO) == IOINFO)refresh_json_ioport_buff();
        if (((eur_buff.mess_type_mask) & VPNINFO) == VPNINFO)refresh_json_vpncls_buff();
        if (((eur_buff.mess_type_mask) & COM0INFO) == COM0INFO)refresh_json_com_buff();
        
    }
    if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RR][AERC_REPORT_NUM]=='B') 
    {
        refresh_eventinfo_buff();
    }

}

//some data of device and config; some other data if config and set_..._valid
static void get_init_buff()
{


    char buff[64];
    int i = 0;
    int pos = 0;


    strcpy(buff,"");
    if (true==UserDB_read_file(VersionFile, "PRODUCT=", buff, 64))
    {
       for (i=0;(i<strlen(buff)-1)&&(buff[i+1]!='"');i++)
            eur_device_buff.imageName[i]=buff[i+1];
        eur_device_buff.imageName[i]=0;
    }
    else
    {
        syslog(LOGOPTS," getSystemVersionInformation no hostname/product keyword in /etc/version\n");
    }      


    strcpy(buff,"");
    if (true==UserDB_read_file(VersionFile, "PRODUCTNAME=", buff, 64))
    {
            int j=0;
            for (i=0;(i<strlen(buff));i++)
            {
                if(buff[i]!='"') 
                {
                    eur_device_buff.productName[j]=buff[i];
                    j++;
                }
            }
        eur_device_buff.productName[j]=0;
    }
    else
    {
        syslog(LOGOPTS," getSystemVersionInformation no productname/image  keyword in /etc/version\n");
    }      


    strcpy(buff,"");
    if (true==UserDB_read_file(VersionFile, "HARDWARE=", buff, 64))
    {
        for (i=0;(i<strlen(buff)-1)&&(buff[i+1]!='"');i++)
            eur_device_buff.hardwareVersion[i]=buff[i+1];
        eur_device_buff.hardwareVersion[i]=0;
    }
    else
    {
        syslog(LOGOPTS," getSystemVersionInformation no hardware keyword in /etc/version\n");
    }      

    strcpy(buff,"");
    if (true==UserDB_read_file(VersionFile, "SOFTWARE=", buff, 64))
    {
        for (i=0;(i<strlen(buff)-1)&&(buff[i+1]!='"');i++)
            eur_device_buff.softwareVersion[i]=buff[i+1];
        eur_device_buff.softwareVersion[i]=0;
    }
    else
    {
        syslog(LOGOPTS," getSystemVersionInformation no software keyword in /etc/version\n");
    }      

    strcpy(buff,"");
    pos=0;
    pos=strlen(eur_device_buff.softwareVersion);
    if (true==UserDB_read_file(VersionFile, "BUILD=", buff, 64))
    {
        eur_device_buff.softwareVersion[pos]='-';
        eur_device_buff.softwareVersion[pos+1]='r';
        for (i=0;(i<strlen(buff)-1)&&(buff[i+1]!='"');i++)
            eur_device_buff.softwareVersion[pos+2+i]=buff[i+1];
        eur_device_buff.softwareVersion[pos+2+i]=0;
    }
    else
    {
        syslog(LOGOPTS," getSystemVersionInformation no build keyword in /etc/version\n");
    }      


    return;
}

/*
 *Configuration should be same to the config file for different system.
 *ETHINFO, CARRINFO, RADINFO/USBINFO(3G), COM0INFO, IOINFO/COM2INFO(3G) 
 */
static void init_conf()
{
    int j=0;
    char buff[50];
    SubProgram_Start("/bin/sh","/etc/exdiscovery");
    buff[0]=0;
    get_option_by_section_name(ctx, "sdpServer","discovery","mlradio",buff);
    if(buff[0]=='1')num_radio=1;
    else if(buff[0]=='2')num_radio=2;
    else num_radio=0;


        eur_buff.interval_tm = atoi(DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_SEC0+AERC_REPORT_NUM]);
        eur_buff.min_interval = eur_buff.interval_tm;

        if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RR][AERC_REPORT_NUM]=='D') 
        {
            eur_buff.product_flage = EURD_MAS_CONF_CHANGED;
            eur_buff.mess_type_mask = JSONINFO;
            if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][0]=='B') {
                eur_buff.mess_type_mask |= ETHINFO;
            }
            if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][1]=='B') {
                eur_buff.mess_type_mask |= CARINFO;
            }
            if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][2]=='B' && num_radio>0) {
                eur_buff.mess_type_mask |= RADINFO;
            }
            if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][3]=='B') {
                eur_buff.mess_type_mask |= COM0INFO;
            }
            if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][4]=='B') {
                eur_buff.mess_type_mask |= IOINFO;
            }
            for (j=0;j< MAX_VPNCLS_NUMBER;j++) 
            {
                if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][j+5]=='B') {
                    eur_buff.mess_type_mask |= VPNINFO;
                    break;
                }
            }

        }//if 'D'

        if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RR][AERC_REPORT_NUM]=='C')
            eur_buff.mess_type_mask = SDPINFO;

        if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RR][AERC_REPORT_NUM]=='B') 
        {
            eur_buff.product_flage = VIP4G_FLAGE;

            for (j=0;j<EUR_MSS_TYPE_NUMBER;j++) 
            {
                int select = DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MSS_TYPE0+j][AERC_REPORT_NUM]-'A';
                if (select == 1) {
                    eur_buff.mess_type_mask |= MODEMINFO;
                    continue;
                }
                if (select == 2) {
                    eur_buff.mess_type_mask |= CARRINFO;
                    continue;
                }
                if (select == 3) {
                    eur_buff.mess_type_mask |= WANINFO;
                    continue;
                }
            }//for
            //syslog(LOGOPTS,"%s report%d after read db mess_type_mask=%d\n",__func__,AERC_REPORT_NUM,eur_buff.mess_type_mask);
        }//if 'B'
}

 //2.get system's hardware list and sign their validation.
static void init_system_set()
{
    int i,j,vpn_i;
    FILE* f;
    char *p;
    char buffer_name[20];
    char buffer_get[256];
    char buffer_str[20];
    char buffer_key;
    char vpn_conf_set;

    if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RR][AERC_REPORT_NUM]=='D') 
    {
        //defualt set:1,2,1,0,2   ------------when changed, related program need to amend.
        set_eur_eths_valid[0]=EUR_SET_ON;
        set_eur_eths_valid[1]=EUR_SET_ETH1_ONOROFF;
        set_eur_carrs_valid[0]=EUR_SET_ON;
        set_eur_radios_valid[0]=EUR_SET_ON;
        set_eur_usbs_valid[0]=EUR_SET_OFF;
        set_eur_coms_valid[0]=EUR_SET_ON;
        set_eur_io_valid[0]=EUR_SET_ON;  //if set OFF, need amend configuration file.
    
    
        vpn_i=0;
        if (!(f = fopen("/etc/config/eurd", "r"))) 
        {
         //do nothing. then all p_vpn_buff[] is NULL.
        }else
        {
            while (fgets(buffer_get, 256, f)) 
            {
                p=NULL;
                j=AERC_REPORT_NUM;
                sprintf(buffer_str,"Event_VPN%d",j);
                p = strstr(buffer_get, buffer_str);
                if(p != NULL)
                {
                    buffer_name[0]=0;
                    buffer_key=' ';
                    p+=strlen(buffer_str);
                    buffer_key=*p;
                    p+=strlen("s_");
                    j=0;
                    while ((*p != ' ')&&(*p != '\'')&&(j<19)) 
                    {
                        buffer_name[j] = *p;
                        p++;
                        j++;
                    }
                    buffer_name[j] = 0;
                    if(*p=='\'') p++;
                    while(*p == ' ') p++;
                    if( *p=='\'' ) p++;
                    vpn_conf_set=*p;
                    if(vpn_conf_set != 'B')vpn_conf_set='A';
                    DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][vpn_i+5]=vpn_conf_set;
                    if(vpn_conf_set == 'B' && vpn_i<MAX_VPNCLS_NUMBER)
                    {
                        eur_buff.mess_type_mask |= VPNINFO;
                        p_eur_vpncls_buff[vpn_i]=malloc(sizeof(struct _EUR_vpncls_data_t));
                        bzero(p_eur_vpncls_buff[vpn_i],sizeof(struct _EUR_vpncls_data_t));
                        //option 'Event_VPN0s_g2g1' 'A'
                        p_eur_vpncls_buff[vpn_i]->vpn_key[0]=buffer_key;
                        p_eur_vpncls_buff[vpn_i]->vpn_key[1]='_';
                        p_eur_vpncls_buff[vpn_i]->vpn_key[2]=0;
                        strcpy(p_eur_vpncls_buff[vpn_i]->name,buffer_name);
                        vpn_i++;
                    }
    
                }//if p
    
            }//while f
        }//if f
        if(f)fclose(f);
    }//if D
}

//3decide the malloc struture needed by configuration.
static void init_eur_data()
{
    int i;

    if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RR][AERC_REPORT_NUM]=='D') 
    {
        if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][0]=='B') 
        {
           for(i=0;i<MAX_ETHS_NUMBER;i++) 
           {
               if(set_eur_eths_valid[i])
               {
                   p_eur_eths_buff[i]=malloc(sizeof(struct _EUR_eths_data_t));
                   bzero(p_eur_eths_buff[i],sizeof(struct _EUR_eths_data_t));
               }
           }
        }
        if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][1]=='B') 
        {
           for(i=0;i<MAX_CARRS_NUMBER;i++) 
           {
               if(set_eur_carrs_valid[i])
               {
                   p_eur_carrs_buff[i]=malloc(sizeof(struct _EUR_carrs_data_t));
                   bzero(p_eur_carrs_buff[i],sizeof(struct _EUR_carrs_data_t));
               }
           }
        }
        if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][2]=='B') 
         {
            for(i=0;i<MAX_RADIOS_NUMBER;i++) 
            {
                if(set_eur_radios_valid[i])
                {
                    p_eur_radios_buff[i]=malloc(sizeof(struct _EUR_radios_data_t));
                    bzero(p_eur_radios_buff[i],sizeof(struct _EUR_radios_data_t));
                }
            }
        }
        if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][3]=='B') 
        {
            for(i=0;i<MAX_COMS_NUMBER;i++) 
            {
               if(set_eur_coms_valid[i])
               {
                   p_eur_coms_buff[i]=malloc(sizeof(struct _EUR_coms_data_t));
                   bzero(p_eur_coms_buff[i],sizeof(struct _EUR_coms_data_t));
               }
            }
        }
        if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+AERC_REPORT_NUM][4]=='B') 
        {
           for(i=0;i<MAX_IO_NUMBER;i++) 
           {
               if(set_eur_io_valid[i])
               {
                   p_eur_io_buff[i]=malloc(sizeof(struct _EUR_io_data_t));
                   bzero(p_eur_io_buff[i],sizeof(struct _EUR_io_data_t));
               }
           }
        }

    }//if 'D'
}//end


//host name may be changed online.
static void server2ip(char *servername,char *IPstr)
{

    struct hostent *hp;
    char **p;
    hp=gethostbyname(servername);
    while (hp == NULL)
    {
        syslog(LOGOPTS,"EURD NMS domain Server's IP not found, program will sleep 10seconds and retry.\n");
        sleep(10);
        hp=gethostbyname(servername);
    }
    p = hp->h_addr_list;
    struct in_addr in;
    memcpy(&in.s_addr, *p, sizeof(in.s_addr));
    //sprintf(DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_REIP0+AERC_REPORT_NUM],"%s",inet_ntoa(in));
    sprintf(IPstr,"%s",inet_ntoa(in));
}


static void init_socket()
{
    int rport;
    /* setup UDP sock to remote server */
    /* Get the Socket file descriptor */
    if ((eur_buff.sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
    {
        //it should be recorded into syslog

        syslog(LOGOPTS,"EURD: %s Failed to obtain remote Socket Descriptor for report%d!\n",__FUNCTION__,AERC_REPORT_NUM);
        return;
    }

    rport=atoi(DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RPORT0+AERC_REPORT_NUM]);


    int i=AERC_REPORT_NUM;
    if(i==3 && DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RR][AERC_REPORT_NUM]=='D')//special for NMS,need to config address as nms domain server.
    {
        get_option_by_section_name(ctx, "eurd", "nms_conf", "nmsserver", DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_REIP0+AERC_REPORT_NUM]);
    }

    /* Fill the socket address struct */
    eur_buff.addr_remote.sin_family = AF_INET;                   // Protocol Family
    eur_buff.addr_remote.sin_port = htons(rport);                 // Port number
    char serverip[32];
    server2ip(DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_REIP0+AERC_REPORT_NUM],serverip);
    inet_pton(AF_INET, serverip, &(eur_buff.addr_remote.sin_addr)); // Net Address
    bzero(&(eur_buff.addr_remote.sin_zero), 8);                  // Flush the rest of struct
}


/*
 * read version file to DB
 */
static void init_eurd()
{

    //2.get system's hardware list and sign their validation.
    init_system_set();

    //1.set config type mask info.
    init_conf();

    //3decide the malloc struture needed by configuration.
    init_eur_data();

    //4.get system initial information(only need to read one time)   if DataBase1_Buff[]  and if p_eur_radios_buff[] not NULL
    get_init_buff();

    //5.init UDP socket
    init_socket();

}

/*
*first read from /etc/config/udprpt.conf(from web) or /etc/config/eurd, save to /etc/config/eurd and unlink the file. 
*
*/
static int read_udpconf(void)
{
    //1.read from /etc/config/eurd
    read_Database(ADVANCED_EUR_CONF);


    ////////////
    //2.check and read from /etc/config/udprpt.conf, save to /etc/config/eurd, unlink the file.
    ////////////





    //check current config data is valid.
    if (atoi(DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_SEC0+AERC_REPORT_NUM])==0) 
    {
            DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RR][AERC_REPORT_NUM]='A';
            syslog(LOGOPTS,"EURD will exit: because of 0 time interval report%d\n",AERC_REPORT_NUM);
    }
}

/*
*SPD information
*/
static int fill_sdpinfo_package(char *buffer_send)
{
    char ip_buffer[32]="0.0.0.0";
    char name_buff[32]=" ";
    struct in_addr p_ip;
    struct ether_addr *p_ether;

    if (false==fetch_Local_IP_MAC(ifname_bridge,ip_buffer)) {
        fetch_Local_IP_MAC(ifname_eth0,ip_buffer);
        //create_wireless_socket();
    }

    p_ether = ether_aton(ip_buffer);
    buffer_send[2] = (unsigned char)p_ether->ether_addr_octet[0];
    buffer_send[3] = (unsigned char)p_ether->ether_addr_octet[1];
    buffer_send[4] = (unsigned char)p_ether->ether_addr_octet[2];
    buffer_send[5] = (unsigned char)p_ether->ether_addr_octet[3];
    buffer_send[6] = (unsigned char)p_ether->ether_addr_octet[4];
    buffer_send[7] = (unsigned char)p_ether->ether_addr_octet[5];

    if (false==fetch_Local_IP_ADDR(ifname_bridge,ip_buffer)) {
        fetch_Local_IP_ADDR(ifname_eth0,ip_buffer);

    }


    int length = 12+strlen(eur_device_buff.deviceName);
    buffer_send[0]= SDP_RESPONSE_INQUIRY;
    buffer_send[1]= length;

    buffer_send[8] = SDP_DISCOVER_STATUS_DISCOVABLE;

    if (0==inet_aton(ip_buffer,&p_ip)) {
        syslog(LOGOPTS,"EURD: %s Failed to invoke inet_aton function!\n",__FUNCTION__);
        return 0;
    } else {
        buffer_send[9] = (ntohl(p_ip.s_addr)>>24)&(0xff);
        buffer_send[10] =(ntohl(p_ip.s_addr)>>16)&(0xff);
        buffer_send[11] = (ntohl(p_ip.s_addr)>>8)&(0xff);
        buffer_send[12] = ntohl(p_ip.s_addr)&(0xff);
    }

    int j = strlen(name_buff);
    int i=0;
    for (i=0;i<j;i++) {
        buffer_send[13+i] = name_buff[i];
    }
    buffer_send[13+j] =0;

    int sendlen= 14+j;

    buffer_send[sendlen]='\0';
    sendlen++;

    char productname_buff[32]=" ";
    char firmware_buff[32]=" ";
    char networkname[128]=" ";

    j = strlen(eur_device_buff.productName);
    for (i=0;i<j;i++) {
        buffer_send[sendlen+i] = eur_device_buff.productName[i];
    }
    buffer_send[sendlen+j]='\0';
    sendlen=sendlen+j+1;

    j = strlen(eur_device_buff.softwareVersion);
    for (i=0;i<j;i++) {
        buffer_send[sendlen+i] = eur_device_buff.softwareVersion[i];
    }
    buffer_send[sendlen+j]='\0';
    sendlen=sendlen+j+1;

    buffer_send[sendlen]='\0';
    sendlen+=1;


    memset( networkname,0, sizeof(networkname));
    strcpy(networkname,"N/A");
    get_option_by_section_name(ctx, "lte", "connect", "apn", networkname);
    check_current_apn(networkname);

    j = strlen(networkname);
    for (i=0;i<j;i++) {
        buffer_send[sendlen+i] = networkname[i];
    }
    buffer_send[sendlen+j]='\0';
    sendlen=sendlen+j+1;

    buffer_send[1]= sendlen-1;

    return sendlen-1;
}

/*
*
*/
static int fill_json_header(char *buf)
{
    char *pos = buf;
    int length = 0;
    int temp=0;
    time_t timestamp;
    char tmpbuf[256];

    /* header information */
    /*
     * protocol and version
     */
    strcpy(buf,"{\"header\":{\"protocol\":\"UDPReport\",\"version\":1.0,");
    length += strlen("{\"header\":{\"protocol\":\"UDPReport\",\"version\":1.0,");
    pos += length;

    /*
     * timestamp
     */
    timestamp = time (NULL);
    sprintf(tmpbuf,"\"timestamp\":%ld},",timestamp);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    return length;
}

static int fill_json_radio_playload(char *buf)
{
    char *pos = buf;
    int length = 0;
    int temp = 0;
    char tmpbuf[256];
    char *p;
    int j=0;
    if(p_eur_radios_buff[0]==NULL)return 0;


    sprintf(tmpbuf,"\"radios\":[{\"mid\":0,");
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    p=p_eur_radios_buff[0]->ipAddr;
    j=0;
    while(*p==' ' && j<4 )
    {
        p++;
        j++;
    }
    if( *(p)=='0' || j>2 ) 
    {
        strcpy(p_eur_radios_buff[0]->ipAddr,"N/A");
    }


    sprintf(tmpbuf,"\"ipAddr\":\"%s\",",p_eur_radios_buff[0]->ipAddr);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"macAddr\":\"%s\",",p_eur_radios_buff[0]->macAddr);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"networkName\":\"%s\",",p_eur_radios_buff[0]->networkName);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"operationMode\":\"%s\",",p_eur_radios_buff[0]->operationMode);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"searchMode\":\"%s\",",p_eur_radios_buff[0]->searchMode);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"authenticationKey\":\"%s\",",p_eur_radios_buff[0]->authenticationKey);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"encryptionType\":\"%s\",",p_eur_radios_buff[0]->encryptionType);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
//    "rssi":["-45","-98","-87"],		//contains multi-rssiValue
    sprintf(tmpbuf,"\"rssi\":%s,",p_eur_radios_buff[0]->rssi);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"childRssi\":\"%s\",",p_eur_radios_buff[0]->childRssi);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"coreTemperature\":\"%s\",",p_eur_radios_buff[0]->coreTemperature);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"supplyVoltage\":\"%s\",",p_eur_radios_buff[0]->supplyVoltage);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"vswr\":\"%s\",",p_eur_radios_buff[0]->vswr);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"linkRate\":\"%s\",",p_eur_radios_buff[0]->linkRate);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"txRate\":\"%s\",",p_eur_radios_buff[0]->txRate);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"channelBandwidth\":\"%s\",",p_eur_radios_buff[0]->channelBandwidth);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"receivedBytes\":%s,",p_eur_radios_buff[0]->receivedBytes);//Sysinfo_packet_statistic_show[0][9]

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"receivedPackets\":%s,",p_eur_radios_buff[0]->receivedPackets);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedBytes\":%s,",p_eur_radios_buff[0]->transmittedBytes);


    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedPackets\":%s}],",p_eur_radios_buff[0]->transmittedPackets);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;


    return length;
}

static int fill_json_eth_playload(char *buf)
{
    char *pos = buf;
    int length = 0;
    int temp = 0;
    int j = 0;
    char tmpbuf[256];
    char proname[32];
    char *p;

    /* ethernet information */

    sprintf(tmpbuf,"\"ethernets\":[{\"mid\":0,");

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * Local IP and mask
     */
    sprintf(tmpbuf,"\"ipAddr\":\"%s\",",p_eur_eths_buff[0]->ipAddr);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"ipMask\":\"%s\",",p_eur_eths_buff[0]->ipMask);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
    * mac address
    */
    sprintf(tmpbuf,"\"macAddr\":\"%s\",",p_eur_eths_buff[0]->macAddr);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /* eth traffic*/
    sprintf(tmpbuf,"\"receivedBytes\":%s,",p_eur_eths_buff[0]->receivedBytes);//Sysinfo_packet_statistic_show[0][9]

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"receivedPackets\":%s,",p_eur_eths_buff[0]->receivedPackets);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedBytes\":%s,",p_eur_eths_buff[0]->transmittedBytes);


    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedPackets\":%s}",p_eur_eths_buff[0]->transmittedPackets);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;


    /*eth1:
     * Local IP and mask
     */
if(p_eur_eths_buff[1]!=NULL)
{
    p=p_eur_eths_buff[1]->ipAddr;
    j=0;
    while(*p==' ' && j<4 )
    {
        p++;
        j++;
    }
    if( *(p)=='0' || j>2 ) 
    {
        strcpy(p_eur_eths_buff[1]->ipAddr,"N/A");
    }

    sprintf(tmpbuf,",{\"mid\":1,\"ipAddr\":\"%s\",",p_eur_eths_buff[1]->ipAddr);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"ipMask\":\"%s\",",p_eur_eths_buff[1]->ipMask);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
    * mac address
    */
    sprintf(tmpbuf,"\"macAddr\":\"%s\",",p_eur_eths_buff[1]->macAddr);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /* eth traffic*/
    sprintf(tmpbuf,"\"receivedBytes\":%s,",p_eur_eths_buff[1]->receivedBytes);//Sysinfo_packet_statistic_show[0][9]
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"receivedPackets\":%s,",p_eur_eths_buff[1]->receivedPackets);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"transmittedBytes\":%s,",p_eur_eths_buff[1]->transmittedBytes);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"transmittedPackets\":%s}",p_eur_eths_buff[1]->transmittedPackets);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
}

    sprintf(tmpbuf,"],");
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;

    return length;
}

static int fill_json_carrinfo_playload(char *buf)
{
    char *pos = buf;
    int length = 0;
    int temp = 0;
    uint8_t rssi=0;
    char tmpbuf[256];
    char *p;
    int j=0;
    if(p_eur_carrs_buff[0]==NULL)return 0;
    /* carriers information */
    sprintf(tmpbuf,"\"carriers\":[{\"mid\":0,");

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    /*
     * wan ip
     */
    p=p_eur_carrs_buff[0]->ipAddr;
    j=0;
    while(*p==' ' && j<4 )
    {
        p++;
        j++;
    }
    if( *(p)=='0' || j>2 ) 
    {
        strcpy(p_eur_carrs_buff[0]->ipAddr,"N/A");
    }

    sprintf(tmpbuf,"\"ipAddr\":\"%s\",",p_eur_carrs_buff[0]->ipAddr);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * dns1
     */
    sprintf(tmpbuf,"\"dns1\":\"%s\",",p_eur_carrs_buff[0]->dns1);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * dns2
     */
    sprintf(tmpbuf,"\"dns2\":\"%s\",",p_eur_carrs_buff[0]->dns2);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * rssi
     */
    sprintf(tmpbuf,"\"rssi\":\"%s\",",p_eur_carrs_buff[0]->rssi);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * rsrp
     */
    sprintf(tmpbuf,"\"rsrp\":\"%s\",",p_eur_carrs_buff[0]->RSRP);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    /*
     * rsrq
     */
    sprintf(tmpbuf,"\"rsrq\":\"%s\",",p_eur_carrs_buff[0]->RSRQ);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * ecno
     */
/*
    sprintf(tmpbuf,"\"ecNo\":\"%s\",",p_eur_carrs_buff[0]->ecNo);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
*/
    /*
     * rscp
     */
/*
    sprintf(tmpbuf,"\"rscp\":\"%s\",",p_eur_carrs_buff[0]->rscp);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
*/
    /*
     * rf band
     */
/*
    sprintf(tmpbuf,"\"frequencyBand\":\"%s\",",p_eur_carrs_buff[0]->frequencyBand);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
*/
    /*
     * network
     */
    sprintf(tmpbuf,"\"threeGNetwork\":\"%s\",",p_eur_carrs_buff[0]->threeGNetwork);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * service type
     */
    sprintf(tmpbuf,"\"serviceType\":\"%s\",",p_eur_carrs_buff[0]->serviceType);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * channel number
     */
/*
    sprintf(tmpbuf,"\"channelNumber\":\"%s\",",p_eur_carrs_buff[0]->channelNumber);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
*/
    /*
     * sim card number
     */

    p=p_eur_carrs_buff[0]->simCardNumber;
    while(*p==' ')p++;
    if( *(p+1)=='n' && *(p+2)=='k') 
    {
        strcpy(p_eur_carrs_buff[0]->simCardNumber,"N/A");
    }

    sprintf(tmpbuf,"\"simCardNumber\":\"%s\",",p_eur_carrs_buff[0]->simCardNumber);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * phone number
     */
    sprintf(tmpbuf,"\"phoneNumber\":\"%s\",",p_eur_carrs_buff[0]->phoneNumber);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * temperature
     */
    sprintf(tmpbuf,"\"coreTemperature\":\"%s\",",p_eur_carrs_buff[0]->coreTemperature);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * voltage
     */
    sprintf(tmpbuf,"\"supplyVoltage\":\"%s\",",p_eur_carrs_buff[0]->supplyVoltage);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * apn
     */
    sprintf(tmpbuf,"\"apn\":\"%s\",",p_eur_carrs_buff[0]->apn);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    //mnc,mcc,lac
    sprintf(tmpbuf,"\"mnc\":\"%s\",",p_eur_carrs_buff[0]->mnc);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"mcc\":\"%s\",",p_eur_carrs_buff[0]->mcc);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"lac\":\"%s\",",p_eur_carrs_buff[0]->lac);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * cell id
     */
    sprintf(tmpbuf,"\"cellId\":\"%s\",",p_eur_carrs_buff[0]->cellId);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * activity status
     */
    sprintf(tmpbuf,"\"activityStatus\":\"%s\",",p_eur_carrs_buff[0]->activityStatus);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * roaming
     */
    sprintf(tmpbuf,"\"homeRoaming\":\"%s\",",p_eur_carrs_buff[0]->homeRoaming);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * IMEI
     */
    sprintf(tmpbuf,"\"imei\":\"%s\",",p_eur_carrs_buff[0]->imei);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * IMSI
     */
    sprintf(tmpbuf,"\"imsi\":\"%s\",",p_eur_carrs_buff[0]->imsi);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;


    sprintf(tmpbuf,"\"latitude\":\"%s\",",p_eur_carrs_buff[0]->latitude);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"longitude\":\"%s\",",p_eur_carrs_buff[0]->longitude);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;


    sprintf(tmpbuf,"\"radius\":\"%s\",",p_eur_carrs_buff[0]->radius);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;


    /* carrier traffic*/
    sprintf(tmpbuf,"\"receivedBytes\":%s,",p_eur_carrs_buff[0]->receivedBytes);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"receivedPackets\":%s,",p_eur_carrs_buff[0]->receivedPackets);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedBytes\":%s,",p_eur_carrs_buff[0]->transmittedBytes);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedPackets\":%s}],",p_eur_carrs_buff[0]->transmittedPackets);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;

    return length;
}

//none definition in VIP4G
static int fill_json_usb_playload(char *buf)
{
    char *pos = buf;
    int length = 0;
    int temp = 0;
    char tmpbuf[256];
    if(p_eur_usbs_buff[0]==NULL)return 0;

    /* usb information */
    sprintf(tmpbuf,"\"usbs\":[{\"mid\":0,");

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /* usb traffic*/
    sprintf(tmpbuf,"\"receivedBytes\":%s,",p_eur_usbs_buff[0]->receivedBytes);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"receivedPackets\":%s,",p_eur_usbs_buff[0]->receivedPackets);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedBytes\":%s,",p_eur_usbs_buff[0]->transmittedBytes);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedPackets\":%s}],",p_eur_usbs_buff[0]->transmittedPackets);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;

    return length;
}
//io definition in VIP4G
static int fill_json_io_playload(char *buf)
{
    char *pos = buf;
    int length = 0;
    int temp = 0;
//    int i;
    char tmpbuf[256];
    if(p_eur_io_buff[0]==NULL)return 0;
    sprintf(tmpbuf,"\"dio\":{");

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /* io input */
//    i=strlen(p_eur_io_buff[0]->InputStatus);
    sprintf(tmpbuf,"\"input\":\"%s\",",p_eur_io_buff[0]->InputStatus);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /* io output */
    sprintf(tmpbuf,"\"output\":\"%s\"},",p_eur_io_buff[0]->OutputStatus);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    return length;
}

static int fill_json_com_header(char *buf)
{
    char *pos=buf;
    int length = 0;
    int temp = 0;
    if(p_eur_coms_buff[0]==NULL)return 0;

    /* com header information */

    temp=scpytrim(pos,"\"coms\":[",strlen("\"coms\":["));
    length += temp;
    return length;
}

static int fill_json_com_comma(char *buf)
{
    char *pos=buf;
    int length = 0;
    int temp = 0;
    if(p_eur_coms_buff[0]==NULL)return 0;
    /* com comma information */

    temp=scpytrim(pos,",",strlen(","));
    length += temp;
    return length;
}

static int fill_json_com_end(char *buf)
{
    char *pos=buf;
    int length = 0;
    int temp = 0;
    if(p_eur_coms_buff[0]==NULL)return 0;
    /* com end information */

    temp=scpytrim(pos,"],",strlen("],"));
    length += temp;
    return length;
}

static int fill_json_com1_playload(char *buf)
{
    char *pos=buf;
    int length = 0;
    int temp = 0;
    char tmpbuf[256];
    if(p_eur_coms_buff[0]==NULL)return 0;
    /* com1 information */
    sprintf(tmpbuf,"{\"mid\":0,");

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    /* com1 traffic*/
    sprintf(tmpbuf,"\"receivedBytes\":%s,",p_eur_coms_buff[0]->receivedBytes);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"receivedPackets\":%s,",p_eur_coms_buff[0]->receivedPackets);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedBytes\":%s,",p_eur_coms_buff[0]->transmittedBytes);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedPackets\":%s}",p_eur_coms_buff[0]->transmittedPackets);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;

    return length;
}

static int fill_json_com2_playload(char *buf)
{
    char *pos=buf;
    int length = 0;
    int temp=0;
    char tmpbuf[256];

    /* com2 information */
    sprintf(tmpbuf,"{\"mid\":1,");

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /* com2 traffic*/
    sprintf(tmpbuf,"\"receivedBytes\":%s,",p_eur_coms_buff[1]->receivedBytes);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"receivedPackets\":%s,",p_eur_coms_buff[1]->receivedPackets);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedBytes\":%s,",p_eur_coms_buff[1]->transmittedBytes);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"\"transmittedPackets\":%s}",p_eur_coms_buff[1]->transmittedPackets);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;

    return length;
}

static int fill_json_vpn_playload(char *buf, struct mod_list *mlistp, int rid)
{
    char *pos = buf;
    int length = 0;
    int temp = 0;
    char tmpbuf[256];
    char cfile[32]="";
    char ipadd[32]="";
    char vpn_status[2]="";
    char inf[8]="";
    int i,j,col;
    int flag=0;
    FILE* fp;
    bool firstone=true;


    for (i=0;i<MAX_VPNCLS_NUMBER;i++) {
        //IF IS USED

        if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_MAS0+rid][i+5]=='A' || p_eur_vpncls_buff[i]==NULL)
            continue;
        if (firstone) {
            firstone=false;
            sprintf(tmpbuf,"\"vpnClients\":[{\"mid\":0,");

            temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
            length += temp;
            pos += temp;
            sprintf(tmpbuf,"\"Vpn%s",p_eur_vpncls_buff[i]->name);
            strcpy(mlistp->vpn,tmpbuf);
        } else {
            sprintf(tmpbuf,",{\"mid\":%d,",flag);
            temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
            length += temp;
            pos += temp;
            sprintf(tmpbuf,",\"Vpn%s\"",p_eur_vpncls_buff[i]->name);
            strcat(mlistp->vpn,tmpbuf);
        }
        flag++;
        /*
         * xl2tp client name and protocol
         */
        sprintf(tmpbuf,"\"name\":\"%s\",\"protocol\":\"%s\",",p_eur_vpncls_buff[i]->name,p_eur_vpncls_buff[i]->protocol);
        temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
        length += temp;
        pos += temp;

        /*
         * xl2tp connection status
         */
        sprintf(tmpbuf,"\"connected\":\"%s\",",p_eur_vpncls_buff[i]->connected);
        temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
        length += temp;
        pos += temp;

        /*
         * xl2tp client interface
         */
        sprintf(tmpbuf,"\"inf\":\"%s\",",p_eur_vpncls_buff[i]->inf);
        temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
        length += temp;
        pos += temp;

        /*
         * xl2tp client IP
         */
        sprintf(tmpbuf,"\"ipAddr\":\"%s\",",p_eur_vpncls_buff[i]->ipAddr);
        temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
        length += temp;
        pos += temp;

        /*
         * xl2tp client server gate IP
         */
        sprintf(tmpbuf,"\"gateway\":\"%s\",",p_eur_vpncls_buff[i]->gateway);
        temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
        length += temp;
        pos += temp;

        /*
        * vpn traffic
        */
/* 
                   ***********see original code to check how to get these data. 
*/ 
        sprintf(tmpbuf,"\"receivedBytes\":%s,",p_eur_vpncls_buff[i]->receivedBytes);

        temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
        length += temp;
        pos += temp;
        sprintf(tmpbuf,"\"receivedPackets\":%s,",p_eur_vpncls_buff[i]->receivedPackets);

        temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
        length += temp;
        pos += temp;
        sprintf(tmpbuf,"\"transmittedBytes\":%s,",p_eur_vpncls_buff[i]->transmittedBytes);

        temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
        length += temp;
        pos += temp;
        sprintf(tmpbuf,"\"transmittedPackets\":%s}",p_eur_vpncls_buff[i]->transmittedPackets);

        temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
        length += temp;
        pos += temp;

    }
    if (flag>0) {
        sprintf(tmpbuf,"],");

        temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
        length += temp;
    }
    //syslog(LOGOPTS,"%s vpnlist=%s\n",__func__,mlistp->vpn);

    return length;
}

static int fill_json_conf_playload(char *buf, struct mod_list *mlistp, int rid)
{
    char *pos = buf;
    int length = 0;
    int temp = 0;
    int j = 0;
    char tmpbuf[1024];

    /* configuration information */

    sprintf(tmpbuf,"\"config\":{");
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * report server
     */
    sprintf(tmpbuf,"\"repServer\":\"%s\",",DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_REIP0+rid]);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
    * report port
    */
    sprintf(tmpbuf,"\"repPort\":\"%s\",",DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RPORT0+rid]);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
    * interval
    */
    sprintf(tmpbuf,"\"repInterval\":%s,",DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_SEC0+rid]);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
    * report id
    */
    sprintf(tmpbuf,"\"webUIRepId\":%d,",rid);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * modulesList  //radio, ethernet, carrier, usb, com
     */
    j=0;
    sprintf(tmpbuf,"\"desiredModules\":[");
    if (mlistp->radio) {
        strcat(tmpbuf,"\"Radio0\"");
        j++;
    }
    if (mlistp->eth) {
        if (j) {
            strcat(tmpbuf,",");
            j = 0;
        }
        strcat(tmpbuf,"\"Ethernet0\"");
        j++;
    }
    if (mlistp->eth > 1) {
        if (j) {
            strcat(tmpbuf,",");
            j = 0;
        }
        strcat(tmpbuf,"\"Ethernet1\"");
        j++;
    }

    if (mlistp->carrier) {
        if (j) {
            strcat(tmpbuf,",");
            j = 0;
        }
        strcat(tmpbuf,"\"Carrier0\"");
        j++;
    }

    if (mlistp->com) {
        if (j) {
            strcat(tmpbuf,",");
            j = 0;
        }
        strcat(tmpbuf,"\"Com0\"");
        j++;
    }
    if (mlistp->ioport) {
        if (j) {
            strcat(tmpbuf,",");
            j = 0;
        }
        strcat(tmpbuf,"\"IOPort\"");
        j++;
    }
    if (strcmp(mlistp->vpn,"")!=0) {
        if (j) {
            strcat(tmpbuf,",");
            j = 0;
        }
        strcat(tmpbuf,mlistp->vpn);
    }
    strcat(tmpbuf,"]},");
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    return length;
}


static int fill_json_domain_playload(char *buf, struct mod_list *mlistp, int rid)
{
//    "domain":{"name":"demo.microhard.com"},
    char *pos = buf;
    int length = 0;
    int temp = 0;
    char tmpbuf[100];

    char dmdata[64];

    sprintf(tmpbuf,"\"domain\":{");
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    get_option_by_section_name(ctx, "eurd", "nms_conf", "domainname", dmdata);
    if(strlen(dmdata)<2) {
        strcpy(dmdata,"N/A");
    }
    sprintf(tmpbuf,"\"name\":\"%s\",",dmdata);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    get_option_by_section_name(ctx, "eurd", "nms_conf", "domainpasswd", dmdata);
    if(strlen(dmdata)<2) {
        strcpy(dmdata,"N/A");
    }
    sprintf(tmpbuf,"\"password\":\"%s\"},",dmdata);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    return length;
}
static int fill_json_gps_playload(char *buf, struct mod_list *mlistp, int rid)
{
    char *pos = buf;
    int length = 0;
    int temp = 0;
    char tmpbuf[256];

    if(eur_gps_buff.gps_flag!=1)
    {
        return 0;
    }

    /* device information */
    sprintf(tmpbuf,"\"gps\":{");
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"latitude\":\"%s\",",eur_gps_buff.latitude);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    sprintf(tmpbuf,"\"longitude\":\"%s\"},",eur_gps_buff.longitude);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    return length;
}


static int fill_json_device_playload(char *buf, struct mod_list *mlistp, int rid)
{
    char *pos = buf;
    int length = 0;
    int temp = 0;
    int j = 0;
    struct sysinfo info;
    long uptimes = 0;
    char tmpbuf[512];
    char vpn_name[32];
    FILE *fp=NULL;

    /* device information */

    sprintf(tmpbuf,"\"device\":{");
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    /*
     * device name
     */
    sprintf(tmpbuf,"\"deviceName\":\"%s\",",eur_device_buff.deviceName);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
    * product name
    */
    sprintf(tmpbuf,"\"productName\":\"%s\",",eur_device_buff.productName);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
    * family name
    */
    sprintf(tmpbuf,"\"imageName\":\"%s\",",eur_device_buff.imageName);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
    * mac address
    */
    sprintf(tmpbuf,"\"macAddr\":\"%s\",",eur_device_buff.macAddr);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * hardware version
     */
    sprintf(tmpbuf,"\"hardwareVersion\":\"%s\",",eur_device_buff.hardwareVersion);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * software version
     */
    sprintf(tmpbuf,"\"softwareVersion\":\"%s\",",eur_device_buff.softwareVersion);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
    * uptime
    */
    sysinfo(&info);
    uptimes = info.uptime;
    sprintf(tmpbuf,"\"uptime\":%ld,",uptimes);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
    * interval
    */
    sprintf(tmpbuf,"\"repInterval\":%s,",DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_SEC0+rid]);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
    * report id
    */
    sprintf(tmpbuf,"\"webUIRepId\":%d,",rid);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    //"httpPort":8080,
    sprintf(tmpbuf,"\"httpPort\":%s,",eur_device_buff.httpPort);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * modulesList   //radio, ethernet, carrier, usb, com
     */
    int eth_num=1+EUR_SET_ETH1_ONOROFF;
    sprintf(tmpbuf,"\"modulesList\":[%d,%d,1,0,1],",num_radio,eth_num);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    /*
     * l2tpclientsList
     */
    sprintf(tmpbuf,"\"vpnClientList\":[%s]",eur_device_buff.vpnClientList);
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;
    sprintf(tmpbuf,"}}");
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    return length;
}

static int fill_modeminfo_package(char *buf)
{
    char proname[256];
    char *pos=buf+EURD_MODEMINFO_HEADER;
    uint16_t length = 0;
    int temp=0;
    struct in_addr t_ip;
    uint16_t tmp16_hton;
    uint32_t tmp32_hton;
        /*
     * modem name
     */
    strcpy(proname,"");
    if(get_option_by_section_name(ctx, "system", "@system[0]", "hostname", proname))
    {
        strcpy(eur_device_buff.deviceName,proname);
    }
    temp = strlen(eur_device_buff.deviceName)+1;
    memcpy(pos,eur_device_buff.deviceName,strlen(eur_device_buff.deviceName)+1);
    length += temp;
    pos += temp;

    /*
     * hardware version
     */
    temp = strlen(eur_device_buff.hardwareVersion)+1;
    memcpy(pos,eur_device_buff.hardwareVersion,strlen(eur_device_buff.hardwareVersion)+1);
    length += temp;
    pos += temp;

    /*
     * software version
     */
    temp = strlen(eur_device_buff.softwareVersion)+1;
    memcpy(pos,eur_device_buff.softwareVersion,strlen(eur_device_buff.softwareVersion)+1);
    length += temp;
    pos += temp;

    /*
     * temperature
     */
    FILE* f;
    char buffer_get[256];
    char *p;
    int j;
    strcpy(proname,"N/A");
    if (!(f = fopen("/var/run/std_lte_statue", "r"))) 
    {
        //do nothing.
    }else
    {
        while (fgets(buffer_get, 256, f)) 
        {
            p=NULL;
            p = strstr(buffer_get, "temp=");//temp= 39 degC
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(proname,"%s",p);
                    break;
                }
            }

        }//while
    }//else
    if(f)fclose(f);

    temp = strlen(proname)+1;
    memcpy(pos,proname,strlen(proname)+1);
    length += temp;
    pos += temp;

    /*
     * voltage
     */
    temp = strlen("N/A")+1;
    memcpy(pos,"N/A",strlen("N/A")+1);
    length += temp;
    pos += temp;

    /*
     * Local IP and mask
     */
    strcpy(proname,"N/A");
    if (false==fetch_Local_IP_ADDR(ifname_bridge,proname))fetch_Local_IP_ADDR(ifname_eth0,proname);
    if (inet_aton(proname,&t_ip)==0) {
        tmp32_hton=0;
    } else {
        tmp32_hton=t_ip.s_addr;//htons_rv32(t_ip.s_addr);
    }
    memcpy(pos,&(tmp32_hton),sizeof(unsigned long));
    temp = sizeof(unsigned long);
    length += temp;
    pos += temp;

    strcpy(proname,"N/A");
    if (false==fetch_Local_IP_MASK(ifname_bridge,proname))fetch_Local_IP_MASK(ifname_eth0,proname);
    if (inet_aton(proname,&t_ip)==0) {
        tmp32_hton=0;
    } else {
        tmp32_hton=t_ip.s_addr;//htons_rv32(t_ip.s_addr);
    }
    memcpy(pos,&(tmp32_hton),sizeof(unsigned long));
    temp = sizeof(unsigned long);
    length += temp;
    pos += temp;

    /*
     * package length
     */

    tmp16_hton=htons_rv16(length);
    memcpy(buf,&tmp16_hton,EURD_MODEMINFO_HEADER);
    length+=EURD_MODEMINFO_HEADER;

    return length;
}

static int fill_carrinfo_package(char *buf)
{
   char *pos=buf+EURD_CARRINFO_HEADER;
       uint16_t tmp16_hton;
    uint16_t length = 0;
    int temp=0;
    uint8_t rssi=0;
    FILE* f;
    char buffer_get[256];
    char *p;
    char tmp_buf[32];
    char phone[30],simid[30],channel[30],service[30],network[30];
    int j;
    /*
     * rssi 
     */

    rssi=0;
    strcpy(phone,"N/A");
    strcpy(simid,"N/A");
    strcpy(channel,"N/A");
    strcpy(service,"N/A");
    strcpy(network,"N/A");
    strcpy(tmp_buf,"N/A");
    if (!(f = fopen("/var/run/std_lte_statue", "r"))) 
    {
        //do nothing.
    }else
    {
        while (fgets(buffer_get, 256, f)) 
        {
        
            p=NULL;
            p = strstr(buffer_get, "rssi=");//rssi="-72"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p = strchr(buffer_get, '-');
                    if ( p!= NULL)
                    {
                        p++;
                        rssi = atoi(p);
                    }
                }
            }

            p=NULL;
            p = strstr(buffer_get, "phonenum=");//phonenum="+14036050307"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(phone,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "simid=");//simid="89302720403007563710"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(simid,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "network=");//network="Verizon"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(network,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "state=");//state="LTE PS (Packet Switched)"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(service,"%s",p);
                }
            }
   
#if 0

            p=NULL;
            p = strstr(buffer_get, "rssi=");//rssi=" GSM RSSI : 72"
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
                        tmp_buf[j]=*p;
                        j++;
                        p++;
                     }
                    if(j>0)
                    {
                        tmp_buf[j]=0;
                        rssi = atoi(tmp_buf);
                    }
                }//if :
            }//if p



            p=NULL;
            p = strstr(buffer_get, "phonenum=");//phonenum="  TELEPHONE , +14036050307 ,145"
            if (p != NULL)
            {
                while ((*p != ',') && (*p != '\n') )p++; //key space
                if (*p==',')
                {
                    p++;
                    while(*p == ' ')p++;
                    j=0;
                    while ((*p != ' ')&& (*p!=',') && (*p!='\0') && (*p!='\n') &&(j<20)) 
                    {
                        phone[j]=*p;
                        p++;
                        j++;
                     }
                    if(j>0)phone[j]=0;
                }//if :
            }//if p


            p=NULL;
            p = strstr(buffer_get, "simid=");//simid=" 89302720403007563710"
            if (p != NULL)
            {
                p+=strlen("simid=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        simid[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0)simid[j]=0;
            }//if p



            p=NULL;
            p = strstr(buffer_get, "network=");//network=" 0,0, Bell ,2"
            if (p != NULL)
            {
                while((*p != ',')&& (*p!='\n'))p++;
                if(*p == ',') 
                {
                    p++;
                    while((*p != ',') && (*p!='\n'))p++;
                    if(*p == ',') 
                    {
                        p++;
                        while(*p == ' ')p++;
                        j=0;
                        while ((*p != ',')&&(*p != '\"')&& (*p!=' ') && (*p!='\n') &&(j<20)) 
                            {
                                network[j]=*p;
                                p++;
                                j++;
                             }
                         if(j>0)network[j]=0;
                    }
                }

            }//if p



            p=NULL;
            p = strstr(buffer_get, "rat=");//rat=" 0,2,0"
            if (p != NULL)
            {
                strcpy(service,"Unknown");
                p=NULL;
                p=strrchr(buffer_get,',');
                if(p!=NULL) 
                {
                    if(*p == ',') 
                    {
                        p++;
                        j=0;
                        while((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n'))
                        {
                            tmp_buf[j]=*p;
                            p++;
                            j++;
                        }
                        tmp_buf[j]=0;
                        j=atoi(tmp_buf);
                        switch (j) 
                        {
                        case 0:
                            strcpy(service,"Searching for Service");
                            break;
                        case 1:
                            strcpy(service,"WCDMA CS (Circuit Switched)");
                            break;
                        case 2:
                            strcpy(service,"WCDMA PS (Packet Switched)");
                            break;
                        case 3:
                            strcpy(service,"WCDMA CS and PS");
                            break;
                        case 4:
                            strcpy(service,"GSM CS (Circuit Switched)");
                            break;
                        case 5:
                            strcpy(service,"GSM PS (Packet Switched)");
                            break;
                        case 6:
                            strcpy(service,"GSM CS and PS");
                            break;
                        case 7:
                            strcpy(service,"LTE CS (Circuit Switched)");
                            break;
                        case 8:
                            strcpy(service,"LTE PS (Packet Switched)");
                            break;
                        case 9:
                            strcpy(service,"LTE CS and PS");
                            break;
                        case 10:
                            strcpy(service,"CDMA CS (Circuit Switched)");
                            break;
                        case 11:
                            strcpy(service,"CDMA PS (Packet Switched)");
                            break;
                        case 12:
                            strcpy(service,"CDMA CS and PS");
                            break;
                        case 13:
                            strcpy(service,"HDR CS (Circuit Switched)");
                            break;
                        case 14:
                            strcpy(service,"HDR PS (Packet Switched)");
                            break;
                        case 15:
                            strcpy(service,"HDR CS and PS");
                            break;
                        default:
                            break;
                        }
                    }
                }
            }//if p


#endif


        } //while
    }//if open
    if(f)fclose(f);

    memcpy(pos,&rssi,sizeof(uint8_t));
    length += sizeof(uint8_t);
    pos += sizeof(uint8_t);

    /*
     * rf band
     */
    strcpy(tmp_buf,"0MHz");
    uint16_t rf=0;
    if (strcmp(tmp_buf,"2100MHz")==0) {
        rf = 2100;
    } else if (strcmp(tmp_buf,"900MHz")==0) {
        rf = 900;
    } else if (strcmp(tmp_buf,"850MHz")==0) {
        rf = 850;
    } else if (strcmp(tmp_buf,"1700MHz")==0) {
        rf = 1700;
    } else if (strcmp(tmp_buf,"1900MHz")==0) {
        rf = 1900;
    } else if (strcmp(tmp_buf,"900MHz")==0) {
        rf = 900;
    } else if (strcmp(tmp_buf,"1800MHz")==0) {
        rf = 1800;
    }

    tmp16_hton=htons_rv16(rf);
    memcpy(pos,&tmp16_hton,sizeof(uint16_t));
    length += sizeof(uint16_t);
    pos += sizeof(uint16_t);

    /*
     * network
     */
    temp = strlen(network)+1;
    memcpy(pos,network,strlen(network)+1);
    length += temp;
    pos += temp;

    /*
     * service type
     */
    temp = strlen(service)+1;
    memcpy(pos,service,strlen(service)+1);
    length += temp;
    pos += temp;

    /*
     * channel number
     */
    temp = strlen(channel)+1;
    memcpy(pos,channel,strlen(channel)+1);
    length += temp;
    pos += temp;
    /*
     * sim card number
     */
    temp = strlen(simid)+1;
    memcpy(pos,simid,strlen(simid)+1);
    length += temp;
    pos += temp;

    /*
     * phone number 
     */
    temp = strlen(phone)+1;
    memcpy(pos,phone,strlen(phone)+1);
    length += temp;
    pos += temp;
    /*
     * package length
     */
    tmp16_hton=htons_rv16(length);
    memcpy(buf,&tmp16_hton,EURD_CARRINFO_HEADER);
    length += EURD_CARRINFO_HEADER;
    //printf("*****simid:%s, phone:%s, channel:%s, network:%s, service:%s, rssi:%d\n",simid,phone,channel,network,service,rssi);
    return length;
}

static int fill_waninfo_package(char *buf)
{
    char *pos=buf+EURD_WANINFO_HEADER;
    uint16_t length = 0;
    int temp=0;
    struct in_addr t_ip;
    char tmp_buf[32];
    char tmp_buf2[32];
    FILE* f;
    char buffer_get[256];
    char *p;
    int j;
    uint32_t tmp32_hton;
    uint16_t tmp16_hton;
    /*
     * wan ip
     */

    strcpy(tmp_buf,"N/A");
    if (false==fetch_Local_IP_ADDR(ifname_wan,tmp_buf))fetch_Local_IP_ADDR(ifname_eth1,tmp_buf);
    if (inet_aton(tmp_buf,&t_ip)!=0) 
    {
        tmp32_hton=t_ip.s_addr;//htons_rv32(t_ip.s_addr);
        memcpy(pos,&(tmp32_hton),sizeof(unsigned long));
        //printf("\n**WANIP:%s  --%x,%x,%x,%x \n",tmp_buf,*pos,*(pos+1),*(pos+2),*(pos+3));
        temp = sizeof(unsigned long);
        length += temp;
        pos += temp;
    }


    /*
     * dns1 dns2
     */
    int trans_sign=0;
    if (!(f = fopen("/tmp/resolv.conf.auto", "r"))) {
        //do nothing.   resolv.conf.auto
    }else
    {
        while (fgets(buffer_get, 256, f)) 
        {
            p=NULL;
            p = strstr(buffer_get, "nameserver");
            if (p != NULL)
            {
                if(trans_sign==1)
                {

                    while (*p == ' ')p++; //first space
                    while (*p != ' ')p++; //nameserver
                    while (*p == ' ')p++; //first space
                    j=0;
                    while ((*p != ' ')&& (*p!='\r') && (*p!='\0') && (*p!='\n')&&(j<20)) 
                    {
                        tmp_buf2[j] = *p;
                        p++;
                        j++;
                     }                   
                     tmp_buf2[j]=0;
                     if(j>5) 
                     {
                         trans_sign++;
                     }
                }//if 1

               if(trans_sign==0) 
                {
                   while (*p == ' ')p++; //first space
                   while (*p != ' ')p++; //nameserver
                   while (*p == ' ')p++; //first space
                   j=0;
                   while ((*p != ' ')&& (*p!='\r') && (*p!='\0') &&(j<20)) 
                   {
                       tmp_buf[j] = *p;
                       p++;
                       j++;
                    }                   
                    tmp_buf[j]=0;
                    if(j>5) 
                    {
                        trans_sign++;
                    }
                }//if 0
                if (trans_sign>1) 
                {
                    break; 
                }
            }//if p
        }//while
    }//else
    if(f)fclose(f);

    if(trans_sign<1)
    {
        strcpy(tmp_buf,"N/A");
    }
    if(trans_sign<2)
    {
        strcpy(tmp_buf2,"N/A");
    }

    if (inet_aton(tmp_buf,&t_ip)!=0) 
    {
        tmp32_hton=t_ip.s_addr;//htons_rv32(t_ip.s_addr);
        memcpy(pos,&(tmp32_hton),sizeof(unsigned long));
        //printf("\n**DNS1:%s  --%x,%x,%x,%x \n",tmp_buf,*pos,*(pos+1),*(pos+2),*(pos+3));
        temp = sizeof(unsigned long);
        length += temp;
        pos += temp;
    }

    if (inet_aton(tmp_buf2,&t_ip)!=0) 
    {
        tmp32_hton=t_ip.s_addr;//htons_rv32(t_ip.s_addr);
        memcpy(pos,&(tmp32_hton),sizeof(unsigned long));
        temp = sizeof(unsigned long);
        length += temp;
        pos += temp;
    }


    /*
     * package length
     */
    tmp16_hton=htons_rv16(length);
    memcpy(buf,&tmp16_hton,EURD_WANINFO_HEADER);
    length += EURD_WANINFO_HEADER;

    return length;
}


static int fill_tailinfo_package(char *buf)
{
    char *pos=buf+EURD_TAILINFO_HEADER;
    uint16_t length = 0;
    int temp=0;
    struct in_addr t_ip;
    char tmp_buf[40];
    char module_rad, module_eth, module_carr, module_usb, module_com;
    //content length --- 2 BYTES(UINT16_T)
    //product name --- STRING(1\u201464 bytes)
    //image name --- STRING(1\u201464 bytes)
    //domain name --- STRING(1-64 bytes)
    //domain password --- STRING(32 bytes)	//MD5 encryption
    //module list --- 5 BYTES		//radio, ethernet, carrier, usb, com

    temp = strlen(eur_device_buff.productName)+1;
    memcpy(pos,eur_device_buff.productName,temp);
    length += temp;
    pos += temp;

    temp = strlen(eur_device_buff.imageName)+1;
    memcpy(pos,eur_device_buff.imageName,temp);
    length += temp;
    pos += temp;

    tmp_buf[0]=0;
    get_option_by_section_name(ctx, "eurd", "nms_conf", "domainname", tmp_buf);
    temp = strlen(tmp_buf)+1;
    memcpy(pos,tmp_buf,temp);
    length += temp;
    pos += temp;

    tmp_buf[0]=0;
    get_option_by_section_name(ctx, "eurd", "nms_conf", "domainpasswd", tmp_buf);
    temp = strlen(tmp_buf)+1;
    memcpy(pos,tmp_buf,temp);
    length += temp;
    pos += temp;


    module_rad=num_radio;
    module_eth=2;
    module_carr=1;
    module_usb=0;
    module_com=1;

    *pos=module_rad;
    pos++;
    *pos=module_eth;
    pos++;
    *pos=module_carr;
    pos++;
    *pos=module_usb;
    pos++;
    *pos=module_com;
    pos++;
    length += 5;


    // package length
    uint16_t tmp16_hton;
    tmp16_hton=htons_rv16(length);
    memcpy(buf,&tmp16_hton,EURD_TAILINFO_HEADER);
    length += EURD_TAILINFO_HEADER;
    return length;
}


static int fill_headinfo_package(char *buf)
{
    char *pos=buf;
    uint16_t tmp16_hton;
    uint8_t tmp8_hton;
    uint64_t tmp64_hton;
    char tmp[2];
//    uint16_t tmp64_hton;

/*
    a fixed header(fixed size 20 bytes)
  modem ID --- uint64_t(8 bytes)
  message type mask --- uint8_t(1 bytes)
  reserved
  mac --- 6 bytes
  version --- uint16_t(2 bytes)---1036
  packet length --- uint16_t(2 bytes)
  note: packet length = length of the fixed header + length of message playload
    uint64_t modem_id;
    uint16_t mess_type_mask;
    uint8_t mac[6];
    uint16_t product_flage;
    uint16_t pack_len;
    */
//    tmp64_hton=htonl(eur_buff.modem_id);
//    tmp64_hton=htons_rv64(eur_buff.modem_id);
    tmp64_hton=eur_buff.modem_id;
    memcpy(pos,&tmp64_hton,8);
    pos += 8;

    tmp8_hton=eur_buff.mess_type_mask;
    memcpy(pos,&tmp8_hton,1);
    pos +=1;

    tmp8_hton=0;
    memcpy(pos,&tmp8_hton,1);
    pos +=1;

    memcpy(pos,eur_buff.mac,6);
    pos +=6;

    tmp16_hton=htons_rv16(eur_buff.product_flage);
    memcpy(pos,&tmp16_hton,2);
    pos +=2;

    tmp16_hton=htons_rv16(eur_buff.pack_len);
    memcpy(pos,&tmp16_hton,2);
    pos +=2;


    // package length
    return MEP_PACKAGE_HEADER;
}


int main( int argc, char *argv[])
{


    switch (argc) {
    case 2:{
            AERC_REPORT_NUM = atoi(argv[1]);
            break;
    }
    default:
        syslog(LOGOPTS,"EURD exit: not argv for eurd_sms\n");
        exit(1);
    }

    if ((AERC_REPORT_NUM>3) || (AERC_REPORT_NUM<0)) {

                    syslog(LOGOPTS,"EURD exit: argv should be 0/1/2/3\n");
                    exit(1);
    }



    char send_buf[5120];
    char comp_buf[10240];
    int i;
    uint16_t tmp16_hton;

    for (i=0;i<MAX_RADIOS_NUMBER;i++)p_eur_radios_buff[i]=NULL; 
    for (i=0;i<MAX_ETHS_NUMBER;i++)p_eur_eths_buff[i]=NULL; 
    for (i=0;i<MAX_CARRS_NUMBER;i++)p_eur_carrs_buff[i]=NULL; 
    for (i=0;i<MAX_IO_NUMBER;i++)p_eur_io_buff[i]=NULL; 
    for (i=0;i<MAX_COMS_NUMBER;i++)p_eur_coms_buff[i]=NULL; 
    for (i=0;i<MAX_VPNCLS_NUMBER;i++)p_eur_vpncls_buff[i]=NULL; 
    bzero(&eur_device_buff,sizeof(struct _EUR_device_data_t));
    bzero(&eur_conf_buff,sizeof(struct _EUR_conf_data_t));
    bzero(set_eur_radios_valid,sizeof(set_eur_radios_valid));
    bzero(set_eur_eths_valid,sizeof(set_eur_eths_valid));
    bzero(set_eur_carrs_valid,sizeof(set_eur_carrs_valid));
    bzero(set_eur_io_valid,sizeof(set_eur_io_valid));
    bzero(set_eur_coms_valid,sizeof(set_eur_coms_valid));
    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        fprintf(stderr, "Out of memory\n");
        return -1;
    }

    read_udpconf();
    if (DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RR][AERC_REPORT_NUM]=='A') 
    {
        syslog(LOGOPTS,"EURD exit: exit because of disable report%d\n",AERC_REPORT_NUM);
        exit(1);
    }

    init_eurd();
    //refresh_buff();
    uci_free_context(ctx);


    int initeddone=0;
    //only get data once and send.
    
        int len=0, vpnnum=0, comflage=0;

        refresh_buff();

        ///// pack send data here:----Begin///// 
        bzero(send_buf,sizeof(send_buf));
        if (eur_buff.mess_type_mask == SDPINFO) 
        {
            len = fill_sdpinfo_package(send_buf);
            eur_buff.pack_len = len;
        } else if (eur_buff.mess_type_mask >= JSONINFO) 
        {
            struct mod_list mlist={0,0,0,0,0,""};

            eur_buff.pack_len = 0;

            len=fill_json_header(send_buf);
            eur_buff.pack_len += len;

            if (((eur_buff.mess_type_mask) & ETHINFO) == ETHINFO) {
                len=fill_json_eth_playload(send_buf+eur_buff.pack_len);
                eur_buff.pack_len += len;
                mlist.eth = 1+ EUR_SET_ETH1_ONOROFF;
            }
            if (((eur_buff.mess_type_mask) & CARINFO) == CARINFO) {
                len=fill_json_carrinfo_playload(send_buf+eur_buff.pack_len);
                eur_buff.pack_len += len;
                mlist.carrier = 1;
            }
            if (((eur_buff.mess_type_mask) & RADINFO) == RADINFO) {
                len=fill_json_radio_playload(send_buf+eur_buff.pack_len);
                eur_buff.pack_len += len;
                mlist.radio = 1;
            }
            if (((eur_buff.mess_type_mask) & COM0INFO) == COM0INFO) {
                len=fill_json_com_header(send_buf+eur_buff.pack_len);
                eur_buff.pack_len += len;
                len=fill_json_com1_playload(send_buf+eur_buff.pack_len);
                eur_buff.pack_len += len;
                len=fill_json_com_end(send_buf+eur_buff.pack_len);
                eur_buff.pack_len += len;
                mlist.com = 1;
            }
            if (((eur_buff.mess_type_mask) & IOINFO) == IOINFO) {
                len=fill_json_io_playload(send_buf+eur_buff.pack_len);
                eur_buff.pack_len += len;
                mlist.ioport = 1;
            }
            if (((eur_buff.mess_type_mask) & VPNINFO) == VPNINFO) {
                len=fill_json_vpn_playload(send_buf+eur_buff.pack_len, &mlist, AERC_REPORT_NUM);
                eur_buff.pack_len += len;

            }
            if (eur_buff.product_flage == EURD_MAS_CONF_CHANGED) {
                len=fill_json_conf_playload(send_buf+eur_buff.pack_len, &mlist,AERC_REPORT_NUM);
                eur_buff.pack_len += len;
                syslog(LOGOPTS,"EURDreport%d: fill conf playload len=%d\n",AERC_REPORT_NUM,len);
                //eur_buff[enabled_report[i]].product_flage = EURD_MAS_CONF_NONE;
            }

            len=fill_json_gps_playload(send_buf+eur_buff.pack_len, &mlist, AERC_REPORT_NUM);
            eur_buff.pack_len += len;

            len=fill_json_domain_playload(send_buf+eur_buff.pack_len, &mlist, AERC_REPORT_NUM);
            eur_buff.pack_len += len;

            len=fill_json_device_playload(send_buf+eur_buff.pack_len, &mlist, AERC_REPORT_NUM);
            eur_buff.pack_len += len;

        } else 
        {
            eur_buff.pack_len = 0;
            len = fill_headinfo_package(send_buf+eur_buff.pack_len);
            eur_buff.pack_len += len;

            //syslog(LOGOPTS,"%s report%d fillpackage mess_type_mask=%d\n",__func__,AERC_REPORT_NUM,eur_buff.mess_type_mask);
            if (((eur_buff.mess_type_mask) & MODEMINFO) == MODEMINFO) {
                len=fill_modeminfo_package(send_buf+eur_buff.pack_len);
                eur_buff.pack_len += len;

            }
            if (((eur_buff.mess_type_mask) & CARRINFO) == CARRINFO) {
                len = fill_carrinfo_package(send_buf+eur_buff.pack_len);
                eur_buff.pack_len += len;
            }
            if (((eur_buff.mess_type_mask) & WANINFO) == WANINFO) {
                len = fill_waninfo_package(send_buf+eur_buff.pack_len);
                eur_buff.pack_len += len;

            }
                len = fill_tailinfo_package(send_buf+eur_buff.pack_len);
                eur_buff.pack_len += len;
                if (eur_buff.pack_len)
                {
                    tmp16_hton=htons_rv16(eur_buff.pack_len);
                    memcpy(send_buf+MEP_PACKAGE_LENGTH,&(tmp16_hton),2);
                }
        }

        uci_free_context(ctx);
        ///// pack send data here:----End///// 


#if defined DEBUG_PRINT 
printf("\n \n \n Now begin send %d Bytes:\n",eur_buff.pack_len);
for(i=0; i<eur_buff.pack_len;i++) {
    printf("%c",send_buf[i]);
}
#endif

        /////send package data here:----Begin/////  
        int num = 0;
        if (eur_buff.mess_type_mask >= JSONINFO) 
        {
            int status_comp;
            uLongf len_comp = sizeof(comp_buf);
            status_comp = compress2(comp_buf, &len_comp, send_buf, eur_buff.pack_len,Z_DEFAULT_COMPRESSION);
            if (status_comp == Z_OK) 
            {
                num = sendto(eur_buff.sockfd, comp_buf, len_comp, 0, (struct sockaddr *)&(eur_buff.addr_remote), sizeof(struct sockaddr_in));
                eur_buff.product_flage = EURD_MAS_CONF_NONE;
            } else syslog(LOGOPTS,"EURDreport%d: Faild to compress report to %d:%d!\n",AERC_REPORT_NUM,len_comp, status_comp);
            memset(comp_buf,0,sizeof(comp_buf));
        } else 
        {
            num = sendto(eur_buff.sockfd, send_buf, eur_buff.pack_len, 0, (struct sockaddr *)&(eur_buff.addr_remote), sizeof(struct sockaddr_in));
        }
        if (num < 0 ) 
        {
            syslog(LOGOPTS,"EURDreport%d: Failed to send report to %s:%s!\n",AERC_REPORT_NUM,DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_REIP0+AERC_REPORT_NUM], DataBase1_Buff[ADVANCED_EUR_CONF][AERC_ITEM_RPORT0+AERC_REPORT_NUM]);
        }
        eur_buff.pack_len = 0;
        memset(send_buf,0,sizeof(send_buf));
        /////send package data here:----End/////  


#if defined DEBUG_PRINT 
printf("\n Now finished send %d Bytes.\n",num);
#endif




return 0;
}
