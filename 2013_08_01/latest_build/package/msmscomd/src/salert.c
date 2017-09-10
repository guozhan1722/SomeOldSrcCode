/* file : salert.c */
/* description : customized part of system health conditions to trigger the alert through short messages   */ 

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
#define LOG_WIDTH 1024

char RSSI_CHECK[5];
char ETH_CHECK[5];
char IO_CHECK[5];
char SAL_Enable[5];
int SAL_Interval;
char SAL_Phone[6][20];
int RSSI_LOW;
char ROAMING_CHECK[5];
char ROAMING_STATUS[5];
char ETH_LINK_STATUS[5];

static struct ifreq ifr;
static int debug;
size_t
sanitize_string(char *buf, size_t size)
{
#define UGLY_WIDTH	4	/* width for ugly character: \OOO */
    size_t len;
    size_t added = 0;
    char *p;


    /* find right side of string to be sanitized and count
     * number of columns to be added.  Stop on end of string
     * or lack of room for more result.
     */
    for (p = buf; *p != '\0' && &p[added] < &buf[size - UGLY_WIDTH]; p++) {
        unsigned char c = *p;

        /* exception is that all veritical space just becomes white space */
        if (c == '\n' || c == '\r') {
            *p = ' ';
            continue;
        }

        if (c == '\\' || !isprint(c))
            added += UGLY_WIDTH - 1;
    }

    /* at this point, p points after last original character to be
     * included.  added is how many characters are added to sanitize.
     * so p[added] will point after last sanitized character.
     */

    p[added] = '\0';
    len = &p[added] - buf;

    /* scan backwards, copying characters to their new home
     * and inserting the expansions for ugly characters.
     *
     * vertical space is changed to horizontal.
     *
     * It is finished when no more shifting is required.
     * This is a predecrement loop.
     */
    while (added != 0) {
        char fmtd[UGLY_WIDTH + 1];
        unsigned char c;

        while ((c = *--p) != '\\' && isprint(c))
            p[added] = c;

        added -= UGLY_WIDTH - 1;
        snprintf(fmtd, sizeof(fmtd), "\\%03o", c);
        memcpy(p + added, fmtd, UGLY_WIDTH);
    }
    return len;
#undef UGLY_WIDTH
}

/* log a debugging message (prefixed by "| ") */
int salert_DBG_log(const char *message, ...)
{
    if (debug==0) return 0;
    va_list args;
    char m[LOG_WIDTH];  /* longer messages will be truncated */

    va_start(args, message);
    vsnprintf(m, sizeof(m), message, args);
    va_end(args);

    /* then sanitize anything else that is left. */
    (void)sanitize_string(m, sizeof(m));

    syslog(LOG_DEBUG, "Salert| %s", m);

    return 0;
}


/*--------------------------------------------------------------------*/

static int mdio_read(int skfd, int location)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    mii->reg_num = location;
    if (ioctl(skfd, SIOCGMIIREG, &ifr) < 0) {
        syslog(LOGOPTS, "SIOCGMIIREG on %s failed\n", ifr.ifr_name);
        return -1;
    }
    return mii->val_out;
}

/*--------------------------------------------------------------------*/

static int read_bmsr(int skfd, char *ifname)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    int bmsr;

    /* Get the vitals from the interface. */
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGMIIPHY, &ifr) < 0) {
        syslog(LOGOPTS, "SIOCGMIIPHY on '%s' failed\n",ifname);
        return 1;
    }

    bmsr = mdio_read(skfd, MII_BMSR);

    return bmsr;
}

static int savesms_to_file(char *filename, char *str, int len)
{
    FILE* sms_fp;
    int wcount;


    if (sizeof(str)>160) {
        syslog(LOGOPTS,"savesms_to_file :the sms size is longer than 160 charactors");
        return false;
    }
    if (!(sms_fp = fopen(filename,"w+"))) {
        syslog(LOGOPTS,"savesms_to_file :can not open %s to write \n",filename); 
        return false;          
    }
    wcount=fwrite(str,sizeof(char),len,sms_fp);

    fclose(sms_fp);
    return true;

}


