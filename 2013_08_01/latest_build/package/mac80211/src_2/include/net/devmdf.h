#ifndef MDF_H
#define MDF_H

#define CARDMODEMASK 0xFF
#define MAXMICROHARDTYPES 3
#define AR5K_MICROHARD49G 0x2004
#define AR5K_MICROHARD24G 0x3011
#define AR5K_MICROHARD23G 0x3012
#define AR5K_ATHROS2G 0x3002
#define AR5K_ATHROS5G 0x3005

#define CARDTYPE49G 4
#define CARDTYPE24G 0x11
#define CARDTYPE23G 0x12
#define CARDTYPEA2G 2
#define CARDTYPEA5G 5

struct wifpara
{
  int lfreq;	    //  Khz
  int rfreq;	    //  Khz  
  int bandwidth;	//  Khz
  int overlap;		//  Khz
  int antgai;
  int maxeirp;
  int regflag;      // regulatory control flag fo this band
  int mdfmode;	    // mdf module next working mode  
  unsigned char curntmd;
  unsigned char cardmod;	 	
  unsigned char pending;
  unsigned char boot;
};

struct wifpara* ieee80211_mdf_read(void);
int mdf_mutex_lock(void);
void mdf_mutex_unlock(void);

#define MESHMACSIZE 4

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

void AddMeshMacBlack(char *Mac);
int LookupMeshMacBlack(char *Mac);
void SetMcastRate(int rate);
int GetMcastRate(void);
void SetMicroHardMode(unsigned int mode);

#endif

