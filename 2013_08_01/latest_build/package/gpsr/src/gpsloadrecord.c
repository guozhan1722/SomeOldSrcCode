#include "gpsrecorder.h"

//record file structure: /log/gpsrec/0,1,2,3,4,...,10...50.
//Data,Time(no ms),5108.56523,N,11404.52363,W,1086.5,0000(Input),0000(Output)

//gpsrecorder readgps
//status file structrue:
//(printf \n)<tr><td width="40%">&nbsp;&nbsp;Altitude:</td><td width="60%">1096</td></tr>
//echo /tmp/run/gpsstatus 

int gpsrecord_file_index=-1;

char str_pos_latlng[50];
char str_pos_alt[20];
char str_iostatus_buff[20];
char valid_c='0';
char utc_time[20];
char utc_date[20];

char base_pos_lat[15];
char base_pos_lng[15];
char base_utc_time[7];
char base_utc_date[7];
char base_pos_lat_NS;
char base_pos_lng_WE;

    int readbytes = 0;
    int sock = 0;
    ssize_t wrote = 0;
    int fd_tcp=0;

char c_Pos_Record_EN;
int i_Pos_Record_NUMS;
int i_Pos_Record_INTERVAL;

int gpsd_port;
char gpsd_status;

char s_SERVER_Address[40];
struct sockaddr_in addr_remote_udp;    // remote host address information

int i_SERVER_Port;
char c_SEND_Mode;
char senddata_list[MAX_REC_FILE_NUM];

int zoom_distance(double lat1, double lon1, double lat2, double lon2) 
{
    gps_position pos1,pos2;
	  double dist;
      pos1.lat=lat1*100;
      pos2.lat=lat2*100;
      pos1.lng=lon1*100;
      pos2.lng=lon2*100;
      dist=compute_distance(&pos1,&pos2);
      int i_dist=(int)dist/1000;
      
      if(i_dist<2)return 16;
      if(i_dist<4)return 15;
      if(i_dist<7)return 14;
      if(i_dist<12)return 13;
      if(i_dist<24)return 12;
      if(i_dist<40)return 11;
      if(i_dist<80)return 10;
      if(i_dist<160)return 9;
      if(i_dist<300)return 8;
      if(i_dist<600)return 7;
      if(i_dist<1200)return 6;
      if(i_dist<2000)return 5;
      if(i_dist<4000)return 4;
      if(i_dist<8000)return 3;
      return 2;
}

int parseback_data(char *base_str,char *diff_str,char *valid_str_out)
{
    int len_base,len_diff;
    int i,j;
    len_base=strlen(base_str);
    if(len_base<6)return -1;
    len_diff=strlen(diff_str);
    if(len_diff>len_base)return -1;
    for(i=0;i<len_base-len_diff;i++){
        valid_str_out[i]=base_str[i];
    }
    for(j=0;j<len_diff;j++)
    {
        valid_str_out[i+j]=diff_str[j];
    }
    valid_str_out[len_base]=0;
    return 1;
}

