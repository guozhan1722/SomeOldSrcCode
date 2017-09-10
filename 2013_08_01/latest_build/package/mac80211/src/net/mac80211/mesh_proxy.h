 /*
 *
 * Copyright (c) 2010 CEDT, Indian Institute of Science, Bangalore 
 *	Pranjal Pandey <pranjal8128@gmail.com>, 
 *	S.Satish <satishs85@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <asm/unaligned.h>
#include "ieee80211_i.h"
#include "mesh.h"

static int mesh_proxy_send_puc(struct ieee80211_sub_if_data *sdata,u8 *nexthop, u8 *proxy, u8 seq_num);
static int mesh_portal_add(struct ieee80211_sub_if_data *sdata, u8 *portal);


