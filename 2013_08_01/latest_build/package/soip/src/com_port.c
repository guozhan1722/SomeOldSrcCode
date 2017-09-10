

#include "com_port.h"
#include <syslog.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#if defined(MPCI) || defined(MPCI2)
    #define COM1 "/dev/ttyUSB0"
    #define COM2 "/dev/ttyS0"
#else
    #include "vip-bsp.h"
    #define COM1 "/dev/ttyS0"
    #define COM2 "/dev/ttyS1"
#endif

#define DEVICE_NAME "/dev/vip-bsp"

#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)   

/*******************************************************************************************************/
/*
void SP_Init(void)
Function: Set speed to 9600, Data bits 8, stop bits 1, Parity None
Input Parameter: None
Output Parameter: None   */

/*******************************************************************************************************/

int COM_Set_Parameters(char dev )
{
    /*************************************Open com speed with  ************************8********/
    int com_speed[18]={300,600,1200,2400,3600,4800,7200,9600,14400,19200,28800,38400,57600,115200,230400,460800,921600};
    char com_parity[15][4]={"8N1","8N2","8E1","8O1","7N1","7N2","7E1","7O1","7E2","7O2"}; //12
    int fd,device_fd;
    int status;
    //printf("%s:set parameters\n",__func__);
    DBG_log("Enter [%s]\n", __func__);
    sleep(5);

    if (dev==1) {    //COM1
        /* Open the BSP Device */
        //printf("%s:open COM1\n");
        //sleep(5);

        fd = OpenDev(COM1);//open com1

        if (fd<0) {
            syslog(LOGOPTS,"open com Error %s\n",strerror(errno));  
            return -1;
        }


        //syslog(LOGOPTS,"open com set ok com1 fd= %d\n",fd);
        Set_Speed(fd,com_speed[DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_BAUD_RATE][0]-'A']);   

        if ('C'==DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_FLOW_CONTROL][0]) {
            if (-1==Set_DataFrame(fd,com_parity[DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A'][0]-'0',
                                  com_parity[DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A'][2]-'0',\
                                  com_parity[DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A'][1],0 )) {//set Databit:8,stop bit:1,parity:FLOW CONTROL
                syslog(LOGOPTS,"Set Parity Error %s\n",strerror(errno));  
                return -1;       
            } else {
                ioctl(fd,TIOCMGET, &status);
                status &=~TIOCM_RTS ;
                ioctl(fd, TIOCMSET,&status);

                return fd;
            }

        } else {
            if (-1==Set_DataFrame(fd,com_parity[DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A'][0]-'0',
                                  com_parity[DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A'][2]-'0',\
                                  com_parity[DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A'][1],
                                  DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_FLOW_CONTROL][0]-'A')) {//set Databit:8,stop bit:1,parity:FLOW CONTROL
                syslog(LOGOPTS,"Set Parity Error2 %s\n",strerror(errno));  
                return -1;
            } else {
                return fd;
            }


        }   
    } else {

        chmod(COM2,0600);       

        fd = OpenDev(COM2);//open com2

        //syslog(LOGOPTS,"open com  com2 fd= %d\n",fd);

        if (fd<0) {
            syslog(LOGOPTS,"open com2 Error %s\n",strerror(errno));  
            return -1;
        }
        Set_Speed(fd,com_speed[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_BAUD_RATE][0]-'A']);

#ifdef VIP4G
        //DBG_log("[%s] COM_Set_Parameters: speed: %d\n", __func__, com_speed[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_BAUD_RATE][0]-'A']);
        //DBG_log("[%s] COM_Set_Parameters: databits: %d\n",  __func__, com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][0]-'0' );
        //DBG_log("[%s] COM_Set_Parameters: parity: %c\n", __func__, com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][1] );
        //DBG_log("[%s] COM_Set_Parameters: stop bit: %d\n", __func__, com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][2]-'0' );
        //DBG_log("[%s] COM_Set_Parameters: FLOW CONTROL: %c\n", __func__, DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_FLOW_CONTROL][0]);


        if ( 'C' == DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_FLOW_CONTROL][0] ) {

            if (-1==Set_DataFrame(fd,com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][0]-'0',
                                  com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][2]-'0',\
                                  com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][1],0 )) {//set Databit:8,stop bit:1,parity:FLOW CONTROL
                syslog(LOGOPTS,"Set Parity Error %s\n",strerror(errno));  
                return -1;       
            } else {
                ioctl(fd,TIOCMGET, &status);
                status &=~TIOCM_RTS ;
                ioctl(fd, TIOCMSET,&status);

                return fd;
            }
        } else {

            if (-1==Set_DataFrame(fd,com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][0]-'0',
                                  com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][2]-'0',\
                                  com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][1],
                                  DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_FLOW_CONTROL][0]-'A')) {//set Databit:8,stop bit:1,parity:FLOW CONTROL
                syslog(LOGOPTS,"Set Parity Error2 %s\n",strerror(errno));  
                return -1;
            } else {
                return fd;
            }
        }
#else

        if (Set_DataFrame(fd,com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][0]-'0',
                          com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][2]-'0',\
                          com_parity[DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_DATA_FORMAT][0]-'A'][1],0) == -1) {//set Databit:8,stop bit:1,parity:none
            syslog(LOGOPTS,"Set com2 Parity Error %s\n",strerror(errno));  
            return -1; 
        } else {
            return fd;
        }

#endif

        return fd;
    }

}
/***********************************************************************************************/





