
#include "database.h"
#include <sys/wait.h>
#include <syslog.h>


char *ConfigDir="/etc/config/";    //currently no use, by uci
char *VersionFile = "/etc/version";
char *BuilttimeFile = "/etc/banner";

char *dnsdefFile = "/tmp/resolv.conf";
char *carrstateFile = "/var/run/VIP4G_status";

char *ifname_bridge = "br-lan";
char *ifname_eth0 = "eth0";
char *ifname_wan = "br-wan";
char *ifname_eth1 = "eth1";
char *ifname_carrier = "br-wan2";
char *ifname_eth2 = "eth2";
char *ifname_radio = "wlan0";
char *ifname_radio_mon = "mon.wlan0";

#define ARG_NUMBER 16
char *args[ARG_NUMBER]; /*can not be static or will have trouble, and must be put here*/
static char cfgbuf[1024];  /*can not be static or will have trouble, and must be put here*/




//#define ARG_NUMBER 16
//char *args[ARG_NUMBER]; /*can not be static or will have trouble, and must be put here*/
//static char cfgbuf[1024];  /*can not be static or will have trouble, and must be put here*/

struct uci_context *ctx = NULL;   //need to initialize in main()



char DataBase2_Buff[1][1][BUFF_LEN]=
{    //SUB_MENU_SYSTEM_CFG
    {"DeviceName"}
};

char DataBase1_Buff[MENUS_NUMBER][ITEM_MAX_NUMBER][BUFF_LEN]=
{
///////Other Data Buf define depends from eurd_single.c
//////Data initialization manner is different from 3G.

    /*ADVANCED_GPS_CONF_REPORT added by Larry at 2011-05-06*/
    {"AAAA","0.0.0.0","0.0.0.0","0.0.0.0","0.0.0.0","0","0","0","0","AAAA","0","0","0","0","AAAA","0","0","0","0","AAAA","AAAA","AAAA","AAAA","AAAA","A","A","A","A"},
    /*GPS_SUB_MENU_SMTP*/
    {"Report0 Message","smtp.gmail.com:465","mailer.serial@gmail.com","SerialPort","host@","1024","Report1 Message","smtp.gmail.com:465","mailer.serial@gmail.com","SerialPort","host@","1024","Report2 Message","smtp.gmail.com:465","mailer.serial@gmail.com","SerialPort","host@","1024","Report3 Message","smtp.gmail.com:465","mailer.serial@gmail.com","SerialPort","host@","1024"},

};


char Item_Keywords[MENUS_NUMBER][ITEM_MAX_NUMBER][40]=
{
//the consequence need same to DataBase1_Buff[],just the name of each Byte.  Sorted by 5 words each line.

    /*     * GPS_CONF     */
    {"AGCR_Remote_Reporting_Status","AGCR_Remote_IP_address0","AGCR_Remote_IP_address1","AGCR_Remote_IP_address2","AGCR_Remote_IP_address3",\
    	"AGCR_Remote_PORT0","AGCR_Remote_PORT1","AGCR_Remote_PORT2","AGCR_Remote_PORT3","AGCR_Timer_trigger","AGCR_Timer0","AGCR_Timer1","AGCR_Timer2",\
    	"AGCR_Timer3","AGCR_Distance_trigger","AGCR_Distance0","AGCR_Distance1","AGCR_Distance2","AGCR_Distance3","AGCR_Trigger_condition",\
    	"AGCR_Message_type0","AGCR_Message_type1","AGCR_Message_type2","AGCR_Message_type3","AGCR_Local_Stream_Set0","AGCR_Local_Stream_Set1","AGCR_Local_Stream_Set2","AGCR_Local_Stream_Set3"
    },
    /*GPS_SUB_MENU_SMTP*/ 
    {"SMTP0_Mail_Subject","SMTP0_Server","SMTP0_User_Name","SMTP0_Password","SMTP0_Recipient","SMTP0_Buffer",
        "SMTP1_Mail_Subject","SMTP1_Server","SMTP1_User_Name","SMTP1_Password","SMTP1_Recipient","SMTP1_Buffer",
        "SMTP2_Mail_Subject","SMTP2_Server","SMTP2_User_Name","SMTP2_Password","SMTP2_Recipient","SMTP2_Buffer",
        "SMTP3_Mail_Subject","SMTP3_Server","SMTP3_User_Name","SMTP3_Password","SMTP3_Recipient","SMTP3_Buffer"},
};


