/* file sockLib.c                                 */   
/* win32 - UNIX utility functions  for sockets    */   

 #include "sockLib.h" 
 #include  <stdio.h>
 #include <string.h>
 #include <ctype.h>

 int sockInit(void) {  
 #ifdef UNIX  
   return 0;  
 #endif  
 #ifndef UNIX  
   WORD wVersionRequested;  
   WSADATA wsaData;  
   wVersionRequested = MAKEWORD( 2, 0 );  
   return WSAStartup( wVersionRequested, &wsaData ); /* zero means no error */  
 #endif   
 }  

 int sockEnd(void) {  
 #ifdef UNIX  
   return 0;  
 #endif  
 #ifndef UNIX  
   return WSACleanup(); /* zero means no error */  
 #endif  
 }  


 
/*
	WARNING: it's safer to use the functions:

	  unsigned long inet_addr ( const char *  );
	and

	  char *inet_ntoa(struct in_addr(const char *);

    instead of inetAddrConvert and inetAddr supplied here because they are
	part of the official socket library on both UNIX and win32.
	  
*/
 
 int inetAddr(char *arg) {
	
	int size;
	int dots;
	int i;

	i=0;
	dots=0;
	size=0;

	if(arg==NULL)
		return 0;
	if(strlen(arg)<7)
		return 0;
	if(strlen(arg) > 15)
		return 0;

	size=strlen(arg);
	for(i=0;i<size;i++) {
		if(arg[i]=='.') {
			dots++;
			continue;
		}

		if(!isdigit(arg[i]))
			return 0;
	}

	return 1;

}

 

int inetAddrConvert(char *ascii_addr,char *addr) {

	char *s;
	char tmp[16];
	
	s=NULL;

	if(addr == NULL)
		return -1;
	if(!inetAddr(ascii_addr))
		return 1;

	strcpy(tmp,ascii_addr);
	for(s=tmp;*s;s++)
		if(*s=='.')
			*s=' ';

	if(sscanf(tmp,"%d %d %d %d",
		&addr[0],&addr[1],&addr[2],&addr[3]) == EOF)
		return 2;

	return 0;
	

}

/***********************/

int sockName(struct sockaddr_in *name,char *hostname,char *hostaddr) { 
  struct hostent *hp; 
  int i = 0;
  hp = NULL; 
  

  hp=gethostbyaddr((char *) &(name->sin_addr),sizeof(name->sin_addr),AF_INET); 
  if(hp==NULL) 
    return 1; 
  
  strcpy(hostname,hp->h_name); 
  for(i=0;hp->h_addr_list[i];i++) {
    sprintf(hostaddr,"%d.%d.%d.%d",
	    (unsigned char) hp->h_addr_list[i][0],
	    (unsigned char) hp->h_addr_list[i][1], 
        (unsigned char) hp->h_addr_list[i][2],
	    (unsigned char) hp->h_addr_list[i][3]);
  }

  /* OR */
  
  /*sprintf(hostaddr,"%d.%d.%d.%d",name->sin_addr.S_un.S_un_b.s_b1,
				name->sin_addr.S_un.S_un_b.s_b2,
				name->sin_addr.S_un.S_un_b.s_b3,
				name->sin_addr.S_un.S_un_b.s_b4);*/

  return 0; 
}


/***********************/

SOCKET bcastSocket(void) {


	int optval;
	SOCKET s;

	optval=1;

	s=socket(AF_INET,SOCK_DGRAM,0);
	if(setsockopt(s,SOL_SOCKET,SO_BROADCAST,(SOCKOPT_TYPE *) &optval,sizeof(int)))
		return INVALID_SOCKET;

	return s;

}


SOCKET asyncSocket(int type) {


	int optval;
	SOCKET s;

	optval=1;

	s=socket(AF_INET,type,0);
	if(ioctlsocket(s,FIONBIO,(IOCTL_TYPE *) &optval))
		return INVALID_SOCKET;

	return s;

}

SOCKET syncSocket(int type) {


	int optval;
	SOCKET s;

	optval=0;

	s=socket(AF_INET,type,0);
	if(ioctlsocket(s,FIONBIO,(IOCTL_TYPE *) &optval))
		return INVALID_SOCKET;

	return s;

}

/**********************/

int asyncRecv(SOCKET s,char *msg,int msg_size,int timeout) {
	
	clock_t endtime;
	int rec;

	endtime=(clock_t) (((double) timeout)/1000.0)*CLOCKS_PER_SEC + clock();
		
	do
		rec=recv(s,msg,msg_size,0);

	while(rec==-1 && endtime>clock());

	return rec;

}

int asyncSend(SOCKET s,char *msg,int msg_size,int timeout) {
	
	clock_t endtime;
	int snd;

	endtime=(clock_t) (((double) timeout)/1000.0)*CLOCKS_PER_SEC + clock();
		
	do
		snd=send(s,msg,msg_size,0);

	while(snd==-1 && endtime>clock());

	return snd;

}

int asyncRecvFrom(SOCKET s,char *msg,int msg_size,
				  struct sockaddr_in *addr,int timeout) {
	
	clock_t endtime;
	int rec;
	int addr_size;
	addr_size=sizeof(struct sockaddr_in);

	endtime=(clock_t) (((double) timeout)/1000.0)*CLOCKS_PER_SEC + clock();
		
	do
		rec=recvfrom(s,msg,msg_size,0,(struct sockaddr *) addr,&addr_size);

	while(rec==-1 && endtime>clock());

	return rec;

}

int asyncSendTo(SOCKET s,char *msg,int msg_size,
				  struct sockaddr_in *addr,int timeout) {
	
	clock_t endtime;
	int snd;

	endtime=(clock_t) (((double) timeout)/1000.0)*CLOCKS_PER_SEC + clock();
		
	do
		snd=sendto(s,msg,msg_size,0,(struct sockaddr *) addr,sizeof(struct sockaddr_in));

	while(snd==-1 && endtime>clock());

	return snd;

}


int recvAll(SOCKET s,char *msg,int msg_size) {
	
	int rec;
	int i;

	i=0;

			
	do {
		rec=recv(s,&msg[i],msg_size-i,0);
		i+=rec;
	} while(rec<msg_size && rec>0);

	
	return rec;

}

int sendAll(SOCKET s,char *msg,int msg_size) {
	
	
	int snd;
	int i;

	i=0;

			
	do {
		snd=send(s,&msg[i],msg_size-i,0);
		i+=snd;
	} while(snd<msg_size && snd>0);

	
	return snd;

}

/**********************/

SOCKET asyncAccept(SOCKET s,struct sockaddr_in *addr,int timeout) {
	
	clock_t endtime;
	SOCKET as;
	int addr_len;

	endtime=(clock_t) (((double) timeout)/1000.0)*CLOCKS_PER_SEC + clock();

	addr_len=sizeof(struct sockaddr_in);	
	as=INVALID_SOCKET;

	endtime=(clock_t) (((double) timeout)/1000.0)*CLOCKS_PER_SEC + clock();
		
	do
		as=accept(s,(struct sockaddr *) addr,&addr_len);

	while(as==INVALID_SOCKET && endtime>clock());

	return as;

}

/**********************/

int asyncConnect(SOCKET s, struct sockaddr_in *addr,int timeout) {

	clock_t endtime;
	int retval;
	
	
	endtime=(clock_t) (((double) timeout)/1000.0)*CLOCKS_PER_SEC + clock();


	do
		retval=connect(s,(struct sockaddr *) addr,sizeof(struct sockaddr_in));

	while(retval && endtime>clock());

	return retval;

}

/**********************/
/* end of source code */
