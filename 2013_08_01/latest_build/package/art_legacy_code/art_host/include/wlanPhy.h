/*
 * Copyright (c) 2000-2002 Atheros Communications, Inc., All Rights Reserved
 *
 * Definitions for core driver
 * This is a common header file for all platforms and operating systems.
 *
 * $Id: wlanPhy.h,v 1.1.1.1 2011/03/08 23:40:25 Siavash Exp $
 */


#ifndef _WLANPHY_H_
#define _WLANPHY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xr.h"

struct wlanDevInfo;
struct atherosDescRxStatus;
struct vportBss;
struct channelValues;
struct WlanRateSet;

typedef enum {
    WLAN_PHY_OFDM  = 0,
    WLAN_PHY_TURBO = 1,
    WLAN_PHY_CCK   = 2,
    WLAN_PHY_XR    = 3,

    WLAN_PHY_MAX
} WLAN_PHY;

/*
 * Rate Table structure for various modes - 'b', 'a', 'g', 'xr';
 * order of fields in info structure is important because hardcoded
 * structures are initialized within the hal for these
 */
#define RATE_TABLE_SIZE             32

typedef struct regDataLenTable {
    A_UINT8     numEntries;
    A_UINT16    frameLenRateIndex[RATE_TABLE_SIZE];
} REG_DATALEN_TABLE;

typedef struct RateTable {
    int         rateCount;
    A_UINT8     rateCodeToIndex[RATE_TABLE_SIZE]; // backward mapping
    struct {
        A_BOOL    valid;            // Valid for use in rate control
        WLAN_PHY  phy;              // CCK/OFDM/TURBO/XR
        A_UINT16  rateKbps;         // Rate in Kbits per second
        A_UINT16  userRateKbps;     // User rate in KBits per second
        A_UINT8   rateCode;         // rate that goes into hw descriptors
        A_UINT8   shortPreamble;    // Mask for enabling short preamble in rate code for CCK
        A_UINT8   dot11Rate;        // Value that goes into supported rates info element of MLME
        A_UINT8   controlRate;      // Index of next lower basic rate, used for duration computation
        A_RSSI    rssiAckValidMin;  // Rate control related information
        A_RSSI    rssiAckDeltaMin;  // Rate control related information
        A_UINT16  lpAckDuration;    // Long preamble ACK duration
        A_UINT16  spAckDuration;    // short preamble ACK duration
#ifdef MULTI_RATE_RETRY_ENABLE
        struct {
            A_UINT32  word4Retries;
            A_UINT32  word5Rates;
        } normalSched;
        struct {
            A_UINT32  word4Retries;
            A_UINT32  word5Rates;
        } shortSched;
        struct {
            A_UINT32  word4Retries;
            A_UINT32  word5Rates;
        } probeSched;
        struct {
            A_UINT32  word4Retries;
            A_UINT32  word5Rates;
        } probeShortSched;
#endif
    } info[RATE_TABLE_SIZE];
    A_UINT32    probeInterval;        // interval for ratectrl to probe for other rates
    A_UINT32    rssiReduceInterval;   // interval for ratectrl to reduce RSSI
    A_UINT8     regularToTurboThresh; // upperbound on regular (11a or 11g) mode's rate before switching to turbo
    A_UINT8     turboToRegularThresh; // lowerbound on turbo mode's rate before switching to regular
    A_UINT8     pktCountThresh;       // mode switch recommendation criterion: number of consecutive packets sent at rate beyond the rate threshold
    A_UINT8     initialRateMax;       // the initial rateMax value used in rcSibUpdate()
    A_UINT8     numTurboRates;        // number of Turbo rates in the rateTable
    REG_DATALEN_TABLE *regDataLenTable; // Frame Lengths that comply with Regulatory reqs.
} RATE_TABLE;

/*
 * Basic Rate Table structure 
 */
#define BASIC_RATE_TABLE_SIZE       32
typedef struct BasicRateTable {
    int         rateCount;
    A_UINT8     basicRates[RATE_TABLE_SIZE]; 
} BASIC_RATE_TABLE;

typedef enum {
    WIRELESS_MODE_11a   = 0,
    WIRELESS_MODE_TURBO = 1,
    WIRELESS_MODE_11b   = 2,
    WIRELESS_MODE_11g   = 3,
    WIRELESS_MODE_108g  = 4,
    WIRELESS_MODE_XR    = 5,

    WIRELESS_MODE_MAX
} WIRELESS_MODE;

typedef enum {
    CCK_RATE_MODE_LONG_RANGE,
    CCK_RATE_MODE_BASIC_RATES

} CCK_RATE_MODE;

