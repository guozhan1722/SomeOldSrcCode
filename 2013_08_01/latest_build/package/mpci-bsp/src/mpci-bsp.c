/* ************************************************************************
* mpci-bsp.c
*
* Description : BSP Routines for the mpci Platform.
*
* Written by : LL
*
* May 2, 2005 - WDT Handler
*
* (C)2005 MicroHard Corp.
*
* ********************************************************************** */
#ifndef __KERNEL__
#define __KERNEL__
#endif
#ifndef MODULE
#define MODULE
#endif

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/gpio.h>

#include <asm/mach-ar71xx/ar71xx.h>

#include <linux/version.h>
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/cache.h>
#include <linux/major.h>
#include <linux/uaccess.h>
#include <asm/mach-ar71xx/gpio.h>


#include "mpci-bsp.h"

#define MPCI_GPIO_PIN_11      11
#define MPCI_GPIO_PIN_10      10
#define MPCI_GPIO_PIN_9      9
#define MPCI_GPIO_PIN_8      8
#define MPCI_GPIO_PIN_7      7
#define MPCI_GPIO_PIN_6      6
#define MPCI_GPIO_PIN_5      5
#define MPCI_GPIO_PIN_4      4
#define MPCI_GPIO_PIN_3      3
#define MPCI_GPIO_PIN_2      2
#define MPCI_GPIO_PIN_1      1
#define MPCI_GPIO_PIN_0      0

#ifdef _BSP_DEBUG
#define dbg(format, arg...) printk(KERN_DEBUG __FILE__": " format "\n" , ## arg)
#else
#define dbg(format, arg...) do {} while (0)
#endif

#define err(format, arg...) printk(KERN_ERR __FILE__ ": " format "\n" , ## arg)


MODULE_LICENSE("GPL");

static struct microhard_bsp_dev
{
	int mpci_major;
	int mpci_minor;
	dev_t dev;
	struct semaphore sem; 
	struct cdev mpci_cdev;
} mpci_bsp_dev;



