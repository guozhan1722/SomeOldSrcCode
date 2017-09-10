/*

  $Id: common.h,v 1.155 2007/10/07 16:51:33 dforsi Exp $

  G N O K I I

  A Linux/Unix toolset and driver for Nokia the phones.

  This file is part of gnokii.

  Gnokii is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Gnokii is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with gnokii; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Copyright (C) 1999-2000 Hugh Blemings & Pavel Janík ml.
  Copyright (C) 2001-2004 Pawel Kot, BORBELY Zoltan
  Copyright (C) 2001      Manfred Jonsson, Marian Jancar, Chris Kemp, Marcin Wiacek
  Copyright (C) 2001-2002 Pavel Machek
  Copyright (C) 2002      Markus Plail

  Header file for the definitions, enums etc. that are used by all models of
  handset.

*/

#ifndef _gnokii_common_h
#define _gnokii_common_h

/* Type of connection. Now we support serial connection with FBUS cable and
   IR (only with 61x0 models) */
typedef enum {
    GN_CT_NONE = -1,/* no connection type (error) */
	GN_CT_Serial,   /* Serial connection. */
	GN_CT_TCP,      /* TCP network connection */
} gn_connection_type;

/* Maximum length of device name for serial port */

#define GN_DEVICE_NAME_MAX_LENGTH (32)

/* Define an enum for specifying memory types for retrieving phonebook
   entries, SMS messages etc. This type is not mobile specific - the model
   code should take care of translation to mobile specific numbers - see 6110
   code.
   01/07/99:  Two letter codes follow GSM 07.07 release 6.2.0
*/
typedef enum {
	GN_MT_ME, /* Internal memory of the mobile equipment */
	GN_MT_SM, /* SIM card memory */
	GN_MT_LAST = GN_MT_SM,
	GN_MT_XX = 0xff	/* Error code for unknown memory type (returned by fbus-xxxx functions). */
} gn_memory_type;

/* Definition of security codes. */
typedef enum {
	GN_SCT_SecurityCode = 0x01, /* Security code. */
	GN_SCT_Pin,                 /* PIN. */
	GN_SCT_Pin2,                /* PIN 2. */
	GN_SCT_Puk,                 /* PUK. */
	GN_SCT_Puk2,                /* PUK 2. */
	GN_SCT_None                 /* Code not needed. */
} gn_security_code_type;

/* Security code definition. */
typedef struct {
	gn_security_code_type type; /* Type of the code. */
	char code[10];              /* Actual code. */
	char new_code[10];          /* New code. */
} gn_security_code;

/* This structure is used to get the current network status */
typedef struct {
	char network_code[10];     /* GSM network code */
	unsigned char cell_id[10]; /* CellID */
	unsigned char LAC[10];     /* LAC */
} gn_network_info;


/* This data type is used to report the number of used and free positions in
   memory (sim or internal). */
typedef struct {
	gn_memory_type memory_type; /* Type of the memory */
	int used;                   /* Number of used positions */
	int free;                   /* Number of free positions */
} gn_memory_status;

/* General date and time structure. It is used for the SMS, calendar, alarm
 * settings, clock etc. */
typedef struct {
	int year;           /* The complete year specification - e.g. 1999. Y2K :-) */
	int month;          /* January = 1 */
	int day;
	int hour;
	int minute;
	int second;
	int timezone;      /* The difference between local time and GMT.
			      Note that different SMSC software treat this field
			      in the different ways. */
} gn_timestamp;

/* Define enum used to describe what sort of date/time support is
   available. */
typedef enum {
	GN_DT_None,     /* The mobile phone doesn't support time and date. */
	GN_DT_TimeOnly, /* The mobile phone supports only time. */
	GN_DT_DateOnly, /* The mobile phone supports only date. */
	GN_DT_DateTime  /* Wonderful phone - it supports date and time. */
} gn_datetime_support;

/* Define enums for RF units. GRF_CSQ asks for units in form used
   in AT+CSQ command as defined by GSM 07.07 */
typedef enum {
	GN_RF_Arbitrary,
	GN_RF_dBm,
	GN_RF_mV,
	GN_RF_uV,
	GN_RF_CSQ,
	GN_RF_Percentage
} gn_rf_unit;
#if 0
typedef enum {
	GN_PROFILE_MESSAGE_NoTone	= 0x00,
	GN_PROFILE_MESSAGE_Standard	= 0x01,
	GN_PROFILE_MESSAGE_Special	= 0x02,
	GN_PROFILE_MESSAGE_BeepOnce	= 0x03,
	GN_PROFILE_MESSAGE_Ascending	= 0x04
} gn_profile_message_type;