A_STATUS
wlanSetCckRateMode(struct wlanDevInfo *pDev, CCK_RATE_MODE mode);

A_STATUS
wlanSet11gBasicRates(struct wlanDevInfo *pDev, A_UINT16 mode);

WIRELESS_MODE
wlanFindModeFromRateTable(struct wlanDevInfo *pDev, struct vportBss *pVportBss);

void
wlanUpdateAggressiveTxParams(struct wlanDevInfo *pDev);

void wlanInitStaWmeParams(struct wlanDevInfo *pDev);

void
wlanUpdatePhyChAPs(struct wlanDevInfo *pDev, struct vportBss *pVportBss, WIRELESS_MODE mode, A_BOOL reset);

A_STATUS
wlanSetupPhyInfo(struct wlanDevInfo *pDev, struct vportBss *pVportBss, 
                 WIRELESS_MODE mode, A_UINT16 defaultRate, A_UINT16 minRate);

void
wlanUpdateWirelessMode(struct vportBss *pVportBss, struct channelValues *pChval, 
                       A_BOOL keepRCContent);

void
wlanUpdateDefaultRate(struct wlanDevInfo *pdevInfo, A_UINT16 rateKbps);

void
wlanUpdateTelecXRMinRate(struct vportBss *pVportBss, struct channelValues *pChval,
                         A_UINT16 *minRateKbps);

A_UINT8
wlanRate2RateCode(const RATE_TABLE *pRateTable, A_UINT16 rateKbps);

const A_UCHAR *
wlanRateKbps2Str(A_UINT32 rateKbps);

A_UINT32
wlanStr2RateKbps(A_UCHAR *rateStr);

void
wlanRateTbl2RateSet(struct vportBss *pVportBss, RATE_TABLE *pRateTable, 
                    struct WlanRateSet *pRateSet);

void
wlanSortByPrefRateSet(struct WlanRateSet *pRateSet, const struct WlanRateSet *pPrefRates);

/*
 * The computation of transmit duration for a frame
 * depends on the PHY parameters - an array of funcs
 * corresponding to the PHY
 */
typedef A_UINT16 (*PHY_TX_TIME_FUNC)(const RATE_TABLE *, A_UINT32 frameLen, A_UINT16 rateIndex, A_BOOL flag);

extern PHY_TX_TIME_FUNC phyComputeTxTime[WLAN_PHY_MAX];

#define PHY_COMPUTE_TX_TIME(_pRateTbl, _frameLen, _rateIndex, _flag)   \
    phyComputeTxTime[(_pRateTbl)->info[(_rateIndex)].phy](_pRateTbl, _frameLen, _rateIndex, _flag)


/*
 * Probe request and response is sent at lowest basic rate,
 * and long preamble in CCK mode; index 0 works for all these
 * conditions
 */
#define PSPOLL_RATE_INDEX    0
#define LOWEST_RATE_INDEX    0
#define XR_PSPOLL_RATE_INDEX XR_DEFAULT_RATE_INDEX
#define XR_LOWEST_RATE_INDEX XR_DEFAULT_RATE_INDEX


/*
 * When to use short preamble for CCK?
 * AP logic may be broken - as we haven't supported a CCK AP
 * as yet! Station side uses it based on capInfo, but uses
 * long preamble for all probe request/response frames
 */
#ifdef BUILD_AP
#define USE_SHORT_PREAMBLE(_pDev, _pSib, _pHdr)        \
    (((!(_pSib)) || (_pSib)->capInfo.shortPreamble) && \
     (_pDev)->localSta->capInfo.shortPreamble)
#else
#define IS_PROBE_REQRESP_FRAME(_fc)                    \
    (((_fc)->fType == FRAME_MGT) &&                    \
    (((_fc)->fSubtype == SUBT_PROBE_REQ) || ((_fc)->fSubtype == SUBT_PROBE_RESP)))
#define USE_SHORT_PREAMBLE(_pDev, _pSib, _pHdr)        \
    ((_pDev)->localSta->capInfo.shortPreamble && (!IS_PROBE_REQRESP_FRAME(&(_pHdr)->frameControl)))
#endif


#define OPT_MSEC_TO_USEC(_ms)       (( _ms) << 10)
#define OPT_SEC_TO_USEC(_sec)       ((_sec) << 20)
#define OPT_USEC_TO_MSEC(_us)       (( _us) >> 10)

typedef enum {
    RADAR             = 5,
    OFDM_FALSE_DETECT = 17,
    CCK_FALSE_DETECT  = 25
} PHY_ERR_TYPE;

