/* ************************************************************************
* vip-bsp.c
*
* Description : BSP Routines for the vip Platform.
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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/slab.h>
#include <linux/ioport.h>
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

//#include <mach/io.h>		/* inb/outb */
#include <mach/gpio.h>

#include "vip-bsp.h"

#ifdef _BSP_DEBUG
#define dbg(format, arg...) printk(KERN_DEBUG __FILE__": " format "\n" , ## arg)
#else
#define dbg(format, arg...) do {} while (0)
#endif

#define err(format, arg...) printk(KERN_ERR __FILE__ ": " format "\n" , ## arg)
#define info(format, arg...) printk(KERN_INFO __FILE__ ": " format "\n" , ## arg)
#define warn(format, arg...) printk(KERN_WARNING __FILE__ ": " format "\n" , ## arg)

#define IXP4XX_GPIO_PIN_13      13
#define IXP4XX_GPIO_PIN_12      12
#define IXP4XX_GPIO_PIN_11      11
#define IXP4XX_GPIO_PIN_10      10
#define IXP4XX_GPIO_PIN_9      9
#define IXP4XX_GPIO_PIN_8      8
#define IXP4XX_GPIO_PIN_7      7
#define IXP4XX_GPIO_PIN_6      6
#define IXP4XX_GPIO_PIN_5      5
#define IXP4XX_GPIO_PIN_4      4
#define IXP4XX_GPIO_PIN_3      3
#define IXP4XX_GPIO_PIN_2      2
#define IXP4XX_GPIO_PIN_1      1
#define IXP4XX_GPIO_PIN_0      0

MODULE_LICENSE("GPL");

static struct microhard_bsp_dev
{
	int vip_major;
	int vip_minor;
	dev_t dev;
	struct semaphore sem; 
	struct cdev vip_cdev;
} vip_bsp_dev;



/* **********************************************
* Function :  vip_bsp_ioctl 
*
* Description : I/O Control of this device 
*
* Params :cmd - Command from user application
*         arg - not used
*
* Returns: 0=Okay
********************************************** */
static int vip_bsp_ioctl(struct inode *inode, struct file *file,
                         unsigned int cmd, unsigned long arg)
{
    struct gpio_ioctl info;
    int err = 0;
    
    dbg("cmd = %x", cmd);
    
    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY before verify_area()
     */
    if (_IOC_TYPE(cmd) != VIP_BSP_MAGICNO) 
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
    
        gpio_line_get(IXP4XX_GPIO_PIN_0 + info.gpio, &info.value);
    
        if (copy_to_user((void *)arg, &info, sizeof(struct gpio_ioctl))) {
            return -EFAULT;
        }
    
        break;
    
    case IOCTL_GPIO_SET:
        if (copy_from_user(&info, (void *)arg, sizeof(struct gpio_ioctl)))
            return -EFAULT;
        gpio_line_config(IXP4XX_GPIO_PIN_0 + info.gpio, IXP4XX_GPIO_OUT);
        gpio_line_set(IXP4XX_GPIO_PIN_0 + info.gpio, (info.value > 0) ? IXP4XX_GPIO_HIGH : IXP4XX_GPIO_LOW);
    
        break;
    
    case IOCTL_GPIO3_HIGH:
        dbg("IOCTL_GPIO3_HIGH");
        gpio_line_config(IXP4XX_GPIO_PIN_3, IXP4XX_GPIO_OUT);
        gpio_line_set(IXP4XX_GPIO_PIN_3, IXP4XX_GPIO_HIGH);
        break;
    
    case IOCTL_GPIO3_LOW:
        dbg("IOCTL_GPIO3_LOW");
        gpio_line_config(IXP4XX_GPIO_PIN_3, IXP4XX_GPIO_OUT);
        gpio_line_set(IXP4XX_GPIO_PIN_3, IXP4XX_GPIO_LOW);
        break;
    
    case IOCTL_GPIO4_HIGH:
        dbg("IOCTL_GPIO4_HIGH");
        gpio_line_config(IXP4XX_GPIO_PIN_4, IXP4XX_GPIO_OUT);
        gpio_line_set(IXP4XX_GPIO_PIN_4, IXP4XX_GPIO_HIGH);
        break;
    
    case IOCTL_GPIO4_LOW:
        dbg("IOCTL_GPIO4_LOW");
        gpio_line_config(IXP4XX_GPIO_PIN_4, IXP4XX_GPIO_OUT);
        gpio_line_set(IXP4XX_GPIO_PIN_4, IXP4XX_GPIO_LOW);
        break;
    
    case IOCTL_GPIO5_HIGH:
        dbg("IOCTL_GPIO5_HIGH");
        gpio_line_config(IXP4XX_GPIO_PIN_5, IXP4XX_GPIO_OUT);
        gpio_line_set(IXP4XX_GPIO_PIN_5, IXP4XX_GPIO_HIGH);
        break;
    
    case IOCTL_GPIO5_LOW:
        dbg("IOCTL_GPIO5_LOW");
        gpio_line_config(IXP4XX_GPIO_PIN_5, IXP4XX_GPIO_OUT);
        gpio_line_set(IXP4XX_GPIO_PIN_5, IXP4XX_GPIO_LOW);
        break;
    
    case IOCTL_GPIO11_HIGH:
        dbg("IOCTL_GPIO11_HIGH");
        gpio_line_config(IXP4XX_GPIO_PIN_11, IXP4XX_GPIO_OUT);
        gpio_line_set(IXP4XX_GPIO_PIN_11, IXP4XX_GPIO_HIGH);
        break;
    
    case IOCTL_GPIO11_LOW:
        dbg("IOCTL_GPIO11_LOW");
        gpio_line_config(IXP4XX_GPIO_PIN_11, IXP4XX_GPIO_OUT);
        gpio_line_set(IXP4XX_GPIO_PIN_11, IXP4XX_GPIO_LOW);
        break;
    
    case IOCTL_GET_HW_REV:
        {
            int revision = (*IXP4XX_EXP_CFG0 >> 17) & 0xf;
    
            if (copy_to_user((void *)arg, &revision, sizeof(int))) {
                return -EFAULT;
            }
        }
    
        break;
    
    case IOCTL_SET_CPU_SPEED:
        {
            int speed = 4, cfg = 0;
            if (copy_from_user(&speed, (void *)arg, sizeof(int)))
                return -EFAULT;
            
            cfg = (*IXP4XX_EXP_CFG0);
            cfg &= ~(7 << 21);
            cfg |= (speed << 21);
    
            *IXP4XX_EXP_CFG0 = cfg;
        }
    
        break;
    
    case IOCTL_GET_CPU_SPEED:
        {
            int speed = 0;
            
            speed = (*IXP4XX_EXP_CFG0 >> 21) & 0x7;
            
            if (copy_to_user((void *)arg, &speed, sizeof(int))) {
                return -EFAULT;
            }
        }
    
        break;
    
    }
    return 0;
}

