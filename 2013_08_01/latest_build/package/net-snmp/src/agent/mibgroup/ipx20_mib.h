/*
 *  Template MIB group interface - VIP_MIB.h
 *
 */

/*
 * Don't include ourselves twice 
 */
#ifndef _MIBGROUP_VIP_MIB_H
#define _MIBGROUP_VIP_MIB_H

/*
 * We use 'header_generic' from the util_funcs module,
 *  so make sure this module is included in the agent.
 */
config_require(util_funcs)

/*
 * Declare our publically-visible functions.
 * Typically, these will include the initialization and shutdown functions,
 *  the main request callback routine and any writeable object methods.
 *
 * Function prototypes are provided for the callback routine ('FindVarMethod')
 *  and writeable object methods ('WriteMethod').
 */
extern void     init_VIP_MIB(void);
extern FindVarMethod var_VIP_MIB;

extern WriteMethod write_VIP_MIBint;
extern WriteMethod write_VIP_MIBstr;
extern WriteMethod write_VIP_MIB_Action_int;     
extern WriteMethod write_VIP_MIBtrap;
extern WriteMethod write_VIP_MIBtrap2;

/*
 * Magic number definitions.
 * These must be unique for each object implemented within a
 *  single mib module callback routine.
 *
 * Typically, these will be the last OID sub-component for
 *  each entry, or integers incrementing from 1.
 *  (which may well result in the same values anyway).
 *
 * Here, the second and third objects are form a 'sub-table' and
 *   the magic numbers are chosen to match these OID sub-components.
 * This is purely for programmer convenience.
 * All that really matters is that the numbers are unique.
 */
#define SIGNED_INT_MAX 65535
#define	VIP_MIBSTRING		    255
#define VIP_MIBINTEGER		    7121
#define	VIP_MIBOBJECTID         7122
#define VIP_MIBTIMETICKS	    7103
#define	VIP_MIBIPADDRESS        7104
#define VIP_MIBCOUNTER		    7105
#define	VIP_MIBGAUGE            7106
#define	VIP_MIBTRIGGERTRAP      7107
#define	VIP_MIBTRIGGERTRAP2     7108

#define RADIO_ASSO_LEN 50000

#define INTEGER_MAX_ARGS 32
struct snmp_validate
{
	char validate_type[64];
	int validate_min;
	int validate_max;
	int argnum;
	char args[INTEGER_MAX_ARGS][32];
};

struct snmp_item
{
	char confile[64];
	char section[64];
	char item[64];
	char val[64];
	unsigned int OID0;
	unsigned int OID1;
	unsigned int OID2;
	unsigned int OID3;
	struct variable4 var_list;
	struct snmp_validate var_validate;
};

/* _MIBGROUP_VIP_MIB_H */
#endif                      