#ifdef BUILD_AP
#define RECORD_RADAR_EVENT(_pDev, _delta, _rssi, _width) {          \
    if ((_pDev)->staConfig.radarDetectEnable &&                     \
        wlanIsDfsChannel(_pDev,(_pDev)->staConfig.phwChannel))      \
    {                                                               \
        recordRadarEvent(_pDev, _delta, _rssi, _width);             \
    }                                                               \
}
#else
#define RECORD_RADAR_EVENT(_pDev, _delta, _rssi, _width)
#endif

/*
 * This structure is used to track the history corresponding
 * to a given type of phy error; the last triggerCount events
 * are remembered; duration captures the total time period over
 * which the last triggerCount events occured.. so that.. say
 * if the last 2500 false detects happened in less than one
 * second - itself captured in the triggerThreshold, the noise
 * immunity levels can be increased; events are remembered
 * by the time delta corresponding to the given event and the
 * immediately preceeding event - the method helps track the
 * duration rather easily; lastTimestamp and lastTick
 * correspond to the actual time of the last event so that
 * the delta for the next event can be computed - need to
 * keep the millisec tick from the system clock and the finer
 * grain timestamp in the rx descriptor; nextEvent corresponds
 * to the oldest event remembered - it is the event forgotten
 * on arrival of a new event
 */
#define PHY_ERR_MAX             3
#define PHY_ERR_HISTORY_MAX     2500
typedef struct phyErrRecord {
    void     (*triggerAction)(struct wlanDevInfo *, PHY_ERR_TYPE);
    A_UINT32 triggerThreshold;
    A_UINT   triggerCount;
    A_UINT32 duration;
    A_UINT32 lastTick;
    A_UINT32 lastTs;
    A_UINT   nextEvent;
    A_UINT32 delta[PHY_ERR_HISTORY_MAX];
} PHY_ERR_RECORD;

#define PHY_ERR_RECORD_LOOKUP(_pDev, _phyErr)                                   \
    (((_phyErr) == RADAR)             ? &(_pDev)->phyErrRecord[0] :             \
    (((_phyErr) == OFDM_FALSE_DETECT) ? &(_pDev)->phyErrRecord[1] :             \
    (((_phyErr) == CCK_FALSE_DETECT)  ? &(_pDev)->phyErrRecord[2] :             \
                                        NULL)))


#define ANI_SUPPORT     1

#ifdef ANI_SUPPORT
#define PHY_ERR_STATS_RESET(_pDev, _phyErr, _action, _freq, _period) {          \
    PHY_ERR_RECORD *p = PHY_ERR_RECORD_LOOKUP(_pDev, _phyErr);                  \
                                                                                \
    p->triggerAction    = _action;                                              \
    p->triggerThreshold = OPT_SEC_TO_USEC(_period);                             \
    p->triggerCount     = (_period) ? ((_freq) * (_period)) : (_freq);          \
    ASSERT(p->triggerCount);                                                    \
    ASSERT(p->triggerCount <= PHY_ERR_HISTORY_MAX);                             \
    p->duration  = 0;                                                           \
    if (((_phyErr) == RADAR) && (_pDev)->staConfig.friendlyDetectEnable) {      \
        p->lastTick = A_MS_TICKGET();                                           \
    } else {                                                                    \
        p->lastTick  = A_MS_TICKGET() - OPT_USEC_TO_MSEC(p->triggerThreshold);  \
    }                                                                           \
    p->nextEvent = 0;                                                           \
    A_MEM_ZERO(p->delta, p->triggerCount * sizeof(p->delta[0]));                \
}

/*
 * The delta is computed with a coarse resolution for large values of
 * delta (i.e. greater than 30ms); doing anything more precise will
 * a) require expensive arithmetic, and b) some way of calibrating
 * the system clock to the mac clock (across resets and all that) to
 * be cognizant of the wrap around point; the current algorithms for
 * ANI as well as RADAR detection don't require a finer resolution
 * so far - so the strategy works out OK for now
 */
