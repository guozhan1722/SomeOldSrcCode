#include "msmslib.h"

struct uci_context *ctx = NULL;   //need to initialize in main()

char *RadioBusyFlagFile="/var/run/VIP4G_STAT_busy";
char *CmdHistoryFile="/etc/smscmd.history";
char *CmdHistoryFilebak="/etc/smscmd.historybak";
char *SendHistoryFile="/etc/smssend.history";
char *SendHistoryFilebak="/etc/smssend.historybak";
char *SAlertHistoryFile="/etc/smsalert.history";
char *SAlertHistoryFilebak="/etc/smsalert.historybak";
char *ReadsmsFile="/var/run/msmsread";
char *SendsmsFile="/var/run/msmssend";
struct gn_statemachine *state;
gn_data *data;
int smtotal, used;
char strOut[1024];
int fd_radiobusy;
int smsread_state;


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


void busterminate(void)
{
    gn_lib_phone_close(state);
    gn_lib_phoneprofile_free(&state);
    gn_lib_library_free();

}

void businit(void)
{
    gn_error err;
    char *configfile = NULL;
    char *configmodel = NULL;
    if ((err = gn_lib_phoneprofile_load_from_file(configfile, configmodel, &state)) != GN_ERR_NONE) {
        syslog(LOGOPTS, "[%s] %s\n", __func__, gn_error_print(err));
        syslog(LOGOPTS, "[%s]  >>> gn_lib_phoneprofile_load_from_file() return with fatal error,  program exit(2). <<<\n", __func__);
        exit(2);
    }

    /* register cleanup function */
    //atexit(busterminate);
    /* signal(SIGINT, bussignal); */

    if ((err = gn_lib_phone_open(state)) != GN_ERR_NONE) {
        syslog(LOGOPTS, "[%s] %s\n", __func__, gn_error_print(err));
        syslog(LOGOPTS, "[%s]  >>> gn_lib_phone_open() return with fatal error,  program exit(2). <<<\n", __func__);
        exit(2);
    }
    data = &state->sm_data;

}



int sms_history_to_flash(char* loginfo,char* logfile)
{
    //rename() unlink()  unlink("/etc/config/udprpt.conf");
    FILE *f;
    int fsize=0;
    char logfilebak[50];

    f =fopen(logfile,"a");
    if (f==NULL) {
        return false;
    }
    if (-1==fseek(f,0L,SEEK_END)) {
        return false;
    }
    fsize=ftell(f);

    if (fsize >2500) {
        fclose(f);      
        sprintf(logfilebak,"%sbak",logfile); 
        unlink(logfilebak);
        rename(logfile,logfilebak);
        f =fopen(logfile,"w+");
        if (f==NULL) {
            return false;
        }
    } 
    fputs(loginfo,f);      
    fclose(f);   
    *loginfo=0;
    return true;
}






bool get_new_sms(char *sms_data, char *phone_number)
{

    gn_raw_data grawdata;
    gn_sms message;
    gn_error    error = GN_ERR_NONE;
    char *pos;
    char *tmp;
    int i=0;

    if (!sms_data) return false;

    gn_data_clear(data);

    memset(&message, 0, sizeof(gn_sms));

    message.memory_type = gn_str2memory_type("SM");

    message.number = 0;

    data->sms = &message;

    data->raw_data = &grawdata;


    //state->config.connect_script[0]=1;
    state->config.connect_script[0]=0;

    error = gn_smslist_get(data,state);
    syslog(LOGOPTS, "[%s] gn_smslist_get() return: %d\n", __func__, error);
printf("\n*****get new sms:%d\n",error);
    if (error == GN_ERR_NONE) {

        syslog(LOGOPTS, "[%s] get smslist context:\n%s\n", __func__, data->raw_data->data);

        tmp = data->raw_data->data;
        pos = tmp;
//printf("\n*****get new sms:%s\n",pos);
        while (tmp) {
            tmp = strstr(pos,"+CMGL: ");
            if (tmp) pos = tmp + strlen("+CMGL: ");
        }
        if (sscanf(pos, "%d,%*d", &data->sms->number) != 1) {
            if (data->raw_data->data) free(data->raw_data->data);
            data->raw_data->data=NULL;
            return false;
        }
        if (data->raw_data->data) free(data->raw_data->data);
        data->raw_data->data=NULL;
        syslog(LOGOPTS, "number=%d\n", data->sms->number);

        data->sms->memory_type = GN_MT_SM;
        error = gn_sms_get(data, state);
        if (error == GN_ERR_NONE) {
            syslog(LOGOPTS, "Sender=%s\n",data->sms->remote.number);
            syslog(LOGOPTS, "sms=%s\n",data->sms->user_data[0].u.text);
            strcpy(phone_number, data->sms->remote.number);
            strcpy(sms_data,data->sms->user_data[0].u.text);
//            if (DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_GETSMS_MODE][0]=='B') {
//                error = gn_sms_delete(data, state);
//                if (error == GN_ERR_NONE) syslog(LOGOPTS, "Deleted smsid=%d\n",data->sms->number);
//            }
            return true;
        }
    }
    return false;

}


