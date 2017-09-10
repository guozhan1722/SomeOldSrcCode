#include <pthread.h>
#include <stdarg.h>
#include <pwd.h>
#include <netinet/tcp.h>
#include <stdio.h>
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
#include <net/if.h>
#include <stdarg.h>
#include <pwd.h>
#include <netinet/tcp.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <termios.h>
#include <syslog.h>
#include <sys/sysinfo.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef EMBED
#include <sys/select.h>
#endif

#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)   


static int fdcom;

/*
 *	Define some parity flags, internal use only.
 */
#define	PARITY_NONE	0
#define	PARITY_EVEN	1
#define	PARITY_ODD	2

/*
 *	Default port settings.
 */
int		clocal;
int		hardware;
int		software;
int		passflow;
int		parity = PARITY_NONE;
int		databits = 8;
int		twostopb = 0;
unsigned int	baud = 115200;


/*
 *	Baud rate table for baud rate conversions.
 */
typedef struct baudmap {
	unsigned int	baud;
	unsigned int	flag;
} baudmap_t;


struct baudmap	baudtable[] = {
	{ 0, B0 },
	{ 50, B50 },
	{ 75, B75 },
	{ 110, B110 },
	{ 134, B134 },
	{ 150, B150 },
	{ 200, B200 },
	{ 300, B300 },
	{ 600, B600 },
	{ 1200, B1200 },
	{ 1800, B1800 },
	{ 2400, B2400 },
	{ 4800, B4800 },
	{ 9600, B9600 },
	{ 19200, B19200 },
	{ 38400, B38400 },
	{ 57600, B57600 },
	{ 115200, B115200 },
	{ 230400, B230400 },
	{ 460800, B460800 }
};

#define	NRBAUDS		(sizeof(baudtable) / sizeof(struct baudmap))

/*****************************************************************************/

/*
 *	Verify that the supplied baud rate is valid.
 */

int baud2flag(unsigned int speed)
{
	int	i;

	for (i = 0; (i < NRBAUDS); i++) {
		if (speed == baudtable[i].baud)
			return(baudtable[i].flag);
	}
	return(-1);
}

int setremotetermios(int rfd)
{
	struct termios	tio;

	memset(&tio, 0, sizeof(tio));
	tio.c_cflag = CREAD | HUPCL | baud2flag(baud);

	if (clocal)
		tio.c_cflag |= CLOCAL;

	switch (parity) {
	case PARITY_ODD:	tio.c_cflag |= PARENB | PARODD; break;
	case PARITY_EVEN:	tio.c_cflag |= PARENB; break;
	default:		break;
	}

	switch (databits) {
	case 5:		tio.c_cflag |= CS5; break;
	case 6:		tio.c_cflag |= CS6; break;
	case 7:		tio.c_cflag |= CS7; break;
	default:	tio.c_cflag |= CS8; break;
	}
	
	if (twostopb)
		tio.c_cflag |= CSTOPB;

	if (software)
		tio.c_iflag |= IXON | IXOFF;
	if (hardware)
		tio.c_cflag |= CRTSCTS;

	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	if (tcsetattr(rfd, TCSAFLUSH, &tio) < 0) {
		fprintf(stderr, "ERROR: remote tcsetattr(TCSAFLUSH) failed, "
			"errno=%d\n", errno);
		exit(1);
	}
	return(0);
}


