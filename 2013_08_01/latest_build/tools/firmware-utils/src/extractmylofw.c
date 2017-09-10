/*
 *  Copyright (C) 2009 Bob Sheng <bsheng@microhardcorp.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>     /* for unlink() */
#include <libgen.h>
#include <getopt.h>     /* for getopt() */
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <endian.h>     /* for __BYTE_ORDER */

#if defined(__CYGWIN__)
#  include <byteswap.h>
#endif

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#  define HOST_TO_LE16(x)	(x)
#  define HOST_TO_LE32(x)	(x)
#  define LE16_TO_HOST(x)	(x)
#  define LE32_TO_HOST(x)	(x)
#else

static inline unsigned short bswap_16(unsigned short x) {
  return (x>>8) | (x<<8);
}

static inline unsigned int bswap_32(unsigned int x) {
  return (bswap_16(x&0xffff)<<16) | (bswap_16(x>>16));
}

#  define HOST_TO_LE16(x)	bswap_16(x)
#  define HOST_TO_LE32(x)	bswap_32(x)
#  define LE16_TO_HOST(x)	bswap_16(x)
#  define LE32_TO_HOST(x)	bswap_32(x)
#endif

#include "myloader.h"

#define MAX_FW_BLOCKS  	8
#define MAX_ARG_COUNT   32
#define MAX_ARG_LEN     1024
#define FILE_BUF_LEN    (16*1024)
#define PART_NAME_LEN	32

#define PARTITION_FILE  "partition.lst"
#define BLOCK_DESC_EXT  ".desc"
#define BLOCK_BIN_EXT  ".bin"

struct fw_block {
	uint32_t	addr;
	uint32_t	blocklen; /* length of the block */
	uint32_t	flags;

	char		name[PART_NAME_LEN];  /* name of the file */
	uint32_t	size;  	/* length of the file */
	uint32_t	crc;    /* crc value of the file */
};

struct fw_part {
	struct mylo_partition	mylo;
	char			name[PART_NAME_LEN];
};

#define BLOCK_FLAG_HAVEHDR    0x0001

struct cpx_board {
	char		*model; /* model number*/
	char		*name;	/* model name*/
	char		*desc;  /* description */
	uint16_t        vid;    /* vendor id */
	uint16_t        did;    /* device id */
	uint16_t        svid;   /* sub vendor id */
	uint16_t        sdid;   /* sub device id */
	uint32_t        flash_size;     /* size of flash */
	uint32_t	part_offset;	/* offset of the partition_table */
	uint32_t	part_size;	/* size of the partition_table */
};

struct vendor {
	char		*vendor_name; 
	uint16_t        vid;    /* vendor id */
};


#define BOARD(_vid, _did, _svid, _sdid, _flash, _mod, _name, _desc, _po, _ps) {		\
	.model = _mod, .name = _name, .desc = _desc,   			\
	.vid = _vid, .did = _did, .svid = _svid, .sdid = _sdid,         \
	.flash_size = (_flash << 20),					\
	.part_offset = _po, .part_size = _ps }

#define CPX_BOARD(_did, _flash, _mod, _name, _desc, _po, _ps) \
	BOARD(VENID_COMPEX, _did, VENID_COMPEX, _did, _flash, _mod, _name, _desc, _po, _ps)

#define CPX_BOARD_ADM(_did, _flash, _mod, _name, _desc) \
	CPX_BOARD(_did, _flash, _mod, _name, _desc, 0x10000, 0x10000)

#define CPX_BOARD_AR71XX(_did, _flash, _mod, _name, _desc) \
	CPX_BOARD(_did, _flash, _mod, _name, _desc, 0x20000, 0x8000)

#define CPX_BOARD_AR23XX(_did, _flash, _mod, _name, _desc) \
	CPX_BOARD(_did, _flash, _mod, _name, _desc, 0x10000, 0x10000)
    
#define NANOCPU_BOARD_SAM9G20(_vid, _did, _mod, _name, _desc) \
	BOARD(_vid, _did, _vid, _did, 16, _mod, _name, _desc, 0x0, 0x0)

#define ALIGN(x,y)	((x)+((y)-1)) & ~((y)-1)

char	*progname;
char	*ofname = NULL;

uint32_t flash_size = 0;
int	fw_num_partitions = 0;
int	fw_num_blocks = 0;
int	verblevel = 0;
int valid_fw_only = 0;   /* if set validate the firmware only without extracting */
char    output_path[PART_NAME_LEN] = ".";

struct mylo_fw_header fw_header;
struct fw_part fw_parts[MYLO_MAX_PARTITIONS];
struct fw_block fw_blocks[MAX_FW_BLOCKS];
struct cpx_board *board;

