

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>
#include "unistd.h"

#include <syslog.h>

#include <sys/wait.h>
#include <sys/time.h>
#define _XOPEN_SOURCE /* glibc2 needs this */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>													

#include <stdbool.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>

#include "uci.h"
#include "gnokii.h"
#include "atgen.h"


#define	LOGOPTS		(LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)
#ifdef MEMWATCH
    #include "memwatch.h"
#endif



#define DISMATCHED 0
#define MATCHED 1

char *RadioBusyFlagFile="/var/run/VIP4G_STAT_busy";
char *ReadsmsFile="/var/run/trackcmdsmsread";
char *SendsmsFile="/var/run/trackcmdsmssend";
char *trackersmsFile="/var/run/trackersmsfile";
char *TrackScmdHistoryFile="/etc/trackersms.history";
char *TrackScmdHistoryFilebak="/etc/trackersms.historybak";
char *TrackSendHistoryFile="/etc/trackersmss.history";
char *TrackSendHistoryFilebak="/etc/trackersmss.historybak";

struct gn_statemachine *state;
gn_data *data;
char clear_sms_sign;


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


gn_error send_sms(char *phone_number, char *sms_data, int len)
{

    gn_sms sms;
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
    return gn_sms_send(data, state);
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


void check_expire_sms_list(void)
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

    char buffstr[512];
    int rc;
    int del_id=0;
    int del_list[100];
    for(del_id=0;del_id<100;del_id++) {del_list[del_id]=-1;}
    del_id=0;


    gn_data_clear(data);
    memset(&message, 0, sizeof(gn_sms));
    message.memory_type = gn_str2memory_type("SM");//GN_MT_ME/SM
    message.number = 4;  //number: 0/1/4 at AT_GetSMSList  atgen.c   <stat>: Status = 0  unread, 1  read, 4  all
    data->sms = &message;
    data->raw_data = &grawdata;
    state->config.connect_script[0]=0;  //set as standard; 1 for motorola
    error = gn_smslist_get(data,state);
    if (error == GN_ERR_NONE) {
        tmp = data->raw_data->data;
        dp = tmp;
        pos = tmp;
        int i=0;
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
                error = decodesms(buff,length,&mdata,state,0);
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
                  char sms_time[50];
                  sms_time[0]=0;
                  get_local_time(mdata.sms, sms_time);
                if (strlen(mdata.sms->user_data[0].u.text)>7) 
                {
                    buffstr[0]=0;
                    strncpy(buffstr,mdata.sms->user_data[0].u.text,7);
                    if(buffstr[0]=='$' && buffstr[1]=='F'&& buffstr[2]=='R'&& buffstr[3]=='C'&& buffstr[4]=='M'&& buffstr[5]=='D'&& buffstr[6]==',')
                    {
                        struct tm *ttime;
                        time_t tm = time(0);
                        ttime = localtime(&tm);
                        sprintf(buffstr, "<tr><td>%s</td><td>%s</td><td>%s</td><td>Expired.@%s</td></tr>", mdata.sms->remote.number,sms_time,mdata.sms->user_data[0].u.text,asctime(ttime));
                        sms_history_to_flash(buffstr,TrackScmdHistoryFile);
                        del_list[del_id]=mdata.sms->number;
                        del_id++;
                    }
                }
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
    if (grawdata.data) free(grawdata.data);
    grawdata.data=NULL;

    while(del_id>0)
    {
        del_id--;
        delete_one_sms(del_list[del_id]);
        del_list[del_id]=-1;
    }

    return;
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
    char buffstr[512];
    int del_id=0;
    int del_list[20];
    for(del_id=0;del_id<20;del_id++) {del_list[del_id]=-1;}
    del_id=0;

    if ((sfp = fopen(ReadsmsFile, "a+")) == 0) {
        syslog(LOGOPTS,"SMS fopen %s error\n", ReadsmsFile);
        return;
    }

    int fsize=ftell(sfp);
    if (fsize >500) 
    {
        fclose(sfp);      
        unlink(ReadsmsFile);
        sfp =fopen(ReadsmsFile,"a+");
        if(sfp==NULL) 
        {
            return;
        }
    } 


    gn_data_clear(data);
    memset(&message, 0, sizeof(gn_sms));
    message.memory_type = gn_str2memory_type("SM");//GN_MT_ME/SM
    message.number = 4;  //number: 0/1/4 at AT_GetSMSList  atgen.c   <stat>: Status = 0  unread, 1  read, 4  all
    data->sms = &message;
    data->raw_data = &grawdata;
    state->config.connect_script[0]=0;  //set as standard; 1 for motorola
    error = gn_smslist_get(data,state);
    if (error == GN_ERR_NONE) {
        tmp = data->raw_data->data;
        dp = tmp;
        pos = tmp;
        int i=0;
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
                error = decodesms(buff,length,&mdata,state,0);
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
                  char sms_time[50];
                  sms_time[0]=0;
                  get_local_time(mdata.sms, sms_time);
                if (strlen(mdata.sms->user_data[0].u.text)>7)
                {
                    buffstr[0]=0;
                    strncpy(buffstr,mdata.sms->user_data[0].u.text,7);
                    if(buffstr[0]=='$' && buffstr[1]=='F'&& buffstr[2]=='R'&& buffstr[3]=='C'&& buffstr[4]=='M'&& buffstr[5]=='D'&& buffstr[6]==',')
                    {
                        sprintf(buffstr, "#%s:%s\r\n", mdata.sms->remote.number,mdata.sms->user_data[0].u.text);
                        fprintf(sfp,"%s",buffstr);

                        struct tm *ttime;
                        time_t tm = time(0);
                        ttime = localtime(&tm);
                        sprintf(buffstr, "<tr><td>%s</td><td>%s</td><td>%s</td><td>Wait to deal@%s</td></tr>", mdata.sms->remote.number,sms_time,mdata.sms->user_data[0].u.text,asctime(ttime));
                        sms_history_to_flash(buffstr,TrackScmdHistoryFile);
                        del_list[del_id]=mdata.sms->number;
                        del_id++;
                    }
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
        buff=NULL;
    }


    rout:
    fflush(sfp);
    fclose(sfp);
    if (grawdata.data) free(grawdata.data);
    grawdata.data=NULL;

    while(del_id>0)
    {
        del_id--;
        delete_one_sms(del_list[del_id]);
        del_list[del_id]=-1;
    }

    return;
}




void send_sms_list()
{
    gn_error    error = GN_ERR_NONE;
    FILE *f;
    char *p;
    char tmp_buff[512];
    int is_del=0;

    if (!(f = fopen(SendsmsFile, "r"))) 
    {
        return -32;
    }

    while(fgets(tmp_buff, 512, f)) 
    {
        if(tmp_buff[0]=='#') 
        {
            p=NULL;
            p=strchr(tmp_buff,':');
            if(p!=NULL)
            {
                *p=0;
                if(strlen(tmp_buff+1)>5)
                {
                    error = send_sms(tmp_buff+1, p+1, strlen(p+1));
                    if (error == GN_ERR_NONE)
                    {
                        is_del=1;
                    }
                    *p=':';
                    struct tm *ttime;
                    time_t tm = time(0);
                    ttime = localtime(&tm);
                    strcat(tmp_buff, asctime(ttime));
                    sms_history_to_flash(tmp_buff,TrackSendHistoryFile);
                }
            }
        }
    }
    fclose(f);

    if(is_del==1)unlink(SendsmsFile);
}

//1.check /tmp/run/trackersmsfile, if null, then read sms and clear all related sms.
//2.try to read/delete sms every 3 seconds and send sms from file:/tmp/run/trackersmsread trackersmssend
int main(int argc, char *argv[])
{
    FILE * f=NULL;
	int i;
    clear_sms_sign=0;
    f = fopen ( trackersmsFile, "r" );   
	if (f==NULL)  
	{
        clear_sms_sign=1;
        f = fopen ( trackersmsFile, "w+" );   
        if(f==NULL)return -1;
        fputs("ok",f);      
        fclose(f);   
	}else fclose(f);

    int fd=-1;
    int index=0;

while(1)
{

    if(check_carrier_satus()!=1)
    {
        sleep(2);
        continue;
    }

    fd = open ( RadioBusyFlagFile, O_WRONLY);
    if (fd<0)
    {
        syslog(LOGOPTS,"%s gpstracker_read_sms %s:fd<0\n",__func__,RadioBusyFlagFile);
        sleep(5);
        continue;
    }

    LockFile(fd); 

    if(clear_sms_sign==1)
    {
        gn_lib_businit(&data, &state);
        check_expire_sms_list();//check and delete
        gn_lib_busterminate(&state);
        clear_sms_sign=0;
    }

    gn_lib_businit(&data, &state);
    get_new_sms_list();
    gn_lib_busterminate(&state); 

    gn_lib_businit(&data, &state);
    send_sms_list();
    gn_lib_busterminate(&state); 

    UnLockFile(fd);

    sleep(2);
}
}
