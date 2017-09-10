
#include <syslog.h>
#include "serialport_driver.h"


#define FALSE  -1
#define TRUE   0

#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)

struct termio {
    unsigned short  c_iflag;        /* input mode flag*/
    unsigned short  c_oflag;                /*out put mode flag*/
    unsigned short  c_cflag;                /* control mode flag*/
    unsigned short  c_lflag;                /* local mode flags */
    unsigned char  c_line;              /* line discipline */
    //        unsigned char  c_cc[NCC];    /* control characters */
    unsigned char c_cc[8];
};

#define B3600  3600
#define B7200 7200
#define B14400 14400
#define B28800 28800
struct  termios Opt;

int speed_arr[] =
{B300,B600,B1200,B2400,B3600,B4800,B7200,B9600,B14400,B19200,B28800,B38400,B57600,B115200,B230400,B460800,B921600};
int name_arr[] ={300,600,1200,2400,3600,4800,7200,9600,14400,19200,28800,38400,57600,115200,230400,460800,921600};

/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

void Set_Speed(int fd, int speed)
{
    int   i;
    int   status;
    struct termios   Opt;

    syslog(LOGOPTS,"Enter [%s]: speed=%d\n", __func__, speed);

    /* no matter com1 or com2 should get attr first or com2 will stop shell and textUI has problem*/
    tcgetattr(fd, &Opt);

#ifdef IXP425COM3//add this will show input with hand exec soipd2 not background mode
    tcgetattr(fd, &Opt);
    tcflush(fd,TCIFLUSH);  
    Opt.c_cflag = CS8 | HUPCL | CREAD;
    Opt.c_cflag |= CLOCAL;

    Opt.c_iflag = Opt.c_lflag = Opt.c_oflag ;
    Opt.c_line = 0;
    Opt.c_cc[VMIN] = 0;
    Opt.c_cc[VTIME] = 0;

#endif 

    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
        if (speed == name_arr[i]) {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);
            if (status != 0) {
                perror("tcsetattr fd1");
                return ;
            }
            tcflush(fd,TCIOFLUSH);
        }
    }
}

/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

int Set_DataFrame(int fd,int databits,int stopbits,char parity,int flowcontrol)
{
    struct termios options;

    syslog(LOGOPTS,"Enter [%s]: databits=%d, stopbits=%d, parity=%c, flowcontrol=%d\n", __func__, databits, stopbits, parity, flowcontrol);

    if ( tcgetattr( fd,&options)  !=  0) {
        perror("SetupSerial 1");
        return(-1);
    }
    cfmakeraw(&options);
    options.c_cflag &= ~CSIZE;
    switch (databits) {
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr,"Unsupported data size\n");
        return(-1);
    }
    switch (parity) {
    case 'n':
    case 'N':
        options.c_cflag &= ~PARENB;   /* Clear parity enable */
        options.c_iflag &= ~INPCK;     /* Enable parity checking */
        break;
    case 'o':
    case 'O':
        options.c_cflag |= (PARODD | PARENB); /* set to Ode checking*/
        options.c_iflag |= INPCK;             /* Disnable parity checking */
        break;
    case 'e':
    case 'E':
        options.c_cflag |= PARENB;     /* Enable parity */
        options.c_cflag &= ~PARODD;   /* set to even checking*/
        options.c_iflag |= INPCK;       /* Disnable parity checking */
        break;
    case 'S':
    case 's':  /*as no parity*/
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;break;
    default:
        fprintf(stderr,"Unsupported parity\n");
        return(-1);
    }
    /* set stop bit*/
    switch (stopbits) {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        {
            fprintf(stderr,"Unsupported stop bits\n");
            return(-1);
        }
    }

    switch (flowcontrol) {
    case 1:
        options.c_cflag |= CRTSCTS;           
        break;
    case 0:
        options.c_cflag &= ~CRTSCTS;           
        break;
    default:
        fprintf(stderr,"Unsupported flow control data size\n");
        return(-1);
    }

    /* Set input parity option */
    if (parity != 'n')
        options.c_iflag |= INPCK;
    /*

    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */

    options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/

    // options.c_lflag = 0;
    options.c_oflag &=~OPOST ;//????
    if (tcsetattr(fd,TCSANOW,&options) != 0) {
        perror("SetupSerial 3");
        return(-1);  
    }
    return(0);
}

/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

int OpenDev(char *Dev)
{

    int     fd = open( Dev, O_RDWR| O_NOCTTY | O_NDELAY );         //
    if (-1 == fd) {
        perror("Can't Open Serial Port");
        return -1;
    } else
        return fd;


}

/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

int CloseDev(int fd)
{
    return(close(fd));

}



