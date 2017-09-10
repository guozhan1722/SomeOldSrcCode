/*
 * $Id: //depot/sw/branches/ART_V53/sw/src/include/xr.h#1 $
 *
 * This file contains the function prototypes, helpful macros and data
 * structure declarations specific to the transmit functions.  It is the
 * same for both the station and access point.
 *
 * Copyright ?2000-2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

#ifndef _XR_H_
#define _XR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * values supplied by HW design - should be
 * considered to be part of the XR spec!
 */
#define XR_SLOT_DELAY                   30      // in usec
#define XR_DATA_DETECT_DELAY            40      // in usec
#define XR_CHIRP_DUR                    24      // in usec

#define XR_AIFS                         0
/* XR uplink should have same cwmin/cwmax value */
#define XR_LOG_CWMIN_CWMAX              3

#define XR_MAX_FRAME_SIZE               AR_BUF_SIZE

#define XR_DNLINK_QUEUE_AIFS_SHIFT      0
#define XR_DNLINK_QUEUE_CWMIN_SHIFT     1
#define XR_BEACON_FACTOR                3

#define XR_DEFAULT_RATE                 250
#define XR_DEFAULT_RATE_INDEX           0
#define XR_MAX_RATE_INDEX               4

/* pick the threshold so that we meet most of the regulatory constraints */
#define XR_FRAGMENTATION_THRESHOLD            540
#define XR_TELEC_FRAGMENTATION_THRESHOLD      442

#define MAX_GRP_POLL_PERIOD             1000 /* Maximum Group Poll Periodicity */

/*
 * Maximum Values in ms for group poll periodicty
 */
#define GRP_POLL_PERIOD_NO_XR_STA_MAX       100
#define GRP_POLL_PERIOD_XR_STA_MAX          30

 /*
 * Percentage of the configured poll periodicity
 */
#define GRP_POLL_PERIOD_FACTOR_NO_XR_STA  100 /* No XR Stations Associated use the prog value */
#define GRP_POLL_PERIOD_FACTOR_XR_STA     30  /* When XR Stations associated freq is 30% higher */

/*
 * Macros to obtain the Group Poll Periodicty in various situations
 *
 * Curerntly there are the two cases
 * (a) When there are no XR STAs associated
 * (b) When there is atleast one XR STA associated
 */
#define GRP_POLL_PERIOD_NO_XR_STA(_pDev) (_pDev)->staConfig.xrPollInterval
#define GRP_POLL_PERIOD_XR_STA(_pDev)                                                   \
        A_MAX(GRP_POLL_PERIOD_FACTOR_XR_STA * (_pDev)->staConfig.xrPollInterval / 100,  \
              GRP_POLL_PERIOD_XR_STA_MAX)

/*
 * When there are no XR STAs and a valid double chirp is received then the Group Polls are
 * transmitted for 10 seconds from the time of the last valid double double-chirp
 */
#define NO_XR_STA_GRPPOLL_TX_DUR    10000
/*
 * CTS protection frame is needed to protect from other stations in
 * the normal BSS - so it needs to go out at the multicast rate for
 * the normal BSS - both for XR mode and maybe 'g' mode operation
 */
#define XR_CTS_RATE(_pDev)                                  \
(                                                           \
    (_pDev)->baseBss->bss.pRateTable->info[                 \
       (_pDev)->xrCtsRateIdx                                \
    ].rateCode                                              \
)

/*
 * Should account for sifs + grp poll time + max backoff
 * time at the station + uplink XR data frame + sifs +
 * downlink XR ack
 */