void trace_record_file_dateline(int file_id)
{
    char *outputfile1="/var/run/gpsloadrec0";
    FILE *sfp1=NULL;
    if ((sfp1 = fopen(outputfile1, "w")) == 0)
    { 
        return;
    }

    char *outputfile="/var/run/gpsloadrectrace";
    FILE *sfp=NULL;
    if ((sfp = fopen(outputfile, "w")) == 0) 
    {
        return;
    }
    
    gpsrecord_file_index=0;

    char fullfile[30];
    char tmp_buff[201];
    char timestr0[30];
    char timestr1[30];
    char pos_buff[30];
    int year0[MAX_REC_FILE_NUM];
    int mon0[MAX_REC_FILE_NUM];
    int day0[MAX_REC_FILE_NUM];//as check ==0
    int hour0[MAX_REC_FILE_NUM];
    int min0[MAX_REC_FILE_NUM];
    int sec0[MAX_REC_FILE_NUM];
    int i=0;
    int i_tmp;
    int j=0;
    int i_zoom=10;//max 15
    char valid_data=0;
    float lat0[MAX_REC_FILE_NUM];
    float lng0[MAX_REC_FILE_NUM];
    float lat1[MAX_REC_FILE_NUM];
    float lng1[MAX_REC_FILE_NUM];
    float lat_max=0;
    float lat_min=0;
    float lng_max=0;
    float lng_min=0;
    float lat_stop,lng_stop;
    FILE* f=NULL;
    char *p,*p1;
    strcpy(timestr0,"000000000000");
    base_pos_lat[0]=0;
    base_pos_lng[0]=0;
    base_utc_time[0]=0;
    base_utc_date[0]=0;
    base_pos_lat_NS=0;
    base_pos_lng_WE=0;
    for(i=0;i<MAX_REC_FILE_NUM;i++)
    {
        day0[i]=0;
        lng0[i]=0;
        lng1[i]=0;
    }

    if(file_id==REC_FILE_ALL) 
    {
        for(i=0;i<MAX_REC_FILE_NUM;i++)
        {
            sprintf(fullfile,"%s%d",gpsrecord_dir,i);
            f = fopen(fullfile, "r");
            if(f==NULL)break;
            j=0;

        read_next_line:
            j++;
            tmp_buff[0]=0;
            if(fgets(tmp_buff,200,f))
            {
                if(tmp_buff[0]=='#')goto read_next_line;
                if(tmp_buff[6]!=',')goto read_next_line;
                if(tmp_buff[1]==',' || tmp_buff[2]==',' || tmp_buff[3]==',' || tmp_buff[4]==',' || tmp_buff[5]==',' )goto read_next_line; 

                tmp_buff[200]=0;
                p1=tmp_buff;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)goto read_next_line;
                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)goto read_next_line;
//                strncpy(base_utc_date,tmp_buff,6);
//                base_utc_date[6]=0;
//                strncpy(base_utc_time,p1,6);
//                base_utc_time[6]=0;

                *p=0;
                sscanf(tmp_buff, "%2d%2d%2d,%2d%2d%2d*",&day0[i], &mon0[i], &year0[i], &hour0[i], &min0[i], &sec0[i]);
                if(day0[i]==0)goto read_next_line;
                sprintf(timestr1,"%02d%02d%02d%02d%02d%02d",year0[i],mon0[i],day0[i],hour0[i],min0[i],sec0[i]);
                if(strcmp(timestr1,timestr0)>0)
                {
                    gpsrecord_file_index=i;
                    strcpy(timestr0,timestr1);
                }
                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)goto read_next_line;
                *p=0;
                strcpy(pos_buff,p1);
                if(strlen(pos_buff)<4)goto read_next_line;
                sscanf(pos_buff, "%f", &lat0[i]);
                i_tmp=lat0[i]/100;
                lat0[i]=i_tmp+(lat0[i]-i_tmp*100)/60+0.000005;
                p++;
                if(*p=='S')lat0[i]=lat0[i]*(-1);
                p1=p+2;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)goto read_next_line;
                *p=0;
                strcpy(pos_buff,p1);
                if(strlen(pos_buff)<4)goto read_next_line;
                sscanf(pos_buff, "%f", &lng0[i]);
                i_tmp=lng0[i]/100;
                lng0[i]=i_tmp+(lng0[i]-i_tmp*100)/60+0.000005;
                p++;
                if(*p=='W')lng0[i]=lng0[i]*(-1);

                while(fgets(tmp_buff,200,f))
                {
                    j++;
                    if(j<200)continue;
                    if(tmp_buff[0]=='#')continue;
                    if(tmp_buff[6]!=',')continue;
                    if(tmp_buff[1]==',' || tmp_buff[2]==',' || tmp_buff[3]==',' || tmp_buff[4]==',' || tmp_buff[5]==',' )continue; 

                    tmp_buff[200]=0;
                    p1=tmp_buff;
                    p=NULL;
                    p=strchr(p1,',');
                    if(p==NULL)continue;
                    p++;
                    p1=p;
                    p=NULL;
                    p=strchr(p1,',');
                    if(p==NULL)continue;
                    p++;
                    p1=p;
                    p=NULL;
                    p=strchr(p1,',');
                    if(p==NULL)continue;
                    *p=0;
                    strcpy(pos_buff,p1);
                    if(strlen(pos_buff)<4)continue;
                    sscanf(pos_buff, "%f", &lat1[i]);
                    i_tmp=lat1[i]/100;
                    lat1[i]=i_tmp+(lat1[i]-i_tmp*100)/60+0.000005;
                    p++;
                    if(*p=='S')lat1[i]=lat1[i]*(-1);
                    p1=p+2;
                    p=NULL;
                    p=strchr(p1,',');
                    if(p==NULL)continue;
                    *p=0;
                    strcpy(pos_buff,p1);
                    if(strlen(pos_buff)<4)continue;
                    sscanf(pos_buff, "%f", &lng1[i]);
                    i_tmp=lng1[i]/100;
                    lng1[i]=i_tmp+(lng1[i]-i_tmp*100)/60+0.000005;
                    p++;
                    if(*p=='W')lng1[i]=lng1[i]*(-1);

                    break;  //data is ok.
                }
            }else 
            {
                fclose(f);
                break;
            }
            fclose(f);
        }

        i_tmp=0;
        for(i=0;i<MAX_REC_FILE_NUM;i++)
        {
            j=i+1+gpsrecord_file_index;
            if(j>=MAX_REC_FILE_NUM)j=j-MAX_REC_FILE_NUM;
            if(day0[j]!=0 && lng0[j]!=0)
            {

                if(i_tmp==0)
                {
                    fprintf(sfp, " new google.maps.LatLng(%.6f,%.6f)",lat0[j],lng0[j]);
                    fprintf(sfp1,"var myLatLng = new google.maps.LatLng(%.6f,%.6f);\n",lat0[j],lng0[j]);
                    i_tmp=1;
                    lat_min=lat0[j];
                    lat_max=lat0[j];
                    lng_min=lng0[j];
                    lng_max=lng0[j];
                    lat_stop=lat0[j];
                    lng_stop=lng0[j];
                }
                else fprintf(sfp, ",\n new google.maps.LatLng(%.6f,%.6f)",lat0[j],lng0[j]);   
                if(lat_min>lat0[j])lat_min=lat0[j];
                if(lat_max<lat0[j])lat_max=lat0[j];
                if(lng_min>lng0[j])lng_min=lng0[j];
                if(lng_max<lng0[j])lng_max=lng0[j];

                if(lng1[j]!=0)
                {
                    fprintf(sfp, ",\n new google.maps.LatLng(%.6f,%.6f)",lat1[j],lng1[j]);
                    if(lat_min>lat1[j])lat_min=lat1[j];
                    if(lat_max<lat1[j])lat_max=lat1[j];
                    if(lng_min>lng1[j])lng_min=lng1[j];
                    if(lng_max<lng1[j])lng_max=lng1[j];
                    lat_stop=lat1[j];
                    lng_stop=lng1[j];
                }
            }
        }

        fprintf(sfp1,"var myLatLng_stop = new google.maps.LatLng(%.6f,%.6f);\n ",lat_stop,lng_stop);
        lat_stop=(lat_min+lat_max)/2;
        lng_stop=(lng_min+lng_max)/2;
        fprintf(sfp1,"var myLatLng_center = new google.maps.LatLng(%.6f,%.6f);\n",lat_stop,lng_stop);
        if(lat_min!=lat_max)i_zoom=zoom_distance(lat_min, lng_min, lat_max, lng_max); 
        fprintf(sfp1,"var mapOptions = {\n zoom: %d,\n",i_zoom);
        fclose(sfp1);
        fclose(sfp);
        return;
    }

    if(file_id<0 || file_id>=MAX_REC_FILE_NUM)return; 


    //for special file record
    base_pos_lng[0]==0;
    base_pos_lat[0]==0;
    //base_utc_date[0]==0;
    //base_utc_time[0]==0;

    sprintf(fullfile,"%s%d",gpsrecord_dir,file_id);
    f = fopen(fullfile, "r");
    if(f==NULL)return;
    j=0;
    while(fgets(tmp_buff,200,f))
    {
//printf("#########%s\n",tmp_buff);

        if(valid_data==1)
        {
//printf("----%.6f,%.6f\n",lat1[0],lng1[0]);
            fprintf(sfp, "new google.maps.LatLng(%.6f,%.6f),\n",lat1[0],lng1[0]);
            valid_data=0;
            if(j==0)
            {
                fprintf(sfp1,"var myLatLng = new google.maps.LatLng(%.6f,%.6f);\n ",lat1[0],lng1[0]);
                j=1;
                lat_min=lat1[0];
                lat_max=lat1[0];
                lng_min=lng1[0];
                lng_max=lng1[0];
            }

            if(lat_min>lat1[0])lat_min=lat1[0];
            if(lat_max<lat1[0])lat_max=lat1[0];
            if(lng_min>lng1[0])lng_min=lng1[0];
            if(lng_max<lng1[0])lng_max=lng1[0];

        }
        if(tmp_buff[0]=='#')continue;        
        tmp_buff[200]=0;

        if(tmp_buff[6]==',' && tmp_buff[1]!=',' && tmp_buff[2]!=',' && tmp_buff[3]!=',' && tmp_buff[4]!=',' && tmp_buff[5]!=',' )
        {
//printf("++++++begin the base\n");
            base_pos_lat[0]=0;
            base_pos_lng[0]=0;
            base_utc_time[0]=0;
            base_utc_date[0]=0;
            base_pos_lat_NS=0;
            base_pos_lng_WE=0;

            p1=tmp_buff;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)continue;
            p++;
            p1=p;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)continue;
            p++;
            p1=p;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)continue;
            *p=0;
            strcpy(pos_buff,p1);
