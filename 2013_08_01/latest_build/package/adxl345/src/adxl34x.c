/*
 * ADXL345/346 Three-Axis Digital Accelerometers
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Copyright (C) 2009 Michael Hennerich, Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/input/adxl34x.h>

#include <asm/irq_cpu.h>
#include <asm/mipsregs.h>

#include <asm/mach-ar71xx/ar71xx.h>
#include <linux/gpio.h>
#include <linux/timer.h>


#include "adxl34x.h"

#define adxl345_debug	1

#ifdef adxl345_debug
#	define ADXL34x_DEBUG(fmt, args...) printk( KERN_ALERT fmt, ## args)
#else
#  	define ADXL34x_DEBUG(fmt, args...) /* not debugging: nothing */
#endif

/* ADXL345/6 Register Map */
#define DEVID		0x00	/* R   Device ID */
#define THRESH_TAP	0x1D	/* R/W Tap threshold */
#define OFSX		0x1E	/* R/W X-axis offset */
#define OFSY		0x1F	/* R/W Y-axis offset */
#define OFSZ		0x20	/* R/W Z-axis offset */
#define DUR		0x21	/* R/W Tap duration */
#define LATENT		0x22	/* R/W Tap latency */
#define WINDOW		0x23	/* R/W Tap window */
#define THRESH_ACT	0x24	/* R/W Activity threshold */
#define THRESH_INACT	0x25	/* R/W Inactivity threshold */
#define TIME_INACT	0x26	/* R/W Inactivity time */
#define ACT_INACT_CTL	0x27	/* R/W Axis enable control for activity and */
				/* inactivity detection */
#define THRESH_FF	0x28	/* R/W Free-fall threshold */
#define TIME_FF		0x29	/* R/W Free-fall time */
#define TAP_AXES	0x2A	/* R/W Axis control for tap/double tap */
#define ACT_TAP_STATUS	0x2B	/* R   Source of tap/double tap */
#define BW_RATE		0x2C	/* R/W Data rate and power mode control */
#define POWER_CTL	0x2D	/* R/W Power saving features control */
#define INT_ENABLE	0x2E	/* R/W Interrupt enable control */
#define INT_MAP		0x2F	/* R/W Interrupt mapping control */
#define INT_SOURCE	0x30	/* R   Source of interrupts */
#define DATA_FORMAT	0x31	/* R/W Data format control */
#define DATAX0		0x32	/* R   X-Axis Data 0 */
#define DATAX1		0x33	/* R   X-Axis Data 1 */
#define DATAY0		0x34	/* R   Y-Axis Data 0 */
#define DATAY1		0x35	/* R   Y-Axis Data 1 */
#define DATAZ0		0x36	/* R   Z-Axis Data 0 */
#define DATAZ1		0x37	/* R   Z-Axis Data 1 */
#define FIFO_CTL	0x38	/* R/W FIFO control */
#define FIFO_STATUS	0x39	/* R   FIFO status */
#define TAP_SIGN	0x3A	/* R   Sign and source for tap/double tap */
/* Orientation ADXL346 only */
#define ORIENT_CONF	0x3B	/* R/W Orientation configuration */
#define ORIENT		0x3C	/* R   Orientation status */

/* DEVIDs */
#define ID_ADXL345	0xE5
#define ID_ADXL346	0xE6

/* INT_ENABLE/INT_MAP/INT_SOURCE Bits */
#define DATA_READY	(1 << 7)
#define SINGLE_TAP	(1 << 6)
#define DOUBLE_TAP	(1 << 5)
#define ACTIVITY	(1 << 4)
#define INACTIVITY	(1 << 3)
#define FREE_FALL	(1 << 2)
#define WATERMARK	(1 << 1)
#define OVERRUN		(1 << 0)

/* ACT_INACT_CONTROL Bits */
#define ACT_ACDC	(1 << 7)
#define ACT_X_EN	(1 << 6)
#define ACT_Y_EN	(1 << 5)
#define ACT_Z_EN	(1 << 4)
#define INACT_ACDC	(1 << 3)
#define INACT_X_EN	(1 << 2)
#define INACT_Y_EN	(1 << 1)
#define INACT_Z_EN	(1 << 0)

/* TAP_AXES Bits */
#define SUPPRESS	(1 << 3)
#define TAP_X_EN	(1 << 2)
#define TAP_Y_EN	(1 << 1)
#define TAP_Z_EN	(1 << 0)

/* ACT_TAP_STATUS Bits */
#define ACT_X_SRC	(1 << 6)
#define ACT_Y_SRC	(1 << 5)
#define ACT_Z_SRC	(1 << 4)
#define ASLEEP		(1 << 3)
#define TAP_X_SRC	(1 << 2)
#define TAP_Y_SRC	(1 << 1)
#define TAP_Z_SRC	(1 << 0)

/* BW_RATE Bits */
#define LOW_POWER	(1 << 4)
#define RATE(x)		((x) & 0xF)

/* POWER_CTL Bits */
#define PCTL_LINK	(1 << 5)
#define PCTL_AUTO_SLEEP (1 << 4)
#define PCTL_MEASURE	(1 << 3)
#define PCTL_SLEEP	(1 << 2)
#define PCTL_WAKEUP(x)	((x) & 0x3)

/* DATA_FORMAT Bits */
#define SELF_TEST	(1 << 7)
#define SPI		(1 << 6)
#define INT_INVERT	(1 << 5)
#define FULL_RES	(1 << 3)
#define JUSTIFY		(1 << 2)
#define RANGE(x)	((x) & 0x3)
#define RANGE_PM_2g	0
#define RANGE_PM_4g	1
#define RANGE_PM_8g	2
#define RANGE_PM_16g	3

