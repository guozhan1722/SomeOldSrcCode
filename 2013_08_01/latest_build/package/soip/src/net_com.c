
#include "net_com.h"

#define LOGOPTS (LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)  

#define sleep_to_try_late() usleep(1)
#define read_loop_condition  (bytes_left>0)

#define NUM_PER_LINE 20
/**********************************************************************************
   Function:  int my_read_from_serial_port(int fd,void* buffer, int length)
   Input:     int fd,void* buffer, int length
   Output:    None
   Return:    -1 if error, 0 if success
   Note:     read until finish or return -1 means unacceptable error
   this functions is used for open radio in radio setting and com ports
   dont use syslog especially at high speed
**********************************************************************************/
extern unsigned int current_time;
extern  unsigned int link_timeout_begin;
extern  unsigned int incoming_timeout,outgoing_timeout;
extern int remote_as_client_sockfd,local_as_client_sockfd;
extern struct sysinfo info;

extern int local_as_udp_server;

extern int local_as_udp_server,local_as_mclient_sockfd;
extern struct sockaddr_in remote_as_server_addr;
extern struct sockaddr_in remote_monitor_addr;
extern unsigned int last_socket_data_time_s;
extern unsigned int last_socket_data_time_us;

extern bool isCommunicating;
extern int Before_Data_DTR_Delay_us;
extern int After_Data_DTR_Delay_us;
/*---------------------------------------------------------------------------*/
extern bool isModbusEnabled();
int read_serial(int fd_, void * buf_, int size_)
{
    extern unsigned int Timing_out_frame_delay;
    char * writePtr;
    int bytesRead;
    struct timeval frameDelay;
    fd_set fds;
    int rt;

    if ( !isModbusEnabled() ) {
        return read( fd_, buf_, size_ );
    }

    writePtr = (char *)buf_;
    bytesRead = 0;

    while ( bytesRead < size_ ) {
        frameDelay.tv_sec = 0;
        frameDelay.tv_usec = ( Timing_out_frame_delay > 1000 ) ?
                             Timing_out_frame_delay : 1000;
        FD_ZERO( &fds );
        FD_SET( fd_, &fds );

        rt = select( fd_ + 1, &fds, NULL, NULL, &frameDelay );
        if ( rt < 0 ) {
            syslog( LOGOPTS, "Failed to select in read_serial [%d,%d]\n", rt, errno );
            return rt;
        }

        if ( FD_ISSET( fd_, &fds ) ) {
            rt = read( fd_, writePtr + bytesRead, size_ - bytesRead );
            if ( rt < 0 ) {
                syslog( LOGOPTS, "Failed to read in read_serial [%d,%d]\n", rt, errno );
                return rt;
            }

            bytesRead += rt;
        } else {
            /* A frame timeout occurred. Break out of the loop */
            break;
        }
    }

    return bytesRead;
}

/*---------------------------------------------------------------------------*/
int read_from_serial_port(int fd, char *buffer, int length)
{
    int bytes_left;
    int bytes_read;
    char *ptr;
    char* buff_ex=NULL;
    int len_ex=0;
    int k,l;
    ptr=buffer;
    bytes_left=length;
    DBG_log("ENTER [%s] \n", __func__);

    do {
        bytes_read=read_serial(fd,ptr,bytes_left);
        if (bytes_read>0) {
            ptr[bytes_read]=0;
        }

        if (bytes_read<0) {
            if (errno==EINTR) {
                //syslog(LOGOPTS,"my_read_from_com error EINTR number=%d:%s\n",errno,strerror(errno));

                bytes_read=0;
            } else
                if (errno==EWOULDBLOCK) {
                //syslog(LOGOPTS,"my_read_from_com EWOULDBLOCK error number=%d:%s\n",errno,strerror(errno));
                bytes_read=0;
                //sleep_to_try_late();
                break;
            } else {
                //syslog(LOGOPTS,"my_read_from_com  error number=%d:%s\n",errno,strerror(errno));
                return -1;
            }
        } else if (bytes_read==0) {// no data from client          
            break;
        }

        bytes_left-=bytes_read;
        ptr+=bytes_read;
    }while (read_loop_condition);

// //syslog(LOGOPTS,"my_read from com number=%d\n",length-bytes_left);
    DBG_log("LEAVE [%s] read from com number=%d bytes\n", __func__, length-bytes_left);
    return(length-bytes_left);

}
#define MY_FC  1
/**********************************************************************************
   Function:  int my_write_to_com(int fd,void* buffer, int length)
   Input:     int fd,void* buffer, int length
   Output:    None
   Return:    -1 if error, 0 if success
   Note:     write until finish or return -1 means unacceptable error
**********************************************************************************/