//printf("**1***%s\n",pos_buff);
            if(strlen(pos_buff)<4)continue;
            strcpy(base_pos_lat,pos_buff);
            sscanf(pos_buff, "%f", &lat1[0]);
            i=lat1[0]/100;
            lat1[0]=i+(lat1[0]-i*100)/60+0.000005;
            p++;
            if(*p=='S')lat1[0]=lat1[0]*(-1);
            base_pos_lat_NS=*p;
            p1=p+2;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)continue;
            *p=0;
            strcpy(pos_buff,p1);
//printf("**2***%s\n",pos_buff);
            if(strlen(pos_buff)<4)continue;
            strcpy(base_pos_lng,pos_buff);
            sscanf(pos_buff, "%f", &lng1[0]);
            i=lng1[0]/100;
            lng1[0]=i+(lng1[0]-i*100)/60+0.000005;
            p++;
            if(*p=='W')lng1[0]=lng1[0]*(-1);
            base_pos_lng_WE=*p;
            valid_data=1;
            lat_stop=lat1[0];
            lng_stop=lng1[0];
        }else
        {
            if(base_pos_lng[0]==0)continue;

            p1=tmp_buff;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)continue;
            p++;
            p1=p;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)continue;
            *p=0;
            if(parseback_data(base_pos_lat,p1,pos_buff)<1)
            {
                continue;
            }
            if(strlen(pos_buff)<4)continue;
            sscanf(pos_buff, "%f", &lat1[0]);
            i=lat1[0]/100;
            lat1[0]=i+(lat1[0]-i*100)/60+0.000005;
            if(base_pos_lat_NS=='S')lat1[0]=lat1[0]*(-1);
            p++;
            p1=p;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)continue;
            *p=0;
            if(parseback_data(base_pos_lng,p1,pos_buff)<1)continue;
            if(strlen(pos_buff)<4)continue;
            sscanf(pos_buff, "%f", &lng1[0]);
            i=lng1[0]/100;
            lng1[0]=i+(lng1[0]-i*100)/60+0.000005;
            if(base_pos_lng_WE=='W')lng1[0]=lng1[0]*(-1);
            valid_data=1;
            lat_stop=lat1[0];
            lng_stop=lng1[0];
        }
    }

    if(valid_data==1)
    {
        fprintf(sfp, "new google.maps.LatLng(%.6f,%.6f),\n",lat1[0],lng1[0]);
        valid_data=0;
        if(j==0)
        {
            fprintf(sfp1,"var myLatLng = new google.maps.LatLng(%.6f,%.6f);\n ",lat1[0],lng1[0]);
            j=1;
            lat_min=lat1[0];
            lat_max=lat1[0];
            lng_min=lng1[0];
            lng_max=lng1[0];
        }

        if(lat_min>lat1[0])lat_min=lat1[0];
        if(lat_max<lat1[0])lat_max=lat1[0];
        if(lng_min>lng1[0])lng_min=lng1[0];
        if(lng_max<lng1[0])lng_max=lng1[0];

    }

/*
    if(valid_data==1)
    {
        if(lat_min>lat1[0])lat_min=lat1[0];
        if(lat_max<lat1[0])lat_max=lat1[0];
        if(lng_min>lng1[0])lng_min=lng1[0];
        if(lng_max<lng1[0])lng_max=lng1[0];

        fprintf(sfp, "new google.maps.LatLng(%.6f,%.6f)\n",lat1[0],lng1[0]);
        if(j==0)
        {
            fprintf(sfp1,"var myLatLng = new google.maps.LatLng(%.6f,%.6f);\n",lat1[0],lng1[0]);
            j=1;
        }
    }
*/

    fprintf(sfp1,"var myLatLng_stop = new google.maps.LatLng(%.6f,%.6f);\n ",lat_stop,lng_stop);
    lat_stop=(lat_min+lat_max)/2;
    lng_stop=(lng_min+lng_max)/2;
    fprintf(sfp1,"var myLatLng_center = new google.maps.LatLng(%.6f,%.6f);\n",lat_stop,lng_stop);
    if(lat_min!=lat_max)i_zoom=zoom_distance(lat_min, lng_min, lat_max, lng_max); 
    fprintf(sfp1,"var mapOptions = {\n zoom: %d,\n",i_zoom);

    fclose(f);
    fclose(sfp);
    fclose(sfp1);

    return;
}


