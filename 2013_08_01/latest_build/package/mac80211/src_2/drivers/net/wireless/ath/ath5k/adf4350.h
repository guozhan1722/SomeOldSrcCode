
/*****************************************************************************
 *                      Copyright (c) 2010 MicroHard Corp.                   
 *****************************************************************************
 *                                                                           
 * $Workfile: ADF_4350.h $                                                   
 *                                                                           
 * Purpose:   This file contains ADF 4350 Driver Interface
 *                                                               
 *                                                                           
 *****************************************************************************/
#ifndef _ADF_4350_h
#define _ADF_4350_h

/*******************************************************************************
 *                               Include Files
 ******************************************************************************/
#include <linux/types.h>

/*******************************************************************************
 *                          Public Defined Constants
 ******************************************************************************/
#define PORT_DOUT   0
#define PORT_CLK    1
#define PORT_CS1    2
#define PORT_CS2    3

#define RF_DIVIDE_BY_1         				0
#define RF_DIVIDE_BY_2         				1
#define RF_DIVIDE_BY_4         				2
#define RF_DIVIDE_BY_8         				3
#define RF_DIVIDE_BY_16        				4

#define NUM_REGS							6
#define REG_ZERO							0
// Reg 0
#define R0_INT_45_MIN						23
#define R0_INT_89_MIN						75
#define R0_INT_MAX							65535
#define R0_FRAC_MAX							4096
#define R0_INTEGER_DEFAULT					163
#define R0_FRAC_DEFAULT                     1952
// Reg 1
#define R1_PRES_MAX							2
#define R1_PHASE_MAX						4096
#define R1_MOD_MIN							2
#define R1_MOD_MAX							4095
#define FRACTIONAL_MODULUS 					(4096-1)  //the maximum
#define RF_OUT_PHASE           				1         //recommended
#define FOUR_FIVE_SCALER					0
#define EIGHT_NINE_SCALER      				1

#define R2_NOISE_MIN						0
#define R2_NOISE_MAX						4
#define R2_MUXOUT_MIN						0
#define R2_MUXOUT_MAX						7
#define R2_REFD_MAX							2
#define R2_REFDIV_MAX						2
#define R2_RCOUNTER_MIN						1
#define R2_RCOUNTER_MAX						1024
#define R2_DBUF_MAX							2
#define R2_CP_MAX							16
#define R2_LDF_MAX							2
#define R2_LDP_MAX							2
#define R2_PDP_MAX							2
#define R2_PDOWN_MAX						2
#define R2_CPS_MAX			  				2
#define R2_COUNTERR_MAX						2
#define LOW_NOISE_MODE         				0
#define LOW_SPUR_MODE          				3
#define MUXOUT_THREE_STATE     				0
#define MUXOUT_R_DIVIDER       				3
#define MUXOUT_N_DIVIDER       				4
#define MUXOUT_ALOCK_DET       				5         //Analog locked
#define MUXOUT_DLOCK_DET       				6         //Digital locked
#define R_COUNTER_REF          				1      		//R counter for reference clock
#define INIT_CP_CURRENT        				12        //ranging from 0 to 15
#define FRAC_LOCK_DET          				0
#define INT_LOCK_DET           				1
#define LOCK_DET_IN_10NS       				0 
#define LOCK_DET_IN_6NS        				1
#define PHASE_DET_NEGATIVE     				0
#define PHASE_DET_POSITIVE     				1
// Reg 3
#define R3_CSR_MAX							2
#define R3_CLKDIVM_MAX						4
#define R3_CLKDIV_MAX						4096
#define CLK_DIV_OFF            				0
#define CLK_DIV_ON            				1
#define CLK_DIVIDER_VALUE      				150       //Maxim is 4095
// Reg 4
#define R4_FBS_MAX							2
#define R4_RFDS_MAX							9
#define R4_BSCLKD_MIN						1
#define R4_BSCLKD_MAX						256										
#define R4_VCOPD_MAX						2
#define R4_MTLD_MAX							2
#define R4_AUXS_MAX							2
#define R4_AUXE_MAX							2
#define R4_AUXO_MAX							4
#define R4_RFOUTE_MAX						2
#define R4_OUTPOWER_MAX						4
#define FAST_LOCK_ENABLE       				1
#define RESYNC_ENABLE          				2
#define VCO_DIVIDER_OUT        				0
#define VCO_FUNDAMENTAL_OUT    				1
#define RF_DIV_INIT 						RF_DIVIDE_BY_4
#define BAND_SEL_CLK_DIV       				255   		//directly user R counter output
#define AUX_RF_DIV_OUT         				0         //after RF output divide
#define AUX_VCO_OUT            				1 				//RFout divider i.e. VCO fundamental
#define AUX_OUT_POWER          				0      		//0:-4dbm; step 3db up to +5dbm
#define RF_OUT_POWER           				2      		//0:-4dbm; step 3db up to +5dbm
// Reg 5
#define R5_LOCK_MAX							4
#define DIGITAL_LOCK_DETECT    				1

