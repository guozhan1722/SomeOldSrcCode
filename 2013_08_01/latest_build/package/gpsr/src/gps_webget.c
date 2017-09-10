/*
 * simple program to get gps data from web get from cell id and lac 
 * parameters: 
 * 1)update 
 * 2)A/B 
 * 3)write latitue longtitude (altitude) 
 * return: 
 * 1:valid data; 
 * -1:no valid data or file; 
 * 0:no changed data 
 */


#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <netinet/ether.h>
#include <sys/wait.h>
#include <syslog.h>



union __hexint_struct
{
    char c[4];
    int i;
};



static int scpytrim(char *dst, const char *src, int len)
{
    char *d = dst;
    const char *s = src;
    int i, j = 0;

    for (i=0; i<len; i++) {
//        if (s[i]=='\r' || s[i]=='\n' || s[i]=='\0' || s[i]==' ') {
        if (s[i]=='\r' || s[i]=='\n' || s[i]=='\0' ) {
            continue;
        }
        d[j] = s[i];
        j++;
    }
    return j;
}

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



int main( int argc, char *argv[])
{

    FILE* f;
    char* p;
    int j;
    char buffer_get[512];
    char buffer_tmp[40];
    char lac[20];
    char mccmnc[10];
    char mcc[4],mnc[4];
    char cellId[20];
    char s_carrs_geodata;
    int argv_type;
    char gps_lat[20],gps_lon[20],gps_elev[20],gps_rad[10],gps_source[10],gps_cellid[20],gps_lac[10],gps_mcc[10],gps_mnc[10];
    time_t start;
    int online_en;

    /*
     * 1)update 
     * 2)A/B 
     * 3)write latitue longtitude (altitude) 
    */
    argv_type=0;
    online_en=0;
    if(argc!=2 && argc!=4 && argc!=5 )return -1; 
    if(argc==2)
    {
        if(strcmp(argv[1],"A")==0)argv_type=2;
        if(strcmp(argv[1],"update")==0)
        {
            argv_type=1;
            online_en=1;
        }
        if(strcmp(argv[1],"B")==0)
        {
            argv_type=2;
            online_en=1;
        }
    }
    if(argc>3 && strcmp(argv[1],"write")==0)
    {
        argv_type=3;
        online_en=0;
        strcpy(gps_lat,argv[2]);
        strcpy(gps_lon,argv[3]);
        gps_elev[0]=0;
        if(argc==5)strcpy(gps_elev,argv[4]);
    }
    if(argv_type==0)return -1;

    lac[0]=0;
    cellId[0]=0;
    mccmnc[0]=0;
    mcc[0]=0;
    mnc[0]=0;

    if (!(f = fopen("/var/run/VIP4G_status", "r"))) 
    {
        //do nothing.
        return -1;
    }else
    {
        while (fgets(buffer_get, 256, f)) 
        {
            p=NULL;
            p = strstr(buffer_get, "cell_id=");//cell_id=" 2,1, 2BC4, 4BA5A63, 2"
            if (p != NULL)
            {
                while (*p != ','  && (*p!='\n') )p++; //first space
                if(*p == ',') 
                {
                    p++;
                    while (*p != ','  && (*p!='\"') )p++; //first space
                    if(*p!='\"')p++;
                    while (*p == ' ')p++; //first space
                    j=0;
                    while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') && (*p!=',')  &&(j<20)) //lac
                        {
                            lac[j]=*p;
                            p++;
                            j++;
                         }
                    if(j>0)lac[j]=0;
                    while (*p != ','  && (*p!='\"') )p++; //first space
                    if(*p!='\"')p++;
                    while (*p == ' ')p++; //first space
                    j=0;
                    while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') && (*p!=',')  &&(j<20)) //cellid
                        {
                            cellId[j]=*p;
                            p++;
                            j++;
                         }
                    if(j>0)cellId[j]=0;
                }


            }//if p

            p = strstr(buffer_get, "mccmnc=");//mccmnc=" 0,2, 302220 ,2"
            if (p != NULL)
            {
                while((*p != ',')&& (*p!='\n'))p++;
                if(*p == ',') 
                {
                    p++;
                    while((*p != ',') && (*p!='\n'))p++;
                    if(*p == ',') 
                    {
                        p++;
                        while(*p == ' ')p++;
                        j=0;
                        while ((*p != ',')&&(*p != '\"')&& (*p!=' ') && (*p!='\n') &&(j<20)) 
                            {
                                mccmnc[j]=*p;
                                p++;
                                j++;
                             }
                         if(j>3)
                         {
                             mccmnc[j]=0;
                             mcc[0]=mccmnc[0];
                             mcc[1]=mccmnc[1];
                             mcc[2]=mccmnc[2];
                             mcc[3]=0;
                             mnc[0]=mccmnc[3];
                             if(j>4)mnc[1]=mccmnc[4];
                             else  mnc[1]=0;
                             if(j>5)mnc[2]=mccmnc[5]; 
                             else  mnc[2]=0;
                             mnc[3]=0;
                         }
                         else
                         {
                             mccmnc[0]=0;
                             mcc[0]=0;
                             mnc[0]=0;
                         }
                    }
                }

            }//if p

        }//while
    }//else
    if(f)fclose(f);

    if(cellId[0]==0||lac[0]==0||mcc[0]==0||mnc[0]==0)
    {
        if (!(f = fopen("/var/run/std_lte_statue", "r"))) 
        {
            //do nothing.
        }else
        {
            while (fgets(buffer_get, 256, f)) 
            {
                p=NULL;
                p = strstr(buffer_get, "cellid=");//cellid="29441"
                if (p != NULL)
                {
                    p = strchr(buffer_get, '=');
                    if ( p!= NULL)
                    {
                        p++;
                        sprintf(cellId,"%s",p);
                    }
                }

                p=NULL;
                p = strstr(buffer_get, "lac=");//lac="65534"
                if (p != NULL)
                {
                    p = strchr(buffer_get, '=');
                    if ( p!= NULL)
                    {
                        p++;
                        sprintf(lac,"%s",p);
                    }
                }

                p=NULL;
                p = strstr(buffer_get, "mccmnc=");//mccmnc=" 0,2, 302220 ,2"
                if (p != NULL)
                {
                    while((*p != ',')&& (*p!='\n'))p++;
                    if(*p == ',') 
                    {
                        p++;
                        while((*p != ',') && (*p!='\n'))p++;
                        if(*p == ',') 
                        {
                            p++;
                            while(*p == ' ')p++;
                            j=0;
                            while ((*p != ',')&&(*p != '\"')&& (*p!=' ') && (*p!='\n') &&(j<20)) 
                                {
                                    mccmnc[j]=*p;
                                    p++;
                                    j++;
                                 }
                             if(j>3)
                             {
                                 mccmnc[j]=0;
                                 mcc[0]=mccmnc[0];
                                 mcc[1]=mccmnc[1];
                                 mcc[2]=mccmnc[2];
                                 mcc[3]=0;
                                 mnc[0]=mccmnc[3];
                                 if(j>4)mnc[1]=mccmnc[4];
                                 else  mnc[1]=0;
                                 if(j>5)mnc[2]=mccmnc[5]; 
                                 else  mnc[2]=0;
                                 mnc[3]=0;
                                 unsigned int MCC=atoi(mcc);
                                 unsigned int MNC=atoi(mnc);
                                 sprintf(mcc,"%d",MCC);
                                 sprintf(mnc,"%d",MNC);
                             } else mccmnc[0]=0;

                        }
                    }

                }//if p
            }
        }
        if(f)fclose(f);
    }


if(argv_type==3)
{
    if (!(f = fopen("/var/run/GPS_position_carr", "w"))) 
    {
        return -1;
    }else
    {
        time(&start);
        sprintf(buffer_tmp,"%s",ctime(&start));
        p=buffer_tmp;
        p+=strlen(buffer_tmp)-3;
        for(j=0;j<3;j++){
            if(*p=='\r' || *p=='\n')*p=0;
            p++;
        }
        *p=0;
        sprintf(buffer_get,"updatetime=\"%s\"\r\n",buffer_tmp);
        sprintf(buffer_tmp,"latitude=\"%s\"\r\n",gps_lat);
        strcat(buffer_get,buffer_tmp);
        sprintf(buffer_tmp,"longitude=\"%s\"\r\n",gps_lon);
        strcat(buffer_get,buffer_tmp);

        sprintf(buffer_tmp,"elevation=\"%s\"\r\n",gps_elev);
        strcat(buffer_get,buffer_tmp);
        sprintf(buffer_tmp,"radius=\"5\"\r\n");
        strcat(buffer_get,buffer_tmp);
        sprintf(buffer_tmp,"source=\"GPS\"\r\n");
        strcat(buffer_get,buffer_tmp);

        sprintf(buffer_tmp,"cellid=\"%s\"\r\n",cellId);
        strcat(buffer_get,buffer_tmp);
        sprintf(buffer_tmp,"lac=\"%s\"\r\n",lac);
        strcat(buffer_get,buffer_tmp);
        sprintf(buffer_tmp,"mcc=\"%s\"\r\n",mcc);
        strcat(buffer_get,buffer_tmp);
        sprintf(buffer_tmp,"mnc=\"%s\"\r\n",mnc);
        strcat(buffer_get,buffer_tmp);

        fprintf(f,"%s",buffer_get);
    }
    if(f)fclose(f);
    return 1;
}


    if(cellId[0]==0||lac[0]==0||mcc[0]==0||mnc[0]==0)return -1;

if(argv_type==2)
{
        gps_cellid[0]=0;
        if (!(f = fopen("/var/run/GPS_position_carr", "r"))) 
        {
            //do nothing.
        }else
        {
            while (fgets(buffer_get, 256, f)) 
            {
                p=NULL;
                p = strstr(buffer_get, "cellid=");//latitude="51.142962"
                if (p != NULL)
                {
                    p+=strlen("cellid=")+1;
                    while (*p == ' ')p++; //first space
                    j=0;
                    while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<10)) 
                        {
                            gps_cellid[j]=*p;
                            p++;
                            j++;
                         }
                    if(j>0) 
                        {
                            gps_cellid[j]=0;
                        }
                }//if p
    
                p=NULL;
                p = strstr(buffer_get, "lac=");//longitude="-114.075094"
                if (p != NULL)
                {
                    p+=strlen("lac=")+1;
                    while (*p == ' ')p++; //first space
                    j=0;
                    while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<10)) 
                        {
                            gps_lac[j]=*p;
                            p++;
                            j++;
                         }
                    if(j>0) 
                        {
                            gps_lac[j]=0;
                        }
                }//if p
    
                p=NULL;
                p = strstr(buffer_get, "mnc=");//longitude="-114.075094"
                if (p != NULL)
                {
                    p+=strlen("mnc=")+1;
                    while (*p == ' ')p++; //first space
                    j=0;
                    while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<10)) 
                        {
                            gps_mnc[j]=*p;
                            p++;
                            j++;
                         }
                    if(j>0) 
                        {
                            gps_mnc[j]=0;
                        }
                }//if p
    
                p=NULL;
                p = strstr(buffer_get, "mcc=");//longitude="-114.075094"
                if (p != NULL)
                {
                    p+=strlen("mcc=")+1;
                    while (*p == ' ')p++; //first space
                    j=0;
                    while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<10)) 
                        {
                            gps_mcc[j]=*p;
                            p++;
                            j++;
                         }
                    if(j>0) 
                        {
                            gps_mcc[j]=0;
                        }
                }//if p
    
            }//while
        }//else
        if(f)fclose(f);

        if(gps_cellid[0]!=0)
        {
            if(strcmp(gps_cellid,cellId)==0)
            {
                if(strcmp(gps_lac,lac)==0)
                {
                    if(strcmp(gps_mcc,mcc)==0)
                    {
                        if(strcmp(gps_mnc,mnc)==0)
                        {
                            return 0;

                        }
                    }
                }
            }
        }
}//type 2

