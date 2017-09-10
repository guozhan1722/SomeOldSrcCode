/*

  $Id: at-emulator.c,v 1.50 2007/11/09 16:27:48 pkot Exp $

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

  Copyright (C) 1999-2000  Hugh Blemings & Pavel Janík ml.
  Copyright (C) 2001-2004  Pawel Kot
  Copyright (C) 2002-2004  BORBELY Zoltan

  This file provides a virtual modem or "AT" interface to the GSM phone by
  calling code in gsm-api.c. Inspired by and in places copied from the Linux
  kernel AT Emulator IDSN code by Fritz Elfert and others.

*/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#ifndef WIN32
    #include <termios.h>
#endif

#include "config.h"
#include "misc.h"
#include "gnokii.h"
#include "compat.h"
#include "data/at-emulator.h"

#define MAX_LINE_LENGTH 256

#define	MAX_CMD_BUFFERS	(2)
#define	CMD_BUFFER_LENGTH (100)

/* Definition of some special Registers of AT-Emulator, pinched in
   part from ISDN driver in Linux kernel */
#define REG_RINGATA   0
#define REG_RINGCNT   1
#define REG_ESC       2
#define REG_CR        3
#define REG_LF        4
#define REG_BS        5
#define	S22          22
#define S35          35
#define REG_CTRLZ   100
#define REG_ESCAPE  101

#define REG_QUIET    14
#define BIT_QUIET     4
#define REG_VERBOSE  14
#define BIT_VERBOSE   8
#define REG_ECHO     14
#define BIT_ECHO      2


#define	MAX_MODEM_REGISTERS	102

/* Message format definitions */
#define PDU_MODE      1
#define TEXT_MODE     0
#define INTERACT_MODE 2

/* Global variables */
bool gn_atem_initialised = false;   /* Set to true once initialised */
extern bool CommandMode;
extern int ConnectCount;
extern bool ExitGnokiid;
extern bool BusyFlag;
bool URC;

struct gn_statemachine *sm;
gn_data data;

static gn_sms sms;
//static gn_call_info callinfo;
static char imei[GN_IMEI_MAX_LENGTH], model[GN_MODEL_MAX_LENGTH], revision[GN_REVISION_MAX_LENGTH], manufacturer[GN_MANUFACTURER_MAX_LENGTH];

/* Local variables */
static int  PtyRDFD;    /* File descriptor for reading and writing to/from */
static int  PtyWRFD;    /* pty interface - only different in debug mode. */

static u8   ModemRegisters[MAX_MODEM_REGISTERS];
static char CmdBuffer[MAX_CMD_BUFFERS][CMD_BUFFER_LENGTH];
static int  CurrentCmdBuffer;
static int  CurrentCmdBufferIndex;
static int  IncomingCallNo;
static int     MessageFormat;          /* Message Format (text or pdu) */
static int  CallerIDMode;

/* Current command parser */
static void     (*Parser)(char *); /* Current command parser */

static gn_memory_type   SMSType;
static int  SMSNumber;

static int SMTotal;
static int METotal;
static gn_sms_memory_status sim_status;
static gn_sms_memory_status *status_list;

static void load_sms_status(const char **lines) {
    char *line, *buf, *loc;
    int i;

    /* Check whether the given memory location is not null */
    if (lines == NULL) {
        dprintf("load_sms_status - passed nil data\n");
        return;
    } else {
        dprintf("Opened /tmp/smsstatus file from memory\n");
    }

    /* Iterate over lines in the file */
    for (i = 0; lines[i] != NULL; i++) {

        line = strdup (lines[i]);
        buf = line;

        /* Strip leading, trailing whitespace */
        while (isspace((int) *line))
            line++;

        while ((strlen(line) > 0) && isspace((int) line[strlen(line) - 1]))
            line[strlen(line) - 1] = '\0';

        /* Ignore blank lines and comments */
        if ((*line == '\n') || (*line == '\0') || (*line == '#')) {
            free(buf);
            continue;
        }

        /* Process key/value line */
        if (strchr(line, '=') != NULL) {
            char *value;

            value = strchr(line, '=');
            *value = '\0';  /* Split string */
            value++;

            while (isspace((int) *value)) { /* Remove leading white */
                value++;
            }

            loc = strdup(value);
            if (status_list) {
                status_list[atoi(loc)].Unread = 0;
            }

            dprintf("delete sms%s\n", loc);

            free(loc);

            free(buf);

            /* Go on to next line */
            continue;
        }

        free(buf);
    }

}
#define READ_CHUNK_SIZE 64
/* Read sms status information file /tmp/smsstatus*/
void read_sms_status(void) {
    FILE *handle;
    char *lines, *line_begin, *line_end, *pos;
    char **split_lines;
    int read, ret, num_lines, i, copied;

    /* Open file */
    if ((handle = fopen("/tmp/smsstatus", "r")) == NULL) {
        //dprintf("read - open /tmp/smsstatus: %s\n", strerror(errno));
        goto out;
    } else {
        dprintf("Opened /tmp/smsstatus\n");
    }

    /* Read the lines */
    lines = NULL;
    read = 0;
    do {
        lines = realloc(lines, read + READ_CHUNK_SIZE);
        if (!lines)
            goto err_read;

        ret = fread(lines + read, 1, READ_CHUNK_SIZE, handle);
        /* Read error */
        if (ret < 0 && feof(handle) == 0)
            goto err_read;

        /* Overflow */
        if (read + ret < read)
            goto err_read;

        read += ret;
    } while (ret > 0);

    fclose(handle);
    lines = realloc(lines, read + 1);
    lines[read] = '\0';

    /* Now split the lines */
    split_lines = NULL;
    line_begin = lines;
    num_lines = 0;
    copied = 0;

    while ((pos = strchr(line_begin, '\n')) && copied < read) {
        char *buf;

        if (!pos) {
            line_end = lines + read;
        } else {
            line_end = pos;
        }

        num_lines++;
        buf = malloc(line_end - line_begin + 1);
        snprintf(buf, line_end - line_begin + 1, "%s", line_begin);
        split_lines = realloc(split_lines,
                              (num_lines + 1) * sizeof(char*));
        split_lines[num_lines - 1] = buf;

        if (pos) {
            copied += (line_end + 1 - line_begin);
            line_begin = line_end + 1;
        }
    }


    free(lines);
    if (split_lines == NULL)
        goto out;
    split_lines[num_lines] = NULL;

    /* load the sms status from the split lines and free the split_lines */
    load_sms_status((const char **)split_lines);
    for (i = 0; split_lines[i] != NULL; i++) {
        free(split_lines[i]);
    }
    free(split_lines);

    if (remove("/tmp/smsstatus")!=0) 
        dprintf("remove /tmp/smsstatus error\n");
    else
        dprintf("remove /tmp/smsstatus OK\n");

    goto out;

    err_read:
    fclose(handle);
    if (lines)
        free(lines);

    out:
    return;
}

