/*
 * Copyright (c) 2008, 2009 open80211s Ltd.
 * Authors:    Luis Carlos Cobo <luisca@cozybit.com>
 * 	       Javier Cardona <javier@cozybit.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/unaligned.h>
#include "ieee80211_i.h"
#include "mesh.h"

#define IEEE80211_MESH_PEER_INACTIVITY_LIMIT (1800 * HZ)
#define IEEE80211_MESH_HOUSEKEEPING_INTERVAL (60 * HZ)
#define IEEE80211_MESH_RANN_INTERVAL	     (1 * HZ)

#ifdef MICROHARD
#define IEEE80211_PRO_RREQ_INTERVAL (100 * HZ)
#define IEEE80211_PORTAL_INTERVAL (200 * HZ)
#define MSEC_TO_TU(x) (x*1000/1024)
//#define IEEE80211_MESH_PRO_RREQ_LIFETIME 15000
#define default_lifetime(s) \
	MSEC_TO_TU(s->u.mesh.mshcfg.dot11MeshHWMPactivePathTimeout)

#define SET_AS_ROOT 0
#define net_traversal_jiffies(s) \
	msecs_to_jiffies(s->u.mesh.mshcfg.dot11MeshHWMPnetDiameterTraversalTime)
#endif

#define MESHCONF_CAPAB_ACCEPT_PLINKS 0x01
#define MESHCONF_CAPAB_FORWARDING    0x08

#define TMR_RUNNING_HK	0
#define TMR_RUNNING_MP	1
#define TMR_RUNNING_MPR	2

int mesh_allocated;
static struct kmem_cache *rm_cache;

void ieee80211s_init(void)
{
	mesh_pathtbl_init();
	mesh_allocated = 1;
	rm_cache = kmem_cache_create("mesh_rmc", sizeof(struct rmc_entry),
				     0, 0, NULL);
}

void ieee80211s_stop(void)
{
	mesh_pathtbl_unregister();
	kmem_cache_destroy(rm_cache);
}

static void ieee80211_mesh_housekeeping_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata = (void *) data;
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

	set_bit(MESH_WORK_HOUSEKEEPING, &ifmsh->wrkq_flags);

	if (local->quiescing) {
		set_bit(TMR_RUNNING_HK, &ifmsh->timers_running);
		return;
	}

	ieee80211_queue_work(&local->hw, &ifmsh->work);
}

/**
 * mesh_matches_local - check if the config of a mesh point matches ours
 *
 * @ie: information elements of a management frame from the mesh peer
 * @sdata: local mesh subif
 *
 * This function checks if the mesh configuration of a mesh point matches the
 * local mesh configuration, i.e. if both nodes belong to the same mesh network.
 */
bool mesh_matches_local(struct ieee802_11_elems *ie, struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

	/*
	 * As support for each feature is added, check for matching
	 * - On mesh config capabilities
	 *   - Power Save Support En
	 *   - Sync support enabled
	 *   - Sync support active
	 *   - Sync support required from peer
	 *   - MDA enabled
	 * - Power management control on fc
	 */
	if (ifmsh->mesh_id_len == ie->mesh_id_len &&
		memcmp(ifmsh->mesh_id, ie->mesh_id, ie->mesh_id_len) == 0 &&
		(ifmsh->mesh_pp_id == ie->mesh_config->meshconf_psel) &&
		(ifmsh->mesh_pm_id == ie->mesh_config->meshconf_pmetric) &&
		(ifmsh->mesh_cc_id == ie->mesh_config->meshconf_congest) &&
		(ifmsh->mesh_sp_id == ie->mesh_config->meshconf_synch) &&
		(ifmsh->mesh_auth_id == ie->mesh_config->meshconf_auth))
		return true;

	return false;
}

/**
 * mesh_peer_accepts_plinks - check if an mp is willing to establish peer links
 *
 * @ie: information elements of a management frame from the mesh peer
 */
bool mesh_peer_accepts_plinks(struct ieee802_11_elems *ie)
{
	return (ie->mesh_config->meshconf_cap &
	    MESHCONF_CAPAB_ACCEPT_PLINKS) != 0;
}

/**
 * mesh_accept_plinks_update: update accepting_plink in local mesh beacons
 *
 * @sdata: mesh interface in which mesh beacons are going to be updated
 */
void mesh_accept_plinks_update(struct ieee80211_sub_if_data *sdata)
{
	bool free_plinks;

	/* In case mesh_plink_free_count > 0 and mesh_plinktbl_capacity == 0,
	 * the mesh interface might be able to establish plinks with peers that
	 * are already on the table but are not on PLINK_ESTAB state. However,
	 * in general the mesh interface is not accepting peer link requests
	 * from new peers, and that must be reflected in the beacon
	 */
	free_plinks = mesh_plink_availables(sdata);

	if (free_plinks != sdata->u.mesh.accepting_plinks)
		ieee80211_mesh_housekeeping_timer((unsigned long) sdata);
}

void mesh_ids_set_default(struct ieee80211_if_mesh *sta)
{
	sta->mesh_pp_id = 0;	/* HWMP */
	sta->mesh_pm_id = 0;	/* Airtime */
	sta->mesh_cc_id = 0;	/* Disabled */
	sta->mesh_sp_id = 0;	/* Neighbor Offset */
	sta->mesh_auth_id = 0;	/* Disabled */
}