/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/      
static char * ws(char **buf)
{
    char *b = *buf;
    char *p;

    /* eat ws */
    while (*b &&  (*b == ' ' || *b == '\n' || *b == '\r' ||  *b == '\t'))
        b++;

    p = b;

    /* find the end */
    while (*p &&  !(*p == ' ' || *p == '\n' || *b == '\r' || *p == '\t'))
        p++;

    *p = 0;
    *buf = p+1;    /* buf is pointer pointer of string end   */
    return b;       /*  return real string head pointer     */
}

/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

static char ** cfgread(FILE *fp)
{
    char *ebuf;
    char *p;
    int i;

    int str_len;
    /*char *args[8]; dont use it here, it will let copy right read wrong because it is first,
    but seems others is ok*/
    if (!fp)
        {
        return(void *)0;
        }

    while (fgets(cfgbuf, sizeof(cfgbuf), fp))
        {/* loop until find not comment string  */

        /* ship comment lines */
        if (cfgbuf[0] == '#') continue;
        /*remove the end '\n' '\r' of the string.*/
        str_len = strlen(cfgbuf);
        for (i=0;i<str_len;i++)
            {
            if ((cfgbuf[i]=='\n')||(cfgbuf[i]=='\r'))
                {
                cfgbuf[i]=' ';
                }
            }

        /*it is ok when replace to space*/
        ebuf = cfgbuf + str_len;

        p = cfgbuf;
        for (i = 0; i <ARG_NUMBER && p < ebuf; i++)
            {
            args[i] = ws(&p);
            }              

        args[i] = (void *)0;

        /* return if we found something */
        if (strlen(args[0])>0) return args; /*  string pointer pointer  */
        }
    return(void *)0;
}

/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

static char ** cfgfind(FILE *fp, char *var)
{
    char **ret;

    if (!fp)
        {
        //  errno = EIO;
        return(void *)0;
        }

    fseek(fp, 0, SEEK_SET);
    while ((ret = cfgread(fp)))
        {
        /* loop to all string  until find string   */
        if (strlen(var)<=strlen(ret[0]))
            {
            if (!strncmp(ret[0], var, strlen(var)))
                return ret;   /*  return string pointer pointer  */
            }
        else
            {
            if (!strncmp(ret[0], var, strlen(ret[0])))
                if (strlen(ret[1]))
                    if (!strncmp(ret[1], &var[strlen(ret[0])], strlen(ret[1])))
                        return ret;   /*  return string pointer pointer   */
            }
        }
    return(void *)0;
}



/**********************************************************************************
   Function: same to 3G version.
   Input: buffer1 key word, int read_buff_len, the length of read_value 
   Output: read_value content
   Return: ture or false
   Note:
**********************************************************************************/

