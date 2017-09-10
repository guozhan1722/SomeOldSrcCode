/*
 * libwebsockets-test-server - libwebsockets test implementation
 *
 * Copyright (C) 2010-2011 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>

#include "../lib/libwebsockets.h"


#include "uci.h"
#include <dirent.h>  
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <sys/sysinfo.h>
#include <assert.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>
#include "unistd.h"
#include <syslog.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <net/if.h>
#include <errno.h>
#include <sys/stat.h>
#include <termios.h>
#include <signal.h>
#include <stdbool.h>
#include <netinet/ip.h>
#include <netdb.h>
#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR) 

char c_WebSockSvr_Enable='A';
int i_WebSockSvr_timeout=60;
int i_WebSockSvr_port=7681;
int i_WebSockSvr_freshinter=10;
char c_WebSockSvr_GPS='A';
char c_WebSockSvr_GPSNMEA='A';
char c_WebSockSvr_COM='A';
char c_WebSockSvr_RADIO='A';
char s_WebSockSvr_password[50]="";

char c_WebSockSvr_GPSNMEA_RMC='A';
char c_WebSockSvr_GPSNMEA_GGA='A';
char c_WebSockSvr_GPSNMEA_GSA='A';
char c_WebSockSvr_GPSNMEA_GSV='A';
char c_WebSockSvr_GPSNMEA_VTG='A';
char c_WebSockSvr_COM_MODE='A';


#define DEFAULT_GPSD_SERVER "127.0.0.1"
static char gps_port[10];
static int sock = 0;
static ssize_t wrote = 0;
static char cbuf[5];
static int gpsd_retry_times=0;

char c_Com_TCPServer_status='0';
static char s_Com_TCPServer_port[10];
static int com_sock = 0;


bool get_option_by_section_name(struct uci_context *ctx,char*package_name,char*section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_element *e= NULL;
    struct uci_ptr ptr;
    struct uci_option *p_option = NULL;

    //syslog(LOGOPTS,"Enter [%s] %s %s\n", __func__, package_name, section_name, option_name);
    
    //printf("%s option_name: %s\n",__func__,option_name);
    strcpy(value, "");

    tuple = malloc( strlen(package_name) + strlen(section_name) + strlen(option_name)+ 3 ); /* "." and trailing \0 */

    if (!tuple) return false;

    strcpy(tuple, package_name);
    strcat(tuple, ".");
    strcat(tuple, section_name);
    strcat(tuple, ".");
    strcat(tuple, option_name);

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK)
        {

        e = ptr.last;
        if ( (ptr.flags & UCI_LOOKUP_COMPLETE) &&
             (e->type == UCI_TYPE_OPTION))
            {
            p_option = ptr.o;
            strcpy(value, p_option->v.string);
            //printf("%s find %s\n",__func__, value);
            free(tuple);
            return true;
            }
        }

    free(tuple);

    return false;

}

bool set_option_by_section_name(struct uci_context *ctx,char*package_name, char *section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_ptr ptr;

    //syslog(LOGOPTS,"Enter [%s] %s %s\n", __func__, package_name, section_name, option_name);

    tuple = malloc( strlen(package_name) + strlen(section_name) + strlen(option_name) + strlen(value) + 4 );    /* "." "=" and trailing \0 */

    if (!tuple) return false;

    strcpy(tuple, package_name);
    strcat(tuple, ".");
    strcat(tuple, section_name);
    strcat(tuple, ".");
    strcat(tuple, option_name);
    strcat(tuple, "=");
    strcat(tuple, value);

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK)
            {
        if ( UCI_OK == uci_set(ctx, &ptr) )
            {
            uci_save(ctx, ptr.p);
            uci_commit(ctx, &ptr.p, false);
            }

        free(tuple);
        return true;
        }

    free(tuple);  
    return false;
}

void close_service_config()
{
    struct uci_context *ctx;
    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        syslog(LOGOPTS,"Web Socket Server EXIT:Out of memory\n");
        exit(-1);
    }

    set_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_Enable","A");

    uci_free_context(ctx);

}

void read_config_comserver()
{
    struct uci_context *ctx;
    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        syslog(LOGOPTS,"Web Socket Server EXIT:Out of memory\n");
        exit(-1);
    }

    char tmpbuff[20];
    char com_stat,com_protocol;
    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "comport2","com2","COM2_Port_Status",tmpbuff);
    com_stat=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "comport2","com2","COM2_IP_Protocol",tmpbuff);
    com_protocol=tmpbuff[0];

    s_Com_TCPServer_port[0]=0;
    get_option_by_section_name(ctx, "comport2","com2","COM2_T_Server_Listen_Port",s_Com_TCPServer_port);

    uci_free_context(ctx);

    c_Com_TCPServer_status='0';
    if(com_stat=='B')
    {
        if(com_protocol=='B' || com_protocol=='C')c_Com_TCPServer_status='1';
    }
}