int mesh_rmc_init(struct ieee80211_sub_if_data *sdata)
{
	int i;

	sdata->u.mesh.rmc = kmalloc(sizeof(struct mesh_rmc), GFP_KERNEL);
	if (!sdata->u.mesh.rmc)
		return -ENOMEM;
	sdata->u.mesh.rmc->idx_mask = RMC_BUCKETS - 1;
	for (i = 0; i < RMC_BUCKETS; i++)
		INIT_LIST_HEAD(&sdata->u.mesh.rmc->bucket[i].list);
	return 0;
}

void mesh_rmc_free(struct ieee80211_sub_if_data *sdata)
{
	struct mesh_rmc *rmc = sdata->u.mesh.rmc;
	struct rmc_entry *p, *n;
	int i;

	if (!sdata->u.mesh.rmc)
		return;

	for (i = 0; i < RMC_BUCKETS; i++)
		list_for_each_entry_safe(p, n, &rmc->bucket[i].list, list) {
			list_del(&p->list);
			kmem_cache_free(rm_cache, p);
		}

	kfree(rmc);
	sdata->u.mesh.rmc = NULL;
}

/**
 * mesh_rmc_check - Check frame in recent multicast cache and add if absent.
 *
 * @sa:		source address
 * @mesh_hdr:	mesh_header
 *
 * Returns: 0 if the frame is not in the cache, nonzero otherwise.
 *
 * Checks using the source address and the mesh sequence number if we have
 * received this frame lately. If the frame is not in the cache, it is added to
 * it.
 */
int mesh_rmc_check(u8 *sa, struct ieee80211s_hdr *mesh_hdr,
		   struct ieee80211_sub_if_data *sdata)
{
	struct mesh_rmc *rmc = sdata->u.mesh.rmc;
	u32 seqnum = 0;
	int entries = 0;
	u8 idx;
	struct rmc_entry *p, *n;

	/* Don't care about endianness since only match matters */
	memcpy(&seqnum, &mesh_hdr->seqnum, sizeof(mesh_hdr->seqnum));
	idx = le32_to_cpu(mesh_hdr->seqnum) & rmc->idx_mask;
	list_for_each_entry_safe(p, n, &rmc->bucket[idx].list, list) {
		++entries;
		if (time_after(jiffies, p->exp_time) ||
				(entries == RMC_QUEUE_MAX_LEN)) {
			list_del(&p->list);
			kmem_cache_free(rm_cache, p);
			--entries;
		} else if ((seqnum == p->seqnum) &&
			   (memcmp(sa, p->sa, ETH_ALEN) == 0))
			return -1;
	}

	p = kmem_cache_alloc(rm_cache, GFP_ATOMIC);
	if (!p) {
		printk(KERN_DEBUG "o11s: could not allocate RMC entry\n");
		return 0;
	}
	p->seqnum = seqnum;
	p->exp_time = jiffies + RMC_TIMEOUT;
	memcpy(p->sa, sa, ETH_ALEN);
	list_add(&p->list, &rmc->bucket[idx].list);
	return 0;
}

void mesh_mgmt_ies_add(struct sk_buff *skb, struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_supported_band *sband;
	u8 *pos;
	int len, i, rate;
	u8 neighbors;

	sband = local->hw.wiphy->bands[local->hw.conf.channel->band];
	len = sband->n_bitrates;
	if (len > 8)
		len = 8;
	pos = skb_put(skb, len + 2);
	*pos++ = WLAN_EID_SUPP_RATES;
	*pos++ = len;
	for (i = 0; i < len; i++) {
		rate = sband->bitrates[i].bitrate;
		*pos++ = (u8) (rate / 5);
	}

	if (sband->n_bitrates > len) {
		pos = skb_put(skb, sband->n_bitrates - len + 2);
		*pos++ = WLAN_EID_EXT_SUPP_RATES;
		*pos++ = sband->n_bitrates - len;
		for (i = len; i < sband->n_bitrates; i++) {
			rate = sband->bitrates[i].bitrate;
			*pos++ = (u8) (rate / 5);
		}
	}

	if (sband->band == IEEE80211_BAND_2GHZ) {
		pos = skb_put(skb, 2 + 1);
		*pos++ = WLAN_EID_DS_PARAMS;
		*pos++ = 1;
		*pos++ = ieee80211_frequency_to_channel(local->hw.conf.channel->center_freq);
	}

	pos = skb_put(skb, 2 + sdata->u.mesh.mesh_id_len);
	*pos++ = WLAN_EID_MESH_ID;
	*pos++ = sdata->u.mesh.mesh_id_len;
	if (sdata->u.mesh.mesh_id_len)
		memcpy(pos, sdata->u.mesh.mesh_id, sdata->u.mesh.mesh_id_len);

	pos = skb_put(skb, 2 + sizeof(struct ieee80211_meshconf_ie));
	*pos++ = WLAN_EID_MESH_CONFIG;
	*pos++ = sizeof(struct ieee80211_meshconf_ie);

	/* Active path selection protocol ID */
	*pos++ = sdata->u.mesh.mesh_pp_id;

	/* Active path selection metric ID   */
	*pos++ = sdata->u.mesh.mesh_pm_id;

	/* Congestion control mode identifier */
	*pos++ = sdata->u.mesh.mesh_cc_id;

	/* Synchronization protocol identifier */
	*pos++ = sdata->u.mesh.mesh_sp_id;

	/* Authentication Protocol identifier */
	*pos++ = sdata->u.mesh.mesh_auth_id;

	/* Mesh Formation Info - number of neighbors */
	neighbors = atomic_read(&sdata->u.mesh.mshstats.estab_plinks);
	/* Number of neighbor mesh STAs or 15 whichever is smaller */
	neighbors = (neighbors > 15) ? 15 : neighbors;
	*pos++ = neighbors << 1;

	/* Mesh capability */
	sdata->u.mesh.accepting_plinks = mesh_plink_availables(sdata);
	*pos = MESHCONF_CAPAB_FORWARDING;
	*pos++ |= sdata->u.mesh.accepting_plinks ?
	    MESHCONF_CAPAB_ACCEPT_PLINKS : 0x00;
	*pos++ = 0x00;

	return;
}

