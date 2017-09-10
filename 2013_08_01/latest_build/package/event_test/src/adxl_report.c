/*
 *
 *  Copyright (c) 1999-2000 Vojtech Pavlik
 *
 *  As modified by Rene van Paassen, May 19, 2005
 *  Event device test program
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <vojtech@ucw.cz>, or by paper mail:
 * Vojtech Pavlik, Ucitelska 1576, Prague 8, 182 00 Czech Republic
 */

#include <linux/input.h>
#include <linux/version.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <pthread.h>
#include <time.h>
#include <syslog.h>
#include <signal.h>
 
#include "uci.h"

//#define PRINT_DEBUG_INFO 1

#define	LOGOPTS		(LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)



char *events[EV_MAX + 1] = { "Reset", "Key", "Relative", "Absolute", "MSC", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, "LED", "Sound", NULL, "Repeat", "ForceFeedback", "Power", "ForceFeedbackStatus"};

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 0)

/* Keys updated with version 2.6.11, am assuming this is 2.6 style */
char *keys[KEY_MAX + 1] = 
  { "Reserved", "Esc", /* 0, 1 */
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", /* 2 - 11 */
    "Minus", "Equal", "Backspace", "Tab", /* 12 - 15 */
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", /* 16 - 25 */
    "LeftBrace", "RightBrace", "Enter", "LeftControl", /* 26 - 29 */
    "A", "S", "D", "F", "G", "H", "J", "K", "L", /* 30 - 38 */
    "Semicolon", "Apostrophe", "Grave", "LeftShift", "BackSlash", /* 39 - 43 */
    "Z", "X", "C", "V", "B", "N", "M",  /* 44 - 50 */
    "Comma", "Dot", "Slash", "RightShift", "KPAsterisk", 
    "LeftAlt", "Space", "CapsLock",  /* 51 - 58 */
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", /* 59 - 68 */
    "NumLock", "ScrollLock", /* 69, 70 */
    "KP7", "KP8", "KP9", "KPMinus", /* 71 - 74 */
    "KP4", "KP5", "KP6", "KPPlus", /* 75 - 78 */
    "KP1", "KP2", "KP3", "KP0", "KPDot", /* 79 - 83 */
    NULL, 
    "ZENKAKUHANKAKU", /* 85 */
    "102nd", "F11", "F12", /* 86 - 88 */
    "R0", "KATAKANA", "HIRAGANA", "HENKAN", /* 89 - 92 */
    "KATAKANAHIRAGANA", "MUHENKAN", "KPJPCOMMA", /* 93 - 95 */
    "KPEnter", "RightCtrl", "KPSlash", "SysRq", /* 96 - 99 */
    "RightAlt", "LineFeed", /*100 - 101 */
    "Home", "Up", "PageUp", "Left", "Right", "End", "Down", 
    "PageDown", "Insert", "Delete", /* 102 - 111 */
    "Macro", "Mute", "VolumeDown", "VolumeUp", "Power", /* 112 - 116 */
    "KPEqual", "KPPlusMinus", "Pause", /* 117 - 119 */
    NULL, 
    "KPComma", /* 121 */
    "Hanguel", "Hanja", "Yen", /* 122 - 124 */
    "LeftMeta", "RightMeta", "Compose", /* 125 - 127 */
    "Stop", "Again", "Props", "Undo", "Front", "Copy", "Open", 
    "Paste", "Find", "Cut", "Help", "Menu", "Calc", "Setup",
    "Sleep", "WakeUp", "File", "SendFile", "DeleteFile", "X-fer", 
    "Prog1", "Prog2", "WWW", "MSDOS", "Coffee", "Direction",
    "CycleWindows", "Mail", "Bookmarks", "Computer", 
    "Back", "Forward", "CloseCD", "EjectCD", "EjectCloseCD", 
    "NextSong", "PlayPause", "PreviousSong", "StopCD", 
    "Record", "Rewind", "Phone", /* 128 - 169 */
    "ISOKey", "Config", "HomePage", "Refresh", "Exit", 
    "Move", "Edit", "ScrollUp", "ScrollDown", /* 170 - 178 */
    "KPLeftParenthesis", "KPRightParenthesis", /* 179 - 180 */
    NULL, NULL,
    "F13", "F14", "F15", "F16", "F17", "F18", 
    "F19", "F20", "F21", "F22", "F23", "F24", /* 183 - 194 */
    NULL, NULL, NULL, NULL, NULL,
    "PlayCD", "PauseCD", "Prog3", "Prog4", NULL, 
    "Suspend", "Close", /* 200 - 206 */
    "Play", "FastForward", "BassBoost", "Print", "HP", "Camera", 
    "Sound", "Question", "Email", "Chat", "Search", 
    "Connect", "Finance", "Sport", "Shop", "AltErase", 
    "Cancel", "BrightnessDown", "BrightnessUp", "Media", /* 207 - 226 */
    "SwitchVideoMode", 
    "KBDIllumToggle", "KBDIllumDown", "KBDIllumUp", /* 227 - 230 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "Unknown", /* 240 */ 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "Btn0", "Btn1", "Btn2", "Btn3", "Btn4", 
    "Btn5", "Btn6", "Btn7", "Btn8", "Btn9", /* 256 - 265 */
    NULL, NULL, NULL, NULL, NULL, NULL,
    "LeftBtn", "RightBtn", "MiddleBtn", "SideBtn", 
    "ExtraBtn", "ForwardBtn", "BackBtn", "TaskBtn", /* 0x110 - 0x117 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "Trigger", "ThumbBtn", "ThumbBtn2", "TopBtn", 
    "TopBtn2", "PinkieBtn", "BaseBtn", "BaseBtn2", 
    "BaseBtn3", "BaseBtn4", "BaseBtn5", "BaseBtn6",
    NULL, NULL, NULL, "BtnDead", /* 0x120 - 0x12f */
    "BtnA", "BtnB", "BtnC", "BtnX", 
    "BtnY", "BtnZ", "BtnTL", "BtnTR", 
    "BtnTL2", "BtnTR2", "BtnSelect", "BtnStart", 
    "BtnMode", "BtnThumbL", "BtnThumbR", /* 0x130 - 0x13e */ NULL,
    "ToolPen", "ToolRubber", "ToolBrush", "ToolPencil", 
    "ToolAirbrush", "ToolFinger", "ToolMouse", "ToolLens", /* 0x140-0x147 */
    NULL, NULL, "Touch", "Stylus", 
    "Stylus2", "DoubleTap", "TripleTap", /* 0x14a - 0x14e */ NULL,
    "GearUp", "GearDown", /* 0x150, 0x151 */
    NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "Ok", "Select", "Goto", "Clear", "Power2", "Option", "Info", "Time",
    "Vendor", "Archive", "Program", "Channel",
    "Favorites", "Epg", "Pvr", "Mhp", 
    "Language", "Title", "Subtitle", "Angle",
    "Zoom", "Mode", "Keyboard", "Screen",
    "Pc", "Tv", "Tv2", "Vcr", "Vcr2", "Sat", "Sat2", "Cd",
    "Tape", "Radio", "Tuner", "Player", "Text", "Dvd", "Aux", "Mp3",
    "Audio", "Video", "Directory", "List",
    "Memo", "Calendar", "Red", "Green", 
    "Yellow", "Blue", "Channelup", "Channeldown",
    "First", "Last", "Ab", "Next", 
    "Restart", "Slow", "Shuffle", "Break", 
    "Previous", "Digits", "Teen", "Twen", /* 160 - 19f */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "DelEol", "DelEos", "InsLine", "DelLine", /* 1c0 - 1c3 */
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "FN", "FN_ESC", 
    "FN_F1", "FN_F2", "FN_F3", "FN_F4", "FN_F5", 
    "FN_F6", "FN_F7", "FN_F8", "FN_F9", "FN_F10", 
    "FN_F11", "FN_F12", 
    "FN_1", "FN_2", "FN_D", "FN_E", "FN_F", "FN_S", "FN_B" /* 0x1d0 - 0x1e4 */
  };