// Default Values
#define HALF_MODULUS        				2048      // FRACTIONAL_MODULUS/2
#define MAX_2_POWER_12						4095
#define VCO_MAX							    4400000000ULL
#define VCO_MIN							    2200000000ULL
#define VCO_MAX_MHZ							4400
#define VCO_MIN_MHZ							2200
#define PRESCALER_VCO_CUTOFF_MHZ            3000
#define PFD_MAX							    32000000ULL
#define PFD_MAX_MHZ							32
#define MHZ_TO_HZ							1000000
#define ADF4350_LOWEST_FREQ					137500000ULL    // 137.5 MHz
#define ADF4350_HIGHEST_FREQ                44000000000ULL   // 4400 MHz
#define DEFAULT_FREQ    2400000000UL
#define FREF            20000000
#define SPICLK          1000          
//#define ADF4350SPILSB

enum { 
	ADF_4350_REG0 ,
	ADF_4350_REG1 ,
	ADF_4350_REG2 ,
	ADF_4350_REG3 ,
	ADF_4350_REG4 ,
	ADF_4350_REG5 ,
	ADF_4350_REG_MAX
};

/*******************************************************************************
 *                          Public Type Declarations
 ******************************************************************************/

#if 1 /* BIG_ENDIAN   */
// current ST is liitle endian
// Linux is little endian
struct adf4350 {                            
	union {                                 	// Register 0               
		unsigned char 			BYTES[4];                   // long Access
    unsigned int    	INT;                        // long Access
    struct {
			unsigned int 		Reserved:     		1;
      unsigned int 		Integer:     			16;                  
      unsigned int 		Fractional:  			12;
      unsigned int 		Command:      		3;
			} BITS;
	} R0;
     
	union {																		//Register 1
    unsigned char 			BYTES[4];                   // long Access
    unsigned int  		INT;                        // long Access
	  struct {
          unsigned int 		Reserved:     	4;
      unsigned int 		Prescaler:    	1;                  
      unsigned int 		Phase:       		12;
      unsigned int 		Modulus:     		12;
      unsigned int 		Command:      	3;
    } BITS;
	} R1;

	union {																		//Register 2
		unsigned char 			BYTES[4];                   // long Access
    unsigned int 			INT;                        // long Access
		struct {
			unsigned int 		Reserved:     	1;
      unsigned int		NoiseMode:    	2;          //Noise Spur Mode                  
      unsigned int		Muxout:       	3;                 
      unsigned int		RefDoubler:   	1;					// D
      unsigned int		RefDiv2:      	1;					// T
      unsigned int		RCounter:    		10;         // R
      unsigned int		DoubleBuff:   	1;
      unsigned int		CP_current:   	4; 
      unsigned int		LDF:          	1;  
      unsigned int		LDP:          	1;
      unsigned int		PD_polarity:  	1;
      unsigned int		PowerDown:    	1;
      unsigned int		CP_3State:    	1;
      unsigned int		CounterRst:   	1;
      unsigned int		Command:      	3;
    } BITS;
	} R2;
    
	union {//Register 3
		unsigned char 			BYTES[4];                   // long Access
    unsigned int			INT;                        // long Access
		struct {
			unsigned int		Reserved:    11;
      unsigned int		Reserved2:    2;            //Noise Spur Mode                  
      unsigned int		CSR:          1;            //Cycle Slip Reduction
      unsigned int		Reserved3:    1;
      unsigned int		ClkDivMode:   2;           
      unsigned int		ClkDivder:    12;       
      unsigned int		Command:      3;
    } BITS;
	} R3; 
         
	union {																		//Register 4
		unsigned char				BYTES[4];                   // long Access
    unsigned int			INT;                        // long Access
		struct {
			unsigned int		Reserved:        8;
      unsigned int		Feedback_Sel:    1;                  
      unsigned int		RFDivSel:        3;                 
      unsigned int		BandSel_clkDiv:  8;
      unsigned int		VCOPowerDown:    1;
      unsigned int		MTLD:            1;        //mute till lock detect       
      unsigned int		AuxOutSel:       1;        //aux output select
      unsigned int		AuxOutEna:       1; 
      unsigned int		AuxOutPwr:       2;  
      unsigned int		RFOutEna:        1;
      unsigned int		OutPwr:          2;
      unsigned int		Command:         3;
		} BITS;
	} R4;

	union {																		//Register 5
		unsigned char				BYTES[4];                   // long Access
    unsigned int			INT;                        // long Access
		struct {
			unsigned int		Reserved:        8;
      unsigned int		LDPinMode:       2;         //Lock Detect Pin Mode
			unsigned int		Reserved2:       1;                                 
			unsigned int		Reserved3:       2;               
			unsigned int		Reserved4:      16;               
      unsigned int		Command:         3;
    } BITS;
	} R5;              
                   
} ; // AD_FRACN_SYN_TYPE;