struct cpx_board boards[] = {
	CPX_BOARD_ADM(DEVID_COMPEX_NP18A, 4,
		"NP18A", "Compex NetPassage 18A",
		"Dualband Wireless A+G Internet Gateway"),
	CPX_BOARD_ADM(DEVID_COMPEX_NP26G8M, 2,
		"NP26G8M", "Compex NetPassage 26G (8M)",
		"Wireless-G Broadband Multimedia Gateway"),
	CPX_BOARD_ADM(DEVID_COMPEX_NP26G16M, 4,
		"NP26G16M", "Compex NetPassage 26G (16M)",
		"Wireless-G Broadband Multimedia Gateway"),
	CPX_BOARD_ADM(DEVID_COMPEX_NP27G, 4,
		"NP27G", "Compex NetPassage 27G",
		"Wireless-G 54Mbps eXtended Range Router"),
	CPX_BOARD_ADM(DEVID_COMPEX_NP28G, 4,
		"NP28G", "Compex NetPassage 28G",
		"Wireless 108Mbps Super-G XR Multimedia Router with 4 USB Ports"),
	CPX_BOARD_ADM(DEVID_COMPEX_NP28GHS, 4,
		"NP28GHS", "Compex NetPassage 28G (HotSpot)",
		"HotSpot Solution"),
	CPX_BOARD_ADM(DEVID_COMPEX_WP18, 4,
		"WP18", "Compex NetPassage WP18",
		"Wireless-G 54Mbps A+G Dualband Access Point"),
	CPX_BOARD_ADM(DEVID_COMPEX_WP54G, 4,
		"WP54G", "Compex WP54G",
		"Wireless-G 54Mbps XR Access Point"),
	CPX_BOARD_ADM(DEVID_COMPEX_WP54Gv1C, 2,
		"WP54Gv1C", "Compex WP54G rev.1C",
		"Wireless-G 54Mbps XR Access Point"),
	CPX_BOARD_ADM(DEVID_COMPEX_WP54AG, 4,
		"WP54AG", "Compex WP54AG",
		"Wireless-AG 54Mbps XR Access Point"),
	CPX_BOARD_ADM(DEVID_COMPEX_WPP54G, 4,
		"WPP54G", "Compex WPP54G",
		"Outdoor Access Point"),
	CPX_BOARD_ADM(DEVID_COMPEX_WPP54AG, 4,
		"WPP54AG", "Compex WPP54AG",
		"Outdoor Access Point"),

	CPX_BOARD_AR71XX(DEVID_COMPEX_WP543, 2,
		"WP543", "Compex WP543",
		"BareBoard"),

	CPX_BOARD_AR23XX(DEVID_COMPEX_NP25G, 4,
		"NP25G", "Compex NetPassage 25G",
		"Wireless 54Mbps XR Router"),
	CPX_BOARD_AR23XX(DEVID_COMPEX_WPE53G, 4,
		"WPE53G", "Compex NetPassage 25G",
		"Wireless 54Mbps XR Access Point"),

