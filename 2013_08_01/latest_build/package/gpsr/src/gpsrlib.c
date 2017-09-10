/*
 * gpsreporting lib
 *
 * there are some functions serve gpsr
 *
 */


#include "gpsrlib.h"

struct gps_position base_gps_pos;
char gps_text[64];

char* find_nmea_start(const char* buffer)
{
	return strchr(buffer,'$');
}

int nmea_length(const char* startp)
{

	char* nsp=strchr(startp,'*');
	if(nsp!=NULL)
		return nsp-startp+3;

	syslog(LOGOPTS,"nmea_length return -1\n");
	return -1;
}

char* find_sep_nmea_start(const char* buffer, const char* sep)
{
	return strstr(buffer,sep);
}


static int scpytrim(char *dst, const char *src, int len)
{
    char *d = dst;
    const char *s = src;
    int i, j = 0;

    for (i=0; i<len; i++) {
        if (s[i]=='\r' || s[i]=='\n' || s[i]=='\0' || s[i]==' ') {
            continue;
        }
        d[j] = s[i];
        j++;
    }
    d[j]='\0';
    return j;
}

static void set_textpos_start()
{
    char tmpbuf[64];
    float lat1=0;
    float lng1=0;
    int i;
    char c_n=' ';
    char c_e=' ';
    i=base_gps_pos.lat/100;
    lat1=i+(base_gps_pos.lat-i*100)/60;
    i=base_gps_pos.lng/100;
    lng1=i+(base_gps_pos.lng-i*100)/60;
    if(base_gps_pos.c_north=='S')
    {
        c_n='-';
    }
    if(base_gps_pos.c_east=='W')
    {
        c_e='-';
    }
//    sprintf(gps_text,"%c%f,%c%f",base_gps_pos.c_nort,lat1,c_e,lng1);

    sprintf(tmpbuf,"%c%f,%c%f",c_n,lat1,c_e,lng1);
    scpytrim(gps_text,tmpbuf,strlen(tmpbuf));

    return ;
}

char* find_type_nmea_start(const char* buffer,int type)
{
	switch(type){
	case GPGGA:
		return strstr(buffer,"$GPGGA");
	case GPGSA:
		return strstr(buffer,"$GPGSA");
	case GPGSV:
		return strstr(buffer,"$GPGSV");
	case GPRMC:
		return strstr(buffer,"$GPRMC");
	case GPVTG:
		return strstr(buffer,"$GPVTG");
	case GPZDA:
		return strstr(buffer,"$GPZDA");
	case GPGLL:
		return strstr(buffer,"$GPGLL");
    default:
		break;
	}//end of switch

	return NULL;
}