u32 mesh_table_hash(u8 *addr, struct ieee80211_sub_if_data *sdata, struct mesh_table *tbl)
{
	/* Use last four bytes of hw addr and interface index as hash index */
	return jhash_2words(*(u32 *)(addr+2), sdata->dev->ifindex, tbl->hash_rnd)
		& tbl->hash_mask;
}

struct mesh_table *mesh_table_alloc(int size_order)
{
	int i;
	struct mesh_table *newtbl;

	newtbl = kmalloc(sizeof(struct mesh_table), GFP_KERNEL);
	if (!newtbl)
		return NULL;

	newtbl->hash_buckets = kzalloc(sizeof(struct hlist_head) *
			(1 << size_order), GFP_KERNEL);

	if (!newtbl->hash_buckets) {
		kfree(newtbl);
		return NULL;
	}

	newtbl->hashwlock = kmalloc(sizeof(spinlock_t) *
			(1 << size_order), GFP_KERNEL);
	if (!newtbl->hashwlock) {
		kfree(newtbl->hash_buckets);
		kfree(newtbl);
		return NULL;
	}

	newtbl->size_order = size_order;
	newtbl->hash_mask = (1 << size_order) - 1;
	atomic_set(&newtbl->entries,  0);
	get_random_bytes(&newtbl->hash_rnd,
			sizeof(newtbl->hash_rnd));
	for (i = 0; i <= newtbl->hash_mask; i++)
		spin_lock_init(&newtbl->hashwlock[i]);

	return newtbl;
}


static void ieee80211_mesh_path_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata =
		(struct ieee80211_sub_if_data *) data;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct ieee80211_local *local = sdata->local;

	if (local->quiescing) {
		set_bit(TMR_RUNNING_MP, &ifmsh->timers_running);
		return;
	}

	ieee80211_queue_work(&local->hw, &ifmsh->work);
}

static void ieee80211_mesh_path_root_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata =
		(struct ieee80211_sub_if_data *) data;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct ieee80211_local *local = sdata->local;

	set_bit(MESH_WORK_ROOT, &ifmsh->wrkq_flags);

	if (local->quiescing) {
		set_bit(TMR_RUNNING_MPR, &ifmsh->timers_running);
		return;
	}

	ieee80211_queue_work(&local->hw, &ifmsh->work);
}

void ieee80211_mesh_root_setup(struct ieee80211_if_mesh *ifmsh)
{
	if (ifmsh->mshcfg.dot11MeshHWMPRootMode)
		set_bit(MESH_WORK_ROOT, &ifmsh->wrkq_flags);
	else {
		clear_bit(MESH_WORK_ROOT, &ifmsh->wrkq_flags);
		/* stop running timer */
		del_timer_sync(&ifmsh->mesh_path_root_timer);
	}
}

/**
 * ieee80211_fill_mesh_addresses - fill addresses of a locally originated mesh frame
 * @hdr:    	802.11 frame header
 * @fc:		frame control field
 * @meshda:	destination address in the mesh
 * @meshsa:	source address address in the mesh.  Same as TA, as frame is
 *              locally originated.
 *
 * Return the length of the 802.11 (does not include a mesh control header)
 */
int ieee80211_fill_mesh_addresses(struct ieee80211_hdr *hdr, __le16 *fc,
				  const u8 *meshda, const u8 *meshsa)
{
	if (is_multicast_ether_addr(meshda)) {
		*fc |= cpu_to_le16(IEEE80211_FCTL_FROMDS);
		/* DA TA SA */
		memcpy(hdr->addr1, meshda, ETH_ALEN);
		memcpy(hdr->addr2, meshsa, ETH_ALEN);
		memcpy(hdr->addr3, meshsa, ETH_ALEN);
		return 24;
	} else {
		*fc |= cpu_to_le16(IEEE80211_FCTL_FROMDS |
				IEEE80211_FCTL_TODS);
		/* RA TA DA SA */
		memset(hdr->addr1, 0, ETH_ALEN);   /* RA is resolved later */
		memcpy(hdr->addr2, meshsa, ETH_ALEN);
		memcpy(hdr->addr3, meshda, ETH_ALEN);
		memcpy(hdr->addr4, meshsa, ETH_ALEN);
		return 30;
	}
}

/**
 * ieee80211_new_mesh_header - create a new mesh header
 * @meshhdr:    uninitialized mesh header
 * @sdata:	mesh interface to be used
 * @addr4:	addr4 of the mesh frame (1st in ae header)
 *              may be NULL
 * @addr5:	addr5 of the mesh frame (1st or 2nd in ae header)
 *              may be NULL unless addr6 is present
 * @addr6:	addr6 of the mesh frame (2nd or 3rd in ae header)
 * 		may be NULL unless addr5 is present
 *
 * Return the header length.
 */
