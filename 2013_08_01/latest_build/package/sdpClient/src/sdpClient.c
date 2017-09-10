/* file : bcast.c */
/* description: sends a broadcast message */

#include <stdio.h>
#include "sockLib.h"
//#include "net_com.h"
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
#include "database.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/ether.h>

static char *sdpServer_conffile="sdpServer";
static char *sdpServer_section="server";

static char *sdpServer_port="server_port";
static char *sdpServer_Status="Discovery_Service_Status";

//this is send broadcast and process received data on pc
int main(int argc,char **argv)
	{
	int i,j,repeat_time;
	struct sockaddr_in broadcast_addr;
	int optval,n=1;
	int optlen;
	int BUFLEN=256;
	SOCKET s_receive;
	struct hostent *hp;
	struct sockaddr_in my_addr;
	unsigned char remotename[128];
	unsigned char remoteaddr[128];
	unsigned char buffer[BUFLEN];
	unsigned char buffer_show[BUFLEN];
	fd_set my_readfd; 
	unsigned char buffer_send[256];
	unsigned char buff128[128];      
	int rec;
	struct sockaddr_in remote_addr;
	int addrlen;
	struct timeval tv_1s; 
	int select_return;
	unsigned short send_port;
	unsigned char port_buff[32];
	struct ifreq intf;
	int time_out=2;
	unsigned char ifname[20];
	int tempi;
	unsigned char command_id=0;
	FILE* logfile;
    struct in_addr ip_in_addr,p_ip;
    char ip_buffer[32]="0.0.0.0";
    char mac_buffer[32]="00:00:00:00:00:00";
    struct ether_addr *p_ether;
    int ststart1,ststart2,ststart3,ststart4,ststart5,ststart6,ststart7;
	ctx = uci_alloc_context();
	if (!ctx) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}


	if (argc!=4)
	{
		printf("Variables not right\nUse: sdpClient ifname timeout\n");
		return false;
	}
	else
	{
		if (strcmp(ifname_bridge,argv[1])&&strcmp(ifname_wireless,argv[1])&&strcmp(ifname_bridge_oldvip,argv[1]))
		{
			printf("Ifname:%s not right\n",argv[1]);
			return false;    
		}
		else
		{
			strcpy(ifname,argv[1]);
		}

		if (atoi(argv[2])<1)
		{
			printf("Timeout:%s not right\n",argv[2]);
			return false;    
		}
		else
		{
			time_out = atoi(argv[2]);
		}

		if (atoi(argv[3])<0)
		{
			printf("command id:%s not right\n",argv[3]);
			return false;    
		}
		else
		{
			command_id = atoi(argv[3]);
		}

	}
	memset(buffer_send,0,sizeof(buffer_send));
	repeat_time=0;
	s_receive=INVALID_SOCKET;
	CLEAR_ADDR(&broadcast_addr);
	hp=NULL;
	CLEAR_ADDR(&my_addr);

	addrlen=sizeof(struct sockaddr_in);

	sockInit();
	/* creates socket */
	s_receive = socket(AF_INET,SOCK_DGRAM,0);

	if (s_receive==INVALID_SOCKET)
	{
		printf("Can't create socket\n");
		sockEnd();
		return 2;
	}

    tv_1s.tv_sec=1;
    tv_1s.tv_usec=0;      
	if (setsockopt(s_receive, SOL_SOCKET, SO_RCVTIMEO, &tv_1s, (socklen_t) sizeof(tv_1s)))
		printf("setsockopt error\n");

	/* links socket to address and port */

	bzero(&intf, sizeof(intf));
	strncpy(intf.ifr_name, ifname, IFNAMSIZ);
	if (setsockopt(s_receive, SOL_SOCKET, SO_BINDTODEVICE,  &intf, sizeof(intf)) < 0)
		printf("setsockopt(SO_BINDTODEVICE) %d\n", errno);

	if (setsockopt(s_receive, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) == -1)
		return -1;

	optval=1;
	optlen=sizeof(int);
	if (setsockopt(s_receive,SOL_SOCKET,SO_BROADCAST,(char *) &optval,optlen))
	{
		printf("Cannot create a BROADCAST socket\n");
		sockEnd();
		return 2;
	}

	/* specifies host and port */
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(0);    
	/* accepts connections from any client */
	my_addr.sin_addr.s_addr=htonl ( INADDR_ANY );

	if (bind(s_receive,(struct sockaddr *) &my_addr, sizeof(struct sockaddr_in)))
	{
		printf("Can't bind socket to address\n");  
		sockEnd();   
		return 3;  
	}
	/* specify broadcast address and port */
	broadcast_addr.sin_family=AF_INET;

    if (command_id==0x00)
    {
        uci_section_read(sdpServer_conffile,sdpServer_section,sdpServer_port,port_buff);
        send_port = atoi(port_buff); 
        if (send_port<1)
        {
            send_port= SDP_SERVER_DEFAULT_LISTON_PORT;
        }
        broadcast_addr.sin_port=htons((unsigned short) send_port);
        broadcast_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);	/* send to all INADDR_ANY);// */
    
        i=0;
        buffer_send[i++]='p';
        buffer_send[i++]='c';
        buffer_send[i++]=command_id;
    
        buffer_send[i++]=0x06; 
        for (j=0;j<6;j++)
        {
            buffer_send[i++]=0xff;        
        }
    }

    /*added push button respond*/
    if (command_id==0x83)
    {
        send_port = 20007; 
        broadcast_addr.sin_port=htons((unsigned short) send_port);
        broadcast_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);	/* send to all INADDR_ANY);// */

        i=0;
