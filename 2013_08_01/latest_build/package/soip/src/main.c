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

#include "soip.h"
#include "com_port.h"
#include "net_com.h"
#include "gnokii.h"

#define SOIPD1_SUPPORT_THREAD 0

int debug = 0;

#define UDP_TIMEOUT 10
#if defined(IXP425COM1)
char *devCom  = "/dev/ttyS0"; //com1  
char *COM12_statistic_file ="/var/run/com1_packet";
#elif  defined(IXP425COM2) || defined(MPCI2)
char *devCom  = "/dev/ttyS1"; //com2   
char *COM12_statistic_file ="/var/run/com2_packet";
char *COM2Script ="/bin/UIscript";
char *COM2Script_exe="soipd2_stop";
static bool thread_return = false;
#elif defined(MPCI)
char *devCom  = "/dev/ttyUSB0"; //com1  
char *COM12_statistic_file ="/var/run/com1_packet";
#elif  defined(IXP425USB)
char *devCom  = "/dev/ttyGS0"; //USB   
char *COM12_statistic_file ="/var/run/usb_packet";
char *USBScript ="/bin/UIscript";
char *USBScript_exe="soipdusb_stop";
static bool thread_return = false;
#else
char *devCom  = "/dev/ttyS1"; //com1  
char *COM12_statistic_file ="/var/run/com1_packet";
#endif

char *COMScript ="/bin/UIscript";

#define COM_READ_ONCE_BUFF_LEN 5120 //com buff 4096, so it is enough and it should not less than 4096
#define SOCKET_READ_ONCE_BUFF_LEN 5120       

//typedef unsigned int uint32_t ;
#ifndef max
    #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
char com_Data_total_str_buff[20][21]={"0","0","0","0","0","0","0","0","0","0",
    "0","0","0","0","0","0","0","0","0","B",};
static unsigned int  com_Data_total_int_buff[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

bool isCommunicating=false;
int Before_Data_DTR_Delay_us=0;
int After_Data_DTR_Delay_us=0;
unsigned int last_socket_data_time_s=0;
unsigned int last_socket_data_time_us=0;


char com_read_once_buff[COM_READ_ONCE_BUFF_LEN+1]={};
char socket_read_once_buff[SOCKET_READ_ONCE_BUFF_LEN+1]={};

char com_total_buff[COM_READ_ONCE_BUFF_LEN*2+2]={};
char socket_total_buff[SOCKET_READ_ONCE_BUFF_LEN*2+2]={};

int fdcom;
//  unsigned int SOCKET_READ_ONCE_BUFF_LEN=5120;// -1;//it can not be too small or will have problem(lock?)
//  unsigned int COM_READ_ONCE_BUFF_LEN= 5120;  //if too small receive will lost(lock)  

#ifdef DEBUG_SET_SOCKET_BUFF
int sndbuf=1048;//if too small have problem of through put
int rcvbuf=4096;
#endif
#ifdef DEBUG_PRINT_INPUT
int f_int1,f_int2; 
#endif

int com_read_once_number,socket_read_once_number;//-65535 to +65535, because it maybe negtive

#if defined(IXP425COM2) ||defined(MPCI2)  
static int plus_num=0; 
static int plus_begin_position=-1; 
static int plus0_position=-1;
static int plus1_position=-1;
static int plus2_position=-1;
#endif

#ifdef IXP425USB  
static int plus_num=0; 
static int plus_begin_position=-1; 
static int plus0_position=-1;
static int plus1_position=-1;
static int plus2_position=-1;
#endif

FILE *f;
bool Timing= false;
bool Seamless =false; 
bool Trasparent =false;

pthread_t pt_show_message;

char* ifname;
char buff_IP[32];
int first_Data_Come=0;
int sin_size;
int remote_server_port_int, local_listen_port_int,remote_monitor_port_int;
int local_as_server_sockfd, local_as_client_sockfd,remote_as_client_sockfd,local_as_mclient_sockfd;

int yesMonitor=false;
struct sockaddr_in remote_as_server_addr,local_as_server_addr;  
struct sockaddr_in remote_as_server2_addr;
struct sockaddr_in remote_as_server3_addr;
struct sockaddr_in remote_monitor_addr;
struct sockaddr remote_client_addr;   
int pending_connection_num=0;
fd_set my_readfd; 

int max_packet_size;/* the length of buffer for read, 1 -1024.*/
unsigned int Timing_out_frame_delay;     
unsigned int Timing_in_frame_delay;
unsigned int link_timeout_begin,current_time,incoming_timeout=0, outgoing_timeout=0,Defalt_Connection_Timeout=300;    
unsigned int com_time_pass_us=1, socket_time_pass_us=1;

/*  struct sockaddr_in saddr;*/
struct in_addr iaddr;
struct ip_mreq imreq;   
unsigned char ttl = 1;
unsigned char one = 1, not_one=0;    
int read_return,write_return,select_return;
int on=1, off=0;

static int socket_receive_num_for_send_total =0, com_receive_num_for_send_total =0;
struct timeval tv_100us,tv_delay;
struct timezone tz;
int socket_time_pass_us_test;

/*use this to count the time between com's and socket's frame and character timeout.*/
time_t read_com_data_start_sec=0, read_com_data_start_usec=0;
suseconds_t read_socket_data_start_sec=0, read_socket_data_start_usec=0;
/* int size_fd;*/
struct sysinfo info;
int receive_more_than_max=0;

int local_as_udp_server=0;


#define REMOTE_TCP_CLIENT_NUMBER 10

static int remote_tcp_client_list[REMOTE_TCP_CLIENT_NUMBER];
static int current_tcp_client_num=0;
static int high_tcp_socket=0;

static int current_polling_unit=-1;
static bool  current_polling_get_response=false;
static time_t current_polling_time_s=0;
static time_t current_polling_time_us=0;
static time_t polling_time_out=0;
/*
 for SMTP
 */
static char *cc_addr    = 0;
static char *err_addr   = 0;
static char *from_addr  = NULL;
static char *mailhost   = NULL;
static int   mailport   = 25;
static char *reply_addr = 0;
static char *subject    = 0;
static int   mime_style = 0;
static int   verbose    = 0;
static int   usesyslog  = 0;

static FILE *sfp;
static FILE *rfp;

#define dprintf  if (verbose) printf
#define dvprintf if (verbose) vprintf


static bool modbus_encryption_enable = true;
static uint32_t ModbusEncryptionKey[4]={0,1,2,3};


int optval;
socklen_t optlen = sizeof(optval);


/*END OF FOR SMTP*/

//for GPS:
#define DEFAULT_GPSD_SERVER "127.0.0.1"
char gps_port[10];
char gps_server[32];
int sock = 0;
ssize_t wrote = 0;
char cbuf[5];
static void gps_process(void);
static int gps_process_init(void);
//end for GPS

static void tcp_process(void);
static void udp_point_to_point_process(void);
static void udp_point_to_multipoint_as_point_process(void);
static void udp_point_to_multipoint_as_multipoint_process(void);
static void udp_multipoint_to_multipoint_process(void);
static void  smtp_process(void);
static void  new_smtp_process(void);
static void sms_process(void);
static int savesms_to_file(char *filename, char *str, int len);
static void sms_at_process(void);

static int data_from_com_process(int socket);
static int data_from_socket_process(int socket);
static int now_have_com_data_process(int socket);
static int now_have_socket_data_process(int socket);
static void get_response(void);
static void chat(char *fmt, ...);
static void toqp(FILE *infile, FILE *outfile);

static int write_to_com_process(int socket_);
static int _write_to_com_process(int socket_, const void * buf_, int size_);
static int write_to_socket_process(int socket_);
static int _write_to_socket_process(int socket_, const void * buf_, int size_);
static int set_select_readfd(int fd1,int fd2);

static unsigned int time_elapse(time_t old_sec, suseconds_t old_us);   
static void write_com_statistics(void);  

#if defined(IXP425COM2) || defined(MPCI2)
static void com2_check_three_plus(void);
void *thread_function(void *arg);
#endif     

#ifdef IXP425USB
static void usb_check_three_plus(void);
void *thread_function(void *arg);
#endif     


static unsigned int get_Timing_In_Frame_Delay( char Baud );
static unsigned int get_Timing_Out_Frame_Delay(char *Timing_out_frame_delay, char Baud );
static void exit_process(void);
static void fctl_com(void);
static int modbusEncrpt(char* inBuf_,int N_);
static int modbusDEcrpt(char* inBuf_,int N_);
static void btea(uint32_t *v, int n, uint32_t const k[4]);
static int ModbusStringKeytoIntKey(char* inStrKey_, int * KeyOut_);
static void *tcp_clients_thread_function(void *arg);
static int setInitialMultiTcpValue();
static int modbusDecrypt(char* inBuf_,int N_);
static void *disableDTRafterNoDataFromSocketTimeOut(void *);

char *RadioBusyFlagFile = "/var/run/VIP4G_STAT_busy";


/**********************************************************************************
   Helper Function
        
**********************************************************************************/

void LockFile(int fd)       
{                               
    struct flock lock;          
    memset (&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;      
    fcntl (fd, F_SETLKW, &lock);
}                               


void UnLockFile(int fd)         
{                           
    struct flock lock;
    lock.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &lock);
    close (fd);         
}   


/* get soip debug flag */
void get_soip_debug_flag()
{
    FILE *fp;
    char debug_flag_file[32];

    sprintf(debug_flag_file, "/etc/soipdebug");
    fp = fopen(debug_flag_file, "r");
    if (fp == NULL) {
        syslog(LOGOPTS, "[%s] debug_flag_file: %s does not exist\n", __func__, debug_flag_file);
        debug = 0;
    } else {
        debug = 1;
        fclose(fp);
    }
}


/************************************************************************** 
  Calculate frame delay and max packet size acccording to customer settings
  Called by main()
 
***************************************************************************/
void calculate_delay_and_size()
{
    DBG_log("ENTER [%s]\n", __func__ );   //

    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_MODE][FIRST_CHARACTOR]=='A')
    /* Seamless,then read database to configure its parameters*/
    {
#ifdef ENCOM
        Seamless = true;    
        Trasparent = false; 
        Timing = true;    
        Timing_out_frame_delay = get_Timing_Out_Frame_Delay(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_CHARACTER_TIMEOUT],\
                                                            DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);
        Timing_in_frame_delay = get_Timing_In_Frame_Delay(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);
        //syslog(LOGOPTS,"MODBUS mode \n");
        //syslog(LOGOPTS,"MODBUS_out_frame_delay:%d  \a\n",MODBUS_out_frame_delay);
        //syslog(LOGOPTS,"MODBUS_in_frame_delay:%d  \a\n",MODBUS_in_frame_delay);
        max_packet_size = atoi(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_MAXIMUM_PACKET_SIZE]);
        //syslog(LOGOPTS,"max_packet_size:%d \n",max_packet_size);
        if ((max_packet_size < 1)||(max_packet_size >2048))
            max_packet_size = 256;
        //syslog(LOGOPTS,"max_packet_size:%d  \a\n",max_packet_size);		

#else
        Seamless = true; 
        Trasparent = false; 
        Timing = true;        
        Timing_out_frame_delay = get_Timing_Out_Frame_Delay(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_CHARACTER_TIMEOUT],\
                                                            DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);
        Timing_in_frame_delay = get_Timing_In_Frame_Delay(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);
        Timing_in_frame_delay = 30000;        
        //syslog(LOGOPTS,"Timing mode \n");
        max_packet_size = atoi(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_MAXIMUM_PACKET_SIZE]);
        //syslog(LOGOPTS,"max_packet_size:%d \n",max_packet_size);
        if ((max_packet_size < 1)||(max_packet_size >2048))
            max_packet_size = 256;
#endif
    } else {  /* transparent, for encom, it is real transparent*/
#ifdef ENCOM
        Trasparent = true;
        Seamless = false; 
        Timing = false;
        //syslog(LOGOPTS,"Trasparent mode\n");
        Timing_out_frame_delay = 0;
        Timing_in_frame_delay = 0;
        max_packet_size    =    atoi(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_MAXIMUM_PACKET_SIZE]);
        /*'A' SHOULD ACCORDING TO DATABASE */
        if ((max_packet_size < 1)||(max_packet_size >1024))
            max_packet_size = 256;

#else
        Trasparent = true;
        Seamless = false; 
        Timing = true;
        //syslog(LOGOPTS,"Trasparent mode\n");
        Timing_out_frame_delay =  get_Timing_Out_Frame_Delay(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_CHARACTER_TIMEOUT],\
                                                             DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);

        Timing_in_frame_delay = 6665534;
        max_packet_size  = atoi(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_MAXIMUM_PACKET_SIZE]);
        /*'A' SHOULD ACCORDING TO DATABASE */
        if ((max_packet_size < 1)||(max_packet_size >2048))
            max_packet_size = 256;
#endif
    }

    DBG_log("[%s] Timing_out_frame_delay: %d \n", __func__, Timing_out_frame_delay);
    DBG_log("[%s] Timing_in_frame_delay: %d \n", __func__, Timing_in_frame_delay);
    DBG_log("[%s] COM12_ITEM_DATA_MODE: %c\n", __func__, DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_MODE][FIRST_CHARACTOR]);
    DBG_log("[%s] max_packet_size: %d \n", __func__, max_packet_size);
}



/************************************************************************** 
    Check if there is ip, if no wait unitl have.
    Called by main()
***************************************************************************/        
void check_ip_address (void)
{
    DBG_log("ENTER [%s]\n", __func__ );   //

    while (false==fetch_Local_IP_ADDR(ifname, buff_IP)) {
#if defined(IXP425COM2) || defined(MPCI2)
        FD_ZERO(&my_readfd);/*clear ,it should not be out side of while*/

        FD_SET(fdcom,&my_readfd);// set com
        tv_100us.tv_sec = 0;  
        tv_100us.tv_usec =100;
        select_return= select(fdcom+1,&my_readfd, NULL,NULL,&tv_100us);//
        if (select_return==-1) {
            syslog(LOGOPTS,"select error %s\n",strerror(errno));
            exit_process();
        }
        /* if the first data is from serial port, then local as tcp client , remote as tcp server */
        if (FD_ISSET(fdcom,&my_readfd)) {

            com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN);

            if (com_read_once_number>0) {
                com2_check_three_plus();              
            } else {
            }
        }
#endif        

#ifdef IXP425USB
        FD_ZERO(&my_readfd);/*clear ,it should not be out side of while*/

        FD_SET(fdcom,&my_readfd);// set com
        tv_100us.tv_sec = 0;  
        tv_100us.tv_usec =100;
        select_return= select(fdcom+1,&my_readfd, NULL,NULL,&tv_100us);//
        if (select_return==-1) {
            syslog(LOGOPTS,"select error %s\n",strerror(errno));
            exit_process();
        }
        /* if the first data is from serial port, then local as tcp client , remote as tcp server */
        if (FD_ISSET(fdcom,&my_readfd)) {
            com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN);

            if (com_read_once_number>0) {
                usb_check_three_plus();              
            } else {
            }
        }
#endif        

        sleep(1);
    } //end while
    DBG_log("LEAVE [%s]\n", __func__ );   //
} // end func



/**********************************************************************************
   Function:    int main(void) 
   Input:       void
   Output:      None
   Return:      int   
   Note:	read database and config com port and according to selected IP protocol 
        to process.        
**********************************************************************************/

int main(void)
{

    get_soip_debug_flag();

    DBG_log(" ======== ENTER [%s] ========\n", __func__);

    signal(SIGPIPE, SIG_IGN);

    setInitialMultiTcpValue();

    ctx = uci_alloc_context();
    if (!ctx) {
        fprintf(stderr, "Out of memory\n");
        return -1;
    }

    //  if (false==system("/bin/UIscript c1222_stop"))
    //         return false;

    (void) setsid();


    //if (false==SubProgram_Start(COMScript,"c1222_stop"))
    //    return false;

#ifdef DEBUG_PRINT_INPUT
    f_int1=open("/var/run/temp1",O_CREAT|O_TRUNC|O_RDWR| O_NOCTTY | O_NDELAY);  
    f_int2=open("/var/run/temp2",O_CREAT|O_TRUNC|O_RDWR| O_NOCTTY | O_NDELAY);  
#endif

    read_Database(SUB_MENU_COM12);
    read_Database(COM12_MODBUS_CONF);
    //read_Database(SUB_MENU_SYSTEM_CFG);
    //read_Database(SUB_MENU_NETWORK);

    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_PORT_STATUS][FIRST_CHARACTOR]=='A') {
        /* com is disabled */
        syslog(LOGOPTS,"disable com port\n");
        return;/*quit soip*/
    }

    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='H')
        read_Database(COM12_SUB_MENU_SMTP);

    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='J')
        read_Database(COM12_SUB_MENU_SMS);

    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='L')
        gps_process_init();

    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]<'D') {
        read_Database(COM12_SUB_MENU_TCLIENT);
        read_Database(COM12_SUB_MENU_TSERVER);
        read_Database(COM12_SUB_MENU_TCLIENTSERVER);
    } else {
        read_Database(COM12_SUB_MENU_UPTOP);
        read_Database(COM12_SUB_MENU_UPTOMP_AS_P);
        read_Database(COM12_SUB_MENU_UPTOMP_AS_MP);
        read_Database(COM12_SUB_MENU_UMPTOMP);         
    }

    //sleep(5);

    polling_time_out = atoi(DataBase1_Buff[COM12_SUB_MENU_TSERVER][TSERVER_ITEM_PULLING_TIMEOUT])*1000;
    modbus_encryption_enable = (DataBase1_Buff[COM12_MODBUS_CONF][COM12_MODBUS_ITEM_PROTECTION_STATUS][0]=='A')? false:true;
    //DBG_log("[%s] modbus_encryption_enable=%d\n",modbus_encryption_enable);
    ModbusStringKeytoIntKey(DataBase1_Buff[COM12_MODBUS_CONF][COM12_MODBUS_ITEM_PROTECTION_KEY], ModbusEncryptionKey);

    /* set com baudrate and parity and flow control.*/
    if ( (fdcom = COM_Set_Parameters(COM12)) == -1) {    /*notice this function return -1 */
        syslog(LOGOPTS,"COM_Set_Parameters ==-1 exit\n");
        exit_process();/* set first */
    }

    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='I' || DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='K') {
        close(fdcom);
        return;/*quit soip*/
        //call c1222ap application here;
    }

#ifdef IXP425COM2 
    (void)close(1);
    (void)close(2);
    (void)close(0);
#endif             

#ifdef MPCI2 
    (void)close(1);
    (void)close(2);
    (void)close(0);
#endif             

#ifdef IXP425USB 
    (void)close(1);
    (void)close(2);
    (void)close(0);
#endif             

    calculate_delay_and_size();

    // syslog(LOGOPTS,"Ip protocol:%s\n",DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG]);
    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_DATA_MODE][FIRST_CHARACTOR]=='A')
    /* Seamless,then read database to configure its parameters*/
    {
#ifdef ENCOM
        Seamless = true;
        Trasparent = false;
        Timing = true;
        Timing_out_frame_delay = get_Timing_Out_Frame_Delay(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_CHARACTER_TIMEOUT],\
                                                            DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);
        Timing_in_frame_delay = get_Timing_In_Frame_Delay(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);
        //syslog(LOGOPTS,"MODBUS mode \n");
        //syslog(LOGOPTS,"MODBUS_out_frame_delay:%d  \a\n",MODBUS_out_frame_delay);
        //syslog(LOGOPTS,"MODBUS_in_frame_delay:%d  \a\n",MODBUS_in_frame_delay);
        max_packet_size = atoi(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_MAXIMUM_PACKET_SIZE]);
        //syslog(LOGOPTS,"max_packet_size:%d \n",max_packet_size);
        if ((max_packet_size < 1)||(max_packet_size >2048))
            max_packet_size = 256;
        //syslog(LOGOPTS,"max_packet_size:%d  \a\n",max_packet_size);           

#else
        Seamless = true;
        Trasparent = false;
        Timing = true;
        Timing_out_frame_delay = get_Timing_Out_Frame_Delay(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_CHARACTER_TIMEOUT],\
                                                            DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);
        Timing_in_frame_delay = get_Timing_In_Frame_Delay(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);
        Timing_in_frame_delay = 30000;
        //syslog(LOGOPTS,"Timing mode \n");
        max_packet_size = atoi(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_MAXIMUM_PACKET_SIZE]);
        //syslog(LOGOPTS,"max_packet_size:%d \n",max_packet_size);
        if ((max_packet_size < 1)||(max_packet_size >2048))
            max_packet_size = 256;
#endif
    } else {  /* transparent, for encom, it is real transparent*/
#ifdef ENCOM
        Trasparent = true;
        Seamless = false;
        Timing = false;
        //syslog(LOGOPTS,"Trasparent mode\n");
        Timing_out_frame_delay = 0;
        Timing_in_frame_delay = 0;
        max_packet_size    =    atoi(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_MAXIMUM_PACKET_SIZE]);
        /*'A' SHOULD ACCORDING TO DATABASE */
        if ((max_packet_size < 1)||(max_packet_size >1024))
            max_packet_size = 256;

#else
        Trasparent = true;
        Seamless = false;
        Timing = true;
        //syslog(LOGOPTS,"Trasparent mode\n");
        Timing_out_frame_delay =  get_Timing_Out_Frame_Delay(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_CHARACTER_TIMEOUT],\
                                                             DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_BAUD_RATE][FIRST_CHARACTOR]);

        Timing_in_frame_delay = 6665534;
        max_packet_size  = atoi(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_MAXIMUM_PACKET_SIZE]);
        /*'A' SHOULD ACCORDING TO DATABASE */
        if ((max_packet_size < 1)||(max_packet_size >2048))
            max_packet_size = 256;
#endif
    }

    //syslog(LOGOPTS,"Timing_out_frame_delay:%d \n",Timing_out_frame_delay);
    //syslog(LOGOPTS,"Timing_in_frame_delay:%d \n",Timing_in_frame_delay);


    com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='A'; /*not active*/
    write_com_statistics();

    //printf("before while\n");
    while (1) {
        //syslog(LOGOPTS,"main loop\n");
        sleep(1);

        ifname = ifname_bridge;
        check_ip_address();     

        DBG_log("[%s] Fetch IP OK, IP: %s, IP_PROTOCOL_CONFIG: %c\n", __func__, buff_IP, DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]);

        if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]<='C') {
            tcp_process();
        }/* end of if ABC*/
        else if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='D') {
            udp_point_to_point_process();
        }/* end of if*/
        else if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='E') {
            udp_point_to_multipoint_as_point_process();
        }/* end of if*/
        else if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='F') {
            udp_point_to_multipoint_as_multipoint_process();
        }/* end of if*/
        else if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='G') {
            udp_multipoint_to_multipoint_process();
        }/* end of if*/
        else if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='H') {
            //smtp_process();
            new_smtp_process();
        }/* end of if*/
        else if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='J') {
            sms_process();
        }/* end of if*/
#if defined(VIP4G)
        else if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='K') {
            sms_at_process();
        }/* end of if*/
#endif
        else if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='L') {
            gps_process();
        }/* end of if*/
        else {
            syslog(LOGOPTS,"IP protocol not supported exit\n");       
            return 0;
        }

    } /*while if normal never die*/
}



/**********************************************************************************
   Function:    void sms_at_process(void) 
   Input:       void
   Output:      None
   Return:      void  
   Note:	   
**********************************************************************************/
#if defined(VIP4G)
static void sms_at_process(void)
{
    DBG_log("[%s] ENTER SMS AT mode\n", __func__);

    while (1) {

        FD_ZERO(&my_readfd);/*clear ,it should not be out side of while*/

        FD_SET(fdcom,&my_readfd);// set com
        tv_100us.tv_sec = 0;  
        tv_100us.tv_usec =100;
        select_return= select(fdcom+1,&my_readfd, NULL,NULL,&tv_100us);//
        if (select_return==-1) {
            syslog(LOGOPTS,"select error %s\n",strerror(errno));
            exit_process();
        }
        /* if the first data is from serial port, then local as tcp client , remote as tcp server */
        if (FD_ISSET(fdcom,&my_readfd)) {

            com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN);

            if (com_read_once_number>0) {
                com2_check_three_plus();              
            } else {
            }
        }
        sleep(1);
    } // end while(1)

}
#endif



