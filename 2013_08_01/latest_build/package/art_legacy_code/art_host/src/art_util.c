#include "project.h"
#include "wlantype.h"
#include "dk_structures.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

unsigned int FbytesSwap(unsigned int input)
{
#ifdef SWAPSOCKET
    return 
        ((input&0x000000FF)<<24) | 
        ((input&0x0000FF00)<<8) | 
        ((input&0x00FF0000)>>8) | 
        ((input&0xFF000000)>>24);
#else
    return input;
#endif
}

unsigned short TbytesSwap(unsigned short input)
{
#ifdef SWAPSOCKET
    return  
        ((input&0x00FF)<<8) | 
        ((input&0xFF00)>>8);    
#else
    return input;
#endif    
}

void AFbytesSwap(unsigned int *input, int length)
{
    int i;

    for(i=0; i<length; i++) {
        *(input+i) =  FbytesSwap(*(input+i));
    }
}

void ATbytesSwap(unsigned short *input, int length)
{
    int i;

    for(i=0; i<length; i++) {
        *(input+i) =  TbytesSwap(*(input+i));
    }
}


// use to convert big endian data to little endian data and vice versa
A_UINT32 ltob_l(A_UINT32 num)
{
	return (((num & 0xff) << 24) | ((num & 0xff00) << 8) | ((num & 0xff0000) >> 8) | ((num & 0xff000000) >> 24));
	
}

A_UINT16 ltob_s(A_UINT16 num)
{
	return (((num & 0xff) << 8) | ((num & 0xff00) >> 8));
	
}

A_UINT32 btol_l(A_UINT32 num)
{
	return (((num & 0xff) << 24) | ((num & 0xff00) << 8) | ((num & 0xff0000) >> 8) | ((num & 0xff000000) >> 24));
	
}

A_UINT16 btol_s(A_UINT16 num)
{
	return (((num & 0xff) << 8) | ((num & 0xff00) >> 8));
	
}

A_UINT32 swap_l(A_UINT32 num)
{
	return (((num & 0xff) << 24) | ((num & 0xff00) << 8) | ((num & 0xff0000) >> 8) | ((num & 0xff000000) >> 24));
	
}

A_UINT16 swap_s(A_UINT16 num)
{
	return (((num & 0xff) << 8) | ((num & 0xff00) >> 8));
	
}

// swap all the data in the memory block pointed by src
// and write the swapped data in dest
void swapAndCopyBlock_l(void *dest,void *src,A_UINT32 size)
{
	A_INT32 i;
	A_INT32 noWords;
	A_UINT32 *srcPtr;
	A_UINT32 *destPtr;
			
		noWords = size / sizeof(A_UINT32);
		srcPtr = (A_UINT32 *)src;
		destPtr = (A_UINT32 *)dest;
			
		for (i = 0;i < noWords; i++)
		{
			*destPtr = swap_l(*srcPtr);
			srcPtr++;
			destPtr++;
		}

	return;
	
}

// swap all the data in the memory block pointed by src
void swapBlock_l(void *src,A_UINT32 size)
{
	A_INT32 i;
	A_INT32 noWords;
	A_UINT32 *srcPtr;
				
		noWords = size / sizeof(A_UINT32);
		srcPtr = (A_UINT32 *)src;
			
        for (i = 0;i < noWords; i++)
		{
			*srcPtr = swap_l(*srcPtr);
			srcPtr++;
		}

	return;
	
}









A_UINT16 driverOpen()
{
  return 1;
}

A_STATUS connectSigHandler(void) 
{
    return A_OK;
}

void close_driver() 
{
}

A_UINT32 hwCfgRead32
(
	A_UINT16 devIndex,
	A_UINT32 address                    /* the address to read from */
)
{
	return 0;
}

A_STATUS get_device_client_info(MDK_WLAN_DEV_INFO *pdevInfo, PDRV_VERSION_INFO pDrvVer, PCLI_INFO cliInfo) 
{
    return A_OK;
}

void close_device(MDK_WLAN_DEV_INFO *pdevInfo) 
{
}

void hwCfgWrite32
(
	A_UINT16 devIndex,
	A_UINT32  offset,                    /* the address to write */
	A_UINT32  value                        /* value to write */
)
{
}

