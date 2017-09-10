/*

  $Id: data.h,v 1.83 2007/07/07 12:21:20 pkot Exp $

  G N O K I I

  A Linux/Unix toolset and driver for the mobile phones.

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

  Copyright (C) 1999-2000 Hugh Blemings & Pavel Jan�k ml.
  Copyright (C) 2002-2004 BORBELY Zoltan, Pawel Kot
  Copyright (C) 2002      Ladis Michl, Markus Plail, Pavel Machek, Chris Kemp

*/

#ifndef _gnokii_data_h
#define _gnokii_data_h

#include <gnokii/common.h>
#include <gnokii/sms.h>
#include <gnokii/error.h>

/* For models table */
typedef struct {
	const char *model;	  /* e.g. 6310 */
	const char *product_name; /* e.g. NPE-4 */
	int flags;
} gn_phone_model;

/* This is a generic holder for high level information - eg a gn_bmp */
typedef struct {
	gn_sms_raw *raw_sms;         /* This is for phone driver, application using libgnokii should not touch this */
	gn_sms *sms;                 /* This is for user communication, phone driver should not have to touch this one */
	gn_memory_status *memory_status;
	gn_sms_status *sms_status;
	gn_sms_message_center *message_center;
	char *imei;
	char *revision;
	char *model;
	char *manufacturer;
    gn_network_info *network_info;
	//gn_bmp *bitmap;
	gn_rf_unit *rf_unit;
	float *rf_level;
	gn_timestamp *datetime;
	gn_raw_data *raw_data;
	int *display_status;
	unsigned char character;
	gn_phone_model *phone;
	gn_locks_info *locks_info;
	void *callback_data; /* this is a pointer to some data that will be needed by any callback function */
} gn_data;

/* 
 * A structure to hold information about the particular link
 * The link comes 'under' the phone
 */
typedef struct {
	/* A regularly called loop function. Timeout can be used to make the
	 * function block or not */
	gn_error (*loop)(struct timeval *timeout, struct gn_statemachine *state);
	/* A pointer to the function used to send out a message. This is used
	 * by the phone specific code to send a message over the link */
	gn_error (*send_message)(unsigned int messagesize, unsigned char messagetype, unsigned char *message,
				 struct gn_statemachine *state);
	void *link_instance;
} gn_link;

typedef struct {
	char model[GN_MODEL_MAX_LENGTH];		/* Phone model */
	char port_device[GN_DEVICE_NAME_MAX_LENGTH];	/* Port device to use (e.g. /dev/ttyS0) */
	gn_connection_type connection_type;		/* Connection type (e.g. serial, ir) */
	int init_length;				/* Number of chars sent to sync the serial port */
	int serial_baudrate;				/* Baud rate to use */
	int serial_write_usleep;			/* Inter character delay or <0 to disable */
	int hardware_handshake;				/* Select between hardware and software handshake */
	int require_dcd;				/* DCD signal check */
	int smsc_timeout;				/* How many seconds should we wait for the SMSC response, defaults to 10 seconds */
	char connect_script[256];			/* Script to run when device connection established */
	char disconnect_script[256];			/* Script to run when device connection closed */
	uint8_t rfcomm_cn;				/* RFCOMM channel number to connect */
	unsigned int sm_retry;				/* Indicates whether statemachine should do retries. Defaults to off. */
							/* Use with caution -- may break newer DCT4 phones */

	unsigned int use_locking;			/* Should we use locking system or not */
	/* do not change the following values from userspace */
	char m_model[GN_MODEL_MAX_LENGTH];
	char m_manufacturer[GN_MANUFACTURER_MAX_LENGTH];
	char m_revision[GN_REVISION_MAX_LENGTH];
	char m_imei[GN_IMEI_MAX_LENGTH];
} gn_config;

typedef struct {
	int fd;
	gn_connection_type type;
	void *device_instance;
} gn_device;

typedef enum {
	GN_OP_Init,
	GN_OP_Terminate,
	GN_OP_GetModel,
	GN_OP_GetRevision,
	GN_OP_GetImei,
	GN_OP_GetManufacturer,
	GN_OP_Identify,
	GN_OP_GetBitmap,
	GN_OP_SetBitmap,
	GN_OP_GetRFLevel,
	GN_OP_GetMemoryStatus,
	GN_OP_GetSMSStatus,
	GN_OP_GetNetworkInfo,
	GN_OP_GetSMS,
	GN_OP_DeleteSMS,
	GN_OP_SendSMS,
    GN_OP_SaveSMS,
	GN_OP_GetSMSCenter,
	GN_OP_SetSMSCenter,
	GN_OP_GetDateTime,
	GN_OP_SetDateTime,
	GN_OP_Reset,
	GN_OP_GetLocksInfo,
    GN_OP_DeleteSMSOption,
    GN_OP_GetSMSList,
	GN_OP_Max,	/* don't append anything after this entry */
} gn_operation;

/* Undefined functions in fbus/mbus files */
extern gn_error gn_unimplemented(void);
#define GN_UNIMPLEMENTED (void *) gn_unimplemented

#endif	/* _gnokii_data_h */