#else

/* Keys I used with 2.4 kernels. */
char *keys[KEY_MAX + 1] = { "Reserved", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Minus", "Equal", "Backspace",
"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "LeftBrace", "RightBrace", "Enter", "LeftControl", "A", "S", "D", "F", "G",
"H", "J", "K", "L", "Semicolon", "Apostrophe", "Grave", "LeftShift", "BackSlash", "Z", "X", "C", "V", "B", "N", "M", "Comma", "Dot",
"Slash", "RightShift", "KPAsterisk", "LeftAlt", "Space", "CapsLock", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
"NumLock", "ScrollLock", "KP7", "KP8", "KP9", "KPMinus", "KP4", "KP5", "KP6", "KPPlus", "KP1", "KP2", "KP3", "KP0", "KPDot", "103rd",
"F13", "102nd", "F11", "F12", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "KPEnter", "RightCtrl", "KPSlash", "SysRq",
"RightAlt", "LineFeed", "Home", "Up", "PageUp", "Left", "Right", "End", "Down", "PageDown", "Insert", "Delete", "Macro", "Mute",
"VolumeDown", "VolumeUp", "Power", "KPEqual", "KPPlusMinus", "Pause", "F21", "F22", "F23", "F24", "KPComma", "LeftMeta", "RightMeta",
"Compose", "Stop", "Again", "Props", "Undo", "Front", "Copy", "Open", "Paste", "Find", "Cut", "Help", "Menu", "Calc", "Setup",
"Sleep", "WakeUp", "File", "SendFile", "DeleteFile", "X-fer", "Prog1", "Prog2", "WWW", "MSDOS", "Coffee", "Direction",
"CycleWindows", "Mail", "Bookmarks", "Computer", "Back", "Forward", "CloseCD", "EjectCD", "EjectCloseCD", "NextSong", "PlayPause",
"PreviousSong", "StopCD", "Record", "Rewind", "Phone", "ISOKey", "Config", "HomePage", "Refresh", "Exit", "Move", "Edit", "ScrollUp",
"ScrollDown", "KPLeftParenthesis", "KPRightParenthesis",
"International1", "International2", "International3", "International4", "International5",
"International6", "International7", "International8", "International9",
"Language1", "Language2", "Language3", "Language4", "Language5", "Language6", "Language7", "Language8", "Language9",
NULL, 
"PlayCD", "PauseCD", "Prog3", "Prog4", "Suspend", "Close",
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
"Btn0", "Btn1", "Btn2", "Btn3", "Btn4", "Btn5", "Btn6", "Btn7", "Btn8", "Btn9",
NULL, NULL,  NULL, NULL, NULL, NULL,
"LeftBtn", "RightBtn", "MiddleBtn", "SideBtn", "ExtraBtn", "ForwardBtn", "BackBtn",
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
"Trigger", "ThumbBtn", "ThumbBtn2", "TopBtn", "TopBtn2", "PinkieBtn",
"BaseBtn", "BaseBtn2", "BaseBtn3", "BaseBtn4", "BaseBtn5", "BaseBtn6",
NULL, NULL, NULL, "BtnDead",
"BtnA", "BtnB", "BtnC", "BtnX", "BtnY", "BtnZ", "BtnTL", "BtnTR", "BtnTL2", "BtnTR2", "BtnSelect", "BtnStart", "BtnMode",
"BtnThumbL", "BtnThumbR", NULL,
"ToolPen", "ToolRubber", "ToolBrush", "ToolPencil", "ToolAirbrush", "ToolFinger", "ToolMouse", "ToolLens", NULL, NULL,
"Touch", "Stylus", "Stylus2" };
#endif

char *absval[5] = { "Value", "Min  ", "Max  ", "Fuzz ", "Flat " };
char *relatives[REL_MAX + 1] = 
  { "X", "Y", "Z", NULL, 
    NULL, NULL, "HWheel", "Dial", 
    /* 0x08: */ "Wheel", "MISC" };
char *absolutes[ABS_MAX + 1] = 
  { "X", "Y", "Z", "Rx",  "Ry", "Rz", "Throttle", "Rudder", 
    "Wheel", "Gas", "Brake", NULL, NULL, NULL, NULL, NULL,
    /* 0x10: */ "Hat0X", "Hat0Y", "Hat1X", "Hat1Y", 
    "Hat2X", "Hat2Y", "Hat3X", "Hat 3Y", 
    "Pressure", "Distance", "XTilt", "YTilt", 
    "ToolWidth", NULL, NULL, NULL, 
    /* 0x20: */ "Volume", NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, 
    /* 0x28: */ "Misc" };
char *leds[LED_MAX + 1] = { "NumLock", "CapsLock", "ScrollLock", "Compose", "Kana", "Sleep", "Suspend", "Mute" };
char *repeats[REP_MAX + 1] = { "Delay", "Period" };
char *sounds[SND_MAX + 1] = { "Bell", "Click" };