void read_config()
{
    struct uci_context *ctx;
    ctx = uci_alloc_context();  // for VIP4G
    if (!ctx) 
    {
        syslog(LOGOPTS,"Web Socket Server EXIT:Out of memory\n");
        exit(-1);
    }

    char tmpbuff[20];
    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_Enable",tmpbuff);
    c_WebSockSvr_Enable=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_GPS",tmpbuff);
    c_WebSockSvr_GPS=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_GPSNMEA",tmpbuff);
    c_WebSockSvr_GPSNMEA=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_COM",tmpbuff);
    c_WebSockSvr_COM=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_RADIO",tmpbuff);
    c_WebSockSvr_RADIO=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_GPSNMEA_RMC",tmpbuff);
    c_WebSockSvr_GPSNMEA_RMC=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_GPSNMEA_GGA",tmpbuff);
    c_WebSockSvr_GPSNMEA_GGA=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_GPSNMEA_GSA",tmpbuff);
    c_WebSockSvr_GPSNMEA_GSA=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_GPSNMEA_GSV",tmpbuff);
    c_WebSockSvr_GPSNMEA_GSV=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_GPSNMEA_VTG",tmpbuff);
    c_WebSockSvr_GPSNMEA_VTG=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_COM_MODE",tmpbuff);
    c_WebSockSvr_COM_MODE=tmpbuff[0];

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_timeout",tmpbuff);
    i_WebSockSvr_timeout=atoi(tmpbuff)*60;  //for seconds.

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_port",tmpbuff);
    i_WebSockSvr_port=atoi(tmpbuff);  
    if(i_WebSockSvr_port<100)i_WebSockSvr_port=7681;

    tmpbuff[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_freshinter",tmpbuff);
    i_WebSockSvr_freshinter=atoi(tmpbuff);  
    if(i_WebSockSvr_freshinter<2)i_WebSockSvr_freshinter=10;

    
    s_WebSockSvr_password[0]=0;
    get_option_by_section_name(ctx, "websockserver","wsr_conf","WebSockSvr_password",s_WebSockSvr_password);

    uci_free_context(ctx);
}

int fetch_Local_IP_ADDR(char *ifname, char *addr)
{
    struct ifreq ifr;
    int sock;
    char *p;
    char temp[32]="N/A";

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if (-1==sock)
    {
        syslog(LOGOPTS,"fetch_Local_IP_ADDR: socket=-1");  
        perror("fetch_Local_IP_ADDR ");
        strcpy(addr, temp); 
        return false;
    }
    strncpy(ifr.ifr_name,ifname,sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';   
    if (-1==ioctl(sock, SIOCGIFADDR, &ifr))
    {
        //perror("ioctl(SIOCGIFADDR) "); /*dont show it, sometimes no ip*/
        //syslog(LOGOPTS,"fetch_Local_IP_ADDR:%s ioctl(SIOCGIFADDR)",ifname);       
        strcpy(addr, temp); 
        close(sock);
        return false;
    }
    p=inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr);
    strncpy(temp,p,sizeof(temp)-1);
    temp[sizeof(temp)-1]='\0';
    strcpy(addr, temp);  
    close(sock);  
    return true;
}




static int close_testing=0;

/*
 * This demo server shows how to use libwebsockets for one or more
 * websocket protocols in the same server
 *
 * It defines the following websocket protocols:
 *
 *  dumb-increment-protocol:  once the socket is opened, an incrementing
 *				ascii string is sent down it every 50ms.
 *				If you send "reset\n" on the websocket, then
 *				the incrementing number is reset to 0.
 *
 *  lws-mirror-protocol: copies any received packet to every connection also
 *				using this protocol, including the sender
 */

enum demo_protocols {
	/* always first */
	PROTOCOL_HTTP = 0,

	PROTOCOL_DUMB_INCREMENT,
	PROTOCOL_GPS_PULLOUT,
    PROTOCOL_GPSNMEA_PULLOUT,
    PROTOCOL_RADIOINFO_PULLOUT,
    PROTOCOL_COMPORT_PULLOUT,
	PROTOCOL_LWS_MIRROR,

	/* always last */
	DEMO_PROTOCOL_COUNT
};


#define LOCAL_RESOURCE_PATH INSTALL_DATADIR"/libwebsockets-test-server"

/* this protocol server (always the first one) just knows how to do HTTP */

static int callback_http(struct libwebsocket_context *context,
		struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user,
							   void *in, size_t len)
{
	char client_name[128];
	char client_ip[128];

	switch (reason) {
	case LWS_CALLBACK_HTTP:
		fprintf(stderr, "serving HTTP URI %s\n", (char *)in);

		if (in && strcmp(in, "/favicon.ico") == 0) {
			if (libwebsockets_serve_http_file(wsi,
			     LOCAL_RESOURCE_PATH"/favicon.ico", "image/x-icon"))
				fprintf(stderr, "Failed to send favicon\n");
			break;
		}

		/* send the script... when it runs it'll start websockets */

		if (libwebsockets_serve_http_file(wsi,
				  LOCAL_RESOURCE_PATH"/test.html", "text/html"))
			fprintf(stderr, "Failed to send HTTP file\n");
		break;

	/*
	 * callback for confirming to continue with client IP appear in
	 * protocol 0 callback since no websocket protocol has been agreed
	 * yet.  You can just ignore this if you won't filter on client IP
	 * since the default uhandled callback return is 0 meaning let the
	 * connection continue.
	 */

	case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:

		libwebsockets_get_peer_addresses((int)(long)user, client_name,
			     sizeof(client_name), client_ip, sizeof(client_ip));

		fprintf(stderr, "Received network connect from %s (%s)\n",
							client_name, client_ip);

		/* if we returned non-zero from here, we kill the connection */
		break;

	default:
		break;
	}

	return 0;
}


/*
 * this is just an example of parsing handshake headers, you don't need this
 * in your code unless you will filter allowing connections by the header
 * content
 */

static void
dump_handshake_info(struct lws_tokens *lwst)
{
	int n;
	static const char *token_names[WSI_TOKEN_COUNT] = {
		/*[WSI_TOKEN_GET_URI]		=*/ "GET URI",
		/*[WSI_TOKEN_HOST]		=*/ "Host",
		/*[WSI_TOKEN_CONNECTION]	=*/ "Connection",
		/*[WSI_TOKEN_KEY1]		=*/ "key 1",
		/*[WSI_TOKEN_KEY2]		=*/ "key 2",
		/*[WSI_TOKEN_PROTOCOL]		=*/ "Protocol",
		/*[WSI_TOKEN_UPGRADE]		=*/ "Upgrade",
		/*[WSI_TOKEN_ORIGIN]		=*/ "Origin",
		/*[WSI_TOKEN_DRAFT]		=*/ "Draft",
		/*[WSI_TOKEN_CHALLENGE]		=*/ "Challenge",

		/* new for 04 */
		/*[WSI_TOKEN_KEY]		=*/ "Key",
		/*[WSI_TOKEN_VERSION]		=*/ "Version",
		/*[WSI_TOKEN_SWORIGIN]		=*/ "Sworigin",

		/* new for 05 */
		/*[WSI_TOKEN_EXTENSIONS]	=*/ "Extensions",

		/* client receives these */
		/*[WSI_TOKEN_ACCEPT]		=*/ "Accept",
		/*[WSI_TOKEN_NONCE]		=*/ "Nonce",
		/*[WSI_TOKEN_HTTP]		=*/ "Http",
		/*[WSI_TOKEN_MUXURL]	=*/ "MuxURL",
	};

	for (n = 0; n < WSI_TOKEN_COUNT; n++) {
		if (lwst[n].token == NULL)
			continue;

		fprintf(stderr, "    %s = %s\n", token_names[n], lwst[n].token);
	}
}

/* dumb_increment protocol */

/*
 * one of these is auto-created for each connection and a pointer to the
 * appropriate instance is passed to the callback in the user parameter
 *
 * for this example protocol we use it to individualize the count for each
 * connection.
 */

struct per_session_data__dumb_increment {
	int number;
};

static int
callback_dumb_increment(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	int n;
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 +
						  LWS_SEND_BUFFER_POST_PADDING];
	unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
	struct per_session_data__dumb_increment *pss = user;

	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		fprintf(stderr, "callback_dumb_increment: "
						 "LWS_CALLBACK_ESTABLISHED\n");
		pss->number = 0;
		break;

	/*
	 * in this protocol, we just use the broadcast action as the chance to
	 * send our own connection-specific data and ignore the broadcast info
	 * that is available in the 'in' parameter
	 */

	case LWS_CALLBACK_BROADCAST:
		n = sprintf((char *)p, "%d", pss->number++);
		n = libwebsocket_write(wsi, p,  n, LWS_WRITE_TEXT);
		if (n < 0) {
			fprintf(stderr, "ERROR writing to socket");
			return 1;
		}
		if (close_testing && pss->number == 50) {
			fprintf(stderr, "close tesing limit, closing\n");
			libwebsocket_close_and_free_session(context, wsi,
						       LWS_CLOSE_STATUS_NORMAL);
		}
		break;

	case LWS_CALLBACK_RECEIVE:
		fprintf(stderr, "rx %d\n", (int)len);
		if (len < 6)
			break;
		if (strcmp(in, "reset\n") == 0)
			pss->number = 0;
		break;
	/*
	 * this just demonstrates how to use the protocol filter. If you won't
	 * study and reject connections based on header content, you don't need
	 * to handle this callback
	 */

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		dump_handshake_info((struct lws_tokens *)(long)user);
		/* you could return non-zero here and kill the connection */
		break;

	default:
		break;
	}

	return 0;
}