A_UINT32 hwIORead
(
	A_UINT16 devIndex,
	A_UINT32 offset                    /* the address to read from */
)
{
  return 0;
}

void hwIOWrite
(
	A_UINT16 devIndex,
	A_UINT32  offset,                    /* the address to write */
	A_UINT32  value                        /* value to write */
)
{
}

A_UINT32 openVisaSession()
{
	return 0;
}

int isRMsessionOpen()
{
		return 1;
}

A_UINT32 closeVisaSession(){
	return 0;
}

int spaMeasSpectralMask11b(const int ud,
                                      const double center,
                                      const double span,
                                      const int reset,
                                      const int verbose,
                                      const int output) {
  return 0;
}

int spaMeasSpectralMask(const int ud,
                                   const double center,
                                   const double span,
                                   const int reset,
                                   const int verbose,
                                   const int output) {
  return 0;
				     
				  }
#include <stdio.h>
#include <termios.h>
//#include <term.h>
//#include <curses.h>
#include <unistd.h>
static struct termios initial_settings, new_settings;
static int peek_character = -1;
void init_keyboard();
void close_keyboard();
int kbhit();
int readch();

void init_keyboard()
{
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}
void close_keyboard()
{
    tcsetattr(0, TCSANOW, &initial_settings);
}

int kbhit()
{
    char ch;
    int nread;
    int rtn;
    
    //if(peek_character != -1) return 1;
    new_settings.c_cc[VMIN]=0;
    rtn = tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    rtn =  tcsetattr(0, TCSANOW, &new_settings);
    if(nread == 1) {
      //peek_character = ch;
      ungetc(ch, stdin);
      return 1;
    }
    return 0;
}

int spaInit(const int adr, const int model) {
    return 0;
}

int spaClose(){
  return 0;
}

int attInit(const int adr, const int model) {
  return 0;
}

int attSet(const int ud, const int value) {
  return 0;
}

int attClose(){
  return 0;
}

//need fix, this is w32api
void _makepath( char *path, const char *drive, const char *dir, const char *fname, const char *ext )
{
}

void ForceSinglePCDACTableGriffin(A_UINT32 devNum, A_UINT16 pcdac, A_UINT16 offset)
{
}
void ForceSinglePCDACTable(A_UINT32 devNum, A_UINT16 pcdac)
{
}

char *strupr(char *c)
{
   char *p;
   if (!c) return NULL;
   for (p = c; *p; p++)
      if (isascii(*p) && islower(*p))
         *p = toupper(*p);
   return c;
}

/*
char *strlwr(char *c)
{
   char *p;
   if (!c) return NULL;
   for (p = c; *p; p++)
      if (isascii(*p) && isupper(*p))
         *p = tolower(*p);
   return c;
}
*/

#include "event.h"

A_INT16 hwCreateEvent
(
	A_UINT16 devIndex,
    A_UINT32 type, 
    A_UINT32 persistent, 
    A_UINT32 param1,
    A_UINT32 param2,
    A_UINT32 param3,
    EVT_HANDLE eventHandle
    )
{
  return 0;
}

double pmMeasAvgPower(const int ud, const int reset) {
  return 0;
}

int dmmInit(const int adr, const int model) {
  return 0;
}

int pmInit(const int ud, const int model) {
  return 0;
}

int gpibRsp(const int ud)
{
  return 0;
}
void gpibSendIFC(const int board)
{
}
long gpibClear(const int ud)
{
  return 0;
}
long gpibRSC(const int board, const int v)
{
  return 0;
}
long gpibRSP(const int ud, char *spr)
{
  return 0;
}
long gpibSIC(const int board)
{
  return 0;
}
long gpibONL(const int ud, const int v)
{
  return 0;
}
char *gpibRead(const int ud, ...)
{
  return 0;
}
long gpibWrite(const int ud, char *wrt)
{
  return 0;
}
char *gpibQuery(const int ud, char *wrt, ...)
{
  return 0;
}

int pmPreset(const int ud, const double trigger_level, const double trigger_delay, 
	     const double gate_start, const double gate_length, const double trace_display_length)
{
  return 0;
}