//:list ->/var/run/gpsloadreclist
void list_record_file_dateline()
{
    char *outputfile="/var/run/gpsloadreclist";
    FILE *sfp=NULL;
    if ((sfp = fopen(outputfile, "w")) == 0) {
        printf("<tr><td colspan=4>System IO Error!</td></tr>\n");
        return;
    }
    
    gpsrecord_file_index=0;

    char fullfile[30];
    char tmp_buff[101];
    char timestr0[30];
    char timestr1[30];
    int year0[MAX_REC_FILE_NUM];
    int mon0[MAX_REC_FILE_NUM];
    int day0[MAX_REC_FILE_NUM];//as check ==0
    int hour0[MAX_REC_FILE_NUM];
    int min0[MAX_REC_FILE_NUM];
    int sec0[MAX_REC_FILE_NUM];
#if 0
    int year1[MAX_REC_FILE_NUM];
    int mon1[MAX_REC_FILE_NUM];
    int day1[MAX_REC_FILE_NUM];
    int hour1[MAX_REC_FILE_NUM];
    int min1[MAX_REC_FILE_NUM];
    int sec1[MAX_REC_FILE_NUM];
#endif
    int i=0;
    int j=0;
    int i_tmp;
    char valid_data=0;

    for(i=0;i<MAX_REC_FILE_NUM;i++)day0[i]=0;

    FILE* f=NULL;
    char *p,*p1;
    strcpy(timestr0,"000000000000");

    for(i=0;i<MAX_REC_FILE_NUM;i++)
    {
        sprintf(fullfile,"%s%d",gpsrecord_dir,i);
        f = fopen(fullfile, "r");
        if(f==NULL)break;
nextline_read_list:
        if(fgets(tmp_buff,100,f))
        {

            if(tmp_buff[0]=='#' || strlen(tmp_buff)<32)goto nextline_read_list;
            if(tmp_buff[6]!=',')goto nextline_read_list;
            if(tmp_buff[1]==',' || tmp_buff[2]==',' || tmp_buff[3]==',' || tmp_buff[4]==',' || tmp_buff[5]==',' )goto nextline_read_list;; 

            tmp_buff[99]=0;
            p1=tmp_buff;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)
            {
                fclose(f);
                break;
            }
            p++;
            p1=p;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)
            {
                fclose(f);
                break;
            }
            *p=0;
            sscanf(tmp_buff, "%2d%2d%2d,%2d%2d%2d*",&day0[i], &mon0[i], &year0[i], &hour0[i], &min0[i], &sec0[i]);
            if(day0[i]==0)
            {
                fclose(f);
                break;
            }
            sprintf(timestr1,"%02d%02d%02d%02d%02d%02d",year0[i],mon0[i],day0[i],hour0[i],min0[i],sec0[i]);
            if(strcmp(timestr1,timestr0)>0)
            {
                gpsrecord_file_index=i;
                strcpy(timestr0,timestr1);
            }
#if 0
            year1[i]=year0[i];
            mon1[i]=mon0[i];
            day1[i]=day0[i];
            hour1[i]=hour0[i];
            min1[i]=min0[i];
            sec1[i]=sec0[i];

            while(fgets(tmp_buff,100,f))
            {
                tmp_buff[99]=0;
                p1=tmp_buff;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)break;
                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)break;
                *p=0;
                sscanf(tmp_buff, "%2d%2d%2d,%2d%2d%2d*",&day1[i], &mon1[i], &year1[i], &hour1[i], &min1[i], &sec1[i]);
                if(day1[i]==0)break;
            }
#endif
        }else 
        {
            fclose(f);
            break;
        }
        fclose(f);
    }


    for(i=0;i<MAX_REC_FILE_NUM;i++)
    {
        j=i+1+gpsrecord_file_index;
        if(j>=MAX_REC_FILE_NUM)j=j-MAX_REC_FILE_NUM;
        if(day0[j]!=0)
        {
            valid_data=1;
            sprintf(timestr0,"20%02d-%02d-%02d %02d:%02d:%02d",year0[j],mon0[j],day0[j],hour0[j],min0[j],sec0[j]);
            i_tmp=j+1;
            if(i_tmp>=MAX_REC_FILE_NUM)i_tmp=i_tmp-MAX_REC_FILE_NUM;
            if(day0[i_tmp]!=0 && i!=MAX_REC_FILE_NUM-1)sprintf(timestr1,"20%02d-%02d-%02d %02d:%02d:%02d",year0[i_tmp],mon0[i_tmp],day0[i_tmp],hour0[i_tmp],min0[i_tmp],sec0[i_tmp]);
            else strcpy(timestr1,"...");

            fprintf(sfp, "<tr><td>%s</td><td>%s</td><td><input type=checkbox name=dataselect%d onclick=\"unselectall(this.form)\" /></td><td><a href=gps-loadrecord.sh?view=%d>View Data</a> &nbsp;&nbsp;<a href=gps-loadrecord.sh?trace=%d>Trace Map</a></td></tr>\n",timestr0,timestr1,j,j,j,j);
        }
    }
    if(valid_data==1)
    {
        fprintf(sfp, "<tr><td colspan=2></td><td><input type=checkbox name=chkAll id=chkAll onclick=\"CheckAll(this.form)\" /></td><td>Select All &nbsp;&nbsp; <a href=gps-loadrecord.sh?trace=quick>Quick Trace</a></td></tr>\n");
        //fprintf(sfp, "<tr><td colspan=4>Export Data Separator:<input type=radio name=sep_char value=com checked=checked />\",\"(comma) &nbsp;&nbsp;  <input type=radio name=sep_char value=scl />\";\"(semi-colon) &nbsp;&nbsp; <input type=radio name=sep_char value=spc />\" \"(space) &nbsp;&nbsp; <input type=submit name=export_submit value=\" Export... \" /></td></tr>\n");
    }else
    {
        fprintf(sfp, "<tr><td colspan=4>There is no record data.</td></tr>\n");
    }

    fclose(sfp);

}

int find_first_record_file()
{
    
    gpsrecord_file_index=0;

    char fullfile[30];
    char tmp_buff[101];
    char timestr0[30];
    char timestr1[30];
    int year0[MAX_REC_FILE_NUM];
    int mon0[MAX_REC_FILE_NUM];
    int day0[MAX_REC_FILE_NUM]={0,0,0,0,0,0,0,0,0,0,0};//as check ==0
    int hour0[MAX_REC_FILE_NUM];
    int min0[MAX_REC_FILE_NUM];
    int sec0[MAX_REC_FILE_NUM];
    int i=0;
    int j=0;
    char valid_data=0;

    FILE* f=NULL;
    char *p,*p1;
    strcpy(timestr0,"000000000000");

    for(i=0;i<MAX_REC_FILE_NUM;i++)
    {
        sprintf(fullfile,"%s%d",gpsrecord_dir,i);
        f = fopen(fullfile, "r");
        if(f==NULL)break;
read_next_line_find1:
        if(fgets(tmp_buff,100,f))
        {
            if(tmp_buff[0]=='#')goto read_next_line_find1;
            if(tmp_buff[6]!=',')goto read_next_line_find1;
            if(tmp_buff[1]==',' || tmp_buff[2]==',' || tmp_buff[3]==',' || tmp_buff[4]==',' || tmp_buff[5]==',' )goto read_next_line_find1;; 

            tmp_buff[99]=0;
            p1=tmp_buff;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)
            {
                fclose(f);
                break;
            }
            p++;
            p1=p;
            p=NULL;
            p=strchr(p1,',');
            if(p==NULL)
            {
                fclose(f);
                break;
            }
            *p=0;
            sscanf(tmp_buff, "%2d%2d%2d,%2d%2d%2d*",&day0[i], &mon0[i], &year0[i], &hour0[i], &min0[i], &sec0[i]);
            if(day0[i]==0)
            {
                fclose(f);
                break;
            }
            sprintf(timestr1,"%02d%02d%02d%02d%02d%02d",year0[i],mon0[i],day0[i],hour0[i],min0[i],sec0[i]);
            if(strcmp(timestr1,timestr0)>0)
            {
                gpsrecord_file_index=i;
                strcpy(timestr0,timestr1);
            }

        }else 
        {
            fclose(f);
            break;
        }
        fclose(f);
    }

    return gpsrecord_file_index;

}