#else       /* little endian */

struct adf4350 {                            
	union {                                 	// Register 0               
		unsigned char BYTES[4];               
        unsigned int INT;                        // long Access
        struct {
			unsigned int 		Command:      		3;
			unsigned int 		Fractional:  		12;
			unsigned int 		Integer:     		16; 
			unsigned int 		Reserved:     		1;
		} BITS;
	} R0;
     
	union {											//Register 1
        unsigned char 	BYTES[4];                   
        unsigned int  INT;                        // long Access
        struct {
			unsigned int 		Command:      	3;
			unsigned int 		Modulus:     	12;
			unsigned int 		Phase:       	12;
			unsigned int 		Prescaler:    	1;
			unsigned int 		Reserved:     	4;      
        } BITS;
	} R1;

	union {											//Register 2
        unsigned char 			BYTES[4];                   
        unsigned int 			INT;                        // long Access
		struct {
			unsigned int		Command:      	3;
			unsigned int		CounterRst:   	1;
			unsigned int		CP_3State:    	1;
			unsigned int		PowerDown:    	1;
			unsigned int		PD_polarity:  	1;
			unsigned int		LDP:          	1;
			unsigned int		LDF:          	1;  
			unsigned int		CP_current:   	4; 
			unsigned int		DoubleBuff:   	1;
			unsigned int		RCounter:    	10;  
			unsigned int		RefDiv2:      	1;
			unsigned int		RefDoubler:   	1;
			unsigned int		Muxout:       	3;    
			unsigned int		NoiseMode:    	2;          //Noise Spur Mode     
			unsigned int 		Reserved:     	1;   
        } BITS;
	} R2;
    
	union {//Register 3
		unsigned char 			BYTES[4];                   // long Access
        unsigned int			INT;                        // long Access
		struct {
			unsigned int		Command:      3;
			unsigned int		ClkDivder:    12;    
			unsigned int		ClkDivMode:   2;    
			unsigned int		Reserved3:    1;
			unsigned int		CSR:          1;            //Cycle Slip Reduction
			unsigned int		Reserved2:    2;            //Noise Spur Mode     
			unsigned int		Reserved:    11;
        } BITS;
	} R3; 
         
	union {																		//Register 4
		unsigned char			BYTES[4];                   
        unsigned int			INT;                        // long Access
		struct {
			unsigned int		Command:         3;
			unsigned int		OutPwr:          2;
			unsigned int		RFOutEna:        1;
			unsigned int		AuxOutPwr:       2;  
			unsigned int		AuxOutEna:       1; 
			unsigned int		AuxOutSel:       1;        //aux output select
			unsigned int		MTLD:            1;        //mute till lock detect 
			unsigned int		VCOPowerDown:    1;
			unsigned int		BandSel_clkDiv:  8;
			unsigned int		RFDivSel:        3;  
			unsigned int		Feedback_Sel:    1;  
			unsigned int		Reserved:        8;
		} BITS;
	} R4;

	union {																		//Register 5
		unsigned char			BYTES[4];                   // long Access
        unsigned int			INT;                        // long Access
		struct {
			unsigned int		Command:         3;
			unsigned int		Reserved4:      16; 
			unsigned int		Reserved3:       2; 
			unsigned int		Reserved2:       1; 
			unsigned int		LDPinMode:       2;         //Lock Detect Pin Mode
			unsigned int		Reserved:        8;
        } BITS;
	} R5;              
                   
};
#endif

// ADF 4350 internal parameters structure
struct adf_params {
	unsigned long long	freq;		// working frequency in Hz, could from 137.5 MHz to 4400 MHz 
	unsigned int	refFreq ; // Reference frequency in Hz, which is limited up to 32 MHz 
	unsigned int	modulusV; // valid values: 0 or 2 to 4095, 0 tells use the channel step
															// otherwise, use this value for modulus    
	unsigned int mixerFreqError;    
    unsigned int delay;
    unsigned int cs;
    struct ath5k_hw *ah;
}; 
 
struct adf_4350_syn {
  unsigned short integer;    	// 11 bits       0 -2048 
  unsigned int numerator;    // up to 22 bits 
};

extern struct adf_params adfParams;

/*******************************************************************************
 *                            Public Data
 ******************************************************************************/

/*******************************************************************************
 *                         Public Function Prototypes
 ******************************************************************************/
void ADF_4350_Init(struct ath_common *);
void ADF_4350_RFOutPutDis(void);
void ADF_4350_RFOutPutEn(void);
int	ADF4350_ComputeParams(unsigned long long freq);
void ADF_4350_synLoad(void);
int ADF4350_SetParams(unsigned long long freq);
void spiOutWord(u32 cs, struct ath5k_hw *ah, u32 data);
void spiInit(struct ath5k_hw *ah);

#endif //  _ADF_4350_h