static int fresh_gpsdata(char * s_out)
{
	char buffer_get[256];
	char latitude[30];
	char longitude[30];
	char *p;
	int j;
	FILE *f;

	*s_out=0;
    latitude[0]=0;
    longitude[0]=0;
    if (!(f = fopen("/var/run/GPS_position", "r"))) 
    {
		return 0;
    }else
    {
        while (fgets(buffer_get, 256, f)) 
        {
            p=NULL;
            p = strstr(buffer_get, "latitude=");//latitude="51.142962"
            if (p != NULL)
            {
                p+=strlen("latitude=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        latitude[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0) 
                    {
                        latitude[j]=0;
                    }
            }//if p

            p=NULL;
            p = strstr(buffer_get, "longitude=");//longitude="-114.075094"
            if (p != NULL)
            {
                p+=strlen("longitude=")+1;
                while (*p == ' ')p++; //first space
                j=0;
                while ((*p != '\"')&& (*p!='\r') && (*p!='\0') && (*p!='\n') &&(j<30)) 
                    {
                        longitude[j]=*p;
                        p++;
                        j++;
                     }
                if(j>0) 
                    {
                        longitude[j]=0;
                    }
            }//if p
        }//while
    }//else
    if(f)fclose(f);
	return sprintf(s_out,"%s,%s", latitude, longitude);

}

struct per_session_data__gps_pullout {
	//char gpsdata[512];
	int status;
	int try_times;
    int pass_times;
};

static int
callback_gps_pullout(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	int n;
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 +
						  LWS_SEND_BUFFER_POST_PADDING];
	unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
	struct per_session_data__gps_pullout *pss = user;

	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		fprintf(stderr, "callback_gps_pullout: "
						 "GPS LWS_CALLBACK_ESTABLISHED\n");
		pss->status=0;
        if(strlen(s_WebSockSvr_password)<1)pss->status=1;
        pss->try_times=0;
        pss->pass_times=0;
		break;

	/*
	 * in this protocol, we just use the broadcast action as the chance to
	 * send our own connection-specific data and ignore the broadcast info
	 * that is available in the 'in' parameter
	 */

    case LWS_CALLBACK_BROADCAST:
        if(pss->status==1)n=fresh_gpsdata((char *)p);
        else n=sprintf((char *)p,"Please send connect password.\n");
        if(n>0)
        {
            n = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
            if (n < 0) {
                fprintf(stderr, "ERROR writing to socket");
                pss->try_times++;
                if(pss->try_times>5)
                {
                    libwebsocket_close_and_free_session(context, wsi,
                                       LWS_CLOSE_STATUS_NORMAL);
                }
                return 1;
            }
            if(n>0)pss->try_times=0;
        }

		if (close_testing) {
			fprintf(stderr, "close tesing limit, closing\n");
			libwebsocket_close_and_free_session(context, wsi,
						       LWS_CLOSE_STATUS_NORMAL);
		}
		break;

	case LWS_CALLBACK_RECEIVE:
		fprintf(stderr, "rx %d\n", (int)len);
        if (len < 1)
            break;
        //if (strcmp(in, "reset\n") == 0)
        //	pss->status = 0;
        if(pss->status == 0)
        {
            if(strcmp(in,s_WebSockSvr_password)==0)pss->status = 1;
            else pss->pass_times++;
            if(pss->pass_times>10)libwebsocket_close_and_free_session(context, wsi,LWS_CLOSE_STATUS_NORMAL);
        }
		break;
	/*
	 * this just demonstrates how to use the protocol filter. If you won't
	 * study and reject connections based on header content, you don't need
	 * to handle this callback
	 */

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		dump_handshake_info((struct lws_tokens *)(long)user);
		/* you could return non-zero here and kill the connection */
		break;

	default:
		break;
	}

	return 0;
}

static int read_gpsnmea(char * s_out)
{
	char buffer_get[256];
    int n=0;
	FILE *f;

	*s_out=0;
    buffer_get[0]=0;
    if (!(f = fopen("/var/run/lws_gpsnmea", "r"))) 
    {
		return 0;
    }else
    {
        while (fgets(buffer_get, 256, f)) 
        {
            strcat(s_out,buffer_get);
            n=n+strlen(buffer_get);
            if(n>1900)break;
        }//while
    }//else
    if(f)fclose(f);
	return n;
}

struct per_session_data__gpsnmea_pullout {
	//char gpsdata[512];
	int status;
	int try_times;
    int pass_times;
};

