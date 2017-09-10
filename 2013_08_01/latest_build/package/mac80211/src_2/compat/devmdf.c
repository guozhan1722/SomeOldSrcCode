#ifndef __KERNEL__
#define __KERNEL__
#endif
#ifndef MODULE
#define MODULE
#endif

#include <linux/proc_fs.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/seq_file.h>
#include <linux/ioctl.h>
#include <linux/fcntl.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>

#define DEVICE_NAME "mdfst0"
struct _mdfdev
{
    struct class *mdf_class;  
    struct cdev mdf_cdev;            
    struct ieee80211_hw *mdfhw;
    struct wifpara wifoffset;	// waiting image	 
    struct wifpara acting;	// the working image	 
    dev_t devno;
    u32 cardtype;
    int MicroHardMode;
}; 
static struct _mdfdev mdfdev = {0};
static DEFINE_MUTEX(mdf_mutex);

static int mdfoffset_open(struct inode *inode, struct file *filp)
{
    struct _mdfdev *dev;
    dev = container_of(inode->i_cdev, struct _mdfdev, mdf_cdev);
    filp->private_data = dev;
    return 0;
}

static int mdfoffset_release(struct inode *inode, struct file *filp)
{
    return 0;
}

// read in fixed length
static int mdfoffset_read(struct file *filp, char *buf, size_t len, loff_t *off)
{  
    struct _mdfdev *dev=filp->private_data;

    if (copy_to_user(buf, &dev->acting, len))
        return - EFAULT;
    else return len;
}

// write in fixed length
static int mdfoffset_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
    struct _mdfdev *dev=filp->private_data;
    struct wifpara tmp;
    u8 valid = 0;

    memset(&tmp, 0, sizeof(struct wifpara));
    if (copy_from_user(&tmp, buf, len))
    {
        return - EFAULT;
    } 
  
    printk(KERN_INFO "\nmdf got request for mode(%d)\n", tmp.mdfmode); 
    if (mutex_is_locked( &mdf_mutex )) return -ENOTBLK;
    if (dev->wifoffset.pending)return -EBUSY; 
    else
    {
        while(!mdf_mutex_lock());	
        switch (dev->cardtype) {
        case CARDTYPE49G:
            if ((tmp.mdfmode == 1) || 
                (tmp.mdfmode == 4)) 
                valid =1;
            break;
        case CARDTYPEA5G:
            if ((tmp.mdfmode == 0) || 
                (tmp.mdfmode == 2)) 
                valid =1;
            break;
        case CARDTYPE23G:
            if (tmp.mdfmode == 3) 
                valid =1;
            break;
        case CARDTYPE24G:
             if (tmp.mdfmode == 5) 
                valid =1;
            break;
        case CARDTYPEA2G:
            if (tmp.mdfmode == 0) 
                valid =1;
            break;
        default:
            valid =0;
            break;
        }
        if(valid) {
            memcpy(&dev->wifoffset, &tmp, sizeof(struct wifpara));        
            printk(KERN_INFO "mdf current mode(%d), apply for mode(%d).\n",
               dev->acting.curntmd, tmp.mdfmode); 
            dev->wifoffset.pending = 1;
        }
        mdf_mutex_unlock();
    }
    return len;
}

static int mdfoffset_ioctl(struct inode *inodp, struct file *filp, unsigned int cmd, unsigned long arg)
{
#define RESETMDF 'r'
#define CMD_RESETMDF _IO(RESETMDF, 0)
#define REMOVEMDF 'm'
#define CMD_REMOVEMDF _IO(REMOVEMDF, 0)

    switch(cmd)
    {
    case CMD_RESETMDF:
      printk(KERN_INFO "reset MDF\n"); 
      break;
    case CMD_REMOVEMDF:
      printk(KERN_INFO "remove MDF\n");  
      break;  
    default:
      break;
    }     
    return 0;
}

static struct file_operations mdf_fops = {
owner:    THIS_MODULE,        
open:		mdfoffset_open,    
release:	mdfoffset_release,    	  
read:     mdfoffset_read,    
write:    mdfoffset_write,   
ioctl:    mdfoffset_ioctl,
};

static int mdfoffset_init(void)
{
	int err = 0;

    memset(&mdfdev, 0x00, sizeof(struct _mdfdev));
	if((err=alloc_chrdev_region(&mdfdev.devno, 0, 1, DEVICE_NAME))<0)
		goto reject;				

    cdev_init(&mdfdev.mdf_cdev, &mdf_fops);
    mdfdev.mdf_cdev.owner = THIS_MODULE;
    mdfdev.mdf_cdev.ops = &mdf_fops;

	if((err = cdev_add (&mdfdev.mdf_cdev, mdfdev.devno , 1))<0)
		return err;

    mdfdev.mdf_class =class_create(THIS_MODULE, DEVICE_NAME);
	if(IS_ERR(mdfdev.mdf_class)) err = -2;
    device_create(mdfdev.mdf_class, NULL, mdfdev.devno, NULL, DEVICE_NAME);

reject:
	return err;
}

