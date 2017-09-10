#ifndef _AR71xx_MPCI_BSP_MODULE_H
#define _AR71xx_MPCI_BSP_MODULE_H

#define MPCI_MAJOR_DEVICE            122
#define MPCI_MINOR_DEVICE 			0

#define MPCI_BSP_MAGICNO 'V'

#define IOCTL_GPIO3_HIGH      _IOW(MPCI_BSP_MAGICNO,0x10,int)      
#define IOCTL_GPIO3_LOW       _IOW(MPCI_BSP_MAGICNO,0x11,int)

#define IOCTL_GPIO4_HIGH            _IOW(MPCI_BSP_MAGICNO,0x20,int)
#define IOCTL_GPIO4_LOW             _IOW(MPCI_BSP_MAGICNO,0x21,int)

#define IOCTL_GPIO5_HIGH            _IOW(MPCI_BSP_MAGICNO,0x30,int)
#define IOCTL_GPIO5_LOW             _IOW(MPCI_BSP_MAGICNO,0x31,int)

#define IOCTL_GPIO11_HIGH           _IOW(MPCI_BSP_MAGICNO,0x40,int)
#define IOCTL_GPIO11_LOW            _IOW(MPCI_BSP_MAGICNO,0x41,int)

#define IOCTL_GPIO_SET              _IOW(MPCI_BSP_MAGICNO,0x50,int)
#define IOCTL_GPIO_GET              _IOR(MPCI_BSP_MAGICNO,0x51,int)

#define IOCTL_GET_HW_REV            _IOR(MPCI_BSP_MAGICNO,0x61,int)
#define IOCTL_SET_CPU_SPEED         _IOW(MPCI_BSP_MAGICNO,0x70,int)
#define IOCTL_GET_CPU_SPEED         _IOR(MPCI_BSP_MAGICNO,0x71,int)
#define IOCTL_REG_SET               _IOR(MPCI_BSP_MAGICNO,0x72,int)
#define IOCTL_REG_GET               _IOR(MPCI_BSP_MAGICNO,0x73,int)

struct gpio_ioctl {
	unsigned int gpio;
	unsigned int value;
};

struct reg_info {
	unsigned int reg;
	unsigned int val;
};

/*some define for I2C-gpio */

#define I2C_SMBUS_INPUT_REG     0
#define I2C_SMBUS_OUTPUT_REG    1
#define I2C_SMBUS_POLARITY_REG  2
#define I2C_SMBUS_CONFIG_REG    3

#define I2C_RADIO_RESET         0
#define I2C_CPU_RING0           1
#define I2C_RADIO_BOOTPGM_MODE  2
#define I2C_CPU_RSMODE          3
#define I2C_RADIO_CONFIG        4            
#define I2C_RADIO_LED1          5
#define I2C_RADIO_LED2          6
#define I2C_RADIO_LED3          7

#endif

