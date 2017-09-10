#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>


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
CHAR  pAPNName[20] = "internet.com";
ULONG  IPAddress = 0;
ULONG  Authentication = 0;
CHAR  pUsername[20] = "" ;
CHAR  pPassword[20] = "";
ULONG  SessionId = 0;
ULONG  FailureReason = 0;
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

	printf("Connect = %d\n",obj->Connect(tt.deviceNode, tt.deviceKey));
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

	for (i = 0; i < 10; i++)
	{
		obj->GetSessionState(&ulStatus);
		printf("SessionState = %d\n",ulStatus);
		obj->GetIPAddress(&ipA);
		printf("ipA = %ld\n",ipA);
		sleep(10);
	}
	obj->StopDataSession(SessionId);
	obj->Disconnect();
	GobiApi::ReleaseInstance();
	return 0;
}



