/*
 * Maximum value our axis may get in full res mode for the input device
 * (signed 13 bits)
 */
#define ADXL_FULLRES_MAX_VAL 4096

/*
 * Maximum value our axis may get in fixed res mode for the input device
 * (signed 10 bits)
 */
#define ADXL_FIXEDRES_MAX_VAL 512

/* FIFO_CTL Bits */
#define FIFO_MODE(x)	(((x) & 0x3) << 6)
#define FIFO_BYPASS	0
#define FIFO_FIFO	1
#define FIFO_STREAM	2
#define FIFO_TRIGGER	3
#define TRIGGER		(1 << 5)
#define SAMPLES(x)	((x) & 0x1F)

/* FIFO_STATUS Bits */
#define FIFO_TRIG	(1 << 7)
#define ENTRIES(x)	((x) & 0x3F)

/* TAP_SIGN Bits ADXL346 only */
#define XSIGN		(1 << 6)
#define YSIGN		(1 << 5)
#define ZSIGN		(1 << 4)
#define XTAP		(1 << 3)
#define YTAP		(1 << 2)
#define ZTAP		(1 << 1)

/* ORIENT_CONF ADXL346 only */
#define ORIENT_DEADZONE(x)	(((x) & 0x7) << 4)
#define ORIENT_DIVISOR(x)	((x) & 0x7)

/* ORIENT ADXL346 only */
#define ADXL346_2D_VALID		(1 << 6)
#define ADXL346_2D_ORIENT(x)		(((x) & 0x3) >> 4)
#define ADXL346_3D_VALID		(1 << 3)
#define ADXL346_3D_ORIENT(x)		((x) & 0x7)
#define ADXL346_2D_PORTRAIT_POS		0	/* +X */
#define ADXL346_2D_PORTRAIT_NEG		1	/* -X */
#define ADXL346_2D_LANDSCAPE_POS	2	/* +Y */
#define ADXL346_2D_LANDSCAPE_NEG	3	/* -Y */

#define ADXL346_3D_FRONT		3	/* +X */
#define ADXL346_3D_BACK			4	/* -X */
#define ADXL346_3D_RIGHT		2	/* +Y */
#define ADXL346_3D_LEFT			5	/* -Y */
#define ADXL346_3D_TOP			1	/* +Z */
#define ADXL346_3D_BOTTOM		6	/* -Z */

#undef ADXL_DEBUG   
#define ADXL_DEBUG   1

#define ADXL_X_AXIS			0
#define ADXL_Y_AXIS			1
#define ADXL_Z_AXIS			2

#define AC_READ(ac, reg)	((ac)->bops->read((ac)->dev, reg))
#define AC_WRITE(ac, reg, val)	((ac)->bops->write((ac)->dev, reg, val))

struct axis_triple {
	int x;
	int y;
	int z;
};


#define MICROHARD_ADXL  1

struct adxl34x {
	struct device *dev;
	struct input_dev *input;
    struct work_struct work;
	struct mutex mutex;	/* reentrant protection for struct */
	struct adxl34x_platform_data pdata;
	struct axis_triple swcal;
	struct axis_triple hwcal;
	struct axis_triple saved;
	char phys[32];
	unsigned orient2d_saved;
	unsigned orient3d_saved;
	bool disabled;	/* P: mutex */
	bool opened;	/* P: mutex */
	bool suspended;	/* P: mutex */
	bool fifo_delay;
	int irq;
	unsigned model;
	unsigned int_mask;

	const struct adxl34x_bus_ops *bops;
#ifdef MICROHARD_ADXL
    struct timer_list adxl34x_timer;
#endif
};

static const struct adxl34x_platform_data adxl34x_default_init = {
	.tap_threshold = 35,
	.tap_duration = 3,
	.tap_latency = 20,
	.tap_window = 20,
	.tap_axis_control = ADXL_TAP_X_EN | ADXL_TAP_Y_EN | ADXL_TAP_Z_EN,
	.act_axis_control = 0xFF,
	.activity_threshold = 6,
	.inactivity_threshold = 4,
	.inactivity_time = 3,
	.free_fall_threshold = 8,
	.free_fall_time = 0x20,
	.data_rate = 8,
	.data_range = ADXL_FULL_RES,    

	.ev_type = EV_ABS,
	.ev_code_x = ABS_X,	/* EV_REL */
	.ev_code_y = ABS_Y,	/* EV_REL */
	.ev_code_z = ABS_Z,	/* EV_REL */

	.ev_code_tap = {BTN_TOUCH, BTN_TOUCH, BTN_TOUCH}, /* EV_KEY {x,y,z} */
    .ev_code_act_inactivity = KEY_A,	/* EV_KEY */
	.power_mode = ADXL_AUTO_SLEEP | ADXL_LINK,
	.fifo_mode = FIFO_FIFO,
	.watermark = 24,
};

static void adxl34x_get_triple(struct adxl34x *ac, struct axis_triple *axis)
{
	short buf[3];

	ac->bops->read_block(ac->dev, DATAX0, DATAZ1 - DATAX0 + 1, buf);
#if 0
    /* So far, VIP4g don't need these position data */
	mutex_lock(&ac->mutex);
	ac->saved.x = (s16) le16_to_cpu(buf[0]);
	axis->x = ac->saved.x;

	ac->saved.y = (s16) le16_to_cpu(buf[1]);
	axis->y = ac->saved.y;

	ac->saved.z = (s16) le16_to_cpu(buf[2]);
	axis->z = ac->saved.z;
	mutex_unlock(&ac->mutex);
#endif
}