	/*productname,vendor,encryptionlevel,linkrate,newlinkrate,oemid,powerlevel,productid,productmode*/
	//   {"nanoIPx20","Microhard",0,0,14,0,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_MICROHARD, DEVID_MICROHARD_NANOIPX20,
		"nanoIPx20", "Microhard NanoIPx 920",
		"Small footprint 900M Wireless ethernet bridge"),

	//	{"nanoIPx21","Microhard",0,0,256,0,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_MICROHARD, DEVID_MICROHARD_NANOIPX21,
		"nanoIPx21", "Microhard NanoIPx 921",
		"Small footprint 900M Wireless ethernet bridge"),

	//   {"nanoIP920E","ENCOM",0,0,14,1,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_ENCOM, DEVID_ENCOM_NANOIP920E,
		"nanoIP920E", "Encom OEM NanoIP 920",
		"Small footprint 900M Wireless ethernet bridge"),

	//	{"nanoIP921E","ENCOM",0,0,256,1,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_ENCOM, DEVID_ENCOM_NANOIP921E,
		"nanoIP921E", "Encom OEM NanoIOP 921",
		"Small footprint 900M Wireless ethernet bridge"),

	//   {"nanoIPx20AES","Microhard",63,0,14,0,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_MICROHARD, DEVID_MICROHARD_NANOIPX20AES,
		"nanoIPx20AES", "Microhard NanoIPx 920 AES version",
		"Small footprint 900M Wireless ethernet bridge"),

	//	{"nanoIPx21AES","Microhard",63,0,256,0,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_MICROHARD, DEVID_MICROHARD_NANOIPX21AES,
		"nanoIPx21AES", "Microhard NanoIPx 921 AES version",
		"Small footprint 900M Wireless ethernet bridge"),

	//   {"nanoIPx20EXP","Microhard",0,0,14,0,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_MICROHARD, DEVID_MICROHARD_NANOIPX20EXP,
		"nanoIPx20EXP", "Microhard NanoIPx 920 EXPORT version",
		"Small footprint 900M Wireless ethernet bridge"),

	//	{"nanoIPx21EXP","Microhard",0,0,256,0,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_MICROHARD, DEVID_MICROHARD_NANOIPX21EXP,
		"nanoIPx21EXP", "Microhard NanoIPx 921 EXPORT version",
		"Small footprint 900M Wireless ethernet bridge"),

	//   {"nanoIP920EPOCH","EPOCH",0,0,14,0,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_EPOCH, DEVID_EPOCH_NANOIP920,
		"nanoIP920EPOCH", "EPOCH OEM version IP920",
		"Small footprint 900M Wireless ethernet bridge"),

	//	{"nanoIP921EPOCH","EPOCH",0,0,256,0,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_EPOCH, DEVID_EPOCH_NANOIP921,
		"nanoIP921EPOCH", "EPOCH OEM version IP921",
		"Small footprint 900M Wireless ethernet bridge"),

	//   {"nanoIP2420EPOCH","Microhard",0,0,32,0,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_EPOCH, DEVID_EPOCH_NANOIP2420,
		"nanoIP2420EPOCH", "EPOCH OEM version IP2420",
		"Small footprint 900M Wireless ethernet bridge"),

	//	{"nanoIP2421EPOCH","Microhard",0,0,256,0,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_EPOCH, DEVID_EPOCH_NANOIP2421,
		"nanoIP2421EPOCH", "EPOCH OEM version IP2421",
		"Small footprint 900M Wireless ethernet bridge"),

	//   {"nanoIPx20LC","Microhard",0,0,4,0,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_MICROHARD, DEVID_MICROHARD_NANOIPX20LC,
		"nanoIPx20LC", "Microhard NanoIPx 920 Low Cost version",
		"Small footprint 900M Wireless ethernet bridge"),

	//	{"nanoIPx21PROSOFT","Prosoft",31,0,256,4,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_PROSOFT, DEVID_PROSOFT_NANOIPX21,
		"nanoIPx21PROSOFT", "Prosoft OEM version NanoIPx 921",
		"Small footprint 900M Wireless ethernet bridge"),

	//	{"nanoIPx21NORALTA","Microhard",63,0,256,5,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_NORALTA, DEVID_NORALTA_NANOIPX21,
		"nanoIPx21NORALTA", "Noralta OEM version NanoIPx 921",
		"Small footprint 900M Wireless ethernet bridge"),

	//   {"nanoIPx20IOSELECT","Microhard",63,0,14,6,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_IOSELECT, DEVID_IOSELECT_NANOIPX20,
		"nanoIPx20IOSELECT", "IOSELECT OEM version NanoIPx 920",
		"Small footprint 900M Wireless ethernet bridge"),

	//	{"nanoIPx21IOSELECT","Microhard",63,0,256,6,0,0,0}
	NANOCPU_BOARD_SAM9G20(VENID_IOSELECT, DEVID_IOSELECT_NANOIPX21,
		"nanoIPx21IOSELECT", "IOSELECT OEM version NanoIPx 921",
		"Small footprint 900M Wireless ethernet bridge"),

	//   {"nanoIPx20CAL","Calamp",63,0,14,7,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_CALAMP, DEVID_CALAMP_NANOIPX20,
		"nanoIPx20CAL", "CALAMP OEM version NanoIPx 920",
		"Small footprint 900M Wireless ethernet bridge"),

	//	{"nanoIPx21CAL","Calamp",63,0,256,7,0,0,0}
	NANOCPU_BOARD_SAM9G20(VENID_CALAMP, DEVID_CALAMP_NANOIPX21,
		"nanoIPx21CAL", "CALAMP OEM version NanoIPx 921",
		"Small footprint 900M Wireless ethernet bridge"),

	//	{"nanoIPx21ELAN","Elan",0,0,256,8,0,0,0},
	NANOCPU_BOARD_SAM9G20(VENID_ELAN, DEVID_ELAN_NANOIPX21,
		"nanoIPx21ELAN", "ELAN OEM version NanoIPx 921",
		"Small footprint 900M Wireless ethernet bridge"),

	// tridition 1-miniPCI VIP board
	BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP, 16,
			VENID_MICROHARD, DEVID_MICROHARD_VIP,
			"VIP421", "Microhard ixp4xx based 1-miniPCI board",
			"Microhard ixp4xx based 1-miniPCI board wifi product",
			0x0, 0x0),

    // export 1-miniPCI VIP board
	BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIPEXP, 16,
			VENID_MICROHARD, DEVID_MICROHARD_VIPEXP,
			"VIP421EXP", "Microhard ixp4xx based 1-miniPCI board",
			"Microhard ixp4xx based 1-miniPCI board wifi product",
			0x0, 0x0),

    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIPANT, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIPANT,
            "VIP421ANT", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIPANTEXP, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIPANTEXP,
            "VIP421ANTEXP", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIPANT18dBiEXP, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIPANT18dBiEXP,
            "VIP421ANT18dBiEXP", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),

    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIPEXPBR, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIPEXPBR,
            "VIP421EXPBR", "Microhard ixp4xx export 4-miniPCI  to Brazil board",
            "Microhard ixp4xx export to brazil 4-miniPCI board wifi product",
            0x0, 0x0),

    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP900, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP900,
            "VIP421900", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),

    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP900EXP, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP900EXP,
            "VIP421900EXP", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),

    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP900ENCOM, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP900ENCOM,
            "VIP421900ENCOM", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),
    // export 4.9GHz 4-miniPCI  VIP board 
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIPEXP49G, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIPEXP49G,
            "VIP421EXP49G", "Microhard ixp4xx export 4-miniPCI 4.9G board",
            "Microhard ixp4xx export 4.9G 4-miniPCI board wifi product",
            0x0, 0x0),

    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP4900, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP4900,
            "VIP4214900", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP4900EXP, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP4900EXP,
            "VIP4214900EXP", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),

    // ENCOM OEM  VIP board 
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIPENCOM, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIPENCOM,
            "VIP421ENCOM", "Microhard ixp4xx oem board",
            "Microhard ixp4xx oem board wifi product",
            0x0, 0x0),

    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP2400X, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP2400X,
            "VIP4212400X", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP2400XEXP, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP2400XEXP,
            "VIP4212400XEXP", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP5800X, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP5800X,
            "VIP4215800X", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP5800XEXP, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP5800XEXP,
            "VIP4215800XEXP", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP4900X, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP4900X,
            "VIP4214900X", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP4900XEXP, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP4900XEXP,
            "VIP4214900XEXP", "Microhard ixp4xx board",
            "Microhard ixp4xx board wifi product",
            0x0, 0x0),

	// new AR71xx base mPCI board
	BOARD(VENID_MICROHARD, DEVID_MICROHARD_MPCI71, 16,
			VENID_MICROHARD, DEVID_MICROHARD_MPCI71,
			"MPCI71", "Microhard ar71xx based 1 micro miniPCI board",
			"Microhard ar71xx based 1 micro miniPCI board",
			0x0, 0x0),

    // new AR71xx base mPCI board
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_IPnVT, 16,
            VENID_MICROHARD, DEVID_MICROHARD_IPnVT,
            "IPnVT", "Microhard ar71xx based 1 micro miniPCI 2300M board",
            "Microhard ar71xx based 1 micro miniPCI 2300M board",
            0x0, 0x0),

    // new AR71xx base mPCI board
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_IPnDDL, 16,
            VENID_MICROHARD, DEVID_MICROHARD_IPnDDL,
            "IPnDDL", "Microhard ar71xx based 1 micro miniPCI 2300M board",
            "Microhard ar71xx based 1 micro miniPCI 2300M board",
            0x0, 0x0),

    // new AR71xx base IPnVTn3G board
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_IPnVTn3G, 16,
            VENID_MICROHARD, DEVID_MICROHARD_IPnVTn3G,
            "IPnVTn3G", "Microhard ar71xx based 1 micro miniPCI +3G board",
            "Microhard ar71xx based 1 micro miniPCI +3G board",
            0x0, 0x0),

    // new AR71xx base IPnVTEXP board
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_IPnVTEXP, 16,
            VENID_MICROHARD, DEVID_MICROHARD_IPnVTEXP,
            "IPnVTEXP", "Microhard ar71xx based 1 micro miniPCI export board",
            "Microhard ar71xx based 1 micro miniPCI export board",
            0x0, 0x0),

    // new AR71xx base IPnVTn3GEXP board
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_IPnVTn3GEXP, 16,
            VENID_MICROHARD, DEVID_MICROHARD_IPnVTn3GEXP,
            "IPnVTn3GEXP", "Microhard ar71xx based 1 micro miniPCI +3G export board",
            "Microhard ar71xx based 1 micro miniPCI +3G export board",
            0x0, 0x0),

    // AR71xx base VIP4G board
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP4G, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP4G,
            "VIP4G", "Microhard ar71xx based wifi+3G+GPS board",
            "Microhard ar71xx based wifi+4G+GPS board",
            0x0, 0x0),

    // AR71xx base VIP4GEXP board
    BOARD(VENID_MICROHARD, DEVID_MICROHARD_VIP4GEXP, 16,
            VENID_MICROHARD, DEVID_MICROHARD_VIP4GEXP,
            "VIP4GEXP", "Microhard ar71xx based WIFI+3G+GPS export board",
            "Microhard ar71xx based WIFI+4G+GPS export board",
            0x0, 0x0),

	{.model = NULL}
};

