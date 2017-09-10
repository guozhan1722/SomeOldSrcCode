/* **************************************************
* Program : vip-getserialnum
*
* Description : Retrieve the stored serial number
* from flash. 
*
* Author : LL
*
* (C)2005 MicroHard Systems Corp.
*
* This File is licensed under the GPL V2
* Some code has been used from RedBoot V1.94
*
************************************************** */
#include <syslog.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "vip-bsp.h"

/* ******************* DEFINES ****************** */
#define DEVICE_NAME "/dev/vip-bsp"

/* Application Defines */

#define FALSE                       0
#define TRUE                        1

/* Exit Values for Main */
#define EXIT_VAL_OK                 0
#define EXIT_VAL_ERR                -1
#define EXIT_VAL_ERR_SERIALNUM      -2
#define EXIT_VAL_ERR_OPEN           -3

/* RedBoot defines for Script and String lengths used in config info */
/* This info may change depending on what is set in CDL, for IXP425 these
    are the set values */

#define MAX_SCRIPT_LENGTH           512
#define MAX_STRING_LENGTH           128

/* Defines to retrieve config data */
#define CONFIG_OBJECT_TYPE(dp)          (dp)[0]
#define CONFIG_OBJECT_KEYLEN(dp)        (dp)[1]
#define CONFIG_OBJECT_ENABLE_SENSE(dp)  (dp)[2]
#define CONFIG_OBJECT_ENABLE_KEYLEN(dp) (dp)[3]
#define CONFIG_OBJECT_KEY(dp)           ((dp)+4)
#define CONFIG_OBJECT_ENABLE_KEY(dp)    ((dp)+4+CONFIG_OBJECT_KEYLEN(dp))
#define CONFIG_OBJECT_VALUE(dp)         ((dp)+4+CONFIG_OBJECT_KEYLEN(dp)+CONFIG_OBJECT_ENABLE_KEYLEN(dp))


/* Define to do Byte Swapping for Little Endian word sizes */
# define CYG_SWAP32(_x_)                        \
    ({ unsigned int _x = (_x_);                   \
       ((_x << 24) |                            \
       ((0x0000FF00UL & _x) <<  8) |            \
       ((0x00FF0000UL & _x) >>  8) |            \
       (_x  >> 24)); })



/* Config Data Types */
#define CONFIG_EMPTY   0
#define CONFIG_BOOL    1
#define CONFIG_INT     2
#define CONFIG_STRING  3
#define CONFIG_SCRIPT  4
#define CONFIG_IP      5
#define CONFIG_ESA     6
#define CONFIG_NETPORT 7


/* Maximum data size of config file */
#define MAX_CONFIG_DATA   4096


/* ********** Local Module Routines ************** */

/* Type definition of a bool type */
typedef unsigned char bool;

/* File descriptos */
FILE* config_fd   = NULL; /* Config File */
FILE* dat_fd   = NULL;    /* Output Text File */


/* Structure for the raw config file */
struct config_option {
    char *key;
    char *title;
    char *enable;
    bool  enable_sense;
    int   type;
    unsigned long dflt;
} CYG_HAL_TABLE_TYPE;

/* Structure for config elements in config file */
static struct _config {
    unsigned long len;
    unsigned long key1;
    unsigned char config_data[MAX_CONFIG_DATA-(4*4)];
    unsigned long key2;
    unsigned long cksum;
};


/* Local Declaration and instanciation 
    of the config element block */
struct _config  ConfigBlock;

/* **********************************************
* Function : open_config_bin 
*
* Description : Open the raw config file  
*
* Params :filename - filename to open
*
* Returns:  True=okay
********************************************** */
static bool open_config_bin(const char *filename)
{
    bool retval = FALSE;
    
    /* Open the binary file in read only mode 
        and check for open error */
    if ((config_fd = fopen(filename,"rb+")) != NULL)
    {
        retval = TRUE;
    }
    
    return(retval);
}


/* **********************************************
* Function : close_config_bin 
*
* Description : Close the config file 
*
* Params : None
*
* Returns: True = okay
********************************************** */
static bool close_config_bin(void)
{
    bool retval = FALSE;
    
    /* Check to if the file descriptor has been assigned */
    if (config_fd != NULL)
    {
        fclose(config_fd);
        
        retval = TRUE;
    }
    
    return(retval);
}


