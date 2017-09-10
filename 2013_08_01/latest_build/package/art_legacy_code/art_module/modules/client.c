/* client.c */
#if defined(OWL_PB42) || defined(PYTHON_EMU)
#include <linux/pci.h>
#endif
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/page-flags.h>
#include <asm/io.h>
#if defined(OWL_PB42) || defined(PYTHON_EMU)
#ifdef PCI_BUS
#include "dk_pci_bus.h"
#endif
#endif

#include "dk.h"
#include "dk_ioctl.h"
#include "dk_event.h"
#include "client.h"

#define ORDER 9
#define ORDER_COMM_CLASS 8
#define ATHEROS_VENDOR_ID 0x168c
#define HOWL_RTC_BASE_ADDRESS 0xb80e0000
#define HOWL_WMAC_BASE_VIR_ADDRESS 0xb80c0000

void ADF_4350_Init(unsigned int baseaddr);
/* global variables */
static atheros_dev dev_table[MAX_CLIENTS_SUPPORTED];
static unsigned long mem_table[MAX_CLIENTS_SUPPORTED];
static UINT32 mem_page_order[MAX_CLIENTS_SUPPORTED];

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
#define SA_SHIRQ IRQF_SHARED
#endif

/* extern declarations */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17)
extern irqreturn_t dk_intr_handler(INT32,VOID *,struct pt_regs *);
#else
extern irqreturn_t dk_intr_handler(INT32,VOID *);
#endif

/* forward declarations */
static VOID reset_device(INT32 cli_id);

VOID init_client
(
 	VOID
)
{
	UINT32 i;

#ifdef DK_DEBUG
	printk(KERN_INFO "DK::Init dev table \n");
#endif

	for (i=0;i<MAX_CLIENTS_SUPPORTED;i++) {
			dev_table[i].cli_id = INVALID_CLIENT;
			mem_table[i] = 0;
	}
		
}

static VOID init_atheros_dev
(
 	p_atheros_dev dev
)
{
        int iIndex;

		dev->bus_dev = NULL;
		dev->reg_phy_addr = 0;
		dev->reg_ker_vir_addr = 0;
		dev->reg_range = 0;
		dev->mem_phy_addr = 0;
		dev->mem_ker_vir_addr = 0;
		dev->mem_size = 0;
		dev->irq = 0;
		dev->dev_busy = 0;
		dev->cli_id = INVALID_CLIENT;
		memset(&dev->isr_event_q,0,sizeof(event_queue));
		memset(&dev->trigered_event_q,0,sizeof(event_queue));
        for(iIndex=0; iIndex<MAX_BARS; iIndex++) {
		     dev->areg_phy_addr[iIndex] = 0;
		     dev->areg_ker_vir_addr[iIndex] = 0;
		     dev->areg_range[iIndex] = 0;
        }
#define ART_ENDIAN_UNKNOWN	0
#define ART_ENDIAN_BIG		1
#define ART_ENDIAN_LITTLE	2
		dev->cd_endianness = ART_ENDIAN_UNKNOWN;
}

#define _cli_readl __raw_readl
#define _cli_writel __raw_writel

static UINT32
cli_readl(UINT32 cli_id, UINT32 *addr, UINT32 offset)
{
	p_atheros_dev dev = &dev_table[cli_id];
	UINT32 retval;
	retval = __raw_readl(addr);
	if (0x4000 <= offset && offset < 0x5000 &&
	    dev->cd_endianness == ART_ENDIAN_BIG)
		retval = swab32(retval);
	return retval;
}

void
cli_set_cd_endianness(INT32 cli_id, UINT32 endian)
{
	p_atheros_dev dev = &dev_table[cli_id];
	if ((UINT32)cli_id >= MAX_CLIENTS_SUPPORTED) return;
	dev->cd_endianness = endian;
}

/*
 * Get the client for this device
 * The table give the mapping between the device and the client 
 * in the dev_table. The minor number is used to idenitfy the client
 * for the device.
 *  Device    client       minor 
 * /dev/dk0 - dev_table[0]   0
 * /dev/dk1 - dev_table[1]   1
 * /dev/dk2 - dev_table[2]   2
 * /dev/dk3 - dev_table[3]   3
 */
static INT32 get_client_id
(
 	INT32 major,
 	INT32 minor
)
{
#ifdef DK_DEBUG
	printk(KERN_INFO "DK::get_client_id:major=%d:minor=%d\n", major, minor);
#endif
	if (major == DK_UART_MAJOR_NUMBER) {
		return dev_table[minor + UART_FN_DEV_START_NUM].cli_id;
	}
	else
		return dev_table[minor].cli_id;
}

