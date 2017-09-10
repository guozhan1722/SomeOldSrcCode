#include "project.h"

#ifdef _WINDOWS
#include <windows.h>
#endif 
#include <stdio.h>
#ifndef LINUX
#include <conio.h>
#endif
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include "wlantype.h"   /* typedefs for A_UINT16 etc.. */
#include "wlanproto.h"
#include "athreg.h"
#include "manlib.h"     /* The Manufacturing Library */
#include "MLIBif.h"     /* Manufacturing Library low level driver support functions */
#ifdef JUNGO
#include "mld.h"        /* Low level driver information */
#endif
#include "common_hw.h"
#include "manlibInst.h" /* The Manufacturing Library Instrument Library extension */
#include "mEeprom.h"    /* Definitions for the data structure */
#include "dynamic_optimizations.h"
#include "maui_cal.h"   /* Definitions for the Calibration Library */
#include "rssi_power.h"
#include "test.h"
#include "parse.h"
#include "dk_cmds.h"
#include "dk_ver.h"

#ifdef LINUX
//daniel #include "linux_ansi.h"
#include "unistd.h"
#endif	

#include "art_if.h"

#include "ar2413/mEEPROM_g.h"
#include "ar5212/mEEPROM_d.h"
#include "cal_gen5.h"

extern  A_UINT32	devlibModeFor[3];
extern  A_UINT32	calModeFor[3];

extern  char		modeName[3][122]  ;
extern  A_INT32		devPM, devSA, devATT;
extern  A_BOOL		REWIND_TEST;
extern  char		ackRecvStr[1024];

extern A_UINT16		RAW_CHAN_LIST_2p4[2][3];

static A_UCHAR  bssID[6]     = {0x50, 0x55, 0x5A, 0x50, 0x00, 0x00};
static A_UCHAR  rxStation[6] = {0x10, 0x11, 0x12, 0x13, 0x00, 0x00};	// DUT
static A_UCHAR  txStation[6] = {0x20, 0x22, 0x24, 0x26, 0x00, 0x00};	// Golden
static A_UCHAR  NullID[6]    = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66};
static A_UCHAR  pattern[2] = {0xaa, 0x55};

static A_UINT16 rates[MAX_RATES] = {6,9,12,18,24,36,48,54};

 RAW_DATA_STRUCT_GEN5 RawDatasetGen5_11a ; // raw power measurements for 11a
 RAW_DATA_STRUCT_GEN5 RawDatasetGen5_11g ; // raw power measurements for 11g
 RAW_DATA_STRUCT_GEN5 RawDatasetGen5_11b ; // raw power measurements for 11b
 RAW_DATA_STRUCT_GEN5 *pRawDataset_gen5[3] = {&RawDatasetGen5_11g, &RawDatasetGen5_11b, &RawDatasetGen5_11a} ; // raw power measurements

 EEPROM_DATA_STRUCT_GEN5 CalDatasetGen5_11a ; // calibration dataset
 EEPROM_DATA_STRUCT_GEN5 CalDatasetGen5_11g ; // calibration dataset
 EEPROM_DATA_STRUCT_GEN5 CalDatasetGen5_11b ; // calibration dataset
 EEPROM_DATA_STRUCT_GEN5 *pCalDataset_gen5[3] = {&CalDatasetGen5_11g, &CalDatasetGen5_11b, &CalDatasetGen5_11a} ; // calibration dataset

 RAW_DATA_STRUCT_GEN5 RawGainDatasetGen5_11a ; // raw gainF measurements for 11a
 RAW_DATA_STRUCT_GEN5 RawGainDatasetGen5_11g ; // raw gainF measurements for 11g
 RAW_DATA_STRUCT_GEN5 RawGainDatasetGen5_11b ; // raw gainF measurements for 11b
 RAW_DATA_STRUCT_GEN5 *pRawGainDataset_gen5[3] = {&RawGainDatasetGen5_11g, &RawGainDatasetGen5_11b, &RawGainDatasetGen5_11a} ; // raw gainF measurements

char calPowerLogFile_gen5[3][122] = {"cal_AR2413_Power_11g.log", "cal_AR2413_Power_11b.log", "cal_AR2413_Power_11a.log"};

static char delimiters[]   = " \t";

A_UINT16 dummpy_eep_map1_data[NUM_DUMMY_EEP_MAP1_LOCATIONS] = {
	0x4d31, //11b channel 2412 data
	0x7f54,
	0x3c93,
	0x1205,
	0x1931,
	0x492d, //11g channel 2442 data
	0x7f50,
	0x3c93,
	0x0e01,
	0x192d
};


void dutCalibration_gen5(A_UINT32 devNum, A_UINT32 mode)
{
	A_BOOL		read_from_file = CalSetup.readFromFile;
	char		*fileName = CalSetup.rawDataFilename;

	if (mode != MODE_11a) {
		read_from_file = CalSetup.readFromFile_2p4[mode];
		fileName       = CalSetup.rawDataFilename_2p4[mode];
	}

	// power datasets setup and measurement
	if (!setup_datasets_for_cal_gen5(devNum, mode)) { 
		uiPrintf("Could not setup gen5 raw datasets for mode %d. Exiting...\n", mode);
		printCalFailed(mode);
        closeEnvironment();
        exit(0);
	}

	if (read_from_file)
	{
		uiPrintf("Reading from file not implemented yet for gen5. Exiting...\n", mode);
		printCalFailed(mode);
        closeEnvironment();
       	exit(0);
		//++JC++ not implemented yet -- read_dataset_from_file_gen5(pRawDataset_gen3[mode], fileName);  
	} else
	{
		if ((mode == MODE_11b) && (CalSetup.useOneCal)){
			uiPrintf("\n\nWARNING: USE_11g_CAL_FOR_11b not supported, performing 11b measurement\n\n");
			//copy_11g_cal_to_11b_gen5(pRawDataset_gen5[MODE_11g], pRawDataset_gen5[MODE_11b]);
		} 
//		else {

		getCalData(devNum, mode, CalSetup.customerDebug);
//		}
		if(REWIND_TEST)
		{
			printCalFailed(mode);
			exit(0);
		}
		if (!dump_raw_data_to_file_gen5(pRawDataset_gen5[mode], calPowerLogFile_gen5[mode]))
		{
			printCalFailed(mode);
		}
	}
	if(!raw_to_eeprom_dataset_gen5(pRawDataset_gen5[mode], pCalDataset_gen5[mode]))
	{
		printCalFailed(mode);
	}
}

void copy_11g_cal_to_11b_gen5(RAW_DATA_STRUCT_GEN5 *pRawDataSrc, RAW_DATA_STRUCT_GEN5 *pRawDataDest) {

	A_UINT32 ii, jj, kk, idxL, idxR;
	RAW_DATA_PER_CHANNEL_GEN5	*pChDest;
	RAW_DATA_PER_CHANNEL_GEN5	*pChSrcL;
	RAW_DATA_PER_CHANNEL_GEN5	*pChSrcR;
	RAW_DATA_PER_PDGAIN_GEN5	*pXpdDest;
	RAW_DATA_PER_PDGAIN_GEN5	*pXpdSrcL;
	RAW_DATA_PER_PDGAIN_GEN5	*pXpdSrcR;
	A_UINT16    wordsForPdgains[] = {4,6,9,12}; // index is 1 less than numPdgains

	assert(pRawDataDest != NULL);
	assert(pRawDataSrc  != NULL);

	// map the 11g data to 2412, 2472, 2484 for 11b to give 2484 its
	// dedicated data.
	pRawDataDest->numChannels = 3;
	CalSetup.TrgtPwrStartAddr += wordsForPdgains[NUM_PDGAINS_PER_CHANNEL-1]*pRawDataDest->numChannels;
	CalSetup.numForcedPiers_2p4[MODE_11b] = pRawDataDest->numChannels;

	pRawDataDest->xpd_mask    = pRawDataSrc->xpd_mask;

	idxL = 0;
	idxR = 0;
	for (ii = 0; ii < pRawDataDest->numChannels ; ii++) {
		CalSetup.piersList_2p4[MODE_11b][ii] = RAW_CHAN_LIST_2p4[MODE_11b][ii];
		pRawDataDest->pChannels[ii] = RAW_CHAN_LIST_2p4[MODE_11b][ii];
		mdk_GetLowerUpperIndex(pRawDataDest->pChannels[ii], pRawDataSrc->pChannels, pRawDataSrc->numChannels, &(idxL), &(idxR));
		pChDest = &(pRawDataDest->pDataPerChannel[ii]);
		pChSrcL  = &(pRawDataSrc->pDataPerChannel[idxL]);
		pChSrcR  = &(pRawDataSrc->pDataPerChannel[idxR]);

		pChDest->channelValue = pRawDataDest->pChannels[ii];
		pChDest->numPdGains  = NUM_PDGAINS_PER_CHANNEL;
		pChDest->maxPower_t4  = mdk_GetInterpolatedValue_Signed16(
									pChDest->channelValue,
									pChSrcL->channelValue,
									pChSrcR->channelValue,
									pChSrcL->maxPower_t4,
									pChSrcR->maxPower_t4
								);
		pChDest->maxPower_t4  += (A_UINT16)(4*CalSetup.cck_ofdm_delta);
		
		if (pChDest->channelValue == 2484) {
			pChDest->maxPower_t4 -= (A_UINT16)(4*CalSetup.ch14_filter_cck_delta);
		}

		for (jj=0; jj<NUM_PDGAINS_PER_CHANNEL; jj++) {
			if ( ((1 << jj) & pRawDataDest->xpd_mask) == 0 ) {
				continue;
			}
			pXpdDest  = &(pChDest->pDataPerPDGain[jj]);
			pXpdSrcL  = &(pChSrcL->pDataPerPDGain[jj]);
			pXpdSrcR  = &(pChSrcR->pDataPerPDGain[jj]);
			// following quantities expected to remain the same for idxL and idxR
			assert(pXpdSrcL->numVpd == pXpdSrcR->numVpd);
			assert(pXpdSrcL->pd_gain  == pXpdSrcR->pd_gain);
			pXpdDest->numVpd = pXpdSrcL->numVpd;
			pXpdDest->pd_gain  = pXpdSrcL->pd_gain;

			for (kk = 0; kk < pXpdDest->numVpd; kk++) {
				pXpdDest->Vpd[kk] = getInterpolatedValue(	pChDest->channelValue,
											pChSrcL->channelValue,
											pChSrcR->channelValue,
											pXpdSrcL->Vpd[kk],
											pXpdSrcR->Vpd[kk],
											FALSE // don't scale_up
										);

				pXpdDest->pwr_t4[kk] = mdk_GetInterpolatedValue_Signed16(
											pChDest->channelValue,
											pChSrcL->channelValue,
											pChSrcR->channelValue,
											pXpdSrcL->pwr_t4[kk],
											pXpdSrcR->pwr_t4[kk]
										);
				
				pXpdDest->pwr_t4[kk] += (A_UINT16)(4*CalSetup.cck_ofdm_delta);

				if (pChDest->channelValue == 2484) {
					pXpdDest->pwr_t4[kk] -= (A_UINT16)(4*CalSetup.ch14_filter_cck_delta);
				}
			}
		}
	}
} // copy_11g_cal_to_11b_gen5