static int
callback_gpsnmea_pullout(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	int n;
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 2048 +
						  LWS_SEND_BUFFER_POST_PADDING];
	unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
	struct per_session_data__gpsnmea_pullout *pss = user;

	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		fprintf(stderr, "callback_gpsnmea_pullout: "
						 "GPSNMEA LWS_CALLBACK_ESTABLISHED\n");
		pss->status=0;
        if(strlen(s_WebSockSvr_password)<1)pss->status=1;
        pss->try_times=0;
        pss->pass_times=0;
		break;

	/*
	 * in this protocol, we just use the broadcast action as the chance to
	 * send our own connection-specific data and ignore the broadcast info
	 * that is available in the 'in' parameter
	 */

	case LWS_CALLBACK_BROADCAST:
        if(pss->status==1)n=read_gpsnmea((char *)p);
        else n=sprintf((char *)p,"Please send connect password.\n");
        if(n>0)
        {
            n = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
            if (n < 0) {
                fprintf(stderr, "ERROR writing to socket");
                pss->try_times++;
                if(pss->try_times>5)
                {
                    libwebsocket_close_and_free_session(context, wsi,
                                       LWS_CLOSE_STATUS_NORMAL);
                }
                return 1;
            }
    
            if(n>0)pss->try_times=0;
        }

		if (close_testing) {
			fprintf(stderr, "close tesing limit, closing\n");
			libwebsocket_close_and_free_session(context, wsi,
						       LWS_CLOSE_STATUS_NORMAL);
		}
		break;

	case LWS_CALLBACK_RECEIVE:
		fprintf(stderr, "rx %d\n", (int)len);
		if (len < 1)
			break;
		//if (strcmp(in, "reset\n") == 0)
		//	pss->status = 0;
        if(pss->status == 0)
        {
            if(strcmp(in,s_WebSockSvr_password)==0)pss->status = 1;
            else pss->pass_times++;
            if(pss->pass_times>10)libwebsocket_close_and_free_session(context, wsi,LWS_CLOSE_STATUS_NORMAL);
        }
		break;
	/*
	 * this just demonstrates how to use the protocol filter. If you won't
	 * study and reject connections based on header content, you don't need
	 * to handle this callback
	 */

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		dump_handshake_info((struct lws_tokens *)(long)user);
		/* you could return non-zero here and kill the connection */
		break;

	default:
		break;
	}

	return 0;
}


//{"statusreport":{"Type":"carrier","ActiveStatus":"$FORM_connect_status","RSSI":"$FORM_rssi","IP":"$wan_ip",
//"Network":"$FORM_network","Servicetype":"$state","Roaming":"$FORM_roaming","Temperature":"$FORM_temp",
//"IMEI":"$FORM_imei","ICCID":"$FORM_simid","Phone":"$FORM_phonenum"}}
static int read_radioinfo_jason(char * s_out)
{
    char ifname_carrier[30]="br-wan2";

	char buffer_get[256];
    char s_ActiveStatus[30]="N/A";
    char s_RSSI[10]="N/A";
    char s_RSRP[10]="N/A";
    char s_RSRQ[10]="N/A";
    char s_IP[30]="N/A";
    char s_Network[30]="N/A";
    char s_Servicetype[30]="N/A";
    char s_Roaming[30]="N/A";
    char s_Temperature[30]="N/A";
    char s_IMEI[30]="N/A";
    char s_IMSI[30]="N/A";
    char s_ICCID[30]="N/A";
    char s_Phone[30]="N/A";

	char *p;
	FILE *f;

    if (!(f = fopen("/var/run/std_lte_statue", "r"))) 
    {
        //do nothing.
    }else
    {

        fetch_Local_IP_ADDR(ifname_carrier,s_IP);

        while (fgets(buffer_get, 256, f)) 
        {
            p=NULL;
            p = strstr(buffer_get, "rssi=");//rssi="-72"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_RSSI,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "rsrp=");//rsrp="-20"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_RSRP,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "rsrq=");//rsrq="-69"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_RSRQ,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "imei=");//imei="012773002002648"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_IMEI,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "imsi=");//imsi="302720500395176"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_IMSI,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "phonenum=");//phonenum="+14036050307"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_Phone,"%s",p);
                }
            }


            p=NULL;
            p = strstr(buffer_get, "simid=");//simid="89302720403007563710"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_ICCID,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "connect_status=");//connect_status="Connected"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_ActiveStatus,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "roaming=");//roaming="HOME"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_Roaming,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "state=");//state="LTE PS (Packet Switched)"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_Servicetype,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "network=");//network="Verizon"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_Network,"%s",p);
                }
            }

            p=NULL;
            p = strstr(buffer_get, "temp=");//temp=" 39 degC"
            if (p != NULL)
            {
                p = strchr(buffer_get, '=');
                if ( p!= NULL)
                {
                    p++;
                    sprintf(s_Temperature,"%s",p);
                }
            }
        }
        fclose(f);
    }

	return sprintf(s_out,"{\"Type\":\"carrier\",\"ActiveStatus\":\"%s\",\"RSSI\":\"%s\",\"IP\":\"%s\",\"Network\":\"%s\",\"Servicetype\":\"%s\",\"Roaming\":\"%s\",\"Temperature\":\"%s\", \
                  \"IMEI\":\"%s\",\"ICCID\":\"%s\",\"Phone\":\"%s\",\"IMSI\":\"%s\",\"RSRP\":\"%s\",\"RSRP\":\"%s\"}", \
                   s_ActiveStatus,s_RSSI,s_IP,s_Network,s_Servicetype,s_Roaming,s_Temperature,s_IMEI,s_ICCID,s_Phone,s_IMSI,s_RSRP,s_RSRQ);
}


struct per_session_data__radioinfo_pullout {
	//char gpsdata[512];
	int status;
	int try_times;
    int pass_times;
};

static int
callback_radioinfo_pullout(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	int n;
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 +
						  LWS_SEND_BUFFER_POST_PADDING];
	unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
	struct per_session_data__radioinfo_pullout *pss = user;

	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		fprintf(stderr, "callback_gpsnmea_pullout: "
						 "Radio Info LWS_CALLBACK_ESTABLISHED\n");
		pss->status=0;
        if(strlen(s_WebSockSvr_password)<1)pss->status=1;
        pss->try_times=0;
        pss->pass_times=0;
		break;

	/*
	 * in this protocol, we just use the broadcast action as the chance to
	 * send our own connection-specific data and ignore the broadcast info
	 * that is available in the 'in' parameter
	 */

    case LWS_CALLBACK_BROADCAST:
		if(pss->status==1)n=read_radioinfo_jason((char *)p);
        else n=sprintf((char *)p,"Please send connect password.\n");
        if(n>0)
        {
            n = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
            if (n < 0) {
                fprintf(stderr, "ERROR writing to socket");
                pss->try_times++;
                if(pss->try_times>5)
                {
                    libwebsocket_close_and_free_session(context, wsi,
                                       LWS_CLOSE_STATUS_NORMAL);
                }
                return 1;
            }
    
            if(n>0)pss->try_times=0;
        }

		if (close_testing) {
			fprintf(stderr, "close tesing limit, closing\n");
			libwebsocket_close_and_free_session(context, wsi,
						       LWS_CLOSE_STATUS_NORMAL);
		}
		break;

	case LWS_CALLBACK_RECEIVE:
		fprintf(stderr, "rx %d\n", (int)len);
        if (len < 1)
            break;
        //if (strcmp(in, "reset\n") == 0)
        //	pss->status = 0;
        if(pss->status == 0)
        {
            if(strcmp(in,s_WebSockSvr_password)==0)pss->status = 1;
            else pss->pass_times++;
            if(pss->pass_times>10)libwebsocket_close_and_free_session(context, wsi,LWS_CLOSE_STATUS_NORMAL);
        }
		break;
	/*
	 * this just demonstrates how to use the protocol filter. If you won't
	 * study and reject connections based on header content, you don't need
	 * to handle this callback
	 */

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		dump_handshake_info((struct lws_tokens *)(long)user);
		/* you could return non-zero here and kill the connection */
		break;

	default:
		break;
	}

	return 0;
}