/************************************************************************** 
    TCP process thread function.
    Called by tcp_process()
***************************************************************************/
void *tcp_clients_thread_function(void *arg)
{
    int i=0;
    int polling_resume_point=0, curent_select=0;
    DBG_log("ENTER [%s]\n", __func__);

    fd_set my_readfd;  //create a local read fd for new thread
    while (12) {
        //disableDTRafterNoDataFromSocketTimeOut();
        /* if connection timeout, then disconnect remote client go to idle mode.*/
        //DBG_log("[%s] tcp_clients_thread_function begin 2\n", __func__);

        if (sysinfo(&info)==0)
            current_time=(int) info.uptime -link_timeout_begin;

        //DBG_log("[%s] current_time=%d,link_timeout_begin=%d\n", __func__, current_time,link_timeout_begin);

        if (current_tcp_client_num<=0) {
            sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",current_tcp_client_num);
            write_com_statistics();
            DBG_log("[%s] no connect exit the thread\n", __func__);
            return NULL;
        }

        if (current_time > incoming_timeout) {
            com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='A';
            write_com_statistics();

            for (i=0;i<REMOTE_TCP_CLIENT_NUMBER;i++) {
                if (remote_tcp_client_list[i]>0) {
                    shutdown(remote_tcp_client_list[i],SHUT_RDWR);
                    close(remote_tcp_client_list[i]);     
                    remote_tcp_client_list[i] = 0;
                    current_tcp_client_num--;
                }
            }

            sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",current_tcp_client_num);
            write_com_statistics();

            DBG_log("[%s] timeout then disconnect client exit thread\n", __func__);
            //goto tcp_main_loop; /* disconnect client*/
            setInitialMultiTcpValue();
            return NULL;
        }

        /*set com and socket to receive data.*/
        FD_ZERO(&my_readfd);/*clear ,it should not be out side of while*/
        FD_SET(fdcom,&my_readfd);
        FD_SET(local_as_server_sockfd,&my_readfd);//  set for remote server socket

        for (i=0;i<REMOTE_TCP_CLIENT_NUMBER;i++) {
            if (remote_tcp_client_list[i]>0) {
                FD_SET(remote_tcp_client_list[i],&my_readfd);
            }
        }

        if (high_tcp_socket < fdcom) {
            high_tcp_socket = fdcom;
        }

        if (first_Data_Come==0) {
            tv_100us.tv_sec = 0;  
            tv_100us.tv_usec =100;
            select_return=select(high_tcp_socket+1,&my_readfd, NULL,NULL,&tv_100us);    
        } else {
            if (Timing) {
                tv_100us.tv_sec = 0;  
                tv_100us.tv_usec =100;
                select_return=select(high_tcp_socket+1,&my_readfd, NULL,NULL,&tv_100us);        
            } else {
                tv_100us.tv_sec = 0;  
                tv_100us.tv_usec =100;
                select_return=select(high_tcp_socket+1,&my_readfd, NULL,NULL,&tv_100us);
            }
        }

        if (select_return==-1) {
            DBG_log("[%s] select(high_tcp_socket+1,) select_return==-1)", __func__);
            exit_process();/*select return -1 exit*/
        }
        //DBG_log("[%s] select(high_tcp_socket+1,) select_return=%d\n)", __func__, select_return);

#if SOIPD1_SUPPORT_THREAD

#else
        if ((current_tcp_client_num<REMOTE_TCP_CLIENT_NUMBER)&&(FD_ISSET(local_as_server_sockfd,&my_readfd))) {
            if (sysinfo(&info)==0)
                link_timeout_begin =(int) info.uptime;
            sin_size=sizeof(struct sockaddr_in);
            /*  accept connection*/
            if ((remote_as_client_sockfd=accept(local_as_server_sockfd,(struct sockaddr*)(&local_as_server_addr),&sin_size))!=-1)
            /*accept a connection from client,block until accepted*/
            {
                /*get client address from accept*/
                //syslog(LOGOPTS,"Server connect with %s\n", inet_ntoa(local_as_server_addr.sin_addr));
                for (i=0;i<REMOTE_TCP_CLIENT_NUMBER;i++) {
                    if (remote_tcp_client_list[i]<=0) {
                        remote_tcp_client_list[i]=remote_as_client_sockfd;
                        if (high_tcp_socket < remote_as_client_sockfd) {
                            high_tcp_socket = remote_as_client_sockfd;
                        }
                        break;
                    }
                }
                current_tcp_client_num++;
                sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",current_tcp_client_num);
                write_com_statistics();
                //syslog(LOGOPTS,"%s current_tcp_client_num %d, local_as_server_sockfd ADDed at %d\n", __func__,current_tcp_client_num,i);
            } else {
                //syslog(LOGOPTS," can not accept new connection\n");
                shutdown(remote_as_client_sockfd,SHUT_RDWR);
                close(remote_as_client_sockfd);
                return NULL;
            }
            setTcpSocketAlive(remote_as_client_sockfd);
            /*NO BLOCK*/
            fcntl (remote_as_client_sockfd, F_SETFL, fcntl(remote_as_client_sockfd, F_GETFL) | O_NONBLOCK);
        }
#endif

#if 0
        /* Select func return, something happen */
        if (FD_ISSET(local_as_server_sockfd, &my_readfd)) {
            DBG_log("[%s] FD_ISSET(local_as_server_sockfd) return\n", __func__);
            if (sysinfo(&info)==0)
                link_timeout_begin =(int) info.uptime;

            sin_size=sizeof(struct sockaddr_in);

            /*	accept connection*/
            if ((remote_as_client_sockfd=accept(local_as_server_sockfd,(struct sockaddr*)(&local_as_server_addr),&sin_size))!=-1)
            /*accept a connection from client,block until accepted*/
            {
                /*get client address from accept*/
                DBG_log("[%s] Server connect with %s\n", __func__, inet_ntoa(local_as_server_addr.sin_addr));
                for (i=0;i<REMOTE_TCP_CLIENT_NUMBER;i++) {
                    if (remote_tcp_client_list[i]<=0) {
                        remote_tcp_client_list[i]=remote_as_client_sockfd;
                        if (high_tcp_socket < remote_as_client_sockfd) {
                            high_tcp_socket = remote_as_client_sockfd;
                        }
                        break;
                    }
                }

                current_tcp_client_num++;
                sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",current_tcp_client_num);
                write_com_statistics();

                DBG_log("[%s] current_tcp_client_num %d\n", __func__, current_tcp_client_num);

            } else {
                DBG_log("[%s]  can not accept new connection\n", __func__);
                shutdown(remote_as_client_sockfd,SHUT_RDWR);
                close(remote_as_client_sockfd);
                return NULL;
            }

            setTcpSocketAlive(remote_as_client_sockfd);
            /*NO BLOCK*/
            fcntl (remote_as_client_sockfd, F_SETFL, fcntl(remote_as_client_sockfd, F_GETFL) | O_NONBLOCK);

        }
#endif

        /******************************* read data from com *************************/
        if (FD_ISSET(fdcom,&my_readfd)) {
            DBG_log("[%s] FD_ISSET(fdcom) return, DATA from com", __func__); //
            if (data_from_com_process(remote_as_client_sockfd)==-1) {
                // goto tcp_main_loop;
                current_tcp_client_num =0;
                sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",current_tcp_client_num);
                write_com_statistics();
                return NULL;
            }

        }/* end of FD_ISSET read data from com*/

        /******************************* read data from remote clients *************************/
        //printf("Polling########################## mode = %s\n",DataBase1_Buff[COM12_SUB_MENU_TSERVER][TSERVER_ITEM_PULLING_MODE])
        if (DataBase1_Buff[COM12_SUB_MENU_TSERVER][TSERVER_ITEM_PULLING_MODE][FIRST_CHARACTOR]=='A') {
            /*monitor mode*/
            for (i=0;i<REMOTE_TCP_CLIENT_NUMBER;i++) {
                if (remote_tcp_client_list[i]>0) {/**/

                    if (FD_ISSET(remote_tcp_client_list[i],&my_readfd)) {

                        DBG_log("[%s] MONITOR MODE: data from %s, remote_tcp_client_list[%d]=%d", __func__, inet_ntoa(local_as_server_addr.sin_addr), i, remote_tcp_client_list[i]); //
                        if (data_from_socket_process(remote_tcp_client_list[i])==-1) {
                            remote_tcp_client_list[i] = 0;
                            current_tcp_client_num--;
                            sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",current_tcp_client_num);
                            write_com_statistics();

                            if (current_tcp_client_num<=0) {
                                //goto tcp_main_loop;
                                current_tcp_client_num =0;
                                return NULL;
                            }
                        }

                    }/* end of if fissset*/

                }
            }
        } else {
            /*pulling mode only read one*/
            if ((current_polling_unit == -1)||( current_polling_get_response==true)||(time_elapse(current_polling_time_s,current_polling_time_us)>polling_time_out)) {/*currently there is no polling unit, or time out, we need select the new polling unit request*/
                if (current_polling_unit >= 0) {
                    polling_resume_point = current_polling_unit+1;
                } else {
                    polling_resume_point =0;
                }
                for (i=0;i<REMOTE_TCP_CLIENT_NUMBER;i++) {
                    curent_select = (i+ polling_resume_point)%REMOTE_TCP_CLIENT_NUMBER;
                    if (remote_tcp_client_list[curent_select]>0) {/**/
                        if (FD_ISSET(remote_tcp_client_list[curent_select], &my_readfd)) {
                            DBG_log("[%s] PULLING MODE: data from %s, remote_tcp_client_list[%d] = %d", __func__, inet_ntoa(local_as_server_addr.sin_addr), i, remote_tcp_client_list[i]); // //
                            if (data_from_socket_process(remote_tcp_client_list[curent_select])==-1) {
                                remote_tcp_client_list[curent_select] = 0;
                                current_tcp_client_num--;
                                sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",current_tcp_client_num);
                                write_com_statistics();
                                if (current_tcp_client_num<=0) {
                                    //goto tcp_main_loop;
                                    current_tcp_client_num =0;
                                    return NULL;
                                }
                            } else {
                                current_polling_unit = curent_select;
                                current_polling_get_response=false;
                                /*update polling time*/
                                gettimeofday(&tv_delay,&tz);
                                current_polling_time_s = tv_delay.tv_sec;
                                current_polling_time_us = tv_delay.tv_usec;  
                                break;/*we only need one*/
                            }

                        }/* end of if fissset*/
                    }
                }
            } else { /*already has a pulling unit and not timeout, just ignore clients requests*/

            }
        }

        /* **************************************************************************** */
        /* if this is last packet from com, then after frame delay send it to socket*/
        if ( com_receive_num_for_send_total >0) {
            DBG_log("[%s] packet from com, com_receive_num_for_send_total=%d\n", __func__, com_receive_num_for_send_total); // //
            if (now_have_com_data_process(remote_as_client_sockfd)==-1)
            /*if there is only one link then send to remote_as_client_sockfd, else send to all sockets at the next layer*/
            {
                if (current_tcp_client_num<=0) {
                    // goto tcp_main_loop;
                    current_tcp_client_num =0;
                    return NULL;
                }
            }
        }

        /* if this is last packet from socket, then after frame delay send it to com*/
        if ( socket_receive_num_for_send_total >0) {
            DBG_log("[%s] packet from socket, socket_receive_num_for_send_total=%d\n", __func__, socket_receive_num_for_send_total); // //
#if 0
            for (i=0;i<REMOTE_TCP_CLIENT_NUMBER;i++) {
                if (remote_tcp_client_list[i]>0) {
#endif                   
                    if (now_have_socket_data_process(remote_as_client_sockfd)==-1) {
                        remote_tcp_client_list[i] = 0;
                        current_tcp_client_num--;
                        sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",current_tcp_client_num);
                        write_com_statistics();
                        if (current_tcp_client_num<=0) {
                            //goto tcp_main_loop;
                            current_tcp_client_num =0;
                            return NULL;
                        }
                    }
#if 0
                }
            } 
#else
                    //syslog(LOGOPTS,"%s shold not be here",__func__);
#endif
                }/*if ( socket_receive_num_for_send_total >0*/
                /* **************************************************************************** */

            }/*end of while 12*/
        } /* end tcp_clients_thread_function */




/**********************************************************************************
   Function:    void tcp_process(void) 
   Input:       void
   Output:      None
   Return:      void  
   Note:	config tcp client and/or server and process data from com port or socket.        
**********************************************************************************/
        static void tcp_process(void)
        {
            int i;
            struct hostent *hp;
            DBG_log("ENTER [%s]\n", __func__ );   //

            incoming_timeout = atoi(DataBase1_Buff[COM12_SUB_MENU_TSERVER][TSERVER_ITEM_1]);
            if ( incoming_timeout < 0 ) {
                syslog(LOGOPTS,"tcp_process incoming_timeout < 0 exit\n");                         
                exit_process();
            }

            outgoing_timeout = atoi(DataBase1_Buff[COM12_SUB_MENU_TCLIENT][TCLIENT_ITEM_2]);
            if ( outgoing_timeout<0) {
                syslog(LOGOPTS,"tcp_process outgoing_timeout < 0 exit\n");      
                exit_process();
            }

            DBG_log("[%s] incoming_timeout = %d\n", __func__ , incoming_timeout);   //
            DBG_log("[%s] outgoing_timeout = %d\n", __func__ , outgoing_timeout);   //

            /* if this unit as TCP server*/
            if ((DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='B')||
                (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='C')) {

                local_listen_port_int = atoi(DataBase1_Buff[COM12_SUB_MENU_TSERVER][TSERVER_ITEM_0]);
                DBG_log("[%s] local_listen_port_int = %d\n", __func__, local_listen_port_int);   //
                if (local_listen_port_int < 0) {     /*atoi(buff)*/
                    syslog(LOGOPTS,"tcp_process local_listen_port_int<0 %s exit\n",strerror(errno));
                    exit_process();
                }

                /*build a socket    IPPROTO_TCP*/
                if ((local_as_server_sockfd = socket(AF_INET, SOCK_STREAM,0))==-1) {
                    syslog(LOGOPTS,"tcp_process Socket error %s exit\n",strerror(errno));
                    exit_process();
                }

                /* reuse address*/
                if (setsockopt(local_as_server_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0) {
                    syslog(LOGOPTS,"tcp_process reuse address error %s exit\n",strerror(errno));
                    exit_process();
                }

                setTcpSocketAlive(local_as_server_sockfd);
                bzero(&local_as_server_addr,sizeof(struct sockaddr_in));

                /*set the server_address to 0*/
                local_as_server_addr.sin_family = AF_INET;/*internet*/
                local_as_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

                /*any address can connect*/
                local_as_server_addr.sin_port = htons(local_listen_port_int);

                /*put the remote_server_port_int from host to internet frame*/
                if (bind(local_as_server_sockfd,(struct sockaddr*)(&local_as_server_addr),sizeof(struct sockaddr))==-1) {
                    /*bind the setting with the socket*/        
                    syslog(LOGOPTS,"tcp_process Bind error %s exit\n",strerror(errno));
                    exit_process();
                }

                /* pending_connection_num = atoi(DataBase1_Buff[COM12_SUB_MENU_TSERVER][TSERVER_ITEM_2]); 
                   if (pending_connection_num<0)
                       {
                       pending_connection_num=0;
                       }
                   else
                       {
                       }           
                */
                if (listen(local_as_server_sockfd,REMOTE_TCP_CLIENT_NUMBER)==-1) {      /*can not block here.*/
                    syslog(LOGOPTS,"tcp_process Listen error %s exit\n",strerror(errno));
                    exit_process();
                }
            }  /* END OF if TCP B/C SERVER*/

            tcp_main_loop:

            DBG_log("[%s] tcp_main_loop\n", __func__);
            setInitialMultiTcpValue();

            /*idle status, wait data from serial port(when as tcp client) or socket(when as tcp server)*/
            while (1) {
                /*initial the data.*/
                first_Data_Come = 0;
                com_receive_num_for_send_total = 0;
                socket_receive_num_for_send_total = 0;

                /*write com port not active again*/
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='A';
                write_com_statistics();

                /* in idle mode write status.*/
                FD_ZERO(&my_readfd);/*clear ,it should not be out side of while*/
                DBG_log("[%s] TCP (ABC) MAIN SELECT FUNC (wait for data from socket/com)\n", __func__);

#if defined(IXP425COM1) || defined(MPCI)
                DBG_log("[%s] 1\n", __func__);           
                if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR] != 'B')
                    FD_SET(fdcom, &my_readfd);// set com
                if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR] > 'A')
                    FD_SET(local_as_server_sockfd, &my_readfd);//set for remote server socket

                if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR] == 'A')
                    select_return= select(fdcom+1, &my_readfd, NULL,NULL,NULL);//
                else if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR] == 'B')
                    select_return= select(local_as_server_sockfd+1, &my_readfd, NULL,NULL,NULL);
                else if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR] == 'C') {
                    if (fdcom>local_as_server_sockfd)
                        select_return= select(fdcom+1, &my_readfd, NULL,NULL,NULL);//&tv_1s
                    else
                        select_return= select(local_as_server_sockfd+1, &my_readfd, NULL,NULL,NULL); //&tv_1s
                }
#endif

#if defined(IXP425COM2)||defined(IXP425USB)||defined(MPCI2)
                DBG_log("[%s] 2\n", __func__);         
                FD_SET(fdcom, &my_readfd);// set com
                if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR] > 'A')
                    FD_SET(local_as_server_sockfd, &my_readfd);//  set for remote server socket

                if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR] == 'A')
                    select_return= select(fdcom+1, &my_readfd, NULL,NULL,NULL);//
                else {
                    if (fdcom>local_as_server_sockfd)
                        select_return= select(fdcom+1, &my_readfd, NULL,NULL,NULL);//&tv_1s
                    else
                        select_return= select(local_as_server_sockfd+1, &my_readfd, NULL,NULL,NULL); //&tv_1s
                }
#endif

                if (select_return==-1) {
                    syslog(LOGOPTS,"tcp_process select error %s exit\n", strerror(errno));
                    exit_process();
                }

                DBG_log("[%s] Select func return. \n", __func__);    // 
                //DBG_log("[%s] socket: %d\n", __func__, socket);    // 


                /**********************************************************************************************    
                     LOCAL AS  TCP Client (A)                     
                *********************************************************************************************/
                /* if the first data is from serial port, then local as tcp client , remote as tcp server */
                if ((current_tcp_client_num<=0) && (FD_ISSET(fdcom,&my_readfd))) {
                    DBG_log("[%s] FD_ISSET(fdcom, &my_readfd) return true\n", __func__);    //
                    DBG_log("[%s] LOCAL AS TCP Client (options: A)\n", __func__);    //

                    com_read_once_number = read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN);
                    DBG_log("[%s] TCP A/C com_read_once_number: %d \n", __func__, com_read_once_number);

                    /* no data , idle mode loop while (1) again */
                    if (com_read_once_number < 0) {
                        syslog(LOGOPTS,"tcp_process com_read_once_number<=0 exit\n");
                        close(fdcom);
                        exit_process();
                    } else if (com_read_once_number==0) {
                        DBG_log("[%s] tcp_process com_read_once_number==0\n", __func__);
                        goto tcp_main_loop;
                    }

#if defined(IXP425COM2) || defined(MPCI2)
                    com2_check_three_plus();
#endif        
#ifdef IXP425USB
                    usb_check_three_plus();
#endif        
                    /* some data, come from serial port */
                    if (Timing_out_frame_delay==0) {
                        if (write_to_socket_process(socket)==-1) {
                            DBG_log("[%s] data come from COM, write_to_socket_process() return -1 \n", __func__);
                            goto tcp_main_loop;
                        }
                    }

                    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='B') {
                        goto tcp_main_loop;
                    }

                    first_Data_Come=1;

                    /*after get first data from serial port, get link timeout begin init time*/
                    if (sysinfo(&info)==0)
                        link_timeout_begin =(int) info.uptime;

                    /*after get first data from serial port, get read com data begin init time*/
                    if (Timing) {
                        gettimeofday (&tv_delay, &tz);
                        read_com_data_start_sec = tv_delay.tv_sec;
                        read_com_data_start_usec = tv_delay.tv_usec; 
                    }

                    com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='B';
                    write_com_statistics();

                    /*read data from serial port*/
                    if ((local_as_client_sockfd = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP))==-1) {
                        syslog(LOGOPTS,"tcp_process Socket error:%s exit\n\a",strerror(errno));
                        com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                        write_com_statistics();
                        exit_process();                        
                    }

#ifdef DEBUG_SET_SOCKET_BUFF
                    setsockopt(local_as_client_sockfd, SOL_SOCKET, SO_SNDBUF, (void*)&sndbuf,sizeof(sndbuf));
                    setsockopt(local_as_client_sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf,sizeof(sndbuf));
#endif  

                    setTcpSocketAlive(local_as_client_sockfd);

                    /* reuse socket address*/
                    if (setsockopt (local_as_client_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))< 0) {
                        syslog(LOGOPTS,"tcp_process SO_REUSEADDR %s exit\n",strerror(errno));
                        com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                        write_com_statistics();
                        exit_process();
                    }

                    bzero(&remote_as_server_addr,sizeof(struct sockaddr_in));/*set the server_address*/
                    remote_as_server_addr.sin_family = AF_INET;/*internet*/
                    while ((hp = gethostbyname(DataBase1_Buff[COM12_SUB_MENU_TCLIENT][TCLIENT_ITEM_0])) == NULL) {
                        syslog(LOGOPTS,"gethostbyname %s: unknown host\n", mailhost);
                        sleep(5);
                    }
                    if (hp->h_addrtype != AF_INET) {
                        syslog(LOGOPTS,"smtp_process unknown address family: %d", hp->h_addrtype);
                        exit_process();
                    }

                    memcpy((char *)&remote_as_server_addr.sin_addr, hp->h_addr, hp->h_length);
                    remote_as_server_addr.sin_family = hp->h_addrtype;



                    //remote_as_server_addr.sin_addr.s_addr =inet_addr(DataBase1_Buff[COM12_SUB_MENU_TCLIENT][TCLIENT_ITEM_0]);

                    /*remote server addr*/
                    //syslog(LOGOPTS,"DataBase1_Buff[COM12_SUB_MENU_TCLIENT][TCLIENT_ITEM_0] %s\n",DataBase1_Buff[COM12_SUB_MENU_TCLIENT][TCLIENT_ITEM_0]);
                    //syslog(LOGOPTS,"DataBase1_Buff[COM12_SUB_MENU_TCLIENT][TCLIENT_ITEM_1] %s\n",DataBase1_Buff[COM12_SUB_MENU_TCLIENT][TCLIENT_ITEM_1]);
                    remote_server_port_int = atoi(DataBase1_Buff[COM12_SUB_MENU_TCLIENT][TCLIENT_ITEM_1]);
                    if (remote_server_port_int<0) {     /*atoi(buff)*/
                        com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                        write_com_statistics();
                        syslog(LOGOPTS,"tcp_process remote_server_port_int<0 exit\n");             
                        exit_process();
                    }
                    remote_as_server_addr.sin_port = (htons(remote_server_port_int));
                    /* Establish connection with remote server, so set socket and com to nonblock */
                    /*connect to remote server*/

#if defined(IXP425COM2) || defined(IXP425USB) || defined(MPCI2)           
                    pthread_create(&pt_show_message,NULL,thread_function,NULL);//(void*)(&fdcom);
#endif
                    if (connect(local_as_client_sockfd,(struct sockaddr*) &remote_as_server_addr, sizeof(struct sockaddr)) < 0) {
                        shutdown(local_as_client_sockfd,SHUT_RDWR);
                        close(local_as_client_sockfd);
                        //syslog(LOGOPTS,"TCP A/C local_as_client_sockfd connection fail %s\n",strerror(errno));

#if defined(IXP425COM2) || defined(IXP425USB) || defined(MPCI2)            
                        pthread_cancel(pt_show_message);
#endif
                        goto tcp_main_loop;/* while(1) idle state resume read data and through away the old data*/
                    }

#if defined(IXP425COM2) || defined(IXP425USB) || defined(MPCI2) 
                    thread_return = true;          
                    if (0!=pthread_cancel(pt_show_message)) {
                        //syslog(LOGOPTS,"pthread_kill fail %s\n",strerror(errno)); 
                    } else {
                        //syslog(LOGOPTS,"pthread_kill ok\n"); 
                    }