p_atheros_dev get_client
(
 	INT32 cli_id
)
{
	p_atheros_dev dev;

	dev = &dev_table[cli_id];
	if (!VALID_CLIENT(dev))  {
		return NULL;
	}

	return dev;
}

INT32 add_client
(
 	VOID *bus_dev,
	UINT32 baseaddr[MAX_BARS],
	UINT32 len[MAX_BARS],
	UINT32 irq,
    UINT32 numBars,
    UINT32 sIndex
)
{
		UINT32 i;
		INT32 cli_id;
		p_atheros_dev dev;
		UINT32 reg_vir_addr[MAX_BARS];
		unsigned long mem_phy_addr;
		unsigned long *mem;
		struct page *page;
		UINT32 no_pages, iIndex;
#if defined(OWL_PB42) || defined(PYTHON_EMU)
        INT8 ret_val;
#endif
        UINT32 page_order=ORDER;

		dev = NULL;
		mem = NULL;
		cli_id = INVALID_CLIENT;
		mem_phy_addr = 0;

		for (i=sIndex;i<MAX_CLIENTS_SUPPORTED;i++) {
				if (dev_table[i].cli_id == INVALID_CLIENT) {
						cli_id = i;
						dev = &dev_table[i];
						mem = &mem_table[i];
						break;
				}
		}

		if (cli_id == INVALID_CLIENT) {
			printk(KERN_INFO "DK:: Only %d clients supported \n",MAX_CLIENTS_SUPPORTED);
			return -1;
		}
		
		printk(KERN_INFO "DK::Add client %d \n",cli_id);

		init_atheros_dev(dev);		

        for(iIndex=0; iIndex<numBars; iIndex++) {
#if defined(OWL_PB42) || defined(PYTHON_EMU)
            (void)pci_read_config_byte(bus_dev, PCI_BASE_ADDRESS_0 + (iIndex *4), &ret_val);
            if (ret_val&0x1) {
               // IO region mapping
               /*
		       printk("DK:: Requesting IO region=%x:range=%d\n", baseaddr[iIndex], len[iIndex]);
		       if (request_region(baseaddr[iIndex],len[iIndex],DRV_NAME) == NULL) {
		          printk(KERN_ERR "DK:: unable to request io region for Bar %d: len = %d\n", iIndex, len[iIndex]);
				  return -1;
	           }
               */
            }
            else {
		      printk(KERN_INFO "DK:: Requesting MEM region=0x%x:range=%d\n", baseaddr[iIndex], len[iIndex]);
		      if (request_mem_region(baseaddr[iIndex],len[iIndex],DRV_NAME) == NULL) {
		          printk(KERN_ERR "DK:: unable to request mem region for Bar %d: len = %d\n", iIndex, len[iIndex]);
				  return -1;
	          }
                      reg_vir_addr[iIndex] = (UINT32)ioremap_nocache(baseaddr[iIndex],len[iIndex]);
                      if ((VOID *)reg_vir_addr[iIndex] == NULL) {
                                free_irq(irq,(void *)dev);
                                for(iIndex=0; iIndex<numBars; iIndex++) {
                                          iounmap((VOID *)reg_vir_addr[iIndex]);
                                          //release_mem_region(baseaddr[iIndex],len[iIndex]);
                                }
                                printk(KERN_ERR "DK:: unable to remap registers \n");
                                return -1;
                      }
                      ADF_4350_Init(reg_vir_addr[iIndex]);
            }

#endif
#ifdef AP83
		      reg_vir_addr[iIndex] = (UINT32)ioremap_nocache(baseaddr[iIndex],len[iIndex]);
		      if ((VOID *)reg_vir_addr[iIndex] == NULL) {
				free_irq(irq,(void *)dev);
                                for(iIndex=0; iIndex<numBars; iIndex++) {
			        	  iounmap((VOID *)reg_vir_addr[iIndex]);
		        	  	  //release_mem_region(baseaddr[iIndex],len[iIndex]);
                                }   
				printk(KERN_ERR "DK:: unable to remap registers \n");
				return -1;
		      }
#endif
        } // end of for

		   /* Pass the pointer to the client id */
		   if (request_irq(irq,dk_intr_handler,SA_SHIRQ,DRV_NAME,(void *)dev) != 0) {
              for(iIndex=0; iIndex<numBars; iIndex++) {
				iounmap((VOID *)reg_vir_addr[iIndex]);
				release_mem_region(baseaddr[iIndex],len[iIndex]);
              }
		        printk(KERN_ERR "DK:: unable to request irq \n");
				return -1;
		   }


        for(iIndex=0; iIndex<numBars; iIndex++) {
		   printk(KERN_INFO "DK::Reg phy addr = 0x%x vir Addr = 0x%x \n",baseaddr[iIndex],reg_vir_addr[iIndex]);
        }
		printk(KERN_INFO "DK::Irq = 0x%x \n",irq);


		if (*mem == 0) {
#if defined(OWL_PB42) || defined(PYTHON_EMU)
                (void)pci_read_config_byte(bus_dev, 0xb, &ret_val);
                if (ret_val == SIMPLE_COMM_CLASS) {
				    *mem = __get_free_pages(GFP_KERNEL,ORDER_COMM_CLASS);
                    page_order = ORDER_COMM_CLASS;
                }
                else {
				    *mem = __get_free_pages(GFP_KERNEL|GFP_DMA,ORDER);
				      //*mem = ioremap_nocache(0xf00000, 0x100000);
                    page_order = ORDER;
                }
#endif
#ifdef AP83
                    *mem = __get_free_pages(GFP_KERNEL|GFP_DMA,ORDER);
                    page_order = ORDER;

#endif
		if (*mem == 0) {
                    for(iIndex=0; iIndex<numBars; iIndex++) {
					   iounmap((VOID *)reg_vir_addr[iIndex]);
					   free_irq(irq,(void *)dev);
					   release_mem_region(baseaddr[iIndex],len[iIndex]);
                    }
					   printk(KERN_ERR "DK:: unable to allocate contigous memory \n");
					   return -1;
		}
				// map the pages as reserved,
				// otherwise remap_page_range wont 
				// do what we expect
				page = virt_to_page((void *)*mem);
				no_pages = 1 << page_order;
                		mem_page_order[cli_id] = page_order;
				for (i=0;i<no_pages;i++) {
					SetPageReserved(page);
					page++;
				}
				
				printk(KERN_INFO "DK::Allocated 0x%x bytes memory \n", (UINT32) (PAGE_SIZE * (1<<page_order)));
		} else {
				printk(KERN_INFO "DK::Using memory already allocated at %lx \n",*mem);
		}
		mem_phy_addr = __pa(*mem);
		printk(KERN_INFO "DK::memory vir Addr = %lx phy Addr = %lx \n",*mem,mem_phy_addr);

		/* 
		 * TO DO: (if necessary)
		 * Kernel already has a mapping for this physical pages. 
		 * They are part of the identity mapping, so they may be 
		 * cached. Remap it to another virtual address space 
		 * if the memory will be accessed from the kernel
		 */
		
		dev->bus_dev = bus_dev;
        for(iIndex=0; iIndex<numBars; iIndex++) {
		   dev->areg_phy_addr[iIndex] = baseaddr[iIndex];
		   dev->areg_ker_vir_addr[iIndex] = reg_vir_addr[iIndex];
		   dev->areg_range[iIndex] = len[iIndex];
        }
		dev->reg_phy_addr = baseaddr[0];
		dev->reg_ker_vir_addr = reg_vir_addr[0];
		dev->reg_range = len[0];
        	dev->numBars = numBars;
		dev->mem_ker_vir_addr = *mem;
		dev->mem_phy_addr = mem_phy_addr;
		dev->mem_size = (1 << page_order) * PAGE_SIZE;
		dev->irq = irq;
		dev->cli_id = cli_id;

#if defined(OWL_PB42) || defined(PYTHON_EMU)
		cli_cfg_read(cli_id,8,4,&dev->device_class);
#endif 
        	dev->device_class >>= 24;

		reset_device(cli_id);
		
		return 0;
}