static int read_com_socksend(unsigned char * s_out)
{
    int n=0;

    int fd=open("/var/run/lws_com_socksend",O_RDONLY);
    if(fd>0)
    {
        n=read(fd,s_out,2048);
        close(fd);
    }
	return n;
}

struct per_session_data__comport_pullout {
	//char gpsdata[512];
	int status;
	int try_times;
    int pass_times;
};

static int
callback_comport_pullout(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
    int n;
    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 2048 +
                          LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
    struct per_session_data__comport_pullout *pss = user;

    switch (reason) {

    case LWS_CALLBACK_ESTABLISHED:
        fprintf(stderr, "callback_comport_pullout: "
                         "GPSNMEA LWS_CALLBACK_ESTABLISHED\n");
        pss->status=0;
        if(strlen(s_WebSockSvr_password)<1)pss->status=1;
        pss->try_times=0;
        pss->pass_times=0;
        break;

    /*
     * in this protocol, we just use the broadcast action as the chance to
     * send our own connection-specific data and ignore the broadcast info
     * that is available in the 'in' parameter
     */

    case LWS_CALLBACK_BROADCAST:
        if(pss->status==1)n=read_com_socksend(p);
        else n=sprintf((char *)p,"Please send connect password.\n");
        //n = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
        if(n>0)
        {
            n = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
            if (n < 0) {
                fprintf(stderr, "ERROR writing to socket");
                pss->try_times++;
                if(pss->try_times>5)
                {
                    libwebsocket_close_and_free_session(context, wsi,
                                       LWS_CLOSE_STATUS_NORMAL);
                }
                return 1;
            }
            if(n>0)pss->try_times=0;
        }

        if (close_testing) {
            fprintf(stderr, "close tesing limit, closing\n");
            libwebsocket_close_and_free_session(context, wsi,
                               LWS_CLOSE_STATUS_NORMAL);
        }
        break;

    case LWS_CALLBACK_RECEIVE:
        fprintf(stderr, "rx %d\n", (int)len);
        if (len < 1)
            break;
        //if (strcmp(in, "reset\n") == 0)
        //	pss->status = 0;
        if(pss->status == 0)
        {
            if(strcmp(in,s_WebSockSvr_password)==0)pss->status = 1;
            else pss->pass_times++;
            if(pss->pass_times>10)libwebsocket_close_and_free_session(context, wsi,LWS_CLOSE_STATUS_NORMAL);
            break;
        }

        int fd=open("/var/run/lws_com_sockrecv",O_WRONLY|O_CREAT);
        if(fd>0)
        {
            write(fd,(unsigned char *)in,len);
            close(fd);
        }
        break;
    /*
     * this just demonstrates how to use the protocol filter. If you won't
     * study and reject connections based on header content, you don't need
     * to handle this callback
     */

    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        dump_handshake_info((struct lws_tokens *)(long)user);
        /* you could return non-zero here and kill the connection */
        break;

    default:
        break;
    }

    return 0;
}

/* lws-mirror_protocol */

#define MAX_MESSAGE_QUEUE 64

struct per_session_data__lws_mirror {
	struct libwebsocket *wsi;
	int ringbuffer_tail;
};

struct a_message {
	void *payload;
	size_t len;
};

static struct a_message ringbuffer[MAX_MESSAGE_QUEUE];
static int ringbuffer_head;


static int
callback_lws_mirror(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	int n;
	struct per_session_data__lws_mirror *pss = user;

	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		fprintf(stderr, "callback_lws_mirror: "
						 "LWS_CALLBACK_ESTABLISHED\n");
		pss->ringbuffer_tail = ringbuffer_head;
		pss->wsi = wsi;
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (close_testing)
			break;
		if (pss->ringbuffer_tail != ringbuffer_head) {

			n = libwebsocket_write(wsi, (unsigned char *)
				   ringbuffer[pss->ringbuffer_tail].payload +
				   LWS_SEND_BUFFER_PRE_PADDING,
				   ringbuffer[pss->ringbuffer_tail].len,
								LWS_WRITE_TEXT);
			if (n < 0) {
				fprintf(stderr, "ERROR writing to socket");
				exit(1);
			}

			if (pss->ringbuffer_tail == (MAX_MESSAGE_QUEUE - 1))
				pss->ringbuffer_tail = 0;
			else
				pss->ringbuffer_tail++;

			if (((ringbuffer_head - pss->ringbuffer_tail) %
				  MAX_MESSAGE_QUEUE) < (MAX_MESSAGE_QUEUE - 15))
				libwebsocket_rx_flow_control(wsi, 1);

			libwebsocket_callback_on_writable(context, wsi);

		}
		break;

	case LWS_CALLBACK_BROADCAST:
		n = libwebsocket_write(wsi, in, len, LWS_WRITE_TEXT);
		if (n < 0)
			fprintf(stderr, "mirror write failed\n");
		break;

	case LWS_CALLBACK_RECEIVE:

		if (ringbuffer[ringbuffer_head].payload)
			free(ringbuffer[ringbuffer_head].payload);

		ringbuffer[ringbuffer_head].payload =
				malloc(LWS_SEND_BUFFER_PRE_PADDING + len +
						  LWS_SEND_BUFFER_POST_PADDING);
		ringbuffer[ringbuffer_head].len = len;
		memcpy((char *)ringbuffer[ringbuffer_head].payload +
					  LWS_SEND_BUFFER_PRE_PADDING, in, len);
		if (ringbuffer_head == (MAX_MESSAGE_QUEUE - 1))
			ringbuffer_head = 0;
		else
			ringbuffer_head++;

		if (((ringbuffer_head - pss->ringbuffer_tail) %
				  MAX_MESSAGE_QUEUE) > (MAX_MESSAGE_QUEUE - 10))
			libwebsocket_rx_flow_control(wsi, 0);

		libwebsocket_callback_on_writable_all_protocol(
					       libwebsockets_get_protocol(wsi));
		break;
	/*
	 * this just demonstrates how to use the protocol filter. If you won't
	 * study and reject connections based on header content, you don't need
	 * to handle this callback
	 */

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		dump_handshake_info((struct lws_tokens *)(long)user);
		/* you could return non-zero here and kill the connection */
		break;

	default:
		break;
	}

	return 0;
}


/* list of supported protocols and callbacks */

static struct libwebsocket_protocols protocols[] = {
	/* first protocol must always be HTTP handler */