int  pmClose()
{
  return 0;
}

double saChanPwr(const int ud, const double center, const double span, const double ref_level, 
							const double rbw, const double vbw, const double chBW, const double chSP, const int saModel,
							const int reset)
{
  return 0;
}

double dmmMeasCurr(const int ud, const int model)
{
  return 0;
}

int spaMeasDSRCSpectralMask(const int ud, const double center, const double span, const int reset,
								   const int bandwidth, const int verbose, const int output)
{
  return 0;
}

double spaMeasOBW(const int ud, const double center, const double span, const double ref_level, const int reset)
{
  return 0;
}

double spaMeasFreqDev(const int ud, const double center, const double ref_level, const int reset)
{
  return 0;
}

void getMacAddr(A_UINT32 devNum, A_UINT16 wmac, A_UINT16 instNo, A_UINT8 *macAddr)
{
}

A_UINT16 getNextEvent
(
	MDK_WLAN_DEV_INFO *pdevInfo,
	EVENT_STRUCT *pEvent
)
{
  return 0;
}

//need fix, microsoft function
int fopen_s( 
   FILE** pFile,
   const char *filename,
   const char *mode 
)
{
  return 0;
}


void itoa(int value, char* str, int base) 
{
    sprintf(str, "%x", value);
}

void milliSleep(A_UINT32 millitime)
{
	usleep(1000*millitime);
}
//////////////////////////////////////////////////////
//socket wrap in linux
#include "dk_common.h"
#include "common_defs.h"
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <sys/sysinfo.h>
#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <pwd.h>
#include <netinet/tcp.h>
#include <netdb.h>

static int socketCreateAccept(int port_num)
{
    int	   sockfd;
//daniel    struct protoent *	proto;
    int	   res;
    struct sockaddr_in sin;
    int	   i;
    int	   sfd;
//daniel	WORD   wVersionRequested;
//daniel	WSADATA wsaData;
 
//daniel	wVersionRequested = MAKEWORD( 2, 2 );
 
//daniel	res = WSAStartup( wVersionRequested, &wsaData );
//daniel	if ( res != 0 ) {
//daniel		uiPrintf("socketCreateAccept: Could not find windows socket library\n");
//daniel		return -1;
//daniel	}
 
//daniel	if ( LOBYTE( wsaData.wVersion ) != 2 ||
//daniel        HIBYTE( wsaData.wVersion ) != 2 ) {
//daniel		uiPrintf("socketCreateAccept: Could not find windows socket library\n");
//daniel		WSACleanup( );
//daniel		return -1; 
//daniel	}

//daniel    if((proto = getprotobyname("tcp")) == NULL) {
//daniel    	uiPrintf("socketCreateAccept: getprotobyname failed: %d\n", WSAGetLastError());
//daniel   		WSACleanup( );
//daniel	    return -1;
//daniel    }

    q_uiPrintf("socket start\n");
//daniel    sockfd = WSASocket(PF_INET, SOCK_STREAM, proto->p_proto, NULL, (GROUP)NULL, 0);
    sockfd = socket(PF_INET, SOCK_STREAM, 0);   
    q_uiPrintf("socket end\n");
    if (sockfd == -1) {
	    uiPrintf("socketCreateAccept: socket failed: %d\n", sockfd);
//daniel        WSACleanup( );
	    return -1;
    }

    /* Allow immediate reuse of port */
    q_uiPrintf("setsockopt SO_REUSEADDR start\n");
    i = 1;
    res = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &i, sizeof(i));
    if (res != 0) {
	    uiPrintf("socketCreateAccept: setsockopt failed: %d\n", res);
//daniel        WSACleanup( );
	    return -1;
    }	
    q_uiPrintf("setsockopt SO_REUSEADDR end\n");

    /* Set TCP Nodelay */
    q_uiPrintf("setsockopt TCP_NODELAY start\n");
    i = 1;
    res = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &i, sizeof(i));
    if (res != 0) {
	    uiPrintf("socketCreateAccept: setsockopt failed: %d\n", res);
//daniel        WSACleanup( );
	    return -1;
    }	
    q_uiPrintf("setsockopt TCP_NODELAY end\n");

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr =  INADDR_ANY;
    sin.sin_port = htons((A_UINT16) port_num);

    q_uiPrintf("bind start\n");
    res = bind(sockfd, (struct sockaddr *) &sin, sizeof(sin));
    q_uiPrintf("bind end\n");
    if (res != 0) {
	    uiPrintf("socketCreateAccept: bind failed: %d\n", res);
//daniel         WSACleanup( );
	    return -1;
    }

    q_uiPrintf("listen start\n");
    res = listen(sockfd, 4);
    q_uiPrintf("listen end\n");
    if (res != 0) {
	    uiPrintf("socketCreateAccept: listen failed: %d\n", res);
//daniel         WSACleanup( );
	    return -1;
    }

    /* Call accept */
    q_uiPrintf("accept start\n");
    i = sizeof(sin);
    uiPrintf("* socket created, waiting for connection...\n");
    sfd = accept(sockfd, (struct sockaddr *) &sin, &i);
    q_uiPrintf("accept end\n");
    if (sfd == -1) {
	    uiPrintf("socketCreateAccept: accept failed: %d\n", sfd);
//daniel         WSACleanup( );
	    return -1;
    }

    q_uiPrintf("accept: sin.sin_family=0x%x\n", (int) sin.sin_family);
    q_uiPrintf("accept: sin.sin_port=0x%x (%d)\n", (int) sin.sin_port,
	    (int) ntohs(sin.sin_port));
    q_uiPrintf("accept: sin.sin_addr=0x%08x\n", (int) ntohl(sin.sin_addr.s_addr));

    res = close(sockfd);
    if (res != 0) {
	    uiPrintf("socketCreateAccept: closesocket failed: %d\n", res);
//daniel         WSACleanup( );
	    return -1;
    }

    return sfd;
}

