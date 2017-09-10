/* dk_ver.h - macros and definitions for memory management */

/*
 *  Copyright © 2000-2001 Atheros Communications, Inc.,  All Rights Reserved.
 */


#ifndef __INCdk_verh
#define __INCdk_verh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ART_VERSION_MAJOR 5
#ifdef __ATH_DJGPPDOS__
#define ART_VERSION_MINOR 3
#define ART_BUILD_NUM     100
#else
#define ART_VERSION_MINOR 4
#define ART_BUILD_NUM     7
#endif
#define TREAT_BUILD_NUM_HEX 0x0

#define MAUIDK_VER1 ("\n   --- Atheros: MDK (multi-device version) ---\n")

#define MAUIDK_VER_SUB ("              - Revision 2.0.3 ")
#ifdef CUSTOMER_REL
#define MAUIDK_VER2  ("              - Revision 5.4 BUILD #7 ")
#ifdef ANWI
#define MAUIDK_VER3 ("\n            - Customer Version (ANWI BUILD)-\n")
#else
#define MAUIDK_VER3 ("\n            - Customer Version -\n")
#endif //ANWI
#else
#define MAUIDK_VER2 ("              - Revision 5.4 BUILD #7")
#define MAUIDK_VER3 ("\n      --- Atheros INTERNAL USE ONLY ---\n")
#endif

#ifdef __ATH_DJGPPDOS__
#define DEVLIB_VER1 ("Devlib Revision 5.3 BUILD #100\n")
#else
#define DEVLIB_VER1 ("Devlib Revision 5.4 BUILD #7\n")
#endif
#ifdef ART
#define MDK_CLIENT_VER1 ("\n   --- Atheros: ART Client (multi-device version) ---\n")
#else
#define MDK_CLIENT_VER1 ("\n   --- Atheros: MDK Client (multi-device version) ---\n")
#endif
#ifdef __ATH_DJGPPDOS__
#define MDK_CLIENT_VER2 ("         - Revision 5.3 BUILD #100 -\n")
#else
#define MDK_CLIENT_VER2 ("         - Revision 5.4 BUILD #7 -\n")
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCdk_verh */