struct vendor vendors[] = {

    {"COMPEX",      VENID_COMPEX},
	{"MICROHARD",   VENID_MICROHARD},
	{"PROSOFT",		VENID_PROSOFT},
	{"EPOCH",		VENID_EPOCH},
	{"ENCOM",		VENID_ENCOM},
	{"IOSELECT",	VENID_IOSELECT},
	{"NORALTA",		VENID_NORALTA},
	{"CALAMP",		VENID_CALAMP},
	{"ELAN",		VENID_ELAN},
	{.vendor_name = NULL}
};


void
errmsgv(int syserr, const char *fmt, va_list arg_ptr)
{
	int save = errno;

	fflush(0);
	fprintf(stderr, "[%s] Error: ", progname);
	vfprintf(stderr, fmt, arg_ptr);
	if (syserr != 0) {
		fprintf(stderr, ": %s", strerror(save));
	}
	fprintf(stderr, "\n");
}

void
errmsg(int syserr, const char *fmt, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	errmsgv(syserr, fmt, arg_ptr);
	va_end(arg_ptr);
}

void
dbgmsg(int level, const char *fmt, ...)
{
	va_list arg_ptr;
	if (verblevel >= level) {
		fflush(0);
		va_start(arg_ptr, fmt);
		vfprintf(stderr, fmt, arg_ptr);
		fprintf(stderr, "\n");
		va_end(arg_ptr);
	}
}