int ieee80211_new_mesh_header(struct ieee80211s_hdr *meshhdr,
		struct ieee80211_sub_if_data *sdata, char *addr4,
		char *addr5, char *addr6)
{
	int aelen = 0;
	memset(meshhdr, 0, sizeof(*meshhdr));
	meshhdr->ttl = sdata->u.mesh.mshcfg.dot11MeshTTL;
	put_unaligned(cpu_to_le32(sdata->u.mesh.mesh_seqnum), &meshhdr->seqnum);
	sdata->u.mesh.mesh_seqnum++;
	if (addr4) {
		meshhdr->flags |= MESH_FLAGS_AE_A4;
		aelen += ETH_ALEN;
		memcpy(meshhdr->eaddr1, addr4, ETH_ALEN);
	}
	if (addr5 && addr6) {
		meshhdr->flags |= MESH_FLAGS_AE_A5_A6;
		aelen += 2 * ETH_ALEN;
		if (!addr4) {
			memcpy(meshhdr->eaddr1, addr5, ETH_ALEN);
			memcpy(meshhdr->eaddr2, addr6, ETH_ALEN);
		} else {
			memcpy(meshhdr->eaddr2, addr5, ETH_ALEN);
			memcpy(meshhdr->eaddr3, addr6, ETH_ALEN);
		}
	}
	return 6 + aelen;
}

static void ieee80211_mesh_housekeeping(struct ieee80211_sub_if_data *sdata,
			   struct ieee80211_if_mesh *ifmsh)
{
	bool free_plinks;

#ifdef CONFIG_MAC80211_VERBOSE_DEBUG
	printk(KERN_DEBUG "%s: running mesh housekeeping\n",
	       sdata->name);
#endif

	ieee80211_sta_expire(sdata, IEEE80211_MESH_PEER_INACTIVITY_LIMIT);
	mesh_path_expire(sdata);

	free_plinks = mesh_plink_availables(sdata);
	if (free_plinks != sdata->u.mesh.accepting_plinks)
		ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_BEACON);

	mod_timer(&ifmsh->housekeeping_timer,
		  round_jiffies(jiffies + IEEE80211_MESH_HOUSEKEEPING_INTERVAL));
}

#ifdef MICROHARD
void ieee80211_send_trace_path (struct ieee80211_sub_if_data *sdata, u8 *dst)
{
	struct ieee80211_local *local = sdata->local;
	struct sk_buff *skb = dev_alloc_skb(local->hw.extra_tx_headroom + 400);
	struct ieee80211_mgmt *mgmt;
	struct mesh_path *mpath;
	
	u8 *pos, next_hop[ETH_ALEN];
	u8 ie_len;

	if (!skb)
		return ;
	
	rcu_read_lock();
	mpath = mesh_path_lookup(dst, sdata);
	if (mpath==NULL) {
#ifdef DEBUG
		printk("\n TRACE PATH : Path to %2X:%2X:%2X:%2X:%2X:%2X   not found",
			dst[0],dst[1],dst[2],dst[3],dst[4],dst[5]);
#endif
		dev_kfree_skb(skb);
		return;	
	}
	memcpy(next_hop,mpath->next_hop->sta.addr,ETH_ALEN);
	rcu_read_unlock();
			
#ifdef DEBUG
	printk(KERN_INFO"\nSending Trace_path:: dst=%2X:%2X:%2X:%2X:%2X:%2X  ",
			dst[0],dst[1],dst[2],dst[3],dst[4],dst[5]);
#endif
	
	if (sdata->vif.type !=  NL80211_IFTYPE_MESH_POINT)
	{
		dev_kfree_skb(skb);
		return ;
	}

	skb_reserve(skb, local->hw.extra_tx_headroom);
	/* 25 is the size of the common mgmt part (24) plus the size of the
	 * common action part (1)
	 */
	mgmt = (struct ieee80211_mgmt *)
		skb_put(skb, 25 + sizeof(mgmt->u.action.u.mesh_action));
	memset(mgmt, 0, 25 + sizeof(mgmt->u.action.u.mesh_action));
	mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT |
					  IEEE80211_STYPE_ACTION);

	memcpy(mgmt->da, next_hop, ETH_ALEN);
	memcpy(mgmt->sa, sdata->dev->dev_addr, ETH_ALEN);
	/* BSSID is left zeroed, wildcard value */
	mgmt->u.action.category = MESH_TRACE_PATH_CATEGORY;
	mgmt->u.action.u.mesh_action.action_code = 0;

	ie_len=13;
	pos = skb_put(skb, 2 + ie_len);
	*pos++ = WLAN_EID_TR_REQ;
	*pos++ = ie_len;
	*pos++ = 0;
	memcpy(pos, sdata->dev->dev_addr, ETH_ALEN);
	pos += ETH_ALEN;
	memcpy(pos, dst, ETH_ALEN);
	pos += ETH_ALEN;
	ieee80211_tx_skb(sdata, skb);
	return ;
}

