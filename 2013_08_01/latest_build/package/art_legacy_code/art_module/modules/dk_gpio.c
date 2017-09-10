#include "dk.h"
#include "dk_gpio.h"
#include "client.h"
#include "dk_pci_bus.h"
#include <asm/io.h>
#include <linux/kernel.h>

/*
 * Set GPIO inputs
 */
int dk_set_gpio_input(UINT32 baseaddr, UINT32 gpio)
{
    UINT32 val;

	if (gpio >= NUM_GPIO) return -EINVAL;
    val = (__raw_readl(baseaddr + GPIOCR) & ~DK_GPIOCR_OUT(gpio)) | DK_GPIOCR_IN(gpio);
    __raw_writel(val, baseaddr + GPIOCR);
    //printk(KERN_INFO "dk_set_gpio_input: val=0x%x, reg=0x%x\n", val, baseaddr + GPIOCR);
	return 0;
}


/*
 * Set GPIO outputs
 */
int dk_set_gpio_output(UINT32 baseaddr, UINT32 gpio)
{
    UINT32 val;

    if (gpio >= NUM_GPIO) return -EINVAL;
    val = (readl(baseaddr + GPIOCR) & ~DK_GPIOCR_OUT(gpio)) | DK_GPIOCR_OUT(gpio);
    writel(val, baseaddr + GPIOCR);
    //printk(KERN_INFO "dk_set_gpio_output: val=0x%x, reg=0x%x\n", val, baseaddr + GPIOCR);
	return 0;
}

/*
 * Get GPIO state
 */
UINT32 dk_get_gpio(UINT32 baseaddr, UINT32 gpio)
{
	if (gpio >=NUM_GPIO) return -EINVAL;

	/* GPIO input magic */
	return ((readl(baseaddr + GPIODI) & GPIOD_MASK) >> gpio) & 0x1;
}

/*
 * Set GPIO state
 */
int dk_set_gpio(UINT32 baseaddr, UINT32 gpio, UINT32 val)
{
	UINT32 data;

	if (gpio >= NUM_GPIO) return -EINVAL;
	/* GPIO output magic */
	data = readl(baseaddr + GPIODO);

	data &= ~(1 << gpio);
	data |= (val & 1) << gpio;

    writel(data, baseaddr + GPIODO);
    //printk(KERN_INFO "dk_set_gpio: data=0x%x, reg=0x%x\n", data, baseaddr + GPIODO);
    return 0;
}