int write_to_serial_port(int fd,char* buffer, int length)
{

    DBG_log("ENTER [%s] \n", __func__);

#if defined(IXP425COM1) || defined(MPCI) 
    int status;
    struct timeval tv;
    struct timezone tz;
    unsigned int us_begin=0, s_begin=0, us_elaped_time=0;
    unsigned int us_send_bytes_delay;
    int Before_Data_Delay_us,After_Data_Delay_us;
#endif

#if MY_FC
#if defined(VIP4G) 
    int status;
    struct timeval tv;
    struct timezone tz;
    unsigned int us_begin=0, s_begin=0, us_elaped_time=0;
    unsigned int us_send_bytes_delay;
    int Before_Data_Delay_us,After_Data_Delay_us;
#endif
#endif  

    int bytes_left;
    int written_bytes;
    char *ptr;
    char* buff_ex=NULL;
    int len_ex=0;
    int k,l;

    if (length<=0) {
        return 0;
    }
    /*if com1 then, put CTS to high before send data  */
#if defined(IXP425COM1) || defined (MPCI)
    if ('C'==DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_FLOW_CONTROL][0]) {
        Before_Data_Delay_us = atoi(DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_CTS_BEFORE_FRAME_DELAY]) ;
        After_Data_Delay_us = atoi(DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_CTS_AFTER_FRAME_DELAY]) ;   
        if (Before_Data_Delay_us<0) {
            Before_Data_Delay_us=0;
        } else {
            Before_Data_Delay_us*=1000;
        }
        if (After_Data_Delay_us<0) {
            After_Data_Delay_us=0;
        } else {
            After_Data_Delay_us*=1000; 
        }

        us_elaped_time=0;

        if (0==gettimeofday (&tv, &tz)) { /*get starttime again*/
            s_begin=tv.tv_sec;
            us_begin=tv.tv_usec;
        } else {
            syslog(LOGOPTS,"write_to_serial_port 0 -1==gettimeofday\n");              
            return -1;
        }

        ioctl(fd,TIOCMGET, &status);
        status |= TIOCM_RTS ;
        ioctl(fd, TIOCMSET,&status);

        while (us_elaped_time<Before_Data_Delay_us) {
            if (-1==gettimeofday (&tv, &tz)) {
                syslog(LOGOPTS,"write_to_serial_port 1 -1==gettimeofday\n");              
                return -1;
            }
            us_elaped_time= (tv.tv_sec-s_begin)*1000000 + tv.tv_usec - us_begin ;
        }
    } else {
    }

#endif

#if MY_FC 
    /*put CTS to high before send data  */
