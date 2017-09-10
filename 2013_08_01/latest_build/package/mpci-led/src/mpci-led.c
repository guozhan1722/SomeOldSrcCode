#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <net/if.h>
#include <errno.h>
#include <string.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>  
#include <netlink/msg.h>
#include <netlink/attr.h>

#include <linux/nl80211.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "iw_rssi.h"
#include "mpci-bsp.h"

#define DEVICE_NAME "/dev/vip-bsp"
#define I2C_DEVICE_NAME "/dev/i2c-0"

#define LEDMAN_CMD_OFF      0xFFFF0000
#define LEDMAN_CMD_ON       0xFFFF0001
#define LEDMAN_CMD_FLASH    0xFFFF0002
#define LED_ALL             0xFFFF0003
/*
static int I2C_RADIO_LED1 = 5;
static int I2C_RADIO_LED2 = 6;
static int I2C_RADIO_LED3 = 7;
*/
static int i2c_radio_led1 = 5;
static int i2c_radio_led2 = 6;
static int i2c_radio_led3 = 7;
static int hardware_version = 1;

static int i2c_address=0x18;
static int i2c_device_fd   = -1;
static int device_fd   = -1;
static int iface_fd = -1;

unsigned int done = 0;

void killHandler(int signum);

int RSSI(char * abc);

static int B2T_MAX = 3;
static int signal_s;
static int tx_pks;
static int rx_pks;
static int tx_pks_ori=0;
static int rx_pks_ori=0;
static int tx_run=0;
static int rx_run=0;
static int i2c_oldvalue ;

char *command;
char *time_str;
int my_time = 1000 * 1000;

static int i2c_bus_access(int file, char read_write, int command, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = I2C_SMBUS_BYTE_DATA;
	args.data = data;
	return ioctl(file,I2C_SMBUS,&args);
}

static void ledman_cmd(int led_cmd, int led)
{
    union i2c_smbus_data i2c_data;
    int led_mask,retval;


    
    if(led == LED_ALL ) {
        //led_mask =  (1 << I2C_RADIO_LED1)| (1<< I2C_RADIO_LED2) | (1<< I2C_RADIO_LED3);
        led_mask =  (1 << i2c_radio_led1)| (1<< i2c_radio_led2) | (1<< i2c_radio_led3);
    } else
        led_mask = 1 << led;
    //fprintf(stderr, "Flag hardware_version = %d\n", hardware_version);
    //fprintf(stderr, "led_mask 0x%x\n", led_mask);
    //fprintf(stderr, "i2c_oldvalue 0x%x\n", i2c_oldvalue);

    if (led_cmd == LEDMAN_CMD_OFF ) {
	if(hardware_version == 2){
             i2c_oldvalue = i2c_oldvalue | led_mask;
        } else {
             i2c_oldvalue = i2c_oldvalue & ~led_mask;
        }
    } else if (led_cmd == LEDMAN_CMD_ON) {
	if(hardware_version == 2){
             i2c_oldvalue= i2c_oldvalue & ~led_mask;
        } else {
             i2c_oldvalue= i2c_oldvalue |  led_mask;
        } 
    } else if (led_cmd == LEDMAN_CMD_FLASH) {
        i2c_oldvalue= i2c_oldvalue ^  led_mask;
    } 

    //fprintf(stderr, "i2c_oldvalue 0x%x\n", i2c_oldvalue);
    i2c_data.byte = i2c_oldvalue;
    //fprintf(stderr, "i2c_data.byte 0x%x\n", i2c_data.byte);

    i2c_device_fd = open(I2C_DEVICE_NAME, O_RDWR);

    if (i2c_device_fd < 0) {
        fprintf(stderr, "can not open i2c dev %s\n",I2C_DEVICE_NAME);
    }

    retval = ioctl(i2c_device_fd, I2C_SLAVE_FORCE , i2c_address); 
    if (retval < 0) {
        fprintf(stderr, "can not set slave address 0x%x\n",i2c_address);
    }

    i2c_bus_access(i2c_device_fd, I2C_SMBUS_WRITE, I2C_SMBUS_OUTPUT_REG, &i2c_data);
    close(i2c_device_fd);

}