static void adxl34x_service_ev_fifo(struct adxl34x *ac)
{
#ifndef MICROHARD_ADXL
	struct adxl34x_platform_data *pdata = &ac->pdata;
#endif
	struct axis_triple axis;
//    printk(KERN_ALERT "adxl34x_service_ev_fifo\n");

	adxl34x_get_triple(ac, &axis);
#if 0
	/* So far, VIP4g don't need these position data */
	input_event(ac->input, pdata->ev_type, pdata->ev_code_x,
		    axis.x - ac->swcal.x);
	input_event(ac->input, pdata->ev_type, pdata->ev_code_y,
		    axis.y - ac->swcal.y);
	input_event(ac->input, pdata->ev_type, pdata->ev_code_z,
		    axis.z - ac->swcal.z);
#endif
}

static void adxl34x_report_key_single(struct input_dev *input, int key)
{
	input_report_key(input, key, true);
	input_sync(input);
	input_report_key(input, key, false);
}

static void adxl34x_send_key_events(struct adxl34x *ac,
		struct adxl34x_platform_data *pdata, int status, int press)
{
	int i;

	for (i = ADXL_X_AXIS; i <= ADXL_Z_AXIS; i++) {
		if (status & (1 << (ADXL_Z_AXIS - i)))
			input_report_key(ac->input,
					 pdata->ev_code_tap[i], press);
	}
}

static void adxl34x_do_tap(struct adxl34x *ac,
		struct adxl34x_platform_data *pdata, int status)
{
	adxl34x_send_key_events(ac, pdata, status, true);
//	input_sync(ac->input);
	adxl34x_send_key_events(ac, pdata, status, false);
}




static void adxl34x_work(struct work_struct *work)
{
	struct adxl34x *ac = container_of(work, struct adxl34x, work);

	struct adxl34x_platform_data *pdata = &ac->pdata;
	int int_stat, tap_stat, samples; 

    static int irq_flag = 0;
    //__ar71xx_gpio_set_value(1, 1);

    if (irq_flag == 1) {
        return IRQ_HANDLED;
    }

    irq_flag = 1;

irq_start:
    
    if (pdata->tap_axis_control & (TAP_X_EN | TAP_Y_EN | TAP_Z_EN))
    	tap_stat = AC_READ(ac, ACT_TAP_STATUS);
    else
    	tap_stat = 0;
    
    int_stat = AC_READ(ac, INT_SOURCE);
    if ( int_stat != 0 ) {
#if 1
    	if (int_stat & FREE_FALL){
    		adxl34x_report_key_single(ac->input, pdata->ev_code_ff);
        }
    
    	if (int_stat & OVERRUN){
    		dev_dbg(ac->dev, "OVERRUN\n");
        }
    
    	if (int_stat & (SINGLE_TAP | DOUBLE_TAP)) {
    		adxl34x_do_tap(ac, pdata, tap_stat);
    
    		if (int_stat & DOUBLE_TAP){
    			adxl34x_do_tap(ac, pdata, tap_stat);
            }
                
    	}
    
    	if (pdata->ev_code_act_inactivity) {
    		if (int_stat & ACTIVITY){
    			input_report_key(ac->input,
    					 pdata->ev_code_act_inactivity, 1);
    		}
    		if (int_stat & INACTIVITY){
    			input_report_key(ac->input,
    					 pdata->ev_code_act_inactivity, 0);
    		}
    	}    
#endif
    
    	if (int_stat & (DATA_READY | WATERMARK)) {
    
    		if (pdata->fifo_mode)
    			samples = ENTRIES(AC_READ(ac, FIFO_STATUS)) + 1;
    		else
    			samples = 1;
           
    		for (; samples > 0; samples--) {            
    			adxl34x_service_ev_fifo(ac);                
    			udelay(5);
    		}           
    	}    
    
//    	input_sync(ac->input);
    
        goto irq_start;
    } //end of if

	int_stat = AC_READ(ac, INT_SOURCE);

    irq_flag = 0;
    enable_irq(ac->irq);
    
    //__ar71xx_gpio_set_value(1, 0);    
    
	
}


static irqreturn_t adxl34x_irq_wq(int irq, void *handle)
{
	struct adxl34x *ac = handle;

	disable_irq_nosync(irq);
    
	schedule_work(&ac->work);

	return IRQ_HANDLED;
}



