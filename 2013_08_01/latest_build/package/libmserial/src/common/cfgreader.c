/*

  $Id: cfgreader.c,v 1.84 2007/11/07 18:28:18 pkot Exp $

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

  Copyright (C) 1999      Tim Potter
  Copyright (C) 1999-2000 Hugh Blemings & Pavel Jan�k ml.
  Copyright (C) 2000      Jan Derfinak
  Copyright (C) 2001      Jan Kratochvil
  Copyright (C) 2001-2005 Pawel Kot
  Copyright (C) 2002-2004 BORBELY Zoltan
  Copyright (C) 2005      Bastien Nocera

  Config file (/etc/gnokiirc and ~/.gnokiirc) reader.

  Modified from code by Tim Potter.

*/

#include "config.h"
#include "compat.h"
#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "cfgreader.h"
#include "gnokii-internal.h"

GNOKII_API struct gn_cfg_header *gn_cfg_info;
static gn_config gn_config_default, gn_config_global;

struct gn_cfg_header *cfg_memory_read(const char **lines) {
    char *line, *buf;
    struct gn_cfg_header *cfg_info = NULL, *cfg_head = NULL;
    int i;

    /* Check whether the given memory location is not null */
    if (lines == NULL) {
        dprintf("cfg_memory_read - passed nil data\n");
        return NULL;
    } else {
        dprintf("Opened configuration file from memory\n");
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

        /* Look for "headings" enclosed in square brackets */
        if ((line[0] == '[') && (line[strlen(line) - 1] == ']')) {
            struct gn_cfg_header *heading;

            /* Allocate new heading entry */
            if ((heading = (struct gn_cfg_header *)malloc(sizeof(*heading))) == NULL) {
                return NULL;
            }

            /* Fill in fields */
            memset(heading, '\0', sizeof(*heading));

            line++;
            line[strlen(line) - 1] = '\0';

            /* FIXME: strdup is not ANSI C compliant. */
            heading->section = strdup(line);

            /* Add to tail of list  */
            heading->prev = cfg_info;

            if (cfg_info != NULL) {
                cfg_info->next = heading;
            } else {
                /* Store copy of head of list for return value */
                cfg_head = heading;
            }

            cfg_info = heading;

            dprintf("Added new section %s\n", heading->section);

            free(buf);

            /* Go on to next line */
            continue;
        }

        /* Process key/value line */
        if ((strchr(line, '=') != NULL) && cfg_info != NULL) {
            struct gn_cfg_entry *entry;
            char *value;

            /* Allocate new entry */
            if ((entry = (struct gn_cfg_entry *)malloc(sizeof(*entry))) == NULL) {
                return NULL;
            }

            /* Fill in fields */
            memset(entry, '\0', sizeof(*entry));

            value = strchr(line, '=');
            *value = '\0';  /* Split string */
            value++;

            while (isspace((int) *value)) { /* Remove leading white */
                value++;
            }

            entry->value = strdup(value);

            while ((strlen(line) > 0) && isspace((int) line[strlen(line) - 1])) {
                line[strlen(line) - 1] = '\0';  /* Remove trailing white */
            }

            /* FIXME: strdup is not ANSI C compliant. */
            entry->key = strdup(line);

            /* Add to head of list */

            entry->next = cfg_info->entries;

            if (cfg_info->entries != NULL) {
                cfg_info->entries->prev = entry;
            }

            cfg_info->entries = entry;

            dprintf("Adding key/value %s/%s\n", entry->key, entry->value);

            free(buf);

            /* Go on to next line */
            continue;
        }

        /* Line not part of any heading */
        fprintf(stderr, _("Orphaned line: %s\n"), line);

        free(buf);
    }

    /* Return pointer to configuration information */
    return cfg_head;
}

GNOKII_API void gn_cfg_free_default()
{
    while (gn_cfg_info) {
        struct gn_cfg_header *next;
        struct gn_cfg_entry *entry;

        /* free section name */
        free(gn_cfg_info->section);

        /* delete all entries */
        entry = gn_cfg_info->entries;
        while (entry) {
            struct gn_cfg_entry *entry_next;
            free(entry->key);
            free(entry->value);
            entry_next = entry->next;
            free(entry);
            entry = entry_next;
        }

        /* proceed with next section */
        next = gn_cfg_info->next;
        free(gn_cfg_info);
        gn_cfg_info = next;
    }
}

#define READ_CHUNK_SIZE 64

/* Read configuration information from a ".INI" style file */
struct gn_cfg_header *cfg_file_read(const char *filename) {
    FILE *handle;
    char *lines, *line_begin, *line_end, *pos;
    char **split_lines;
    int read, ret, num_lines, i, copied;
    struct gn_cfg_header *header = NULL;