static int i2c_led_init(void)
{
    union i2c_smbus_data i2c_data;
    int retval = -1;

    i2c_device_fd = open(I2C_DEVICE_NAME, O_RDWR);

    if (i2c_device_fd < 0) {
        fprintf(stderr, "can not open i2c dev %s\n",I2C_DEVICE_NAME);
        return retval; 
    }

    retval = ioctl(i2c_device_fd, I2C_SLAVE_FORCE , i2c_address); 
    if (retval < 0) {
        fprintf(stderr, "can not set slave address 0x%x\n",i2c_address);
        return retval; 
    }

    /*init i2c-gpio to output*/
    i2c_data.byte = 0;
    retval = i2c_bus_access(i2c_device_fd, I2C_SMBUS_WRITE, I2C_SMBUS_CONFIG_REG, &i2c_data);
    if (retval < 0) {
        fprintf(stderr, "can not write i2c reg \n");
        return retval; 
    }

    /*init i2c-gpio no POLARITY invert*/
    i2c_data.byte = 0;
    retval=i2c_bus_access(i2c_device_fd, I2C_SMBUS_WRITE, I2C_SMBUS_POLARITY_REG, &i2c_data);
    if (retval < 0) {
        fprintf(stderr, "can not write i2c reg \n");
        return retval; 
    }

    /*read i2c init value*/
    i2c_data.byte = 0;
    retval=i2c_bus_access(i2c_device_fd, I2C_SMBUS_READ, I2C_SMBUS_INPUT_REG, &i2c_data);
    if (retval < 0) {
        fprintf(stderr, "can not read i2c reg \n");
        return retval; 
    }

    /*init i2c-gpio output values*/
    i2c_data.byte |= 1 << I2C_CPU_RSMODE;
    i2c_data.byte |= 1 << I2C_RADIO_RESET;
    
    retval=i2c_bus_access(i2c_device_fd, I2C_SMBUS_WRITE, I2C_SMBUS_OUTPUT_REG, &i2c_data);
    if (retval < 0) {
        fprintf(stderr, "can not write i2c reg \n");
        return retval; 
    }

    i2c_oldvalue= i2c_data.byte;
    close(i2c_device_fd);
    return 0;
}


int identify_hardware_version()
{
    int version_fd;
    int read_cnt;
    char version[2048];
    char line[1024];
    char *hardware;
    char str;
    int i=0;

    version_fd=open("/etc/version", O_RDWR );
    if(version_fd > 0) {
        read_cnt = read(version_fd, version, sizeof version);  
        version[read_cnt]='\0';
        close(version_fd);
    } else {
        fprintf(stderr,"Can not open the version file /etc/version\n");
        return -1;
    }
    //printf("%s\n", version);

    hardware = strstr(version, "HARDWARE=");
    if (hardware == NULL){
        fprintf(stderr,"No HARDWARE parameter found in file /etc/version\n");
        return -1;
    }
    hardware += strlen("HARDWARE=");
    str = hardware[0];
    //fprintf(stderr, "FOUND string: %c \n", str);
    while (str != '\n'){
	line[i] = str;
	hardware++;
        str = hardware[0];
        //fprintf(stderr, "FOUND string: %c \n", str);
	i++; 	
	}
    fprintf(stderr, "FOUND HARDWARE version : %s \n", line);


    if (strcmp(line, "v2.0.0") == 0){
       //I2C_RADIO_LED1 = 1;
       //I2C_RADIO_LED2 = 2;
       //I2C_RADIO_LED3 = 3;
       i2c_radio_led1 = 1;
       i2c_radio_led2 = 2;
       i2c_radio_led3 = 3;
       hardware_version = 2;
    }

    //fprintf(stderr, "Hardware: version <%d> rssi_led < %d %d %d >\n", hardware_version, i2c_radio_led1, i2c_radio_led2, i2c_radio_led3);
    return 0;

}


