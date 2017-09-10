/* file : udprecv.c */
/* description : receives and prints a message */ 

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
#include <netinet/ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <syslog.h>

//#include <netinet/if_ether.h>
//#include "UI_Menu_Process.h"
#include "database.h"
#define BUFLEN 128

#define CLEAR_ADDR(addr) memset(addr,0,sizeof(struct sockaddr_in))
ssize_t  my_sendto( const  void  *buf, size_t len, int flags, const struct sockaddr *to, socklen_t
					tolen);

static char *sdpServer_conffile="sdpServer";
static char *sdpServer_section="server";

static char *sdpServer_port="server_port";
static char *sdpServer_Status="Discovery_Service_Status";


int main(void)
{
	struct hostent *hp,*lp;
	struct sockaddr_in remote_addr;      
	int on=1;
	int optval;    
	SOCKET s;
	struct sockaddr_in any_addr;
	unsigned char buffer_get[32];
	char port_buff[32];
	int listen_port;
	char ip_buffer[32]="0.0.0.0";
	char mac_buffer[32]="00:00:00:00:00:00";
	char name_buff[32]=" ";
    char productname_buff[32]=" ";
    char mode_buff[32]=" ";
    char firmware_buff[32]=" ";
    char ssid_buff[128]=" ";
    char radiofw_buff[128]=" ";
    char apn_buff[128]=" ";
    char imagename_buff[128]=" ";
    char domainname_buff[128]=" ";
    char domainpwd_buff[128]=" ";
    char mlradio_buff[3]=" ";
    char mlethernet_buff[3]=" ";
    char mlcarrier_buff[3]=" ";
    char mlusb_buff[3]=" ";
    char mlcom_buff[3]=" ";
	char buffer_send[168];
	char buffer_temp[32];
	char buff[80];
	int rec;
	int addrlen;
	int length,j,i;
	struct ether_addr *p_ether;
	struct in_addr ip_in_addr,p_ip;
	int mine_packet=1;
	int discover_status=1;   
	FILE* f;
	int count=0, sendlen=0;
	struct ifreq intf;

	ctx = uci_alloc_context();
	if (!ctx) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}

	(void) setsid();

	while (1)
	{
		s=INVALID_SOCKET;
		hp=NULL;
		CLEAR_ADDR(&any_addr);
		addrlen=sizeof(struct sockaddr_in);

		/* specifies host and port */
		any_addr.sin_family=AF_INET;

		uci_section_read(sdpServer_conffile,sdpServer_section,sdpServer_port,port_buff);
		listen_port = atoi(port_buff); 
		if (listen_port<1)
			listen_port= SDP_SERVER_DEFAULT_LISTON_PORT;

		any_addr.sin_port=htons(listen_port);        
		/* accepts connections from any client */
		any_addr.sin_addr.s_addr=htonl ( INADDR_ANY );     
	
		/* creates socket */
		s = socket(AF_INET,SOCK_DGRAM,0);
	
		if (s==INVALID_SOCKET)
		{
			fprintf(stderr, "\nCan't create socket\n");
			return -1;
		}
		/* reuse address*/
		if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))< 0)
		{
			fprintf(stderr, "reuse address error %s\n",strerror(errno));
			return -1;
		}

