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
struct uci_context *ctx = NULL;

#define MAX_RULES 100
#define CONFILE "firewall"
//#define FW_DEBUG 1
struct fw_rules {
    char src[32];
    char src_ip[32];
    char src_mac[32];
    char src_port[32];
    char dest[32];
    char dest_ip[32];
    char dest_port[32];
    char proto[32];
    char icmp_type[32];
    char target[32];
    char ruleset[32];
    char pri[32];
    char ZONE[32];
    char TARGET[32];
    int index;
    struct fw_rules *next;
};

static struct fw_rules *fwrules_head;
static struct fw_rules fwrules;

 
