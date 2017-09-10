#ifndef GPSRLIB_H
#define GPSRLIB_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "database.h"



#ifdef DEBUG
#define debug(fmt,args...)   syslog(LOGOPTS,fmt,##args)
#else
#define debug(fmt,args...)
#endif

#define GPALL 1
#define GPGGA 2
#define GPGSA 3
#define GPGSV 4
#define GPRMC 5
#define GPVTG 6
#define GPZDA 7
#define GPGLL 8
#define TXTLL 9
#define GATED 10

#define READY 1
#define WAITING 0

typedef struct gps_position
{
   float lat;
   float lng;
   char c_north;
   char c_east;
}gps_position;
extern struct gps_position base_gps_pos;
extern char gps_text[64];
extern char* find_nmea_start(const char* buffer);
extern int nmea_length(const char* start);
extern char* find_sep_nmea_start(const char* buffer, const char* sep);
extern char* find_type_nmea_start(const char* buffer,int type);
extern int fetch_one_pattern(const char* rbuf,char* pbuf);
extern int send_select_nmea(const char* rbuf,char* mss_type,char* sbuf);
//extern int get_gpsposition(char* start, int type, gps_position *result);
extern int get_gps_pos(const char* buf, struct gps_position *result);
extern int check_gps_data(const char* rbuf, struct gps_position *gpsp);
extern double compute_distance(gps_position *current, gps_position *desired);
extern void addeq_oper_timeval(struct timeval *leftp, const struct timeval *rightp);
extern void add_oper_timeval(struct timeval *resultp,const struct timeval *leftp, const struct timeval *rightp);
extern void minuseq_oper_timeval(struct timeval *resultp, const struct timeval *rightp);
extern void minus_oper_timeval(struct timeval *resultp, const struct timeval *leftp, const struct timeval *rightp);

extern void set_nmea_checksum(char *buf,int buf_inx);

#endif