void
usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;
	struct cpx_board *board;

	fprintf(stream, "Usage: %s [OPTION...] <file>\n", progname);
	fprintf(stream,
"\n"
"  <file>          binary firmware file\n"
"\n"
"Options:\n");

	fprintf(stream,
"  -V              verify firmware file\n"
"  -d              specify output directory\n"
"  -v              increase verblevel\n"
"  -h              show this screen\n"
	);

	exit(status);
}

/*
 * Code to compute the CRC-32 table. Borrowed from
 * gzip-1.0.3/makecrc.c.
 */

static uint32_t crc_32_tab[256];

void
init_crc_table(void)
{
	/* Not copyrighted 1990 Mark Adler	*/

	uint32_t c;      /* crc shift register */
	uint32_t e;      /* polynomial exclusive-or pattern */
	int i;           /* counter for all possible eight bit values */
	int k;           /* byte being shifted into crc apparatus */

	/* terms of polynomial defining this crc (except x^32): */
	static const int p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};

	/* Make exclusive-or pattern from polynomial */
	e = 0;
	for (i = 0; i < sizeof(p)/sizeof(int); i++)
		e |= 1L << (31 - p[i]);

	crc_32_tab[0] = 0;

	for (i = 1; i < 256; i++) {
		c = 0;
		for (k = i | 256; k != 1; k >>= 1) {
			c = c & 1 ? (c >> 1) ^ e : c >> 1;
			if (k & 1)
				c ^= e;
		}
		crc_32_tab[i] = c;
	}
}


void
update_crc(uint8_t *p, uint32_t len, uint32_t *crc)
{
	uint32_t t;

	t = *crc ^ 0xFFFFFFFFUL;
	while (len--) {
		t = crc_32_tab[(t ^ *p++) & 0xff] ^ (t >> 8);
	}
	*crc = t ^ 0xFFFFFFFFUL;
}


uint32_t
get_crc(uint8_t *p, uint32_t len)
{
	uint32_t crc;

	crc = 0;
	update_crc(p ,len , &crc);
	return crc;
}

struct cpx_board *
find_board_from_did(uint16_t did){
	struct cpx_board *board;
	struct cpx_board *tmp;

	board = NULL;
	for (tmp = boards; tmp->model != NULL; tmp++){
		if (tmp->did == did) {
			board = tmp;
			break;
		}
	};

	return board;
}

struct vendor *
find_vendor(uint16_t vid){
	struct vendor *result;
	struct vendor *tmp;

	result = NULL;
	for (tmp = vendors; tmp->vendor_name != NULL; tmp++){
		if (tmp->vid == vid) {
			result = tmp;
			break;
		}
	};

	return result;
}