void mesh_rx_trace_path(struct ieee80211_sub_if_data *sdata, struct ieee80211_mgmt *mgmt, size_t len, struct sk_buff *skb)
{
	
	struct ieee802_11_elems elems;
	//struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	//struct ieee80211_hdr *hdr;
	struct mesh_path *mpath;
	size_t baselen;
	u8 *elems_trace;
	u8 *skb_ptr, *pos;
	int i;

	struct sk_buff *skb2 = skb_clone(skb, GFP_ATOMIC);

	if (!skb2) return;
	
	//printk("\n Rececived a trace path packet");
	//hdr = (struct ieee80211_hdr *) skb->data;
	baselen = (u8 *) mgmt->u.action.u.mesh_action.variable - (u8 *) mgmt;
	ieee802_11_parse_elems(mgmt->u.action.u.mesh_action.variable,
			len - baselen, &elems);

	if(elems.tr_req)
	{
		elems_trace=elems.tr_req;
		pos=mgmt->u.action.u.mesh_action.variable;
		if(memcmp(sdata->dev->dev_addr,elems_trace+7,ETH_ALEN))
		{
#ifdef DEBUG
			printk("\n received trace route rreq ...forwarding");
#endif
			(*elems_trace)++;
			skb_ptr=skb_put(skb2, ETH_ALEN);
			memcpy(skb_ptr,sdata->dev->dev_addr,ETH_ALEN);
			*(pos+1)=*(pos+1)+6;
			rcu_read_lock();
			mpath = mesh_path_lookup(elems_trace+7, sdata);
			if (mpath==NULL) {
#ifdef DEBUG
				printk("\n rx_trace_path: Path not found\n"); 
#endif
				goto out;
			}
			memcpy(mgmt->da,mpath->next_hop->sta.addr,ETH_ALEN);
			rcu_read_unlock();
			ieee80211_tx_skb(sdata, skb2);
			return;
		}
		else 
		{	
#ifdef DEBUG
			printk("\n received trace route rreq from %2x:%2x:%2x:%2x:%2x:%2x...replying", *(elems_trace+1),*(elems_trace+2),*(elems_trace+3),*(elems_trace+4),*(elems_trace+5),*(elems_trace+6));
#endif
			*pos=WLAN_EID_TR_REP;
			(*elems_trace)++;
			rcu_read_lock();
			mpath = mesh_path_lookup(elems_trace+1, sdata);
			if (mpath==NULL) {printk("\n rx_trace_path: Path not found\n"); goto out;}
			memcpy(mgmt->da,mpath->next_hop->sta.addr,ETH_ALEN);
			rcu_read_unlock();
			memcpy(mgmt->sa,sdata->dev->dev_addr,ETH_ALEN);
			ieee80211_tx_skb(sdata, skb2);
			return;
		}
	}
	else if(elems.tr_rep)
	{	
		elems_trace=elems.tr_rep;
		if(memcmp(sdata->dev->dev_addr,elems_trace+1,ETH_ALEN))
		{
#ifdef DEBUG
			printk("\n received trace route rrep ...forwarding");
#endif
			rcu_read_lock();
			mpath = mesh_path_lookup(elems_trace+1, sdata);
			if (mpath==NULL){
#ifdef DEBUG
				printk("\n rx_trace_path: Path not found\n"); 
#endif
				goto out;
			}
			memcpy(mgmt->da,mpath->next_hop->sta.addr,ETH_ALEN);
			rcu_read_unlock();
			ieee80211_tx_skb(sdata, skb2);	
			return;
		}
		else
		{
#ifdef DEBUG
			printk("\n TRACE PATH: HOP COUNT=%d\n%2x:%2x:%2x:%2x:%2x:%2x",*((unsigned char *)elems_trace), *(elems_trace+1),*(elems_trace+2),*(elems_trace+3),*(elems_trace+4),*(elems_trace+5),*(elems_trace+6));
		
			for(i=1;i<(*elems_trace);i++)
			{	if (i%3==0) printk("\n");
				pos=elems_trace+6*(i-1)+13;
				printk(" -> %2x:%2x:%2x:%2x:%2x:%2x", *pos,*(pos+1),*(pos+2),*(pos+3),*(pos+4),*(pos+5));
			}
			printk(" -> %2x:%2x:%2x:%2x:%2x:%2x  \n", *(elems_trace+7),*(elems_trace+8),*(elems_trace+9),*(elems_trace+10),*(elems_trace+11),*(elems_trace+12));
#endif
		}
	}

out:

kfree_skb(skb2);

}


void ieee80211_pro_rreq_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata = (void *) data;
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

	ifmsh->pro_rreq = true;
	queue_work(local->workqueue, &ifmsh->work);
}

void ieee80211_portal_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata = (void *) data;
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

#ifdef DEBUG
	printk(KERN_INFO"ieee80211_portal_timer\n");
#endif

	ifmsh->portal_timer_flag = true;
	queue_work(local->workqueue, &ifmsh->work);
}

void ieee80211_pro_rreq(struct ieee80211_sub_if_data *sdata, struct ieee80211_if_mesh *ifmsh)
{
	//u8 pro_dst[ETH_ALEN]={255,255,255,255,255,255};
	u8 pro_dst[ETH_ALEN]={255,255,255,255,255,255};
	u8 ttl=10;
	u32 lifetime = default_lifetime(sdata);
	u8 bdcst_add[6]={255,255,255,255,255,255};
	

	if( (memcmp (ifmsh->root, sdata->dev->dev_addr, ETH_ALEN)==0)
		|| (memcmp (ifmsh->root, bdcst_add, ETH_ALEN))==0) {

		if (time_after(jiffies, ifmsh->last_sn_update + net_traversal_jiffies(sdata)) ||
		    	time_before(jiffies, ifmsh->last_sn_update)) {
				ifmsh->sn++;
				ifmsh->last_sn_update = jiffies;
			}	
#ifdef DEBUG
		printk("Sending proactive rreq\n");	
#endif
		mesh_path_sel_frame_tx(MPATH_PREQ, 0, sdata->dev->dev_addr,
			cpu_to_le32(ifmsh->sn), 0xC0, pro_dst,cpu_to_le32(0), sdata->dev->broadcast, 0,
			ttl, cpu_to_le32(lifetime), 0,cpu_to_le32(ifmsh->preq_id++),sdata);
	}

	ifmsh->pro_rreq=false;
	if (ifmsh->root_node)
		mod_timer(&ifmsh->pro_rreq_timer,round_jiffies(jiffies + (ifmsh->mshcfg.pro_rreq_interval*HZ)/1000));
#ifdef DEBUG
	else
		printk("\nieee80211_pro_rreq::ROOT_FLAG = false\n");
#endif
}

