/*
 * $Id: log.c,v 1.1 2010/07/14 22:53:05 Bob Exp $
 *
 * Copyright (C) 1995,1996,1997 Lars Fenneberg
 *
 * See the file COPYRIGHT for the respective terms and conditions.
 * If the file is missing contact me at lf@elemental.net
 * and I'll send you a copy.
 *
 */

#include <radiusclient_config.h>
#include <radiusclient_includes.h>
#include <radiusclient-ng.h>

/*
 * Function: rc_openlog
 *
 * Purpose: open log
 *
 * Arguments: identification string
 *
 * Returns: nothing
 *
 */

void rc_openlog(char *ident)
{
   // openlog(ident, LOG_PID, RC_LOG_FACILITY);
}

/*
 * Function: rc_log
 *
 * Purpose: log information
 *
 * Arguments: priority (just like syslog), rest like printf
 *
 * Returns: nothing
 *
 */

void rc_log(int prio, const char *format, ...)
{
	char buff[1024];
	va_list ap;

	va_start(ap,format);
    vsnprintf(buff, sizeof(buff), format, ap);
    va_end(ap);

	syslog(prio, "%s", buff);
}
