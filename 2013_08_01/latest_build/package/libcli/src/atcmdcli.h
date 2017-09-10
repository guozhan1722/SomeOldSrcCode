#include <stdio.h>
#include <sys/types.h>
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <fcntl.h>

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
#include <linux/types.h>

#define CLITEST_PORT                8000
#define MODE_CONFIG_INT             10

#ifdef __GNUC__
# define UNUSED(d) d __attribute__ ((unused))
#else
# define UNUSED(d) d
#endif

#include "libcli.h"
#include "gnokii.h"
#include "atgen.h"
#include "uci.h"


extern int cmd_echo_ok(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_at_test(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cli_at_history(struct cli_def *cli, UNUSED(char *command), UNUSED(char *argv[]), UNUSED(int argc));
extern int cli_at_list(struct cli_def *cli, UNUSED(char *command), UNUSED(char *argv[]), UNUSED(int argc));
extern int cli_at_quit(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_conf_read(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_display_sysconf(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_conf_enable(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_reboot(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_dis_service(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_admin_pwd(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_auth_conf(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_ppp_urd(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_icmp_enable(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_icmp_ka(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_ddns_enable(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_ddns_conf(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);


extern int cmd_read_sms(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_read_sms_unchange(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_list_sms(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_list_sms_unchange(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_delete_sms(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_send_sms(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_sys_info(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_modem_record(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_modem_name(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_manu_id(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_phone_number(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_modem_imi(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_modem_sim(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);

extern int cmd_com2_port_status(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);

extern int cmd_local_eip(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_dhcp_enable(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_dhcp_address(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_eth_mac(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_local_weip(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_wdhcp_enable(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_wdhcp_address(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_weth_mac(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_wan_sip(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_lan_sip(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_lan_sct(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_wan_sct(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);

extern int cmd_define_ntp(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_reset_default(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_traffic_wtimer(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_default_button(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_sms_command(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);


extern int cmd_nat_conf(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_ppp_status(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_ip_pass(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_dod_conf(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_idle_timeout(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_connect_timeout(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);

extern int cmd_com2_baud_rate(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_data_format(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_data_mode(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_ct(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_mps(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_priority(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_ncdi(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_mtc(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_ipm(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_tcpclient(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_tcpserver(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_tcpcs(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_upp(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_upmp(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_upmm(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_com2_umpmp(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);

extern int cmd_eurd_1(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_eurd_2(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_eurd_3(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_nms_r(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);

extern int cmd_gpsr_1(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_gpsr_2(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_gpsr_3(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_gpsr_4(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);

extern int cmd_input_s(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);
extern int cmd_output_s(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);

extern int cmd_console_timeout(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);


extern int cmd_not_defined(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc);

