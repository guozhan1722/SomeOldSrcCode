/*
 *  Template MIB group implementation - VIP_MIB.c
 *
 */

/*
 * include important headers
 */
//#include "UI_Menu_Process.h"

#include <net-snmp/net-snmp-config.h>
#if HAVE_STDLIB_H
    #include <stdlib.h>
#endif
#if HAVE_STRING_H
    #include <string.h>
#else
    #include <strings.h>
#endif

/*
 * needed by util_funcs.h
 */
#if TIME_WITH_SYS_TIME
    #ifdef WIN32
        #include <sys/timeb.h>
    #else
        #include <sys/time.h>
    #endif
    #include <time.h>
#else
    #if HAVE_SYS_TIME_H
        #include <sys/time.h>
    #else
        #include <time.h>
    #endif
#endif

#if HAVE_WINSOCK_H
    #include <winsock.h>
#endif
#if HAVE_NETINET_IN_H
    #include <netinet/in.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/*
 * header_generic() comes from here
 */
#include "util_funcs.h"
/*
 * include our .h file
 */
#include "ipx20_mib.h"
#include "uci.h"
/*
 *  Certain objects can be set via configuration file directives.
 *  These variables hold the values for such objects, as they need to
 *   be accessible to both the config handlers, and the callback routine.
 */
#define  VIP4G_OID 6000
#define VIP_MIB_STR_LEN	300
#define VIP_MIB_STR_DEFAULT	"Device Config Data:"
int             VIP_MIB_int = 42;
char            VIP_MIB_str[VIP_MIB_STR_LEN];
static int string_char_num;
static unsigned short current_string;

#include <syslog.h>
//#define	LOGOPTS		(LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)

#define rBuff_LEN 140
char rBuff[rBuff_LEN];
#define tmpBuff_LEN 140
char tmpBuff[tmpBuff_LEN];

static int SYSTEM_NEED_SET=0;
static int IP_NEED_SET=0;
static int NTP_NEED_SET=0;
static int DHCP_NEED_SET=0;
static int SNMP_NEED_SET=0;
static int BRIDGE_NEED_SET=0;

static int WIFI_NEED_SET=0;
static int COM1_NEED_SET=0;
static int AP_WIRELESS_SEC_NEED_SET=0;
static int STA_WIRELESS_SEC_NEED_SET=0;
static int REPEATER_WIRELESS_SEC_NEED_SET=0;
static int DISCOVERY_SERVICE_NEED_SET=0;
static int SECURITY_UI_ACCESS_NEED_SET=0;
static int SECURITY_AUTH_NEED_SET=0;
static int TOOL_PING_NEED_SET=0;

struct uci_context *ctx = NULL;

static unsigned char confile[64], section[64],item[64];
static char  *SNMP_ITEM_FILE="/lib/snmp_item";
static struct snmp_item MhVIP2Item[512];
#define MhVIP2Item_DEBUG 0
static int arrary_num;
#define read_MIB 0
struct variable4 *VIP_MIB_variables;

//static char notavailabe[4]="N/A";

/*
 * Forward declarations for the config handlers
 */
void VIP_MIB_parse_config_VIP_MIBint(const char *token,  char *cptr);
void VIP_MIB_parse_config_VIP_MIBstr(const char *token,  char *cptr);
void VIP_MIB_free_config_VIP_MIBint(void);
void VIP_MIB_free_config_VIP_MIBstr(void);
bool get_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value);
bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value);
bool get_vface_by_radio(struct uci_context *ctx, char *radio_name, char *vface);
static bool IP_Address_check(const char* buffer,const unsigned int buff_len);
static bool digit_check(const char* buffer,const unsigned int buff_len, int num_low, int num_high);
static bool init_item_group(char * filename);
static bool build_mib_var();
static void variable_dump();
static bool validate_item_value(char * validate_type, char * set_var, size_t var_val_len,int linenum);
static int set_interger_item_value(u_short magic, long intval);
static int set_string_item_value(u_short magic, char *set_var,size_t var_val_len);
static int get_single_item_value(u_short magic);
static bool isMhAlpha(char * result);
static bool mhsplite(char * input,char * val,char *v_name);
static int get_single_item(u_short magic);


/*********************
*
*  Initialisation & common implementation functions
*
*********************/

/*
 * This array structure defines a representation of the
 *  MIB being implemented.
 *
 * The type of the array is 'struct variableN', where N is
 *  large enough to contain the longest OID sub-component
 *  being loaded.  This will normally be the maximum value
 *  of the fifth field in each line.  In this case, the second
 *  and third entries are both of size 2, so we're using
 *  'struct variable2'
 *
 * The supported values for N are listed in <agent/var_struct.h>
 *  If the value you need is not listed there, simply use the
 *  next largest that is.
 *
 * The format of each line is as follows
 *  (using the first entry as an example):
 *      1: VIP_MIBSTRING:
 *          The magic number defined in the VIP_MIB header file.
 *          This is passed to the callback routine and is used
 *            to determine which object is being queried.
 *      2: ASN_OCTET_STR:
 *          The type of the object.
 *          Valid types are listed in <snmp_impl.h>
 *      3: RONLY (or RWRITE):
 *          Whether this object can be SET or not.
 *      4: var_VIP_MIB:
 *          The callback routine, used when the object is queried.
 *          This will usually be the same for all objects in a module
 *            and is typically defined later in this file.
 *      5: 1:
 *          The length of the OID sub-component (the next field)
 *      6: {1}:
 *          The OID sub-components of this entry.
 *          In other words, the bits of the full OID that differ
 *            between the various entries of this array.
 *          This value is appended to the common prefix (defined later)
 *            to obtain the full OID of each entry.
 */
 
#include <syslog.h>
#define	LOGOPTS		(LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)

/*
 * This array defines the OID of the top of the mib tree that we're
 *  registering underneath.
 * Note that this needs to be the correct size for the OID being
 *  registered, so that the length of the OID can be calculated.
 *  The format given here is the simplest way to achieve this.
 */
oid             VIP_MIB_variables_oid[] = { 1, 3, 6, 1, 4, 1, 21703};

/*
 * This function is called at the time the agent starts up
 *  to do any initializations that might be required.
 *
 * In theory it is optional and can be omitted if no
 *  initialization is needed.  In practise, every module
 *  will need to register itself (or the objects being
 *  implemented will not appear in the MIB tree), and this
 *  registration is typically done here.
 *
 * If this function is added or removed, you must re-run
 *  the configure script, to detect this change.
 */
void
init_VIP_MIB(void)
{
    /*
     * Register ourselves with the agent to handle our mib tree.
     * The arguments are:
     *    descr:   A short description of the mib group being loaded.
     *    var:     The variable structure to load.
     *                  (the name of the variable structure defined above)
     *    vartype: The type of this variable structure
     *    theoid:  The OID pointer this MIB is being registered underneath.
     */

	/*collect data from the file SNMP_ITEM_FILE
	*/
	if(init_item_group(SNMP_ITEM_FILE)==FALSE){
    	syslog(LOGOPTS, "The file %s cannot use, use default value.\n",SNMP_ITEM_FILE);
    } 

	struct variable4 G_MIB_variables[arrary_num+1];
	VIP_MIB_variables=G_MIB_variables;
	build_mib_var();

    REGISTER_MIB("VIP_MIB", G_MIB_variables, variable4,
                 VIP_MIB_variables_oid);


    /*
     *  Register config handlers for the two objects that can be set
     *   via configuration file directive.
     *  Also set a default value for the string object.  Note that the
     *   VIP_MIB integer variable was initialised above.
     */
    strncpy(VIP_MIB_str, VIP_MIB_STR_DEFAULT, VIP_MIB_STR_LEN);

    snmpd_register_config_handler("VIP_MIBint",
                                  VIP_MIB_parse_config_VIP_MIBint,
                                  VIP_MIB_free_config_VIP_MIBint,
                                  "VIP_MIBint value");
    snmpd_register_config_handler("VIP_MIBstr",
                                  VIP_MIB_parse_config_VIP_MIBstr,
                                  VIP_MIB_free_config_VIP_MIBstr,
                                  "VIP_MIBstr value");
    snmpd_register_config_handler("VIP_MIBstring",
                                  VIP_MIB_parse_config_VIP_MIBstr,
                                  VIP_MIB_free_config_VIP_MIBstr,
                                  "VIP_MIBstring value");

    ctx = uci_alloc_context();
    if (!ctx) {
        fprintf(stderr, "Out of memory\n");
        return;
    }

    /*
     * One common requirement is to read values from the kernel.
     * This is usually initialised here, to speed up access when the
     *  information is read in, as a response to an incoming request.
     *
     * This module doesn't actually use this mechanism,
     * so this call is commented out here.
     */
    /*
     * auto_nlist( "VIP_MIB_symbol", 0, 0 );
     */
}