static irqreturn_t adxl34x_irq(int irq, void *handle)
{
	struct adxl34x *ac = handle;
	struct adxl34x_platform_data *pdata = &ac->pdata;
	int int_stat, tap_stat, samples, orient, orient_code;     

    static int irq_flag = 0;

    if (irq_flag == 1) {
        return IRQ_HANDLED;
    }

    irq_flag = 1;  

irq_start:
	/*
	 * ACT_TAP_STATUS should be read before clearing the interrupt
	 * Avoid reading ACT_TAP_STATUS in case TAP detection is disabled
	 */    
   	if (pdata->tap_axis_control & (TAP_X_EN | TAP_Y_EN | TAP_Z_EN))
   		tap_stat = AC_READ(ac, ACT_TAP_STATUS);
   	else
   		tap_stat = 0;
    

    int_stat = AC_READ(ac, INT_SOURCE);
    if ( int_stat != 0 ) { 
    
    	if (int_stat & FREE_FALL){
    		adxl34x_report_key_single(ac->input, pdata->ev_code_ff);
        }
    
    	if (int_stat & OVERRUN){
    		dev_dbg(ac->dev, "OVERRUN\n");
        }
    
    	if (int_stat & (SINGLE_TAP | DOUBLE_TAP)) {
    		adxl34x_do_tap(ac, pdata, tap_stat);
    
    		if (int_stat & DOUBLE_TAP){
    			adxl34x_do_tap(ac, pdata, tap_stat);
            }
                
    	}
    
    	if (pdata->ev_code_act_inactivity) {
    		if (int_stat & ACTIVITY){
    			input_report_key(ac->input,
    					 pdata->ev_code_act_inactivity, 1);
    		}
    		if (int_stat & INACTIVITY){
    			input_report_key(ac->input,
    					 pdata->ev_code_act_inactivity, 0);
    		}
    	}
    
    	/*
    	 * ORIENTATION SENSING ADXL346 only
    	 */
    	if (pdata->orientation_enable) {
    		orient = AC_READ(ac, ORIENT);
    		if ((pdata->orientation_enable & ADXL_EN_ORIENTATION_2D) &&
    		    (orient & ADXL346_2D_VALID)) {
    
    			orient_code = ADXL346_2D_ORIENT(orient);
    			/* Report orientation only when it changes */
    			if (ac->orient2d_saved != orient_code) {
    				ac->orient2d_saved = orient_code;
    				adxl34x_report_key_single(ac->input,
    					pdata->ev_codes_orient_2d[orient_code]);
    			}
    		}
    
    		if ((pdata->orientation_enable & ADXL_EN_ORIENTATION_3D) &&
    		    (orient & ADXL346_3D_VALID)) {
    
    			orient_code = ADXL346_3D_ORIENT(orient) - 1;
    			/* Report orientation only when it changes */
    			if (ac->orient3d_saved != orient_code) {
    				ac->orient3d_saved = orient_code;
    				adxl34x_report_key_single(ac->input,
    					pdata->ev_codes_orient_3d[orient_code]);
    			}
    		}
    	}
    
    	if (int_stat & (DATA_READY | WATERMARK)) {
    
    		if (pdata->fifo_mode)
    			samples = ENTRIES(AC_READ(ac, FIFO_STATUS)) + 1;
    		else
    			samples = 1;
            
    		for (; samples > 0; samples--) {
    			adxl34x_service_ev_fifo(ac);
    			/*
    			 * To ensure that the FIFO has
    			 * completely popped, there must be at least 5 us between
    			 * the end of reading the data registers, signified by the
    			 * transition to register 0x38 from 0x37 or the CS pin
    			 * going high, and the start of new reads of the FIFO or
    			 * reading the FIFO_STATUS register. For SPI operation at
    			 * 1.5 MHz or lower, the register addressing portion of the
    			 * transmission is sufficient delay to ensure the FIFO has
    			 * completely popped. It is necessary for SPI operation
    			 * greater than 1.5 MHz to de-assert the CS pin to ensure a
    			 * total of 5 us, which is at most 3.4 us at 5 MHz
    			 * operation.
    			 */
    			//if (ac->fifo_delay && (samples > 1))
    			//	udelay(3);
                if (samples > 1)
    				udelay(3);
    		}
           
    	}    
    
//    	input_sync(ac->input);
    
        goto irq_start; // make sure INT_SOURCE clear completely
    } //end of if

    irq_flag = 0;
    
	return IRQ_HANDLED;
}

#ifdef MICROHARD_ADXL
void adxl34x_timer_callback( unsigned long data )
{
      struct adxl34x *ac = (struct adxl34x *) data;
      int gpio6_value = __ar71xx_gpio_get_value(6);
  
      if (gpio6_value == 1) {
          //printk(KERN_ALERT "adxl34x_timer_callback get gpio 6 value: %d\n", gpio6_value);
          AC_READ(ac, INT_SOURCE); //clear interrupt source
      }
      mod_timer( &ac->adxl34x_timer, jiffies + msecs_to_jiffies(3000) );
}
#endif

static void __adxl34x_disable(struct adxl34x *ac)
{
#ifdef MICROHARD_ADXL
    del_timer( &ac->adxl34x_timer);    
#endif
	/*
	 * A '0' places the ADXL34x into standby mode
	 * with minimum power consumption.
	 */
	AC_WRITE(ac, POWER_CTL, 0);
}

static void __adxl34x_enable(struct adxl34x *ac)
{
#ifdef MICROHARD_ADXL
    // Starting timer to fire in 3000ms 
    mod_timer( &ac->adxl34x_timer, jiffies + msecs_to_jiffies(3000) );    
#endif

	AC_WRITE(ac, POWER_CTL, ac->pdata.power_mode | PCTL_MEASURE);
}

void adxl34x_suspend(struct adxl34x *ac)
{
	mutex_lock(&ac->mutex);

	if (!ac->suspended && !ac->disabled && ac->opened)
		__adxl34x_disable(ac);

	ac->suspended = true;

	mutex_unlock(&ac->mutex);
}
EXPORT_SYMBOL_GPL(adxl34x_suspend);

void adxl34x_resume(struct adxl34x *ac)
{
	mutex_lock(&ac->mutex);

	if (ac->suspended && !ac->disabled && ac->opened)
		__adxl34x_enable(ac);

	ac->suspended = false;

	mutex_unlock(&ac->mutex);
}
EXPORT_SYMBOL_GPL(adxl34x_resume);

static ssize_t adxl34x_disable_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct adxl34x *ac = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", ac->disabled);
}

static ssize_t adxl34x_disable_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct adxl34x *ac = dev_get_drvdata(dev);
	unsigned long val;
	int error;

	error = strict_strtoul(buf, 10, &val);
	if (error)
		return error;

	mutex_lock(&ac->mutex);

	if (!ac->suspended && ac->opened) {
		if (val) {
			if (!ac->disabled)
				__adxl34x_disable(ac);
		} else {
			if (ac->disabled)
				__adxl34x_enable(ac);
		}
	}

	ac->disabled = !!val;

	mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(disable, 0664, adxl34x_disable_show, adxl34x_disable_store);