int fetch_one_pattern(const char* rbuf,char* pbuf)
{
	   char* startp;
	   char* endp;
	   char* sp;
	   char* ep;
	   char* tp;
	   char* pos;
	   char pch[32];

	   char* epch;
	   int flag=0;

	   //syslog(LOGOPTS,"GPSRLIB fetch_one_pattern start\n");
	   pos=pbuf;
	   /* search one pattern */

	   startp = strchr(rbuf,'$');
	   if(startp==NULL){
		   syslog(LOGOPTS,"GPSRLIB fetch_one_pattern error: can't find any sentence.\n");
		   return -3;
	   }
	   //syslog(LOGOPTS,"GPSRLIB fetch_one_pattern startp at\n%s\n",startp);
       endp = strrchr(rbuf,'*');
	   if(endp==NULL){
		   syslog(LOGOPTS,"GPSRLIB fetch_one_pattern error: can't find any sentence.\n");
		   return -3;
	   }
	   sp=startp;
	   //syslog(LOGOPTS,"GPSRLIB fetch_one_pattern sp at\n%s\n",sp);
	   tp = strchr(sp,',');
	   //syslog(LOGOPTS,"GPSRLIB fetch_one_pattern gets fisrt sentence is %s\n",tp);
	   //syslog(LOGOPTS,"GPSRLIB fetch_one_pattern sp at\n%s\n",sp);
	   if(tp==NULL){
		   syslog(LOGOPTS,"GPSRLIB fetch_one_pattern error: can't find any sentence.\n");
		   return -3;
	   }
	   bzero(pch,sizeof(pch));
	   //syslog(LOGOPTS,"GPSRLIB fetch_one_pattern tp-sp=%d\n",tp-sp);
	   epch=strncpy(pch,sp,tp-sp);
	   if(pch==NULL) syslog(LOGOPTS,"GPSRLIB fetch_one_pattern strncat pch==null\n");
	   //syslog(LOGOPTS,"GPSRLIB fetch_one_pattern strncat pch %s\n",pch);
	   if(strcmp(pch,"$GPGSV")==0){
		   flag=1;
		   int i=0;
		   for(i=0;i<2;i++){
			   tp = strchr(tp+1,',');

		   }
		   bzero(pch,sizeof(pch));
		   strncpy(pch,sp,tp-sp);
		   //syslog(LOGOPTS,"GPSRLIB fetch_one_pattern gets third word of GSV sentence is %s\n",pch);
	   }

	   sp = strstr(startp,pch);
	   if(sp==NULL) syslog(LOGOPTS,"GPSRLIB fetch_one_pattern strncat sp==null\n");
	   //syslog(LOGOPTS,"GPSRLIB fetch_one_pattern sp at\n%s\n",sp);
	   ep = strstr(sp+1,pch);

	   if(ep==NULL){
		   endp+=4;
	   }else{
		   //syslog(LOGOPTS,"GPSRLIB fetch_one_pattern ep at\n%s\n",ep);
		   endp=ep;

	   }


	   size_t plen=endp-startp;
       if(strncpy(pos,startp,plen)>0){
    	   pos[plen]='\0';
    	   //syslog(LOGOPTS,"The one pattern data:\n%s\n",pos);
    	   //syslog(LOGOPTS,"GPSRLIB fetch_one_pattern return\n");
		   return 0;
	   }else{
		   syslog(LOGOPTS,"GPSRLIB fetch_one_pattern error: strncpy copy error or just 0 byte.\n");
		   return 1;
	  }


}

#define NIBBLE2HEX(c) ((c) > 9 ? (c) + 'A' - 10 : (c) + '0')
void set_nmea_checksum(char *buf,int buf_inx)
{
    // buf is a char[] array which contains the NMEA sentence without trailing checksum.
    // E.g. "$FRLIN,,user1,8IVHF" and "*7A" will be added
    // buf_inx is an index to the last free position in the buffer
    int checksum = 0;
    int inx;
    for(inx = 1; inx < buf_inx; inx++)
    {
          checksum ^= buf[inx];
    }
    buf[buf_inx++] = '*';
    buf[buf_inx++] = NIBBLE2HEX((checksum >> 4) & 0xf);
    buf[buf_inx++] = NIBBLE2HEX(checksum & 0xf);
}