	{
		"http-only",		/* name */
		callback_http,		/* callback */
		0			/* per_session_data_size */
	},
	{
		"dumb-increment-protocol",
		callback_dumb_increment,
		sizeof(struct per_session_data__dumb_increment),
	},
	{
		"mh-gps-protocol",
		callback_gps_pullout,
		sizeof(struct per_session_data__gps_pullout),
	},
    {
        "mh-gpsnmea-protocol",
        callback_gpsnmea_pullout,
        sizeof(struct per_session_data__gpsnmea_pullout),
    },
    {
        "mh-radioinfo-protocol",
        callback_radioinfo_pullout,
        sizeof(struct per_session_data__radioinfo_pullout),
    },
    {
        "mh-comport-protocol",
        callback_comport_pullout,
        sizeof(struct per_session_data__comport_pullout),
    },
	{
		"lws-mirror-protocol",
		callback_lws_mirror,
		sizeof(struct per_session_data__lws_mirror)
	},
	{
		NULL, NULL, 0		/* End of list */
	}
};

static struct option options[] = {
	{ "help",	no_argument,		NULL, 'h' },
	{ "port",	required_argument,	NULL, 'p' },
	{ "ssl",	no_argument,		NULL, 's' },
	{ "killmask",	no_argument,		NULL, 'k' },
	{ "interface",  required_argument,	NULL, 'i' },
	{ "closetest",  no_argument,		NULL, 'c' },
	{ NULL, 0, 0, 0 }
};



int netlib_connectsock(const char *host, const char *service, const char *protocol)
{
    struct hostent *phe;
    struct servent *pse;
    struct protoent *ppe;
    struct sockaddr_in sin;
    int s, type, proto, one = 1;

    memset((char *) &sin, 0, sizeof(sin));
    /*@ -type -mustfreefresh @*/
    sin.sin_family = AF_INET;
    if ((pse = getservbyname(service, protocol)))
        sin.sin_port = htons(ntohs((unsigned short) pse->s_port));
    else if ((sin.sin_port = htons((unsigned short) atoi(service))) == 0)
        return -3;

    if ((phe = gethostbyname(host)))
        memcpy((char *) &sin.sin_addr, phe->h_addr, phe->h_length);
    else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
        return -2;

    ppe = getprotobyname(protocol);
    if (strcmp(protocol, "udp") == 0) {
        type = SOCK_DGRAM;
        proto = (ppe) ? ppe->p_proto : IPPROTO_UDP;
    } else {
        type = SOCK_STREAM;
        proto = (ppe) ? ppe->p_proto : IPPROTO_TCP;
    }

    if ((s = socket(PF_INET, type, proto)) < 0)
        return -4;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one))==-1) {
        (void)close(s);
        return -5;
    }
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        (void)close(s);
        return -6;
    }

#ifdef IPTOS_LOWDELAY
    int opt = IPTOS_LOWDELAY;
    (void)setsockopt(s, IPPROTO_IP, IP_TOS, &opt, sizeof opt);

#endif
#ifdef TCP_NODELAY
    if (type == SOCK_STREAM)
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
#endif
    return s;

}

void write_stop_sign()
{
    FILE *sfp;
    sfp=NULL;
    sfp =fopen("/var/run/lws_stop","w");
    if(sfp!=NULL) 
    {
        fprintf(sfp,"stop");
        fflush(sfp);
        fclose(sfp);
    }
}

int check_stop_sign()
{
    int return_i=0;
    char tmp_buff[30]="";
    FILE *sfp;
    sfp=NULL;
    sfp =fopen("/var/run/lws_stop","r");
    if(sfp!=NULL) 
    {
        fgets(tmp_buff,30,sfp);
        fclose(sfp);
    }
    if(strcmp(tmp_buff,"stop")==0)return_i=1;
    return return_i;
}

int fresh_gpsnmea_file()
{
    int return_i=0;

    char tmp_buff[2048];
    unlink("/var/run/lws_gpsnmea");

    if(gpsd_retry_times>5)
    {
        if(sock>0)
        {
            shutdown(sock,SHUT_RDWR);
            close(sock);
        }
        sock=0;
        gpsd_retry_times=0;
    }

    if(sock<=0)
    {
        struct uci_context *ctx;
        ctx = uci_alloc_context();  // for VIP4G
        if (!ctx) 
        {
            syslog(LOGOPTS,"Web Socket Server EXIT:Out of memory\n");
            exit(-1);
        }
        tmp_buff[0]=0;
        get_option_by_section_name(ctx, "gpsd","gpsd_conf","status",tmp_buff);
        if(tmp_buff[0]!='1')
        {
            uci_free_context(ctx);
            return -1;
        }
        get_option_by_section_name(ctx, "gpsd","gpsd_conf","GPSD_TCP_Port",gps_port);

        uci_free_context(ctx);

        sock = netlib_connectsock( "127.0.0.1", gps_port, "tcp");
        if (sock < 0) 
        {
            syslog(LOGOPTS,"LWS GPS COM Error: could not connect to gpsd local:%s\n", gps_port);
            sock=0;
            return -1;
        }
    }

    //read(sock, tmp_buff, sizeof(tmp_buff));
    strcpy(cbuf, "r=1");
    wrote = write(sock, cbuf, strlen(cbuf));
    if ((ssize_t)strlen(cbuf) != wrote) {
        syslog(LOGOPTS,"LWS GPS COM send first message: ship r=1: write error\n");
        shutdown(sock,SHUT_RDWR);
        close(sock);
        sock=0;
        return -1;
    }

    int readbytes = 0;
    //sleep(1);
    //usleep(250000);
    readbytes = (int)read(sock, tmp_buff, sizeof(tmp_buff));
    if (readbytes <=21)
    {
        gpsd_retry_times++;
        return 0;
    }

    tmp_buff[readbytes]='$';
    tmp_buff[readbytes+1]='\0';
    bzero(cbuf,sizeof(cbuf));
    strcpy(cbuf, "r=0");
    wrote = write(sock, cbuf, strlen(cbuf));
    gpsd_retry_times=0;

    FILE *sfp;
    sfp=NULL;
    sfp =fopen("/var/run/lws_gpsnmea","w");
    if(sfp!=NULL) 
    {
        char valid_buff[200];
        char *p1;
        char *p2;
        char *p3;
        int n;
        if(c_WebSockSvr_GPSNMEA_RMC=='B')
        {
            p1=NULL;
            p2=NULL;
            p1=strstr(tmp_buff,"$GPRMC");
            if(p1!=NULL)p2=strchr(p1+5,'*');
            if(p2!=NULL)
            {
                p2=p2+3;
                while(*p2==10 || *p2==13)p2++;
                n=p2-p1;
                if(n<200)
                {
                    strncpy(valid_buff,p1,n);
                    valid_buff[n]=0;
                    fprintf(sfp,"%s",valid_buff);
                    return_i=1;
                }
            }
        }

        if(c_WebSockSvr_GPSNMEA_GGA=='B')
        {
            p1=NULL;
            p2=NULL;
            p1=strstr(tmp_buff,"$GPGGA");
            if(p1!=NULL)p2=strchr(p1+5,'*');
            if(p2!=NULL)
            {
                p2=p2+3;
                while(*p2==10 || *p2==13)p2++;
                n=p2-p1;
                if(n<200)
                {
                    strncpy(valid_buff,p1,n);
                    valid_buff[n]=0;
                    fprintf(sfp,"%s",valid_buff);
                    return_i=1;
                }
            }
        }

        if(c_WebSockSvr_GPSNMEA_GSA=='B')
        {
            p1=NULL;
            p2=NULL;
            p1=strstr(tmp_buff,"$GPGSA");
            if(p1!=NULL)p2=strchr(p1+5,'*');
            if(p2!=NULL)
            {
                p2=p2+3;
                while(*p2==10 || *p2==13)p2++;
                n=p2-p1;
                if(n<200)
                {
                    strncpy(valid_buff,p1,n);
                    valid_buff[n]=0;
                    fprintf(sfp,"%s",valid_buff);
                    return_i=1;
                }
            }
        }

        if(c_WebSockSvr_GPSNMEA_VTG=='B')
        {
            p1=NULL;
            p2=NULL;
            p1=strstr(tmp_buff,"$GPVTG");
            if(p1!=NULL)p2=strchr(p1+5,'*');
            if(p2!=NULL)
            {
                p2=p2+3;
                while(*p2==10 || *p2==13)p2++;
                n=p2-p1;
                if(n<200)
                {
                    strncpy(valid_buff,p1,n);
                    valid_buff[n]=0;
                    fprintf(sfp,"%s",valid_buff);
                    return_i=1;
                }
            }
        }

        if(c_WebSockSvr_GPSNMEA_GSV=='B')
        {
            p1=NULL;
            p2=NULL;
            p1=strstr(tmp_buff,"$GPGSV");
            if(p1!=NULL)
            {
                p3=p1;
                while(p3!=NULL)
                {
                    p2=p3;
                    p3=strchr(p3+5,'$');
                    if(p3!=NULL)
                    {
                        if(*(p3+1)!='G' || *(p3+2)!='P' ||*(p3+3)!='G' ||*(p3+4)!='S' ||*(p3+5)!='V')break;
                    }
                }
                p2=strchr(p2+5,'*');
            }
            if(p2!=NULL)
            {
                p2=p2+3;
                while(*p2==10 || *p2==13)p2++;
                *p2=0;
                fprintf(sfp,"%s",p1);
                return_i=1;
            }
        }

        fflush(sfp);
        fclose(sfp);
    }
    return return_i;
}