ART_SOCK_INFO *osSockCreate(char *pname)
{
    ART_SOCK_INFO *pOSSock;

    pOSSock = (ART_SOCK_INFO *) A_MALLOC(sizeof(ART_SOCK_INFO));
    if(!pOSSock) {
		uiPrintf("osSockCreate: malloc failed for pOSSock\n");
        return NULL;
	}

    strcpy(pOSSock->hostname, "localhost");
    pOSSock->port_num = SOCK_PORT_NUM;

    pOSSock->sockfd = socketCreateAccept(pOSSock->port_num);
	if(pOSSock->sockfd < 0) {
		uiPrintf("osPipeCreate: socket creation failed\n");
        A_FREE(pOSSock);
		return NULL;
	}

    return pOSSock;
}

static int socketConnect(char *target_hostname, int target_port_num, A_UINT32 *ip_addr)
{
    int	   sfd;
//daniel    struct protoent *	proto;
    int	   res;
    struct sockaddr_in	sin;
    int	   i;
    int	   ffd;
    struct hostent *hostent;
//daniel	WORD   wVersionRequested;
//daniel	WSADATA wsaData;

//daniel    ffd = fileno(stdout);
//daniel	wVersionRequested = MAKEWORD( 2, 2 );
 
//daniel	res = WSAStartup( wVersionRequested, &wsaData );
//daniel	if ( res != 0 ) {
//daniel		uiPrintf("socketConnect: Could not find windows socket library\n");
//daniel		return -1;
//daniel	}
 
//daniel	if ( LOBYTE( wsaData.wVersion ) != 2 ||
//daniel        HIBYTE( wsaData.wVersion ) != 2 ) {
//daniel		uiPrintf("socketConnect: Could not find windows socket library\n");
//daniel		WSACleanup( );
//daniel		return -1; 
//daniel	}

//daniel    if((proto = getprotobyname("tcp")) == NULL) {
//daniel    	uiPrintf("ERROR::socketConnect: getprotobyname failed: %d\n", WSAGetLastError());
//daniel   		WSACleanup( );
//daniel	    return -1;
//daniel    }

    q_uiPrintf("socket start\n");
//daniel    sfd = WSASocket(PF_INET, SOCK_STREAM, proto->p_proto, NULL, (GROUP)NULL,0);
    sfd = socket(PF_INET, SOCK_STREAM, 0);   
    q_uiPrintf("socket end\n");
    if (sfd == -1) {
	    uiPrintf("ERROR::socketConnect: socket failed: %d\n", sfd);
//daniel        WSACleanup( );
	    return -1;
    }

    /* Allow immediate reuse of port */
    q_uiPrintf("setsockopt SO_REUSEADDR start\n");
    i = 1;
    res = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char *) &i, sizeof(i));
    if (res != 0) {
	    uiPrintf("ERROR::socketConnect: setsockopt SO_REUSEADDR failed: %d\n", res);
//daniel        WSACleanup( );
	    return -1;
    }	
    q_uiPrintf("setsockopt SO_REUSEADDR end\n");

    /* Set TCP Nodelay */
    q_uiPrintf("setsockopt TCP_NODELAY start\n");
    i = 1;
    res = setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (char *) &i, sizeof(i));
    if (res != 0) {
	    uiPrintf("ERROR::socketCreateAccept: setsockopt TCP_NODELAY failed: %d\n", res);
//daniel        WSACleanup( );
	    return -1;
    }	
    q_uiPrintf("setsockopt TCP_NODELAY end\n");

    q_uiPrintf("gethostbyname start\n");
    q_uiPrintf("socket_connect: target_hostname = '%s'\n", target_hostname);
    hostent = gethostbyname(target_hostname);
    q_uiPrintf("gethostbyname end\n");
    if (!hostent) {
	    uiPrintf("ERROR::socketConnect: gethostbyname failed: %d\n", hostent);
//daniel        WSACleanup( );
	    return -1;
    }	

    memcpy(ip_addr, hostent->h_addr_list[0], hostent->h_length);
    *ip_addr = ntohl(*ip_addr);

    sin.sin_family = AF_INET;
    memcpy(&sin.sin_addr.s_addr, hostent->h_addr_list[0], hostent->h_length);
    sin.sin_port = htons((short)target_port_num);

    for (i = 0; i < 20; i++) {
        q_uiPrintf("connect start %d\n", i);
	    res = connect(sfd, (struct sockaddr *) &sin, sizeof(sin));
        q_uiPrintf("connect end %d\n", i);
	    if (res == 0) {
	        break;
	    }
	    milliSleep(1);
    }
    if (i == 20) {
	    uiPrintf("ERROR::connect failed completely\n");
//daniel        WSACleanup( );
	    return -1;
    }

    return sfd;
}