//		fix_fd(s);
		sleep(2);

		/* links socket to address and port */
		if (bind(s,(struct sockaddr *) &any_addr, sizeof(struct sockaddr_in)))
		{
			fprintf(stderr, "\nCan't bind socket to address\n");
			return -1;
		}

		if (false==fetch_Local_IP_MAC(ifname_bridge,mac_buffer))
		{
			fetch_Local_IP_MAC(ifname_ethernet,mac_buffer);
		}

		p_ether = ether_aton(mac_buffer);
		buffer_send[2] = (unsigned char)p_ether->ether_addr_octet[0];
		buffer_send[3] = (unsigned char)p_ether->ether_addr_octet[1];
		buffer_send[4] = (unsigned char)p_ether->ether_addr_octet[2];
		buffer_send[5] = (unsigned char)p_ether->ether_addr_octet[3];
		buffer_send[6] = (unsigned char)p_ether->ether_addr_octet[4];
		buffer_send[7] = (unsigned char)p_ether->ether_addr_octet[5];      

		/*change enable or disable*/
		uci_section_read(sdpServer_conffile,sdpServer_section,sdpServer_Status,port_buff);

		if ('A'== port_buff[0])
		{
			discover_status = SDP_DISCOVER_STATUS_DISABLE;  
			exit(0);
		}
		else if ('C'== port_buff[0])
		{
			discover_status = SDP_DISCOVER_STATUS_CHANGABLE;  
		}
		else
		{
			discover_status = SDP_DISCOVER_STATUS_DISCOVABLE;  
		}            

		while (2)
		{
			hp=NULL;
			lp=NULL;
			mine_packet=1;
			CLEAR_ADDR(&remote_addr);   
			memset( buffer_get,0, sizeof(buffer_get));

			rec = recvfrom(s, buffer_get, BUFLEN, 0,(struct sockaddr *)&remote_addr, &addrlen); 

			if (rec==-1)
			{
				fprintf(stderr, "\nCan't receive messages\n");
				goto fail_process;  
			}

			buffer_get[rec]=0x00;    

			remote_addr.sin_family=AF_INET;
			//remote_addr.sin_port=htons(response_port);
			if (strncmp(buffer_get,"pc",2)!=0)
			{
				fprintf(stderr, " \nFirst 2 bytes are not pc\n");
				continue;
			}

            if (ctx)
            {
                uci_free_context(ctx);
                ctx=NULL;
            }
            ctx = uci_alloc_context();
            if (!ctx)
            {
                fprintf(stderr, "Out of memory\n");
                return NULL;
            }

            /*inquiry*/
            if (buffer_get[2]==SDP_CMD_INQUIRY_TOPOLOGY) {	

                /*length is not right*/
                if (0x06 != buffer_get[3]) {
                    continue;
                }

                if (0xff == buffer_get[4]) {
                    for (i=0;i<6;i++) {
                        if (buffer_get[4+i] != 0xff) {
                            mine_packet=0;
                        }
                    }
                }  else  {
                    for (i=0;i<6;i++) {
                        if (buffer_get[4+i] != buffer_send[2+i]) {
                            mine_packet=0;
                        }
                    }
                }

                if (!mine_packet)
                    continue;


                if (false==fetch_Local_IP_ADDR(ifname_bridge,ip_buffer))
                {
                    fprintf(stderr, " \ncan not get bridge ip adderss \n");
                }

                uci_section_read("system","@system[0]","hostname",name_buff);

                length = 12+strlen(name_buff);             
                buffer_send[0]= 0x01;
                buffer_send[1]= length;

                buffer_send[8] = discover_status;

                if (0==inet_aton(ip_buffer,&p_ip))
                {
                    continue;
                }   else   {
                    buffer_send[9] = (ntohl(p_ip.s_addr)>>24)&(0xff); 
                    buffer_send[10] =(ntohl(p_ip.s_addr)>>16)&(0xff); 
                    buffer_send[11] = (ntohl(p_ip.s_addr)>>8)&(0xff); 
                    buffer_send[12] = ntohl(p_ip.s_addr)&(0xff);                
                }

                j = strlen(name_buff);
                for (i=0;i<j;i++)
                {
                    buffer_send[13+i] = name_buff[i];
                }
                buffer_send[13+j] =0;  

                remote_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);                    
                /*send packet us broadcast packet*/
                //Need random delay here to avoid congestions

                //usleep(1+(int) (100000.0*rand()/(RAND_MAX+1.0)));

                if (my_sendto(buffer_send,13+j,0,(struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in))==-1)
                    fprintf(stderr," \nCan't send messages\n");

                SubProgram_Start("/bin/sh","/etc/br_maclist");

                FILE *file;
                char *buffer;
                unsigned long fileLen;

                //Open file
                file = fopen("/var/run/br_maclist", "rb");
                if (!file) {
                    fprintf(stderr, "\nUnable to open file %s", "/var/run/br_maclist");
                    return;
                }

                //Get file length
                fseek(file, 0, SEEK_END);
                fileLen=ftell(file);
                fseek(file, 0, SEEK_SET);

                //Allocate memory
                buffer=(char *)malloc(fileLen+1);
                if (!buffer) {
                    fprintf(stderr, "Memory error!");
                    fclose(file);
                    return;
                }

                //Read file contents into buffer
                fread(buffer, fileLen, 1, file);
                fclose(file);

                //Do send out file
                if (my_sendto(buffer,fileLen,0,(struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in))==-1)
                    fprintf(stderr," \nCan't send messages\n");
                free(buffer);
            }

            /*Mesh inquiry*/
            else if (buffer_get[2]==SDP_CMD_INQUIRY_MESH) {	

                /*length is not right*/
                if (0x06 != buffer_get[3]) {
                    continue;
                }

                if (0xff == buffer_get[4]) {
                    for (i=0;i<6;i++) {
                        if (buffer_get[4+i] != 0xff) {
                            mine_packet=0;
                        }
                    }
                }  else  {
                    for (i=0;i<6;i++) {
                        if (buffer_get[4+i] != buffer_send[2+i]) {
                            mine_packet=0;
                        }
                    }
                }

                if (!mine_packet)
                    continue;


                if (false==fetch_Local_IP_ADDR(ifname_bridge,ip_buffer))
                {
                    fprintf(stderr, " \ncan not get bridge ip adderss \n");
                }

                uci_section_read("system","@system[0]","hostname",name_buff);

                length = 12+strlen(name_buff);             
                buffer_send[0]= 0x01;
                buffer_send[1]= length;

                buffer_send[8] = discover_status;

                if (0==inet_aton(ip_buffer,&p_ip))
                {
                    continue;
                }   else   {
                    buffer_send[9] = (ntohl(p_ip.s_addr)>>24)&(0xff); 
                    buffer_send[10] =(ntohl(p_ip.s_addr)>>16)&(0xff); 
                    buffer_send[11] = (ntohl(p_ip.s_addr)>>8)&(0xff); 
                    buffer_send[12] = ntohl(p_ip.s_addr)&(0xff);                
                }

                j = strlen(name_buff);
                for (i=0;i<j;i++)
                {
                    buffer_send[13+i] = name_buff[i];
                }
                buffer_send[13+j] =0;  

                remote_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);                    
                /*send packet us broadcast packet*/
                //Need random delay here to avoid congestions

                //usleep(1+(int) (100000.0*rand()/(RAND_MAX+1.0)));

                if (my_sendto(buffer_send,13+j,0,(struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in))==-1)
                    fprintf(stderr," \nCan't send messages\n");

                SubProgram_Start("/bin/sh","/etc/br_maclist");

                FILE *file;
                char *buffer;
                unsigned long fileLen;

                //Open file
                file = fopen("/var/run/br_maclist", "rb");
                if (!file) {
                    fprintf(stderr, "\nUnable to open file %s", "/var/run/br_maclist");
                    return;
                }

                //Get file length
                fseek(file, 0, SEEK_END);
                fileLen=ftell(file);
                fseek(file, 0, SEEK_SET);

                //Allocate memory
                buffer=(char *)malloc(fileLen+1);
                if (!buffer) {
                    fprintf(stderr, "Memory error!");
                    fclose(file);
                    return;
                }

                //Read file contents into buffer
                fread(buffer, fileLen, 1, file);
                fclose(file);

                //Do send out file
                if (my_sendto(buffer,fileLen,0,(struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in))==-1)
                    fprintf(stderr," \nCan't send messages\n");
                free(buffer);

                SubProgram_Start("/bin/sh","/etc/br_meshlist");

                //Open file
                file = fopen("/var/run/br_meshlist", "rb");
                if (!file) {
                    fprintf(stderr, "\nUnable to open file %s", "/var/run/br_meshlist");
                    return;
                }

                //Get file length
                fseek(file, 0, SEEK_END);
                fileLen=ftell(file);
                fseek(file, 0, SEEK_SET);

                //Allocate memory
                buffer=(char *)malloc(fileLen+1);
                if (!buffer) {
                    fprintf(stderr, "Memory error!");
                    fclose(file);
                    return;
                }

                //Read file contents into buffer
                fread(buffer, fileLen, 1, file);
                fclose(file);

                //Do send out file
                if (my_sendto(buffer,fileLen,0,(struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in))==-1)
                    fprintf(stderr," \nCan't send messages\n");
                free(buffer);

            }

            else if (buffer_get[2]==SDP_CMD_INQUIRY)	/*inquiry*/
			{
				if (0x06 != buffer_get[3])	 /*length is not right*/
				{
			    	continue;
				}

				if (0xff == buffer_get[4])
				{
					for (i=0;i<6;i++)
					{
						if (buffer_get[4+i] != 0xff)
						{
							mine_packet=0;
						}
					}
				}
				else
				{
					for (i=0;i<6;i++)
					{
						if (buffer_get[4+i] != buffer_send[2+i])
						{
							mine_packet=0;
						}
					}
				}

				if (!mine_packet)
					continue;


				if (false==fetch_Local_IP_ADDR(ifname_bridge,ip_buffer))
				{
					fprintf(stderr, " \ncan not get bridge ip adderss \n");
				}

                memset( name_buff,0, sizeof(name_buff));
                memset( productname_buff,0, sizeof(productname_buff));
                memset( firmware_buff,0, sizeof(firmware_buff));
                memset( mode_buff,0, sizeof(mode_buff));
                memset( ssid_buff,0, sizeof(ssid_buff));

				uci_section_read("system","@system[0]","hostname",name_buff);
                uci_section_read("eurd","nms_conf","domainname",domainname_buff);
                uci_section_read("eurd","nms_conf","domainpasswd",domainpwd_buff);
                SubProgram_Start("/bin/sh","/etc/exdiscovery");
                uci_section_read("sdpServer","discovery","productname",productname_buff);
                uci_section_read("sdpServer","discovery","firmware",firmware_buff);
                uci_section_read("sdpServer","discovery","mode",mode_buff);
                uci_section_read("sdpServer","discovery","ssid",ssid_buff);
                uci_section_read("sdpServer","discovery","radiofw",radiofw_buff);
                /*to do 4g apn*/
                uci_section_read("lte","@carrier[0]","apn",apn_buff);
                /*end of 4g apn*/
                uci_section_read("sdpServer","discovery","imagename",imagename_buff);
                uci_section_read("sdpServer","discovery","mlradio",mlradio_buff);
                uci_section_read("sdpServer","discovery","mlethernet",mlethernet_buff);
                uci_section_read("sdpServer","discovery","mlcarrier",mlcarrier_buff);
                uci_section_read("sdpServer","discovery","mlusb",mlusb_buff);
                uci_section_read("sdpServer","discovery","mlcom",mlcom_buff);

				length = 12+strlen(name_buff);             
				buffer_send[0]= 0x01;
//				buffer_send[1]= length;

				buffer_send[8] = discover_status | 0x10;

				if (0==inet_aton(ip_buffer,&p_ip))
				{
					continue;
				}
				else
				{
					buffer_send[9] = (ntohl(p_ip.s_addr)>>24)&(0xff); 
					buffer_send[10] =(ntohl(p_ip.s_addr)>>16)&(0xff); 
					buffer_send[11] = (ntohl(p_ip.s_addr)>>8)&(0xff); 
					buffer_send[12] = ntohl(p_ip.s_addr)&(0xff);                
				}

				j = strlen(name_buff);
				for (i=0;i<j;i++)
				{
					buffer_send[13+i] = name_buff[i];
				}
				buffer_send[13+j] ='\0';  
                sendlen=13+j+1;
                
                buffer_send[sendlen]='\0';
                sendlen++;

                j = strlen(productname_buff);
                for (i=0;i<j;i++)
                {
                    buffer_send[sendlen+i] = productname_buff[i];
                }
                buffer_send[sendlen+j]='\0';
                sendlen=sendlen+j+1;

                j = strlen(firmware_buff);
                for (i=0;i<j;i++)
                {
                    buffer_send[sendlen+i] = firmware_buff[i];
                }
                buffer_send[sendlen+j]='\0';
                sendlen=sendlen+j+1;

                j = strlen(mode_buff);
                for (i=0;i<j;i++)
                {
                    buffer_send[sendlen+i] = mode_buff[i];
                }
                buffer_send[sendlen+j]='\0';
                sendlen=sendlen+j+1;

                j = strlen(ssid_buff);
                for (i=0;i<j;i++)
                {
                    buffer_send[sendlen+i] = ssid_buff[i];
                }
                buffer_send[sendlen+j]='\0';
                sendlen=sendlen+j+1;

                j = strlen(radiofw_buff);
                for (i=0;i<j;i++)
                {
                    buffer_send[sendlen+i] = radiofw_buff[i];
                }
                buffer_send[sendlen+j]='\0';
                sendlen=sendlen+j+1;

                j = strlen(apn_buff);
                for (i=0;i<j;i++)
                {
                    buffer_send[sendlen+i] = apn_buff[i];
                }
                buffer_send[sendlen+j]='\0';
                sendlen=sendlen+j+1;

                j = strlen(imagename_buff);
                for (i=0;i<j;i++)
                {
                    buffer_send[sendlen+i] = imagename_buff[i];
                }
                buffer_send[sendlen+j]='\0';
                sendlen=sendlen+j+1;

                j = strlen(domainname_buff);
                for (i=0;i<j;i++)
                {
                    buffer_send[sendlen+i] = domainname_buff[i];
                }
                buffer_send[sendlen+j]='\0';
                sendlen=sendlen+j+1;

                j = strlen(domainpwd_buff);
                for (i=0;i<j;i++)
                {
                    buffer_send[sendlen+i] = domainpwd_buff[i];
                }
                buffer_send[sendlen+j]='\0';
                sendlen=sendlen+j+1;

                buffer_send[sendlen] = atoi(mlradio_buff);
                sendlen=sendlen+1;

                buffer_send[sendlen] = atoi(mlethernet_buff);
                sendlen=sendlen+1;

                buffer_send[sendlen] = atoi(mlcarrier_buff);
                sendlen=sendlen+1;

                buffer_send[sendlen] = atoi(mlusb_buff);
                sendlen=sendlen+1;

                buffer_send[sendlen] = atoi(mlcom_buff);
                sendlen=sendlen+1;

                //j = strlen(modulelist_buff);
                //for (i=0;i<j;i++)
                //{
                //    buffer_send[sendlen+i] = atoi(modulelist_buff[i]);
                //}
                //buffer_send[sendlen+j]='\0';
                //sendlen=sendlen+j+1;
                //sendlen=sendlen+j;

                buffer_send[1]= sendlen-1;

				remote_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);                    
				/*send packet us broadcast packet*/
				//Need random delay here to avoid congestions

				//usleep(1+(int) (100000.0*rand()/(RAND_MAX+1.0)));

				if (my_sendto(buffer_send,sendlen,0,(struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in))==-1)
					fprintf(stderr," \nCan't send messages\n");
			}

            /*when get 0x80 request , make leds flash for 5 second*/
            else if (buffer_get[2]==0x80)	/*inquiry*/
            {
                if (0x06 != buffer_get[3])	 /*length is not right*/
                {
                    continue;
                }

                if (0xff == buffer_get[4])
                {
                    for (i=0;i<6;i++)
                    {
                        if (buffer_get[4+i] != 0xff)
                        {
                            mine_packet=0;
                        }
                    }
                } else if ( buffer_send[2] == buffer_get[4] ) {
                    for (i=0;i<6;i++)
                    {
                        if (buffer_get[4+i] != buffer_send[2+i] )
                        {
                            mine_packet=0;
                        }
                    }

                } else
                    mine_packet=0;

                if (!mine_packet)
                    continue;
                SubProgram_Start("/bin/sh","/etc/ledflash");

            }

			else if ((buffer_get[2]==SDP_CMD_CHANGE_IP)&& (discover_status == SDP_DISCOVER_STATUS_CHANGABLE))		/*change ip*/
			{
				for (i=0;i<6;i++)
				{
					if (buffer_get[4+i] != buffer_send[2+i])
					{
						mine_packet=0;
					}
				}
				if (!mine_packet)
					continue;

				if (buffer_get[3]==0x12)/*static*/
				{

					uci_section_write(network_conffile, network_lan_section, "proto", "static");

					ip_in_addr.s_addr = (((unsigned long)buffer_get[10])<<24) + (((unsigned long)buffer_get[11])<<16) +(((unsigned long)buffer_get[12])<<8) + (unsigned long)buffer_get[13] ;
					ip_in_addr.s_addr=htonl(ip_in_addr.s_addr);
					uci_section_write(network_conffile, network_lan_section, "ipaddr", inet_ntoa(ip_in_addr));


					ip_in_addr.s_addr = (((unsigned long)buffer_get[14])<<24) + (((unsigned long)buffer_get[15])<<16) + (((unsigned long)buffer_get[16])<<8) + (unsigned long)buffer_get[17] ;              
					ip_in_addr.s_addr=htonl(ip_in_addr.s_addr);
					uci_section_write(network_conffile, network_lan_section, "netmask", inet_ntoa(ip_in_addr));

					ip_in_addr.s_addr = (((unsigned long)buffer_get[18])<<24) + (((unsigned long)buffer_get[19])<<16) + (((unsigned long)buffer_get[20])<<8) + (unsigned long)buffer_get[21] ;
					ip_in_addr.s_addr=htonl(ip_in_addr.s_addr);
					uci_section_write(network_conffile, network_lan_section, "gateway", inet_ntoa(ip_in_addr));

					buffer_send[0]= 0x03;
					buffer_send[1]= 8;
					buffer_send[8]= 'O';
					buffer_send[9]= 'K';
					buffer_send[10]= 0;
					remote_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);
					/*send packet us broadcast packet*/
					if (my_sendto(buffer_send,10,0,(struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in))==-1)
						fprintf(stderr," \nCan't send messages\n");
					SubProgram_Start("/etc/init.d/network", "restart");
				}
				else if (buffer_get[3]==0x06)	/*dynamic*/
				{
					uci_section_write(network_conffile, network_lan_section, "proto", "dhcp");

					buffer_send[0]= 0x03;
					buffer_send[1]= 8;
					buffer_send[8]= 'O';
					buffer_send[9]= 'K';
					buffer_send[10]= 0;             
					remote_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);                    
					/*send packet us broadcast packet*/
					if (my_sendto(buffer_send,10,0,(struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in))==-1)
						fprintf(stderr," \nCan't send messages\n");
					SubProgram_Start("/etc/init.d/network", "restart");
				}
				else /*wrong*/
				{
					fprintf(stderr," \nWrong protcol  !\n");
				}
			}

			else if ((buffer_get[2]==SDP_CMD_REBOOT)&& (discover_status== SDP_DISCOVER_STATUS_CHANGABLE))	/*reboot*/
			{
				for (i=0;i<6;i++)
				{
					if (buffer_get[4+i] != buffer_send[2+i])
					{
						mine_packet=0;
					}
				} 

				if (!mine_packet)
					continue;

				buffer_send[0]= 0x05;
				buffer_send[1]= 0x08;
				buffer_send[8]= 'O';
				buffer_send[9]= 'K';
				buffer_send[10]= 0;             

				if (my_sendto(buffer_send,10,0,(struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in))==-1)
					fprintf(stderr," \nCan't send messages\n");
				SubProgram_Start("reboot", "-d 1");
			}

			else	 /*other packets*/
				fprintf(stderr," \nreceived error packet:%02x%02x%02x\n",buffer_get[2],buffer_get[3],buffer_get[10]);  
		}  /*while 2*/


		fail_process: 
		if (closesocket(s))
		{
			fprintf(stderr, "\nError on closing socket s\n");
		}

        if (ctx)
        {
            uci_free_context(ctx);
            ctx=NULL;
        }
	}	/*while 1*/
	return 0;
}