#if defined(VIP4G)
    //DBG_log("[%s] FLOW CONTROL: %c\n", __func__, DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_FLOW_CONTROL][0]);
    if ('C'==DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_FLOW_CONTROL][0]) {
        syslog(LOGOPTS,"[%s] put CTS to high before send data.\n", __func__); 
        Before_Data_Delay_us = atoi(DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_CTS_BEFORE_FRAME_DELAY]) ;
        After_Data_Delay_us  = atoi(DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_CTS_AFTER_FRAME_DELAY]) ;   
        if (Before_Data_Delay_us<0) {
            Before_Data_Delay_us=0;
        } else {
            Before_Data_Delay_us*=1000;
        }
        if (After_Data_Delay_us<0) {
            After_Data_Delay_us=0;
        } else {
            After_Data_Delay_us*=1000; 
        }

        us_elaped_time=0;

        if (0==gettimeofday (&tv, &tz)) { /*get starttime again*/
            s_begin=tv.tv_sec;
            us_begin=tv.tv_usec;
        } else {
            syslog(LOGOPTS,"write_to_serial_port 0 -1==gettimeofday\n");              
            return -1;
        }

        ioctl(fd,TIOCMGET, &status);
        syslog(LOGOPTS,"[%s] Before status=%d\n", __func__, status); 
        status |= TIOCM_RTS ;
        syslog(LOGOPTS,"[%s] After status=%d\n", __func__, status);
        ioctl(fd, TIOCMSET,&status);

        while (us_elaped_time<Before_Data_Delay_us) {
            if (-1==gettimeofday (&tv, &tz)) {
                syslog(LOGOPTS,"write_to_serial_port 1 -1==gettimeofday\n");              
                return -1;
            }
            us_elaped_time= (tv.tv_sec-s_begin)*1000000 + tv.tv_usec - us_begin ;
        }
    } else {
    }

#endif  
#endif

    /*if com1 then, Enable DTR before send data  */
#ifdef IXP425COM1
    if ('D'==DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_FLOW_CONTROL][0]) {
        if (isCommunicating==false) {/*toggle pin to high and send the data after delay*/
            ioctl(fd,TIOCMGET, &status);
            status |= TIOCM_DTR ;
            ioctl(fd, TIOCMSET,&status);
            //syslog(LOGOPTS,"eNABLE DSR last_socket_data_time_s=%lu last_socket_data_time_us=%lu\n",last_socket_data_time_s, last_socket_data_time_us);
            us_elaped_time=0;
            if (0==gettimeofday (&tv, &tz)) { /*get starttime again*/
                s_begin=tv.tv_sec;
                us_begin=tv.tv_usec;
            } else {
                syslog(LOGOPTS,"write_to_serial_port 0 -1==gettimeofday\n");
                return -1;
            }


            while (us_elaped_time<Before_Data_DTR_Delay_us) {
                if (-1==gettimeofday (&tv, &tz)) {
                    syslog(LOGOPTS,"write_to_serial_port 1 -1==gettimeofday\n");
                    return -1;
                }
                us_elaped_time= (tv.tv_sec-s_begin)*1000000 + tv.tv_usec - us_begin ;
                usleep(1);
            }
        } else {/*is communicating, do nothing*/
        }

        if (0==gettimeofday (&tv, &tz)) { /*get starttime again*/
            //syslog(LOGOPTS,"last_socket_data_time_s=%lu last_socket_data_time_us=%lu\n",last_socket_data_time_s, last_socket_data_time_us);
            last_socket_data_time_s=tv.tv_sec;
            last_socket_data_time_us=tv.tv_usec;
            isCommunicating=true;
        } else {
            syslog(LOGOPTS,"write_to_serial_port 0 -1==gettimeofday\n");
            return -1;
        }
    } else {
    }

#endif
    ptr=buffer;
    bytes_left=length;
    DBG_log("[%s] %d bytes will write to com \n", __func__, length);

    while (bytes_left>0) {
        //begin to write
        written_bytes=write(fd,ptr,bytes_left);
        DBG_log("[%s] write() function return %d\n", __func__, written_bytes);

        if (written_bytes==-1) {//there is a error
            if (errno==EINTR) {//interrup error go on write
                syslog(LOGOPTS,"[%s] my_write to com EINTR error number=%d:%s\n",__func__,errno,strerror(errno)); 
                written_bytes=0;
            } else if (errno==ENOSPC) {//no space left on device
                syslog(LOGOPTS,"[%s] my_write to com  error number=%d:%s\n",__func__,errno,strerror(errno));
                written_bytes=0;
                sleep_to_try_late(   );                
            }

            else if (errno==EWOULDBLOCK) {
                syslog(LOGOPTS,"[%s] my_write errno==EWOULDBLOCK error number=%d:%s\n",__func__,errno,strerror(errno));
                sleep_to_try_late();
                written_bytes=0;
            } else if (errno==EPIPE) {
                syslog(LOGOPTS,"[%s] my_write errno==EPIPE error number=%d:%s\n", __func__, errno, strerror(errno));
                sleep_to_try_late();
                written_bytes=0;
            } else {  //other mistake , exit
                syslog(LOGOPTS,"[%s] my_write to com error number=%d:%s\n",__func__,errno,strerror(errno));
                return -1;
            }
        }

        if (written_bytes!=bytes_left) {
            syslog(LOGOPTS,"[%s] my_write to com bytes_left=%d,written_bytes=%d\n",__func__,bytes_left,written_bytes);
        }
        bytes_left-=written_bytes;
        ptr+=written_bytes;
    }
