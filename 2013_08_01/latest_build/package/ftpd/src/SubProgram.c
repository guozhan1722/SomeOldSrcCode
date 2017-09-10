#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>  

#include "SubProgram.h"
/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

int SubProgram_Start(char *pszPathName, char *pszArguments)
{

    int MAX_ARGS=8,MAX_STRING=8;
    char * pArgs[MAX_ARGS];
   // unsigned long ulNumArgs = 0;
   //   char tmpArgs[MAX_STRING];
  //  char *pArg = NULL;
    int pid = 0;
    int status;  
    pArgs[0] = NULL;
   
   if (pszArguments != NULL)
       {
       /*
       strcpy(tmpArgs, pszArguments);

       // 'execl' return -1 on error.
       pArg = strtok(tmpArgs, " ");
       while ((pArg != NULL) && (ulNumArgs < MAX_ARGS))
           {

           pArgs[ulNumArgs] = pArg;
           ulNumArgs++;

           pArg = strtok(NULL, " ");
           }
	   
	   */
 pArgs[0]=pszPathName;	   
	   
 pArgs[1]=pszArguments;
	   
 pArgs[2] = NULL;
	   
       }    

  //  pArgs[ulNumArgs] = NULL;
//printf("pArgs0 = %s\n",pArgs[0]);  
//printf("pArgs1 = %s\n",pArgs[1]);  
    if (!(pid = vfork()))
        {
        // We are the child!      
 //syslog(LOGOPTS,"pArgs = %s\n",pArgs[0]);  
	// printf("pszPathName = %s\n",pszPathName); 
		seteuid(0);        
        if (execvp(pszPathName, pArgs) == -1)
            {
			printf("error execvp\n");  
            //  exit(1);
            }
       // syslog(LOGOPTS,"Child PID after = %d\n",pid);          
        }
    else
        {
         wait(&status);
        }

    return(pid);

}

int fetch_Local_IP_MAC(char *ifname, char *mac)
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
		return 0;
		}
	strncpy(ifr.ifr_name,ifname,sizeof(ifr.ifr_name)-1);
	ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';

	if (-1==ioctl(sock, SIOCGIFHWADDR, &ifr))
		{
		syslog(LOGOPTS,"fetch_Local_IP_MAC:%s ioctl(SIOCGIFHWADDR)",ifname);         
		//perror("ioctl(SIOCGIFHWADDR) ");
		strcpy(mac, temp);    
		close(sock);
		return 0;
		}

	for (j=0, k=0; j<6; j++)
		{
		k+=snprintf(temp+k, sizeof(temp)-k-1, j ? ":%02X" : "%02X",
					(int)(unsigned int)(unsigned char)ifr.ifr_hwaddr.sa_data[j]);
		}
	temp[sizeof(temp)-1]='\0';
	strcpy(mac, temp);          
	close(sock);  
	return 1;
	}