static int open_with_sms_satatus(struct gn_statemachine *sm)
{
    int fd;
    gn_error error;

    struct flock lock; 
    fd = open ("/var/run/radio_busy", O_WRONLY);
    if (fd<0) {
        fprintf(stderr, _("mh_device_open fd<0\n"));
        return fd;
    }
    dprintf("mh_device_open lock OK.\n");
    memset (&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl (fd, F_SETLKW, &lock);

    read_sms_status();
    /* Initialise the code for the GSM interface. */

    gn_data *datap = NULL;
    gn_data *p_data;
    dprintf("model: %s\n", sm->config.model);
    if (sm->config.model) {
        datap = calloc(1, sizeof(gn_data));
        datap->model = sm->config.model;
        p_data = datap;
    } else p_data = NULL;

    sm->config.disconnect_script[0] = MessageFormat;

    error = gn_sm_functions(GN_OP_Init, p_data, sm);

    if (datap) free(datap);

    if (error != GN_ERR_NONE) {
        fprintf(stderr, _("GSM/FBUS init failed! (Unknown model?). Quitting.\n"));
        lock.l_type = F_UNLCK;
        fcntl (fd, F_SETLKW, &lock);
        close (fd);
        fd = -1;
    }

    return(fd);
}

static int mh_device_open(struct gn_statemachine *sm)
{
    int fd;
    gn_error error;

    struct flock lock; 
    fd = open ("/var/run/radio_busy", O_WRONLY);
    if (fd<0) {
        fprintf(stderr, _("mh_device_open fd<0\n"));
        return fd;
    }
    dprintf("mh_device_open lock OK.\n");
    memset (&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl (fd, F_SETLKW, &lock);
    /* Initialise the code for the GSM interface. */

    gn_data *datap = NULL;
    gn_data *p_data;
    dprintf("model: %s\n", sm->config.model);
    if (sm->config.model) {
        datap = calloc(1, sizeof(gn_data));
        datap->model = sm->config.model;
        p_data = datap;
    } else p_data = NULL;

    sm->config.disconnect_script[0] = MessageFormat;

    error = gn_sm_functions(GN_OP_Init, p_data, sm);

    if (datap) free(datap);

    if (error != GN_ERR_NONE) {
        fprintf(stderr, _("GSM/FBUS init failed! (Unknown model?). Quitting.\n"));
        lock.l_type = F_UNLCK;
        fcntl (fd, F_SETLKW, &lock);
        close (fd);
        fd = -1;
    }

    return(fd);
}

static void mh_device_close(int fd, struct gn_statemachine *sm)
{
    struct flock lock; 

    gn_sm_functions(GN_OP_Terminate, NULL, sm); 
    lock.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &lock);
    close (fd); 
    dprintf("mh_device_close unlock OK.\n"); 
}

/* If initialised in debug mode, stdin/out is used instead
   of ptys for interface. */
bool gn_atem_initialise(int read_fd, int write_fd, struct gn_statemachine *vmsm)
{
    PtyRDFD = read_fd;
    PtyWRFD = write_fd;
    gn_sms_status smsstatus = {0, 0, 0, 0};
    gn_data_clear(&data);
    memset(&sms, 0, sizeof(sms));
    //memset(&callinfo, 0, sizeof(callinfo));

    data.sms = &sms;
    //data.call_info = &callinfo;
    data.manufacturer = manufacturer;
    data.model = model;
    data.revision = revision;
    data.imei = imei;
    data.sms_status = &smsstatus;

    sm = vmsm;

    /* Initialise command buffer variables */
    CurrentCmdBuffer = 0;
    CurrentCmdBufferIndex = 0;

    /* Initialise registers */
    gn_atem_registers_init();

    /* Initial parser is AT routine */
    Parser = gn_atem_at_parse;

    /* Setup defaults for AT*C interpreter. */
    SMSNumber = 1;
    SMSType = GN_MT_SM;

    /* get sms memory status */
    gn_memory_status simmemorystatus   = {GN_MT_SM, 0, 0};
    data.memory_status = &simmemorystatus;
    if (gn_sm_functions(GN_OP_GetSMSStatus, &data, sm) != GN_ERR_NONE)
        return false;
    SMTotal=data.sms_status->number;
    sim_status.Unread = data.sms_status->unread;
    sim_status.Number = data.sms_status->number;
    dprintf("smtotal=%d, used=%d\n",sim_status.Number,sim_status.Unread);
    status_list = (gn_sms_memory_status*)calloc(sim_status.Number, sizeof(gn_sms_memory_status));
    /* Default message format is PDU */
    MessageFormat = TEXT_MODE;

    /* We're ready to roll... */
    gn_atem_initialised = true;
    return(true);
}

static void gn_atm_data_init(void)
{
    gn_data_clear(&data);
    memset(&sms, 0, sizeof(sms));
    //memset(&callinfo, 0, sizeof(callinfo));

    data.sms = &sms;
    //data.call_info = &callinfo;
    data.manufacturer = manufacturer;
    data.model = model;
    data.revision = revision;
    data.imei = imei;
}

/* Initialise the "registers" used by the virtual modem. */
void    gn_atem_registers_init(void)
{
    memset(ModemRegisters, 0, sizeof(ModemRegisters));

    ModemRegisters[REG_RINGATA] = 0;
    ModemRegisters[REG_RINGCNT] = 0;
    ModemRegisters[REG_ESC] = '+';
    ModemRegisters[REG_CR] = 13;
    ModemRegisters[REG_LF] = 10;
    ModemRegisters[REG_BS] = 8;
    ModemRegisters[S35] = 7;
    ModemRegisters[REG_ECHO] |= BIT_ECHO;
    ModemRegisters[REG_VERBOSE] |= BIT_VERBOSE;
    ModemRegisters[REG_CTRLZ] = 26;
    ModemRegisters[REG_ESCAPE] = 27;

    CallerIDMode = 0;
}

#if 0
static void  gn_atem_hangup_phone(void)
{
    if (IncomingCallNo > 0) {
        rlp_user_request_set(Disc_Req, true);
        gn_sm_loop(10, sm);
    }
    if (IncomingCallNo > 0) {
        data.call_info->call_id = IncomingCallNo;
        gn_sm_functions(GN_OP_CancelCall, &data, sm);
        IncomingCallNo = -1;
    }
    dp_Initialise(PtyRDFD, PtyWRFD);
}


static void  gn_atem_answer_phone(void)
{
    /* For now we'll also initialise the datapump + rlp code again */
    dp_Initialise(PtyRDFD, PtyWRFD);
    data.call_notification = dp_CallPassup;
    gn_sm_functions(GN_OP_SetCallNotification, &data, sm);
    data.call_info->call_id = IncomingCallNo;
    gn_sm_functions(GN_OP_AnswerCall, &data, sm);
    CommandMode = false;
}

/* This gets called to indicate an incoming call */
void gn_atem_call_passup(gn_call_status CallStatus, gn_call_info *CallInfo, struct gn_statemachine *state, void *callback_data)
{
    dprintf("gn_atem_call_passup called with %d\n", CallStatus);

    switch (CallStatus) {
    case GN_CALL_Incoming:
        gn_atem_modem_result(MR_RING);
        IncomingCallNo = CallInfo->call_id;
        ModemRegisters[REG_RINGCNT]++;
        gn_atem_cid_out(CallInfo);
        if (ModemRegisters[REG_RINGATA] != 0) gn_atem_answer_phone();
        break;
    case GN_CALL_LocalHangup:
    case GN_CALL_RemoteHangup:
        IncomingCallNo = -1;
        break;
    default:
        break;
    }
}

/* This gets called to output caller id info of incoming call */
void gn_atem_cid_out(gn_call_info *CallInfo)
{
    struct tm *now;
    time_t nowh;
    char buf[14]; /* 7 for "DATE = " + 4 digits + \n + \r + \0 */

    nowh = time(NULL);
    now = localtime(&nowh);

    switch (CallerIDMode) {
    case 0: /* no output */
        break;
    case 1: /* formatted CID */
        snprintf(buf, sizeof(buf), "DATE = %02d%02d\r\n", now->tm_mon + 1, now->tm_mday);
        gn_atem_string_out(buf);

        snprintf(buf, sizeof(buf), "TIME = %02d%02d\r\n", now->tm_hour, now->tm_min);
        gn_atem_string_out(buf);

        /* TO DO: handle P and O numbers */
        gn_atem_string_out("NMBR = ");
        gn_atem_string_out(1 + CallInfo->number); /* skip leading "+" */
        gn_atem_string_out("\r\nNAME = ");
        gn_atem_string_out(CallInfo->name);
        gn_atem_string_out("\r\n");

        /* FIX ME: do a real emulation of rings after the first one (at a lower level than this) */
        gn_atem_modem_result(MR_RING);

        break;

    }
}
#endif
/* Handler called when characters received from serial port.
   calls state machine code to process it. */
void    gn_atem_incoming_data_handle(char *buffer, int length)
{
    int count;
    unsigned char out_buf[3];

    for (count = 0; count < length ; count++) {

        /* If it's a command terminator character, parse what
           we have so far then go to next buffer. */
        if (buffer[count] == ModemRegisters[REG_CR] ||
            buffer[count] == ModemRegisters[REG_LF] ||
            buffer[count] == ModemRegisters[REG_CTRLZ] ||
            buffer[count] == ModemRegisters[REG_ESCAPE]) {

            /* Echo character if appropriate. */
            if (buffer[count] == ModemRegisters[REG_CR] &&
                (ModemRegisters[REG_ECHO] & BIT_ECHO)) {
                gn_atem_string_out("\r\n");
            }

            /* Save CTRL-Z and ESCAPE for the parser */
            if (buffer[count] == ModemRegisters[REG_CTRLZ] ||
                buffer[count] == ModemRegisters[REG_ESCAPE])
                CmdBuffer[CurrentCmdBuffer][CurrentCmdBufferIndex++] = buffer[count];

            CmdBuffer[CurrentCmdBuffer][CurrentCmdBufferIndex] = 0x00;

            Parser(CmdBuffer[CurrentCmdBuffer]);

            CurrentCmdBuffer++;
            if (CurrentCmdBuffer >= MAX_CMD_BUFFERS) {
                CurrentCmdBuffer = 0;
            }
            CurrentCmdBufferIndex = 0;

        } else if (buffer[count] == ModemRegisters[REG_BS]) {
            if (CurrentCmdBufferIndex > 0) {
                /* Echo character if appropriate. */
                if (ModemRegisters[REG_ECHO] & BIT_ECHO) {
                    gn_atem_string_out("\b \b");
                }

                CurrentCmdBufferIndex--;
            }
        } else {
            /* Echo character if appropriate. */
            if (ModemRegisters[REG_ECHO] & BIT_ECHO) {
                out_buf[0] = buffer[count];
                out_buf[1] = 0;
                gn_atem_string_out((char *)out_buf);
            }

            /* Collect it to command buffer */
            CmdBuffer[CurrentCmdBuffer][CurrentCmdBufferIndex++] = buffer[count];
            if (CurrentCmdBufferIndex >= CMD_BUFFER_LENGTH) {
                CurrentCmdBufferIndex = CMD_BUFFER_LENGTH;
            }
        }
    }

}


/* Parser for standard AT commands.  cmd_buffer must be null terminated. */
void    gn_atem_at_parse(char *cmd_buffer)
{
    char *buf;

    if (!cmd_buffer[0]) return;

    if (strncasecmp (cmd_buffer, "AT", 2) != 0) {
        gn_atem_modem_result(MR_ERROR);
        return;
    }
    if (strlen(cmd_buffer)==2) {
        gn_atem_modem_result(MR_OK);
        return;
    }
    for (buf = &cmd_buffer[2]; *buf;) {
        switch (toupper(*buf)) {
        case 'A':
            /* Quit */
            dprintf("Quit function. gn_vm_terminate.\n");
            ExitGnokiid = true;
            gn_atem_string_out("Bye.\r\n");
            if (status_list) free(status_list);
            status_list = NULL;
            dprintf("ata free status_list");
            gn_vm_terminate();
            return;
            break;
        case 'E':
            /* E - Turn Echo on/off */
            buf++;
            switch (gn_atem_num_get(&buf)) {
            case 0:
                ModemRegisters[REG_ECHO] &= ~BIT_ECHO;
                gn_atem_modem_result(MR_OK);
                break;
            case 1:
                ModemRegisters[REG_ECHO] |= BIT_ECHO;
                gn_atem_modem_result(MR_OK);
                break;
            default:
                gn_atem_modem_result(MR_ERROR);
                return;
            }
            break;

            /* + is the precursor to another set of commands */
        case '+':
            buf++;
            switch (toupper(*buf)) {
            case 'C':
                buf++;
                /* Returns true if error occured */
                if (gn_atem_command_plusc(&buf) == true) {
                    dprintf("plusc return true\n");
                    gn_atem_modem_result(MR_ERROR);
                    return;
                }
                break;
            case 'M':
                buf++;
                if (gn_atem_command_plusm(&buf) == true) {
                    gn_atem_modem_result(MR_ERROR);
                    return;
                }
                break;
            default:
                dprintf("+ default return\n");
                gn_atem_modem_result(MR_ERROR);
                return;
            }
            break;

        default:
            dprintf("at parse default return\n");
            return;
        }
    }

}


static void gn_atem_sms_print(char *line, gn_sms *message, int mode)
{
    int mstatus;
    char tbuf[4];
    switch (mode) {
    case INTERACT_MODE:
        gsprintf(line, MAX_LINE_LENGTH,
                 _("\r\nDate/time: %d/%d/%d %d:%02d:%02d Sender: %s Msg Center: %s\r\nText: %s\r\n"),
                 message->smsc_time.day, message->smsc_time.month, message->smsc_time.year,
                 message->smsc_time.hour, message->smsc_time.minute, message->smsc_time.second,
                 message->remote.number, message->smsc.number, message->user_data[0].u.text);
        break;
    case TEXT_MODE:
        if (message->status == GN_SMS_Unread) mstatus=0;
        else mstatus=1;
        dprintf("message status=%d mstatus=%d\n",message->status,mstatus);
        if ((message->dcs.type == GN_SMS_DCS_GeneralDataCoding) &&
            (message->dcs.u.general.alphabet == GN_SMS_DCS_8bit))
            gsprintf(line, MAX_LINE_LENGTH,
                     _("\"%s\",\"%s\",,\"%02d/%02d/%02d,%02d:%02d:%02d+%02d\"\r\n%s"),
                     (mstatus ? _("REC READ") : _("REC UNREAD")),
                     message->remote.number,
                     message->smsc_time.year, message->smsc_time.month, message->smsc_time.day,
                     message->smsc_time.hour, message->smsc_time.minute, message->smsc_time.second,
                     message->time.timezone, _("<Not implemented>"));
        else {
            if (message->smsc_time.timezone) {
                if (message->smsc_time.timezone > 0)
                    sprintf(tbuf,"+%02d", message->smsc_time.timezone);
                else
                    sprintf(tbuf,"%03d", message->smsc_time.timezone);
            } else
                sprintf(tbuf,"+00");
            gsprintf(line, MAX_LINE_LENGTH,
                     _("\"%s\",\"%s\",,\"%02d/%02d/%02d,%02d:%02d:%02d%s\"\r\n%s"),
                     (mstatus ? _("REC READ") : _("REC UNREAD")),
                     message->remote.number,
                     message->smsc_time.year, message->smsc_time.month, message->smsc_time.day,
                     message->smsc_time.hour, message->smsc_time.minute, message->smsc_time.second,
                     tbuf, message->user_data[0].u.text);

        }
        break;
    case PDU_MODE:
        break;
    default:
        gsprintf(line, MAX_LINE_LENGTH, _("<Unknown mode>"));
        break;
    }
}


static void gn_atem_sms_handle()
{
    gn_error    error;
    char        buffer[MAX_LINE_LENGTH];

    data.sms->memory_type = SMSType;
    data.sms->number = SMSNumber;
    error = gn_sms_get(&data, sm);

    switch (error) {
    case GN_ERR_NONE:
        gn_atem_sms_print(buffer, data.sms, INTERACT_MODE);
        gn_atem_string_out(buffer);
        break;
    default:
        gsprintf(buffer, MAX_LINE_LENGTH, _("\r\nNo message under number %d\r\n"), SMSNumber);
        gn_atem_string_out(buffer);
        break;
    }
    return;
}

/* Parser for SMS interactive mode */
void    gn_atem_sms_parse(char *buff)
{
    if (!strcasecmp(buff, "HELP")) {
        gn_atem_string_out(_("\r\nThe following commands work...\r\n"));
        gn_atem_string_out("DIR\r\n");
        gn_atem_string_out("EXIT\r\n");
        gn_atem_string_out("HELP\r\n");
        return;
    }

    if (!strcasecmp(buff, "DIR")) {
        SMSNumber = 1;
        gn_atem_sms_handle();
        Parser = gn_atem_dir_parse;
        return;
    }
    if (!strcasecmp(buff, "EXIT")) {
        Parser = gn_atem_at_parse;
        gn_atem_modem_result(MR_OK);
        return;
    }
    gn_atem_modem_result(MR_ERROR);
}

/* Parser for DIR sub mode of SMS interactive mode. */
void    gn_atem_dir_parse(char *buff)
{
    switch (toupper(*buff)) {
    case 'P':
        SMSNumber--;
        gn_atem_sms_handle();
        return;
    case 'N':
        SMSNumber++;
        gn_atem_sms_handle();
        return;
    case 'D':
        data.sms->memory_type = SMSType;
        data.sms->number = SMSNumber;
        if (gn_sm_functions(GN_OP_DeleteSMS, &data, sm) == GN_ERR_NONE) {
            gn_atem_modem_result(MR_OK);
        } else {
            gn_atem_modem_result(MR_ERROR);
        }
        return;
    case 'Q':
        Parser= gn_atem_sms_parse;
        gn_atem_modem_result(MR_OK);
        return;
    }
    gn_atem_modem_result(MR_ERROR);
}

/* Parser for entering message content (+CMGS) */
void    gn_atem_sms_parseText(char *buff)
{
    static int index = 0;
    int i, length;
    char buffer[MAX_LINE_LENGTH];
    gn_error error;
    int fd;

    length = strlen(buff);

    sms.user_data[0].type = GN_SMS_DATA_Text;

    if (data.sms->remote.number[0] == '+')
        data.sms->remote.type = GN_GSM_NUMBER_International;
    else
        data.sms->remote.type = GN_GSM_NUMBER_Unknown;

    for (i = 0; i < length; i++) {

        if (buff[i] == ModemRegisters[REG_CTRLZ]) {
            /* Exit SMS text mode with sending */
            sms.user_data[0].u.text[index] = 0;
            sms.user_data[0].length = index;
            index = 0;
            Parser = gn_atem_at_parse;
            fd = mh_device_open(sm);
            if (fd>=0)
                dprintf("GN_OP_Init OK.\n");
            else {
                fprintf(stderr, _("GN_OP_Init failed.\n"));
                gn_atem_modem_result(MR_ERROR);
                return;
            }
            data.message_center = calloc(1, sizeof(gn_sms_message_center));
            data.message_center->id = 1;
            if (gn_sm_functions(GN_OP_GetSMSCenter, &data, sm) == GN_ERR_NONE) {
                snprintf(sms.smsc.number, sizeof(sms.smsc.number), "%s", data.message_center->smsc.number);
                sms.smsc.type = data.message_center->smsc.type;
            } else {
                fprintf(stderr, _("Cannot read the SMSC number from your phone. If the sms send will fail, please use --smsc option explicitely giving the number.\n"));
                free(data.message_center);
                gn_atem_modem_result(MR_ERROR);
                mh_device_close(fd,sm);
                return;
            }
            free(data.message_center);


            if (!sms.smsc.type) sms.smsc.type = GN_GSM_NUMBER_Unknown;
            dprintf("Sending SMS to %s (text: %s)\n", data.sms->remote.number, data.sms->user_data[0].u.text);

            /* FIXME: set more SMS fields before sending */
            error = gn_sms_send(&data, sm);

            if (error == GN_ERR_NONE) {
                gsprintf(buffer, MAX_LINE_LENGTH, "\r\n+CMGS: 0\r\n");
                gn_atem_string_out(buffer);
                gn_atem_modem_result(MR_OK);
            } else
                gn_atem_modem_result(MR_ERROR);

            mh_device_close(fd,sm);
            return;
        } else if (buff[i] == ModemRegisters[REG_ESCAPE]) {
            /* Exit SMS text mode without sending */
            sms.user_data[0].u.text[index] = 0;
            sms.user_data[0].length = index;
            index = 0;
            Parser = gn_atem_at_parse;
            gn_atem_modem_result(MR_OK);
            return;
        } else {
            /* Appent next char to message text */
            sms.user_data[0].u.text[index++] = buff[i];
        }
    }

    /* reached the end of line so insert \n and wait for more */
    sms.user_data[0].u.text[index++] = '\r';
    sms.user_data[0].u.text[index++] = '\n';
    gn_atem_string_out("\r\n> ");
}

static int gn_get_sms_handle(int number, int flag, int status)
{
    gn_error    error;
    char        tbuffer[MAX_LINE_LENGTH];
    char        tbuffer2[MAX_LINE_LENGTH];

    data.sms->memory_type = SMSType;
    data.sms->number = number;

    sm->config.connect_script[0] = flag;
    sm->config.disconnect_script[0] = MessageFormat;
    if (MessageFormat == TEXT_MODE) {
        error = gn_sms_get(&data, sm);
    } else {
        error = gn_sms_get_pdu(&data, sm);
    }

    switch (error) {
    case GN_ERR_NONE:
        if (MessageFormat == TEXT_MODE) {
            gn_atem_sms_print(tbuffer2, data.sms, TEXT_MODE);
            switch (status) {
            case GN_SMS_Read:
                switch (flag) {
                case 0:
                    if (data.sms->status == GN_SMS_Read) {

                        gsprintf(tbuffer, MAX_LINE_LENGTH, "+CMGL: %d,%s\r\n", number, tbuffer2);
                        gn_atem_string_out(tbuffer);
                    }
                    break;
                case 1:
                    if (data.sms->status == GN_SMS_Read) {

                        gsprintf(tbuffer, MAX_LINE_LENGTH, "+MMGL: %d,%s\r\n", number, tbuffer2);
                        gn_atem_string_out(tbuffer);
                    }
                    break;
                default:
                    break;
                }
                break;
            case GN_SMS_Unread:
                switch (flag) {
                case 0:
                    if (data.sms->status == GN_SMS_Unread) {

                        gsprintf(tbuffer, MAX_LINE_LENGTH, "+CMGL: %d,%s\r\n", number, tbuffer2);
                        gn_atem_string_out(tbuffer);
                    }
                    break;
                case 1:
                    if (data.sms->status == GN_SMS_Unread) {

                        gsprintf(tbuffer, MAX_LINE_LENGTH, "+MMGL: %d,%s\r\n", number, tbuffer2);
                        gn_atem_string_out(tbuffer);
                    }
                    break;
                default:
                    break;
                }
                break;

            case 4:
                switch (flag) {
                case 0:
                    gsprintf(tbuffer, MAX_LINE_LENGTH, "+CMGL: %d,%s\r\n", number, tbuffer2);
                    gn_atem_string_out(tbuffer);
                    break;
                case 1:
                    gsprintf(tbuffer, MAX_LINE_LENGTH, "+MMGL: %d,%s\r\n", number, tbuffer2);
                    gn_atem_string_out(tbuffer);
                    break;
                default:
                    break;
                }
                break;
            case 5:
                switch (flag) {
                case 0:
                    gsprintf(tbuffer, MAX_LINE_LENGTH, "+CMGR: %s\r\n", tbuffer2);
                    gn_atem_string_out(tbuffer);
                    break;
                case 1:
                    gsprintf(tbuffer, MAX_LINE_LENGTH, "+MMGR: %s\r\n", tbuffer2);
                    gn_atem_string_out(tbuffer);
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }


        } else
            gn_atem_string_out(data.raw_sms->user_data);

        break;
    default:
        break;
    }
    sm->config.connect_script[0] = 0;
    sm->config.disconnect_script[0] = 0;
    return error;
}

static int gn_get_smslist_handle(int flag, int status)
{
    gn_raw_data grawdata;
    gn_error    error = GN_ERR_NONE;
    int index;
    int used;

    gn_atm_data_init();
    data.sms->memory_type = SMSType;
    data.sms->number = status;
    if (grawdata.data) free(grawdata.data);
    grawdata.data=NULL;
    data.raw_data=&grawdata;
    gn_sms_status smsstatus = {0, 0, 0, 0};
    gn_memory_status simmemorystatus = {GN_MT_SM, 0, 0};

    sm->config.connect_script[0] = flag;
    sm->config.disconnect_script[0] = MessageFormat;
    if (MessageFormat == PDU_MODE) {

        error = gn_smslist_get(&data,sm);

        if (error == GN_ERR_NONE) {
            gn_atem_string_out(data.raw_data->data);
            dprintf("get smslist context: %s",data.raw_data->data);
        }
        if (grawdata.data) free(grawdata.data);
        grawdata.data=NULL;
    } else if (MessageFormat == TEXT_MODE) {
        data.sms_status = &smsstatus;
        data.memory_status = &simmemorystatus;
        if (gn_sm_functions(GN_OP_GetSMSStatus, &data, sm) != GN_ERR_NONE)
            goto errorreturn;
        sim_status.Unread = data.sms_status->unread;
        sim_status.Number = data.sms_status->number;
        dprintf("smtotal=%d, used=%d\n",sim_status.Number,sim_status.Unread);
        used = 1;
        for (index=0;index<sim_status.Number;index++) {
            gn_atm_data_init();
            error = gn_get_sms_handle(index,flag,status);
            if (error == GN_ERR_NONE) used++;
            if (used > sim_status.Unread) break;

        }
    }
    errorreturn:
    sm->config.connect_script[0] = 0;
    sm->config.disconnect_script[0] = 0;
    return error;

}

/* Handle AT+C commands, this is a quick hack together at this
   stage. */
bool    gn_atem_command_plusc(char **buf)
{
    float       rflevel = -1;
    gn_rf_unit  rfunits = GN_RF_CSQ;
    char        buffer[MAX_LINE_LENGTH];
    int     status, index;
    gn_error    error;
    int fd;

    if (strncasecmp(*buf, "SQ", 2) == 0) {
        buf[0] += 2;
        gn_atm_data_init();
        data.rf_unit = &rfunits;
        data.rf_level = &rflevel;
        fd = mh_device_open(sm);
        if (fd<0) {
            fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
            return(true);
        }
        if (gn_sm_functions(GN_OP_GetRFLevel, &data, sm) == GN_ERR_NONE) {
            gsprintf(buffer, MAX_LINE_LENGTH, "+CSQ: %0.0f, 99\r\n", *(data.rf_level));
            gn_atem_string_out(buffer);
            mh_device_close(fd, sm);
            gn_atem_modem_result(MR_OK);
            return(false);
        } else {
            mh_device_close(fd, sm);
            return(true);
        }
    }

    /* AT+CCLK gets module time and date */
    if (strncasecmp(*buf, "CLK?", 4) == 0) {
        gn_timestamp    date_time;


        gn_sms_raw rawsms;

        gn_atm_data_init();
        memset(&rawsms, 0, sizeof(gn_sms_raw));
        data.datetime = &date_time;
        data.raw_sms = &rawsms;

        fd = mh_device_open(sm);
        if (fd<0) {
            fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
            return(true);
        }
        if (gn_sm_functions(GN_OP_GetDateTime, &data, sm) == GN_ERR_NONE) {
            gn_atem_string_out(data.raw_sms->user_data);
            gn_atem_string_out("\r\n");
            mh_device_close(fd, sm);
            gn_atem_modem_result(MR_OK);
            return(false);
        } else {
            mh_device_close(fd, sm);
            return(true);
        }
    }

    if (strncasecmp(*buf, "CLK=?", 5) == 0) {

        gn_atem_string_out("+CCLK: \"88/12/31, 23:59:59, (-47-+48)\"\r\n");

        gn_atem_modem_result(MR_OK);
        return(false);
    }

    if (strncasecmp(*buf, "NMI?", 4) == 0) {

        if (URC) gn_atem_string_out("+CNMI: 2,1\r\n");
        else gn_atem_string_out("+CNMI: 0,0\r\n");

        gn_atem_modem_result(MR_OK);
        return(false);
    }

    if (strncasecmp(*buf, "NMI=2,1", 7) == 0) {
        if (URC) {
            gn_atem_modem_result(MR_OK);
            return(false);
        }
        URC = true;
        if (!status_list) {
            gn_atem_modem_result(MR_ERROR);
            return(false);
        }

        fd = open_with_sms_satatus(sm);
        if (fd<0) {
            fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
            return(true);
        }

        read_sms_status();
        
        gn_atem_urc_status(4);

        mh_device_close(fd, sm);

        gn_atem_modem_result(MR_OK);
        return(false);
    }

    if (strncasecmp(*buf, "NMI=0,0", 7) == 0) {

        URC = false;
        //if (status_list) memset(status_list, 0, sim_status.Number*sizeof(gn_sms_memory_status));
        //remove("/tmp/smsstatus");
        gn_atem_modem_result(MR_OK);
        return(false);
    }
    /* AT+CREG gets Network Registration Status */
    if (strncasecmp(*buf, "REG?", 4) == 0) {

        gn_raw_data rawdata;

        gn_atm_data_init();
        memset(&rawdata, 0, sizeof(gn_raw_data));
        data.raw_data = &rawdata;

        fd = mh_device_open(sm);
        if (fd<0) {
            fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
            return(true);
        }
        if (gn_sm_functions(GN_OP_GetNetworkInfo, &data, sm) == GN_ERR_NONE) {
            gn_atem_string_out(data.raw_data->data);
            gn_atem_string_out("\r\n");
            mh_device_close(fd, sm);
            if (data.raw_data->data) free(data.raw_data->data);
            data.raw_data->data = NULL;
            gn_atem_modem_result(MR_OK);
            return(false);
        } else {
            mh_device_close(fd, sm);
            if (data.raw_data->data) free(data.raw_data->data);
            data.raw_data->data = NULL;
            return(true);
        }
    }

    /* AT+CSCA gets and sets Service Center Address */
    if (strncasecmp(*buf, "SCA?", 4) == 0) {

        gn_atm_data_init();

        fd = mh_device_open(sm);
        if (fd<0) {
            fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
            return(true);
        }
        data.message_center = calloc(1, sizeof(gn_sms_message_center));
        data.message_center->id = 1;
        if (gn_sm_functions(GN_OP_GetSMSCenter, &data, sm) == GN_ERR_NONE) {
            gsprintf(buffer, MAX_LINE_LENGTH, "+CSCA: \"%s\",%d\r\n", data.message_center->smsc.number, data.message_center->smsc.type);
            gn_atem_string_out(buffer);
            mh_device_close(fd, sm);
            free(data.message_center);
            gn_atem_modem_result(MR_OK);
            return(false);
        } else {
            free(data.message_center);
            mh_device_close(fd,sm);
            return(true);
        }

    }

    if (strncasecmp(*buf, "SCA=", 4) == 0) {
        gn_sms message;
        buf[0] += 4;
        fd = mh_device_open(sm);
        if (fd<0) {
            fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
            return(true);
        }
        memset(&message, 0, sizeof(gn_sms));
        gn_atm_data_init();
        data.sms = &message;
        if (sscanf(*buf, "\"%[+*#0-9a-zA-Z]\",%d", data.sms->smsc.number, &(data.sms->smsc.type))) {
            dprintf ("number=%s,%d\n", data.sms->smsc.number, data.sms->smsc.type);
            if (gn_sm_functions(GN_OP_SetSMSCenter, &data, sm) == GN_ERR_NONE) {
                gn_atem_modem_result(MR_OK);
                mh_device_close(fd, sm);
                return(false);
            }
        }
        mh_device_close(fd, sm);
        return(true);
    }

    /* AT+CGMI is Manufacturer information for the ME (phone) so
       it should be Nokia rather than gnokii... */
    if (strncasecmp(*buf, "GMI", 3) == 0) {
        buf[0] += 3;
        gn_atm_data_init();
        snprintf(data.manufacturer, GN_MANUFACTURER_MAX_LENGTH, "+CGMI ERROR: 0");
        fd = mh_device_open(sm);
        if (fd<0) {
            fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
            return(true);
        }
        if (gn_sm_functions(GN_OP_GetManufacturer, &data, sm) == GN_ERR_NONE) {
            gsprintf(buffer, GN_MANUFACTURER_MAX_LENGTH, "%s\r\n", data.manufacturer);
            gn_atem_string_out(buffer);
            mh_device_close(fd, sm);
            gn_atem_modem_result(MR_OK);
            return(false);
        } else {
            mh_device_close(fd, sm);
            return(true);
        }
    }

    /* AT+CGSN is IMEI */
    if (strncasecmp(*buf, "GSN", 3) == 0) {
        buf[0] += 3;
        gn_atm_data_init();
        snprintf(data.imei, GN_IMEI_MAX_LENGTH, "+CGSN ERROR: 0");
        fd = mh_device_open(sm);
        if (fd<0) {
            fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
            return(true);
        }
        if (gn_sm_functions(GN_OP_GetImei, &data, sm) == GN_ERR_NONE) {
            gsprintf(buffer, MAX_LINE_LENGTH, "%s\r\n", data.imei);
            gn_atem_string_out(buffer);
            mh_device_close(fd, sm);
            gn_atem_modem_result(MR_OK);
            return(false);
        } else {
            mh_device_close(fd, sm);
            return(true);
        }
    }

    /* AT+CGMR is Revision (hardware) */
    if (strncasecmp(*buf, "GMR", 3) == 0) {
        buf[0] += 3;
        gn_atm_data_init();
        snprintf(data.revision, GN_REVISION_MAX_LENGTH, "+CGMR ERROR: 0");
        fd = mh_device_open(sm);
        if (fd<0) {
            fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
            return(true);
        }
        if (gn_sm_functions(GN_OP_GetRevision, &data, sm) == GN_ERR_NONE) {
            gsprintf(buffer, MAX_LINE_LENGTH, "%s\r\n", data.revision);
            gn_atem_string_out(buffer);
            mh_device_close(fd, sm);
            gn_atem_modem_result(MR_OK);
            return(false);
        } else {
            mh_device_close(fd, sm);
            return(true);
        }
    }

    /* AT+CGMM is Model code  */
    if (strncasecmp(*buf, "GMM", 3) == 0) {
        buf[0] += 3;

        fd = mh_device_open(sm);
        if (fd<0) {
            fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
            return(true);
        }
        gn_atm_data_init();
        if (gn_sm_functions(GN_OP_GetModel, &data, sm) == GN_ERR_NONE) {
            gsprintf(buffer, MAX_LINE_LENGTH, "\"%s\"\r\n", data.model);
            gn_atem_string_out(buffer);
            mh_device_close(fd, sm);
            gn_atem_modem_result(MR_OK);
            return(false);
        } else {
            mh_device_close(fd, sm);
            return(true);
        }
    }

    /* AT+CMGD is deleting a message */
    if (strncasecmp(*buf, "MGD", 3) == 0) {
        buf[0] += 3;
        switch (**buf) {
        case '=':
            buf[0]++;
            if (sscanf(*buf, "%u", &index)==1) {
                dprintf("buf=%s index=%d\n",*buf,index);
                buf[0] += strlen(*buf);


                if (index>=SMTotal) {
                    fprintf (stderr, _("index>=SMTotal\n"));
                    return(true);
                }
                gn_atm_data_init();
                data.sms->memory_type = SMSType;
                data.sms->number = index;
                fd = mh_device_open(sm);
                if (fd<0) {
                    fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
                    return(true);
                }
                error = gn_sms_delete(&data, sm);
                if (error == GN_ERR_NONE) {
                    if (status_list) {
                        status_list[index].Unread = 0;
                        status_list[index].Number = 0;
                        dprintf("after delete one sms, status_list\n");
                    }
                }
            } else if (strcasecmp(*buf, ",1") == 0 ||
                       strcasecmp(*buf, ",4") == 0) {
                sscanf(*buf, ",%u", &index);
                dprintf("buf=%s index=%d\n",*buf,index);
                buf[0] += strlen(*buf);

                data.sms->memory_type = SMSType;
                data.sms->number = index;
                fd = mh_device_open(sm);
                if (fd<0) {
                    fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
                    return(true);
                }
                error = gn_sms_delete_option(&data, sm);
                if (error == GN_ERR_NONE) {
                    if (status_list) {
                        if (index == 4) {
                            memset(status_list, 0, sim_status.Number*sizeof(gn_sms_memory_status));
                        } else if (index == 1) {
                            int it;
                            for (it=0;it<sim_status.Number;it++) {
                                if (status_list[it].Unread == GN_SMS_Read)
                                    status_list[it].Unread = 0;
                            }
                        }

                        dprintf("after delete sms option, re-init status_list\n");
                    }
                }
            } else return(true);
            switch (error) {
            case GN_ERR_NONE:
                mh_device_close(fd, sm);
                break;
            default:
                gsprintf(buffer, MAX_LINE_LENGTH, "\r\n+CMGD ERROR: %d\r\n", error);
                gn_atem_string_out(buffer);
                mh_device_close(fd, sm);
                return(true);
            }
            break;
        default:
            return(true);
        }
        gn_atem_modem_result(MR_OK);
        return(false);
    }

    /* AT+CMGF */
    if (strncasecmp(*buf, "MGF", 3) == 0) {
        buf[0] += 3;
        switch (**buf) {
        case '=':
            buf[0]++;
            if (**buf=='?') {
                gn_atem_string_out("+CMGF: (0-1)\r\n");
                gn_atem_modem_result(MR_OK);
                return(false);
            }
            if (sscanf(*buf, "%u", &index)==1) {
                dprintf("buf=%s index=%d\n",*buf,index);
                buf[0] += strlen(*buf);
            } else return(true);

            if ((index>1)||(index<0)) {
                fprintf (stderr, _("argument of CMGF >1 or <0\n"));
                return(true);
            }

            if (index==1) MessageFormat = TEXT_MODE;
            else MessageFormat = PDU_MODE;
            break;
        case '?':
            if (MessageFormat == TEXT_MODE)
                gn_atem_string_out("+CMGF: 1\r\n");
            else
                gn_atem_string_out("+CMGF: 0\r\n");
            gn_atem_modem_result(MR_OK);
            return(false);
            break;
        default:
            return(true);
        }
        gn_atem_modem_result(MR_OK);
        return(false);
    }

    /* AT+CMGR is reading a message */
    if (strncasecmp(*buf, "MGR", 3) == 0) {
        buf[0] += 3;
        switch (**buf) {
        case '=':
            buf[0]++;
            if (sscanf(*buf, "%u", &index)==1) {
                dprintf("buf=%s index=%d\n",*buf,index);
                buf[0] += strlen(*buf);
            } else return(true);

            if (index>=SMTotal) {
                fprintf (stderr, _("index>=SMTotal\n"));
                return(true);
            }

            fd = mh_device_open(sm);
            if (fd<0) {
                fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
                return(true);
            }
            gn_atm_data_init();
            error = gn_get_sms_handle(index,0,5);
            if ((error == GN_ERR_NONE) && (status_list)) status_list[index].Unread = GN_SMS_Read;
            break;
        default:
            return(true);
        }
        gn_atem_modem_result(MR_OK);
        mh_device_close(fd, sm);
        return(false);
    }

    /* AT+CMGS is sending a message */
    if (strncasecmp(*buf, "MGS", 3) == 0) {
        buf[0] += 3;
        switch (**buf) {
        case '=':
            buf[0]++;
            memset(&sms.remote.number, 0, sizeof(sms.remote.number));
            gn_atm_data_init();
            if (sscanf(*buf, "\"%[+*#0-9a-zA-Z]\"", sms.remote.number)) {
                Parser = gn_atem_sms_parseText;
                buf[0] += strlen(*buf);
                gn_atem_string_out("> ");
            } else return(true);

            return(false);
        default:
            return(true);
        }
        gn_atem_modem_result(MR_OK);
        return(false);
    }

    /* AT+CMGL is listing messages */
    if (strncasecmp(*buf, "MGL", 3) == 0) {
        buf[0] += 3;
        status = -1;

        switch (**buf) {
        case 0:
        case '=':
            buf[0]++;

            if (**buf=='?') {
                if (MessageFormat == TEXT_MODE) gn_atem_string_out("+CMGL: (\"REC UNREAD\",\"REC READ\",\"ALL\")\r\n");
                else gn_atem_string_out("+CMGL: (0,1,4)\r\n");
                gn_atem_modem_result(MR_OK);
                return(false);
            }
            /* process <stat> parameter */
            if (*(*buf-1) == 0 || /* i.e. no parameter given */
                strcasecmp(*buf, "0") == 0 ||
                strcasecmp(*buf, "\"REC UNREAD\"") == 0) {
                status = GN_SMS_Unread;
            } else if (strcasecmp(*buf, "1") == 0 ||
                       strcasecmp(*buf, "\"REC READ\"") == 0) {
                status = GN_SMS_Read;
            } else if (strcasecmp(*buf, "4") == 0 ||
                       strcasecmp(*buf, "\"ALL\"") == 0) {
                status = 4; /* ALL */
            } else {
                return true;
            }
            buf[0] += strlen(*buf);

            fd = mh_device_open(sm);
            if (fd<0) {
                fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
                return(true);
            }
            error = gn_get_smslist_handle(0,status);
            if ((error == GN_ERR_NONE) && (status == 4) && (status_list)) {
                int it;
                for (it=0;it<sim_status.Number;it++) {
                    if (status_list[it].Unread > 0) {
                        status_list[it].Unread = GN_SMS_Read;
                    }
                }

            }
            mh_device_close(fd, sm);
            break;
        default:
            return(true);
        }
        gn_atem_modem_result(MR_OK);
        return(false);
    }

    return(true);
}

/* Handle AT+M commands, this is a quick hack together at this
   stage. */
bool    gn_atem_command_plusm(char **buf)
{
    char        buffer[MAX_LINE_LENGTH];
    int     status, index;
    gn_error    error;
    int fd;

    /* AT+MMGR is reading a message */
    if (strncasecmp(*buf, "MGR", 3) == 0) {
        buf[0] += 3;
        switch (**buf) {
        case '=':
            buf[0]++;
            if (sscanf(*buf, "%u", &index)==1) {
                dprintf("buf=%s index=%d\n",*buf,index);
                buf[0] += strlen(*buf);
            } else return(true);

            if (index>=SMTotal) {
                fprintf (stderr, _("index>=SMTotal\n"));
                return(true);
            }

            fd = mh_device_open(sm);
            if (fd<0) {
                fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
                return(true);
            }
            gn_atm_data_init();
            error = gn_get_sms_handle(index,1,5);
            break;
        default:
            return(true);
        }
        gn_atem_modem_result(MR_OK);
        mh_device_close(fd, sm);
        return(false);
    }

    /* AT+MMGL is listing messages */
    if (strncasecmp(*buf, "MGL", 3) == 0) {
        buf[0] += 3;
        status = -1;

        switch (**buf) {
        case 0:
        case '=':
            buf[0]++;

            if (**buf=='?') {
                if (MessageFormat == TEXT_MODE) gn_atem_string_out("+MMGL: (\"REC UNREAD\",\"REC READ\",\"ALL\")\r\n");
                else gn_atem_string_out("+MMGL: (0,1,4)\r\n");
                gn_atem_modem_result(MR_OK);
                return(false);
            }

            /* process <stat> parameter */
            if (*(*buf-1) == 0 || /* i.e. no parameter given */
                strcasecmp(*buf, "0") == 0 ||
                strcasecmp(*buf, "\"REC UNREAD\"") == 0) {
                status = GN_SMS_Unread;
            } else if (strcasecmp(*buf, "1") == 0 ||
                       strcasecmp(*buf, "\"REC READ\"") == 0) {
                status = GN_SMS_Read;
            } else if (strcasecmp(*buf, "4") == 0 ||
                       strcasecmp(*buf, "\"ALL\"") == 0) {
                status = 4; /* ALL */
            } else {
                return true;
            }
            buf[0] += strlen(*buf);

            fd = mh_device_open(sm);
            if (fd<0) {
                fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
                return(true);
            }
            error = gn_get_smslist_handle(1,status);
            mh_device_close(fd, sm);
            break;
        default:
            return(true);
        }
        gn_atem_modem_result(MR_OK);
        return(false);
    }

    return(true);
}

void gn_atem_urc_status(int status)
{
    int i, newcount;
    char *tmp;
    char *pos;
    char *dp;
    char *sp;
    char index[32];

    gn_raw_data grawdata;
    gn_error    error = GN_ERR_NONE;

    gn_atm_data_init();

    data.sms->memory_type = SMSType;
    data.sms->number = status;
    if (grawdata.data) free(grawdata.data);
    grawdata.data=NULL;
    data.raw_data=&grawdata;

    sm->config.connect_script[0] = 1;

    error = gn_smslist_get(&data,sm);

    sm->config.connect_script[0] = 0;
    if (error == GN_ERR_NONE) {
        dprintf("get smslist context: %s",data.raw_data->data);
        tmp = data.raw_data->data;
        pos = tmp;
        index[0]=0; 
        tmp = strstr(pos,"+MMGL: ");
        if (!tmp) goto freeandreturn;

        if (!status_list) goto freeandreturn;

        newcount = 0;
        i = 0;
        while (pos) {
            dp = strchr(tmp,',');
            if (dp) {
                memcpy(index, tmp+7, dp-tmp-7);
                index[dp-tmp-7]=0;
                dprintf("dp index=%s\n",index);
                newcount = atoi(index);
                dprintf("newcount=%d\n",newcount);
                if (status == 4) {
                    sp = strchr(dp+1,',');
                    if (sp) {
                        memcpy(index, dp+1, sp-dp-1);
                        index[sp-dp-1]=0;
                        dprintf("sp index=%s\n",index);
                        if (status_list[newcount].Unread == 0)
                            status_list[newcount].Unread = atoi(index)>0 ? GN_SMS_Read: GN_SMS_Unread;
                        else if (atoi(index) > 0) {
                            status_list[newcount].Unread = GN_SMS_Read;
                        }
                        dprintf("unread=%d\n",status_list[newcount].Unread);
                    }
                } else {
                    if (status_list[newcount].Unread == 0) status_list[newcount].Unread = GN_SMS_Unread;
                    if (status_list[newcount].Unread == GN_SMS_Unread) {
                        i++;
                        status_list[newcount].Unread = GN_SMS_Sent;
                        status_list[i].Number = newcount;
                    }
                }
            }
            pos = strstr(tmp+7, "+MMGL: ");
            tmp=pos;
        }

        if (status == 0) {
            newcount = 0;
            for (newcount=i;newcount>0;newcount--) {
                gsprintf(index, 32, "\r\n+CMTI: \"SM\",%d\r\n", status_list[newcount].Number);
                gn_atem_string_out(index);
            }
            i = 0;
        }

    }

    freeandreturn:
    if (grawdata.data) free(grawdata.data);
    grawdata.data=NULL;

}

void gn_atem_urc_process(void)
{
    int fd;

    if (!status_list) {
        gn_atem_modem_result(MR_ERROR);
        return;
    }
    fd = open_with_sms_satatus(sm);
    if (fd<0) {
        fprintf (stderr, _("gn_atm_command_plusc - gn_gsm_initialise failed!\n"));
        return;
    }

    gn_atem_urc_status(0);

    mh_device_close(fd, sm);
}

void gn_atm_urc_free(void)
{
    if (status_list) {
        fprintf (stderr, _("free status_list\n"));
        free(status_list);
    }

    status_list = NULL;
}
#if 0
/* AT+G commands.  Some of these responses are a bit tongue in cheek... */
bool    gn_atem_command_plusg(char **buf)
{
    char        buffer[MAX_LINE_LENGTH];

    /* AT+GMI is Manufacturer information for the TA (Terminal Adaptor) */
    if (strncasecmp(*buf, "MI", 3) == 0) {
        buf[0] += 2;

        gn_atem_string_out(_("Hugh Blemings, Pavel Janik ml. and others...\r\n"));
        return(false);
    }

    /* AT+GMR is Revision information for the TA (Terminal Adaptor) */
    if (strncasecmp(*buf, "MR", 3) == 0) {
        buf[0] += 2;
        gsprintf(buffer, MAX_LINE_LENGTH, "%s %s %s\r\n", VERSION, __TIME__, __DATE__);

        gn_atem_string_out(buffer);
        return(false);
    }

    /* AT+GMM is Model information for the TA (Terminal Adaptor) */
    if (strncasecmp(*buf, "MM", 3) == 0) {
        buf[0] += 2;

        gsprintf(buffer, MAX_LINE_LENGTH, _("gnokii configured on %s for models %s\r\n"), sm->config.port_device, sm->driver.phone.models);
        gn_atem_string_out(buffer);
        return(false);
    }

    /* AT+GSN is Serial number for the TA (Terminal Adaptor) */
    if (strncasecmp(*buf, "SN", 3) == 0) {
        buf[0] += 2;

        gsprintf(buffer, MAX_LINE_LENGTH, _("none built in, choose your own\r\n"));
        gn_atem_string_out(buffer);
        return(false);
    }

    return(true);
}
#endif
/* Handle AT# commands */
bool    gn_atem_command_diesis(char **buf)
{
    int number;
    char    buffer[MAX_LINE_LENGTH];

    if (strncasecmp(*buf, "CID", 3) == 0) {
        buf[0] += 3;
        switch (**buf) {
        case '?':
            buf[0]++;
            gsprintf(buffer, MAX_LINE_LENGTH, "%d\r\n", CallerIDMode);
            gn_atem_string_out(buffer);
            return(false);
        case '=':
            buf[0]++;
            if (**buf == '?') {
                buf[0]++;
                gn_atem_string_out("0,1\r\n");
                return(false);
            } else {
                number = gn_atem_num_get(buf);
                if ( number == 0 || number == 1 ) {
                    CallerIDMode = number;
                    return(false);
                }
            }
        }
    }
    return(true);
}

/* Send a result string back.  There is much work to do here, see
   the code in the isdn driver for an idea of where it's heading... */
void    gn_atem_modem_result(int code)
{
    char    buffer[16];

    if (BusyFlag) BusyFlag=false;
    if (!(ModemRegisters[REG_VERBOSE] & BIT_VERBOSE)) {
        snprintf(buffer, sizeof(buffer), "%d\r\n", code);
        gn_atem_string_out(buffer);
    } else {
        switch (code) {
        case MR_OK:
            gn_atem_string_out("\r\nOK\r\n");
            break;

        case MR_ERROR:
            gn_atem_string_out("\r\nERROR\r\n");
            break;

        case MR_CARRIER:
            gn_atem_string_out("CARRIER\r\n");
            break;

        case MR_CONNECT:
            gn_atem_string_out("CONNECT\r\n");
            break;

        case MR_NOCARRIER:
            gn_atem_string_out("NO CARRIER\r\n");
            break;
        case MR_RING:
            gn_atem_string_out("RING\r\n");
            break;
        default:
            gn_atem_string_out(_("\r\nUnknown Result Code!\r\n"));
            break;
        }
    }

}


/* Get integer from char-pointer, set pointer to end of number
   stolen basically verbatim from ISDN code.  */
int gn_atem_num_get(char **p)
{
    int v = -1;

    while (*p[0] >= '0' && *p[0] <= '9') {
        v = ((v < 0) ? 0 : (v * 10)) + (int) ((*p[0]++) - '0');
    }

    return v;
}

/* Write string to virtual modem port, either pty or
   STDOUT as appropriate.  This function is only used during
   command mode - data pump is used when connected.  */
void    gn_atem_string_out(char *buffer)
{
    int count = 0;
    char    out_char;

    while (count < strlen(buffer)) {

        /* Translate CR/LF/BS as appropriate */
        switch (buffer[count]) {
        case '\r':
            out_char = ModemRegisters[REG_CR];
            break;
        case '\n':
            if ((count>0)&&(buffer[count-1]!='\r')) {
                out_char = ModemRegisters[REG_CR];
                if (write(PtyWRFD, &out_char, 1) < 0) {
                    fprintf(stderr, _("Failed to output string.\n"));
                    perror("write");
                    return;
                }
            }
            out_char = ModemRegisters[REG_LF];
            break;
        case '\b':
            out_char = ModemRegisters[REG_BS];
            break;
        default:
            out_char = buffer[count];
            break;
        }

        if (write(PtyWRFD, &out_char, 1) < 0) {
            fprintf(stderr, _("Failed to output string.\n"));
            perror("write");
            return;
        }
        count++;
    }
}