/*
void measure_all_channels_gen5(A_UINT32 devNum, A_UINT32 debug, A_UINT32 mode) {

	A_UINT32	pd_gain, xgain_list[2]; // handle upto two pd_gains
	A_INT32		ii, jj, kk, ll;
	A_UINT16	channel;
	A_UINT16	pcdac, anchor_pcdac;
	A_UINT32    offset;
	A_UINT16	reset = 0;
	double		txPower;
	A_UCHAR		devlibMode ; // use setResetParams to communicate appropriate mode to devlib
	double		maxPower;
	double		myAtten;
	A_UINT16    sleep_interval = 150;
	A_UINT32 	rddata;
	A_INT16		power_pcdac_offset_t4, power_to_store_t4, anchor_power_t4;
	A_UINT16 	current_pcdac;
	A_UINT32 	initial_power[2], power_delta[2];
	A_INT32 	initial_pcdac[2];
	A_UINT16    wordsForPdgains[] = {4,6,9,12}; // index is 1 less than numPdgains

	switch (mode) {
	case MODE_11a :
		devlibMode = MODE_11A;
		pd_gain   = CalSetup.xgain;
		myAtten	   = CalSetup.attenDutPM;
		CalSetup.TrgtPwrStartAddr += 5; // upto 10 piers
//printf("SNOOP: meas_all_gen5 : calTgtPwrStart = 0x%x\n", CalSetup.TrgtPwrStartAddr);
		break;
	case MODE_11g :
		devlibMode = MODE_11G;
		pd_gain   = CalSetup.xgain_2p4[mode];
		myAtten	   = CalSetup.attenDutPM_2p4[mode];
		CalSetup.TrgtPwrStartAddr += 2; // upto 4 piers
		break;
	case MODE_11b :
		devlibMode = MODE_11B;
		pd_gain   = CalSetup.xgain_2p4[mode];
		myAtten	   = CalSetup.attenDutPM_2p4[mode];
		CalSetup.TrgtPwrStartAddr += 2; // upto 4 piers
		if (CalSetup.pmModel == NRP_Z11) {
			sleep_interval = 50;
		} else {
			sleep_interval = 250;
		}
		break;
	default:
		uiPrintf("Unknown mode supplied to measure_all_channels_gen5 : %d \n", mode);
		break;
	}
	CalSetup.TrgtPwrStartAddr += wordsForPdgains[NUM_PDGAINS_PER_CHANNEL-1]*pRawDataset_gen5[mode]->numChannels;
//uiPrintf("SNOOP: meas_all_gen5 : calTgtPwrStart = 0x%x (mode=%d,wrdsperch=%d)\n", CalSetup.TrgtPwrStartAddr,mode,wordsForPdgains[NUM_PDGAINS_PER_CHANNEL-1]);
	configSetup.eepromLoad = 0;
	art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
					(A_BOOL)configSetup.eepromHeaderLoad, (A_UCHAR)devlibMode, configSetup.use_init);		

						
	uiPrintf("\nCollecting raw data for the adapter for mode %s\n", modeName[mode]);

	for (ii=0; ii<pRawDataset_gen5[mode]->numChannels; ii++)
	{
		channel = pRawDataset_gen5[mode]->pDataPerChannel[ii].channelValue;

		if (ii == 0)
		{
			art_resetDevice(devNum, txStation, NullID, channel, 0);
		} else
		{
			art_resetDevice(devNum, txStation, NullID, channel, 0);
//			art_changeChannel(devNum, channel); // for efficiency
		}

		uiPrintf("ch: %d  --> ", channel);
		
		//++JC++ For now only two pd_adc_gain supported -- if (CalSetup.cal_mult_xpd_gain_mask[mode] == 0) {
		if (1) {
				jj = 0;
				pRawDataset_gen5[mode]->xpd_mask = (1 << XPD_GAIN1_GEN5) | (1 << XPD_GAIN2_GEN5);
				xgain_list[0] = XPD_GAIN1_GEN5;
				xgain_list[1] = XPD_GAIN2_GEN5;
		} else {
			jj = 0;
			pRawDataset_gen5[mode]->xpd_mask = (A_UINT16) CalSetup.cal_mult_xpd_gain_mask[mode];
			pd_gain = CalSetup.cal_mult_xpd_gain_mask[mode];
			kk = 0;
			ll = 0;
			while (ll < 4) {
				if ((pd_gain & 0x1) == 1) {
					if (kk > 2) {
						uiPrintf("ERROR: A maximum of 2 pd_gains allowed to be specified in the rf_pd_gain parameter in the eep file.\n");
						exit(0);
					}
					xgain_list[kk++] = ll;
				}
				ll++;
				pd_gain = (pd_gain >> 1);
			}
			if (kk == 1) {
				xgain_list[1] = xgain_list[0];
			}
		}


		art_writeField(devNum, "bb_force_pdadc_gain", 1);
		art_writeField(devNum, "bb_forced_pdadc_gain", 1);

		art_forceSinglePowerTxMax(devNum, 0);
		art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, 
					    rates[0], DESC_ANT_A | USE_DESC_ANT);
		Sleep(50); // sleep 20 ms

		pcdac = 63;
		art_ForceSinglePCDACTable(devNum, pcdac);
		Sleep(sleep_interval);
		maxPower = pmMeasAvgPower(devPM, reset) + myAtten;				
		if (maxPower < 10) {
			Sleep(sleep_interval);
			maxPower = pmMeasAvgPower(devPM, reset) + myAtten;				
		}

		art_txContEnd(devNum);
		if (maxPower < 10) {
			uiPrintf("The max transmit Power is too small (%3.2f) at ch = %d. Giving up.\n", maxPower, channel);
			REWIND_TEST = TRUE;
			return;
		}

		if (CalSetup.customerDebug) {
			uiPrintf("maxPower = %3.2f", maxPower);
		} else {
			uiPrintf("max pwr is %3.2f dBm\n", maxPower);
		}
		pRawDataset_gen5[mode]->pDataPerChannel[ii].maxPower_t4 = (A_UINT16)(4*maxPower + 0.5);				

		// Measure for a pcdac and determine the correlation between pcdac and power
		anchor_pcdac = 22;
		art_ForceSinglePCDACTable(devNum, anchor_pcdac);

		// First get power for a setting that corresponds to almost the mid of the range required
		art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, 
					DataRate[configSetup.dataRateIndex], configSetup.antenna);
		Sleep(sleep_interval);

		txPower = pmMeasAvgPower(devPM, reset) + myAtten;
		while ((txPower < 5) && (anchor_pcdac < 56)){
//			Sleep(sleep_interval);
//			txPower = pmMeasAvgPower(devPM, reset) + myAtten;				

			anchor_pcdac += 4;
//printf("SNOOP: trying an anchor pcdac of %d\n", anchor_pcdac);
			art_txContEnd(devNum);
			art_ForceSinglePCDACTable(devNum, anchor_pcdac);

			// First get power for a setting that corresponds to almost the mid of the range required
			art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, 
					DataRate[configSetup.dataRateIndex], configSetup.antenna);
			Sleep(sleep_interval);

			txPower = pmMeasAvgPower(devPM, reset) + myAtten;				

		}
		art_txContEnd(devNum);
//printf("First Power measured: %f\n", txPower);		//++JC++

		//Try to get near mid power - 10dBm
		anchor_pcdac = anchor_pcdac - 2 * ((A_UINT16)txPower - 10);
		if(anchor_pcdac > 63) {
			uiPrintf("The first Power measurement is too small (%f) at ch = %d. Giving up.\n", txPower, channel);
			REWIND_TEST = TRUE;
			return;
		}

		art_ForceSinglePCDACTable(devNum, anchor_pcdac);

		// First get power for a setting that corresponds to almost the mid of the range required
		art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, 
					DataRate[configSetup.dataRateIndex], configSetup.antenna);
		Sleep(sleep_interval);

		txPower = pmMeasAvgPower(devPM, reset) + myAtten;
		if (txPower < 5) {
			Sleep(sleep_interval);
			txPower = pmMeasAvgPower(devPM, reset) + myAtten;				
		}
		art_txContEnd(devNum);
		//uiPrintf("Anchor Power measured: %f\n", txPower);		//++JC++
		//uiPrintf("Anchor pcdac : %d\n", anchor_pcdac);		//++JC++
		anchor_power_t4 = (A_INT16)((4 * txPower) + 0.5);
		//uiPrintf("Debug:: Anchor_power_t4: %d\n", anchor_power_t4);  //++JC++

		power_pcdac_offset_t4 = (2 * anchor_pcdac) - anchor_power_t4;

		// Setup the lowest power and deltas of power from the lowest for each xpd gain
		initial_power[0] = 0;		// 0 dBm for first xpd gain
		initial_power[1] = 7;		// 7 dBm for second xpd gain
		power_delta[0] = 3;		// 3 dB steps for first xpd gain
		power_delta[1] = 4;		// 4 dB steps for second xpd gain

		for (jj=0; jj<NUM_PDGAINS_PER_CHANNEL; jj++) {
			initial_pcdac[jj] = ((4 * initial_power[jj]) + power_pcdac_offset_t4)/2;		// Get pcdac correspoding to initial pwr
			offset = 30;
			if ((initial_pcdac[jj] < 0) || (initial_pcdac[jj] > 63)){
				offset = 30 + initial_pcdac[jj];
				initial_pcdac[jj] = 0;
			}

			pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].pd_gain = (A_UINT16)(xgain_list[jj]);

			if (jj == (NUM_PDGAINS_PER_CHANNEL - 1)){	// For the last xpd gain we have one more than the others
				pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].numVpd = NUM_POINTS_LAST_PDGAIN;
			} else {
				pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].numVpd = NUM_POINTS_OTHER_PDGAINS;
			}
			art_writeField(devNum, "bb_force_pdadc_gain", 1);
			art_writeField(devNum, "bb_forced_pdadc_gain", xgain_list[jj]);

			for (ll=0; ll < pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].numVpd; ll++) {
				current_pcdac = (A_UINT16)(initial_pcdac[jj] + (ll * 2 * power_delta[jj]));
				if (current_pcdac > 63) {
					current_pcdac = 63;
				}
				art_ForceSinglePCDACTableGriffin(devNum, current_pcdac, (A_UINT16)offset);
				art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, 
							DataRate[configSetup.dataRateIndex], configSetup.antenna);
				Sleep(sleep_interval);
				power_to_store_t4 = (A_UINT16)((2 * (current_pcdac - (30 - offset))) - power_pcdac_offset_t4);


				if ((power_to_store_t4) > 48) {		// Measure for > 15 dBm - remove later if possible
					txPower = pmMeasAvgPower(devPM, reset) + myAtten;
					if (txPower < 5) {
						Sleep(sleep_interval);
						txPower = pmMeasAvgPower(devPM, reset) + myAtten;				
					}
//					uiPrintf("\nPower measured: %f\n", txPower);		//++JC++
					power_to_store_t4 = (A_INT16)((txPower * 4) + 0.5);
				}


				art_txContEnd(devNum);
				rddata = art_regRead(devNum, 0xa264);
				if (power_to_store_t4 < 0) {power_to_store_t4 = 0;}
				pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].pwr_t4[ll]  = (A_INT16)(power_to_store_t4);
				pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll]  = (A_UINT16)(((rddata >> 1) & 0xff));

				//uiPrintf("Pcdac: %d \tAvg Out: %d \tPower_t4: %d \n", current_pcdac, pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll], pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].pwr_t4[ll]);	//++JC++ debug
			}
		}



//++JC++


		if (CalSetup.customerDebug) {
			uiPrintf("\n");
		}

	} // end channel loop

}
*/
//Microhard version of the calibration program for measuring channel powers
void MH_measure_all_channels(A_UINT32 devNum, A_UINT16 debug, A_UINT32 mode){
// algorithm is follows:
// Set the dac values we want to measure for each channel (For now its all 64 values)
// Set the fixed channels that is requested in the Cal setup
// Set the gain values requested in EEPROM manual (High , Low)
// Force the DAC value and set the device registers for TX contiious mode.
// Measure using the Spectrum analyzer
// The result of this fuction would be an array of DAC values and powers

	#define MAX_PCDAC 64

	A_UINT32	pd_gain, xgain_list[MAX_NUM_PDGAINS_PER_CHANNEL]; // handle max pd_gains
	A_UINT32    local_num_pd_gains;
	A_UINT16	channel;
	A_UINT16	pcdac;//, anchor_pcdac; removed siavash
//	A_UINT32    offset; removed Siavash
	A_UINT16	reset = 0;
//	double		txPower; removed Siavash

	A_UCHAR		devlibMode ; // use setResetParams to communicate appropriate mode to devlib
	double		maxPower;
	double		myAtten;
//	double		diffPwr; removed Siavash
	A_UINT16    sleep_interval = 50;
	A_UINT32 	rddata;
//	A_INT16		power_pcdac_offset_t4, power_to_store_t4; removed Siavash
//	A_INT16		anchor_power_t4; removed Siavash
//	A_UINT16 	current_pcdac; removed Siavash
//	double   	initial_power[MAX_NUM_PDGAINS_PER_CHANNEL]; removed Siavash
//	double		power_delta[MAX_NUM_PDGAINS_PER_CHANNEL]; removed Siavash
//	A_INT32 	initial_pcdac[MAX_NUM_PDGAINS_PER_CHANNEL]; removed Siavash
	A_UINT16    wordsForPdgains[] = {4,6,9,12}; // index is 1 less than numPdgains
	A_UINT16    target_anchor_power = 10; // dBm
	A_BOOL      backoff_power = FALSE;
	A_UINT16    backoff_step = 1; // 0.5 dB
	A_UINT16    MEAS_OVERLAP_HALF_DB = 2; // 1dB overlap
	A_BOOL      findFirstAnchor = TRUE;
	A_BOOL      refineAnchor = TRUE;
	A_UINT16    maxPdadc = 0, pdadc = 0, accPdadc = 0;
	A_BOOL      doneThrowAwayCal = FALSE;
	A_INT32		jj, ll;
	A_UINT16 addPow = 0;
	A_UINT16 pcdactemp;


	//Measurement variables
	double rbw;  //Resolution Bandwidth 
	double vbw;  //Video Bandwidth  
	double span; //span 
	double chsp; // Channel Spacing
	double chbw; // Channel Bandwidth
	
	int channelIndex;
	int depugProfile = 1;
	unsigned int gainIndex;
//PD parameters
	FILE* dfile;
	int tempData;


	A_UINT16	pdGainOverlap_t2;
	A_UINT32	pdGainSetting;
	tempData = 0;
	// Open for read (will fail if file "crt_fopen_s.c" does not exist)
	if ( fopen_s( &dfile, "adcdata.txt", "a+" ))  {
		 printf( "The file 'adcdata.txt' was not opened\n" );
	}else {
	  printf( "The file 'adcdata.txt' was opened\n" );		  
	}


	switch (mode) {
		case MODE_11a :
			devlibMode = MODE_11A;
			pd_gain   = CalSetup.xgain;
			myAtten	   = CalSetup.attenDutPM;
			CalSetup.cal_mult_xpd_gain_mask[MODE_11a] = 10;
	//		CalSetup.TrgtPwrStartAddr += 5; // upto 10 piers
			rbw = 30e3; vbw = 100e3; span = 60e6; chsp = 11e6; chbw = 22e6; //Based on Standard 15

			break;

		case MODE_11g :
			devlibMode = MODE_11G;
			CalSetup.cal_mult_xpd_gain_mask[MODE_11g] = 10;
			pd_gain   = CalSetup.xgain_2p4[mode];
			myAtten	   = CalSetup.attenDutPM_2p4[mode];
	//		CalSetup.TrgtPwrStartAddr += 2; // upto 4 piers

			//Measurement variables set for Channel Power measurements
//			rbw = 100e3; vbw = 100e3; span = 60e6; chsp = 11e6; chbw = 22e6; //Based on Standard 15
			rbw = 30e3; vbw = 100e3; span = 60e6; chsp = 11e6; chbw = 22e6; //Based on Standard 15
			break;
		case MODE_11b :
			CalSetup.cal_mult_xpd_gain_mask[MODE_11b] = 10;
			devlibMode = MODE_11B;
			pd_gain   = CalSetup.xgain_2p4[mode];
			myAtten	   = CalSetup.attenDutPM_2p4[mode];
	//		CalSetup.TrgtPwrStartAddr += 2; // upto 4 piers
//			rbw = 100e3; vbw = 30e3; span = 60e6; chsp = 11e6; chbw = 22e6; //Based on Standard 14.
			rbw = 30e3; vbw = 30e3; span = 60e6; chsp = 11e6; chbw = 22e6; //Based on Standard 14.
			break;
		default:
			uiPrintf("Unknown mode supplied to MH_measure_all_channels_2p4 : %d \n", mode);
			REWIND_TEST = TRUE;
			break;
	}

		if (CalSetup.pmModel == PM_436A) {
			sleep_interval = 250;
		}
// RAW_DATA_STRUCT_GEN5 *pRawDataset_gen5[3] = {&RawDatasetGen5_11g, &RawDatasetGen5_11b, &RawDatasetGen5_11a} ; // raw power measurements
// EEPROM_DATA_STRUCT_GEN5 *pCalDataset_gen5[3] = {&CalDatasetGen5_11g, &CalDatasetGen5_11b, &CalDatasetGen5_11a} ; // calibration dataset
// RAW_DATA_STRUCT_GEN5 *pRawGainDataset_gen5[3] = {&RawGainDatasetGen5_11g, &RawGainDatasetGen5_11b, &RawGainDatasetGen5_11a} ; // raw gainF measurements
	
		configSetup.antenna =   USE_DESC_ANT |DESC_ANT_A | DESC_ANT_B; //Siavash added B
		if (CalSetup.cal_mult_xpd_gain_mask[mode] == 0) {
				jj = 0;
				pRawDataset_gen5[mode]->xpd_mask = (1 << XPD_GAIN1_GEN5) | (1 << XPD_GAIN2_GEN5);
				xgain_list[0] = XPD_GAIN1_GEN5;
				xgain_list[1] = XPD_GAIN2_GEN5;
		} else {
			jj = 0;
			pRawDataset_gen5[mode]->xpd_mask = (A_UINT16) CalSetup.cal_mult_xpd_gain_mask[mode];
			pd_gain = CalSetup.cal_mult_xpd_gain_mask[mode];
			local_num_pd_gains = 0;
			ll = 0;
			while (ll < 4) {
				if ((pd_gain & 0x8) == 0x8) {
					if (local_num_pd_gains >= 4) {
						uiPrintf("ERROR: A maximum of 4 pd_gains allowed to be specified in the eep file.\n");
						exit(0);
					}
					xgain_list[local_num_pd_gains++] = (MAX_NUM_PDGAINS_PER_CHANNEL - 1) - ll;
				}
				ll++;
				pd_gain = (pd_gain << 1);
			}
		}

	if (depugProfile) uiPrintf("\n A-Collecting raw data for the adapter for mode %s\n", modeName[mode]);
	CalSetup.TrgtPwrStartAddr += wordsForPdgains[local_num_pd_gains-1]*pRawDataset_gen5[mode]->numChannels;
	configSetup.eepromLoad = 0;

 //A_UINT32 devNum,                          // In
 //A_UINT16 channel,                         // In       
 //RAW_DATA_STRUCT_GEN5 *pRawDataset,        // In
 //A_UINT16 pdGainOverlap_t2,                // In
 //A_INT16 *pMinCalPower,                    // Out	(2 x min calibrated power)
 //A_UINT16 pPdGainBoundaries[],             // Out
 //A_UINT16 pPdGainValues[],                 // Out
 //A_UINT16 pPDADCValues[]                   // Out 

	channel = pRawDataset_gen5[mode]->pDataPerChannel[0].channelValue;
	pdGainOverlap_t2 = art_regRead(devNum, 0xa26c) & 0xf;
	pdGainSetting = art_regRead(devNum, 0xa258);

	art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
					(A_BOOL)configSetup.eepromHeaderLoad, (A_UCHAR)devlibMode, configSetup.use_init);

	for (channelIndex =0; channelIndex < pRawDataset_gen5[mode]->numChannels; channelIndex++)
	{
		channel = pRawDataset_gen5[mode]->pDataPerChannel[channelIndex].channelValue;
//		art_resetDevice(devNum, txStation, NullID, channel, 0);
//		art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, rates[0], configSetup.antenna);	
		if (depugProfile) uiPrintf("ch: %d  --> ", channel);		
		for (gainIndex = 0; gainIndex < local_num_pd_gains; gainIndex++){
			art_resetDevice(devNum, txStation, NullID, channel, 0);
			art_forceSinglePowerTxMax(devNum, 0);
			art_writeField(devNum, "bb_force_pdadc_gain", 1);
			art_writeField(devNum, "bb_forced_pdadc_gain", xgain_list[gainIndex]);  //higher gain index is lower gain  
			//art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, rates[0], configSetup.antenna);				
			usleep(1000*sleep_interval);
			rddata = art_regRead(devNum, 0xa264);
			for (pcdac = 106; (pcdac > 20) && (pcdac <128) ; pcdac+= -5 ){	//MAX_PCDAC

				//art_ForceSinglePCDACTable(devNum, pcdac);
				if (pcdac > 97 ) {
					addPow = pcdac - 98 ;
					pcdactemp = 98;
				}else {
					addPow = 0;
					pcdactemp = pcdac;
				}
				art_ForceSinglePCDACTableGriffin(devNum, pcdactemp, 30+addPow); // use max gain setting
				art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, rates[0],configSetup.antenna);	// configSetup.antenna
 				Sleep(sleep_interval);
				// rbw,vbw,span, chsp, chbw;

				if (CalSetup.pmModel != NO_PM){
					maxPower = pmMeasAvgPower(devPM, reset);
					maxPower+= myAtten;				
				}else{
					maxPower = saChanPwr( devSA, channel*1e6, span, -5, rbw, vbw,chbw, chsp,CalSetup.saModel, reset);
					maxPower += myAtten;
				}
				// Read the pdadc corresponding to the measured power, pd_gain and pcdac
				//rddata = art_regRead(devNum, 0xa264);
				art_txContEnd(devNum);
				//readPdadc(devNum, &rddata);
				rddata = ((art_regRead(devNum, 0xa264)) & 0x1ff);
				fprintf(dfile,"%d,%d,%d,%d,%3.2f,%d\n",mode, channel,  (rddata >> 1) & 0xff,pcdac, maxPower, xgain_list[gainIndex] );
				uiPrintf("\nPower measured = %f, pdadc = %d , pcdac = %d, gainindex = %d\n", (float)maxPower, (rddata >> 1) & 0xff, pcdac, xgain_list[gainIndex]);		//++JC++
				//if (pcdac >= 33) 
				//	pcdac+=-5;
				//else 
				//	pcdac+=-18;
			}
		}// gainIndex
		//art_txContEnd(devNum);
	}
		fclose(dfile);
}
//*****************************************************************************************
//   Reads a set of pDADC values and stores in file
//*****************************************************************************************
void readPdadc(A_UINT32 devNum, A_UINT32 *rddata){
		
#define MAXREAD 5

	    //FILE* dfile;
		A_UINT32 regData[MAXREAD];
		int readCount;
		int tempData;
		readCount = 0;
		tempData = 0;
		//Sleep(1000);

	   //// Open for read (will fail if file "crt_fopen_s.c" does not exist)
	   //if ( fopen_s( &dfile, "adcdata.txt", "a+" ))  {
		  ////printf( "The file 'adcdata.txt' was not opened\n" );
	   //}
	   //else {
		  ////printf( "The file 'adcdata.txt' was opened\n" );		  
	   //}

		for (readCount = 0; readCount <MAXREAD; readCount++){
			tempData = ((art_regRead(devNum, 0xa264)) & 0x1ff);
			regData[readCount] = tempData;

			//isort_c(regData, readCount+1 );
		}

		*rddata = regData[2];

}