#endif

                    setTcpSocketAlive(local_as_client_sockfd);
                    fcntl (local_as_client_sockfd, F_SETFL, fcntl(local_as_client_sockfd, F_GETFL) | O_NONBLOCK);  /* dont block read*/
                    fctl_com();

                    /*after get first data from serial port, move it to receive Frame buffer*/
                    memcpy(com_total_buff, com_read_once_buff,com_read_once_number);
                    com_receive_num_for_send_total = com_read_once_number; /* this is first time to get data*/

                    if (!Timing) {
                        if (write_to_socket_process(local_as_client_sockfd)==-1) {
                            //goto tcp_main_loop;
                            close(local_as_client_sockfd);
                            //return;
                            goto tcp_main_loop;
                        }
                    }

                    //	pthread_create(&pt_show_message,NULL,thread_function,NULL);//(void*)(&fdcom);
                    while (11) {               /* belong to if com1 has data at first. or there is no socket connection*/
                        /* if timeout, disconnet connection*/
                        if (sysinfo(&info)==0)
                            current_time=(int) info.uptime -link_timeout_begin;

                        if (current_time >= outgoing_timeout) {
                            shutdown(local_as_client_sockfd,SHUT_RDWR);
                            close(local_as_client_sockfd);
                            current_time = 0;
                            //syslog(LOGOPTS,"1TCP A/C  connetction timeout disconnect socket ok\n");
                            goto tcp_main_loop; /* disconnect client*/
                        }

                        if (set_select_readfd(fdcom, local_as_client_sockfd)==-1) {
                            syslog(LOGOPTS,"tcp_process exit exit\n");  
                            exit_process();/*select return -1 exit*/
                        }

                        /************************** read data from serial port *************/
                        if (FD_ISSET(fdcom,&my_readfd)) {
                            if (data_from_com_process(local_as_client_sockfd)==-1) {
                                //goto tcp_main_loop;                  
                                close(local_as_client_sockfd);
                                //return;
                                goto tcp_main_loop;
                            }

                        }/* end of FD_ISSET read data from com*/

                        /******************************* read data from remote server *************************/
                        if (FD_ISSET(local_as_client_sockfd,&my_readfd)) {
                            if (data_from_socket_process(local_as_client_sockfd)==-1) {
                                goto tcp_main_loop;
                            }
                        }/* end of if fissset*/

                        /* if this is last packet from com, then after frame delay send it to socket*/
                        if ( com_receive_num_for_send_total >0) {
                            if (now_have_com_data_process(local_as_client_sockfd)==-1) {
                                //goto tcp_main_loop;
                                close(local_as_client_sockfd);
                                //return;
                                goto tcp_main_loop;
                            }
                        }

                        /* if this is last packet from socket, then after frame delay send it to com*/
                        if ( socket_receive_num_for_send_total >0) {
                            if (now_have_socket_data_process(local_as_client_sockfd)==-1) {
                                syslog(LOGOPTS,"tcp_process exit\n");                    
                                close(local_as_client_sockfd);
                                //return;
                                goto tcp_main_loop;
                            }
                        }/*if ( socket_receive_num_for_send_total >0)*/

                    }/*endo of while 11*/
                }/* end of if (FD_ISSET(fdcom,&my_readfd)) tcp client*/


                /**********************************************************************************************    
                     LOCAL AS  TCP Server(B/C)                     
                 *********************************************************************************************/
                if (FD_ISSET(local_as_server_sockfd, &my_readfd)) {
                    DBG_log("[%s] FD_ISSET(local_as_server_sockfd, &my_readfd) return true\n", __func__);    //
                    DBG_log("[%s] LOCAL AS TCP Server(options: B or C)\n", __func__);    // 
                                                                                         //      
                    first_Data_Come=1; 
                    if (sysinfo(&info)==0)
                        link_timeout_begin =(int) info.uptime;
                    DBG_log("[%s] tcp client request link to this unit\n", __func__);    //

                    if (Timing) {
                        gettimeofday(&tv_delay,&tz);
                        read_socket_data_start_sec = tv_delay.tv_sec;
                        read_socket_data_start_usec = tv_delay.tv_usec;
                    }

                    com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='B';
                    write_com_statistics();

                    if ((DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='B') || (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]=='C')) {

                        sin_size=sizeof(struct sockaddr_in);

                        /*	accept connection*/
                        if ((remote_as_client_sockfd=accept(local_as_server_sockfd,(struct sockaddr*)(&local_as_server_addr),&sin_size))!=-1)
                        /*accept a connection from client, block until accepted*/
                        {
                            /*get client address from accept*/
                            DBG_log("[%s] Server connect with %s\n",  __func__, inet_ntoa(local_as_server_addr.sin_addr));
                            for (i=0;i<REMOTE_TCP_CLIENT_NUMBER;i++) {
                                if (remote_tcp_client_list[i]<=0) {
                                    remote_tcp_client_list[i]=remote_as_client_sockfd;
                                    if (high_tcp_socket < remote_as_client_sockfd) {
                                        high_tcp_socket = remote_as_client_sockfd;
                                    }
                                    break;
                                }
                            }
                            current_tcp_client_num++;
                            sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",current_tcp_client_num);
                            write_com_statistics();
                            DBG_log("[%s] current_tcp_client_num %d\n", __func__, current_tcp_client_num);
                        } else {
                            DBG_log("[%s]  can not accept new connection\n", __func__);
                            shutdown(remote_as_client_sockfd,SHUT_RDWR);
                            close(remote_as_client_sockfd);
                            goto tcp_main_loop;
                        }

                        /*NO BLOCK*/
#ifdef DEBUG_SET_SOCKET_BUFF
                        setsockopt(remote_as_client_sockfd, SOL_SOCKET, SO_SNDBUF, (void*)&sndbuf,sizeof(sndbuf));
                        setsockopt(remote_as_client_sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf,sizeof(sndbuf));
#endif
                        setTcpSocketAlive(remote_as_client_sockfd);

                        fcntl (remote_as_client_sockfd, F_SETFL, fcntl(remote_as_client_sockfd, F_GETFL) | O_NONBLOCK);
                        fctl_com();

                        //	pthread_create(&pt_show_message,NULL,thread_function,NULL);//(void*)(&remote_as_client_sockfd));
                        /* after build connection go to loop to check data from serial port and socket*/
#if SOIPD1_SUPPORT_THREAD
                        if (current_tcp_client_num==1) {
                            //syslog(LOGOPTS,"create thread\n");
                            pthread_create(&pt_show_message,NULL,tcp_clients_thread_function,NULL);//(void*)(&fdcom);
                        }
#else
                        tcp_clients_thread_function(NULL);
#endif

                    }/*end of if BC*/

                }/* end of F_ISEET AS server*/
            }/* end of while 1*/        
        } // end of tcp_process