int trans_to_comdata(unsigned char *pbuf,int bytes_num)
{
    if(c_WebSockSvr_COM_MODE!='B')return bytes_num;

    int i,j;
    char cs[3];
    unsigned int cj;

    i=0;
    j=0;
    while(j<(bytes_num-1))
    {
        cs[0]=*(pbuf+j);
        cs[1]=*(pbuf+j+1);
        cs[2]=0;
        sscanf(cs,"%X",&cj);
        *(pbuf+i)=cj;

        i++;
        j++;
        j++;
    }

    return i;
}
//to be sure buffer is enough.  From text to ascii.
int trans_to_sockdata(unsigned char *pbuf,int bytes_num)
{
    if(c_WebSockSvr_COM_MODE!='B')return bytes_num;

    int i,j;
    unsigned char cs[3];
    unsigned char c;
    i=bytes_num-1;
    bytes_num=2*bytes_num;
    j=bytes_num-1;

    while(i>=0) 
    {
        c=*(pbuf+i);
        cs[0]=c>>4;
        cs[1]=c&0x0F;
        if(cs[0]>9)cs[0]=cs[0]+0x37;
        else cs[0]=cs[0]+0x30; 
        if(cs[1]>9)cs[1]=cs[1]+0x37;
        else cs[1]=cs[1]+0x30; 
        *(pbuf+j)=cs[1];
        *(pbuf+j-1)=cs[0];

        i--;
        j--;
        j--;
    }
    return bytes_num;
}

int com_server_retry=0;
int fresh_comport_with_file()
{
    //printf("-----%c,%d,%s,%d\n",c_Com_TCPServer_status,com_sock,s_Com_TCPServer_port,com_server_retry);
    if(c_Com_TCPServer_status!='1' || strlen(s_Com_TCPServer_port)<2)
    {
        if(com_sock>0)
        {
            shutdown(com_sock,SHUT_RDWR);
            close(com_sock);
        }
        com_sock=0;
    }

    if(com_sock<=0)
    {
        read_config_comserver();
        //printf("++++++++%c,%d,%s,%d\n",c_Com_TCPServer_status,com_sock,s_Com_TCPServer_port,com_server_retry);
        if(c_Com_TCPServer_status!='1')return -1;

        com_sock = netlib_connectsock( "127.0.0.1", s_Com_TCPServer_port, "tcp");
        if (com_sock < 0) 
        {
            syslog(LOGOPTS,"LWS COM TCP Error: could not connect to tcp server local:%s\n", s_Com_TCPServer_port);
            com_sock=0;
            return -1;
        }
        fcntl (com_sock, F_SETFL, fcntl(com_sock, F_GETFL) | O_NONBLOCK);
        com_server_retry=0;
    }

    //firstly send
    int fd=0;
    int bytes_num=0;
    int tmp_i;
    unsigned char tmp_buff[4096];
    fd=open("/var/run/lws_com_sockrecv",O_RDONLY);
    if(fd>0)
    {
        bytes_num=read(fd,tmp_buff,2048);
        close(fd);
        unlink("/var/run/lws_com_sockrecv");
    }
    if(bytes_num>0)
    {
        bytes_num=trans_to_comdata(tmp_buff,bytes_num);
        tmp_i=write(com_sock, tmp_buff, bytes_num);
        bytes_num=0;
        if(tmp_i<0)
        {
            c_Com_TCPServer_status='0';
            return -1;
        }

    }

    //read and save
    tmp_i=0;
    unlink("/var/run/lws_com_socksend");
    bytes_num = read(com_sock, tmp_buff, 1024);
    if(bytes_num>0)
    {
        fd=open("/var/run/lws_com_socksend",O_WRONLY|O_CREAT);
        if(fd>0)
        {
            bytes_num=trans_to_sockdata(tmp_buff,bytes_num);
            tmp_i=write(fd,tmp_buff,bytes_num);
            close(fd);
        }
        com_server_retry=0;
    }
    if(bytes_num<0)
    {
        //printf("***************com server read fail:%d:%s\n",com_server_retry,strerror(errno));
        if(errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)com_server_retry++;
        if(com_server_retry>6)c_Com_TCPServer_status='0';
        return -1;
    }
    if(bytes_num==0)c_Com_TCPServer_status='0';  //closed by remote.

    return tmp_i;
}