//:list ->/var/run/gpsloadrecview
void view_record_file_dateline(int file_id)
{
    char *outputfile="/var/run/gpsloadrecview";
    FILE *sfp=NULL;
    if ((sfp = fopen(outputfile, "w")) == 0) {
        printf("<tr><td colspan=4>System IO Error!</td></tr>\n");
        return;
    }
    
    int i_datacnt=0;    
    char fullfile[30];
    char tmp_buff[201];
    char timestr1[20];
    char pos_buff[30];
    char input_buff[10];
    char output_buff[10];
    char speed_buff[10];
    char direction_buff[10];
    char rssi_buff[10];
    char alt_buff[10];
    int year1=0;
    int mon1=0;
    int day1=0;
    int hour1=0;
    int min1=0;
    int sec1=0;
    int i=0;
    int j=0;
    char valid_data=0;
    char c_n=' ';
    char c_e=' ';
    float lat1=0;
    float lng1=0;

    base_pos_lat[0]=0;
    base_pos_lng[0]=0;
    base_utc_time[0]=0;
    base_utc_date[0]=0;
    base_pos_lat_NS=0;
    base_pos_lng_WE=0;
    input_buff[0]=0;
    output_buff[0]=0;

    FILE* f=NULL;
    char *p,*p1;
    strcpy(timestr1,"000000000000");
    sprintf(fullfile,"%s%d",gpsrecord_dir,file_id);
    f = fopen(fullfile, "r");
    if(f!=NULL)
    {
        while(fgets(tmp_buff,200,f))
        {
            speed_buff[0]=0;
            direction_buff[0]=0;
            rssi_buff[0]=0;
            alt_buff[0]=0;

            tmp_buff[200]=0;

            if(tmp_buff[0]=='#')
            {
                //#Local Time,DI,DO,RSSI
                p=NULL;
                p=strchr(tmp_buff,',');
                if(p==NULL)continue;

                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                strcpy(input_buff,p1);

                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                strcpy(output_buff,p1);

                p++;
                p1=p;
                strcpy(rssi_buff,p1);

                fprintf(sfp, "<tr><td>Local Record</td><td></td><td></td><td>%s</td><td>%s</td><td></td><td></td><td>-%s</td><td></td></tr>\n",input_buff,output_buff,rssi_buff);
                input_buff[0]=0;
                output_buff[0]=0;

                continue;
            }

            if(tmp_buff[6]==',' && tmp_buff[1]!=',' && tmp_buff[2]!=',' && tmp_buff[3]!=',' && tmp_buff[4]!=',' && tmp_buff[5]!=',' )
            {
                base_pos_lat[0]=0;
                base_pos_lng[0]=0;
                base_utc_time[0]=0;
                base_utc_date[0]=0;
                base_pos_lat_NS=0;
                base_pos_lng_WE=0;
                c_n=' ';
                c_e=' ';
                p1=tmp_buff;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                strncpy(base_utc_date,tmp_buff,6);
                base_utc_date[6]=0;
                strncpy(base_utc_time,p1,6);
                base_utc_time[6]=0;
                *p=0;
                sscanf(tmp_buff, "%2d%2d%2d,%2d%2d%2d*",&day1, &mon1, &year1, &hour1, &min1, &sec1);
                if(day1<1 || day1>31)continue;
                sprintf(timestr1,"20%02d-%02d-%02d %02d:%02d:%02d",year1,mon1,day1,hour1,min1,sec1);
                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                strcpy(pos_buff,p1);
                if(strlen(pos_buff)<4)continue;
                strcpy(base_pos_lat,pos_buff);
                sscanf(pos_buff, "%f", &lat1);
                i=lat1/100;
                lat1=i+(lat1-i*100)/60+0.000005;
                p++;
                if(*p=='S')c_n='-';
                base_pos_lat_NS=*p;
                p1=p+2;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                strcpy(pos_buff,p1);
                if(strlen(pos_buff)<4)continue;
                strcpy(base_pos_lng,pos_buff);
                sscanf(pos_buff, "%f", &lng1);
                i=lng1/100;
                lng1=i+(lng1-i*100)/60+0.000005;
                p++;
                if(*p=='W')c_e='-';
                base_pos_lng_WE=*p;
                p1=p+2;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;

                *p=0;
                if(p-p1>0) 
                {
                    strcpy(alt_buff,p1);
                }

                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                strcpy(input_buff,p1);

                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                strcpy(output_buff,p1);

                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                strcpy(speed_buff,p1);

                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                strcpy(direction_buff,p1);

                p++;
                p1=p;
                strcpy(rssi_buff,p1);

                i_datacnt++;
                fprintf(sfp, "<tr><td>%s</td><td>%c%.6f</td><td>%c%.6f</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>-%s</td><td>%s</td></tr>\n",timestr1,c_n,lat1,c_e,lng1,input_buff,output_buff,speed_buff,direction_buff,rssi_buff,alt_buff);

            }else
            {
                if(base_pos_lng[0]==0)continue;

                //Time difference replace,Lat difference replace,Lon difference replace,alt,speed, Direction
                c_n=' ';
                c_e=' ';
                p1=tmp_buff;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                if(parseback_data(base_utc_time,p1,timestr1)<1)continue;
                sscanf(timestr1, "%2d%2d%2d",&hour1, &min1, &sec1);
                sscanf(base_utc_date, "%2d%2d%2d",&day1, &mon1, &year1);
                if(day1<1 || day1>31)continue;
                sprintf(timestr1,"20%02d-%02d-%02d %02d:%02d:%02d",year1,mon1,day1,hour1,min1,sec1);

                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                if(parseback_data(base_pos_lat,p1,pos_buff)<1)continue;
                if(strlen(pos_buff)<4)continue;
                sscanf(pos_buff, "%f", &lat1);
                i=lat1/100;
                lat1=i+(lat1-i*100)/60+0.000005;
                if(base_pos_lat_NS=='S')c_n='-';
                //printf("***%s:%s,%s,%s,%.6f<br>\n",timestr1,base_pos_lat,p1,pos_buff,lat1);
                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                if(parseback_data(base_pos_lng,p1,pos_buff)<1)continue;
                if(strlen(pos_buff)<4)continue;
                sscanf(pos_buff, "%f", &lng1);
                i=lng1/100;
                lng1=i+(lng1-i*100)/60+0.000005;
                if(base_pos_lng_WE=='W')c_e='-';

                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                strcpy(alt_buff,p1);

                p++;
                p1=p;
                p=NULL;
                p=strchr(p1,',');
                if(p==NULL)continue;
                *p=0;
                strcpy(speed_buff,p1);

                p++;
                p1=p;
                strcpy(direction_buff,p1);
                if(strlen(direction_buff)>4)direction_buff[0]=0;

                i_datacnt++;
                fprintf(sfp, "<tr><td>%s</td><td>%c%.6f</td><td>%c%.6f</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td></td><td>%s</td></tr>\n",timestr1,c_n,lat1,c_e,lng1,input_buff,output_buff,speed_buff,direction_buff,alt_buff);

            }


        }
        fclose(f);
        fprintf(sfp, "<tr><td colspan=6>Totally here are %d valid position records.</td></tr>\n",i_datacnt);
    }else
    {
        fprintf(sfp, "<tr><td colspan=6>There is no record data.</td></tr>\n");
    }
    fclose(sfp);

}



