/* hw.c - contain the access hrdware routines for SPIRIT  */

/* Copyright (c) 2001 Atheros Communications, Inc., All Rights Reserved */
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "dk_mem.h"
#include "dk_client.h"
#include "dk_common.h"
#include "dk_ioctl.h"

#include "ar5211reg.h"
#include "common_hw.h"
#include "mConfig.h"

//flash read varibles..

//flash block size is defined as 64 k
#define blk_size (64 << 10)

//// FORWARD DECLARATIONS
void hwInit(MDK_WLAN_DEV_INFO *pdevInfo, A_UINT32 resetMask);

//void deviceCleanup(A_UINT16 devIndex);

// global variables
MDK_WLAN_DRV_INFO	globDrvInfo;				
//#define  HWMEMRDWR

void hwInit
(
	MDK_WLAN_DEV_INFO *pdevInfo, A_UINT32 resetMask
)
{
	A_UINT32  reg, reg_pci, l_resetMask=0, pci_resetMask=0;
	 A_UINT16 devIndex = (A_UINT16)pdevInfo->pdkInfo->devIndex;
}
	

//#define AR5313_MAJOR_REV0	5
void hwClose
(
	MDK_WLAN_DEV_INFO *pdevInfo
)
{
	A_UINT32      reg, l_resetMask=0;
	A_UINT32 majorVer;
	 A_UINT16 devIndex = (A_UINT16)pdevInfo->pdkInfo->devIndex;

}

A_UINT32 validDevAddress(A_UINT32 address) 
{
    return 1;
	
}

/**************************************************************************
* apRegRead32 - read an 32 bit value.
*
* This routine will read the register pointed by the address 
* and return the value. This function is used only in AP, to read non-wmac
* registers. The address should be a 32-bit absolute address
*
* RETURNS: the value read
*/

A_UINT32 apRegRead32
(
	A_UINT32 devIndex,
    	A_UINT32 address                  
)
{
    	A_UINT32 value=0;
	 struct cfg_op co;
	  MDK_WLAN_DEV_INFO    *pdevInfo;
	  pdevInfo = globDrvInfo.pDevInfoArray[devIndex];
	  address = address & 0xFFFFFFFC;
	  if (validDevAddress(address)) {
	  // make an ioctl  call
	   co.offset=address;
	    co.size = 4;
            co.value = 0;
              if (ioctl(pdevInfo->hDevice,DK_IOCTL_SYS_REG_READ_32,&co) < 0) {
                   uiPrintf("Error: Sys reg Read to %x failed \n", address);
                   value = 0xdeadbeef;
             } else {
			 value = co.value;
             }
#ifdef DEBUG
           q_uiPrintf("Register at address %08lx: %08lx\n", address, value);
#endif
        } else {
                uiPrintf("ERROR: apRegRead32 could not access hardware address: 0x%08lx\n", address);
        }

        return value;
					      
}

/**************************************************************************
* apRegWrite32 - write a 32 bit value
*
* This routine will write the value into the register pointed 
* by the address/ This function is used only in AP, to write non-wmac
* registers. The address should be a 32-bit absolute address
*
*
* RETURNS: N/A
*/
void apRegWrite32
(
		A_UINT32 devIndex,
    	A_UINT32 address,         
    	A_UINT32  value           
)
{
       MDK_WLAN_DEV_INFO    *pdevInfo;
       struct cfg_op co;

        pdevInfo = globDrvInfo.pDevInfoArray[devIndex];

	address = address & 0xFFFFFFFC;

	if (validDevAddress(address)) {
         // make an ioctl  call
	co.offset=address;
	co.size = 4;
	co.value = value;
	if (ioctl(pdevInfo->hDevice,DK_IOCTL_SYS_REG_WRITE_32,&co) < 0) {
		uiPrintf("Error: Sys reg write to %x failed \n", address);
	}

#ifdef DEBUG
	q_uiPrintf("Register write @ address %08lx: %08lx\n", address, value);
#endif

	}  else {
		uiPrintf("ERROR: apRegWrite32 could not access hardware address: 0x%08lx\n", address);
	}
	       
	return;
			
}