//	fsync(fd);
    //syslog(LOGOPTS,"my_write to com1 number=%d\n",length);
    /*if com1 then, put CTS to low after send data  */
#if defined(IXP425COM1) || defined(MPCI) 
    if ('C'==DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_FLOW_CONTROL][0]) {
        us_elaped_time=0;
        us_send_bytes_delay = get_bytes_time(length, DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);
        us_send_bytes_delay=us_send_bytes_delay+After_Data_Delay_us+Before_Data_Delay_us;
        while (us_elaped_time < us_send_bytes_delay) {
            if (-1==gettimeofday (&tv, &tz)) {  /*get starttime again*/
                syslog(LOGOPTS,"write_to_serial_port 2 -1==gettimeofday\n");
                return -1;
            }

            us_elaped_time= (tv.tv_sec-s_begin)*1000000 + tv.tv_usec - us_begin ;       
        }

        ioctl(fd,TIOCMGET, &status);
        status &=~TIOCM_RTS ;
        ioctl(fd, TIOCMSET,&status);
    } else {
    }
#endif 
#if MY_FC
    /* put CTS to low after send data  */
#if defined(VIP4G) 
    if ('C'==DataBase1_Buff[SUB_MENU_COM2][COM2_ITEM_FLOW_CONTROL][0]) {
        syslog(LOGOPTS,"[%s] put CTS to low after send data.\n", __func__);  
        us_elaped_time=0;
        us_send_bytes_delay = get_bytes_time(length, DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);
        us_send_bytes_delay=us_send_bytes_delay+After_Data_Delay_us+Before_Data_Delay_us;
        while (us_elaped_time < us_send_bytes_delay) {
            if (-1==gettimeofday (&tv, &tz)) {  /*get starttime again*/
                syslog(LOGOPTS,"write_to_serial_port 2 -1==gettimeofday\n");
                return -1;
            }

            us_elaped_time= (tv.tv_sec-s_begin)*1000000 + tv.tv_usec - us_begin ;       
        }

        ioctl(fd,TIOCMGET, &status);
        status &=~TIOCM_RTS ;
        ioctl(fd, TIOCMSET,&status);
    } else {
    }
#endif
#endif

    DBG_log("LEAVE [%s] \n", __func__);
    return 0;
}



/**********************************************************************************
   Function:
   Input:
   Output:
   Return: -1, error, 0, or more
   Note:
**********************************************************************************/

int read_from_tcp_socket(int fd, char *buffer, int length)
{
    int bytes_left;
    int bytes_read;
    char *ptr;
    ptr=buffer;
    bytes_left=length;

    DBG_log("ENTER [%s]\n", __func__);

    do {
        bytes_read=recv(fd,ptr,bytes_left,0);

        if (bytes_read<0) {
            if (errno==EINTR) {
                //syslog(LOGOPTS,"my_read_from_tcp_socket error EINTR number=%d:%s\n",errno,strerror(errno));

                bytes_read=0;
            } else
                if (errno==EWOULDBLOCK) {
                //syslog(LOGOPTS,"my_read_from_tcp_socket EWOULDBLOCK error number=%d:%s\n",errno,strerror(errno));
                bytes_read=0;
                //sleep_to_try_late();
                break;
            } else {
                //syslog(LOGOPTS,"my_read_from_tcp_socket  error number=%d:%s\n",errno,strerror(errno));
                return(-1);
            }
        } else if (bytes_read==0)// no data from client
            break;

        bytes_left-=bytes_read;
        ptr+=bytes_read;
    }while (read_loop_condition);
//	 //syslog(LOGOPTS,"my_read from socket number=%d\n",length-bytes_left);
    DBG_log("[%s] read %d bytes from socket\n", __func__, length-bytes_left);
    return(length-bytes_left);

}


