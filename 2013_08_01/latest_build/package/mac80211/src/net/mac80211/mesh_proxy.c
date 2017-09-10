
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
#include <linux/list.h>
#include "ieee80211_i.h"
#include "mesh_proxy.h"
#include "wme.h"

int mesh_proxy_update(struct ieee80211_sub_if_data *sdata, u8 *proxy, u8 *sta_mac, u8 flag, u8 seq_num, 
u8 *destination , u8 *next_hop, u8 *source)
{
	struct ieee80211_local *local = sdata->local;
	struct sk_buff *skb = dev_alloc_skb(local->hw.extra_tx_headroom + 400);
	struct ieee80211_mgmt *mgmt;
	//struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u8 *pos;
	int ie_len;
#ifdef DEBUG
	printk(KERN_INFO"Sending PU:: proxy=%X:%X:%X:%X:%X:%X  sta=%X:%X:%X:%X:%X:%X \n",
			proxy[0],proxy[1],proxy[2],proxy[3],proxy[4],proxy[5],
			sta_mac[0],sta_mac[1],sta_mac[2],sta_mac[3],sta_mac[4],sta_mac[5]);
#endif

	if (sdata->vif.type !=  NL80211_IFTYPE_MESH_POINT)
		return -EINVAL;
	if (!skb)
		return -1;
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
	mgmt->u.action.category = MESH_PROXY_CATEGORY;
	mgmt->u.action.u.mesh_action.action_code = PU;

	ie_len=27;
	pos = skb_put(skb, 2 + ie_len);
	*pos++ = WLAN_EID_PU;
	*pos++ = ie_len;
	*pos++ = flag;
	*pos++ = seq_num;//cpu_to_le32(ifmsh->pu_seq_num++)
	memcpy(pos, proxy, ETH_ALEN);
	pos += ETH_ALEN;
	*pos++ = 1;
	memcpy(pos, sta_mac, ETH_ALEN);
	pos += ETH_ALEN;
	memcpy(pos, destination, ETH_ALEN);
	pos += ETH_ALEN;
	memcpy(pos, source, ETH_ALEN);
	pos += ETH_ALEN;

	ieee80211_tx_skb(sdata, skb);
	return 0;
}

int mesh_proxy_send_puc(struct ieee80211_sub_if_data *sdata, u8 *nexthop, u8 *proxy, u8 seq_num)
{
	struct ieee80211_local *local = sdata->local;
	struct sk_buff *skb = dev_alloc_skb(local->hw.extra_tx_headroom + 400);
	struct ieee80211_mgmt *mgmt;
	u8 *pos;
	int ie_len;
#ifdef DEBUG
	printk(KERN_INFO"Sending PUC:: proxy=%x:%x:%x:%x:%x:%x  nexthop=%x:%x:%x:%x:%x:%x \n",
		proxy[0],proxy[1],proxy[2],proxy[3],proxy[4],proxy[5],
		nexthop[0],nexthop[1],nexthop[2],nexthop[3],nexthop[4],nexthop[5]);
#endif

	if (sdata->vif.type !=  NL80211_IFTYPE_MESH_POINT)
		return -EINVAL;
	if (!skb)
		return -1;
	skb_reserve(skb, local->hw.extra_tx_headroom);
	/* 25 is the size of the common mgmt part (24) plus the size of the
	 * common action part (1)
	 */
	mgmt = (struct ieee80211_mgmt *)
		skb_put(skb, 25 + sizeof(mgmt->u.action.u.mesh_action));
	memset(mgmt, 0, 25 + sizeof(mgmt->u.action.u.mesh_action));
	mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT |
					  IEEE80211_STYPE_ACTION);

	memcpy(mgmt->da, nexthop, ETH_ALEN);
	memcpy(mgmt->sa, sdata->dev->dev_addr, ETH_ALEN);
	/* BSSID is left zeroed, wildcard value */
	mgmt->u.action.category = MESH_PROXY_CATEGORY;
	mgmt->u.action.u.mesh_action.action_code = PUC;

	ie_len=8;
	pos = skb_put(skb, 2 + ie_len);
	*pos++ = WLAN_EID_PUC;
	*pos++ = ie_len;
	*pos++ = 0;//flag
	*pos++ = seq_num;
	memcpy(pos, proxy, ETH_ALEN);
	pos += ETH_ALEN;

	ieee80211_tx_skb(sdata, skb);
	return 0;
}