int send_select_nmea(const char* startp,char* mss_type,char* sbuf)
{

   char* sp;
   char* ep;
   char* pos;
   int i,len,type,nmealen,num;

   pos = sbuf;

   /* select all type nmea message */
   if(strchr(mss_type,'B')){

		  if(strcpy(sbuf,startp)>0){
			  //syslog(LOGOPTS,"send all messages, return\n");
			  return 0;
		  }else{
	    	  syslog(LOGOPTS,"GPSRLIB send_select_nmea error: strncpy copy error or just 0 byte.\n");
	    	  return -1;
		  }

   }

   len = (int)strlen(mss_type);
   //syslog(LOGOPTS,"GPSRLIB send_select_name: there are %d message types\n",len);
   if(len<=0){
	   syslog(LOGOPTS,"GPSRLIB send_select_nmea error: message type is NULL\n");
	   return -1;
   }
   for (i=0;i<len;i++){
	  type = mss_type[i]-'A';

	  if(type == 0) continue;
      if(type == TXTLL) 
      {
          set_textpos_start();
          strcat(sbuf,gps_text);
          strcat(sbuf,"\n");
          continue;
      }
      if(type == GATED) 
      {
          sp=find_type_nmea_start(startp,GPRMC);
          if(sp!=NULL)
          {
              nmealen=nmea_length(sp);
              if(nmealen>0)
               {
                  char tmp[100];
                  char imei[30]="";

                  int j;
                  FILE* f;
                  char *p;
                  char buffer_get[256];

                  j=0;
                  if (!(f = fopen("/var/run/VIP4G_status", "r"))) 
                  {
                      //do nothing.
                  }else
                  {
                      while (fgets(buffer_get, 256, f)) 
                      {
                        p=NULL;
                        p = strstr(buffer_get, "imei=");//imei=" 012773002002648"
                        if (p != NULL)
                        {
                            p+=strlen("imei=")+1;
                            while (*p == ' ')p++; //first space
                            while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                                {
                                    imei[j]=*p;
                                    p++;
                                    j++;
                                 }
                            if(j>0)imei[j]=0;
                        }//if p
                      }
                  }
                  if(f)fclose(f);
                  if(j==0)continue;


//                  strcpy(tmp,"$FRLIN,,gpsadmin,CHDAZJ*00");
                  sprintf(tmp,"$FRLIN,IMEI,%s,*00",imei);
                  set_nmea_checksum(tmp,(strlen(tmp)-3));
                  strcpy(sbuf,tmp);
                  strcat(sbuf,"\r\n");
                  tmp[0]=0;
                 if(strncat(tmp,sp,nmealen)>0)
                 {
                    set_nmea_checksum(tmp,(nmealen-3));
                    strcat(sbuf,tmp);
                    strcat(sbuf,"\r\n");
                 }
              }
          }
          continue;
      }
      
	  //syslog(LOGOPTS,"search type %c\n",mss_type[i]);
      sp=find_type_nmea_start(startp,type);
      //syslog(LOGOPTS,"search start at \n%s\n",sp);
      while(sp!=NULL)
      {

          if(sp!=NULL){
    	     nmealen=nmea_length(sp);
    	     if(nmealen<0){
    	    	 syslog(LOGOPTS,"GPSRLIB send_select_nmea error: nmea_length return < 0\n");
    	    	 break;
    	     }
             if(strncat(sbuf,sp,nmealen)>0){
            	 strcat(sbuf,"\n");
            	 //syslog(LOGOPTS,"copied sentence :\n%s\n",sbuf);
            	 sp=find_type_nmea_start(sp+1,type);
                 //pos = pos+nmealen;

             }else{
   	    	     syslog(LOGOPTS,"GPSRLIB send_select_nmea error: strncpy copy error or just 0 byte.\n");
   	    	     continue;
             }
          }
      }//end of while

   }//end of for
   if(sbuf==NULL)
	   return -1;
   else{

	   //syslog(LOGOPTS,"the context of sbuf:\n%s\n",sbuf);

	   return 0;
   }
}