#define XR_UPLINK_TRANSACTION_TIME(_pDev, _dataLen)         \
(                                                           \
    PHY_COMPUTE_TX_TIME(                                    \
        (_pDev)->xrBss->bss.pRateTable,                     \
        sizeof(WLAN_DATA_MAC_HEADER3) + FCS_FIELD_SIZE,     \
        (_pDev)->xrBss->bss.defaultRateIndex, 0             \
    )                                                       \
    + ((XR_AIFS +                                           \
        LOG_TO_CW(XR_LOG_CWMIN_CWMAX)) * XR_SLOT_DELAY)     \
    + PHY_COMPUTE_TX_TIME(                                  \
        (_pDev)->xrBss->bss.pRateTable,                     \
        (_dataLen),                                         \
        (_pDev)->xrBss->bss.defaultRateIndex, 0             \
    )                                                       \
    + PHY_COMPUTE_TX_TIME(                                  \
        (_pDev)->xrBss->bss.pRateTable,                     \
        WLAN_CTRL_FRAME_SIZE,                               \
        (_pDev)->xrBss->bss.defaultRateIndex, 0             \
    )                                                       \
)

#define XR_UPLINK_DEFAULT_TRANSACTION_TIME(_pDev)                               \
        XR_UPLINK_TRANSACTION_TIME(_pDev, 2*sizeof(WLAN_FRAME_AUTH))            \
      + PHY_COMPUTE_TX_TIME((_pDev)->baseBss->bss.pRateTable,                   \
        sizeof(WLAN_FRAME_CFEND) + FCS_FIELD_SIZE,                              \
        (_pDev)->baseBss->bss.defaultRateIndex,0                                \
        )

#define XR_UPLINK_DATA_TRANSACTION_TIME(_pDev)                          \
        XR_UPLINK_TRANSACTION_TIME(_pDev, XR_FRAGMENTATION_THRESHOLD)


#if defined(BUILD_AP)
#define isXrAp(_pDev)   (((_pDev)->staConfig.abolt & ABOLT_XR) && GET_XR_BSS((_pDev)))
#else
#define isXrAp(_pDev)   FALSE
#endif

/* 
 * toggle the 2nd bit in the first octet to set the xr magic bit 
 * in the mac address. As per the definition of the MAC address in 
 * IEEE standards, the same bit defines whether the address is a 
 * locally or universally administered address. This will ensure the 
 * BSSID (which is derived from the ethernet MAC address) does not 
 * collide with other global OUI domains.
 */
#define _OCTET(_pBssId) ((_pBssId)->octets[0])

#define _OCTET_MASK     0x02

#define setBaseBssIdMask(_pBssId)       do { _OCTET(_pBssId) &= ~_OCTET_MASK; } while (0)

#define setXrBssIdMask(_pBssId)         do { _OCTET(_pBssId) ^= _OCTET_MASK; } while (0)

/*
 * Get the BSSID of the BSS to report to the apps. Only the base BSSID
 * is reported.
 */
#define GET_REPORT_ASSOC_BSSID(_pDev, _pVportBss, _pAssocBssId)      do{                \
    if ((_pDev)->localSta->staState != STATE_QUIET) {                                   \
        *(_pAssocBssId) = (_pVportBss)->bss.bssId;                                      \
        if ((_pDev)->bssDescr && ((_pDev)->bssDescr->bsstype == INFRASTRUCTURE_BSS)) {  \
            setBaseBssIdMask((_pAssocBssId));                                           \
        }                                                                               \
    } else {                                                                            \
        *(_pAssocBssId) = broadcastMacAddr;                                             \
    }                                                                                   \
} while (0)

#define isXrBssRxFrame(_pDev, _pHdr)    (isXrAp(_pDev) &&                       \
                                         (_OCTET(&(_pHdr)->address1) ==         \
                                          _OCTET(&(_pDev)->xrBss->bss.bssId)))

#define isXrBssSib(_pDevInfo, _pSib)    ((_pSib)->pVportBss == ((_pDevInfo)->xrBss))

#define isXrRate(_pDev, _rate)          (((_rate) == 250)  ||    \
                                         ((_rate) == 500)  ||    \
                                         ((_rate) == 1000) ||    \
                                         ((_rate) == 2000) ||    \
                                         ((_rate) == 3000))

