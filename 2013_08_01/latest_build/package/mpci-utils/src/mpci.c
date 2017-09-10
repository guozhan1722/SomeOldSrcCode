/* ************************************************************************
* mpci.c
*
* Description : User application interface to MPCI BSP Driver
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
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <errno.h>

#include "mpci-bsp.h"

#define DEVICE_NAME "/dev/vip-bsp"

#define I2C_DEVICE_NAME "/dev/i2c-0"

int i2c_address=0x18;
int device_fd   = -1;
int i2c_device_fd   = -1;



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
    printf("mpci [OPTIONS] [-?]=help\n");
    printf("(C)2010 Microhard Systems Inc.\n\n");
    
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
    
//    printf("Options[ -wdt <on,off,test [timeout = 30]> ]\n");
    printf("       [ -gpio <set> <gpio> <value> ]\n");
    printf("            ---- value > 15 using i2c-gpio\n");
    printf("       [ -gpio <get> <gpio> ]\n");
    printf("       [ -reg <get> <reg> ]\n");
    printf("       [ -reg <set> <reg> <value> ]\n");
    printf("            reg address and value format are hex\n");
    printf("            ex: mpci -gpio get 0xb8040008 \n");
    printf("            ex: mpci -gpio set 0xb8040008 0xded (turn on CPU status led)\n");
    printf("            ex: mpci -gpio set 0xb8040008 0x6ed (turn off CPU status led)\n");
//    printf("       [ -cpu  <set> <4-533MHz, 1-400MHz,3-266MHz> ]\n");
//    printf("       [ -cpu  <get> ]\n");
    printf("       [ -rev   <get> ]\n");
    
    retval = 1;
    
    return(retval);
}


static int i2c_bus_access(int file, char read_write, int command, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = I2C_SMBUS_BYTE_DATA;
	args.data = data;
	return ioctl(file,I2C_SMBUS,&args);
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
    struct reg_info rinfo;
    int cpu_speed = 4, revision = 0;
    struct i2c_smbus_ioctl_data i2c_info;
    int i2c_oldvalue,i2c_value;
    union i2c_smbus_data i2c_data;

        
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

    if (strcmp(argv[1],"-reg") == 0)
    {
        /* Set gpio */
        if (strcmp(argv[2],"set") == 0)
        {
            cmd = IOCTL_REG_SET;

            if(argc < 5)
            {
                display_header();
                display_args();
                return(retval);
            }

            sscanf(argv[3], "%x", &rinfo.reg) ;
            sscanf(argv[4], "%x", &rinfo.val) ;

            //rinfo.reg = atoi(argv[3]);
            //rinfo.val = atoi(argv[4]);
        }
        /* Get gpio */
        else if (strcmp(argv[2],"get") == 0)
        {
            cmd = IOCTL_REG_GET;
            if(argc < 4)
            {
                display_header();
                display_args();
                return(retval);
            }
            sscanf(argv[3], "%x", &rinfo.reg) ;
            //rinfo.reg = atoi(argv[3]);
            rinfo.val = 0;
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

    if ((strcmp(argv[1],"-gpio")== 0) &&  ( info.gpio > 15))
    {
        i2c_device_fd = open(I2C_DEVICE_NAME, O_RDWR);

        if (i2c_device_fd >=0)
        {
            retval = ioctl(i2c_device_fd, I2C_SLAVE_FORCE , i2c_address); 

            if(retval<0) {
                printf("set slave address failed !\n") ;
                close(i2c_device_fd);
                return retval; 
            }


            /*init i2c-gpio to output*/
            i2c_data.byte = 0;
            retval = i2c_bus_access(i2c_device_fd, I2C_SMBUS_WRITE, I2C_SMBUS_CONFIG_REG, &i2c_data);

            if(retval<0) {
                printf("write config reg failed !\n") ;
                close(i2c_device_fd);
                return retval; 
            }

             /*init i2c-gpio no POLARITY invert*/
            i2c_data.byte = 0;

            retval = i2c_bus_access(i2c_device_fd, I2C_SMBUS_WRITE, I2C_SMBUS_POLARITY_REG, &i2c_data);
            if(retval<0) {
                printf("write polarity reg failed !\n") ;
                close(i2c_device_fd);
                return retval; 
            }

            /*read i2c init value*/
            i2c_data.byte = 0;

            retval = i2c_bus_access(i2c_device_fd, I2C_SMBUS_READ, I2C_SMBUS_INPUT_REG, &i2c_data);

            if(retval<0) {
                printf("cannot read i2c input reg!\n");
                close(i2c_device_fd);
                return(retval);
            }

            i2c_oldvalue= i2c_data.byte;

            /*init i2c-gpio output values*/
            i2c_data.byte = i2c_data.byte | 1 << I2C_CPU_RSMODE;

            retval = i2c_bus_access(i2c_device_fd, I2C_SMBUS_WRITE, I2C_SMBUS_OUTPUT_REG, &i2c_data);

            if(retval<0) {
                printf("write polarity reg failed !\n") ;
                close(i2c_device_fd);
                return retval; 
            }

            /* Send the command to the driver */
            if ( cmd == IOCTL_GPIO_GET)
            {
                i2c_data.byte = 0;

                retval = i2c_bus_access(i2c_device_fd, I2C_SMBUS_READ, I2C_SMBUS_INPUT_REG, &i2c_data);

                if(retval<0) {
                    printf("cannot read i2c input reg!\n");
                    close(i2c_device_fd);
                    return(retval);
                }

                i2c_value= i2c_data.byte;
                printf("GPIO%d is %s.\n", info.gpio, (((i2c_value << 16 )&(1<<info.gpio)) == 1<<info.gpio) ? "HIGH" : "LOW");

            } else if (cmd == IOCTL_GPIO_SET) {

                i2c_data.byte = 0;
                retval = i2c_bus_access(i2c_device_fd, I2C_SMBUS_READ, I2C_SMBUS_INPUT_REG, &i2c_data);

                if(retval<0)
                {
                    printf("cannot read i2c input reg!\n");
                    close(i2c_device_fd);
                    return(retval);
                }

                i2c_oldvalue= i2c_data.byte;

                if (info.value ==1)
                    i2c_data.byte = ((i2c_oldvalue << 16) | (1<<info.gpio))>>16;
                else
                    i2c_data.byte = ((i2c_oldvalue << 16) & ~(1<<info.gpio))>>16;

                retval = i2c_bus_access(i2c_device_fd, I2C_SMBUS_WRITE, I2C_SMBUS_OUTPUT_REG, &i2c_data);

                if(retval<0)
                {
                    printf("cannot write i2c values!\n");
                    close(i2c_device_fd);
                    return(retval);
                }
            }else {
                printf("I2c is not getting and setting\n");
            }
            /* Close device */        
            close(i2c_device_fd);
        } else /* Failed to open i2c Device */   {
            printf("Failed to open %s\n",I2C_DEVICE_NAME);
        }
    
    } else {
    
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
            }else if ( cmd == IOCTL_REG_SET || cmd == IOCTL_REG_GET){
                    retval = ioctl(device_fd,cmd,&rinfo);

                    if ( cmd == IOCTL_REG_GET){
                        printf("0x%x\n",rinfo.val);
                        retval = rinfo.val; 
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
    }

    return(retval);
}