//        buffer_send[i++]='p';
//        buffer_send[i++]='c';
        buffer_send[i++]=command_id;
        buffer_send[i++]=0xa;

        if (false==fetch_Local_IP_MAC(ifname_bridge,mac_buffer))
        {
            fetch_Local_IP_MAC(ifname_ethernet,mac_buffer);
        }
        p_ether = ether_aton(mac_buffer);
        buffer_send[i++] = (unsigned char)p_ether->ether_addr_octet[0];
        buffer_send[i++] = (unsigned char)p_ether->ether_addr_octet[1];
        buffer_send[i++] = (unsigned char)p_ether->ether_addr_octet[2];
        buffer_send[i++] = (unsigned char)p_ether->ether_addr_octet[3];
        buffer_send[i++] = (unsigned char)p_ether->ether_addr_octet[4];
        buffer_send[i++] = (unsigned char)p_ether->ether_addr_octet[5];      

        if (false==fetch_Local_IP_ADDR(ifname_bridge,ip_buffer))
        {
            fprintf(stderr, " \ncan not get bridge ip adderss \n");
        }

        if (0==inet_aton(ip_buffer,&p_ip))
        {
            fprintf(stderr, " \ncan not get bridge ip adderss \n");
        }   else   {
            buffer_send[i++] = (ntohl(p_ip.s_addr)>>24)&(0xff); 
            buffer_send[i++] =(ntohl(p_ip.s_addr)>>16)&(0xff); 
            buffer_send[i++] = (ntohl(p_ip.s_addr)>>8)&(0xff); 
            buffer_send[i++] = ntohl(p_ip.s_addr)&(0xff);                
        }
    }

     /* send data */
	if (sendto(s_receive,buffer_send,i,0,(struct sockaddr *) &broadcast_addr,sizeof(struct sockaddr_in))==-1)
	{
		printf("Can't send message\n");  
		sockEnd();   
		return 3;  
	}

    if (command_id==0x83)
    {
        sockEnd();   
        return 0;
    }

    logfile = fopen (WirelessNetworkConnectionStatusFile,"w+");
	if (logfile==NULL)
	{
		printf("Can't open file %s to write\n",WirelessNetworkConnectionStatusFile);
		return false;
	}

	while (1)
		{
		FD_ZERO(&my_readfd);
		FD_SET(s_receive,&my_readfd);
		tv_1s.tv_sec=1;
		tv_1s.tv_usec=0;      
		select_return= select(s_receive+1,&my_readfd, NULL,NULL,&tv_1s);
		if (select_return==-1)
		{
			printf("select error %s\n",strerror(errno));
			exit(1);
		}
		else if (select_return==0)
		{
			repeat_time++;
			if (repeat_time>=time_out)
			{
				break;
			}
		}
		else
		{
			if (FD_ISSET(s_receive,&my_readfd))
			{
				memset(buffer,0,sizeof(buffer));               
				rec=recvfrom(s_receive,buffer,BUFLEN,0,(struct sockaddr *) &remote_addr,&addrlen);
				if (rec==-1)
				{
					printf("Can't receive messages\n");  
					sockEnd();   
					return 4;  
				}

				buffer[rec]=0x00;
                ststart1=13;
                ststart2=ststart1+strlen(&buffer[ststart1])+1;
                ststart3=ststart2+strlen(&buffer[ststart2])+1;
                ststart4=ststart3+strlen(&buffer[ststart3])+1;
                ststart5=ststart4+strlen(&buffer[ststart4])+1;
                ststart6=ststart5+strlen(&buffer[ststart5])+1;
                ststart7=ststart6+strlen(&buffer[ststart6])+1;
                                  
				/* output received data  with client information */
				sprintf(buffer_show,"%02X:%02X:%02X:%02X:%02X:%02X\t%d.%d.%d.%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t",
						buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],
						buffer[9],buffer[10],buffer[11],buffer[12],&buffer[ststart1],&buffer[ststart2],&buffer[ststart3],&buffer[ststart4],&buffer[ststart5],&buffer[ststart6],&buffer[ststart7] );
				fprintf(logfile,"%s\n",buffer_show);
			}
		}
	}//end of while

	fclose(logfile);
	if (closesocket(s_receive))
	{
		perror("closesocket");
		return 5;
	}
	sockEnd();
	return 0;
}     

/* end of source code */








