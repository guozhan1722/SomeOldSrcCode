#ifndef _ARM_VIP_BSP_MODULE_H
#define _ARM_VIP_BSP_MODULE_H

#define VIP_MAJOR_DEVICE            122
#define VIP_MINOR_DEVICE 			0

#define VIP_BSP_MAGICNO 'V'

#define IOCTL_GPIO3_HIGH      _IOW(VIP_BSP_MAGICNO,0x10,int)      
#define IOCTL_GPIO3_LOW       _IOW(VIP_BSP_MAGICNO,0x11,int)

#define IOCTL_GPIO4_HIGH            _IOW(VIP_BSP_MAGICNO,0x20,int)
#define IOCTL_GPIO4_LOW             _IOW(VIP_BSP_MAGICNO,0x21,int)

#define IOCTL_GPIO5_HIGH            _IOW(VIP_BSP_MAGICNO,0x30,int)
#define IOCTL_GPIO5_LOW             _IOW(VIP_BSP_MAGICNO,0x31,int)

#define IOCTL_GPIO11_HIGH           _IOW(VIP_BSP_MAGICNO,0x40,int)
#define IOCTL_GPIO11_LOW            _IOW(VIP_BSP_MAGICNO,0x41,int)

#define IOCTL_GPIO_SET              _IOW(VIP_BSP_MAGICNO,0x50,int)
#define IOCTL_GPIO_GET              _IOR(VIP_BSP_MAGICNO,0x51,int)

#define IOCTL_GET_HW_REV            _IOR(VIP_BSP_MAGICNO,0x61,int)
#define IOCTL_SET_CPU_SPEED         _IOW(VIP_BSP_MAGICNO,0x70,int)
#define IOCTL_GET_CPU_SPEED         _IOR(VIP_BSP_MAGICNO,0x71,int)

struct gpio_ioctl {
	unsigned int gpio;
	unsigned int value;
};

#endif