/* **********************************************
* Function :  mpci_bsp_ioctl 
*
* Description : I/O Control of this device 
*
* Params :cmd - Command from user application
*         arg - not used
*
* Returns: 0=Okay
********************************************** */
static int mpci_bsp_ioctl(struct inode *inode, struct file *file,
                         unsigned int cmd, unsigned long arg)
{
    struct gpio_ioctl info;
    struct reg_info rinfo;
    int err = 0;
    
    void __iomem *base = ar71xx_gpio_base;

    dbg("cmd = %x", cmd);
    
    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY before verify_area()
     */
    if (_IOC_TYPE(cmd) != MPCI_BSP_MAGICNO) 
        return -ENOTTY;
    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. `Type' is user-oriented, while
     * verify_area is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
       err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
   	else if (_IOC_DIR(cmd) & _IOC_WRITE)
      err =  !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));	
	if (err) 
    return -EFAULT;
    
    /* Act on the command that is set by the user
      application which has opened this device */
    switch (cmd) {
    case IOCTL_GPIO_GET:
        if (copy_from_user(&info, (void *)arg, sizeof(struct gpio_ioctl)))
            return -EFAULT;
    
        info.value=gpio_get_value(info.gpio);
    
        if (copy_to_user((void *)arg, &info, sizeof(struct gpio_ioctl))) {
            return -EFAULT;
        }
    
        break;
    
    case IOCTL_GPIO_SET:
        if (copy_from_user(&info, (void *)arg, sizeof(struct gpio_ioctl)))
            return -EFAULT;

        gpio_set_value(info.gpio ,info.value);
    
        break;
    
    case IOCTL_REG_GET:
        if (copy_from_user(&rinfo, (void *)arg, sizeof(struct reg_info)))
            return -EFAULT;
        //printk(KERN_INFO "the reg val=%04x and reg=%04x\n",rinfo.val,rinfo.reg);
        rinfo.val=*(volatile unsigned int *)rinfo.reg;

        if (copy_to_user((void *)arg, &rinfo, sizeof(struct reg_info))) {
            return -EFAULT;
        }

        break;

    case IOCTL_REG_SET:
        if (copy_from_user(&rinfo, (void *)arg, sizeof(struct reg_info)))
            return -EFAULT;

         *(volatile unsigned int  *)rinfo.reg = rinfo.val;

        break;
            
    case IOCTL_GPIO3_HIGH:
        dbg("IOCTL_GPIO3_HIGH");
        __raw_writel(__raw_readl(base + GPIO_REG_OE) | (1 << MPCI_GPIO_PIN_0 ),
                 base + GPIO_REG_OE);

        __raw_writel(1 << MPCI_GPIO_PIN_0, base + GPIO_REG_SET);

        break;
    
    case IOCTL_GPIO3_LOW:
        dbg("IOCTL_GPIO3_LOW");
        __raw_writel(__raw_readl(base + GPIO_REG_OE) | (1 << MPCI_GPIO_PIN_0 ),
                 base + GPIO_REG_OE);

        __raw_writel(1 << MPCI_GPIO_PIN_0, base + GPIO_REG_CLEAR);
        break;
    
    case IOCTL_GPIO4_HIGH:
        dbg("IOCTL_GPIO4_HIGH");

        __raw_writel(__raw_readl(base + GPIO_REG_OE) | (1 << MPCI_GPIO_PIN_1 ),
                 base + GPIO_REG_OE);

        __raw_writel(1 << MPCI_GPIO_PIN_1, base + GPIO_REG_SET);

        break;
    
    case IOCTL_GPIO4_LOW:
        dbg("IOCTL_GPIO4_LOW");
        __raw_writel(__raw_readl(base + GPIO_REG_OE) | (1 << MPCI_GPIO_PIN_1 ),
                 base + GPIO_REG_OE);

        __raw_writel(1 << MPCI_GPIO_PIN_1, base + GPIO_REG_CLEAR);
        break;
    
/*    case IOCTL_GPIO5_HIGH:
        dbg("IOCTL_GPIO5_HIGH");
        gpio_line_config(MPCI_GPIO_PIN_5, MPCI_GPIO_OUT);
        gpio_line_set(MPCI_GPIO_PIN_5, MPCI_GPIO_HIGH);
        break;
    
    case IOCTL_GPIO5_LOW:
        dbg("IOCTL_GPIO5_LOW");
        gpio_line_config(MPCI_GPIO_PIN_5, MPCI_GPIO_OUT);
        gpio_line_set(MPCI_GPIO_PIN_5, MPCI_GPIO_LOW);
        break;
    
    case IOCTL_GPIO11_HIGH:
        dbg("IOCTL_GPIO11_HIGH");
        gpio_line_config(MPCI_GPIO_PIN_11, MPCI_GPIO_OUT);
        gpio_line_set(MPCI_GPIO_PIN_11, MPCI_GPIO_HIGH);
        break;
    
    case IOCTL_GPIO11_LOW:
        dbg("IOCTL_GPIO11_LOW");
        gpio_line_config(MPCI_GPIO_PIN_11, MPCI_GPIO_OUT);
        gpio_line_set(MPCI_GPIO_PIN_11, MPCI_GPIO_LOW);
        break;
*/    
    case IOCTL_GET_HW_REV:
        {
            int revision = 0x1;
    
            if (copy_to_user((void *)arg, &revision, sizeof(int))) {
                return -EFAULT;
            }
        }
    
        break;
    
    }
    return 0;
}

/* Static Device structure declaration and defaults */
static const struct file_operations mpci_bsp_fops = {
	.ioctl=  mpci_bsp_ioctl,
};

static struct class *mpci_bsp;

/* **********************************************
* Function : mpci_bsp_init 
*
* Description : Initialize the BSP Driver
*
* Params : None
*
* Returns: 0=okay
********************************************** */
static int __init mpci_bsp_init(void)
{ 
    int ret;
    struct microhard_bsp_dev *vbsp_dev=&mpci_bsp_dev;
    int revision = 1;


    vbsp_dev->mpci_major = MPCI_MAJOR_DEVICE;
    vbsp_dev->mpci_minor = MPCI_MINOR_DEVICE;
    /*
     * if it is given major number use it otherwise use the one which be given by system
     */
    if (vbsp_dev->mpci_major) {
        vbsp_dev->dev = MKDEV(vbsp_dev->mpci_major, vbsp_dev->mpci_minor);
        ret = register_chrdev_region(vbsp_dev->dev, 1, "vip-bsp");
    } else {
        ret = alloc_chrdev_region(&vbsp_dev->dev, vbsp_dev->mpci_minor, 1,"vip-bsp");
        vbsp_dev->mpci_major = MAJOR(vbsp_dev->dev);
    }

    if (ret < 0) {
        err("mpci_bsp: can't get mpci-bsp device major %d\n", vbsp_dev->mpci_major);
        return ret;
    }

    /*
     * init cdev and prepare file operation 
     */
    cdev_init(&vbsp_dev->mpci_cdev, &mpci_bsp_fops);
    vbsp_dev->mpci_cdev.owner = THIS_MODULE;
    ret= cdev_add (&vbsp_dev->mpci_cdev, vbsp_dev->dev, 1);
    if (ret) {
        err("Error %d adding mpci_bsp device module", ret);
        return ret;
    }

    mpci_bsp = class_create( THIS_MODULE,"vip-bsp");
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,25)
    device_create(mpci_bsp, NULL, vbsp_dev->dev, "vip-bsp");
#else
//    device_create_drvdata(mpci_bsp, NULL, vbsp_dev->dev, NULL, "mpci-bsp");
    device_create(mpci_bsp, NULL, vbsp_dev->dev, NULL, "vip-bsp");
#endif

    return ret;
}


/* **********************************************
* Function : mpci_bsp_cleanup
*
* Description : Called when the driver is unloaded from 
*       memory 
*
* Params :None
*
* Returns:None 
********************************************** */

static void __exit mpci_bsp_cleanup(void)
{ 
    struct microhard_bsp_dev *vbsp_dev=&mpci_bsp_dev;

    device_destroy(mpci_bsp,vbsp_dev->dev);
    class_destroy(mpci_bsp);
    unregister_chrdev_region(vbsp_dev->dev, 1);
    cdev_del(&vbsp_dev->mpci_cdev);
}

module_init(mpci_bsp_init);
module_exit(mpci_bsp_cleanup);