VOID remove_client
(
 	VOID *bus_dev
)
{
		UINT32 i, iIndex;
		p_atheros_dev dev;

		printk(KERN_INFO "DK::Remove client \n");

		dev = NULL;
		for (i=0;i<MAX_CLIENTS_SUPPORTED;i++) {
				if (dev_table[i].cli_id == INVALID_CLIENT) continue;
			   	if (dev_table[i].bus_dev == bus_dev) {
						dev = &dev_table[i];
						break;
				}
		}
		
		if (dev == NULL) {
			printk(KERN_INFO "DK:: Invalid client in remove_client \n");
			return;
		}
		
		reset_device(dev->cli_id);

        for(iIndex=0; iIndex<dev->numBars; iIndex++) {
		    iounmap((VOID *)dev->areg_ker_vir_addr[iIndex]);
		    release_mem_region(dev->areg_phy_addr[iIndex], dev->areg_range[iIndex]);
		    printk(KERN_INFO "DK::Free reg space phy=0x%x vir=0x%x \n",dev->areg_phy_addr[iIndex],dev->areg_ker_vir_addr[iIndex]);
        }
		
		free_irq(dev->irq,(void *)dev);
		printk(KERN_INFO "DK::Free irq = 0x%x \n",dev->irq);

		init_atheros_dev(dev);		

		return;
}


