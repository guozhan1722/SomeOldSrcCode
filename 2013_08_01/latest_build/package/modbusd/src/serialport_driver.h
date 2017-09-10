
#ifndef SERIALPORT_DRIVER_H
#define SERIALPORT_DRIVER_H

#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif


#include <stdlib.h>
#include <unistd.h> 
#include <sys/stat.h>
#include <termios.h>  
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
// #include <sys/ioctl.h>
#include <sys/wait.h>


#include <fcntl.h>
void Set_Speed(int fd, int speed);//set serial port communication speed
int Set_DataFrame(int fd,int databits,int stopbits,char parity,int flowcontrol);//set serial port communication Date frame
int OpenDev(char *Dev);// open serial port
int CloseDev(int fd);//close serial port

#endif
