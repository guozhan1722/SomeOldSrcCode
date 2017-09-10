/*
 * t_watchdog.c
 *
 *  Created on: May 10, 2012
 *      Author: zguo
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include "traffic.hpp"

static int rboot_count=600;
static int rset_count=30;
static int boot_count=60;
static int boot_watchdog=0;
const char *version = "0.0.1";
static int begin_time;
//static int current_time;
const char	*iface="eth2";
const char	*hostname="8.8.8.8";

void usage(FILE *fp, int rc)
{
    fprintf(fp, "Usage: traffic-watchdog [-t N[s]] [-T N[s]] [-b N[s]]\n\n"
    		"Periodically refresh watchdog \n"
    		"Options:\n"
            "\t-h?\t\tThis help\n"
            "\t-v\t\tShow version information\n"
            "\t-b N\t\tStart after N seconads at system booting up (default 60s)\n"
    		"\t-i IFACE\tinterface to be watched (default eth2)\n"
    		"\t-p HOST\t\tSend ICMP ECHO_REQUEST packets to network hosts(default 8.8.8.8)\n"
    		"\t-T N\t\tReboot after N seconds if not reset (default 600s)\n"
    		"\t-t N\t\tReset every N seconds (default 30s)\n");
    return;
}

/**********************************************************************************
   Function:  int SubProgram_Start(char *pszPathName, char *pszArguments)   
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/


int main (int argc, char **argv)
{
	int	c;
	int end_s;

    while ((c = getopt(argc, argv, "?hvi:b:T:t:p:")) > 0) {
        switch (c) {
        case 'T':
        	rboot_count = atoi(optarg);
            break;
        case 't':
        	rset_count = atoi(optarg);
            break;
        case 'b':
        	boot_watchdog++;
            boot_count = atoi(optarg);
        	break;
        case 'i':
            iface = optarg;
            break;
        case 'p':
            hostname = optarg;
            break;
        case 'v':
            printf("%s: version %s\n", argv[0], version);
            return 0;
        case 'h':
        case '?':
            usage(stdout, 0);
            break;
        default:
            fprintf(stderr, "ERROR: unkown option '%c'\n", c);
            usage(stderr, 1);
            break;
        }
    }

    if (argc<2) {
        fprintf(stderr, "Need Some argument\n");
        usage(stderr, 1);
        return -1;
    }

    if (boot_watchdog > 0)
    {
    	if (boot_count < 60)
    	{
    		W_DBG("running when booting up can not less then 60s seconds, reset your %d seconds to 60 seconds\n",boot_count);
    		boot_count = 60;
    	}
    	sleep(boot_count);
    }

    begin_time=get_uptime();

    do {
            end_s=keep_watch(begin_time, rset_count, rboot_count,iface,hostname);
    } while(end_s==0) ;

	return end_s;
}