/* **********************************************
* Function : open_dat_txt 
*
* Description : Open and truncate the text data  
*            file to write to
*
* Params : filename - File name to write to
*
* Returns: True=okay
********************************************** */
static bool open_dat_txt(const char *filename)
{
    bool retval = FALSE;
    
    /* Open the File,truncate in write only mode
        and check for open error */
    if ((dat_fd = fopen(filename,"w")) != NULL)
    {
        retval = TRUE;
    }
    
    return(retval);
}

/* **********************************************
* Function : close_dat_txt 
*
* Description : Close the data file 
*
* Params : None
*
* Returns: True=okay
********************************************** */
static bool close_dat_txt(void)
{
    bool retval = FALSE;
    
    /* Check to if the file descriptor has been assigned */
    if (dat_fd != NULL)
    {
        fclose(dat_fd);
        
        retval = TRUE;
    }
    
    return(retval);
}



/* **********************************************
* Function : swap32
*
* Description : Only for Intel Builds, byte swapping
*           of specific 32bit members of the  config structure
*
* Params : None 
*
* Returns: True=okay
********************************************** */
#if !defined(CYG_BIG_ENDIAN)
static bool swap32(void)
{
    ConfigBlock.key1 = CYG_SWAP32(ConfigBlock.key1);
    ConfigBlock.key2 = CYG_SWAP32(ConfigBlock.key2);
    ConfigBlock.len =  CYG_SWAP32(ConfigBlock.len);
    ConfigBlock.cksum = CYG_SWAP32(ConfigBlock.cksum);
    
    return 0;
}
#endif

/* **********************************************
* Function : read_config_block 
*
* Description : Read the config file.
*
* Params : None
*
* Returns: True=okay
********************************************** */
static bool read_config_block(void)
{
    bool retval = FALSE;
    
    /* Read the config file into local instantiation , also ensure it is the right size */
    if (fread((void*)&ConfigBlock,1,sizeof(struct _config),config_fd) == MAX_CONFIG_DATA)
    {
#if defined(DEBUG_ON)        
        printf("Read the correct number of data in\n");
#endif

#if !defined(CYG_BIG_ENDIAN)
        swap32();
#endif

        retval  = TRUE;
    }
    
    return(retval);
}

/* **********************************************
* Function : config_length 
*
* Description :  Used by support config routines,
*            check and validate the size of the config
*            element type.
*
* Params : type - config element type
*
* Returns: size of type
********************************************** */
static int config_length(int type)
{
    switch (type) {
    case CONFIG_BOOL:
        return sizeof(int);
    case CONFIG_INT:
        return sizeof(unsigned long);
    case CONFIG_IP:
//        return sizeof(struct in_addr);
        return sizeof(in_addr_t);
    case CONFIG_ESA:
        // Would like this to be sizeof(enet_addr_t), but that causes much
        // pain since it fouls the alignment of data which follows.
        return 8;
    case CONFIG_NETPORT:
        return MAX_STRING_LENGTH;
    case CONFIG_STRING:
        return MAX_STRING_LENGTH;
    case CONFIG_SCRIPT:
        return MAX_SCRIPT_LENGTH;
    default:
        return 0;
    }
}


/* **********************************************
* Function : flash_lookup_config
*
* Description : Look up key in config database 
*
* Params : *key - String to looik up.
*
* Returns:  data pointer in database of key
********************************************** */
static unsigned char * flash_lookup_config(char *key)
{
    unsigned char *dp;
    int len;
    struct _config *config = &ConfigBlock;
    
    dp = &config->config_data[0];
    while (dp < &config->config_data[sizeof(config->config_data)]) {
        len = 4 + CONFIG_OBJECT_KEYLEN(dp) + CONFIG_OBJECT_ENABLE_KEYLEN(dp) +
            config_length(CONFIG_OBJECT_TYPE(dp));
        if (strcmp(key, CONFIG_OBJECT_KEY(dp)) == 0) {
            return dp;
        }
        dp += len;
    
    }
    
    printf("Can't find config data for '%s'\n", key);
    
    return FALSE;
}