static int gpsposition(char* start, int type, struct gps_position *result)
{
	char* sp;
    char* p;
	char tempString[100];
	int i,j;
		
	sp = start;
	if(sp==NULL)
	{
        syslog(LOGOPTS,"GPSRLIB gpsposition error: start is a null porinter\n");
		return -1;
	}
	switch (type)
	{
	case GPGGA:
	{
		for(i=0;i<2;i++)
		{
			sp = strchr(sp+1,',');
		}
		sp++;
		j=0;
		while(*sp != ',')
		{
			tempString[j]=*sp;
			sp++;
			j++;
		}
		tempString[j]='\0';
        if(j==0)return -2;
        sscanf(tempString, "%f", &(result->lat));
        result->c_north=*(sp+1);
		sp=strchr(sp+1,',');
		sp++;
		j=0;
		while(*sp != ',')
		{
			tempString[j]=*sp;
			sp++;
			j++;
		}
		tempString[j]='\0';
        if(j==0)return -2;
		sscanf(tempString, "%f", &(result->lng));	
        result->c_east=*(sp+1);
		return 0;
	}//end of gga
	case GPGLL:
	{
		sp = strchr(sp+1,',');
		sp++;
		j=0;
		while(*sp != ',')
		{
			tempString[j]=*sp;
			sp++;
			j++;
		}
		tempString[j]='\0';
        if(j==0)return -2;
		sscanf(tempString, "%f", &(result->lat));
        result->c_north=*(sp+1);
		sp=strchr(sp+1,',');
		sp++;
		j=0;
		while(*sp != ',')
		{
			tempString[j]=*sp;
			sp++;
			j++;
		}
		tempString[j]='\0';
        if(j==0)return -2;
		sscanf(tempString, "%f", &(result->lng));	
        result->c_east=*(sp+1);
		return 0;
	}//end of gll
	case GPRMC:
	{
		for(i=0;i<3;i++)
		{
			sp = strchr(sp+1,',');
		}
		sp++;
		j=0;
		while(*sp != ',')
		{
			tempString[j]=*sp;
			sp++;
			j++;
		}
		tempString[j]='\0';
        if(j==0)return -2;
		sscanf(tempString, "%f", &(result->lat));
        result->c_north=*(sp+1);
		sp=strchr(sp+1,',');
		sp++;
		j=0;
		while(*sp != ',')
		{
			tempString[j]=*sp;
			sp++;
			j++;
		}
		tempString[j]='\0';
        if(j==0)return -2;
		sscanf(tempString, "%f", &(result->lng));	
        result->c_east=*(sp+1);
		return 0;
	}//end of rmc
	default:
		break;
		
	
	}//end of switch
	return -1;		
}

int get_gps_pos(const char* buf, struct gps_position *result)
{
	char* sp;
	char* tp;
	char tempString[100];
	int i,j;

	sp = find_nmea_start(buf);
	if(sp==NULL)
	{
        syslog(LOGOPTS,"GPSRLIB get_gps_pos error: return null porinter for sp\n");
		return -1;
	}
	tp = find_sep_nmea_start(sp,"GPGGA");
	if(tp!=NULL){
		return gpsposition(tp,GPGGA,result);
	}
	tp = find_sep_nmea_start(sp,"GPGLL");
	if(tp!=NULL){
		return gpsposition(tp,GPGLL,result);
	}
	tp = find_sep_nmea_start(sp,"GPRMC");
	if(tp!=NULL){
		return gpsposition(tp,GPRMC,result);
	}
	return -1;
}

#define pi 3.14159265358979323846
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts decimal degrees to radians             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double deg2rad(double deg) 
{
	  return (deg * pi / 180);
}
	 
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts radians to decimal degrees             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double rad2deg(double rad) 
{
	  return (rad * 180 / pi);
}

double distance(double lat1, double lon1, double lat2, double lon2, char unit) 
{
	  double theta, dist;
	  theta = lon1 - lon2;
	  dist = sin(deg2rad(lat1)) * sin(deg2rad(lat2)) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));
	  dist = acos(dist);
	  dist = rad2deg(dist);
      if(dist<0)
          {
          dist=dist*(-1);
          }
	  dist = dist * 60 * 1.1515;
	  switch(unit) {
	    case 'M':
	      break;
	    case 'K':
	      dist = dist * 1.609344;
	      break;
	    case 'm':
	      dist = dist * 1609.344;
	      break;
	    case 'N':
	      dist = dist * 0.8684;
	      break;
	  }
//printf("Here %lf decible:%lf,%lf--%lf,%lf**\n",dist,lat1,lon1,lat2,lon2);
	  return (dist);
}
	 