int main (int argc, char *argv[]) {

    int led = 0;
    char c;
    int led_on = 0;

    int sequence = 0;
    int i = 0, rssi = 0;
    int ret = identify_hardware_version();

    //int bottom2top[] = {I2C_RADIO_LED1,I2C_RADIO_LED2,I2C_RADIO_LED3};
    int bottom2top[3];
    if( hardware_version == 2 ){
    	bottom2top[0] = i2c_radio_led3;
    	bottom2top[1] = i2c_radio_led2;
    	bottom2top[2] = i2c_radio_led1;
    } else {
    	bottom2top[0] = i2c_radio_led1;
    	bottom2top[1] = i2c_radio_led2;
    	bottom2top[2] = i2c_radio_led3;
    }
    //static int curled = I2C_RADIO_LED1;
    //int curled = I2C_RADIO_LED1;
    int curled = i2c_radio_led3;
    struct sigaction sa;
    int retval=-1;
    int i2c_value;
    union i2c_smbus_data i2c_data;
    int rssi_on = 0;
    int rssi_flash = 0;
    int rssi_tmp;
    char ifaces[256];
    char wiface1[32];
    char *start,*end;
    int read_cnt=0;
    int max_i_rssi;
    (void) setsid();
    //int ret = identify_hardware_version();
    if (ret == -1){
	printf("<Error Happen> - can not identify hardware version for rssi led.\n");
        exit(-1);
    }

    switch (argc) {
    case 1 :
    case 2 :
        usage:      
        fprintf(stderr, "Usage:\t%s -s <sequence> -t <time ms>\n", argv[0]);
        fprintf(stderr, "\t\t -l <led 1-3 > -o <0-off,1-on,2-flash> \n");
        exit(1);
        break;
    default :
        while ((c = getopt (argc, argv, "s:l:t:o:")) != -1) {
            switch (c) {
            case 's':
                sequence = atoi(optarg);
                break;
            case 'l':
                led = atoi(optarg);
                led = led + 4;
                break;
            case 't':
                my_time = atoi(optarg)  * 1000;
                break;

            case 'o':
                led_on = atoi(optarg);
                break;

            default:
                goto usage;
                break;
            }
        }
        break;
    } 

    sa.sa_handler = (void *)killHandler;
    //sa.sa_mask = 0; //dont block any signals while this one is working
    sa.sa_flags = SA_RESTART; //restart the signal
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGKILL, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    sigaction(SIGUSR2, &sa, NULL);

    retval = i2c_led_init();

    if (retval < 0 ) {
        fprintf(stderr, "can not init i2c dev\n");
        close(i2c_device_fd);
        return retval; 
    }
    
    switch (sequence) {
    case 1 :
        while (!done) {
            for (i = 0; i < B2T_MAX; i++) {
                ledman_cmd(LEDMAN_CMD_ON, bottom2top[i]);
                usleep(my_time);
                ledman_cmd(LEDMAN_CMD_OFF, bottom2top[i]);
            }
        }
        break;

    case 2:
        while (!done) {
            for (i = B2T_MAX - 1; i >= 0; i--) {
                ledman_cmd(LEDMAN_CMD_ON, bottom2top[i]);
                usleep(my_time);
                ledman_cmd(LEDMAN_CMD_OFF, bottom2top[i]);
            }
        }
        break;

    case 3:
        ledman_cmd(LEDMAN_CMD_OFF,LED_ALL);
        while (!done) {
            for (i = 0; i < B2T_MAX; i++) {
                ledman_cmd(LEDMAN_CMD_ON, bottom2top[i]);
                usleep(my_time);
            }
            ledman_cmd(LEDMAN_CMD_OFF, LED_ALL);
            usleep(my_time);
        }
        break;

    case 4:
        while (!done) {
            ledman_cmd(LEDMAN_CMD_ON, LED_ALL);
            usleep(my_time);
            ledman_cmd(LEDMAN_CMD_OFF,LED_ALL);
            usleep(my_time);
        }
        break;

    case 5 :
        ledman_cmd(LEDMAN_CMD_OFF, LED_ALL);
        while (!done) {

//here fixed the led for multi interfaces
#if 1
            system("ls /sys/class/ieee80211/phy0/device/net/ > /var/run/wiface");

            iface_fd=open("/var/run/wiface", O_RDWR );
            if(iface_fd > 0) {
              read_cnt= read(iface_fd,ifaces, sizeof(ifaces));
              ifaces[read_cnt]='\0';
              close(iface_fd);
            } else {
                fprintf(stderr,"Can not open the interface list file /var/run/wiface \n");
                return -1;
            }

            start=ifaces;
            end=ifaces;

            max_i_rssi=0;
            for(i=0;i<read_cnt;i++) {
                //found the end of iface name
                if(ifaces[i]== 0xa) {
                    end=&ifaces[i];
                    strncpy (wiface1, start,(end - start));
                    wiface1[end-start]='\0';
                    start=end+1;
                    if(strncmp(wiface1,"mon",3)==0) {
                        //fprintf(stderr,"JBDG: This is the monitor interface do not care\n");
                        continue ;
                    }else {
                        signal_s = 0;
                        RSSI(wiface1);
                        if(max_i_rssi != 0) 
                            max_i_rssi = (max_i_rssi>signal_s) ? max_i_rssi:signal_s;
                        else
                            max_i_rssi=signal_s;
                        //fprintf(stderr,"JDBG: single iface = %s   max rssi = %d \n",wiface1,max_i_rssi);
                    }
                }
            }

            rssi = 110 + max_i_rssi;
            if(rssi < 0) 
                rssi=0;
            else if(rssi>110) {
                rssi=110;
            }
#else
//The old code 2011/08/26
            signal_s=0;
            RSSI("wlan0");
            rssi = 110 + signal_s;

            signal_s=0;
            RSSI("wlan0.sta1");
            rssi_tmp = 110 + signal_s;

            if (rssi == 110 || rssi_tmp == 110) {
                if ( rssi > rssi_tmp){
                    rssi= rssi_tmp;
                }
            } else if (rssi_tmp > rssi) {
                rssi=rssi_tmp;
            }

            signal_s=0;
            RSSI("wlan0.rep");
            fprintf(stderr, "repeater signal strength: signal = %d others rssi = %d\n",signal_s, rssi);
            rssi_tmp = 110 + signal_s;

            if (rssi == 110 || rssi_tmp == 110) {
                if ( rssi > rssi_tmp){
                    rssi= rssi_tmp;
                }
            } else if (rssi_tmp > rssi) {
                rssi=rssi_tmp;
            }

            fprintf(stderr, "signal strength: signal = %d rssi = %d\n",signal_s, rssi);
#endif    
            if (rssi == 110 ) {
                ledman_cmd(LEDMAN_CMD_OFF, LED_ALL);
                //if (curled > I2C_RADIO_LED3) curled = I2C_RADIO_LED1;
                if( hardware_version == 2 ){
                    ledman_cmd(LEDMAN_CMD_ON, curled--);
                    if (curled < i2c_radio_led1)    curled = i2c_radio_led3;
                } else {
    	            ledman_cmd(LEDMAN_CMD_ON, curled++);
                    if (curled > i2c_radio_led3)    curled = i2c_radio_led1;
                }            
                                
            } else if (rssi >= 29) {
                    ledman_cmd(LEDMAN_CMD_ON, LED_ALL);
            } else {
                rssi_on = rssi / 8;
                rssi_flash = (rssi / 4) % 2 ;
                /*turn on leds*/
                for (i = 0; i < rssi_on; i++) {
                    ledman_cmd(LEDMAN_CMD_ON, bottom2top[i]);
                }
    
                if(rssi_on == 0) {
                     rssi_flash = 1;
                }
    
                /*flash leds*/
                if (rssi_flash != 0) {
                    ledman_cmd(LEDMAN_CMD_FLASH, bottom2top[rssi_on]);
                    i= rssi_on + 1;
                }  else
                    i = rssi_on;
    
                /*turn off others leds*/
                for (i = i; i < B2T_MAX; i++) {
                    ledman_cmd(LEDMAN_CMD_OFF, bottom2top[i]);
                }
            }
            usleep(my_time);
        }
        break;
    }

    if (sequence == 0) {
        ledman_cmd(LEDMAN_CMD_OFF, LED_ALL);
        if (led_on == 0) {
            ledman_cmd(LEDMAN_CMD_OFF, led);
        }
        if (led_on == 1) {
            ledman_cmd(LEDMAN_CMD_ON, led);
        }
        if (led_on == 2) {
            ledman_cmd(LEDMAN_CMD_FLASH, led);
        }
    }

    return 0;
}