//*****************************************************************************************
//  Parses power table from file
//*****************************************************************************************
A_BOOL parsePowerCal(int mode) {

    FILE* fStream;

    char lineBuf[222], *pLine;
	int ii;

	//A_UINT32 tempSubSystemID; Siavash removed
	A_UINT16 vpdVal, pwrVal, gainNum, freqNum;
		
	gainNum = 0;
	vpdVal = 0;
	pwrVal = 0;
	freqNum = 0;

    uiPrintf("\nReading in Calibration Setup from %s\n",  CalSetup.rawDataFilename_2p4[mode]);
    if( (fStream = fopen(  CalSetup.rawDataFilename_2p4[mode], "r")) == NULL ) {
        uiPrintf("Failed to open %s - using Defaults\n",  CalSetup.rawDataFilename_2p4[mode]);
        return 0;
    }

    while(fgets(lineBuf, 120, fStream) != NULL) {
        pLine = lineBuf;
		uiPrintf(pLine);
        while(isspace(*pLine)) pLine++;
        if((*pLine == '#') || (*pLine == 0)) {
            continue;
        }else if(((strnicmp("PWR", pLine, strlen("PWR")) == 0) &&
			     ((pLine[strlen("PWR")] == ' ') || 
				   (pLine[strlen("PWR")] == '\t') ) ) ){

				pLine = strchr(pLine, '=');
				pLine++;
				pLine = strtok(pLine,";#");
				
				ii = 0;
				pLine = strtok(pLine, " ,");
				while  ((pLine != NULL) && (pLine[0] != ';') && (ii < 5))
				{
					if(!sscanf(pLine, "%d", &pwrVal )){						
						uiPrintf("Unable to read next power cal value from -->%s<--\n", pLine);
					} else
					{
						pRawDataset_gen5[mode]->pDataPerChannel[freqNum].pDataPerPDGain[gainNum].pwr_t4[ii] = (A_INT16)(( pwrVal * 4) + 0.5);
						ii++;
					}
					pLine = strtok(NULL, " ,");
				}
				if (ii < 1) {
					uiPrintf("Unable to read the power Cal value from %s\n",  CalSetup.rawDataFilename_2p4[mode]);
				}
		}else if(((strnicmp("VPD", pLine, strlen("VPD")) == 0) &&
			     ((pLine[strlen("VPD")] == ' ') || 
				   (pLine[strlen("VPD")] == '\t') ) ) ){

				pLine = strchr(pLine, '=');
				pLine++;
				pLine = strtok(pLine,";#");
				
				ii = 0;
				pLine = strtok(pLine, " ,");
				while  ((pLine != NULL) && (pLine[0] != ';') && (ii < 5))
				{
					if(!sscanf(pLine, "%d", &vpdVal )){						
						uiPrintf("Unable to read next ADC cal value from -->%s<--\n", pLine);
					} else
					{
						pRawDataset_gen5[mode]->pDataPerChannel[freqNum].pDataPerPDGain[gainNum].Vpd[ii] = (A_UINT16) (vpdVal); 
						ii++;
					}
					pLine = strtok(NULL, " ,");
				}
				if (ii < 1) {
					uiPrintf("Unable to read the ADC cal value from %s\n",  CalSetup.rawDataFilename_2p4[mode]);
				}
		}else if(strnicmp("GAIN_NUM", pLine, strlen("GAIN_NUM")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, " \t" ); //get past any white space etc
            if(!sscanf(pLine, "%ld", &gainNum)) {
                uiPrintf("Unable to read the GAIN_NUM from %s\n",  CalSetup.rawDataFilename_2p4[mode]);
				return 0;
			}
		}else if(strnicmp("FREQ_NUM", pLine, strlen("FREQ_NUM")) == 0) {
            pLine = strchr(pLine, '=');
            pLine++;
	        pLine = strtok( pLine, " \t" ); //get past any white space etc

            if(!sscanf(pLine, "%ld", &freqNum)) {
                uiPrintf("Unable to read the FREQ_NUM from %s\n",  CalSetup.rawDataFilename_2p4[mode]);
				return 0;
			}
		}		
	}
	fclose(fStream);
	return TRUE;
}
//*****************************************************************************************
//   Set gain list for calibration program
//*****************************************************************************************
void  setGainList(A_UINT32 mode,
				  A_UINT32  *xgain_list,
				  A_UINT32  *pd_gain,
				  A_UINT32  *local_num_pd_gains
	  ){
		  A_UINT32 kk;
		  A_INT32		jj,  ll;

		  if (CalSetup.cal_mult_xpd_gain_mask[mode] == 0) {
			  jj = 0;
			  pRawDataset_gen5[mode]->xpd_mask = (1 << XPD_GAIN1_GEN5) | (1 << XPD_GAIN2_GEN5);
			  xgain_list[0] = XPD_GAIN1_GEN5;
			  xgain_list[1] = XPD_GAIN2_GEN5;
			 
		  } else {
			  jj = 0;
			  pRawDataset_gen5[mode]->xpd_mask = (A_UINT16) CalSetup.cal_mult_xpd_gain_mask[mode];
			  (*pd_gain) = CalSetup.cal_mult_xpd_gain_mask[mode];
			  kk = 0;
			  ll = 0;
			  while (ll < 4) {
			  	  if (((*pd_gain) & 0x8) == 0x8) {
					  if (kk >= 4) {
					  	  uiPrintf("ERROR: A maximum of 4 pd_gains allowed to be specified in the eep file.\n");
						  exit(0);
					  }
					  xgain_list[kk++] = (MAX_NUM_PDGAINS_PER_CHANNEL - 1) - ll;
				  }
				  ll++;
				  (*pd_gain) = ((*pd_gain) << 1);
			  }
		  }

		  (*local_num_pd_gains) = kk;
		  if ((A_UINT16) (*local_num_pd_gains) != pRawDataset_gen5[mode]->pDataPerChannel[0].numPdGains) {
		 		uiPrintf("ERROR: num_pd_gains [%d] from xpd_mask [0x%x] does not match with the dataset [%d]\n", (local_num_pd_gains),
				CalSetup.cal_mult_xpd_gain_mask[mode], 
				pRawDataset_gen5[mode]->pDataPerChannel[0].numPdGains);
				exit(0);
			}
			if ((A_UINT16) (*local_num_pd_gains) != CalSetup.numPdGains) {
				uiPrintf("ERROR: num_pd_gains [%d] from xpd_mask [0x%x] does not match with the CalSetup.numPdGains in the .eep file [%d]\n", local_num_pd_gains,
				CalSetup.cal_mult_xpd_gain_mask[mode], 
				CalSetup.numPdGains);
				exit(0);
			}

}
//*****************************************************************************************
//  sets up calibration variables and controls
//*****************************************************************************************
A_BOOL setupCal(A_UINT32 mode,
			  A_UCHAR	*devlibMode,
			  A_UINT32	*pd_gain,
			  double	*myAtten,
			  A_UINT16  *startAddrOffset,
			  A_UINT16  *sleepInterval ){
		
		*sleepInterval = 50;
		switch (mode) {
			case MODE_11a :
				*devlibMode = MODE_11A;
				*pd_gain   = CalSetup.xgain;
				*myAtten	  = CalSetup.attenDutPM;//+CalSetup.offsetCalPower;
				*startAddrOffset = 5; // upto 10 piers
				if (CalSetup.pmModel == PM_436A) {
					*sleepInterval = 150;
				}
				return FALSE;
				break;
			case MODE_11g :
				*devlibMode = MODE_11G;
				*pd_gain   = CalSetup.xgain_2p4[mode];
				*myAtten	  = CalSetup.attenDutPM_2p4[mode];//+CalSetup.offsetCalPower;
				*startAddrOffset = 2; // upto 4 piers
				if (CalSetup.pmModel == PM_436A) {
					*sleepInterval = 150;
				}
				return FALSE;
				break;
			case MODE_11b :
				*devlibMode = MODE_11B;
				*pd_gain   = CalSetup.xgain_2p4[mode];
				*myAtten	  = CalSetup.attenDutPM_2p4[mode];//+CalSetup.offsetCalPower;
				*startAddrOffset = 2; // upto 4 piers
				if (CalSetup.pmModel == PM_436A) {
					*sleepInterval = 250;
				}
				return FALSE;
				break;
			default:
				uiPrintf("Unknown mode supplied to measure_all_channels_gen5 : %d \n", mode);
				return TRUE;
				break;
		}
}
//*****************************************************************************************
//   Finds correlation between power and pcdac values
//*****************************************************************************************
void pwrDacLin(
			   A_INT16 *anchor_power_t4, 
			   A_INT16 *power_pcdac_offset_t4,
			   A_UINT16 sleepInterval,
			   double myAtten, 
			   double maxPower, 
			   A_UINT32 mode,
			   A_UINT32 devNum,
			   A_UINT16 channel
			   ){

		A_INT16 anchor_pcdac;
		A_BOOL findFirstAnchor;
		A_BOOL refineAnchor;

		double txPower;
		double diffPwr;
		A_UINT16	reset = 0;
		A_UINT16    target_anchor_power = 10; // dBm


		anchor_pcdac = 22;
		findFirstAnchor = TRUE;
		while (findFirstAnchor) {
			art_ForceSinglePCDACTable(devNum, anchor_pcdac);
			art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, DataRate[configSetup.dataRateIndex], configSetup.antenna );
			usleep(1000*sleepInterval);
			
				txPower = pmMeasAvgPower(devPM, reset) + myAtten;

				if (txPower < 5) {
					usleep(1000*sleepInterval);
					txPower = pmMeasAvgPower(devPM, reset) + myAtten;				
				}
				art_txContEnd(devNum);
				if(txPower < 0) {
					anchor_pcdac += 10;
				}else {
					findFirstAnchor = FALSE;
				}
		} //while
		//Try to get near mid power - 10dBm, or maxpower - 15 if maxpower > 23 dBm
		if (maxPower > 23) {
			target_anchor_power = (A_UINT16)(maxPower - 15);
		}

 		diffPwr = 2 * (txPower - target_anchor_power);
		
		if (diffPwr > anchor_pcdac)
			anchor_pcdac =0;
		else
			anchor_pcdac = anchor_pcdac - (A_UINT16)diffPwr;

		
		if(anchor_pcdac > 63) {
			uiPrintf("The first Power measurement is too small (%f) at ch = %d. Giving up.\n", txPower, channel);
			REWIND_TEST = TRUE;
			return;
		}

		//FJC: try to get the anchor pcdac power within +/- 1dbm of mid power
		

		refineAnchor = TRUE;
		while(refineAnchor) {
			art_ForceSinglePCDACTable(devNum, anchor_pcdac);
			art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, DataRate[configSetup.dataRateIndex], configSetup.antenna);
			usleep(1000*sleepInterval);
			txPower = pmMeasAvgPower(devPM, reset) + myAtten;

			if (txPower < 5) {
				usleep(1000*sleepInterval);
				txPower = pmMeasAvgPower(devPM, reset) + myAtten;				
			}
			art_txContEnd(devNum);

			if( (txPower > target_anchor_power + 1)	|| (txPower < target_anchor_power - 1)) {
				if (2 * ((A_UINT16)txPower - target_anchor_power) <= anchor_pcdac)
					anchor_pcdac = anchor_pcdac - 2 * ((A_UINT16)txPower - target_anchor_power);
				else
					anchor_pcdac = 0;

			} else {
				refineAnchor = FALSE;
			}
		}// while

		//adjustment needed for 11g 
		if((mode == MODE_11g) && (!isNala(swDeviceID)) ) {
			txPower -= 0.5;
		}

		(*anchor_power_t4) = (A_INT16)((4 * (txPower-CalSetup.offsetCalPower)) + 0.5);
		(*power_pcdac_offset_t4) = (2 * anchor_pcdac) - (*anchor_power_t4);
}
//*****************************************************************************************
//   sets power deltas ...
//*****************************************************************************************
void setPowerDelta(
		double   	*initial_power,
		double		*power_delta,
		double		powerOffset
		){

		A_UINT16    MEAS_OVERLAP_HALF_DB = 2; // 1dB overlap
		A_INT32	    gainIdx;
		double		max_Power = 31;

		gainIdx = 0;		
		while (gainIdx < CalSetup.numPdGains) {
			if (gainIdx < 1) {
				initial_power[0] = 0;
			} else {
			    initial_power[gainIdx] = (CalSetup.pdGainBoundary[gainIdx-1] - MEAS_OVERLAP_HALF_DB)/2.0;
			}
			if (gainIdx == (CalSetup.numPdGains - 1)) {
				power_delta[gainIdx] = ((0.8 + (double)(CalSetup.pdGainBoundary[gainIdx]-4+2*(CalSetup.offsetCalPower) +
									MEAS_OVERLAP_HALF_DB - 2*(initial_power[gainIdx]+CalSetup.offsetCalPower)) / (NUM_POINTS_LAST_PDGAIN - 1)))/2;
				
				//power_delta[gainIdx] = ((0.8 + (double)(2*max_Power +
				//					MEAS_OVERLAP_HALF_DB - 2*(initial_power[gainIdx]+CalSetup.offsetCalPower)) / (NUM_POINTS_LAST_PDGAIN - 1)))/2;

			} else {
				power_delta[gainIdx] = ((0.8 + (double)(CalSetup.pdGainBoundary[gainIdx] + 2*(CalSetup.offsetCalPower)+
									MEAS_OVERLAP_HALF_DB - 2*(initial_power[gainIdx]+ CalSetup.offsetCalPower)) / (NUM_POINTS_OTHER_PDGAINS - 1)))/2;
			}
				gainIdx++;	
		} //while gainIdx

}
//*****************************************************************************************
//   Modified power Calibration
//***************************************************************************************** 
void MH_cal_pm_gen5(A_UINT32 devNum, A_UINT32 debug, A_UINT32 mode){

	A_UINT32	pd_gain;
	A_UINT32	xgain_list[MAX_NUM_PDGAINS_PER_CHANNEL]; // handle max pd_gains
	A_UINT32	local_num_pd_gains;
	A_UINT32 	rddata;
	A_UINT32    offset;
	A_UINT32    fixOffset;
	
	A_UCHAR		devlibMode;

	A_INT32		gainIdx;
	A_INT32		chIdx;
	A_INT32     vpdIdx;
	A_INT32 	initial_pcdac[MAX_NUM_PDGAINS_PER_CHANNEL];

	A_UINT16	startAddrOffset;
	A_UINT16	sleepInterval;
	A_UINT16	channel;
	A_UINT16    wordsForPdgains[] = {4,6,9,12}; // index is 1 less than numPdgains
	A_UINT16	pcdac;
	//A_UINT16	anchor_pcdac; removed Siavash
	A_UINT16	reset = 0;
	A_UINT16    MEAS_OVERLAP_HALF_DB = 2; // 1dB overlap
	A_UINT16    backoff_step = 1; // 0.5 dB
	A_UINT16 	current_pcdac;

	A_INT16		power_pcdac_offset_t4, power_to_store_t4, anchor_power_t4;

	A_BOOL      findFirstAnchor = TRUE;
	//A_BOOL		refineAnchor;  removed siavash
	A_BOOL		powerOutOfBand = FALSE;

	double   	initial_power[MAX_NUM_PDGAINS_PER_CHANNEL];
	double		power_delta[MAX_NUM_PDGAINS_PER_CHANNEL];
	double      intercept_scale[NUM_POINTS_LAST_PDGAIN] = {1,1,1,1,1};
	double		powerOffset = 0;
//	double		diffPwr; removed diffPwr
	double		maxPower;
	double		txPower;
	double		myAtten;

	A_BOOL      backoff_power = FALSE;
	A_BOOL      doneThrowAwayCal = FALSE;
	int			initVar;

	// pcdac vs power should be linear this is charachteristic of the Atheros chip internally
	// Any non linearity is due to saturation and front end filter design an dshould be catched

	startAddrOffset = 0;
	sleepInterval = 150;
	pd_gain =0;
	devlibMode = MODE_11G;
	local_num_pd_gains = 2;
	fixOffset = 30;
	offset = fixOffset;

	for (initVar = 0; initVar <MAX_NUM_PDGAINS_PER_CHANNEL;initVar++){
		xgain_list[initVar] = 0;
	}

	if (setupCal(mode, &devlibMode, &pd_gain, &myAtten, &startAddrOffset, &sleepInterval)){
		uiPrintf("Unknown mode supplied to measure_all_channels_gen5 : %d \n", mode);
		REWIND_TEST = TRUE;
	}
	
	configSetup.antenna =   USE_DESC_ANT | DESC_ANT_A | DESC_ANT_B; // remove B later Ubi works with B
	
	setGainList(mode, xgain_list, &pd_gain, &local_num_pd_gains);

	//Set EEPROM addresses for Calibration
	CalSetup.TrgtPwrStartAddr += startAddrOffset;
	CalSetup.TrgtPwrStartAddr += wordsForPdgains[local_num_pd_gains-1]*pRawDataset_gen5[mode]->numChannels;
	
	configSetup.eepromLoad = 0;
	art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
					(A_BOOL)configSetup.eepromHeaderLoad, (A_UCHAR)devlibMode, configSetup.use_init);	

	uiPrintf("\nCollecting raw data for the adapter for mode %s\n", modeName[mode]);

	// Go over all channels specified in the calSetup
	for (chIdx = 0; chIdx < pRawDataset_gen5[mode]->numChannels; chIdx++){

		//Ensures that temperature is stabilized 
		if((chIdx == 1) && (!doneThrowAwayCal)) {
			chIdx = 0;
			doneThrowAwayCal = 1;
		}

		channel = pRawDataset_gen5[mode]->pDataPerChannel[chIdx].channelValue;
		art_resetDevice(devNum, txStation, NullID, channel, 0);

		art_writeField(devNum, "bb_force_pdadc_gain", 1);
		art_writeField(devNum, "bb_forced_pdadc_gain", xgain_list[0]);

		art_forceSinglePowerTxMax(devNum, 0);
		art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, rates[0], configSetup.antenna);
		usleep(1000*250); 

		pcdac = MAX_PCDAC-1;
		art_ForceSinglePCDACTableGriffin(devNum, pcdac, (A_UINT16)fixOffset); // use max gain setting
		//art_ForceSinglePCDACTable(devNum, 255); // use max gain setting
		usleep(1000*sleepInterval);

		maxPower = pmMeasAvgPower(devPM, reset) + myAtten;	
		if (maxPower < 10) {
			usleep(1000*sleepInterval);
			maxPower = pmMeasAvgPower(devPM, reset) + myAtten;				
		}

		art_txContEnd(devNum);
		if (maxPower < 10) {
			uiPrintf("The max transmit Power is too small (%3.2f) at ch = %d. Giving up.\n", maxPower, channel);
			REWIND_TEST = TRUE;
			return;
		}
		
		pRawDataset_gen5[mode]->pDataPerChannel[chIdx].maxPower_t4 = (A_UINT16)(4*maxPower + 0.5);	

		// Measure for a pcdac and determine the correlation between pcdac and power
		// PCDAC vs power is linear with slope of 1 therefore either find difference or directly find slope 
		// In this method we will record every measurement.


		// Measure for a pcdac and determine the correlation between pcdac and power
		pwrDacLin(&anchor_power_t4, &power_pcdac_offset_t4, sleepInterval, myAtten, maxPower, mode, devNum, channel);
		
		setPowerDelta(initial_power, power_delta, powerOffset);

		for (gainIdx=0; gainIdx < CalSetup.numPdGains; gainIdx++) {
			initial_pcdac[gainIdx] = (A_INT32)(((4 * initial_power[gainIdx]) + power_pcdac_offset_t4)/2);		// Get pcdac correspoding to initial pwr

			if ((initial_pcdac[gainIdx] < 0) || (initial_pcdac[gainIdx] > 63)){
				//offset = fixOffset + (initial_pcdac[gainIdx]-63); original program
				offset = fixOffset + (initial_pcdac[gainIdx]);
				initial_pcdac[gainIdx] = 0;
					//initial_pcdac[gainIdx] = 63; original program
			}
			//else if (initial_pcdac[gainIdx] < 0) {
			//	initial_pcdac[gainIdx] = 0; original program
			//}

			pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].pd_gain = (A_UINT16)(xgain_list[gainIdx]);

			if (gainIdx == (CalSetup.numPdGains - 1)){	// For the last xpd gain we have one more than the others
				pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].numVpd = NUM_POINTS_LAST_PDGAIN;
			} else {
				pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].numVpd = NUM_POINTS_OTHER_PDGAINS;
			}

			art_writeField(devNum, "bb_force_pdadc_gain", 1);
			art_writeField(devNum, "bb_forced_pdadc_gain", xgain_list[gainIdx]);

			for (vpdIdx=0; vpdIdx < pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].numVpd; vpdIdx++) {

				backoff_step = 0;
				backoff_power = TRUE; // mechanism to ensure Vpd_delta < 64

				while (backoff_power) {
					current_pcdac = (A_UINT16)(initial_pcdac[gainIdx] + (vpdIdx * 2 * power_delta[gainIdx] * intercept_scale[vpdIdx]) - backoff_step);
					if (current_pcdac > 63){
						//current_pcdac = 63; original
						powerOutOfBand = TRUE;
					}else{
						powerOutOfBand = FALSE;
					}
					art_ForceSinglePCDACTableGriffin(devNum, current_pcdac, (A_UINT16)offset);
					art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, DataRate[configSetup.dataRateIndex], configSetup.antenna);
					usleep(1000*sleepInterval);
					power_to_store_t4 = (A_UINT16)((2 * (current_pcdac - (30 - offset))) - power_pcdac_offset_t4);

					//if (powerOutOfBand){ 
						if ((power_to_store_t4+(4*CalSetup.offsetCalPower)) > 4*(maxPower - 10)) {		// Measure for > (maxPower - 10) dBm - remove later if possible
								txPower = pmMeasAvgPower(devPM, reset) + myAtten;
								if (txPower < 5) {
									usleep(1000*sleepInterval);
									txPower = pmMeasAvgPower(devPM, reset) + myAtten;				
								}
							//adjustment needed for 11g
								if (mode == MODE_11g){
								txPower -= 0.5;
							}
								power_to_store_t4 = (A_INT16)(((txPower- CalSetup.offsetCalPower) * 4) + 0.5);
						}
					//} 
					art_txContEnd(devNum);
					rddata = art_regRead(devNum, 0xa264);
					
					if (power_to_store_t4 < 0) {power_to_store_t4 = 0;}
						
					pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].pwr_t4[vpdIdx]  = (A_INT16)(power_to_store_t4);
					//if (!powerOutOfBand){
						pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx]  = (A_UINT16)(((rddata >> 1) & 0xff));
					//}else{
					//	pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx]  = 
					//		pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx-1]+1;
					//}

					backoff_power = setBackOff(mode, gainIdx, chIdx, vpdIdx, &backoff_step);
					if (!backoff_power)
						uiPrintf("Pcdac: %d \tAvg Out: %d \tPower_t4: %d \n", 
						current_pcdac, 
						pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx], 
						pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].pwr_t4[vpdIdx]);
					else{
						uiPrintf("backoff active: Pcdac: %d \tAvg Out: %d \tPower_t4: %d \n", 				
						current_pcdac, 
						pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx], 
						pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].pwr_t4[vpdIdx]);
					}

					if(CalSetup.fixZeroPowerDuringCal && (gainIdx ==0))//(xgain_list[gainIdx] == 3)) //only fix on low power (high gain) curve
											checkVpdPow_M1(mode, gainIdx, chIdx, vpdIdx);
                    
				}
			}
		}		

	} //chIdx
}
//*****************************************************************************************
//   Sets back off values 
//*****************************************************************************************
A_BOOL setBackOff(A_UINT32 mode, A_INT32 gainIdx, A_INT32 chIdx, A_INT32 vpdIdx, A_UINT16 *backoff_step){

		
	if (vpdIdx > 0) {
		if((pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx] -
			pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx-1]) > 63){
				(*backoff_step) += 1;    // 0.5dB step
				return TRUE;
		} else if((pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].pwr_t4[vpdIdx] -
		pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].pwr_t4[vpdIdx-1]) > 28){
				(*backoff_step) += (pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].pwr_t4[vpdIdx] -
						pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].pwr_t4[vpdIdx-1] - 28)/2;    // 0.5dB step
				return TRUE;
				} else {
					return FALSE;
				}
	}
	else {
		return  FALSE;
	}
}
//*****************************************************************************************
//   check poer and vpd values
//*****************************************************************************************
void checkVpdPow_M1(A_UINT32 mode, A_INT32 gainIdx, A_INT32 chIdx, A_INT32 vpdIdx){

	if(pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx] < 15) {
		pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx] = 15;
	}

	if(vpdIdx > 0) {
		if(pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx] <
			pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx - 1] + 8) {

				pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx] = 
					pRawDataset_gen5[mode]->pDataPerChannel[chIdx].pDataPerPDGain[gainIdx].Vpd[vpdIdx - 1] + 8;
		}
	}
}
//*****************************************************************************************
//  
//*****************************************************************************************
void measure_all_channels_generalized_gen5(A_UINT32 devNum, A_UINT32 debug, A_UINT32 mode) {

	A_UINT32	pd_gain, xgain_list[MAX_NUM_PDGAINS_PER_CHANNEL]; // handle max pd_gains
	A_UINT32    local_num_pd_gains;
	A_INT32		ii, jj, kk, ll;// ww; removed ww
	A_UINT16	channel;
	A_UINT16	pcdac, anchor_pcdac;
	A_UINT32    offset;
	A_UINT16	reset = 0;
	double		txPower;
	A_UCHAR		devlibMode ; // use setResetParams to communicate appropriate mode to devlib
	double		maxPower;
	double		myAtten;
	A_UINT16    sleep_interval = 50;
	A_UINT32 	rddata;
	A_INT16		power_pcdac_offset_t4, power_to_store_t4, anchor_power_t4;
	A_UINT16 	current_pcdac;
	double   	initial_power[MAX_NUM_PDGAINS_PER_CHANNEL], power_delta[MAX_NUM_PDGAINS_PER_CHANNEL];
	A_INT32 	initial_pcdac[MAX_NUM_PDGAINS_PER_CHANNEL];
	A_UINT16    wordsForPdgains[] = {4,6,9,12}; // index is 1 less than numPdgains
	A_UINT16    target_anchor_power = 10; // dBm
	A_BOOL      backoff_power = FALSE;
	A_UINT16    backoff_step = 1; // 0.5 dB
	A_UINT16    MEAS_OVERLAP_HALF_DB = 2; // 1dB overlap
	A_BOOL      findFirstAnchor = TRUE;
	A_BOOL      refineAnchor = TRUE;
	A_UINT16    maxPdadc = 0, pdadc = 0, accPdadc = 0;
	A_BOOL      doneThrowAwayCal = FALSE;

	//Measurement variables
	double rbw;  //Resolution Bandwidth 
	double vbw;  //Video Bandwidth  
	double span; //span 
	double chsp; // Channel Spacing
	double chbw; // Channel Bandwidth


	//  map intercepts {0, 25, 50, 75, 100} to {0, 35, 65, 85, 100}
	//double      intercept_scale[NUM_POINTS_LAST_PDGAIN] = {1,1.4,1.3,1.13,1};
	double      intercept_scale[NUM_POINTS_LAST_PDGAIN] = {1,1,1,1,1};

	switch (mode) {
	case MODE_11a :
		devlibMode = MODE_11A;
		pd_gain   = CalSetup.xgain;
		myAtten	   = CalSetup.attenDutPM;
		CalSetup.TrgtPwrStartAddr += 5; // upto 10 piers
		if (CalSetup.pmModel == PM_436A) {
			sleep_interval = 150;
		}
		break;
	case MODE_11g :
		devlibMode = MODE_11G;
		pd_gain   = CalSetup.xgain_2p4[mode];
		myAtten	   = CalSetup.attenDutPM_2p4[mode];
		CalSetup.TrgtPwrStartAddr += 2; // upto 4 piers
		if (CalSetup.pmModel == PM_436A) {
			sleep_interval = 150;
		}

		//Measurement variables set for Channel Power measurements
		rbw = 100e3; // Resolution Bandwidth 
		vbw = 100e3; // Video Bandwidth  
		span = 60e6; // span 
		chsp = 11e6; // Channel Spacing
		chbw = 22e6; // Channel Bandwidth
		break;
	case MODE_11b :
		devlibMode = MODE_11B;
		pd_gain   = CalSetup.xgain_2p4[mode];
		myAtten	   = CalSetup.attenDutPM_2p4[mode];
		CalSetup.TrgtPwrStartAddr += 2; // upto 4 piers
		if (CalSetup.pmModel == PM_436A) {
			sleep_interval = 250;
		}
		
		//Measurement variables set for Channel Power measurements
		rbw = 100e3; // Resolution Bandwidth 
		vbw = 30e3;  // Video Bandwidth  
		span = 60e6; // span 
		chsp = 11e6; // Channel Spacing
		chbw = 22e6; // Channel Bandwidth
		break;
	default:
		uiPrintf("Unknown mode supplied to measure_all_channels_gen5 : %d \n", mode);
		REWIND_TEST = TRUE;
		break;
	}

	//configSetup.antenna = DESC_ANT_B |  USE_DESC_ANT | DESC_ANT_A;

	//++PD++ generalized 4 pd_adc_gain supported -- 
	if (CalSetup.cal_mult_xpd_gain_mask[mode] == 0) {
			jj = 0;
			pRawDataset_gen5[mode]->xpd_mask = (1 << XPD_GAIN1_GEN5) | (1 << XPD_GAIN2_GEN5);
			xgain_list[0] = XPD_GAIN1_GEN5;
			xgain_list[1] = XPD_GAIN2_GEN5;
	} else {
		jj = 0;
		pRawDataset_gen5[mode]->xpd_mask = (A_UINT16) CalSetup.cal_mult_xpd_gain_mask[mode];
		pd_gain = CalSetup.cal_mult_xpd_gain_mask[mode];
		kk = 0;
		ll = 0;
		while (ll < 4) {
			if ((pd_gain & 0x8) == 0x8) {
				if (kk >= 4) {
					uiPrintf("ERROR: A maximum of 4 pd_gains allowed to be specified in the eep file.\n");
					exit(0);
				}
				xgain_list[kk++] = (MAX_NUM_PDGAINS_PER_CHANNEL - 1) - ll;
			}
			ll++;
			pd_gain = (pd_gain << 1);
		}
	}

	local_num_pd_gains = kk;
	if (local_num_pd_gains != pRawDataset_gen5[mode]->pDataPerChannel[0].numPdGains) {
		uiPrintf("ERROR: num_pd_gains [%d] from xpd_mask [0x%x] does not match with the dataset [%d]\n", local_num_pd_gains,
			CalSetup.cal_mult_xpd_gain_mask[mode], 
			pRawDataset_gen5[mode]->pDataPerChannel[0].numPdGains);
		exit(0);
	}
	if (local_num_pd_gains != CalSetup.numPdGains) {
		uiPrintf("ERROR: num_pd_gains [%d] from xpd_mask [0x%x] does not match with the CalSetup.numPdGains in the .eep file [%d]\n", local_num_pd_gains,
			CalSetup.cal_mult_xpd_gain_mask[mode], 
			CalSetup.numPdGains);
		exit(0);
	}

	CalSetup.TrgtPwrStartAddr += wordsForPdgains[local_num_pd_gains-1]*pRawDataset_gen5[mode]->numChannels;
	configSetup.eepromLoad = 0;
	art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
					(A_BOOL)configSetup.eepromHeaderLoad, (A_UCHAR)devlibMode, configSetup.use_init);		

						
	uiPrintf("\nCollecting raw data for the adapter for mode %s\n", modeName[mode]);

	for (ii=0; ii<pRawDataset_gen5[mode]->numChannels; ii++)
	{
		if((ii == 1) && (!doneThrowAwayCal)) {
			ii = 0;
			doneThrowAwayCal = 1;
		}

		channel = pRawDataset_gen5[mode]->pDataPerChannel[ii].channelValue;

		if (ii == 0)
		{
			art_resetDevice(devNum, txStation, NullID, channel, 0);
		} else
		{
			art_resetDevice(devNum, txStation, NullID, channel, 0);
//			art_changeChannel(devNum, channel); // for efficiency
		}

		if(doneThrowAwayCal) {
			uiPrintf("ch: %d  --> ", channel);		
		}

		art_writeField(devNum, "bb_force_pdadc_gain", 1);
		// lowest pdadc_gain for max power
		art_writeField(devNum, "bb_forced_pdadc_gain", xgain_list[0]);

		art_forceSinglePowerTxMax(devNum, 0);
		art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, 
					    rates[0], DESC_ANT_A | USE_DESC_ANT | DESC_ANT_B); //Siavash Added B
		usleep(1000*50); // sleep 20 ms

		pcdac = 63;
		art_ForceSinglePCDACTableGriffin(devNum, pcdac, 60); // use max gain setting
		usleep(1000*sleep_interval);
		
		if (CalSetup.pmModel != NO_PM){
			maxPower = pmMeasAvgPower(devPM, reset) + myAtten;	
		}else{
			// rbw,vbw,span, chsp, chbw;
			maxPower = saChanPwr( devSA, channel*1e6, span, -10, rbw, vbw,chbw, chsp,CalSetup.saModel, reset);
			maxPower += myAtten;
		}

		if (maxPower < 10) {
			usleep(1000*sleep_interval);
			if (CalSetup.pmModel != NO_PM){
				maxPower = pmMeasAvgPower(devPM, reset) + myAtten;				
			}else{
				maxPower = saChanPwr( devSA, channel*1e6, span, -10, rbw, vbw,chbw, chsp,CalSetup.saModel, reset);
				maxPower += myAtten;
			}
		}

		art_txContEnd(devNum);
		if (maxPower < 10) {
			uiPrintf("The max transmit Power is too small (%3.2f) at ch = %d. Giving up.\n", maxPower, channel);
			REWIND_TEST = TRUE;
			return;
		}

		if(doneThrowAwayCal) {
			if (CalSetup.customerDebug) {
				uiPrintf("maxPower = %3.2f", maxPower);
			} else {
				uiPrintf("max pwr is %3.2f dBm\n", maxPower);
			}
		}
		pRawDataset_gen5[mode]->pDataPerChannel[ii].maxPower_t4 = (A_UINT16)(4*maxPower + 0.5);				

		// Measure for a pcdac and determine the correlation between pcdac and power
		anchor_pcdac = 22;
		//FJC: try not to get the first pcdac measurement -ve. 
		findFirstAnchor = TRUE;
		while (findFirstAnchor) {
			art_ForceSinglePCDACTable(devNum, anchor_pcdac);

			// First get power for a setting that corresponds to almost the mid of the range required
			art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, 
						DataRate[configSetup.dataRateIndex], configSetup.antenna);
			usleep(1000*sleep_interval);
			
		if (CalSetup.pmModel != NO_PM){
				txPower = pmMeasAvgPower(devPM, reset) + myAtten;
		}else{
			txPower = saChanPwr( devSA, channel*1e6, span, -10, rbw, vbw,chbw, chsp,CalSetup.saModel, reset);
			txPower += myAtten;
		}
			if (txPower < 5) {
				usleep(1000*sleep_interval);
				if (CalSetup.pmModel != NO_PM){
					txPower = pmMeasAvgPower(devPM, reset) + myAtten;				
				}else{
					txPower = saChanPwr( devSA, channel*1e6, span, -10, rbw, vbw,chbw, chsp,CalSetup.saModel, reset);
					txPower += myAtten;
				}
			}
			art_txContEnd(devNum);
                        //uiPrintf("First Power measured: %f\n", txPower);		//++JC++
		
			//check to see if first anchor is -ve if so try to get +ve
			if(txPower < 0) {
				anchor_pcdac += 10;
			}
			else {
				findFirstAnchor = FALSE;
			}
		}

		//Try to get near mid power - 10dBm, or maxpower - 15 if maxpower > 23 dBm
		if (maxPower > 23) {
			target_anchor_power = ((A_UINT16)maxPower - 12);
		}

		anchor_pcdac = anchor_pcdac - 2 * ((A_UINT16)txPower - target_anchor_power);
		if(anchor_pcdac > 63) {
			uiPrintf("The first Power measurement is too small (%f) at ch = %d. Giving up.\n", txPower, channel);
			REWIND_TEST = TRUE;
			return;
		}

		//FJC: try to get the anchor pcdac power within +/- 1dbm of mid power
		refineAnchor = TRUE;
		while(refineAnchor) {
			art_ForceSinglePCDACTable(devNum, anchor_pcdac);

			// First get power for a setting that corresponds to almost the mid of the range required
			art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, 
						DataRate[configSetup.dataRateIndex], configSetup.antenna);
			usleep(1000*sleep_interval);
			if (CalSetup.pmModel != NO_PM){
				txPower = pmMeasAvgPower(devPM, reset) + myAtten;
			}else{
				txPower = saChanPwr( devSA, channel*1e6, span, -10, rbw, vbw,chbw, chsp,CalSetup.saModel, reset);
				txPower += myAtten;
			}
			if (txPower < 5) {
				usleep(1000*sleep_interval);
				if (CalSetup.pmModel != NO_PM){
					txPower = pmMeasAvgPower(devPM, reset) + myAtten;				
				}else{
					txPower = saChanPwr( devSA, channel*1e6, span, -10, rbw, vbw,chbw, chsp,CalSetup.saModel, reset);
					txPower += myAtten;
				}
			}
			art_txContEnd(devNum);