/**********************************************************************************
   Function:   static  static void udp_point_to_point_process(void) 
   Input:       void
   Output:      None
   Return:      void  
   Note:        bind to a port to listen to and get data from.
        config the other port to send data to remote. then process incoming
        and outgoing data.
**********************************************************************************/
        static void udp_point_to_point_process(void)
        {
            if ((local_as_server_sockfd = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP))==-1) {/*build a socket*/
                syslog(LOGOPTS,"udp_point_to_point_process Socket error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            if (setsockopt (local_as_server_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))< 0) {
                syslog(LOGOPTS,"udp_point_to_point_process setsockopt:%s exit\n\a",strerror(errno));       
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            bzero(&local_as_server_addr,sizeof(struct sockaddr_in));/*set the server_address to 0*/
            local_as_server_addr.sin_family = AF_INET;/*internet*/
            local_as_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);/*any address can connect*/
            local_listen_port_int=atoi(DataBase1_Buff[COM12_SUB_MENU_UPTOP][UPTOP_ITEM_2]);  /*UDP local port;*/
            if (local_listen_port_int<0) {     /*atoi(buff)*/
                syslog(LOGOPTS,"udp_point_to_point_process UDP listen port:%d exit \n",local_listen_port_int );         
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            //syslog(LOGOPTS,"udp_point_to_point_process UDP listen port:%d \n",local_listen_port_int );     
            local_as_server_addr.sin_port = htons(local_listen_port_int);
            /*put the remote_server_port_int from host to internet frame*/
            if (bind(local_as_server_sockfd,(struct sockaddr*)(&local_as_server_addr),sizeof(struct sockaddr_in))==-1)
            /*bined the setting with the socket*/
            {
                syslog(LOGOPTS,"udp_point_to_point_process Bind error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            /* then build udp client, this unit as client*/
            bzero(&remote_as_server_addr,sizeof(struct sockaddr_in));/* */
            bzero(&remote_as_server2_addr,sizeof(struct sockaddr_in));/* */
            bzero(&remote_as_server3_addr,sizeof(struct sockaddr_in));/* */
            remote_as_server_addr.sin_family = AF_INET;/*internet*/

            if (atoi(DataBase1_Buff[COM12_SUB_MENU_UPTOP][UPTOP_ITEM_0])==0) {
                local_as_udp_server=1;
            } else {
                remote_as_server_addr.sin_addr.s_addr =inet_addr(DataBase1_Buff[COM12_SUB_MENU_UPTOP][UPTOP_ITEM_0]);
                remote_server_port_int=atoi(DataBase1_Buff[COM12_SUB_MENU_UPTOP][UPTOP_ITEM_1]);
                if (remote_server_port_int<0) {
                    syslog(LOGOPTS,"udp_point_to_point_process UDP remote_server_port_int:%d exit \n",remote_server_port_int ); 
                    com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                    write_com_statistics();
                    exit_process();
                }
                remote_as_server_addr.sin_port = (htons(remote_server_port_int));
            }

            /*put the remote_server_port_int from host to internet frame*/
            //syslog(LOGOPTS,"UDP server Port:%d  \a\n",remote_server_port_int );
            com_receive_num_for_send_total =0;
#if 0 //this way will use difference source
            if ((local_as_client_sockfd = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP))==-1) {/*build a udp client socket*/
                syslog(LOGOPTS,"udp_point_to_point_process Socket error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();    
            }
#else // this way will use same source as binding
            local_as_client_sockfd = local_as_server_sockfd;
#endif
            com_receive_num_for_send_total =0;
            socket_receive_num_for_send_total =0;
            first_Data_Come=0;
            fcntl (local_as_server_sockfd, F_SETFL, fcntl(local_as_server_sockfd, F_GETFL) | O_NONBLOCK);  /* dont block read*/
            fctl_com(); 
            udp_pp_loop:
            while (4) {
                if (set_select_readfd(fdcom,local_as_server_sockfd)==-1) {
                    syslog(LOGOPTS,"udp_point_to_point_process set_select_readfd==-1 exit\n");            
                    exit_process();/*select return -1 exit*/
                }
                if (first_Data_Come==1) {
                    if (sysinfo(&info)==0)
                        current_time=(int) info.uptime -link_timeout_begin;
                    if (current_time >= UDP_TIMEOUT) {
                        //syslog(LOGOPTS,"udp_point_to_point_process current_time =%d \n",current_time);

                        com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='A';
                        write_com_statistics();
                        current_time = 0;
                        first_Data_Come=0; 
                        if (local_as_udp_server) {
                            remote_as_server_addr.sin_addr.s_addr =inet_addr(DataBase1_Buff[COM12_SUB_MENU_UPTOP][UPTOP_ITEM_0]);
                            remote_server_port_int=atoi(DataBase1_Buff[COM12_SUB_MENU_UPTOP][UPTOP_ITEM_1]);
                        }
                    }
                }

                /************************** read data from serial port *************/
                if (FD_ISSET(fdcom,&my_readfd)) {
                    if (data_from_com_process(local_as_client_sockfd)==-1) {
                        //goto udp_pp_loop;
                        close(local_as_client_sockfd);
                        close(local_as_server_sockfd);
                        return;
                    }
                }/* end of FD_ISSET read data from com*/
                /******************************* read data from remote server *************************/
                if (FD_ISSET(local_as_server_sockfd,&my_readfd)) {    /**/
                    if (data_from_socket_process(local_as_server_sockfd)==-1) {
                        goto udp_pp_loop;
                    }
                }/* end of if fissset*/
                /* if this is last packet from com, then after frame delay send it to socket*/
                if ( com_receive_num_for_send_total >0) {
                    if (now_have_com_data_process(local_as_client_sockfd)==-1) {
                        // goto udp_pp_loop;
                        close(local_as_client_sockfd);
                        close(local_as_server_sockfd);
                        return;           
                    }
                }
                /* if this is last packet from socket, then after frame delay send it to com*/
                if ( socket_receive_num_for_send_total >0) {
                    if (now_have_socket_data_process(local_as_server_sockfd)==-1) {
                        syslog(LOGOPTS,"udp_point_to_point_process exit\n");   
                        exit_process();
                    }
                }/*if ( socket_receive_num_for_send_total >0)*/
            }/*endo of while 42*/
        }

/**********************************************************************************
   Function:   static  void udp_point_to_multipoint_as_point_process(void) 
   Input:       void
   Output:      None
   Return:      void  
   Note:        bind to a port to listen to and get data from.
        config the other multicast address and port to send data to remote. 
        then process incoming and outgoing data.
**********************************************************************************/
        static void udp_point_to_multipoint_as_point_process(void)
        {
            /*build UDP PTOMP Server first*/
            local_as_server_addr.sin_family = AF_INET;/*internet*/
            local_as_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);/*any address can connect*/
            local_listen_port_int=atoi(DataBase1_Buff[COM12_SUB_MENU_UPTOMP_AS_P][UPTOMP_AS_P_ITEM_2]);
            //syslog(LOGOPTS,"UDP as p listen port:%d  \a\n",local_listen_port_int );
            /*UDP local port;*/
            if (local_listen_port_int<0) {     /*atoi(buff)*/
                syslog(LOGOPTS,"udp_point_to_multipoint_as_point_process local_listen_port_int<0 exit\n");  
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            local_as_server_addr.sin_port = (htons(local_listen_port_int));
            /*put the local_listen_port_int from host to internet frame*/
            if ((local_as_server_sockfd = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP))==-1) {/*build a socket*/
                syslog(LOGOPTS,"udp_point_to_multipoint_as_point_process Socket error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            memset(&imreq, 0, sizeof(struct ip_mreq));
            if (setsockopt (local_as_server_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))< 0) {
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                syslog(LOGOPTS,"udp_point_to_multipoint_as_point_process setsockopt SO_REUSEADDR error:%s exit\n\a",strerror(errno));
                exit_process();
            }
            if (bind(local_as_server_sockfd,(struct sockaddr*)(&local_as_server_addr),sizeof(struct sockaddr))==-1)
            /*bined the setting with the socket*/
            {
                syslog(LOGOPTS,"udp_point_to_multipoint_as_point_process Bind error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }

#if 0 //this way will use difference source
            /*then build udp PMP AS P  multicast client to send multicast */
            if ((local_as_client_sockfd = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP))==-1)
            /*build a udp client socket*/
            {
                syslog(LOGOPTS,"udp_point_to_multipoint_as_point_process Socket error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();                        
            }
#else // this way will use same source as binding
            local_as_client_sockfd = local_as_server_sockfd;
#endif
            bzero(&remote_as_server_addr,sizeof(struct sockaddr_in));/*set the client_addr*/
            remote_as_server_addr.sin_family = PF_INET;/*internet*/
            remote_as_server_addr.sin_addr.s_addr =inet_addr(DataBase1_Buff[COM12_SUB_MENU_UPTOMP_AS_P][UPTOMP_AS_P_ITEM_0]);
            //syslog(LOGOPTS,"Remote UDP Multicaster server addR:%s  \a\n",DataBase1_Buff[COM12_SUB_MENU_UPTOMP_AS_P][UPTOMP_AS_P_ITEM_0] );
            remote_server_port_int=atoi(DataBase1_Buff[COM12_SUB_MENU_UPTOMP_AS_P][UPTOMP_AS_P_ITEM_1]);
            //syslog(LOGOPTS,"Remote UDP Multicaster server Port:%d  \a\n",remote_server_port_int );
            remote_as_server_addr.sin_port = (htons(remote_server_port_int));
            /*put the remote_server_port_int from host to internet frame*/
            com_receive_num_for_send_total =0;
            if (remote_server_port_int<0) {     /*atoi(buff)*/
                syslog(LOGOPTS,"udp_point_to_multipoint_as_point_process remote_server_port_int:%d exit\n\a",remote_server_port_int);
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            iaddr.s_addr = INADDR_ANY; /* use DEFAULT interface*/
            while (false==fetch_Local_IP_ADDR(ifname, buff_IP)) {
                syslog(LOGOPTS,"%s: failed to get %s address\n\a",__func__,ifname);
                sleep(1);
            }
            inet_aton(buff_IP,&iaddr);
            /* Set the outgoing interface to DEFAULT*/
            setsockopt(local_as_client_sockfd, IPPROTO_IP, IP_MULTICAST_IF, &iaddr,sizeof(struct in_addr));
            ttl = atoi(DataBase1_Buff[COM12_SUB_MENU_UPTOMP_AS_P][UPTOMP_AS_P_ITEM_3]);
            //syslog(LOGOPTS,"ttl:%d  \a\n",ttl );
            /* Set multicast packet TTL ; default TTL is 1*/
            setsockopt(local_as_client_sockfd, IPPROTO_IP, IP_MULTICAST_TTL,&ttl, sizeof(unsigned char));
            /* dont send multicast traffic to myself  */
            setsockopt(local_as_client_sockfd, IPPROTO_IP, IP_MULTICAST_LOOP,&not_one, sizeof(unsigned char));
            com_receive_num_for_send_total =0;
            socket_receive_num_for_send_total =0;
            first_Data_Come=0;
            /* dont block read*/
            fcntl (local_as_server_sockfd, F_SETFL, fcntl(local_as_server_sockfd, F_GETFL) | O_NONBLOCK); 
            fctl_com(); 
            udp_pmp_as_point_loop:
            while (5) {
                write_com_statistics();
                if (set_select_readfd(fdcom,local_as_server_sockfd)==-1) {
                    syslog(LOGOPTS,"udp_point_to_multipoint_as_point_process set_select_readfd==-1exit\n");
                    exit_process();/*select return -1 exit*/
                }
                if (first_Data_Come==1) {
                    if (sysinfo(&info)==0)
                        current_time=(int) info.uptime -link_timeout_begin;
                    if (current_time >= UDP_TIMEOUT) {
                        com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='A';
                        write_com_statistics();
                        current_time = 0;
                        //syslog(LOGOPTS,"TCP A/C  connetction timeout disconnect socket ok\n");
                        first_Data_Come=0;  
                    }
                }
                if (FD_ISSET(fdcom,&my_readfd)) {
                    if (data_from_com_process(local_as_client_sockfd)==-1) {
                        // goto udp_pmp_as_point_loop;
                        close(local_as_client_sockfd);
                        close(local_as_server_sockfd);
                        return;         

                    }
                }/* end of FD_ISSET read data from com*/
                /******************************* read data from remote server *************************/
                if (FD_ISSET(local_as_server_sockfd,&my_readfd)) {    /**/
                    if (data_from_socket_process(local_as_server_sockfd)==-1) {
                        goto udp_pmp_as_point_loop;
                    }
                }/* end of if fissset*/
                /* if this is last packet from com, then after frame delay send it to socket*/
                if ( com_receive_num_for_send_total >0) {
                    if (now_have_com_data_process(local_as_client_sockfd)==-1) {
                        // goto udp_pmp_as_point_loop;
                        close(local_as_client_sockfd);
                        close(local_as_server_sockfd);
                        return; 
                    }
                }
                /* if this is last packet from socket, then after frame delay send it to com*/
                if ( socket_receive_num_for_send_total >0) {
                    if (now_have_socket_data_process(local_as_server_sockfd)==-1) {
                        syslog(LOGOPTS,"udp_point_to_multipoint_as_point_process exit \n");             
                        exit_process();
                    }
                }/*if ( socket_receive_num_for_send_total >0)*/
            }
        }

/**********************************************************************************
   Function:   static  void udp_point_to_multipoint_as_multipoint_process(void) 
   Input:       void
   Output:      None
   Return:      void  
   Note:        bind to a multicast address and port to listen to and get data from.
        and config the other port to send data to remote. then process incoming
        and outgoing data.
**********************************************************************************/
        static void udp_point_to_multipoint_as_multipoint_process(void)
        {
            /*build UDP PTOMP Server first */
            bzero(&local_as_server_addr,sizeof(struct sockaddr_in));/*set the server_address to 0*/
            local_as_server_addr.sin_family = AF_INET;/*internet  */
            local_as_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);/*any address can connect*/
            local_listen_port_int=atoi(DataBase1_Buff[COM12_SUB_MENU_UPTOMP_AS_MP][UPTOMP_AS_MP_ITEM_3]);
            /*UDP Multicastlistenport;*/
            //syslog(LOGOPTS,"UDP as mp listen port:%d  \a\n",local_listen_port_int );
            if (local_listen_port_int<0) {     /*atoi(buff)*/
                syslog(LOGOPTS,"udp_point_to_multipoint_as_multipoint_process local_listen_port_int<0 exit\n");
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            local_as_server_addr.sin_port = (htons(local_listen_port_int));
            /*put the remote_server_port_int from host to internet frame*/
            if ((local_as_server_sockfd = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP))==-1) {/*build a socket*/
                syslog(LOGOPTS,"udp_point_to_multipoint_as_multipoint_process Socket error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            memset(&imreq, 0, sizeof(struct ip_mreq));
            imreq.imr_multiaddr.s_addr = inet_addr(DataBase1_Buff[COM12_SUB_MENU_UPTOMP_AS_MP][UPTOMP_AS_MP_ITEM_2]);
/*set the mp SERVER listen address*/
            imreq.imr_interface.s_addr = INADDR_ANY; /* use DEFAULT interface*/
            while (false==fetch_Local_IP_ADDR(ifname, buff_IP)) {
                syslog(LOGOPTS,"%s: failed to get %s address\n\a",__func__,ifname);
                sleep(1);
            }
            inet_aton(buff_IP,&imreq.imr_interface);

/* JOIN multicast group on default interface*/
            setsockopt(local_as_server_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                       (const void *)&imreq, sizeof(struct ip_mreq));
            if (setsockopt (local_as_server_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))< 0) {
                syslog(LOGOPTS,"udp_point_to_multipoint_as_multipoint_process setsockopt:%s exit\n\a",strerror(errno));         
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }

            if (bind(local_as_server_sockfd,(struct sockaddr*)(&local_as_server_addr),sizeof(struct sockaddr))==-1)
            /*bined the setting with the socket*/
            {
                syslog(LOGOPTS,"udp_point_to_multipoint_as_multipoint_process Bind error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }

#if 0 //this way will use difference source
            /*MP build udp client to send to udp point */
            if ((local_as_client_sockfd = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP))==-1)
            /*build a udp client socket*/
            {
                syslog(LOGOPTS,"udp_point_to_multipoint_as_multipoint_process Socket error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();                      
            }
#else // this way will use same source as binding
            local_as_client_sockfd = local_as_server_sockfd;// 
#endif
            bzero(&remote_as_server_addr,sizeof(struct sockaddr_in));/*set the client_addr*/
            remote_as_server_addr.sin_family = PF_INET;/*internet*/
            remote_as_server_addr.sin_addr.s_addr =inet_addr(DataBase1_Buff[COM12_SUB_MENU_UPTOMP_AS_MP][UPTOMP_AS_MP_ITEM_0]);
            remote_server_port_int=atoi(DataBase1_Buff[COM12_SUB_MENU_UPTOMP_AS_MP][UPTOMP_AS_MP_ITEM_1]);
            //syslog(LOGOPTS,"Remote UDP server Port:%d %s \a\n",remote_server_port_int,DataBase1_Buff[COM12_SUB_MENU_UPTOMP_AS_MP][UPTOMP_AS_MP_ITEM_0] );
            remote_as_server_addr.sin_port = (htons(remote_server_port_int));
            /*put the remote_server_port_int from host to internet frame*/
            com_receive_num_for_send_total =0;
            if (remote_server_port_int<0) {
                syslog(LOGOPTS,"udp_point_to_multipoint_as_multipoint_process remote_server_port_int<0 exit\n"); 
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            if (setsockopt (local_as_client_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))< 0) {
                syslog(LOGOPTS,"udp_point_to_multipoint_as_multipoint_process setsockopt:%s exit\n\a",strerror(errno));  
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }

            com_receive_num_for_send_total =0;
            socket_receive_num_for_send_total =0;
            first_Data_Come=0;
            fcntl (local_as_server_sockfd, F_SETFL, fcntl(local_as_server_sockfd, F_GETFL) | O_NONBLOCK);  /* dont block read*/
            fctl_com(); 
            udp_pmp_as_multipoint_loop:
            while (6) {
                if (set_select_readfd(fdcom,local_as_server_sockfd)==-1) {
                    syslog(LOGOPTS,"udp_point_to_multipoint_as_multipoint_process set_select_readfd==-1 exit\n");           
                    exit_process();/*select return -1 exit*/
                }
                if (first_Data_Come==1) {
                    if (sysinfo(&info)==0)
                        current_time=(int) info.uptime -link_timeout_begin;
                    if (current_time >= UDP_TIMEOUT) {
                        com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='A';
                        write_com_statistics();
                        current_time = 0;
                        //syslog(LOGOPTS,"TCP A/C  connetction timeout disconnect socket ok\n");
                        first_Data_Come=0;  
                    }
                }
                /******************************* read data from serial port *************************/
                if (FD_ISSET(fdcom,&my_readfd)) {
                    if (data_from_com_process(local_as_client_sockfd)==-1) {
                        //goto udp_pmp_as_multipoint_loop;
                        close(local_as_client_sockfd);
                        close(local_as_server_sockfd);
                        return;        
                    }
                }/* end of FD_ISSET read data from com*/
                /******************************* read data from remote server *************************/
                if (FD_ISSET(local_as_server_sockfd,&my_readfd)) {    /**/
                    if (data_from_socket_process(local_as_server_sockfd)==-1) {
                        goto udp_pmp_as_multipoint_loop;
                    }
                }/* end of if fissset*/

                /* if this is last packet from com, then after frame delay send it to socket*/
                if ( com_receive_num_for_send_total >0) {
                    if (now_have_com_data_process(local_as_client_sockfd)==-1) {
                        //goto udp_pmp_as_multipoint_loop;
                        close(local_as_client_sockfd);
                        close(local_as_server_sockfd);
                        return;          
                    }
                }
                /* if this is last packet from socket, then after frame delay send it to com*/
                if ( socket_receive_num_for_send_total >0) {
                    if (now_have_socket_data_process(local_as_server_sockfd)==-1) {
                        syslog(LOGOPTS,"udp_point_to_multipoint_as_multipoint_process exit\n");  
                        exit_process();
                    }
                }/*if ( socket_receive_num_for_send_total >0)*/
            }/*endo of while 6  */
        }


/**********************************************************************************
   Function:    static void udp_multipoint_to_multipoint_process(void) 
   Input:       void
   Output:      None
   Return:      void  
   Note:        bind to a port to listen to and get data from.
        config the other multicast address and port to send data to remote. 
        then process incoming and outgoing data.
**********************************************************************************/
        static void udp_multipoint_to_multipoint_process(void)
        {
            /*build UDP PTOMP Server first */
            bzero(&local_as_server_addr,sizeof(struct sockaddr_in));/*set the server_address to 0*/
            local_as_server_addr.sin_family = AF_INET;/*internet  */
            local_as_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);/*any address can connect*/
            local_listen_port_int=atoi(DataBase1_Buff[COM12_SUB_MENU_UMPTOMP][UMPTOMP_LISTEN_MULTICAST_PORT]);
            /*UDP Multicastlistenport;*/
            //syslog(LOGOPTS,"UDP as mp listen port:%d  \a\n",local_listen_port_int );
            if (local_listen_port_int<0) {     /*atoi(buff)*/
                syslog(LOGOPTS,"udp_multipoint_to_multipoint_process local_listen_port_int<0 exit\n");
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            local_as_server_addr.sin_port = (htons(local_listen_port_int));
            /*put the remote_server_port_int from host to internet frame*/
            if ((local_as_server_sockfd = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP))==-1) {/*build a socket*/
                syslog(LOGOPTS,"udp_multipoint_to_multipoint_process Socket error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            memset(&imreq, 0, sizeof(struct ip_mreq));
            imreq.imr_multiaddr.s_addr = inet_addr(DataBase1_Buff[COM12_SUB_MENU_UMPTOMP][UMPTOMP_LISTEN_MULTICAST_IP]);
/*set the mp SERVER listen address*/
            imreq.imr_interface.s_addr = INADDR_ANY; /* use DEFAULT interface*/
            while (false==fetch_Local_IP_ADDR(ifname, buff_IP)) {
                syslog(LOGOPTS,"%s: failed to get %s address\n\a",__func__,ifname);
                sleep(1);
            }
            inet_aton(buff_IP,&imreq.imr_interface);

/* JOIN multicast group on default interface*/
            setsockopt(local_as_server_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                       (const void *)&imreq, sizeof(struct ip_mreq));
            if (setsockopt (local_as_server_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))< 0) {
                syslog(LOGOPTS,"udp_multipoint_to_multipoint_process setsockopt:%s exit\n\a",strerror(errno));         
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }

            if (bind(local_as_server_sockfd,(struct sockaddr*)(&local_as_server_addr),sizeof(struct sockaddr))==-1)
            /*bined the setting with the socket*/
            {
                syslog(LOGOPTS,"udp_multipoint_to_multipoint_process Bind error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }

#if 0 //this way will use difference source
            /*then build udp PMP AS P  multicast client to send multicast */
            if ((local_as_client_sockfd = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP))==-1)
            /*build a udp client socket*/
            {
                syslog(LOGOPTS,"udp_multipoint_to_multipoint_process Socket error:%s exit\n\a",strerror(errno));
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();                        
            }
#else // this way will use same source as binding
            local_as_client_sockfd = local_as_server_sockfd;
#endif

            bzero(&remote_as_server_addr,sizeof(struct sockaddr_in));/*set the client_addr*/
            remote_as_server_addr.sin_family = PF_INET;/*internet*/
            remote_as_server_addr.sin_addr.s_addr =inet_addr(DataBase1_Buff[COM12_SUB_MENU_UMPTOMP][UMPTOMP_SEND_MULTICAST_IP]);
            //syslog(LOGOPTS,"Remote UDP Multicaster server addR:%s  \a\n",DataBase1_Buff[COM12_SUB_MENU_UPTOMP_AS_P][UPTOMP_AS_P_ITEM_0] );
            remote_server_port_int=atoi(DataBase1_Buff[COM12_SUB_MENU_UMPTOMP][UMPTOMP_SEND_MULTICAST_PORT]);
            //syslog(LOGOPTS,"Remote UDP Multicaster server Port:%d  \a\n",remote_server_port_int );
            remote_as_server_addr.sin_port = (htons(remote_server_port_int));
            /*put the remote_server_port_int from host to internet frame*/
            com_receive_num_for_send_total =0;
            if (remote_server_port_int<0) {     /*atoi(buff)*/
                syslog(LOGOPTS,"udp_multipoint_to_multipoint_process remote_server_port_int:%d exit\n\a",remote_server_port_int);
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                exit_process();
            }
            iaddr.s_addr = INADDR_ANY; /* use DEFAULT interface*/
            /* Set the outgoing interface to DEFAULT*/
            while (false==fetch_Local_IP_ADDR(ifname, buff_IP)) {
                syslog(LOGOPTS,"%s: failed to get %s address\n\a",__func__,ifname);
                sleep(1);
            }
            inet_aton(buff_IP,&iaddr);                    
            setsockopt(local_as_client_sockfd, IPPROTO_IP, IP_MULTICAST_IF, &iaddr,sizeof(struct in_addr));
            ttl = atoi(DataBase1_Buff[COM12_SUB_MENU_UMPTOMP][UMPTOMP_SEND_MULTICAST_TTL]);
            //syslog(LOGOPTS,"ttl:%d  \a\n",ttl );
            /* Set multicast packet TTL ; default TTL is 1*/
            setsockopt(local_as_client_sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
                       sizeof(unsigned char));
            /* dont send multicast traffic to myself  */
            setsockopt(local_as_client_sockfd, IPPROTO_IP, IP_MULTICAST_LOOP,
                       &not_one, sizeof(unsigned char));
            com_receive_num_for_send_total =0;
            socket_receive_num_for_send_total =0;
            first_Data_Come=0;
            /* dont block read*/
            fcntl (local_as_server_sockfd, F_SETFL, fcntl(local_as_server_sockfd, F_GETFL) | O_NONBLOCK); 
            fctl_com(); 
            udp_mptomp_loop:
            while (7) {
                write_com_statistics();
                if (set_select_readfd(fdcom,local_as_server_sockfd)==-1) {
                    syslog(LOGOPTS,"udp_multipoint_to_multipoint_process set_select_readfd==-1 exit\n");
                    exit_process();/*select return -1 exit*/
                }
                if (first_Data_Come==1) {
                    if (sysinfo(&info)==0)
                        current_time=(int) info.uptime -link_timeout_begin;
                    if (current_time >= UDP_TIMEOUT) {
                        com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='A';
                        write_com_statistics();
                        current_time = 0;
                        //syslog(LOGOPTS,"TCP A/C  connetction timeout disconnect socket ok\n");
                        first_Data_Come=0;  
                    }
                }
                if (FD_ISSET(fdcom,&my_readfd)) {
                    if (data_from_com_process(local_as_client_sockfd)==-1) {
                        // goto udp_mptomp_loop;
                        close(local_as_client_sockfd);
                        close(local_as_server_sockfd);
                        return;        
                    }
                }/* end of FD_ISSET read data from com*/
                /******************************* read data from remote server *************************/
                if (FD_ISSET(local_as_server_sockfd,&my_readfd)) {    /**/
                    if (data_from_socket_process(local_as_server_sockfd)==-1) {
                        goto udp_mptomp_loop;
                    }
                }/* end of if fissset*/
                /* if this is last packet from com, then after frame delay send it to socket*/
                if ( com_receive_num_for_send_total >0) {
                    if (now_have_com_data_process(local_as_client_sockfd)==-1) {
                        // goto udp_mptomp_loop;
                        close(local_as_client_sockfd);
                        close(local_as_server_sockfd);
                        return;      
                    }
                }
                /* if this is last packet from socket, then after frame delay send it to com*/
                if ( socket_receive_num_for_send_total >0) {
                    if (now_have_socket_data_process(local_as_server_sockfd)==-1) {
                        syslog(LOGOPTS,"udp_multipoint_to_multipoint_process exit\n");             
                        exit_process();
                    }
                }/*if ( socket_receive_num_for_send_total >0)*/
            }
        }


/**********************************************************************************
   Function:    static void  new_smtp_process(void) 
   Input:       
   Output:     
   Return:     
   Note:       
       
        
**********************************************************************************/ 
        static void  new_smtp_process(void)
        {

            static char buf[BUFSIZ];
            static char my_name[BUFSIZ]="VIP4G";
            static char my_ip[BUFSIZ]="0.0.0.0";
            int s;
            int r;
            struct passwd *pwd;
            char *src_host = NULL;
            char *des_mail = NULL;
            int cache_in = 0;
            char *line_cache = NULL;
            size_t max_buff_size = 1024;
            int send_buff_size =1024;
            int send_time_out = 10;
            size_t pos = 0; 


            syslog(LOGOPTS, "ENTER [%s]\n", __func__);
            DBG_log("[%s] DataBase1_Buff[%d][%d]: %s\n", __func__,
                    COM12_SUB_MENU_SMTP, SMTP_ITEM_MESSAGE_TITLE,
                    DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_MESSAGE_TITLE]);
            DBG_log("[%s] DataBase1_Buff[%d][%d]: %s\n", __func__,
                    COM12_SUB_MENU_SMTP, SMTP_ITEM_HOST,
                    DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_HOST]);
            DBG_log("[%s] DataBase1_Buff[%d][%d]: %s\n", __func__,
                    COM12_SUB_MENU_SMTP, SMTP_ITEM_USERNAME,
                    DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_USERNAME]);
            DBG_log("[%s] DataBase1_Buff[%d][%d]: %s\n", __func__,
                    COM12_SUB_MENU_SMTP, SMTP_ITEM_PASSWORD,
                    DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_PASSWORD]);


            DBG_log("[%s] DataBase1_Buff[%d][%d]: %s\n", __func__,
                    COM12_SUB_MENU_SMTP, SMTP_ITEM_RECEIVER,
                    DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_RECEIVER]);
            DBG_log("[%s] DataBase1_Buff[%d][%d]: %s\n", __func__,
                    COM12_SUB_MENU_SMTP, SMTP_ITEM_BUFF_SIZE,
                    DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_BUFF_SIZE]);
            DBG_log("[%s] DataBase1_Buff[%d][%d]: %s\n", __func__,
                    COM12_SUB_MENU_SMTP, SMTP_ITEM_TIME_OUT,
                    DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_TIME_OUT]);
            DBG_log("[%s] DataBase1_Buff[%d][%d]: %s\n", __func__,
                    COM12_SUB_MENU_SMTP, SMTP_ITEM_TRANSFER_MODE,
                    DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_TRANSFER_MODE]);           


            subject = DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_MESSAGE_TITLE];  
            mime_style = 0;
            usesyslog = 1;
            verbose = 1;
            cache_in = 1;     
            mailhost = DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_HOST];

            des_mail = DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_RECEIVER];
            send_buff_size = atoi(DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_BUFF_SIZE]);
            if (send_buff_size<=0)
                send_buff_size = 1024;
            max_buff_size = send_buff_size;
            send_time_out = atoi(DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_TIME_OUT]);
            if ( send_time_out<=0)
                send_time_out = 10;

            //DBG_log("new_smtp_process in");
            while (8) { /*loop for ever if everything is right*/
                ifname = ifname_bridge;
                fetch_Local_IP_ADDR(ifname, my_ip);

                //src_host = DataBase1_Buff[SUB_MENU_SYSTEM_CFG][SYSTEM_CFG_ITEM_DEVICE_NAME];

                //strcpy(my_name, src_host);

                from_addr = NULL;
                if (from_addr == NULL) {
                    snprintf(buf, BUFSIZ, "%s@%s", my_name, my_ip);
                    from_addr = strdup(buf);
                }

                max_buff_size = send_buff_size;       
                first_Data_Come=0;
                com_receive_num_for_send_total =0;
                socket_receive_num_for_send_total =0;
                /*write com port not active again*/
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='A';
                write_com_statistics();
                if (cache_in) {

                    void safecpy(const char *p)
                    {
                        int k;
                        int l = strlen(p);            
                        char hex_buf[4];
                        memset(hex_buf,0,4);
                        if ('C'==DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_TRANSFER_MODE][0]) {/* Hex*/
                            l=2*l;
                        }

                        if (pos+l >= max_buff_size) {
                            max_buff_size =pos+l+100;
                            line_cache = realloc(line_cache, max_buff_size*sizeof(char));                
                        }
                        if ('C'==DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_TRANSFER_MODE][0]) {/* Hex*/
                            for (k=0;k<l/2;k++) {
                                sprintf(hex_buf,"%02X",p[k]);
                                memmove(line_cache+pos+k*2, hex_buf,2);                   
                            }

                        } else {
                            memcpy(line_cache+pos, p,l);
                        }

                        pos += l;
                    }

                    if (line_cache!=NULL)
                        free(line_cache);
                    if (line_cache==NULL)
                        line_cache = malloc(send_buff_size * sizeof(char));
                    if (line_cache==NULL) {
                        DBG_log("[%s] memory malloc wrong\n", __func__);              
                        exit_process();
                    }

                    memset(line_cache,0,send_buff_size*sizeof(char));
                    if (sysinfo(&info)==0)
                        link_timeout_begin =(int) info.uptime;
                    current_time=0;
                    com_read_once_number=0;
                    pos=0;

                    while ((pos<send_buff_size)&&(current_time<send_time_out)) {/*buffer not full and time not over timeout*/
                        //syslog(LOGOPTS,"wait for data smtp_process");

                        FD_ZERO(&my_readfd);/*clear ,it should not be out side of while*/
                        FD_SET(fdcom,&my_readfd);// set com          

                        if (sysinfo(&info)==0)
                            current_time=(int) info.uptime -link_timeout_begin;

                        tv_100us.tv_sec = 1;  
                        tv_100us.tv_usec =0;           
                        select_return=select(fdcom+1,&my_readfd, NULL,NULL,&tv_100us);
                        if (select_return==-1) {
                            DBG_log("[%s] select error %s\n", __func__, strerror(errno));
                            exit_process();
                        }
                        if (pos==0)
                            if (sysinfo(&info)==0)
                                link_timeout_begin =(int) info.uptime;

                            /* if the first data is from serial port, then local as tcp client , remote as tcp server */
                        if (FD_ISSET(fdcom,&my_readfd)) {
                            memset(com_read_once_buff,0,COM_READ_ONCE_BUFF_LEN);
                            com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN);
                            //DBG_log("[%s] get data %d", __func__, com_read_once_number); 

                            if (com_read_once_number>0) {
                                safecpy(com_read_once_buff);
                                //DBG_log("[%s] get data %d, safecpy %s\n", __func__, com_read_once_number, com_read_once_buff);                   
                            }
                        }
                    }
                    if (com_read_once_buff[com_read_once_number-1]=='.') {
                        memcpy(line_cache+pos, ".\r\n",3); 
                        *(line_cache+pos+3)=0;
                        pos+=3;
                    } else {
                        memcpy(line_cache+pos, "\r\n",2);  
                        *(line_cache+pos+2)=0;     
                        pos+=2;
                    }              
                }

                if ((sfp = fopen("/tmp/email.dat", "w")) == 0) {
                    DBG_log("[%s] new_smtp_process fopen: %s", __func__, strerror(errno));
                    exit_process();
                }

                /* 
                 *  Give out Message header. 
                 */
                fprintf(sfp, "From: %s\r\n", from_addr);
                if (subject)
                    fprintf(sfp, "Subject: %s\r\n", subject);

                if (reply_addr)
                    fprintf(sfp, "Reply-To: %s\r\n", reply_addr);
                if (err_addr)
                    fprintf(sfp, "Errors-To: %s\r\n", err_addr);

                fprintf(sfp, "Sender: %s@%s\r\n", my_name, my_ip);

                fprintf(sfp, "To: %s\r\n", des_mail);
                if (cc_addr)
                    fprintf(sfp, "Cc: %s\r\n", cc_addr);


                fprintf(sfp, "MIME-Version: 1.0\r\n"); 
                fprintf(sfp, "Content-Type: multipart/mixed; boundary=\"bound123\"\r\n");  
                fprintf(sfp, "--bound123\r\n");

                fprintf(sfp, "Content-Type: text/plain; charset=ISO-8859-1 \r\n");  
                //  fprintf(sfp, "Content-Transfer-Encoding: quoted-printable\r\n");  
                fprintf(sfp, "Content-Transfer-Encoding: binary\r\n");  
                fprintf(sfp, "Content-Disposition: inline\r\n");   
                fprintf(sfp, "\r\n");           
                fflush(sfp); 
                if ('B'==DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_TRANSFER_MODE][0]) { /*attach*/
                    fprintf(sfp,"COM Message is attached\r\n");

                    fprintf(sfp, "--bound123\r\n");        

                    fprintf(sfp, "Content-Type: [attatchment type], name=[attatchment]\r\n");        
                    fprintf(sfp, "Content-Transfer-Encoding:  binary\r\n");   
                    fprintf(sfp, "Content-Disposition: inline; filename=COM_Message\r\n");    
                    fprintf(sfp, "\r\n"); 
                    fflush(sfp); 
                } else if ('C'==DataBase1_Buff[COM12_SUB_MENU_SMTP][SMTP_ITEM_TRANSFER_MODE][0]) {/* Hex*/
                    //  fprintf(sfp, "Content-Transfer-Encoding:  binary\r\n");
                }


                /* 
                *  Give out Message body.
                */
                if (cache_in) {
                    fwrite(line_cache, 1, pos, sfp);
                    DBG_log("[%s] write data in email msg body >>>: %s\n", __func__, line_cache);
                    fprintf(sfp, "--bound123--\r\n");
                    //syslog(LOGOPTS,"[%s] email msg >>>>>>>>>>>>>:\n%s\n>>>>>>>>>>>>>>>>>\n", __func__, sfp);    
                    fflush(sfp);            
                } else if (mime_style) {
                    toqp(stdin, sfp);
                } else {
                    fflush(sfp);               
                }            

                if (line_cache)
                    free(line_cache);
                line_cache=NULL;

                fclose(sfp);

                //call shell to send email here
                SubProgram_Start("/bin/serialsmtp", des_mail);
            }

        }



/*
**  examine message from server 
*/
        static void get_response(void)
        {
            char buf[BUFSIZ];

            while (fgets(buf, sizeof(buf), rfp)) {
                buf[strlen(buf)-1] = 0;
                dprintf("%s --> %s\n", mailhost, buf);
                if (!isdigit(buf[0]) || buf[0] > '3') {
                    syslog(LOGOPTS,"unexpected reply: %s", buf);
                    exit_process();
                }
                if (buf[4] != '-')
                    break;
            }
            return;
        }

/*
**  say something to server and check the response
*/
        static void chat(char *fmt, ...)
        {
            va_list ap;     
            va_start(ap, fmt);
            vfprintf(sfp, fmt, ap);
            va_end(ap);

            va_start(ap, fmt);
            dprintf("%s <-- ", mailhost);
            dvprintf(fmt, ap);
            va_end(ap);

            fflush(sfp);
            get_response();
        }

/*
**  transform to MIME-style quoted printable
**
**  Extracted from the METAMAIL version 2.7 source code (codes.c)
**  and modified to emit \r\n at line boundaries.
*/

        static char basis_hex[] = "0123456789ABCDEF";

        static void toqp(FILE *infile, FILE *outfile)
        {
            int c;
            int ct = 0;
            int prevc = 255;

            while ((c = getc(infile)) != EOF) {
                if (   (c < 32 && (c != '\n' && c != '\t'))
                       || (c == '=')
                       || (c >= 127)
                       || (ct == 0 && c == '.')               ) {
                    putc('=', outfile);
                    putc(basis_hex[c >> 4], outfile);
                    putc(basis_hex[c & 0xF], outfile);
                    ct += 3;
                    prevc = 'A'; /* close enough */
                } else if (c == '\n') {
                    if (prevc == ' ' || prevc == '\t') {
                        putc('=', outfile);  /* soft & hard lines */
                        putc(c, outfile);
                        fflush(outfile);
                    }
                    putc(c, outfile);
                    fflush(outfile);
                    ct = 0;
                    prevc = c;
                } else {
                    if (c == 'F' && prevc == '\n') {
                        /*
                         * HORRIBLE but clever hack suggested by MTR for
                         * sendmail-avoidance
                         */
                        c = getc(infile);
                        if (c == 'r') {
                            c = getc(infile);
                            if (c == 'o') {
                                c = getc(infile);
                                if (c == 'm') {
                                    c = getc(infile);
                                    if (c == ' ') {
                                        /* This is the case we are looking for */
                                        fputs("=46rom", outfile);
                                        ct += 6;
                                    } else {
                                        fputs("From", outfile);
                                        ct += 4;
                                    }
                                } else {
                                    fputs("Fro", outfile);
                                    ct += 3;
                                }
                            } else {
                                fputs("Fr", outfile);
                                ct += 2;
                            }
                        } else {
                            putc('F', outfile);
                            ++ct;
                        }
                        ungetc(c, infile);
                        prevc = 'x'; /* close enough -- printable */
                    } else {
                        putc(c, outfile);
                        ++ct;
                        prevc = c;
                    }
                }
                if (ct > 72) {
                    putc('=', outfile);
                    putc('\r', outfile); 
                    putc('\n', outfile);
                    fflush(outfile);
                    ct = 0;
                    prevc = '\n';
                }
            }
            if (ct) {
                putc('=', outfile);
                putc('\r', outfile); 
                putc('\n', outfile);
                fflush(outfile);
            }
            return;
        }




