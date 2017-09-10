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
#include "libcli.h"
#include "atcmdcli.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CLITEST_PORT                8000
#define MODE_CONFIG_INT             10

#ifdef __GNUC__
# define UNUSED(d) d __attribute__ ((unused))
#else
# define UNUSED(d) d
#endif

unsigned int regular_count = 0;
unsigned int debug_regular = 0;

static char u_name[20];
static char u_passwd[20];

static int child_done=0;
static int child_status=256;
static int cmd_child_pid=0;

#define FILE_PATH "/etc/m_cli/"
#define CONF_FILE "cli.conf"

struct menu{
    char *name;
    char *desc;
    int *cmd;
    int cnt;
    struct menu *sub;
    struct menu *next;
};

static struct menu *pmenu_head;

#ifdef WIN32
typedef int socklen_t;

int winsock_init()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    // Start up sockets
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        // Tell the user that we could not find a usable WinSock DLL.
        return 0;
    }

    /*
     * Confirm that the WinSock DLL supports 2.2
     * Note that if the DLL supports versions greater than 2.2 in addition to
     * 2.2, it will still return 2.2 in wVersion since that is the version we
     * requested.
     * */
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        // Tell the user that we could not find a usable WinSock DLL.
        WSACleanup();
        return 0;
    }
    return 1;
}
#endif

char *strtrimr(char * buf)
{
    int len,i;
    char * tmp = NULL;
    len = strlen(buf);
    tmp = (char*)malloc(len);

    memset(tmp,0x00,len);
    for(i = 0;i < len;i++)
    {
        if ((buf[i] !=' ') && (buf[i] != 9))
            break;
    }
    if (i < len) {
        strncpy(tmp,(buf+i),(len-i));
    }
    strncpy(buf,tmp,len);
    free(tmp);
    return buf;
}

char *strtriml(char * buf)
{
    int len,i;
    char * tmp=NULL;
    len = strlen(buf);
    tmp = (char*)malloc(len);
    memset(tmp,0x00,len);
    for(i = 0;i < len;i++)
    {
        if( (buf[len-i-1] !=' ') && (buf[len-i-1] != 9) )
            break;
    }
    if (i < len) {
        strncpy(tmp,buf,len-i);
    }
    strncpy(buf,tmp,len);
    free(tmp);
    return buf;
}

void getvalue(char *s,char *value,FILE *fp)
{
    char buf[1024];
    bzero(value,sizeof(value));

    while(!feof(fp)){
        fgets(buf,sizeof(buf),fp);
        //printf("str buf = %d\n",strlen(buf));
        //if(buf[strlen(buf)-1]=='\n')
        // buf[strlen(buf)-1]=0;
        strtrimr(buf);
        strtriml(buf);
        if(buf[0]=='#')
            continue;
        if(strncmp(s,buf,strlen(s))==0){
            //printf("%d\n",buf[strlen(buf)-1]);
            sscanf(buf,"%*[^ ] %[^\n]",value);
        }
    }
    fseek(fp,0,SEEK_SET);
}

void getcmd(struct menu *head, FILE *fp)
{
    char buf[2048];
    char *start, *end;
    char tmp_buf[1024], get_cmd[64][1024];
    int i=0, cmd_cnt, is_new_menu;
    struct menu *p_menu;

    while(!feof(fp)){
        fgets(buf,sizeof(buf),fp);
        strtrimr(buf);
        strtriml(buf);
        if(strncmp("cmd",buf,strlen("cmd"))==0){
            if((start = strchr(buf,' '))!=NULL)
            {
                start = start+1;
                i=0;
                while((end = strchr(start,':'))!=NULL)
                {
                    memset(tmp_buf,0,sizeof(char)*1024);
                    memset(get_cmd[i],0,sizeof(get_cmd[i]));
                    strncpy (tmp_buf, start,(end - start));
                    tmp_buf[end - start]='\0';
                    strncpy (get_cmd[i], tmp_buf,strlen(tmp_buf));
                    i++;
                    start = end+1;
                    if(*start=='\0')
                        break;
                }

                cmd_cnt = i;
                i=0;
                p_menu = head;
                is_new_menu=0;

                if(p_menu->name == NULL)
                {
                    is_new_menu =1;
                    p_menu->name = malloc(sizeof(get_cmd[0]));
                    strncpy(p_menu->name,get_cmd[0],sizeof(get_cmd[0]));
                    p_menu->desc = malloc(sizeof(get_cmd[i]));
                    strncpy(p_menu->desc,get_cmd[1],sizeof(get_cmd[1]));
                    p_menu->next = NULL;
                    p_menu->sub = NULL;
                    p_menu->cmd = NULL;
                    p_menu->cnt = 1;
                }

                LOOP1:
                while(strncmp(p_menu->name,get_cmd[i],strlen(get_cmd[i]))!=0)
                {
                    if(p_menu->next !=NULL)
                    {
                        p_menu=p_menu->next;
                    }else{ 
                        is_new_menu =1;
                        p_menu->next = malloc(sizeof(struct menu));
                        p_menu = p_menu->next;
                        p_menu->name = malloc(sizeof(get_cmd[i]));
                        strncpy(p_menu->name,get_cmd[i],sizeof(get_cmd[i]));
                        p_menu->desc = malloc(sizeof(get_cmd[i+1]));
                        strncpy(p_menu->desc,get_cmd[i+1],sizeof(get_cmd[i+1]));
                        p_menu->next = NULL;
                        p_menu->sub = NULL;
                        break;
                    }
                }

                i=i+2;

                while( (cmd_cnt-i) > 0)
                {
                    if((cmd_cnt -i) ==1)
                    {
                        //p_menu->cmd = my_cmd;
                        p_menu->cmd = malloc(sizeof(get_cmd[i]));
                        strncpy(p_menu->cmd , get_cmd[i],strlen(get_cmd[i]));
                        break;
                    } else p_menu->cmd = NULL;


                    if((is_new_menu == 1) || (p_menu->sub == NULL))
                    {
                        p_menu->sub = malloc(sizeof(struct menu));
                        p_menu = p_menu->sub;
                        p_menu->name = malloc(sizeof(get_cmd[i]));
                        strncpy(p_menu->name,get_cmd[i],sizeof(get_cmd[i]));
                        p_menu->desc = malloc(sizeof(get_cmd[i+1]));
                        strncpy(p_menu->desc,get_cmd[i+1],sizeof(get_cmd[i+1]));
                        p_menu->cmd = NULL;
                        p_menu->cnt = i-1;
                        p_menu->sub = NULL;
                        p_menu->next = NULL;
                        i=i+2;
                    }else {
                        p_menu = p_menu->sub;
                        goto LOOP1;
                    }
                }
            }
        }
    }

    fseek(fp,0,SEEK_SET);
}