/* **********************************************
* Function : set_dat_file 
*
* Description : Create Data File with
*           Get and Set the Serial Number
*           Get and Set the MHX MAC Address
*           Get and Set the IXP MAC Address
*           This is all done by reading and parsing the
*           RedBoot Data File 
*
* Params : None
*
* Returns:  True=Everything is okay
********************************************** */
static bool set_dat_file(void)
{

    bool        retval = TRUE; /* TRUE */
    unsigned    char *dp = NULL;
    char        buff[100];
    unsigned int value;
	int device_fd;
        
    /* Clear block config structure to zero values */
    bzero((char*)&ConfigBlock,sizeof(struct _config));
    
#if defined(DEBUG_ON)        
    printf("Config Info\n");
    printf("-------------------------\n");
    printf("Length %x\n",ConfigBlock.len);
    printf("Key 1 %x\n",ConfigBlock.key1);
    printf("Key 2 %x\n",ConfigBlock.key2);
    printf("Checksum %x\n",ConfigBlock.cksum);
#endif    
    
    /* Read the config block */
    read_config_block();
    
#if defined(DEBUG_ON)        
    printf("Config Info\n");
    printf("-------------------------\n");
    printf("Length %x\n",ConfigBlock.len);
    printf("Key 1 %x\n",ConfigBlock.key1);
    printf("Key 2 %x\n",ConfigBlock.key2);
    printf("Checksum %x\n",ConfigBlock.cksum);
#endif    
    
    /* Look up the Serial Number */
    if ( (dp = flash_lookup_config("vip_serial")) != NULL)
    {
        value = 0;        
#if defined(DEBUG_ON)        
        printf("Found Config Item %s\n",CONFIG_OBJECT_KEY(dp));
#endif
        
        /* Get the serial number value */
        memcpy((unsigned char *)&value, CONFIG_OBJECT_VALUE(dp), sizeof(unsigned int));
        
#if !defined(CYG_BIG_ENDIAN)
        value = CYG_SWAP32(value);
#endif        
        /* Write the serial number to local buffer */
        sprintf(buff,"VIP_SERIALNO=012-%2d",value);
        
#if defined(DEBUG_ON)        
        /* Print the Data to Console */
        printf("%s\n",buff);
#endif
        
        /* Write the Data to File */
        fprintf(dat_fd,"%s\n",buff);
        
    }
    else
    {
        retval = FALSE;
    }
    
    /* Look up the IXP1 MAC Address  */
    if ( (dp = flash_lookup_config("npe_eth0_esa")) != NULL)
    {

        unsigned char *e_addr = NULL;
        
#if defined(DEBUG_ON)        
        printf("Found Config Item %s\n",CONFIG_OBJECT_KEY(dp));
#endif
        
        /* Get the MAC address value */
        e_addr = CONFIG_OBJECT_VALUE(dp);
        
        /* Write the IXP0 MAC Address to local buffer */
        sprintf(buff,"IXP0_MACADDR=%02x:%02x:%02x:%02x:%02x:%02x",e_addr[0],e_addr[1],e_addr[2],e_addr[3],e_addr[4],e_addr[5]);
        
#if defined(DEBUG_ON)        
        /* Print the Data to Console */
        printf("%s\n",buff);
#endif
        
        /* Write the Data to File */
        fprintf(dat_fd,"%s\n",buff);

    }
    else
    {
        retval = FALSE;
    }

    /* Look up the IXP1 MAC Address  */
    if ( (dp = flash_lookup_config("npe_eth1_esa")) != NULL)
    {

        unsigned char *e_addr = NULL;
        
#if defined(DEBUG_ON)        
        printf("Found Config Item %s\n",CONFIG_OBJECT_KEY(dp));
#endif
        
        /* Get the MAC address value */
        e_addr = CONFIG_OBJECT_VALUE(dp);
        
        /* Write the IXP1 MAC Address to local buffer */
        sprintf(buff,"IXP1_MACADDR=%02x:%02x:%02x:%02x:%02x:%02x",e_addr[0],e_addr[1],e_addr[2],e_addr[3],e_addr[4],e_addr[5]);
        
#if defined(DEBUG_ON)        
        /* Print the Data to Console */
        printf("%s\n",buff);
#endif
        
        /* Write the Data to File */
        fprintf(dat_fd,"%s\n",buff);

    }
    else
    {
        retval = FALSE;
    }

    /* Look up the Product Name */
    if ( (dp = flash_lookup_config("vip_productsku")) != NULL)
    {
        char svalue[MAX_STRING_LENGTH] = "MHS100700";
        
#if defined(DEBUG_ON)        
        printf("Found Config Item %s\n",CONFIG_OBJECT_KEY(dp));
#endif
        
       if (CONFIG_OBJECT_TYPE(dp) == CONFIG_STRING) {
            strcpy(svalue, CONFIG_OBJECT_VALUE(dp));
       }
        /* Write the serial number to local buffer */
        sprintf(buff,"PRODUCT_SKU=%s",svalue);
        
#if defined(DEBUG_ON)        
        /* Print the Data to Console */
        printf("%s\n",buff);
#endif
        
        /* Write the Data to File */
        fprintf(dat_fd,"%s\n",buff);
        
    }
    else
    {
        retval = FALSE;

        //Set default to 0
        sprintf(buff,"PRODUCT_NAME=%s", "MHS100700");
        
        /* Write the Data to File */
        fprintf(dat_fd,"%s\n",buff);
    }

    if ((device_fd = open(DEVICE_NAME,O_RDONLY)) >= 0 )
    {
        int revision = 0;

        ioctl(device_fd,IOCTL_GET_HW_REV,&revision);
        if (revision == 0) {
            sprintf(buff,"HARDWARE=v0.5.0");
        } else {
            sprintf(buff,"HARDWARE=v%01d.0.0", revision);
        }
        /* Write the Data to File */
        fprintf(dat_fd,"%s\n",buff);
        close(device_fd);
    } else {
        sprintf(buff,"HARDWARE=v0.5.0");
        /* Write the Data to File */
        fprintf(dat_fd,"%s\n",buff);
    }

    return(retval);    
}