ssize_t  my_sendto( const  void  *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
{
	SOCKET s_send;
	int optval;
	struct ifreq intf;
	struct ifreq intf_wan;
	ssize_t  sent;
	char mode_buff[32];


	/* create socket */
	s_send = socket(AF_INET,SOCK_DGRAM,0);

	if (s_send==INVALID_SOCKET)
	{
		fprintf(stderr," \nCan't create socket s_send\n"); 
		exit(1);           
	}

	optval=1;

    if (-1==setsockopt(s_send, SOL_SOCKET, SO_BROADCAST, ( char*)&optval, sizeof(optval)))
	{
		fprintf(stderr," \nCan't setsockopt\n"); 
		close(s_send);
        return -1;
        //exit(1);           
	}

	bzero(&intf, sizeof(intf));
	strncpy(intf.ifr_name, ifname_bridge, IFNAMSIZ);

	if (setsockopt(s_send, SOL_SOCKET, SO_BINDTODEVICE,  &intf, sizeof(intf)) < 0)
	{
		fprintf(stderr, "setsockopt(SO_BINDTODEVICE) %d\n", errno);
		close(s_send);
        return -1;
        //exit(1); 
	}

	sent = sendto(s_send,  buf, len,flags,to,tolen);

	uci_section_read("system","@system[0]","systemmode",mode_buff);

	if (strcmp(mode_buff,"ap_router")==0 ||strcmp(mode_buff,"client_router")==0)
	{
		bzero(&intf, sizeof(intf));
		strncpy(intf.ifr_name, ifname_wireless, IFNAMSIZ);
		if (setsockopt(s_send, SOL_SOCKET, SO_BINDTODEVICE,  &intf, sizeof(intf)) < 0)
			{
			fprintf(stderr, "setsockopt(SO_BINDTODEVICE) %d\n", errno);
			close(s_send);
            return -1;
            //exit(1); 
			}
		sent = sendto(s_send,  buf, len,flags,to,tolen);
	}

	close(s_send);
	return sent;
}

/* end of source code */