void ieee80211_portal_pann(struct ieee80211_sub_if_data *sdata, struct ieee80211_if_mesh *ifmsh)
{
	u8 bdcst_add[6]={255,255,255,255,255,255};
#ifdef DEBUG
	printk("\nieee80211_portal_pann\n");
#endif
	ifmsh->portal_timer_flag=false;
	if( (memcmp (ifmsh->root, sdata->dev->dev_addr, ETH_ALEN))
		&& (memcmp (ifmsh->root, bdcst_add, ETH_ALEN))
	) {
#ifdef DEBUG
		printk("\nSending PANN\n");
#endif
		mesh_portal_send_pann(sdata,ifmsh->root_nexthop,sdata->dev->dev_addr,0,0,
			ifmsh->mshcfg.dot11MeshTTL, ifmsh->pann_seq_num);
		ifmsh->pann_seq_num++;
	}
	if (ifmsh->portal_flag)
		mod_timer(&ifmsh->portal_timer,
		  	round_jiffies(jiffies + IEEE80211_PORTAL_INTERVAL));
}
#endif /* MICROHARD */

static void ieee80211_mesh_rootpath(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

	mesh_path_tx_root_frame(sdata);
	mod_timer(&ifmsh->mesh_path_root_timer,
		  round_jiffies(jiffies + IEEE80211_MESH_RANN_INTERVAL));
}

#ifdef CONFIG_PM
void ieee80211_mesh_quiesce(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

	/* might restart the timer but that doesn't matter */
	cancel_work_sync(&ifmsh->work);

	/* use atomic bitops in case both timers fire at the same time */

	if (del_timer_sync(&ifmsh->housekeeping_timer))
		set_bit(TMR_RUNNING_HK, &ifmsh->timers_running);
	if (del_timer_sync(&ifmsh->mesh_path_timer))
		set_bit(TMR_RUNNING_MP, &ifmsh->timers_running);
	if (del_timer_sync(&ifmsh->mesh_path_root_timer))
		set_bit(TMR_RUNNING_MPR, &ifmsh->timers_running);
}

void ieee80211_mesh_restart(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

	if (test_and_clear_bit(TMR_RUNNING_HK, &ifmsh->timers_running))
		add_timer(&ifmsh->housekeeping_timer);
	if (test_and_clear_bit(TMR_RUNNING_MP, &ifmsh->timers_running))
		add_timer(&ifmsh->mesh_path_timer);
	if (test_and_clear_bit(TMR_RUNNING_MPR, &ifmsh->timers_running))
		add_timer(&ifmsh->mesh_path_root_timer);
	ieee80211_mesh_root_setup(ifmsh);
}
#endif

void ieee80211_start_mesh(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct ieee80211_local *local = sdata->local;

#ifdef MICROHARD
	u8 root_local[ETH_ALEN]={255,255,255,255,255,255};

	memcpy(ifmsh->root, root_local, ETH_ALEN);
	memcpy(ifmsh->root_nexthop, root_local, ETH_ALEN);

	ifmsh->pro_rreq_dsn=0;
	memset(ifmsh->depend_mp,0,11);
	//memset(ifmsh->depend_mp+11,0,50);
	//printk(KERN_INFO"The present ROOT is %x:%x:%x:%x:%x:%x\n",ifmsh->root[0],ifmsh->root[1],ifmsh->root[2],ifmsh->root[3],ifmsh->root[4],ifmsh->root[5] );

	if (ifmsh->root_node)
	{ ieee80211_pro_rreq_timer((unsigned long)sdata);
	  memcpy(ifmsh->root, sdata->dev->dev_addr, ETH_ALEN);
	}

	if (ifmsh->portal_flag){
		ieee80211_portal_timer((unsigned long)sdata);
	}
#endif

	set_bit(MESH_WORK_HOUSEKEEPING, &ifmsh->wrkq_flags);
	ieee80211_mesh_root_setup(ifmsh);
	ieee80211_queue_work(&local->hw, &ifmsh->work);
	sdata->vif.bss_conf.beacon_int = MESH_DEFAULT_BEACON_INTERVAL;
	ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_BEACON |
						BSS_CHANGED_BEACON_ENABLED |
						BSS_CHANGED_BEACON_INT);
}

void ieee80211_stop_mesh(struct ieee80211_sub_if_data *sdata)
{
	del_timer_sync(&sdata->u.mesh.housekeeping_timer);
	del_timer_sync(&sdata->u.mesh.mesh_path_root_timer);
	/*
	 * If the timer fired while we waited for it, it will have
	 * requeued the work. Now the work will be running again
	 * but will not rearm the timer again because it checks
	 * whether the interface is running, which, at this point,
	 * it no longer is.
	 */
	cancel_work_sync(&sdata->u.mesh.work);

	/*
	 * When we get here, the interface is marked down.
	 * Call synchronize_rcu() to wait for the RX path
	 * should it be using the interface and enqueuing
	 * frames at this very time on another CPU.
	 */
	rcu_barrier(); /* Wait for RX path and call_rcu()'s */
	skb_queue_purge(&sdata->u.mesh.skb_queue);
}