VOID cleanup_client
(
 	VOID
)
{
		UINT32 i;
		UINT32 j;
		struct page *page;
		UINT32 no_pages;

#ifdef DK_DEBUG
		printk(KERN_INFO "DK::Cleanup dev table \n");
#endif // DK_DEBUG
		for (i=0;i<MAX_CLIENTS_SUPPORTED;i++) {
				if (dev_table[i].cli_id != INVALID_CLIENT) {
						remove_client(dev_table[i].bus_dev);
						dev_table[i].cli_id = INVALID_CLIENT;
				}
				if (mem_table[i] != 0) {
						printk(KERN_INFO "DK::Freeing memory at 0x%lx \n",mem_table[i]);
						// remove the reserved flag
						
						page = virt_to_page((void *)mem_table[i]);
						no_pages = 1 << mem_page_order[i];
                        			printk(KERN_INFO "page order=%d\n", mem_page_order[i]);
						for (j=0;j<no_pages;j++) {
							 ClearPageReserved(page);
							page++;
						}
						
						free_pages(mem_table[i],8);
						mem_table[i] = 0;
				}
		}
		return;
}

INT32 register_client
(
 	INT32 major,
 	INT32 minor
)
{
	INT32 cli_id;
	p_atheros_dev dev;
#if defined(OWL_PB42) || defined(PYTHON_EMU)
	UINT32 vendor_id;
#endif

	/* get the client for this device */
	cli_id = get_client_id(major, minor);

	if (cli_id == INVALID_CLIENT) {
		printk(KERN_INFO "DK::register_client:Device not found \n");
	 	return -ENODEV;
	}

	printk(KERN_INFO "DK::Regsitering client %d \n",cli_id);
	dev = &dev_table[cli_id];

	if (!VALID_CLIENT(dev)) {
		printk(KERN_INFO "DK::register_client:Invalid client \n");
	 	return -ENODEV;
    }

	if (BUSY_CLIENT(dev)) {
		printk(KERN_INFO "DK::register_client:Client alreay in use \n");
		return -EACCES;
	}
	
	// check whether the device is present
	// by reading the vendor id
#if defined(OWL_PB42) || defined(PYTHON_EMU)
	cli_cfg_read(cli_id,0,4,&vendor_id);
	if ((vendor_id & 0xffff) != ATHEROS_VENDOR_ID) {
		printk(KERN_INFO "DK::Device not present \n");
	 	return -ENODEV;
	}
#ifndef PYTHON_EMU
	if (bus_dev_init(dev->bus_dev) < 0) {
		printk(KERN_INFO "DK::register_client:Cannot initialize client \n");
		return -EACCES;
	}
#endif
#endif
	initEventQueue(&dev->isr_event_q);
	initEventQueue(&dev->trigered_event_q);
	
	reset_device(cli_id);
					
	dev->dev_busy = 1;
				  
	return dev->cli_id;
}

VOID unregister_client
(
 	INT32 cli_id
)
{
	p_atheros_dev dev;

	printk(KERN_INFO "DK::Unregsitering client %d \n",cli_id);
	
	dev = &dev_table[cli_id];

	if (!VALID_CLIENT(dev)) {
		printk(KERN_INFO "DK::unregister_client:Invalid client \n");
	 	return;
    }
	if (!BUSY_CLIENT(dev)) {
		printk(KERN_INFO "DK::unregister_client:Client not registered \n");
		return;
	}
	
	reset_device(dev->cli_id);
	
	deleteEventQueue(&dev->isr_event_q);
	deleteEventQueue(&dev->trigered_event_q);
#if defined(OWL_PB42) || defined(PYTHON_EMU)	
	bus_dev_exit(dev->bus_dev);
#endif
	dev->dev_busy = 0;
	
	return;
}