int
get_file_crc(struct fw_block *ff)
{
	FILE *f;
	uint8_t buf[FILE_BUF_LEN];
	uint32_t readlen = sizeof(buf);
	int res = -1;
	size_t len;

	if ((ff->flags & BLOCK_FLAG_HAVEHDR) == 0) {
		res = 0;
		goto out;
	}

	errno = 0;
	f = fopen(ff->name,"r");
	if (errno) {
		errmsg(1,"unable to open file %s", ff->name);
		goto out;
	}

	ff->crc = 0;
	len = ff->size;
	while (len > 0) {
		if (len < readlen)
			readlen = len;

		errno = 0;
		fread(buf, readlen, 1, f);
		if (errno) {
			errmsg(1,"unable to read from file %s",	ff->name);
			goto out_close;
		}

		update_crc(buf, readlen, &ff->crc);
		len -= readlen;
	}

	res = 0;

out_close:
	fclose(f);
out:
	return res;
}

int
required_arg(char c, char *arg)
{
	if ((optarg != NULL) && (*arg == '-')){
		errmsg(0,"option %c requires an argument\n", c);
		return -1;
	}

	return 0;
}


int
is_empty_arg(char *arg)
{
	int ret = 1;
	if (arg != NULL) {
		if (*arg) ret = 0;
	};
	return ret;
}

int
parse_opt_dir(char ch, char *arg)
{
	char buf[MAX_ARG_LEN];
	char *argv[MAX_ARG_COUNT];
	int argc;
	struct fw_block *b;
	char *p;


	if (required_arg(ch, arg)) {
		goto err_out;
	}

    dbgmsg(1,"output directory is %s", arg);

    strcpy(output_path, arg);

success:

	return 0;

err_out:
	return -1;
}


/*
 * routines to read data from the input file
 */
int
read_in_data(FILE *infile, uint8_t *data, size_t len, uint32_t *crc)
{
	errno = 0;

	fread(data, len, 1, infile);
	if (errno) {
		errmsg(1,"unable to read file");
		return -1;
	}

	if (crc) {
		update_crc(data, len, crc);
	}

	return 0;
}

int
read_in_desc(FILE *infile, struct mylo_fw_blockdesc *desc, uint32_t *crc)
{
	return read_in_data(infile, (uint8_t *)desc,
		sizeof(*desc), crc);
}

int
write_out_file(FILE *infile, struct fw_block *block, uint32_t *crc)
{
	char buff[FILE_BUF_LEN];
	size_t  buflen = sizeof(buff);
    struct mylo_partition_header ph;

	FILE *f;
    char ofname[PART_NAME_LEN];
	size_t len;

	errno = 0;

	if (block->name == NULL) {
		return 0;
	}

    /* try to read the partition head from this block */
    memset(&ph, 0, sizeof(ph));

    if (read_in_data(infile, (uint8_t *)&ph, sizeof(ph), crc) != 0)
        return -1;

    if ( (block->size - sizeof(struct mylo_partition_header)) == LE32_TO_HOST(ph.len) )
    {
        block->flags |= BLOCK_FLAG_HAVEHDR;
    }

    /* output the block description file */
    strcpy(ofname, output_path);
    if(output_path[strlen(output_path)-1] != '/')
    {
        strcat(ofname, "/");
    }

    strcat(ofname, block->name);
    strcat(ofname, BLOCK_DESC_EXT);

    if (!valid_fw_only)
    {

        f = fopen(ofname, "w");

        if(f == NULL)
        {
            dbgmsg(1,"Could not create file %s.", ofname);
            return -1;
        }

        fprintf(f, "0x%x\n", block->flags);
        fprintf(f, "0x%x\n", block->addr);
        fprintf(f, "0x%x\n", block->blocklen);
        fprintf(f, "0x%x\n", block->size - sizeof(struct mylo_partition_header));

        fclose(f);

        /* output the block binary file */

        strcpy(ofname, output_path);
        if(output_path[strlen(output_path)-1] != '/')
        {
            strcat(ofname, "/");
        }

        strcat(ofname, block->name);
        strcat(ofname, BLOCK_BIN_EXT);

        f = fopen(ofname, "w");

        if(f == NULL)
        {
            dbgmsg(1,"Could not create file %s.", ofname);
            return -1;
        }

        if ( !(block->flags & BLOCK_FLAG_HAVEHDR) ) {
            /* this block has no header, we have to output it as data */
            errno = 0;
            fwrite((uint8_t *)&ph, sizeof(struct mylo_partition_header), 1, f);
            if (errno) {
                errmsg(1,"unable to write block binary file");
                return -1;
            }
        }
    }

    len = block->size - sizeof(struct mylo_partition_header);

	while (len > 0) {
		if (len < buflen)
			buflen = len;

		/* read data from firmware file */
		if (read_in_data(infile, buff, buflen, crc) != 0)
			return -1;

        if (!valid_fw_only)
        {

		    errno = 0;
		    fwrite(buff, buflen, 1, f);
		    if (errno != 0) {
			    errmsg(1,"unable to read from file: %s",ofname);
			    return -1;
		    }
        }

		len -= buflen;
	}

    if (!valid_fw_only)
    {
	    fclose(f);
	    dbgmsg(1,"file %s written out", ofname);
    }
	return 0;
}