OS_SOCK_INFO *osSockConnect(char *pname)
{
    char		pname_lcl[256];
    char *		mach_name;
    char *		cp;
    OS_SOCK_INFO *pOSSock;
    int			res;
    A_UINT32 err;
    HANDLE handle;

    strncpy(pname_lcl, pname, sizeof(pname_lcl));
    pname_lcl[sizeof(pname_lcl) - 1] = '\0';
#ifdef _DEBUG
    q_uiPrintf("osSockConnect: pname_lcl = '%s'\n", pname_lcl);
#endif
    mach_name = pname_lcl;
    while (*mach_name == '\\') {
	    mach_name++;
    }
    for (cp = mach_name; (*cp != '\0') && (*cp != '\\'); cp++) {
    }
    *cp = '\0';
    
#ifdef _DEBUG
    q_uiPrintf("osSockConnect: starting mach_name = '%s'\n", mach_name);
#endif

    if (!strcmp(mach_name, ".")) {
	    /* A windows convention meaning "local machine" */
	    mach_name = "localhost";
    }

    q_uiPrintf("osSockConnect: revised mach_name = '%s'\n", mach_name);

    pOSSock = (OS_SOCK_INFO *) A_MALLOC(sizeof(OS_SOCK_INFO));
    if(!pOSSock) {
		uiPrintf("ERROR::osSockConnect: malloc failed for pOSSock\n");
        return NULL;
	}

    strncpy(pOSSock->hostname, mach_name, sizeof(pOSSock->hostname));
    pOSSock->hostname[sizeof(pOSSock->hostname) - 1] = '\0';

    pOSSock->port_num = SOCK_PORT_NUM;

	if (!stricmp(pOSSock->hostname, "COM1")) {
       pOSSock->port_num = COM1_PORT_NUM;
	}
	if (!stricmp(pOSSock->hostname, "COM2")) {
       pOSSock->port_num = COM2_PORT_NUM;
	}
	if (!stricmp(pOSSock->hostname, "COM3")) {
       pOSSock->port_num = COM3_PORT_NUM;
	}
	if (!stricmp(pOSSock->hostname, "COM4")) {
       pOSSock->port_num = COM4_PORT_NUM;
	}
	if (!stricmp(pOSSock->hostname, "USB")) {
       pOSSock->port_num = USB_PORT_NUM;
	}

    switch(pOSSock->port_num) {
#if 0      
       case USB_PORT_NUM: {
             pOSSock->sockDisconnect = 0;
             pOSSock->sockClose = 0;
             handle = open_device(USB_FUNCTION, 0, inPipe);
             pOSSock->inHandle  = (A_INT32) handle;
             if (handle == INVALID_HANDLE_VALUE) {
                printf("Error:osSockConnect::Invalid Handle to inPipe:%s\n", inPipe);
                A_FREE(pOSSock);
	            return NULL;
             }
             handle = open_device(USB_FUNCTION, 0, outPipe);
             pOSSock->outHandle  = (A_INT32) handle;
             if (handle == INVALID_HANDLE_VALUE) {
                printf("Error:osSockConnect::Invalid Handle outPipe:%s\n", outPipe);
                A_FREE(pOSSock);
	            return NULL;
             }
             break;
             }

       case COM1_PORT_NUM:
       case COM2_PORT_NUM:
       case COM3_PORT_NUM: 
	   case COM4_PORT_NUM:   {
          pOSSock->sockDisconnect = 0;
          pOSSock->sockClose = 0;
          if ((err=os_com_open(pOSSock)) != 0) {
              uiPrintf("ERROR::osSockConnect::Com port open failed with error = %x\n", err);
              exit(0);
          }
       break;
       } // end of case
#endif       
       case SOCK_PORT_NUM: {
         res = socketConnect(pOSSock->hostname, pOSSock->port_num,
				      &pOSSock->ip_addr);;
         if (res < 0) {
   	      uiPrintf("ERROR::osSockConnect: pipe connect failed\n");
          A_FREE(pOSSock);
	      return NULL;
         }
	     q_uiPrintf("osSockConnect: Connected to pipe\n");

	     q_uiPrintf("ip_addr = %d.%d.%d.%d\n",
	        (pOSSock->ip_addr >> 24) & 0xff,
	        (pOSSock->ip_addr >> 16) & 0xff,
	        (pOSSock->ip_addr >> 8) & 0xff,
                  (pOSSock->ip_addr >> 0) & 0xff);
               pOSSock->sockfd = res;
               break;
	   } // end of else
    }

	return pOSSock;
}