/* Static Device structure declaration and defaults */
static const struct file_operations vip_bsp_fops = {
	.ioctl=  vip_bsp_ioctl,
};

static struct class *vip_bsp;

/* **********************************************
* Function : vip_bsp_init 
*
* Description : Initialize the BSP Driver
*
* Params : None
*
* Returns: 0=okay
********************************************** */
static int __init vip_bsp_init(void)
{ 
    int ret;
    struct microhard_bsp_dev *vbsp_dev=&vip_bsp_dev;
    int revision = (*IXP4XX_EXP_CFG0 >> 17) & 0xf;


    vbsp_dev->vip_major = VIP_MAJOR_DEVICE;
    vbsp_dev->vip_minor = VIP_MINOR_DEVICE;
    /*
     * if it is given major number use it otherwise use the one which be given by system
     */
    if (vbsp_dev->vip_major) {
        vbsp_dev->dev = MKDEV(vbsp_dev->vip_major, vbsp_dev->vip_minor);
        ret = register_chrdev_region(vbsp_dev->dev, 1, "vip-bsp");
    } else {
        ret = alloc_chrdev_region(&vbsp_dev->dev, vbsp_dev->vip_minor, 1,"vip-bsp");
        vbsp_dev->vip_major = MAJOR(vbsp_dev->dev);
    }

    if (ret < 0) {
        err("vip_bsp: can't get vip-bsp device major %d\n", vbsp_dev->vip_major);
        return ret;
    }

    /*
     * init cdev and prepare file operation 
     */
    cdev_init(&vbsp_dev->vip_cdev, &vip_bsp_fops);
    vbsp_dev->vip_cdev.owner = THIS_MODULE;
    ret= cdev_add (&vbsp_dev->vip_cdev, vbsp_dev->dev, 1);
    if (ret) {
        err("Error %d adding vip_bsp device module", ret);
        return ret;
    }

    vip_bsp = class_create( THIS_MODULE,"vip-bsp");
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,25)
    device_create(vip_bsp, NULL, vbsp_dev->dev, "vip-bsp");
#else
//    device_create_drvdata(vip_bsp, NULL, vbsp_dev->dev, NULL, "vip-bsp");
    device_create(vip_bsp, NULL, vbsp_dev->dev, NULL, "vip-bsp");
#endif

    //init reset button to be input and internal pull-up
    gpio_line_config(IXP4XX_GPIO_PIN_13, IXP4XX_GPIO_IN);

    return ret;
}


/* **********************************************
* Function : vip_bsp_cleanup
*
* Description : Called when the driver is unloaded from 
*       memory 
*
* Params :None
*
* Returns:None 
********************************************** */

static void __exit vip_bsp_cleanup(void)
{ 
    struct microhard_bsp_dev *vbsp_dev=&vip_bsp_dev;

    device_destroy(vip_bsp,vbsp_dev->dev);
    class_destroy(vip_bsp);
    unregister_chrdev_region(vbsp_dev->dev, 1);
    cdev_del(&vbsp_dev->vip_cdev);
}

module_init(vip_bsp_init);
module_exit(vip_bsp_cleanup);

