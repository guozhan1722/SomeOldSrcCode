#include "atcmdcli.h"


#define FALSE 0 
#define TRUE 1
#define	LOGOPTS		(LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)
#define SIGNED_INT_MAX 65535
#define CLI_SMS_PRO			1

#ifdef MEMWATCH
    #include "memwatch.h"
#endif

static struct gn_statemachine *state;
static gn_data *data;
char *RadioBusyFlagFile="/var/run/VIP4G_STAT_busy";
char *ifname_bridge = "br-lan";
char *ifname_eth0 = "eth0";
char *ifname_wan = "br-wan";
char *ifname_eth1 = "eth1";

int cmd_echo_ok(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{

    if (argc==0) {
        cli_print(cli,"OK");
        return CLI_OK;
    }

    int len=strlen(argv[argc-1]);
    char sign=argv[argc-1][len-1];
    if ( '?'== sign ) {
        cli_print(cli, "\n: Command Syntax:AT\n");
        cli_print(cli, "OK\n");
        return CLI_OK;
    }

    cli_print(cli,"Invalid parameters\n");
    return CLI_OK;
}

int cmd_at_test(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
/*
VIP2> AT+TEST=a,b c , d, e , f

AT ECHO TEST:
:6
:=
:a
:b c 
:d
:e 
:f

*/

    int i=0;
    cli_print(cli,"\nAT ECHO TEST:\n");
    cli_print(cli,":%d\n",argc);
    while(i<argc)
    {
        cli_print(cli,":%s\n",argv[i]);
        i++;
    }
    return CLI_OK;
}

int cli_at_history(struct cli_def *cli, UNUSED(char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
    int i,j;

    cli_print(cli, "\nAT Command history:");
    j=0;
    for (i = 0; i < MAX_HISTORY; i++)
    {
        if (cli->history[i])
        {
            if(cli->history[i][0]=='A' && cli->history[i][1]=='T')
            {
                j++;
                cli_print(cli, "%3d. %s", j, cli->history[i]);
            }
        }
    }

    return CLI_OK;
}


int cli_show_at_help(struct cli_def *cli, struct cli_command *c)
{
    struct cli_command *p;

    for (p = c; p; p = p->next)
    {
        if (p->command && p->callback && cli->privilege >= p->privilege &&
            (p->mode == cli->mode || p->mode == MODE_ANY))
        {
            char *str;
            str=NULL;
            str = cli_command_name(cli, p);
            if(str!=NULL)
            {
                if( *str=='A' && *(str+1)=='T' )
                {
                    cli_print(cli, "  %-20s %s", str, p->help ? : "");
                }
            }
        }

        if (p->children)
            cli_show_at_help(cli, p->children);
    }

    return CLI_OK;
}

int cli_at_list(struct cli_def *cli, UNUSED(char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
    cli_print(cli, "\nAT Commands available:\n");
    cli_show_at_help(cli, cli->commands);
    return CLI_OK;
}

int cli_at_quit(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
        cli_print(cli, "A: Command Syntax:ATA\n");
        cli_print(cli, "O: Command Syntax:ATO\n");
        cli_print(cli, "OK\n");
        return CLI_OK;
        }
    }
    if (argc==0) {
        cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
        cli_set_configmode(cli, MODE_EXEC, NULL);
        cli_print(cli, "OK");
        return CLI_QUIT;
    } else {
        cli_print(cli,"Invalid parameters\n");
        return CLI_OK;
    }

}


/*
int cli_at_***(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if(argc>0)
    {
        if(strcmp(argv[0], "=") == 0 && strcmp(argv[argc-1], "?") == 0)
        {
            cli_print(cli, "\n+***: Command Syntax:AT+***=****\n");
            cli_print(cli, "OK\n");
            return CLI_OK;
        }
    }




    //here 




    cli_print(cli,"Invalid parameters\n");
    return CLI_OK;

}
*/


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


static bool character_check (const char* buffer,const unsigned int buff_len,const unsigned char char_low,const unsigned char char_high)
{
    if ((buff_len!=1)|| (buffer[0]<char_low)||(buffer[0]>char_high))
        return FALSE;
    else {
        return TRUE;
    }
}


static bool gateway_check(const char* gw_buff,char* ip_addr_buff,char *str_maskbits)
{
    struct in_addr gw_addr,ip_addr;
    unsigned long gw_ul,ip_ul,mask_ul;
    int maskbits;
    int i;
    if ( ! inet_aton(gw_buff,&gw_addr) ) {
        return false;
    }
    if ( ! inet_aton(ip_addr_buff,&ip_addr) ) {
        return false;
    }

    /* Calculate the number of network bits */
    gw_ul = ntohl(gw_addr.s_addr);
    ip_ul = ntohl(ip_addr.s_addr);
    maskbits = atoi(str_maskbits);
    if ((maskbits<0)||(maskbits>32)) {
        /*syslog(LOGOPTS," start_address_check: maskbits =%d \n",maskbits); */
        return false;
    }
    mask_ul=0;
    for ( i=0 ; i<maskbits ; i++ )
        mask_ul |= 1<<(31-i);      

    if ((gw_ul&mask_ul)!=(ip_ul&mask_ul)) {
        /*syslog(LOGOPTS," start_address_check begin_addr_ul&mask_ul\n");*/
        return false;
    }
    if (gw_ul==(gw_ul&mask_ul)) {
        /*syslog(LOGOPTS," start_address_check begin_addr_ul<=subnet_ul\n");*/
        return false;
    }

    return true;       
}

bool netmask_check(const char* buffer,char *str_maskbits)
{
    struct in_addr netmask;
    unsigned long mask;
    int maskbits;
    int i;


    /* If the netmask has a dot, assume it is the netmask as an ip address*/
    if ( ! inet_aton(buffer,&netmask) ) {
        return false;
    }
    /* Calculate the number of network bits */
    mask = ntohl(netmask.s_addr);        

    for ( maskbits=32 ; ((mask & (1L<<(32-maskbits))) == 0)&&(maskbits>0) ; maskbits-- ) {
        /* syslog(LOGOPTS," netmask_check: maskbits =%d \n",maskbits); */
    }
    /* Re-create the netmask and compare to the origianl
     * to make sure it is a valid netmask.
     */
    mask = 0;
    for ( i=0 ; i<maskbits ; i++ )
        mask |= 1<<(31-i);

    if ( mask != ntohl(netmask.s_addr) ) {
        return false;
    } else {
        if (maskbits<10) {
            str_maskbits[0]=maskbits +'0';
            str_maskbits[1]=0;    
        } else {
            str_maskbits[0]=maskbits/10 +'0';
            str_maskbits[1]=maskbits%10 +'0';
            str_maskbits[2]=0;
        }

        return true;
    }
}

static bool IP_Address_check(const char* buffer,const unsigned int buff_len)
{
    int i;
    int dot[3]={0,0,0};

    for (i=0;i<buff_len; i++) {    /* get the position of dot*/
        if (buffer[i]=='.') {

            if (dot[0]==0)
                dot[0]= i;
            else {
                if (dot[1]==0)
                    dot[1]=i;
                else {
                    if (dot[2]==0)
                        dot[2]=i;
                    else
                        return FALSE;  /* more than three dots return false*/



                }
            }
        } else { /* not dot,Others must be between 0-9*/
            if (buffer[i]!=0) {
                if ((buffer[i]<'0')||(buffer[i]>'9'))
                /*  if(!isdigit(buffer[i]))*/
                {
                    return FALSE;
                }
            }
        }
    }/*end of for loop*/

    if ((dot[0]<1)||(dot[0]>3))  /* dot[0] position must be 1 to 3.*/
        return FALSE;

    for (i=0;i<2;i++) {
        if (((dot[i+1]- dot[i]) <2)||((dot[i+1]- dot[i]) >4))  /* not 1 and 3*/
            return FALSE;
    } /* check the position between dot is right or wrong*/

    if ((dot[2]==buff_len-1)||(buff_len-dot[2]>4))
        return FALSE;
    if (dot[0]==3) {
        if (buffer[0]>'2')
            return FALSE;
        if ((buffer[0]=='2')&&(buffer[1]>'5'))
            return FALSE;
        if ((buffer[0]=='2')&&(buffer[1]=='5')&&(buffer[2]>'5'))
            return FALSE;
    }
    if (dot[1]-dot[0]==4) {
        if (buffer[dot[0]+1]>'2')/* if number after dot is 2 then number 5,6 should be 55*/
            return FALSE;
        if ((buffer[dot[0]+1]=='2')&&(buffer[dot[0]+2]>'5'))
            return FALSE;
        if ((buffer[dot[0]+1]=='2')&&(buffer[dot[0]+2]=='5')&&(buffer[dot[0]+3]>'5'))
            return FALSE;
    }
    if (dot[2]-dot[1]==4) {
        if (buffer[dot[1]+1]>'2')/* if number after dot is 2 then number 5,6 should be 55*/
            return FALSE;
        if ((buffer[dot[1]+1]=='2')&&(buffer[dot[1]+2]>'5'))
            return FALSE;
        if ((buffer[dot[1]+1]=='2')&&(buffer[dot[1]+2]=='5')&&(buffer[dot[1]+3]>'5'))
            return FALSE;
    }

    if (buff_len-dot[2]==4) {
        if (buffer[dot[2]+1]>'2')
            return FALSE;
        if ((buffer[dot[2]+1]=='2')&&(buffer[dot[2]+2]>'5'))
            return FALSE;
        if ((buffer[dot[2]+1]=='2')&&(buffer[dot[2]+2]=='5')&&(buffer[dot[2]+3]>'5'))
            return FALSE;
    }
    return TRUE;
}

bool digit_check(const char* buffer,const unsigned int buff_len, int num_low, int num_high)
{
    int i;  

    for (i=0;i<buff_len;i++) {
        if (!isdigit(buffer[i])) {
            if ((i==0)&&(buffer[i]=='-')) {
                continue;
            }
            return FALSE;
        }
    }

    i=atoi(buffer);

    if ((i<num_low)||(i>num_high))
        return FALSE;
    else
        return TRUE;       
} 

void LockFile(int fd)
{
    struct flock lock;       
    memset (&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl (fd, F_SETLKW, &lock);
}

void UnLockFile(int fd)
{
    struct flock lock;  
    memset (&lock, 0, sizeof(lock));
    lock.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &lock);
    close (fd);     
}

void get_sms(struct cli_def *cli, int index, int flag)
{
    gn_sms message;
    gn_error    error;
    int mstatus;
    char tbuf[4];
    char cmd[6];

    if (flag==0) strcpy(cmd,"+CMGR");
    else strcpy(cmd,"+MMGR");
    gn_data_clear(data);
    memset(&message, 0, sizeof(gn_sms));
    message.memory_type = gn_str2memory_type("SM");
    message.number = index;
    //cli_print(cli, "number=%d,%d",index, message.number);
    data->sms = &message;
    state->config.connect_script[0] = flag;
    error = gn_sms_get(data, state);
    state->config.connect_script[0] = 0;
    if (error==GN_ERR_NONE) {
        if (message.status == GN_SMS_Unread) mstatus=0;
        else mstatus=1;

        if ((message.dcs.type == GN_SMS_DCS_GeneralDataCoding) &&
            (message.dcs.u.general.alphabet == GN_SMS_DCS_8bit))
            cli_print(cli, "%s: \"%s\",\"%s\",,\"%02d/%02d/%02d,%02d:%02d:%02d+%02d\"\r\n%s\r\n",cmd,
                      (mstatus ? "REC READ" : "REC UNREAD"),
                      message.remote.number,
                      message.smsc_time.year, message.smsc_time.month, message.smsc_time.day,
                      message.smsc_time.hour, message.smsc_time.minute, message.smsc_time.second,
                      message.time.timezone, "<Not implemented>");
        else {
            if (message.smsc_time.timezone) {
                if (message.smsc_time.timezone > 0)
                    sprintf(tbuf,"+%02d", message.smsc_time.timezone);
                else
                    sprintf(tbuf,"%03d", message.smsc_time.timezone);
            } else
                sprintf(tbuf,"+00");
            cli_print(cli, "%s: \"%s\",\"%s\",,\"%02d/%02d/%02d,%02d:%02d:%02d%s\"\r\n%s\r\n",cmd,
                      (mstatus ? "REC READ" : "REC UNREAD"),
                      message.remote.number,
                      message.smsc_time.year, message.smsc_time.month, message.smsc_time.day,
                      message.smsc_time.hour, message.smsc_time.minute, message.smsc_time.second,
                      tbuf, message.user_data[0].u.text);

        }
        cli_print(cli, "\r\nOK");
    } else
        cli_print(cli, "%s ERROR: %d",cmd,error);

}

void delete_sms(struct cli_def *cli, int index, int dflag)
{
    gn_sms message;
    gn_error    error;

    gn_data_clear(data);
    memset(&message, 0, sizeof(gn_sms));
    message.memory_type = gn_str2memory_type("SM");
    message.number = index;

    data->sms = &message;
    cli_print(cli, "index=%d dflag=%d",index,dflag);
    switch (dflag) {
    case 0:
        error = gn_sms_delete(data, state);
        break;
    case 1:
    case 4:
        message.number = dflag;
        error = gn_sms_delete_option(data, state);
        break;
    default:
        cli_print(cli, "+CMGD: Invalid parameters\n");
        return;
    }

    if (error==GN_ERR_NONE) {
        cli_print(cli, "\r\nOK");
    } else
        cli_print(cli, "+CMGD ERROR: %d",error);

}

gn_error send_sms(struct cli_def *cli, char *phone_number, char *sms_data)
{
    gn_sms sms;
    gn_error error;
    gn_data_clear(data);
    memset(&sms, 0, sizeof(gn_sms));
    data->sms=&sms;
    sms.user_data[0].type = GN_SMS_DATA_Text;
    gn_number_sanitize(phone_number, 32);
    strcpy(data->sms->remote.number, phone_number);

    if (data->sms->remote.number[0] == '+')
        data->sms->remote.type = GN_GSM_NUMBER_International;
    else
        data->sms->remote.type = GN_GSM_NUMBER_Unknown;

    strcpy(sms.user_data[0].u.text, sms_data);
    sms.user_data[0].length = strlen(sms_data);
    data->message_center = calloc(1, sizeof(gn_sms_message_center));
    data->message_center->id = 1;
    error = gn_sm_functions(GN_OP_GetSMSCenter, data, state);
    if (error == GN_ERR_NONE) {
        snprintf(sms.smsc.number, sizeof(sms.smsc.number), "%s", data->message_center->smsc.number);
        sms.smsc.type = data->message_center->smsc.type;
    } else {
        syslog(LOGOPTS,"Cannot read the SMSC number from your phone. If the sms send will fail, please use --smsc option explicitely giving the number.\n");
        free(data->message_center);

        return error;
    }
    free(data->message_center);


    if (!sms.smsc.type) sms.smsc.type = GN_GSM_NUMBER_Unknown;
    //cli_print(cli, "Sending SMS to %s (text: %s)\n", data->sms->remote.number, data->sms->user_data[0].u.text);

    /* FIXME: set more SMS fields before sending */
    error = gn_sms_send(data, state);

    return error;

}

gn_error decodesms(unsigned char *buffer, int length, gn_data *data, struct gn_statemachine *state, int flag)
{
    at_line_buffer buf;
    gn_error ret = GN_ERR_NONE;
    unsigned int sms_len, l, extraoffset, offset = 0;
    unsigned char tmp[256];
    unsigned char *t;
    gn_error error;
    at_driver_instance *drvinst = AT_DRVINST(state);

    if (!data->raw_sms)
        return GN_ERR_INTERNALERROR;

    buf.line1 = buffer + 1;
    buf.length = length;

    splitlines(&buf);

    t = strrchr(buf.line1, ',');
    /* The following sequence is correct for emtpy location:
     * w: AT+CMGR=9
     * r: AT+CMGR=9
     *  :
     *  : OK
     */
    if (!t)
        return GN_ERR_EMPTYLOCATION;
    sms_len = atoi(t+1);
    if (sms_len == 0)
        return GN_ERR_EMPTYLOCATION;

    sms_len = strlen(buf.line2) / 2;
    //tmp = calloc(sms_len, 1);
    if (!tmp) {
        syslog(LOGOPTS, "Not enough memory for buffer.\n");
        return GN_ERR_INTERNALERROR;
    }
    memset(tmp,0,sms_len);
    hex2bin(tmp, buf.line2, sms_len);

    if (!drvinst->no_smsc) {
        l = tmp[offset] + 1;
        if (l > sms_len || l > GN_SMS_SMSC_NUMBER_MAX_LENGTH) {
            syslog(LOGOPTS, "Invalid message center length (%d)\n", l);
            ret = GN_ERR_INTERNALERROR;
            goto out;
        }
        memcpy(data->raw_sms->message_center, tmp, l);
        offset += l;
    }

    data->raw_sms->reject_duplicates   = 0;
    data->raw_sms->report_status       = 0;
    data->raw_sms->reference           = 0;
    data->raw_sms->reply_via_same_smsc = 0;
    data->raw_sms->report              = 0;

    data->raw_sms->type                = (tmp[offset] & 0x03) << 1;
    if (data->raw_sms->type == GN_SMS_MT_Deliver) {

        data->raw_sms->more_messages       = tmp[offset] & 0x04;
        // bits 3 and 4 of the first octet unused;
        data->raw_sms->report_status       = tmp[offset] & 0x20;
        extraoffset = 10;
    } else if (data->raw_sms->type == GN_SMS_MT_Submit) {

        data->raw_sms->reject_duplicates   = tmp[offset] & 0x04;
        data->raw_sms->validity_indicator  = (tmp[offset] & 0x18) >> 3;
        extraoffset = 3;
    } else {

        return GN_ERR_INTERNALERROR;
    }
    data->raw_sms->more_messages       = tmp[offset];
    data->raw_sms->udh_indicator       = tmp[offset] & 0x40;
    offset++;

    if (data->raw_sms->type == GN_SMS_MT_Submit) {
        data->raw_sms->reference = tmp[offset++];
    } else {
        data->raw_sms->reference = 0;
    }

    l = (tmp[offset] % 2) ? tmp[offset] + 1 : tmp[offset];
    l = l / 2 + 2;
    if (l + offset + extraoffset > sms_len || l > GN_SMS_NUMBER_MAX_LENGTH) {
        syslog(LOGOPTS, "Invalid remote number length (%d)\n", l);
        ret = GN_ERR_INTERNALERROR;
        goto out;
    }
    memcpy(data->raw_sms->remote_number, tmp + offset, l);
    offset += l;
    data->raw_sms->pid                 = tmp[offset++];
    data->raw_sms->dcs                 = tmp[offset++];
    if (data->raw_sms->type == GN_SMS_MT_Deliver) {
        memcpy(data->raw_sms->smsc_time, tmp + offset, 7);
        offset += 7;
    }
    if (data->raw_sms->type == GN_SMS_MT_Submit) {
        if (data->raw_sms->validity_indicator == GN_SMS_VP_None) {
            offset += 0;
        } else if (data->raw_sms->validity_indicator == GN_SMS_VP_RelativeFormat) {
            // FIXME save this info
            offset += 1;
        } else if (data->raw_sms->validity_indicator == GN_SMS_VP_EnhancedFormat ||
                   data->raw_sms->validity_indicator == GN_SMS_VP_AbsoluteFormat) {
            // FIXME save this info
            offset += 7;
        } else {
            syslog(LOGOPTS, "Internal Error on validity_indicator");
            return GN_ERR_INTERNALERROR;
        }
    }
    data->raw_sms->length              = tmp[offset++];
    data->raw_sms->user_data_length = data->raw_sms->length;
    if (data->raw_sms->udh_indicator & 0x40)
        data->raw_sms->user_data_length -= tmp[offset] + 1;
    if (sms_len - offset > 1000) {
        syslog(LOGOPTS, "Phone gave as poisonous (too short?) reply %s, either phone went crazy or communication went out of sync\n", buf.line2);
        ret = GN_ERR_INTERNALERROR;
        goto out;
    }
    memcpy(data->raw_sms->user_data, tmp + offset, sms_len - offset);

    /* Try to figure out the location and status first */
    if (flag==0) {
        if (sscanf(buf.line1, "CMGL: %d,%d,%*s", &data->raw_sms->number, &data->raw_sms->status) != 2)
            return GN_ERR_FAILED;
    } else {
        if (sscanf(buf.line1, "MMGL: %d,%d,%*s", &data->raw_sms->number, &data->raw_sms->status) != 2)
            return GN_ERR_FAILED;
    }

    data->sms->number = data->raw_sms->number;

    switch (data->raw_sms->status) {
    case 0:
        data->raw_sms->status = GN_SMS_Unread;
        break;
    case 1:
        data->raw_sms->status = GN_SMS_Read;
        break;
    case 2:
        data->raw_sms->status = GN_SMS_Unsent;
        break;
    case 3:
        data->raw_sms->status = GN_SMS_Sent;
        break;
    }
    data->sms->status = data->raw_sms->status;

    out:
    //free(tmp);
    return ret;
}

static void get_new_sms_list(struct cli_def *cli, int status, int flag)
{
    gn_raw_data grawdata;
    gn_sms message;
    gn_data mdata;
    gn_error    error = GN_ERR_NONE;
    char *pos;
    char *tmp;
    char *buffp = NULL;
    char *buff;
    char *dp;
    int length,mstatus;
    char cmd[6];
    char tbuf[4];

    if (flag==0) strcpy(cmd,"+CMGL");
    else strcpy(cmd,"+MMGL");
    gn_data_clear(data);
    memset(&message, 0, sizeof(gn_sms));
    message.memory_type = gn_str2memory_type("SM");
    message.number = status;
    data->sms = &message;
    data->raw_data = &grawdata;
    state->config.connect_script[0]=flag;
    error = gn_smslist_get(data,state);
    state->config.connect_script[0]=0;
    //syslog(LOGOPTS,"gn_smslist_get:\n%s\n",data->raw_data->data);
    if (error == GN_ERR_NONE) {

        tmp = data->raw_data->data;
        dp = tmp;
        pos = tmp;

        tmp = strstr(pos,cmd);
        if (!tmp) {
            goto rout;
        }

        while (pos) {

            pos = strstr(tmp+7, cmd);
            if (pos)
                length = pos-tmp;
            else
                length = data->raw_data->length - (tmp-dp);
            if (length<=0) {
                syslog(LOGOPTS,"atclid getsmslist length=%d\n",length);
                break;
            }
            buff = (char*)realloc(buffp, length*sizeof(char));
            if (buff) {
                gn_sms_raw rawsms;
                buffp=buff;
                memcpy(buff, tmp, length);
                gn_data_clear(&mdata);
                memset(&message, 0, sizeof(gn_sms));
                message.memory_type = gn_str2memory_type("SM");
                memset(&rawsms, 0, sizeof(gn_sms_raw));
                mdata.raw_sms = &rawsms;
                mdata.sms = &message;
                error = decodesms(buff,length,&mdata,state,flag);
                if (error != GN_ERR_NONE) {
                    syslog(LOGOPTS,"atclid getsmslist, decodesms return error=%d\n",error);
                    free(buffp);
                    buffp=NULL;
                    buff=NULL;
                    goto rout;
                }

                error = gn_sms_parse(&mdata);
                if (error != GN_ERR_NONE) {
                    syslog(LOGOPTS,"atclid getsmslist, sms_parse return error=%d\n",error);
                    free(buffp);
                    buffp=NULL;
                    buff=NULL;
                    goto rout;
                }

                if (message.status == GN_SMS_Unread) mstatus=0;
                else mstatus=1;

                if ((message.dcs.type == GN_SMS_DCS_GeneralDataCoding) &&
                    (message.dcs.u.general.alphabet == GN_SMS_DCS_8bit))
                    cli_print(cli, "%s: %d,\"%s\",\"%s\",,\"%02d/%02d/%02d,%02d:%02d:%02d+%02d\"\r\n%s\r\n",cmd,message.number,
                              (mstatus ? "REC READ" : "REC UNREAD"),
                              message.remote.number,
                              message.smsc_time.year, message.smsc_time.month, message.smsc_time.day,
                              message.smsc_time.hour, message.smsc_time.minute, message.smsc_time.second,
                              message.time.timezone, "<Not implemented>");
                else {
                    if (message.smsc_time.timezone) {
                        if (message.smsc_time.timezone > 0)
                            sprintf(tbuf,"+%02d", message.smsc_time.timezone);
                        else
                            sprintf(tbuf,"%03d", message.smsc_time.timezone);
                    } else
                        sprintf(tbuf,"+00");
                    cli_print(cli, "%s: %d,\"%s\",\"%s\",,\"%02d/%02d/%02d,%02d:%02d:%02d%s\"\r\n%s\r\n",cmd,message.number,
                              (mstatus ? "REC READ" : "REC UNREAD"),
                              message.remote.number,
                              message.smsc_time.year, message.smsc_time.month, message.smsc_time.day,
                              message.smsc_time.hour, message.smsc_time.minute, message.smsc_time.second,
                              tbuf, message.user_data[0].u.text);

                }

            } else {
                free(buffp);
                buffp=NULL;
                buff=NULL;
                goto rout;
            }

            tmp=pos;
        }

        free(buffp);
        buffp=NULL;
        cli_print(cli, "\r\nOK");
    } else cli_print(cli, "%s ERROR: %d",cmd,error);
    rout:
    if (grawdata.data) free(grawdata.data);
    grawdata.data=NULL;
    return;
}

int cmd_read_sms(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    int index;
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) {
            cli_print(cli, "+CMGR: Command Syntax:AT+CMGR=<index>\n");

            cli_print(cli, "OK");

            return CLI_OK;
        } else {
            if (!digit_check(argv[1], strlen(argv[1]),0,SIGNED_INT_MAX)) {
                cli_print(cli, "+CMGR: Invalid parameters\n");
                return CLI_OK;
            }
            index = atoi(argv[1]);
            int fd = open ( RadioBusyFlagFile, O_WRONLY);
            if (fd<0) {
                syslog(LOGOPTS,"%s cmd_read_sms %s:fd<0\n",__func__,RadioBusyFlagFile);
                cli_print(cli, "ERROR: Module busy");
                return CLI_OK;
            }
            LockFile(fd);
            gn_lib_businit(&data, &state);
            get_sms(cli, index, 0);
            gn_lib_busterminate(&state);
            UnLockFile(fd);
            return CLI_OK;
        }
    }


    cli_print(cli, "+CMGR: Invalid parameters\n");
    return CLI_OK;
}