/*********************
*
*  Configuration file handling functions
*
*********************/

void
VIP_MIB_parse_config_VIP_MIBint(const char *token, char *cptr)
{
    VIP_MIB_int = atoi(cptr);
}

void
VIP_MIB_parse_config_VIP_MIBstr(const char *token, char *cptr)
{
    /*
     * Make sure the string fits in the space allocated for it.
     */
    if (strlen(cptr) < VIP_MIB_STR_LEN)
        strcpy(VIP_MIB_str, cptr);
    else {
        /*
         * Truncate the string if necessary.
         * An alternative approach would be to log an error,
         *  and discard this value altogether.
         */
        strncpy(VIP_MIB_str, cptr, VIP_MIB_STR_LEN - 4);
        VIP_MIB_str[VIP_MIB_STR_LEN - 4] = 0;
        strcat(VIP_MIB_str, "...");
        VIP_MIB_str[VIP_MIB_STR_LEN - 1] = 0;
    }
}

/*
 * We don't need to do anything special when closing down
 */
void
VIP_MIB_free_config_VIP_MIBint(void)
{
}

void
VIP_MIB_free_config_VIP_MIBstr(void)
{
}

/*********************
*
*  System specific implementation functions
*
*********************/

/*
 * Define the callback function used in the VIP_MIB_variables structure.
 * This is called whenever an incoming request refers to an object
 *  within this sub-tree.
 *
 * Four of the parameters are used to pass information in.
 * These are:
 *    vp      The entry from the 'VIP_MIB_variables' array for the
 *             object being queried.
 *    name    The OID from the request.
 *    length  The length of this OID.
 *    exact   A flag to indicate whether this is an 'exact' request
 *             (GET/SET) or an 'inexact' one (GETNEXT/GETBULK).
 *
 * Four of the parameters are used to pass information back out.
 * These are:
 *    name     The OID being returned.
 *    length   The length of this OID.
 *    var_len  The length of the answer being returned.
 *    write_method   A pointer to the SET function for this object.
 *
 * Note that name & length serve a dual purpose in both roles.
 */

u_char         *
var_VIP_MIB(struct variable *vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{

    /*
     *  The result returned from this function needs to be a pointer to
     *    static data (so that it can be accessed from outside).
     *  Define suitable variables for any type of data we may return.
     */
    static char     string[VIP_MIB_STR_LEN];      /* for VIP_MIBSTRING   */
    static oid      oid_ret[8]; /* for VIP_MIBOBJECTID */
    static long     long_ret;   /* for everything else */
    static char     AssociationB[RADIO_ASSO_LEN]="\n\r";

    int i,num;
    FILE* f;
    int freq;

    //unsigned int Menu, Item;
    /*
     * Before returning an answer, we need to check that the request
     *  refers to a valid instance of this object.  The utility routine
     *  'header_generic' can be used to do this for scalar objects.
     *
     * This routine 'header_simple_table' does the same thing for "simple"
     *  tables. (See the AGENT.txt file for the definition of a simple table).
     *
     * Both these utility routines also set up default values for the
     *  return arguments (assuming the check succeeded).
     * The name and length are set suitably for the current object,
     *  var_len assumes that the result is an integer of some form,
     *  and write_method assumes that the object cannot be set.
     *
     * If these assumptions are correct, this callback routine simply
     * needs to return a pointer to the appropriate value (using 'long_ret').
     * Otherwise, 'var_len' and/or 'write_method' should be set suitably.
     */
    DEBUGMSGTL(("VIP_MIB", "var_VIP_MIB entered\n"));
    if (header_generic(vp, name, length, exact, var_len, write_method) ==
        MATCH_FAILED)
        return NULL;


    /*
     * Many object will need to obtain data from the operating system in
     *  order to return the appropriate value.  Typically, this is done
     *  here - immediately following the 'header' call, and before the
     *  switch statement. This is particularly appropriate if a single
     *  interface call can return data for all the objects supported.
     *
     * This VIP_MIB module does not rely on external data, so no such
     *  calls are needed in this case.
     */

    /*
     * Now use the magic number from the variable pointer 'vp' to
     *  select the particular object being queried.
     * In each case, one of the static objects is set up with the
     *  appropriate information, and returned mapped to a 'u_char *'
     */


    current_string=vp->magic;

    if (current_string<1) {
        return NULL;
    } else {
        if (!SYSTEM_NEED_SET) {
            if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
            }
            ctx = uci_alloc_context();
            if (!ctx) {
                fprintf(stderr, "Out of memory\n");
                return NULL;
            }
        }
    }

    memset(rBuff,0,sizeof(rBuff));

    memset(confile, 0, sizeof(confile));
    memset(section, 0, sizeof(section));
    memset(item, 0, sizeof(item));

    switch (current_string) {
	
    case VIP_MIBSTRING:
        sprintf(string, "Device Configuration ");
        /*
         * Note that the assumption that the answer will be an
         *  integer does not hold true in this case, so the length
         *  of the answer needs to be set explicitly.
         */
        *var_len = strlen(string);
        return(u_char *) string;

    case VIP_MIBINTEGER:
        /*
         * Here the length assumption is correct, but the
         *  object is writeable, so we need to set the
         *  write_method pointer as well as the current value.
         */
        long_ret = VIP_MIB_int;
        *write_method = write_VIP_MIB_Action_int;
        return(u_char *) & long_ret;

    case VIP_MIBOBJECTID:
        oid_ret[0] = 1;
        oid_ret[1] = 3;
        oid_ret[2] = 6;
        oid_ret[3] = 1;
        oid_ret[4] = 4;
        oid_ret[5] = oid_ret[6] = oid_ret[7] = 42;
        /*
         * Again, the assumption regarding the answer length is wrong.
         */
        *var_len = 8 * sizeof(oid);
        return(u_char *) oid_ret;

    case VIP_MIBTIMETICKS:
        /*
         * Here both assumptions are correct,
         *  so we just need to set up the answer.
         */
        long_ret = 363136200;   /* 42 days, 42 minutes and 42.0 seconds */
        return(u_char *) & long_ret;

    case VIP_MIBIPADDRESS:
        /*
         * ipaddresses get returned as a long.  ick
         */
        /*
         * we're returning 127.0.0.1
         */
        long_ret = ntohl(INADDR_LOOPBACK);
        return(u_char *) & long_ret;

    case VIP_MIBCOUNTER:
        long_ret = 42;
        return(u_char *) & long_ret;

    case VIP_MIBGAUGE:
        long_ret = 42;          /* Do we detect a theme running through these answers? */
        return(u_char *) & long_ret;

    case VIP_MIBTRIGGERTRAP:
        /*
         * This object is essentially "write-only".
         * It only exists to trigger the sending of a trap.
         * Reading it will always return 0.
         */
        long_ret = 0;
        *write_method = write_VIP_MIBtrap;
        return(u_char *) & long_ret;

    case VIP_MIBTRIGGERTRAP2:
        /*
         * This object is essentially "write-only".
         * It only exists to trigger the sending of a v2 trap.
         * Reading it will always return 0.
         */
        long_ret = 0;
        *write_method = write_VIP_MIBtrap2;
        return(u_char *) & long_ret;

    default:
		/*Here will get the single item value from uci*/
        num=get_single_item_value(current_string);
        if(num<0){
        	DEBUGMSGTL(("snmpd", "unknown sub-id %d in VIP_MIBs/var_VIP_MIB\n",
                        vp->magic));
            return NULL;
        }

		/*handle special magic item here*/
		if(strcmp(MhVIP2Item[num].section, "asso0")== 0){
			SubProgram_Start("/bin/sh","/etc/tmpstatus asso0");
			i=0;
			/*there is database*/
			if ( i == 0 ) {
				if (!(f = fopen("/var/run/radio0_association","r"))) {
					*var_len = strlen("None");
					return(u_char *) "None"; 
				} else {
					fseek(f, 0, SEEK_END);
					*var_len = ftell(f);
					/*try to avoid memery overflow*/
					if (*var_len >= 50000-10) {
						*var_len = 50000-10;
					}
					fseek(f, 0, SEEK_SET);
				}
			} else {
				if (!(f = fopen("/var/run/radio0_association","w+"))) {
					*var_len = strlen("None");
					return(u_char *) "None"; 
				}
			}
			rewind(f);
			fread(AssociationB+2, *var_len, 1, f); 
			*var_len+=2;
			fclose(f);	
			if (*var_len<10) {
				*var_len = strlen("None");
				return(u_char *) "None"; 
			} else
				return(u_char *) AssociationB; 


		}
		else if(strcmp(MhVIP2Item[num].item , "ACTION")== 0){
			long_ret = 0;
			*write_method = write_VIP_MIB_Action_int;
			return(u_char *) & long_ret;
		} else if(MhVIP2Item[num].var_list.type == ASN_OCTET_STR){
        	*var_len = strlen(MhVIP2Item[num].val);
			if(MhVIP2Item[i].var_list.acl == RWRITE ){
        		*write_method =write_VIP_MIBstr;
			}
        	return(u_char *) MhVIP2Item[num].val;
        }else if (MhVIP2Item[num].var_list.type == ASN_INTEGER) {
			//if(isMhAlpha(MhVIP2Item[num].val)){
			//	long_ret=MhVIP2Item[num].val[0]-'A';
			//	if(MhVIP2Item[i].var_list.acl == RWRITE ){
			//		*write_method = write_VIP_MIBint;
			//	}
			//	return(u_char *) & long_ret;
			
			//}else {
				long_ret=atoi(MhVIP2Item[num].val);
				if(MhVIP2Item[i].var_list.acl == RWRITE ){
					*write_method = write_VIP_MIBint;
				}
            	return(u_char *) & long_ret;
			//}
        }
		return NULL;

    }


    /*
     * If we fall through to here, fail by returning NULL.
     * This is essentially a continuation of the 'default' case above.
     */
}
/*********************
*
*  Writeable object SET handling routines	created at 20005 08 30
*
*********************/
int
write_VIP_MIBstr(int action,
                 u_char * var_val,
                 u_char var_val_type,
                 size_t var_val_len,
                 u_char * statP, oid * name, size_t name_len)
{
    /*
     * Define an arbitrary maximum permissible value
     */
    //syslog(LOGOPTS,"write_VIP_MIBstr  current_string=%d\n",current_string);

	int retval;
	
    memset(rBuff,0,sizeof(rBuff));
    var_val[var_val_len]=0;
    switch (action) {
    case RESERVE1:
        /*
         *  Check that the value being set is acceptable
         */
        if (var_val_type != ASN_OCTET_STR) {
            DEBUGMSGTL(("VIP_MIB", "%x not integer type", var_val_type));
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len >32) {
            DEBUGMSGTL(("VIP_MIB", "wrong length %x", var_val_len));
            return SNMP_ERR_WRONGLENGTH;
        }

        switch (current_string) {
        default:
			retval=set_string_item_value(current_string,var_val,var_val_len);
        	if (retval==-1) {
            	return SNMP_ERR_NOERROR;
       		}else if (retval==-2) {
				return SNMP_ERR_WRONGLENGTH;
       		}else {
				SNMP_NEED_SET =1;
			}
		}
        //set_option_by_section_name(ctx,confile, section,item, var_val);

        break;

    case RESERVE2:
        /*
         *  This is conventially where any necesary
         *   resources are allocated (e.g. calls to malloc)
         *  Here, we are using static variables
         *   so don't need to worry about this.
         */
        break;

    case FREE:
        /*
         *  This is where any of the above resources
         *   are freed again (because one of the other
         *   values being SET failed for some reason).
         *  Again, since we are using static variables
         *   we don't need to worry about this either.
         */
        break;

    case ACTION:
        /*
         *  Set the variable as requested.
         *   Note that this may need to be reversed,
         *   so save any information needed to do this.
         */
        //	old_intval = VIP_MIB_int;
        //	VIP_MIB_int = intval;
        break;

    case UNDO:
        /*
         *  Something failed, so re-set the
         *   variable to its previous value
         *  (and free any allocated resources).
         */
        //	VIP_MIB_int = old_intval;
        break;

    case COMMIT:
        /*
         *  Everything worked, so we can discard any
         *   saved information, and make the change
         *   permanent (e.g. write to the config file).
         *  We also free any allocated resources.
         *
         *  In this case, there's nothing to do.
         */

        break;
    }
    return SNMP_ERR_NOERROR;
}

