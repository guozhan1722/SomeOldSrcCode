#ifndef GPSR_H
#define GPSR_H

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
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
#include "database.h"
#define DEFAULT_GPSD_PORT "2947"
#define DEFAULT_GPSD_SERVER "127.0.0.1"
#define REPORT_NUMBER 4

/* for schedule timer triggle and distance triggle */
#define ENABLE 1
#define DISABLE 0
#define ANDCONF 1
#define ORCONF 2
#define TIMER 1
#define TIMERANDDIS 2
#define TIMERORDIS 3
#define ORDIS 4
#define NO 0
#define FAILED -1
#define SUCCESS 0
#define POSRECORD 8

char *gpsrscriptFile="/bin/gpsrscript";

/* data struct for return value of init gpsr */
typedef struct return_value_initgpsr
{
	int count; // how many can reports be enabled after restrict check
	int pos_flag; // inform if to record gps position
}return_value_initgpsr;

/* data struct for deciding scheduler */
typedef struct schedule_source
{
	struct timeval tval; // time interval
	struct timeval tnex; // accumulate time interval
	struct timeval texp; // accumulate cx setting timer
	struct timeval tcxs; // cx setting timer(sec)
	struct gps_position basepos; // base gps position
	int distance_enable;
	int timer_enable;
	int trigger_conf;
	int rnum;
	int scheduled;
}schedule_source;

/* data struct for UDP socket for remote server*/
typedef struct socket_source
{
	int sockfd;
	int num;                           // Counter of sent bytes
	struct sockaddr_in addr_remote;    // Host address information
	int errorno;
}socket_source;

#endif
