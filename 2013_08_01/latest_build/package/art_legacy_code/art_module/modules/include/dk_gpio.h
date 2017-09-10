#ifndef DK_GPIO_H
#define DK_GPIO_H

#define EINVAL          22      /* Invalid argument */

#define NUM_GPIO        6		// Six numbered 0 to 5.
#define GPIOCR          0x4014  // GPIO control register
#define GPIODO          0x4018  // GPIO data output access register
#define GPIODI          0x401C  // GPIO data input access register
#define GPIOD_MASK      0x0000002F // Mask for reading or writing GPIO data regs

#define DK_GPIOCR_IN(n)	(0 << ((n) * 2))	/* Mode 0 for pin n */
#define DK_GPIOCR_OUT0(n)	(1 << ((n) * 2))	/* Mode 1 for pin n */
#define DK_GPIOCR_OUT1(n)	(2 << ((n) * 2))	/* Mode 2 for pin n */
#define DK_GPIOCR_OUT(n)	(3 << ((n) * 2))	/* Mode 3 for pin n */
#define DK_GPIOCR_INT_SEL(n)	((n) << 12)		/* Interrupt for GPIO pin n */


int dk_set_gpio_input(UINT32 baseaddr, UINT32 gpio);
int dk_set_gpio_output(UINT32 baseaddr, UINT32 gpio);
UINT32 dk_get_gpio(UINT32 baseaddr, UINT32 gpio);
int dk_set_gpio(UINT32 baseaddr, UINT32 gpio, UINT32 val);

#endif