/*********************
*
*  Writeable object SET handling routines
*
*********************/
int
write_VIP_MIBint(int action,
                 u_char * var_val,
                 u_char var_val_type,
                 size_t var_val_len,
                 u_char * statP, oid * name, size_t name_len)
{
    /*
     * Define an arbitrary maximum permissible value
     */
#define MAX_VIP_MIB_INT	100
    static long     intval;
    static long     old_intval;
    int menu_process_return;
    char buffer[32];
	int retval;
	int num;
//syslog(LOGOPTS,"write_VIP_MIBint  action=%d\n",action);

    switch (action) {
    case RESERVE1:
        /*
         *  Check that the value being set is acceptable
         */
        if (var_val_type != ASN_INTEGER) {
            DEBUGMSGTL(("VIP_MIB", "%x not integer type", var_val_type));
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            DEBUGMSGTL(("VIP_MIB", "wrong length %x", var_val_len));
            return SNMP_ERR_WRONGLENGTH;
        }

        intval = *((long *) var_val);
        //if (intval > 100) {
        //    DEBUGMSGTL(("VIP_MIB", "wrong value %x", intval));
        //    return SNMP_ERR_WRONGVALUE;
        //}
        buffer[0]=intval + 'A';
        buffer[1]='\0';
        var_val_len =1;
        syslog(LOGOPTS,"write_VIP_MIBint RESERVE1 current_string=%d intval=%d\n",current_string,intval);

        switch (current_string) {
        default:
        	//sprintf(buffer,"%d",intval);
        	
			//num=get_single_item_value(current_string);
			//if(num<0){
			//	DEBUGMSGTL(("snmpd", "unknown sub-id %d in VIP_MIBs/var_VIP_MIB\n",
			//				vp->magic));
			//	return NULL;
			//}

			//if(isMhAlpha(MhVIP2Item[num].val)){
			//	buffer[0]=intval + 'A';
			//	buffer[1]='\0';
			//	var_val_len =1;
			//	retval=set_string_item_value(current_string,buffer,var_val_len);
        	//	if (retval == -1) {
            //		return SNMP_ERR_NOERROR;
        	//	}else if (retval == -2) {
			//		return SNMP_ERR_WRONGLENGTH;
			//	} else {
			//		SNMP_NEED_SET =1;
			//	}
				
			//}else {
				retval = set_interger_item_value(current_string,intval);
        		if (retval == -1) {
            		return SNMP_ERR_NOERROR;
        		}else if (retval == -2) {
					return SNMP_ERR_WRONGLENGTH;
				} else {
					SNMP_NEED_SET =1;
				}
			//}
            break;
        }

        break;

    case RESERVE2:
        /*
         *  This is conventially where any necesary
         *   resources are allocated (e.g. calls to malloc)
         *  Here, we are using static variables
         *   so don't need to worry about this.
         */
        break;

    case FREE:
        /*
         *  This is where any of the above resources
         *   are freed again (because one of the other
         *   values being SET failed for some reason).
         *  Again, since we are using static variables
         *   we don't need to worry about this either.
         */
        break;

    case ACTION:
        /*
         *  Set the variable as requested.
         *   Note that this may need to be reversed,
         *   so save any information needed to do this.
         */
        old_intval = VIP_MIB_int;
        VIP_MIB_int = intval;
        break;

    case UNDO:
        /*
         *  Something failed, so re-set the
         *   variable to its previous value
         *  (and free any allocated resources).
         */
        VIP_MIB_int = old_intval;
        break;

    case COMMIT:
        /*
         *  Everything worked, so we can discard any
         *   saved information, and make the change
         *   permanent (e.g. write to the config file).
         *  We also free any allocated resources.
         *
         *  In this case, there's nothing to do.
         */
        /*if it is confirm or cancel, then reset NEED_SET value to 0 */

        break;
    }
    return SNMP_ERR_NOERROR;
}