int main(int argc, char ** argv)
{
    int fd;
    time_t start, end;
    int sbuf_account=0;
    char sendbuf[160];
//    float voltage, voltage_high, voltage_low;
    int i,j,eth_old,roaming_old,roaming,rssi;
    FILE* fp;
    FILE* f;
    char *p;
    char tmp_buff[256];
    char rssi_buff[5];
    char input1_s_bak[5],input2_s_bak[5],input3_s_bak[5],input4_s_bak[5];
    char output1_s_bak[5],output2_s_bak[5],output3_s_bak[5],output4_s_bak[5];

    (void) setsid();                                  
    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        syslog(LOGOPTS,"Salertd EXIT:Out of memory\n");
        return -1;
    }

    SAL_Enable[0]=0;
    if(get_option_by_section_name(ctx, "salertd","salert_conf","SAL_Enable",SAL_Enable)==false)return -2;
    if(SAL_Enable[0]!='B')return 0;

    get_option_by_section_name(ctx, "salertd","salert_conf","RSSI_CHECK",RSSI_CHECK);
    get_option_by_section_name(ctx, "salertd","salert_conf","ETH_CHECK",ETH_CHECK);
    get_option_by_section_name(ctx, "salertd","salert_conf","IO_CHECK",IO_CHECK);
    get_option_by_section_name(ctx, "salertd","salert_conf","ROAMING_CHECK",ROAMING_CHECK);
    get_option_by_section_name(ctx, "salertd","salert_conf","ROAMING_STATUS",ROAMING_STATUS);
    get_option_by_section_name(ctx, "salertd","salert_conf","ETH_LINK_STATUS",ETH_LINK_STATUS);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "salertd","salert_conf","SAL_Interval",tmp_buff);
    SAL_Interval=atoi(tmp_buff);

    tmp_buff[0]=0;
    get_option_by_section_name(ctx, "salertd","salert_conf","RSSI_LOW",tmp_buff);
    RSSI_LOW=atoi(tmp_buff);

    for(i=0;i<6;i++)
    {
        SAL_Phone[i][0]=0;
        sprintf(tmp_buff,"SAL_Phone%d",(i+1));
        get_option_by_section_name(ctx, "salertd","salert_conf",tmp_buff,SAL_Phone[i]);
        if(strlen(SAL_Phone[i])<3)SAL_Phone[i][0]=0;
    }


    int interval;
    if(SAL_Interval<5)interval=5;
    else interval=SAL_Interval;

    eth_old=0;
    if ((fd = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        syslog(LOGOPTS,"socket faild\n");                
    } else {
        eth_old=read_bmsr(fd,"eth0");
        syslog(LOGOPTS,"init eth link=%d\n",eth_old); 
        close (fd);
    }


    input1_s_bak[0]=0;
    input2_s_bak[0]=0;
    input3_s_bak[0]=0;
    input4_s_bak[0]=0;
    output1_s_bak[0]=0;
    output2_s_bak[0]=0;
    output3_s_bak[0]=0;
    output4_s_bak[0]=0;
    get_option_by_section_name(ctx, "ioports", "input", "input1", input1_s_bak);
    get_option_by_section_name(ctx, "ioports", "input", "input2", input2_s_bak);
    get_option_by_section_name(ctx, "ioports", "input", "input3", input3_s_bak);
    get_option_by_section_name(ctx, "ioports", "input", "input4", input4_s_bak);
    get_option_by_section_name(ctx, "ioports", "output", "output1", output1_s_bak);
    get_option_by_section_name(ctx, "ioports", "output", "output2", output2_s_bak);
    get_option_by_section_name(ctx, "ioports", "output", "output3", output3_s_bak);
    get_option_by_section_name(ctx, "ioports", "output", "output4", output4_s_bak);


    start = time(NULL);
    memset(sendbuf, 0, sizeof(sendbuf));
    roaming_old=-9;

    while (1) {

        if(check_carrier_satus()!=1)
        {
            sleep(5);
            continue;
        }


        sprintf(sendbuf, "Alert!");
        
        //read system data
        roaming=-9;
        rssi=999;
        if (!(f = fopen("/var/run/VIP4G_status", "r"))) 
        {
            //do nothing.
        }else
        {
            while (fgets(tmp_buff, 256, f)) 
            {

                if(ROAMING_CHECK[0]=='B') 
                {
                    p=NULL;
                    p = strstr(tmp_buff, "roaming=");//roaming=" 0,0"
                    if (p != NULL)
                    {
                        while((*p != ',') && (*p!='\r') && (*p!='\0') && (*p!='\n'))p++;
                        if(*p == ',') 
                        {
                            p++;
                            while(*p == ' ')p++;
                            roaming=-1;
                            //if(*p=='0')roaming=0;
                            //strcpy(p_eur_carrs_buff[0]->homeRoaming,"Not Searching");
                            if(*p=='1')roaming=1;
                            //if(*p=='2')roaming=2;
                            //if(*p=='3')roaming=3;
                            //if(*p=='4')roaming=4;
                            if(*p=='5')roaming=5;
                        }
                    }//if p
                }

                if(RSSI_CHECK[0]=='B') 
                {
                    p=NULL;
                    p = strstr(tmp_buff, "rssi=");//rssi=" GSM RSSI : 72"
                    if (p != NULL)
                    {
                        while ((*p != ':') && (*p != '\n') && (*p!=0))p++; //key space
                        if (*p==':')
                        {
                            p++;
                            while (*p == ' ')p++; //first space
                            j=0;
                            while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<5)) 
                            {
                                j++;
                                rssi_buff[j]=*p;
                                p++;
                             }
                            if(j>0)
                            {
                                j++;
                                rssi_buff[j]=0;
                                rssi_buff[0]='-';
                                rssi=atoi(rssi_buff);
                            }
                        }//if :
                    }//if p
                }


            }//while
        }//else
        if(f)fclose(f);

        //RSSI check
        if(RSSI_CHECK[0]=='B' && rssi!=999 ) 
        {
            if(rssi<RSSI_LOW)
            {
                sprintf(tmp_buff,"\r\nRSSI=%d",rssi);
                strcat(sendbuf,tmp_buff);
            }

        }

        //core temperature

        //Roaming
        int check_ok=0;
        if(ROAMING_CHECK[0]=='B' && roaming!=-9 && roaming_old!=-9) 
        {
            //1:Home; 5:Roaming
            if(ROAMING_STATUS[0]=='B')
            {
                if(roaming!=roaming_old)
                {
                    check_ok=1;
                }
            }

            if(ROAMING_STATUS[0]=='C')
            {
                if(roaming==5)
                {
                    check_ok=1;
                }
            }

            if(ROAMING_STATUS[0]=='D')
            {
                if(roaming==5 || roaming!=roaming_old)
                {
                    check_ok=1;
                }
            }

            if(ROAMING_STATUS[0]=='E')
            {
                if(roaming==5 && roaming!=roaming_old)
                {
                    check_ok=1;
                }
            }

            if(check_ok==1)
            {
                if(roaming==1)
                {
                    strcat(sendbuf,"\r\nNetwork=Home");
                }
                if(roaming==5)
                {
                    strcat(sendbuf,"\r\nNetwork=Roaming");
                }
            }

            roaming_old=roaming;
        }

        //IO check
        if(IO_CHECK[0]=='B' || IO_CHECK[0]=='C' || IO_CHECK[0]=='D') 
        {
            uci_free_context(ctx);
            ;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                syslog(LOGOPTS,"Salertd EXIT:Out of memory\n");
                return -1;
            }
        }
        if(IO_CHECK[0]=='B' || IO_CHECK[0]=='D') 
        {
            tmp_buff[0]=0;
            get_option_by_section_name(ctx, "ioports", "input", "input1", tmp_buff);
            if(strlen(tmp_buff)>0 && tmp_buff[0]!=input1_s_bak[0])
            {
                strcpy(input1_s_bak,tmp_buff);
                strcat(sendbuf, "\r\nINPUT1=");
                strcat(sendbuf, tmp_buff);
            }
            tmp_buff[0]=0;
            get_option_by_section_name(ctx, "ioports", "input", "input2", tmp_buff);
            if(strlen(tmp_buff)>0 && tmp_buff[0]!=input2_s_bak[0])
            {
                strcpy(input2_s_bak,tmp_buff);
                strcat(sendbuf, "\r\nINPUT2=");
                strcat(sendbuf, tmp_buff);
            }
            tmp_buff[0]=0;
            get_option_by_section_name(ctx, "ioports", "input", "input3", tmp_buff);
            if(strlen(tmp_buff)>0 && tmp_buff[0]!=input3_s_bak[0])
            {
                strcpy(input3_s_bak,tmp_buff);
                strcat(sendbuf, "\r\nINPUT3=");
                strcat(sendbuf, tmp_buff);
            }
            tmp_buff[0]=0;
            get_option_by_section_name(ctx, "ioports", "input", "input4", tmp_buff);
            if(strlen(tmp_buff)>0 && tmp_buff[0]!=input4_s_bak[0])
            {
                strcpy(input4_s_bak,tmp_buff);
                strcat(sendbuf, "\r\nINPUT4=");
                strcat(sendbuf, tmp_buff);
            }
        }
        if(IO_CHECK[0]=='C' || IO_CHECK[0]=='D') 
        {
            tmp_buff[0]=0;
            get_option_by_section_name(ctx, "ioports", "output", "output1", tmp_buff);
            if(strlen(tmp_buff)>0 && tmp_buff[0]!=output1_s_bak[0])
            {
                strcpy(output1_s_bak,tmp_buff);
                strcat(sendbuf, "\r\nOUTPUT1=");
                strcat(sendbuf, tmp_buff);
            }
            tmp_buff[0]=0;
            get_option_by_section_name(ctx, "ioports", "output", "output2", tmp_buff);
            if(strlen(tmp_buff)>0 && tmp_buff[0]!=output2_s_bak[0])
            {
                strcpy(output2_s_bak,tmp_buff);
                strcat(sendbuf, "\r\nOUTPUT2=");
                strcat(sendbuf, tmp_buff);
            }
            tmp_buff[0]=0;
            get_option_by_section_name(ctx, "ioports", "output", "output3", tmp_buff);
            if(strlen(tmp_buff)>0 && tmp_buff[0]!=output3_s_bak[0])
            {
                strcpy(output3_s_bak,tmp_buff);
                strcat(sendbuf, "\r\nOUTPUT3=");
                strcat(sendbuf, tmp_buff);
            }
            tmp_buff[0]=0;
            get_option_by_section_name(ctx, "ioports", "output", "output4", tmp_buff);
            if(strlen(tmp_buff)>0 && tmp_buff[0]!=output4_s_bak[0])
            {
                strcpy(output4_s_bak,tmp_buff);
                strcat(sendbuf, "\r\nOUTPUT4=");
                strcat(sendbuf, tmp_buff);
            }
        }


        //eth0 link
        if(ETH_CHECK[0]=='B') 
        {
            //get info of eth
            //check B/C/D/E   strcmp(old/Roaming
            int skfd=-1;
            if ((skfd = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
                syslog(LOGOPTS,"socket faild\n");                
            } else {
                int link=read_bmsr(skfd,"eth0");
                switch (ETH_LINK_STATUS[0]) {
                case 'B':
                    if (link != eth_old)
                        strcat(sendbuf, (link & MII_BMSR_LINK_VALID) ? "\r\nEthernet:link ok" : "\r\nEthernet:no link");
                    break;
                case 'C':
                    if (!(link & MII_BMSR_LINK_VALID))
                        strcat(sendbuf, "\r\nEthernet:no link");
                    break;
                case 'D':
                    if (link != eth_old || (!(link & MII_BMSR_LINK_VALID)))
                        strcat(sendbuf, (link & MII_BMSR_LINK_VALID) ? "\r\nEthernet:link ok" : "\r\nEthernet:no link");
                    break;
                case 'E':
                    if (link != eth_old && (!(link & MII_BMSR_LINK_VALID)))
                        strcat(sendbuf, (link & MII_BMSR_LINK_VALID) ? "\r\nEthernet:link ok" : "\r\nEthernet:no link");
                    break;
                default:
                    break;
                }
                eth_old=link;

                close(skfd);
            }
        }

        if (strlen(sendbuf)<9) {
            salert_DBG_log("sendbuf=Alert! goto break_process\n");
            goto break_process;
        }

        strOut[0]=0;
        struct tm *ttime;
        time_t tm = time(0);
        ttime = localtime(&tm);
        sprintf(strOut, "<tr><td>system</td><td>%s</td><td>%s</td><td>",asctime(ttime),sendbuf);

        //send sms
        fd = open ( RadioBusyFlagFile, O_WRONLY);
        if (fd<0) {
            syslog(LOGOPTS,"%s:fd<0\n",__func__);
            strcat(strOut,"Failed to send.");
            goto read_diag_fail_process;
        }
        LockFile(fd); 
        businit();

        for (i=0;i<6;i++) 
        {
            if (SAL_Phone[i][0]!=0)
            {
                send_sms(SAL_Phone[i], sendbuf, strlen(sendbuf));
                strcat(strOut,SAL_Phone[i]);
                strcat(strOut,",");
            }
        }

        read_diag_fail_process:
        busterminate();
        UnLockFile(fd);

        strcat(strOut,"</td></tr>\r\n");
        sms_history_to_flash(strOut,SAlertHistoryFile);

        break_process:
        end = time(NULL);
        if (end-start<interval) sleep(interval+start-end);
        start=time(NULL);
    }

    return 0;
}
/* end of source code */

