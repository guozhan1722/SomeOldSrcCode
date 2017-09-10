
#include "database.h"
#include <sys/wait.h>
#include <syslog.h>


char *Com1StatisticFile="/var/run/com1_packet";
char *Com2StatisticFile="/var/run/com2_packet";
char *COM2EnableFile="/etc/soipd2_runing";
char *USBStatisticFile="/var/run/usb_packet";
char *USBEnableFile="/etc/soipdusb_runing";
/*the following two files need to be changed based on real structure*/
char *UserDB="/etc/comport";
char *FlashUserDB="/etc/comport";/**/

#define ARG_NUMBER 16
char *args[ARG_NUMBER]; /*can not be static or will have trouble, and must be put here*/
static char cfgbuf[1024];  /*can not be static or will have trouble, and must be put here*/
char *ifname_bridge = "br-lan";

struct uci_context *ctx = NULL;


char DataBase1_Buff[MENUS_NUMBER][ITEM_MAX_NUMBER][BUFF_LEN]=
{
    {"B","A","H","A","A","100","100","B","0","1024","A","B","","F"},   /*  COM1 content  5 items      0 */
//#ifdef VIP4G
    {"A","A","N","A","A","100","100","B","0","1024","A","B","","F"},    /*  vip4g add 4 items in Com2 content           6 items  1*/    
//#else
//    {"A","N","A","B","0","1024","A","B","","F"},    /*  Com2 content           6 items  1*/
//#endif

    {"A"},    /* USB content  1 item     2*/

    {"0.0.0.0","20001","60"},     /* COM1 TCP CLIENT  4 items  3*/
    {"A","100","20001","300",},     /*  COM1 TCP SERVER          1 items   4*/
    {"0.0.0.0","20001","60","A","100","20001","300"},    /* c/s    5*/
    {"0.0.0.0","20001","20001"},    /*   COM1 UDP Point to Point   6 items  */
    {"224.1.1.1","20001","20011","1"},    /*  COM1 UDP Point to Multipoint as P   7 items  */

    {"0.0.0.0","20011","224.1.1.1","20001"},    /*   COM1 UDP Point to Multipoint as MP    8 items  */
    {"224.1.1.1","20011","1","224.1.1.1","20011"},    /* COM1 UDP multipoint to multipoint 9*/
    {"COM1 Message","0.0.0.0","host@","","","1024","10","A",},    /* COM1 SMTP  10*/
    {"B",  "A","A","A","192.168.2.2","27016","27015"},    /*c1222 11*/

    {"0.0.0.0","20002","60"},     /*   COM2 TCP CLIENT    4 items  12*/
    {"A","100","20002","300"},     /*   COM2 TCP SERVER     1 items  13 */ 
    {"0.0.0.0","20002","60","A","100","20002","300"},/* 14 */ 
    {"0.0.0.0","20002","20002"}, /* 14 */     /*  COM2 UDP Point to Point  3 items  15*/
    {"224.1.1.2","20002","20012","1"},    /*  COM2 UDP Point to Multipoint as point   16*/

    {"0.0.0.0","20012","224.1.1.2","20002"},    /*  COM2 UDP Point to Multipoint as multipoint 17 */
    {"224.1.1.2","20012","1","224.1.1.2","20012"},    /* COM2 multipoint to multipoint 18*/

    /* 70  USB TCP CLIENT and SERVER*/
    /* COM1 TCP CLIENT  4 items  */
    {"A","N","A","B","0","1024","A","B","","F"},    /*  USB 19*/
    {"A","100","20003","300",},     /*  COM1 TCP SERVER          1 items   20*/
    {"0.0.0.0","20003","60","A","100","20003","300"},
    {"0.0.0.0","20003","20003"},     /* 71 USB UDP Point to Point  3 items  22*/

    {"224.1.1.3","20003","20013","1"},    /* 72 USB UDP Point to Multipoint as point   23*/

    {"0.0.0.0","20013","224.1.1.3","20003"},    /* 73 USB UDP Point to Multipoint as multipoint  24*/

    {"224.1.1.3","20013","1","224.1.1.3","20013"},    /* 74 USB multipoint to multipoint 26*/

    {"0","0","928.425000","0","2"},
    {"A","A","1234","",""}, /* COM1 Modbus TCP Setting... 27*/
    {"A","A","1234","",""}, /* COM2 Modbus TCP Setting... 28*/
    {"A","A","1234","",""}, /* USB Modbus TCP Setting... 29*/
    {"COM2 Message","0.0.0.0","host@", "", "", "1024","10","A",},    /* COM2 SMTP 30 */           /* Add com2 smtp, menu number: 30 */
    {"","","","","","160","10","A","A","","","","","","","",""},    /*  COM1_SUB_MENU_SMS 31 */   /* Add com1 sms, menu number: 31 */     
    {"","","","","","160","10","A","A","","","","","","","",""},    /*  COM2_SUB_MENU_SMS 32 */   /* Add com2 sms, menu number: 32 */  
}; 