#define isXrOnlyRate(_pDev, _rate)      (((_rate) == 250)  ||    \
                                         ((_rate) == 500)  ||    \
                                         ((_rate) == 3000))

#define is11bOnlyRate(_pDev, _rate)     (((_rate) == 5500) ||    \
                                         ((_rate) == 11000))

#define XR_MIN_RATE_FOR_TELEC            1000

/*
 * Structures for tuning the XR Group Polls
 */

/*
 * The maximum number of the different Group Poll Rates that are not contiguous
 * in the Group Poll Queue. For e.g. if the Group Poll Queue has Group Polls
 * with the following rates, ( 0.25 0.25 1 1 1 0.25 3 3 6 ), then the number
 * of group poll rates is 5 (0.25 1 0.25 3 6 ).
 */
#define MAX_GROUP_POLL_RATES   16
#define MAX_NUM_GROUP_POLL     50 /* Allow a maximum of 50 Group Polls of a rate */

/*
 * The XR Group Poll Rate and Number of Group Polls for each rate
 * is entered as a string by the user. The XR Group Poll rates
 * cannot exceed 6Mbps.
 */ 
typedef struct groupPollPerRateInfo {
    A_UINT8  rateCode;
    A_UINT8  numPolls;
} GROUP_POLL_PER_RATE_INFO;

/*
 * XR Group Poll Rate Code Frequency Table;
 */
typedef struct xrGroupPollRateInfo {
        A_UINT8                  totalPolls;
        A_UINT8                  numEntries;
        GROUP_POLL_PER_RATE_INFO perRateInfo[MAX_GROUP_POLL_RATES];
} XR_GROUP_POLL_RATE_INFO;

/* 
 * number of descriptors needed to send _num group polls...
 * _num polls on each antenna (FIXED_A/FIXED_B) + 2 CF-END's
 */

#define XR_NUM_GROUP_POLL_DESCS(_num)  ((_num) * (ANTENNA_DUMMY_MAX - 1) + 2 ) 

/*
 * An XR Probe request is two double chirps. After the first double chirp is sent, there 
 * is a delay of XR_CHIRP_SEND_WAIT_TIME before a check is made to see if the next double
 * chirp can be sent. If the next one cant be sent, then a check is made every 
 * XR_CHIRP_SEND_WAIT_TIME_PER_LOOP for a max additional wait time of 
 * XR_CHIRP_SEND_EXTRA_WAIT_TIME.
 */
#define NUM_DCHIRP_PER_PROBE_REQ             2
#define XR_CHIRP_SEND_WAIT_TIME             60   /* microseconds */
#define XR_CHIRP_SEND_EXTRA_WAIT_TIME       20   /* microseconds */
#define XR_CHIRP_SEND_WAIT_TIME_PER_LOOP     2   /* microseconds */
/*
 * Time in ms between two double chirps to be considered a valid probe 
 * by the AP. If the second double chirp is received within this time,
 * it is considered a valid probe request by the AP.
 */
#define MAX_DCHIRP_RCV_INTERVAL  (XR_CHIRP_SEND_WAIT_TIME + XR_CHIRP_SEND_EXTRA_WAIT_TIME)
/*
 * Older STAs had a different interval between 2 Double Chirps. From measurements in open air,
 * the interval is about 218 uSec. The Older STAs here refer to the STAs that were based off 
 * 3.3.0.x release.
 */
#define MAX_DCHIRP_RCV_INTERVAL_OLD_STA_MIN 215
#define MAX_DCHIRP_RCV_INTERVAL_OLD_STA_MAX 225

/* 
 * It is possible that the 2 double chirps, sent as part of the XR probe request, could
 * be lost. The Group Polls are sent by the AP, only on receipt of a double double 
 * chirp. Increase the probability of receipt of the double-double chirps by sending more 
 * than 1 during JOIN.
 */
#define NUM_JOIN_XR_PROBE_REQ  1

#ifdef __cplusplus
}
#endif

#endif /* _XR_H_ */