int
read_in_header(FILE *infile, uint32_t *crc)
{
	struct mylo_fw_header hdr;

	memset(&hdr, 0, sizeof(hdr));
    memset(&fw_header, 0, sizeof(fw_header));

	if (fseek(infile, 0, SEEK_SET) != 0) {
		errmsg(1,"fseek failed on input file");
		return -1;
	}

    read_in_data(infile, (uint8_t *)&hdr, sizeof(hdr), NULL);

	fw_header.magic = LE32_TO_HOST(hdr.magic);
    fw_header.crc = LE32_TO_HOST(hdr.crc);
    fw_header.vid = LE16_TO_HOST(hdr.vid);
    fw_header.did = LE16_TO_HOST(hdr.did);
    fw_header.svid = LE16_TO_HOST(hdr.svid);
    fw_header.sdid = LE16_TO_HOST(hdr.sdid);
	fw_header.rev = LE32_TO_HOST(hdr.rev);
	fw_header.fwhi = LE32_TO_HOST(hdr.fwhi);
	fw_header.fwlo = LE32_TO_HOST(hdr.fwlo);
	fw_header.flags = LE32_TO_HOST(hdr.flags);

    hdr.crc = 0;
    update_crc((uint8_t *)&hdr, sizeof(hdr), crc);

	return 0;
}

int
write_out_partitions(FILE *infile, uint32_t *crc)
{
	struct mylo_partition_table p;
	char part_names[MYLO_MAX_PARTITIONS][PART_NAME_LEN];
	int ret;
	int i;
    FILE *f;
    char ofname[PART_NAME_LEN];

	if (fw_num_partitions == 0)
		return 0;

	memset(&p, 0, sizeof(p));

	ret = read_in_data(infile, (uint8_t *)&p, sizeof(p), crc);
	if (ret)
		return ret;

	memset(part_names, 0, sizeof(part_names));
	ret = read_in_data(infile, (uint8_t *)part_names, sizeof(part_names),
				crc);

    p.magic = LE32_TO_HOST(p.magic);

	if (p.magic != MYLO_MAGIC_PARTITIONS)
    {
        return -1;
    }

    memset(&fw_parts, 0, sizeof(fw_parts));

	for (i = 0; i < MYLO_MAX_PARTITIONS; i++) {
		struct mylo_partition *mp;
		struct fw_part *fp;

		mp = &p.partitions[i];
		fp = &fw_parts[i];

        if(LE16_TO_HOST(mp->type) == PARTITION_TYPE_FREE)
            break;

		fp->mylo.flags = LE16_TO_HOST(mp->flags);
        fp->mylo.addr = LE32_TO_HOST(mp->addr);
        fp->mylo.size = LE32_TO_HOST(mp->size);
        fp->mylo.param = LE32_TO_HOST(mp->param);

		memcpy(fp->name, part_names[i], PART_NAME_LEN);
	}

    fw_num_partitions = i;

    if (valid_fw_only)
        return ret;

    strcpy(ofname, output_path);
    if(output_path[strlen(output_path)-1] != '/')
    {
        strcat(ofname, "/");
    }
    strcat(ofname, PARTITION_FILE);
    f = fopen(ofname, "w");

    if(f) {
        for(i =0; i < fw_num_partitions; i++)
        {
            fprintf(f, "%s\n", fw_parts[i].name);
            fprintf(f, "0x%x\n", fw_parts[i].mylo.flags);
            fprintf(f, "0x%x\n", fw_parts[i].mylo.addr);
            fprintf(f, "0x%x\n", fw_parts[i].mylo.size);
            fprintf(f, "0x%x\n", fw_parts[i].mylo.param);
        }

        fclose(f);
    }
    else {

        errmsg(1, "could not open \"%s\" for writing", PARTITION_FILE);
    }


	return ret;
}