INT32 cli_reg_read
(
 	INT32 cli_id,
	INT32 offset,
	UINT32 *data
)
{
	p_atheros_dev dev;
#if defined(OWL_PB42) || defined(PYTHON_EMU) || defined(AP83)
	UINT32 *addr = 0;
#endif
	
	dev = &dev_table[cli_id];
	if (!VALID_CLIENT(dev)) {
		printk(KERN_INFO "DK::reg_read:Invalid client \n");
	 	return -1;
    }
#if defined(OWL_PB42) || defined(PYTHON_EMU)	
	addr = (UINT32 *)(dev->areg_ker_vir_addr[0] + offset);
#elif defined(AP83)
        addr = (UINT32 *)(HOWL_WMAC_BASE_VIR_ADDRESS + offset);
#else
        printk(KERN_INFO "invalid read from 0x%08x\n", offset);
#endif
#if defined(OWL_PB42) || defined(PYTHON_EMU) || defined(AP83)
	*data = cli_readl(cli_id, addr, offset);
#endif
#ifdef DK_DEBUG
	printk(KERN_INFO "DK::Reg read @ 0x%x : 0x%x \n",(UINT32)addr,*data);
#endif

	return 0;
}

INT32 cli_reg_write
(
 	INT32 cli_id,
	INT32 offset,
	UINT32 data
)
{
	p_atheros_dev dev;
#if defined(OWL_PB42) || defined(PYTHON_EMU) || defined(AP83)
	UINT32 *addr = 0;
#endif

	dev = &dev_table[cli_id];
	if (!VALID_CLIENT(dev)) {
		printk(KERN_INFO "DK::reg_write:Invalid client \n");
	 	return -1;
    }
#if defined(OWL_PB42) || defined(PYTHON_EMU)	
	addr = (UINT32 *)(dev->areg_ker_vir_addr[0] + offset);
#elif defined(AP83)
        addr = (UINT32 *)(HOWL_WMAC_BASE_VIR_ADDRESS + offset);
#else
#error Invalid configuration - PB42 (PCI) or AP83 (AHB) config must be selected
        printk(KERN_INFO "invalid write 0x%08x\n", offset);
#endif
#if defined(OWL_PB42) || defined(PYTHON_EMU) || defined(AP83)
	writel(data,addr);
#endif
#ifdef DK_DEBUG
	printk(KERN_INFO "DK::Reg write @ 0x%x : 0x%x \n",(UINT32)addr,data);
#endif

	return 0;
}
#if defined(OWL_PB42) || defined(PYTHON_EMU)
INT32 cli_cfg_read
(
 	INT32 cli_id,
	INT32 offset,
	INT32 size,
	INT32 *ret_val
)
{
	p_atheros_dev dev;

	dev = &dev_table[cli_id];
	
	if (!VALID_CLIENT(dev)) {
		printk(KERN_INFO "DK::cfg_read:Invalid client \n");
	 	return -1;
    }
	
	return bus_cfg_read(dev->bus_dev,offset,size,ret_val);
}
#endif
INT32 rtc_reg_read
(
        INT32 cli_id,           
        INT32 offset,                   
        INT32 *ret_val                  
)                               
{
        p_atheros_dev dev;      
        UINT32 *addr;           

        dev = &dev_table[cli_id];
        if (!VALID_CLIENT(dev)) {
                printk(KERN_INFO "DK::rtc_reg_read:Invalid client \n");
                return -1;      
    }                                           
          addr = (UINT32 *)(HOWL_RTC_BASE_ADDRESS+offset);
          *ret_val = cli_readl(cli_id, addr, offset);
          return 0;
                                
} 
INT32 get_chip_id                     
(                               
        INT32 cli_id,                   
        INT32 offset,           
        INT32 size,             
        INT32 *ret_val          
)                       
{                               
        p_atheros_dev dev;                      
        UINT32 *addr;           
        dev = &dev_table[cli_id];
                                        
        if (!VALID_CLIENT(dev)) { 
                printk(KERN_INFO "DK::cfg_read:Invalid client \n");
                return -1;      
    }                           
          addr = (UINT32 *)(0x00000000+offset);
          *ret_val = cli_readl(cli_id, addr, offset);
          return 0;             
                                
}
#if defined(OWL_PB42) || defined(PYTHON_EMU)  
INT32 cli_cfg_write
(
 	INT32 cli_id,
	INT32 offset,
	INT32 size,
	INT32 ret_val
)
{
	p_atheros_dev dev;
	
	dev = &dev_table[cli_id];
	
	if (!VALID_CLIENT(dev)) {
		printk(KERN_INFO "DK::cfg_write:Invalid client \n");
	 	return -1;
    }
	
	return bus_cfg_write(dev->bus_dev,offset,size,ret_val);
}
#endif