volatile A_BOOL inSignalHandler = FALSE;

void osSockClose(OS_SOCK_INFO* pOSSock)
{

#ifdef _DEBUG
    q_uiPrintf("osSockClose::hostname=%s\n", pOSSock->hostname);
#endif

    switch(pOSSock->port_num) {          
            case SOCK_PORT_NUM: {
	            if (inSignalHandler == TRUE) return;
                close(pOSSock->sockfd);
	            A_FREE(pOSSock);
                break;
	        }  
            } 
}

/**************************************************************************
* osSockWrite - write len bytes into the socket, pSockInfo, from *buf
*
* This routine calls a OS specific routine for socket writing
* 
* RETURNS: length read
*/
#define SEND_BUF_SIZE		1024
A_INT32 osSockWrite(OS_SOCK_INFO *pSockInfo, A_UINT8 *buf, A_INT32 len)
{
		int	dwWritten, i = 0;
		A_INT32 bytes,cnt;
		A_UINT8* bufpos; 
		A_INT32 tmp_len;
        	A_UINT32 err;
		A_UINT8  *pad_buf;
		A_INT32 numblocks, pad_len, total_len;

//		if (inSignalHandler == TRUE) return 0;
	if (pSockInfo->port_num != USB_PORT_NUM && inSignalHandler == TRUE) return -1;

#ifdef _DEBUG 
    q_uiPrintf("osSockWrite: buf=0x%08lx  len=%d\n", (unsigned long) buf, len);
    //for (i=0;(i<len)&&(i<16);i++) {
    for (i=0; i<len; i++) {
      if (!(i%16)) q_uiPrintf("\n");
      q_uiPrintf(" %02X",*((unsigned char *)buf +i));
      if (3==(i%4)) q_uiPrintf(" ");
    }
    q_uiPrintf("\n");
#endif // _DEBUG

    switch(pSockInfo->port_num) {
#if 0
            case COM1_PORT_NUM:
            case COM2_PORT_NUM:
            case COM3_PORT_NUM: 
	    case COM4_PORT_NUM: {
                if ((err=write_device(pSockInfo, buf, &len)) != 0) {
                        uiPrintf("ERROR::osSockWrite::Com port write failed with error = %x\n", err);
                        return 0;
                }
	            dwWritten = len;
                break;
    } // end of case
#endif
            case SOCK_PORT_NUM: {
		        tmp_len = len;
	   	        bufpos = buf;
		        dwWritten = 0;

			
		        while (len) {
			        if (len < SEND_BUF_SIZE) bytes = len;
			        else bytes = SEND_BUF_SIZE;
	    	
			        cnt = send(pSockInfo->sockfd, (char *)bufpos, bytes, 0);
    
			        if (cnt == -1) break;
    
			        dwWritten += cnt;
		    	
			        len  -= cnt;
			        bufpos += cnt;
    	        }

		        len = tmp_len;
    
		        if (dwWritten != len) {
			        dwWritten = 0;
		        }

            break;
	        } // end of case
#if 0
            case USB_PORT_NUM: {
		numblocks = (len/MIN_TRANSFER_SIZE) + ((len%MIN_TRANSFER_SIZE)?1:0);
		total_len = numblocks * MIN_TRANSFER_SIZE;
		pad_buf = (A_UINT8 *)A_MALLOC(total_len * sizeof(A_UINT8));
		pad_len = (numblocks*MIN_TRANSFER_SIZE) - len;
		memcpy(pad_buf, buf, len);
#ifdef _DEBUG
		q_uiPrintf("osSockWrite::numblocks=%d:total_len=%d:pad_len=%d:actual len=%d\n", numblocks, total_len, pad_len, len);
#endif
				pSockInfo->sockfd = pSockInfo->outHandle;
                if ((err=write_device(pSockInfo, pad_buf, &total_len)) != 0) {
                        uiPrintf("ERROR::osSockWrite::USB port write failed with error = %x\n", err);
			A_FREE(pad_buf);
                        return 0;
                }
#ifdef _DEBUG
		q_uiPrintf("osSockWrite::total bytes written = %d:Padded len = %d:Actual len = %d\n", total_len, (numblocks * MIN_TRANSFER_SIZE), len);
#endif
		A_FREE(pad_buf);
		if (total_len == (numblocks * MIN_TRANSFER_SIZE))
	        	dwWritten = len;
		else 
			dwWritten = total_len;
            break;
            }
#endif
    }// end of switch

    return dwWritten;
}