/*********************
*
*  Writeable object SET handling routines
*
*********************/
int
write_VIP_MIB_Action_int(int action,
                         u_char * var_val,
                         u_char var_val_type,
                         size_t var_val_len,
                         u_char * statP, oid * name, size_t name_len)
{
    /*
     * Define an arbitrary maximum permissible value
     */
#define MAX_VIP_MIB_INT	100
    static long     intval;
    static long     old_intval;
    int menu_process_return;
	int itemnum;
	char tmpstatu_cmd[64];
    //syslog(LOGOPTS,"write_VIP_MIB_Action_int \n");

    switch (action) {
    case RESERVE1:
        /*
         *  Check that the value being set is acceptable
         */
        if (var_val_type != ASN_INTEGER) {
            DEBUGMSGTL(("VIP_MIB", "%x not integer type", var_val_type));
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            DEBUGMSGTL(("VIP_MIB", "wrong length %x", var_val_len));
            return SNMP_ERR_WRONGLENGTH;
        }

        intval = *((long *) var_val);
        if (intval > MAX_VIP_MIB_INT) {
            DEBUGMSGTL(("VIP_MIB", "wrong value %x", intval));
            return SNMP_ERR_WRONGVALUE;
        }

        if ((intval !=0)&&(intval !=1)&&(intval !=2)) {
            DEBUGMSGTL(("VIP_MIB", "wrong value %x", intval));
            return SNMP_ERR_WRONGVALUE;
        }

        switch (current_string) {
#if 0
        case    System_Config_ACTION:
            if (intval==1) {
                menu_process_return=SubProgram_Start("/etc/init.d/systemmode","start");
                if (menu_process_return!=1)
                    return SNMP_ERR_WRONGVALUE;
                menu_process_return=SubProgram_Start("/bin/sh","/usr/lib/webif/apply.sh");
                SYSTEM_NEED_SET =0;
                if (menu_process_return!=1)
                    return SNMP_ERR_WRONGVALUE;
            } else if (intval==2) {
                menu_process_return=SubProgram_Start("/bin/sh","/etc/rmuci");
                SYSTEM_NEED_SET =0;
                if (menu_process_return!=1)
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case    Tool_Reset_System_ACTION:
            if (intval==1)
                menu_process_return=SubProgram_Start("reboot","");
            break;
#endif
        default:
			SNMP_NEED_SET = 0;
			itemnum=get_single_item(current_string);
			if(itemnum < 0) return SNMP_ERR_WRONGVALUE;
#if 0
			if (intval==1) {
				if(strcmp(MhVIP2Item[itemnum].confile,"tools")==0 ){
				/*do something for tools action*/
					if(strcmp(MhVIP2Item[itemnum].section,"reboot")==0){
						menu_process_return=SubProgram_Start("reboot","");
					}
                }else {
					menu_process_return=SubProgram_Start("/bin/sh","/usr/lib/webif/apply.sh");
                	if (menu_process_return!=1)
                    	return SNMP_ERR_WRONGVALUE;
                }
            } else if (intval==2) {
                syslog(LOGOPTS,"write_VIP_MIB_Action_int intval is 2 will reverse \n");
                menu_process_return=SubProgram_Start("/bin/sh","/etc/rmuci");
                if (menu_process_return!=1)
                    return SNMP_ERR_WRONGVALUE;
            }
#else
			if(strcmp(MhVIP2Item[itemnum].confile,"tools")==0 ){
				if (intval==1) {
					strcpy(tmpstatu_cmd,"/etc/tmpstatus ");
					strcat(tmpstatu_cmd,MhVIP2Item[itemnum].section);
					menu_process_return=SubProgram_Start("/bin/sh",tmpstatu_cmd);
					if (menu_process_return!=1)
						return SNMP_ERR_WRONGVALUE;
				} else if (intval==2) {
					syslog(LOGOPTS,"write_VIP_MIB_Action_int intval is 2 for  will reverse \n");
				}
			} else {
				if (intval==1) {
					menu_process_return=SubProgram_Start("/bin/sh","/usr/lib/webif/apply.sh");
					if (menu_process_return!=1)
						return SNMP_ERR_WRONGVALUE;
				} else if (intval==2) {
					syslog(LOGOPTS,"write_VIP_MIB_Action_int intval is 2 will reverse \n");
					menu_process_return=SubProgram_Start("/bin/sh","/etc/rmuci");
					if (menu_process_return!=1)
						return SNMP_ERR_WRONGVALUE;
				}
			}
#endif
            break;
        }

        break;
    case RESERVE2:
        /*
         *  This is conventially where any necesary
         *   resources are allocated (e.g. calls to malloc)
         *  Here, we are using static variables
         *   so don't need to worry about this.
         */
        break;

    case FREE:
        /*
         *  This is where any of the above resources
         *   are freed again (because one of the other
         *   values being SET failed for some reason).
         *  Again, since we are using static variables
         *   we don't need to worry about this either.
         */
        break;

    case ACTION:
        /*
         *  Set the variable as requested.
         *   Note that this may need to be reversed,
         *   so save any information needed to do this.
         */
        old_intval = VIP_MIB_int;
        VIP_MIB_int = intval;
        break;

    case UNDO:
        /*
         *  Something failed, so re-set the
         *   variable to its previous value
         *  (and free any allocated resources).
         */
        VIP_MIB_int = old_intval;
        break;

    case COMMIT:
        /*
         *  Everything worked, so we can discard any
         *   saved information, and make the change
         *   permanent (e.g. write to the config file).
         *  We also free any allocated resources.
         *
         *  In this case, there's nothing to do.
         */
        /*if it is confirm or cancel, then reset NEED_SET value to 0 */


        break;
    }
    return SNMP_ERR_NOERROR;
}



int
write_VIP_MIBtrap(int action,
                  u_char * var_val,
                  u_char var_val_type,
                  size_t var_val_len,
                  u_char * statP, oid * name, size_t name_len)
{
    long            intval;

    DEBUGMSGTL(("VIP_MIB", "write_VIP_MIBtrap entered: action=%d\n",
                action));
    switch (action) {
    case RESERVE1:
        /*
         *  The only acceptable value is the integer 1
         */
        if (var_val_type != ASN_INTEGER) {
            DEBUGMSGTL(("VIP_MIB", "%x not integer type", var_val_type));
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            DEBUGMSGTL(("VIP_MIB", "wrong length %x", var_val_len));
            return SNMP_ERR_WRONGLENGTH;
        }

        intval = *((long *) var_val);
        if (intval != 1) {
            DEBUGMSGTL(("VIP_MIB", "wrong value %x", intval));
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case RESERVE2:
        /*
         * No resources are required....
         */
        break;

    case FREE:
        /*
         * ... so no resources need be freed
         */
        break;

    case ACTION:
        /*
         *  Having triggered the sending of a trap,
         *   it would be impossible to revoke this,
         *   so we can't actually invoke the action here.
         */
        break;

    case UNDO:
        /*
         * We haven't done anything yet,
         * so there's nothing to undo
         */
        break;

    case COMMIT:
        /*
         *  Everything else worked, so it's now safe
         *   to trigger the trap.
         *  Note that this is *only* acceptable since
         *   the trap sending routines are "failsafe".
         *  (In fact, they can fail, but they return no
         *   indication of this, which is the next best thing!)
         */
        DEBUGMSGTL(("VIP_MIB", "write_VIP_MIBtrap sending the trap\n",
                    action));
        send_easy_trap(SNMP_TRAP_ENTERPRISESPECIFIC, 99);
        DEBUGMSGTL(("VIP_MIB", "write_VIP_MIBtrap trap sent\n", action));
        break;

    }
    return SNMP_ERR_NOERROR;
}

/*
 * this documents how to send a SNMPv2 (and higher) trap via the
 * send_v2trap() API.
 *
 * Coding SNMP-v2 Trap:
 *
 * The SNMPv2-Trap PDU contains at least a pair of object names and
 * values: - sysUpTime.0 whose value is the time in hundredths of a
 * second since the netwok management portion of system was last
 * reinitialized.  - snmpTrapOID.0 which is part of the trap group SNMPv2
 * MIB whose value is the object-id of the specific trap you have defined
 * in your own MIB.  Other variables can be added to caracterize the
 * trap.
 *
 * The function send_v2trap adds automaticallys the two objects but the
 * value of snmpTrapOID.0 is 0.0 by default. If you want to add your trap
 * name, you have to reconstruct this object and to add your own
 * variable.
 *
 */



int
write_VIP_MIBtrap2(int action,
                   u_char * var_val,
                   u_char var_val_type,
                   size_t var_val_len,
                   u_char * statP, oid * name, size_t name_len)
{
    long            intval;

    /*
     * these variales will be used when we send the trap
     */
    oid             objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};     /* snmpTrapOID.0 */
    oid             demo_trap[] = { 1, 3, 6, 1, 4, 1, 21703, 113, 990};  /*demo-trap */
    oid             VIP_MIB_string_oid[] =
    { 1, 3, 6, 1, 4, 1, 2021, 254, 1, 0};
    static netsnmp_variable_list var_trap;
    static netsnmp_variable_list var_obj;

    DEBUGMSGTL(("VIP_MIB", "write_VIP_MIBtrap2 entered: action=%d\n",
                action));
    switch (action) {
    case RESERVE1:
        /*
         *  The only acceptable value is the integer 1
         */
        if (var_val_type != ASN_INTEGER) {
            DEBUGMSGTL(("VIP_MIB", "%x not integer type", var_val_type));
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            DEBUGMSGTL(("VIP_MIB", "wrong length %x", var_val_len));
            return SNMP_ERR_WRONGLENGTH;
        }

        intval = *((long *) var_val);
        if (intval != 1) {
            DEBUGMSGTL(("VIP_MIB", "wrong value %x", intval));
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case RESERVE2:
        /*
         * No resources are required....
         */
        break;

    case FREE:
        /*
         * ... so no resources need be freed
         */
        break;

    case ACTION:
        /*
         *  Having triggered the sending of a trap,
         *   it would be impossible to revoke this,
         *   so we can't actually invoke the action here.
         */
        break;

    case UNDO:
        /*
         * We haven't done anything yet,
         * so there's nothing to undo
         */
        break;

    case COMMIT:
        /*
         *  Everything else worked, so it's now safe
         *   to trigger the trap.
         *  Note that this is *only* acceptable since
         *   the trap sending routines are "failsafe".
         *  (In fact, they can fail, but they return no
         *   indication of this, which is the next best thing!)
         */

        /*
         * trap definition objects
         */

        var_trap.next_variable = &var_obj;      /* next variable */
        var_trap.name = objid_snmptrap; /* snmpTrapOID.0 */
        var_trap.name_length = sizeof(objid_snmptrap) / sizeof(oid);    /* number of sub-ids */
        var_trap.type = ASN_OBJECT_ID;
        var_trap.val.objid = demo_trap; /* demo-trap objid */
        var_trap.val_len = sizeof(demo_trap);   /* length in bytes (not number of subids!) */


        /*
         * additional objects
         */


        var_obj.next_variable = NULL;   /* No more variables after this one */
        var_obj.name = VIP_MIB_string_oid;
        var_obj.name_length = sizeof(VIP_MIB_string_oid) / sizeof(oid); /* number of sub-ids */
        var_obj.type = ASN_OCTET_STR;   /* type of variable */
        var_obj.val.string = VIP_MIB_str;         /* value */
        var_obj.val_len = strlen(VIP_MIB_str);
        DEBUGMSGTL(("VIP_MIB", "write_VIP_MIBtrap2 sending the v2 trap\n",
                    action));
        send_v2trap(&var_trap);
        DEBUGMSGTL(("VIP_MIB", "write_VIP_MIBtrap2 v2 trap sent\n",
                    action));

        break;

    }
    return SNMP_ERR_NOERROR;
}


bool get_vface_by_radio(struct uci_context *ctx, char *radio_name, char *vface)
{
    char *tuple;
    struct uci_element *e= NULL;
    struct uci_ptr ptr;
    struct uci_option *p_option = NULL;
    char sec[15];
    char value[7];
    char *section=sec;
    int i;

    strcpy(vface, "none");
    strcpy(section,"@wifi-iface[0]");
    for (i=0;i<4;i++) {
        sec[12]=i+48;
        tuple = malloc( strlen("wireless") + strlen(section) + strlen("device")+ 3 ); /* "." and trailing \0 */

        if (!tuple) return false;

        strcpy(tuple, "wireless");
        strcat(tuple, ".");
        strcat(tuple, section);
        strcat(tuple, ".");
        strcat(tuple, "device");

        if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK) {
            e = ptr.last;

            if ( (ptr.flags & UCI_LOOKUP_COMPLETE) && (e->type == UCI_TYPE_OPTION) ) {
                p_option = ptr.o;
                strcpy(value, p_option->v.string);

                if (strcmp(value,radio_name)==0) {
                    strcpy(vface,section);
                    free(tuple);
                    return true;
                }
            }
        }
        free(tuple);
    }

    return false;
}

/**********************************************************************************
   Function:
  Output:
   Output:
   Return:
   Note:
**********************************************************************************/

bool get_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_element *e= NULL;
    struct uci_ptr ptr;
    struct uci_option *p_option = NULL;

    //printf("%s option_name: %s\n",__func__,option_name);
    strcpy(value, "");

    tuple = malloc( strlen(package_name) + strlen(section_name) + strlen(option_name)+ 3 ); /* "." and trailing \0 */

    if (!tuple) return false;

    strcpy(tuple, package_name);
    strcat(tuple, ".");
    strcat(tuple, section_name);
    strcat(tuple, ".");
    strcat(tuple, option_name);

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK) {

        e = ptr.last;

        if ( (ptr.flags & UCI_LOOKUP_COMPLETE) &&
             (e->type == UCI_TYPE_OPTION) ) {
            p_option = ptr.o;
            strcpy(value, p_option->v.string);
            //printf("%s find %s\n",__func__, value);
            free(tuple);
            return true;
        }
    }

    free(tuple);

    return false;
}