INT32 rtc_reg_write
(
        INT32 cli_id,
        INT32 offset,
        UINT32 data
)
{
        p_atheros_dev dev;
        UINT32 *addr;

        dev = &dev_table[cli_id];
        if (!VALID_CLIENT(dev)) {
                printk(KERN_INFO "DK::reg_write:Invalid client \n");
                return -1;
    }

        addr = (UINT32 *)(HOWL_RTC_BASE_ADDRESS + offset);
        writel(data,addr);


        return 0;
}

INT32 get_cli_info
(
 	INT32 cli_id,
	struct client_info *ci
)
{
	p_atheros_dev dev;
    int iIndex;
	
	dev = &dev_table[cli_id];
	
	if (!VALID_CLIENT(dev)) {
		printk(KERN_INFO "DK::get_client_info:Invalid client \n");
	 	return -1;
    }
	
	ci->reg_phy_addr = dev->areg_phy_addr[0];
	ci->reg_range = dev->areg_range[0];
    for (iIndex=0; iIndex<dev->numBars; iIndex++) {
	   ci->areg_phy_addr[iIndex] = dev->areg_phy_addr[iIndex];
	   ci->areg_range[iIndex] = dev->areg_range[iIndex];
    }
    ci->numBars = dev->numBars;
	ci->mem_phy_addr = dev->mem_phy_addr;
	ci->mem_size = dev->mem_size;
	ci->irq = dev->irq;
    ci->device_class = dev->device_class;

	return 0;
}

static VOID reset_device
(
 	INT32 cli_id
)
{
#if defined(AP83) || (!defined(PYTHON_EMU) && defined(DK_DEBUG)) || \
    ((defined(OWL_PB42) || defined(PYTHON_EMU)) && !defined(MB62))
	UINT32 out;
#endif
#ifdef DK_DEBUG
	printk(KERN_INFO "DK::Disable the interrupts and reset the device \n");	
#ifndef PYTHON_EMU
	cli_reg_read(cli_id, 0x4020, &out);
	printk(KERN_INFO "Device Mac Rev : 0x%x\n",out);			
#endif 
#endif
#if defined(OWL_PB42) || defined(PYTHON_EMU)
	if (dev_table[cli_id].device_class == NETWORK_CLASS) {
	   // disable interrupts 	
	   cli_reg_write(cli_id,0x0024,0x0);
#ifndef MB62
	   // put the device in reset state		
	   cli_reg_write(cli_id,0x704c,0x1); 				
	   cli_reg_write(cli_id,0x7040,0x0); 				
	   cli_reg_write(cli_id,0x7040,0x1); 	

	   cli_reg_read(cli_id, 0x7044, &out);
	   printk(KERN_INFO "Device Status is : 0x%x\n",out);			
#endif
	}
	if (dev_table[cli_id].device_class == SIMPLE_COMM_CLASS) {
	   cli_reg_write(cli_id,0x4,0x0); // IER
	   cli_reg_write(cli_id,0x114,0x0); // Extended IER
	   cli_reg_write(cli_id,0x104,0x1); // RC
	}
#endif
#ifdef AP83
        //if (dev_table[cli_id].device_class == NETWORK_CLASS) {
           // disable interrupts
           cli_reg_write(cli_id,0x0024,0x0);
           // put the device in reset state
           rtc_reg_write(cli_id,0x004c,0x1);
           rtc_reg_write(cli_id,0x0040,0x0);
           rtc_reg_write(cli_id,0x0040,0x1);

           rtc_reg_read(cli_id, 0x0044, &out);
           printk(KERN_INFO "Device Status is : 0x%x\n",out);

//}
        /*if (dev_table[cli_id].device_class == SIMPLE_COMM_CLASS) {
           cli_reg_write(cli_id,0x4,0x0); // IER
           cli_reg_write(cli_id,0x114,0x0); // Extended IER
           cli_reg_write(cli_id,0x104,0x1); // RC
        }*/




#endif

	return;
}	