void mesh_rx_proxy_frame(struct ieee80211_sub_if_data *sdata, struct ieee80211_mgmt *mgmt,size_t len)
{
	struct ieee802_11_elems elems;
	//struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct mesh_path *mpath;
	size_t baselen;
	u8 seq_num;
	u8 *proxy_ie;
	u8 flag;
	u8 proxy_mac[ETH_ALEN], sta_mac[ETH_ALEN], dst[ETH_ALEN], src[ETH_ALEN];
	u8 nexthop[6];

	/* need action_code */
	if (len < IEEE80211_MIN_ACTION_SIZE + 1)
		return;

	baselen = (u8 *) mgmt->u.action.u.mesh_action.variable - (u8 *) mgmt;
	ieee802_11_parse_elems(mgmt->u.action.u.mesh_action.variable,
			len - baselen, &elems);

	switch (mgmt->u.action.u.mesh_action.action_code) {
	case PU:
#ifdef DEBUG
		printk(KERN_INFO "Received PU\n");
#endif
		if (!elems.pu)// || elems.pu_len != 12)
			/* Right now we support only one destination per PU */
			return;
		
		proxy_ie=elems.pu;
		flag=* (proxy_ie );
		seq_num=*(proxy_ie + 1);
		memcpy (proxy_mac, proxy_ie + 2, ETH_ALEN);
		memcpy (sta_mac, proxy_ie + 9, ETH_ALEN);
		memcpy (dst, proxy_ie + 15, ETH_ALEN);
		memcpy (src, proxy_ie + 21, ETH_ALEN);
		
		
		if (memcmp(dst, sdata->dev->dev_addr, ETH_ALEN))
			{
			rcu_read_lock();
			mpath = mesh_path_lookup(dst, sdata);
			if (!mpath || (!mpath->flags & MESH_PATH_ACTIVE) ) {
				rcu_read_unlock();
#ifdef DEBUG
				printk(KERN_INFO"  No path (or path not Active) to send PUC\n");
#endif
			}
			else {
				spin_lock_bh(&mpath->state_lock);
				memcpy(nexthop, mpath->next_hop->sta.addr, ETH_ALEN);
				spin_unlock_bh(&mpath->state_lock);
				rcu_read_unlock();
				mesh_proxy_update(sdata, proxy_mac, sta_mac, flag, seq_num, dst, nexthop, src);
			}
			
						
			}
		else {
			//add paths
			if (flag==ADD_PROXY_INFO)
			{
#ifdef DEBUG
			printk(KERN_INFO"  mesh_rx_proxy_frame:: adding path for %x:%x:%x:%x:%x:%x\n",
				sta_mac[0],sta_mac[1],sta_mac[2],sta_mac[3],sta_mac[4],sta_mac[5]);
#endif
			if (mesh_path_add_station(proxy_mac,NULL,sta_mac,sdata))
				{
#ifdef DEBUG
				printk(KERN_INFO"  mesh_rx_proxy_frame:: mesh_path_add_station failed\n");
#endif
				}
			}
			else if (flag==DEL_PROXY_INFO)
			{
#ifdef DEBUG
			printk(KERN_INFO"  mesh_rx_proxy_frame::deleting the path for %x:%x:%x:%x:%x:%x\n",
				sta_mac[0],sta_mac[1],sta_mac[2],sta_mac[3],sta_mac[4],sta_mac[5]);
#endif
			if (mesh_path_del(sta_mac,sdata))
				{
#ifdef DEBUG
				printk(KERN_INFO"  mesh_rx_proxy_frame:: mesh_path_del_station failed\n");
#endif
				}
			}
						
			//send puc
			rcu_read_lock();
			mpath = mesh_path_lookup(src, sdata);
			if (!mpath || (!mpath->flags & MESH_PATH_ACTIVE) ) {
				rcu_read_unlock();
#ifdef DEBUG
				printk(KERN_INFO"  No path (or path not Active) to send PUC\n");
#endif
			}
			else {
				spin_lock_bh(&mpath->state_lock);
				memcpy(nexthop, mpath->next_hop->sta.addr, ETH_ALEN);
				spin_unlock_bh(&mpath->state_lock);
				mesh_proxy_send_puc(sdata,nexthop,proxy_mac, seq_num);
				rcu_read_unlock();
			}
		}
		break;

	case PUC:
#ifdef DEBUG
		printk(KERN_INFO "Received PUC\n");
#endif
		return;
	default:
		return;
	}

}