bool UserDB_read_file(char *UserDB , char* key_word, char* read_buff, int read_buff_len)
{
    FILE* UserDB_fp;
    char **args;   
    int i,j=0;     
    int m=0;

    if (strlen(key_word)<2)
        {
        //syslog(LOGOPTS,"UserDB_read :can not find %s to read,too short \n",key_word);
        return false;
        }
    i = access ( UserDB, F_OK );      
    if ( i == 0 )
        {/*  there is database  */
        if (!(UserDB_fp = fopen(UserDB,"r+")))
            {
            perror("UserDB_read can't read ");
            return false;
            }
        }

    else
        {               /*  no database creat one  */
        if (!(UserDB_fp = fopen(UserDB,"w+")))
            {
            syslog(LOGOPTS, "UserDB_read can't write: %s", UserDB);
            perror("UserDB_read can't write ");
            return false;
            }
        }         

    if (!(args = cfgfind(UserDB_fp, key_word)))
        { /*  can not find this key word   */

        /* syslog(LOGOPTS,"UserDB_read :can not find %s to read \n",key_word);*/
        if(UserDB_fp)fclose(UserDB_fp);              
        return false;             
        }
    else  /*find*/
        {
        m= strlen(args[0])-strlen(key_word);
        if (m>0)
            {/*  the get value is lardger than key word  */
            if (m<read_buff_len)
                {
                for (i=0;i<m;i++)
                    {
                    read_buff[i]=args[0][i+strlen(key_word)];
                    /*  get the thing after key word  before space  */
                    }
                }
            else
                {
                syslog(LOGOPTS,"UserDB_read :%s ? too long \n",key_word);  
                if(UserDB_fp)fclose(UserDB_fp);              
                return false;
                }
            read_buff[m]= ' ';

            for (i=1;(i<ARG_NUMBER) && (m<read_buff_len &&args[i]) ;i++)
                {    /*  get others in the string after blackspace  */

                if (strlen(args[i])>0)
                    {
                    for (j=0;j<strlen(args[i]);j++)
                        {
                        if (m+1+j<read_buff_len)
                            read_buff[m+1+j]=args[i][j];
                        else
                            {
                            syslog(LOGOPTS,"UserDB_read :%s? too long \n",key_word);  
                            if(UserDB_fp)fclose(UserDB_fp);                              
                            return false;
                            }                      
                        }
                    read_buff[m+1+j]= ' ';
                    m+=strlen(args[i])+1;
                    }
                }
            read_buff[m]='\0';       /*  end of string   */
            }
        /*  the get arg[0] is equal key word  */
        else if (m==0)
            {/*  the get arg[0] is equal key word  */

            for (i=1;(i<ARG_NUMBER) && (m<read_buff_len &&args[i]);i++)
                {               /*  get others in the string after blackspace  */
                if (strlen(args[i])>0)
                    {
                    for (j=0;j<strlen(args[i]);j++)
                        {
                        if (m+j<read_buff_len)
                            read_buff[m+j]=args[i][j];
                        else
                            {
                            syslog(LOGOPTS,"UserDB_read :%s? too long \n",key_word);   
                            if(UserDB_fp)fclose(UserDB_fp);                        
                            return false;
                            }                      
                        }
                    read_buff[m+j]= ' ';
                    m+=strlen(args[i])+1;
                    }

                }
            read_buff[m]='\0';       /*  end of string   */
            }

        else /*m<0, there are two keywords*/
            {
            /*  there is two key words   */
            m=0;
            for (i=2;i<ARG_NUMBER && m<read_buff_len &&args[i];i++)
                {              /*  get others in the string after blackspace  */
                if (strlen(args[i]))
                    {
                    for (j=0;j<strlen(args[i]);j++)
                        {
                        if (m+j<read_buff_len)
                            read_buff[m+j]=args[i][j]; /*    */
                        else
                            {
                            syslog(LOGOPTS,"UserDB_read :%s? too long \n",key_word);   
                            if(UserDB_fp)fclose(UserDB_fp);                                 
                            return false;
                            }       
                        }
                    j++;
                    read_buff[m+j]= ' ';
                    m+=strlen(args[i])+1;                
                    }
                }            
            read_buff[m]='\0';  /*  end of string   */
            } 
        /*   get the read value to buffer_read   this is not very good.  */
        if(UserDB_fp)fclose(UserDB_fp);
        return true;
        } 
}


/**********************************************************************************
   Function:  int SubProgram_Start(char *pszPathName, char *pszArguments)   
   Input:
   Output:
   Return:
   Note: none changed.
**********************************************************************************/
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