static ssize_t adxl34x_calibrate_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct adxl34x *ac = dev_get_drvdata(dev);
	ssize_t count;

	mutex_lock(&ac->mutex);
	count = sprintf(buf, "%d,%d,%d\n",
			ac->hwcal.x * 4 + ac->swcal.x,
			ac->hwcal.y * 4 + ac->swcal.y,
			ac->hwcal.z * 4 + ac->swcal.z);
	mutex_unlock(&ac->mutex);

	return count;
}

static ssize_t adxl34x_calibrate_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	struct adxl34x *ac = dev_get_drvdata(dev);

	/*
	 * Hardware offset calibration has a resolution of 15.6 mg/LSB.
	 * We use HW calibration and handle the remaining bits in SW. (4mg/LSB)
	 */

	mutex_lock(&ac->mutex);
	ac->hwcal.x -= (ac->saved.x / 4);
	ac->swcal.x = ac->saved.x % 4;

	ac->hwcal.y -= (ac->saved.y / 4);
	ac->swcal.y = ac->saved.y % 4;

	ac->hwcal.z -= (ac->saved.z / 4);
	ac->swcal.z = ac->saved.z % 4;

	AC_WRITE(ac, OFSX, (s8) ac->hwcal.x);
	AC_WRITE(ac, OFSY, (s8) ac->hwcal.y);
	AC_WRITE(ac, OFSZ, (s8) ac->hwcal.z);
	mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(calibrate, 0664,
		   adxl34x_calibrate_show, adxl34x_calibrate_store);

static ssize_t adxl34x_rate_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct adxl34x *ac = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", RATE(ac->pdata.data_rate));
}

static ssize_t adxl34x_rate_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct adxl34x *ac = dev_get_drvdata(dev);
	unsigned long val;
	int error;

	error = strict_strtoul(buf, 10, &val);
	if (error)
		return error;

	mutex_lock(&ac->mutex);

	ac->pdata.data_rate = RATE(val);
	AC_WRITE(ac, BW_RATE,
		 ac->pdata.data_rate |
			(ac->pdata.low_power_mode ? LOW_POWER : 0));

	mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(rate, 0664, adxl34x_rate_show, adxl34x_rate_store);

static ssize_t adxl34x_autosleep_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct adxl34x *ac = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n",
		ac->pdata.power_mode & (PCTL_AUTO_SLEEP | PCTL_LINK) ? 1 : 0);
}

static ssize_t adxl34x_autosleep_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct adxl34x *ac = dev_get_drvdata(dev);
	unsigned long val;
	int error;

	error = strict_strtoul(buf, 10, &val);
	if (error)
		return error;

	mutex_lock(&ac->mutex);

	if (val)
		ac->pdata.power_mode |= (PCTL_AUTO_SLEEP | PCTL_LINK);
	else
		ac->pdata.power_mode &= ~(PCTL_AUTO_SLEEP | PCTL_LINK);

	if (!ac->disabled && !ac->suspended && ac->opened)
		AC_WRITE(ac, POWER_CTL, ac->pdata.power_mode | PCTL_MEASURE);

	mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(autosleep, 0664,
		   adxl34x_autosleep_show, adxl34x_autosleep_store);

static ssize_t adxl34x_position_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct adxl34x *ac = dev_get_drvdata(dev);
	ssize_t count;

    struct axis_triple axis;
	adxl34x_get_triple(ac, &axis);

	mutex_lock(&ac->mutex);
	count = sprintf(buf, "(%d, %d, %d)\n",
			ac->saved.x, ac->saved.y, ac->saved.z);
	mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(position, S_IRUGO, adxl34x_position_show, NULL);

#ifdef ADXL_DEBUG
static ssize_t adxl34x_write_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct adxl34x *ac = dev_get_drvdata(dev);
	unsigned long val;
	int error;

	/*
	 * This allows basic ADXL register write access for debug purposes.
	 */
	error = strict_strtoul(buf, 16, &val);
	if (error)
		return error;

    //printk(KERN_ALERT "Write reg:0X%x val:0x%x\n", val >> 8, val & 0xFF);
    mutex_lock(&ac->mutex);
    AC_WRITE(ac, val >> 8, val & 0xFF);
    mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(write, 0664, NULL, adxl34x_write_store);
#endif