int main(int argc, char **argv)
{
    (void) setsid();
    unlink("/var/run/lws_stop");                                  
    read_config();
    if(c_WebSockSvr_Enable!='B')return 0;

	int n = 0;
	const char *cert_path =
			    LOCAL_RESOURCE_PATH"/libwebsockets-test-server.pem";
	const char *key_path =
			LOCAL_RESOURCE_PATH"/libwebsockets-test-server.key.pem";
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 1024 +
						  LWS_SEND_BUFFER_POST_PADDING];
	int port = i_WebSockSvr_port;//7681;
	int use_ssl = 0;
	struct libwebsocket_context *context;
	int opts = 0;
	char interface_name[128] = "";
	const char *interface = NULL;
#ifdef LWS_NO_FORK
	unsigned int oldus = 0;
#endif

	fprintf(stderr, "libwebsockets test server\n"
			"(C) Copyright 2010-2011 Andy Green <andy@warmcat.com> "
						    "licensed under LGPL2.1\n");

	while (n >= 0) {
		n = getopt_long(argc, argv, "ci:khsp:", options, NULL);
		if (n < 0)
			continue;
		switch (n) {
		case 's':
			use_ssl = 1;
			break;
		case 'k':
			opts = LWS_SERVER_OPTION_DEFEAT_CLIENT_MASK;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'i':
			strncpy(interface_name, optarg, sizeof interface_name);
			interface_name[(sizeof interface_name) - 1] = '\0';
			interface = interface_name;
			break;
		case 'c':
			close_testing = 1;
			fprintf(stderr, " Close testing mode -- closes on "
					   "client after 50 dumb increments"
					   "and suppresses lws_mirror spam\n");
			break;
		case 'h':
			fprintf(stderr, "Usage: test-server "
					     "[--port=<p>] [--ssl]\n");
			exit(1);
		}
	}

	if (!use_ssl)
		cert_path = key_path = NULL;

	context = libwebsocket_create_context(port, interface, protocols,
				libwebsocket_internal_extensions,
				cert_path, key_path, -1, -1, opts);
	if (context == NULL) {
		fprintf(stderr, "libwebsocket init failed\n");
		return -1;
	}

	buf[LWS_SEND_BUFFER_PRE_PADDING] = 'x';

#ifdef LWS_NO_FORK

	/*
	 * This example shows how to work with no forked service loop
	 */

	fprintf(stderr, " Using no-fork service loop\n");

	n = 0;
	while (n >= 0) {
		struct timeval tv;

		gettimeofday(&tv, NULL);

		/*
		 * This broadcasts to all dumb-increment-protocol connections
		 * at 20Hz.
		 *
		 * We're just sending a character 'x', in these examples the
		 * callbacks send their own per-connection content.
		 *
		 * You have to send something with nonzero length to get the
		 * callback actions delivered.
		 *
		 * We take care of pre-and-post padding allocation.
		 */

		if (((unsigned int)tv.tv_usec - oldus) > 50000) {
			libwebsockets_broadcast(
					&protocols[PROTOCOL_DUMB_INCREMENT],
					&buf[LWS_SEND_BUFFER_PRE_PADDING], 1);
			oldus = tv.tv_usec;
		}

		/*
		 * This example server does not fork or create a thread for
		 * websocket service, it all runs in this single loop.  So,
		 * we have to give the websockets an opportunity to service
		 * "manually".
		 *
		 * If no socket is needing service, the call below returns
		 * immediately and quickly.  Negative return means we are
		 * in process of closing
		 */

		n = libwebsocket_service(context, 50);
	}

#else

	/*
	 * This example shows how to work with the forked websocket service loop
	 */

	fprintf(stderr, " Now Using forked service loop\n");

	/*
	 * This forks the websocket service action into a subprocess so we
	 * don't have to take care about it.
	 */

	n = libwebsockets_fork_service_loop(context);
	if (n < 0) {
		fprintf(stderr, "Unable to fork service loop %d\n", n);
		return 1;
	}

    int ms_i=0;
    int time_count_i=0;
    i_WebSockSvr_timeout=4*i_WebSockSvr_timeout;
    i_WebSockSvr_freshinter=4*i_WebSockSvr_freshinter;

	while (check_stop_sign()==0) {

        //condition for three type service:
        if(c_WebSockSvr_GPS=='B')
        {
            libwebsockets_broadcast(&protocols[PROTOCOL_GPS_PULLOUT],
                        &buf[LWS_SEND_BUFFER_PRE_PADDING], 1);
        }

        if(c_WebSockSvr_GPSNMEA=='B')
        {

            if(c_WebSockSvr_COM=='B')  //usleep process in gpsnmea function. add one more.
            {
                if(fresh_comport_with_file()>0)
                libwebsockets_broadcast(&protocols[PROTOCOL_COMPORT_PULLOUT],
                            &buf[LWS_SEND_BUFFER_PRE_PADDING], 1);
            }

            if(fresh_gpsnmea_file()>0)
            libwebsockets_broadcast(&protocols[PROTOCOL_GPSNMEA_PULLOUT],
                        &buf[LWS_SEND_BUFFER_PRE_PADDING], 1);
            time_count_i++;
        }
        
        if(c_WebSockSvr_RADIO=='B')
        {
            libwebsockets_broadcast(&protocols[PROTOCOL_RADIOINFO_PULLOUT],
                        &buf[LWS_SEND_BUFFER_PRE_PADDING], 1);
        }


        if(close_testing==1)break;

        for(ms_i=1;ms_i<i_WebSockSvr_freshinter;ms_i++)
        {
            if(c_WebSockSvr_COM=='B')
            {
                if(fresh_comport_with_file()>0)
                libwebsockets_broadcast(&protocols[PROTOCOL_COMPORT_PULLOUT],
                            &buf[LWS_SEND_BUFFER_PRE_PADDING], 1);
            }
            usleep(250000);
            time_count_i++;
        }


        if(i_WebSockSvr_timeout>0)
        {
            if(time_count_i>i_WebSockSvr_timeout)
            {
                close_testing=1;
                close_service_config();
                sleep(1);
                libwebsocket_context_destroy(context);
                write_stop_sign();
                exit(0);
            }
        }

	}//while

#endif

	libwebsocket_context_destroy(context);

	return 0;
}