char **names[EV_MAX + 1] = { events, keys, relatives, absolutes, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, leds, sounds, NULL, repeats, NULL, NULL, NULL };

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)

/**********************************************************************************
    adxl         
**********************************************************************************/

/* Global variable */

/*
 *	Signal handling.
 */
struct sigaction	sact;

struct uci_context *ctx = NULL;

// Device Serial Number
char MHX_SERIAL[32];

char send_buf[5120];
 
/* current adxl status, updated by low level adxl interrupt */ 
struct input_event current_adxl_status; 

pthread_t tid;
void      *tret;
pthread_mutex_t adxl_status_mutex = PTHREAD_MUTEX_INITIALIZER;


/* adxl report config file  */
static char *adxlr_confile="/etc/config/adxl";

/* adxl report config data, read from above config file */
struct adxlr_conf_data_t
{
    char adxlr_status[8];
    char adxlr_mode[8];
    char adxlr_type[8];    
    char adxlr_ip[20];
    char adxlr_port[10];
    char adxlr_timer[10];
    char adxlr_message1[8];
    char adxlr_message2[8];
    char adxlr_message3[8];
    char adxlr_message4[8];
};
struct adxlr_conf_data_t adxlr_conf;


/* adxl report data structure */
struct adxlr_def_t
{
    uint16_t mess_type_mask;
    uint8_t mac[6];
    uint16_t product_flage;
    uint16_t pack_len;

    int sockfd;
    struct sockaddr_in addr_remote;    // remote host address information
    int state;
    int scheduled;
    uint16_t interval_tm;
    uint16_t min_interval;
    time_t last_action;
};
struct adxlr_def_t adxlr_buff;



/*
    Function prototype
*/



/**********************************************************************************
   Function:    int main(void) 
   Input:       void
   Output:      None
   Return:      int   
   Note:	read database and config com port and according to selected IP protocol 
        to process.        
**********************************************************************************/

static bool get_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
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

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK)
        {

        e = ptr.last;

        if ( (ptr.flags & UCI_LOOKUP_COMPLETE) &&
             (e->type == UCI_TYPE_OPTION) )
            {
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
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

static bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
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

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK)
        {
        if ( UCI_OK == uci_set(ctx, &ptr) )
            {
            uci_save(ctx, ptr.p);
         //   uci_commit(ctx, &ptr.p, false);
            }

        free(tuple);
        return true;
        }

    free(tuple);

    return false;
}


static int scpytrim(char *dst, const char *src, int len)
{
    char *d = dst;
    const char *s = src;
    int i, j = 0;

    for (i=0; i<len; i++) {
        if (s[i]=='\r' || s[i]=='\n' || s[i]=='\0' || s[i]==' ') {
            continue;
        }
        d[j] = s[i];
        j++;
    }
    return j;
}



void check_serial_no()
{
  FILE *fp;
  int status;
//  char MHX_SERIAL[32];
//  char tmpbuf[128];

  /* Open the command for reading. */
//  fp = popen("grep params /proc/mtd | cut -d: -f1", "r");
//  if (fp == NULL) {
//    printf("Failed to run command\n" );
//    exit;
//  }
//
//  /* Read the output a line at a time - output it. */
//  while (fgets(path, sizeof(path)-1, fp) != NULL) {
//    printf("%s", path);
//  }
//
//  sprintf(tmpbuf,"grep 'MHX_SERIAL' /dev/%s | cut -d= -f2", path);
//  fp = popen(tmpbuf, "r");

  fp = popen("grep 'MHX_SERIAL' /dev/mtd1 | cut -d= -f2", "r");  
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit;
  }
  while (fgets(MHX_SERIAL, sizeof(MHX_SERIAL)-1, fp) != NULL) {
    printf("Device Serial No.:%s \n", MHX_SERIAL);
  }

  /* close */
  pclose(fp);

}