//uiPrintf("Anchor Power measured: %f\n", txPower);		//++JC++
//uiPrintf("Anchor pcdac : %d\n", anchor_pcdac);		//++JC++

			if( (txPower > target_anchor_power + 1)	|| (txPower < target_anchor_power - 1)) {
				//further refine the anchor to get closer to target
				anchor_pcdac = anchor_pcdac - 2 * ((A_UINT16)txPower - target_anchor_power);
			} else {
				refineAnchor = FALSE;
			}
		}
		
		//adjustment needed for 11g 
		if((mode == MODE_11g) && (!isNala(swDeviceID)) ) {
			txPower -= 0.5;
		}

		anchor_power_t4 = (A_INT16)((4 * txPower) + 0.5);
//uiPrintf("Debug:: Anchor_power_t4: %d\n", anchor_power_t4);  //++JC++

		power_pcdac_offset_t4 = (2 * anchor_pcdac) - anchor_power_t4;

		// Setup the lowest power and deltas of power from the lowest for each xpd gain
/*		initial_power[0] = 0;		// 0 dBm for first xpd gain
		initial_power[1] = 7;		// 7 dBm for second xpd gain
		power_delta[0] = 3;		// 3 dB steps for first xpd gain
		power_delta[1] = 4;		// 4 dB steps for second xpd gain
*/
		jj = 0;		
		while (jj < CalSetup.numPdGains) {
			if (jj < 1) {
				initial_power[0] = 0;
			} else {
//				initial_power[jj] = (CalSetup.pdGainBoundary[jj-1] - CalSetup.pdGainOverlap/2)/2.0;
			    initial_power[jj] = (CalSetup.pdGainBoundary[jj-1] - MEAS_OVERLAP_HALF_DB)/2.0;
			}

			if (jj == (CalSetup.numPdGains - 1)) {

				power_delta[jj] = ((0.8 + (double)(CalSetup.pdGainBoundary[jj] +
//									CalSetup.pdGainOverlap/2 - 2*initial_power[jj]) / (NUM_POINTS_LAST_PDGAIN - 1)))/2;
									MEAS_OVERLAP_HALF_DB - 2*initial_power[jj]) / (NUM_POINTS_LAST_PDGAIN - 1)))/2;
//				printf("SNOOP: last jj=%d, up=%d, init=%3.1f, ovr=%d, numPt=%d\n", jj, CalSetup.pdGainBoundary[jj], initial_power[jj], 
//				CalSetup.pdGainOverlap, NUM_POINTS_LAST_PDGAIN);
			} else {
				power_delta[jj] = ((0.8 + (double)(CalSetup.pdGainBoundary[jj] +
//									CalSetup.pdGainOverlap/2 - 2*initial_power[jj]) / (NUM_POINTS_OTHER_PDGAINS - 1)))/2;
									MEAS_OVERLAP_HALF_DB - 2*initial_power[jj]) / (NUM_POINTS_OTHER_PDGAINS - 1)))/2;
//				printf("SNOOP: earlier jj=%3.1f, up=%d, init=%d, ovr=%d, numPt=%d\n", jj, CalSetup.pdGainBoundary[jj], initial_power[jj], 
//				CalSetup.pdGainOverlap, NUM_POINTS_OTHER_PDGAINS);
			}
//			printf("SNOOP: jj=%d : initial_power = %3.1f, power_delta = %3.1f\n", jj, initial_power[jj],
//				power_delta[jj]);
			jj++;			
		}
		
		for (jj=0; jj < CalSetup.numPdGains; jj++) {
			initial_pcdac[jj] = ((A_INT32)(4 * initial_power[jj]) + power_pcdac_offset_t4)/2;		// Get pcdac correspoding to initial pwr
			offset = 30;
			if ((initial_pcdac[jj] < 0) || (initial_pcdac[jj] > 63)){
				offset = 30 + initial_pcdac[jj];
				initial_pcdac[jj] = 0;
			}

			pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].pd_gain = (A_UINT16)(xgain_list[jj]);

			if (jj == (CalSetup.numPdGains - 1)){	// For the last xpd gain we have one more than the others
				pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].numVpd = NUM_POINTS_LAST_PDGAIN;
			} else {
				pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].numVpd = NUM_POINTS_OTHER_PDGAINS;
			}
			art_writeField(devNum, "bb_force_pdadc_gain", 1);
			art_writeField(devNum, "bb_forced_pdadc_gain", xgain_list[jj]);

			for (ll=0; ll < pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].numVpd; ll++) {

				backoff_step = 0;
				backoff_power = TRUE; // mechanism to ensure Vpd_delta < 64

				while (backoff_power) {
					current_pcdac = (A_UINT16)(initial_pcdac[jj] + (ll * 2 * power_delta[jj] * intercept_scale[ll]) - backoff_step);
//					printf("SNOOP: curr_pcdac = %d, scale = %3.1f\n", current_pcdac, intercept_scale[ll]);

	/*
					if (current_pcdac > 63) {
						current_pcdac = 63;
					}
	*/

					art_ForceSinglePCDACTableGriffin(devNum, current_pcdac, (A_UINT16)offset);
					art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, 
								DataRate[configSetup.dataRateIndex], configSetup.antenna);
					usleep(1000*sleep_interval);
					power_to_store_t4 = (A_UINT16)((2 * (current_pcdac - (30 - offset))) - power_pcdac_offset_t4);

	/* ++JC++ */
	//				if ((power_to_store_t4) > 48) {		// Measure for > 15 dBm - remove later if possible
					if (!isNala(swDeviceID))
					{
    				if ((power_to_store_t4) > 4*(maxPower - 10)) {		// Measure for > (maxPower - 10) dBm - remove later if possible
						if (CalSetup.pmModel != NO_PM){
							txPower = pmMeasAvgPower(devPM, reset) + myAtten;
						}else{
							txPower = saChanPwr( devSA, channel*1e6, span, -10, rbw, vbw,chbw, chsp,CalSetup.saModel, reset);
							txPower += myAtten;
						}
						if (txPower < 5) {
							usleep(1000*sleep_interval);
							if (CalSetup.pmModel != NO_PM){
								txPower = pmMeasAvgPower(devPM, reset) + myAtten;				
							}else{
								txPower = saChanPwr( devSA, channel*1e6, span, -10, rbw, vbw,chbw, chsp,CalSetup.saModel, reset);
								txPower += myAtten;
							}
						}
						
						//adjustment needed for 11g
							if((mode == MODE_11g) && (!isNala(swDeviceID)) ){
							txPower -= 0.5;
						}
		//					uiPrintf("\nPower measured: %f\n", txPower);		//++JC++
							power_to_store_t4 = (A_INT16)((txPower * 4) + 0.5);
						}
					}
					else
					{
						if (power_to_store_t4 > 12)
						{
							if (CalSetup.pmModel != NO_PM){
								txPower = pmMeasAvgPower(devPM, reset) + myAtten;
							}else{
								txPower = saChanPwr( devSA, channel*1e6, span, -10, rbw, vbw,chbw, chsp,CalSetup.saModel, reset);
								txPower += myAtten;
							}
							if (txPower < 5) {
								usleep(1000*sleep_interval);
								if (CalSetup.pmModel != NO_PM){
									txPower = pmMeasAvgPower(devPM, reset) + myAtten;
								}else{
									txPower = saChanPwr( devSA, channel*1e6, span, -10, rbw, vbw,chbw, chsp,CalSetup.saModel, reset);
									txPower += myAtten;								}
							}
						
						//	uiPrintf("\nPower measured: %f\n", txPower);		//++JC++
						power_to_store_t4 = (A_INT16)((txPower * 4) + 0.5);
     				}
					}
	/*++JC++ */

					//loop round several pdadc values and take the max
/*					maxPdadc = 0;
					accPdadc = 0;
					for (ww=0; ww < 5; ww++) {
						rddata = art_regRead(devNum, 0xa264);
						pdadc = (A_UINT16)(((rddata >> 1) & 0xff));
						//printf("Pdadc = %d\n", pdadc);
						Sleep(1);
						if(pdadc > maxPdadc) 
						{ 
							maxPdadc = pdadc; 
						}
						accPdadc = (accPdadc + pdadc);
					}
					//double check that we did not get a spurious max value, compare against average
					if(maxPdadc > (accPdadc/ww + 2)) 
					{ 
						maxPdadc = accPdadc/ww; 
					}
//					printf("Maxpdadc = %d, avePdadc = %d\n", maxPdadc, accPdadc/ww);
*/					art_txContEnd(devNum);
					rddata = art_regRead(devNum, 0xa264);
//		uiPrintf("\nPower measured: %f, pdadc = %d\n", (float)power_to_store_t4/4, (rddata >> 1) & 0xff);		//++JC++
					if (power_to_store_t4 < 0) {power_to_store_t4 = 0;}
					pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].pwr_t4[ll]  = (A_INT16)(power_to_store_t4);
					pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll]  = /*maxPdadc;*/ (A_UINT16)(((rddata >> 1) & 0xff));
					if (ll > 0) {
						if((pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll] -
						 pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll-1]) > 63){
							backoff_power = TRUE; // mechanism to ensure Vpd_delta < 64
							backoff_step += 1;    // 0.5dB step
						} else if((pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].pwr_t4[ll] -
						 pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].pwr_t4[ll-1]) > 28){
							backoff_power = TRUE; // mechanism to ensure Vpd_delta < 64
							backoff_step += (pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].pwr_t4[ll] -
											 pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].pwr_t4[ll-1] - 28)/2;    // 0.5dB step