#define PHY_ERR_REPORT(_pDev, _pRx, _pRxDesc) {                                 \
    PHY_ERR_RECORD *p = PHY_ERR_RECORD_LOOKUP(_pDev, (_pRx)->phyError);         \
                                                                                \
    if (p) {                                                                    \
        A_UINT32 curTick = A_MS_TICKGET();                                      \
        p->duration -= p->delta[p->nextEvent];                                  \
        p->delta[p->nextEvent] = OPT_MSEC_TO_USEC(curTick - p->lastTick);       \
        if (p->delta[p->nextEvent] < OPT_MSEC_TO_USEC(80)) {                    \
            p->delta[p->nextEvent] = ((_pRx)->timestamp - p->lastTs) & 0x7fff;  \
        }                                                                       \
        p->lastTick  = curTick;                                                 \
        p->lastTs    = (_pRx)->timestamp;                                       \
        p->duration += p->delta[p->nextEvent];                                  \
        if ((_pRx)->phyError == RADAR) {                                        \
            RECORD_RADAR_EVENT(_pDev, p->delta[p->nextEvent],                   \
                (A_UINT32)((_pRx)->rssi),                                       \
                ((_pRx)->dataLength)                                            \
                    ? (A_UINT32)(*((A_UINT8 *)(_pRxDesc)->pBufferVirtPtr.word)) \
                    : 0);                                                       \
        }                                                                       \
        p->nextEvent = (p->nextEvent + 1) % p->triggerCount;                    \
        if ((_pRx)->phyError == RADAR && (_pDev)->staConfig.friendlyDetectEnable) { \
            if (p->duration > p->triggerThreshold) {                            \
                if (p->triggerAction) {                                         \
                    p->triggerAction(_pDev, (_pRx)->phyError);                  \
                }                                                               \
            }                                                                   \
        } else {                                                                \
            if (p->duration < p->triggerThreshold) {                            \
                if (p->triggerAction) {                                         \
                    p->triggerAction(_pDev, (_pRx)->phyError);                  \
                }                                                               \
            }                                                                   \
        }                                                                       \
    }                                                                           \
}

#define PHY_ERR_STATS_TIME(_p)                                                  \
    ((A_MS_TICKGET() - (_p)->lastTick) + OPT_USEC_TO_MSEC((_p)->duration))

#define PHY_ERR_RESET(_pDev)            {                                       \
    if ((_pDev)->staConfig.radarDetectEnable) {                                 \
        PHY_ERR_STATS_RESET(_pDev, RADAR, NULL, 1, 0);                          \
    }                                                                           \
    aniReset(_pDev);                                                            \
}

#define PHY_ERR_POLL_FUNCTION(_pDev)    aniPollFunction(_pDev)

#else
#define PHY_ERR_STATS_RESET(a, b, c, d, e)
#define PHY_ERR_REPORT(a, b, c)
#define PHY_ERR_STATS_TIME(a)           1
#define PHY_ERR_RESET(a)
#define PHY_ERR_POLL_FUNCTION(a)
#endif

#define ANI_TRIGGER_THRESHOLD           1       /* in sec */

/* default ANI parameters */
#define ANI_OFDM_TRIGGER_COUNT_HIGH     (pDevInfo ? pDevInfo->staConfig.ofdmTrigHigh : 250)
#define ANI_OFDM_TRIGGER_COUNT_LOW      (pDevInfo ? pDevInfo->staConfig.ofdmTrigLow  : 100)
#define ANI_CCK_TRIGGER_COUNT_HIGH      (pDevInfo ? pDevInfo->staConfig.cckTrigHigh  : 200)
#define ANI_CCK_TRIGGER_COUNT_LOW       (pDevInfo ? pDevInfo->staConfig.cckTrigLow   : 100)

#define MAX_SPUR_IMMUNITY_LEVEL          2
#define MAX_NOISE_IMMUNITY_LEVEL         4
#define MAX_FIR_STEP_LEVEL               2


#if defined(DEBUG) || defined(_DEBUG)
extern int aniDebug;
#define aniLog0(_pChan, _str)           do {                                    \
    if (aniDebug) {                                                             \
        uiPrintf("[ch:%d] "_str, (_pChan)->channel);                            \
    }                                                                           \
} while (0)
#define aniLog1(_pChan, _str, _p1)      do {                                    \
    if (aniDebug) {                                                             \
        uiPrintf("[ch:%d] "_str, (_pChan)->channel, _p1);                       \
    }                                                                           \
} while (0)
#else
#define aniLog0(_pChan, _str)
#define aniLog1(_pChan, _str, _p1)
#endif

void
aniReset(struct wlanDevInfo *pdevInfo);

void
aniPollFunction(struct wlanDevInfo *pdevInfo);

/* new ANI functions for the station side */
void
aniRestart(struct wlanDevInfo *pDevInfo);

void
aniOfdmErrTrigger(struct wlanDevInfo *pDevInfo);

void
aniCckErrTrigger(struct wlanDevInfo *pDevInfo);

void
aniLowerImmunity(struct wlanDevInfo *pDevInfo);

void
aniPeriodicCheck(struct wlanDevInfo *pDevInfo);

void
aniReceiveCheck(struct wlanDevInfo *pDevInfo, struct atherosDescRxStatus *pStatus);

void
logAni (struct wlanDevInfo *pDevInfo);


#ifdef __cplusplus
}
#endif

#endif /* _WLANPHY_H_ */
