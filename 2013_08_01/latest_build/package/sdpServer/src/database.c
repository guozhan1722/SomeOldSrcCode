
#include "database.h"
#include <sys/wait.h>


struct uci_context *ctx = NULL;

bool uci_section_read( char*conf_fname, char* sect_name, char* key_word, char* read_buff)
{

    get_option_by_section_name(ctx, conf_fname, sect_name, key_word, read_buff);
//  get_option_by_section_name(ctx, "comport", "com2", key_word, read_buff);

    return true;
}

bool uci_section_write( char*conf_fname, char* sect_name, char* key_word, char* write_value)
{
    set_option_by_section_name(ctx, conf_fname, sect_name, key_word, write_value );
    return true;
}


bool get_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_element *e= NULL;
    struct uci_ptr ptr;
    struct uci_option *p_option = NULL;
    
    //printf("%s option_name: %s\n",__func__,option_name);
    strcpy(value, "");

    tuple = malloc( strlen(package_name) + strlen(section_name) + strlen(option_name)+ 3 ); /* "." and trailing \0 */

    if (!tuple) return false;

    strcpy(tuple, package_name);
    strcat(tuple, ".");
    strcat(tuple, section_name);
    strcat(tuple, ".");
    strcat(tuple, option_name);

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK)
        {

        e = ptr.last;

        if ( (ptr.flags & UCI_LOOKUP_COMPLETE) &&
             (e->type == UCI_TYPE_OPTION) )
            {
            p_option = ptr.o;
            strcpy(value, p_option->v.string);
            //printf("%s find %s\n",__func__, value);
            free(tuple);
            return true;
            }
        }

    free(tuple);

    return false;
}

bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_ptr ptr;

    tuple = malloc( strlen(package_name) + strlen(section_name) + strlen(option_name) + strlen(value) + 4 );    /* "." "=" and trailing \0 */

    if (!tuple) return false;

    strcpy(tuple, package_name);
    strcat(tuple, ".");
    strcat(tuple, section_name);
    strcat(tuple, ".");
    strcat(tuple, option_name);
    strcat(tuple, "=");
    strcat(tuple, value);

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK)
        {
        if ( UCI_OK == uci_set(ctx, &ptr) )
            {
            uci_save(ctx, ptr.p);
            uci_commit(ctx, &ptr.p, false);
            }

        free(tuple);
        return true;
        }

    free(tuple);

    return false;
}



/**********************************************************************************
   Function:  int SubProgram_Start(char *pszPathName, char *pszArguments)   
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

int SubProgram_Start(char *pszPathName, char *pszArguments)
{
    int  sub_status; 
    int MAX_ARGS=8;
    char * pArgs[MAX_ARGS];
    unsigned long ulNumArgs = 1;
    char tmpArgs[256];
    char *pArg = NULL;
    int pid = 0;    

    pArgs[0]=pszPathName;
    if (pszArguments != NULL)
        {
        strcpy(tmpArgs, pszArguments);    
        /* 'execl' return -1 on error. */
        pArg = strtok(tmpArgs, " ");
        while ((pArg != NULL) && (ulNumArgs < MAX_ARGS))
            {
            pArgs[ulNumArgs] = pArg;
            ulNumArgs++;          
            pArg = strtok(NULL, " ");
            }

        /*
        pArgs[0]=pszPathName;  
        pArgs[1]=pszArguments;  
        pArgs[2] = NULL; */   
        }
    pArgs[ulNumArgs] = NULL; 
    if (!(pid = vfork()))
        {
        seteuid((uid_t)0);
        /* We are the child! */            
        if (execvp(pszPathName, pArgs) == -1)
            {
            //syslog(LOGOPTS,"SubProgram_Start false\n"); 
            return false;
            }
        //syslog(LOGOPTS,"Child PID after = %d\n",pid);          
        }
    else
        {
        waitpid(pid,&sub_status,0);
        return true;        
        }    
    return true; 
}

/**********************************************************************************
   Function:
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/
int fetch_Local_IP_ADDR(char *ifname, char *addr)
{
    struct ifreq ifr;
    int sock;
    char *p;
    char temp[32]="0.0.0.0";

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if (-1==sock)
        {
        //syslog(LOGOPTS,"fetch_Local_IP_ADDR: socket=-1");  
        perror("fetch_Local_IP_ADDR ");
        strcpy(addr, temp); 
        return false;
        }
    strncpy(ifr.ifr_name,ifname,sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';   
    if (-1==ioctl(sock, SIOCGIFADDR, &ifr))
        {
        //perror("ioctl(SIOCGIFADDR) "); /*dont show it, sometimes no ip*/
        //syslog(LOGOPTS,"fetch_Local_IP_ADDR:%s ioctl(SIOCGIFADDR)",ifname);       
        strcpy(addr, temp); 
        close(sock);
        return false;
        }
    p=inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr);
    strncpy(temp,p,sizeof(temp)-1);
    temp[sizeof(temp)-1]='\0';
    strcpy(addr, temp);  
    close(sock);  
    return true;
}

bool fetch_Local_IP_MAC(char *ifname, char *mac)
{
    struct ifreq ifr;
    int sock, j, k;
    char temp[32]="00:00:00:00:00:00";

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if (-1==sock)
        {
//        syslog(LOGOPTS,"fetch_Local_IP_MAC:%s socket() ",ifname);      
        perror("socket() ");
        strcpy(mac, temp);        
        return false;
        }
    strncpy(ifr.ifr_name,ifname,sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';

    if (-1==ioctl(sock, SIOCGIFHWADDR, &ifr))
        {
 //       syslog(LOGOPTS,"fetch_Local_IP_MAC:%s ioctl(SIOCGIFHWADDR)",ifname);         
        //perror("ioctl(SIOCGIFHWADDR) ");
        strcpy(mac, temp);    
        close(sock);
        return false;
        }

    for (j=0, k=0; j<6; j++)
        {
        k+=snprintf(temp+k, sizeof(temp)-k-1, j ? ":%02X" : "%02X",
                    (int)(unsigned int)(unsigned char)ifr.ifr_hwaddr.sa_data[j]);
        }
    temp[sizeof(temp)-1]='\0';
    strcpy(mac, temp);          
    close(sock);  
    return true;
}

/**********************************************************************************
                               END THANK YOU
**********************************************************************************/





