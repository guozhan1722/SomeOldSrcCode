#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include "dk_adf4350.h"

static struct adf4350 ADF_4350;
struct adf_params adfParams = {
    .freq = DEFAULT_FREQ,
    .refFreq = FREF,
    .modulusV = 0,  
    .delay = SPICLK,
};

void serialDrive(u32 data) {
   if (adfParams.baseaddr) {
        spiOutWord(adfParams.cs, adfParams.baseaddr, data);
   }
}

/******************************************************************************                        
 * Name:        ADF_4350_Init
 * Description: Initialize the synthesizer.  
 * Parameters:  None 
 * Returns:     
 ******************************************************************************/
void ADF_4350_Init(unsigned int baseaddr)
{
    memset(&adfParams, 0, sizeof(adfParams));
    printk(KERN_INFO "ADF_4350_Init: baseaddr=0x%x\n", baseaddr);   
    if (baseaddr) adfParams.baseaddr = baseaddr;   

    spiInit(adfParams.baseaddr);
        
    adfParams.freq = DEFAULT_FREQ;
    adfParams.refFreq = FREF;
    adfParams.modulusV = 4095 ; 
    adfParams.delay = SPICLK;
    
    if(adfParams.freq&0x200000000LL) adfParams.cs = 1;
    else  adfParams.cs = 0; 
    adfParams.freq &= 0x1FFFFFFFFLL;
    ADF4350_ComputeParams(adfParams.freq);

	// Configure the ADF 4350
	serialDrive(ADF_4350.R0.INT);
	serialDrive(ADF_4350.R1.INT);
	serialDrive(ADF_4350.R2.INT);
	serialDrive(ADF_4350.R3.INT);
	serialDrive(ADF_4350.R4.INT);
	serialDrive(ADF_4350.R5.INT);
}
    
int ADF4350_SetParams(u64 freq)
{
    int rtn = 0;
    
    if(freq&0x200000000LL) adfParams.cs = 1;
    else  adfParams.cs = 0; 
    freq &= 0x1FFFFFFFFLL;

    spiInit(adfParams.baseaddr);
    memset(&ADF_4350, 0, sizeof(ADF_4350));

    rtn = ADF4350_ComputeParams(freq);
    if(rtn) return rtn;

    // Configure the ADF 4350 
    if (freq  == 0) {
        serialDrive(ADF_4350.R4.INT);
    } else {
        serialDrive(ADF_4350.R0.INT);
        serialDrive(ADF_4350.R1.INT);
        serialDrive(ADF_4350.R2.INT);
        serialDrive(ADF_4350.R3.INT);
        serialDrive(ADF_4350.R4.INT);
        serialDrive(ADF_4350.R5.INT);
    }
    return 0;
}

/******************************************************************************                 
 * Name:        ADF4350_ComputeParams
 * Description: Calculate corresponding ADF 4350 register values for a specified 
 *              frequency 
 * Parameters:  ptr[IN/OUT]: Pointer to working requirement for input and register 
 *                           values for output 
 * Returns:     TRUE if OK, otherwise FALSE 
 *  For reference.  Not sure about it. 
 ******************************************************************************/        