int cmd_read_sms_unchange(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    int index;
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MMGR: Command Syntax:AT+MMGR=<index>\n");

            cli_print(cli, "OK");

            return CLI_OK;
        } else {
            if (!digit_check(argv[1], strlen(argv[1]),0,SIGNED_INT_MAX)) {
                cli_print(cli, "+MMGR: Invalid parameters\n");
                return CLI_OK;
            }
            index = atoi(argv[1]);
            int fd = open ( RadioBusyFlagFile, O_WRONLY);
            if (fd<0) {
                syslog(LOGOPTS,"%s cmd_read_sms_unchange %s:fd<0\n",__func__,RadioBusyFlagFile);
                cli_print(cli, "ERROR: Module busy");
                return CLI_OK;
            }
            LockFile(fd);
            gn_lib_businit(&data, &state);
            get_sms(cli, index, 1);
            gn_lib_busterminate(&state);
            UnLockFile(fd);
            return CLI_OK;
        }
    }
    cli_print(cli, "+MMGR: Invalid parameters\n");
    return CLI_OK;
}


int cmd_list_sms(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    int index;
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+CMGL: Command Syntax:AT+CMGL=<status>\n");

            cli_print(cli, "OK");

            return CLI_OK;
        } else {
            if (!digit_check(argv[1], strlen(argv[1]),0,4)) {
                cli_print(cli, "+CMGL: Invalid parameters\n");
                return CLI_OK;
            }
            index = atoi(argv[1]);
            if (index==2||index==3) {
                cli_print(cli, "+CMGL: Invalid parameters\n");
                return CLI_OK;
            }
            int fd = open ( RadioBusyFlagFile, O_WRONLY);
            if (fd<0) {
                syslog(LOGOPTS,"%s cmd_list_sms %s:fd<0\n",__func__,RadioBusyFlagFile);
                cli_print(cli, "ERROR: Module busy");
                return CLI_OK;
            }
            LockFile(fd);
            gn_lib_businit(&data, &state);
            get_new_sms_list(cli, index, 0);
            gn_lib_busterminate(&state);
            UnLockFile(fd);
            return CLI_OK;
        }
    }


    cli_print(cli, "+CMGL: Invalid parameters\n");
    return CLI_OK;
}

int cmd_list_sms_unchange(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    int index;
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MMGL: Command Syntax:AT+MMGL=<status>\n");

            cli_print(cli, "OK");

            return CLI_OK;
        } else 
        {
            if (!digit_check(argv[1], strlen(argv[1]),0,4)) {
                cli_print(cli, "+MMGL: Invalid parameters\n");
                return CLI_OK;
            }
            index = atoi(argv[1]);
            if (index==2||index==3) {
                cli_print(cli, "+MMGL: Invalid parameters\n");
                return CLI_OK;
            }
            int fd = open ( RadioBusyFlagFile, O_WRONLY);
            if (fd<0) {
                syslog(LOGOPTS,"%s cmd_list_sms_unchange %s:fd<0\n",__func__,RadioBusyFlagFile);
                cli_print(cli, "ERROR: Module busy");
                return CLI_OK;
            }
            LockFile(fd);
            gn_lib_businit(&data, &state);
            get_new_sms_list(cli, index, 1);
            gn_lib_busterminate(&state);
            UnLockFile(fd);
            return CLI_OK;
        }
    }


    cli_print(cli, "+MMGL: Invalid parameters\n");
    return CLI_OK;
}



int cmd_send_sms(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+CMGS: Command Syntax:AT+CMGS=<Phone Number><CR>\n");
            cli_print(cli, "text is entered<ctrl-Z/ESC>\n");
            cli_print(cli, "OK");

            return CLI_OK;
        } else {
            cli_set_hostname(cli, argv[1]);
            cli_set_promptchar(cli, ">");
            return CLI_SMS_PRO;
        }
    }


    cli_print(cli, "+CMGS: Invalid parameters\n");
    return CLI_OK;
}

int cmd_delete_sms(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    int index, dflag;

    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+CMGD: Command Syntax:AT+CMGD=<index>,<delflag>\n");

            cli_print(cli, "OK");

            return CLI_OK;
        } else {
            if (!digit_check(argv[1], strlen(argv[1]),0,SIGNED_INT_MAX)) {
                cli_print(cli, "+CMGD: Invalid parameters\n");
                return CLI_OK;
            }
            index = atoi(argv[1]);
            dflag = 0;
            int fd = open ( RadioBusyFlagFile, O_WRONLY);
            if (fd<0) {
                syslog(LOGOPTS,"%s cmd_delete_sms %s:fd<0\n",__func__,RadioBusyFlagFile);
                cli_print(cli, "ERROR: Module busy");
                return CLI_OK;
            }
            LockFile(fd);
            gn_lib_businit(&data, &state);
            delete_sms(cli, index, dflag);
            gn_lib_busterminate(&state);
            UnLockFile(fd);
            return CLI_OK;
        }
    }

    if (argc == 3 && strcmp(argv[0], "=") == 0) {
        if (!digit_check(argv[1], strlen(argv[1]),0,SIGNED_INT_MAX)) {
            cli_print(cli, "+CMGD: Invalid parameters\n");
            return CLI_OK;
        }
        index = atoi(argv[1]);
        if (!digit_check(argv[2], strlen(argv[2]),0,4)) {
            cli_print(cli, "+CMGD: Invalid parameters\n");
            return CLI_OK;
        }

        dflag = atoi(argv[2]);
        int fd = open ( RadioBusyFlagFile, O_WRONLY);
        if (fd<0) {
            syslog(LOGOPTS,"%s cmd_delete_sms %s:fd<0\n",__func__,RadioBusyFlagFile);
            cli_print(cli, "ERROR: Module busy");
            return CLI_OK;
        }
        LockFile(fd);
        gn_lib_businit(&data, &state);
        delete_sms(cli, index, dflag);
        gn_lib_busterminate(&state);
        UnLockFile(fd);
        return CLI_OK;
    }
    cli_print(cli, "+CMGD: Invalid parameters\n");
    return CLI_OK;
}

bool Item_read_file(char *UserDB , char* key_word, char* read_buff, int read_buff_len)
{
    FILE* f;
    int j=0;
    char *p;     
    char buffer_get[256];
    if (strlen(key_word)<2)return false;

    if (!(f = fopen(UserDB, "r"))) 
    {
        return false;
    }

    read_buff[0]=0;
    while (fgets(buffer_get, 256, f)) 
    {
        p=NULL;
        p = strstr(buffer_get, key_word);//imei=" 012773002002648"
        if (p != NULL)
        {
            p+=strlen(key_word)+1;
            while (*p == ' ' || *p == '\"')p++; //first space
            j=0;
            while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<read_buff_len-1)) 
                {
                    read_buff[j]=*p;
                    p++;
                    j++;
                 }
            read_buff[j]=0;
            break;
        }//if p
    }
    if(f)fclose(f);
    return true;
}

bool get_option_by_section_name(struct uci_context *ctx,char*package_name,char*section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_element *e= NULL;
    struct uci_ptr ptr;
    struct uci_option *p_option = NULL;

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

//        struct uci_context *ctx;
//        ctx = uci_alloc_context();  // for VIP4G
//        if (!ctx) 
//        {
//            cli_print(cli,"ERROR: Out of memory\n");
//            return CLI_OK;
//        }
//
//        uci_free_context(ctx);
//




int cmd_reboot(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if(strcmp(argv[1], "?") == 0) 
        {
        cli_print(cli, "+MREB: Command Syntax:AT+MREB\n");
        cli_print(cli, "OK");
        return CLI_OK;
        }
    }

    if (argc == 0) 
    {
        cli_print(cli, "OK. Rebooting...\n");
        SubProgram_Start("/sbin/reboot","");
        return CLI_QUIT;
    }

    cli_print(cli, "+MREB: Invalid parameters");
    return CLI_OK;
}