int
read_in_blocks(FILE *infile, uint32_t *crc)
{
	struct mylo_fw_blockdesc desc;
	struct fw_block *b;
	uint32_t dlen;
	int i;

	/*
	 * read whether the first block is partition table
	 */

    if (read_in_desc(infile, &desc, crc) != 0)
        return -1;

    desc.type = LE32_TO_HOST(desc.type);
    desc.addr = LE32_TO_HOST(desc.addr);
    desc.dlen = LE32_TO_HOST(desc.dlen);
    desc.blen = LE32_TO_HOST(desc.blen);

    if ((desc.type == FW_DESC_TYPE_USED) &&
        (desc.addr == board->part_offset) &&
        (desc.blen == board->part_size) ) {
        fw_num_partitions = MYLO_MAX_PARTITIONS;
    }
    else
    {
        fw_num_partitions = 0;
    }

    /* read the blocks */
    if(fw_num_partitions > 0)
    {
        if (read_in_desc(infile, &desc, crc) != 0)
            return -1;
        desc.type = LE32_TO_HOST(desc.type);
        desc.addr = LE32_TO_HOST(desc.addr);
        desc.dlen = LE32_TO_HOST(desc.dlen);
        desc.blen = LE32_TO_HOST(desc.blen);

    }

    memset(&fw_blocks, 0, sizeof(fw_blocks));

    while(desc.type == FW_DESC_TYPE_USED)
    {
        sprintf(fw_blocks[fw_num_blocks].name, "block%0d", fw_num_blocks);
        fw_blocks[fw_num_blocks].size = desc.dlen;
        fw_blocks[fw_num_blocks].addr = desc.addr;
        fw_blocks[fw_num_blocks].blocklen = desc.blen;
        fw_num_blocks++;

        if (read_in_desc(infile, &desc, crc) != 0)
            return -1;
        desc.type = LE32_TO_HOST(desc.type);
        desc.addr = LE32_TO_HOST(desc.addr);
        desc.dlen = LE32_TO_HOST(desc.dlen);
        desc.blen = LE32_TO_HOST(desc.blen);

    }


	if (write_out_partitions(infile, crc) != 0)
		return -1;
	/*
	 * write out data for each blocks
	 */
	for (i = 0; i < fw_num_blocks; i++) {
		b = &fw_blocks[i];
		if (write_out_file(infile, b, crc) != 0)
		        return -1;
	}

	return 0;
}


int validate_firmware(struct mylo_fw_header *hdr, uint32_t crc)
{
    if( (hdr->magic == MYLO_MAGIC_FIRMWARE) &&
        (hdr->crc == crc) )
        return 1;
    else
        return 0;
}


/*
 * main
 */
int
main(int argc, char *argv[])
{
    FILE *stream = stdout;
	int optinvalid = 0;   /* flag for invalid option */
	int c;
	int res = EXIT_FAILURE;

	FILE  *fwfile;
	uint32_t crc;

	progname=basename(argv[0]);

	memset(&fw_header, 0, sizeof(fw_header));

	opterr = 0;  /* could not print standard getopt error messages */
	while ((c = getopt(argc, argv, "d:hvV")) != -1) {
		optinvalid = 0;
		switch (c) {
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'V':
			valid_fw_only++;
			break;
        case 'd':
            optinvalid = parse_opt_dir(c,optarg);
            break;
		case 'v':
			verblevel++;
			break;
		default:
			optinvalid = 1;
			break;
		}
		if (optinvalid != 0 ){
			errmsg(0, "invalid option: -%c", optopt);
			goto out;
		}
	}

	if (optind == argc) {
		errmsg(0, "no firmware file specified");
		goto out;
	}

	ofname = argv[optind++];

	if (optind < argc) {
		errmsg(0, "invalid option: %s", argv[optind]);
		goto out;
	}

	fwfile = fopen(ofname, "r");
	if (fwfile == NULL) {
		errmsg(1, "could not open firmware file \"%s\"", ofname);
		goto out;
	}

	crc = 0;
	init_crc_table();

	if (read_in_header(fwfile, &crc) != 0)
		goto out_flush;

    board = find_board_from_did(fw_header.did);

	if (read_in_blocks(fwfile, &crc) != 0)
		goto out_flush;

    if(!validate_firmware(&fw_header, crc))
    {
        fprintf(stream, "Firmware %s is Invalid\n", ofname);
        goto out_flush;
    }

    if(valid_fw_only)
    {
            struct vendor *tmp = find_vendor(fw_header.vid);
            fprintf(stream, "Firmware %s is valid\n", ofname);

            if(tmp)
            {
                fprintf(stream, "Vendor: %s \n", tmp->vendor_name);
            }
            board = find_board_from_did(fw_header.did);
            fprintf(stream, "Device Model: %s \n", board->model);
            fprintf(stream, "Device Name: %s \n", board->name);
            fprintf(stream, "             %s \n", board->desc);
            fprintf(stream, "Board Revision:  %d \n", fw_header.rev);
            fprintf(stream, "Firmware Version:  %0d.%02d \n", fw_header.fwhi, fw_header.fwlo);
    }
    else
    {
	    dbgmsg(1,"Firmware file %s extracted.", ofname);
    }

	res = EXIT_SUCCESS;

out_flush:
	fclose(fwfile);
out:

	return res;
}