//							printf("SNOOP: backoff_step = %d\n", backoff_step);
						} else {
							backoff_power = FALSE;
						}
					}
					else {
						backoff_power = FALSE;
					}

                    //only fix on low power (high gain) curve
                    if(CalSetup.fixZeroPowerDuringCal && (xgain_list[jj] == 3)) {
                        if(pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll] < 15) {
                            pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll] = 15;
                        }
                    
                        if(ll > 0) {
                            if(pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll] <
                                pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll - 1] + 8) {
                            
                                pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll] = 
                                      pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll - 1] + 8;
                            }
                        }
                    }

//uiPrintf("Pcdac: %d \tAvg Out: %d \tPower_t4: %d \n", current_pcdac, pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].Vpd[ll], pRawDataset_gen5[mode]->pDataPerChannel[ii].pDataPerPDGain[jj].pwr_t4[ll]);	//++JC++ debug
				}
			}
		}

//++JC++

		if (CalSetup.customerDebug) {
			uiPrintf("\n");
		}

	} // end channel loop

}


A_BOOL setup_datasets_for_cal_gen5(A_UINT32 devNum, A_UINT32 mode) {
	
	A_UINT16	myNumRawChannels ; 
	A_UINT16	*pMyRawChanList ;
	A_BOOL		read_from_file = CalSetup.readFromFile;
	A_BOOL		force_piers = CalSetup.forcePiers;


	if (mode != MODE_11a) {
		read_from_file		= CalSetup.readFromFile_2p4[mode];
		myNumRawChannels	= 3; 
		force_piers		= CalSetup.forcePiers_2p4[mode];
	}

	
	// handle forcePierList case here
	if(force_piers && !read_from_file) // 'cause read from file supercedes
	{
		myNumRawChannels = (A_UINT16) ((mode == MODE_11a) ? CalSetup.numForcedPiers : CalSetup.numForcedPiers_2p4[mode]);
		if ((mode == MODE_11b) && (CalSetup.useOneCal)) {
			myNumRawChannels = 3; // 2412, 2472, 2484
		}
		pMyRawChanList	 = ((mode == MODE_11a) ? CalSetup.piersList : CalSetup.piersList_2p4[mode]);
	} else {
		uiPrintf("Automatic pier computation for gen5 not supported yet. Please specify the forced_piers_list\n");
	}

	pRawDataset_gen5[mode]->xpd_mask = (A_UINT16) CalSetup.cal_mult_xpd_gain_mask[mode];
	if (!setup_raw_dataset_gen5(devNum, pRawDataset_gen5[mode], myNumRawChannels, pMyRawChanList)) {
		uiPrintf("Could not setup raw dataset for gen5 cal for mode %d\n", mode);
		return(0);
	}

	pRawGainDataset_gen5[mode]->xpd_mask = (A_UINT16)CalSetup.cal_mult_xpd_gain_mask[mode];
	if (!setup_raw_dataset_gen5(devNum, pRawGainDataset_gen5[mode], myNumRawChannels, pMyRawChanList)) {
		uiPrintf("Could not setup raw gainF dataset for gen5 cal for mode %d\n", mode);
		return(0);
	}

	if (!setup_EEPROM_dataset_gen5(devNum, pCalDataset_gen5[mode], myNumRawChannels, pMyRawChanList)) {
		uiPrintf("Could not setup cal dataset for gen5 cal for mode %d\n", mode);
		return(0);
	}

	return(1);
}