/* **********************************************
* Function : display_header 
*
* Description : Display Program Info 
*
* Params : None
*
* Returns: Always TRUE
********************************************** */
static bool display_header(void)
{
    bool retval = FALSE;
    
    printf("\n");
    printf("vip-config [OPTIONS] [-?]=help\n");
    printf("(C)2005 Microhard Systems Inc.\n\n");
    
    retval = TRUE;
    
    return(retval);
}

/* **********************************************
* Function : display_header
*
* Description : Display program command line arguments 
*
* Params : None
*
* Returns: Always TRUE
********************************************** */
static bool display_args(void)
{
    bool retval = FALSE;
    
    printf("Options[ -i <file> -o <file> ]\n");
    
    retval = TRUE;
    
    return(retval);
}


/* **********************************************
* Function :  main
*
* Description : Start of program 
*
* Params :  argc - Number of command line args.
*           argv - String ponter of cli args.
*
* Returns: 0=Okay
********************************************** */
int main(int argc, char *argv[])
{
    int retval          = EXIT_VAL_ERR;
    int i               = 0x00000000;
    bool input_set      = FALSE;
    bool output_set     = FALSE;
    char input_file[100];             
    char output_file[100];             
    


    /* No less then the 2 required params */
    if (argc < 5)
    {
        display_header();
        display_args();
        return(retval);
    }

    
    /* Go through all 5 cli agruments */
    for (i=1;i<5;i++)
    {
        /* File Input (Config File) */
        if (strcmp(argv[i],"-i") == 0)
        {
            input_set = TRUE;
            i++;
            /* Set the Config File Name */
            strcpy(input_file,argv[i]);
        }
        /* File Output (Data File) */
        if (strcmp(argv[i],"-o") == 0)
        {
            output_set = TRUE;
            i++;
            /* Set the Data File Name */
            strcpy(output_file,argv[i]);
        }
    
    }
    
    /* Is everything okay with the cli and the right 
       number of agruments has been sent?
       If not then exit
    */
    if ( (output_set == FALSE) || (input_set == FALSE) ) 
    {
        display_args();
        return(retval);
    }
    
    /* Open the Config File */
    if ( open_config_bin(input_file) == TRUE)
    {

#if defined(DEBUG_ON)        
        printf("File %s has been opened successfully\n",input_file);
#endif        
        
        /* Open the Data File */
        if (open_dat_txt(output_file) == TRUE)
        {
#if defined(DEBUG_ON)        
            printf("File %s has been opened successfully\n",output_file);
#endif        
        
            /* Process and build Text Data File */
            set_dat_file();
        
            /* Close the data file */
            close_dat_txt();
        }
        else
        {
            retval = EXIT_VAL_ERR_OPEN;
        }
        
        /* Close the Config File */
        close_config_bin();
        
        /* Life is good */
        retval = EXIT_VAL_OK;
    }
    else
    {
        retval = EXIT_VAL_ERR_OPEN;
    }
    
    return(retval);
}