    /* Open file */
    if ((handle = fopen(filename, "r")) == NULL) {
        dprintf("cfg_file_read - open %s: %s\n", filename, strerror(errno));
        goto out;
    } else {
        dprintf("Opened configuration file %s\n", filename);
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

    /* Finally, load the configuration from the split lines */
    header = cfg_memory_read((const char **)split_lines);

    /* Free the split_lines */
    for (i = 0; split_lines[i] != NULL; i++) {
        free(split_lines[i]);
    }
    free(split_lines);

    goto out;

    err_read:
    fclose(handle);
    if (lines)
        free(lines);

    out:
    return header;
}

/*  Write configuration information to a config file */
int cfg_file_write(struct gn_cfg_header *cfg, const char *filename)
{
    /* Not implemented - tricky to do and preserve comments */
    return 0;
}

/*
 * Find the value of a key in a config file.  Return value associated
 * with key or NULL if no such key exists.
 */
GNOKII_API char *gn_cfg_get(struct gn_cfg_header *cfg, const char *section, const char *key)
{
    struct gn_cfg_header *h;
    struct gn_cfg_entry *e;

    if ((cfg == NULL) || (section == NULL) || (key == NULL)) {
        return NULL;
    }

    /* Search for section name */
    for (h = cfg; h != NULL; h = h->next) {
        if (strcmp(section, h->section) == 0) {
            /* Search for key within section */
            for (e = h->entries; e != NULL; e = e->next) {
                if (strcmp(key, e->key) == 0) {
                    /* Found! */
                    return e->value;
                }
            }
        }
    }
    /* Key not found in section */
    return NULL;
}

int cfg_section_exists(struct gn_cfg_header *cfg, const char *section)
{
    struct gn_cfg_header *h;

    if (!cfg || !section)
        return false;
    /* Search for section name */
    for (h = cfg; h != NULL; h = h->next)
        if (!strcmp(section, h->section))
            return true;
    return false;
}

/*
 * Return all the entries of the given section.
 */
void cfg_foreach(const char *section, cfg_foreach_func func)
{
    struct gn_cfg_header *h;
    struct gn_cfg_entry *e;
    struct gn_cfg_header *cfg = gn_cfg_info;

    if ((cfg == NULL) || (section == NULL) || (func == NULL)) {
        return;
    }

    /* Search for section name */
    for (h = cfg; h != NULL; h = h->next) {
        if (strcmp(section, h->section) == 0) {
            /* Search for key within section */
            for (e = h->entries; e != NULL; e = e->next)
                (*func)(section, e->key, e->value);
        }
    }
}

/*  Set the value of a key in a config file.  Return the new value if
    the section/key can be found, else return NULL.  */
char *cfg_set(struct gn_cfg_header *cfg, const char *section, const char *key,
              const char *value)
{
    struct gn_cfg_header *h;
    struct gn_cfg_entry *e;

    if ((cfg == NULL) || (section == NULL) || (key == NULL) ||
        (value == NULL)) {
        return NULL;
    }

    /* Search for section name */
    for (h = cfg; h != NULL; h = h->next) {
        if (strcmp(section, h->section) == 0) {
            /* Search for key within section */
            for (e = h->entries; e != NULL; e = e->next) {
                if ((e->key != NULL) && strcmp(key, e->key) == 0) {
                    /* Found - set value */
                    free(e->key);
                    /* FIXME: strdup is not ANSI C compliant. */
                    e->key = strdup(value);
                    return e->value;
                }
            }
        }
    }
    /* Key not found in section */
    return NULL;
}

static gn_error cfg_psection_load(gn_config *cfg, const char *section, const gn_config *def)
{
    const char *val;
    char ch;

    memset(cfg, '\0', sizeof(gn_config));

    if (!cfg_section_exists(gn_cfg_info, section)) {
        fprintf(stderr, _("No %s section in the config file.\n"), section);
        return GN_ERR_NOPHONE;
    }

    /* You need to specify at least model and port in the phone section */
    if (!(val = gn_cfg_get(gn_cfg_info, section, "model"))) {
        fprintf(stderr, _("You need to define '%s' in the config file.\n"), "model");
        return GN_ERR_NOMODEL;
    } else
        snprintf(cfg->model, sizeof(cfg->model), "%s", val);

    if (!(val = gn_cfg_get(gn_cfg_info, section, "port"))) {
        fprintf(stderr, _("You need to define '%s' in the config file.\n"), "port");
        return GN_ERR_NOPORT;
    } else
        snprintf(cfg->port_device, sizeof(cfg->port_device), "%s", val);

    if (!(val = gn_cfg_get(gn_cfg_info, section, "connection"))) {
        fprintf(stderr, _("You need to define '%s' in the config file.\n"), "connection");
        return GN_ERR_NOCONNECTION;
    } else {
        cfg->connection_type = gn_get_connectiontype(val);
        if (cfg->connection_type == GN_CT_NONE) {
            fprintf(stderr, _("Unsupported [%s] %s value \"%s\"\n"), section, "connection", val);
            return GN_ERR_NOCONNECTION;
        }
    }

    if (!(val = gn_cfg_get(gn_cfg_info, section, "initlength")))
        cfg->init_length = def->init_length;
    else {
        if (!strcasecmp(val, "default"))
            cfg->init_length = 0;
        else if (sscanf(val, " %d %c", &cfg->init_length, &ch) != 1) {
            fprintf(stderr, _("Unsupported [%s] %s value \"%s\"\n"), section, "initlength", val);
            fprintf(stderr, _("Assuming: %d\n"), def->init_length);
            cfg->init_length = def->init_length;
        }
    }

    if (!(val = gn_cfg_get(gn_cfg_info, section, "serial_baudrate")))
        cfg->serial_baudrate = def->serial_baudrate;
    else if (sscanf(val, " %d %c", &cfg->serial_baudrate, &ch) != 1) {
        fprintf(stderr, _("Unsupported [%s] %s value \"%s\"\n"), section, "serial_baudrate", val);
        fprintf(stderr, _("Assuming: %d\n"), def->serial_baudrate);
        cfg->serial_baudrate = def->serial_baudrate;
    }

    if (!(val = gn_cfg_get(gn_cfg_info, section, "serial_write_usleep")))
        cfg->serial_write_usleep = def->serial_write_usleep;
    else if (sscanf(val, " %d %c", &cfg->serial_write_usleep, &ch) != 1) {
        fprintf(stderr, _("Unsupported [%s] %s value \"%s\"\n"), section, "serial_write_usleep", val);
        fprintf(stderr, _("Assuming: %d\n"), def->serial_write_usleep);
        cfg->serial_write_usleep = def->serial_write_usleep;
    }

    if (!(val = gn_cfg_get(gn_cfg_info, section, "handshake")))
        cfg->hardware_handshake = def->hardware_handshake;
    else if (!strcasecmp(val, "software") || !strcasecmp(val, "rtscts"))
        cfg->hardware_handshake = false;
    else if (!strcasecmp(val, "hardware") || !strcasecmp(val, "xonxoff"))
        cfg->hardware_handshake = true;
    else {
        fprintf(stderr, _("Unsupported [%s] %s value \"%s\"\n"), section, "handshake", val);
        fprintf(stderr, _("Use either \"%s\" or \"%s\".\n"), "software", "hardware");
        fprintf(stderr, _("Assuming: %s\n"), def->hardware_handshake ? "software" : "hardware");
        cfg->hardware_handshake = def->hardware_handshake;
    }

    if (!(val = gn_cfg_get(gn_cfg_info, section, "require_dcd")))
        cfg->require_dcd = def->require_dcd;
    else if (sscanf(val, " %d %c", &cfg->require_dcd, &ch) != 1) {
        fprintf(stderr, _("Unsupported [%s] %s value \"%s\"\n"), section, "require_dcd", val);
        fprintf(stderr, _("Assuming: %d\n"), def->require_dcd);
        cfg->require_dcd = def->require_dcd;
    }

    if (!(val = gn_cfg_get(gn_cfg_info, section, "smsc_timeout")))
        cfg->smsc_timeout = def->smsc_timeout;
    else if (sscanf(val, " %d %c", &cfg->smsc_timeout, &ch) == 1)
        cfg->smsc_timeout *= 10;
    else {
        fprintf(stderr, _("Unsupported [%s] %s value \"%s\"\n"), section, "smsc_timeout", val);
        fprintf(stderr, _("Assuming: %d\n"), def->smsc_timeout);
        cfg->smsc_timeout = def->smsc_timeout;
    }

    if (!(val = gn_cfg_get(gn_cfg_info, section, "connect_script")))
        snprintf(cfg->connect_script, sizeof(cfg->connect_script), "%s", def->connect_script);
    else
        snprintf(cfg->connect_script, sizeof(cfg->connect_script), "%s", val);

    if (!(val = gn_cfg_get(gn_cfg_info, section, "disconnect_script")))
        snprintf(cfg->disconnect_script, sizeof(cfg->disconnect_script), "%s", def->disconnect_script);
    else
        snprintf(cfg->disconnect_script, sizeof(cfg->disconnect_script), "%s", val);

    if (!(val = gn_cfg_get(gn_cfg_info, section, "rfcomm_channel")))
        cfg->rfcomm_cn = def->rfcomm_cn;
    else if (sscanf(val, " %hhu %c", &cfg->rfcomm_cn, &ch) != 1) {
        fprintf(stderr, _("Unsupported [%s] %s value \"%s\"\n"), section, "rfcomm_channel", val);
        fprintf(stderr, _("Assuming: %d\n"), def->rfcomm_cn);
        cfg->rfcomm_cn = def->rfcomm_cn;
    }

    if (!(val = gn_cfg_get(gn_cfg_info, section, "sm_retry")))
        cfg->sm_retry = def->sm_retry;
    else if (sscanf(val, " %d %c", &cfg->sm_retry, &ch) != 1) {
        fprintf(stderr, _("Unsupported [%s] %s value \"%s\"\n"), section, "sm_retry", val);
        fprintf(stderr, _("Assuming: %d\n"), def->sm_retry);
        cfg->sm_retry = def->sm_retry;
    }

    /* There is no global setting. You need to set irda_string
     * in each section if you want it working */
#if 0
    if (!(val = gn_cfg_get(gn_cfg_info, section, "irda_string")))
        cfg->irda_string[0] = 0;
    else {
        snprintf(cfg->irda_string, sizeof(cfg->irda_string), "%s", val);
        dprintf("Setting irda_string in section %s to %s\n", section, cfg->irda_string);
    }
#endif
    if (!(val = gn_cfg_get(gn_cfg_info, section, "use_locking")))
        cfg->use_locking = def->use_locking;
    else if (!strcasecmp(val, "no"))
        cfg->use_locking = 0;
    else if (!strcasecmp(val, "yes"))
        cfg->use_locking = 1;
    else {
        fprintf(stderr, _("Unsupported [%s] %s value \"%s\"\n"), section, "use_locking", val);
        fprintf(stderr, _("Use either \"%s\" or \"%s\".\n"), "yes", "no");
        fprintf(stderr, _("Assuming: %s\n"), def->use_locking ? "yes" : "no");
        cfg->use_locking = def->use_locking;
    }

    return GN_ERR_NONE;
}

static bool cfg_get_log_target(gn_log_target *t, const char *opt)
{
    char *val;

    if (!(val = gn_cfg_get(gn_cfg_info, "logging", opt)))
        val = "off";

    if (!strcasecmp(val, "off"))
        *t = GN_LOG_T_NONE;
    else if (!strcasecmp(val, "on"))
        *t = GN_LOG_T_STDERR;
    else {
        fprintf(stderr, _("Unsupported [%s] %s value \"%s\"\n"), "logging", opt, val);
        fprintf(stderr, _("Use either \"%s\" or \"%s\".\n"), "off", "on");
        fprintf(stderr, _("Assuming: %s\n"), "off");
        *t = GN_LOG_T_NONE;
    }

    return true;
}

#define MAX_PATH_LEN 200
GNOKII_API gn_error gn_cfg_read_default()
{
    gn_error error;
    char *homedir;
    char rcfile[MAX_PATH_LEN];
#ifdef WIN32
    char globalrc[MAX_PATH_LEN];
    char *systemroot;

    systemroot = getenv("SYSTEMROOT");
    strncpy(globalrc, systemroot ? systemroot : "", MAX_PATH_LEN);
    strncat(globalrc, "\\gnokiirc", MAX_PATH_LEN);

    homedir = getenv("HOMEDRIVE");
    strncpy(rcfile, homedir ? homedir : "", MAX_PATH_LEN);
    homedir = getenv("HOMEPATH");
    strncat(rcfile, homedir ? homedir : "", MAX_PATH_LEN);
    strncat(rcfile, "\\_gnokiirc", MAX_PATH_LEN);
#else
    const char globalrc[] = "/etc/gnokiirc";

    homedir = getenv("HOME");
    if (homedir)
        strncpy(rcfile, homedir, MAX_PATH_LEN);
    strncat(rcfile, "/.gnokiirc", MAX_PATH_LEN);
#endif

    /* Try opening .gnokirc from users home directory first */
    if ((error = gn_cfg_file_read(rcfile)) != GN_ERR_NONE) {
        //fprintf(stderr, _("Couldn't read %s config file.\n"), rcfile);
        /* It failed so try for /etc/gnokiirc */
        if ((error == GN_ERR_NOCONFIG) && ((error = gn_cfg_file_read(globalrc)) != GN_ERR_NONE)) {
            /* That failed too so exit */
            fprintf(stderr, _("Couldn't read %s config file.\n"), globalrc);
        }
    }
    return error;
}

/* DEPRECATED */
GNOKII_API gn_error gn_cfg_read(char **bindir)
{
    gn_error retval;

    retval = gn_cfg_read_default();

    *bindir = gn_cfg_get(gn_cfg_info, "global", "bindir");
    if (!*bindir)
        *bindir = gn_cfg_get(gn_cfg_info, "gnokiid", "bindir");
    if (!*bindir)
        *bindir = "/usr/local/sbin";

    return retval;
}

static gn_error cfg_file_or_memory_read(const char *file, const char **lines)
{
    char *val;
    gn_error error;

    if (file == NULL && lines == NULL) {
        fprintf(stderr, _("Couldn't open a config file or memory.\n"));
        return GN_ERR_NOCONFIG;
    }

    /* I know that it doesn't belong here but currently there is now generic
     * application init function anywhere.
     */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /*
     * Try opening a given config file
     */
    if (file != NULL)
        gn_cfg_info = cfg_file_read(file);
    else
        gn_cfg_info = cfg_memory_read(lines);

    if (gn_cfg_info == NULL) {
        /* this is bad, but the previous was much worse - bozo */
#if 0
        if (file)
            fprintf(stderr, _("Couldn't read %s config file.\n"), file);
        else
            fprintf(stderr, _("Couldn't read config.\n"));
#endif
        return GN_ERR_NOCONFIG;
    }
    gn_config_default.model[0] = 0;
    gn_config_default.port_device[0] = 0;
    gn_config_default.connection_type = GN_CT_Serial;
    gn_config_default.init_length = 0;
    gn_config_default.serial_baudrate = 19200;
    gn_config_default.serial_write_usleep = -1;
    gn_config_default.hardware_handshake = false;
    gn_config_default.require_dcd = false;
    gn_config_default.smsc_timeout = -1;
    //gn_config_default.irda_string[0] = 0;
    gn_config_default.connect_script[0] = 0;
    gn_config_default.disconnect_script[0] = 0;
    gn_config_default.rfcomm_cn = 1;
    gn_config_default.sm_retry = 0;
    gn_config_default.use_locking = 0;

    if ((error = cfg_psection_load(&gn_config_global, "global", &gn_config_default)) != GN_ERR_NONE)
        return error;

    /* hack to support [sms] / smsc_timeout parameter */
    if (gn_config_global.smsc_timeout < 0) {
        if (!(val = gn_cfg_get(gn_cfg_info, "sms", "timeout")))
            gn_config_global.smsc_timeout = 100;
        else
            gn_config_global.smsc_timeout = 10 * atoi(val);
    }

    if (!cfg_get_log_target(&gn_log_debug_mask, "debug") ||
        !cfg_get_log_target(&gn_log_rlpdebug_mask, "rlpdebug") ||
        !cfg_get_log_target(&gn_log_xdebug_mask, "xdebug"))
        return GN_ERR_NOLOG;

    gn_log_debug("LOG: debug mask is 0x%x\n", gn_log_debug_mask);
    gn_log_rlpdebug("LOG: rlpdebug mask is 0x%x\n", gn_log_rlpdebug_mask);
    gn_log_xdebug("LOG: xdebug mask is 0x%x\n", gn_log_xdebug_mask);
    if (file)
        dprintf("Config read from file %s.\n", file);
    return GN_ERR_NONE;
}

GNOKII_API gn_error gn_cfg_file_read(const char *file)
{
    return cfg_file_or_memory_read(file, NULL);
}

GNOKII_API gn_error gn_cfg_memory_read(const char **lines)
{
    return cfg_file_or_memory_read(NULL, lines);
}

GNOKII_API gn_error gn_cfg_phone_load(const char *iname, struct gn_statemachine *state)
{
    char section[256];
    gn_error error;

    if (iname == NULL || *iname == '\0') {
        state->config = gn_config_global;
    } else {
        snprintf(section, sizeof(section), "phone_%s", iname);
        if ((error = cfg_psection_load(&state->config, section, &gn_config_global)) != GN_ERR_NONE)
            return error;
    }

    if (state->config.model[0] == '\0') {
        fprintf(stderr, _("Config error - no model specified.\n"));
        return GN_ERR_NOMODEL;
    }
    if (state->config.port_device[0] == '\0') {
        fprintf(stderr, _("Config error - no port specified.\n"));
        return GN_ERR_NOPORT;
    }

    return GN_ERR_NONE;
}