static void ieee80211_mesh_rx_bcn_presp(struct ieee80211_sub_if_data *sdata,
					u16 stype,
					struct ieee80211_mgmt *mgmt,
					size_t len,
					struct ieee80211_rx_status *rx_status)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee802_11_elems elems;
	struct ieee80211_channel *channel;
	u32 supp_rates = 0;
	size_t baselen;
	int freq;
	enum ieee80211_band band = rx_status->band;

	/* ignore ProbeResp to foreign address */
	if (stype == IEEE80211_STYPE_PROBE_RESP &&
	    compare_ether_addr(mgmt->da, sdata->vif.addr))
		return;

	baselen = (u8 *) mgmt->u.probe_resp.variable - (u8 *) mgmt;
	if (baselen > len)
		return;

	ieee802_11_parse_elems(mgmt->u.probe_resp.variable, len - baselen,
			       &elems);

	if (elems.ds_params && elems.ds_params_len == 1)
		freq = ieee80211_channel_to_frequency(elems.ds_params[0]);
	else
		freq = rx_status->freq;

	channel = ieee80211_get_channel(local->hw.wiphy, freq);

	if (!channel || channel->flags & IEEE80211_CHAN_DISABLED)
		return;

	if (elems.mesh_id && elems.mesh_config &&
	    mesh_matches_local(&elems, sdata)) {
		supp_rates = ieee80211_sta_get_rates(local, &elems, band);

		mesh_neighbour_update(mgmt->sa, supp_rates, sdata,
				      mesh_peer_accepts_plinks(&elems));
	}
}

static void ieee80211_mesh_rx_mgmt_action(struct ieee80211_sub_if_data *sdata,
					  struct ieee80211_mgmt *mgmt,
					  size_t len,
					  struct ieee80211_rx_status *rx_status
#ifdef MICROHARD
		              , struct sk_buff *skb
#endif
										  )
{
	switch (mgmt->u.action.category) {
	case MESH_PLINK_CATEGORY:
		mesh_rx_plink_frame(sdata, mgmt, len, rx_status);
		break;
	case MESH_PATH_SEL_CATEGORY:
		mesh_rx_path_sel_frame(sdata, mgmt, len);
		break;
#ifdef MICROHARD
	case MESH_PROXY_CATEGORY:
		mesh_rx_proxy_frame(sdata, mgmt, len);
		break;
	case MESH_PORTAL_CATEGORY:
		mesh_rx_portal_frame(sdata, mgmt, len);
		break;
	case MESH_TRACE_PATH_CATEGORY:
		mesh_rx_trace_path(sdata, mgmt, len, skb);
		break;
#endif
	}
}

static void ieee80211_mesh_rx_queued_mgmt(struct ieee80211_sub_if_data *sdata,
					  struct sk_buff *skb)
{
	struct ieee80211_rx_status *rx_status;
	struct ieee80211_if_mesh *ifmsh;
	struct ieee80211_mgmt *mgmt;
	u16 stype;

	ifmsh = &sdata->u.mesh;

	rx_status = IEEE80211_SKB_RXCB(skb);
	mgmt = (struct ieee80211_mgmt *) skb->data;
	stype = le16_to_cpu(mgmt->frame_control) & IEEE80211_FCTL_STYPE;

	switch (stype) {
	case IEEE80211_STYPE_PROBE_RESP:
	case IEEE80211_STYPE_BEACON:
		ieee80211_mesh_rx_bcn_presp(sdata, stype, mgmt, skb->len,
					    rx_status);
		break;
	case IEEE80211_STYPE_ACTION:
		//printk("\n the skb_tailroom is %d",skb_tailroom(skb));
#ifdef MICROHARD
		ieee80211_mesh_rx_mgmt_action(sdata, mgmt, skb->len, rx_status, skb);
#else
		ieee80211_mesh_rx_mgmt_action(sdata, mgmt, skb->len, rx_status);
#endif
		break;
	}

	kfree_skb(skb);
}

static void ieee80211_mesh_work(struct work_struct *work)
{
	struct ieee80211_sub_if_data *sdata =
		container_of(work, struct ieee80211_sub_if_data, u.mesh.work);
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct sk_buff *skb;

	if (!ieee80211_sdata_running(sdata))
		return;

	if (local->scanning)
		return;

	while ((skb = skb_dequeue(&ifmsh->skb_queue)))
		ieee80211_mesh_rx_queued_mgmt(sdata, skb);

	if (ifmsh->preq_queue_len &&
	    time_after(jiffies,
		       ifmsh->last_preq + msecs_to_jiffies(ifmsh->mshcfg.dot11MeshHWMPpreqMinInterval)))
		mesh_path_start_discovery(sdata);

	if (test_and_clear_bit(MESH_WORK_GROW_MPATH_TABLE, &ifmsh->wrkq_flags))
		mesh_mpath_table_grow();

	if (test_and_clear_bit(MESH_WORK_GROW_MPATH_TABLE, &ifmsh->wrkq_flags))
		mesh_mpp_table_grow();

	if (test_and_clear_bit(MESH_WORK_HOUSEKEEPING, &ifmsh->wrkq_flags))
		ieee80211_mesh_housekeeping(sdata, ifmsh);

	if (test_and_clear_bit(MESH_WORK_ROOT, &ifmsh->wrkq_flags))
		ieee80211_mesh_rootpath(sdata);

#ifdef MICROHARD
	if (ifmsh->pro_rreq)
		ieee80211_pro_rreq(sdata, ifmsh);

	if (ifmsh->portal_timer_flag) {
#ifdef DEBUG
		printk(KERN_INFO"ieee80211_mesh_work: PORTAL_TIMER_FLAG = true\n");
#endif
		ifmsh->portal_timer_flag=false;
		ieee80211_portal_pann(sdata, ifmsh);
		}
#endif
}

