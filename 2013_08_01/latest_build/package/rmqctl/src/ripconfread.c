#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <syslog.h>
#include <sys/sysinfo.h>
#include <assert.h>
#include <stdlib.h>
#include "ripconfread.h"

const char* rdconf = "/etc/config/route";
const char* wtconf = "/var/run/ripconf";
const char* wtconfz = "/var/run/zebraconf";

char RouterType = 2;
char RouterAble = 0;

static void Ipstr2Ipuint(char *ipstr, unsigned int *Ipuint)
{
    unsigned char a, b, c, d;
    sscanf(ipstr, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d );
    *Ipuint = ( a << 24 ) | ( b << 16 ) | ( c << 8 ) | d;
}

int ConvertMask(char *net, char *Mask)
{
    char *tmp;
    unsigned int Mint = 0;
    int i;
    int cnt = 0;
    char t[8];

    tmp = strchr(Mask, '/');
    if(tmp) {
        memcpy(net, Mask, tmp-Mask+1);
        net[tmp-Mask+1] = 0;
    }    
    Ipstr2Ipuint(tmp+1, &Mint);
    for(i=0; i<32; i++) {
        if(Mint&(1<<i)) cnt++;
    }
    if( (cnt>=0) && (cnt<33) ) {
        sprintf(t, "%d", cnt);
        strcat(net, t);
        return 0;
    }
    return -1;
}

int main(void)
{
    FILE* frd = NULL;
    FILE* fwt = NULL;
    char line[LINE_LENGTH];
    char ipadd[32];
    char network[100];
    char net[32];
    char *temp;
    int rtn = 0;
    
    fwt = fopen(wtconfz, "wt");
    if(fwt == NULL) 
    {
        printf("Couldn't write %s.\n", wtconfz);
        rtn = -1;
        goto end;
    }
    fwrite("hostname microhard\n", strlen("hostname microhard\n"), 1, fwt);
    fwrite("password zebra\n", strlen("password zebra\n"), 1, fwt);
    fwrite("enable password zebra\n", strlen("enable password zebra\n"), 1, fwt);
    fclose(fwt);

    frd = fopen(rdconf, "rt");
    if(frd == NULL) 
    {
        printf("Couldn't read %s.\n", rdconf);
        rtn = -2;
        goto end;
    }

    fwt = fopen(wtconf, "wt");
    if(fwt == NULL) 
    {
        printf("Couldn't write %s.\n", wtconf);
        rtn = -3;
        goto end;
    }   

    while(fgets(line,LINE_LENGTH,frd)) {
        temp = strstr(line, "protocol_0");
        if(temp) {
            temp = fgets(line,LINE_LENGTH,frd);
            if(temp) {  
                temp = strstr(line, "select");
                if (temp) { // version first
                    temp = strstr(temp, "enable");
                    if(temp) {
                        RouterAble = 1;
                        temp = fgets(line,LINE_LENGTH,frd);
                        if(temp) {
                            temp = strstr(line, "version");
                            if(temp) {
                                temp = strstr(temp, "1");
                                if(temp) RouterType = 1;
                                else RouterType = 2; 
                                break;                               
                            }
                        }    
                    } else {
                        RouterAble = 0;
                        break;
                    }
                } else {    // enable first
                    temp = strstr(line, "version");
                    if (temp) {
                        temp = strstr(temp, "1");
                        if(temp) RouterType = 1;
                        else RouterType = 2;
                    }

                    temp = fgets(line,LINE_LENGTH,frd);
                    if (temp) {
                        temp = strstr(line, "select");
                        if(temp) {
                            temp = strstr(temp, "enable");
                            if(temp) {
                                 RouterAble = 1;
                                 break;
                            } else {
                                RouterAble = 0;
                                break;
                            }
                        }
                    }                   
                }
            }
        }
    }

    //printf("RouterAble=%d, RouterType=%d.\n", RouterAble, RouterType);
    if(!RouterAble) goto end;

    fwrite("hostname ripd\n", strlen("hostname ripd\n"), 1, fwt);
    fwrite("password zebra\n", strlen("password zebra\n"), 1, fwt);
    fwrite("log stdout\n", strlen("log stdout\n"), 1, fwt);
    fwrite("!\n", strlen("!\n"), 1, fwt);
    fwrite("router rip\n", strlen("router rip\n"), 1, fwt);
    if(RouterType == 1)
        fwrite("version 1\n", strlen("version 1\n"), 1, fwt);

    fseek(frd, SEEK_SET, 0);
    while(fgets(line,LINE_LENGTH,frd)){
        temp = strstr(line, "network_");
        if(temp) {
            temp = fgets(line,LINE_LENGTH,frd);
            if(temp) {                
                temp = strstr(line, "network");
                if(temp) {
                    if(strlen(temp+10)<=30) {
                        strcpy(ipadd, temp+10);
                        temp = strchr(ipadd, '\'');
                        if(temp) {                            
                            *temp = 0;
                            strcpy(network, " network ");
                            if(!ConvertMask(net, ipadd)){
                                strcat(network, net);
                                strcat(network, "\n");
                                fwrite(network, strlen(network), 1, fwt);
                            }
                        }
                    }
                }
            }
        }
    }
    fwrite("!\n", strlen("!\n"), 1, fwt);
        
end:
    if(frd)fclose(frd);   
    if(fwt)fclose(fwt);     
    if((!rtn) && RouterAble) {
       system("/usr/local/zebra/zebra -d -f /var/run/zebraconf");
       system("/usr/local/zebra/ripd -d -f /var/run/ripconf");
    }
    return rtn;
}


