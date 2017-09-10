//
//  SPI emuilation for AR5414
//
//  Copyright SG Microhard Systems Inc
//
//  2011.06.14 - created
//
#include "ath5k.h"
#include "reg.h"
#include "base.h"
#include "adf4350.h"

extern int ath5k_hw_set_gpio(struct ath5k_hw *ah, u32 gpio, u32 val);
extern int ath5k_hw_set_gpio_output(struct ath5k_hw *ah, u32 gpio);

#define SET_CLOCK  ath5k_hw_set_gpio(ah, PORT_CLK, 1)
#define CLR_CLOCK  ath5k_hw_set_gpio(ah, PORT_CLK, 0)
#define DATA_OUT(x) ath5k_hw_set_gpio(ah, PORT_DOUT, x)


void spiInit(struct ath5k_hw *ah) {
// set direction of ports - all out
    ath5k_hw_set_gpio_output(ah, PORT_DOUT);
    ath5k_hw_set_gpio_output(ah, PORT_CLK);
    ath5k_hw_set_gpio_output(ah, PORT_CS1);
    ath5k_hw_set_gpio_output(ah, PORT_CS2);
    DATA_OUT(1);
    CLR_CLOCK;
    ath5k_hw_set_gpio(ah, PORT_CS1, 1); // set CS high
    ath5k_hw_set_gpio(ah, PORT_CS2, 1); // set CS high
}

//
//  Output one byte
//
static void spiOutOne(struct ath5k_hw *ah, u8 data) {
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

void spiOut32bits(struct ath5k_hw *ah, u32 data)
{
    udelay(1);
#ifdef ADF4350SPILSB
    spiOutOne(ah, data & 0xff);
    spiOutOne(ah, (data >> 8) & 0xff);
    spiOutOne(ah, (data >> 16) & 0xff);
    spiOutOne(ah, (data >> 24) & 0xff);
#else
    spiOutOne(ah, (data >> 24) & 0xff);
    spiOutOne(ah, (data >> 16) & 0xff);
    spiOutOne(ah, (data >> 8) & 0xff);
    spiOutOne(ah, data & 0xff);
#endif
    udelay(1);
}

void spiOutWord(u32 cs, struct ath5k_hw *ah, u32 data) {
    //printk(KERN_INFO "spiOutWord: write 0x%x=%d to CS%d\n", data, data, cs+1); 
    if(cs) {
        ath5k_hw_set_gpio(ah, PORT_CS2, 0);     // assert CS2
        spiOut32bits(ah, data);   
        ath5k_hw_set_gpio(ah, PORT_CS2, 1);     // de-assert CS2    
    } else {
        ath5k_hw_set_gpio(ah, PORT_CS1, 0);     // assert CS1
        spiOut32bits(ah, data);
        ath5k_hw_set_gpio(ah, PORT_CS1, 1);     // de-assert CS1
    }
    udelay(1);
    DATA_OUT(1);    
}