#ifdef ADXL_DEBUG
static ssize_t adxl34x_read_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct adxl34x *ac = dev_get_drvdata(dev);
	ssize_t count;

    int val_DEVID		     = AC_READ(ac, 0x00);   	/* R   Device ID */
    int val_THRESH_TAP	     = AC_READ(ac, 0x1D);   	/* R/W Tap threshold */
    int val_OFSX		     = AC_READ(ac, 0x1E);   	/* R/W X-axis offset */
    int val_OFSY		     = AC_READ(ac, 0x1F);   	/* R/W Y-axis offset */
    int val_OFSZ		     = AC_READ(ac, 0x20);   	/* R/W Z-axis offset */
    int val_DUR		         = AC_READ(ac, 0x21);   	/* R/W Tap duration */
    int val_LATENT		     = AC_READ(ac, 0x22);   	/* R/W Tap latency */
    int val_WINDOW		     = AC_READ(ac, 0x23);   	/* R/W Tap window */
    int val_THRESH_ACT	     = AC_READ(ac, 0x24);   	/* R/W Activity threshold */
    int val_THRESH_INACT	 = AC_READ(ac, 0x25);   	/* R/W Inactivity threshold */
    int val_TIME_INACT	     = AC_READ(ac, 0x26);   	/* R/W Inactivity time */
    int val_ACT_INACT_CTL	 = AC_READ(ac, 0x27);   	/* R/W Axis enable control for activity and inactivity detection */
    int val_THRESH_FF	     = AC_READ(ac, 0x28);   	/* R/W Free-fall threshold */
    int val_TIME_FF		     = AC_READ(ac, 0x29);   	/* R/W Free-fall time */
    int val_TAP_AXES	     = AC_READ(ac, 0x2A);   	/* R/W Axis control for tap/double tap */
    int val_ACT_TAP_STATUS	 = AC_READ(ac, 0x2B);   	/* R   Source of tap/double tap */
    int val_BW_RATE		     = AC_READ(ac, 0x2C);   	/* R/W Data rate and power mode control */
    int val_POWER_CTL	     = AC_READ(ac, 0x2D);   	/* R/W Power saving features control */
    int val_INT_ENABLE	     = AC_READ(ac, 0x2E);   	/* R/W Interrupt enable control */
    int val_INT_MAP		     = AC_READ(ac, 0x2F);   	/* R/W Interrupt mapping control */
    int val_INT_SOURCE	     = AC_READ(ac, 0x30);   	/* R   Source of interrupts */
    int val_DATA_FORMAT	     = AC_READ(ac, 0x31);   	/* R/W Data format control */
    int val_FIFO_CTL	     = AC_READ(ac, 0x38);   	/* R/W FIFO control */
    int val_FIFO_STATUS	     = AC_READ(ac, 0x39);   	/* R   FIFO status */

	mutex_lock(&ac->mutex);

	count = sprintf(buf,    
"DEVID          :%02x\n\
THRESH_TAP     :%02x\n\
OFSX           :%02x\n\
OFSY           :%02x\n\
OFSZ           :%02x\n\
DUR            :%02x\n\
LATENT         :%02x\n\
WINDOW         :%02x\n\
THRESH_ACT     :%02x\n\
THRESH_INACT   :%02x\n\
TIME_INACT     :%02x\n\
ACT_INACT_CTL  :%02x\n\
THRESH_FF      :%02x\n\
TIME_FF        :%02x\n\
TAP_AXES       :%02x\n\
ACT_TAP_STATUS :%02x\n\
BW_RATE        :%02x\n\
POWER_CTL      :%02x\n\
INT_ENABLE     :%02x\n\
INT_MAP        :%02x\n\
INT_SOURCE     :%02x\n\
DATA_FORMAT    :%02x\n\
FIFO_CTL       :%02x\n\
FIFO_STATUS    :%02x\n",
                    val_DEVID, val_THRESH_TAP, val_OFSX, val_OFSY, val_OFSZ, val_DUR, val_LATENT, val_WINDOW, val_THRESH_ACT,  
                    val_THRESH_INACT,  val_TIME_INACT, val_ACT_INACT_CTL, val_THRESH_FF, val_TIME_FF, 
                    val_TAP_AXES, val_ACT_TAP_STATUS, val_BW_RATE, val_POWER_CTL,  val_INT_ENABLE, val_INT_MAP, val_INT_SOURCE, 
                    val_DATA_FORMAT,  val_FIFO_CTL, val_FIFO_STATUS);

	mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(read, S_IRUGO, adxl34x_read_show, NULL);
#endif


#ifdef ADXL_DEBUG
static ssize_t adxl34x_gpio6interrupt_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct adxl34x *ac = dev_get_drvdata(dev);
	unsigned long val;
	int error;
    void __iomem *base = ar71xx_gpio_base;
	/*
	 * This allows basic ADXL register write access for debug purposes.
	 */
	error = strict_strtoul(buf, 16, &val);
	if (error)
		return error;

    if (val)
		val = 0x40;
	else
		val = 0x0;

    //printk(KERN_ALERT "Write GPIO_REG_INT_MODE val:0x%x\n", val);
    mutex_lock(&ac->mutex);    
    /* enable/disable GPIO 6 interrupts */
    __raw_writel(val, base + GPIO_REG_INT_MODE);
    mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(gpio6interrupt, 0664, NULL, adxl34x_gpio6interrupt_store);
#endif

static struct attribute *adxl34x_attributes[] = {
	&dev_attr_disable.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_rate.attr,
	&dev_attr_autosleep.attr,
	&dev_attr_position.attr,
#ifdef ADXL_DEBUG
	&dev_attr_write.attr,
    &dev_attr_read.attr,
    &dev_attr_gpio6interrupt.attr,
#endif
	NULL
};

static const struct attribute_group adxl34x_attr_group = {
	.attrs = adxl34x_attributes,
};

static int adxl34x_input_open(struct input_dev *input)
{
	struct adxl34x *ac = input_get_drvdata(input);

	mutex_lock(&ac->mutex);

	if (!ac->suspended && !ac->disabled)
		__adxl34x_enable(ac);

	ac->opened = true;

	mutex_unlock(&ac->mutex);

	return 0;
}

static void adxl34x_input_close(struct input_dev *input)
{
	struct adxl34x *ac = input_get_drvdata(input);

	mutex_lock(&ac->mutex);

	if (!ac->suspended && !ac->disabled)
		__adxl34x_disable(ac);

	ac->opened = false;

	mutex_unlock(&ac->mutex);
}

