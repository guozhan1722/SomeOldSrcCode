/* file sockLib.h                           */   
/* win32 - UNIX include for sockets support */   

 #ifndef _SOCKETS_H_   
 #define _SOCKETS_H_   

 #ifndef UNIX   
 #include <winsock2.h>   
 #include <process.h>   
 #include <time.h>
 typedef u_long IOCTL_TYPE;   
 typedef char SOCKOPT_TYPE;
 typedef unsigned int pid_t;
 #define getProcessId GetCurrentThreadId
 #endif   

 #ifdef UNIX  

 #include <sys/types.h>  
 #include <sys/socket.h>  
 #include <netinet/in.h>  
 #include <sys/ioctl.h>  
 #include <sys/wait.h>  
 #include <netdb.h>  
 #include <unistd.h>  
 #include <signal.h>
 #include <time.h> 
   
 typedef int SOCKET;  
 typedef char IOCTL_TYPE;
 typedef void SOCKOPT_TYPE;

 #define INVALID_SOCKET -1  
 #define closesocket(s) close(s)  
 #define ioctlsocket(s,cmd,argp) ioctl(s,cmd,argp)  
 #define Sleep(t) usleep(t*1000)   
 #define getProcessId getpid	

 #endif  

 #ifdef __cplusplus
 extern "C" {
 #endif	
 /* functions to handle initialization */   
 extern int sockInit(void);  
 extern int sockEnd(void);  

 /* utility functions */
 #define CLEAR_ADDR(addr) memset(addr,0,sizeof(struct sockaddr_in))
 extern int inetAddrConvert(char *ascii_addr,char *addr); /* use inet_addr instead */
 extern int inetAddr(char *arg);
 extern int sockName(struct sockaddr_in *name,char *hostname,char *hostaddr);
 
 
 /* creation */
 extern SOCKET bcastSocket(void);
 extern SOCKET syncSocket(int type);
 extern SOCKET asyncSocket(int type);
 
 /* data */
 extern int asyncSend(SOCKET s,char *buffer,int bufsize,int timeout);
 extern int asyncRecv(SOCKET s,char *buffer,int bufsize,int timeout);
 extern int asyncRecvFrom(SOCKET s,char *msg,int msg_size,
				  struct sockaddr_in *addr,int timeout);	
 extern int asyncSendTo(SOCKET s,char *msg,int msg_size,
				  struct sockaddr_in *addr,int timeout);

 extern int recvAll(SOCKET s,char *msg,int msg_size); 
 extern int sendAll(SOCKET s,char *msg,int msg_size); 

 #define syncSend(s,buffer,bufsize) asyncSend(s,buffer,bufsize,-1)
 #define syncRecv(s,buffer,bufsize) asyncSend(s,buffer,bufsize,-1)
 #define syncSendTo(s,buffer,bufsize,addr) asyncSendTo(s,buffer,bufsize,addr,-1)
 #define syncRecvFrom(s,buffer,bufsize,addr) asyncRecvFrom(s,buffer,bufsize,addr,-1)

 /* client */
 extern int asyncConnect(SOCKET s, struct sockaddr_in *addr,int timeout);
 #define syncConnect(s,addr) asyncConnect(s,addr,-1) 

 /* server */
 extern SOCKET asyncAccept(SOCKET s,struct sockaddr_in *addr,int timeout);
 #define syncAccept(s,addr) asyncAccept(s,addr,-1)
 

#ifdef __cplusplus
 }
 #endif	
 

 #endif   /* _SOCKETS_H_ */