void ieee80211_mesh_notify_scan_completed(struct ieee80211_local *local)
{
	struct ieee80211_sub_if_data *sdata;

	rcu_read_lock();
	list_for_each_entry_rcu(sdata, &local->interfaces, list)
		if (ieee80211_vif_is_mesh(&sdata->vif))
			ieee80211_queue_work(&local->hw, &sdata->u.mesh.work);
	rcu_read_unlock();
}

void ieee80211_mesh_init_sdata(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

	INIT_WORK(&ifmsh->work, ieee80211_mesh_work);
	setup_timer(&ifmsh->housekeeping_timer,
		    ieee80211_mesh_housekeeping_timer,
		    (unsigned long) sdata);
	skb_queue_head_init(&sdata->u.mesh.skb_queue);

#ifdef MICROHARD
	ifmsh->root_node = SET_AS_ROOT;
#endif
	ifmsh->mshcfg.dot11MeshRetryTimeout = MESH_RET_T;
	ifmsh->mshcfg.dot11MeshConfirmTimeout = MESH_CONF_T;
	ifmsh->mshcfg.dot11MeshHoldingTimeout = MESH_HOLD_T;
	ifmsh->mshcfg.dot11MeshMaxRetries = MESH_MAX_RETR;
	ifmsh->mshcfg.dot11MeshTTL = MESH_TTL;
	ifmsh->mshcfg.auto_open_plinks = true;
	ifmsh->mshcfg.dot11MeshMaxPeerLinks =
		MESH_MAX_ESTAB_PLINKS;
	ifmsh->mshcfg.dot11MeshHWMPactivePathTimeout =
		MESH_PATH_TIMEOUT;
	ifmsh->mshcfg.dot11MeshHWMPpreqMinInterval =
		MESH_PREQ_MIN_INT;
	ifmsh->mshcfg.dot11MeshHWMPnetDiameterTraversalTime =
		MESH_DIAM_TRAVERSAL_TIME;
	ifmsh->mshcfg.dot11MeshHWMPmaxPREQretries =
		MESH_MAX_PREQ_RETRIES;
	ifmsh->mshcfg.path_refresh_time =
		MESH_PATH_REFRESH_TIME;
	ifmsh->mshcfg.min_discovery_timeout =
		MESH_MIN_DISCOVERY_TIMEOUT;
#ifdef MICROHARD
	ifmsh->mshcfg.pro_rreq_interval=
		IEEE80211_PRO_RREQ_INTERVAL;
#endif
	ifmsh->accepting_plinks = true;
	ifmsh->preq_id = 0;
	ifmsh->sn = 0;
	atomic_set(&ifmsh->mpaths, 0);
	mesh_rmc_init(sdata);
	ifmsh->last_preq = jiffies;
	/* Allocate all mesh structures when creating the first mesh interface. */
	if (!mesh_allocated)
		ieee80211s_init();
	mesh_ids_set_default(ifmsh);
	setup_timer(&ifmsh->mesh_path_timer,
		    ieee80211_mesh_path_timer,
		    (unsigned long) sdata);
	setup_timer(&ifmsh->mesh_path_root_timer,
		    ieee80211_mesh_path_root_timer,
		    (unsigned long) sdata);

#ifdef MICROHARD
	/* Proactive RREQ Timer */
	//if (ifmsh->root_node)		
		setup_timer(&ifmsh->pro_rreq_timer, ieee80211_pro_rreq_timer,
		    		(unsigned long) sdata);

	/* Portan Announcement Timer */
	//if (ifmsh->portal_flag)		
		setup_timer(&ifmsh->portal_timer, ieee80211_portal_timer,
		    		(unsigned long) sdata);
#endif

	INIT_LIST_HEAD(&ifmsh->preq_queue.list);
	spin_lock_init(&ifmsh->mesh_preq_queue_lock);

#ifdef MICROHARD
	/* Setting portal list */
	INIT_LIST_HEAD(&ifmsh->portal_list.list);
	spin_lock_init(&ifmsh->portal_list_lock);
	ifmsh->portal_list_len=0;
#endif
}

ieee80211_rx_result
ieee80211_mesh_rx_mgmt(struct ieee80211_sub_if_data *sdata, struct sk_buff *skb)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct ieee80211_mgmt *mgmt;
	u16 fc;

	if (skb->len < 24)
		return RX_DROP_MONITOR;

	mgmt = (struct ieee80211_mgmt *) skb->data;
	fc = le16_to_cpu(mgmt->frame_control);

	switch (fc & IEEE80211_FCTL_STYPE) {
	case IEEE80211_STYPE_ACTION:
	case IEEE80211_STYPE_PROBE_RESP:
	case IEEE80211_STYPE_BEACON:
		skb_queue_tail(&ifmsh->skb_queue, skb);
		ieee80211_queue_work(&local->hw, &ifmsh->work);
		return RX_QUEUED;
	}

	return RX_CONTINUE;
}