static void mdfoffset_cleanup(void)
{
    unregister_chrdev_region(mdfdev.devno, 1); 
    cdev_del(&mdfdev.mdf_cdev);	
    device_destroy(mdfdev.mdf_class, mdfdev.devno);
    class_destroy(mdfdev.mdf_class);   
}

struct wifpara* ieee80211_mdf_read(void)
{ 
    return &mdfdev.wifoffset;  
}
EXPORT_SYMBOL(ieee80211_mdf_read);

int mdf_mutex_lock(void)
{ 
    return mutex_trylock(&mdf_mutex);
    // != 0, Got the lock!
}
EXPORT_SYMBOL(mdf_mutex_lock);

void mdf_mutex_unlock(void)
{ 
    mutex_unlock(&mdf_mutex); 
}
EXPORT_SYMBOL(mdf_mutex_unlock);

#if 0
void AddMeshMacBlack(char *Mac)
{
    int i=0;
    char empty[ETH_ALEN] = {0};

    for(i=0; i<MESHMACSIZE; i++ ) {
        if(memcmp(empty, MeshMacBlack[i], ETH_ALEN) == 0) break;   
        if(memcmp(Mac, MeshMacBlack[i], ETH_ALEN) == 0) return;   
    }
    if(i != MESHMACSIZE) memcpy(MeshMacBlack[i], Mac, ETH_ALEN); 
    else  printk(KERN_INFO "MeshMacBlack full!\n");
}

EXPORT_SYMBOL(AddMeshMacBlack);

int LookupMeshMacBlack(char *Mac)
{
    int i=0;
    char empty[ETH_ALEN] = {0};

    for(i=0; i<MESHMACSIZE; i++ ) {
        printk(KERN_INFO "LookupMeshMacBlack: MeshMacBlack[%d]=%pM, Mac=%pM,\n", i, MeshMacBlack[i], Mac);
        if(memcmp(MeshMacBlack[i], empty, ETH_ALEN) == 0) return -1; 
        if(memcmp(MeshMacBlack[i], Mac, ETH_ALEN) == 0) return 0;   // mach
    }
    return -1;
}
EXPORT_SYMBOL(LookupMeshMacBlack);
#endif

static const struct wifpara MicroHardType[MAXMICROHARDTYPES] = {
    {4940000,4990000,5000,2500,10,30,48},
    {2407000,2470000,5000,2500,10,30,48},
    {2299000,2365000,5000,0,10,30,48},    
};

void SetMicroHardMode(unsigned int mode)
{
    memset(&mdfdev.wifoffset, 0, sizeof(struct wifpara));
    mdfdev.MicroHardMode = -1;    
    if ((mode == AR5K_MICROHARD23G) || (mode == AR5K_MICROHARD24G)) {
        mdfdev.MicroHardMode = mode - 0x3010;
    } else if (mode == AR5K_MICROHARD49G) {
        mdfdev.MicroHardMode = 0;     
    } else if (mode == AR5K_ATHROS2G) {
        mdfdev.acting.cardmod = mode & CARDMODEMASK;
        mdfdev.cardtype = CARDTYPEA2G;         
    } else if (mode = AR5K_ATHROS5G) {
        mdfdev.acting.cardmod = mode & CARDMODEMASK;
        mdfdev.cardtype = CARDTYPEA5G;  
    } else {
         mdfdev.cardtype = 0;
    }
    if(mdfdev.MicroHardMode<0 || mdfdev.MicroHardMode>=MAXMICROHARDTYPES) return;
    memcpy(&mdfdev.wifoffset,  &MicroHardType[mdfdev.MicroHardMode], sizeof(struct wifpara));      

    // define default mode
    switch(mode) {
    case AR5K_MICROHARD49G: // 4940-4990, 2.5M per channel    
        mdfdev.cardtype = CARDTYPE49G;
        mdfdev.wifoffset.mdfmode = 4;   //5,10,15...
        break;
    case AR5K_MICROHARD23G: // 2.3G
        mdfdev.cardtype = CARDTYPE23G;
        mdfdev.wifoffset.mdfmode = 3;   
        break;
    case AR5K_MICROHARD24G: // 2.4G
        mdfdev.cardtype = CARDTYPE24G;
        mdfdev.wifoffset.mdfmode = 5;   
        break;    
    default: 
        break;
    }

    mdfdev.wifoffset.cardmod = mode & CARDMODEMASK;     
    mdfdev.wifoffset.pending = 1;
    mdfdev.wifoffset.boot = 1;
    memset(&mdfdev.acting, 0, sizeof(struct wifpara));
    printk(KERN_INFO "SubID=0x%x, MicroHardMode=%d, cardtype=%d\n", mode, mdfdev.MicroHardMode, mdfdev.cardtype);
}
EXPORT_SYMBOL(SetMicroHardMode);

