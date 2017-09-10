/*

  $Id: acconfig.h,v 1.12 2006/04/24 20:08:49 pkot Exp $

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

  Copyright (C) 2001-2004 Pawel Kot
  Copyright (C) 2002      Marcel Holtmann
  Copyright (C) 2002-2003 BORBELY Zoltan
  Copyright (C) 2004      Martin Goldhahn

  Various system features. Autoheader generates config.h.in based on this
  file.

*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

@TOP@

#undef VERSION
#undef XVERSION
#undef XGNOKIIDIR

/***** Features *****/

/* Define if you want debug */
#undef DEBUG
#undef XDEBUG
#undef RLP_DEBUG

/* Define if you want security options enabled */
#undef SECURITY

/* Use UNIX98 style pty support instead of the traditional */
#undef USE_UNIX98PTYS

/* Define if you want IrDA support. Linux only */
#undef HAVE_IRDA

/* Define if you want Bluetooth support. Cross platform */
#undef HAVE_BLUETOOTH


/**** Platform specific *****/

/* Define if you have __ptr_t type defined. It misses eg on FreeBSD. */
#undef HAVE_PTR_T

/* Define if you have timerisset, timerclear, timercmp, timeradd and timersub */
#undef HAVE_TIMEOPS

/* Define if you have tm_gmtoff field in tm structure */
#undef HAVE_TM_GMTON

/* Define if you have cfsetspeed */
#undef HAVE_CFSETSPEED

/* Define if you have cfsetispeed and cfsetospeed */
#undef HAVE_CFSETISPEED
#undef HAVE_CFSETOSPEED

/* Define if you have c_ispeed and c_ospeed in struct termios */
#undef HAVE_TERMIOS_CSPEED

/* Define if your snprintf implementation is fully C99 compliant */
#undef HAVE_C99_SNPRINTF

/* Define if your vsnprintf implementation is fully C99 compliant */
#undef HAVE_C99_VSNPRINTF

/* Define if struct msghdr has msg_control field */
#undef HAVE_MSGHDR_MSG_CONTROL

/* Define if you compile for M$ Windows */
#undef WIN32

/* Define if you have the intl library (-lintl). */
#undef HAVE_LIBINTL

/* Define if you have XPM components */
#undef XPM

/* NLS support */
#undef HAVE_CATGETS

@BOTTOM@

#endif /* __CONFIG_H__ */