A_BOOL dump_raw_data_to_file_gen5( RAW_DATA_STRUCT_GEN5 *pRawData, char *filename )
{
	A_UINT16		i, j, kk;
    FILE			*fStream;
//	A_UINT16		single_xpd = 0xDEAD;
	A_UINT16		xgain_list[4];
	A_UINT16		xpd_mask;

	xgain_list[0] = 0xDEAD;
	xgain_list[1] = 0xDEAD;
	xgain_list[2] = 0xDEAD;
	xgain_list[3] = 0xDEAD;
 
	kk = 0;
	xpd_mask = pRawData->xpd_mask;

	for (j = 0; j < MAX_NUM_PDGAINS_PER_CHANNEL; j++) {
		if (((xpd_mask >> j) & 1) > 0) {
			if (kk > 3) {
				uiPrintf("ERROR: A maximum of 4 xpd_gains supported in raw data\n");
				exit(0);
			}
			xgain_list[kk++] = (A_UINT16) j;			
		}
	}

	if (CalSetup.customerDebug)
		uiPrintf("\nWriting raw data to file %s\n", filename);
	
    if( (fStream = fopen( filename, "w")) == NULL ) {
        uiPrintf("Failed to open %s for writing - giving up\n", filename);
        return FALSE;
    }

	fprintf(fStream, "XPD_Gain_mask = 0x%x\n\n", pRawData->xpd_mask);

	//print the frequency values
	fprintf(fStream, "freq    pwr1_x1  pwr2_x1  pwr3_x1  pwr4_x1  pwr5_x1");
	if (xgain_list[3] != 0xDEAD) {
		fprintf(fStream, "  pwr1_x3 pwr2_x3 pwr3_x3 pwr4_x3\n");
		fprintf(fStream, "        [vpd]    [vpd]    [vpd]    [vpd]   [vpd]    [vpd]    [vpd]   [vpd]   [vpd]\n");
	} else {
		fprintf(fStream, "\n        [vpd]    [vpd]    [vpd]    [vpd]    [vpd]    \n");
	}

	for (i = 0; i < pRawData->numChannels; i++) {
		fprintf(fStream, "%d\t", pRawData->pChannels[i]);
		j = xgain_list[0];
		for (kk=0; kk<pRawData->pDataPerChannel[i].pDataPerPDGain[j].numVpd; kk++) {
			fprintf(fStream, "%3.2f    ", (double)(pRawData->pDataPerChannel[i].pDataPerPDGain[j].pwr_t4[kk]/4.0));
		}
		if (xgain_list[3] != 0xDEAD) {
			j = xgain_list[3];
			for (kk=0; kk<pRawData->pDataPerChannel[i].pDataPerPDGain[j].numVpd; kk++) {
				fprintf(fStream, "%3.2f    ", (double)(pRawData->pDataPerChannel[i].pDataPerPDGain[j].pwr_t4[kk]/4.0));
			}
		}
		fprintf(fStream, "%3.2f    ", (double)(pRawData->pDataPerChannel[i].maxPower_t4/4.0));
		fprintf(fStream,"\n\t");		
		for (kk=0; kk<pRawData->pDataPerChannel[i].pDataPerPDGain[j].numVpd; kk++) {
			fprintf(fStream, "    [%3d]", pRawData->pDataPerChannel[i].pDataPerPDGain[j].Vpd[kk]);
		}
		if (xgain_list[3] != 0xDEAD) {
			j = xgain_list[3];
			for (kk=0; kk<pRawData->pDataPerChannel[i].pDataPerPDGain[j].numVpd; kk++) {
				fprintf(fStream, "    [%3d]", pRawData->pDataPerChannel[i].pDataPerPDGain[j].Vpd[kk]);
			}
		}
		fprintf(fStream,"\n");		
	}
	
	fclose(fStream);
	return TRUE;
}

A_BOOL raw_to_eeprom_dataset_gen5(RAW_DATA_STRUCT_GEN5 *pRawDataset, EEPROM_DATA_STRUCT_GEN5 *pCalDataset) {

	A_UINT16	ii, jj, kk;
	EEPROM_DATA_PER_CHANNEL_GEN5	*pCalCh;
	RAW_DATA_PER_CHANNEL_GEN5		*pRawCh;
	A_UINT16		xgain_list[MAX_NUM_PDGAINS_PER_CHANNEL];
	A_UINT16		xpd_mask, pd_gain;
	double			Vpd_I_diff, pwr_I_delta_t4, pwr_I_diff_t4, Vpd_I_delta;
	double			vpd_delta_diff, pwr_delta_delta_t4, pwr_delta_diff_t4, vpd_delta_delta;
	A_INT16			pwr_current_t4, vpd_current;
	A_BOOL			success = TRUE;

	xgain_list[0] = 0xDEAD;
	xgain_list[1] = 0xDEAD;
 
	pCalDataset->xpd_mask  = pRawDataset->xpd_mask;

	kk = 0;
	xpd_mask = pRawDataset->xpd_mask;

	for (jj = 0; jj < MAX_NUM_PDGAINS_PER_CHANNEL; jj++) {
		if (((xpd_mask >> jj) & 1) > 0) {
			if (kk > (MAX_NUM_PDGAINS_PER_CHANNEL - 1)) {
				uiPrintf("A maximum of %d xpd_gains supported in raw data\n", MAX_NUM_PDGAINS_PER_CHANNEL);
				exit(0);
			}
			xgain_list[kk++] = (A_UINT16) jj;			
		}
	}

	pCalDataset->numChannels = pRawDataset->numChannels;
	for (ii = 0; ii < pCalDataset->numChannels; ii++) {
		pCalCh = &(pCalDataset->pDataPerChannel[ii]);
		pRawCh = &(pRawDataset->pDataPerChannel[ii]);
		pCalCh->channelValue = pRawCh->channelValue;
		pCalCh->maxPower_t4  = pRawCh->maxPower_t4;
		pCalCh->numPdGains  = pRawCh->numPdGains;

		for (jj = 0; jj < pCalCh->numPdGains; jj++) {
			pd_gain = xgain_list[jj];
			pCalCh->pwr_I[jj]   = (A_INT16)((pRawCh->pDataPerPDGain[jj].pwr_t4[0] + 2)/4);
			pwr_I_delta_t4   = ((4*pCalCh->pwr_I[jj]) - pRawCh->pDataPerPDGain[jj].pwr_t4[0]);
			Vpd_I_diff  = (pRawCh->pDataPerPDGain[jj].Vpd[1] - pRawCh->pDataPerPDGain[jj].Vpd[0]);
			pwr_I_diff_t4  = (pRawCh->pDataPerPDGain[jj].pwr_t4[1] - pRawCh->pDataPerPDGain[jj].pwr_t4[0]);
			if (pwr_I_diff_t4 != 0) {
				Vpd_I_delta  = ((Vpd_I_diff*pwr_I_delta_t4)/pwr_I_diff_t4) + 0.5;
			} else {
				Vpd_I_delta  = 0;
			}
			pCalCh->Vpd_I[jj]   = (pRawCh->pDataPerPDGain[jj].Vpd[0] + (A_INT16)Vpd_I_delta);

			pwr_current_t4 = 4*pCalCh->pwr_I[jj];
			vpd_current = pCalCh->Vpd_I[jj];
			for (kk = 0; kk < (pRawCh->pDataPerPDGain[jj].numVpd - 1); kk++) {
				pCalCh->pwr_delta_t2[kk][jj]   = (A_INT16)((pRawCh->pDataPerPDGain[jj].pwr_t4[kk+1] - pwr_current_t4 + 1 )/2);
				pwr_current_t4 = pwr_current_t4 + (2 * pCalCh->pwr_delta_t2[kk][jj]);

				pwr_delta_delta_t4   = pwr_current_t4 - pRawCh->pDataPerPDGain[jj].pwr_t4[kk+1];
				vpd_delta_diff  = (pRawCh->pDataPerPDGain[jj].Vpd[kk+1] - pRawCh->pDataPerPDGain[jj].Vpd[kk]);
				pwr_delta_diff_t4  = (pRawCh->pDataPerPDGain[jj].pwr_t4[kk+1] - pRawCh->pDataPerPDGain[jj].pwr_t4[kk]);
				if (pwr_delta_diff_t4 != 0) {
					vpd_delta_delta  = ((vpd_delta_diff*pwr_delta_delta_t4)/pwr_delta_diff_t4) + 0.5;
				} else {
					vpd_delta_delta  = 0;
				}

				if (pRawCh->pDataPerPDGain[jj].Vpd[kk+1] < vpd_current) {
					pCalCh->Vpd_delta[kk][jj] = 0;
				} else {
				pCalCh->Vpd_delta[kk][jj]      = (pRawCh->pDataPerPDGain[jj].Vpd[kk+1] - vpd_current) + (A_INT16)vpd_delta_delta;
				}
				vpd_current = vpd_current + pCalCh->Vpd_delta[kk][jj];
			}
		}
	}
	return success;
}