static int read_adxl_config()
{

    //struct uci_context *ctx = NULL;
    if (ctx)
      {
        uci_free_context(ctx);
        ctx=NULL;
      }
    ctx = uci_alloc_context();  
    if (!ctx) 
    {
        fprintf(stderr, "uci_alloc_context Out of memory.\n");
        return -1;
    }    
    if( get_option_by_section_name(ctx, adxlr_confile, "adxl_report_conf", "ADXL_Remote_Reporting_Status", adxlr_conf.adxlr_status) == false ){
        printf("can not get ADXL_Remote_Reporting_Status from %s\n", adxlr_confile);
        if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
        }
        return -1;
    }else{
#ifdef PRINT_DEBUG_INFO
        printf("ADXL Report ADXL_Remote_Reporting_Status %s\n", adxlr_conf.adxlr_status);
#endif                
    }
    ////////////////////////////////////////////////////////////
    if (ctx)
      {
        uci_free_context(ctx);
        ctx=NULL;
      }
    ctx = uci_alloc_context();  
    if (!ctx) 
    {
        fprintf(stderr, "uci_alloc_context Out of memory.\n");
        return -1;
    }
    if( get_option_by_section_name(ctx, adxlr_confile, "adxl_report_conf", "ADXL_Remote_Reporting_Mode",  adxlr_conf.adxlr_mode) == false ){
        printf("can not get %s from %s\n", "ADXL_Remote_Reporting_Mode", adxlr_confile);
        if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
        }
        return -1;
    }else{
#ifdef PRINT_DEBUG_INFO
        printf("ADXL Report %s : %s\n", "ADXL_Remote_Reporting_Mode", adxlr_conf.adxlr_mode);
#endif                
    }
    if (ctx)
      {
        uci_free_context(ctx);
        ctx=NULL;
      }
    ////////////////////////////////////////////////////////////
    ctx = uci_alloc_context();  
    if (!ctx) 
    {
        fprintf(stderr, "uci_alloc_context Out of memory.\n");
        return -1;
    }
    if( get_option_by_section_name(ctx, adxlr_confile, "adxl_report_conf", "ADXL_Remote_Reporting_Type",  adxlr_conf.adxlr_type) == false ){
        printf("can not get %s from %s\n", "ADXL_Remote_Reporting_Type", adxlr_confile);
        if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
        }
        return -1;
    }else{
#ifdef PRINT_DEBUG_INFO
        printf("ADXL Report %s : %s\n", "ADXL_Remote_Reporting_Type", adxlr_conf.adxlr_type); 
#endif               
    }
    if (ctx)
      {
        uci_free_context(ctx);
        ctx=NULL;
      }
    ////////////////////////////////////////////////////////////
    ctx = uci_alloc_context();  
    if (!ctx) 
    {
        fprintf(stderr, "uci_alloc_context Out of memory.\n");
        return -1;
    }
    if( get_option_by_section_name(ctx, adxlr_confile, "adxl_report_conf", "ADXL_Remote_Reporting_IP",  adxlr_conf.adxlr_ip) == false ){
        printf("can not get %s from %s\n", "ADXL_Remote_Reporting_IP", adxlr_confile);
        if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
        }
        return -1;
    }else{
#ifdef PRINT_DEBUG_INFO
        printf("ADXL Report %s : %s\n", "ADXL_Remote_Reporting_IP", adxlr_conf.adxlr_ip); 
#endif               
    }
    if (ctx)
      {
        uci_free_context(ctx);
        ctx=NULL;
      }
    ////////////////////////////////////////////////////////////
    ctx = uci_alloc_context();  
    if (!ctx) 
    {
        fprintf(stderr, "uci_alloc_context Out of memory.\n");
        return -1;
    }
    if( get_option_by_section_name(ctx, adxlr_confile, "adxl_report_conf", "ADXL_Remote_Reporting_PORT",  adxlr_conf.adxlr_port) == false ){
        printf("can not get %s from %s\n", "ADXL_Remote_Reporting_PORT", adxlr_confile);
        if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
        }
        return -1;
    }else{
#ifdef PRINT_DEBUG_INFO
        printf("ADXL Report %s : %s\n", "ADXL_Remote_Reporting_PORT", adxlr_conf.adxlr_port); 
#endif               
    }
    if (ctx)
      {
        uci_free_context(ctx);
        ctx=NULL;
      }
    ////////////////////////////////////////////////////////////
    ctx = uci_alloc_context();  
    if (!ctx) 
    {
        fprintf(stderr, "uci_alloc_context Out of memory.\n");
        return -1;
    }
    if( get_option_by_section_name(ctx, adxlr_confile, "adxl_report_conf", "ADXL_Remote_Reporting_Timer",  adxlr_conf.adxlr_timer) == false ){
        printf("can not get %s from %s\n", "ADXL_Remote_Reporting_Timer", adxlr_confile);
        if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
        }
        return -1;
    }else{
#ifdef PRINT_DEBUG_INFO
        printf("ADXL Report %s : %s\n", "ADXL_Remote_Reporting_Timer", adxlr_conf.adxlr_timer);
#endif                
    }    
    if (ctx)
    {
        uci_free_context(ctx);
        ctx=NULL;
     }
    ////////////////////////////////////////////////////////////
    ctx = uci_alloc_context();  
    if (!ctx) 
    {
        fprintf(stderr, "uci_alloc_context Out of memory.\n");
        return -1;
    }
    if( get_option_by_section_name(ctx, adxlr_confile, "adxl_report_conf", "ADXL_Remote_Reporting_Message1",  adxlr_conf.adxlr_message1) == false ){
        printf("can not get %s from %s\n", "ADXL_Remote_Reporting_Message1", adxlr_confile);
        if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
        }
        return -1;
    }else{
#ifdef PRINT_DEBUG_INFO
        printf("ADXL Report %s : %s\n", "ADXL_Remote_Reporting_Message1", adxlr_conf.adxlr_message1);
#endif                
    }    
    if (ctx)
    {
        uci_free_context(ctx);
        ctx=NULL;
     }
    ////////////////////////////////////////////////////////////
    ctx = uci_alloc_context();  
    if (!ctx) 
    {
        fprintf(stderr, "uci_alloc_context Out of memory.\n");
        return -1;
    }
    if( get_option_by_section_name(ctx, adxlr_confile, "adxl_report_conf", "ADXL_Remote_Reporting_Message2",  adxlr_conf.adxlr_message2) == false ){
        printf("can not get %s from %s\n", "ADXL_Remote_Reporting_Message2", adxlr_confile);
        if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
        }
        return -1;
    }else{
#ifdef PRINT_DEBUG_INFO
        printf("ADXL Report %s : %s\n", "ADXL_Remote_Reporting_Message2", adxlr_conf.adxlr_message2);
#endif                
    }    
    if (ctx)
    {
        uci_free_context(ctx);
        ctx=NULL;
     }
    ////////////////////////////////////////////////////////////
    ctx = uci_alloc_context();  
    if (!ctx) 
    {
        fprintf(stderr, "uci_alloc_context Out of memory.\n");
        return -1;
    }
    if( get_option_by_section_name(ctx, adxlr_confile, "adxl_report_conf", "ADXL_Remote_Reporting_Message3",  adxlr_conf.adxlr_message3) == false ){
        printf("can not get %s from %s\n", "ADXL_Remote_Reporting_Message3", adxlr_confile);
        if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
        }
        return -1;
    }else{
#ifdef PRINT_DEBUG_INFO
        printf("ADXL Report %s : %s\n", "ADXL_Remote_Reporting_Message3", adxlr_conf.adxlr_message3);
#endif                
    }    
    if (ctx)
    {
        uci_free_context(ctx);
        ctx=NULL;
     }
    ////////////////////////////////////////////////////////////
    ctx = uci_alloc_context();  
    if (!ctx) 
    {
        fprintf(stderr, "uci_alloc_context Out of memory.\n");
        return -1;
    }
    if( get_option_by_section_name(ctx, adxlr_confile, "adxl_report_conf", "ADXL_Remote_Reporting_Message4",  adxlr_conf.adxlr_message4) == false ){
        printf("can not get %s from %s\n", "ADXL_Remote_Reporting_Message4", adxlr_confile);
        if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
        }
        return -1;
    }else{
#ifdef PRINT_DEBUG_INFO
        printf("ADXL Report %s : %s\n", "ADXL_Remote_Reporting_Message4", adxlr_conf.adxlr_message4);
#endif                
    }    
    if (ctx)
    {
        uci_free_context(ctx);
        ctx=NULL;
     }
    ////////////////////////////////////////////////////////////
    printf("=========================================================\n");   
}