if(online_en!=1)return -1;
//argv_type=1 and need to update

    //geo data comes from cell id and lac
    s_carrs_geodata='0';
    if(lac[0]!=0 && mcc[0]!=0)
    {
        unsigned int cid0,lac0;
        sscanf(cellId,"%x",&cid0);
        sscanf(lac,"%x",&lac0);
        unsigned int MCC=atoi(mcc);
        unsigned int MNC=atoi(mnc);
        if(sendLocationRequest(MNC,MCC,cid0,lac0)==true)
        {
            s_carrs_geodata='1';
        }
    }


    if(s_carrs_geodata=='1')
    {
        double lat = (double)0;
        double lon = (double)0; 
        int rad = 0;
        char pd[55];
        int ifp = open("/var/run/nmap", O_RDWR,S_IRWXU|S_IRWXO|S_IRWXG);
        if (ifp > 0) 
        {
            memset(pd, 0, 55);
            read(ifp, pd, 55);
            close(ifp);

            short opcode1 = (short)(pd[0] << 8 | pd[1]);
            char opcode2 = pd[2];
            if ((opcode1 == 0x0e)&&(opcode2 == 0x1b)) 
            {
               int ret_code = (int)((pd[3] << 24) | (pd[4] << 16) | (pd[5] << 8) | (pd[6]));
                if (ret_code == 0) 
                {
                    union __hexint_struct tmp_ic;
                    tmp_ic.c[0]=pd[7];
                    tmp_ic.c[1]=pd[8];
                    tmp_ic.c[2]=pd[9];
                    tmp_ic.c[3]=pd[10];
                    lat=(double)tmp_ic.i/1000000;
                    tmp_ic.c[0]=pd[11];
                    tmp_ic.c[1]=pd[12];
                    tmp_ic.c[2]=pd[13];
                    tmp_ic.c[3]=pd[14];
                    lon=(double)tmp_ic.i/1000000;
                    tmp_ic.c[0]=pd[15];
                    tmp_ic.c[1]=pd[16];
                    tmp_ic.c[2]=pd[17];
                    tmp_ic.c[3]=pd[18];
                    rad=tmp_ic.i;
                    //don't use like this: lat = ((double)((pd[7] << 24) | (pd[8] << 16) | (pd[9] << 8) | (pd[10])));

    //printf("\n***lat:%f lon:%f rad:%d***\n",lat,lon,rad);
    //                sprintf(buffer_get,"latitude=\"%f\"\r\nlongitude=\"%f\"\r\nradius=\"%d\"\r\nupdatetime=\"%s\"\r\n",lat,lon,rad,ctime(&start));
    //printf("%s",buffer_get);

                    if (!(f = fopen("/var/run/GPS_position_carr", "w"))) 
                    {
                        return -1;
                    }else
                    {
                        time(&start);
                        sprintf(buffer_tmp,"%s",ctime(&start));
                        p=buffer_tmp;
                        p+=strlen(buffer_tmp)-3;
                        for(j=0;j<3;j++){
                            if(*p=='\r' || *p=='\n')*p=0;
                            p++;
                        }
                        *p=0;
                        sprintf(buffer_get,"updatetime=\"%s\"\r\n",buffer_tmp);
                        sprintf(buffer_tmp,"latitude=\"%f\"\r\n",lat);
                        strcat(buffer_get,buffer_tmp);
                        sprintf(buffer_tmp,"longitude=\"%f\"\r\n",lon);
                        strcat(buffer_get,buffer_tmp);

                        sprintf(buffer_tmp,"elevation=\"\"\r\n");
                        strcat(buffer_get,buffer_tmp);
                        sprintf(buffer_tmp,"radius=\"%d\"\r\n",rad);
                        strcat(buffer_get,buffer_tmp);
                        sprintf(buffer_tmp,"source=\"Network\"\r\n");
                        strcat(buffer_get,buffer_tmp);

                        sprintf(buffer_tmp,"cellid=\"%s\"\r\n",cellId);
                        strcat(buffer_get,buffer_tmp);
                        sprintf(buffer_tmp,"lac=\"%s\"\r\n",lac);
                        strcat(buffer_get,buffer_tmp);
                        sprintf(buffer_tmp,"mcc=\"%s\"\r\n",mcc);
                        strcat(buffer_get,buffer_tmp);
                        sprintf(buffer_tmp,"mnc=\"%s\"\r\n",mnc);
                        strcat(buffer_get,buffer_tmp);

                        fprintf(f,"%s",buffer_get);
                    }
                    if(f)fclose(f);
                    return 1;
                }
            }
        } 
    }
    return -1;

}








