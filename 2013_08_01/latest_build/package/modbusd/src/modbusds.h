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

#include "uci.h"


#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)   

#ifndef SIOCGMIIPHY
#define SIOCGMIIPHY (SIOCDEVPRIVATE)	/* Read from current PHY */
#define SIOCGMIIREG (SIOCDEVPRIVATE+1) 	/* Read any PHY register */
#define SIOCSMIIREG (SIOCDEVPRIVATE+2) 	/* Write any PHY register */
#define SIOCGPARAMS (SIOCDEVPRIVATE+3) 	/* Read operational parameters */
#define SIOCSPARAMS (SIOCDEVPRIVATE+4) 	/* Set operational parameters */
#endif

//#define MAX_MASTER_DATAPACK 17 //only accept 
#define RECV_BUFF_LEN_MAX 150 
#define SEND_BUFF_LEN_MAX 516
#define REMOTE_TCP_MAX_NUMBER 10

#define INPUT1 98
#define OUTPUT1 95
//#define DEVICE_NAME "/dev/mhx-bsp"   //3g
#define DEVICE_NAME "/dev/vip-bsp"     //4g
#define ADC_DEVICE "/dev/at91_adc"
#define OPEN 0
#define CLOSE 1
#define ADC_READ 2

#define KEEP_IDLE 1200  //20 minutes, max time before check if connection is ok.
#define KEEP_COUNT 15  //max times for above check.
#define KEEP_INTVL 60 //60 seconds, interval time between two times' check. 

#define MII_BMSR    0x01
struct mii_data {
    __u16	phy_id;
    __u16	reg_num;
    __u16	val_in;
    __u16	val_out;
};



union _bsint2
{
    int16_t i;
    char c[2];
};

union _bsint4
{
    int32_t i;
    char c[4];
};

union _bint2
{
    uint16_t i;
    char c[2];
};
union _bint4
{
        uint32_t i;
        char c[4];
};
union _bint8
{
        uint64_t i;
        char c[8];
};
union __hton16_struct
{
    char u_c[2];
    uint16_t u_i;
};
union __hton32_struct
{
    char u_c[4];
    uint32_t u_i;
};
union __hton64_struct
{
    char u_c[8];
    uint64_t u_i;
};





#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)  

#define DBG_log soip_DBG_log

#define LOG_WIDTH   1024    /* roof of number of chars in log line */
/**********************************************************************************
   Function:     int main(int argc, char *argv[])
   Input:        no input
   Output:       no output
   Return:       0
   Note:         the main function of soip
**********************************************************************************/
  
#define COM_STATISTICS_RECEIVED_BYTES 0
#define COM_STATISTICS_RECEIVED_PACKETS 1     
#define COM_STATISTICS_RECEIVE_DROPED_BYTES 2
#define COM_STATISTICS_RECEIVE_DROPED_PACKETS 3     
#define COM_STATISTICS_SEND_BYTES 8
#define COM_STATISTICS_SEND_PACKETS 9
#define COM_STATISTICS_SENT_DROPED_BYTES 10
#define COM_STATISTICS_SENT_DROPED_PACKETS 11
#define COM_STATISTICS_PROGRAM_STATUS 19




#define VIP_MAJOR_DEVICE            122
#define VIP_MINOR_DEVICE 			0

#define VIP_BSP_MAGICNO 'V'

#define IOCTL_GPIO3_HIGH      _IOW(VIP_BSP_MAGICNO,0x10,int)      
#define IOCTL_GPIO3_LOW       _IOW(VIP_BSP_MAGICNO,0x11,int)

#define IOCTL_GPIO4_HIGH            _IOW(VIP_BSP_MAGICNO,0x20,int)
#define IOCTL_GPIO4_LOW             _IOW(VIP_BSP_MAGICNO,0x21,int)

#define IOCTL_GPIO5_HIGH            _IOW(VIP_BSP_MAGICNO,0x30,int)
#define IOCTL_GPIO5_LOW             _IOW(VIP_BSP_MAGICNO,0x31,int)

#define IOCTL_GPIO11_HIGH           _IOW(VIP_BSP_MAGICNO,0x40,int)
#define IOCTL_GPIO11_LOW            _IOW(VIP_BSP_MAGICNO,0x41,int)

#define IOCTL_GPIO_SET              _IOW(VIP_BSP_MAGICNO,0x50,int)
#define IOCTL_GPIO_GET              _IOR(VIP_BSP_MAGICNO,0x51,int)

#define IOCTL_GET_HW_REV            _IOR(VIP_BSP_MAGICNO,0x61,int)
#define IOCTL_SET_CPU_SPEED         _IOW(VIP_BSP_MAGICNO,0x70,int)
#define IOCTL_GET_CPU_SPEED         _IOR(VIP_BSP_MAGICNO,0x71,int)

struct gpio_ioctl {
	unsigned int gpio;
	unsigned int value;
};