double compute_distance(gps_position *current, gps_position *desired)
{

    double delng,delat,clng,clat;
    int i;
    double i_d;
    double c_n,c_e;

    if(current->lat==0 && current->lng==0) 
    {
        return 0;
    }

    if(current->c_north=='S')
    {
        c_n=-1;
    }else
    {
        c_n=1;
    }

    if(current->c_east=='W')
    {
        c_e=-1;
    }else
    {
        c_e=1;
    }
    clat=(double)current->lat;
    i=current->lat/100;
    i_d=(double)i;
    clat=c_n*(i_d+(clat-i_d*100)/60);

    clng=(double)current->lng;
    i=current->lng/100;
    i_d=(double)i;
    clng=c_e*(i_d+(clng-i_d*100)/60);

    if(desired->c_north=='S')
    {
        c_n=-1;
    }else
    {
        c_n=1;
    }

    if(desired->c_east=='W')
    {
        c_e=-1;
    }else
    {
        c_e=1;
    }
    delat=(double)desired->lat;
    i=desired->lat/100;
    i_d=(double)i;
    delat=c_n*(i_d+(delat-i_d*100)/60);

    delng=(double)desired->lng;
    i=desired->lng/100;
    i_d=(double)i;
    delng=c_e*(i_d+(delng-i_d*100)/60);

    return distance(clat,clng,delat,delng,'m');
}

/*
 * check gps data received in rbuf
 * return ready or waiting
 */
int check_gps_data(const char* rbuf, struct gps_position *gpsp)
{
	char* pos;

	pos = find_type_nmea_start(rbuf,GPRMC);
	if(pos == NULL) return WAITING;
	if(gpsposition(pos,GPRMC,gpsp)==0){
		if((gpsp->lng>0.0)&&(gpsp->lat>0.0)) return READY;
	}
	return WAITING;
}

/*
 * += operator for struct timeval
 */
void addeq_oper_timeval(struct timeval *leftp, const struct timeval *rightp){
	int carry;
	if((leftp->tv_usec+rightp->tv_usec)>=1000000){
	    carry = 1;
	    leftp->tv_usec=leftp->tv_usec+rightp->tv_usec-1000000;

	}
	else{
		carry = 0;
		leftp->tv_usec=leftp->tv_usec+rightp->tv_usec;
	}
	leftp->tv_sec=leftp->tv_sec+rightp->tv_sec+carry;
}
/*
 * + operator for struct timeval
 */
void add_oper_timeval(struct timeval *resultp,const struct timeval *leftp, const struct timeval *rightp){
	int carry;
	if(leftp->tv_usec+rightp->tv_usec>=1000000){
	    carry = 1;
	    resultp->tv_usec=leftp->tv_usec+rightp->tv_usec-1000000;

	}
	else{
		carry = 0;
		resultp->tv_usec=leftp->tv_usec+rightp->tv_usec;
	}
	resultp->tv_sec=leftp->tv_sec+rightp->tv_sec+carry;
}
/*
 * - operator for struct timeval
 */
void minus_oper_timeval(struct timeval *resultp, const struct timeval *leftp, const struct timeval *rightp){
	int carry;
	if(leftp->tv_usec<rightp->tv_usec){
		carry = 1;
		resultp->tv_usec = 1000000+leftp->tv_usec-rightp->tv_usec;
	}else{
		carry = 0;
		resultp->tv_usec = leftp->tv_usec-rightp->tv_usec;
	}
	resultp->tv_sec=leftp->tv_sec-rightp->tv_sec-carry;
}
/*
 * -= operator for struct timeval
 */
void minuseq_oper_timeval(struct timeval *resultp, const struct timeval *rightp){
	int carry;
	if(resultp->tv_usec<rightp->tv_usec){
		carry = 1;
		resultp->tv_usec = 1000000+resultp->tv_usec-rightp->tv_usec;
	}else{
		carry = 0;
		resultp->tv_usec = resultp->tv_usec-rightp->tv_usec;
	}
	resultp->tv_sec=resultp->tv_sec-rightp->tv_sec-carry;
}