void cmd_clean_up_child_process (int signal_number)
{
    /* Clean up the child process. */
    int status;
    //wait (&status);
    waitpid(cmd_child_pid,&status,0);
    /* Store its exit status in a global variable. */
    child_status = status;
    child_done = 1;
}

int cmd_shell(struct cli_def *cli, char *command, char *argv[], int argc)
{
    int tube[2];
    int status;
    int oldfdout;
    int fdsocket;
    int i;
    
    char m_cmd[1024],*end,*start,act_cmd[1024];
     
    struct menu *p_menu;
    p_menu = pmenu_head;

    start = command;

    /* Handle SIGCHLD by calling cmd clean_up_child_process. */
    struct sigaction sigchld_action;
    memset (&sigchld_action, 0, sizeof (sigchld_action));
    sigchld_action.sa_handler = &cmd_clean_up_child_process;
    sigaction (SIGCHLD, &sigchld_action, NULL);

    while((end = strchr(start,' '))!=NULL)
    {
        memset(m_cmd,0,sizeof(char)*1024);
        strncpy (m_cmd, start,(end - start));
        m_cmd[end - start]='\0';

        while (strncmp(p_menu->name, m_cmd, strlen(m_cmd))!=0)
        {
            if(p_menu->next !=NULL)
            {
                p_menu=p_menu->next;
            }
        }
        p_menu=p_menu->sub;

        start = end+1;
        if( *start == '\0')
           break;
    }

    while(strncmp(p_menu->name,start,strlen(start))!=0)
    {
        p_menu = p_menu->next;
    }

    if(p_menu->cmd !=NULL)
    {
        strncpy(act_cmd, p_menu->cmd, strlen(p_menu->cmd));
    } else {
        perror("get cmd from struct");
        exit(1);
    }

    if(pipe(tube)<0)
        exit(1);

    fcntl(tube[0], F_SETFL, O_NONBLOCK | fcntl(tube[0], F_GETFL));

    fflush(stdout);
    
    if((oldfdout=dup(1))<0){
        perror("server stdout :dup");
        exit(1);
    }
    
    fdsocket=fileno(cli->client);
    
    int pid=fork();

    if(pid<0){
        perror("error: server fork");
        exit(1);
    }

    fflush(NULL);
    fflush(stdin);
    fflush(stdout);

    //int pid_ctrlc;
    if(pid==0){//fils
    //pid_ctrlc=getpid();
        fflush(NULL);
        fflush(stdin);
        fflush(stdout);
        close(tube[0]);
        char *c[argc+3];
        c[0]=(char *)malloc(sizeof(char)*4);
        c[0]="sh";

        char *cd=(char *)malloc(sizeof(char)*1024);
        bzero(cd,sizeof(cd));
        strcat(cd,FILE_PATH);
        strcat(cd,p_menu->cmd);

        c[1]=(char *)malloc(sizeof(char)*1024);
        strcpy(c[1] , cd);
        c[argc+2]=(char *)malloc(sizeof(char)*1024);
        c[argc+2]=NULL;

        for(i=2;i<argc+2;i++){
            c[i]=(char *)malloc(sizeof(char)*1024);
            strcpy(c[i],argv[i-2]);
        }

        if(dup2(tube[1],1)<0){
            perror("server: dup2");
            exit(1);
        }

        if(dup2(tube[1],2)<0){
            perror("server: dup2");
            exit(1);
        }

        seteuid((uid_t)0);

        if(execvp(c[0],c)<0)
            perror("\nerr on execvp");
       // exit(0);

    } else { //pere
        fflush(NULL);
        fflush(stdin);
        fflush(stdout);
        close(tube[1]);
        char buf[1000];
    
        char my_c;
        fd_set r;
        fd_set rds;
        int sr;
        struct timeval tm;
        struct timeval rtm;
        tm.tv_sec = 1;
        tm.tv_usec = 0;
        rtm.tv_sec =1;
        rtm.tv_usec =0;
        int n,nfds;
        //fd1=fdopen(tube[0],"r");
        FILE *in_ch;

        cmd_child_pid=pid;
        FD_ZERO(&rds);
        child_done=0;
        i=0;
        while(1){
            FD_SET(tube[0], &rds);
            nfds = select(tube[0] + 1, &rds, NULL, NULL, &rtm);
            n = 0;
            if (nfds > 0) {
                n=read(tube[0],buf,sizeof(buf));
                if (n < 0)
                    n = 0;
            }


            if(n>0)
                i=0;
            else {
                if((i>30)||(child_done==1))
                {
                  break;   
                }
            }

            i++;

            buf[n]=0;
            cli_print(cli,"%s",buf);

            fflush(NULL);

            FD_ZERO(&r);
            FD_SET(fdsocket, &r);
            if ((sr = select(fdsocket + 1, &r, NULL, NULL, &tm)) < 0)
            {
                /* select error */
                if(errno == EINTR)
                    continue;
                perror("read");
                //l = -1;
                break;
            }
            if(sr==0){//timeout
                tm.tv_sec = 1;
                tm.tv_usec = 0;
                continue;
            }
            my_c=fgetc(cli->client);
            //fprintf(stderr,"fgetc: =%d\n",c);
            if(my_c==3){
                kill(pid,SIGINT);
                break;
            }

            /*need accept some word from console not enable*/
            if (access("/tmp/cli_console_input", F_OK )==0) {
                in_ch=fopen("/tmp/cli_console_input","w");
                fputc(my_c,in_ch);
                //fprintf(in_ch,"%s\n",my_c);
                fclose(in_ch);
            }
       }
        close(tube[0]);
        waitpid(pid,&status,0);
      //wait(&status);
    }
    return CLI_OK;
}

