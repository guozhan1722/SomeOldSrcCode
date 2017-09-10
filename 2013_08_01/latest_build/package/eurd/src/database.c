
#include "database.h"
#include <sys/wait.h>
#include <syslog.h>



char *EURDStatisticFile="/var/run/eurd_packet";  //currently no use.
char *ConfigDir="/etc/config/";    //currently no use, by uci
char *EURDConfigFileTemp="udprpt.conf";  //only use for once and then delete it by web administrator to test.
char *EURDConfigFile="eurd";      //currently no use, by uci
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



//to avoid too much memery space. UserDB[][][] index the name in UserDB_Name[]
char UserDB_Name[USERDB_NAME_NUMBER][10]=
{
"NULL","eurd","eur_conf","network","lan",
"wan","wan2","","","",
};

//it is need to define in each function to fit for different config file with uci.      
unsigned int UserDB[MENUS_NUMBER][ITEM_MAX_NUMBER][2]=
{
//[0]packagename index +[1]sectionname index("com1" or "@port[0]")  keywords=optionname

/*ADVANCED_EUR_CONF*/  
{ 
  {1,2},{1,2},{1,2},{1,2},{1,2},
  {1,2},{1,2},{1,2},{1,2},{1,2},
  {1,2},{1,2},{1,2},{1,2},{1,2},
  {1,2},{1,2},{1,2},{1,2},{1,2}
},

};
char DataBase1_Buff[MENUS_NUMBER][ITEM_MAX_NUMBER][BUFF_LEN]=
{
///////Other Data Buf define depends from eurd_single.c
//////Data initialization manner is different from 3G.

    /*ADVANCED_EUR_CONF*/  
    {"AAAA","0.0.0.0","0.0.0.0","0.0.0.0","0.0.0.0","0","0","0","0","0","0","0","0","AAAA","AAAA","AAAA","","","",""},

};



