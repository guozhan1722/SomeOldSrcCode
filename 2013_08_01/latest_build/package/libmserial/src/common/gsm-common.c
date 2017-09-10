/*

  $Id: gsm-common.c,v 1.25 2007/10/07 16:51:33 dforsi Exp $

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

  Copyright (C) 1999-2000 Hugh Blemings & Pavel Janík ml.
  Copyright (C) 2001-2004 Pawel Kot
  Copyright (C) 2001      Chris Kemp, Pavel Machek
  Copyright (C) 2002-2004 BORBELY Zoltan

*/

#include <string.h>
#include "config.h"
#include "compat.h"
#include "gnokii.h"

gn_error unimplemented(void)
{
	return GN_ERR_NOTIMPLEMENTED;
}

GNOKII_API gn_memory_type gn_str2memory_type(const char *s)
{
#define X(a) if (!strcmp(s, #a)) return GN_MT_##a;
	X(ME);
	X(SM);
	return GN_MT_XX;
#undef X
}

GNOKII_API const char *gn_memory_type2str(gn_memory_type mt)
{
	switch (mt) {
	case GN_MT_ME: return _("Internal memory");
	case GN_MT_SM: return _("SIM card");
	default: return _("Unknown");
	}
}

/**
 * gn_number_sanitize - Remove all white charactes from the string
 * @number: input/output number string
 * @maxlen: maximal number length
 *
 * Use this function to sanitize GSM phone number format. It changes
 * number argument.
 */
GNOKII_API void gn_number_sanitize(char *number, int maxlen)
{
	char *iter, *e;

	iter = e = number;
	while (*iter && *e && (e - number < maxlen)) {
		*iter = *e;
		if (isspace(*iter)) {
			while (*e && isspace(*e) && (e - number < maxlen)) {
				e++;
			}
		}
		*iter = *e;
		iter++;
		e++;
	}
	*iter = 0;
}

/**
 * gn_phonebook_entry_sanitize - Remove all white charactes from the string
 * @entry: phonebook entry to sanitize
 *
 * Use this function before any attempt to write an entry to the phone.
 */
#if 0
GNOKII_API void gn_phonebook_entry_sanitize(gn_phonebook_entry *entry)
{
	int i;

	gn_number_sanitize(entry->number, GN_PHONEBOOK_NUMBER_MAX_LENGTH + 1);
	for (i = 0; i < entry->subentries_count; i++) {
		if (entry->subentries[i].entry_type == GN_PHONEBOOK_ENTRY_Number)
			gn_number_sanitize(entry->subentries[i].data.number, GN_PHONEBOOK_NUMBER_MAX_LENGTH + 1);
	}
}
#endif
/* 
 * This very small function is just to make it easier to clear
 * the data struct every time one is created 
 */
GNOKII_API void gn_data_clear(gn_data *data)
{
	memset(data, 0, sizeof(gn_data));
}