/**********************************************************************************
   Function:  bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
  Output:
   Output:
   Return:
   Note:
**********************************************************************************/

bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_ptr ptr;

    tuple = malloc( strlen(package_name) + strlen(section_name) + strlen(option_name) + strlen(value) + 4 );    /* "." "=" and trailing \0 */

    if (!tuple) return false;

    strcpy(tuple, package_name);
    strcat(tuple, ".");
    strcat(tuple, section_name);
    strcat(tuple, ".");
    strcat(tuple, option_name);
    strcat(tuple, "=");
    strcat(tuple, value);

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK) {
        if ( UCI_OK == uci_set(ctx, &ptr) ) {
            uci_save(ctx, ptr.p);
            //   uci_commit(ctx, &ptr.p, false);
        }

        free(tuple);
        return true;
    }

    free(tuple);

    return false;
}



/**********************************************************************************
   Function:  int SubProgram_Start(char *pszPathName, char *pszArguments)
  Output:
   Output:
   Return:
   Note:
**********************************************************************************/

int SubProgram_Start(char *pszPathName, char *pszArguments)
{
    int  sub_status;
    int MAX_ARGS=8;
    char * pArgs[MAX_ARGS];
    unsigned long ulNumArgs = 1;
    char tmpArgs[256];
    char *pArg = NULL;
    int pid = 0;

    pArgs[0]=pszPathName;
    if (pszArguments != NULL) {
        strcpy(tmpArgs, pszArguments);
        /* 'execl' return -1 on error. */
        pArg = strtok(tmpArgs, " ");
        while ((pArg != NULL) && (ulNumArgs < MAX_ARGS)) {
            pArgs[ulNumArgs] = pArg;
            ulNumArgs++;
            pArg = strtok(NULL, " ");
        }

        /*
        pArgs[0]=pszPathName;
        pArgs[1]=pszArguments;
        pArgs[2] = NULL; */
    }
    pArgs[ulNumArgs] = NULL;
    if (!(pid = vfork())) {
        seteuid((uid_t)0);
        /* We are the child! */
        if (execvp(pszPathName, pArgs) == -1) {
            //syslog(LOGOPTS,"SubProgram_Start false\n");
            return false;
        }
        //syslog(LOGOPTS,"Child PID after = %d\n",pid);
    } else {
        waitpid(pid,&sub_status,0);
        return true;
    }
    return true;
}

/**********************************************************************************
   Function:	static bool IP_Address_check(const char* buffer,const unsigned int buff_len)
  Output:	char* buffer,int buff_len
   Output:	None
   Return:	bool
   Note:	check the buffer to see if ip  address is right according to buff_len
**********************************************************************************/