int main( int argc, char *argv[])
{
    int retval = 0;
    unsigned short tech = 0;
    char read_only=0;
    int i,j,k,l,buff_len;

    if(argc < 2 || argc>3 )
    {
        printf("Please use parameters with port id: /dev/ttyUSB? [get/(setting code:0/128/16384/32768)] \n");
        return -3;
    }
    char portstr[100];
    strcpy(portstr,argv[1]);

    if (argc == 2) tech=0;

    //get tech
    if (argc == 3) 
    {
        if(strcmp(argv[2],"get")==0)
        {
            read_only=1;
        }else
        {
            tech = atoi(argv[2]);
        }
    }

    //printf("Open and set with port:%s\n",portstr);
    i=0;
retry_opencom:
    fdcom = open( portstr, O_RDWR | O_NDELAY);
    if (fdcom<0) 
    {
        i++;
        syslog(LOGOPTS,"open com Error%d %s\n",i,strerror(errno)); 
        //printf("Open com port error:%s\n",portstr); 
        if(i<5)
        {
            sleep(1);
            goto retry_opencom;
        }
        return -1;
    }
    setremotetermios(fdcom);


    unsigned char recvdata[1000];
    unsigned char senddata[140];//need 136 bytes
    strcpy(senddata,"AT$NWDMAT=0");
    j=write(fdcom, senddata, strlen(senddata));
    //printf("Now send mode cmd %d Bytes\n",j);

    memset(senddata,0,140);
    senddata[0]=0x26;
    senddata[1]=0xA6;
    senddata[2]=0x04;
    senddata[133]=0x9D;
    senddata[134]=0x57;
    senddata[135]=0x7E;
    usleep(20000);
    j=read( fdcom, recvdata, 200);

    int ii;
    unsigned char recv_ok=0;
    unsigned short r1190_sign=0;
    for(ii=0;ii<30;ii++)
    {
        j=write(fdcom, senddata, 136);
        usleep(20000);
        //printf("\nNow No.%d send read 1190 cmd %d Bytes\n",ii,j);
        if(j<1)
        {
            sleep(1);
            continue;
        }

        memset(recvdata,0,1000);
        buff_len=0;
        for(i=0;i<5;i++)
        {
            j=read( fdcom, recvdata+buff_len, 200);
            if(j>0)
            {
                for(k=buff_len;k<buff_len+j;k++) 
                {
                    //printf("%x ",recvdata[k]);
                    if(recvdata[k]==0x26 && recvdata[k+1]==0xA6 && recvdata[k+2]==0x04 && recvdata[k+135]==0x7E)
                    {
                        recv_ok=99;
                        r1190_sign=recvdata[k+3] | recvdata[k+4];
                        if(read_only==1)
                        {
                            //tech=recvdata[k+3]&0x00FF;
                            //tech=(tech<<8)+recvdata[k+4]&0x00FF;
                            printf("%02X%02X\n",recvdata[k+3],recvdata[k+4]);
                            goto exit0;
                        }
                    }
                }
                buff_len=buff_len+j;
            }
            if(recv_ok==99)break;
            usleep(20000);
        }
        //printf("\nTotal recv %d Bytes:%d,%x.\n",buff_len,recv_ok,r1190_sign);
        if(recv_ok==99)break;
        sleep(1);
    }

    if(recv_ok!=99)
    {
        //printf("\n Error data and stop setting\n");
        retval = -1;
        goto exit0;
    }

    if(r1190_sign==0 && tech == 0)
    {
        //printf("\n Correct setting and exit\n");
        retval = 0;
        goto exit0;
    }

    memset(senddata,0,140);
    senddata[0]=0x27;
    senddata[1]=0xA6;
    senddata[2]=0x04;
    senddata[3]= (tech >> 8) & 0xFF;
    senddata[4]= tech & 0xFF;
    if (tech == 0) {
        senddata[133]=0x88;
        senddata[134]=0x67;
    } else if (tech == 0x0080) {
        senddata[133]=0xe4;
        senddata[134]=0xb8;
    } else if (tech == 0x4000) {
        senddata[133]=0x5a;
        senddata[134]=0xb7;
    } else if (tech == 0x8000) {
        senddata[133]=0x3d;
        senddata[134]=0xce;
    }else goto exit0;

    senddata[135]=0x7E;

    j=read( fdcom, recvdata, 200);
    for(ii=0;ii<30;ii++)
    {
        j=write(fdcom, senddata, 136);
        usleep(20000);
        //printf("\nNow No.%d send write 1190 cmd %d Bytes\n",ii,j);
        if(j>0)
        {
            for(i=0;i<5;i++)
            {
                j=read(fdcom, recvdata, 200);
                if(j>0) {
                    syslog(LOGOPTS,"Set RPLM_NACT(1190) to %d: %02X %02X\n",tech,senddata[3],senddata[4]); 
                    printf("%02X%02X\n",senddata[3],senddata[4]);
                    break;
                }
                usleep(20000);
            }
            if(j>0) {
                retval = 1;
                goto exit0;
            }
        }
        sleep(1);
    }
    retval = -2;
   
exit0: 
    
    strcpy(senddata,"AT$NWDMAT=1");
    j=write(fdcom, senddata, strlen(senddata));

    usleep(20000);
    j=read( fdcom, recvdata, 200);

    close(fdcom);
    return retval;
}


/*


    //connect echo
    senddata[0]=0x4B;
    senddata[1]=0x04;
    senddata[2]=0x0E;
    senddata[3]=0x00;
    senddata[4]=0x0D;
    senddata[5]=0xD3;
    senddata[6]=0x7E;

    //offline
    senddata[0]=0x29;
    senddata[1]=0x01;
    senddata[2]=0x00;
    senddata[3]=0x31;
    senddata[4]=0x40;
    senddata[5]=0x7E;

    //reset
    senddata[0]=0x29;
    senddata[1]=0x02;
    senddata[2]=0x00;
    senddata[3]=0x59;
    senddata[4]=0x6A;
    senddata[5]=0x7E;
*/






