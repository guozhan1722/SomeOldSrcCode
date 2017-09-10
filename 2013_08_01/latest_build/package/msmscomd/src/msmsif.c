/* 
* A simple interface for sms application to send and receive sms, specially in webpage. 
*msmsif read 0/1/4;  del id; delall id1 id2; msmsif send 1/2 (/tmp/run/msmssend1/2)
*  /tmp/run/msmssend1/2:+1234567890:SNDMSG:  if continue with next line:1#******(256chars) 2#******* 
  //read read_num msg from module and write to file:/tmp/run/msmsread:#1#+1234567890#time#msg context
*/

#include "msmslib.h"



int main( int argc, char *argv[])
{

    int argv_type;
    int read_num,del_id2,del_id1;
    char msg_tmp[50];
    smsread_state=1;
    /*
     * 1)msmsif read 0/1/4; 
     * 2)msmsif del id; 
     * 3)msmsif delall id1 id2; 
     * 4)msmsif send /tmp/run/msmssend1/2
    */
    argv_type=0;
    if(argc!=3 && argc!=4 )
    {
        printf("\nPlease use as: readsms 0/1/4; delsms id; sendsms filename; delall idfrom idto\n");
        return -99;
    }
    if(argc==3)
    {
        if(strcmp(argv[1],"readsms")==0)
        {
            argv_type=1;
            read_num=atoi(argv[2]);
            if(read_num==0||read_num==1||read_num==4) 
            {
                smsread_state=read_num;
            }
        }
        if(strcmp(argv[1],"delsms")==0)
        {
            argv_type=2;
            strcpy(msg_tmp,argv[2]);
            if(strlen(msg_tmp)<5)return -2;
        }
        if(strcmp(argv[1],"sendsms")==0)
        {
            argv_type=3;
            strcpy(msg_tmp,argv[2]);
            if(strlen(msg_tmp)<4)return -3;
        }
    }
    if(argc==4)
    {
        if(strcmp(argv[1],"delall")==0)
        {
            argv_type=4;
            del_id1=atoi(argv[2]);
            del_id2=atoi(argv[3]);
            if(del_id1<0 || del_id2<1 || del_id1>del_id2)return -4;
        }
    }
    if(argv_type==0)return -98;

    (void) setsid();                                  


    int i;
    char buttonn[16];
    struct flock lock; 
    char tmp_buff[512];
    char tmp_phone[20];
    FILE* f;
    char *p;
    int j;
    FILE *sfp=NULL;
    tmp_buff[0]=0;
//    strcpy(tmp_buff,"abcd Alex test 1234567 +-*%,hello end.");
//    strcpy(tmp_phone,"+15877196686");



    if(argv_type==1)
    {
        if(check_carrier_satus()!=1)
        {
            if ((sfp = fopen(ReadsmsFile, "w")) == 0) {
                syslog(LOGOPTS,"SMS fopen %s error\n", ReadsmsFile);
                return 0;
            }
            fprintf(sfp, "<tr><td colspan=4><font color='Red'>Carrier failed, please check and try later.</font></td></tr>\r\n");
            fflush(sfp);
            fclose(sfp);
            return -10;
        }

        //read read_num msg from module and write to file:/tmp/run/msmsread:#1#+1234567890#time#msg context
            fd_radiobusy = open ( RadioBusyFlagFile, O_WRONLY);
            if (fd_radiobusy<0) 
            {
                syslog(LOGOPTS,"%s sms-summary showform %s:fd<0\n",__func__,RadioBusyFlagFile);

                if ((sfp = fopen(ReadsmsFile, "w")) == 0) {
                    syslog(LOGOPTS,"SMS fopen %s error\n", ReadsmsFile);
                    return 0;
                }
                fprintf(sfp, "<tr><td colspan=4><font color='Red'>Failed, please try again.</font></td></tr>\r\n");
                fflush(sfp);
                fclose(sfp);

                return -11;
            } else 
            {
                LockFile(fd_radiobusy);
                gn_lib_businit(&data, &state);
                get_new_sms_list();
//              bool error = get_new_sms(tmp_buff, tmp_phone);
                gn_lib_busterminate(&state); 
                UnLockFile(fd_radiobusy);
            }

    }

     if(argv_type==2)
     {   

         if(check_carrier_satus()!=1)
         {
             return -20;
         }

         if (!(f = fopen(ReadsmsFile, "r"))) 
         {
             return -22;
         }

         while (fgets(tmp_buff, 256, f)) 
         {
             p=NULL;
             p = strstr(tmp_buff, msg_tmp);
             if (p != NULL)break;
         }
         fclose(f);
         if(p==NULL)return -23;

         //3PQ15877196686
         p=NULL;
         p = strstr(msg_tmp, "P");
         if(p==NULL)return -24;
         i=p-msg_tmp;
         if(i>5)return -25; 

         *p=0;
         del_id1=atoi(msg_tmp);

         fd_radiobusy = open ( RadioBusyFlagFile, O_WRONLY);
         if (fd_radiobusy<0) 
         {
             syslog(LOGOPTS,"%s sms-summary showform %s:fd<0\n",__func__,RadioBusyFlagFile);
             sprintf(strOut, "<font color='Red'>Failed, please try again<br></font>\n");
             return -21;
         } else 
         {
             if ((sfp = fopen(ReadsmsFile, "w")) == 0) {
                 syslog(LOGOPTS,"SMS fopen %s error\n", ReadsmsFile);
                 return 0;
             }
             fprintf(sfp, "<tr><td colspan=4>Added to deleting list. Waiting to refresh...</font></td></tr>\r\n");
             fflush(sfp);
             fclose(sfp);

             LockFile(fd_radiobusy);
             gn_lib_businit(&data, &state);
             delete_one_sms(del_id1);
             gn_lib_busterminate(&state);
             UnLockFile(fd_radiobusy);
         }
     }

     if(argv_type==3)
     {   

         if (!(f = fopen(msg_tmp, "r"))) 
         {
             return -32;
         }

         if(fgets(tmp_buff, 256, f)) 
         {
             if(tmp_buff[0]=='#') 
             {
                 p=tmp_buff+1;
                 strcpy(tmp_phone,p);
                 tmp_buff[0]=0;
                 fgets(tmp_buff, 256, f);
             }
         }
         fclose(f);
         i=strlen(tmp_buff);
         if(strlen(tmp_phone)<5 || i==0)return -33;

         strOut[0]=0;
         struct tm *ttime;
         time_t tm = time(0);
         ttime = localtime(&tm);
         if(check_carrier_satus()!=1)
         {
             sprintf(strOut, "<tr><td>%s</td><td>%s</td><td>%s</td><td>Carrier failed.</td></tr>\r\n",tmp_phone,asctime(ttime),tmp_buff);
             sms_history_to_flash(strOut,SendHistoryFile);
             return -30;
         }


         fd_radiobusy = open ( RadioBusyFlagFile, O_WRONLY);
         if (fd_radiobusy<0) 
         {
             syslog(LOGOPTS,"%s sms-summary showform %s:fd<0\n",__func__,RadioBusyFlagFile);
             sprintf(strOut, "<tr><td>%s</td><td>%s</td><td>%s</td><td>Failed.</td></tr>\r\n",tmp_phone,asctime(ttime),tmp_buff);
             sms_history_to_flash(strOut,SendHistoryFile);
             return -34;
         } else 
         {
             LockFile(fd_radiobusy);
             gn_lib_businit(&data, &state);
             send_sms(tmp_phone, tmp_buff, i);
             gn_lib_busterminate(&state);
             UnLockFile(fd_radiobusy);

             sprintf(strOut, "<tr><td>%s</td><td>%s</td><td>%s</td><td>Succeed to send.</td></tr>\r\n",tmp_phone,asctime(ttime),tmp_buff);
             sms_history_to_flash(strOut,SendHistoryFile);

         }
     }

     if(argv_type==4)
     {   

         if(check_carrier_satus()!=1)
         {
             return -20;
         }

         fd_radiobusy = open ( RadioBusyFlagFile, O_WRONLY);
         if (fd_radiobusy<0) 
         {
             syslog(LOGOPTS,"%s sms-summary showform %s:fd<0\n",__func__,RadioBusyFlagFile);
             sprintf(strOut, "<font color='Red'>Failed, please try again<br></font>\n");
             return -11;
         } else 
         {
             if (!(f = fopen(ReadsmsFile, "r"))) 
             {
                 UnLockFile(fd_radiobusy);
                 return -32;
             }
             LockFile(fd_radiobusy);
             gn_lib_businit(&data, &state);
             while(fgets(tmp_buff, 510, f)) 
             {
                 p=NULL;
                 i=0;
                 p=strstr(tmp_buff,"deleteid=");
                 if(p!=NULL)
                 {
                     p+=strlen("deleteid=");
                     j=0;
                     while(*p!='P' && j<5 && *p!=0 )
                     {
                         tmp_phone[j]=*p;
                         p++;
                         j++;
                     }
                     tmp_phone[j]=0;
                     i=atoi(tmp_phone);
                     //maybe to check i in id1-id2.
                     delete_one_sms(i);
                 }
             }
             fclose(f);
             gn_lib_busterminate(&state);
             UnLockFile(fd_radiobusy);
         }

     }

     UnLockFile(fd_radiobusy);
     return 0;

}


