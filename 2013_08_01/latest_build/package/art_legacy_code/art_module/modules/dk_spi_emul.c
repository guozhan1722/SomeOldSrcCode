//
//  SPI emuilation for AR5414
//
//  Copyright SG Microhard Systems Inc
//
//  2011.06.14 - created
//
#include "dk.h"
#include "dk_gpio.h"
#include "dk_adf4350.h"
#include <linux/kernel.h>
#include <linux/delay.h>


#define SET_CLOCK  dk_set_gpio(baseaddr, PORT_CLK, 1)
#define CLR_CLOCK  dk_set_gpio(baseaddr, PORT_CLK, 0)
#define DATA_OUT(x) dk_set_gpio(baseaddr, PORT_DOUT, x)


void spiInit(unsigned int baseaddr) {
// set direction of ports - all out
    dk_set_gpio_output(baseaddr, PORT_DOUT);
    dk_set_gpio_output(baseaddr, PORT_CLK);
    dk_set_gpio_output(baseaddr, PORT_CS1);
    dk_set_gpio_output(baseaddr, PORT_CS2);
    DATA_OUT(1);
    CLR_CLOCK;
    dk_set_gpio(baseaddr, PORT_CS1, 1); // set CS high
    dk_set_gpio(baseaddr, PORT_CS2, 1); // set CS high
}

//
//  Output one byte
//
static void spiOutOne(unsigned int baseaddr, unsigned char data) {
    u32 i;

    for (i = 0; i < 8; i++ ) {
#ifdef ADF4350SPILSB
        DATA_OUT(data & 1);
        udelay(1);
        data >>= 1;         
        SET_CLOCK;
        udelay(adfParams.delay);
        CLR_CLOCK;
#else
        DATA_OUT((data & 0x80)>>7);
        udelay(1);
        data <<= 1;         
        SET_CLOCK;
        udelay(adfParams.delay);
        CLR_CLOCK;
#endif
    }
}

void spiOut32bits(unsigned int baseaddr, unsigned int data)
{
    udelay(1);
#ifdef ADF4350SPILSB
    spiOutOne(baseaddr, data & 0xff);
    spiOutOne(baseaddr, (data >> 8) & 0xff);
    spiOutOne(baseaddr, (data >> 16) & 0xff);
    spiOutOne(baseaddr, (data >> 24) & 0xff);
#else
    spiOutOne(baseaddr, (data >> 24) & 0xff);
    spiOutOne(baseaddr, (data >> 16) & 0xff);
    spiOutOne(baseaddr, (data >> 8) & 0xff);
    spiOutOne(baseaddr, data & 0xff);
#endif
    udelay(1);
}

void spiOutWord(unsigned int cs, unsigned int baseaddr, unsigned int data) {
    printk(KERN_INFO "spiOutWord: write 0x%x=%d to CS%d\n", data, data, cs+1); 
    if(cs) {
        dk_set_gpio(baseaddr, PORT_CS2, 0);     // assert CS2
        spiOut32bits(baseaddr, data);   
        dk_set_gpio(baseaddr, PORT_CS2, 1);     // de-assert CS2    
    } else {
        dk_set_gpio(baseaddr, PORT_CS1, 0);     // assert CS1
        spiOut32bits(baseaddr, data);
        dk_set_gpio(baseaddr, PORT_CS1, 1);     // de-assert CS1
    }
    udelay(1);
    DATA_OUT(1);    
}