/**********************************************************************************
   Function:    static void sms_process(void) 
   Input:       void
   Output:      None
   Return:      void  
   Note:        listen to get data from com1 and then send sms to phonelist.
        receive sms and send to com1
**********************************************************************************/ 
        static struct gn_statemachine *state;
        static gn_data *data;
        static void busterminate(void)
        {
            gn_lib_phone_close(state);
            gn_lib_phoneprofile_free(&state);
            gn_lib_library_free();
        }

        static void businit(void)
        {
            gn_error err;
            char *configfile = NULL;
            char *configmodel = NULL;
            if ((err = gn_lib_phoneprofile_load_from_file(configfile, configmodel, &state)) != GN_ERR_NONE) {
                DBG_log("[%s] %s\n", __func__, gn_error_print(err));
                DBG_log("[%s]  >>> gn_lib_phoneprofile_load_from_file() return with fatal error,  program exit(2). <<<\n", __func__);
                exit(2);
            }

            /* register cleanup function */
            //atexit(busterminate);
            /* signal(SIGINT, bussignal); */

            if ((err = gn_lib_phone_open(state)) != GN_ERR_NONE) {
                DBG_log("[%s] %s\n", __func__, gn_error_print(err));
                DBG_log("[%s]  >>> gn_lib_phone_open() return with fatal error,  program exit(2). <<<\n", __func__);
                exit(2);
            }
            data = &state->sm_data;
        }

        static bool get_new_sms(char *sms_data, char *phone_number)
        {
            gn_raw_data grawdata;
            gn_sms message;
            gn_error    error = GN_ERR_NONE;
            char *pos;
            char *tmp;
            int i=0;

            if (!sms_data) return false;

            gn_data_clear(data);

            memset(&message, 0, sizeof(gn_sms));

            message.memory_type = gn_str2memory_type("SM");

            message.number = 0;

            data->sms = &message;

            data->raw_data = &grawdata;


            //state->config.connect_script[0]=1;
            state->config.connect_script[0]=0;

            error = gn_smslist_get(data,state);
            DBG_log("[%s] gn_smslist_get() return: %d\n", __func__, error);

            if (error == GN_ERR_NONE) {

                DBG_log("[%s] get smslist context:\n%s\n", __func__, data->raw_data->data);

                tmp = data->raw_data->data;
                pos = tmp;
                while (tmp) {
                    //tmp = strstr(pos,"+MMGL: ");
                    //if (tmp) pos = tmp + strlen("+MMGL: ");
                    tmp = strstr(pos,"+CMGL: ");
                    if (tmp) pos = tmp + strlen("+CMGL: ");
                }
                if (sscanf(pos, "%d,%*d", &data->sms->number) != 1) {
                    if (data->raw_data->data) free(data->raw_data->data);
                    data->raw_data->data=NULL;
                    return false;
                }
                if (data->raw_data->data) free(data->raw_data->data);
                data->raw_data->data=NULL;
                DBG_log("number=%d\n", data->sms->number);

                data->sms->memory_type = GN_MT_SM;
                error = gn_sms_get(data, state);
                if (error == GN_ERR_NONE) {
                    DBG_log("Sender=%s\n",data->sms->remote.number);
                    DBG_log("sms=%s\n",data->sms->user_data[0].u.text);
                    if (DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_ACL][0]=='B') {
                        phone_number[0]=0;
                        for (i=0;i<5;i++) {
                            if (DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i][0]==0) continue;
                            if (strstr(DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i], data->sms->remote.number)) {
                                DBG_log("phone%d=%s=%s\n",i,DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i],data->sms->remote.number);
                                strcpy(phone_number, data->sms->remote.number);
                                break;
                            } else if (strstr(data->sms->remote.number, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i])) {
                                DBG_log("phone%d=%s=%s\n",i,DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i],data->sms->remote.number);
                                strcpy(phone_number, data->sms->remote.number);
                                break;
                            }
                        }
                        if (phone_number[0]!=0) goto returnsms;
                        return false;
                    }
                    strcpy(phone_number, data->sms->remote.number);
                    returnsms:
                    strcpy(sms_data,data->sms->user_data[0].u.text);
                    if (DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_GETSMS_MODE][0]=='B') {
                        error = gn_sms_delete(data, state);
                        if (error == GN_ERR_NONE) DBG_log("Deleted smsid=%d\n",data->sms->number);
                    }
                    return true;
                }
            }
            return false;
        }

        static void send_sms(char *phone_number, char *sms_data, int len)
        {
            gn_sms sms;
            gn_error error;
            gn_data_clear(data);
            memset(&sms, 0, sizeof(gn_sms));
            data->sms=&sms;
            sms.user_data[0].type = GN_SMS_DATA_Text;
            gn_number_sanitize(phone_number, 32);
            strcpy(data->sms->remote.number, phone_number);

            if (data->sms->remote.number[0] == '+')
                data->sms->remote.type = GN_GSM_NUMBER_International;
            else
                data->sms->remote.type = GN_GSM_NUMBER_Unknown;

            strcpy(sms.user_data[0].u.text, sms_data);
            sms.user_data[0].length = strlen(sms_data);
            data->message_center = calloc(1, sizeof(gn_sms_message_center));
            data->message_center->id = 1;
            if (gn_sm_functions(GN_OP_GetSMSCenter, data, state) == GN_ERR_NONE) {
                snprintf(sms.smsc.number, sizeof(sms.smsc.number), "%s", data->message_center->smsc.number);
                sms.smsc.type = data->message_center->smsc.type;
            } else {
                DBG_log("Cannot read the SMSC number from your phone. If the sms send will fail, please use --smsc option explicitely giving the number.\n");
                free(data->message_center);
                return;
            }
            free(data->message_center);


            if (!sms.smsc.type) sms.smsc.type = GN_GSM_NUMBER_Unknown;
            DBG_log("Sending SMS to %s (text: %s)\n", data->sms->remote.number, data->sms->user_data[0].u.text);

            /* FIXME: set more SMS fields before sending */
            error = gn_sms_send(data, state);

            if (error == GN_ERR_NONE)
                DBG_log("\r\n+CMGS: 0\r\n");

        }

        static void sms_string_out(int fd, char *buffer, int length)
        {
            int count = 0;
            char    out_char;

            while (count < length) {

                /* Translate CR/LF/BS as appropriate */
                switch (buffer[count]) {
                case '\r':
                    out_char = 13;
                    if (buffer[count+1]!='\n') {
                        if (write(fd, &out_char, 1) < 0) {
                            syslog(LOGOPTS, "Failed to output string to com port.\n");
                            return;
                        }
                        out_char = 10;
                    }
                    break;
                case '\n':
                    if ((count>0)&&(buffer[count-1]!='\r')) {
                        out_char = 13;
                        if (write(fd, &out_char, 1) < 0) {
                            syslog(LOGOPTS, "Failed to output string to com port.\n");
                            return;
                        }
                    }
                    out_char = 10;
                    break;
                case '\b':
                    out_char = 8;
                    break;
                default:
                    out_char = buffer[count];
                    break;
                }

                if (write(fd, &out_char, 1) < 0) {
                    syslog(LOGOPTS, "Failed to output string to com port.\n");
                    return;
                }
                count++;
            }
        }
        static void  sms_process(void)
        {

            static char buf[BUFSIZ];
            int i;
            char *line_cache = NULL;
            size_t max_buff_size = 160;
            int send_buff_size =160;
            int send_time_out = 10;
            int com_time_out = 0;
            int receive_begin = 0;
            int send_time = 0;
            size_t pos = 0; 
            int fd; 
            int zflag = 0;  
            bool reply_time = false;
            char rphone[32];
            FILE* fp;

            syslog(LOGOPTS, "ENTER %s \n", __func__);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS,SMS_ITEM_PHONE1, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_PHONE2,DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE2]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_PHONE3, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE3]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_PHONE4, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE4]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_MMS, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE5]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_PHONE5,DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_MMS]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_TIMEOUT, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_TIMEOUT]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_ACL, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_ACL]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_GETSMS_MODE, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_GETSMS_MODE]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_SENDER, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_SENDER]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_SUBJECT, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_SUBJECT]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_DT, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_DT]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_LOC, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_LOC]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_SM_TOTAL, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_SM_TOTAL]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_SM_USED, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_SM_USED]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_ME_TOTAL, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_ME_TOTAL]);
            // syslog(LOGOPTS, "[%s] DataBase1_Buff[%d][%d]: %s\n", __func__, COM12_SUB_MENU_SMS, SMS_ITEM_ME_USED, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_ME_USED]);



            /*get customer's opt*/
            sprintf(rphone,"/etc/soipdebug");
            if ((fp=fopen(rphone,"r"))==NULL) {
                syslog(LOGOPTS,"%s %s not exists\n",__func__,rphone);
                debug=0;
            } else {
                debug=1;
                fclose(fp);
            }

            send_buff_size = atoi(DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_MMS]);
            if (send_buff_size<=0)
                send_buff_size = 160;
            max_buff_size = send_buff_size;
            send_time_out = atoi(DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_TIMEOUT]);
            com_time_out = atoi(DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_CHARACTER_TIMEOUT]);
            if ( send_time_out<=0)
                send_time_out = 10;
            DBG_log("[%s] send_buff_size = %d, send_time_out = %d, com_time_out = %d", __func__, send_buff_size,send_time_out,com_time_out);

            for (i=0;i<5;i++) {
                gn_number_sanitize(DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i], 32);
                DBG_log("phone%d=%d%d\n",i, DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i][0], DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i][1]);                
            }
            rphone[0]=0;

            while (8) { /*loop for ever if everything is right*/
                max_buff_size = send_buff_size;       
                //first_Data_Come=0;
                com_receive_num_for_send_total =0;
                socket_receive_num_for_send_total =0;
                /*write com port not active again*/
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='A';                
                write_com_statistics();


                void safecpy(const char *p)
                {
                    int l = strlen(p);  
                    DBG_log("safecopy %d characters",l);          
                    if (pos+l >= max_buff_size) {
                        max_buff_size = pos+l;
                        line_cache = realloc(line_cache, max_buff_size*sizeof(char)); 
                    }

                    memcpy(line_cache+pos, p,l);

                    pos += l;
                } //end of safecpy

                if (line_cache!=NULL)
                    free(line_cache);
                if (line_cache==NULL)
                    line_cache = malloc(send_buff_size * sizeof(char));
                if (line_cache==NULL) {
                    syslog(LOGOPTS,"memory malloc wrong\n");              
                    exit_process();
                }
                memset(line_cache,0,send_buff_size*sizeof(char));
                if (sysinfo(&info)==0)
                    link_timeout_begin =(int) info.uptime;
                current_time=0;
                com_read_once_number=0;
                pos=0;
                if (com_time_out==0) {
                    DBG_log("com_time_out==0 so set zflag==1");
                    zflag=1;
                    com_time_out=1;
                }

                /*buffer not full and time not over timeout*/
                while ((pos<send_buff_size)&&(current_time<com_time_out)) {
                    syslog(LOGOPTS,"[%s] wait for data", __func__);

                    FD_ZERO(&my_readfd);/*clear ,it should not be out side of while*/
                    FD_SET(fdcom,&my_readfd);// set com          

                    if (zflag==1) tv_100us.tv_sec = 0;
                    else tv_100us.tv_sec = 1;  
                    tv_100us.tv_usec =0;           
                    select_return=select(fdcom+1,&my_readfd, NULL,NULL,&tv_100us);
                    if (select_return==-1) {
                        syslog(LOGOPTS,"sms_process: select error %s\n",strerror(errno));
                        exit_process();
                    }


                    /* if the first data is from serial port*/
                    if (FD_ISSET(fdcom,&my_readfd)) {
                        memset(com_read_once_buff,0,COM_READ_ONCE_BUFF_LEN);
                        com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN);
                        DBG_log("[%s] get %d characters from com", __func__, com_read_once_number);                         

                        if (com_read_once_number>0) {
                            safecpy(com_read_once_buff);  
                            if (sysinfo(&info)==0)
                                link_timeout_begin =(int) info.uptime;
                        }
                    }

                    if (zflag==1) {
                        zflag=0;
                        com_time_out=0;
                        DBG_log("zflag==1 break while of waiting com data");
                        break;
                    }
                    if (sysinfo(&info)==0)
                        current_time=(int) info.uptime -link_timeout_begin;
                }//end of waiting for data from com while



                if (pos<0) {
                    syslog(LOGOPTS,"%s soip sms_process pos<0",__func__);
                    if (line_cache)
                        free(line_cache);
                    line_cache=NULL;
                    exit_process();
                } else if (pos==0) {
                    //DBG_log("pos==0 goto getsms and send them to com port");
                    //DBG_log("[%s] pos==0 goto getsms and send them to com port" ,__func__);
                    goto getsms;
                }
                DBG_log("line_cache=%s",line_cache);
                /*send data in line_cache using sms which max size is send_buff_size per sms*/


                if (max_buff_size > send_buff_size ) {
                    int lpos = 0;
                    int bsize = 0;
                    char *lsend_cache = NULL;
                    lsend_cache = malloc(send_buff_size * sizeof(char));

                    if (lsend_cache==NULL) {
                        syslog(LOGOPTS,"lsend-cache memory malloc wrong\n");
                        if (line_cache) free(line_cache);
                        line_cache=NULL;
                        exit_process();
                    }
                    while (lpos<max_buff_size) {
                        if (lpos+send_buff_size>max_buff_size) {
                            bsize = max_buff_size - lpos;
                            lsend_cache = realloc(lsend_cache,bsize * sizeof(char));
                        } else
                            bsize = send_buff_size;

                        if (lsend_cache==NULL) {
                            syslog(LOGOPTS,"lsend-cache memory malloc wrong\n");  
                            if (line_cache) free(line_cache);
                            line_cache=NULL;            
                            exit_process();
                        }
                        memset(lsend_cache,0,bsize * sizeof(char));
                        memcpy(lsend_cache,line_cache+lpos,bsize * sizeof(char));
                        strncpy(lsend_cache,line_cache+lpos,bsize);
                        fd = open ( RadioBusyFlagFile, O_WRONLY);
                        if (fd<0) {
                            syslog(LOGOPTS,"%s Soip sms_process open %s:fd<0\n",__func__,RadioBusyFlagFile);
                            continue;
                        }
                        LockFile(fd);
                        businit();
                        DBG_log("[%s] soip sms_process lock %s Ok!",__func__,RadioBusyFlagFile);


                        if (rphone[0]!=0) {
                            DBG_log("lsend sms:%s to rphone:%s total:%d characters",lsend_cache,rphone,bsize);

                            send_sms(rphone, lsend_cache, bsize);
                        } else {
                            for (i=0;i<5;i++) {
                                if (strcmp(DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i],"")==0) {
                                    continue;
                                }
                                DBG_log("lsend sms:%s to phone:%s total:%d characters",lsend_cache,DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i],bsize);

                                send_sms(DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i], lsend_cache, bsize);
                            }

                        }
                        busterminate();
                        UnLockFile(fd);
                        DBG_log("%s soip sms_process unlock %s Ok!",__func__,RadioBusyFlagFile);
                        lpos+=bsize;
                    }

                    if (lsend_cache)
                        free(lsend_cache);
                    lsend_cache=NULL;
                } else {
                    if (pos>0) {
                        fd = open ( RadioBusyFlagFile, O_WRONLY);
                        if (fd<0) {
                            syslog(LOGOPTS,"%s Soip sms_process open %s:fd<0\n",__func__,RadioBusyFlagFile);
                            continue;
                        }
                        LockFile(fd);
                        businit();
                        DBG_log("%s soip sms_process lock %s Ok!",__func__,RadioBusyFlagFile);


                        if (rphone[0]!=0) {
                            DBG_log("send sms:%s to rphone:%s total:%d characters",line_cache,rphone,pos);

                            send_sms(rphone, line_cache, pos);
                            // statistics code for future 
                            //com_Data_total_int_buff[COM_STATISTICS_SEND_BYTES] += pos;
                            //com_Data_total_int_buff[COM_STATISTICS_SEND_PACKETS] += 1;
                            //sprintf(com_Data_total_str_buff[COM_STATISTICS_SEND_BYTES],"%u",com_Data_total_int_buff[COM_STATISTICS_SEND_BYTES]);
                            //sprintf(com_Data_total_str_buff[COM_STATISTICS_SEND_PACKETS],"%u",com_Data_total_int_buff[COM_STATISTICS_SEND_PACKETS]);
                            //write_com_statistics();
                        } else {
                            for (i=0;i<5;i++) {
                                if (strcmp(DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i],"")==0) {
                                    continue;
                                }

                                DBG_log("send sms:%s to phone:%s total:%d characters",line_cache,DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i],pos);

                                send_sms(DataBase1_Buff[COM12_SUB_MENU_SMS][SMS_ITEM_PHONE1+i], line_cache, pos);
                                // statistics code for future 
                                //com_Data_total_int_buff[COM_STATISTICS_SEND_BYTES] += pos;
                                //com_Data_total_int_buff[COM_STATISTICS_SEND_PACKETS] += 1;
                                //sprintf(com_Data_total_str_buff[COM_STATISTICS_SEND_BYTES],"%u",com_Data_total_int_buff[COM_STATISTICS_SEND_BYTES]);
                                //sprintf(com_Data_total_str_buff[COM_STATISTICS_SEND_PACKETS],"%u",com_Data_total_int_buff[COM_STATISTICS_SEND_PACKETS]);
                                //write_com_statistics();
                            }

                        }
                        busterminate();
                        UnLockFile(fd);
                        DBG_log("%s soip sms_process unlock %s Ok!",__func__,RadioBusyFlagFile);
                    }
                }



                /*receive sms and write to serial port*/
                getsms:

                //DBG_log("[%s] >>>>>> getsms \n", __func__);                

                fd = open ( RadioBusyFlagFile, O_WRONLY);
                if (fd<0) {
                    syslog(LOGOPTS,"%s Soip sms_process open %s:fd<0\n",__func__,RadioBusyFlagFile);
                    continue;
                }
                LockFile(fd);
                businit();
                //DBG_log("[%s]  getsms after businit()\n", __func__);// /////////////////

                char line[160];
                bool error = get_new_sms(line, rphone);
                busterminate();
                UnLockFile(fd);
                //DBG_log("[%s] >>>>>> get_new_sms() return %d, line:%s\n", __func__, error, line);
                //DBG_log("[%s]  getsms after busterminate()\n", __func__);// /////////////////
                if (error) {
                    DBG_log("[%s] write len=%d data:%s : to >>> com port\n", __func__, strlen(line), line);                    

                    sms_string_out(fdcom,line,strlen(line));

                    // statistics code for future 
                    //com_Data_total_int_buff[COM_STATISTICS_RECEIVED_BYTES] += strlen(line);
                    //com_Data_total_int_buff[COM_STATISTICS_RECEIVED_PACKETS] += 1;
                    //sprintf(com_Data_total_str_buff[COM_STATISTICS_RECEIVED_BYTES],"%u",com_Data_total_int_buff[COM_STATISTICS_RECEIVED_BYTES]);
                    //sprintf(com_Data_total_str_buff[COM_STATISTICS_RECEIVED_PACKETS],"%u",com_Data_total_int_buff[COM_STATISTICS_RECEIVED_PACKETS]);
                    //write_com_statistics();

                    if (sysinfo(&info)==0)
                        receive_begin =(int) info.uptime;
                    reply_time = true;
                } else {   //DBG_log("[%s]  getsms else branch\n", __func__);// /////////////////
                    if (reply_time) {

                        if (sysinfo(&info)==0)
                            send_time=(int) info.uptime -receive_begin;

                        if (send_time >= send_time_out) {
                            rphone[0]=0;
                            DBG_log("send_time = %d",send_time);
                            reply_time = false;
                        }
                    }
                }
                if (line_cache)
                    free(line_cache);
                line_cache=NULL;     
            }//end of while8

        }

        int savesms_to_file(char *filename, char *str, int len)
        {
            FILE* sms_fp;
            int wcount;

            DBG_log("str=%s len=%d",str,len);
            if (sizeof(str)>160) {
                syslog(LOGOPTS,"savesms_to_file :the sms size is longer than 160 charactors");
                return false;
            }
            if (!(sms_fp = fopen(filename,"w+"))) {
                syslog(LOGOPTS,"savesms_to_file :can not open %s to write \n",filename); 
                return false;          
            }
            wcount=fwrite(str,sizeof(char),len,sms_fp);
            DBG_log("savesms_to_file: fwrite to file and return %d",wcount);
            fclose(sms_fp);
            return true;

        }
        size_t
        sanitize_string(char *buf, size_t size)
        {
#define UGLY_WIDTH	4	/* width for ugly character: \OOO */
            size_t len;
            size_t added = 0;
            char *p;


            /* find right side of string to be sanitized and count
             * number of columns to be added.  Stop on end of string
             * or lack of room for more result.
             */
            for (p = buf; *p != '\0' && &p[added] < &buf[size - UGLY_WIDTH]; p++) {
                unsigned char c = *p;

                /* exception is that all veritical space just becomes white space */
                if (c == '\n' || c == '\r') {
                    *p = ' ';
                    continue;
                }

                if (c == '\\' || !isprint(c))
                    added += UGLY_WIDTH - 1;
            }

            /* at this point, p points after last original character to be
             * included.  added is how many characters are added to sanitize.
             * so p[added] will point after last sanitized character.
             */

            p[added] = '\0';
            len = &p[added] - buf;

            /* scan backwards, copying characters to their new home
             * and inserting the expansions for ugly characters.
             *
             * vertical space is changed to horizontal.
             *
             * It is finished when no more shifting is required.
             * This is a predecrement loop.
             */
            while (added != 0) {
                char fmtd[UGLY_WIDTH + 1];
                unsigned char c;

                while ((c = *--p) != '\\' && isprint(c))
                    p[added] = c;

                added -= UGLY_WIDTH - 1;
                snprintf(fmtd, sizeof(fmtd), "\\%03o", c);
                memcpy(p + added, fmtd, UGLY_WIDTH);
            }
            return len;
#undef UGLY_WIDTH
        }
        /* log a debugging message (prefixed by "| ") */

        int
        soip_DBG_log(const char *message, ...)
        {
            if (debug==0) return 0;
            va_list args;
            char m[LOG_WIDTH];  /* longer messages will be truncated */

            va_start(args, message);
            vsnprintf(m, sizeof(m), message, args);
            va_end(args);

            /* then sanitize anything else that is left. */
            (void)sanitize_string(m, sizeof(m));

            syslog(LOG_DEBUG, "Soip| %s", m);

            return 0;
        }




static void gps_com_out(int fd, char *buffer, int length)
{
    int count = 0;
    char    out_char;

    while(count<length)
    {
        if(buffer[count]=='$')break;
        count++;
    }

    while (count < length) {

        /* Translate CR/LF/BS as appropriate */
        switch (buffer[count]) {
        case '\r':
            out_char = 13;
            if (buffer[count+1]!='\n') {
                if (write(fd, &out_char, 1) < 0) {
                    syslog(LOGOPTS, "Failed to output string to com port.\n");
                    return;
                }
                out_char = 10;
            }
            break;
        case '\n':
            if ((count>0)&&(buffer[count-1]!='\r')) {
                out_char = 13;
                if (write(fd, &out_char, 1) < 0) {
                    syslog(LOGOPTS, "Failed to output string to com port.\n");
                    return;
                }
            }
            out_char = 10;
            break;
        case '\b':
            out_char = 8;
            break;
        default:
            out_char = buffer[count];
            break;
        }

        if (write(fd, &out_char, 1) < 0) {
            syslog(LOGOPTS, "Failed to output string to com port.\n");
            DBG_log("send err:%d\n",length);
            return;
        }
        count++;
    }
    DBG_log("send gps:%d\n",length);

}


/*
 * connect to gpsd with socket
 */
static int netlib_connectsock(const char *host, const char *service, const char *protocol)
{
    struct hostent *phe;
    struct servent *pse;
    struct protoent *ppe;
    struct sockaddr_in sin;
    int s, type, proto, one = 1;

    memset((char *) &sin, 0, sizeof(sin));
    /*@ -type -mustfreefresh @*/
    sin.sin_family = AF_INET;
    if ((pse = getservbyname(service, protocol)))
        sin.sin_port = htons(ntohs((unsigned short) pse->s_port));
    else if ((sin.sin_port = htons((unsigned short) atoi(service))) == 0)
        return -3;

    if ((phe = gethostbyname(host)))
        memcpy((char *) &sin.sin_addr, phe->h_addr, phe->h_length);
    else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
        return -2;

    ppe = getprotobyname(protocol);
    if (strcmp(protocol, "udp") == 0) {
        type = SOCK_DGRAM;
        proto = (ppe) ? ppe->p_proto : IPPROTO_UDP;
    } else {
        type = SOCK_STREAM;
        proto = (ppe) ? ppe->p_proto : IPPROTO_TCP;
    }

    if ((s = socket(PF_INET, type, proto)) < 0)
        return -4;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one))==-1) {
        (void)close(s);
        return -5;
    }
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        (void)close(s);
        return -6;
    }

#ifdef IPTOS_LOWDELAY
    int opt = IPTOS_LOWDELAY;
    (void)setsockopt(s, IPPROTO_IP, IP_TOS, &opt, sizeof opt);

#endif
#ifdef TCP_NODELAY
    if (type == SOCK_STREAM)
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
#endif
    return s;

}


