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


static int display_header(void)
{
    int retval = 0;
    
    return(retval);
}

static int display_args(void)
{
    int retval = 0;
    return(retval);
}


int main(int argc, char *argv[])
{
    return 0;
}