void killHandler(int signum) {

    switch (signum) {
    case SIGINT:
    case SIGKILL:
        done = 1;
        ledman_cmd(LEDMAN_CMD_FLASH, LED_ALL);
        break;

    case SIGTERM:
        done = 1;
        break;

    case SIGUSR2:
        ledman_cmd(LEDMAN_CMD_FLASH, LED_ALL);
       // defaults(8);
        ledman_cmd(LEDMAN_CMD_OFF, LED_ALL);

        break;
    }
    
    return;
}

int iw_debug = 0;

static void nl80211_cleanup(struct nl80211_state *state)
{
	genl_family_put(state->nl80211);
	nl_cache_free(state->nl_cache);
	nl_socket_free(state->nl_sock);
}

static int print_sta_handler(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	char mac_addr[20], state_name[10], dev[20];
	static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
		[NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
		[NL80211_STA_INFO_TX_BITRATE] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_LLID] = { .type = NLA_U16 },
		[NL80211_STA_INFO_PLID] = { .type = NLA_U16 },
		[NL80211_STA_INFO_PLINK_STATE] = { .type = NLA_U8 },
	};

	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
		[NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
		[NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_40_MHZ_WIDTH] = { .type = NLA_FLAG },
		[NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
	};

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	/*
	 * TODO: validate the interface and mac address!
	 * Otherwise, there's a race condition as soon as
	 * the kernel starts sending station notifications.
	 */

	if (!tb[NL80211_ATTR_STA_INFO]) {
		fprintf(stderr, "sta stats missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
			     tb[NL80211_ATTR_STA_INFO],
			     stats_policy)) {
		fprintf(stderr, "failed to parse nested attributes!\n");
		return NL_SKIP;
	}

	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);