/**********************************************************************************
   Function:  int my_write_to_socket(int fd,void* buffer, int length)
   Input:     int fd,void* buffer, int length
   Output:    None
   Return:    -1 if error, 0 if success
   Note:     write until finish or return -1 means unacceptable error
**********************************************************************************/

int write_to_tcp_socket(int fd,char* buffer, int length)
{
    int bytes_left;
    int written_bytes;
    char *ptr;
    DBG_log("ENTER [%s] %d Bytes data will write to tcp socket\n", __func__, length);

    if (length<=0) {
        return 0;
    }
    int errorcount=0;

    ptr=buffer;
    bytes_left=length;
    while (bytes_left>0) {
        //begin to write
        written_bytes=send(fd,ptr,bytes_left,0);
        DBG_log("[%s] send func return %d, errno=%d\n", __func__, written_bytes, errno);

        if (written_bytes==-1) {
            if (fd==remote_as_client_sockfd) {
                DBG_log("[%s] remote_as_client_sockfd %d\n", __func__, written_bytes);
                if (sysinfo(&info)==0)
                    current_time=(int) info.uptime -link_timeout_begin;

                if (current_time > incoming_timeout) {
                    shutdown(fd,SHUT_RDWR);
                    close(fd); 
                    current_time = 0;                  
                    return -1;
                }
            } else if (fd==local_as_client_sockfd) {
                DBG_log("[%s] local_as_client_sockfd %d\n", __func__, written_bytes);
                if (sysinfo(&info)==0)
                    current_time=(int) info.uptime -link_timeout_begin;

                if (current_time > outgoing_timeout) {
                    DBG_log("[%s] current_time %d > outgoing_timeout %d\n", __func__, current_time, outgoing_timeout);
                    shutdown(fd,SHUT_RDWR);
                    close(fd);
                    current_time = 0;
                    return -1;
                }
            }

            //there is a error
            errorcount++;
            if (errno==EINTR) {//interrup error go on write
                //syslog(LOGOPTS,"my_write_to_tcp_socket EINTR error number=%d:%s\n",errno,strerror(errno)); 
                written_bytes=0;
            } else if (errno==ENOSPC) {//no space left on device
                //syslog(LOGOPTS,"my_write_to_tcp_socket 28 error number=%d:%s\n",errno,strerror(errno));
                sleep_to_try_late();
                written_bytes=0;
            } else if (errno==EAGAIN) {//no space left on device
                //syslog(LOGOPTS,"my_write_to_tcp_socket EAGAIN error number=%d:%s\n",errno,strerror(errno));
                sleep_to_try_late();
                written_bytes=0;
            }

            else if (errno==EWOULDBLOCK) {
                //syslog(LOGOPTS,"my_write_to_tcp_socket errno==EWOULDBLOCK error number=%d:%s\n",errno,strerror(errno));
                sleep_to_try_late();             
                written_bytes=0;
            }
            // else if (errno==EPIPE)
            // 	{
            // 	syslog(LOGOPTS,"[%s] my_write_to_tcp_socket errno==EPIPE error number=%d:%s\n", __func__, errno, strerror(errno));
            // 	sleep_to_try_late();
            // 	written_bytes=0;
            // 	}
            else {  //other mistake , exit
                DBG_log("[%s] my_write_to_tcp_socket error number=%d:%s\n", __func__, errno,strerror(errno));
                return(-1);
            }
        } else {
            errorcount=0;
        }

        if (errorcount>100) {
            syslog(LOGOPTS,"my_write_to_tcp_socket error number=-1 error count=%d\n",errorcount);
            return(-1);
        }

        bytes_left-=written_bytes;
        ptr+=written_bytes;
    }

//	//syslog(LOGOPTS,"my_write_to_tcp_socket number=%d\n",length);
    DBG_log("LEAVE [%s], %d Bytes data left\n", __func__, bytes_left);
    return(0);
}



