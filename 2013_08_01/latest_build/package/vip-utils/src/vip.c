/* ************************************************************************
* vip.c
*
* Description : User application interface to VIP BSP Driver
*
* Written by : LL
*
* May 2, 2005 - WDT Handler
*
* (C)2007 MicroHard Corp.
*
* ********************************************************************** */
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "vip-bsp.h"

#define DEVICE_NAME "/dev/vip-bsp"

int device_fd   = -1;



/* **********************************************
* Function : display_header
*
* Description : Display application info. 
*
* Params : None
*
* Returns: Always True
********************************************** */
static int display_header(void)
{
    int retval = 0;
    
    printf("\n");
    printf("vip [OPTIONS] [-?]=help\n");
    printf("(C)2007 Microhard Systems Inc.\n\n");
    
    retval = 1;
    
    return(retval);
}

/* **********************************************
* Function : display_args 
*
* Description : Display application command line
*       available arguments 
*
* Params : None
*
* Returns: Always True
********************************************** */
static int display_args(void)
{
    int retval = 0;
    
    printf("Options[ -wdt <on,off,test [timeout = 30]> ]\n");
    printf("       [ -gpio <set> <gpio> <value> ]\n");
    printf("       [ -gpio <get> <gpio> ]\n");
    printf("       [ -cpu  <set> <4-533MHz, 1-400MHz,3-266MHz> ]\n");
    printf("       [ -cpu  <get> ]\n");
    printf("       [ -rev   <get> ]\n");
    
    retval = 1;
    
    return(retval);
}

         
/* **********************************************
* Function : main
*
* Description : Start of application 
*
* Params : argc - Number of Agruments in command line
*          argv - String poniter to command line 
                  argument
*
* Returns:  0 = Okay
********************************************** */
int main(int argc, char *argv[])
{
    int retval = -1;
    int cmd = 0;
    unsigned int wdt_timeout;
    struct gpio_ioctl info;
    int cpu_speed = 4, revision = 0;
    
    
        
    /* No less then the 2 required params */
    if (argc < 3 )
    {
        /* Display Application Information */
        display_header();
        display_args();
        return(retval);
    }
    
    /* GPIO control */
    if (strcmp(argv[1],"-gpio") == 0)
    {
        /* Set gpio */
        if (strcmp(argv[2],"set") == 0)
        {
            cmd = IOCTL_GPIO_SET;
            
            if(argc < 5)
            {
                display_header();
                display_args();
                return(retval);
            }

            info.gpio = atoi(argv[3]);
            info.value = atoi(argv[4]);
        }
        /* Get gpio */
        else if (strcmp(argv[2],"get") == 0)
        {
            cmd = IOCTL_GPIO_GET;
            if(argc < 4)
            {
                display_header();
                display_args();
                return(retval);
            }

            info.gpio = atoi(argv[3]);
            info.value = 0;
        }
    }

    /* CPU control */
    if (strcmp(argv[1],"-cpu") == 0)
    {
        /* Set cpu speed */
        if (strcmp(argv[2],"set") == 0)
        {
            cmd = IOCTL_SET_CPU_SPEED;
            
            if(argc < 4)
            {
                display_header();
                display_args();
                return(retval);
            }

            cpu_speed = atoi(argv[3]);
        }
        /* Get cpu speed */
        else if (strcmp(argv[2],"get") == 0)
        {
            cmd = IOCTL_GET_CPU_SPEED;
            if(argc < 3)
            {
                display_header();
                display_args();
                return(retval);
            }
        }
    }

    /* REV control */
    if (strcmp(argv[1],"-rev") == 0)
    {
        if (strcmp(argv[2],"get") == 0)
        {
            cmd = IOCTL_GET_HW_REV;
            if(argc < 3)
            {
                display_header();
                display_args();
                return(retval);
            }
        }
    }

    if(cmd == 0)
    {
        printf("No command was found!\n");

        return(retval);
    }
    /* Open the BSP Device */
    if ((device_fd = open(DEVICE_NAME,O_RDONLY)) >= 0 )
    {
        /* Send the command to the driver */

        
        if ( cmd == IOCTL_GPIO_GET || cmd == IOCTL_GPIO_SET){
            retval = ioctl(device_fd,cmd,&info);
            if ( cmd == IOCTL_GPIO_GET){
                printf("GPIO%d is %s.\n", info.gpio, (info.value == 1) ? "HIGH" : "LOW");
                retval = info.value; 
            }
        } else if ( cmd == IOCTL_GET_CPU_SPEED || cmd == IOCTL_SET_CPU_SPEED){
            retval = ioctl(device_fd,cmd,&cpu_speed);
            if ( cmd == IOCTL_GET_CPU_SPEED){

                printf("CPU is running at ");
                if (cpu_speed == 4) {
                    printf("533MHz.\n");
                } else if (cpu_speed == 1) {
                    printf("400MHz.\n");
                } else if (cpu_speed == 3) {
                    printf("266MHz.\n");
                }
            }
        } else if ( cmd == IOCTL_GET_HW_REV){
            retval = ioctl(device_fd,cmd,&revision);
            printf("Hardware revision is %d\n", revision);
        } else {
            retval = ioctl(device_fd,cmd,NULL);
        }

        /* Close device */        
        close(device_fd);
        
        /* Life is good */
//        retval = 0;
    }
    else /* Failed to open BSP Device */
    {
        printf("Failed to open %s\n",DEVICE_NAME);
    }

    return(retval);
}