A_UINT8 sysFlashConfigRead(A_UINT32 devNum, int fcl , int offset) {
    	A_UINT8  value=0;
	 struct  flash_op fop;
	  MDK_WLAN_DEV_INFO    *pdevInfo;
	   // pdevInfo = globDrvInfo.pDevInfoArray[devIndex];
	    pdevInfo = globDrvInfo.pDevInfoArray[0];
	  fop.fcl=fcl;
	  fop.offset = offset;
	  fop.len= 1;
	  fop.value=0;

#ifdef OWL_PB42
       if(isHowlAP(0)){

        if(fcl==2){
           offset=offset+0x1000;
         }

        int fd, i, j, count;
        unsigned char *fn,buf[10];
        unsigned int  val;
                fn = "/dev/mtdblock4";

        if ((fd = open(fn, O_RDWR)) < 0) {
                perror("Could not open flash");
                exit(1);
        }

        lseek(fd, -blk_size+offset, SEEK_END);
        if(read(fd, &value,1)<0){
	        perror("read");
        }

        close(fd);

       } else if(isFlashCalData()) {

         LIB_DEV_INFO  *pLibDev = gLibInfo.pLibDevArray[devNum];
	     offset += pLibDev->devMap.devIndex*16*1024; // Base of caldata (16K bytes each block)
         if(fcl==2){
           offset=offset+0x1000;
         }

		 int fd;
         unsigned char *fn;
         fn = "/dev/caldata";
         if ((fd = open(fn, O_RDWR)) < 0) {
            perror("Could not open flash");
            exit(1);
         }
         lseek(fd, -blk_size+offset, SEEK_END);
         if(read(fd, &value,1)<0){
	        perror("read");
         }
         close(fd);

       } else {
             printf("TEST:: Caldata sector is not defined for the design\n");
       }

#endif

	  return value;
         
}
A_UINT8  flash_read(A_UINT32 devNum, A_UINT32 fcl ,A_UINT8 *buf,A_UINT32 len, A_UINT32 offset) {
        A_UINT8  value=0;
         struct  flash_op fop;
          MDK_WLAN_DEV_INFO    *pdevInfo;
           // pdevInfo = globDrvInfo.pDevInfoArray[devIndex];
            pdevInfo = globDrvInfo.pDevInfoArray[0];
          fop.fcl=fcl;
          fop.offset = offset;
          fop.len= 1;
          fop.value=0;

#ifdef OWL_PB42
       if(isHowlAP(0)){
         if(fcl==2){
            offset=offset+0x1000;
         }

        int fd, i, j, count;
        unsigned char *fn;
        unsigned int  val;
                fn = "/dev/mtdblock4";

        if ((fd = open(fn, O_RDWR)) < 0) {
                perror("Could not open flash");
                exit(1);
        }

        lseek(fd, -blk_size+offset, SEEK_END);
        if(read(fd, buf,len*2)<0){
        	perror("read");
	    }
        close(fd);
      } else if (isFlashCalData()){
        LIB_DEV_INFO  *pLibDev = gLibInfo.pLibDevArray[devNum];
	    offset += pLibDev->devMap.devIndex*16*1024; // Base of caldata (16K bytes each block)
	
        if(fcl==2){
            offset=offset+0x1000;
        }

        int fd;
        unsigned char *fn;
        fn = "/dev/caldata";

        if ((fd = open(fn, O_RDWR)) < 0) {
                perror("Could not open flash");
                exit(1);
        }

        lseek(fd, -blk_size+offset, SEEK_END);
        if(read(fd, buf,len*2)<0){
        	perror("read");
	    }
        close(fd);

      } else {
			printf ("Error: flash access flash_read()\n");
	  }
#endif


      return 0;
}

void sysFlashConfigWrite(A_UINT32 devNum, int fcl, int offset, A_UINT8 *data, int len) {
    	A_UINT8 value=0;
	 struct  flash_op_wr fop;
	  MDK_WLAN_DEV_INFO    *pdevInfo;
	   // pdevInfo = globDrvInfo.pDevInfoArray[devIndex];
	  pdevInfo = globDrvInfo.pDevInfoArray[0];
	  fop.fcl=fcl;
	  fop.offset = offset;
	  fop.len=len;
	  fop.pvalue=data;

#ifdef OWL_PB42
       if(isHowlAP(0)){
         if(fcl==2){
               offset=offset+0x1000;
         }

        int fd, i, j, count;
        unsigned char *fn;
        fn = "/dev/mtdblock4";

        if ((fd = open(fn, O_RDWR)) < 0) {
                perror("Could not open flash");
                exit(1);
        }

        lseek(fd, -blk_size+offset, SEEK_END);
        if (write(fd, data, len) != len) {
        	perror("\nwrite");
	    }

        close(fd);

      } else if(isFlashCalData()){
         LIB_DEV_INFO  *pLibDev = gLibInfo.pLibDevArray[devNum];
	     offset += pLibDev->devMap.devIndex*16*1024; // Base of caldata (16K bytes each block)
         if(fcl==2){
               offset=offset+0x1000;
         }

        int fd;
        unsigned char *fn;
        fn = "/dev/caldata";

        if ((fd = open(fn, O_RDWR)) < 0) {
                perror("Could not open flash");
                exit(1);
        }

        lseek(fd, -blk_size+offset, SEEK_END);
        if (write(fd, data, len) != len) {
        	perror("\nwrite");
	    }

        close(fd);

     } else{
          printf("TEST:: Caldata sector is not defined for the design\n");
     }
#endif
	  
}

/************************************************************************/
// Function Name: writeProdDataWlanMac()
// Input:   Device Number
//			WLAN MAC address of size 6 char
// Output:  None
// Description: This fucntion writes WLAN mac address for PB42.
/************************************************************************/
void writeProdDataMacAddr(A_UINT32 devNum, A_UCHAR *mac0Addr, A_UCHAR *mac1Addr)
{
//	A_UINT16 *mac;
//	mac = (A_UINT16 *)mac0Addr;
//	printf("TEST:: called writeProdDataMacAddr \n");

//	printf("0x%04x:0x%04x:0x%04x\n", mac[0], mac[1], mac[2]);
//	mac = (A_UINT16 *)mac1Addr;
//	printf("0x%04x:0x%04x:0x%04x\n", mac[0], mac[1], mac[2]);

#if 0
	sysFlashConfigWrite(devNum, FLC_BOARDDATA, 0, &mac0Addr[0], 6);
	sysFlashConfigWrite(devNum, FLC_BOARDDATA, 6, &mac1Addr[0], 6);
#endif
}


void taskLock() {

}

void taskUnlock() {

}