int cmd_test(struct cli_def *cli, char *command, char *argv[], int argc)
{
    int i;
    cli_print(cli, "called %s with \"%s\"", __FUNCTION__, command);
    cli_print(cli, "%d arguments:", argc);
    for (i = 0; i < argc; i++)
        cli_print(cli, "        %s", argv[i]);

    return CLI_OK;
}

int cmd_set(struct cli_def *cli, UNUSED(char *command), char *argv[],
    int argc)
{
    if (argc < 2 || strcmp(argv[0], "?") == 0)
    {
        cli_print(cli, "Specify a variable to set");
        return CLI_OK;
    }

    if (strcmp(argv[1], "?") == 0)
    {
        cli_print(cli, "Specify a value");
        return CLI_OK;
    }

    if (strcmp(argv[0], "regular_interval") == 0)
    {
        unsigned int sec = 0;
        if (!argv[1] && !&argv[1])
        {
            cli_print(cli, "Specify a regular callback interval in seconds");
            return CLI_OK;
        }
        sscanf(argv[1], "%d", &sec);
        if (sec < 1)
        {
            cli_print(cli, "Specify a regular callback interval in seconds");
            return CLI_OK;
        }
        cli->timeout_tm.tv_sec = sec;
        cli->timeout_tm.tv_usec = 0;
        cli_print(cli, "Regular callback interval is now %d seconds", sec);
        return CLI_OK;
    }

    cli_print(cli, "Setting \"%s\" to \"%s\"", argv[0], argv[1]);
    return CLI_OK;
}

int cmd_config_int(struct cli_def *cli, UNUSED(char *command), char *argv[],
    int argc)
{
    if (argc < 1)
    {
        cli_print(cli, "Specify an interface to configure");
        return CLI_OK;
    }

    if (strcmp(argv[0], "?") == 0)
        cli_print(cli, "  test0/0");

    else if (strcasecmp(argv[0], "test0/0") == 0)
        cli_set_configmode(cli, MODE_CONFIG_INT, "test");
    else
        cli_print(cli, "Unknown interface %s", argv[0]);

    return CLI_OK;
}

int cmd_config_int_exit(struct cli_def *cli, UNUSED(char *command),
    UNUSED(char *argv[]), UNUSED(int argc))
{
    cli_set_configmode(cli, MODE_CONFIG, NULL);
    return CLI_OK;
}

int cmd_show_regular(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    cli_print(cli, "cli_regular() has run %u times", regular_count);
    return CLI_OK;
}

int cmd_info_system(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    cli_print(cli, "Firmware: 100");
    cli_run_command(cli,"ls -al");
    return CLI_OK;
}

int cmd_debug_regular(struct cli_def *cli, UNUSED(char *command), char *argv[], int argc)
{
    debug_regular = !debug_regular;
    cli_print(cli, "cli_regular() debugging is %s", debug_regular ? "enabled" : "disabled");
    return CLI_OK;
}

int check_auth(char *username, char *password)
{
    if (strcasecmp(username, u_name) != 0)
        return CLI_ERROR;
    if (strcasecmp(password, u_passwd) != 0)
        return CLI_ERROR;
    return CLI_OK;
}

int regular_callback(struct cli_def *cli)
{
    regular_count++;
    if (debug_regular)
    {
        cli_print(cli, "Regular callback - %u times so far", regular_count);
        cli_reprompt(cli);
    }
    return CLI_OK;
}

int check_enable(char *password)
{
    return !strcasecmp(password, "topsecret");
}

int idle_timeout(struct cli_def *cli)
{
    cli_print(cli, "Custom idle timeout");

    char *c[3];
    c[0]=(char *)malloc(sizeof(char)*10);
    c[0]="killall";

    char *cd=(char *)malloc(sizeof(char)*1024);
    bzero(cd,sizeof(cd));
    strcat(cd,"telnet");

    c[1]=(char *)malloc(sizeof(char)*1024);
    strcpy(c[1] , cd);
    c[2]=(char *)malloc(sizeof(char)*1024);
    c[2]=NULL;

    if(execvp(c[0],c)<0)
        perror("\nerr on execvp");

    return CLI_QUIT;
}

void pc(UNUSED(struct cli_def *cli), char *string)
{
    printf("%s\n", string);
}