int mesh_portal_send_pann(struct ieee80211_sub_if_data *sdata,
			u8 *dst,u8 *portal,u8 flag,u8 hop_count,
			u8 ttl,u8 seq_num)
{
	struct ieee80211_local *local = sdata->local;
	struct sk_buff *skb = dev_alloc_skb(local->hw.extra_tx_headroom + 400);
	struct ieee80211_mgmt *mgmt;
	u8 *pos;
	int ie_len;
#ifdef DEBUG
	printk(KERN_INFO"mesh_portal_send_pann: ENTERED\n");
#endif
	if (sdata->vif.type !=  NL80211_IFTYPE_MESH_POINT)
		return -EINVAL;
	if (!skb)
		return -1;
	skb_reserve(skb, local->hw.extra_tx_headroom);
	// 25 is the size of the common mgmt part (24) plus the size of the
	// common action part (1)
	
	mgmt = (struct ieee80211_mgmt *)
		skb_put(skb, 25 + sizeof(mgmt->u.action.u.mesh_action));
	memset(mgmt, 0, 25 + sizeof(mgmt->u.action.u.mesh_action));
	mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT |
					  IEEE80211_STYPE_ACTION);

	memcpy(mgmt->da, dst, ETH_ALEN);
	memcpy(mgmt->sa, sdata->dev->dev_addr, ETH_ALEN);
	//* BSSID is left zeroed, wildcard value //
	mgmt->u.action.category = MESH_PORTAL_CATEGORY;
	mgmt->u.action.u.mesh_action.action_code = PANN;

	ie_len=10;
	pos = skb_put(skb, 2 + ie_len);
	*pos++ = WLAN_EID_PANN;
	*pos++ = ie_len;
	*pos++ = flag;
	*pos++ = hop_count;
	*pos++ = ttl;
	memcpy(pos, portal, ETH_ALEN);
	pos += ETH_ALEN;
	*pos++ = seq_num;//cpu_to_le32(ifmsh->pann_seq_num++)

#ifdef DEBUG
	printk(KERN_INFO"mesh_portal_send_pann: Calling ieee80211_tx_skb\n");
#endif
	ieee80211_tx_skb(sdata, skb);

#ifdef DEBUG	
	printk(KERN_INFO"mesh_portal_send_pann: EXITING\n");
#endif
	return 0;
}






void mesh_rx_portal_frame(struct ieee80211_sub_if_data *sdata, struct ieee80211_mgmt *mgmt,size_t len)
{
	struct ieee802_11_elems elems;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	size_t baselen;
	u8 *proxy_ie;
	u8 flag, ttl;
	u8 portal[ETH_ALEN];
	u8 seq_num;
	u8 hop_count;

	/* need action_code */
	if (len < IEEE80211_MIN_ACTION_SIZE + 1)
		return;

	baselen = (u8 *) mgmt->u.action.u.mesh_action.variable - (u8 *) mgmt;
	ieee802_11_parse_elems(mgmt->u.action.u.mesh_action.variable,
			len - baselen, &elems);

	switch (mgmt->u.action.u.mesh_action.action_code) {
	case PANN:
#ifdef DEBUG
		printk(KERN_INFO "Received PANN\n");
#endif
		if (!elems.pann )//|| elems.pann_len != 13) 
			{
#ifdef DEBUG
			printk(KERN_INFO"Returning due to !elems.pann, pan_len=%d\n",elems.pann_len );
#endif
			return;
			}
		/* extract info */
		proxy_ie=elems.pann;
		flag=* (proxy_ie );
		hop_count=* (proxy_ie + 1);
		ttl=* (proxy_ie + 2);
		memcpy(portal,proxy_ie+3, ETH_ALEN);
		seq_num=* (proxy_ie + 9);
		/* forward if not root */
		if (memcmp(ifmsh->root, sdata->dev->dev_addr, ETH_ALEN) && ttl>1) {
#ifdef DEBUG
			printk(KERN_INFO"Forwarding the received PANN to root\n");
#endif
			mesh_portal_send_pann(sdata, ifmsh->root_nexthop, portal, 
						flag, hop_count+1,ttl-1, seq_num);
		}
		else if (mesh_path_lookup(portal,sdata)){
		/* add portal to the portal list */
		if (mesh_portal_add(sdata,portal))
			{
#ifdef DEBUG
			printk(KERN_INFO"mesh_rx_portal_frame:: mesh_portal_add failed\n");
#endif
			}
		}
		break;
	}

}

static int mesh_portal_add(struct ieee80211_sub_if_data *sdata, u8 *portal)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct mesh_portal_list *portal_node;
	//bool create=FALSE;


	spin_lock(&ifmsh->portal_list_lock);
#ifdef DEBUG
	printk(KERN_INFO"Traversing the portal_list\n");