typedef enum {
	GN_PROFILE_WARNING_Off		= 0xff,
	GN_PROFILE_WARNING_On		= 0x04
} gn_profile_warning_type;

typedef enum {
	GN_PROFILE_VIBRATION_Off	= 0x00,
	GN_PROFILE_VIBRATION_On		= 0x01
} gn_profile_vibration_type;

typedef enum {
	GN_PROFILE_CALLALERT_Ringing		= 0x01,
	GN_PROFILE_CALLALERT_BeepOnce		= 0x02,
	GN_PROFILE_CALLALERT_Off		= 0x04,
	GN_PROFILE_CALLALERT_RingOnce		= 0x05,
	GN_PROFILE_CALLALERT_Ascending		= 0x06,
	GN_PROFILE_CALLALERT_CallerGroups	= 0x07
} gn_profile_callalert_type;

typedef enum {
	GN_PROFILE_KEYVOL_Off		= 0xff,
	GN_PROFILE_KEYVOL_Level1	= 0x00,
	GN_PROFILE_KEYVOL_Level2	= 0x01,
	GN_PROFILE_KEYVOL_Level3	= 0x02
} gn_profile_keyvol_type;

typedef enum {
	GN_PROFILE_VOLUME_Level1	= 0x06,
	GN_PROFILE_VOLUME_Level2	= 0x07,
	GN_PROFILE_VOLUME_Level3	= 0x08,
	GN_PROFILE_VOLUME_Level4	= 0x09,
	GN_PROFILE_VOLUME_Level5	= 0x0a,
} gn_profile_volume_type;

/* Structure to hold profile entries. */
typedef struct {
	int number;           /* The number of the profile. */
	char name[40];        /* The name of the profile. */
	int default_name;     /* 0-6, when default name is used, -1, when not. */
	int keypad_tone;      /* Volume level for keypad tones. */
	int lights;           /* Lights on/off. */
	int call_alert;       /* Incoming call alert. */
	int ringtone;         /* Ringtone for incoming call alert. */
	int volume;           /* Volume of the ringing. */
	int message_tone;     /* The tone for message indication. */
	int warning_tone;     /* The tone for warning messages. */
	int vibration;        /* Vibration? */
	int caller_groups;    /* CallerGroups. */
	int automatic_answer; /* Does the phone auto-answer incoming call? */
} gn_profile;
#endif
/* Limits for IMEI, Revision, Model and Manufacturer string storage. */
#define GN_IMEI_MAX_LENGTH         20
#define GN_REVISION_MAX_LENGTH     20
#define GN_MODEL_MAX_LENGTH        96
#define GN_MANUFACTURER_MAX_LENGTH 32

#define GN_BCD_STRING_MAX_LENGTH 40

/* This data-type is used to specify the type of the number. See the official
   GSM specification 03.40, version 6.1.0, section 9.1.2.5, page 35-37. */
typedef enum {
	GN_GSM_NUMBER_Unknown       = 0x81, /* Unknown number */
	GN_GSM_NUMBER_International = 0x91, /* International number */
	GN_GSM_NUMBER_National      = 0xa1, /* National number */
	GN_GSM_NUMBER_Network       = 0xb1, /* Network specific number */
	GN_GSM_NUMBER_Subscriber    = 0xc1, /* Subscriber number */
	GN_GSM_NUMBER_Alphanumeric  = 0xd0, /* Alphanumeric number */
	GN_GSM_NUMBER_Abbreviated   = 0xe1  /* Abbreviated number */
} gn_gsm_number_type;

typedef struct {
	gn_gsm_number_type type;
	char number[GN_BCD_STRING_MAX_LENGTH];
} gn_gsm_number;
#if 0
/* Data structures for the call divert */
typedef enum {
	GN_CDV_Busy = 0x01,
	GN_CDV_NoAnswer,
	GN_CDV_OutOfReach,
	GN_CDV_NotAvailable,
	GN_CDV_AllTypes
} gn_call_divert_type;

typedef enum {
	GN_CDV_VoiceCalls = 0x01,
	GN_CDV_FaxCalls,
	GN_CDV_DataCalls,
	GN_CDV_AllCalls
} gn_call_divert_call_type;

typedef enum {
	GN_CDV_Disable  = 0x00,
	GN_CDV_Enable   = 0x01,
	GN_CDV_Query    = 0x02,
	GN_CDV_Register = 0x03,
	GN_CDV_Erasure  = 0x04
} gn_call_divert_operation;