char Item_Keywords[MENUS_NUMBER][ITEM_MAX_NUMBER][40]=
{
//the consequence need same to DataBase1_Buff[],just the name of each Byte.  Sorted by 5 words each line.

    /*     * ADVANCED_EUR_CONF     */
    {"Event_Remote_Reporting_Status","Event_Remote_IP_address0","Event_Remote_IP_address1","Event_Remote_IP_address2","Event_Remote_IP_address3",\
     "Event_Remote_PORT0","Event_Remote_PORT1","Event_Remote_PORT2","Event_Remote_PORT3","Event_Timer0",\
     "Event_Timer1","Event_Timer2","Event_Timer3","Event_Message_type0","Event_Message_type1",\
     "Event_Message_type2","Event_Management0","Event_Management1","Event_Management2","Event_Management3"
    },

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
   Function:
   Input: buffer1 key word, int read_buff_len, the length of read_value 
   Output: read_value content
   Return: true or false
   Note:
**********************************************************************************/
///////Different key words need different data source; not same to 3G; may need read data from uci connection.
//if ADVANCED_EUR_CONF then can from EURDConfigFile/Temp 
//get_option_by_section_name(ctx, "comport", "com1",key_word, read_buff);  ----redefine
bool UserDB_read(unsigned int *UserDB , char* key_word, char* read_buff, int read_buff_len)
{
//////currently read_buff_len is not checked. Be carefully.
//////maybe need to be cased by key_word: need a list for different packages to read more data.

    //syslog(LOGOPTS,"Enter [%s]\n", __func__);
    
      if(UserDB[0]==0 || UserDB[1]==0)
     	{
	return false;
	}
      if( UserDB[0]>USERDB_NAME_NUMBER || UserDB[1]>USERDB_NAME_NUMBER )
     	{
	return false;
	}
      

      get_option_by_section_name(ctx, UserDB_Name[UserDB[0]], UserDB_Name[UserDB[1]], key_word, read_buff);


    return true;


}

//set_option_by_section_name(ctx, "comport2", "com2", key_word, write_value);   -----redefine
bool UserDB_write(unsigned int *UserDB ,char* key_word,int key_word_len,char* write_value,int write_value_len)
{
   //syslog(LOGOPTS,"Enter [%s]\n", __func__);

      if(UserDB[0]==0 || UserDB[1]==0)
     	{
	return false;
	}
      if( UserDB[0]>USERDB_NAME_NUMBER || UserDB[1]>USERDB_NAME_NUMBER )
     	{
	return false;
	}
      
      set_option_by_section_name(ctx, UserDB_Name[UserDB[0]], UserDB_Name[UserDB[1]], key_word, write_value);


    return true;

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
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/
bool read_Database(int menu)
{
    int j;
    char buffer[32];
    bool noerror=true;
    //syslog(LOGOPTS,"Enter [%s]\n", __func__);

    for (j=0;j<ITEM_MAX_NUMBER;j++)
        {
        //syslog(LOGOPTS, "strlen(Item_Keywords[%d][%d]) = %d\n",menu,j,strlen(Item_Keywords[menu][j]) );
        if (strlen(Item_Keywords[menu][j])>1)
            {
            if (true==UserDB_read(UserDB[menu][j],Item_Keywords[menu][j],buffer,UserDB_Buff2_len))
                {
                /*  read keywords and return to buffer  */
                //printf("%s menu=%d buff=%s\n",__func__,menu,buffer);
                //syslog(LOGOPTS,"%s DataBase1_Buff[%d][%d]=%s\n",__func__, menu, j, buffer);
                strcpy(DataBase1_Buff[menu][j],buffer); /*  copy to database buffer  */
                }
            else
                {
                //syslog(LOGOPTS,"read_Database false menu=%d,Item=%d\n",menu,j);
                noerror=false;            
                }
            }
        else
            {
            }
        }
    return noerror;
}   

/**********************************************************************************
   Function: bool write_Database(int menu)
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/  
bool write_Database(int menu)
{
    //syslog(LOGOPTS,"Enter [%s]\n", __func__);
    int j;
    for (j=0;j<ITEM_MAX_NUMBER;j++)
        {
        if (strlen(Item_Keywords[menu][j])>2)
            {
            UserDB_write(UserDB[menu][j],Item_Keywords[menu][j],UserDB_Buff1_len, DataBase1_Buff[menu][j],UserDB_Buff2_len); 
            }
        else
            {
            }          
        }
    //SubProgram_Start(UIscriptFile,"cp_db1_to_rom");   
    return TRUE;      
}                  


/************
//////////Following function may be used, but need to confirm them
*********/


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
    char temp[32]="N/A";

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



int sendLocationRequest(unsigned int MNC,unsigned int MCC, unsigned int CID, unsigned int LAC)
{
    int ifp=-1;
    char pd[55];

    memset(pd,0x00,sizeof(pd));

    pd[1]=0x0e;

    if (CID > 65536) /* GSM: 4 hex digits, UTMS: 6 hex digits */
        pd[0x1c] = 5;
    else
        pd[0x1c] = 3;

    pd[0x10] = 0x1b;
    pd[0x11] = (MNC >> 24) & 0xFF;
    pd[0x12] = ((MNC >> 16) & 0xFF);
    pd[0x13] = (char)((MNC >> 8) & 0xFF);
    pd[0x14] = (char)((MNC >> 0) & 0xFF);

    pd[0x15] = (char)((MCC >> 24) & 0xFF);
    pd[0x16] = (char)((MCC >> 16) & 0xFF);
    pd[0x17] = (char)((MCC >> 8) & 0xFF);
    pd[0x18] = (char)((MCC >> 0) & 0xFF);

    pd[0x1f] = (char)((CID >> 24) & 0xFF);
    pd[0x20] = (char)((CID >> 16) & 0xFF);
    pd[0x21] = (char)((CID >> 8) & 0xFF);          
    pd[0x22] = (char)((CID >> 0) & 0xFF);

    pd[0x23] = (char)((LAC >> 24) & 0xFF);
    pd[0x24] = (char)((LAC >> 16) & 0xFF);
    pd[0x25] = (char)((LAC >> 8) & 0xFF);
    pd[0x26] = (char)((LAC >> 0) & 0xFF);

    pd[0x27] = (char)((MNC >> 24) & 0xFF);
    pd[0x28] = (char)((MNC >> 16) & 0xFF);
    pd[0x29] = (char)((MNC >> 8) & 0xFF);
    pd[0x2a] = (char)((MNC >> 0) & 0xFF);

    pd[0x2b] = (char)((MCC >> 24) & 0xFF);
    pd[0x2c] = (char)((MCC >> 16) & 0xFF);
    pd[0x2d] = (char)((MCC >> 8) & 0xFF);
    pd[0x2e] = (char)((MCC >> 0) & 0xFF);

    pd[0x2f] =0xFF;
    pd[0x30] =0xFF;
    pd[0x31] =0xFF;
    pd[0x32] =0xFF;

    ifp = open("/var/run/wget.location", O_CREAT|O_RDWR,S_IRWXU|S_IRWXO|S_IRWXG);
    if (ifp<=0) {
        return false;
    }
    write(ifp, pd, 55);
    close(ifp);

    if (false==SubProgram_Start("/bin/eurdscript","wget_loc"))return false;

    return true;
}