static bool IP_Address_check(const char* buffer,const unsigned int buff_len)
{
    int i;
    int dot[3]={0,0,0};

    for (i=0;i<buff_len; i++) {    /* get the position of dot*/
        if (buffer[i]=='.') {

            if (dot[0]==0)
                dot[0]= i;
            else {
                if (dot[1]==0)
                    dot[1]=i;
                else {
                    if (dot[2]==0)
                        dot[2]=i;
                    else
                        return FALSE;  /* more than three dots return false*/



                }
            }
        } else { /* not dot,Others must be between 0-9*/
            if (buffer[i]!=0) {
                if ((buffer[i]<'0')||(buffer[i]>'9'))
                /*  if(!isdigit(buffer[i]))*/
                {
                    return FALSE;
                }
            }
        }
    }/*end of for loop*/

    if ((dot[0]<1)||(dot[0]>3))  /* dot[0] position must be 1 to 3.*/
        return FALSE;

    for (i=0;i<2;i++) {
        if (((dot[i+1]- dot[i]) <2)||((dot[i+1]- dot[i]) >4))  /* not 1 and 3*/
            return FALSE;
    } /* check the position between dot is right or wrong*/

    if ((dot[2]==buff_len-1)||(buff_len-dot[2]>4))
        return FALSE;
    if (dot[0]==3) {
        if (buffer[0]>'2')
            return FALSE;
        if ((buffer[0]=='2')&&(buffer[1]>'5'))
            return FALSE;
        if ((buffer[0]=='2')&&(buffer[1]=='5')&&(buffer[2]>'5'))
            return FALSE;
    }
    if (dot[1]-dot[0]==4) {
        if (buffer[dot[0]+1]>'2')/* if number after dot is 2 then number 5,6 should be 55*/
            return FALSE;
        if ((buffer[dot[0]+1]=='2')&&(buffer[dot[0]+2]>'5'))
            return FALSE;
        if ((buffer[dot[0]+1]=='2')&&(buffer[dot[0]+2]=='5')&&(buffer[dot[0]+3]>'5'))
            return FALSE;
    }
    if (dot[2]-dot[1]==4) {
        if (buffer[dot[1]+1]>'2')/* if number after dot is 2 then number 5,6 should be 55*/
            return FALSE;
        if ((buffer[dot[1]+1]=='2')&&(buffer[dot[1]+2]>'5'))
            return FALSE;
        if ((buffer[dot[1]+1]=='2')&&(buffer[dot[1]+2]=='5')&&(buffer[dot[1]+3]>'5'))
            return FALSE;
    }

    if (buff_len-dot[2]==4) {
        if (buffer[dot[2]+1]>'2')
            return FALSE;
        if ((buffer[dot[2]+1]=='2')&&(buffer[dot[2]+2]>'5'))
            return FALSE;
        if ((buffer[dot[2]+1]=='2')&&(buffer[dot[2]+2]=='5')&&(buffer[dot[2]+3]>'5'))
            return FALSE;
    }
    return TRUE;
}
/**********************************************************************************
   Function:	static bool digit_check(const char* buffer,const unsigned int buff_len)
  Output:	const char* buffer,const unsigned int buff_len
   Output:	None
   Return:	bool
   Note:	check the buffer to see if every byte is digit according to buff_len
**********************************************************************************/

static bool digit_check(const char* buffer,const unsigned int buff_len, int num_low, int num_high)
{
    int i;

    for (i=0;i<buff_len;i++) {
        if (!isdigit(buffer[i]))
            return FALSE;
    }

    i=atoi(buffer);

    if ((i<num_low)||(i>num_high))
        return FALSE;
    else
        return TRUE;
}

/*
 * using this for debug
 */
static void MhVIP2Item_dump()
{
	int i;
	syslog(LOGOPTS, "The file %s dump line by line for debug\n",SNMP_ITEM_FILE);

	for(i=0;i<512;i++)
	{
		if(strlen(MhVIP2Item[i].confile) < 2)
			break;
		syslog(LOGOPTS, "line[%d]:[%s]:[%s]:[%s]:[%s]:[%d]:[%d]:[%d]:[%d]\n",i,
				MhVIP2Item[i].confile,MhVIP2Item[i].section,MhVIP2Item[i].item,MhVIP2Item[i].val,MhVIP2Item[i].OID0,MhVIP2Item[i].OID1,MhVIP2Item[i].OID2,MhVIP2Item[i].OID3);
		syslog(LOGOPTS, "table[%d]:[%d]:[%d]:[%d]:[%x]:[%d]:[%d]:[%d]:[%d][%d]\n",i,
				MhVIP2Item[i].var_list.magic ,MhVIP2Item[i].var_list.type,MhVIP2Item[i].var_list.acl,MhVIP2Item[i].var_list.findVar,MhVIP2Item[i].var_list.namelen,
				MhVIP2Item[i].var_list.name[0],MhVIP2Item[i].var_list.name[1],MhVIP2Item[i].var_list.name[2],MhVIP2Item[i].var_list.name[3] );
	}
}

static void MhVIP2Item_validate_dump()
{
	int i;
	syslog(LOGOPTS, "The file %s MhVIP2Item_validate_dump line by line for debug\n",SNMP_ITEM_FILE);

	for(i=0;i<512;i++)
	{
		if(strlen(MhVIP2Item[i].confile) < 2)
			break;
		syslog(LOGOPTS, "magic[%d]:[%s]:[%d]:[%d]:[%d]:[%s]:[%s]:[%s]:[%s]\n",MhVIP2Item[i].var_list.magic,
				MhVIP2Item[i].var_validate.validate_type,MhVIP2Item[i].var_validate.validate_min,MhVIP2Item[i].var_validate.validate_max,
				MhVIP2Item[i].var_validate.argnum,MhVIP2Item[i].var_validate.args[0],
				MhVIP2Item[i].var_validate.args[1],
				MhVIP2Item[i].var_validate.args[2],
				MhVIP2Item[i].var_validate.args[3]);
	}
}

static void variable_dump()
{
	int i;
	syslog(LOGOPTS, "The Build snmp mib variable array dump for debug\n",SNMP_ITEM_FILE);

	for(i=0;i<arrary_num + 1 ; i++)
	{
		if(VIP_MIB_variables[i].magic==0 )	break;
		syslog(LOGOPTS, "table[%d]:[%d]:[%d]:[%d]:[%x]:[%d]:[%d]:[%d]:[%d][%d]\n",i,
				VIP_MIB_variables[i].magic ,VIP_MIB_variables[i].type,VIP_MIB_variables[i].acl,VIP_MIB_variables[i].findVar,VIP_MIB_variables[i].namelen,
				VIP_MIB_variables[i].name[0],VIP_MIB_variables[i].name[1],VIP_MIB_variables[i].name[2],VIP_MIB_variables[i].name[3] );
	}
}

static bool isMhAlpha(char * result)
{
	int rc=0;
	rc=strlen(result);
	if (rc> 1){
		return FALSE;
	}
	if (result[0] >= 65 && result[0] <=90){
		return TRUE;
	}else
		return FALSE;
}

static bool mhsplite(char * input,char * val,char *v_name)
{
	int i;
	char *pb=NULL;

	pb=strstr(input,"|");
	if (pb == NULL) {
		strcpy(val,input);
	}else {
		strncpy(val,input,pb-input);
		val[pb-input]='\0';
		strcpy(v_name,pb+1);
	}
		
	return TRUE;	
}

static int get_single_item(u_short magic){
	int i;
	int rc;
	
	for(i=0;i<512;i++){
		rc=strlen(MhVIP2Item[i].confile);
		if(rc<2) break;
		if(MhVIP2Item[i].var_list.magic == magic){
			//syslog(LOGOPTS, "[get_single_item] -- > confile = %s magic =%d \n",MhVIP2Item[i].confile,magic);
			return i;
		}
	}
	return -1;
}

#if 0
static int set_interger_item_value(u_short magic, long intval)
{
	int i;
	int rc;
	char buffer[32];
	char val[32];
	char v_name[32];
	
	for(i=0;i<512;i++){
		rc=strlen(MhVIP2Item[i].confile);
		if(rc<2) break;
		if(MhVIP2Item[i].var_list.magic == magic){
			sprintf(buffer,"%d",intval);
			if(validate_item_value(MhVIP2Item[i].var_validate.validate_type,buffer,1,i) == FALSE)
				return -2;
			if(MhVIP2Item[i].var_validate.argnum >0){
				mhsplite(MhVIP2Item[i].var_validate.args[intval],val,v_name);	
				strcpy(MhVIP2Item[i].val,val);
			}else {
				strcpy(MhVIP2Item[i].val,buffer);
			}
			set_option_by_section_name(ctx,MhVIP2Item[i].confile,MhVIP2Item[i].section, MhVIP2Item[i].item,MhVIP2Item[i].val);
			return i;
		}
	}
	return -1;
}