int cmd_reset_default(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0)
    {
        if(strcmp(argv[1], "?") == 0)
        {
            cli_print(cli, "+MRTF: Command Syntax:AT+MRTF=<Action>\n");
            cli_print(cli, "Action:\n");
            cli_print(cli, "0     pre-set action\n");
            cli_print(cli, "1     confirm action\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MRTF: Invalid parameters");
            return CLI_OK;
        }

        if(argv[1][0]=='0')
        {
            at_flag=9;
            cli_print(cli, "+MRTF: Please confirm action to restore immediatly.\n");
        }
        if(argv[1][0]=='1')
        {
            if(at_flag==9)
            {
                cli_print(cli, "OK. Restore now and rebooting...\n");
                SubProgram_Start("/etc/m_cli/common_at.sh","restore 1");
                return CLI_QUIT;
            }else
                cli_print(cli, "+MRTF: Please pre-set action firstly.\n");
            at_flag=0;
        }
        return CLI_OK;
    }

    cli_print(cli, "+MRTF: Invalid parameters");
    return CLI_OK;
}


int cmd_traffic_wtimer(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MTWT: Command Syntax:AT+MTWT=<Mode>[,<Interval_s>,<Reboot Time Limit_s>]\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0   Disable\n");
            cli_print(cli, "1   Enalbe\n");
            cli_print(cli, "Reboot Time Limit:300-60000\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MTWT: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff[10];
        get_option_by_section_name(ctx, "twatchdog", "vip4g", "enable", buff);
        if(argv[1][0]!=buff[0])
        {
            if(set_option_by_section_name(ctx, "twatchdog", "vip4g", "enable", argv[1]))
            {
                SubProgram_Start("/etc/init.d/twatchdog","stop");
                SubProgram_Start("/etc/init.d/twatchdog","start");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 4 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MTWT: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),0,65535))
        {
            cli_print(cli, "+MTWT: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),300,60000))
        {
            cli_print(cli, "+MTWT: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff[10],last_set;
        get_option_by_section_name(ctx, "twatchdog", "vip4g", "enable", buff);
        last_set=buff[0];
        if(set_option_by_section_name(ctx, "twatchdog", "vip4g", "enable", argv[1]) \
            && set_option_by_section_name(ctx, "twatchdog", "vip4g", "reset_timer", argv[2]) \
            && set_option_by_section_name(ctx, "twatchdog", "vip4g", "reboot_timer", argv[3]) )
        {
            if(last_set!=argv[1][0] || last_set=='1')
            {
                SubProgram_Start("/etc/init.d/twatchdog","stop");
                SubProgram_Start("/etc/init.d/twatchdog","start");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0], "?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff[10],buff2[10],buff3[10];
        get_option_by_section_name(ctx, "twatchdog", "vip4g", "enable", buff);
        get_option_by_section_name(ctx, "twatchdog", "vip4g", "reset_timer", buff2);
        get_option_by_section_name(ctx, "twatchdog", "vip4g", "reboot_timer", buff3);
        switch(buff[0])
        {
        case '0':
            cli_print(cli, "+MTWT: \nMode:0  Disabled \nCheck Interval:%s seconds\nReboot Time Limit:%s seconds",buff2,buff3);
            break;
        case '1':
            cli_print(cli, "+MTWT: \nMode:1  Enabled \nCheck Interval:%s seconds\nReboot Time Limit:%s seconds",buff2,buff3);
            break;
        default:
            cli_print(cli, "+MTWT: Invalid Current Settings.");
            break;
        }
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MTWT: Invalid parameters");
    return CLI_OK;
}


int cmd_dis_service(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MDISS: Command Syntax:AT+MDISS=<Mode>\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Discoverable\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MDISS: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff[10],last_set;
        int set_i=atoi(argv[1]);
        get_option_by_section_name(ctx, "sdpServer", "server", "Discovery_Service_Status", buff);
        last_set=buff[0];
        buff[0]=set_i+'A';
        buff[1]=0;
        if(last_set!=buff[0])
        {
            if(set_option_by_section_name(ctx, "sdpServer", "server", "Discovery_Service_Status", buff))
            {
                SubProgram_Start("/etc/init.d/sdpServer","stop");
                SubProgram_Start("/etc/init.d/sdpServer","start");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff[10];
        get_option_by_section_name(ctx, "sdpServer", "server", "Discovery_Service_Status", buff);
        switch(buff[0])
        {
        case 'A':
            cli_print(cli, "+MDISS: 0     Disable");
            break;
        case 'B':
            cli_print(cli, "+MDISS: 1     Discoverable");
            break;
        default:
            cli_print(cli, "+MDISS: Invalid Current Settings.");
            break;
        }
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MDISS: Invalid parameters");
    return CLI_OK;
}


int cmd_conf_read(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if( strcmp(argv[1], "?") == 0)
        {
            cli_print(cli, "&R: Command Syntax:AT&R\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if (argc==0) 
    {
        cli_print(cli, "OK");
        return CLI_OK;
    }

    cli_print(cli, "Invalid parameters");
    return CLI_OK;

}

int cmd_display_sysconf(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if( strcmp(argv[1], "?") == 0)
        {
        cli_print(cli, "&V: Command Syntax:AT&V\n");
        cli_print(cli, "OK");
        return CLI_OK;
        }
    }

    if (argc == 0) 
    {
        char buff[50];
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        cli_print(cli, "&V:\n");
        buff[0]=0;
        if(get_option_by_section_name(ctx, "system", "@system[0]", "hostname", buff))
        {
            cli_print(cli, "  hostname:%s\n",buff);
        }
        buff[0]=0;
        if(get_option_by_section_name(ctx, "system", "@system[0]", "timezone", buff))
        {
            cli_print(cli, "  timezone:%s\n",buff);
        }
        buff[0]=0;
        if(get_option_by_section_name(ctx, "system", "@system[0]", "systemmode", buff))
        {
            cli_print(cli, "  systemmode:%s\n",buff);
        }
        buff[0]=0;
        if(get_option_by_section_name(ctx, "system", "datetime", "mode", buff))
        {
            cli_print(cli, "  time mode:%s\n",buff);
        }

        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;

    }

    cli_print(cli, "&V: Invalid parameters");
    return CLI_OK;
}

int cmd_conf_enable(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if( strcmp(argv[1], "?") == 0)
        {
        cli_print(cli, "&W: Command Syntax:AT&W\n");
        cli_print(cli, "OK");
        return CLI_OK;
        }
    }

    if (argc==0) 
    {
        cli_print(cli, "OK");
        return CLI_OK;
    }

    cli_print(cli,"Invalid parameters");
    return CLI_OK;
}


int cmd_admin_pwd(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MPWD: Command Syntax:AT+MPWD=<New password>,<confirm password>\n");
            cli_print(cli, "password:at least 5 characters\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }
    }
    if (argc == 3 && strcmp(argv[0], "=") == 0)
    {
        if(strcmp(argv[1],argv[2])==0 && strlen(argv[1])>=5 ) 
        {
            char str_buff[100];
            sprintf(str_buff,"passwd admin %s %s",argv[1],argv[2]);
            SubProgram_Start("/etc/m_cli/common_at.sh",str_buff);
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    cli_print(cli, "+MPWD: Invalid parameters");
    return CLI_OK;
}

int cmd_sys_info(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0)
    {
        if(strcmp(argv[1], "?") == 0) 
        {

        cli_print(cli, "+MSYSI: Command Syntax:AT+MSYSI\n");
        cli_print(cli, "OK");
        return CLI_OK;
        }
    }

    if (argc==0) {

        char* p;
        char* p1;
        int i = 0;
        char buff[100];
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        cli_print(cli,"Carrier:\n");
        buff[0]=0;
        if (true==Item_read_file("/var/run/VIP4G_status", "imei=", buff, 64))
        {
            cli_print(cli, "  IMEI:%s\n",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/var/run/VIP4G_status", "simid=", buff, 64))
        {
            cli_print(cli, "  SIMID:%s\n",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/var/run/VIP4G_status", "imsi=", buff, 64))
        {
            cli_print(cli, "  IMSI:%s\n",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/var/run/VIP4G_status", "phonenum=", buff, 64))
        {
            //phonenum=" , 14034611576 ,129"
            p=strchr(buff,',');
            if(p!=NULL)
            {
                p++;
                p1=strchr(p,',');
                if(p1!=NULL) 
                {
                    *p1=0;
                    cli_print(cli, "  Phone Num:%s\n",p);
                }
            }
        }
        buff[0]=0;
        if (true==Item_read_file("/var/run/VIP4G_status", "connect_status=", buff, 64))
        {
            if(buff[0]=='_')buff[0]=' ';
            cli_print(cli, "  Status:%s\n",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/var/run/VIP4G_status", "network=", buff, 64))
        {
            //network=" 0,0, Bell ,7"
            p=NULL;
            p1=buff;
            p=strchr(buff,',');
            if(p!=NULL)
            {
                p++;
                p1=p;
                p=strchr(p1,',');
                if(p!=NULL)
                {
                    p++;
                    p1=p;
                    p=strchr(p1,',');
                    if(p!=NULL)
                    {
                        *p=0;
                    }
                }
            }
            cli_print(cli, "  Network:%s\n",p1);
        }
        buff[0]=0;
        if (true==Item_read_file("/var/run/VIP4G_status", "rssi=", buff, 64))
        {
            cli_print(cli, "  RSSI:%s\n",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/var/run/VIP4G_status", "temp=", buff, 64))
        {
            cli_print(cli, "  Temperature:%s\n",buff);
        }

        cli_print(cli, "Ethernet Port:\n");
        buff[0]=0;
        if (false==fetch_Local_IP_MAC(ifname_bridge,buff))fetch_Local_IP_MAC(ifname_eth0,buff);
        cli_print(cli, "  MAC:%s\n",buff);
        buff[0]=0;
        if (false==fetch_Local_IP_ADDR(ifname_bridge,buff))fetch_Local_IP_ADDR(ifname_eth0,buff);
        cli_print(cli, "  IP:%s\n",buff);
        buff[0]=0;
        if (false==fetch_Local_IP_MASK(ifname_bridge,buff))fetch_Local_IP_MASK(ifname_eth0,buff);
        cli_print(cli, "  MASK:%s\n",buff);
        buff[0]=0;
        if (false==fetch_Local_IP_MAC(ifname_wan,buff))fetch_Local_IP_MAC(ifname_eth1,buff);
        if(buff[0]!=0)
        {
            cli_print(cli, "  Wan MAC:%s\n",buff);
            buff[0]=0;
            if (false==fetch_Local_IP_ADDR(ifname_wan,buff))fetch_Local_IP_ADDR(ifname_eth1,buff);
            if(buff[0]!=0)cli_print(cli, "  Wan IP:%s\n",buff);
            buff[0]=0;
            if (false==fetch_Local_IP_MASK(ifname_wan,buff))fetch_Local_IP_MASK(ifname_eth1,buff);
            if(buff[0]!=0)cli_print(cli, "  Wan MASK:%s\n",buff);
        }

        cli_print(cli, "System:\n");
        buff[0]=0;
        if(get_option_by_section_name(ctx, "system", "@system[0]", "hostname", buff))
        {
            cli_print(cli, "  Device:%s\n",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/etc/version", "PRODUCTNAME=", buff, 64))
        {
            cli_print(cli, "  Product:%s\n",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/etc/version", "PRODUCT=", buff, 64))
        {
            cli_print(cli, "  Image:%s\n",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/etc/version", "HARDWARE=", buff, 64))
        {
            cli_print(cli, "  Hardware:%s\n",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/etc/version", "SOFTWARE=", buff, 64))
        {
            cli_print(cli, "  Software:%s",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/etc/version", "BUILD=", buff, 64))
        {
            cli_print(cli, " build %s\n",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/etc/version", "VENDOR=", buff, 100))
        {
            cli_print(cli, "\nCopyright: %s\n",buff);
        }
        time_t start;
        time(&start);
        cli_print(cli, "Time: %s\n",ctime(&start));

        uci_free_context(ctx);
        return CLI_OK;
    } else {
        cli_print(cli,"+MSYSI: Invalid parameters\n");

        return CLI_OK;
    }
    return CLI_OK;
}

int cmd_modem_record(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if(strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+GMR: Command Syntax:AT+GMR\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if (argc == 0) {

        char* p;
        char* p1;
        int i = 0;
        char buff[100];
        cli_print(cli, "+GMR:\n");
        buff[0]=0;
        if (true==Item_read_file("/etc/version", "HARDWARE=", buff, 64))
        {
            cli_print(cli, " Hardware Version:%s",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/etc/version", "SOFTWARE=", buff, 64))
        {
            cli_print(cli, " Software Version:%s",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/etc/version", "BUILD=", buff, 64))
        {
            cli_print(cli, " build %s\n",buff);
        }
        buff[0]=0;
        if (true==Item_read_file("/etc/version", "VENDOR=", buff, 100))
        {
            cli_print(cli, " Copyright: %s\n",buff);
        }
        time_t start;
        time(&start);
        cli_print(cli, " System Time: %s",ctime(&start));
        cli_print(cli, "OK");

        return CLI_OK;
    }

    cli_print(cli, "+GMR: Invalid parameters\n");

    return CLI_OK;

}

int cmd_modem_name(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MMNAME: Command Syntax:AT+MMNAME=<modem_name>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        if(set_option_by_section_name(ctx, "system", "@system[0]", "hostname", argv[1]))
        {
            cli_print(cli, "OK");
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MMNAME: Invalid parameters\n");
    return CLI_OK;
}


int cmd_manu_id(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0)
    {
        if(strcmp(argv[1], "?") == 0) 
        {
        cli_print(cli, "+GMI: Command Syntax:AT+GMI\n");
        cli_print(cli, "OK");
        return CLI_OK;
        }
    }

    if (argc == 0) {

        char buff[100];
        buff[0]=0;
        Item_read_file("/etc/version", "VENDOR=", buff, 100);
        cli_print(cli, "+GMI: %s\n",buff);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    cli_print(cli, "+GMI: Invalid parameters");

    return CLI_OK;
}

int cmd_phone_number(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if(strcmp(argv[1], "?") == 0) 
        {
        cli_print(cli, "+CNUM: Command Syntax:AT+CNUM\n");
        cli_print(cli, "OK");
        return CLI_OK;
        }
    }

    if (argc == 0) {

        char buff[100];
        char *p; 
        char *p1;
        buff[0]=0;
        if (true==Item_read_file("/var/run/VIP4G_status", "phonenum=", buff, 64))
        {
            //phonenum=" , 14034611576 ,129"
            p=strchr(buff,',');
            if(p!=NULL)
            {
                p++;
                p1=strchr(p,',');
                if(p1!=NULL) 
                {
                    *p1=0;
                }
            }
        }
        cli_print(cli, "+CNUM: %s\n",p);
        cli_print(cli, "OK");
        return CLI_OK;
    }
    cli_print(cli, "+CNUM: Invalid parameters");

    return CLI_OK;
}


int cmd_modem_imi(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if(strcmp(argv[1], "?") == 0) 
        {
        cli_print(cli, "+CIMI: Command Syntax:AT+CIMI\n");
        cli_print(cli, "OK");
        return CLI_OK;
        }
    }

    if (argc == 0) {

        char buff[100];
        buff[0]=0;
        Item_read_file("/var/run/VIP4G_status", "imei=", buff, 64);
        cli_print(cli, "+CIMI: IMEI:%s, ",buff);
        buff[0]=0;
        Item_read_file("/var/run/VIP4G_status", "imsi=", buff, 64);
        cli_print(cli, "IMSI:%s\n",buff);

        cli_print(cli, "OK");
        return CLI_OK;
    }
    cli_print(cli, "+CIMI: Invalid parameters");

    return CLI_OK;
}

int cmd_modem_sim(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if(strcmp(argv[1], "?") == 0) 
        {
        cli_print(cli, "+CCID: Command Syntax:AT+CCID\n");
        cli_print(cli, "OK");
        return CLI_OK;
        }
    }

    if (argc == 0) {

        char buff[100];
        buff[0]=0;
        Item_read_file("/var/run/VIP4G_status", "simid=", buff, 64);

        cli_print(cli, "+CCID: %s\n",buff);
        cli_print(cli, "OK");
        return CLI_OK;
    }
    cli_print(cli, "+CCID: Invalid parameters");

    return CLI_OK;
}

int cmd_local_eip(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MLEIP: Command Syntax:AT+MLEIP=<IP Address>,<Netmask>,<Gateway>\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }
    }

    char buff[100];
    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        if (false==fetch_Local_IP_ADDR(ifname_bridge,buff))fetch_Local_IP_ADDR(ifname_eth0,buff);
        cli_print(cli, "+MLEIP: \"%s\"",buff);
        buff[0]=0;
        if (false==fetch_Local_IP_MASK(ifname_bridge,buff))fetch_Local_IP_MASK(ifname_eth0,buff);
        cli_print(cli, ",\"%s\"",buff);
        buff[0]=0;
        get_option_by_section_name(ctx, "network", "lan", "gateway", buff);
        cli_print(cli, ",\"%s\"\n",buff);
        cli_print(cli, "OK");

        uci_free_context(ctx);
        return CLI_OK;
    }

    if (argc == 4 && strcmp(argv[0], "=") == 0) 
    {
        syslog(LOGOPTS,"leip argv=%s,%s,%s",argv[1],argv[2],argv[3]);
        char netmasklen[5];
        if (false==IP_Address_check(argv[1],strlen(argv[1])))
        {
            cli_print(cli, "+MLEIP: Invalid parameter: IP Address %s\n",argv[1]);
            return CLI_OK;
        } else 
        {
            if (false==netmask_check(argv[2],netmasklen)) 
            {
                cli_print(cli, "+MLEIP: Invalid parameter: IP MASK %s\n",argv[2]);
                return CLI_OK;
            } else
            {
                if(false==gateway_check(argv[3],argv[1],netmasklen))
                {
                    cli_print(cli, "+MLEIP: Invalid parameter: Gateway %s\n",argv[3]);
                    return CLI_OK;
                }
                //here go out and it is ok.
            }
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        if(set_option_by_section_name(ctx, "network", "lan", "ipaddr", argv[1]) \
           && set_option_by_section_name(ctx, "network", "lan", "netmask", argv[2]) \
           && set_option_by_section_name(ctx, "network", "lan", "gateway", argv[3]) )
        {
            buff[0]=0;
            get_option_by_section_name(ctx, "network", "lan", "proto", buff);
            if(strcmp(buff,"dhcp")==0)
            {
                cli_print(cli, "+MLEIP: DHCP enabled. Effected after disable dhcp. \n");
            }else
            {
                cli_print(cli, "+MLEIP: setting and restarting network...\n");
                if(SubProgram_Start("/etc/init.d/network","restart")==false)cli_print(cli, "+MLEIP: Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "+MLEIP: Not succeed.\n");
        }

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }
    cli_print(cli, "+MLEIP: Invalid parameter\n");
    return CLI_OK;
}


int cmd_local_weip(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MWEIP: Command Syntax:AT+MWEIP=<IP Address>,<Netmask>,<Gateway>\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }
    }

    char buff[100];
    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        if (false==fetch_Local_IP_ADDR(ifname_wan,buff))fetch_Local_IP_ADDR(ifname_eth1,buff);
        cli_print(cli, "+MWEIP: \"%s\"",buff);
        buff[0]=0;
        if (false==fetch_Local_IP_MASK(ifname_wan,buff))fetch_Local_IP_MASK(ifname_eth1,buff);
        cli_print(cli, ",\"%s\"",buff);
        buff[0]=0;
        get_option_by_section_name(ctx, "network", "wan", "gateway", buff);
        cli_print(cli, ",\"%s\"\n",buff);
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    if (argc == 4 && strcmp(argv[0], "=") == 0) 
    {
        syslog(LOGOPTS,"weip argv=%s,%s,%s",argv[1],argv[2],argv[3]);
        char netmasklen[5];
        if (false==IP_Address_check(argv[1],strlen(argv[1]))) 
        {
            cli_print(cli, "+MWEIP: Invalid parameter: IP Address %s\n",argv[1]);
            return CLI_OK;
        } else 
        {
            if (false==netmask_check(argv[2],netmasklen)) 
            {
                cli_print(cli, "+MWEIP: Invalid parameter: IP MASK %s\n",argv[2]);
                return CLI_OK;
            } else
            {
                if(false==gateway_check(argv[3],argv[1],netmasklen))
                {
                    cli_print(cli, "+MWEIP: Invalid parameter: Gateway %s\n",argv[3]);
                    return CLI_OK;
                }
                //here go out and it is ok.
            }
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        if(set_option_by_section_name(ctx, "network", "wan", "ipaddr", argv[1]) \
           && set_option_by_section_name(ctx, "network", "wan", "netmask", argv[2]) \
           && set_option_by_section_name(ctx, "network", "wan", "gateway", argv[3]) )
        {

            buff[0]=0;
            get_option_by_section_name(ctx, "network", "wan", "proto", buff);
            if(strcmp(buff,"dhcp")==0)
            {
                cli_print(cli, "+MWEIP: DHCP enabled. Effected after disable dhcp. \n");
            }else
            {
                cli_print(cli, "+MWEIP: setting and restarting network...\n");
                if(SubProgram_Start("/etc/init.d/network","restart")==false)cli_print(cli, "+MWEIP: Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "+MWEIP: Not succeed.\n");
        }

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }
    cli_print(cli, "+MLEIP: Invalid parameter\n");
    cli_print(cli, "ERROR");
    return CLI_OK;
}


int cmd_lan_sct(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{

    char buff[100];
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MSCT: Command Syntax:AT+MSCT=<Mode>\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0    DHCP\n");
            cli_print(cli, "1    Static IP\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MSCT: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "network", "lan", "ipaddr",buff);
        int set_i=atoi(argv[1]);
        if (false==IP_Address_check(buff,strlen(buff)) && set_i==1 ) 
        {
            uci_free_context(ctx);
            cli_print(cli, "+MSCT: Please firstly set static IP Address. \n");
            cli_print(cli, "Usage: AT+MLEIP or AT+MSIP \n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "network", "lan", "stp",buff);
        int set_i_last=-1;
        if(strcmp(buff,"on")==0)set_i_last=1;
        else if(strcmp(buff,"off")==0)set_i_last=0;

        if(set_i==1 && set_i!=set_i_last)
        {
            set_option_by_section_name(ctx, "network", "lan", "stp", "on");
            set_option_by_section_name(ctx, "network", "lan", "proto", "static");
        }
        if(set_i==0 && set_i!=set_i_last)
        {
            set_option_by_section_name(ctx, "network", "lan", "stp", "off");
            set_option_by_section_name(ctx, "network", "lan", "proto", "dhcp");
        }

        if(set_i!=set_i_last)
        {
            cli_print(cli, "+MSCT: setting and restarting network...\n");
            if(SubProgram_Start("/etc/init.d/network","restart")==false)cli_print(cli, "+MSCT: Program restart fault, Please reboot to effect new settings.\n");
        }

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "network", "lan", "ipaddr", buff);
        cli_print(cli, "+MSCT: %s",buff);
        buff[0]=0;
        get_option_by_section_name(ctx, "network", "lan", "stp", buff);
        cli_print(cli, "  StaticIP Set:%s",buff);
        buff[0]=0;
        get_option_by_section_name(ctx, "network", "lan", "proto", buff);
        cli_print(cli, "  Type:%s\n",buff);
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MSCT: Invalid parameters");
    return CLI_OK;
}


int cmd_wan_sct(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{

    char buff[100];
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MWSCT: Command Syntax:AT+MWSCT=<Mode>\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0    DHCP\n");
            cli_print(cli, "1    Static IP\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MWSCT: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "network", "wan", "ipaddr",buff);
        int set_i=atoi(argv[1]);
        if (false==IP_Address_check(buff,strlen(buff)) && set_i==1 ) 
        {
            uci_free_context(ctx);
            cli_print(cli, "+MWSCT: Please firstly set static IP Address. \n");
            cli_print(cli, "Usage: AT+MWEIP or AT+MWSIP \n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "network", "wan", "stp",buff);
        int set_i_last=-1;
        if(strcmp(buff,"on")==0)set_i_last=1;
        else if(strcmp(buff,"off")==0)set_i_last=0;

        if(set_i==1 && set_i!=set_i_last)
        {
            set_option_by_section_name(ctx, "network", "wan", "stp", "on");
            set_option_by_section_name(ctx, "network", "wan", "proto", "static");
        }
        if(set_i==0 && set_i!=set_i_last)
        {
            set_option_by_section_name(ctx, "network", "wan", "stp", "off");
            set_option_by_section_name(ctx, "network", "wan", "proto", "dhcp");
        }

        if(set_i!=set_i_last)
        {
            cli_print(cli, "+MWSCT: setting and restarting network...\n");
            if(SubProgram_Start("/etc/init.d/network","restart")==false)cli_print(cli, "+MWSCT: Program restart fault, Please reboot to effect new settings.\n");
        }

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "network", "wan", "ipaddr", buff);
        cli_print(cli, "+MWSCT: %s",buff);
        buff[0]=0;
        get_option_by_section_name(ctx, "network", "wan", "stp", buff);
        cli_print(cli, "  StaticIP Set:%s",buff);
        buff[0]=0;
        get_option_by_section_name(ctx, "network", "wan", "proto", buff);
        cli_print(cli, "  Type:%s\n",buff);
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MWSCT: Invalid parameters");
    return CLI_OK;
}


int cmd_lan_sip(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{

    char buff[100];
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MSIP: Command Syntax:AT+MSIP=<static IP address>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if (false==IP_Address_check(argv[1],strlen(argv[1]))) 
        {
            cli_print(cli, "+MSIP: Invalid parameter: IP Address %s\n",argv[1]);
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        if(set_option_by_section_name(ctx, "network", "lan", "ipaddr", argv[1]))
        {
            buff[0]=0;
            get_option_by_section_name(ctx, "network", "lan", "stp", buff);
            if(strcmp(buff,"on")!=0)
            {
                cli_print(cli, "+MSIP: Static IP disabled. Effected after ensable. \n");
            }else
            {
                cli_print(cli, "+MSIP: setting and restarting network...\n");
                if(SubProgram_Start("/etc/init.d/network","restart")==false)cli_print(cli, "+MSIP: Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "+MSIP: Not succeed.\n");
        }

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "network", "lan", "ipaddr", buff);
        cli_print(cli, "+MSIP: %s",buff);
        buff[0]=0;
        get_option_by_section_name(ctx, "network", "lan", "stp", buff);
        cli_print(cli, "  StaticIP Set:%s\n",buff);
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MWSIP: Invalid parameters");
    return CLI_OK;
}

int cmd_wan_sip(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{

    char buff[100];
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MWSIP: Command Syntax:AT+MWSIP=<static IP address>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if (false==IP_Address_check(argv[1],strlen(argv[1]))) 
        {
            cli_print(cli, "+MWSIP: Invalid parameter: IP Address %s\n",argv[1]);
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        if(set_option_by_section_name(ctx, "network", "wan", "ipaddr", argv[1]))
        {

            buff[0]=0;
            get_option_by_section_name(ctx, "network", "wan", "stp", buff);
            if(strcmp(buff,"on")!=0)
            {
                cli_print(cli, "+MWSIP: Static IP disabled. Effected after ensable. \n");
            }else
            {
                cli_print(cli, "+MWSIP: setting and restarting network...\n");
                if(SubProgram_Start("/etc/init.d/network","restart")==false)cli_print(cli, "+MWSIP: Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "+MWSIP: Not succeed.\n");
        }

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "network", "wan", "ipaddr", buff);
        cli_print(cli, "+MWSIP: %s",buff);
        buff[0]=0;
        get_option_by_section_name(ctx, "network", "wan", "stp", buff);
        cli_print(cli, "  StaticIP Set:%s\n",buff);
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MWSIP: Invalid parameters");
    return CLI_OK;
}


int cmd_dhcp_enable(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    char buff[10];

    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MDHCP: Command Syntax:AT+MDHCP=<Action>\n");
            cli_print(cli, "Action:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        int j=argv[1][0]-'0';
        int i=atoi(argv[1]);
        if(i!=j || i>1 || i<0)
        {
            cli_print(cli, "+MDHCP: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "dhcp", "lan", "ignore", buff);
        if(strcmp(buff,argv[1])==0)  //it is different in two system, same but infact it is different.
        {
            if(buff[0]=='0')buff[0]='1';
            else buff[0]='0';
            if(set_option_by_section_name(ctx, "dhcp", "lan", "ignore", buff))
            {
                cli_print(cli, "+MDHCP: setting and restarting DHCP Server...\n");
                if(SubProgram_Start("/etc/init.d/dnsmasq","restart")==false)cli_print(cli, "+MDHCP: Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "+MDHCP: Not succeed.\n");
            }
        }

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "dhcp", "lan", "ignore", buff);
        if(buff[0]=='0')buff[0]='1';
        else buff[0]='0';

        cli_print(cli, "+MDHCP: %s\n",buff);
        cli_print(cli, "OK");

        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MDHCP: Invalid parameters");

    return CLI_OK;

}


int cmd_wdhcp_enable(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    char buff[10];

    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MWDHCP: Command Syntax:AT+MWDHCP=<Action>\n");
            cli_print(cli, "Action:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        int j=argv[1][0]-'0';
        int i=atoi(argv[1]);
        if(i!=j || i>1 || i<0)
        {
            cli_print(cli, "+MWDHCP: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "dhcp", "wan", "ignore", buff);
        if(strcmp(buff,argv[1])==0)  //it is different in two system, same but infact it is different.
        {
            if(buff[0]=='0')buff[0]='1';
            else buff[0]='0';
            if(set_option_by_section_name(ctx, "dhcp", "wan", "ignore", buff))
            {
                cli_print(cli, "+MWDHCP: setting and restarting DHCP Server...\n");
                if(SubProgram_Start("/etc/init.d/dnsmasq","restart")==false)cli_print(cli, "+MWDHCP: Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "+MWDHCP: Not succeed.\n");
            }
        }

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        buff[0]=0;
        get_option_by_section_name(ctx, "dhcp", "wan", "ignore", buff);
        if(buff[0]=='0')buff[0]='1';
        else buff[0]='0';

        cli_print(cli, "+MWDHCP: %s\n",buff);
        cli_print(cli, "OK");

        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MWDHCP: Invalid parameters");

    return CLI_OK;

}


int cmd_dhcp_address(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MDHCPA: Command Syntax:AT+MDHCPA=<Start IP>,<End IP>\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }
    }

    char buff[100],IPbase[40];
    char *p;
    int ip_start,ip_end;
    IPbase[0]=0;
    if (false==fetch_Local_IP_ADDR(ifname_bridge,IPbase))fetch_Local_IP_ADDR(ifname_eth0,IPbase);
    p=NULL;
    p=strrchr(IPbase,'.');
    if(p!=NULL)
    {
        p++;
        *p=0;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }


        buff[0]=0;
        get_option_by_section_name(ctx, "dhcp", "lan", "start", buff);
        ip_start=atoi(buff);
        if(ip_start<0)ip_start=0; 
        if(ip_start>255)ip_start=255; 
        get_option_by_section_name(ctx, "dhcp", "lan", "limit", buff);
        ip_end=ip_start+atoi(buff);
        if(ip_end>255)ip_end=255;
        cli_print(cli, "+MDHCPA: %s%d - %s%d \n",IPbase,ip_start,IPbase,ip_end);
        cli_print(cli, "OK");

        uci_free_context(ctx);
        return CLI_OK;
    }

    if (argc == 3 && strcmp(argv[0], "=") == 0) 
    {

        if (false==IP_Address_check(argv[1],strlen(argv[1]))) 
        {
            cli_print(cli, "+MDHCPA: Invalid parameter: Start IP Address %s\n",argv[1]);
            return CLI_OK;
        }
        if (false==IP_Address_check(argv[2],strlen(argv[2]))) 
        {
            cli_print(cli, "+MDHCPA: Invalid parameter: End IP Address %s\n",argv[2]);
            return CLI_OK;
        }

        strcpy(buff,argv[1]);
        p=strrchr(buff,'.');
        p++;
        ip_start=atoi(p);
        *p=0;
        if(strcmp(IPbase,buff)!=0)
        {
            cli_print(cli, "+MDHCPA: Invalid parameter: Start IP Address %s:%s\n",IPbase,argv[1]);
            return CLI_OK;
        }

        strcpy(buff,argv[2]);
        p=strrchr(buff,'.');
        p++;
        ip_end=atoi(p);
        *p=0;
        if(strcmp(IPbase,buff)!=0 || ip_end<ip_start)
        {
            cli_print(cli, "+MDHCPA: Invalid parameter: End IP Address %s:%s\n",IPbase,argv[2]);
            return CLI_OK;
        }

        ip_end=ip_end-ip_start;

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        sprintf(IPbase,"%d",ip_start);
        sprintf(buff,"%d",ip_end);
        if(set_option_by_section_name(ctx, "dhcp", "lan", "start", IPbase) \
           && set_option_by_section_name(ctx, "dhcp", "lan", "limit", buff) )
        {

            buff[0]=0;
            get_option_by_section_name(ctx, "dhcp", "lan", "ignore", buff);
            if(strcmp(buff,"0")==0)
            {
                cli_print(cli, "+MDHCPA: setting and restarting network...\n");
                if(SubProgram_Start("/etc/init.d/dnsmasq","restart")==false)cli_print(cli, "+MDHCPA: Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "+MDHCPA: DHCP server disabled. Effected after enable dhcp. \n");
            }
        }else
        {
            cli_print(cli, "+MDHCPA: Not succeed.\n");
        }

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    cli_print(cli, "+MDHCPA: Invalid parameters");
    return CLI_OK;
}


int cmd_wdhcp_address(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MWDHCPA: Command Syntax:AT+MWDHCPA=<Start IP>,<End IP>\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }
    }

    char buff[100],IPbase[40];
    char *p;
    int ip_start,ip_end;
    IPbase[0]=0;
    if (false==fetch_Local_IP_ADDR(ifname_wan,IPbase))fetch_Local_IP_ADDR(ifname_eth1,IPbase);
    p=NULL;
    p=strrchr(IPbase,'.');
    if(p!=NULL)
    {
        p++;
        *p=0;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }


        buff[0]=0;
        get_option_by_section_name(ctx, "dhcp", "wan", "start", buff);
        ip_start=atoi(buff);
        if(ip_start<0)ip_start=0; 
        if(ip_start>255)ip_start=255; 
        get_option_by_section_name(ctx, "dhcp", "wan", "limit", buff);
        ip_end=ip_start+atoi(buff);
        if(ip_end>255)ip_end=255;
        cli_print(cli, "+MWDHCPA: %s%d - %s%d \n",IPbase,ip_start,IPbase,ip_end);
        cli_print(cli, "OK");

        uci_free_context(ctx);
        return CLI_OK;
    }

    if (argc == 3 && strcmp(argv[0], "=") == 0) 
    {

        if (false==IP_Address_check(argv[1],strlen(argv[1]))) 
        {
            cli_print(cli, "+MWDHCPA: Invalid parameter: Start IP Address %s\n",argv[1]);
            return CLI_OK;
        }
        if (false==IP_Address_check(argv[2],strlen(argv[2]))) 
        {
            cli_print(cli, "+MWDHCPA: Invalid parameter: End IP Address %s\n",argv[2]);
            return CLI_OK;
        }

        strcpy(buff,argv[1]);
        p=strrchr(buff,'.');
        p++;
        ip_start=atoi(p);
        *p=0;
        if(strcmp(IPbase,buff)!=0)
        {
            cli_print(cli, "+MWDHCPA: Invalid parameter: Start IP Address %s:%s\n",IPbase,argv[1]);
            return CLI_OK;
        }

        strcpy(buff,argv[2]);
        p=strrchr(buff,'.');
        p++;
        ip_end=atoi(p);
        *p=0;
        if(strcmp(IPbase,buff)!=0 || ip_end<ip_start)
        {
            cli_print(cli, "+MWDHCPA: Invalid parameter: End IP Address %s:%s\n",IPbase,argv[2]);
            return CLI_OK;
        }

        ip_end=ip_end-ip_start;

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        sprintf(IPbase,"%d",ip_start);
        sprintf(buff,"%d",ip_end);
        if(set_option_by_section_name(ctx, "dhcp", "wan", "start", IPbase) \
           && set_option_by_section_name(ctx, "dhcp", "wan", "limit", buff) )
        {

            buff[0]=0;
            get_option_by_section_name(ctx, "dhcp", "wan", "ignore", buff);
            if(strcmp(buff,"0")==0)
            {
                cli_print(cli, "+MWDHCPA: setting and restarting network...\n");
                if(SubProgram_Start("/etc/init.d/dnsmasq","restart")==false)cli_print(cli, "+MWDHCPA: Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "+MWDHCPA: DHCP server disabled. Effected after enable dhcp. \n");
            }
        }else
        {
            cli_print(cli, "+MWDHCPA: Not succeed.\n");
        }

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    cli_print(cli, "+MWDHCPA: Invalid parameters");
    return CLI_OK;
}


int cmd_eth_mac(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MEMAC: Command Syntax:AT+MEMAC\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }
    }

    if (argc == 0) {
        char ems[20]={"00:00:00:00:00:00"};
        if (false==fetch_Local_IP_MAC(ifname_bridge,ems)) {
            cli_print(cli, "ERROR");
            syslog(LOGOPTS,"atclid: %s %s invokes fetch_Local_IP_MAC error\n",__FUNCTION__, command);
            return CLI_OK;
        }
        cli_print(cli, "+MEMAC: \"%s\"\n",ems);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    cli_print(cli, "+MEMAC: Invalid parameters");

    return CLI_OK;
}

int cmd_weth_mac(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MWEMAC: Command Syntax:AT+MWEMAC\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }
    }

    if (argc == 0) {
        char ems[20]={"00:00:00:00:00:00"};
        if (false==fetch_Local_IP_MAC(ifname_wan,ems)) {
            cli_print(cli, "ERROR");
            syslog(LOGOPTS,"atclid: %s %s invokes fetch_Local_WAN_IP_MAC error\n",__FUNCTION__, command);
            return CLI_OK;
        }
        cli_print(cli, "+MWEMAC: \"%s\"\n",ems);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    cli_print(cli, "+MWEMAC: Invalid parameters");

    return CLI_OK;
}

int cmd_icmp_enable(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MIKACE: Command Syntax:AT+MIKACE=<Mode>\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MIKACE: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "keepalive", "vip4g", "enable", buff);
        if(buff[0]!=argv[1][0])
        {
            if(set_option_by_section_name(ctx, "keepalive", "vip4g", "enable", argv[1]))
            {
                set_option_by_section_name(ctx, "keepalive", "vip4g", "type", "ICMP");
                SubProgram_Start("/etc/init.d/keepalive","stop");
                SubProgram_Start("/etc/init.d/keepalive","start");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }


    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "keepalive", "vip4g", "enable", buff);
        cli_print(cli, "+MIKACE: %s ",buff);
        get_option_by_section_name(ctx, "keepalive", "vip4g", "type", buff);
        cli_print(cli, "  Type:%s \n",buff);
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MIKACE: Invalid parameters");
    return CLI_OK;
}


int cmd_icmp_ka(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MIKAC: Command Syntax:AT+MIKAC=<host name>,<interval in seconds>,<count>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if (argc == 4  && strcmp(argv[0], "=") == 0) 
    {
        if(strlen(argv[1])<4)
        {
            cli_print(cli, "+MIKAC: Invalid parameters");
            return CLI_OK;
        }

        if(!digit_check(argv[2], strlen(argv[2]),0,65535))
        {
            cli_print(cli, "+MIKAC: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MIKAC: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10],buff1[10];
        get_option_by_section_name(ctx, "keepalive", "vip4g", "enable", buff);
        get_option_by_section_name(ctx, "keepalive", "vip4g", "type", buff1);
        if(buff[0]=='1'&& strcmp(buff1,"ICMP")!=0)
        {
            cli_print(cli, "+MIKAC: Current type is not ICMP. Please set with AT+MIKACE firstly.\n");
            cli_print(cli, "OK");
            uci_free_context(ctx);
            return CLI_OK;
        }
        if(set_option_by_section_name(ctx, "keepalive", "vip4g", "hostname", argv[1]))
        {
            set_option_by_section_name(ctx, "keepalive", "vip4g", "interval", argv[2]);
            set_option_by_section_name(ctx, "keepalive", "vip4g", "count", argv[3]);
            if(buff[0]=='1')
            {
                SubProgram_Start("/etc/init.d/keepalive","stop");
                SubProgram_Start("/etc/init.d/keepalive","start");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }

        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }


    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "keepalive", "vip4g", "type", buff);
        cli_print(cli, "+MIKAC: %s\n",buff);
        get_option_by_section_name(ctx, "keepalive", "vip4g", "enable", buff);
        cli_print(cli, "status:%s \n",buff);
        get_option_by_section_name(ctx, "keepalive", "vip4g", "hostname", buff);
        cli_print(cli, "hostname:%s \n",buff);
        get_option_by_section_name(ctx, "keepalive", "vip4g", "interval", buff);
        cli_print(cli, "interval:%s \n",buff);
        get_option_by_section_name(ctx, "keepalive", "vip4g", "count", buff);
        cli_print(cli, "count:%s \n",buff);
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MIKAC: Invalid parameters");
    return CLI_OK;
}

int cmd_ddns_enable(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MDDNSE: Command Syntax:AT+MDDNSE=<Mode>\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MDDNSE: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "updatedd", "ddns", "update", buff);
        if(buff[0]!=argv[1][0])
        {
            if(set_option_by_section_name(ctx, "updatedd", "ddns", "update", argv[1]))
            {
                SubProgram_Start("/etc/init.d/updatedd","stop");
                SubProgram_Start("/etc/init.d/updatedd","start");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }


    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "updatedd", "ddns", "update", buff);
        cli_print(cli, "+MDDNSE: Mode %s \n",buff);
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MDDNSE: Invalid parameters");
    return CLI_OK;
}


int cmd_ddns_conf(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MDDNS: Command Syntax:AT+MDDNS=<service type>,<host>,<user name>,<password>\n");
            cli_print(cli, "service type:\n  0  changeip\n  1  dyndns\n  2  eurodyndns\n  3  hn\n  4  noip\n  5  ods\n  6  ovh\n  7  regfish\n  8  tzo\n  9  zoneedit\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if (argc == 5 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,9))
        {
            cli_print(cli, "+MDDNS: Invalid parameters");
            return CLI_OK;
        }

       if( strlen(argv[2])<4 || strlen(argv[3])<3 || strlen(argv[4])<2  )
        {
            cli_print(cli, "+MDDNS: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10],str_buff[10];
        get_option_by_section_name(ctx, "updatedd", "ddns", "update", buff);

        set_option_by_section_name(ctx, "updatedd", "ddns", "host", argv[2]);
        set_option_by_section_name(ctx, "updatedd", "ddns", "username", argv[3]);
        set_option_by_section_name(ctx, "updatedd", "ddns", "password", argv[4]);
        switch(argv[1][0])
        {
        case '0':
            set_option_by_section_name(ctx, "updatedd", "ddns", "service", "changeip");
            break;
        case '1':
            set_option_by_section_name(ctx, "updatedd", "ddns", "service", "dyndns");
            break;
        case '2':
            set_option_by_section_name(ctx, "updatedd", "ddns", "service", "eurodyndns");
            break;
        case '3':
            set_option_by_section_name(ctx, "updatedd", "ddns", "service", "hn");
            break;
        case '4':
            set_option_by_section_name(ctx, "updatedd", "ddns", "service", "noip");
            break;
        case '5':
            set_option_by_section_name(ctx, "updatedd", "ddns", "service", "ods");
            break;
        case '6':
            set_option_by_section_name(ctx, "updatedd", "ddns", "service", "ovh");
            break;
        case '7':
            set_option_by_section_name(ctx, "updatedd", "ddns", "service", "regfish");
            break;
        case '8':
            set_option_by_section_name(ctx, "updatedd", "ddns", "service", "tzo");
            break;
        case '9':
            set_option_by_section_name(ctx, "updatedd", "ddns", "service", "zoneedit");
            break;
        default:
            break;
        }

        if(buff[0]=='1')
        {
            SubProgram_Start("/etc/init.d/updatedd","stop");
            SubProgram_Start("/etc/init.d/updatedd","start");
        }
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;

    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[30];
        get_option_by_section_name(ctx, "updatedd", "ddns", "service", buff);
        cli_print(cli, "+MDDNS:\nService: %s \n",buff);
        get_option_by_section_name(ctx, "updatedd", "ddns", "host", buff);
        cli_print(cli, "Host: %s \n",buff);
        get_option_by_section_name(ctx, "updatedd", "ddns", "username", buff);
        cli_print(cli, "username: %s \n",buff);
        get_option_by_section_name(ctx, "updatedd", "ddns", "password", buff);
        cli_print(cli, "password: %s \n",buff);
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MDDNS: Invalid parameters");
    return CLI_OK;
}

int cmd_define_ntp(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    int i;
    char buff[100];
    char str_buff[100];
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MNTP: Command Syntax:AT+MNTP=<Status>[,<NTP server>[,<Port>]]\n");
            cli_print(cli, "Status:\n");
            cli_print(cli, "0     Disable, Local Time\n");
            cli_print(cli, "1     Enable, Network Time\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MNTP: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        strcpy(buff,"local");
        if(set_i==1)strcpy(buff,"sync");

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        str_buff[0]=0;
        get_option_by_section_name(ctx, "system", "datetime", "mode", str_buff);
        if(strcmp(str_buff,buff)!=0) 
        {
            if(set_option_by_section_name(ctx, "system", "datetime", "mode", buff))
            {
                SubProgram_Start("/etc/init.d/ntpclient","stop");
                SubProgram_Start("/etc/init.d/ntpclient","start");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    if (argc > 2 && argc<5 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MNTP: Invalid parameters");
            return CLI_OK;
        }

        if(strlen(argv[2])<3)
        {
            cli_print(cli, "+MNTP: Invalid parameters");
            return CLI_OK;
        }

        int iport=123;
        if(argc==4) 
        {
            if(!digit_check(argv[3], strlen(argv[3]),1,65535))
            {
                cli_print(cli, "+MNTP: Invalid parameters");
                return CLI_OK;
            }
            iport=atoi(argv[3]);
            if(iport<1)
            {
                cli_print(cli, "+MNTP: Invalid parameters: Port Number %s\n",argv[3]);
                return CLI_OK;
            }
        }

        int set_i=atoi(argv[1]);
        strcpy(buff,"local");
        if(set_i==1)strcpy(buff,"sync");

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        sprintf(str_buff,"%d",iport);
        if(set_option_by_section_name(ctx, "system", "datetime", "mode", buff) \
           && set_option_by_section_name(ctx, "ntpclient", "@ntpserver[0]", "hostname", argv[2]) )
        {
            //such @*[0] can't do continue times
            uci_free_context(ctx);
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }
            set_option_by_section_name(ctx, "ntpclient", "@ntpserver[0]", "port", str_buff);

            SubProgram_Start("/etc/init.d/ntpclient","stop");
            SubProgram_Start("/etc/init.d/ntpclient","start");
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }

        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0], "?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        get_option_by_section_name(ctx, "system", "datetime", "mode", buff);
        cli_print(cli, "+MNTP: \nStatus:%s\n",buff);
        i=0;
        while(i<100) 
        {
            buff[0]=0;
            sprintf(str_buff,"@ntpserver[%d]",i);
            if(get_option_by_section_name(ctx, "ntpclient", str_buff, "hostname", buff))
            {
                if(strlen(buff)<3)break;
                cli_print(cli, "%s:",buff);
                buff[0]=0;
                get_option_by_section_name(ctx, "ntpclient", str_buff, "port", buff);
                cli_print(cli, "%s ",buff);
            }else break;
            i++;
        }

        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MNTP: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_port_status(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTPS: Command Syntax:AT+MCTPS=<Mode>\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }


        int j=argv[1][0]-'0';
        int i=atoi(argv[1]);
        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MCTPS: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        if(i==1)strcpy(buff,"B"); 
        else strcpy(buff,"A");
        if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff))
        {
            if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            else cli_print(cli, "OK");
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        return CLI_OK;
    }


    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
        if(buff[0]=='B')strcpy(buff,"Enabled");
        else strcpy(buff,"Disabled");
        cli_print(cli, "+MCTPS: %s",buff);
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }


    cli_print(cli, "+MCTPS: Invalid parameters");
    return CLI_OK;
}

int cmd_com2_baud_rate(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTBR: Command Syntax:AT+MCTBR=<Baud Rate Type>\n");
            cli_print(cli, "Baud Rate Option:\n");
            cli_print(cli, "0  300\n");
            cli_print(cli, "1  600\n");
            cli_print(cli, "2  1200\n");
            cli_print(cli, "3  2400\n");
            cli_print(cli, "4  3600\n");
            cli_print(cli, "5  4800\n");
            cli_print(cli, "6  7200\n");
            cli_print(cli, "7  9600\n");
            cli_print(cli, "8  14400\n");
            cli_print(cli, "9  19200\n");
            cli_print(cli, "10  28800\n");
            cli_print(cli, "11  38400\n");
            cli_print(cli, "12  57600\n");
            cli_print(cli, "13  115200\n");
            cli_print(cli, "14  230400\n");
            cli_print(cli, "15  460800\n");
            cli_print(cli, "16  921600\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,16))
        {
            cli_print(cli, "+MCTBR: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        if (set_i>=0 && set_i<=16) 
        {
            struct uci_context *ctx;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }

            char buff[10];
            strcpy(buff,"A");
            buff[0]=buff[0]+set_i;
            if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_Data_Baud_Rate", buff))
            {
                buff[0]=0;
                get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
                if(buff[0]=='B')
                {
                    if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
                }
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
            uci_free_context(ctx);

            cli_print(cli, "OK");
            return CLI_OK;
        }
    }


    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_Data_Baud_Rate", buff);
        int i=buff[0]-'A';
        switch (i) 
        {
        case 0:
            cli_print(cli, "+MCTBR: 0  300");
            break;
        case 1:
            cli_print(cli, "+MCTBR: 1  600");
            break;
        case 2:
            cli_print(cli, "+MCTBR: 2  1200");
            break;
        case 3:
            cli_print(cli, "+MCTBR: 3  2400");
            break;
        case 4:
            cli_print(cli, "+MCTBR: 4  3600");
            break;
        case 5:
            cli_print(cli, "+MCTBR: 5  4800");
            break;
        case 6:
            cli_print(cli, "+MCTBR: 6  7200");
            break;
        case 7:
            cli_print(cli, "+MCTBR: 7  9600");
            break;
        case 8:
            cli_print(cli, "+MCTBR: 8  14400");
            break;
        case 9:
            cli_print(cli, "+MCTBR: 9  19200");
            break;
        case 10:
            cli_print(cli, "+MCTBR: 10  28800");
            break;
        case 11:
            cli_print(cli, "+MCTBR: 11  38400");
            break;
        case 12:
            cli_print(cli, "+MCTBR: 12  57600");
            break;
        case 13:
            cli_print(cli, "+MCTBR: 13  115200");
            break;
        case 14:
            cli_print(cli, "+MCTBR: 14  230400");
            break;
        case 15:
            cli_print(cli, "+MCTBR: A  460800");
            break;
        case 16:
            cli_print(cli, "+MCTBR: A  921600");
            break;
        default:
            cli_print(cli, "+MCTBR: Invalid Current Settings.");
            break;
        }

        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTBR: Invalid parameters");
    return CLI_OK;
}

//good for copy as model to others function.
int cmd_com2_data_format(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTDF: Command Syntax:AT+MCTDF=<Data Formate Type>\n");
            cli_print(cli, "Data Formate Option:\n");
            cli_print(cli, "0  8N1\n");
            cli_print(cli, "1  8N2\n");
            cli_print(cli, "2  8E1\n");
            cli_print(cli, "3  8O1\n");
            cli_print(cli, "4  7N1\n");
            cli_print(cli, "5  7N2\n");
            cli_print(cli, "6  7E1\n");
            cli_print(cli, "7  7O1\n");
            cli_print(cli, "8  7E2\n");
            cli_print(cli, "9  7O2\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,9))
        {
            cli_print(cli, "+MCTDF: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        if (set_i>=0 && set_i<=9) 
        {
            struct uci_context *ctx;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }

            char buff[10];
            strcpy(buff,"A");
            buff[0]=buff[0]+set_i;
            if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_Data_Format", buff))
            {
                buff[0]=0;
                get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
                if(buff[0]=='B')
                {
                    if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
                }
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
            uci_free_context(ctx);

            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_Data_Format", buff);
        int i=buff[0]-'A';
        switch (i) 
        {
        case 0:
            cli_print(cli, "+MCTDF: 0  8N1");
            break;
        case 1:
            cli_print(cli, "+MCTDF: 1  8N2");
            break;
        case 2:
            cli_print(cli, "+MCTDF: 2  8E1");
            break;
        case 3:
            cli_print(cli, "+MCTDF: 3  8O1");
            break;
        case 4:
            cli_print(cli, "+MCTDF: 4  7N1");
            break;
        case 5:
            cli_print(cli, "+MCTDF: 5  7N2");
            break;
        case 6:
            cli_print(cli, "+MCTDF: 6  7E1");
            break;
        case 7:
            cli_print(cli, "+MCTDF: 7  7O1");
            break;
        case 8:
            cli_print(cli, "+MCTDF: 8  7E2");
            break;
        case 9:
            cli_print(cli, "+MCTDF: 9  7O2");
            break;
        default:
            cli_print(cli, "+MCTDF: Invalid Current Settings.");
            break;
        }

        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTDF: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_data_mode(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTDM: Command Syntax:AT+MCTDM=<Data Mode Type>\n");
            cli_print(cli, "Data Mode Option:\n");
            cli_print(cli, "0  Seamless\n");
            cli_print(cli, "1  Transparent\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MCTDM: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        if (set_i>=0 && set_i<=1) 
        {
            struct uci_context *ctx;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }

            char buff[10];
            strcpy(buff,"A");
            buff[0]=buff[0]+set_i;
            if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_Data_Mode", buff))
            {
                buff[0]=0;
                get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
                if(buff[0]=='B')
                {
                    if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
                }
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
            uci_free_context(ctx);

            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_Data_Mode", buff);
        int i=buff[0]-'A';
        switch (i) 
        {
        case 0:
            cli_print(cli, "+MCTDM: 0  Seamless");
            break;
        case 1:
            cli_print(cli, "+MCTDM: 1  Transparent");
            break;
        default:
            cli_print(cli, "+MCTDM: Invalid Current Settings.");
            break;
        }

        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTDM: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_ct(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTCT: Command Syntax:AT+MCTCT=<timeout_s>\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,65535))
        {
            cli_print(cli, "+MCTCT: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        if (set_i>=0) 
        {
            struct uci_context *ctx;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }

            char buff[10];
            if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_Character_Timeout", argv[1]))
            {
                buff[0]=0;
                get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
                if(buff[0]=='B')
                {
                    if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
                }
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
            uci_free_context(ctx);

            cli_print(cli, "OK");
            return CLI_OK;
        }
    }


    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_Character_Timeout", buff);
        cli_print(cli, "+MCTCT: %s seconds",buff);
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTCT: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_mps(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTMPS: Command Syntax:AT+MCTMPS=<size>\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,65535))
        {
            cli_print(cli, "+MCTMPS: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        if (set_i>=0) 
        {
            struct uci_context *ctx;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }

            char buff[10];
            if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_Max_Packet_Len", argv[1]))
            {
                buff[0]=0;
                get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
                if(buff[0]=='B')
                {
                    if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
                }
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
            uci_free_context(ctx);

            cli_print(cli, "OK");
            return CLI_OK;
        }

    }


    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_Max_Packet_Len", buff);
        cli_print(cli, "+MCTMPS: %s",buff);
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTMPS: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_priority(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTP: Command Syntax:AT+MCTP=<Mode>");
            cli_print(cli, "Mode Option:\n");
            cli_print(cli, "0  Normal\n");
            cli_print(cli, "1  Medium\n");
            cli_print(cli, "2  High\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,2))
        {
            cli_print(cli, "+MCTP: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        if (set_i>=0) 
        {
            struct uci_context *ctx;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }

            char buff[10];
            strcpy(buff,"A");
            buff[0]=buff[0]+set_i;
            if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_QoS", buff))
            {
                buff[0]=0;
                get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
                if(buff[0]=='B')
                {
                    if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
                }
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
            uci_free_context(ctx);

            cli_print(cli, "OK");
            return CLI_OK;
        }

    }


    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_QoS", buff);
        int i=buff[0]-'A';
        switch (i) 
        {
        case 0:
            cli_print(cli, "+MCTP: 0  Normal");
            break;
        case 1:
            cli_print(cli, "+MCTP: 1  Medium");
            break;
        case 2:
            cli_print(cli, "+MCTP: 2  High");
            break;
        default:
            cli_print(cli, "+MCTP: Invalid Current Settings.");
            break;
        }

        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTP: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_ncdi(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTNCDI: Command Syntax:AT+MCTNCDI=<Mode>\n");
            cli_print(cli, "Mode Option:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable\n");
            cli_print(cli, "OK");

            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MCTNCDI: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        if (set_i>=0) 
        {
            struct uci_context *ctx;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }

            char buff[10];
            strcpy(buff,"A");
            buff[0]=buff[0]+set_i;
            if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_NoConnect_Data_Intake", buff))
            {
                buff[0]=0;
                get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
                if(buff[0]=='B')
                {
                    if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
                }
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
            uci_free_context(ctx);

            cli_print(cli, "OK");
            return CLI_OK;
        }
    }


    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_NoConnect_Data_Intake", buff);
        int i=buff[0]-'A';
        switch (i) 
        {
        case 0:
            cli_print(cli, "+MCTNCDI: 0  Disabled");
            break;
        case 1:
            cli_print(cli, "+MCTNCDI: 1  Enabled");
            break;
        default:
            cli_print(cli, "+MCTNCDI: Invalid Current Settings.");
            break;
        }

        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTNCDI: Invalid parameters");
    return CLI_OK;
}

int cmd_com2_mtc(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTMTC: Command Syntax:AT+MCTMTC=<Status>[,<Protection status>[,<Protection Key>]]\n");
            cli_print(cli, "Status and Protection Status:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MCTMTC: Invalid parameters");
            return CLI_OK;
        }
        int set_i=atoi(argv[1]);
        if (set_i>=0) 
        {
            struct uci_context *ctx;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }

            char buff[10];
            strcpy(buff,"A");
            buff[0]=buff[0]+set_i;
            if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_Modbus_Mode", buff))
            {
                buff[0]=0;
                get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
                if(buff[0]=='B')
                {
                    if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
                }
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
            uci_free_context(ctx);

            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if ((argc == 4 || argc == 3) && strcmp(argv[0], "=") == 0 ) 
    {

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MCTMTC: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),0,1))
        {
            cli_print(cli, "+MCTMTC: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        int set_i1=atoi(argv[2]);
        if (set_i>=0) 
        {
            struct uci_context *ctx;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }

            char buff[10];
            strcpy(buff,"A");
            buff[0]=buff[0]+set_i;
            char buff1[10];
            strcpy(buff1,"A");
            buff1[0]=buff1[0]+set_i1;
            if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_Modbus_Mode", buff))
            {
                set_option_by_section_name(ctx, "comport2", "com2", "COM2_Modbus_Protect_Status", buff1);
                if(argc==4)set_option_by_section_name(ctx, "comport2", "com2", "COM2_Modbus_Protect_Key", argv[3]);
                buff[0]=0;
                get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
                if(buff[0]=='B')
                {
                    if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
                }
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
            uci_free_context(ctx);

            cli_print(cli, "OK");
            return CLI_OK;
        }
    }


    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[20];
        char buff1[20],buff2[40];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_Modbus_Mode", buff);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_Modbus_Protect_Status", buff1);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_Modbus_Protect_Key", buff2);
        int i=buff[0]-'A';
        int j=buff1[0]-'A';
        switch (i) 
        {
        case 0:
            strcpy(buff,"0  Disabled");
            break;
        case 1:
            strcpy(buff,"1  Enabled");
            break;
        default:
            strcpy(buff,"?  Invalid");
            break;
        }
        switch (j) 
        {
        case 0:
            strcpy(buff1,"0  Disabled");
            break;
        case 1:
            strcpy(buff1,"1  Enabled");
            break;
        default:
            strcpy(buff1,"?  Invalid");
            break;
        }
        cli_print(cli, "+MCTMTC: %s\nProtect: %s\nProtect Key: %s ",buff,buff1,buff2);
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTMTC: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_ipm(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTIPM: Command Syntax:AT+MCTIPM=<Mode>\n");
            cli_print(cli, "Mode:\n");

            cli_print(cli, "0  TCP Client\n");
            cli_print(cli, "1  TCP Server\n");
            cli_print(cli, "2  TCP Client/Server\n");
            cli_print(cli, "3  UDP Point to Point\n");
            cli_print(cli, "4  UDP Point to Multipoint(P)\n");
            cli_print(cli, "5  UDP Point to Multipoint(MP)\n");
            cli_print(cli, "6  UDP Multipoint to Multipoin\n");
            cli_print(cli, "7  SMTP Client\n");
            cli_print(cli, "9  SMS Transparent Mode\n");
            cli_print(cli, "11  GPS Transparent Mode\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,11))
        {
            cli_print(cli, "+MCTIPM: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        if (set_i>=0) 
        {
            struct uci_context *ctx;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }

            char buff[10];
            strcpy(buff,"A");
            buff[0]=buff[0]+set_i;
            if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_IP_Protocol", buff))
            {
                buff[0]=0;
                get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
                if(buff[0]=='B')
                {
                    if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
                }
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
            uci_free_context(ctx);

            cli_print(cli, "OK");
            return CLI_OK;
        }
    }


    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_IP_Protocol", buff);
        int i=buff[0]-'A';
        switch (i) 
        {
        case 0:
            cli_print(cli, "+MCTIPM: 0  TCP Client");
            break;
        case 1:
            cli_print(cli, "+MCTIPM: 1  TCP Server");
            break;
        case 2:
            cli_print(cli, "+MCTIPM: 2  TCP Client/Server");
            break;
        case 3:
            cli_print(cli, "+MCTIPM: 3  UDP Point to Point");
            break;
        case 4:
            cli_print(cli, "+MCTIPM: 4  UDP Point to Multipoint(P)");
            break;
        case 5:
            cli_print(cli, "+MCTIPM: 5  UDP Point to Multipoint(MP)");
            break;
        case 6:
            cli_print(cli, "+MCTIPM: 6  UDP Multipoint to Multipoint");
            break;
        case 7:
            cli_print(cli, "+MCTIPM: 7  SMTP Client");
            break;
        case 9:
            cli_print(cli, "+MCTIPM: 9  SMS Transparent Mode");
            break;
        case 11:
            cli_print(cli, "+MCTIPM: 11  GPS Transparent Mode");
            break;
        default:
            cli_print(cli, "+MCTIPM: Invalid Current Settings.");
            break;
        }

        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTIPM: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_tcpclient(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTTC: Command Syntax:AT+MCTTC=<Remote Server IP>,<Remote Server Port>,<Outgoning timeout_s>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }
    if (argc == 4 && strcmp(argv[0], "=") == 0) 
    {
        if (false==IP_Address_check(argv[1],strlen(argv[1]))) 
        {
            cli_print(cli, "+MCTTC: Invalid parameter: IP Address %s",argv[1]);
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),1,65535))
        {
            cli_print(cli, "+MCTTC: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MCTTC: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Server_Addr", argv[1]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Server_Port", argv[2]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Timeout", argv[3]) )
        {
            char buff[10],buff1[10];
            buff[0]=0;
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_IP_Protocol", buff1);
            if(buff[0]=='B' && buff1[0]=='A')
            {
                if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Server_Addr", buff1);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Server_Port", buff2);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Timeout", buff3);
        cli_print(cli, "+MCTTC: \nRemote Server IP Address:%s \nRemote Server Port:%s \nOutgoing Connection Timeout:%s seconds",buff1,buff2,buff3);
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTTC: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_tcpserver(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTTS: Command Syntax:AT+MCTTS=<Local Listener Port>,<Connection timeout_s>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if (argc == 3 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),1,65535))
        {
            cli_print(cli, "+MCTTS: Invalid parameters: %s",argv[1]);
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),0,65535))
        {
            cli_print(cli, "+MCTTS: Invalid parameters: %s",argv[2]);
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Server_Listen_Port", argv[1]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Server_Timeout", argv[2]) )
        {
            char buff[10],buff1[10];
            buff[0]=0;
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_IP_Protocol", buff1);
            if(buff[0]=='B' && buff1[0]=='B')
            {
                if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff2[10],buff3[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Server_Listen_Port", buff2);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Server_Timeout", buff3);
        cli_print(cli, "+MCTTS: \nLocal Listening Port:%s \nIncoming Connection Timeout:%s seconds",buff2,buff3);
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTTS: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_tcpcs(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTTCS: Command Syntax:AT+MCTTCS=<Remote Server IP>,<Remote Server Port>,<Outgoning timeout_s>,<Local Listener Port>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }
    if (argc == 5 && strcmp(argv[0], "=") == 0) 
    {
        if (false==IP_Address_check(argv[1],strlen(argv[1]))) 
        {
            cli_print(cli, "+MCTTCS: Invalid parameter: IP Address %s",argv[1]);
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),1,65535))
        {
            cli_print(cli, "+MCTTCS: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MCTTCS: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[4], strlen(argv[4]),1,65535))
        {
            cli_print(cli, "+MCTTCS: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Server_Addr", argv[1]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Server_Port", argv[2]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Timeout", argv[3]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Server_Listen_Port", argv[4]) )
        {
            char buff[10],buff1[10];
            buff[0]=0;
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_IP_Protocol", buff1);
            if(buff[0]=='B' && buff1[0]=='C')
            {
                if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10],buff4[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Server_Addr", buff1);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Server_Port", buff2);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Client_Timeout", buff3);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_T_Server_Listen_Port", buff4);
        cli_print(cli, "+MCTTCS: \nRemote Server IP Address:%s \nRemote Server Port:%s \nOutgoing Connection Timeout:%s seconds \nLocal Listening Port:%s",buff1,buff2,buff3,buff4);
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTTCS: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_upp(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTUPP: Command Syntax:AT+MCTUPP=<Remote IP>,<Remote Port>,<Listener Port>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }
    if (argc == 4 && strcmp(argv[0], "=") == 0) 
    {
        if (false==IP_Address_check(argv[1],strlen(argv[1]))) 
        {
            cli_print(cli, "+MCTUPP: Invalid parameter: IP Address %s",argv[1]);
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),1,65535))
        {
            cli_print(cli, "+MCTUPP: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),1,65535))
        {
            cli_print(cli, "+MCTUPP: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_U_PtoP_Remote_Addr", argv[1]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_U_PtoP_Remote_Port", argv[2]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_U_PtoP_Listen_Port", argv[3]) )
        {
            char buff[10],buff1[10];
            buff[0]=0;
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_IP_Protocol", buff1);
            if(buff[0]=='B' && buff1[0]=='D')
            {
                if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_U_PtoP_Remote_Addr", buff1);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_U_PtoP_Remote_Port", buff2);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_U_PtoP_Listen_Port", buff3);
        cli_print(cli, "+MCTUPP: \nRemote IP Address:%s \nRemote Port:%s \nListening Port:%s",buff1,buff2,buff3);
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTUPP: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_upmp(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTUPMP: Command Syntax:AT+MCTUPMP=<Multicast IP>,<Multicast Port>,<Listener Port>,<Time to live>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if (argc == 5 && strcmp(argv[0], "=") == 0) 
    {

        if (false==IP_Address_check(argv[1],strlen(argv[1]))) 
        {
            cli_print(cli, "+MCTUPMP: Invalid parameter: IP Address %s",argv[1]);
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),1,65535))
        {
            cli_print(cli, "+MCTUPMP: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),1,65535))
        {
            cli_print(cli, "+MCTUPMP: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[4], strlen(argv[4]),0,65535))
        {
            cli_print(cli, "+MCTUPMP: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_P_Multicast_Addr", argv[1]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_P_Multicast_Port", argv[2]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_P_Listen_Port", argv[3]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_P_TTL", argv[4]) )
        {
            char buff[10],buff1[10];
            buff[0]=0;
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_IP_Protocol", buff1);
            if(buff[0]=='B' && buff1[0]=='E')
            {
                if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10],buff4[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_P_Multicast_Addr", buff1);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_P_Multicast_Port", buff2);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_P_Listen_Port", buff3);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_P_TTL", buff4);
        cli_print(cli, "+MCTUPMP: \nMulticast IP Address:%s \nMulticast Port:%s \nListening Port:%s \nTime to Live:%s",buff1,buff2,buff3,buff4);
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTUPMP: Invalid parameters");
    return CLI_OK;
}

int cmd_com2_upmm(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTUPMM: Command Syntax:AT+MCTUPMM=<Remote IP>,<Remote Port>,<Multicast IP>,<Multicast Port>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if (argc == 5 && strcmp(argv[0], "=") == 0) 
    {
        if (false==IP_Address_check(argv[1],strlen(argv[1]))) 
        {
            cli_print(cli, "+MCTUPMM: Invalid parameter: IP Address %s",argv[1]);
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),1,65535))
        {
            cli_print(cli, "+MCTUPMM: Invalid parameters");
            return CLI_OK;
        }
        if (false==IP_Address_check(argv[3],strlen(argv[3]))) 
        {
            cli_print(cli, "+MCTUPMM: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[4], strlen(argv[4]),1,65535))
        {
            cli_print(cli, "+MCTUPMM: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_M_Remote_Addr", argv[1]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_M_Remote_Port", argv[2]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_M_Multicast_Addr", argv[3]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_M_Multicast_Port", argv[4]) )
        {
            char buff[10],buff1[10];
            buff[0]=0;
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_IP_Protocol", buff1);
            if(buff[0]=='B' && buff1[0]=='F')
            {
                if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[30],buff4[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_M_Remote_Addr", buff1);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_M_Remote_Port", buff2);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_M_Multicast_Addr", buff3);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UM_M_Multicast_Port", buff4);
        cli_print(cli, "+MCTUPMM: \nRemote IP Address:%s \nRemote Port:%s \nMulticast IP Address:%s \nMulticast Port:%s",buff1,buff2,buff3,buff4);
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTUPMM: Invalid parameters");
    return CLI_OK;
}


int cmd_com2_umpmp(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCTUMPMP: Command Syntax:AT+MCTUMPMP=<Multicast IP>,<Multicast Port>,<Time to live>,<Listen Multicast IP>,<Listen Multicast Port>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }
    if (argc == 6 && strcmp(argv[0], "=") == 0) 
    {
        if (false==IP_Address_check(argv[1],strlen(argv[1]))) 
        {
            cli_print(cli, "+MCTUMPMP: Invalid parameter: IP Address %s",argv[1]);
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),1,65535))
        {
            cli_print(cli, "+MCTUMPMP: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MCTUMPMP: Invalid parameters");
            return CLI_OK;
        }
        if (false==IP_Address_check(argv[4],strlen(argv[4]))) 
        {
            cli_print(cli, "+MCTUMPMP: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[5], strlen(argv[5]),1,65535))
        {
            cli_print(cli, "+MCTUMPMP: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        if(set_option_by_section_name(ctx, "comport2", "com2", "COM2_UMtoM_Multicast_Addr", argv[1]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_UMtoM_Multicast_Port", argv[2]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_UMtoM_Multicast_TTL", argv[3]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_UMtoM_Listen_Multicast_Addr", argv[4]) \
           && set_option_by_section_name(ctx, "comport2", "com2", "COM2_UMtoM_Listen_Multicast_Port", argv[5]) )
        {
            char buff[10],buff1[10];
            buff[0]=0;
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_Port_Status", buff);
            get_option_by_section_name(ctx, "comport2", "com2", "COM2_IP_Protocol", buff1);
            if(buff[0]=='B' && buff1[0]=='G')
            {
                if(SubProgram_Start("/etc/init.d/soip2","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[30],buff4[10],buff[10];
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UMtoM_Multicast_Addr", buff1);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UMtoM_Multicast_Port", buff2);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UMtoM_Multicast_TTL", buff);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UMtoM_Listen_Multicast_Addr", buff3);
        get_option_by_section_name(ctx, "comport2", "com2", "COM2_UMtoM_Listen_Multicast_Port", buff4);
        cli_print(cli, "+MCTUMPMP: \nMulticast IP Address:%s \nMulticast Port:%s \nTime To Live:%s \nListen Multicast IP Address:%s \nListen Multicast Port:%s",buff1,buff2,buff,buff3,buff4);
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MCTUMPMP: Invalid parameters");
    return CLI_OK;
}


int cmd_eurd_1(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MEURD1: Command Syntax:AT+MEURD1=<Mode>[,<Remote IP>,<Remote Port>,<Interval Time_s>]\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Moden Event Report\n");
            cli_print(cli, "2     SDP Event Report\n");
            cli_print(cli, "3     Management Report\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,3))
        {
            cli_print(cli, "+MEURD1: Invalid parameters");
            return CLI_OK;
        }
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        last_set=buff[0];
        buff[0]='A'+set_i;
        if(last_set!=buff[0])
        {
            if(set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff))
            {
                if(SubProgram_Start("/etc/init.d/eurd","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;

    }

    if (argc == 5 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,3))
        {
            cli_print(cli, "+MEURD1: Invalid parameters");
            return CLI_OK;
        }
        if (false==IP_Address_check(argv[2],strlen(argv[2]))) 
        {
            cli_print(cli, "+MEURD1: Invalid parameter: IP Address %s",argv[2]);
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MEURD1: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[4], strlen(argv[4]),0,65535))
        {
            cli_print(cli, "+MEURD1: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        last_set=buff[0];
        buff[0]='A'+set_i;
        if(set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff) \
            && set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_IP_address0", argv[2]) \
            && set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_PORT0", argv[3]) \
            && set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Timer0", argv[4]) )
        {
            if(last_set!=buff[0] || last_set>'A')
            {
                if(SubProgram_Start("/etc/init.d/eurd","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10],buff[10];
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_IP_address0", buff1);
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_PORT0", buff2);
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Timer0", buff3);
        switch(buff[0])
        {
        case 'A':
            cli_print(cli, "+MEURD1: \nMode: Disabled \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'B':
            cli_print(cli, "+MEURD1: \nMode: Modem Event Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'C':
            cli_print(cli, "+MEURD1: \nMode: SDP Event Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'D':
            cli_print(cli, "+MEURD1: \nMode: Management Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        default:
            cli_print(cli, "+MEURD1: Invalid Current Settings.");
            break;
        }
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MEURD1: Invalid parameters");
    return CLI_OK;
}

int cmd_eurd_2(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MEURD2: Command Syntax:AT+MEURD2=<Mode>[,<Remote IP>,<Remote Port>,<Interval Time_s>]\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Moden Event Report\n");
            cli_print(cli, "2     SDP Event Report\n");
            cli_print(cli, "3     Management Report\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,3))
        {
            cli_print(cli, "+MEURD2: Invalid parameters");
            return CLI_OK;
        }
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        last_set=buff[1];
        buff[1]='A'+set_i;
        if(last_set!=buff[1])
        {
            if(set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff))
            {
                if(SubProgram_Start("/etc/init.d/eurd","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;

    }

    if (argc == 5 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,3))
        {
            cli_print(cli, "+MEURD2: Invalid parameters");
            return CLI_OK;
        }
        if (false==IP_Address_check(argv[2],strlen(argv[2]))) 
        {
            cli_print(cli, "+MEURD2: Invalid parameter: IP Address %s",argv[2]);
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MEURD2: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[4], strlen(argv[4]),0,65535))
        {
            cli_print(cli, "+MEURD2: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        last_set=buff[1];
        buff[1]='A'+set_i;
        if(set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff) \
            && set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_IP_address1", argv[2]) \
            && set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_PORT1", argv[3]) \
            && set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Timer1", argv[4]) )
        {
            if(last_set!=buff[1] || last_set>'A')
            {
                if(SubProgram_Start("/etc/init.d/eurd","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10],buff[10];
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_IP_address1", buff1);
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_PORT1", buff2);
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Timer1", buff3);
        switch(buff[1])
        {
        case 'A':
            cli_print(cli, "+MEURD2: \nMode: Disabled \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'B':
            cli_print(cli, "+MEURD2: \nMode: Modem Event Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'C':
            cli_print(cli, "+MEURD2: \nMode: SDP Event Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'D':
            cli_print(cli, "+MEURD2: \nMode: Management Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        default:
            cli_print(cli, "+MEURD2: Invalid Current Settings.");
            break;
        }
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MEURD2: Invalid parameters");
    return CLI_OK;
}


int cmd_eurd_3(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MEURD3: Command Syntax:AT+MEURD3=<Mode>[,<Remote IP>,<Remote Port>,<Interval Time_s>]\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Moden Event Report\n");
            cli_print(cli, "2     SDP Event Report\n");
            cli_print(cli, "3     Management Report\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,3))
        {
            cli_print(cli, "+MEURD3: Invalid parameters");
            return CLI_OK;
        }
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        last_set=buff[2];
        buff[2]='A'+set_i;
        if(last_set!=buff[2])
        {
            if(set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff))
            {
                if(SubProgram_Start("/etc/init.d/eurd","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;

    }

    if (argc == 5 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,3))
        {
            cli_print(cli, "+MEURD3: Invalid parameters");
            return CLI_OK;
        }
        if (false==IP_Address_check(argv[2],strlen(argv[2]))) 
        {
            cli_print(cli, "+MEURD3: Invalid parameter: IP Address %s",argv[2]);
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MEURD3: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[4], strlen(argv[4]),0,65535))
        {
            cli_print(cli, "+MEURD3: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        last_set=buff[2];
        buff[2]='A'+set_i;
        if(set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff) \
            && set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_IP_address2", argv[2]) \
            && set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_PORT2", argv[3]) \
            && set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Timer2", argv[4]) )
        {
            if(last_set!=buff[2] || last_set>'A')
            {
                if(SubProgram_Start("/etc/init.d/eurd","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10],buff[10];
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_IP_address2", buff1);
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_PORT2", buff2);
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Timer2", buff3);
        switch(buff[2])
        {
        case 'A':
            cli_print(cli, "+MEURD3: \nMode: Disabled \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'B':
            cli_print(cli, "+MEURD3: \nMode: Modem Event Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'C':
            cli_print(cli, "+MEURD3: \nMode: SDP Event Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'D':
            cli_print(cli, "+MEURD3: \nMode: Management Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        default:
            cli_print(cli, "+MEURD3: Invalid Current Settings.");
            break;
        }
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MEURD3: Invalid parameters");
    return CLI_OK;
}


int cmd_nms_r(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MNMSR: Command Syntax:AT+MNMSR=<Mode>[,<Remote Port>,<Interval Time_s>]\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable NMS Report\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MNMSR: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        last_set=buff[3];
        if(set_i==0)buff[3]='A';
        else buff[3]='D';

        if(last_set!=buff[3])
        {
            if(set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff))
            {
                if(SubProgram_Start("/etc/init.d/eurd","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;

    }

    if (argc == 4 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MNMSR: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),0,65535))
        {
            cli_print(cli, "+MNMSR: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MNMSR: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        last_set=buff[3];
        if(set_i==0)buff[3]='A';
        else buff[3]='D';
        if(set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff) \
            && set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_PORT3", argv[2]) \
            && set_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Timer3", argv[3]) )
        {
            if(last_set!=buff[3] || last_set>'A')
            {
                if(SubProgram_Start("/etc/init.d/eurd","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10],buff[10];
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_Reporting_Status", buff);
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Remote_PORT3", buff2);
        get_option_by_section_name(ctx, "eurd", "eur_conf", "Event_Timer3", buff3);
        switch(buff[3])
        {
        case 'A':
            cli_print(cli, "+MNMSR: \nMode: Disabled \nRemote Port:%s \nInterval:%s seconds",buff2,buff3);
            break;
        case 'D':
            cli_print(cli, "+MNMSR: \nMode: NMS Report Enabled \nRemote Port:%s \nInterval:%s seconds",buff2,buff3);
            break;
        default:
            cli_print(cli, "+MNMSR: Invalid Current Settings.");
            break;
        }
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MNMSR: Invalid parameters");
    return CLI_OK;
}


int cmd_gpsr_1(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MGPSR1: Command Syntax:AT+MGPSR1=<Mode>[,<Remote IP>,<Remote Port>,<Interval Time_s>]\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable UDP Report\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MGPSR1: Invalid parameters");
            return CLI_OK;
        }
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        last_set=buff[0];
        buff[0]='A'+set_i;
        if(last_set!=buff[0])
        {
            if(set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff))
            {
                if(SubProgram_Start("/etc/init.d/gpsr","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 5 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MGPSR1: Invalid parameters");
            return CLI_OK;
        }
        if (false==IP_Address_check(argv[2],strlen(argv[2]))) 
        {
            cli_print(cli, "+MGPSR1: Invalid parameter: IP Address %s",argv[2]);
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MGPSR1: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[4], strlen(argv[4]),0,65535))
        {
            cli_print(cli, "+MGPSR1: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        last_set=buff[0];
        buff[0]='A'+set_i;
        if(set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_IP_address0", argv[2]) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_PORT0", argv[3]) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Timer0", argv[4]) )
        {
            if(last_set!=buff[0] || last_set>'A')
            {
                if(SubProgram_Start("/etc/init.d/gpsr","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10],buff[10];
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_IP_address0", buff1);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_PORT0", buff2);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Timer0", buff3);
        switch(buff[0])
        {
        case 'A':
            cli_print(cli, "+MGPSR1: \nMode: Disabled \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'B':
            cli_print(cli, "+MGPSR1: \nMode: UDP Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'C':
            cli_print(cli, "+MGPSR1: \nMode: Email Report \nPlease config via web client",buff1,buff2,buff3);
            break;
        default:
            cli_print(cli, "+MGPSR1: Invalid Current Settings.");
            break;
        }
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MGPSR1: Invalid parameters");
    return CLI_OK;
}



int cmd_gpsr_2(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MGPSR2: Command Syntax:AT+MGPSR2=<Mode>[,<Remote IP>,<Remote Port>,<Interval Time_s>]\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable UDP Report\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MGPSR2: Invalid parameters");
            return CLI_OK;
        }
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        last_set=buff[1];
        buff[1]='A'+set_i;
        if(last_set!=buff[1])
        {
            if(set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff))
            {
                if(SubProgram_Start("/etc/init.d/gpsr","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 5 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MGPSR2: Invalid parameters");
            return CLI_OK;
        }
        if (false==IP_Address_check(argv[2],strlen(argv[2]))) 
        {
            cli_print(cli, "+MGPSR2: Invalid parameter: IP Address %s",argv[2]);
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MGPSR2: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[4], strlen(argv[4]),0,65535))
        {
            cli_print(cli, "+MGPSR2: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        last_set=buff[1];
        buff[1]='A'+set_i;
        if(set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_IP_address1", argv[2]) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_PORT1", argv[3]) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Timer1", argv[4]) )
        {
            if(last_set!=buff[1] || last_set>'A')
            {
                if(SubProgram_Start("/etc/init.d/gpsr","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10],buff[10];
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_IP_address1", buff1);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_PORT1", buff2);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Timer1", buff3);
        switch(buff[1])
        {
        case 'A':
            cli_print(cli, "+MGPSR2: \nMode: Disabled \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'B':
            cli_print(cli, "+MGPSR2: \nMode: UDP Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'C':
            cli_print(cli, "+MGPSR2: \nMode: Email Report \nPlease config via web client",buff1,buff2,buff3);
            break;
        default:
            cli_print(cli, "+MGPSR2: Invalid Current Settings.");
            break;
        }
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MGPSR2: Invalid parameters");
    return CLI_OK;
}



int cmd_gpsr_3(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MGPSR3: Command Syntax:AT+MGPSR3=<Mode>[,<Remote IP>,<Remote Port>,<Interval Time_s>]\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable UDP Report\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MGPSR3: Invalid parameters");
            return CLI_OK;
        }
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        last_set=buff[2];
        buff[2]='A'+set_i;
        if(last_set!=buff[2])
        {
            if(set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff))
            {
                if(SubProgram_Start("/etc/init.d/gpsr","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 5 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MGPSR3: Invalid parameters");
            return CLI_OK;
        }
        if (false==IP_Address_check(argv[2],strlen(argv[2]))) 
        {
            cli_print(cli, "+MGPSR3: Invalid parameter: IP Address %s",argv[2]);
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MGPSR3: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[4], strlen(argv[4]),0,65535))
        {
            cli_print(cli, "+MGPSR3: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        last_set=buff[2];
        buff[2]='A'+set_i;
        if(set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_IP_address2", argv[2]) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_PORT2", argv[3]) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Timer2", argv[4]) )
        {
            if(last_set!=buff[2] || last_set>'A')
            {
                if(SubProgram_Start("/etc/init.d/gpsr","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10],buff[10];
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_IP_address2", buff1);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_PORT2", buff2);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Timer2", buff3);
        switch(buff[2])
        {
        case 'A':
            cli_print(cli, "+MGPSR3: \nMode: Disabled \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'B':
            cli_print(cli, "+MGPSR3: \nMode: UDP Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'C':
            cli_print(cli, "+MGPSR3: \nMode: Email Report \nPlease config via web client",buff1,buff2,buff3);
            break;
        default:
            cli_print(cli, "+MGPSR3: Invalid Current Settings.");
            break;
        }
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MGPSR3: Invalid parameters");
    return CLI_OK;
}


int cmd_gpsr_4(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MGPSR4: Command Syntax:AT+MGPSR4=<Mode>[,<Remote IP>,<Remote Port>,<Interval Time_s>]\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable UDP Report\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MGPSR4: Invalid parameters");
            return CLI_OK;
        }
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        last_set=buff[3];
        buff[3]='A'+set_i;
        if(last_set!=buff[3])
        {
            if(set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff))
            {
                if(SubProgram_Start("/etc/init.d/gpsr","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 5 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MGPSR4: Invalid parameters");
            return CLI_OK;
        }
        if (false==IP_Address_check(argv[2],strlen(argv[2]))) 
        {
            cli_print(cli, "+MGPSR4: Invalid parameter: IP Address %s",argv[2]);
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,65535))
        {
            cli_print(cli, "+MGPSR4: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[4], strlen(argv[4]),0,65535))
        {
            cli_print(cli, "+MGPSR4: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        int set_i=atoi(argv[1]);
        char buff[10],last_set;
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        last_set=buff[3];
        buff[3]='A'+set_i;
        if(set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_IP_address3", argv[2]) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_PORT3", argv[3]) \
            && set_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Timer3", argv[4]) )
        {
            if(last_set!=buff[3] || last_set>'A')
            {
                if(SubProgram_Start("/etc/init.d/gpsr","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }
        }else
        {
            cli_print(cli, "Not succeed.\n");
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff1[30],buff2[10],buff3[10],buff[10];
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_Reporting_Status", buff);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_IP_address3", buff1);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Remote_PORT3", buff2);
        get_option_by_section_name(ctx, "gpsr", "gpsr_conf", "AGCR_Timer3", buff3);
        switch(buff[3])
        {
        case 'A':
            cli_print(cli, "+MGPSR4: \nMode: Disabled \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'B':
            cli_print(cli, "+MGPSR4: \nMode: UDP Report \nRemote IP:%s \nRemote Port:%s \nInterval:%s seconds",buff1,buff2,buff3);
            break;
        case 'C':
            cli_print(cli, "+MGPSR4: \nMode: Email Report \nPlease config via web client",buff1,buff2,buff3);
            break;
        default:
            cli_print(cli, "+MGPSR4: Invalid Current Settings.");
            break;
        }
        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MGPSR4: Invalid parameters");
    return CLI_OK;
}


int cmd_ip_pass(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MPIPP: Command Syntax:AT+MPIPP=<Mode>\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0  Disable\n");
            cli_print(cli, "1  Ethernet\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MPIPP: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        if (set_i>=0 && set_i<=1) 
        {
            struct uci_context *ctx;
            ctx = uci_alloc_context();  // for VIP4G
            if (!ctx) 
            {
                cli_print(cli,"ERROR: Out of memory\n");
                return CLI_OK;
            }

            char buff[20];
            int last_set=2;
            get_option_by_section_name(ctx, "lte", "connect", "ippassthrough", buff);
            if(strcmp(buff,"Disable")==0)last_set=0;
            if(strcmp(buff,"Ethernet")==0)last_set=1;

            if(set_i==0)strcpy(buff,"Disable");
            else strcpy(buff,"Ethernet");
            if(set_option_by_section_name(ctx, "lte", "connect", "ippassthrough", buff))
            {
                buff[0]=0;
                get_option_by_section_name(ctx, "lte", "connect", "disabled", buff);
                if(buff[0]=='0' && last_set!=set_i)
                {
                    SubProgram_Start("/etc/init.d/lte","stop");
                    SubProgram_Start("/etc/init.d/lte","start");
                }
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
            uci_free_context(ctx);

            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        char buff[20];
        get_option_by_section_name(ctx, "lte", "connect", "ippassthrough", buff);
        int i=2;
        if(strcmp(buff,"Disable")==0)i=0;
        if(strcmp(buff,"Ethernet")==0)i=1;
        switch (i) 
        {
        case 0:
            cli_print(cli, "+MPIPP: 0  Disable");
            break;
        case 1:
            cli_print(cli, "+MPIPP: 1  Ethernet");
            break;
        default:
            cli_print(cli, "+MPIPP: Invalid Current Settings.");
            break;
        }

        cli_print(cli, "\nOK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MPIPP: Invalid parameters");
    return CLI_OK;
}


int cmd_console_timeout(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MCNTO: Command Syntax:AT+MCNTO=<Timeout_s>\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,65535))
        {
            cli_print(cli, "+MCNTO: Invalid parameters");
            return CLI_OK;
        }

        int set_i=atoi(argv[1]);
        at_parameter_timeout=set_i;
        at_flag=3;

        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0], "?") == 0) 
    {

        cli_print(cli, "+MCNTO: %d seconds\n", at_parameter_timeout);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    cli_print(cli, "+MCNTO: Invalid parameters");

    return CLI_OK;
}


int cmd_sms_command(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    int set_i=atoi(argv[1]);
    char buff[20],buff1[20],last_set;
    char buff_phone1[20],buff_phone2[20],buff_phone3[20],buff_phone4[20],buff_phone5[20],buff_phone6[20];

    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MSCMD: Command Syntax:AT+MSCMD=<Mode>[,<Filter Mode>[,<Phone No.1>[,...,<Phone No.6>]]]\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable SMS Command\n");
            cli_print(cli, "Filter Mode:\n");
            cli_print(cli, "0     Disable\n");
            cli_print(cli, "1     Enable Phone Filter\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MSCMD: Invalid parameters");
            return CLI_OK;
        }
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        set_i=atoi(argv[1]);
        get_option_by_section_name(ctx, "msmscomd", "msms_conf", "System_SMS_Commad", buff);
        last_set=buff[0];
        buff[0]='A'+set_i;
        if(last_set!=buff[0])
        {
            buff[1]=0;
            if(set_option_by_section_name(ctx, "msmscomd", "msms_conf", "System_SMS_Commad", buff))
            {
                if(SubProgram_Start("/etc/init.d/msmscomd","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc >2 && argc<10  && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),0,1))
        {
            cli_print(cli, "+MSCMD: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),0,1))
        {
            cli_print(cli, "+MSCMD: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        set_i=atoi(argv[1]);
        get_option_by_section_name(ctx, "msmscomd", "msms_conf", "System_SMS_Commad", buff);
        last_set=buff[0];
        buff[0]='A'+set_i;
        buff[1]=0;
        set_option_by_section_name(ctx, "msmscomd", "msms_conf", "System_SMS_Commad", buff);

        set_i=atoi(argv[2]);
        buff1[0]='A'+set_i;
        buff1[1]=0;
        set_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Set", buff1);


        if(argc>2)
        {
            buff1[0]=0;
            if(argc>3)
            {
                if(strlen(argv[3])>3)
                {
                    strcpy(buff1,argv[3]);
                }
            }
            set_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone1", buff1);

            buff1[0]=0;
            if(argc>4)
            {
                if(strlen(argv[4])>3)
                {
                    strcpy(buff1,argv[4]);
                }
            }
            set_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone2", buff1);

            buff1[0]=0;
            if(argc>5)
            {
                if(strlen(argv[5])>3)
                {
                    strcpy(buff1,argv[5]);
                }
            }
            set_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone3", buff1);

            buff1[0]=0;
            if(argc>6)
            {
                if(strlen(argv[6])>3)
                {
                    strcpy(buff1,argv[6]);
                }
            }
            set_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone4", buff1);

            buff1[0]=0;
            if(argc>7)
            {
                if(strlen(argv[7])>3)
                {
                    strcpy(buff1,argv[7]);
                }
            }
            set_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone5", buff1);

            buff1[0]=0;
            if(argc>8)
            {
                if(strlen(argv[8])>3)
                {
                    strcpy(buff1,argv[8]);
                }
            }
            set_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone6", buff1);
        }

        if(last_set!=buff[0] || last_set=='B')
        {
            if(SubProgram_Start("/etc/init.d/msmscomd","restart")==false)cli_print(cli, "Program restart fault, Please reboot to effect new settings.\n");
        }

        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        get_option_by_section_name(ctx, "msmscomd", "msms_conf", "System_SMS_Commad", buff);
        get_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Set", buff1);
        get_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone1", buff_phone1);
        get_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone2", buff_phone2);
        get_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone3", buff_phone3);
        get_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone4", buff_phone4);
        get_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone5", buff_phone5);
        get_option_by_section_name(ctx, "msmscomd", "msms_conf", "SMS_CMD_Filter_Phone6", buff_phone6);
        if(buff1[0]=='B')strcpy(buff1,"1  Enabled");
        else strcpy(buff1,"0  Disabled");
        switch(buff[0])
        {
        case 'A':
            cli_print(cli, "+MSCMD: \nMode:0  Disabled \nFilter Mode:%s\nFilter Phone1:%s\nFilter Phone2:%s\nFilter Phone3:%s\nFilter Phone4:%s\nFilter Phone5:%s\nFilter Phone6:%s\n",buff1,buff_phone1,buff_phone2,buff_phone3,buff_phone4,buff_phone5,buff_phone6);
            break;
        case 'B':
            cli_print(cli, "+MSCMD: \nMode:1  Enabled \nFilter Mode:%s\nFilter Phone1:%s\nFilter Phone2:%s\nFilter Phone3:%s\nFilter Phone4:%s\nFilter Phone5:%s\nFilter Phone6:%s\n",buff1,buff_phone1,buff_phone2,buff_phone3,buff_phone4,buff_phone5,buff_phone6);
            break;
        default:
            cli_print(cli, "+MSCMD: Invalid Current Settings.");
            break;
        }
        cli_print(cli, "OK");
        uci_free_context(ctx);
        return CLI_OK;
    }

    cli_print(cli, "+MSCMD: Invalid parameters");
    return CLI_OK;

}


int cmd_input_s(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MIS: Command Syntax:AT+MIS\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }
    }

    if (argc == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        cli_print(cli, "+MIS: avaliable input status\n");
        char buff[10];
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "input", "input1", buff);
        if(buff[0]=='1') cli_print(cli, "INPUT 1: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "INPUT 1: 0    open\n");
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "input", "input2", buff);
        if(buff[0]=='1') cli_print(cli, "INPUT 2: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "INPUT 2: 0    open\n");
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "input", "input3", buff);
        if(buff[0]=='1') cli_print(cli, "INPUT 3: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "INPUT 3: 0    open\n");
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "input", "input4", buff);
        if(buff[0]=='1') cli_print(cli, "INPUT 4: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "INPUT 4: 0    open\n");

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    cli_print(cli, "+MIS: Invalid parameters");
    return CLI_OK;
}


int cmd_output_s(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    if (argc == 2 && strcmp(argv[0], "=") == 0) 
    {
        if (strcmp(argv[1], "?") == 0) 
        {
            cli_print(cli, "+MOS: Command Syntax:AT+MOS=<Mode>[,<Setting No.>,<Status>]\n");
            cli_print(cli, "Mode:\n");
            cli_print(cli, "0  All Output Status\n");
            cli_print(cli, "1  Output Setting\n");
            cli_print(cli, "Setting No.: 1, 2, 3, 4(if output avaliable)\n");
            cli_print(cli, "Status:\n0  open\n1  close\n");
            cli_print(cli, "OK");
            return CLI_OK;
        }

        if(!digit_check(argv[1], strlen(argv[1]),0,0))
        {
            cli_print(cli, "+MOS: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        cli_print(cli, "+MOS: avaliable output status\n");
        char buff[10];
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "output", "output1", buff);
        if(buff[0]=='1') cli_print(cli, "OUTPUT 1: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "OUTPUT 1: 0    open\n");
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "output", "output2", buff);
        if(buff[0]=='1') cli_print(cli, "OUTPUT 2: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "OUTPUT 2: 0    open\n");
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "output", "output3", buff);
        if(buff[0]=='1') cli_print(cli, "OUTPUT 3: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "OUTPUT 3: 0    open\n");
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "output", "output4", buff);
        if(buff[0]=='1') cli_print(cli, "OUTPUT 4: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "OUTPUT 4: 0    open\n");

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 4 && strcmp(argv[0], "=") == 0) 
    {
        if(!digit_check(argv[1], strlen(argv[1]),1,1))
        {
            cli_print(cli, "+MOS: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[2], strlen(argv[2]),1,4))
        {
            cli_print(cli, "+MOS: Invalid parameters");
            return CLI_OK;
        }
        if(!digit_check(argv[3], strlen(argv[3]),0,1))
        {
            cli_print(cli, "+MOS: Invalid parameters");
            return CLI_OK;
        }

        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }
        char buff[10],str_buff[10];
        sprintf(str_buff,"output%s",argv[2]);
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "output", str_buff, buff);
        if(buff[0]!=argv[3][0])
        {
            if(set_option_by_section_name(ctx,"ioports", "output", str_buff, argv[3]))
            {
                SubProgram_Start("/etc/init.d/ioports","stop");
                SubProgram_Start("/etc/init.d/ioports","start");
            }else
            {
                cli_print(cli, "Not succeed.\n");
            }
        }
        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    if (argc == 1 && strcmp(argv[0],"?") == 0) 
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            cli_print(cli,"ERROR: Out of memory\n");
            return CLI_OK;
        }

        cli_print(cli, "+MOS: avaliable output status\n");
        char buff[10];
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "output", "output1", buff);
        if(buff[0]=='1') cli_print(cli, "OUTPUT 1: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "OUTPUT 1: 0    open\n");
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "output", "output2", buff);
        if(buff[0]=='1') cli_print(cli, "OUTPUT 2: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "OUTPUT 2: 0    open\n");
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "output", "output3", buff);
        if(buff[0]=='1') cli_print(cli, "OUTPUT 3: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "OUTPUT 3: 0    open\n");
        buff[0]=0;
        get_option_by_section_name(ctx, "ioports", "output", "output4", buff);
        if(buff[0]=='1') cli_print(cli, "OUTPUT 4: 1    close\n");
        else if(buff[0]=='0') cli_print(cli, "OUTPUT 4: 0    open\n");

        uci_free_context(ctx);
        cli_print(cli, "OK");
        return CLI_OK;
    }

    cli_print(cli, "+MOS: Invalid parameters");
    return CLI_OK;
}


int cmd_idle_timeout(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    cli_print(cli, "+MPITO: Not supported in 4G moduels.\n");
    return CLI_OK;
}

int cmd_connect_timeout(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    cli_print(cli, "+MPCTO: Not supported in 4G moduels.\n");
    return CLI_OK;
}

int cmd_nat_conf(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    cli_print(cli, "+MNAT: Not supported in 4G moduels.\n");
    return CLI_OK;
}
int cmd_ppp_status(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    cli_print(cli, "+MPPPS: Not supported in 4G moduels.\n");
    return CLI_OK;
}

int cmd_dod_conf(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    cli_print(cli, "+MDOD: Not supported in 4G moduels.\n");
    return CLI_OK;
}
int cmd_default_button(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    cli_print(cli, "+MSDBE: Not supported in 4G moduels.\n");
    return CLI_OK;
}
int cmd_auth_conf(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    cli_print(cli, "+MAUTH: Not supported in 4G moduels.\n");
    return CLI_OK;
}

int cmd_not_defined(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    cli_print(cli, "Not defined in 4G moduels.\n");
    return CLI_OK;
}



