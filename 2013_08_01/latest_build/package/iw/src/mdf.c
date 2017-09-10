#include "mdf.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <linux/ioctl.h>

#define RESETMDF 'r'
#define CMD_RESETMDF _IO(RESETMDF, 0)
#define REMOVEMDF 'm'
#define CMD_REMOVEMDF _IO(REMOVEMDF, 0)
#define MAXCFGLENGTH 2048
#define WIFIDEVICE "/proc/sys/net/ipv4/conf/wlan0"

static int rd_mdfcfg(char *BandName, struct wifpara* tmp)
{
  char buffer[MAXCFGLENGTH];
  char *posname, *pos, *pos1;
  char tmpname[20];
  FILE *pmdf = NULL;
  int rtn = -1;

  if(strlen(BandName)> sizeof(tmpname)-4)return -10;
  strcpy(tmpname, ":");
  strcat(tmpname, BandName);
    
  pmdf = fopen(MDFCFG, "rb");
  if (pmdf == NULL){
    printf("Can't open file %s \n", MDFCFG);
    return -10;
  } 
  
  while (fgets(buffer,MAXCFGLENGTH,pmdf))
  {
      posname = strstr(buffer, tmpname);
      if (posname<=0) continue;      
      tmp->mdfmode = atoi(buffer);

      pos = strchr(buffer, '=');   
      if(pos<=0){
          rtn = -2;
          break;
      }
      tmp->lfreq = atoi(pos+1);
  
      pos = strchr(pos, ',');   
      if(pos<=0){
          rtn = -3;
          break;
      }
      pos++;
      tmp->rfreq = atoi(pos);
  
      pos = strchr(pos, ',');   
      if(pos<=0){
          rtn = -4;
          break;
      }
      pos++;
      tmp->bandwidth = atoi(pos);
  
      pos = strchr(pos, ',');   
      if(pos<=0){
          rtn = -5;
          break;
      }
      pos++;
      tmp->overlap = atoi(pos);
  
      pos = strchr(pos, ',');   
      if(pos<=0){
          rtn = -6;
          break;
      }
      pos++;
      tmp->antgai = atoi(pos);
  
      pos = strchr(pos, ',');   
      if(pos<=0){
          rtn = -7;
          break;
      }
      pos++;
      tmp->maxeirp = atoi(pos);

      pos = strchr(pos, ',');   
      if(pos<=0){
          rtn = -8;
          break;
      }
      pos++;
      tmp->regflag = atoi(pos);
      fclose(pmdf);
      return 0;
  }
  fclose(pmdf); 
  return rtn;
}

static int setup_mdf(char *arg)
{
    struct wifpara tmp;  
    int pdev = 0;
    int rtn = 0;
 
    rtn = access(WIFIDEVICE, 0);  
    if(rtn)return -20;

    memset(&tmp, 0, sizeof(struct wifpara));
  
    rtn = rd_mdfcfg(arg, &tmp);  
    if(rtn)return rtn; 

    printf("%s: %d, %d, %d, %d, %d, %d, %d, %d\n", arg, 
           tmp.lfreq, tmp.rfreq, tmp.bandwidth, tmp.overlap, 
           tmp.antgai, tmp.maxeirp, tmp.regflag, tmp.mdfmode);
     
    pdev= open(MDFDEV,O_RDWR);
    if (!pdev){    
        printf("Can't open file %s \n", MDFDEV);
        return -1;
    }
    write(pdev, &tmp, sizeof(struct wifpara));
    close(pdev);
    return 0;
}

static int CreateCardtype(struct wifpara * wifi)
{
    FILE *pmdf = NULL;
    char type[256] = {0};
    char temp[32];

    switch(wifi->cardmod)
    {
    case CARDMODE(AR5K_MICROHARD49G):
        strcpy(type, "49G ");
        
        break;    
    case CARDMODE(AR5K_MICROHARD23G):
        strcpy(type, "23G ");
        break;
    case CARDMODE(AR5K_MICROHARD24G):
        strcpy(type, "24G ");
        break;
    case CARDMODE(AR5K_ATHROS2G):
        strcpy(type, "A2G ");
        break;
    case CARDMODE(AR5K_ATHROS5G):
        strcpy(type, "A5G ");        
        break;
    default:        
        strcpy(type, "UNKNOWN ");
        break;
    }
  
    sprintf(temp, "%d\n", wifi->curntmd);
    strcat(type, temp);

    pmdf = fopen(CARDTYPE, "wt");
    if (pmdf == NULL){
        printf("Can't open file %s \n", CARDTYPE);
        return -1;
    }
    fwrite(type, strlen(type), 1, pmdf);
    fclose(pmdf);             
}

static int mdf_status(void)
{
  struct wifpara tmp;  
  FILE *pmdf = NULL;
  int pdev = 0;
  int rtn;  

  rtn = access(WIFIDEVICE, 0);  
  if(rtn)
  {
      printf("%s not found\n", WIFIDEVICE);
      return -20;
  }

  memset(&tmp, 0 , sizeof(tmp));
  pdev= open(MDFDEV,O_RDWR);
  if (!pdev){    
    printf("Can't open file %s \n", MDFDEV);
    return -1;
  }
  read(pdev, &tmp, sizeof(struct wifpara));  
  close(pdev); 

  CreateCardtype(&tmp);
  printf("Card type %d, Current mode %d\n", tmp.cardmod, tmp.curntmd);
  return 0;
}	

static void mdf_usage(void)
{
	printf("mdf -w bandname\n");  
	printf("mdf -v: show version info\n");  
	printf("mdf -h: show help message\n");  
	printf("mdf -r: read current mdf mode\n");  
}

static void mdf_version(void)
{
	printf("mdf versio 0.03\n");  
}


int main(int argc,  char **argv)
{
	int goterr = 0;

	switch(argc)
	{
	case 2:			
		if(!strcmp(argv[1], "-r"))	// read current mode
      			goterr = mdf_status();	
		else if(!strcmp(argv[1], "-v"))	// show version     
      			mdf_version();			
		else mdf_usage();
		break;
	case 3:
		if(!strcmp(argv[1], "-w"))	// setup band config
		{
			goterr = setup_mdf(argv[2]);	
		}
		else mdf_usage();
		break;
	default:
		mdf_usage();
		break;
	}
	return(goterr);
}