void send_sms(char *phone_number, char *sms_data, int len)
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
    if (gn_sm_functions(GN_OP_GetSMSCenter, data, state) == GN_ERR_NONE) {
        snprintf(sms.smsc.number, sizeof(sms.smsc.number), "%s", data->message_center->smsc.number);
        sms.smsc.type = data->message_center->smsc.type;
    } else {
        syslog(LOGOPTS, "Cannot read the SMSC number from your phone. If the sms send will fail, please use --smsc option explicitely giving the number.\n");
        free(data->message_center);
        return;
    }
    free(data->message_center);


    if (!sms.smsc.type) sms.smsc.type = GN_GSM_NUMBER_Unknown;
    syslog(LOGOPTS, "Sending SMS to %s (text: %s)\n", data->sms->remote.number, data->sms->user_data[0].u.text);

    /* FIXME: set more SMS fields before sending */
    error = gn_sms_send(data, state);

    if (error == GN_ERR_NONE)
        syslog(LOGOPTS, "\r\n+CMGS: 0\r\n");

}



gn_error decodesms(unsigned char *buffer, int length, gn_data *data, struct gn_statemachine *state)
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
    //syslog(LOGOPTS,"advanced_sms, decodesms, sms_len=%d\n",sms_len);
    if (sms_len == 0)
        return GN_ERR_EMPTYLOCATION;

    sms_len = strlen(buf.line2) / 2;
    //syslog(LOGOPTS,"advanced_sms, decodesms, sms_len2=%d\n",sms_len);
    //tmp = calloc(sms_len, 1);
    memset(tmp,0,sms_len);
    if (!tmp) {
        syslog(LOGOPTS, "Not enough memory for buffer.\n");
        return GN_ERR_INTERNALERROR;
    }
    //syslog(LOGOPTS,"advanced_sms, decodesms, before tmp=%s\n",tmp);
    hex2bin(tmp, buf.line2, sms_len);
    //syslog(LOGOPTS,"advanced_sms, decodesms, after tmp=%s\n",tmp);
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
        syslog(LOGOPTS,"advanced_sms,decodesms,smstype=%d return\n",data->raw_sms->type);
        ret = GN_ERR_INTERNALERROR;
        goto out;
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
            syslog(LOGOPTS, "Internal Error on validity_indicator\n");
            ret = GN_ERR_INTERNALERROR;
            goto out;
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

    if (sscanf(buf.line1, "CMGL: %d,%d,%*s", &data->raw_sms->number, &data->raw_sms->status) != 2) {
        syslog(LOGOPTS,"advanced_sms, decodesms, error buf.line1=%s\n",buf.line1);
        ret = GN_ERR_FAILED;
        goto out;
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

void delete_one_sms(int location)
{
    gn_sms message;
    gn_error error = GN_ERR_NONE;
    gn_data_clear(data);
    memset(&message, 0, sizeof(gn_sms));
    message.memory_type = gn_str2memory_type("SM");
    message.number = location;
    data->sms = &message;
    gn_sms_delete(data, state);

    if (error == GN_ERR_NONE) {
        char key_word[8];
        char write_value[8];
        sprintf(key_word, "sms%d=",location);
        sprintf(write_value, "%d",location);
        //UserDB_write(db ,key_word,strlen(key_word),write_value,strlen(write_value));
    }

}

int get_smtotal(void)
{
    gn_data_clear(data);
    gn_sms_status smsstatus = {0, 0, 0, 0};
    gn_memory_status simmemorystatus = {GN_MT_SM, 0, 0};
    data->sms_status = &smsstatus;
    data->memory_status = &simmemorystatus;
    if (gn_sm_functions(GN_OP_GetSMSStatus, data, state) != GN_ERR_NONE)
        return;

    return data->sms_status->number;
}

void get_local_time(gn_sms *sms, char *dt)
{
    struct tm t, *loctime;
    time_t caltime;
    if (!dt) {
        syslog(LOGOPTS, "dt==NULL\n");
        return;
    }
    t.tm_sec = sms->smsc_time.second;
    t.tm_min = sms->smsc_time.minute;
    t.tm_hour = sms->smsc_time.hour;
    t.tm_mday = sms->smsc_time.day;
    t.tm_mon = sms->smsc_time.month-1;
    t.tm_year = sms->smsc_time.year-1900;
    t.tm_isdst = -1;

    if (sms->smsc_time.timezone)
        t.tm_gmtoff = sms->smsc_time.timezone * 3600;

    caltime = mktime(&t);
    loctime = localtime(&caltime);
    strftime(dt, 31, "%d/%m/%Y %H:%M:%S %z (%Z)\n", loctime);
    char *p=dt;
    p+=31;
    *p=0;
}


//get all sms list and write to readsmsfile
void get_new_sms_list(void)
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
    int length;
    FILE *sfp=NULL;
//    int sms_no=1;

    if ((sfp = fopen(ReadsmsFile, "w")) == 0) {
        syslog(LOGOPTS,"SMS fopen %s error\n", ReadsmsFile);
        return 0;
    }

    gn_data_clear(data);
    memset(&message, 0, sizeof(gn_sms));
    message.memory_type = gn_str2memory_type("SM");//GN_MT_ME/SM
     if(smsread_state!=0&&smsread_state!=1&&smsread_state!=4) 
     {
         smsread_state=1;
     }
    message.number = smsread_state;  //number: 0/1/4 at AT_GetSMSList  atgen.c   <stat>: Status = 0  unread, 1  read, 4  all
    data->sms = &message;
    data->raw_data = &grawdata;
    state->config.connect_script[0]=0;  //set as standard; 1 for motorola
    used = 0;
    error = gn_smslist_get(data,state);
//printf("\n******get sms result:%d***\n",error);
    if (error == GN_ERR_NONE) {
        tmp = data->raw_data->data;
        dp = tmp;
        pos = tmp;
        int i=0;
//printf("\n******2get sms result:%s***\n",pos);
        tmp = strstr(pos,"+CMGL: ");
        if (!tmp) {
            goto rout;
        }
        while (pos) {

            pos = strstr(tmp+7, "+CMGL: ");
            if (pos)
                length = pos-tmp;
            else
                length = data->raw_data->length - (tmp-dp);
            if (length<=0) {
                syslog(LOGOPTS,"advanced_sms length=%d\n",length);
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
                error = decodesms(buff,length,&mdata,state);
                if (error != GN_ERR_NONE) {
                    syslog(LOGOPTS,"advanced_sms, decodesms return error=%d\n",error);
                    free(buffp);
                    buffp=NULL;
                    buff=NULL;
                    goto rout;
                }
                error = gn_sms_parse(&mdata);
                if (error != GN_ERR_NONE) {
                    syslog(LOGOPTS,"advanced_sms, sms_parse return error=%d\n",error);
                    free(buffp);
                    buffp=NULL;
                    buff=NULL;
                    goto rout;
                }
                //syslog(LOGOPTS,"advanced_sms, decodesms return error=%d OK\n",error);
//                memset(DataBase1_Buff[COM1_SUB_MENU_SMS][SMS_ITEM_DT], 0, 32*sizeof(char));
//                memset(DataBase1_Buff[COM1_SUB_MENU_SMS][SMS_ITEM_SUBJECT], 0, 32*sizeof(char));
//                get_local_time(mdata.sms, DataBase1_Buff[COM1_SUB_MENU_SMS][SMS_ITEM_DT]);
                  char sms_time[50];
                  sms_time[0]=0;
                  get_local_time(mdata.sms, sms_time);
//                memcpy(DataBase1_Buff[COM1_SUB_MENU_SMS][SMS_ITEM_SUBJECT],mdata.sms->user_data[0].u.text,20);
                if (strlen(mdata.sms->user_data[0].u.text)>20) {
//                    strcat(DataBase1_Buff[COM1_SUB_MENU_SMS][SMS_ITEM_SUBJECT],"...");
                }
//                sprintf(strOut, "<tr id='sr%d'><td align='center'><input type='checkbox' name='check%d' onclick=\"smschecked(this);\"></td><td style=\"cursor:pointer;\"  onclick=\"showsms('#C0C0C0','%d');\">%s</td><td style=\"cursor:pointer;\"  onclick=\"showsms('#C0C0C0','%d');\">%s</td><td style=\"cursor:pointer;\"  onclick=\"showsms('#C0C0C0','%d');\">%s</td></tr>\n",
//                        used+1, mdata.sms->number, used+1, mdata.sms->remote.number, 
//                        used+1, DataBase1_Buff[COM1_SUB_MENU_SMS][SMS_ITEM_SUBJECT],
//                        used+1, DataBase1_Buff[COM1_SUB_MENU_SMS][SMS_ITEM_DT]);
//                sprintf(strOut, "<div id='sc%d' name='smscontent' style='display:none;'><pre><hr style='width:800;' align='left'>\n", used+1);
//                sprintf(strOut, "<input type='submit' name='subdel%d' value='Delete'><input type='submit' name='reply%d' value='Reply'>\n", mdata.sms->number, mdata.sms->number);
//                sprintf(strOut, "Date/time: %s\n", DataBase1_Buff[COM1_SUB_MENU_SMS][SMS_ITEM_DT]);
//                sprintf(strOut, "From: %s\n", mdata.sms->remote.number);
//                sprintf(strOut, "Subject: %s\n\n", DataBase1_Buff[COM1_SUB_MENU_SMS][SMS_ITEM_SUBJECT]);
//                sprintf(strOut, "%s\n</pre><hr style='width:800;' align='left'></div>\n", mdata.sms->user_data[0].u.text);
//printf("******sms info:%s:%s******\n",mdata.sms->remote.number,mdata.sms->user_data[0].u.text);
                // /tmp/run/msmsread:#1#+1234567890#time#msg context
                char tmp_phone_q[20];
                strcpy(tmp_phone_q,mdata.sms->remote.number);
                if(tmp_phone_q[0]=='+')
                {
                    tmp_phone_q[0]='Q';
                }
                fprintf(sfp, "<tr><td>%d</td><td>%s</td><td>%s</td><td>%s <a href=carrier-sms.sh?deleteid=%dP%s>Delete</a> <a href=carrier-sms.sh?replyid=%dP%s>Reply</a></td></tr>\r\n", (used+1),mdata.sms->remote.number,sms_time,mdata.sms->user_data[0].u.text,mdata.sms->number,tmp_phone_q,mdata.sms->number,tmp_phone_q);
                used++;
                //syslog(LOGOPTS,"advanced_sms used=%d\n",used);
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
        buff=NULL;
    }


    rout:
    if(used == 0)
    {
        fprintf(sfp, "<tr><td colspan=4>There is no message untreated in sim card.</td></tr>\r\n");
    }
    fflush(sfp);
    fclose(sfp);
    if (grawdata.data) free(grawdata.data);
    grawdata.data=NULL;
    return;
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


//1:connect ok.
int check_carrier_satus(void)
{
    char tmp_buff[256];
    FILE* f;
    char *p;
    int result=-1;

//    connect_status="_CONNECTED"

    if (!(f = fopen("/var/run/VIP4G_status", "r"))) 
    {
        //do nothing.
    }else
    {
        while (fgets(tmp_buff, 256, f)) 
        {
            p=NULL;
            p = strstr(tmp_buff, "connect_status=");  //connect_status="_CONNECTED"
            if (p != NULL)
            {
                p=NULL;
                p = strstr(tmp_buff,"_CONNECTED");
                if(p!=NULL)
                {
                    result=1;
                }
                break;
            }//if p
        }
    }
    if(f)fclose(f);

    return result;
}