/**************************************************************************
* osSockRead - read len bytes into *buf from the socket in pSockInfo
*
* This routine calls recv for socket reading
* 
* RETURNS: length read
*/

A_INT32 osSockRead(OS_SOCK_INFO *pSockInfo, A_UINT8 *buf, A_INT32 len)
{
    int dwRead, i = 0;
	A_INT32 cnt;
	A_UINT8* bufpos; 
	A_INT32 tmp_len;
    	A_UINT32 err;
	A_INT32 total_len;
	A_INT32		bytesRemaining; 
	static A_INT32  bytesRead=0; 
	static A_INT32    cmdlen_read=0, cmdlen_size=0;


#ifdef _DEBUG
    q_uiPrintf("osSockRead: buf=0x%08lx  len=%d\n", (unsigned long) buf, len);
#endif
	if (pSockInfo->port_num != USB_PORT_NUM && inSignalHandler == TRUE) return -1;

	tmp_len = len;
   	bufpos = buf;
	dwRead = 0;
	
    switch(pSockInfo->port_num) {
#if 0
            case COM1_PORT_NUM:
            case COM2_PORT_NUM:
            case COM3_PORT_NUM:
	    case COM4_PORT_NUM:{
                    if ((err=os_com_read(pSockInfo, buf, &tmp_len)) != 0) {
                        uiPrintf("ERROR::osSockRead::Com port read failed with error = %x\n", err);
                        return 0;
                    }
                    dwRead = tmp_len; // return number of bytes read
                break;
	            } // end of case
#endif
            case SOCK_PORT_NUM: {
                while (len) {
                    cnt = recv(pSockInfo->sockfd, (char *)bufpos, len, 0);

                    if ((cnt == -1) || (!cnt)) break;

                    dwRead += cnt;
                    len  -= cnt;
                    bufpos += cnt;
                }
#ifdef _DEBUG
                q_uiPrintf(":osSockRead data:%d read\n", dwRead);
                for (i=0;(i<dwRead)&&(i<16);i++) {
                    q_uiPrintf(" %02X",*((unsigned char *)buf+i));
                    if (3==(i%4)) q_uiPrintf(" ");
                }
                q_uiPrintf("\n");
#endif

                len = tmp_len;

                if (dwRead != len) {
                    dwRead = 0;
                }
                break;
                }
#if 0
            case USB_PORT_NUM: {
	            if (!cmdlen_read) {
					total_len = MIN_TRANSFER_SIZE;
						pSockInfo->sockfd = pSockInfo->inHandle;
                    	if ((err=read_device(pSockInfo, recvBuffer, &total_len)) != 0) {
                      		uiPrintf("ERROR::osSockRead::USB  read failed with error = %x\n", err);
                      		return 0;
                    	}
#ifdef _DEBUG
		        q_uiPrintf("osSockRead::cmdlen_read=%d:recv_buf_sem set\n", cmdlen_read);
#endif
			memcpy(rBuffer, recvBuffer, MIN_TRANSFER_SIZE);
			memcpy(buf, rBuffer, len);
			rBufferIndex = MIN_TRANSFER_SIZE;
			cmdlen_size = len;
			bytesRead = MIN_TRANSFER_SIZE - len;
		    }
		    else {
	  		  bytesRemaining = len - bytesRead;
#ifdef _DEBUG
			  q_uiPrintf("bytesRemaining=%d\n", bytesRemaining);
#endif
	  		  while(bytesRemaining>0) {
				total_len = MIN_TRANSFER_SIZE;
							pSockInfo->sockfd = pSockInfo->inHandle;
                    		if ((err=read_device(pSockInfo, recvBuffer, &total_len)) != 0) {
                      		   uiPrintf("ERROR::osSockRead::USB  read failed with error = %x\n", err);
                      		   return 0;
                    		}
#ifdef _DEBUG
				q_uiPrintf("osSockRead::cmdlen_read=%d:recv_buf_sem set\n", cmdlen_read);
#endif
				memcpy(rBuffer+rBufferIndex, recvBuffer, MIN_TRANSFER_SIZE);
	    			bytesRead+= MIN_TRANSFER_SIZE;
				rBufferIndex+=MIN_TRANSFER_SIZE;
	    			bytesRemaining = len - bytesRead;
	  		 }
	  		 memcpy(buf, rBuffer+cmdlen_size, len);
		    }
		    cmdlen_read = !cmdlen_read;
		    dwRead = bytesRead;
#ifdef _DEBUG
                    for (i=0;i<len;i++) {
                        q_uiPrintf(" %02X",*((unsigned char *)buf+i));
                        if (3==(i%4)) q_uiPrintf(" ");
                    }
                    q_uiPrintf("\n");
#endif
              		break;
            		}// end of case
#endif      
    }
#ifdef _DEBUG
    //q_uiPrintf(":osSockRead:%d read\n", dwRead);
#endif

    return dwRead;
}

void xq_uiPrintf(const char * format, ... ){};

/**************************************************************************
* enableLogging - enable logging of any lib messages to file 
*
*  
*
*/
void enableLogging
(
 A_CHAR *pFilename
)
{

}

/**************************************************************************
* disableLogging - turn off logging flag
*
*
*/
void disableLogging(void)
{

}