int ADF4350_ComputeParams(u64 freq)
{
    u64 Fvco, tfreq ; 
	u16 integer ;
	u32 pfd, i = 0, numerator ; 
	u32 rf_div = 1, ref_div, ref_doubler;
    struct adf4350 			temp;
	struct adf_params 	adfParams ;

    printk(KERN_INFO "\nADF4350_SetParams: chip%d freq=%lld=0x%llx.\n", adfParams.cs+1, freq, freq);	
    
	adfParams.freq     = DEFAULT_FREQ ;
	adfParams.refFreq   = FREF ;
	adfParams.modulusV = 4095 ;
	adfParams.delay    = SPICLK ; 

  	temp.R0.BITS.Reserved 			= 0 ; 
    temp.R0.BITS.Integer 			    = R0_INTEGER_DEFAULT;
    temp.R0.BITS.Fractional 		    = R0_FRAC_DEFAULT;
    temp.R0.BITS.Command 			    = ADF_4350_REG0;
    temp.R1.BITS.Reserved 			= 0;
	//use the maxim for channel step resolution
    temp.R1.BITS.Modulus 				= FRACTIONAL_MODULUS; 
    temp.R1.BITS.Phase 				= RF_OUT_PHASE;      //recommended
    temp.R1.BITS.Prescaler 			= EIGHT_NINE_SCALER; //prescaler is 8/9 
    temp.R1.BITS.Command 				= ADF_4350_REG1;

    temp.R2.BITS.Reserved 			= 0;
    temp.R2.BITS.NoiseMode 			= LOW_NOISE_MODE;
    temp.R2.BITS.Muxout 			    = MUXOUT_THREE_STATE;
    temp.R2.BITS.RefDoubler 		    = 0;       
    temp.R2.BITS.RefDiv2 		    	= 0;
    temp.R2.BITS.RCounter 			= 2; // R_COUNTER_REF;     //R Counter
    temp.R2.BITS.DoubleBuff 		    = 0; //Disable double buffering R4 20:22 
    temp.R2.BITS.CP_current 		    = INIT_CP_CURRENT,       //ranging from 0 to 15
        temp.R2.BITS.LDF 					= FRAC_LOCK_DET;   //fractional lock detect
    temp.R2.BITS.LDP 					= LOCK_DET_IN_6NS;   //6ns precision detect of locking
    temp.R2.BITS.PD_polarity 		    = PHASE_DET_POSITIVE; //phase detect negative
    temp.R2.BITS.PowerDown 			= 0;   //set normal operting mode
    temp.R2.BITS.CP_3State 			= 0;             //normal case
    temp.R2.BITS.CounterRst 		    = 0;  //normal mode no reset for N/R counter 
    temp.R2.BITS.Command 				= ADF_4350_REG2;

    temp.R3.BITS.Reserved 			= 0;
    temp.R3.BITS.Reserved2 			= 0;
    temp.R3.BITS.CSR 					= 1; // MICROHARD_FALSE; //disabled cycle slip reduction
    temp.R3.BITS.Reserved3 			= 0;
    temp.R3.BITS.ClkDivMode 		    = CLK_DIV_ON;     // CLK_DIV_OFF;//to be changed for fast-lock
    temp.R3.BITS.ClkDivder 			= CLK_DIVIDER_VALUE;  //Maxim is 4095
    temp.R3.BITS.Command 				= ADF_4350_REG3;

    temp.R4.BITS.Reserved 			= 0;  
    temp.R4.BITS.Feedback_Sel 	    = VCO_FUNDAMENTAL_OUT;//regular method to feedback to N counter
    temp.R4.BITS.RFDivSel 			= RF_DIV_INIT;   //
    temp.R4.BITS.BandSel_clkDiv	    = BAND_SEL_CLK_DIV, //how many time used for VCO bandselect 
        temp.R4.BITS.VCOPowerDown 	    = 0;  //get VCO on
    temp.R4.BITS.MTLD 			    = 0;   //disable mute function before locked 
    temp.R4.BITS.AuxOutSel 			= AUX_RF_DIV_OUT; //output divided
    temp.R4.BITS.AuxOutEna 			= 0;    //enable the aux output as above
    temp.R4.BITS.AuxOutPwr 			= AUX_OUT_POWER; 
    temp.R4.BITS.RFOutEna 			= 1;   //enable RF output
    temp.R4.BITS.OutPwr 				= RF_OUT_POWER; 
    temp.R4.BITS.Command 				= ADF_4350_REG4;

    temp.R5.BITS.Reserved 			= 0,
        temp.R5.BITS.LDPinMode 			= DIGITAL_LOCK_DETECT;  //enable detect mode for the pin  
    temp.R5.BITS.Reserved2 			= 0;
    temp.R5.BITS.Reserved3 			= 3;
    temp.R5.BITS.Reserved4 			= 0;
    temp.R5.BITS.Command 				= ADF_4350_REG5;

    if(freq == 0) {
        temp.R4.BITS.RFOutEna 	= 0 ;
        ADF_4350.R4.INT = temp.R4.INT;
        return 0;
    } else if ((ADF4350_LOWEST_FREQ  > freq) || (ADF4350_HIGHEST_FREQ  < freq)) {
		return -EINVAL ;
	}

    for (i = 0; i < RF_DIVIDE_BY_16 + 1;) {
        if (VCO_MIN <= (freq << i) && VCO_MAX >= (freq << i)) {
            break ;
        } else {
            i++ ;
        }
	}

    // printk(KERN_INFO "ADF4350_ComputeParams:RF_DIVIDE_BY_16, i=%d\n",i);
	if (RF_DIVIDE_BY_16 < i) {
        return -EINVAL; 
	}

	rf_div  = 1 << i;
	temp.R4.BITS.RFDivSel = i;
    temp.R1.BITS.Modulus = 4095 ;


	if (PFD_MAX < adfParams.refFreq) {
        // This help function only works at the reference freq less 32 MHz
        printk(KERN_INFO "ADF4350_ComputeParams: PFD_MAX=%lld, adfParams.refFreq=%d\n", PFD_MAX, adfParams.refFreq);
        return -EINVAL;
	} else {
		if (PFD_MAX >= adfParams.refFreq * 2) {
			temp.R2.BITS.RefDoubler = 1 ;
		} else {
			temp.R2.BITS.RefDoubler = 0 ;
		}
	}

	tfreq = Fvco = freq * rf_div;
	pfd = 1 ; 
	integer = 0 ;
	numerator = 0 ;

	temp.R2.BITS.RefDiv2  = 0;
	ref_div = 1;

	temp.R2.BITS.RCounter = 1;
	ref_doubler = temp.R2.BITS.RefDoubler + 1;

    pfd	= ref_doubler * adfParams.refFreq / ref_div;

    // i = Fvco / pfd;
    for(i=0; tfreq>=pfd;i++) {
        tfreq -= pfd;
    }
    // printk(KERN_INFO "ADF4350_ComputeParams: Fvco=%lld, pfd=%d, i=%d\n", Fvco, pfd, i);

	if (R0_INT_MAX >= i ) {
		if (R0_INT_89_MIN < i) {
			temp.R1.BITS.Prescaler = EIGHT_NINE_SCALER ;
		} else if (R0_INT_45_MIN <= i){
			temp.R1.BITS.Prescaler = FOUR_FIVE_SCALER ;
		} else {
            printk(KERN_INFO "ADF4350_ComputeParams: R0_INT_89_MIN=%d, R0_INT_45_MIN=%d\n", R0_INT_89_MIN, R0_INT_45_MIN);
			return -EINVAL;
		}
	}
	 	
	integer = (u16)i;
    // i = Fvco%pfd;
	 while(Fvco>=pfd) {
         Fvco -= pfd;
     }
     // numerator = (( (long long)i * temp.R1.BITS.Modulus ) + pfd / 2)/ pfd ;
     tfreq = Fvco * temp.R1.BITS.Modulus + pfd / 2 ;
     for(i=0; tfreq>=pfd;i++) {
         tfreq -= pfd;
     }
     numerator = i+ (u32)tfreq/pfd;		
     // printk(KERN_INFO "ADF4350_ComputeParams: numerator=%d, i=%d\n", numerator, i);

     // Update the Reg0
     temp.R0.BITS.Integer = integer ;
     temp.R0.BITS.Fractional = numerator ;	
     temp.R2.BITS.RCounter = 2;

     // only now update the register structure.
     ADF_4350.R0.INT = temp.R0.INT;
     ADF_4350.R1.INT = temp.R1.INT;
     ADF_4350.R2.INT = temp.R2.INT;
     ADF_4350.R3.INT = temp.R3.INT;
     ADF_4350.R4.INT = temp.R4.INT;
     ADF_4350.R5.INT = temp.R5.INT;

     return 0;
}
