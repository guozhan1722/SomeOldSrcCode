
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>

#include "BaseDef.h"
#include "GobiDef.h"
#include "GobiApi.h"
#include "CallBackFun.h"


typedef struct
{
	char deviceNode[256];
	char deviceKey[16];
}DEVICE;



ULONG  Technology = 1;
ULONG  PrimaryDNS = 0;
ULONG  SecondaryDNS = 0;
ULONG  PrimaryNBNS  = 0;
ULONG  SecondaryNBNS = 0;
CHAR  pAPNName[20] = "staticip.apn";
ULONG  IPAddress = 0;
ULONG  Authentication = 0;
CHAR  pUsername[20] = "" ;
CHAR  pPassword[20] = "";
ULONG  SessionId = 0;
ULONG  FailureReason = 0;


struct sigaction	sact;
ULONG  gFinish = 0;


void sighandler(int signal)
{
    gFinish = 1;
    return;
}


int main(int arg, char *argv[])
{
	void *dlHandle = NULL;
	BYTE size = 2;
	ULONG ipA = 0;
	ULONG tec = 1;
	DEVICE tt;
	ULONG power = 9;
	CHAR revision[50];
	ULONG ulStatus = 0;
	GobiApi *obj = GobiApi::GetInstance();
    ULONG ulerror = 1;

    int i;

	if(NULL == obj)
	{
		printf("Get GobiApi instance failed!\n");
		return -1;
	}


    //Setup Signal handler here
    #if 0
    sact.sa_handler = sighandler;
    sigaction(SIGHUP, &sact, NULL);
    sigaction(SIGINT, &sact, NULL);
    sigaction(SIGQUIT, &sact, NULL);
    sigaction(SIGPIPE, &sact, NULL);
    sigaction(SIGTERM, &sact, NULL);
    #endif

    ulerror=obj->EnumerateDevices((BYTE *)&size, (BYTE *)&tt);
	printf("ulerror=%d\n",ulerror);

	printf("detective devices num:%d\n",(int)size);
    printf("%s %s\n",tt.deviceNode, tt.deviceKey);
    printf("%s\n","node");
	for (i = 0; i < sizeof(tt.deviceNode); i++)
	{
       // printf("%02x\n",tt.deviceNode[i]);
    }
    printf("%s\n","key");
	for (i = 0; i < sizeof(tt.deviceKey); i++)
	{
      //  printf("%02x\n",tt.deviceKey[i]);
    }
    
	//printf("Connect = %d\n",obj->Connect(tt.deviceNode, tt.deviceKey));
	//printf("callback = %ld\n",obj->SetSessionStateCallback(SessionStateCallback));
  /*  ULONG                      profileType,
    ULONG *                    pPDPType, 
    ULONG *                    pIPAddress, 
    ULONG *                    pPrimaryDNS, 
    ULONG *                    pSecondaryDNS, 
    ULONG *                    pAuthentication, 
    CHAR *                     pName, 
    CHAR *                     pAPNName, 
    CHAR *                     pUsername,
    CHAR *                     pPassword );*/

    ulerror = obj->Connect(tt.deviceNode, tt.deviceKey);
    printf("Connect=%d\n",ulerror);

    sleep(2);

    ulerror = obj->SetDefaultProfile(0,NULL,NULL,NULL,NULL,NULL,NULL,pAPNName,NULL,NULL);
    printf("setprofile=%d\n",ulerror);

    sleep(2);

    ULONG foo = 1;
	ulerror = obj->SetEnhancedAutoconnect(1,&foo);
    printf("Autoconnect=%d\n",ulerror);

/*
	printf("callback = %ld\n",obj->SetSessionStateCallback(SessionStateCallback));


	printf("start= %ld\n",obj->StartDataSession(&Technology,
                                        &PrimaryDNS,
					&SecondaryDNS,
					&PrimaryNBNS,
					&SecondaryNBNS,
					 pAPNName,
					&IPAddress,
					&Authentication,
					pUsername,
					pPassword,
					& SessionId,
					& FailureReason));
	printf("FailureReason = %ld\n",FailureReason);
 

	for (i = 0; i < 1000000; i++)
	{
		obj->GetSessionState(&ulStatus);
		printf("SessionState = %d\n",ulStatus);
		obj->GetIPAddress(&ipA);
		printf("ipA = %ld\n",ipA);
		sleep(10);
	}

	obj->StopDataSession(SessionId);
    */
    

    for (i = 0; i < 600; i++)

    {

        sleep(1);
    }

    ulerror = obj->SetEnhancedAutoconnect(0,&foo);
    printf("Disconnect=%d\n",ulerror);

    obj->Disconnect();

    GobiApi::ReleaseInstance();

	return 0;
}