static int init_adxl_socket()
{
    int rport;

    if ((adxlr_buff.sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
    {
        printf("ADXL_REPORT [%s]: Failed to obtain remote Socket Descriptor for report!\n", __FUNCTION__);
        return -1;
    }

    rport=atoi(adxlr_conf.adxlr_port);

    /* Fill the socket address struct */
    adxlr_buff.addr_remote.sin_family = AF_INET;                   // Protocol Family
    adxlr_buff.addr_remote.sin_port = htons(rport);                 // Port number
    inet_pton(AF_INET, adxlr_conf.adxlr_ip, &(adxlr_buff.addr_remote.sin_addr)); // Net Address
    bzero(&(adxlr_buff.addr_remote.sin_zero), 8);                  // Flush the rest of struct
}


/*
 * 
 */
static void init_adxl_report()
{
    read_adxl_config();
    init_adxl_socket();
}


void DEBUG_PRINT_SEND_BUFFER()
{
    int i = 0;
    //printf("\n \n \n Now begin send %d Bytes:\n", adxlr_buff.pack_len);
    printf("\n");
    for(i=0; i<adxlr_buff.pack_len; i++) {
        printf("%c", send_buf[i]);
    }
    printf("\n\n\n");
}


/*
 *  Fill json header
 *     
 *     
 *      
 *  
 */
static int fill_json_header(char *buf)
{
    char *pos = buf;
    int length = 0;
    int temp=0;
    time_t timestamp;
    char tmpbuf[256];

    /* header information */
    /*
     * protocol and version
     */
    strcpy(buf,"{\"header\":{\"protocol\":\"UDPReport\",\"version\":1.0,");
    length += strlen("{\"header\":{\"protocol\":\"UDPReport\",\"version\":1.0,");
    pos += length;

    /*
     * timestamp
     */
    timestamp = time (NULL);
    sprintf(tmpbuf,"\"timestamp\":%ld},",timestamp);
    //sprintf(tmpbuf,"\"timestamp\":%ld},\"adxl.input_event\":[{\"type\":%d,\"code\":%d,\"value\":%d}]}", timestamp, current_adxl_status.type,current_adxl_status.code,current_adxl_status.value);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += length;


    return length;
}




/*
 * fill json adxl payload
 */
static int fill_json_adxl_playload(char *buf)
{
    char *pos = buf;
    int length = 0;
    int temp = 0;
//    int i;
    char tmpbuf[256];
    //printf("ADXL_REPORT [%s] current_axdl_status:[%d %d %d]\n", __FUNCTION__, current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);

    sprintf(tmpbuf,"{\"Accelerometer\":");
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += temp;

    // 
    if (current_adxl_status.type == 1) {
        switch(current_adxl_status.code) {
            case 30:
                if (current_adxl_status.value == 1) {
                   sprintf(tmpbuf,"\"Activity\"}");
                }else if (current_adxl_status.value == 0) {
                   sprintf(tmpbuf,"\"Inactivity\"}");
                }
                break;
            case 330:
                sprintf(tmpbuf,"\"Impact\"}");
                break;
            default:
                printf("ADXL_REPORT [%s] current_axdl_status:[%d %d %d]\n", __FUNCTION__, current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);
                break;
        }
        temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
        length += temp;
        pos += temp;

    }

    return length;
}




static int fill_json_tail(char *buf)
{
    char *pos = buf;
    int length = 0;
    int temp = 0;
    char tmpbuf[512];


    sprintf(tmpbuf,"}");
    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    return length;
}


/*
 * Packing adxl report - binary 
 *      A: input_event.type
 *      BBB: input_event.code
 *      C: input_event.value 
 *  
 */
static int fill_binary_package(char *buffer_send)
{
    int sendlen = 0;
    int i = 0, j = 0;
    char tmpbuf[256];
#ifdef PRINT_DEBUG_INFO
    printf("ADXL_REPORT [%s] current_axdl_status:[%d %d %d]\n", __FUNCTION__, current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);
#endif
    sprintf(tmpbuf,"%d%03d%d", current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);
    j = strlen(tmpbuf);
    //printf("ADXL_REPORT [%s] tmpbuf[%d]\n", __FUNCTION__, j); 
    for (i=0 ; i < j; i++) {
        buffer_send[sendlen+i] = tmpbuf[i];
    }    
    sendlen += j;

    return sendlen;
}



/*
 * TAIP 
 *  
 * Message Format: ABB{C}[;ID=DDDD][;*FF]< 
 *  
 *  
 * C.1.1 Start of a New Message: 
 *      The > character (ASCII code 62 decimal) is used to specify the start of a new sentence.
 *  
 * C.1.2 Message Qualifier (one character)
 *      R: Response to a query or a scheduled report (from the receiver)
 *  
 * C.1.3 Message Identifier (two-character) 
 *      Unique two-character message identifier is used to identify type messages. For VIP4G: AC for Accelerometer Report *
 *  
 * C.1.4 Data String 
 *      The data string can contain any printable ASCII character with the exception of the >, <, and ; characters.
 *  
 *      For VIP4G Accelerometer Application:
 *      AC data string format: ABBBC
 *  
 *      A:      input_event.type
 *      BBB:    input_event.code
 *      C:      input_event.value
 *  
 * C.1.5 Vehicle ID  (Optional 4-character) 
 *      MHX_SERIAL
 *  
 * C.1.6 Checksum  (Optional 2-character)
 *  
 * C.1.7 Message Delimiter 
 *      The < character signifies end of a sentence and is used as the message delimiter. 
 *  
 *  
 *  
 */

/*
 * Packing adxl report - TAIP
 */
static int fill_taip_package(char *buffer_send)
{
    int sendlen = 0;
    int ret = 0;
    int i = 0, j = 0;
    char tmpbuf[256];
#ifdef PRINT_DEBUG_INFO
    printf("ADXL_REPORT [%s] current_axdl_status:[%d %d %d]\n", __FUNCTION__, current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);
#endif
    // C.1.1 Start of a New Message
    buffer_send[0] = '>';

    // C.1.2 Message Qualifier (one character)
    buffer_send[1] = 'R';
     
    // C.1.3 Message Identifier (two-character)
    buffer_send[2] = 'A';
    buffer_send[3] = 'C';
    sendlen = 4;
     
    // C.1.4 Data String
    // __u16 current_adxl_status.type;
    // __u16 current_adxl_status.code;    
    // __s32 current_adxl_status.value;

    if (current_adxl_status.type == 1) {
        switch(current_adxl_status.code) {
            case 30:
                if (current_adxl_status.value == 1) {
                   sprintf(tmpbuf,"Activity;%d%03d%d", current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);
                }else if (current_adxl_status.value == 0) {
                   sprintf(tmpbuf,"Inactivity;%d%03d%d", current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);
                }
                break;
            case 330:
                sprintf(tmpbuf,"Impact;%d%03d%d", current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);
                break;
            default:
                printf("ADXL_REPORT [%s] current_axdl_status:[%d %d %d]\n", __FUNCTION__, current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);
                break;
        }
    }

    //sprintf(tmpbuf,";%d%03d%d", current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);
    j = strlen(tmpbuf);
    //printf("ADXL_REPORT [%s] tmpbuf[%d]\n", __FUNCTION__, j); 
    for (i=0 ; i < j; i++) {
        buffer_send[sendlen+i] = tmpbuf[i];
    }    
    sendlen += j;
    
    // C.1.5 Vehicle ID  (Optional 4-character)
    buffer_send[sendlen+0] = ';';
    buffer_send[sendlen+1] = 'I';
    buffer_send[sendlen+2] = 'D';
    buffer_send[sendlen+3] = '=';
    //buffer_send[sendlen+4] = 'G';
    //buffer_send[sendlen+5] = 'G';
    //buffer_send[sendlen+6] = 'G';
    //buffer_send[sendlen+7] = 'G';
    sendlen += 4;

    j = strlen(MHX_SERIAL)-1;
    for (i=0 ; i < j; i++) {
        buffer_send[sendlen+i] = MHX_SERIAL[i];
    }    
    sendlen += j;
    
    // C.1.6 Checksum  (Optional 2-character)
    buffer_send[sendlen+0] = ';';
    buffer_send[sendlen+1] = '*';
    buffer_send[sendlen+2] = 'H';
    buffer_send[sendlen+3] = 'H';
    sendlen += 4;
     
    // C.1.7 Message Delimiter 
    buffer_send[sendlen] = '<';
   
    buffer_send[sendlen+1] = '\0';
    sendlen += 1;    

    return sendlen;
}


/*
 * Packing adxl report - binary
 */
static int fill_json_package(char *send_buf)
{
    int sendlen = 0;
    //printf("ADXL_REPORT [%s] current_axdl_status:[%d %d %d]\n", __FUNCTION__, current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);

    adxlr_buff.pack_len = 0;
    
    sendlen=fill_json_header(send_buf);
    adxlr_buff.pack_len += sendlen;   

    sendlen=fill_json_adxl_playload(send_buf + adxlr_buff.pack_len);
    adxlr_buff.pack_len += sendlen; 

    //sendlen=fill_json_device_playload(send_buf+adxlr_buff.pack_len, &mlist, AERC_REPORT_NUM);
    //adxlr_buff.pack_len += sendlen;

    sendlen=fill_json_tail(send_buf + adxlr_buff.pack_len);
    adxlr_buff.pack_len += sendlen;
    //printf("DEBUG [%s] length: %d,%s\n", __FUNCTION__, adxlr_buff.pack_len, send_buf);
    
    return adxlr_buff.pack_len;
}




static int new_fill_json_package(char *buf)
{
    char *pos = buf;
    int length = 0;
    int temp=0;
    time_t timestamp;
    char tmpbuf[256];

    /* header information */
    /*
     * protocol and version
     */
    strcpy(buf,"{\"header\":{\"protocol\":\"UDPReport\",\"version\":1.0,");
    length += strlen("{\"header\":{\"protocol\":\"UDPReport\",\"version\":1.0,");
    pos += length;

    /*
     * timestamp
     */
    timestamp = time (NULL);
    //sprintf(tmpbuf,"\"timestamp\":%ld},",timestamp);
    sprintf(tmpbuf,"\"timestamp\":%ld},\"adxl.input_event\":[{\"type\":%d,\"code\":%d,\"value\":%d}]}", timestamp, current_adxl_status.type,current_adxl_status.code,current_adxl_status.value);

    temp=scpytrim(pos,tmpbuf,strlen(tmpbuf));
    length += temp;
    pos += length;


    return length;
}


/*
 * Packing adxl report
 */
static void adxlr_pack_msg()
{
    /* ********pack send data ---- Begin here:******** */ 
    bzero(send_buf, sizeof(send_buf));

    //printf("ADXL_REPORT [%s] current_axdl_status:[%d %d %d]\n", __FUNCTION__, current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);

//    if ( strcmp(adxlr_conf.adxlr_type, "A") == 0 ) {
//        //printf("ADXL_REPORT [%s] adxlr_type: %s\n", __FUNCTION__, adxlr_conf.adxlr_type);
//        adxlr_buff.pack_len = fill_binary_package(send_buf);

    if ( strcmp(adxlr_conf.adxlr_type, "A") == 0 ) {
        //printf("ADXL_REPORT [%s] adxlr_type: %s\n", __FUNCTION__, adxlr_conf.adxlr_type);
        adxlr_buff.pack_len = fill_json_package(send_buf);
        //adxlr_buff.pack_len = new_fill_json_package(send_buf);

    } else if ( strcmp(adxlr_conf.adxlr_type, "B") == 0 ) {
        //printf("ADXL_REPORT [%s] adxlr_type: %s\n", __FUNCTION__, adxlr_conf.adxlr_type);
        adxlr_buff.pack_len = fill_taip_package(send_buf);
    }

    //printf("DEBUG [%s] length: %d,%s\n", __FUNCTION__, adxlr_buff.pack_len, send_buf);
    
    /* ******** pack send data here:----End******** */     

} // end of adxlr_pack_msg


/*
 * send adxl report
 */
static void adxlr_send_report()
{
    int ret = 0;
    //printf("DEBUG [%s] length: %d,%s\n", __FUNCTION__, adxlr_buff.pack_len, send_buf);

    ret = sendto(adxlr_buff.sockfd, send_buf, adxlr_buff.pack_len, 0, (struct sockaddr *)&(adxlr_buff.addr_remote), sizeof(struct sockaddr_in));

    if ( ret < 0 ){         
        printf("ADXL_REPORT [%s]: Failed send ADXL Report to %s : %s\n", __FUNCTION__, adxlr_conf.adxlr_ip, adxlr_conf.adxlr_port);
        syslog(LOGOPTS, "ADXL_REPORT [%s]: Failed to send report to %s : %s [length:%d]%s\n", __FUNCTION__, adxlr_conf.adxlr_ip, adxlr_conf.adxlr_port, adxlr_buff.pack_len, send_buf);
    }else{
#ifdef PRINT_DEBUG_INFO
        printf("ADXL_REPORT [%s]: Sent to %s : %s [length:%d]%s\n", __FUNCTION__, adxlr_conf.adxlr_ip, adxlr_conf.adxlr_port, adxlr_buff.pack_len, send_buf);
#endif
        syslog(LOGOPTS, "ADXL_REPORT [%s]: Sent to %s : %s [length:%d]%s\n", __FUNCTION__, adxlr_conf.adxlr_ip, adxlr_conf.adxlr_port, adxlr_buff.pack_len, send_buf);
    }    

    //DEBUG_PRINT_SEND_BUFFER();

    /* clear send buffer */
    adxlr_buff.pack_len = 0;
    memset(send_buf, 0, sizeof(send_buf));
}



void *do_timer_thread(void *vptr)
{
    int i = 0;
    
    for (;;) {
        sleep(atoi(adxlr_conf.adxlr_timer));
        //printf("ADXL_REPORT [%s]: mode: [%s] timer driven, interval: %s current_axdl_status:[%d %d %d]\n", __FUNCTION__, adxlr_conf.adxlr_mode, adxlr_conf.adxlr_timer,
        //       current_adxl_status.type, current_adxl_status.code, current_adxl_status.value);

        // Request ACT msg AND current_adxl_status is ACT
        if (( strcmp(adxlr_conf.adxlr_message1, "1" ) == 0 ) || ( strcmp(adxlr_conf.adxlr_message3, "1" ) == 0 )){
            if ((current_adxl_status.code == 30) && (current_adxl_status.value == 1)) {
                adxlr_pack_msg();
                adxlr_send_report();
            }
        }
        // Request INACT msg AND current_adxl_status is INACT
        if (( strcmp(adxlr_conf.adxlr_message1, "1" ) == 0 ) || ( strcmp(adxlr_conf.adxlr_message4, "1" ) == 0 )){
            if ((current_adxl_status.code == 30) && (current_adxl_status.value == 0)) {
                adxlr_pack_msg();
                adxlr_send_report();
            }
        }
        // Request IMPACT msg AND current_adxl_status is IMPACT
        if (( strcmp(adxlr_conf.adxlr_message1, "1" ) == 0 ) || ( strcmp(adxlr_conf.adxlr_message2, "1" ) == 0 )){
            if ((current_adxl_status.code == 330) && (current_adxl_status.value == 1)) {
                adxlr_pack_msg();
                adxlr_send_report();
            }
        }         
       
    }
    return NULL;
}




void sighandler(int signal)
{
    int i = 0;
    static int reentrant_flag = 0;


    printf("\n\nGot signal %d!\n", signal);
    printf("adxl_report [%s] Cleaning up...\n", __FUNCTION__);

    if ( ( strcmp(adxlr_conf.adxlr_mode, "B" ) == 0 ) || ( strcmp(adxlr_conf.adxlr_mode, "C" ) == 0 ) ){
        pthread_join(tid, &tret);        
    }

    printf("adxl_report [%s] reentrant_flag=%d\n", __FUNCTION__, reentrant_flag);     
    if (reentrant_flag != 1) {
        if (ctx){
            uci_free_context(ctx);
            ctx=NULL;
        }
        reentrant_flag = 1;
        printf("adxl_report [%s] uci_free_context done flag=%d\n", __FUNCTION__, reentrant_flag);      
    } 

    printf("adxl_report [%s] Cleaning up done\n", __FUNCTION__);
	
	exit(1);
}
                             


int main (int argc, char **argv)
{
	int fd, rd, i, j, k;
	struct input_event ev[64];
	int version;
	unsigned short id[4];
	unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
	char name[256] = "Unknown";
	int abs[5];
    fd_set read_fd_set;

    
	if (argc < 2) {
		printf ("Usage: evtest /dev/inputX\n");
		printf ("Where X = input device number\n");
		exit (1);
	}

	if ((fd = open(argv[argc - 1], O_RDONLY)) < 0) {
		perror("evtest");
		exit(1);
	}

	ioctl(fd, EVIOCGVERSION, &version);
	printf("Input driver version is %d.%d.%d\n",
		version >> 16, (version >> 8) & 0xff, version & 0xff);

	ioctl(fd, EVIOCGID, id);
	printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
		id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

	ioctl(fd, EVIOCGNAME(sizeof(name)), name);
	printf("Input device name: \"%s\"\n", name);

	memset(bit, 0, sizeof(bit));
	ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);

#ifdef PRINT_DEBUG_INFO
	printf("Supported events:\n");

	for (i = 0; i < EV_MAX; i++)
    {
		if (test_bit(i, bit[0])) 
        {
			printf("  Event type %d (%s)\n", i, events[i] ? events[i] : "?");
			ioctl(fd, EVIOCGBIT(i, KEY_MAX), bit[i]);
			for (j = 0; j < KEY_MAX; j++) 
            {
				if (test_bit(j, bit[i])) 
                {
					printf("    Event code %d (%s)\n", j, names[i] ? (names[i][j] ? names[i][j] : "?") : "?");
					if (i == EV_ABS) 
                    {
						ioctl(fd, EVIOCGABS(j), abs);
						for (k = 0; k < 5; k++)
                        {
							if ((k < 3) || abs[k])
                            {
								printf("      %s %6d\n", absval[k], abs[k]);
                            }
                        }
					}
				}
            }
		}
    }
#endif   
    check_serial_no();
    /* initialize */
    init_adxl_report(); 

    printf("Waiting for ... \n\n");

    /* initialize adxl status*/
    current_adxl_status.type = 1;
    current_adxl_status.code = 30;
    current_adxl_status.value = 0;

    /* DO NOT EXECUTE IF ADXL REPORT DISABLED */
    if ( strcmp(adxlr_conf.adxlr_status, "0" ) == 0 ) {
        printf("ADXL_REPORT [%s]: Exit because adxl report diabled [%s]\n", __FUNCTION__, adxlr_conf.adxlr_status);
        syslog(LOGOPTS, "ADXL_REPORT [%s]: Exit because adxl report diabled [%s]\n", __FUNCTION__, adxlr_conf.adxlr_status);
        exit(1);
    }
    /* DO NOT EXECUTE IF ADXL REPORT IP not CONFIGED */
    if ( strcmp(adxlr_conf.adxlr_ip, "0.0.0.0" ) == 0 ) {
        printf("ADXL_REPORT [%s]: Exit because adxl report ip not configed [%s]\n", __FUNCTION__, adxlr_conf.adxlr_ip);
        syslog(LOGOPTS, "ADXL_REPORT [%s]: Exit because adxl report ip not configed [%s]\n", __FUNCTION__, adxlr_conf.adxlr_ip);
        exit(1);
    }

    /* start a thread to do timer driven adxl report */
    //if (adxlr_conf.adxlr_mode == 'B'){
    if ( ( strcmp(adxlr_conf.adxlr_mode, "B" ) == 0 ) || ( strcmp(adxlr_conf.adxlr_mode, "C" ) == 0 ) ){
#ifdef PRINT_DEBUG_INFO
        printf("ADXL_REPORT [%s]: adxl report mode: [%s] timer driven, report interval: %s, Create pthread.\n", __FUNCTION__, adxlr_conf.adxlr_mode, adxlr_conf.adxlr_timer);
#endif
        pthread_create(&tid, NULL, &do_timer_thread, NULL);
    }


    /* Initialize the set of active sockets. */
    FD_ZERO (&read_fd_set);
    FD_SET (fd, &read_fd_set);	

   	/*
	 *	Set the signal handler to restore the old termios .
	 */
	sact.sa_handler = sighandler;
	sigaction(SIGHUP, &sact, NULL);
	sigaction(SIGINT, &sact, NULL);
	sigaction(SIGQUIT, &sact, NULL);	
	sigaction(SIGTERM, &sact, NULL);
    sigaction(SIGKILL, &sact, NULL);	

	while (1) 
    {
        /* Block until input arrives  */
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
            perror ("select");
            exit (EXIT_FAILURE);
        }
        if (FD_ISSET (fd, &read_fd_set)){       
            
            		rd = read(fd, ev, sizeof(struct input_event) * 64);
            
            		if (rd < (int) sizeof(struct input_event)) 
                    {
            			printf("yyy\n");
            			perror("\nevtest: error reading");
            			exit (1);
            		}
            
            		for (i = 0; i < rd / sizeof(struct input_event); i++)
                    {
                        printf("ADXL_REPORT [%s]: message: [%s %s %s %s] \n", __FUNCTION__, adxlr_conf.adxlr_message1, adxlr_conf.adxlr_message2, adxlr_conf.adxlr_message3, adxlr_conf.adxlr_message4);
                        // get VIP4G needed event info only
                        if ( (ev[i].type == 1) && (ev[i].code == 30) && (ev[i].value == 1) ) {
                            printf("VIP4G Accelerometer event: Activity\n");

                            if (( strcmp(adxlr_conf.adxlr_message1, "1" ) == 0 ) || ( strcmp(adxlr_conf.adxlr_message3, "1" ) == 0 )){
                                printf("ADXL_REPORT [%s]:  include ACT message\n", __FUNCTION__);                            

                                // update current adxl status
                                pthread_mutex_lock(&adxl_status_mutex);
                                current_adxl_status.type = 1;
                                current_adxl_status.code = 30;
                                current_adxl_status.value = 1;
                                pthread_mutex_unlock(&adxl_status_mutex);
                                
                                // send adxl report if report mode is event driven
                                if (( strcmp(adxlr_conf.adxlr_mode, "A" ) == 0 ) || ( strcmp(adxlr_conf.adxlr_mode, "C" ) == 0)){
                                    printf("ADXL_REPORT [%s]: report mode: [%s] event driven\n", __FUNCTION__, adxlr_conf.adxlr_mode);
                                    adxlr_pack_msg();
                                    adxlr_send_report();
                                } else {
                                    printf("ADXL_REPORT [%s]: adxl report mode: [%s] timer driven\n", __FUNCTION__, adxlr_conf.adxlr_mode);
                                }
                            }

                        }
                        if ( (ev[i].type == 1) && (ev[i].code == 30) && (ev[i].value == 0) ) {
                            printf("VIP4G Accelerometer event: Inactivity\n");
                            if (( strcmp(adxlr_conf.adxlr_message1, "1" ) == 0 ) || ( strcmp(adxlr_conf.adxlr_message4, "1" ) == 0 )){
                                printf("ADXL_REPORT [%s]:  include INACT message\n", __FUNCTION__); 
                            

                                // update current adxl status
                                pthread_mutex_lock(&adxl_status_mutex);
                                current_adxl_status.type = 1;
                                current_adxl_status.code = 30;
                                current_adxl_status.value = 0;
                                pthread_mutex_unlock(&adxl_status_mutex);
                                
                                // send adxl report if report mode is event driven
                                if (( strcmp(adxlr_conf.adxlr_mode, "A" ) == 0 ) || ( strcmp(adxlr_conf.adxlr_mode, "C" ) == 0 )){
                                    printf("ADXL_REPORT [%s]: report mode: [%s] event driven\n", __FUNCTION__, adxlr_conf.adxlr_mode);
                                    adxlr_pack_msg();
                                    adxlr_send_report();
                                } else {
                                    printf("ADXL_REPORT [%s]: report mode: [%s] timer driven\n", __FUNCTION__, adxlr_conf.adxlr_mode);
                                }
                            }
                        }
                        if ( (ev[i].type == 1) && (ev[i].code == 330) && (ev[i].value == 1) ) {
                            printf("VIP4G Accelerometer event: TAP\n");
                            if (( strcmp(adxlr_conf.adxlr_message1, "1" ) == 0 ) || ( strcmp(adxlr_conf.adxlr_message2, "1" ) == 0 )){
                                printf("ADXL_REPORT [%s]:  include IMPACT message\n", __FUNCTION__);                             

                                // update current adxl status
                                pthread_mutex_lock(&adxl_status_mutex);
                                current_adxl_status.type = 1;
                                current_adxl_status.code = 330;
                                current_adxl_status.value = 1;
                                pthread_mutex_unlock(&adxl_status_mutex);
                                
                                // send adxl report if report mode is event driven
                                if (( strcmp(adxlr_conf.adxlr_mode, "A" ) == 0 ) || ( strcmp(adxlr_conf.adxlr_mode, "C" ) == 0 )){ 
                                    printf("ADXL_REPORT [%s]: report mode: [%s] Event driven\n", __FUNCTION__, adxlr_conf.adxlr_mode);
                                    adxlr_pack_msg();
                                    adxlr_send_report();
                                } 
                                if ( strcmp(adxlr_conf.adxlr_mode, "B" ) == 0 ){ 
                                    printf("ADXL_REPORT [%s]: report mode: [%s] Timer driven\n", __FUNCTION__, adxlr_conf.adxlr_mode);
                                }
                            }

                        }

                    }
            
        } //end of if FD_ISSET
	} //end of while loop

    if ( strcmp(adxlr_conf.adxlr_mode, "B" ) == 0 ) {
        pthread_join(tid, &tret);
        printf("thread  exit code %d\n", (int)tret);
    }

} //end if main
