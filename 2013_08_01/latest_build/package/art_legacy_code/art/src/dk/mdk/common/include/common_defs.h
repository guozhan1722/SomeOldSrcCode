#ifndef _INC_COMMON_DEFS_H
#define _INC_COMMON_DEFS_H
/* Reset Flags */
#define MAC_RESET       0x1
#define BB_RESET        0x2
#define BUS_RESET       0x4

//modes
#define MODE_11A				0
#define MODE_11G				1
#define MODE_11B				2
#define MODE_11O				3	//OFDM at 2.4
#define HALF_SPEED_MODE			50
#define QUARTER_SPEED_MODE		51
#define TURBO_ENABLE			1


#if defined(MDK_AP) || defined(SOC_AP)
#define mSleep(x) (milliSleep(x))
extern A_UINT32 milliTime();
extern void milliSleep(A_UINT32);
#endif

#if !defined(ECOS)
#ifndef SWIG
A_INT32 uiPrintf ( const char *format, ...);


A_INT32 q_uiPrintf ( const char *format, ...);

A_INT16 statsPrintf ( FILE *pFile, const char *format, ...);
#endif
#endif

#ifdef WIN32
#define mSleep(x) (Sleep((DWORD) x))
#if !defined(__ATH_DJGPPDOS__)
#define milliTime() (GetTickCount())

#include <time.h>
#include <windows.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
    #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
    #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
     
struct timezone 
{
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};
     
static __inline int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag;
     
    if (NULL != tv)
    {
        GetSystemTimeAsFileTime(&ft);
     
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;
     
        /*converting file time to unix epoch*/
        tmpres /= 10;  /*convert into microseconds*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS; 
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
    }
     
    if (NULL != tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }
     
    return 0;
}

#endif
#endif

#ifdef LINUX
#define mSleep(x) (milliSleep(x))
extern A_UINT32 milliTime(void);
extern void milliSleep(A_UINT32);
#define microSleep(x) usleep(x)
#endif	

#define ART_ENDIAN_UNKNOWN      0
#define ART_ENDIAN_BIG          1
#define ART_ENDIAN_LITTLE       2

#endif