int check_carrier_imei(char *imeibuff)
{
    char tmp_buff[256];
    FILE* f;
    char *p;
    int result=-1;
    int j;
    imeibuff[0]=0;

    if (!(f = fopen("/var/run/VIP4G_status", "r"))) 
    {
        //do nothing.
    }else
    {
        while (fgets(tmp_buff, 256, f)) 
        {
            p=NULL;
            p = strstr(tmp_buff, "imei=");//imei=" 012773002002648"
            if (p != NULL)
            {
                p+=strlen("imei=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        imeibuff[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0)
                {
                    imeibuff[j]=0;
                    result=1;
                }
                break;
            }//if p
        }
    }
    if(f)fclose(f);

    return result;
}

/*
 * connect to gpsd with socket
 */
static int netlib_connectsock(const char *host, const char *service, const char *protocol)
{
    struct hostent *phe;
    struct servent *pse;
    struct protoent *ppe;
    struct sockaddr_in sin;
    int s, type, proto, one = 1;

    memset((char *) &sin, 0, sizeof(sin));
    /*@ -type -mustfreefresh @*/
    sin.sin_family = AF_INET;
    if ((pse = getservbyname(service, protocol)))
        sin.sin_port = htons(ntohs((unsigned short) pse->s_port));
    else if ((sin.sin_port = htons((unsigned short) atoi(service))) == 0)
        return -3;

    if ((phe = gethostbyname(host)))
        memcpy((char *) &sin.sin_addr, phe->h_addr, phe->h_length);
    else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
        return -2;

    ppe = getprotobyname(protocol);
    if (strcmp(protocol, "udp") == 0) {
        type = SOCK_DGRAM;
        proto = (ppe) ? ppe->p_proto : IPPROTO_UDP;
    } else {
        type = SOCK_STREAM;
        proto = (ppe) ? ppe->p_proto : IPPROTO_TCP;
    }

    if ((s = socket(PF_INET, type, proto)) < 0)
        return -4;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one))==-1) {
        (void)close(s);
        return -5;
    }

    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        (void)close(s);
        return -6;
    }

#ifdef IPTOS_LOWDELAY
    int opt = IPTOS_LOWDELAY;
    (void)setsockopt(s, IPPROTO_IP, IP_TOS, &opt, sizeof opt);

#endif
#ifdef TCP_NODELAY
    if (type == SOCK_STREAM)
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
#endif
    return s;

}