void fill_words_for_eeprom_gen5(A_UINT32 *word, A_UINT16 numWords, A_UINT16 *fbin, 
							 A_UINT16 dbmmask, A_UINT16 pcdmask, A_UINT16 freqmask)
{

	A_UINT16 idx = 0;
	EEPROM_DATA_PER_CHANNEL_GEN5 *pCalCh;
	A_UINT16	ii;
	A_UINT16	numPiers;
	A_UINT16	pwr_mask = 0x1F;	// 5 bits
	A_UINT16	vpd_mask = 0x7F;	// 7 bits
	A_UINT16	pwr_delta_mask = 0xF;	// 4 bits
	A_UINT16	vpd_delta_mask = 0x3F;	// 6 bits
	A_UINT16    i, num_words;

	//fill in dummy eep_map 1 data for driver's that does not know how to
	//crunch the eep_map = 2 data
	for (i = 0; i < NUM_DUMMY_EEP_MAP1_LOCATIONS; i++) {
		word[idx++] = dummpy_eep_map1_data[i];
	}

	// Group 1. 11a Frequency pier locations
	if(CalSetup.calPower && (CalSetup.Amode|| CalSetup.enableJapanMode11aNew))
	{
		word[idx++] = ( (fbin[1] & freqmask) << 8) | (fbin[0] & freqmask)  ;
		word[idx++] = ( (fbin[3] & freqmask) << 8) | (fbin[2] & freqmask)  ;
		word[idx++] = ( (fbin[5] & freqmask) << 8) | (fbin[4] & freqmask)  ;
		word[idx++] = ( (fbin[7] & freqmask) << 8) | (fbin[6] & freqmask)  ;
		word[idx++] = ( (fbin[9] & freqmask) << 8) | (fbin[8] & freqmask)  ;
	} else {
	}

	//Group 2. 11a calibration data for all frequency piers
	if(CalSetup.calPower && (CalSetup.Amode|| CalSetup.enableJapanMode11aNew))
	{
		
		for (ii=0; ii<pCalDataset_gen5[MODE_11a]->numChannels; ii++)
		{
			pCalCh = &(pCalDataset_gen5[MODE_11a]->pDataPerChannel[ii]);

			num_words = fill_eeprom_words_for_curr_ch_gen5(&(word[idx]), pCalCh);
			idx += num_words;

/*
			word[idx++] = ( ( (pCalCh->pwr_I[0] & pwr_mask) << 0) |
							( (pCalCh->Vpd_I[0] & vpd_mask) << 5) |
							( (pCalCh->pwr_delta_t2[0][0] & pwr_delta_mask) << 12 ));

			word[idx++] = ( ( (pCalCh->Vpd_delta[0][0] & vpd_delta_mask) << 0) |
							( (pCalCh->pwr_delta_t2[1][0] & pwr_delta_mask) << 6) |
							( (pCalCh->Vpd_delta[1][0] & vpd_delta_mask) << 10));

			word[idx++] = ( ( (pCalCh->pwr_delta_t2[2][0] & pwr_delta_mask) << 0) |
							( (pCalCh->Vpd_delta[2][0] & vpd_delta_mask) << 4) |
							( (pCalCh->pwr_I[1] & pwr_mask) << 10) |
							( (pCalCh->Vpd_I[1] & vpd_mask) << 15 ));

			word[idx++] = ( ( (pCalCh->Vpd_I[1] & vpd_mask) >> 1) |
							( (pCalCh->pwr_delta_t2[0][1] & pwr_delta_mask) << 6) |
							( (pCalCh->Vpd_delta[0][1] & vpd_delta_mask) << 10));

			word[idx++] = (	( (pCalCh->pwr_delta_t2[1][1] & pwr_delta_mask) << 0 ) |
							( (pCalCh->Vpd_delta[1][1] & vpd_delta_mask) << 4) |
							( (pCalCh->pwr_delta_t2[2][1] & pwr_delta_mask) << 10) |
							( (pCalCh->Vpd_delta[2][1] & vpd_delta_mask) << 14));

			word[idx++] = ( ( (pCalCh->Vpd_delta[2][1] & vpd_delta_mask) >> 2) |
							( (pCalCh->pwr_delta_t2[3][1] & pwr_delta_mask) << 4) |
							( (pCalCh->Vpd_delta[3][1] & vpd_delta_mask) << 8));
*/
		}
	} else
	{
	}

	// Group 3. 11b Frequency pier locations for eep_map = 2
	if(CalSetup.calPower && CalSetup.Bmode)
	{
		word[idx++] = (freq2fbin(CalSetup.piersList_2p4[MODE_11b][0]) & 0xff) |
			(( (CalSetup.numForcedPiers_2p4[MODE_11b] > 1) ? (freq2fbin(CalSetup.piersList_2p4[MODE_11b][1]) & 0xFF) : 0) << 8);
		if (CalSetup.numForcedPiers_2p4[MODE_11b] > 2) {
			word[idx++] = (freq2fbin(CalSetup.piersList_2p4[MODE_11b][2]) & 0xff) |
				(( (CalSetup.numForcedPiers_2p4[MODE_11b] > 3) ? (freq2fbin(CalSetup.piersList_2p4[MODE_11b][3]) & 0xFF) : 0) << 8);
		} else {
			word[idx++] =  0 ;
		}
	} else {
	}

	//Group 4. 11b Calibration Information 
	if(CalSetup.calPower && CalSetup.Bmode)
	{

		numPiers = pCalDataset_gen5[MODE_11b]->numChannels;
		for (ii=0; ii<numPiers; ii++)
		{
			fbin[ii] = freq2fbin(pCalDataset_gen5[MODE_11b]->pChannels[ii]) ;
		}
		if (numPiers < NUM_PIERS_2p4) {
			for (ii=numPiers; ii<NUM_PIERS_2p4; ii++) {
				fbin[ii] = 0;
			}
		}

		for (ii=0; ii<pCalDataset_gen5[MODE_11b]->numChannels; ii++)
		{
			pCalCh = &(pCalDataset_gen5[MODE_11b]->pDataPerChannel[ii]);

			num_words = fill_eeprom_words_for_curr_ch_gen5(&(word[idx]), pCalCh);
			idx += num_words;

/*
			word[idx++] = ( ( (pCalCh->pwr_I[0] & pwr_mask) << 0) |
							( (pCalCh->Vpd_I[0] & vpd_mask) << 5) |
							( (pCalCh->pwr_delta_t2[0][0] & pwr_delta_mask) << 12 ));

			word[idx++] = ( ( (pCalCh->Vpd_delta[0][0] & vpd_delta_mask) << 0) |
							( (pCalCh->pwr_delta_t2[1][0] & pwr_delta_mask) << 6) |
							( (pCalCh->Vpd_delta[1][0] & vpd_delta_mask) << 10));

			word[idx++] = ( ( (pCalCh->pwr_delta_t2[2][0] & pwr_delta_mask) << 0) |
							( (pCalCh->Vpd_delta[2][0] & vpd_delta_mask) << 4) |
							( (pCalCh->pwr_I[1] & pwr_mask) << 10) |
							( (pCalCh->Vpd_I[1] & vpd_mask) << 15 ));

			word[idx++] = ( ( (pCalCh->Vpd_I[1] & vpd_mask) >> 1) |
							( (pCalCh->pwr_delta_t2[0][1] & pwr_delta_mask) << 6) |
							( (pCalCh->Vpd_delta[0][1] & vpd_delta_mask) << 10));

			word[idx++] = (	( (pCalCh->pwr_delta_t2[1][1] & pwr_delta_mask) << 0 ) |
							( (pCalCh->Vpd_delta[1][1] & vpd_delta_mask) << 4) |
							( (pCalCh->pwr_delta_t2[2][1] & pwr_delta_mask) << 10) |
							( (pCalCh->Vpd_delta[2][1] & vpd_delta_mask) << 14));

			word[idx++] = ( ( (pCalCh->Vpd_delta[2][1] & vpd_delta_mask) >> 2) |
							( (pCalCh->pwr_delta_t2[3][1] & pwr_delta_mask) << 4) |
							( (pCalCh->Vpd_delta[3][1] & vpd_delta_mask) << 8));

*/
		}
	} else
	{
	}


	// Group 5. 11g Frequency pier locations for eep_map = 2
	if(CalSetup.calPower && CalSetup.Gmode)
	{
		word[idx++] = (freq2fbin(CalSetup.piersList_2p4[MODE_11g][0]) & 0xff) |
			(( (CalSetup.numForcedPiers_2p4[MODE_11g] > 1) ? (freq2fbin(CalSetup.piersList_2p4[MODE_11g][1]) & 0xFF) : 0) << 8);
		if (CalSetup.numForcedPiers_2p4[MODE_11g] > 2) {
			word[idx++] = (freq2fbin(CalSetup.piersList_2p4[MODE_11g][2]) & 0xff) |
				(( (CalSetup.numForcedPiers_2p4[MODE_11g] > 3) ? (freq2fbin(CalSetup.piersList_2p4[MODE_11g][3]) & 0xFF) : 0) << 8);
		} else {
			word[idx++] =  0;
		}
	} else {
	}
	
	//Group 6. 11g Calibration Piers and Calibration Information 
	if(CalSetup.calPower && CalSetup.Gmode)
	{
		numPiers = pCalDataset_gen5[MODE_11g]->numChannels;
		for (ii=0; ii<numPiers; ii++)
		{
			fbin[ii] = freq2fbin(pCalDataset_gen5[MODE_11g]->pChannels[ii]) ;
		}
		if (numPiers < NUM_PIERS_2p4) {
			for (ii=numPiers; ii<NUM_PIERS_2p4; ii++) {
				fbin[ii] = 0;
			}
		}

		for (ii=0; ii<pCalDataset_gen5[MODE_11g]->numChannels; ii++)
		{
			pCalCh = &(pCalDataset_gen5[MODE_11g]->pDataPerChannel[ii]);

			num_words = fill_eeprom_words_for_curr_ch_gen5(&(word[idx]), pCalCh);
			idx += num_words;

/*
			word[idx++] = ( ( (pCalCh->pwr_I[0] & pwr_mask) << 0) |
							( (pCalCh->Vpd_I[0] & vpd_mask) << 5) |
							( (pCalCh->pwr_delta_t2[0][0] & pwr_delta_mask) << 12 ));

			word[idx++] = ( ( (pCalCh->Vpd_delta[0][0] & vpd_delta_mask) << 0) |
							( (pCalCh->pwr_delta_t2[1][0] & pwr_delta_mask) << 6) |
							( (pCalCh->Vpd_delta[1][0] & vpd_delta_mask) << 10));

			word[idx++] = ( ( (pCalCh->pwr_delta_t2[2][0] & pwr_delta_mask) << 0) |
							( (pCalCh->Vpd_delta[2][0] & vpd_delta_mask) << 4) |
							( (pCalCh->pwr_I[1] & pwr_mask) << 10) |
							( (pCalCh->Vpd_I[1] & vpd_mask) << 15 ));

			word[idx++] = ( ( (pCalCh->Vpd_I[1] & vpd_mask) >> 1) |
							( (pCalCh->pwr_delta_t2[0][1] & pwr_delta_mask) << 6) |
							( (pCalCh->Vpd_delta[0][1] & vpd_delta_mask) << 10));

			word[idx++] = (	( (pCalCh->pwr_delta_t2[1][1] & pwr_delta_mask) << 0 ) |
							( (pCalCh->Vpd_delta[1][1] & vpd_delta_mask) << 4) |
							( (pCalCh->pwr_delta_t2[2][1] & pwr_delta_mask) << 10) |
							( (pCalCh->Vpd_delta[2][1] & vpd_delta_mask) << 14));

			word[idx++] = ( ( (pCalCh->Vpd_delta[2][1] & vpd_delta_mask) >> 2) |
							( (pCalCh->pwr_delta_t2[3][1] & pwr_delta_mask) << 4) |
							( (pCalCh->Vpd_delta[3][1] & vpd_delta_mask) << 8));
*/
		}
	} else
	{
	}
//printf("SNOOP: fill_wrds_gen5 : startidx = 0x%x\n", idx);
	fill_Target_Power_and_Test_Groups(&(word[idx]), (A_UINT16)(numWords - idx + 1), dbmmask, pcdmask, freqmask);
}

A_UINT16 fill_eeprom_words_for_curr_ch_gen5(A_UINT32 *word, EEPROM_DATA_PER_CHANNEL_GEN5 *pCalCh)
{
	A_UINT16	pwr_mask = 0x1F;	// 5 bits
	A_UINT16	vpd_mask = 0x7F;	// 7 bits
	A_UINT16	pwr_delta_mask = 0xF;	// 4 bits
	A_UINT16	vpd_delta_mask = 0x3F;	// 6 bits
	A_UINT16 idx = 0;

	word[idx++] = ( ( (pCalCh->pwr_I[0] & pwr_mask) << 0) |
					( (pCalCh->Vpd_I[0] & vpd_mask) << 5) |
					( (pCalCh->pwr_delta_t2[0][0] & pwr_delta_mask) << 12 ));

	word[idx++] = ( ( (pCalCh->Vpd_delta[0][0] & vpd_delta_mask) << 0) |
					( (pCalCh->pwr_delta_t2[1][0] & pwr_delta_mask) << 6) |
					( (pCalCh->Vpd_delta[1][0] & vpd_delta_mask) << 10));

	if (pCalCh->numPdGains < 2) {  // 5 points if last pd_gain
		word[idx++] = ( ( (pCalCh->pwr_delta_t2[2][0] & pwr_delta_mask) << 0) |
						( (pCalCh->Vpd_delta[2][0] & vpd_delta_mask) << 4) |
						( (pCalCh->pwr_delta_t2[3][0] & pwr_delta_mask) << 10) |
						( (pCalCh->Vpd_delta[3][0] & 0x3) << 14));

		word[idx++] = ( ( (pCalCh->Vpd_delta[3][0] >> 2) & 0xF) << 0);

		return(idx); // idx = 4
	} else {
		word[idx++] = ( ( (pCalCh->pwr_delta_t2[2][0] & pwr_delta_mask) << 0) |
						( (pCalCh->Vpd_delta[2][0] & vpd_delta_mask) << 4) |
						( (pCalCh->pwr_I[1] & pwr_mask) << 10) |
						( (pCalCh->Vpd_I[1] & vpd_mask) << 15 ));

		word[idx++] = ( ( (pCalCh->Vpd_I[1] & vpd_mask) >> 1) |
						( (pCalCh->pwr_delta_t2[0][1] & pwr_delta_mask) << 6) |
						( (pCalCh->Vpd_delta[0][1] & vpd_delta_mask) << 10));
	}

	word[idx++] = (	( (pCalCh->pwr_delta_t2[1][1] & pwr_delta_mask) << 0 ) |
					( (pCalCh->Vpd_delta[1][1] & vpd_delta_mask) << 4) |
					( (pCalCh->pwr_delta_t2[2][1] & pwr_delta_mask) << 10) |
					( (pCalCh->Vpd_delta[2][1] & 0x3) << 14));

	if (pCalCh->numPdGains < 3) {
		word[idx++] = ( ( (pCalCh->Vpd_delta[2][1] & vpd_delta_mask) >> 2) |
						( (pCalCh->pwr_delta_t2[3][1] & pwr_delta_mask) << 4) |
						( (pCalCh->Vpd_delta[3][1] & vpd_delta_mask) << 8));

		return(idx); // idx = 6
	} else {
		word[idx++] = ( ( (pCalCh->Vpd_delta[2][1] & vpd_delta_mask) >> 2) |
						( (pCalCh->pwr_I[2] & pwr_mask) << 4) |
						( (pCalCh->Vpd_I[2] & vpd_mask) << 9));
	}

	word[idx++] = (	( (pCalCh->pwr_delta_t2[0][2] & pwr_delta_mask) << 0 ) |
					( (pCalCh->Vpd_delta[0][2] & vpd_delta_mask) << 4) |
					( (pCalCh->pwr_delta_t2[1][2] & pwr_delta_mask) << 10) |
					( (pCalCh->Vpd_delta[1][2] & 0x3) << 14));

	if (pCalCh->numPdGains < 4) {
		word[idx++] = (	( (pCalCh->Vpd_delta[1][2] & vpd_delta_mask) >> 2) |
						( (pCalCh->pwr_delta_t2[2][2] & pwr_delta_mask) << 4) |
						( (pCalCh->Vpd_delta[2][2] & vpd_delta_mask) << 8) |
						( (pCalCh->pwr_delta_t2[3][2] & 0x3) << 14));

		word[idx++] = ( ( (pCalCh->pwr_delta_t2[3][2] & pwr_delta_mask) >> 2) |
						( (pCalCh->Vpd_delta[3][2] & vpd_delta_mask) << 2) );

		return(idx); // idx = 9
	} else {
		word[idx++] = (	( (pCalCh->Vpd_delta[1][2] & vpd_delta_mask) >> 2) |
						( (pCalCh->pwr_delta_t2[2][2] & pwr_delta_mask) << 4) |
						( (pCalCh->Vpd_delta[2][2] & vpd_delta_mask) << 8) |
						( (pCalCh->pwr_I[3] & 0x3) << 14) );

		word[idx++] = ( ( (pCalCh->pwr_I[3] & pwr_mask) >> 2) |
						( (pCalCh->Vpd_I[3] & vpd_mask) << 3) |
						( (pCalCh->pwr_delta_t2[0][3] & pwr_delta_mask) << 10) |
						( (pCalCh->Vpd_delta[0][3] & 0x3) << 14));
	}

	word[idx++] = ( ( ( pCalCh->Vpd_delta[0][3] & vpd_delta_mask) >> 2) |
					( ( pCalCh->pwr_delta_t2[1][3] & pwr_delta_mask) << 4) |
					( ( pCalCh->Vpd_delta[1][3] & vpd_delta_mask) << 8) |
					( ( pCalCh->pwr_delta_t2[2][3] & 0x3) << 14) );

	word[idx++] = ( ( ( pCalCh->pwr_delta_t2[2][3] & pwr_delta_mask) >> 2) |
					( ( pCalCh->Vpd_delta[2][3] & vpd_delta_mask) << 2) |
					( ( pCalCh->pwr_delta_t2[3][3] & pwr_delta_mask) << 8) |
					( ( pCalCh->Vpd_delta[3][3] & 0xF) << 12));

	word[idx++] = ((pCalCh->Vpd_delta[3][3] & vpd_delta_mask) >> 4);

	return(idx);

}