static int set_string_item_value(u_short magic, char *set_var,size_t var_val_len){
	int i;
	int rc;
	for(i=0;i<512;i++){
		rc=strlen(MhVIP2Item[i].confile);
		if(rc<2) break;
		if(MhVIP2Item[i].var_list.magic == magic){
			if(validate_item_value(MhVIP2Item[i].var_validate.validate_type,set_var,var_val_len,i) == FALSE)
				return -2;
			strcpy(MhVIP2Item[i].val,set_var);
			set_option_by_section_name(ctx,MhVIP2Item[i].confile,MhVIP2Item[i].section, MhVIP2Item[i].item,MhVIP2Item[i].val);
			return i;
		}
	}
	return -1;
}
#else
static int set_interger_item_value(u_short magic, long intval)
{
	int i;
	int rc;
	char buffer[32];
	char val[32];
	char v_name[32];

	
	i=get_single_item(magic);
	if(i<0) return -1; 
	if(MhVIP2Item[i].var_list.acl == RONLY) return -2;
	sprintf(buffer,"%d",intval);
	if(validate_item_value(MhVIP2Item[i].var_validate.validate_type,buffer,1,i) == FALSE)
		return -2;
	if(MhVIP2Item[i].var_validate.argnum >0){
		mhsplite(MhVIP2Item[i].var_validate.args[intval],val,v_name);	
		strcpy(MhVIP2Item[i].val,val);
	}else {
		strcpy(MhVIP2Item[i].val,buffer);
	}
	set_option_by_section_name(ctx,MhVIP2Item[i].confile,MhVIP2Item[i].section, MhVIP2Item[i].item,MhVIP2Item[i].val);
	return i;
}

static int set_string_item_value(u_short magic, char *set_var,size_t var_val_len)
{
	int i;
	int rc;

	i=get_single_item(magic);
	if(i<0) return -1; 
	if(MhVIP2Item[i].var_list.acl == RONLY) return -2;
	if(validate_item_value(MhVIP2Item[i].var_validate.validate_type,set_var,var_val_len,i) == FALSE)
		return -2;
	strcpy(MhVIP2Item[i].val,set_var);
	set_option_by_section_name(ctx,MhVIP2Item[i].confile,MhVIP2Item[i].section, MhVIP2Item[i].item,MhVIP2Item[i].val);
	return i;
}

#endif

static int get_single_item_value(u_short magic){
	int i,j;
	int rc;
	char buf[256];
	char val[32];
	char v_name[32];
	char tmpstatu_cmd[64];
	char tmpval[64];


	i=get_single_item(magic);
	if(i<0) return -1; 
	/*if request info need run scrip first and give the value from tmpstatus file*/
	memset(tmpval,0,sizeof(tmpval));
	strcpy(tmpval,MhVIP2Item[i].val);
	memset(MhVIP2Item[i].val,0,sizeof(MhVIP2Item[i].val));

	//syslog(LOGOPTS, "[get_single_item_value] -- > confile = %s \n",MhVIP2Item[i].confile);
	if(strcmp(MhVIP2Item[i].confile,"tmpstatus")== 0){
		strcpy(tmpstatu_cmd,"/etc/tmpstatus ");
		strcat(tmpstatu_cmd,MhVIP2Item[i].section);
		strcat(tmpstatu_cmd," ");
		strcat(tmpstatu_cmd,MhVIP2Item[i].item);
		//syslog(LOGOPTS, "[get_single_item_value] -- > cmd = %s \n",tmpstatu_cmd);
		SubProgram_Start("/bin/sh",tmpstatu_cmd);
	}
		
	get_option_by_section_name(ctx,MhVIP2Item[i].confile,MhVIP2Item[i].section, MhVIP2Item[i].item,buf);
	//syslog(LOGOPTS, "[get_single_item_value] -- > magic = %d get buf value =%s\n",MhVIP2Item[i].var_list.magic,buf);
	if(MhVIP2Item[i].var_list.type == ASN_INTEGER){
		if (MhVIP2Item[i].var_validate.argnum > 0){
			for(j=0;j<MhVIP2Item[i].var_validate.argnum;j++){
				mhsplite(MhVIP2Item[i].var_validate.args[j],val,v_name);
				//syslog(LOGOPTS, "[get_single_item_value] -- > magic = %d get value =%s get buf val =%s num=%d\n",MhVIP2Item[i].var_list.magic,val,buf,j);
				if(strcmp(buf,val)==0){
					sprintf(MhVIP2Item[i].val,"%d",j);
					return i;
				}
			}
		}
	}
	if(strlen(buf)<1){
		strcpy(MhVIP2Item[i].val,"N/A");
		//syslog(LOGOPTS, "[get_single_item_value] -- > magic = %d get empty value, will use default\n",MhVIP2Item[i].var_list.magic);
	}else {
		strcpy(MhVIP2Item[i].val,buf);
	}
	return i;
}

static bool build_mib_var()
{
	int i;

#if MhVIP2Item_DEBUG
	syslog(LOGOPTS, "The Build snmp mib variable array  the function var_VIP_MIB address 0x%x \n",var_VIP_MIB);
#endif

	// Build the first line of variable struct
	VIP_MIB_variables[0].magic = VIP_MIBSTRING;
	VIP_MIB_variables[0].type = ASN_OCTET_STR;
	VIP_MIB_variables[0].acl = RONLY;
	VIP_MIB_variables[0].findVar = (void *)var_VIP_MIB;
	VIP_MIB_variables[0].namelen = 1;
	VIP_MIB_variables[0].name[0] = 6000;
	VIP_MIB_variables[0].name[1] = 0;
	VIP_MIB_variables[0].name[2] = 0;
	VIP_MIB_variables[0].name[3] = 0;

	for(i=0;i<512;i++)
	{
		if(strlen(MhVIP2Item[i].confile) < 2)
			break;
		VIP_MIB_variables[i+1].magic = MhVIP2Item[i].var_list.magic;
		VIP_MIB_variables[i+1].type = MhVIP2Item[i].var_list.type;
		VIP_MIB_variables[i+1].acl = MhVIP2Item[i].var_list.acl;
		VIP_MIB_variables[i+1].findVar = (void *)var_VIP_MIB;
		VIP_MIB_variables[i+1].namelen = 4;
		VIP_MIB_variables[i+1].name[0] = MhVIP2Item[i].var_list.name[0];
		VIP_MIB_variables[i+1].name[1] = MhVIP2Item[i].var_list.name[1];
		VIP_MIB_variables[i+1].name[2] = MhVIP2Item[i].var_list.name[2];
		VIP_MIB_variables[i+1].name[3] = MhVIP2Item[i].var_list.name[3];
	}

#if MhVIP2Item_DEBUG
	//variable_dump();
#endif
}