void recursive_register(struct cli_def *cli, struct menu *p_menu,struct cli_command *parent, struct cli_command *self_r,int sub)
{
    struct menu *list_menu;
    struct cli_command *p,*s,*d,*m;

    list_menu = p_menu;
    p = parent;

    while(list_menu != NULL)
    {
        if(sub ==1)
        {
            sub = 0;
            
        }  else
        {
            sub =1 ;
        }

        
        char *cd=(char *)malloc(sizeof(char)*4);
        bzero(cd,sizeof(cd));
        strcat(cd,"\n");
        strcat(list_menu->desc ,cd);
        free(cd);

        if(list_menu->cmd != NULL)
            s = cli_register_command(cli, p, list_menu->name, cmd_shell, PRIVILEGE_UNPRIVILEGED,
                                      MODE_EXEC, list_menu->desc);
        else
            s = cli_register_command(cli, p, list_menu->name, NULL, PRIVILEGE_UNPRIVILEGED,
                                      MODE_EXEC, list_menu->desc);
        d=p;
        m=s;

        if(p !=NULL)
            printf("register %s, sub = %d  parent = %s\n",list_menu->name,sub,p->command);
        else
            printf("register %s, sub = %d  \n",list_menu->name,sub);

        if(sub ==0)
        {
            recursive_register(cli,list_menu->next,p,s,sub);
            list_menu=list_menu->sub;
            p = s;

        }  else
        {
            recursive_register(cli,list_menu->sub,s,s,sub);
            list_menu=list_menu->next;
            p=d;
            s=m;
        }
    }
}

void free_menu(struct menu *p_menu, int sub)
{
    struct menu *list_menu, *pre_menu;
    list_menu = pre_menu=p_menu;

    if((pmenu_head->sub == NULL)&&(pmenu_head->next==NULL))
    {
        free(pmenu_head->name);
        free(pmenu_head->desc);
        free(pmenu_head->cmd);
        free(pmenu_head);
        return;
    } 

    pre_menu = list_menu;

    if(sub==1)
    {
        list_menu = list_menu->sub;
        sub =0;
    } else
    {
        list_menu = list_menu->next;
        sub = 1;
    }

    while(list_menu!=NULL)
    {
        if((list_menu->sub == NULL)&&(list_menu->next==NULL))
        {
            free(list_menu->name);
            free(list_menu->desc);
            free(list_menu->cmd);
            free(list_menu);
            if(sub==0)
            {
                pre_menu->sub = NULL;
            }else
            {
                pre_menu->next = NULL;

            }

            list_menu = pmenu_head;
            sub=1;
        }
        free_menu(list_menu, sub);

        pre_menu = list_menu;
        if(sub==0)
        {
            list_menu = list_menu->sub;
        } else
        {
            list_menu = list_menu->next;
        }
    }
}