char Item_Keywords[MENUS_NUMBER][ITEM_MAX_NUMBER][40]=
{

    {"COM1_Port_Status","COM1_Chanel_Mode", "COM1_Data_Baud_Rate","COM1_Data_Format","COM1_Flow_Control",
        "COM1_Pre_Data_Delay", "COM1_Post_Data_Delay", "COM1_Data_Mode","COM1_Character_Timeout","COM1_Max_Packet_Len","COM1_QoS","COM1_NoConnect_Data_Intake",
        "", "COM1_IP_Protocol"},
    /* 3 COM1 Sub Menu   5    */

#ifdef VIP4G
    {"COM2_Port_Status","COM2_Chanel_Mode", "COM2_Data_Baud_Rate","COM2_Data_Format","COM2_Flow_Control",
        "COM2_Pre_Data_Delay", "COM2_Post_Data_Delay", "COM2_Data_Mode","COM2_Character_Timeout","COM2_Max_Packet_Len","COM2_QoS","COM2_NoConnect_Data_Intake",
        "", "COM2_IP_Protocol"},
#else
    {"COM2_Port_Status","COM2_Data_Baud_Rate","COM2_Data_Format",
        "COM2_Data_Mode","COM2_Character_Timeout","COM2_Max_Packet_Len","COM2_QoS","COM2_NoConnect_Data_Intake",
        "", "COM2_IP_Protocol"},
    /* 4 COM2 Sub Menu   6   */
#endif



    {"USB_Device_Mode"},    
    /* USB       */  

    {"COM1_T_Client_Server_Addr", "COM1_T_Client_Server_Port","COM1_T_Client_Timeout"},
    /*   TCP CLIENT     */
    {"COM1_T_Server_Polling_Mode","COM1_T_Server_Polling_Timeout","COM1_T_Server_Listen_Port","COM1_T_Server_Timeout"},   
    /*   TCP SERVER     */
    {"COM1_T_Client_Server_Addr", "COM1_T_Client_Server_Port", "COM1_T_Client_Timeout","COM1_T_Server_Polling_Mode","COM1_T_Server_Polling_Timeout", "COM1_T_Server_Listen_Port","COM1_T_Server_Timeout"},
    /*   TCP CLIENT/SERVER     */

    {"COM1_U_PtoP_Remote_Addr", "COM1_U_PtoP_Remote_Port","COM1_U_PtoP_Listen_Port"}, 
    /*  UDP Point to Point   */        

    {"COM1_UM_P_Multicast_Addr", "COM1_UM_P_Multicast_Port",
        "COM1_UM_P_Listen_Port","COM1_UM_P_TTL"},
    /*   UDP Point to Multipoint as p    */
    {"COM1_UM_M_Remote_Addr", "COM1_UM_M_Remote_Port" ,
        "COM1_UM_M_Multicast_Addr", "COM1_UM_M_Multicast_Port"},   
    /*   UDP Point to Multipoint as mp    */
    {"COM1_UMtoM_Multicast_Addr", "COM1_UMtoM_Multicast_Port","COM1_UMtoM_Multicast_TTL",
        "COM1_UMtoM_Listen_Multicast_Addr", "COM1_UMtoM_Listen_Multicast_Port"},   
    /*   UDP Multipoint to Multipoint  */   
    {"COM1_SMTP_Mail_Subject","COM1_SMTP_Server","COM1_SMTP_Username", "COM1_SMTP_Password", "COM1_SMTP_Recipient","COM1_SMTP_Buffer",
        "COM1_SMTP_Timeout","COM1_SMTP_Transfer_Mode"},
    /* COM1 SMTP AGENT */ 
    {"COM1_C1222_Reg_to_Net",  "COM1_C1222_Log_Net_Comm","COM1_C1222_Diff_Socket","COM1_C1222_Reassembly_Packet","COM1_C1222_Host_IP","COM1_C1222_Host_Port","COM1_C1222_Local_Port"},
    /*c1222*/
    {"COM2_T_Client_Server_Addr", "COM2_T_Client_Server_Port","COM2_T_Client_Timeout"},
    /*   TCP CLIENT      */
    {"COM2_T_Server_Polling_Mode","COM2_T_Server_Polling_Timeout","COM2_T_Server_Listen_Port","COM2_T_Server_Timeout"},
    /*  TCP SERVER    */
    {"COM2_T_Client_Server_Addr", "COM2_T_Client_Server_Port", "COM2_T_Client_Timeout","COM2_T_Server_Polling_Mode","COM2_T_Server_Polling_Timeout",
        "COM2_T_Server_Listen_Port","COM2_T_Server_Timeout",}, 
    /*"COM2_T_Server_Pending_Connect_Num"*/

    {"COM2_U_PtoP_Remote_Addr", "COM2_U_PtoP_Remote_Port","COM2_U_PtoP_Listen_Port"}, 
    /*   UDP Point to Point   */
    {"COM2_UM_P_Multicast_Addr", "COM2_UM_P_Multicast_Port",
        "COM2_UM_P_Listen_Port","COM2_UM_P_TTL"},
    /*   UDP Point to Multipoint as p      */
    {"COM2_UM_M_Remote_Addr", "COM2_UM_M_Remote_Port" ,
        "COM2_UM_M_Multicast_Addr", "COM2_UM_M_Multicast_Port"}, 
    /*   UDP Point to Multipoint as mp      */    
    {"COM2_UMtoM_Multicast_Addr", "COM2_UMtoM_Multicast_Port","COM2_UMtoM_Multicast_TTL",
        "COM2_UMtoM_Listen_Multicast_Addr", "COM2_UMtoM_Listen_Multicast_Port"},   
    /*   UDP Multipoint to Multipoint  */

    {"USB_Port_Status","USB_Data_Baud_Rate","USB_Data_Format",
        "USB_Data_Mode","USB_Character_Timeout","USB_Max_Packet_Len","USB_QoS","USB_NoConnect_Data_Intake",
        "", "USB_IP_Protocol"},
    /* 67 USB Sub Menu    */

    {"USB_T_Client_Server_Addr", "USB_T_Client_Server_Port","USB_T_Client_Timeout"},
    /* 68 USB TCP CLIENT      */

    {"USB_T_Server_Polling_Mode","USB_T_Server_Polling_Timeout","USB_T_Server_Listen_Port","USB_T_Server_Timeout"},
    /*  TCP SERVER    */
    {"USB_T_Client_Server_Addr", "USB_T_Client_Server_Port", "USB_T_Client_Timeout","USB_T_Server_Polling_Mode","USB_T_Server_Polling_Timeout",
        "USB_T_Server_Listen_Port","USB_T_Server_Timeout",}, 
    /*"COM2_T_Server_Pending_Connect_Num"*/
    /* 70 "USB_T_Server_Pending_Connect_Num"*/

    {"USB_U_PtoP_Remote_Addr", "USB_U_PtoP_Remote_Port","USB_U_PtoP_Listen_Port"}, 
    /* 71 USB UDP Point to Point   */

    {"USB_UM_P_Multicast_Addr", "USB_UM_P_Multicast_Port",
        "USB_UM_P_Listen_Port","USB_UM_P_TTL"},
    /* 72 USB  UDP Point to Multipoint as p      */

    {"USB_UM_M_Remote_Addr", "USB_UM_M_Remote_Port" ,
        "USB_UM_M_Multicast_Addr", "USB_UM_M_Multicast_Port"}, 
    /* 73 USB  UDP Point to Multipoint as mp      */    

    {"USB_UMtoM_Multicast_Addr", "USB_UMtoM_Multicast_Port","USB_UMtoM_Multicast_TTL",
        "USB_UMtoM_Listen_Multicast_Addr", "USB_UMtoM_Listen_Multicast_Port"},   
    /* 74 USB UDP Multipoint to Multipoint  */

    {"COM1_MODBUS_Mode","COM1_Modbus_Protect_Status", "COM1_Modbus_Protect_Key"},
    /* !!!!!!!!Please note COM1_Modbus_Mode should not be changed in order for backward compatibility !!!!!!!*/
    {"COM2_Modbus_Mode","COM2_Modbus_Protect_Status", "COM2_Modbus_Protect_Key"},
    /* !!!!!!!!Please note COM1_Modbus_Mode should not be changed in order for backward compatibility !!!!!!!*/
    {"USB_Modbus_Mode","USB_Modbus_Protect_Status", "USB_Modbus_Protect_Key"},
    /* !!!!!!!!Please note COM1_Modbus_Mode should not be changed in order for backward compatibility !!!!!!!*/
    {"COM2_SMTP_Mail_Subject","COM2_SMTP_Server","COM2_SMTP_Username", "COM2_SMTP_Password", "COM2_SMTP_Recipient","COM2_SMTP_Buffer",
        "COM2_SMTP_Timeout","COM2_SMTP_Transfer_Mode"},
    /* COM2 SMTP AGENT */ /* Add com2 smtp, menu number: 30 */
    {""},    /* reserved for COM1 SMS */ /* Add com1 sms , menu number: 31 */
    {"COM2_SMS_Phone_Number_1","COM2_SMS_Phone_Number_2","COM2_SMS_Phone_Number_3","COM2_SMS_Phone_Number_4","COM2_SMS_Phone_Number_5","COM2_SMS_Message_Max_Size","COM2_SMS_Reply_Timeout","COM2_SMS_Access_Control","COM2_SMS_Read_SMS_Control",\
        "From:","Subject:","Date/time:","COM2_SMS_LOCATION","SMS_SM_TOTAL","SMS_SM_USED","SMS_ME_TOTAL","SMS_ME_USED"},
    /*COM2_SUB_MENU_SMS*/  /* Add com2 sms, menu number: 32 */
} ;




