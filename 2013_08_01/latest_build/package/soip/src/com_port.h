

#ifndef SERIAL_COM1_H
#define SERIAL_COM1_H

#include <stdio.h>
#include <string.h>
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

#include "serialport_driver.h"
#include "database.h"
#include "soip.h"

 


/*******************************************************************************************************/
void SP_Init(void);
int COM_Set_Parameters(char dev);



/*************************************************************************************************/

#endif