/**********************************************************************************
   Function:
   Input:
   Output:
   Return: -1, error, 0, or more
   Note:
**********************************************************************************/

int read_from_udp_socket(int fd, char *buffer, int length)//,struct sockaddr *remote_sock_add, socklen_t sock_len)
{
    int bytes_left;
    int bytes_read;
    char *ptr;
    ptr=buffer;
    bytes_left=length;
    struct sockaddr_in si_other;
    int slen=sizeof(si_other);
    DBG_log("ENTER [%s]\n", __func__);
    //syslog(LOGOPTS,"my_read_from_udp_socket begin \n");
    do {
        if (local_as_udp_server) {
            bytes_read=recvfrom(fd, ptr, bytes_left, 0, &si_other, &slen);
            memcpy(&(remote_as_server_addr.sin_addr.s_addr),&si_other.sin_addr,sizeof(struct in_addr));
            remote_as_server_addr.sin_port = si_other.sin_port;
        } else {
            bytes_read=read(fd,ptr,bytes_left);//,0,remote_sock_add,sock_len);
        }
        if (bytes_read<0) {
            if (errno==EINTR) {
                //syslog(LOGOPTS,"my_read_from_udp_socket error EINTR number=%d:%s\n",errno,strerror(errno));

                bytes_read=0;
            } else
                if (errno==EWOULDBLOCK) {
                //syslog(LOGOPTS,"my_read EWOULDBLOCK error number=%d:%s\n",errno,strerror(errno));
                bytes_read=0;
                //sleep_to_try_late();                
                break;
            } else {
                //syslog(LOGOPTS,"my_read_from_udp_socket error number=%d:%s\n",errno,strerror(errno));
                return(-1);
            }
        } else if (bytes_read==0)// no data from client
            break;

        bytes_left-=bytes_read;
        ptr+=bytes_read;
    }while (read_loop_condition);
//	 //syslog(LOGOPTS,"my_read from socket number=%d\n",length-bytes_left);
    return(length-bytes_left);

}


/**********************************************************************************
   Function:  int my_write_to_socket(int fd,void* buffer, int length)
   Input:     int fd,void* buffer, int length
   Output:    None
   Return:    -1 if error, 0 if success
   Note:     write until finish or return -1 means unacceptable error
**********************************************************************************/

int write_to_udp_socket(int fd,char* buffer, int length,struct sockaddr *remote_sock_add, socklen_t sock_len )
{
    int bytes_left;
    int written_bytes;
    char *ptr;
    DBG_log("ENTER [%s]\n", __func__);

    if (length<=0) {
        return 0;
    }

    ptr=buffer;
    bytes_left=length;
    while (bytes_left>0) {
        //begin to write
        written_bytes=sendto(fd,ptr,bytes_left,0, remote_sock_add, sock_len);
        if (written_bytes==-1) {//there is a error
            if (errno==EINTR) {//interrup error go on write
                //syslog(LOGOPTS,"my_write_to_udp_socket EINTR error number=%d:%s\n",errno,strerror(errno)); 
                written_bytes=0;
            } else if (errno==ENOSPC) {//no space left on device
                //syslog(LOGOPTS,"my_write_to_udp_socket 28 error number=%d:%s\n",errno,strerror(errno));
                written_bytes=0;
                sleep_to_try_late();
            } else if (errno==EAGAIN) {
                //syslog(LOGOPTS,"my_write_to_udp_socket EAGAIN error number=%d:%s\n",errno,strerror(errno));
                written_bytes=0;
                sleep_to_try_late();               
            }

            else if (errno==EWOULDBLOCK) {
                //syslog(LOGOPTS,"my_write_to_udp_socket errno==EWOULDBLOCK error number=%d:%s\n",errno,strerror(errno));
                written_bytes=0;
                sleep_to_try_late();
            } else {  //other mistake , exit
                //syslog(LOGOPTS,"my_write_to_udp_socket error number=%d:%s\n",errno,strerror(errno));
                return(-1);
            }
        }
        if (written_bytes!=bytes_left) {
            ////syslog(LOGOPTS,"my_write to socket bytes_left=%d,written_bytes=%d\n",bytes_left,written_bytes); 
        }

        bytes_left-=written_bytes;
        ptr+=written_bytes;
    }

    //syslog(LOGOPTS,"my_write_to_udp_socket number=%d\n",length);     
    return(0);
}