#endif
	list_for_each_entry (portal_node, &ifmsh->portal_list.list, list ) {
#ifdef DEBUG
		printk(KERN_INFO"   portal= %2X:%2X:%2X:%2X:%2X:%2X\n", portal_node->portal[0], portal_node->portal[1],
			 portal_node->portal[2], portal_node->portal[3], portal_node->portal[4], portal_node->portal[5]);
#endif
		if (memcmp(portal,portal_node->portal, ETH_ALEN)==0){
			portal_node->last_updated=jiffies;
			spin_unlock(&ifmsh->portal_list_lock);
			return 0;
		}
	}

#ifdef DEBUG
	printk(KERN_INFO"Creating fresh portal entry in the portal list\n");
#endif
	portal_node = kmalloc(sizeof(struct mesh_portal_list), GFP_KERNEL);
	if (!portal_node) {
#ifdef DEBUG
		printk(KERN_DEBUG "Mesh HWMP: could not allocate PREQ node\n");
#endif
		return -ENOBUFS;
	}
	memcpy(portal_node->portal, portal, ETH_ALEN);
	portal_node->last_updated=jiffies;
	list_add_tail(&portal_node->list, &ifmsh->portal_list.list);
	++ifmsh->portal_list_len;
	spin_unlock(&ifmsh->portal_list_lock);
	return 0;
}


void mesh_portal_send(struct ieee80211_sub_if_data *sdata,struct sk_buff *skb)
{	
	struct sk_buff *skb2;
	struct ieee80211_local *local=sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct mesh_portal_list *portal_node;
	struct ieee80211_hdr *hdr;
	struct ieee80211s_hdr *mesh_hdr;
	unsigned int hdrlen;
	struct mesh_path *mpath;
#ifndef BS_DEBUG
	struct ieee80211_tx_info *info;
#endif
	//u8 add_bdcst[ETH_ALEN]={255,255,255,255,255,255},add_zero[ETH_ALEN]={0,0,0,0,0,0} ;

#ifdef DEBUG
	printk(KERN_INFO"\n mesh_portal_send: ENTERED\n");
#endif
	if (ifmsh->portal_list_len==0) {
#ifdef DEBUG
		printk(KERN_INFO"mesh_portal_send: returning due to empty portal_list\n");
#endif
		return;
	}
#ifdef DEBUG
	printk(KERN_INFO"mesh_portal_send: Now will send the packet to all portals\n");
#endif

	
	list_for_each_entry (portal_node, &ifmsh->portal_list.list, list ) {

#ifdef DEBUG
		printk(KERN_INFO"mesh_portal_send: portal=%X:%X:%X:%X:%X:%X\n",portal_node->portal[0],
			portal_node->portal[1],portal_node->portal[2],portal_node->portal[3],portal_node->portal[4],portal_node->portal[5]);		
#endif
		hdr = (struct ieee80211_hdr *) skb->data;
		if (memcmp(portal_node->portal, hdr->addr4, ETH_ALEN)==0)
		{
#ifdef DEBUG
		printk("decided not to send it to the above portal\n");
#endif
		continue;
		}
			
		skb2 = skb_copy(skb, GFP_ATOMIC);
		if (!skb2) 
			continue;
			
		hdr = (struct ieee80211_hdr *) skb2->data;
		hdrlen = ieee80211_hdrlen(hdr->frame_control);
		mesh_hdr = (struct ieee80211s_hdr *) (skb2->data + hdrlen);
	   
		/* make the header for forwording and then send directly through master_xmit */	
		
		rcu_read_lock();
		mpath=mesh_path_lookup(portal_node->portal, sdata);
		if (!mpath || memcmp(portal_node->portal, mpath->dst, ETH_ALEN)) {
			dev_kfree_skb(skb2);
			continue;
		}
		memcpy(hdr->addr3,portal_node->portal, ETH_ALEN );
		memcpy(hdr->addr1,mpath->next_hop->sta.addr, ETH_ALEN );
		rcu_read_unlock();

#ifdef BS_DEBUG
		skb2->dev = sdata->dev;
		skb2->iif = sdata->dev->ifindex;

		dev_queue_xmit(skb2);
#else
		info = IEEE80211_SKB_CB(skb2);
		memset(info, 0, sizeof(*info));
		info->flags |= IEEE80211_TX_INTFL_NEED_TXPROCESSING;
		info->control.vif = &sdata->vif;
		skb_set_queue_mapping(skb,
			ieee80211_select_queue(sdata, skb2));
		ieee80211_set_qos_hdr(local, skb);

		IEEE80211_IFSTA_MESH_CTR_INC(&sdata->u.mesh, fwded_frames);
		ieee80211_add_pending_skb(local, skb2);
#endif
	}
	
	dev_kfree_skb(skb);
}