//init
int gpsd_retry_times=0;
int gps_reconnect_times=0;
static int gps_process_init(void)
{

    char gps_status[5],tmp_buff[32];
    int first_step=0;

reinit_gps:
    gpsd_retry_times++;
    if(gpsd_retry_times>20)
    {
        gpsd_retry_times=0;
        SubProgram_Start("/etc/init.d/gpsd","stop");
        sleep(1);
        SubProgram_Start("/etc/init.d/gpsd","start");
    }

    if(first_step!=0) {
        uci_free_context(ctx);
        sleep(5);
        ctx = uci_alloc_context();
        if (!ctx) 
        {
            fprintf(stderr, "Out of memory\n");
            exit(-1);
        }
    }
    first_step=1;

    gps_status[0]='0';
    get_option_by_section_name(ctx, "gpsd","gpsd_conf","status",gps_status);

    if(gps_status[0]!='1')        
        goto reinit_gps;

    strcpy(gps_server,DEFAULT_GPSD_SERVER);
    get_option_by_section_name(ctx, "gpsd","gpsd_conf","GPSD_TCP_Port",gps_port);
    
       
reconnect:
    gps_reconnect_times++;
    if(gps_reconnect_times>15)
    {
        gps_reconnect_times=0;
        goto reinit_gps;
    }
    sock = netlib_connectsock( gps_server, gps_port, "tcp");
    if (sock < 0) {
        syslog(LOGOPTS,"GPS COM Error: could not connect to gpsd %s:%s\n",gps_server, gps_port);
        goto reinit_gps;
    }   
   
    int readbytes = 0;

    bzero(com_total_buff,sizeof(com_total_buff));
    bzero(cbuf,sizeof(cbuf));
    strcpy(cbuf, "r=1");
    wrote = write(sock, cbuf, strlen(cbuf));
    if ((ssize_t)strlen(cbuf) != wrote) 
    {
        syslog(LOGOPTS,"GPS COM send first message: ship r=1: write error\n");
        shutdown(sock,SHUT_RDWR);
        close(sock);
        syslog(LOGOPTS,"GPS COM: shutdown sock and reconnect\n");
        goto reconnect;
    }
    sleep(1);
    readbytes = (int)read(sock, com_total_buff, sizeof(com_total_buff));
    if (readbytes >0) {
        com_total_buff[readbytes]='\0';
        bzero(cbuf,sizeof(cbuf));
        strcpy(cbuf, "r=0");
        wrote = write(sock, cbuf, strlen(cbuf));
        if ((ssize_t)strlen(cbuf) != wrote) {
            syslog(LOGOPTS,"GPS COM send first message: ship r=0: write error\n");

            shutdown(sock,SHUT_RDWR);
            close(sock);
            syslog(LOGOPTS,"GPS COM: shutdown sock and reconnect\n");
            goto reconnect;
        }
    }

    return 1;
}

static void gps_process(void)
{

    int readbytes = 0;
    int plus_num=0;
    int data_invalid_times=0;
    FD_ZERO(&my_readfd);/*clear ,it should not be out side of while*/
    FD_SET(fdcom,&my_readfd);// set com          

while(1) 
{

    strcpy(cbuf, "r=1");
    wrote = write(sock, cbuf, strlen(cbuf));
    if ((ssize_t)strlen(cbuf) != wrote) {
        syslog(LOGOPTS,"GPS COM send first message: ship r=1: write error\n");
        shutdown(sock,SHUT_RDWR);
        close(sock);
        syslog(LOGOPTS,"GPS COM: shutdown sock and reconnect\n");
        goto re_init_gps;
    }


    FD_ZERO(&my_readfd);/*clear ,it should not be out side of while*/
    FD_SET(fdcom,&my_readfd);// set com          
    tv_100us.tv_sec = 1;  
    tv_100us.tv_usec =0;           
    select_return=select(fdcom+1,&my_readfd, NULL,NULL,&tv_100us);
    if (select_return==-1) 
    {
        syslog(LOGOPTS,"gps_process: select error %s\n",strerror(errno));
        exit_process();
    }

    com_read_once_number=0;
    if (FD_ISSET(fdcom,&my_readfd)) 
    {
        memset(com_read_once_buff,0,COM_READ_ONCE_BUFF_LEN);
        com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN);
        //syslog(LOGOPTS,"gps_process: buff:%d:%s********%s\n",com_read_once_number,com_read_once_buff,strerror(errno));

        if (com_read_once_number>0) 
        {
            int i=0;
            while(i<com_read_once_number)
            {
                if (com_read_once_buff[i]=='+') 
                {
                    plus_num++;
                } else 
                {
                    plus_num=0;
                }

                if (plus_num>=3) 
                {
                      plus_num=0;
                      exit_process();
                      return;/*quit soipd2*/
                 }
                i++;
            }
        }
    }


    readbytes = 0;
    sleep(1);
    readbytes = (int)read(sock, com_total_buff, sizeof(com_total_buff));
    if (readbytes >0) {
        com_total_buff[readbytes]='\0';
        bzero(cbuf,sizeof(cbuf));
        strcpy(cbuf, "r=0");
        wrote = write(sock, cbuf, strlen(cbuf));
        if ((ssize_t)strlen(cbuf) != wrote) {
            syslog(LOGOPTS,"GPS COM send first message: ship r=0: write error\n");
            shutdown(sock,SHUT_RDWR);
            close(sock);
            syslog(LOGOPTS,"GPS COM: shutdown sock and reconnect\n");
            goto re_init_gps;
        }
        if(readbytes<21)
        {
            data_invalid_times++;
            if(data_invalid_times>20) 
            {
                data_invalid_times=0;
                goto re_init_gps;
            }
        }else
        {
            data_invalid_times=0;
            gps_reconnect_times=0;
            gpsd_retry_times=0;
        }
        gps_com_out(fdcom,com_total_buff,readbytes);
        continue;
    }

    re_init_gps:
        gps_process_init();
        data_invalid_times=0;
}//while
}




/**********************************************************************************
   Function:    int data_from_com_process(int socket)
   Input:       int socket
   Output:      None
   Return:      int int -1 :error , 0: ok.  
   Note:       	used when new data come from com port. If time has over one frame.
        write last data from com port to socket and update statistics, at
        the mean time, save the new data to nexe frame. If over inner fame
        delay, throw this data and last data away.If not, this data still
        belong to last frame. save it. 
**********************************************************************************/
        int data_from_com_process(int socket)
        {
            DBG_log("ENTER [%s]\n", __func__);
            if (first_Data_Come==0) {
                gettimeofday(&tv_delay,&tz);
                read_com_data_start_sec = tv_delay.tv_sec;
                read_com_data_start_usec = tv_delay.tv_usec;       
                first_Data_Come=1;
            }
            if (sysinfo(&info)==0)
                link_timeout_begin =(int) info.uptime;

            com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='B';
            //syslog(LOGOPTS,"data_from_com_process begin\n");
            if (Timing) {
                com_time_pass_us = time_elapse(read_com_data_start_sec, read_com_data_start_usec);
                //syslog(LOGOPTS,"com_time_pass_us: %d\n",com_time_pass_us);
                gettimeofday(&tv_delay,&tz);
                read_com_data_start_sec = tv_delay.tv_sec;
                read_com_data_start_usec = tv_delay.tv_usec;
            }
            /*frame timeout,means alread get a whole frame,write last frame to socket.*/
            if (com_time_pass_us>Timing_out_frame_delay) {
                if ( com_receive_num_for_send_total>0) {
                    //syslog(LOGOPTS," read com data timeout, write %d to server \n",com_receive_num_for_send_total);
                    /*after frame delay, send it to server*/
#ifdef DEBUG_PRINT_INPUT
                    write_to_serial_port(f_int1,com_total_buff,com_receive_num_for_send_total);
#endif

                    if (write_to_socket_process(socket)==-1) {
                        syslog(LOGOPTS,"data_from_com_process return -1 \n");  
                        return -1;
                    }
                }/*if ( com_receive_num_for_send_total>0)*/
                /* the getted data dont belong to this frame. so save it to next.*/
                com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN);
                if (com_read_once_number<0) {
                    syslog(LOGOPTS,"\ndata_from_com_process local as client com_read_once_number <0:%d exit\n",com_read_once_number);  
                    exit_process();
                    //exit_process();
                    com_read_once_number=0;
                }
                if (com_read_once_number==0) {
                    // syslog(LOGOPTS,"\ndata_from_com_process local as client com_read_once_number ==0 exit\n");  
                    //exit_process();
                    com_read_once_number=0;
                } else {
                    //syslog(LOGOPTS," read com get %d data from com\n",com_read_once_number);
                    /*if not Timing write to server then continue while*/

                    memcpy(com_total_buff, com_read_once_buff,com_read_once_number);
                    com_receive_num_for_send_total = com_read_once_number;   
#if defined(IXP425COM2) || defined(MPCI2)
                    com2_check_three_plus();
#endif
#ifdef IXP425USB
                    usb_check_three_plus();
#endif
                    if (Timing_out_frame_delay==0) {
                        if (write_to_socket_process(socket)==-1) {
                            syslog(LOGOPTS,"data_from_com_process return -1 \n");  
                            return -1;
                        }
                    }
                }/*if ((com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, max_packet_size))>0)*/
            }/*end of if (com_time_pass_us>Timing_out_frame_delay)*/
            else if (com_time_pass_us>Timing_in_frame_delay)/* didnt over frame delay, so it also  belong to this frame*/
            /*if it over character timeout, then discard it.*/
            {
                if ((com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN))>0) {
#if defined(IXP425COM2) || defined(MPCI2) 
                    com2_check_three_plus();
#endif 
#ifdef IXP425USB
                    usb_check_three_plus();
#endif 




                    //syslog(LOGOPTS,"read com get %d data from com\n",com_read_once_number);
                    /*update droped bytes and packet*/
                    com_Data_total_int_buff[COM_STATISTICS_SENT_DROPED_BYTES] += com_read_once_number;
                    com_Data_total_int_buff[COM_STATISTICS_SENT_DROPED_PACKETS] += 1;
                    sprintf(com_Data_total_str_buff[COM_STATISTICS_SENT_DROPED_BYTES],"%u",com_Data_total_int_buff[COM_STATISTICS_SENT_DROPED_BYTES]);  
                    sprintf(com_Data_total_str_buff[COM_STATISTICS_SENT_DROPED_PACKETS],"%u",com_Data_total_int_buff[COM_STATISTICS_SENT_DROPED_PACKETS]);                                                
                    write_com_statistics();
                    com_receive_num_for_send_total = 0; /* clear buffer to discard data. */
                } else {/*serial port is broken close socket and exit*/
                    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]<='C') {
                        shutdown(socket,SHUT_RDWR);  
                        close(socket);
                    }
                    close(fdcom);
                    syslog(LOGOPTS,"data_from_com_process read com <=0 :%d\n",com_read_once_number);
                    exit_process();
                }
                //syslog(LOGOPTS,"read com character with character time out \n");
                /* get a whole message,jump out of loop*/
            } else {
                //syslog(LOGOPTS,"read com not timeout\n");
                /* save it to buffer.*/
                if ((com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN))>0) {
                    //syslog(LOGOPTS,"read com get %d data from com\n",com_read_once_number);
                    memcpy(&com_total_buff[com_receive_num_for_send_total], com_read_once_buff,com_read_once_number);
                    com_receive_num_for_send_total += com_read_once_number;
#if defined(IXP425COM2)|| defined(MPCI2)
                    com2_check_three_plus();
#endif      
#ifdef IXP425USB
                    usb_check_three_plus();
#endif      
                } else {/*serial port is broken close socket and exit*/
                    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]<='C') {
                        shutdown(socket,SHUT_RDWR);  
                        close(socket);
                    }
                    close(fdcom);
                    syslog(LOGOPTS,"data_from_com_process read com <=0 exit\n");
                    exit_process();
                }
                if (com_receive_num_for_send_total >=max_packet_size) {  /* force out.*/
                    receive_more_than_max =1;
                    //syslog(LOGOPTS,"read com data,not timeout,but buffer full write %d to socket\n",com_receive_num_for_send_total);
                    if (write_to_socket_process(socket)==-1) {
                        syslog(LOGOPTS,"%s==-1 exit\n",__func__);              
                        exit_process();
                    }
                    receive_more_than_max =0;            
                }
            }/*end of else*/     
            //syslog(LOGOPTS,"data_from_com_process end\n");
            return 0;
        }

/**********************************************************************************
   Function:    int data_from_socket_process(int socket)
   Input:       int socket
   Output:      None
   Return:      int int -1 :error , 0: ok.  
   Note:       	used when new data come. If time has over one frame.write last data from 
        socket to com port and update statistics, at the mean time, save the new data
        to nexe frame. If over inner fame delay, throw this data and last data away.
        If not, this data still belong to last frame. save it. 
**********************************************************************************/
        int data_from_socket_process(int socket)
        {
            DBG_log("ENTER [%s]\n", __func__);

            if (first_Data_Come==0) {
                gettimeofday(&tv_delay,&tz);
                read_com_data_start_sec = tv_delay.tv_sec;
                read_com_data_start_usec = tv_delay.tv_usec;       
                first_Data_Come=1;
            }

            if (sysinfo(&info)==0)
                link_timeout_begin =(int) info.uptime;

            //syslog(LOGOPTS,"data_from_socket_process begin link_timeout_begin=%d\n",link_timeout_begin);
            com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='B';
            //syslog(LOGOPTS,"data_from_socket_process begin\n");

            if (Timing) {
                socket_time_pass_us = time_elapse(read_socket_data_start_sec, read_socket_data_start_usec);
                gettimeofday(&tv_delay,&tz);
                read_socket_data_start_sec = tv_delay.tv_sec;
                read_socket_data_start_usec = tv_delay.tv_usec;
            }

            /* if it is first socket data, then get its time.*/
            /*read socket data*/
            if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]<='C')
                socket_read_once_number=read_from_tcp_socket(socket,socket_read_once_buff,SOCKET_READ_ONCE_BUFF_LEN);
            else
                socket_read_once_number=read_from_udp_socket(socket,socket_read_once_buff,SOCKET_READ_ONCE_BUFF_LEN);

            //(struct sockaddr *) &remote_client_addr, sizeof(remote_client_addr));
            /*write it to socket*/

            /*  remote close the socket connection,then local should disconnect socket connection too and*/
            /*return to idle status.*/
            if (socket_read_once_number<0) { /*read error */
                DBG_log("\n[%s] socket_read_once_number %d \n", __func__, socket_read_once_number);
                if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]<='C') {
                    shutdown(socket,SHUT_RDWR);
                    close(socket);
                }
                return -1;
            } else if (socket_read_once_number==0) {/*REMOTE close, goto main loop*/
                DBG_log("\n[%s] socket_read_once_number %d \n", __func__, socket_read_once_number);
                if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]<='C') {
                    shutdown(socket,SHUT_RDWR);
                    close(socket);
                }
                return -1;
            }
            /*if get some data, then process data*/
            else {/* dont set it  to large value*/
                DBG_log("\n[%s] read %d bytes data from TCP server\n", __func__, socket_read_once_number);  // //
                /*frame timeout, this data belong to new frame. so send out last frame to com port.*/
#if 1
                DBG_log("[%s]  socket_time_pass_us=%d, Timing_out_frame_delay=%d, Timing_in_frame_delay=%d\n", __func__, socket_time_pass_us, Timing_out_frame_delay, Timing_in_frame_delay);
                if (socket_time_pass_us>Timing_out_frame_delay) {
                    DBG_log("[%s] socket_receive_num_for_send_total=%d\n", __func__, socket_receive_num_for_send_total);
                    if ( socket_receive_num_for_send_total>0) {/*before this data, it is a frame.*/

                        DBG_log("[%s] socket data frame timeout,  write %d to com \n", __func__, socket_receive_num_for_send_total);
                        if (write_to_com_process(socket)==-1) {
                            syslog(LOGOPTS,"data_from_socket_process write_to_com_process(socket)==-1\n");
                            exit_process();
                        }
                    }
                    /*save the new socket data to buffer.*/
                    socket_receive_num_for_send_total =0;
                    memcpy(socket_total_buff, socket_read_once_buff,socket_read_once_number);
                    socket_receive_num_for_send_total =socket_read_once_number;
                } else if (socket_time_pass_us>Timing_in_frame_delay)
                /* not frame timeout*/
                /*if it over character timeout, then discard it.*/
                {
                    DBG_log("[%s] it over character timeout, then discard it \n", __func__);
                    /*update droped bytes and packets number*/
                    com_Data_total_int_buff[COM_STATISTICS_RECEIVE_DROPED_BYTES] += socket_read_once_number;
                    com_Data_total_int_buff[COM_STATISTICS_RECEIVE_DROPED_PACKETS] += 1;
                    sprintf(com_Data_total_str_buff[COM_STATISTICS_RECEIVE_DROPED_BYTES],"%u",com_Data_total_int_buff[COM_STATISTICS_RECEIVE_DROPED_BYTES]);  
                    sprintf(com_Data_total_str_buff[COM_STATISTICS_RECEIVE_DROPED_PACKETS],"%u",com_Data_total_int_buff[COM_STATISTICS_RECEIVE_DROPED_PACKETS]);                                                
                    write_com_statistics();
                    socket_receive_num_for_send_total = 0;  /* clear buffer to discard data.*/
                    syslog(LOGOPTS,"socket data from server inter character time out 1\n");
                } else { /* the new data is good and still belong to last frame*/
                    DBG_log("[%s] new data is good and still belong to last frame \n", __func__);
                    memcpy(&socket_total_buff[socket_receive_num_for_send_total], socket_read_once_buff,socket_read_once_number);
                    socket_receive_num_for_send_total +=socket_read_once_number;
                    DBG_log("[%s] socket data inter frame delay not timeout\n",  __func__);  
                    /*if receive too long, force it out to com port.*/
                    if (socket_receive_num_for_send_total >=max_packet_size) {
                        //syslog(LOGOPTS,"socket data buffer full,not timeout, write %d to com \n",socket_receive_num_for_send_total);
                        if (write_to_com_process(socket)==-1) {
                            syslog(LOGOPTS,"data_from_socket_process write_to_com_process(socket)==-1\n");                
                            exit_process();
                        }
                    }
                }     /* end of esle good data still belong to last frame*/
#else /*always send it out*/
                if ( socket_receive_num_for_send_total>0) {/*before this data, it is a frame.*/
                    //syslog(LOGOPTS,"socket data frame timeout,  write %d to com \n",socket_receive_num_for_send_total);
                    if (write_to_com_process(socket)==-1) {
                        syslog(LOGOPTS,"data_from_socket_process write_to_com_process(socket)==-1\n");
                        exit_process();
                    }
                }
                /*save the new socket data to buffer.*/
                socket_receive_num_for_send_total =0;
                memcpy(socket_total_buff, socket_read_once_buff,socket_read_once_number);
                socket_receive_num_for_send_total =socket_read_once_number;
#endif

            }/* there is data from server*/
            //syslog(LOGOPTS,"data_from_socket_process end\n");
            DBG_log("Leave [%s]\n", __func__);
            return 0;
        }

/**********************************************************************************
   Function:    int now_have_com_data_process(int socket)
   Input:       int socket
   Output:      None
   Return:      int int -1 :error , 0: ok.  
   Note:       	if time has over one frame.write the data from  socket to com port 
                and update statistics. 
**********************************************************************************/
        int now_have_com_data_process(int socket)
        {
            //syslog(LOGOPTS,"now_have_com_data_process  begin \n");
            if (Timing) {
                com_time_pass_us = time_elapse(read_com_data_start_sec, read_com_data_start_usec);
            }
            if (com_time_pass_us>Timing_out_frame_delay) {
#ifdef DEBUG_PRINT_INPUT 
                write_to_serial_port(f_int1,com_total_buff,com_receive_num_for_send_total );
#endif
                if (write_to_socket_process(socket)==-1) {
                    syslog(LOGOPTS,"now_have_com_data_process write_to_socket_process(socket)==-1\n");            
                    return -1;
                }
            }
            //syslog(LOGOPTS,"now_have_com_data_process  end \n");
            return 0;
        }

/**********************************************************************************
   Function:    int now_have_socket_data_process(int socket)
   Input:       int socket
   Output:      None
   Return:      int int -1 :error , 0: ok.  
   Note:       	if time has over one frame.write the data from com port to socket
                and update statistics. 
**********************************************************************************/
        int  now_have_socket_data_process(int socket)
        {
            DBG_log("ENTER [%s]\n", __func__);
            //syslog(LOGOPTS,"now_have_socket_data_process begin \n");	
            if (Timing) {
                socket_time_pass_us = time_elapse(read_socket_data_start_sec, read_socket_data_start_usec);
            }
            if (socket_time_pass_us>Timing_out_frame_delay) {
                if (write_to_com_process(socket)==-1) {
                    syslog(LOGOPTS,"now_have_socket_data_process(socket)==-1 return -1\n");   
                    return -1;
                }
            }
            //syslog(LOGOPTS,"now_have_socket_data_process end \n");
            DBG_log("Leave [%s]\n", __func__);      
            return 0;
        }

/**********************************************************************************
   Function:    int _write_to_socket_process(int socket_, const void * buf_, int size_)
   Input:       int socket
   Output:      None
   Return:      int -1 :error , 0: ok. 
   Note:       	write the data from com port to socket and update statistics. 
**********************************************************************************/
        int _write_to_socket_process(int socket_, const void * buf_, int size_)
        {
            int i,j;
            int sent_len;
            DBG_log("ENTER [%s] max_packet_size=%d size=%d\n", __func__, max_packet_size, size_);

            //syslog(LOGOPTS,"write_to_socket_process begin max_packet_size=%d\n",max_packet_size);
            for (i=0;i<=size_/max_packet_size;i++) {
                if (i<size_/max_packet_size) {
                    sent_len = max_packet_size;         
                } else {
                    sent_len = size_%max_packet_size;
                    if (0) {//receive_more_than_max)
                        size_ = sent_len;
                        sent_len=0;
                        memmove(buf_,&buf_[i*max_packet_size],size_);            
                    }
                }
                if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]<='C') {

                    /*process multiple tcp connnections*/
                    if (current_tcp_client_num>=1) {//local as tcp server and have link
                        if (DataBase1_Buff[COM12_SUB_MENU_TSERVER][TSERVER_ITEM_PULLING_MODE][FIRST_CHARACTOR]=='A') {/*monitor mode send data to all clients*/

                            for (j=0;j<REMOTE_TCP_CLIENT_NUMBER;j++) {
                                if (remote_tcp_client_list[j]>0) {
                                    //syslog(LOGOPTS,"write to socket %d,i=%d",remote_tcp_client_list[j],j);
                                    write_return=write_to_tcp_socket(remote_tcp_client_list[j],&buf_[i*max_packet_size],sent_len);
                                    if (write_return==-1) {

                                        shutdown(remote_tcp_client_list[j],SHUT_RDWR);
                                        close(remote_tcp_client_list[j]);
                                        remote_tcp_client_list[j] = 0;
                                        current_tcp_client_num--;
                                        sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",current_tcp_client_num);
                                        write_com_statistics();
                                    }
                                }
                            }
                        } else { /*polling mode send the data to the one who is polling*/
                            write_return=write_to_tcp_socket(remote_tcp_client_list[current_polling_unit],&buf_[i*max_packet_size],sent_len);
                            if (write_return==-1) {
                                shutdown(remote_tcp_client_list[current_polling_unit],SHUT_RDWR);
                                close(remote_tcp_client_list[current_polling_unit]);
                                remote_tcp_client_list[current_polling_unit] = 0;
                                current_tcp_client_num--;
                                sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",current_tcp_client_num);
                                write_com_statistics();
                                current_polling_get_response= true;
                            } else {
                                /*clear current_polling_unit and get the new one*/
                                current_polling_get_response=true;
                            }
                        }
                        /*write it to socket*/
                        if (current_tcp_client_num<=0) {
                            //syslog(LOGOPTS,"write to socket Error return to idle :%s\n",strerror(errno));
                            sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",0);
                            com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                            write_com_statistics();
                            syslog(LOGOPTS,"%s write_return==-1\n",__func__); 
                            return -1;                  
                        } else {
                            com_Data_total_int_buff[COM_STATISTICS_SEND_BYTES] += size_;
                            com_Data_total_int_buff[COM_STATISTICS_SEND_PACKETS] += 1;
                            sprintf(com_Data_total_str_buff[COM_STATISTICS_SEND_BYTES],"%u",com_Data_total_int_buff[COM_STATISTICS_SEND_BYTES]);
                            sprintf(com_Data_total_str_buff[COM_STATISTICS_SEND_PACKETS],"%u",com_Data_total_int_buff[COM_STATISTICS_SEND_PACKETS]);
                            write_com_statistics();
                            //com_read_once_number=0;
                            // size_  = 0;
                            //return 0;
                        }
                    } else { //local as client
                        write_return=write_to_tcp_socket(socket_,&buf_[i*max_packet_size],sent_len);
                        /*write it to socket*/
                        if (write_return==-1) {
                            shutdown(socket_,SHUT_RDWR);
                            close(socket_);
                            sprintf(com_Data_total_str_buff[COM_STATISTICS_CLIENT_LINK_NUM],"%d",0);
                            com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                            write_com_statistics();
                            syslog(LOGOPTS,"%s write_return==-1\n",__func__); 
                            return -1;                  
                        } else {
                            com_Data_total_int_buff[COM_STATISTICS_SEND_BYTES] += size_;
                            com_Data_total_int_buff[COM_STATISTICS_SEND_PACKETS] += 1;
                            sprintf(com_Data_total_str_buff[COM_STATISTICS_SEND_BYTES],"%u",com_Data_total_int_buff[COM_STATISTICS_SEND_BYTES]);
                            sprintf(com_Data_total_str_buff[COM_STATISTICS_SEND_PACKETS],"%u",com_Data_total_int_buff[COM_STATISTICS_SEND_PACKETS]);
                            write_com_statistics();
                            //com_read_once_number=0;
                            // size_  = 0;
                            // return 0;
                        }
                    }
                } else {
                    write_return=write_to_udp_socket(socket_,&buf_[i*max_packet_size],sent_len,
                                                     (struct sockaddr *) &remote_as_server_addr, sizeof(remote_as_server_addr));
                    /*write it to socket*/
                    if (write_return==-1) {
                        com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                        write_com_statistics();
                        syslog(LOGOPTS,"%s write_return==-1\n",__func__); 
                        return -1;                  
                    } else {
                        com_Data_total_int_buff[COM_STATISTICS_SEND_BYTES] += size_;
                        com_Data_total_int_buff[COM_STATISTICS_SEND_PACKETS] += 1;
                        sprintf(com_Data_total_str_buff[COM_STATISTICS_SEND_BYTES],"%u",com_Data_total_int_buff[COM_STATISTICS_SEND_BYTES]);
                        sprintf(com_Data_total_str_buff[COM_STATISTICS_SEND_PACKETS],"%u",com_Data_total_int_buff[COM_STATISTICS_SEND_PACKETS]);
                        write_com_statistics();
                        //com_read_once_number=0;
                        //size_  = 0;
                        //return 0;
                    }
                }
            }
            DBG_log("Leave [%s] \n", __func__);
            return 0;
        }

