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
#include "gnokii.h"
#include "atgen.h"



#define	LOGOPTS		(LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)
#ifdef MEMWATCH
    #include "memwatch.h"
#endif

extern struct uci_context *ctx;

extern char *RadioBusyFlagFile;
extern char *CmdHistoryFile;
extern char *CmdHistoryFilebak;
extern char *SendHistoryFile;
extern char *SendHistoryFilebak;
extern char *ReadsmsFile;
extern char *SendsmsFile;
extern char *SAlertHistoryFile;
extern char *SAlertHistoryFilebak;


extern struct gn_statemachine *state;
extern gn_data *data;
extern  int smtotal, used;
extern char strOut[1024];
extern int fd_radiobusy;
extern int smsread_state;


extern int sms_history_to_flash(char* loginfo,char* logfile);
extern bool get_new_sms(char *sms_data, char *phone_number);
extern void busterminate(void);
extern void businit(void);
extern void send_sms(char *phone_number, char *sms_data, int len);
extern void LockFile(int fd);
extern void UnLockFile(int fd);
extern gn_error decodesms(unsigned char *buffer, int length, gn_data *data, struct gn_statemachine *state);
extern void delete_one_sms(int location);
extern int get_smtotal(void);
extern void get_local_time(gn_sms *sms, char *dt);
extern void get_new_sms_list(void);
extern int check_carrier_satus(void);

extern int SubProgram_Start(char *pszPathName, char *pszArguments);
extern bool get_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value);
extern bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char*section_name, char*option_name, char *value);