//:list ->/var/run/gpsloadreclist
//:view id ->/var/run/gpsloadrecview
//:send mode,server,port,items...
//:trace id/quick ? 
int main(int argc, char *argv[])
{
    int i,j;
    FILE* f=NULL;
    char *p,*p1,*p2;
    int load_type=0;
    int load_para=-1;
    if(argc==2)
    {
        if(strcmp(argv[1],"list")==0)
        {
            load_type=1;
            list_record_file_dateline();
            return 0;
        }
    }
    if(argc==3)
    {
        if(strcmp(argv[1],"view")==0)
        {
            load_type=2;
            load_para=atoi(argv[2]);
            if(load_para<0 || load_para>=MAX_REC_FILE_NUM)
            {
                load_type=0;
            }
        }else if(strcmp(argv[1],"trace")==0)
        {
            load_type=3;
            if(strcmp(argv[2],"quick")==0)load_para=REC_FILE_ALL;
            else
            {
                load_para=atoi(argv[2]);
                if(load_para<0 || load_para>=MAX_REC_FILE_NUM)
                {
                    load_type=0;
                }
            }
        }
    }
    if(argc>4 && strcmp(argv[1],"send")==0)
    {
        load_type=4;
        //  /bin/gpsloadrecord send FORM_Mode FORM_SERVER_Port FORM_SERVER_Address
        if(argc!=6)load_type=0;
        else
        {
            c_SEND_Mode=argv[2][0];
            if(c_SEND_Mode<'A' || c_SEND_Mode>'F')return 0;
            i_SERVER_Port=atoi(argv[3]);
            if(i_SERVER_Port<1||i_SERVER_Port>65535)return 0;
            strcpy(s_SERVER_Address,argv[4]);
            if(strlen(s_SERVER_Address)<4)return 0;
            p1=argv[5];
            p=strchr(p1,'-');
            if(p==NULL)return 0;
            else
            {
                for(j=0;j<MAX_REC_FILE_NUM;j++)senddata_list[j]=0;
                load_para=0;

                p++;
                p1=p;
                p=strchr(p1,'-');
                while(p!=NULL)
                {
                    *p=0;
                    j=atoi(p1);
                    if(j>=0 && j<MAX_REC_FILE_NUM)
                    {
                        load_para=1;
                        senddata_list[j]=1;
                    }else break;
                    p++;
                    p1=p;
                    p=strchr(p1,'-');
                }
            }
            if(load_para!=1)load_type=0;
        }
    }
    if (load_type<1 || load_type >4)return 0;


    if(load_type==1)
    {
        list_record_file_dateline();
        return 0;
    }

    if(load_type==2)
    {
        view_record_file_dateline(load_para);
        return 0;
    }

    if(load_type==3)
    {
        trace_record_file_dateline(load_para);
        return 0;
    }

    if(load_type==4)
    {

        int fd=0;
        char tmp_buff[151];
        char send_buff[256];

        char time_buff[20];
        char date_buff[20];
        char pos_buff[30];
        char alt_buff[10];
        int year0;
        int mon0;
        int day0=0;//as check ==0
        int hour0;
        int min0;
        int sec0;

        int send_err_cnt=0;
        int i_tmp;
        char fullfile[30];
        float lat1=0;
        float lng1=0;
        float alt1=0;

        char s_TRACKER_IMEI[50];

        s_TRACKER_IMEI[0]=0;
        if(c_SEND_Mode=='C' || c_SEND_Mode=='D')
        {
            check_carrier_imei(s_TRACKER_IMEI);
            if(strlen(s_TRACKER_IMEI)<8)
            {
                printf("Error! Can't get valid IMEI.\n");
                return -1;
            }
        }

        find_first_record_file();
        //printf("send:%c,%s,%d:%d\n",c_SEND_Mode,s_SERVER_Address,i_SERVER_Port,gpsrecord_file_index);


        sprintf(tmp_buff,"%d",i_SERVER_Port);
        if(c_SEND_Mode=='A' || c_SEND_Mode=='C' || c_SEND_Mode=='E')//UDP
        {
            //fd = netlib_connectsock( s_SERVER_Address, tmp_buff, "udp");

            
            struct hostent *hp;
            char **p0;
            struct in_addr in;

            hp=gethostbyname(s_SERVER_Address);
            if(hp == NULL) 
            {
                printf("Error!Can't set up network connection.\n");
                return -1;
            }

            if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
            {
                printf("Error!Can't set up network connection.\n");
                return -1;
            }


            p0 = hp->h_addr_list;
            memcpy(&in.s_addr, *p0, sizeof(in.s_addr));
            sprintf(tmp_buff,"%s",inet_ntoa(in));

            addr_remote_udp.sin_family = AF_INET;                   // Protocol Family
            addr_remote_udp.sin_port = htons(i_SERVER_Port);                 // Port number
            inet_pton(AF_INET, tmp_buff, &(addr_remote_udp.sin_addr)); // Net Address
            bzero(&(addr_remote_udp.sin_zero), 8);                  // Flush the rest of struct
            
        }

        if(c_SEND_Mode=='B' || c_SEND_Mode=='D' || c_SEND_Mode=='F')//TCP
        {
            fd = netlib_connectsock( s_SERVER_Address, tmp_buff, "tcp");
        }
        if(fd<=0)
        {
            //printf("Error!Can't set up network connection.%c,%s,%s,%d\n",c_SEND_Mode,s_SERVER_Address,tmp_buff,fd);
            printf("Error!Can't set up network connection.\n");
            return -1;
        }

        if(c_SEND_Mode=='D')//TCP
        {
            sprintf(send_buff,"$FRLIN,IMEI,%s,*00",s_TRACKER_IMEI);
            set_nmea_checksum(send_buff,(strlen(send_buff)-3));
            strcat(send_buff,"\r\n");
            send(fd, send_buff, strlen(send_buff),0);
            usleep(5000);
            strcpy(send_buff,"$FRWDT,NMEA*78\r\n");
            send(fd, send_buff, strlen(send_buff),0);
        }

        char input_buff[10];
        char output_buff[10];
        char direction_buff[10];
        char speed_buff[10];
        float f_speed;
        char c_n=' ';
        char c_e=' ';
        char new_lat_buff[20];
        char new_lng_buff[20];


        for(i=0;i<MAX_REC_FILE_NUM;i++)
        {
            j=i+1+gpsrecord_file_index;
            if(j>=MAX_REC_FILE_NUM)j=j-MAX_REC_FILE_NUM;
            if(senddata_list[j]==1)
            {

                base_pos_lat[0]=0;
                base_pos_lng[0]=0;
                base_utc_time[0]=0;
                base_utc_date[0]=0;
                base_pos_lat_NS=0;
                base_pos_lng_WE=0;
                input_buff[0]=0;
                output_buff[0]=0;
                sprintf(fullfile,"%s%d",gpsrecord_dir,j);
                f = fopen(fullfile, "r");
                if(f==NULL)break;
                while(fgets(tmp_buff,150,f))
                {
                    tmp_buff[149]=0;
                    if(tmp_buff[0]<'0' || tmp_buff[0]>'9')continue;

                    //time_buff,pos_buff,date_buff,alt_buff,day0,year0...
                    day0=0;
                    direction_buff[0]=0;
                    alt_buff[0]=0;
                    speed_buff[0]=0;
                    new_lat_buff[0]=0;
                    new_lng_buff[0]=0;
                    lat1=0;
                    lng1=0;

                    //decode record data here:
                    if(tmp_buff[6]==',' && tmp_buff[1]!=',' && tmp_buff[2]!=',' && tmp_buff[3]!=',' && tmp_buff[4]!=',' && tmp_buff[5]!=',' )
                    {
                        base_pos_lat[0]=0;
                        base_pos_lng[0]=0;
                        base_utc_time[0]=0;
                        base_utc_date[0]=0;
                        base_pos_lat_NS=0;
                        base_pos_lng_WE=0;
                        input_buff[0]=0;
                        output_buff[0]=0;
                        c_n=' ';
                        c_e=' ';
                        day0=0;

                        p1=tmp_buff;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        p++;
                        p1=p;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        strncpy(base_utc_date,tmp_buff,6);
                        base_utc_date[6]=0;
                        strncpy(base_utc_time,p1,6);
                        base_utc_time[6]=0;
                        *p=0;
                        sscanf(tmp_buff, "%2d%2d%2d,%2d%2d%2d*",&day0, &mon0, &year0, &hour0, &min0, &sec0);
                        if(day0<1 || day0>31)continue;
                        p++;
                        p1=p;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        *p=0;
                        strcpy(pos_buff,p1);
                        if(strlen(pos_buff)<4)continue;
                        strcpy(base_pos_lat,pos_buff);
                        strcpy(new_lat_buff,pos_buff);
                        sscanf(pos_buff, "%f", &lat1);
                        i_tmp=lat1/100;
                        lat1=i_tmp+(lat1-i_tmp*100)/60+0.000005;
                        p++;
                        c_n=*p;
                        base_pos_lat_NS=*p;
                        p1=p+2;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        *p=0;
                        strcpy(pos_buff,p1);
                        if(strlen(pos_buff)<4)continue;
                        strcpy(base_pos_lng,pos_buff);
                        strcpy(new_lng_buff,pos_buff);
                        sscanf(pos_buff, "%f", &lng1);
                        i_tmp=lng1/100;
                        lng1=i_tmp+(lng1-i_tmp*100)/60+0.000005;
                        p++;
                        c_e=*p;
                        base_pos_lng_WE=*p;
                        p1=p+2;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;

                        *p=0;
                        if(p-p1>0) 
                        {
                            strcpy(alt_buff,p1);
                        }

                        p++;
                        p1=p;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        *p=0;
                        strcpy(input_buff,p1);

                        p++;
                        p1=p;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        *p=0;
                        strcpy(output_buff,p1);

                        p++;
                        p1=p;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        *p=0;
                        strcpy(speed_buff,p1);

                        p++;
                        p1=p;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        *p=0;
                        strcpy(direction_buff,p1);
                    }else
                    {
                        if(base_pos_lng[0]==0 || base_utc_date[0]==0)continue;

                        //Time difference replace,Lat difference replace,Lon difference replace,alt,speed, Direction
                        c_n=' ';
                        c_e=' ';
                        p1=tmp_buff;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        *p=0;
                        if(parseback_data(base_utc_time,p1,pos_buff)<1)continue;
                        sscanf(pos_buff, "%2d%2d%2d",&hour0, &min0, &sec0);
                        sscanf(base_utc_date, "%2d%2d%2d",&day0, &mon0, &year0);
                        if(day0<1 || day0>31)continue;

                        p++;
                        p1=p;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        *p=0;
                        if(parseback_data(base_pos_lat,p1,pos_buff)<1)continue;
                        if(strlen(pos_buff)<4)continue;
                        strcpy(new_lat_buff,pos_buff);
                        sscanf(pos_buff, "%f", &lat1);
                        i_tmp=lat1/100;
                        lat1=i_tmp+(lat1-i_tmp*100)/60+0.000005;
                        c_n=base_pos_lat_NS;

                        p++;
                        p1=p;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        *p=0;
                        if(parseback_data(base_pos_lng,p1,pos_buff)<1)continue;
                        if(strlen(pos_buff)<4)continue;
                        strcpy(new_lng_buff,pos_buff);
                        sscanf(pos_buff, "%f", &lng1);
                        i_tmp=lng1/100;
                        lng1=i_tmp+(lng1-i_tmp*100)/60+0.000005;
                        c_e=base_pos_lng_WE;

                        p++;
                        p1=p;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        *p=0;
                        strcpy(alt_buff,p1);

                        p++;
                        p1=p;
                        p=NULL;
                        p=strchr(p1,',');
                        if(p==NULL)continue;
                        *p=0;
                        strcpy(speed_buff,p1);

                        p++;
                        p1=p;
                        while(*p!='\r' && *p!='\n' && *p!=0)p++;
                        *p=0;
                        strcpy(direction_buff,p1);
                    }

                    if(day0<1||day0>31||lng1==0)continue;

                    //pack data
                    if(c_SEND_Mode=='A' || c_SEND_Mode=='B' || c_SEND_Mode=='C'  || c_SEND_Mode=='D')
                    {
                        if(speed_buff[0]!=0)
                        {
                            f_speed=(float)atoi(speed_buff);
                            f_speed=f_speed/1.852;
                            sprintf(speed_buff,"%.2f",f_speed);
                        }
                        sprintf(tmp_buff,"$GPRMC,%02d%02d%02d,A,%s,%c,%s,%c,%s,%s,%02d%02d%02d,,,*00",hour0,min0,sec0,new_lat_buff,c_n,new_lng_buff,c_e,speed_buff,direction_buff,day0,mon0,year0);
                        set_nmea_checksum(tmp_buff,(strlen(tmp_buff)-3));
                        strcat(tmp_buff,"\r\n");

                        send_buff[0]=0;
                        if(c_SEND_Mode=='C') 
                        {
                            //$FRLIN,IMEI,1234123412341234,*7B
                            //$GPRMC,154403.000,A,6311.64120,N,01438.02740,E,0.000,0.0,270707,,*0A
                            sprintf(send_buff,"$FRLIN,IMEI,%s,*00",s_TRACKER_IMEI);
                            set_nmea_checksum(send_buff,(strlen(send_buff)-3));
                            strcat(send_buff,"\r\n");
                            strcat(send_buff,tmp_buff);
                        }

                        if(c_SEND_Mode=='D')
                        {
                            //Here some problems about tcp will not accept for history data.   "Live Event Rule"??


                            strcpy(send_buff,tmp_buff);
                            /*
                            //$FRCMD,IMEI,_SendMessage,,DDMM.mmmm,N,DDMM.mmmm,E,AA.a,SSS.ss,HHH.h,DDMMYY,hhmmss.dd,valid,var1=value,var2=value...*XX
                            sprintf(send_buff,"$FRCMD,%s,_SendMessage,,%s,%s,,,%s,%s,1*00",s_TRACKER_IMEI,pos_buff,alt_buff,date_buff,time_buff);
                            set_nmea_checksum(send_buff,(strlen(send_buff)-3));
                            strcat(send_buff,"\r\n");
                            */
                        }

                        if(c_SEND_Mode=='A' || c_SEND_Mode=='B') 
                        {
                            if(alt_buff[0]!=0)
                            {
                                sprintf(send_buff,"$GPGGA,%02d%02d%02d,%s,%c,%s,%c,8,,,%s,M,,,,*00",hour0,min0,sec0,new_lat_buff,c_n,new_lng_buff,c_e,alt_buff);
                                set_nmea_checksum(send_buff,(strlen(send_buff)-3));
                                strcat(send_buff,"\r\n");
                            }
                            strcat(send_buff,tmp_buff);
                        }
                    }

                    if(c_SEND_Mode=='E' || c_SEND_Mode=='F')
                    {
                        if(c_n=='S')lat1=(-1)*lat1;
                        if(c_e=='W')lng1=(-1)*lng1;
                         sprintf(send_buff,"20%02d-%02d-%02d %02d:%02d:%02d,%.6f,%.6f,%s\n",year0,mon0,day0,hour0,min0,sec0,lat1,lng1,alt_buff);
                    }


                    if(c_SEND_Mode=='A' || c_SEND_Mode=='C' || c_SEND_Mode=='E')
                    {
                        i_tmp=sendto(fd,send_buff, strlen(send_buff),0,(struct sockaddr *)&(addr_remote_udp), sizeof(struct sockaddr_in));
                    }else i_tmp= send(fd, send_buff, strlen(send_buff),0);
                    if(i_tmp<=0)
                    {
                          if(errno==EINTR || errno==ENOSPC || errno==EAGAIN || errno==EWOULDBLOCK)
                          {
                              send_err_cnt++;
                          }else
                          {
                              printf("Network error!Can't finish data sending.\n");
                              shutdown(fd,SHUT_RDWR);
                              close(fd);
                              fclose(f);
                              return -2;
                          }
                    }else send_err_cnt=0;
                    if(send_err_cnt>20)
                    {
                        printf("Sending error!Can't finish data sending.\n");
                        shutdown(fd,SHUT_RDWR);
                        close(fd);
                        fclose(f);
                        return -3;
                    }
                }
                fclose(f);
            }
        }

        printf("Successfully sent position data to %s:%d.\n",s_SERVER_Address,i_SERVER_Port);
        shutdown(fd,SHUT_RDWR);
        close(fd);
        return 0;
    }

return 0;

}