/**********************************************************************************
   Function:    int _write_to_com_process(int socket_, const void * buf_, int size_)
   Input:       int socket
   Output:      None
   Return:      int int -1 :error , 0: ok.  
   Note:       	write the data from socket to com port and update statistics. 
**********************************************************************************/
        static int _write_to_com_process(int socket_, const void * buf_, int size_)
        {
            //syslog(LOGOPTS,"write_to_com_process begin \n");
            DBG_log("ENTER [%s] %d bytes\n", __func__, size_);

            if (DataBase1_Buff[COM12_SUB_MENU_TSERVER][TSERVER_ITEM_PULLING_MODE][FIRST_CHARACTOR]=='A') {/*monitor mode send data to all clients*/

            }

            write_return=write_to_serial_port(fdcom,(char *)buf_,size_ );/*write it to serial port*/
            if (write_return==-1) {
                if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_IP_PROTOCOL_CONFIG][FIRST_CHARACTOR]<='C') {
                    shutdown(socket_,SHUT_RDWR);
                    close(socket_);   
                }
                com_Data_total_str_buff[COM_STATISTICS_PROGRAM_STATUS][FIRST_CHARACTOR]='C';
                write_com_statistics();
                syslog(LOGOPTS,"write_to_com_process write_return==-1\n");         
                return -1;
            } else {
                /*if the packet is not right, then user will see packets grow and bytes be 0*/
                if (size_<=0) {
                    com_Data_total_int_buff[COM_STATISTICS_RECEIVE_DROPED_PACKETS] += 1;
                    sprintf(com_Data_total_str_buff[COM_STATISTICS_RECEIVE_DROPED_PACKETS],"%u",com_Data_total_int_buff[COM_STATISTICS_RECEIVE_DROPED_PACKETS]);
                } else {
                    com_Data_total_int_buff[COM_STATISTICS_RECEIVED_BYTES] += size_;
                    com_Data_total_int_buff[COM_STATISTICS_RECEIVED_PACKETS] += 1;
                    sprintf(com_Data_total_str_buff[COM_STATISTICS_RECEIVED_BYTES],"%u",com_Data_total_int_buff[COM_STATISTICS_RECEIVED_BYTES]);
                    sprintf(com_Data_total_str_buff[COM_STATISTICS_RECEIVED_PACKETS],"%u",com_Data_total_int_buff[COM_STATISTICS_RECEIVED_PACKETS]);
                }
                write_com_statistics();
                socket_read_once_number=0;
                size_  = 0;
                //syslog(LOGOPTS,"write_to_com_process end \n");
                DBG_log("LEAVE [%s]\n", __func__);
                return 0;
            }
        }

/**********************************************************************************
   Function:    int set_select_readfd(int fd1,int fd2)
   Input:       (int fd1,int fd2)
   Output:      None
   Return:      int -1: error, 0: ok.  
   Note:       	to check three plus to exit com2 program. 
**********************************************************************************/
        int set_select_readfd(int fd1,int fd2)
        {
            FD_ZERO(&my_readfd);/*clear ,it should not be out side of while*/
            FD_SET(fd1,&my_readfd);/*add the thread*/
            FD_SET(fd2,&my_readfd);
            if (first_Data_Come==0) {
                if (fd1>fd2)
                    select_return=select(fd1+1,&my_readfd, NULL,NULL,NULL);
                else
                    select_return=select(fd2+1,&my_readfd, NULL,NULL,NULL);
            } else {
                if (Timing) {
                    tv_100us.tv_sec = 0;  
                    tv_100us.tv_usec =100;
                    if (fd1>fd2)
                        select_return=select(fd1+1,&my_readfd, NULL,NULL,&tv_100us);
                    else
                        select_return=select(fd2+1,&my_readfd, NULL,NULL,&tv_100us);        
                } else {
                    tv_100us.tv_sec = 0;  
                    tv_100us.tv_usec =100;
                    if (fd1>fd2)
                        select_return=select(fd1+1,&my_readfd, NULL,NULL,&tv_100us);
                    else
                        select_return=select(fd2+1,&my_readfd, NULL,NULL,&tv_100us);
                }
            }
            if (select_return==-1) {
                syslog(LOGOPTS,"select_return==-1)");
                return -1;
            }

            return 0;
        }

/**********************************************************************************
   Function:   static  void com2_check_three_plus(void)
   Input:       void
   Output:      None
   Return:      None  
   Note:       	to check three plus to exit com2 program. 
**********************************************************************************/
#if defined(IXP425COM2) || defined(MPCI2)
static void com2_check_three_plus(void)
{
    int i;
    if (com_read_once_number<=0)
        return;

    if (com_read_once_number>1) {
        plus_num=0;
        return;                
    }

    //syslog(LOGOPTS,"com_read_once_buff=%s, com_read_once_number=%d \n",com_read_once_buff,com_read_once_number);  
    if (com_read_once_buff[0]=='+') {
        plus_num++;
    } else {
        plus_num=0;
    }

    if (plus_num>=3) {
          plus_num=0;
          close(fdcom);
          UserDB_write(UserDB,Item_Keywords[SUB_MENU_COM2][0],32,"A",32);  
          UserDB_write(FlashUserDB,Item_Keywords[SUB_MENU_COM2][0],32,"A",32);  
          unlink(Com2StatisticFile);
          unlink(COM2EnableFile);
          exit(0);
          return;/*quit soipd2*/
     }


}
#endif


/**********************************************************************************
   Function:   static  void usb_check_three_plus(void)
   Input:       void
   Output:      None
   Return:      None  
   Note:       	to check three plus to exit usb program. 
**********************************************************************************/
#ifdef IXP425USB
                static void usb_check_three_plus(void)
                {
                    int i;
                    if (com_read_once_number<=0)
                        return;

                    for (i=0;i<com_read_once_number;i++) {
                        //syslog(LOGOPTS,"com_read_once_buff=%s, com_read_once_number=%d \n",com_read_once_buff,com_read_once_number);  
                        if (com_read_once_buff[i]=='+') {
                            //syslog(LOGOPTS,"plus_num=%d, buff[%d]='+' \n",plus_num,i);              
                            if (plus_num==0) {
                                plus_num=1;
                                plus_begin_position=0;//reset position to zero
                                plus0_position=plus_begin_position+i;                
                            } else if (plus_num==1) {
                                plus1_position=plus_begin_position+i;
                                if ((plus1_position-plus0_position)==1) {
                                    plus_num=2;                                          
                                } else {

                                    plus_num=1;                
                                    plus_begin_position=0;
                                    plus0_position=plus_begin_position+i;  
                                }
                            } else if (plus_num==2) {
                                plus2_position=plus_begin_position+i;   
                                if ((plus2_position-plus1_position)==1) {
                                    plus_num=0;
                                    close(fdcom);
                                    UserDB_write(UserDB,Item_Keywords[USB_SUB_MENU_DATA_MODE][0],32,"A",32);  
                                    UserDB_write(FlashUserDB,Item_Keywords[USB_SUB_MENU_DATA_MODE][0],32,"A",32);  

                                    UserDB_write(UserDB,Item_Keywords[SUB_MENU_USB][0],32,"A",32);  
                                    UserDB_write(FlashUserDB,Item_Keywords[SUB_MENU_USB][0],32,"A",32);  

                                    unlink(USBStatisticFile);
                                    unlink(USBEnableFile);
                                    exit(0);
                                    return;/*quit soipd2*/
                                } else {
                                    plus_num=1;                
                                    plus_begin_position=0;
                                    plus0_position=plus_begin_position+i;  
                                }
                            }

                        }
                    }
                    if (plus_num==0)
                        plus_begin_position =com_read_once_number;
                    else if (plus_num==1)
                        plus_begin_position =com_read_once_number;
                    else if (plus_num==2)
                        if ((plus_begin_position-plus1_position)>2) {
                            plus_num=0;                
                            plus_begin_position=0;
                        } else
                            plus_begin_position +=com_read_once_number;

                }
#endif

/**********************************************************************************
   Function:  void write_com_statistics(void)   
   Input:     void   
   Output:    void   
   Return:    None   
   Note:      update the com port statics to file   
**********************************************************************************/
                static void write_com_statistics(void)
                {
                    FILE* f_fd;
                    int i;
                    while (!(f_fd = fopen(COM12_statistic_file, "w+"))) {
                        //syslog(LOGOPTS,"%s %s\n",COM12_statistic_file,strerror(errno));            
                    }
                    for (i=0;i<20;i++) {
                        fputs( com_Data_total_str_buff[i],f_fd);
                        fputc( '\n',f_fd);
                    }
                    fclose(f_fd);
                }

/**********************************************************************************
   Function: int time_elapse(time_t old_sec, suseconds_t old_us)    
   Input:    int old_sec, int old_us    
   Output:       
   Return:       
   Note:   if time elapse more than 100s, it will return 100000000 only. it is enough in use.      
**********************************************************************************/
                static unsigned int time_elapse(time_t old_sec, suseconds_t old_us)
                {
                    struct timeval tv;
                    struct timezone tz;
                    unsigned int return_value; 

                    gettimeofday (&tv, &tz); /*get starttime again*/
                    if ((tv.tv_sec-old_sec)<100)
                        return_value= (tv.tv_sec-old_sec)*1000000 + tv.tv_usec - old_us ;
                    else
                        return_value=100000000;
                    return return_value;
                }

/**********************************************************************************
   Function:    int get_Timing_Out_Frame_Delay(char *Timing_out_frame_delay, char Baud )
   Input:       Timing_out_frame_delay, Baud
   Output:      None
   Return:      the unisecond value of delay  
   Note:        according to character delay and Baud rate to generate unisecond delay
                of inner frame delay.
**********************************************************************************/
                static unsigned int get_Timing_In_Frame_Delay( char Baud )
                {
                    int com_speed[18]={3,6,12,24,36,48,72,96,144,192,288,384,576,1152,2304,4608,9216};
                    unsigned int return_delay,Character_delay,Baudrate;
                    double time;
                    int byte_len;
                    int i;
                    byte_len=caculate_current_data_byte_len(); 
                    Character_delay = 1.5;
                    i =  Baud-65;
                    if ((i>=0)&&(i<15))
                        Baudrate = com_speed[i];
                    else
                        Baudrate = 96;
                    if (i<8) {/*9600 and below should be more than 3.5 charactor time out*/
                        time = Character_delay*10000*byte_len; /*11*10000*/
                        return_delay = time/Baudrate;    
                    } else/*more than 9600, should be 1.75ms*/
                        return_delay = 750;

                    return(return_delay);  /* return us*/
                }

/**********************************************************************************
   Function:  int get_Timing_Out_Frame_Delay(char *Timing_out_frame_delay, char Baud )   
   Input:     (char *Timing_out_frame_delay, char Baud )   
   Output:    None    
   Return:    int   
   Note:      this function will turn the string charactor delay time to us time according
              to different baud rate.   
**********************************************************************************/
                static unsigned int get_Timing_Out_Frame_Delay(char *Timing_out_frame_delay, char Baud )
                {
                    int com_speed[18]={3,6,12,24,36,48,72,96,144,192,288,384,576,1152,2304,4608,9216};
                    unsigned int return_delay,Baudrate;
                    double time;
                    unsigned int byte_len;
                    int i;
                    int Character_delay ;

                    byte_len=caculate_current_data_byte_len();    
                    Character_delay = atoi(Timing_out_frame_delay);
                    if ((Character_delay<4)&&(!Trasparent))
                        Character_delay=3.5;
                    /*//syslog(LOGOPTS,"Character_delay %d\n",Character_delay);*/
                    i =  Baud-'A';
                    if ((i>=0)&&(i<15))
                        Baudrate = com_speed[i];
                    else
                        Baudrate = 96;
                    /* //syslog(LOGOPTS,"Baudrate %d\n",Baudrate);*/

                    if (Character_delay >1000)
                        time = Character_delay*1000*byte_len;
                    else
                        time = Character_delay*10000*byte_len; /*11*10000*/

                    /*9600 and below should be more than 3.5 charactor time out*/
                    return_delay = time/Baudrate;
                    if (Character_delay >1000)
                        return_delay = return_delay*10;

                    if (i>7) {/*more than 9600, should be 1.75ms*/
                        /*9600 and below should be more than 3.5 charactor time out*/
                        if ((return_delay<1750)&&(!Trasparent)) {
                            return_delay = 1750;
                        }
                    }
                    return(return_delay);  /* return us*/
                }
#if defined(IXP425COM2) || defined(MPCI2) 
                void *thread_function(void *arg)
                {
                    while (1) {
                        if (thread_return) {
                            return NULL;
                        }
                        sleep(1);
                        com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN);    
                        if (com_read_once_number>0)
                            com2_check_three_plus();
                    }
                }

#endif

#ifdef IXP425USB 
                void *thread_function(void *arg)
                {
                    while (1) {
                        if (thread_return) {
                            return NULL;
                        }
                        sleep(1);
                        com_read_once_number=read_from_serial_port(fdcom, com_read_once_buff, COM_READ_ONCE_BUFF_LEN);    
                        if (com_read_once_number>0)
                            usb_check_three_plus();
                    }
                }

#endif


                static void exit_process(void)
                {
                    DBG_log("ENTER [%s]\n", __func__);
#if defined(IXP425COM2) || defined(MPCI2) 
                    UserDB_write(UserDB,Item_Keywords[SUB_MENU_COM2][0],32,"A",32);  
                    UserDB_write(FlashUserDB,Item_Keywords[SUB_MENU_COM2][0],32,"A",32);  
                    unlink(Com2StatisticFile);
                    unlink(COM2EnableFile);      
                    close(fdcom); 
                    exit(1);
#endif
#ifdef IXP425USB 
//	UserDB_write(UserDB,Item_Keywords[USB_SUB_MENU_DATA_MODE][0],32,"A",32);  
//	UserDB_write(FlashUserDB,Item_Keywords[USB_SUB_MENU_DATA_MODE][0],32,"A",32);  
//	unlink(USBStatisticFile);
//	unlink(USBEnableFile);      
#endif

                }

                static void fctl_com(void)
                {
                    if (DataBase1_Buff[SUB_MENU_COM12][COM12_ITEM_NO_CONNECTION_DATA_INTAKE][FIRST_CHARACTOR]=='B') {
                        fcntl (fdcom, F_SETFL, fcntl(fdcom, F_GETFL) | O_NONBLOCK);    /* dont block read*/
                    } else {
                        fcntl (fdcom, F_SETFL, fcntl(fdcom, F_GETFL) | O_NONBLOCK);
                        tcflush(fdcom,TCIOFLUSH);
                    }       
                }


/*-----------------------------------------------------------------------------
 * Constants for MODBUS communication
 *---------------------------------------------------------------------------*/
#define MODBUS_DEBUG                  0
#define MODBUS_DEBUG_DETAILED         0

#define MODBUS_PDU_MAX_SIZE           1024
#define MODBUS_SERIAL_ADU_OVERHEAD    3
#define MODBUS_SERIAL_ADU_MAX_SIZE    ( MODBUS_SERIAL_ADU_OVERHEAD + MODBUS_PDU_MAX_SIZE ) //1027
#define MODBUS_TCP_ADU_OVERHEAD       7
#define MODBUS_TCP_ADU_MAX_SIZE       ( MODBUS_TCP_ADU_OVERHEAD + MODBUS_PDU_MAX_SIZE )


/*-----------------------------------------------------------------------------
 * Type definitions for MODBUS communication
 *---------------------------------------------------------------------------*/
                typedef struct {
                    // MBAP
                    unsigned short  transactionId;
                    unsigned short  protocolId;
                    unsigned short  length;
                    unsigned char   unitId;
                    // payload
                    unsigned char   funcCode;
                    char            data[ 1 ];
                } MODBUS_TCP_PACKET;

                typedef struct {
                    // MBAP
                    unsigned short  transactionId;
                    unsigned short  protocolId;
                    unsigned short  length;
                    char            encrpteddata[ 1 ];
                } ENCRYPTED_MODBUS_TCP_PACKET;

                typedef struct {
                    unsigned char slaveAddr;
                    unsigned char funcCode;
                    char          data[ 1 ];
                    /*
                      The data is immediately followed by the CRC field
                      unsigned short crc;
                    */
                } MODBUS_SERIAL_PACKET;

                typedef enum {
                    TAR_INVALID = -1,
                    TAR_START,
                    TAR_HEADER,
                    TAR_DATA,
                    TAR_FINISH,
                } TAR_STATE;

                typedef struct {
                    TAR_STATE m_currentState;
                    int m_socket;
                    const char * m_inputBuf;
                    int m_inputSize;
                    char m_outputBuf[ MODBUS_TCP_ADU_MAX_SIZE ];
                    int m_outputSize;
                    int m_aduLength;
                } TCP_ADU_RECEIVER;

                typedef enum {
                    MODBUS_MODE_UNKNOWN,
                    MODBUS_MODE_ENABLED,
                    MODBUS_MODE_DISABLED,
                } MODBUS_MODE;


/*-----------------------------------------------------------------------------
 * Static data for MODBUS communication
 *---------------------------------------------------------------------------*/
                static unsigned short s_transactionId = 0;
                static unsigned short s_protocolId = 0;


/*-----------------------------------------------------------------------------
 * Prototypes
 *---------------------------------------------------------------------------*/
                bool isModbusEnabled();

                static void TAR_process(TCP_ADU_RECEIVER * this_, int socket_, const void * buf_, int size_);
                static void TAR_changeState(TCP_ADU_RECEIVER * this_, TAR_STATE newState );
                static TAR_STATE TAR_processStart(TCP_ADU_RECEIVER * this_);
                static TAR_STATE TAR_processHeader(TCP_ADU_RECEIVER * this_);
                static TAR_STATE TAR_processData(TCP_ADU_RECEIVER * this_);
                static TAR_STATE TAR_enterStart(TCP_ADU_RECEIVER * this_);
                static TAR_STATE TAR_enterHeader(TCP_ADU_RECEIVER * this_);
                static TAR_STATE TAR_enterData(TCP_ADU_RECEIVER * this_);
                static void TAR_write_to_com(TCP_ADU_RECEIVER * this_);

                static int modbus_serial_to_tcp(const char * inBuf_, int inSize_, void * outBuf_);
                static int modbus_tcp_to_serial(const char * inBuf_, int inSize_, void * outBuf_);
                unsigned short calculate_crc(unsigned char buf_[], int start_, int count_);
                static char mheader[6];


/*-----------------------------------------------------------------------------
 * Implementation
 *---------------------------------------------------------------------------*/
                bool isModbusEnabled()
                {
                    static MODBUS_MODE s_modbusMode = MODBUS_MODE_UNKNOWN;

                    if ( s_modbusMode == MODBUS_MODE_UNKNOWN ) {
                        s_modbusMode =
                        ( DataBase1_Buff[ COM12_MODBUS_CONF ][ COM12_MODBUS_ITEM_STATUS ][ FIRST_CHARACTOR ] == 'B' ) ?
                        MODBUS_MODE_ENABLED : MODBUS_MODE_DISABLED;
                    }

                    return( s_modbusMode == MODBUS_MODE_ENABLED );
                }


/*---------------------------------------------------------------------------*/
                static int write_to_com_process(int socket_)
                {
                    DBG_log("ENTER [%s]\n", __func__ );
                    static TCP_ADU_RECEIVER tar =
                    {
                        TAR_START,  /* TAR_STATE m_currentState;                    */
                        -1,         /* int m_socket;                                */
                        NULL,       /* const char * m_inputBuf;                     */
                        0,          /* int m_inputSize;                             */
                        { 0},      /* char m_outputBuf[ MODBUS_TCP_ADU_MAX_SIZE ]; */
                        0,          /* int m_outputSize;                            */
                        0           /* int m_aduLength;                             */
                    } ;

                    int rt = 0;

#if MODBUS_DEBUG
                    int i;
                    syslog( LOGOPTS, "TCP->[%d]", socket_receive_num_for_send_total );
#if MODBUS_DEBUG_DETAILED
                    for ( i = 0; i < socket_receive_num_for_send_total; i++ ) {
                        syslog( LOGOPTS, "%02x ", socket_total_buff[ i ] );
                    }
                    syslog( LOGOPTS, "\n" );
#endif
#endif

                    if ( isModbusEnabled() ) {
#if 0
                        TAR_process( &tar,
                                     socket_,
                                     socket_total_buff,
                                     socket_receive_num_for_send_total );
#else
                        mheader[0]=socket_total_buff[ 0 ];
                        mheader[1]=socket_total_buff[ 1 ];
                        mheader[2]=socket_total_buff[ 2 ];
                        mheader[3]=socket_total_buff[ 3 ];
                        mheader[4]=socket_total_buff[ 4 ];
                        mheader[5]=socket_total_buff[ 5 ];

                        char sendBuf[1024];
                        int sendSize=-1;
                        sendSize = modbus_tcp_to_serial(socket_total_buff,
                                                        socket_receive_num_for_send_total,
                                                        sendBuf );


#if MODBUS_DEBUG
                        int i;
                        syslog( LOGOPTS, "->COM[%d]", sendSize );
#if MODBUS_DEBUG_DETAILED
                        for ( i = 0; i < sendSize; i++ )
                            syslog( LOGOPTS, "%02x ", sendBuf[ i ] );
                        syslog( LOGOPTS, "\n" );
#endif
#endif

                        if ( _write_to_com_process( socket_, sendBuf, sendSize ) < 0 ) {
                            syslog( LOGOPTS, "Failed to write to com in TAR\n" );
                        }
#endif
                        rt = 0;
                    } else { /*not modbus packet*/

#if MODBUS_DEBUG
                        syslog( LOGOPTS, "->COM[%d]", socket_receive_num_for_send_total );
#if MODBUS_DEBUG_DETAILED
                        for ( i = 0; i < socket_receive_num_for_send_total; i++ )
                            syslog( LOGOPTS, "%02x ", socket_total_buff[ i ] );
                        syslog( LOGOPTS, "\n" );
#endif
#endif

                        rt = _write_to_com_process( socket_,
                                                    socket_total_buff,
                                                    socket_receive_num_for_send_total );
                    }

                    /* conform to the original logic */
                    if ( rt >= 0 ) {
                        socket_receive_num_for_send_total = 0;
                    }

                    return rt;
                }