#if defined(IXP425COM1)|| defined(MPCI) || defined(VIP4G)

/**********************************************************************************
 Function:  int get_bytes_time(int bytes , char* Baud, char* Format );

 Input:     (char *MODBUS_out_frame_delay, char Baud )   
 Output:    None    
 Return:    int   
 Note:      this function will turn the string charactor delay time to us time according
            to different baud rate.   
**********************************************************************************/
int get_bytes_time(int bytes , char Baud)
{
    int com_speed[18]={3,6,12,24,36,48,72,96,144,192,288,384,576,1152,2304,4608,9216};
    int return_delay,Baudrate;
    double time;
    int byte_len;
    int i;   
    byte_len=caculate_current_data_byte_len(); 

    i =  Baud-'A';
    if ((i>=0)&&(i<15))
        Baudrate = com_speed[i];
    else
        Baudrate = 96;
    /* //syslog(LOGOPTS,"Baudrate %d\n",Baudrate);*/
    if (bytes >1000)
        time = bytes*1000*byte_len;
    else
        time = bytes*10000*byte_len; 

    return_delay = time/Baudrate;
    if (bytes >1000)
        return_delay = return_delay*10;

    return(return_delay);  /* return us*/
}

#endif

/**********************************************************************************
   Function:    static int caculate_current_data_byte_len( void ) 
   Input:     (void)  
   Output:    None    
   Return:    int   
   Note:      this function will turn the string charactor delay time to us time according
              to different baud rate.   
**********************************************************************************/

int caculate_current_data_byte_len( void )
{
    int  byte_len;
#if 0
    if (0==strncmp("8N1", Menu_Item_Options[SUB_MENU_COM1][COM12_ITEM_DATA_FORMAT][DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][0]-'A'],3))
        byte_len = 10;
    else if (0==strncmp("8N2", Menu_Item_Options[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][0]-'A'],3))
        byte_len = 11;
    else if (0==strncmp("8E1", Menu_Item_Options[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][0]-'A'],3))
        byte_len = 11;
    else if (0==strncmp("8O2", Menu_Item_Options[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][0]-'A'],3))
        byte_len = 12;
    else if (0==strncmp("7N1", Menu_Item_Options[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][0]-'A'],3))
        byte_len = 9;
    else if (0==strncmp("7E1", Menu_Item_Options[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][0]-'A'],3))
        byte_len = 10;
    else if (0==strncmp("7O1", Menu_Item_Options[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][0]-'A'],3))
        byte_len = 10;
    else if (0==strncmp("7E2", Menu_Item_Options[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][0]-'A'],3))
        byte_len = 10;
    else if (0==strncmp("7O2", Menu_Item_Options[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_FORMAT][0]-'A'],3))
        byte_len = 10;
    else
        byte_len = 11;
#else


    if (DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A' == 0) {
        byte_len = 10;
    } else if (DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A' == 1) {
        byte_len = 11;
    } else if (DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A' == 2) {
        byte_len = 11;
    } else if (DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A' == 3) {
        byte_len = 12;
    } else if (DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A' == 4) {
        byte_len = 9;
    } else if (DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A' == 5) {
        byte_len = 10;
    } else if (DataBase1_Buff[SUB_MENU_COM2][COM1_ITEM_DATA_FORMAT][0]-'A' == 6) {
        byte_len = 10;
    } else if (DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A' == 7) {
        byte_len = 10;
    } else if (DataBase1_Buff[SUB_MENU_COM1][COM1_ITEM_DATA_FORMAT][0]-'A' == 8) {
        byte_len = 10;
    } else {
        byte_len = 11;
    }
#endif
    return byte_len;   
}