//++JC++
/*

void read_dataset_from_file_gen5(RAW_DATA_STRUCT_GEN5 *pDataSet, char *filename) {

	uiPrintf("Reading raw data from file not supported for gen5 yet. Please provide force_piers_list\n");
	exit(0);

}

void golden_iq_cal(A_UINT32 devNum, A_UINT32 mode, A_UINT32 channel) {

	A_UINT32  maxIter = 40;
	A_UINT32  iter = 0;
	A_UINT32  complete_timeout = 4000;
	A_BOOL    TransmitOn = FALSE;

	configSetup.eepromLoad = 1;
	art_setResetParams(devNum, configSetup.pCfgFile, (A_BOOL)configSetup.eepromLoad,
						(A_BOOL)configSetup.eepromHeaderLoad, (A_UCHAR)mode, configSetup.use_init);		

	art_resetDevice(devNum, txStation, NullID, channel, 0);
	art_forceSinglePowerTxMax(devNum, 10);

	waitForAck(CalSetup.customerDebug);
	while (!verifyAckStr("Done with iq_cal for mode") && (iter++ < (maxIter + 1))) {
		if (REWIND_TEST) {
			if (TransmitOn) {
				art_txContEnd(devNum);
				TransmitOn = FALSE;
			}
			return;
		}
		//art_txDataSetup(devNum, IQ_CAL_RATEMASK, rxStation, 4*IQ_CAL_NUMPACKETS, 100, pattern, 2, 0, 2, 1);
		if (!TransmitOn) {
			art_txContBegin(devNum, CONT_FRAMED_DATA, RANDOM_PATTERN, IQ_CAL_RATE, DESC_ANT_A | USE_DESC_ANT);
			TransmitOn = TRUE;
		}

		Sleep(100);
		sendAck(devNum, "Ready to TX for iq_cal at", mode, channel, 0,CalSetup.customerDebug);
//		Sleep(30);
//		art_txDataBegin(devNum, complete_timeout, 0);
		waitForAck(CalSetup.customerDebug);
		if (verifyAckStr("Failed iq_cal. Rewind Test")) {				
			uiPrintf("\n\n*************** Failed iq_cal for mode %s. Retry. ********************\n",modeName[calModeFor[mode]]);
			uiPrintf("recv Str = %s\n", ackRecvStr);
			REWIND_TEST = TRUE;
		}
	}
	if (TransmitOn) {
		art_txContEnd(devNum);
		TransmitOn = FALSE;
	}
	sendAck(devNum, "Consent to quit iq_cal", 0, 0, 0, CalSetup.customerDebug);
}

void dut_iq_cal(A_UINT32 devNum, A_UINT32 mode, A_UINT32 channel) {
	
// mode is the devlibmode here

	A_UINT32 iter = 0;
	A_UINT32 maxIter = 40;
	A_UINT32 iterRX = 0;
	A_UINT32 maxIterRX = 200;
//	double pwr_meas_i = 0;
//	double pwr_meas_q = 0;
	A_UINT32 pwr_meas_i = 0;
	A_UINT32 pwr_meas_q = 0;
	A_UINT32 iq_corr_meas = 0, iq_corr_neg = 0;
	A_UINT32 i_coeff, q_coeff;
	A_UINT32 rddata;

	uiPrintf("\nPerforming iq_cal for mode %s :", modeName[calModeFor[mode]]);

	sendAck(devNum, "Initiate iq_cal at mode", mode, channel, 0, CalSetup.customerDebug);

//	art_resetDevice(devNum, rxStation, NullID, channel, 0);

	while ((iter++ < maxIter) && 
		((pwr_meas_i < 0x0FFFFFFF) || (pwr_meas_q < 0x0FFFFFFF) || (iq_corr_meas == 0)) ){

		art_resetDevice(devNum, rxStation, NullID, channel, 0);
		
		sendAck(devNum, "Ready to RX for iq_cal at", mode, channel, iter, CalSetup.customerDebug);
		waitForAck(CalSetup.customerDebug) ;
		if (!verifyAckStr("Ready to TX for iq_cal at")) {
			uiPrintf("ERROR: Got out of sync in iq_cal [ackStr: %s]\n", ackRecvStr);
			exit(0);
		}

		Sleep(10);

		art_regWrite(devNum, 0x9920, 0x1f000) ; // enable iq_cal and set sample size to 11
		rddata = art_regRead(devNum, 0x9920);
		rddata = (rddata >> 16) & 0x1;

		iterRX = 0;
		while ((rddata == 0x1) && (iterRX++ < maxIterRX)){
			rddata = art_regRead(devNum, 0x9920);
			rddata = (rddata >> 16) & 0x1;
			Sleep(10);
//			uiPrintf("[%d] rddata = %d\n", iterRX, rddata);
			
		}

		if (iterRX == maxIterRX) {
			uiPrintf("iq_cal iter : %d.\n", iter);
		}

		pwr_meas_i = 0;
		pwr_meas_q = 0;
		iq_corr_meas = 0;

		pwr_meas_i   = art_regRead(devNum, (0x9800 + (260 << 2))) ;
		pwr_meas_q   = art_regRead(devNum, (0x9800 + (261 << 2))) ;
		iq_corr_meas = art_regRead(devNum, (0x9800 + (262 << 2))) ;

//	uiPrintf("SNOOP: pwr_I = 0x%x, pwr_Q = 0x%x, IQ_Corr = 0x%x\n", pwr_meas_i, pwr_meas_q, iq_corr_meas);

		iq_corr_neg = 0;


	}

	if ((iter > maxIter) && 
		((pwr_meas_i < 0x0FFFFFFF) || (pwr_meas_q < 0x0FFFFFFF) || (iq_corr_meas == 0)) ){
			uiPrintf("\n\n*************** Failed iq_cal for mode %s. Retry. ********************\n",modeName[calModeFor[mode]]);
			REWIND_TEST = TRUE;
			sendAck(devNum, "Failed iq_cal. Rewind Test", 0, 0, 0, CalSetup.customerDebug);
			return;
	}

	if (iq_corr_meas >= 0x80000000) {
		iq_corr_meas = 0xFFFFFFFF - iq_corr_meas;
		iq_corr_neg = 1;
	}


    i_coeff	= (A_UINT32)floor((128.0*(double)iq_corr_meas)/(((double)pwr_meas_i+(double)pwr_meas_q)/2.0) + 0.5);
    q_coeff = (A_UINT32)floor((((double)pwr_meas_i/(double)pwr_meas_q) - 1.0)*128.0  + 0.5);

//	uiPrintf("SNOOP: interm i_coeff = %d, q_coeff = %d\n", i_coeff, q_coeff);


	if (CalSetup.customerDebug > 0) {
		uiPrintf("pwr_I = %d, pwr_Q = %d, IQ_Corr = %d\n", pwr_meas_i, pwr_meas_q, iq_corr_meas);
	}

	// print i_coeff/q_coeff before truncation.
	uiPrintf("      i_coeff = %d, q_coeff = %d\n", i_coeff, q_coeff);

    i_coeff = (i_coeff & 0x3f);
    if (iq_corr_neg == 0x0) {
        i_coeff = 0x40 - i_coeff;
    }
    q_coeff = q_coeff & 0x1f;

//	uiPrintf("      i_coeff = %d, q_coeff = %d\n", i_coeff, q_coeff);
	
    rddata = art_regRead(devNum, 0x9920);
    rddata = rddata | (1 << 11) | (i_coeff << 5) | q_coeff;
    art_regWrite(devNum, 0x9920, rddata);

	CalSetup.iqcal_i_corr[calModeFor[mode]] = i_coeff;
	CalSetup.iqcal_q_corr[calModeFor[mode]] = q_coeff;

	sendAck(devNum, "Done with iq_cal for mode", mode, channel, 0, CalSetup.customerDebug);
	waitForAck(CalSetup.customerDebug);
}

// returns numWords and eeprom cal data for desired mode : MODE_11a, _11b, _11g
// starts filling array "word" from idx=0, so feed the pointer to the beginning of where you need to start filling
void get_cal_info_for_mode_gen5(A_UINT32 *word, A_UINT16 numWords, A_UINT16 *fbin, 
							 A_UINT16 dbmmask, A_UINT16 pcdmask, A_UINT16 freqmask, A_UINT32 mode)
{

	A_UINT16    idx = 0;
	EEPROM_DATA_PER_CHANNEL_GEN5 *pCalCh;
	A_UINT16	ii;
	A_UINT16	pcdac_delta_mask = 0x1F;
	A_UINT16	pcdac_mask = 0x3F;

	// ideally want a more stringent check to ensure not to walk off the end of array
	// but don't know how many words remain in "word" array. this is the absolute upper limit.
	assert(numWords < 0x400) ;

	if(mode == MODE_11a)
	{
		// Group 1. 11a Frequency pier locations	
		word[idx++] = ( (fbin[1] & freqmask) << 8) | (fbin[0] & freqmask)  ;
		word[idx++] = ( (fbin[3] & freqmask) << 8) | (fbin[2] & freqmask)  ;
		word[idx++] = ( (fbin[5] & freqmask) << 8) | (fbin[4] & freqmask)  ;
		word[idx++] = ( (fbin[7] & freqmask) << 8) | (fbin[6] & freqmask)  ;
		word[idx++] = ( (fbin[9] & freqmask) << 8) | (fbin[8] & freqmask)  ;

		//Group 2. 11a calibration data for all frequency piers		
		for (ii=0; ii<pCalDataset_gen5[MODE_11a]->numChannels; ii++)
		{
			pCalCh = &(pCalDataset_gen5[MODE_11a]->pDataPerChannel[ii]);

			word[idx++] = ( ( (pCalCh->pwr1_xg0 & dbmmask) << 0) |
							( (pCalCh->pwr2_xg0 & dbmmask) << 8) );

			word[idx++] = ( ( (pCalCh->pwr3_xg0 & dbmmask) << 0) |
							( (pCalCh->pwr4_xg0 & dbmmask) << 8) );

			word[idx++] = ( ( (pCalCh->pcd2_delta_xg0 & pcdac_delta_mask) << 0) |
							( (pCalCh->pcd3_delta_xg0 & pcdac_delta_mask) << 5) |
							( (pCalCh->pcd4_delta_xg0 & pcdac_delta_mask) << 10) );

			word[idx++] = ( ( (pCalCh->pwr1_xg3 & dbmmask) << 0) |
							( (pCalCh->pwr2_xg3 & dbmmask) << 8) );

			word[idx++] = ( ( (pCalCh->pwr3_xg3 & dbmmask) << 0) |
//							( (pCalCh->maxPower_t4 & dbmmask) << 8) );
							( (pCalCh->pcd1_xg0 & pcdac_mask) << 8) ); // starting eeprom 4.3
		}

		if (idx > numWords) {
			uiPrintf("ERROR: get_cal_info_for_mode_gen5: ran over expected numwords [%d] in 11a cal data : %d (actual)\n", idx, numWords);
			exit(0);
		}
	} 

	
	if(mode == MODE_11b)
	{
		//Group 3. 11b Calibration Piers and Calibration Information 
		for (ii=0; ii<pCalDataset_gen5[MODE_11b]->numChannels; ii++)
		{
			pCalCh = &(pCalDataset_gen5[MODE_11b]->pDataPerChannel[ii]);

			word[idx++] = ( ( (pCalCh->pwr1_xg0 & dbmmask) << 0) |
							( (pCalCh->pwr2_xg0 & dbmmask) << 8) );

			word[idx++] = ( ( (pCalCh->pwr3_xg0 & dbmmask) << 0) |
							( (pCalCh->pwr4_xg0 & dbmmask) << 8) );

			word[idx++] = ( ( (pCalCh->pcd2_delta_xg0 & pcdac_delta_mask) << 0) |
							( (pCalCh->pcd3_delta_xg0 & pcdac_delta_mask) << 5) |
							( (pCalCh->pcd4_delta_xg0 & pcdac_delta_mask) << 10) );

			word[idx++] = ( ( (pCalCh->pwr1_xg3 & dbmmask) << 0) |
							( (pCalCh->pwr2_xg3 & dbmmask) << 8) );

			word[idx++] = ( ( (pCalCh->pwr3_xg3 & dbmmask) << 0) |
//							( (pCalCh->maxPower_t4 & dbmmask) << 8) );
							( (pCalCh->pcd1_xg0 & pcdac_mask) << 8) ); // starting eeprom 4.3
		}

		if (idx > numWords) {
			uiPrintf("ERROR: get_cal_info_for_mode_gen5: ran over expected numwords [%d] in 11b cal data : %d (actual)\n", idx, numWords);
			exit(0);
		}
	} 	

	if(mode == MODE_11g)
	{
		//Group 4. 11g Calibration Piers and Calibration Information 
		for (ii=0; ii<pCalDataset_gen5[MODE_11g]->numChannels; ii++)
		{
			pCalCh = &(pCalDataset_gen5[MODE_11g]->pDataPerChannel[ii]);

			word[idx++] = ( ( (pCalCh->pwr1_xg0 & dbmmask) << 0) |
							( (pCalCh->pwr2_xg0 & dbmmask) << 8) );

			word[idx++] = ( ( (pCalCh->pwr3_xg0 & dbmmask) << 0) |
							( (pCalCh->pwr4_xg0 & dbmmask) << 8) );

			word[idx++] = ( ( (pCalCh->pcd2_delta_xg0 & pcdac_delta_mask) << 0) |
							( (pCalCh->pcd3_delta_xg0 & pcdac_delta_mask) << 5) |
							( (pCalCh->pcd4_delta_xg0 & pcdac_delta_mask) << 10) );

			word[idx++] = ( ( (pCalCh->pwr1_xg3 & dbmmask) << 0) |
							( (pCalCh->pwr2_xg3 & dbmmask) << 8) );

			word[idx++] = ( ( (pCalCh->pwr3_xg3 & dbmmask) << 0) |
//							( (pCalCh->maxPower_t4 & dbmmask) << 8) );
							( (pCalCh->pcd1_xg0 & pcdac_mask) << 8) ); // starting eeprom 4.3
		}

		if (idx > numWords) {
			uiPrintf("ERROR: get_cal_info_for_mode_gen5: ran over expected numwords [%d] in 11g cal data : %d (actual)\n", idx, numWords);
			exit(0);
		}
	} 
}
*/