/*---------------------------------------------------------------------------*/
                static int write_to_socket_process(int socket_)
                {
                    char sendBuf[ MODBUS_TCP_ADU_MAX_SIZE ];
                    int sendSize;
                    int rt = 0;
                    unsigned short tmp_len;
                    DBG_log("ENTER [%s]\n", __func__);

#if MODBUS_DEBUG
                    int i;
                    syslog( LOGOPTS, "COM->[%d]", com_receive_num_for_send_total );
#if MODBUS_DEBUG_DETAILED
                    for ( i = 0; i < com_receive_num_for_send_total; i++ )
                        syslog( LOGOPTS, "%02x ", com_total_buff[ i ] );
                    syslog( LOGOPTS, "\n" );
#endif
#endif

                    if ( isModbusEnabled() ) {
                        // here we assume that the data received from serial port should be a integrated packet.

                        sendSize = modbus_serial_to_tcp( com_total_buff,
                                                         com_receive_num_for_send_total,
                                                         sendBuf );
                        sendBuf[0]=mheader[0];
                        sendBuf[1]=mheader[1];
                        sendBuf[2]=mheader[2];
                        sendBuf[3]=mheader[3];

                        tmp_len= (short)com_receive_num_for_send_total - 2 ;
                        mheader[4]=(char)(tmp_len>>8);
                        mheader[5]=(char)(tmp_len);
                        sendBuf[4]=mheader[4];
                        sendBuf[5]=mheader[5];

                        if ( sendSize <= 0 ) { /*changed for modbus crc error process,pakcet error, then ignore it*/
                            com_receive_num_for_send_total = 0;
                            return 0;
                        }

#if MODBUS_DEBUG
                        syslog( LOGOPTS, "->TCP[%d]", sendSize );
#if MODBUS_DEBUG_DETAILED
                        for ( i = 0; i < sendSize; i++ )
                            syslog( LOGOPTS, "%02x ", sendBuf[ i ] );
                        syslog( LOGOPTS, "\n" );
#endif
#endif

                        rt = _write_to_socket_process( socket_,
                                                       sendBuf,
                                                       sendSize );
                    } else {
#if MODBUS_DEBUG
                        syslog( LOGOPTS, "->TCP[%d]", com_receive_num_for_send_total );
#if MODBUS_DEBUG_DETAILED
                        for ( i = 0; i < com_receive_num_for_send_total; i++ )
                            syslog( LOGOPTS, "%02x ", com_total_buff[ i ] );
                        syslog( LOGOPTS, "\n" );
#endif
#endif

                        rt = _write_to_socket_process( socket_,
                                                       com_total_buff,
                                                       com_receive_num_for_send_total );
                    }

                    /* conform to the original logic */
                    if ( rt >= 0 ) {
                        com_receive_num_for_send_total = 0;
                    }

                    return rt;
                }


/*---------------------------------------------------------------------------*/
                static void TAR_process
                (
                TCP_ADU_RECEIVER * this_,
                int socket_,
                const void * buf_,
                int size_
                )
                {
                    if ( this_ == NULL ) {
                        syslog( LOGOPTS, "Invalid TCP ADU receiver\n" );
                        return;
                    }

                    if ( buf_ == NULL || size_ <= 0 ) {
                        syslog( LOGOPTS, "Invalid TCP MODBUS data\n" );
                        return;
                    }

                    this_->m_socket = socket_;
                    this_->m_inputBuf = buf_;
                    this_->m_inputSize = size_;

                    switch ( this_->m_currentState ) {
                    case TAR_START: 
                        TAR_changeState( this_, TAR_processStart( this_ ) );
                        break;
                    case TAR_HEADER:
                        TAR_changeState( this_, TAR_processHeader( this_ ) );
                        break;
                    case TAR_DATA:
                        TAR_changeState( this_, TAR_processData( this_ ) );
                        break;
                    default:
                        syslog( LOGOPTS,
                                "Invalid TAR state %d in process\n",
                                this_->m_currentState );
                        break;
                    }
                }
/*---------------------------------------------------------------------------*/
                static void TAR_changeState(TCP_ADU_RECEIVER * this_, TAR_STATE newState )
                {
                    while ( newState != this_->m_currentState ) {
                        this_->m_currentState = newState;
                        switch ( this_->m_currentState ) {
                        case TAR_START:
                            newState = TAR_enterStart( this_ );
                            break;
                        case TAR_HEADER:
                            newState = TAR_enterHeader( this_ );
                            break;
                        case TAR_DATA:
                            newState = TAR_enterData( this_ );
                            break;
                        default:
                            syslog( LOGOPTS,
                                    "Invalid TAR state %d in changeState\n",
                                    this_->m_currentState );
                            break;
                        }
                    }
                }
/*---------------------------------------------------------------------------*/
                static TAR_STATE TAR_processStart(TCP_ADU_RECEIVER * this_)
                {
                    return TAR_HEADER;
                }
/*---------------------------------------------------------------------------*/
                static TAR_STATE TAR_processHeader(TCP_ADU_RECEIVER * this_)
                {
                    int freeSize;
                    int copySize;

                    freeSize = MODBUS_TCP_ADU_OVERHEAD - this_->m_outputSize;
                    if ( freeSize < 0 ) freeSize = 0;
                    copySize = ( freeSize < this_->m_inputSize ) ? freeSize : this_->m_inputSize;

                    if ( copySize > 0 ) {
                        memcpy( this_->m_outputBuf + this_->m_outputSize, this_->m_inputBuf, copySize );
                        this_->m_inputBuf += copySize;
                        this_->m_inputSize -= copySize;
                        this_->m_outputSize += copySize;
                    }

                    if ( this_->m_outputSize >= MODBUS_TCP_ADU_OVERHEAD ) {
                        if (modbus_encryption_enable) {
                            this_->m_aduLength =  max(8,(((MODBUS_TCP_PACKET *)this_->m_outputBuf )->length+3)&-3) + MODBUS_TCP_ADU_OVERHEAD - 1;
                        } else {
                            this_->m_aduLength = ( (MODBUS_TCP_PACKET *)this_->m_outputBuf )->length + MODBUS_TCP_ADU_OVERHEAD - 1;
                        }
                        return TAR_DATA;
                    }

                    return this_->m_currentState;
                }
/*---------------------------------------------------------------------------*/
                static TAR_STATE TAR_processData(TCP_ADU_RECEIVER * this_)
                {
                    int freeSize;
                    int copySize;

                    freeSize = this_->m_aduLength - this_->m_outputSize;
                    if ( freeSize < 0 ) freeSize = 0;
                    copySize = ( freeSize < this_->m_inputSize ) ? freeSize : this_->m_inputSize;

                    if ( copySize > 0 ) {
                        memcpy( this_->m_outputBuf + this_->m_outputSize, this_->m_inputBuf, copySize );
                        this_->m_inputBuf += copySize;
                        this_->m_inputSize -= copySize;
                        this_->m_outputSize += copySize;
                    }

                    if ( this_->m_outputSize >= this_->m_aduLength ) {
                        /* we have got an integral ADU here */
                        TAR_write_to_com( this_ );
                        return TAR_START;
                    }

                    return this_->m_currentState;
                }
/*---------------------------------------------------------------------------*/
                static TAR_STATE TAR_enterStart(TCP_ADU_RECEIVER * this_)
                {
                    /* Reset the state machine here */
                    this_->m_outputSize = 0;
                    this_->m_aduLength = 0;

                    if ( this_->m_inputBuf != NULL && this_->m_inputSize > 0 ) {
                        /* There is still data left in the incoming buffer */
                        return TAR_HEADER;
                    }

                    return this_->m_currentState;
                }
/*---------------------------------------------------------------------------*/
                static TAR_STATE TAR_enterHeader(TCP_ADU_RECEIVER * this_)
                {
                    return TAR_processHeader( this_ );
                }
/*---------------------------------------------------------------------------*/
                static TAR_STATE TAR_enterData(TCP_ADU_RECEIVER * this_)
                {
                    return TAR_processData( this_ );
                }
/*---------------------------------------------------------------------------*/
                static void TAR_write_to_com(TCP_ADU_RECEIVER * this_)
                {
                    char sendBuf[ MODBUS_SERIAL_ADU_MAX_SIZE ];
                    int sendSize;

                    sendSize = modbus_tcp_to_serial( this_->m_outputBuf,
                                                     this_->m_outputSize,
                                                     sendBuf );
                    if ( sendSize <= 0 ) {
                        return;
                    }

#if MODBUS_DEBUG
                    int i;
                    syslog( LOGOPTS, "->COM[%d]", sendSize );
#if MODBUS_DEBUG_DETAILED
                    for ( i = 0; i < sendSize; i++ )
                        syslog( LOGOPTS, "%02x ", sendBuf[ i ] );
                    syslog( LOGOPTS, "\n" );
#endif
#endif

                    if ( _write_to_com_process( this_->m_socket, sendBuf, sendSize ) < 0 ) {
                        syslog( LOGOPTS, "Failed to write to com in TAR\n" );
                    }
                }


/*-----------------------------------------------------------------------------
 * Function: modbus_serial_to_tcp
 * Return:   the size of the converted packet, or -1 in the case of error
 *---------------------------------------------------------------------------*/
                static int modbus_serial_to_tcp( const char * inBuf_, int inSize_, void * outBuf_)
                {
                    const MODBUS_SERIAL_PACKET * serialPacket;
                    MODBUS_TCP_PACKET * tcpPacket;
                    ENCRYPTED_MODBUS_TCP_PACKET *encryptedModbusTcpPacket;
                    unsigned short crc, crcInPkt;
                    int N2=0;
                    char* tempBuf=NULL;

                    if ( inBuf_ == NULL || inSize_ <= 0 ) {
                        syslog( LOGOPTS, "Invalid MODBUS serial data\n" );  
                        return 0;
                    }

                    if ( inSize_ > MODBUS_SERIAL_ADU_MAX_SIZE ) {
                        syslog( LOGOPTS, "Oversize MODBUS serial ADU, size=%d\n", inSize_ );  
                        return 0;
                    }
                    if ( inSize_ < MODBUS_TCP_ADU_OVERHEAD - MODBUS_SERIAL_ADU_OVERHEAD) {
                        syslog( LOGOPTS, "Less MODBUS serial ADU, size=%d\n", inSize_ );  
                        return 0;
                    }
                    serialPacket = (const MODBUS_SERIAL_PACKET *)inBuf_;

                    /* Check CRC here */
                    crc = calculate_crc( (unsigned char *)serialPacket, 0, inSize_ - sizeof( crc ) );
                    /* To avoid data alignment exception, use memcpy here */
                    memcpy( &crcInPkt,
                            ( (char *)serialPacket ) + ( inSize_ - sizeof( crcInPkt ) ),
                            sizeof( crcInPkt ) );
                    /* Assumes the processor is of big-endian */
                    crc = htons(crc);
                    if ( crc != crcInPkt ) {
                        syslog( LOGOPTS, "CRC error in MODBUS serial ADU [%02x,%02x:%02x]\n", inSize_,crcInPkt, crc );  
                        return 0; /*crc check should be done and cannot return -1*/
                    }
                    if (modbus_encryption_enable) {
                        encryptedModbusTcpPacket = (ENCRYPTED_MODBUS_TCP_PACKET *)outBuf_;
                        encryptedModbusTcpPacket->transactionId = htons(s_transactionId);
                        encryptedModbusTcpPacket->protocolId    = htons(s_protocolId);
                        encryptedModbusTcpPacket->length        = htons(inSize_ - 2); //remove CRC
                        tempBuf=malloc(inSize_+1);
                        if (tempBuf==NULL) {
                            syslog( LOGOPTS, "malloc error\n");  
                            return 0;        
                        } else {
                            memcpy(tempBuf,inBuf_,inSize_);
                        }
                        N2=modbusEncrpt(tempBuf,inSize_-2);
                        if (N2<8) {
                            syslog( LOGOPTS, "modbusEncrpt error\n");  
                            return 0;
                        }
                        memcpy( encryptedModbusTcpPacket->encrpteddata, tempBuf, N2);
                        free(tempBuf);
                        tempBuf=NULL;

                        //syslog( LOGOPTS, "modbus encrypted len:%d\n",MODBUS_TCP_ADU_OVERHEAD + N2 - MODBUS_SERIAL_ADU_OVERHEAD);  
                        return MODBUS_TCP_ADU_OVERHEAD + N2 - 1;
                    } else {
                        tcpPacket = (MODBUS_TCP_PACKET *)outBuf_;
                        tcpPacket->transactionId = htons(s_transactionId);
                        tcpPacket->protocolId    = htons(s_protocolId);
                        tcpPacket->length        = htons(inSize_ - MODBUS_SERIAL_ADU_OVERHEAD + 1);
                        tcpPacket->unitId        = serialPacket->slaveAddr;
                        tcpPacket->funcCode      = serialPacket->funcCode;
                        memcpy( tcpPacket->data, serialPacket->data, inSize_ - MODBUS_SERIAL_ADU_OVERHEAD - 1 );
                        return MODBUS_TCP_ADU_OVERHEAD + inSize_ - MODBUS_SERIAL_ADU_OVERHEAD;
                    }


                }


/*-----------------------------------------------------------------------------
 * Function: modbus_tcp_to_serial
 * Return:   the size of the converted packet, or -1 in the case of error
 *---------------------------------------------------------------------------*/
                static int modbus_tcp_to_serial( const char * inBuf_, int inSize_, void * outBuf_)
                {
                    const MODBUS_TCP_PACKET * tcpPacket;
                    MODBUS_SERIAL_PACKET * serialPacket;
                    //ENCRYPTED_MODBUS_TCP_PACKET *encryptedModbusTcpPacket;

                    int tcpDataSize;
                    int ModbusDataSize;

                    unsigned short crc;
                    int N2=-1;
                    //int i;
                    char* tempBuf=NULL;
                    if ( inBuf_ == NULL ) {
                        syslog( LOGOPTS, "%s inBuf_ == NULL\n",__func__ );  
                        return 0;
                    }
                    if ( inSize_ <= 0 ) {
                        syslog( LOGOPTS, "%s inSize_=%d\n",__func__,inSize_ );  
                        return 0;
                    }
                    if ( inSize_ > MODBUS_TCP_ADU_MAX_SIZE ) {
                        syslog( LOGOPTS, "Oversize MODBUS tcp ADU, size=%d\n", inSize_ );  
                        return 0;
                    }
                    if ( inSize_ < MODBUS_TCP_ADU_OVERHEAD - MODBUS_SERIAL_ADU_OVERHEAD) {
                        syslog( LOGOPTS, "Less MODBUS serial ADU, size=%d\n", inSize_ );  
                        return 0;
                    }

                    //syslog( LOGOPTS, "correct MODBUS tcp ADU, size=%d\n", inSize_ );  
                    tcpPacket = (const MODBUS_TCP_PACKET *)inBuf_;
                    serialPacket = (MODBUS_SERIAL_PACKET *)outBuf_;
                    s_transactionId         = ntohs(tcpPacket->transactionId);
                    s_protocolId            = ntohs(tcpPacket->protocolId);

                    /* Copy the data */
                    tcpDataSize = ntohs(tcpPacket->length); /*the real data value*/
                    //syslog( LOGOPTS, "correct MODBUS tcp ADU, tcpDataSize=%d\n", tcpDataSize );  
                    if (tcpDataSize>1000) {
                        syslog( LOGOPTS, "%s: modbusDecrypt error len=%d\n",__func__,tcpDataSize);  
                        return 0;
                    }

                    if (modbus_encryption_enable) {
                        N2 = inSize_ -6;
                        if ((N2<8)||((N2&3)!=0)) {
                            syslog( LOGOPTS, "N2=%d is not right no need to decrypt\n", N2 );  
                            return 0;
                        }

                        if (inSize_) {
                        }

                        tempBuf=malloc(inSize_+1);
                        if (tempBuf==NULL) {
                            syslog( LOGOPTS, "malloc error\n");  
                            return 0;        
                        } else {
                            memcpy(tempBuf,inBuf_+6,N2);
                        }
                        N2=modbusDecrypt(tempBuf,tcpDataSize);

                        if (N2<8) {
                            syslog( LOGOPTS, "modbusDecrypt error\n");  
                            return 0;
                        }

                        /*recaculate packet*/

                        serialPacket->slaveAddr = tempBuf[0];
                        serialPacket->funcCode  = tempBuf[1];

                        ModbusDataSize = tcpDataSize -2;

                        memcpy( serialPacket->data, &tempBuf[2], ModbusDataSize );
                        free(tempBuf);
                        tempBuf=NULL;
                        /* Calculate CRC here */
                        crc = calculate_crc( (unsigned char *)serialPacket, 0, 2 + ModbusDataSize );
                        crc = ntohs(crc);
                        memcpy( serialPacket->data + ModbusDataSize, &crc, sizeof( crc ) );

                        return ModbusDataSize + 4;
                    } else { /*not modbus encryption*/
                        tcpPacket = (const MODBUS_TCP_PACKET *)inBuf_;
                        serialPacket = (MODBUS_SERIAL_PACKET *)outBuf_;
                        s_transactionId         = ntohs(tcpPacket->transactionId);
                        s_protocolId            = ntohs(tcpPacket->protocolId);
                        serialPacket->slaveAddr = tcpPacket->unitId;
                        serialPacket->funcCode  = tcpPacket->funcCode;

                        /* Copy the data */
                        ModbusDataSize = inSize_ - MODBUS_TCP_ADU_OVERHEAD - 1;
                        memcpy( serialPacket->data, tcpPacket->data, ModbusDataSize );

                        /* Calculate CRC here */
                        crc = calculate_crc( (unsigned char *)serialPacket, 0, 2 + ModbusDataSize );
                        crc = ntohs(crc);
                        memcpy( serialPacket->data + ModbusDataSize, &crc, sizeof( crc ) );

                        return MODBUS_SERIAL_ADU_OVERHEAD + inSize_ - MODBUS_TCP_ADU_OVERHEAD;
                    }

                }
/*---------------------------------------------------------------------------*/
                unsigned short calculate_crc(unsigned char buf_[], int start_, int count_)
                {
                    int i, j;
                    unsigned short temp, temp2, flag;

                    temp = 0xffff;

                    for ( i = start_; i < count_; i++ ) {
                        temp = ( temp ^ buf_[ i ] );

                        for ( j = 0; j < 8; j++ ) {
                            flag = ( temp & 0x0001 );
                            temp = ( temp >> 1 );
                            if ( flag ) {
                                temp = temp ^ 0xa001;
                            }
                        }
                    }

                    /* Reverse byte order. */
                    temp2 = temp >> 8;
                    temp = ( ( temp << 8 ) | temp2 );
                    return temp;
                }




#include <stdint.h>

#define DELTA 0x9e3779b9
#define MX ((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (k[(p&3)^e] ^ z));



                void btea(uint32_t *v, int n, uint32_t const k[4]) {
                    uint32_t y, z, sum;
                    unsigned int  p, rounds, e;
                    if (n > 1) {          /* Coding Part */
                        rounds = 6 + 52/n;
                        sum = 0;
                        z = v[n-1];
                        do {
                            sum += DELTA;
                            e = (sum >> 2) & 3;
                            for (p=0; p<n-1; p++)
                                y = v[p+1], z = v[p] += MX;
                            y = v[0];
                            z = v[n-1] += MX;
                        } while (--rounds);

                    } else if (n < -1) {  /* Decoding Part */
                        //syslog( LOGOPTS, "%s N_=%d\n", __func__,n);  
                        n = -n;
                        rounds = 6 + 52/n;
                        sum = rounds*DELTA;
                        y = v[0];
                        do {
                            e = (sum >> 2) & 3;
                            for (p=n-1; p>0; p--)
                                z = v[p-1], y = v[p] -= MX;
                            z = v[n-1];
                            y = v[0] -= MX;
                        } while ((sum -= DELTA) != 0);
                    }
                }




                static int modbusEncrpt(char* inBuf_,int N_)
                {
                    int N2=-1;
                    int i=0;

                    //syslog( LOGOPTS, "modbusEncrpt N_=%d\n", N_);  
                    if (N_<2) {
                        return -1;
                    } else {
                        N2 = max(8, (N_+3)&~3);
                        for (i=0;i<N2-N_;i++) {
                            inBuf_[N_+i]=rand()%255;
                            //syslog( LOGOPTS, "%s before  inBuf_[N_+i]=%02x,i=%d\n",__func__, inBuf_[N_+i],N_+i);  
                        }

                        btea((uint32_t*) inBuf_, N2/4, ModbusEncryptionKey); 

                    }
                    return N2;
                }

                static int modbusDecrypt(char* inBuf_,int N_)
                {
                    int N2=-1;

                    int i=0;

                    //syslog( LOGOPTS, "modbusDecrypt N_=%d\n", N_);  


                    if (N_<2) {
                        return -1;
                    } else {
                        N2 = max(8, (N_+3)&~3);
                        for (i=0;i<N2;i++) {
                            //syslog( LOGOPTS, "modbusDecrypt before decrypt inBuf_[i]=%02x,i=%d\n", inBuf_[i],i);  
                        }
                        btea((uint32_t*) inBuf_, 0-N2/4, ModbusEncryptionKey); 
                    }
                    for (i=0;i<N2;i++) {
                        //syslog( LOGOPTS, "modbusDecrypt after decrypt inBuf_[i]=%02x,i=%d\n", inBuf_[i],i);  
                    }
                    return N2;
                }


                static int ModbusStringKeytoIntKey(char* inStrKey_, int * KeyOut_)
                {

                    memcpy(KeyOut_,(int*)inStrKey_, sizeof(inStrKey_)/4);

                    return true;
                }

                static int setInitialMultiTcpValue()
                {
                    memset(remote_tcp_client_list,0,sizeof(remote_tcp_client_list));
                    current_tcp_client_num = 0;
                    high_tcp_socket = 0;
                    return true;
                }


                int setTcpSocketAlive(int s)
                {
#define KEEP_IDLE 60
#define KEEP_COUNT 5
#define KEEP_INTVL 60

                    int optval;
                    socklen_t optlen = sizeof(optval);

                    /* Check the status for the keepalive option */
                    if (getsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) < 0) {
                        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
                    }
                    //syslog( LOGOPTS,"SO_KEEPALIVE is %s\n", (optval ? "ON" : "OFF"));

                    /* Set the option active */
                    optval = 1;
                    optlen = sizeof(optval);
                    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
                        syslog( LOGOPTS,"%s setsockopt %s\n", __func__,strerror(errno));
                    }
                    //syslog( LOGOPTS,"SO_KEEPALIVE set on socket\n");

                    /* Check the status again */
                    if (getsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) < 0) {
                        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
                    }

                    if (getsockopt(s, SOL_TCP, TCP_KEEPCNT, &optval, &optlen) < 0) {
                        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
                    }

                    optval = KEEP_COUNT;
                    if (setsockopt(s, SOL_TCP, TCP_KEEPCNT, &optval, optlen) < 0) {
                        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
                    }
                    if (getsockopt(s, SOL_TCP, TCP_KEEPCNT, &optval, &optlen) < 0) {
                        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
                    }


                    if (getsockopt(s, SOL_TCP, TCP_KEEPIDLE, &optval, &optlen) < 0) {
                        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
                    }

                    optval = KEEP_IDLE;
                    if (setsockopt(s, SOL_TCP, TCP_KEEPIDLE, &optval, optlen) < 0) {
                        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
                    }

                    if (getsockopt(s, SOL_TCP, TCP_KEEPIDLE, &optval, &optlen) < 0) {
                        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
                    }


                    if (getsockopt(s, SOL_TCP, TCP_KEEPINTVL, &optval, &optlen) < 0) {
                        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
                    }


                    optval = KEEP_INTVL;
                    if (setsockopt(s, SOL_TCP, TCP_KEEPINTVL, &optval, optlen) < 0) {
                        syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));

                        if (getsockopt(s, SOL_TCP, TCP_KEEPINTVL, &optval, &optlen) < 0) {
                            syslog( LOGOPTS,"%s getsockopt %s\n", __func__,strerror(errno));
                        }
                    }

                    return 0;
                }

/**********************************************************************************
                  END THANK YOU  
**********************************************************************************/







                