/**********************************************************************************
   Function:
   Input: buffer1 key word, int read_buff_len, the length of read_value 
   Output: read_value content
   Return: true or false
   Note:
**********************************************************************************/

bool UserDB_read( char* UserDB, char* key_word, char* read_buff, int read_buff_len)
{
    //syslog(LOGOPTS,"Enter [%s]\n", __func__);

    if (key_word[0]=='C') {
        if (key_word[3]=='1') {
            return get_option_by_section_name(ctx, "comport", "com1", key_word, read_buff);
        } else {
            return get_option_by_section_name(ctx, "comport2", "com2", key_word, read_buff);
        }
    } else if (key_word[0]=='U') {
        return get_option_by_section_name(ctx, "comport", "usb", key_word, read_buff);
    } else {
        return false;
    }

    return true;
}



/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

/*  the length of buffer 1 and 2 is stable ,not the length of buffer content   */

bool UserDB_write(char *UserDB ,char* key_word,int key_word_len,char* write_value,int write_value_len)
{
    //syslog(LOGOPTS,"Enter [%s]\n", __func__);

    if (key_word[0]=='C') {
        if (key_word[3]=='1') {
            set_option_by_section_name(ctx, "comport", "com1", key_word, write_value);
        } else {
            set_option_by_section_name(ctx, "comport2", "com2", key_word, write_value);
        }
    } else if (key_word[0]=='U') {
        set_option_by_section_name(ctx, "comport", "usb", key_word, write_value);
    } else {
        return false;
    }

    return true;
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

    for (j=0;j<ITEM_MAX_NUMBER;j++) {
        //syslog(LOGOPTS, "strlen(Item_Keywords[%d][%d]) = %d\n",menu,j,strlen(Item_Keywords[menu][j]) );
        if (strlen(Item_Keywords[menu][j])>2) {
            if (true==UserDB_read(UserDB,Item_Keywords[menu][j],buffer,UserDB_Buff2_len)) {
                /*  read keywords and return to buffer  */
                //printf("%s menu=%d buff=%s\n",__func__,menu,buffer);
                //syslog(LOGOPTS,"%s DataBase1_Buff[%d][%d]=%s\n",__func__, menu, j, buffer);
                if (strlen(buffer)>=1) {
                    strcpy(DataBase1_Buff[menu][j],buffer); /*  copy to database buffer  */
                }
            } else {
                //syslog(LOGOPTS,"read_Database false menu=%d,Item=%d\n",menu,j);
                noerror=false;            
            }
        } else {
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
    for (j=0;j<ITEM_MAX_NUMBER;j++) {
        if (strlen(Item_Keywords[menu][j])>2) {
            UserDB_write(UserDB,Item_Keywords[menu][j],UserDB_Buff1_len, DataBase1_Buff[menu][j],UserDB_Buff2_len); 
        } else {
        }          
    }
    //SubProgram_Start(UIscriptFile,"cp_db1_to_rom");   
    return TRUE;      
}                  

/**********************************************************************************
   Function:  int SubProgram_Start(char *pszPathName, char *pszArguments)   
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

int SubProgram_Start(char *pszPathName, char *pszArguments)
{
    int  sub_status; 
    int MAX_ARGS=8;
    char * pArgs[MAX_ARGS];
    unsigned long ulNumArgs = 1;
    char tmpArgs[256];
    char *pArg = NULL;
    int pid = 0; 
    //syslog(LOGOPTS,"Enter [%s]\n", __func__);   

    pArgs[0]=pszPathName;
    if (pszArguments != NULL) {
        strcpy(tmpArgs, pszArguments);    
        /* 'execl' return -1 on error. */
        pArg = strtok(tmpArgs, " ");
        while ((pArg != NULL) && (ulNumArgs < MAX_ARGS)) {
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
    if (!(pid = vfork())) {
        seteuid((uid_t)0);
        /* We are the child! */            
        if (execvp(pszPathName, pArgs) == -1) {
            //syslog(LOGOPTS,"SubProgram_Start false\n"); 
            return false;
        }
        //syslog(LOGOPTS,"Child PID after = %d\n",pid);          
    } else {
        waitpid(pid,&sub_status,0);
        return true;        
    }    
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
    syslog(LOGOPTS,"Enter [%s]\n", __func__);

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if (-1==sock) {
        //syslog(LOGOPTS,"fetch_Local_IP_ADDR: socket=-1");  
        perror("fetch_Local_IP_ADDR ");
        strcpy(addr, temp); 
        return false;
    }
    strncpy(ifr.ifr_name,ifname,sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';   
    if (-1==ioctl(sock, SIOCGIFADDR, &ifr)) {
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



bool get_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
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

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK) {

        e = ptr.last;

        if ( (ptr.flags & UCI_LOOKUP_COMPLETE) &&
             (e->type == UCI_TYPE_OPTION) ) {
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



bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
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

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK) {
        if ( UCI_OK == uci_set(ctx, &ptr) ) {
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
                               END THANK YOU
**********************************************************************************/