/**********************************************************************************
   Function:fetch_Local_IP_MAC()
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/
bool fetch_Local_IP_MAC(char *ifname, char *mac)
{
    struct ifreq ifr;
    int sock, j, k;
    char temp[32]="00:00:00:00:00:00";

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if (-1==sock)
    {
        syslog(LOGOPTS,"fetch_Local_IP_MAC:%s socket() ",ifname);      
        perror("socket() ");
        strcpy(mac, temp);        
        return false;
    }
    strncpy(ifr.ifr_name,ifname,sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';

    if (-1==ioctl(sock, SIOCGIFHWADDR, &ifr))
    {
        syslog(LOGOPTS,"fetch_Local_IP_MAC:%s ioctl(SIOCGIFHWADDR)",ifname);         
        //perror("ioctl(SIOCGIFHWADDR) ");
        strcpy(mac, temp);    
        close(sock);
        return false;
    }

    for (j=0, k=0; j<6; j++)
    {
        k+=snprintf(temp+k, sizeof(temp)-k-1, j ? ":%02X" : "%02X",
                    (int)(unsigned int)(unsigned char)ifr.ifr_hwaddr.sa_data[j]);
    }
    temp[sizeof(temp)-1]='\0';
    strcpy(mac, temp);          
    close(sock);  
    return true;
}

/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/
int fetch_Local_IP_ADDR(char *ifname, char *addr)
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


/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/
int fetch_Local_IP_MASK(char *ifname, char *mask)
{
    struct ifreq ifr;
    int sock;
    char *p;
    char temp[32]="0.0.0.0";

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if (-1==sock)
    {
        perror("socket() ");
        strcpy(mask, temp);       
        return false;
    }
    strncpy(ifr.ifr_name,ifname,sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';   
    if (-1==ioctl(sock, SIOCGIFNETMASK, &ifr))
    {
        //   perror("ioctl(SIOCGIFNETMASK) ");
        strcpy(mask, temp); 
        close(sock);
        return false;
    }
    p=inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_netmask))->sin_addr);
    strncpy(temp,p,sizeof(temp)-1);
    temp[sizeof(temp)-1]='\0';
    strcpy(mask, temp);    
    close(sock);       
    return true;
}


int find_pid_by_name( char* ProcName, char* cmdstr)  
{  
        DIR             *dir;  
        struct dirent   *d;  
        int             pid, i;  
        char            *s;  
        int pnlen;  
  
        i = 0;  
        pnlen = strlen(ProcName);  
  
        /* Open the /proc directory. */  
        dir = opendir("/proc");  
        if (!dir)  
        {  
                printf("cannot open /proc");  
                return -1;  
        }  
  
        /* Walk through the directory. */  
        while ((d = readdir(dir)) != NULL) {  
  
                char exe [PATH_MAX+1];  
                char path[PATH_MAX+1];  
                int len;  
                int namelen;  
  
                /* See if this is a process */  
                if ((pid = atoi(d->d_name)) == 0)       continue;  
  
                snprintf(exe, sizeof(exe), "/proc/%s/exe", d->d_name);  
                if ((len = readlink(exe, path, PATH_MAX)) < 0)  
                        continue;  
                path[len] = '\0';  
  
                /* Find ProcName */  
                s = strrchr(path, '/');  
                if(s == NULL) continue;  
                s++;  
  
                /* we don't need small name len */  
                namelen = strlen(s);  
                if(namelen < pnlen)     continue;  
  
                if(!strncmp(ProcName, s, pnlen)) {  
                        /* to avoid subname like search proc tao but proc taolinke matched */  
                        if(s[pnlen] == ' ' || s[pnlen] == '\0') 
                        {

                            int cmdstrlen=strlen(cmdstr);
                            if(cmdstrlen<1){i++;break;}
                            char tmp_buff[100];
                            FILE* f=NULL;
                            char *p;
                            sprintf(tmp_buff,"/proc/%s/cmdline", d->d_name);
                            //printf("****find 1:%s\n",tmp_buff);
                            f = fopen(tmp_buff, "r");
                            if(f!=NULL)
                            {
                                tmp_buff[99]=0;
                                cmdstrlen=fread(tmp_buff,1,99,f);
                                fclose(f);
                                if(cmdstrlen>0)
                                {
                                    //printf("****find 2:%d:%s\n",cmdstrlen,tmp_buff);
                                    p=tmp_buff+cmdstrlen-1;
                                    while(p>=tmp_buff)
                                    {
                                         if(*p==*cmdstr)
                                         {
                                             if(strstr(p,cmdstr)!=NULL)
                                             {
                                                 //printf("****find 3:%s\n",p);
                                                 i++;
                                                 break;
                                             }
                                         }
                                        p--;
                                    }
                                }
                            }
                        }  
                }  
        }  
  
        closedir(dir);  
        return  i;  
}  

int gpsd_pid_check()
{
    if(find_pid_by_name("gpsd","dev")>0)return 0;
    SubProgram_Start("/etc/init.d/gpsd","stop");
    SubProgram_Start("/etc/init.d/gpsd","start");
    return 0;
}


