

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>
#include "unistd.h"

#include <syslog.h>

#include <sys/wait.h>
#include <sys/time.h>
#define _XOPEN_SOURCE /* glibc2 needs this */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>													

#include <stdbool.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>

#include "uci.h"
#include "gpsrlib.h"
//#include "database.c"
#define MAX_REC_FILE_NUM 51 //max 249
#define REC_FILE_ALL 251

char *gpsrecord_dir="/etc/gpsdata/rec";//4g
char *gpsrecord_varfile="/var/run/gpsrec";//4g
//char *gpsrecord_dir="/log/gpsrec/";//3g