#if 1
static bool validate_item_value(char * validate_type, char * set_var, size_t var_val_len,int linenum)
{
	bool retval=FALSE;
	
	if (strcmp(validate_type,"string")==0 || strcmp(validate_type,"none")==0){
		retval = TRUE;
	}else if (strcmp(validate_type,"int")==0){
		retval = digit_check(set_var,var_val_len,MhVIP2Item[linenum].var_validate.validate_min,MhVIP2Item[linenum].var_validate.validate_max);
		if (retval == FALSE){
			syslog(LOGOPTS, "validate_item_value type int return false\n");
		}
	}else if (strcmp(validate_type,"ip")==0){
		retval = IP_Address_check(set_var,var_val_len);
		if (retval == FALSE){
			syslog(LOGOPTS, "validate_item_value type ip return false\n");
		}
	}else if (strcmp(validate_type,"0")==0){
		if((atoi(set_var)<MhVIP2Item[linenum].var_validate.validate_min)||(atoi(set_var)>MhVIP2Item[linenum].var_validate.validate_max)){
			retval = FALSE;
			syslog(LOGOPTS, "validate_item_value type 0 return false\n");
		}else
			retval = TRUE;
	} else { 
		if(atoi(set_var)>=MhVIP2Item[linenum].var_validate.argnum){
			retval = FALSE;
			syslog(LOGOPTS, "validate_item_value type 0 return false\n");
		}else
			retval = TRUE;
	}

	return retval;

}
#endif
static bool init_item_group(char * filename)
{
	FILE *st_fd=NULL;
	char buf[256];
	int i, j,use_default=0,id_num;
	char *start, *end, *pos;
	char num[32];

	st_fd=fopen(filename,"rb");
	if(st_fd == NULL)
	{
		syslog(LOGOPTS, "can not open SNMP ITEM file %s\n",filename);
		return FALSE;
	}
	/*
	 * In SNMP_FILE the context will
	 * #This is comment
	 * confile;section;item;val;OID1;OID2;OID3;OID4;MagicID;type;access;
	 */
	i=0;
	memset(buf,0,sizeof(buf));
	while(fgets(buf,sizeof(buf),st_fd))
	{
		if( (strlen(buf)>2) && (buf[0] != '#') )
		{
			start=buf;
			end = strchr(start, ';');

			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get magic in %d format is not correct\n",filename,i);
				use_default =1;
				break;
			}
			memset(num,0,sizeof(num));
			strncpy(num, start,end-start);
			MhVIP2Item[i].var_list.magic = (u_short)atoi(num);

			start=end+1;
			end = strchr(start, ';');
			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get config file  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
				use_default =1;
				break;
			}
			strncpy(MhVIP2Item[i].confile, start,end-start);

			start=end+1;
			end = strchr(start, ';');
			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get section  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
				use_default =1;
				break;
			}
			strncpy(MhVIP2Item[i].section, start,end-start);

			start=end+1;
			end = strchr(start, ';');
			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get item  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
				use_default =1;
				break;
			}
			strncpy(MhVIP2Item[i].item, start,end-start);

			start=end+1;
			end = strchr(start, ';');
			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get item value in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
				use_default =1;
				break;
			}
			strncpy(MhVIP2Item[i].val, start,end-start);

			start=end+1;
			end = strchr(start, ';');
			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get OID1  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
				use_default =1;
				break;
			}
			memset(num,0,sizeof(num));
			strncpy(num, start,end-start);
			MhVIP2Item[i].OID0 = atoi(num);
			MhVIP2Item[i].var_list.name[0]=MhVIP2Item[i].OID0;

			start=end+1;
			end = strchr(start, ';');
			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get OID2 in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
				use_default =1;
				break;
			}
			memset(num,0,sizeof(num));
			strncpy(num, start,end-start);
			MhVIP2Item[i].OID1 = atoi(num);
			MhVIP2Item[i].var_list.name[1]=MhVIP2Item[i].OID1;

			start=end+1;
			end = strchr(start, ';');
			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get OID3  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
				use_default =1;
				break;
			}
			memset(num,0,sizeof(num));
			strncpy(num, start,end-start);
			MhVIP2Item[i].OID2 = atoi(num);
			MhVIP2Item[i].var_list.name[2]=MhVIP2Item[i].OID2;

			start=end+1;
			end = strchr(start, ';');
			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get OID4  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
				use_default =1;
				break;
			}
			memset(num,0,sizeof(num));
			strncpy(num, start,end-start);
			MhVIP2Item[i].OID3 = atoi(num);
			MhVIP2Item[i].var_list.name[3]=MhVIP2Item[i].OID3;

			start=end+1;
			end = strchr(start, ';');
			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get type  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
				use_default =1;
				break;
			}
			memset(num,0,sizeof(num));
			strncpy(num, start,end-start);
			if(strcmp(num,"int") == 0){
				MhVIP2Item[i].var_list.type = ASN_INTEGER;
			} else if(strcmp(num,"str") == 0){
				MhVIP2Item[i].var_list.type = ASN_OCTET_STR;
			}

			start=end+1;
			end = strchr(start, ';');
			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get access in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
				use_default =1;
				break;
			}
			memset(num,0,sizeof(num));
			strncpy(num, start,end-start);
			if(strcmp(num,"ro") == 0){
				MhVIP2Item[i].var_list.acl = RONLY  ;
			} else if(strcmp(num,"rw") == 0){
				MhVIP2Item[i].var_list.acl = RWRITE ;
			}

			start=end+1;
			end = strchr(start, ';');
			if(end == NULL) {
				syslog(LOGOPTS, "The file %s get validate  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
				use_default =1;
				break;
			}
			strncpy(MhVIP2Item[i].var_validate.validate_type,start,end-start);

			if(strcmp(MhVIP2Item[i].var_validate.validate_type,"none") != 0) {
				if(MhVIP2Item[i].var_list.type == ASN_INTEGER){
					/* if validate type = 0, continun to get min and max*/
					MhVIP2Item[i].var_validate.argnum=atoi(MhVIP2Item[i].var_validate.validate_type);
					if(MhVIP2Item[i].var_validate.argnum == 0){
						start=end+1;
						end = strchr(start, ';');
						if(end == NULL) {
							syslog(LOGOPTS, "The file %s get string int min value  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
							use_default =1;
							break;
						}
						memset(num,0,sizeof(num));
						strncpy(num, start,end-start);
						MhVIP2Item[i].var_validate.validate_min = atoi(num);

						start=end+1;
						end = strchr(start, ';');
						if(end == NULL) {
							syslog(LOGOPTS, "The file %s get string int max value  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
							use_default =1;
							break;
						}
						memset(num,0,sizeof(num));
						strncpy(num, start,end-start);
						MhVIP2Item[i].var_validate.validate_max = atoi(num);
						
					}/* if validate type great than 0, continun to loop the num to get strings*/
					else if(MhVIP2Item[i].var_validate.argnum> 0) {
						if(MhVIP2Item[i].var_validate.argnum >INTEGER_MAX_ARGS){
							syslog(LOGOPTS, "The file %s get int num  in %d exceed the max args\n",filename,MhVIP2Item[i].var_list.magic);
						}else {					
							for(j=0;j<MhVIP2Item[i].var_validate.argnum;j++){
								start=end+1;
								end = strchr(start, ';');
								if(end == NULL) {
									syslog(LOGOPTS, "The file %s get int num file  in magic = %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
									use_default =1;
									break;
								}
								strncpy(MhVIP2Item[i].var_validate.args[j], start,end-start);
								//syslog(LOGOPTS, "debug --->i=%d args %d =[%s] \n",i,j,MhVIP2Item[i].var_validate.args[j]);
							}
						}
					}else{
						syslog(LOGOPTS, "The file %s get int num  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
					}
					
					
				} else if(MhVIP2Item[i].var_list.type == ASN_OCTET_STR){
					/*if it needs validate to int  get min and max value*/	
					if(strcmp(MhVIP2Item[i].var_validate.validate_type,"int")== 0){
						start=end+1;
						end = strchr(start, ';');
						if(end == NULL) {
							syslog(LOGOPTS, "The file %s get string int min value  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
							use_default =1;
							break;
						}
						memset(num,0,sizeof(num));
						strncpy(num, start,end-start);
						MhVIP2Item[i].var_validate.validate_min = atoi(num);

						start=end+1;
						end = strchr(start, ';');
						if(end == NULL) {
							syslog(LOGOPTS, "The file %s get string int max value  in %d format is not correct\n",filename,MhVIP2Item[i].var_list.magic);
							use_default =1;
							break;
						}
						memset(num,0,sizeof(num));
						strncpy(num, start,end-start);
						MhVIP2Item[i].var_validate.validate_max = atoi(num);

					}

				}
			}
			MhVIP2Item[i].var_list.findVar = var_VIP_MIB ;
			MhVIP2Item[i].var_list.namelen = 4 ;
			i++;
		}
		memset(buf,0,sizeof(buf));
	}
	fclose(st_fd);

	strcpy(MhVIP2Item[i].confile," ");
	strcpy(MhVIP2Item[i].section," ");
	strcpy(MhVIP2Item[i].item," ");

	if(use_default==1){
		return FALSE;
	}

	arrary_num = i;
#if MhVIP2Item_DEBUG
	MhVIP2Item_validate_dump();

	//MhVIP2Item_dump();
#endif

#if read_MIB
    struct tree    *tp;
    struct tree    *tree_head = NULL;
    netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIB_WARNINGS, 2);

    init_mib();

    syslog(LOGOPTS, "parse MIB file now\n");

	read_mib("/www/Microhard-VIP4G.MIB");
	syslog(LOGOPTS, "parse MIB file read done\n");

    for (tp = tree_head; tp; tp = tp->next_peer)
    {
		syslog(LOGOPTS, "parse MIB file label[%s] subid [%ld]\n", tp->label, tp->subid);
 //       print_subtree(stdout, tp, 0);
    }
	syslog(LOGOPTS, "parse MIB file done\n");
        //    free_tree(tree_head);
#endif
	return TRUE;
}

/**********************************************************************************
                               END THANK YOU
**********************************************************************************/