typedef struct {
	gn_call_divert_type           type;
	gn_call_divert_call_type     ctype;
	gn_call_divert_operation operation;
	gn_gsm_number               number;
	unsigned int               timeout;
} gn_call_divert;
#endif
typedef struct {
	int full; /* indicates if we have full data read */
	unsigned int length;
	unsigned char *data;
} gn_raw_data;
#if 0
/* This enum is used for display status. */
typedef enum {
	GN_DISP_Call_In_Progress, /* Call in progress. */
	GN_DISP_Unknown,          /* The meaning is unknown now :-( */
	GN_DISP_Unread_SMS,       /* There is Unread SMS. */
	GN_DISP_Voice_Call,       /* Voice call active. */
	GN_DISP_Fax_Call,         /* Fax call active. */
	GN_DISP_Data_Call,        /* Data call active. */
	GN_DISP_Keyboard_Lock,    /* Keyboard lock status. */
	GN_DISP_SMS_Storage_Full  /* Full SMS Memory. */
} gn_display_status;

#define	GN_DRAW_SCREEN_MAX_WIDTH  27
#define	GN_DRAW_SCREEN_MAX_HEIGHT  6

typedef enum {
	GN_DISP_DRAW_Clear,
	GN_DISP_DRAW_Text,
	GN_DISP_DRAW_Status
} gn_display_draw_command;

typedef struct {
	int x;
	int y;
	unsigned char text[GN_DRAW_SCREEN_MAX_WIDTH + 1];
} gn_display_text;

typedef struct {
	gn_display_draw_command cmd;
	union {
		gn_display_text text;
		gn_display_status status;
	} data;
} gn_display_draw_msg;

typedef struct {
	void (*output_fn)(gn_display_draw_msg *draw);
	int state;
	struct timeval last;
} gn_display_output;

typedef enum {
	GN_KEY_NONE = 0x00,
	GN_KEY_1 = 0x01,
	GN_KEY_2,
	GN_KEY_3,
	GN_KEY_4,
	GN_KEY_5,
	GN_KEY_6,
	GN_KEY_7,
	GN_KEY_8,
	GN_KEY_9,
	GN_KEY_0,
	GN_KEY_HASH,
	GN_KEY_ASTERISK,
	GN_KEY_POWER,
	GN_KEY_GREEN,
	GN_KEY_RED,
	GN_KEY_INCREASEVOLUME,
	GN_KEY_DECREASEVOLUME,
	GN_KEY_UP = 0x17,
	GN_KEY_DOWN,
	GN_KEY_MENU,
	GN_KEY_NAMES
} gn_key_code;

typedef struct {
	int field;
	char screen[50];
} gn_netmonitor;
#endif
typedef struct {
	int  userlock;		/* TRUE = user lock, FALSE = factory lock */
	int  closed;
	char  data[12];
	int   counter;
} gn_locks_info;
#if 0
typedef struct {
	int frequency;
	int volume;
} gn_tone;

#define GN_RINGTONE_MAX_NAME 20
#define GN_RINGTONE_MAX_COUNT 256

typedef struct {
	int location;
	char name[20];
	int user_defined;
	int readable;
	int writable;
} gn_ringtone_info;

typedef struct {
	int count;
	int userdef_location;
	int userdef_count;
	gn_ringtone_info ringtone[GN_RINGTONE_MAX_COUNT];
} gn_ringtone_list;
#endif
typedef enum {
	GN_LOG_T_NONE = 0,
	GN_LOG_T_STDERR = 1
} gn_log_target;

#if 0
typedef enum {
	GN_FT_None = 0,
	GN_FT_NOL,
	GN_FT_NGG,
	GN_FT_NSL,
	GN_FT_NLM,
	GN_FT_BMP,
	GN_FT_OTA,
	GN_FT_XPMF,
	GN_FT_RTTTL,
	GN_FT_OTT,
	GN_FT_MIDI,
	GN_FT_NOKRAW_TONE,
	GN_FT_GIF,
	GN_FT_JPG,
	GN_FT_MID,
	GN_FT_NRT,
	GN_FT_PNG,
} gn_filetypes;

typedef struct {
	gn_filetypes filetype;   /* file type */
	unsigned char *id;	/* file id */
	char name[512];		/* file name */
	int year;		/* datetime of creation/modification */
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int file_length;	/* size of the file */
	int togo;		/* amount of bytes to be sent yet */
	int just_sent;		/* ??? */
	int folderId;           /* folder id of the file */
	unsigned char *file;	/* file contents */
} gn_file;

#define GN_FILES_MAX_COUNT 1024

typedef struct {
	char path[512];
	gn_file *files[GN_FILES_MAX_COUNT];
	int file_count;
} gn_file_list;
#endif
#endif	/* _gnokii_common_h */
