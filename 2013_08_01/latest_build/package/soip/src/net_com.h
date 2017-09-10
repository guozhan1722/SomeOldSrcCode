
#ifndef NET_COM_H
#define NET_COM_H

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>   
#include <unistd.h>
#include <termios.h>
#include <syslog.h>
#include <sys/time.h>
#include <time.h>
#include "soip.h"
#include <sys/sysinfo.h>


#define TCP_PROTO "tcp"

int read_from_serial_port(int fd, char *buffer, int length);
int write_to_serial_port(int fd,char* buffer, int length);

int read_from_tcp_socket(int fd, char *buffer, int length);
int write_to_tcp_socket(int fd,char* buffer, int length);

int read_from_udp_socket(int fd, char *buffer, int length);//,struct sockaddr *remote_sock_add, socklen_t sock_len);
int write_to_udp_socket(int fd,char* buffer, int length,struct sockaddr *remote_sock_add, socklen_t sock_len );

int caculate_current_data_byte_len( void );
int get_bytes_time(int bytes , char Baud);



#endif