struct adxl34x *adxl34x_probe(struct device *dev, int irq,
			      bool fifo_delay_default,
			      const struct adxl34x_bus_ops *bops)
{
	struct adxl34x *ac;
	struct input_dev *input_dev;
	const struct adxl34x_platform_data *pdata;
	int err, range, i;
	unsigned char revid;
	void __iomem *base = ar71xx_gpio_base;
#ifdef MICROHARD_ADXL
    /* set GPIO 6 interrupt for adxl34x chip on Microhard VIP4G board*/
    u32 gpio_mode;
    u32 gpio_type;
    u32 gpio_polarity;
    //u32 gpio_status;
    u32 gpio_mask;
#endif        

	if (!irq) {
		dev_err(dev, "no IRQ?\n");
		err = -ENODEV;
		goto err_out;
	}

	ac = kzalloc(sizeof(*ac), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!ac || !input_dev) {
		err = -ENOMEM;
		goto err_free_mem;
	}

	ac->fifo_delay = fifo_delay_default;

	pdata = dev->platform_data; // use initialization code in platform data: mach-VIP4G.c
    pdata = NULL; // force to use default initialization code in driver
	if (!pdata) {
		dev_dbg(dev,
			"No platfrom data: Using default initialization\n");
		pdata = &adxl34x_default_init;
	}

	ac->pdata = *pdata;
	pdata = &ac->pdata;

	ac->input = input_dev;
	ac->dev = dev;
	ac->irq = irq;
	ac->bops = bops;

    INIT_WORK(&ac->work, adxl34x_work);
	mutex_init(&ac->mutex);

	input_dev->name = "ADXL34x accelerometer";
	revid = ac->bops->read(dev, DEVID);

	switch (revid) {
	case ID_ADXL345:
		ac->model = 345;
		break;
	case ID_ADXL346:
		ac->model = 346;
		break;
	default:
		dev_err(dev, "Failed to probe %s\n", input_dev->name);
		err = -ENODEV;
		goto err_free_mem;
	}

	snprintf(ac->phys, sizeof(ac->phys), "%s/input0", dev_name(dev));

	input_dev->phys = ac->phys;
	input_dev->dev.parent = dev;
	input_dev->id.product = ac->model;
	input_dev->id.bustype = bops->bustype;
	input_dev->open = adxl34x_input_open;
	input_dev->close = adxl34x_input_close;

	input_set_drvdata(input_dev, ac);

	__set_bit(ac->pdata.ev_type, input_dev->evbit);

	if (ac->pdata.ev_type == EV_REL) {
		__set_bit(REL_X, input_dev->relbit);
		__set_bit(REL_Y, input_dev->relbit);
		__set_bit(REL_Z, input_dev->relbit);
	} else {
		/* EV_ABS */
		__set_bit(ABS_X, input_dev->absbit);
		__set_bit(ABS_Y, input_dev->absbit);
		__set_bit(ABS_Z, input_dev->absbit);

		if (pdata->data_range & FULL_RES)
			range = ADXL_FULLRES_MAX_VAL;	/* Signed 13-bit */
		else
			range = ADXL_FIXEDRES_MAX_VAL;	/* Signed 10-bit */

		input_set_abs_params(input_dev, ABS_X, -range, range, 3, 3);
		input_set_abs_params(input_dev, ABS_Y, -range, range, 3, 3);
		input_set_abs_params(input_dev, ABS_Z, -range, range, 3, 3);
	}

	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(pdata->ev_code_tap[ADXL_X_AXIS], input_dev->keybit);
	__set_bit(pdata->ev_code_tap[ADXL_Y_AXIS], input_dev->keybit);
	__set_bit(pdata->ev_code_tap[ADXL_Z_AXIS], input_dev->keybit);

	if (pdata->ev_code_ff) {
		ac->int_mask = FREE_FALL;
		__set_bit(pdata->ev_code_ff, input_dev->keybit);
	}

	if (pdata->ev_code_act_inactivity)
		__set_bit(pdata->ev_code_act_inactivity, input_dev->keybit);

	ac->int_mask |= ACTIVITY | INACTIVITY;

	if (pdata->watermark) {
		ac->int_mask |= WATERMARK;
		if (!FIFO_MODE(pdata->fifo_mode))
			ac->pdata.fifo_mode |= FIFO_STREAM;
	} else {
		ac->int_mask |= DATA_READY;
	}

	if (pdata->tap_axis_control & (TAP_X_EN | TAP_Y_EN | TAP_Z_EN))
		ac->int_mask |= SINGLE_TAP | DOUBLE_TAP;

	if (FIFO_MODE(pdata->fifo_mode) == FIFO_BYPASS)
		ac->fifo_delay = false;

	ac->bops->write(dev, POWER_CTL, 0);

#ifdef MICROHARD_ADXL
    /* set GPIO 6 interrupt for adxl34x chip on Microhard VIP4G board*/
    gpio_mode = __raw_readl(base + GPIO_REG_INT_MODE);
    gpio_type = __raw_readl(base + GPIO_REG_INT_TYPE);
    gpio_polarity = __raw_readl(base + GPIO_REG_INT_POLARITY);
    //gpio_status = __raw_readl(base + GPIO_REG_INT_PENDING);
    gpio_mask =  __raw_readl(base + GPIO_REG_INT_ENABLE);

#define GPIO_PIN_6 (0x1 << 6)
    gpio_mode |= GPIO_PIN_6;
    gpio_type &= ~GPIO_PIN_6;
    gpio_polarity  |= GPIO_PIN_6;        
    gpio_mask  |= GPIO_PIN_6;

    /* Config GPIO 6 interrupts */
    __raw_writel(gpio_mode, base + GPIO_REG_INT_MODE);              /* Config GPIO Interrupt Enable */
    __raw_writel(gpio_type, base + GPIO_REG_INT_TYPE);              /* Config GPIO Interrupt Type */
    __raw_writel(gpio_polarity, base + GPIO_REG_INT_POLARITY);      /* GPIO Interrupt Polarity: active High */
    __raw_writel(gpio_mask, base + GPIO_REG_INT_ENABLE);            /* GPIO Interrupt Mask */
#endif

    //err = request_irq(ac->irq, adxl34x_irq_wq,
	//		  IRQF_TRIGGER_RISING, "adxl345", ac);
	err = request_threaded_irq(ac->irq, NULL, adxl34x_irq,
                   IRQF_TRIGGER_RISING | IRQF_ONESHOT,
				   "adxl345", ac);
	if (err) {
		dev_err(dev, "irq %d busy?\n", ac->irq);
		goto err_free_mem;
	}    

	err = sysfs_create_group(&dev->kobj, &adxl34x_attr_group);
	if (err)
		goto err_free_irq;

	err = input_register_device(input_dev);
	if (err)
		goto err_remove_attr;

	AC_WRITE(ac, THRESH_TAP, pdata->tap_threshold);
	AC_WRITE(ac, OFSX, pdata->x_axis_offset);
	ac->hwcal.x = pdata->x_axis_offset;
	AC_WRITE(ac, OFSY, pdata->y_axis_offset);
	ac->hwcal.y = pdata->y_axis_offset;
	AC_WRITE(ac, OFSZ, pdata->z_axis_offset);
	ac->hwcal.z = pdata->z_axis_offset;
	AC_WRITE(ac, THRESH_TAP, pdata->tap_threshold);
	AC_WRITE(ac, DUR, pdata->tap_duration);
	AC_WRITE(ac, LATENT, pdata->tap_latency);
	AC_WRITE(ac, WINDOW, pdata->tap_window);
	AC_WRITE(ac, THRESH_ACT, pdata->activity_threshold);
	AC_WRITE(ac, THRESH_INACT, pdata->inactivity_threshold);
	AC_WRITE(ac, TIME_INACT, pdata->inactivity_time);
	AC_WRITE(ac, THRESH_FF, pdata->free_fall_threshold);
	AC_WRITE(ac, TIME_FF, pdata->free_fall_time);
	AC_WRITE(ac, TAP_AXES, pdata->tap_axis_control);
	AC_WRITE(ac, ACT_INACT_CTL, pdata->act_axis_control);
	AC_WRITE(ac, BW_RATE, RATE(ac->pdata.data_rate) |
		 (pdata->low_power_mode ? LOW_POWER : 0));
	AC_WRITE(ac, DATA_FORMAT, pdata->data_range );
    
	AC_WRITE(ac, FIFO_CTL, FIFO_MODE(pdata->fifo_mode) |
			SAMPLES(pdata->watermark));


	if (pdata->use_int2) {
		/* Map all INTs to INT2 */
		AC_WRITE(ac, INT_MAP, ac->int_mask | OVERRUN);
	} else {
		/* Map all INTs to INT1 */
		AC_WRITE(ac, INT_MAP, 0);
	}

	if (ac->model == 346 && ac->pdata.orientation_enable) {
		AC_WRITE(ac, ORIENT_CONF,
			ORIENT_DEADZONE(ac->pdata.deadzone_angle) |
			ORIENT_DIVISOR(ac->pdata.divisor_length));

		ac->orient2d_saved = 1234;
		ac->orient3d_saved = 1234;

		if (pdata->orientation_enable & ADXL_EN_ORIENTATION_3D)
			for (i = 0; i < ARRAY_SIZE(pdata->ev_codes_orient_3d); i++)
				__set_bit(pdata->ev_codes_orient_3d[i],
					  input_dev->keybit);

		if (pdata->orientation_enable & ADXL_EN_ORIENTATION_2D)
			for (i = 0; i < ARRAY_SIZE(pdata->ev_codes_orient_2d); i++)
				__set_bit(pdata->ev_codes_orient_2d[i],
					  input_dev->keybit);
	} else {
		ac->pdata.orientation_enable = 0;
	}

	//AC_WRITE(ac, INT_ENABLE, ac->int_mask | OVERRUN);   
    AC_WRITE(ac, INT_ENABLE, 0x58);     // enable SINGLE_TAP ACTIVITY INACTIVITY interrupt for VIP4G application

	ac->pdata.power_mode &= (PCTL_AUTO_SLEEP | PCTL_LINK);

#ifdef MICROHARD_ADXL
    setup_timer( &ac->adxl34x_timer, adxl34x_timer_callback, (unsigned long )ac );
#endif

 
#ifdef MICROHARD_ADXL 
    // start adxl34x measuring on module loading time
    AC_WRITE(ac, POWER_CTL, ac->pdata.power_mode | PCTL_MEASURE);
    // Starting timer to fire in 3000ms 
    mod_timer( &ac->adxl34x_timer, jiffies + msecs_to_jiffies(3000) );    
#endif 	

    printk(KERN_ALERT "adxl34x_probe successful...\n");

	return ac;

 err_remove_attr:
	sysfs_remove_group(&dev->kobj, &adxl34x_attr_group);
 err_free_irq:
	free_irq(ac->irq, ac);
 err_free_mem:
	input_free_device(input_dev);
	kfree(ac);
 err_out:
	return ERR_PTR(err);
}
EXPORT_SYMBOL_GPL(adxl34x_probe);

int adxl34x_remove(struct adxl34x *ac)
{
	sysfs_remove_group(&ac->dev->kobj, &adxl34x_attr_group);
	free_irq(ac->irq, ac);
#ifdef MICROHARD_ADXL
    del_timer( &ac->adxl34x_timer );
#endif
	input_unregister_device(ac->input);
	dev_dbg(ac->dev, "unregistered accelerometer\n");

	kfree(ac);

	return 0;
}
EXPORT_SYMBOL_GPL(adxl34x_remove);

MODULE_AUTHOR("Michael Hennerich <hennerich@blackfin.uclinux.org>");
MODULE_DESCRIPTION("ADXL345/346 Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