void register_at_commands(struct cli_def *cli)
{
    //system command
    cli_register_command(cli, NULL, "AT", cmd_echo_ok, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "AT Echo OK\n");
    cli_register_command(cli, NULL, "AT+TEST", cmd_at_test, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "AT Echo TEST\n");
    cli_register_command(cli, NULL, "ATH", cli_at_history, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Show a list of previously run AT commands\n");
    cli_register_command(cli, NULL, "ATL", cli_at_list, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "List all available AT commands\n");
    cli_register_command(cli, NULL, "AT&R", cmd_conf_read, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Read modem active profile to editable profile\n");
    cli_register_command(cli, NULL, "AT&V", cmd_display_sysconf, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Display modem active profile\n");
    cli_register_command(cli, NULL, "AT&W", cmd_conf_enable, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable configurations you have been entered\n");
    cli_register_command(cli, NULL, "AT+MREB", cmd_reboot, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Reboot the modem\n");
    cli_register_command(cli, NULL, "ATA", cli_at_quit, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Quit\n");
    cli_register_command(cli, NULL, "ATO", cli_at_quit, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Quit\n");

    //system information
    cli_register_command(cli, NULL, "AT+MSYSI", cmd_sys_info, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "System summary information\n");
    cli_register_command(cli, NULL, "AT+GMR", cmd_modem_record, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Modem Record Information\n");
    cli_register_command(cli, NULL, "AT+MMNAME", cmd_modem_name, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Modem Name Setting\n");
    cli_register_command(cli, NULL, "AT+GMI", cmd_manu_id, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Get Manufacturer Identification\n");
    cli_register_command(cli, NULL, "AT+CNUM", cmd_phone_number, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Check Modem's Phone Number\n");
    cli_register_command(cli, NULL, "AT+CIMI", cmd_modem_imi, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Check Modem's IMEI and IMSI\n");
    cli_register_command(cli, NULL, "AT+CCID", cmd_modem_sim, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Check Modem's SIM Card Number\n");

    cli_register_command(cli, NULL, "AT+MLEIP", cmd_local_eip, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set the IP address of the modem LAN Ethernet interface\n");
    cli_register_command(cli, NULL, "AT+MDHCP", cmd_dhcp_enable, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable DHCP server running on the Ethernet interface\n");
    cli_register_command(cli, NULL, "AT+MDHCPA", cmd_dhcp_address, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set the range of IP addresses to be assigned by the DHCP server\n");
    cli_register_command(cli, NULL, "AT+MEMAC", cmd_eth_mac, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Query the MAC address of local Ethernet interface\n");
    cli_register_command(cli, NULL, "AT+MSIP", cmd_lan_sip, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set LAN static IP\n");
    cli_register_command(cli, NULL, "AT+MSCT", cmd_lan_sct, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set LAN Connection Type \n");
    //add for wan
    cli_register_command(cli, NULL, "AT+MWEIP", cmd_local_weip, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set the IP address of the modem WAN Ethernet interface\n");
    cli_register_command(cli, NULL, "AT+MWDHCP", cmd_wdhcp_enable, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable DHCP server running on the WAN Ethernet interface\n");
    cli_register_command(cli, NULL, "AT+MWDHCPA", cmd_wdhcp_address, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set the range of WAN IP addresses to be assigned by the DHCP server\n");
    cli_register_command(cli, NULL, "AT+MWEMAC", cmd_weth_mac, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Query the MAC address of WAN Ethernet interface\n");
    cli_register_command(cli, NULL, "AT+MWSIP", cmd_wan_sip, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set WAN static IP\n");
    cli_register_command(cli, NULL, "AT+MWSCT", cmd_wan_sct, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set WAN Connection Type \n");

    //system or carrier
    cli_register_command(cli, NULL, "AT+MNTP", cmd_define_ntp, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Define NTP server\n");
    cli_register_command(cli, NULL, "AT+MPIPP", cmd_ip_pass, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable IP-Passthrough\n");
    cli_register_command(cli, NULL, "AT+MCNTO", cmd_console_timeout, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set console timeout\n");
    cli_register_command(cli, NULL, "AT+MRTF", cmd_reset_default, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Reset the modem to the factory default settings of  from non-volatile (NV) memory\n");
    cli_register_command(cli, NULL, "AT+MTWT", cmd_traffic_wtimer, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable traffic watchdog timer used to reset the modem\n");
    cli_register_command(cli, NULL, "AT+MSCMD", cmd_sms_command, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable system sms command service\n");
    cli_register_command(cli, NULL, "AT+MDISS", cmd_dis_service, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set discovery service used by the modem\n");
    cli_register_command(cli, NULL, "AT+MPWD", cmd_admin_pwd, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set password\n");
    cli_register_command(cli, NULL, "AT+MIKACE", cmd_icmp_enable, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable ICMP keep-alive check\n");
    cli_register_command(cli, NULL, "AT+MIKAC", cmd_icmp_ka, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set ICMP keep-alive check\n");
    cli_register_command(cli, NULL, "AT+MDDNSE", cmd_ddns_enable, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable DDNS\n");
    cli_register_command(cli, NULL, "AT+MDDNS", cmd_ddns_conf, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set DDNS\n");

    //report command
    cli_register_command(cli, NULL, "AT+MEURD1", cmd_eurd_1, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Define Event UDP Report No.1\n");
    cli_register_command(cli, NULL, "AT+MEURD2", cmd_eurd_2, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Define Event UDP Report No.2\n");
    cli_register_command(cli, NULL, "AT+MEURD3", cmd_eurd_3, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Define Event UDP Report No.3\n");
    cli_register_command(cli, NULL, "AT+MNMSR", cmd_nms_r, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Define NMS Report\n");
    cli_register_command(cli, NULL, "AT+MGPSR1", cmd_gpsr_1, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Define GPS Report No.1\n");
    cli_register_command(cli, NULL, "AT+MGPSR2", cmd_gpsr_2, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Define GPS Report No.2\n");
    cli_register_command(cli, NULL, "AT+MGPSR3", cmd_gpsr_3, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Define GPS Report No.3\n");
    cli_register_command(cli, NULL, "AT+MGPSR4", cmd_gpsr_4, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Define GPS Report No.4\n");

    //sms command;       MMG*: motorola support.
    cli_register_command(cli, NULL, "AT+CMGS", cmd_send_sms, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Send SMS\n");
    cli_register_command(cli, NULL, "AT+CMGR", cmd_read_sms, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Read SMS with changing status\n");
    cli_register_command(cli, NULL, "AT+MMGR", cmd_read_sms_unchange, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Read SMS without changing status\n");
    cli_register_command(cli, NULL, "AT+CMGL", cmd_list_sms, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "List SMSs with changing status\n");
    cli_register_command(cli, NULL, "AT+MMGL", cmd_list_sms_unchange, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "List SMSs without changing status\n");
    cli_register_command(cli, NULL, "AT+CMGD", cmd_delete_sms, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Delete SMSs\n");

    //com2
    cli_register_command(cli, NULL, "AT+MCTPS", cmd_com2_port_status, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable com2 port\n");
    cli_register_command(cli, NULL, "AT+MCTBR", cmd_com2_baud_rate, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port baud rate\n");
    cli_register_command(cli, NULL, "AT+MCTDF", cmd_com2_data_format, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port data format\n");
    cli_register_command(cli, NULL, "AT+MCTDM", cmd_com2_data_mode, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port data mode\n");
    cli_register_command(cli, NULL, "AT+MCTCT", cmd_com2_ct, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port character timeout\n");
    cli_register_command(cli, NULL, "AT+MCTMPS", cmd_com2_mps, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port maximum packet size\n");
    cli_register_command(cli, NULL, "AT+MCTP", cmd_com2_priority, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port priority\n");
    cli_register_command(cli, NULL, "AT+MCTNCDI", cmd_com2_ncdi, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable com2 port no-connection data intake\n");
    cli_register_command(cli, NULL, "AT+MCTMTC", cmd_com2_mtc, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port modbus tcp configuration\n");
    cli_register_command(cli, NULL, "AT+MCTIPM", cmd_com2_ipm, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port IP protocol mode\n");
    cli_register_command(cli, NULL, "AT+MCTTC", cmd_com2_tcpclient, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port tcp client configuration when IP protocol mode be set to TCP Client\n");
    cli_register_command(cli, NULL, "AT+MCTTS", cmd_com2_tcpserver, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port tcp server configuration when IP protocol mode be set to TCP Server\n");
    cli_register_command(cli, NULL, "AT+MCTTCS", cmd_com2_tcpcs, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port tcp client/server configuration when IP protocol mode be set to TCP Client/Server\n");
    cli_register_command(cli, NULL, "AT+MCTUPP", cmd_com2_upp, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port UDP point to point configuration when IP protocol mode be set to UDP point to point\n");
    cli_register_command(cli, NULL, "AT+MCTUPMP", cmd_com2_upmp, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port UDP point to multipoint as point configuration when IP protocol mode be set to UDP point to multipoint(P)\n");
    cli_register_command(cli, NULL, "AT+MCTUPMM", cmd_com2_upmm, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port UDP point to multipoint as MP configuration when IP protocol mode be set to UDP point to multipoint(MP)\n");
    cli_register_command(cli, NULL, "AT+MCTUMPMP", cmd_com2_umpmp, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set com2 port UDP multipoint to multipoint configuration when IP protocol mode be set to UDP multipoint to multipoint\n");
    //there are some additional settings left for com2 in 4G.

    //for IO status and control
    cli_register_command(cli, NULL, "AT+MIS", cmd_input_s, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Module Input status\n");
    cli_register_command(cli, NULL, "AT+MOS", cmd_output_s, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Module Output status and setting\n");

    //not valid in 4G.
/*
   cli_register_command(cli, NULL, "AT+MNAT", cmd_nat_conf, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable NAT\n");
   cli_register_command(cli, NULL, "AT+MPPPS", cmd_ppp_status, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable PPP\n");
   cli_register_command(cli, NULL, "AT+MDOD", cmd_dod_conf, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable Dial-on-Demand\n");
   cli_register_command(cli, NULL, "AT+MPITO", cmd_idle_timeout, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set idel Timeout\n");
   cli_register_command(cli, NULL, "AT+MPCTO", cmd_connect_timeout, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set Connect Timeout\n");
   cli_register_command(cli, NULL, "AT+MSDBE", cmd_default_button, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable system default button\n");
   cli_register_command(cli, NULL, "AT+MAUTH", cmd_auth_conf, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set authentication used by the modem\n");
    cli_register_command(cli, NULL, "AT+MPDMR", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set dialing max retries\n");
    cli_register_command(cli, NULL, "AT+MPAT", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set authentication type used by PPP\n");
    cli_register_command(cli, NULL, "AT+MPUP", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set authentication type used by PPP\n");
    cli_register_command(cli, NULL, "AT+MPDN", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set PPP dial number\n");
    cli_register_command(cli, NULL, "AT+MPBR", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set PPP baud rate\n");
    cli_register_command(cli, NULL, "AT+MPCS", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set PPP connect string\n");
    cli_register_command(cli, NULL, "AT+MAPN", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set access point name\n");
    cli_register_command(cli, NULL, "AT+MPINS1", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set initialization string #1\n");
    cli_register_command(cli, NULL, "AT+MPINS2", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set initialization string #2\n");
    cli_register_command(cli, NULL, "AT+MPINS3", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set initialization string #3\n");
    cli_register_command(cli, NULL, "AT+MPINS4", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set initialization string #4\n");
    cli_register_command(cli, NULL, "AT+MURD", cmd_not_defined, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable use remote DNS\n");
*/
/*
    cli_register_command(cli, NULL, "AT+MURD", cmd_ppp_urd, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Enable or disable use remote DNS\n");
    cli_register_command(cli, NULL, "AT+MPDMR", cmd_dial_max_retries, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set dialing max retries\n");
    cli_register_command(cli, NULL, "AT+MPAT", cmd_ppp_auth_type, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set authentication type used by PPP\n");
    cli_register_command(cli, NULL, "AT+MPUP", cmd_ppp_usr_pwd, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set authentication type used by PPP\n");
    cli_register_command(cli, NULL, "AT+MPDN", cmd_dial_number, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set PPP dial number\n");
    cli_register_command(cli, NULL, "AT+MPBR", cmd_ppp_baud, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set PPP baud rate\n");
    cli_register_command(cli, NULL, "AT+MPCS", cmd_ppp_connectstring, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set PPP connect string\n");
    cli_register_command(cli, NULL, "AT+MAPN", cmd_ppp_apn, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set access point name\n");
    cli_register_command(cli, NULL, "AT+MPINS1", cmd_ppp_ins1, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set initialization string #1\n");
    cli_register_command(cli, NULL, "AT+MPINS2", cmd_ppp_ins2, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set initialization string #2\n");
    cli_register_command(cli, NULL, "AT+MPINS3", cmd_ppp_ins3, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set initialization string #3\n");
    cli_register_command(cli, NULL, "AT+MPINS4", cmd_ppp_ins4, PRIVILEGE_UNPRIVILEGED,MODE_EXEC, "Set initialization string #4\n");
*/

    /*
     * usb port commands
     */
/*
    cli_register_command(cli, NULL, "AT+MUMAC", cmd_usb_mac, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Query the MAC address of local USB Ethernet interface");
    cli_register_command(cli, NULL, "AT+MUDPM", cmd_usb_mode, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set the USB device mode");
    cli_register_command(cli, NULL, "AT+MUNDIS", cmd_usb_ndis, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Configuration of USB device that be set to NDIS mode");
    cli_register_command(cli, NULL, "AT+MUDPS", cmd_usb_port_status, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Enable or disable usb data port");
    cli_register_command(cli, NULL, "AT+MUDBR", cmd_usb_baud_rate, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port baud rate");
    cli_register_command(cli, NULL, "AT+MUDDF", cmd_usb_data_format, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port data format");
    cli_register_command(cli, NULL, "AT+MUDDM", cmd_usb_data_mode, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port data mode");
    cli_register_command(cli, NULL, "AT+MUDCT", cmd_usb_ct, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port character timeout");
    cli_register_command(cli, NULL, "AT+MUDMPS", cmd_usb_mps, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port maximum packet size");
    cli_register_command(cli, NULL, "AT+MUDPL", cmd_usb_priority, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port priority");
    cli_register_command(cli, NULL, "AT+MUDNCDI", cmd_usb_ncdi, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Enable or disable usb data port no-connection data intake");
    cli_register_command(cli, NULL, "AT+MUDMTC", cmd_usb_mtc, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port modbus tcp configuration");
    cli_register_command(cli, NULL, "AT+MUDIPM", cmd_usb_ipm, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port IP protocol mode");
    cli_register_command(cli, NULL, "AT+MUDTC", cmd_usb_tcpclient, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port tcp client configuration when IP protocol mode be set to TCP Client");
    cli_register_command(cli, NULL, "AT+MUDTS", cmd_usb_tcpserver, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port tcp server configuration when IP protocol mode be set to TCP Server");
    cli_register_command(cli, NULL, "AT+MUDTCS", cmd_usb_tcpcs, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port tcp client/server configuration when IP protocol mode be set to TCP Client/Server");
    cli_register_command(cli, NULL, "AT+MUDUPP", cmd_usb_upp, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port UDP point to point configuration when IP protocol mode be set to UDP point to point");
    cli_register_command(cli, NULL, "AT+MUDUPMP", cmd_usb_upmp, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port UDP point to multipoint as point configuration when IP protocol mode be set to UDP point to multipoint(P)");
    cli_register_command(cli, NULL, "AT+MUDUPMM", cmd_usb_upmm, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port UDP point to multipoint as MP configuration when IP protocol mode be set to UDP point to multipoint(MP)");
    cli_register_command(cli, NULL, "AT+MUDUMPMP", cmd_usb_umpmp, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set usb data port UDP multipoint to multipoint configuration when IP protocol mode be set to UDP multipoint to multipoint");
*/ 
/*
*/
    /*
     * com1 commands
     */
/*
    cli_register_command(cli, NULL, "AT+MCOPS", cmd_com1_port_status, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Enable or disable com1 port");
    cli_register_command(cli, NULL, "AT+MCOCM", cmd_com1_channel_mode, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port channel mode");
    cli_register_command(cli, NULL, "AT+MCOBR", cmd_com1_baud_rate, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port baud rate");
    cli_register_command(cli, NULL, "AT+MCODF", cmd_com1_data_format, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port data format");
    cli_register_command(cli, NULL, "AT+MCOFC", cmd_com1_flow_control, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port flow control");
    cli_register_command(cli, NULL, "AT+MCOPRDD", cmd_com1_prdd, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port pre-data delay(ms)");
    cli_register_command(cli, NULL, "AT+MCOPODD", cmd_com1_podd, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port post-data delay(ms)");
    cli_register_command(cli, NULL, "AT+MCODM", cmd_com1_data_mode, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port data mode");
    cli_register_command(cli, NULL, "AT+MCOCT", cmd_com1_ct, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port character timeout");
    cli_register_command(cli, NULL, "AT+MCOMPS", cmd_com1_mps, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port maximum packet size");
    cli_register_command(cli, NULL, "AT+MCOP", cmd_com1_priority, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port priority");
    cli_register_command(cli, NULL, "AT+MCONCDI", cmd_com1_ncdi, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Enable or disable no-connection data intake");
    cli_register_command(cli, NULL, "AT+MCOMTC", cmd_com1_mtc, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port modbus tcp configuration");
    cli_register_command(cli, NULL, "AT+MCOIPM", cmd_com1_ipm, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port IP protocol mode");
    cli_register_command(cli, NULL, "AT+MCOTC", cmd_com1_tcpclient, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port tcp client configuration when IP protocol mode be set to TCP Client");
    cli_register_command(cli, NULL, "AT+MCOTS", cmd_com1_tcpserver, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port tcp server configuration when IP protocol mode be set to TCP Server");
    cli_register_command(cli, NULL, "AT+MCOTCS", cmd_com1_tcpcs, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port tcp client/server configuration when IP protocol mode be set to TCP Client/Server");
    cli_register_command(cli, NULL, "AT+MCOUPP", cmd_com1_upp, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port UDP point to point configuration when IP protocol mode be set to UDP point to point");
    cli_register_command(cli, NULL, "AT+MCOUPMP", cmd_com1_upmp, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port UDP point to multipoint as point configuration when IP protocol mode be set to UDP point to multipoint(P)");
    cli_register_command(cli, NULL, "AT+MCOUPMM", cmd_com1_upmm, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port UDP point to multipoint as MP configuration when IP protocol mode be set to UDP point to multipoint(MP)");
    cli_register_command(cli, NULL, "AT+MCOUMPMP", cmd_com1_umpmp, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port UDP multipoint to multipoint configuration when IP protocol mode be set to UDP multipoint to multipoint");
    cli_register_command(cli, NULL, "AT+MCOSMTP", cmd_com1_smtp, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port SMTP client configuration when IP protocol mode be set to SMTP Client");
    cli_register_command(cli, NULL, "AT+MCOPPP", cmd_com1_ppp, PRIVILEGE_UNPRIVILEGED,
                         MODE_EXEC, "Set com1 port PPP configuration when IP protocol mode be set to PPP");
*/ 


}


int main()
{
    struct cli_command *c , *d, *e,*parent;
    struct cli_def *cli;
    int s, x;
    struct sockaddr_in addr;
    int on = 1;

    char prompt[1024];
    char banner[1024];
    char auth[3];
    //struct cmd list_cmd[MAX_MENU_CNT];
    struct menu  *list_menu,*list_sub,*list_next,*list_tmp, *menu_head;
    FILE *fp;
    int i;

    char *parse_file_name = (char *)malloc(sizeof(char)*1024);
    bzero(parse_file_name , sizeof(parse_file_name));
    strcat(parse_file_name ,FILE_PATH);
    strcat(parse_file_name ,CONF_FILE);

#ifndef WIN32
    signal(SIGCHLD, SIG_IGN);
#endif
#ifdef WIN32
    if (!winsock_init()) {
        printf("Error initialising winsock\n");
        return 1;
    }
#endif

    if((fp=fopen(parse_file_name,"r"))==NULL){
        perror("find config file");
        return 1;
    }
    
    getvalue("banner",banner,fp);
    getvalue("prompt",prompt,fp);
    getvalue("enable_auth",auth,fp);

    cli = cli_init();

    cli_set_banner(cli, banner);
    cli_set_hostname(cli, prompt);
    cli_regular(cli, regular_callback);
    cli_regular_interval(cli, 5); // Defaults to 1 second
    cli_set_idle_timeout_callback(cli, 120, idle_timeout); // 60 second idle timeout
    at_parameter_timeout=120;
    at_flag=0;

    /*cli_register_command(cli, NULL, "test", cmd_test, PRIVILEGE_UNPRIVILEGED,
        MODE_EXEC, NULL);

    cli_register_command(cli, NULL, "simple", NULL, PRIVILEGE_UNPRIVILEGED,
        MODE_EXEC, NULL);

    cli_register_command(cli, NULL, "simon", NULL, PRIVILEGE_UNPRIVILEGED,
        MODE_EXEC, NULL);

    cli_register_command(cli, NULL, "set", cmd_set, PRIVILEGE_PRIVILEGED,
        MODE_EXEC, NULL);

    c = cli_register_command(cli, NULL, "show", NULL, PRIVILEGE_UNPRIVILEGED,
        MODE_EXEC, NULL);

    cli_register_command(cli, c, "regular", cmd_show_regular, PRIVILEGE_UNPRIVILEGED,
        MODE_EXEC, "Show the how many times cli_regular has run");

    cli_register_command(cli, c, "counters", cmd_test, PRIVILEGE_UNPRIVILEGED,
        MODE_EXEC, "Show the counters that the system uses");

    cli_register_command(cli, c, "junk", cmd_test, PRIVILEGE_UNPRIVILEGED,
        MODE_EXEC, NULL);

    cli_register_command(cli, NULL, "interface", cmd_config_int,
        PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Configure an interface");

    cli_register_command(cli, NULL, "exit", cmd_config_int_exit,
        PRIVILEGE_PRIVILEGED, MODE_CONFIG_INT,
        "Exit from interface configuration");

    cli_register_command(cli, NULL, "address", cmd_test, PRIVILEGE_PRIVILEGED,
        MODE_CONFIG_INT, "Set IP address");

    c = cli_register_command(cli, NULL, "debug", NULL, PRIVILEGE_UNPRIVILEGED,
        MODE_EXEC, NULL);

    cli_register_command(cli, c, "regular", cmd_debug_regular, PRIVILEGE_UNPRIVILEGED,
        MODE_EXEC, "Enable cli_regular() callback debugging");

    //added by Microhard Command line interface

    c = cli_register_command(cli, NULL, "info", NULL, PRIVILEGE_UNPRIVILEGED,
        MODE_EXEC, NULL);

    cli_register_command(cli, c, "system", cmd_info_system, PRIVILEGE_UNPRIVILEGED,
        MODE_EXEC, "Show system informations");
    */

    menu_head = malloc(sizeof(struct menu)); 
    menu_head->cnt=0;
    menu_head->desc = NULL;
    menu_head->name = NULL;
    menu_head->cmd = NULL;
    menu_head->sub = NULL;
    menu_head->next = NULL;

    pmenu_head = menu_head;
    getcmd(menu_head , fp);

    list_menu= pmenu_head;
    d=NULL;
    c=NULL;

    recursive_register(cli,list_menu,c,d,0);

    register_at_commands(cli);

    if(strcmp(auth,"yes")==0){
        getvalue("user_name",u_name,fp);
        getvalue("passwd",u_passwd,fp);
       // getvalue("enable_pw",enable_pw,fp);
        cli_set_auth_callback(cli, check_auth);
        cli_set_enable_callback(cli, check_enable);
    }

    {
        FILE *fh;

        if ((fh = fopen("clitest.txt", "r")))
        {
            // This sets a callback which just displays the cli_print() text to stdout
            cli_print_callback(cli, pc);
            cli_file(cli, fh, PRIVILEGE_UNPRIVILEGED, MODE_EXEC);
            cli_print_callback(cli, NULL);
            fclose(fh);
        }
    }

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return 1;
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(CLITEST_PORT);
    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    if (listen(s, 50) < 0)
    {
        perror("listen");
        return 1;
    }

    printf("Listening on port %d\n", CLITEST_PORT);
    while ((x = accept(s, NULL, 0)))
    {
#ifndef WIN32
        int pid = fork();
        if (pid < 0)
        {
            perror("fork");
            return 1;
        }

        /* parent */
        if (pid > 0)
        {
            socklen_t len = sizeof(addr);
            if (getpeername(x, (struct sockaddr *) &addr, &len) >= 0)
                printf(" * accepted connection from %s\n", inet_ntoa(addr.sin_addr));

            close(x);
            continue;
        }

        /* child */
        close(s);
        cli_loop(cli, x);
        exit(0);
#else
        cli_loop(cli, x);
        shutdown(x, SD_BOTH);
        close(x);
#endif
    }

    list_menu = pmenu_head;
    free(parse_file_name);
    free_menu(list_menu,1);
    cli_done(cli);
    return 0;
}