//    if (sinfo[NL80211_STA_INFO_SIGNAL])
//		printf("\n\tsignal:  \t%d dBm",
//			(int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]));

//	printf("\n");
    if (sinfo[NL80211_STA_INFO_SIGNAL]){
        signal_s = (int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]);
        tx_pks = nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]);
        rx_pks = nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]);

    } else {
        signal_s=0;
        tx_pks=0;
        rx_pks=0;
    }

    return NL_SKIP;
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
			 void *arg)
{
	int *ret = arg;
	*ret = err->error;
	return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_STOP;
}

static int nl80211_init(struct nl80211_state *state)
{
	int err;

	state->nl_sock = nl_socket_alloc();
    if (!state->nl_sock) {
		fprintf(stderr, "Failed to allocate netlink socket.\n");
		return -ENOMEM;
	}

	if (genl_connect(state->nl_sock)) {
		fprintf(stderr, "Failed to connect to generic netlink.\n");
		err = -ENOLINK;
		goto out_handle_destroy;
	}

	if (genl_ctrl_alloc_cache(state->nl_sock, &state->nl_cache)) {
		fprintf(stderr, "Failed to allocate generic netlink cache.\n");
		err = -ENOMEM;
		goto out_handle_destroy;
	}

	state->nl80211 = genl_ctrl_search_by_name(state->nl_cache, "nl80211");
    if (!state->nl80211) {
       fprintf(stderr, "nl80211 not found.\n");
		err = -ENOENT;
		goto out_cache_free;
	}

	return 0;

out_cache_free:
	nl_cache_free(state->nl_cache);
out_handle_destroy:
	nl_socket_free(state->nl_sock);
	return err;
}

static int handle_station_dump(struct nl80211_state *state,
			       struct nl_cb *cb,
			       struct nl_msg *msg,
			       int argc, char **argv)
{
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_sta_handler, NULL);
	return 0;
}

static struct cmd my_cmd=						
{	
    .name = "dump",					
    .args = NULL,					
    .cmd = NL80211_CMD_GET_STATION,					
    .nl_msg_flags = NLM_F_DUMP,				
    .hidden = 0,					
    .idby = CIB_NETDEV,					
    .handler = handle_station_dump,					
    .parent = "station",					
} ;

int RSSI(char *ifname)
{
//	char *ifname = "wlan0";
    struct nl80211_state nlstate;
    struct nl80211_state *state;
    int err;
    int argc;
    char ab[20][256];
    char **argv;
    struct cmd *cmd = NULL;
    int rssi;

   // SECTION(station);

    
    cmd=&my_cmd;

    struct nl_cb *cb;
    struct nl_msg *msg;
    int devidx = 0;

    err = nl80211_init(&nlstate);
    if (err)
        return 1;

//    devidx = if_nametoindex("wlan0");
    devidx = if_nametoindex(ifname);
    if (devidx == 0)
        devidx = -1;

    cmd=&my_cmd;
    state=&nlstate;

    msg = nlmsg_alloc();
    if (!msg) {
        fprintf(stderr, "failed to allocate netlink message\n");
        return 2;
    }

    cb = nl_cb_alloc(iw_debug ? NL_CB_DEBUG : NL_CB_DEFAULT);
    if (!cb) {
        fprintf(stderr, "failed to allocate netlink callbacks\n");
        err = 2;
        goto out_free_msg;
    }

    genlmsg_put(msg, 0, 0, genl_family_get_id(state->nl80211), 0,
            cmd->nl_msg_flags, cmd->cmd, 0);

    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);

    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_sta_handler, NULL);

    err = nl_send_auto_complete(state->nl_sock, msg);

    err = 1;

    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

    while (err > 0)
        nl_recvmsgs(state->nl_sock, cb);

    out:
       nl_cb_put(cb);

    if (err < 0)
          fprintf(stderr, "command failed: %s (%d)\n", strerror(-err), err);

    nl80211_cleanup(&nlstate);

    out_free_msg:
       nlmsg_free(msg);
       return err;
    nla_put_failure:
       fprintf(stderr, "building message failed\n");
       return 2;
    return 0;
}

/* **********************************************
* Function : defaults
*
* Description : counts to reset system to default 
*
* Returns:  0 = Okay
********************************************** */
int defaults(int resetlimit)
{
    return 0;
}
