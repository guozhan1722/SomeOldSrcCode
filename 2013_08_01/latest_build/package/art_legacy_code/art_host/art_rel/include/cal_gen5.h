/* cal_gen5.h - gen5 calibration header definitions */

/* Copyright (c) 2004 Atheros Communications, Inc., All Rights Reserved */

#ifndef __INCcalgen5h
#define __INCcalgen5h
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define IQ_CAL_RATEMASK		 0x01
#define IQ_CAL_RATE				6
#define IQ_CAL_NUMPACKETS    100
#define NUM_DUMMY_EEP_MAP1_LOCATIONS 10


void dutCalibration_gen5(A_UINT32 devNum, A_UINT32 mode);
A_BOOL setup_datasets_for_cal_gen5(A_UINT32 devNum, A_UINT32 mode);
void measure_all_channels_gen5(A_UINT32 devNum, A_UINT32 debug, A_UINT32 mode);
void measure_all_channels_generalized_gen5(A_UINT32 devNum, A_UINT32 debug, A_UINT32 mode);
void MH_cal_pm_gen5(A_UINT32 devNum, A_UINT32 debug, A_UINT32 mode);  // Adds offset to the previous algorithm (Bad Method)
void checkVpdPow_M1(A_UINT32 mode, A_INT32 gainIdx, A_INT32 chIdx, A_INT32 vpdIdx);
A_BOOL setBackOff(A_UINT32 mode, A_INT32 gainIdx, A_INT32 chIdx, A_INT32 vpdIdx, A_UINT16 *backoff_step);
//void MH_cal(A_UINT32 devNum, A_UINT32 debug, A_UINT32 mode);
//void MH_cal_nonProduction(A_UINT32 devNum, A_UINT32 debug, A_UINT32 mode, int cmd);
void MH_measure_all_channels(A_UINT32 devNum, A_UINT16 debug, A_UINT32 mode) ;
A_BOOL dump_raw_data_to_file_gen5( RAW_DATA_STRUCT_GEN5 *pRawData, char *filename);
A_BOOL raw_to_eeprom_dataset_gen5(RAW_DATA_STRUCT_GEN5 *pRawDataset, EEPROM_DATA_STRUCT_GEN5 *pCalDataset) ;
void fill_words_for_eeprom_gen5(A_UINT32 *word, A_UINT16 numWords, A_UINT16 *fbin, 
							 A_UINT16 dbmmask, A_UINT16 pcdmask, A_UINT16 freqmask);

A_UINT16 fill_eeprom_words_for_curr_ch_gen5(A_UINT32 *word, EEPROM_DATA_PER_CHANNEL_GEN5 *pCalCh);


void copy_11g_cal_to_11b_gen5(RAW_DATA_STRUCT_GEN5 *pRawDataSrc, RAW_DATA_STRUCT_GEN5 *pRawDataDest);
A_BOOL parsePowerCal(int mode);

void readPdadc(A_UINT32 devNum, A_UINT32 *rddata);
A_BOOL setupCal(A_UINT32 mode, A_UCHAR	*devlibMode, A_UINT32	*pd_gain,  double	*myAtten,  A_UINT16  *startAddrOffset,  A_UINT16  *sleepInterval );
void  setGainList(A_UINT32 mode,  A_UINT32  *xgain_list,  A_UINT32  *pd_gain, A_UINT32  *local_num_pd_gains );
void pwrDacLin(
			   A_INT16 *anchor_power_t4, 
			   A_INT16 *power_pcdac_offset_t4,
			   A_UINT16 sleepInterval,
			   double myAtten, 
			   double maxPower, 
			   A_UINT32 mode,
			   A_UINT32 devNum,
			   A_UINT16 channel
			   );

void setPowerDelta(
		double   	*initial_power,
		double		*power_delta,
		double		powerOffset
		);
/*
void read_dataset_from_file_gen5(RAW_DATA_STRUCT_GEN5 *pDataSet, char *filename);

void golden_iq_cal(A_UINT32 devNum, A_UINT32 mode, A_UINT32 channel);
void dut_iq_cal(A_UINT32 devNum, A_UINT32 mode, A_UINT32 channel);
void get_cal_info_for_mode_gen5(A_UINT32 *word, A_UINT16 numWords, A_UINT16 *fbin, 
					            A_UINT16 dbmmask, A_UINT16 pcdmask, A_UINT16 freqmask, A_UINT32 mode);
*/
#ifdef __cplusplus
}
#endif

#endif //__INCmauicalh



