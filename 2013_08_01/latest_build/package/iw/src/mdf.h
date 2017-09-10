#ifndef MDF_H
#define MDF_H

// these defines must be same as ath5k driver
#define MAXMODE 6
#define AR5K_MICROHARD49G 0x2004
#define AR5K_MICROHARD24G 0x3011
#define AR5K_MICROHARD23G 0x3012
#define AR5K_ATHROS2G 0x3002
#define AR5K_ATHROS5G 0x3005
#define CARDMODEMASK 0xFF

#define CARDMODE(x) ((x)&(CARDMODEMASK))

enum ieee80211_channel_flags {
	IEEE80211_CHAN_DISABLED		= 1<<0,
	IEEE80211_CHAN_PASSIVE_SCAN	= 1<<1,
	IEEE80211_CHAN_NO_IBSS		= 1<<2,
	IEEE80211_CHAN_RADAR		= 1<<3,
	IEEE80211_CHAN_NO_HT40PLUS	= 1<<4,
	IEEE80211_CHAN_NO_HT40MINUS	= 1<<5,
};

#define CARDTYPE "/lib/mdf/cardtype"
#define MDFCFG "/lib/mdf/mdfcfg"
#define MDFDEV "/dev/mdfst0"
#define RESETMDF 'r'
#define CMD_RESETMDF _IO(RESETMDF, 0)
#define REMOVEMDF 'm'
#define CMD_REMOVEMDF _IO(REMOVEMDF, 0)

struct wifpara
{
	int lfreq;	// Khz
	int rfreq;	// Khz  
	int bandwidth;	// Khz
	int overlap;	// Khz
	int antgai;
	int maxeirp;
	int regflag;	// regulatory control flag fo this band
	int mdfmode;	// mdf module next working mode
	unsigned char curntmd;
    unsigned char cardmod;	
};

#endif
