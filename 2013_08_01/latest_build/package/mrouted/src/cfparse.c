//daniel
#define lint

#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>
#include <string.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20070509

#define YYEMPTY (-1)
#define yyclearin    (yychar = YYEMPTY)
#define yyerrok      (yyerrflag = 0)
#define YYRECOVERING (yyerrflag != 0)

extern int yyparse(void);

static int yygrowstack(void);
#define YYPREFIX "yy"
//#line 2 "cfparse.y"
/*
 * Configuration file parser for mrouted.
 *
 * Written by Bill Fenner, NRL, 1994
 *
 * cfparse.y,v 3.8.4.30 1998/03/01 01:48:58 fenner Exp
 */
#include <stdio.h>
#include <stdarg.h>
#include "defs.h"
#include <netdb.h>
#include <ifaddrs.h>

/*
 * Local function declarations
 */
static void		fatal(char *fmt, ...);
static void		warn(char *fmt, ...);
static void		yyerror(char *s);
static char *		next_word(void);
static int		yylex(void);
static u_int32		valid_if(char *s);
static const char *	ifconfaddr(u_int32_t a);
int			yyparse(void);

static FILE *f;

char *configfilename = _PATH_MROUTED_CONF;

extern int cache_lifetime;
extern int prune_lifetime;

int allow_black_holes = 0;

static int lineno;

static struct uvif *v;

static int order, state;
static int noflood = 0;
static int rexmit = VIFF_REXMIT_PRUNES;

struct addrmask {
	u_int32	addr;
	int	mask;
};

struct boundnam {
	char		*name;
	struct addrmask	 bound;
};

#define MAXBOUNDS 20

struct boundnam boundlist[MAXBOUNDS];	/* Max. of 20 named boundaries */
int numbounds = 0;			/* Number of named boundaries */

//#line 61 "cfparse.y"
typedef union
{
	int num;
	char *ptr;
	struct addrmask addrmask;
	u_int32 addr;
	struct vf_element *filterelem;
} YYSTYPE;
//#line 90 "y.tab.c"
#define CACHE_LIFETIME 257
#define PRUNE_LIFETIME 258
#define PRUNING 259
#define BLACK_HOLE 260
#define NOFLOOD 261
#define PHYINT 262
#define TUNNEL 263
#define NAME 264
#define DISABLE 265
#define IGMPV1 266
#define SRCRT 267
#define BESIDE 268
#define METRIC 269
#define THRESHOLD 270
#define RATE_LIMIT 271
#define BOUNDARY 272
#define NETMASK 273
#define ALTNET 274
#define ADVERT_METRIC 275
#define FILTER 276
#define ACCEPT 277
#define DENY 278
#define EXACT 279
#define BIDIR 280
#define REXMIT_PRUNES 281
#define REXMIT_PRUNES2 282
#define PASSIVE 283
#define ALLOW_NONPRUNERS 284
#define NOTRANSIT 285
#define BLASTER 286
#define FORCE_LEAF 287
#define PRUNE_LIFETIME2 288
#define NOFLOOD2 289
#define SYSNAM 290
#define SYSCONTACT 291
#define SYSVERSION 292
#define SYSLOCATION 293
#define BOOLEAN 294
#define NUMBER 295
#define STRING 296
#define ADDRMASK 297
#define ADDR 298
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,   10,   10,   11,   13,   11,   15,   11,   11,   11,
   11,   11,   11,   11,   11,   11,   11,   11,   11,   11,
   14,   14,   16,   16,   16,   16,   12,   12,   18,   18,
   18,   18,   18,   18,   18,   18,   18,   17,   17,   17,
   17,   17,   17,   17,   17,   17,   17,   17,   17,   17,
   17,   17,   17,   17,   17,   17,   17,   17,   17,   17,
    1,    1,    2,    2,    3,    3,    4,    5,    5,    6,
    6,    7,    7,    8,    8,    9,
};
short yylen[] = {                                         2,
    1,    0,    2,    1,    0,    4,    0,    5,    2,    2,
    2,    1,    1,    1,    2,    3,    2,    2,    2,    2,
    0,    2,    1,    1,    2,    1,    0,    2,    1,    1,
    1,    2,    1,    2,    1,    1,    2,    2,    1,    2,
    1,    2,    1,    2,    1,    2,    1,    1,    2,    1,
    1,    1,    1,    1,    2,    2,    1,    2,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    2,    1,    2,    1,
};
short yydefred[] = {                                      2,
    0,    0,    4,    0,    0,    0,   12,   13,    0,    0,
    0,    0,    0,    0,    0,    0,    3,    9,   10,   11,
   62,   61,    5,    0,    0,   15,   17,   18,   19,   20,
   27,   64,   63,    7,   67,   16,    0,   21,   30,   31,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   60,
    0,   50,   54,   52,   53,    0,    0,   51,   29,   28,
    0,   40,   38,   44,   66,   46,   65,   32,   68,   69,
   34,   42,   71,   76,   56,   70,    0,    0,   58,   49,
   37,   55,   26,    0,   22,   23,   73,   75,   25,
};
short yydgoto[] = {                                       1,
   23,   34,   66,   36,   71,   75,   76,   77,   78,    2,
   17,   37,   31,   61,   38,   85,   59,   60,
};
short yysindex[] = {                                      0,
    0, -254,    0, -295, -274, -272,    0,    0, -285, -285,
 -273, -270, -271, -268, -267, -266,    0,    0,    0,    0,
    0,    0,    0, -284, -265,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -225,    0,    0,    0,
 -269, -264, -262, -281, -284, -280, -261, -277, -277,    0,
 -259,    0,    0,    0,    0, -252, -244,    0,    0,    0,
 -200,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -243, -223,    0,    0,
    0,    0,    0, -229,    0,    0,    0,    0,    0,
};
short yyrindex[] = {                                      0,
    0,   66,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  553,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  595,    0,    0,    0,
   43,   81,  119,  157,  395,  433,  195,  233,  277,    0,
  319,    0,    0,    0,    0,  471,    0,    0,    0,    0,
  633,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  357,    1,    0,    0,
    0,    0,    0,  509,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
   63,   29,    0,   32,    0,   30,   10,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   31,    0,
};
#define YYTABLESIZE 926
short yytable[] = {                                      18,
   74,    3,    4,    5,    6,    7,    8,    9,   10,   11,
   21,   32,   22,   33,   65,   35,   69,   70,   73,   74,
   19,   20,   25,   26,   27,   62,   12,   28,   29,   30,
   63,   35,   64,   72,   80,   13,   14,   15,   16,   39,
   40,   81,   41,   41,   42,   43,   44,   45,   46,   47,
   82,   48,   49,   74,   50,   88,   51,   52,   53,   54,
   55,   56,   57,   58,   89,    1,   83,   84,   41,   42,
   43,   44,   24,   68,   47,   67,   48,   49,   79,   50,
   39,   51,   52,   53,   54,   55,   87,   57,   58,    0,
    0,   86,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   45,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   47,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   43,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   57,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   59,   74,   74,    0,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,    0,    0,    0,   74,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   48,   41,
   41,    0,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   72,   39,   39,    0,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   33,   45,   45,    0,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   35,   47,   47,    0,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   43,   43,   43,   43,   43,   43,   43,   43,   43,   43,
   43,   43,   43,   43,   43,   43,   43,   43,   43,   43,
   36,   43,   43,    0,   43,   43,   43,   43,   43,   43,
   43,   43,   43,   43,   43,   43,   43,   43,   57,   57,
   57,   57,   57,   57,   57,   57,   57,   57,   57,   57,
   57,   57,   57,   57,   57,   57,   57,   57,   24,   57,
   57,    0,   57,   57,   57,   57,   57,   57,   57,   57,
   57,   57,   57,   57,   57,   57,    0,    0,    0,    0,
    0,    0,   59,   59,   59,   59,   59,   59,   59,   59,
   59,   59,   59,   59,   59,   59,   59,   59,   59,   59,
   59,   59,   14,   59,   59,    0,   59,   59,   59,   59,
   59,   59,   59,   59,   59,   59,   59,   59,   59,   59,
    0,    0,    0,    0,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,    6,   48,   48,    0,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   72,   72,   72,   72,   72,   72,   72,   72,
   72,   72,   72,   72,   72,   72,   72,   72,   72,   72,
   72,   72,    8,   72,   72,    0,   72,   72,   72,   72,
   72,   72,   72,   72,   72,   72,   72,   72,   72,   72,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,    0,    0,   33,   33,   33,   33,   33,   33,   33,
    0,   33,   33,    0,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,    0,
    0,   35,   35,   35,   35,   35,   35,   35,    0,   35,
   35,    0,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,    0,    0,   36,
   36,   36,   36,   36,   36,   36,    0,   36,   36,    0,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   24,   24,   24,   24,   24,   24,
   24,   24,   24,    0,    0,   24,   24,   24,   24,   24,
   24,    0,    0,   24,    0,   24,   24,    0,   24,   24,
   24,   24,   24,   24,   24,    0,   24,   24,   24,   24,
   24,   24,    0,    0,    0,    0,    0,    0,   14,   14,
   14,   14,   14,   14,   14,   14,   14,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   14,    0,    0,    0,    0,    0,    0,
    0,    0,   14,   14,   14,   14,    0,    0,    0,    0,
    6,    6,    6,    6,    6,    6,    6,    6,    6,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    6,    0,    0,    0,    0,
    0,    0,    0,    0,    6,    6,    6,    6,    8,    8,
    8,    8,    8,    8,    8,    8,    8,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    8,    0,    0,    0,    0,    0,    0,
    0,    0,    8,    8,    8,    8,
};
short yycheck[] = {                                     295,
    0,  256,  257,  258,  259,  260,  261,  262,  263,  264,
  296,  296,  298,  298,  296,  297,  297,  298,  296,  297,
  295,  294,  296,  294,  296,  295,  281,  296,  296,  296,
  295,  297,  295,  295,  294,  290,  291,  292,  293,  265,
  266,  294,    0,  269,  270,  271,  272,  273,  274,  275,
  295,  277,  278,  297,  280,  279,  282,  283,  284,  285,
  286,  287,  288,  289,  294,    0,  267,  268,  269,  270,
  271,  272,   10,   45,  275,   44,  277,  278,   49,  280,
    0,  282,  283,  284,  285,  286,   77,  288,  289,   -1,
   -1,   61,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    0,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  256,  257,  258,  259,
  260,  261,  262,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,    0,  277,  278,   -1,
  280,  281,  282,  283,  284,  285,  286,  287,  288,  289,
  290,  291,  292,  293,   -1,   -1,   -1,  297,  256,  257,
  258,  259,  260,  261,  262,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,    0,  277,
  278,   -1,  280,  281,  282,  283,  284,  285,  286,  287,
  288,  289,  290,  291,  292,  293,  256,  257,  258,  259,
  260,  261,  262,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,    0,  277,  278,   -1,
  280,  281,  282,  283,  284,  285,  286,  287,  288,  289,
  290,  291,  292,  293,  256,  257,  258,  259,  260,  261,
  262,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,    0,  277,  278,   -1,  280,  281,
  282,  283,  284,  285,  286,  287,  288,  289,  290,  291,
  292,  293,  256,  257,  258,  259,  260,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,    0,  277,  278,   -1,  280,  281,  282,  283,
  284,  285,  286,  287,  288,  289,  290,  291,  292,  293,
  256,  257,  258,  259,  260,  261,  262,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
    0,  277,  278,   -1,  280,  281,  282,  283,  284,  285,
  286,  287,  288,  289,  290,  291,  292,  293,  256,  257,
  258,  259,  260,  261,  262,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,    0,  277,
  278,   -1,  280,  281,  282,  283,  284,  285,  286,  287,
  288,  289,  290,  291,  292,  293,   -1,   -1,   -1,   -1,
   -1,   -1,  256,  257,  258,  259,  260,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,    0,  277,  278,   -1,  280,  281,  282,  283,
  284,  285,  286,  287,  288,  289,  290,  291,  292,  293,
   -1,   -1,   -1,   -1,  256,  257,  258,  259,  260,  261,
  262,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,    0,  277,  278,   -1,  280,  281,
  282,  283,  284,  285,  286,  287,  288,  289,  290,  291,
  292,  293,  256,  257,  258,  259,  260,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,    0,  277,  278,   -1,  280,  281,  282,  283,
  284,  285,  286,  287,  288,  289,  290,  291,  292,  293,
  256,  257,  258,  259,  260,  261,  262,  263,  264,  265,
  266,   -1,   -1,  269,  270,  271,  272,  273,  274,  275,
   -1,  277,  278,   -1,  280,  281,  282,  283,  284,  285,
  286,  287,  288,  289,  290,  291,  292,  293,  256,  257,
  258,  259,  260,  261,  262,  263,  264,  265,  266,   -1,
   -1,  269,  270,  271,  272,  273,  274,  275,   -1,  277,
  278,   -1,  280,  281,  282,  283,  284,  285,  286,  287,
  288,  289,  290,  291,  292,  293,  256,  257,  258,  259,
  260,  261,  262,  263,  264,  265,  266,   -1,   -1,  269,
  270,  271,  272,  273,  274,  275,   -1,  277,  278,   -1,
  280,  281,  282,  283,  284,  285,  286,  287,  288,  289,
  290,  291,  292,  293,  256,  257,  258,  259,  260,  261,
  262,  263,  264,   -1,   -1,  267,  268,  269,  270,  271,
  272,   -1,   -1,  275,   -1,  277,  278,   -1,  280,  281,
  282,  283,  284,  285,  286,   -1,  288,  289,  290,  291,
  292,  293,   -1,   -1,   -1,   -1,   -1,   -1,  256,  257,
  258,  259,  260,  261,  262,  263,  264,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  281,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  290,  291,  292,  293,   -1,   -1,   -1,   -1,
  256,  257,  258,  259,  260,  261,  262,  263,  264,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  281,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  290,  291,  292,  293,  256,  257,
  258,  259,  260,  261,  262,  263,  264,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  281,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  290,  291,  292,  293,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 298
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"CACHE_LIFETIME","PRUNE_LIFETIME",
"PRUNING","BLACK_HOLE","NOFLOOD","PHYINT","TUNNEL","NAME","DISABLE","IGMPV1",
"SRCRT","BESIDE","METRIC","THRESHOLD","RATE_LIMIT","BOUNDARY","NETMASK",
"ALTNET","ADVERT_METRIC","FILTER","ACCEPT","DENY","EXACT","BIDIR",
"REXMIT_PRUNES","REXMIT_PRUNES2","PASSIVE","ALLOW_NONPRUNERS","NOTRANSIT",
"BLASTER","FORCE_LEAF","PRUNE_LIFETIME2","NOFLOOD2","SYSNAM","SYSCONTACT",
"SYSVERSION","SYSLOCATION","BOOLEAN","NUMBER","STRING","ADDRMASK","ADDR",
};
char *yyrule[] = {
"$accept : conf",
"conf : stmts",
"stmts :",
"stmts : stmts stmt",
"stmt : error",
"$$1 :",
"stmt : PHYINT interface $$1 ifmods",
"$$2 :",
"stmt : TUNNEL interface addrname $$2 tunnelmods",
"stmt : CACHE_LIFETIME NUMBER",
"stmt : PRUNE_LIFETIME NUMBER",
"stmt : PRUNING BOOLEAN",
"stmt : BLACK_HOLE",
"stmt : NOFLOOD",
"stmt : REXMIT_PRUNES",
"stmt : REXMIT_PRUNES BOOLEAN",
"stmt : NAME STRING boundary",
"stmt : SYSNAM STRING",
"stmt : SYSCONTACT STRING",
"stmt : SYSVERSION STRING",
"stmt : SYSLOCATION STRING",
"tunnelmods :",
"tunnelmods : tunnelmods tunnelmod",
"tunnelmod : mod",
"tunnelmod : BESIDE",
"tunnelmod : BESIDE BOOLEAN",
"tunnelmod : SRCRT",
"ifmods :",
"ifmods : ifmods ifmod",
"ifmod : mod",
"ifmod : DISABLE",
"ifmod : IGMPV1",
"ifmod : NETMASK addrname",
"ifmod : NETMASK",
"ifmod : ALTNET addrmask",
"ifmod : ALTNET",
"ifmod : FORCE_LEAF",
"ifmod : FORCE_LEAF BOOLEAN",
"mod : THRESHOLD NUMBER",
"mod : THRESHOLD",
"mod : METRIC NUMBER",
"mod : METRIC",
"mod : ADVERT_METRIC NUMBER",
"mod : ADVERT_METRIC",
"mod : RATE_LIMIT NUMBER",
"mod : RATE_LIMIT",
"mod : BOUNDARY bound",
"mod : BOUNDARY",
"mod : REXMIT_PRUNES2",
"mod : REXMIT_PRUNES2 BOOLEAN",
"mod : PASSIVE",
"mod : NOFLOOD2",
"mod : NOTRANSIT",
"mod : BLASTER",
"mod : ALLOW_NONPRUNERS",
"mod : PRUNE_LIFETIME2 NUMBER",
"mod : ACCEPT filter",
"mod : ACCEPT",
"mod : DENY filter",
"mod : DENY",
"mod : BIDIR",
"interface : ADDR",
"interface : STRING",
"addrname : ADDR",
"addrname : STRING",
"bound : boundary",
"bound : STRING",
"boundary : ADDRMASK",
"addrmask : ADDRMASK",
"addrmask : ADDR",
"filter : filtlist",
"filter : STRING",
"filtlist : filtelement",
"filtlist : filtelement filtlist",
"filtelement : filtelem",
"filtelement : filtelem EXACT",
"filtelem : ADDRMASK",
};
#endif
#if YYDEBUG
#include <stdio.h>
#endif

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

int      yydebug;
int      yynerrs;
int      yyerrflag;
int      yychar;
short   *yyssp;
YYSTYPE *yyvsp;
YYSTYPE  yyval;
YYSTYPE  yylval;
struct _mcastctl *mcastctl;
int detectrp;      

/* variables for the parser stack */
static short   *yyss;
static short   *yysslim;
static YYSTYPE *yyvs;
static int      yystacksize;
//#line 628 "cfparse.y"
static void fatal(char *fmt, ...)
{
	va_list ap;
	char buf[MAXHOSTNAMELEN + 100];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	logit(LOG_ERR, 0, "%s: %s near line %d", configfilename, buf, lineno);
}

static void warn(char *fmt, ...)
{
    va_list ap;
    char buf[200];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    logit(LOG_WARNING, 0, "%s: %s near line %d", configfilename, buf, lineno);
}

static void yyerror(char *s)
{
    logit(LOG_ERR, 0, "%s: %s near line %d", configfilename, s, lineno);
}

static char *next_word(void)
{
    static char buf[1024];
    static char *p=NULL;
    char *q;

    while (1) {
        if (!p || !*p) {
            lineno++;
            if (fgets(buf, sizeof(buf), f) == NULL)
                return NULL;
            p = buf;
        }
        while (*p && (*p == ' ' || *p == '\t'))	/* skip whitespace */
            p++;
        if (*p == '#') {
            p = NULL;		/* skip comments */
            continue;
        }
        q = p;
#ifdef SNMP
        if (*p == '"') {
            p++;
            while (*p && *p != '"' && *p != '\n')
                p++;		/* find next whitespace */
            if (*p == '"')
                p++;
        } else
#endif
        while (*p && *p != ' ' && *p != '\t' && *p != '\n')
            p++;		/* find next whitespace */
        *p++ = '\0';	/* null-terminate string */

        if (!*q) {
            p = NULL;
            continue;	/* if 0-length string, read another line */
        }

        return q;
    }
}

/*
 * List of keywords.  Must have an empty record at the end to terminate
 * list.  If a second value is specified, the first is used at the beginning
 * of the file and the second is used while parsing interfaces (e.g. after
 * the first "phyint" or "tunnel" keyword).
 */
static struct keyword {
	char	*word;
	int	val1;
	int	val2;
} words[] = {
	{ "cache_lifetime",	CACHE_LIFETIME, 0 },
	{ "prune_lifetime",	PRUNE_LIFETIME,	PRUNE_LIFETIME2 },
	{ "pruning",		PRUNING, 0 },
	{ "phyint",		PHYINT, 0 },
	{ "tunnel",		TUNNEL, 0 },
	{ "disable",		DISABLE, 0 },
	{ "metric",		METRIC, 0 },
	{ "advert_metric",	ADVERT_METRIC, 0 },
	{ "threshold",		THRESHOLD, 0 },
	{ "rate_limit",		RATE_LIMIT, 0 },
	{ "force_leaf",		FORCE_LEAF, 0 },
	{ "srcrt",		SRCRT, 0 },
	{ "sourceroute",	SRCRT, 0 },
	{ "boundary",		BOUNDARY, 0 },
	{ "netmask",		NETMASK, 0 },
	{ "igmpv1",		IGMPV1, 0 },
	{ "altnet",		ALTNET, 0 },
	{ "name",		NAME, 0 },
	{ "accept",		ACCEPT, 0 },
	{ "deny",		DENY, 0 },
	{ "exact",		EXACT, 0 },
	{ "bidir",		BIDIR, 0 },
	{ "allow_nonpruners",	ALLOW_NONPRUNERS, 0 },
#ifdef ALLOW_BLACK_HOLES
	{ "allow_black_holes",	BLACK_HOLE, 0 },
#endif
	{ "noflood",		NOFLOOD, NOFLOOD2 },
	{ "notransit",		NOTRANSIT, 0 },
	{ "blaster",		BLASTER, 0 },
	{ "rexmit_prunes",	REXMIT_PRUNES, REXMIT_PRUNES2 },
	{ "passive",		PASSIVE, 0 },
	{ "beside",		BESIDE, 0 },
#ifdef SNMP
	{ "sysName",		SYSNAM, 0 },
	{ "sysContact",		SYSCONTACT, 0 },
	{ "sysVersion",		SYSVERSION, 0 },
	{ "sysLocation",	SYSLOCATION, 0 },
#endif
	{ NULL,			0, 0 }
};


static int yylex(void)
{
    u_int32 addr, n;
    char *q;
    struct keyword *w;

    if ((q = next_word()) == NULL) {
        return 0;
    }

    for (w = words; w->word; w++)
        if (!strcmp(q, w->word))
            return (state && w->val2) ? w->val2 : w->val1;

    if (!strcmp(q,"on") || !strcmp(q,"yes")) {
        yylval.num = 1;
        return BOOLEAN;
    }
    if (!strcmp(q,"off") || !strcmp(q,"no")) {
        yylval.num = 0;
        return BOOLEAN;
    }
    if (!strcmp(q,"default")) {
        yylval.addrmask.mask = 0;
        yylval.addrmask.addr = 0;
        return ADDRMASK;
    }
    if (sscanf(q,"%[.0-9]/%u%c",s1,&n,s2) == 2) {
        if ((addr = inet_parse(s1,1)) != 0xffffffff) {
            yylval.addrmask.mask = n;
            yylval.addrmask.addr = addr;
            return ADDRMASK;
        }
        /* fall through to returning STRING */
    }
    if (sscanf(q,"%[.0-9]%c",s1,s2) == 1) {
        if ((addr = inet_parse(s1,4)) != 0xffffffff &&
            inet_valid_host(addr)) { 
            yylval.addr = addr;
            return ADDR;
        }
    }
    if (sscanf(q,"0x%8x%c", &n, s1) == 1) {
        yylval.addr = n;
        return ADDR;
    }
    if (sscanf(q,"%u%c",&n,s1) == 1) {
        yylval.num = n;
        return NUMBER;
    }
#ifdef SNMP
    if (*q=='"') {
        if (q[ strlen(q)-1 ]=='"')
            q[ strlen(q)-1 ]='\0'; /* trash trailing quote */
        yylval.ptr = q+1;
        return STRING;
    }
#endif
    yylval.ptr = q;
    return STRING;
}

#include <fcntl.h>
#define LINE_LENGTH 256

/*
        temp = strstr(buf[i], "channel");
        if(temp) {
            strcpy(channel[FoundStart], temp + sizeof("channel"));
            ClrOption(channel[FoundStart]);
            continue;
        }
*/

static void Ipstr2Ipuint(char *ipstr, unsigned int *Ipuint)
{
    unsigned char a, b, c, d;
    sscanf(ipstr, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d );
    *Ipuint = ( a << 24 ) | ( b << 16 ) | ( c << 8 ) | d;
}

static void ClrOption(char* Option)
{
    char tmp[32];
    int i, j, k;

    memset(tmp, 0, sizeof(tmp));
    for(i=0, j=0, k=0; Option[i]; i++) {
        if(i && (Option[i-1] == '\'') && (Option[i] == '\''))break;
        if(k) tmp[j++] = Option[i];
        if( (!k) && (Option[i] != ' ') && (Option[i] != '\'') ) {
            tmp[j++] = Option[i];
            k = 1;
        }
    }

    for(i=0; tmp[i]; i++) {
        if((tmp[i] == ' ') || (tmp[i] == '\'')) {
            tmp[i] = 0;
            break;
        }
    }
    strcpy(Option, tmp);    
}

#define GETOPTION(opt) \
temp = strstr(buf[i], #opt);\
if(temp) {\
    strcpy(opt, temp + sizeof(#opt));\
    ClrOption(opt);\
    continue;\
}

static void UciParser(char (*buf)[256], struct _mcastctl *tmcastctl)
{
    char channel[32]={0};
    char tunnel[32]={0};
    char localip[32]={0};
    char remoteip[32]={0};
    char srcip[32]={0};
    char srcmask[32]={0};
    char grpip[32]={0};
    char *temp=0;
    int i;  

    for(i=0; i<20; i++) 
    {
        if(!buf[i][0]) break;
        //printf("buf[%d]:%s\n", i, buf[i]); 
        GETOPTION(channel);
        strcpy(tmcastctl->channel, channel);
        GETOPTION(tunnel);
        strcpy(tmcastctl->tunnel, tunnel);
        GETOPTION(localip);
        GETOPTION(remoteip); 
        GETOPTION(srcip);
        GETOPTION(srcmask);
        GETOPTION(grpip);
    }
    //printf("UciParser: channel=%s, tunnel=%s, localip=%s, remoteip=%s, srcip=%s, srcmask=%s, grpip=%s\n", 
    //       channel, tunnel, localip, remoteip, srcip, srcmask, grpip);
    Ipstr2Ipuint(localip, &tmcastctl->localip32);
    Ipstr2Ipuint(remoteip, &tmcastctl->remoteip32);
    Ipstr2Ipuint(srcip, &tmcastctl->srcip32);
    Ipstr2Ipuint(srcmask, &tmcastctl->srcmask32);
    Ipstr2Ipuint(grpip, &tmcastctl->grpip32);

    if( (tmcastctl->localip32&tmcastctl->srcmask32) == (tmcastctl->srcip32&tmcastctl->srcmask32) ) {
        detectrp = 1;
        //printf("detectrp=%d\n", detectrp);
    }
}

static void AddNewMcastctl(struct _mcastctl* tmcastctl)
{
    struct _mcastctl *tmp = mcastctl;
    if(!tmp) return;

    while(tmp->next) {
        tmp = tmp->next;
    }
    tmp->next = tmcastctl;
    tmcastctl->last = tmp;
}

void FreeMcastctl(void)
{
    struct _mcastctl *tmp = mcastctl;
    struct _mcastctl *current;
    struct _mcastctl *last;

    if(!tmp) return;
    // find the last one
    while(tmp->next) {
        tmp = tmp->next;
    }

    // get last before free current
    current = tmp;
    last = tmp->last;
    free(current);
    while(last) {        
        current = last;
        last = last->last;
        free(current);
    }
}

void Dbgmcastctl(void)
{
    struct _mcastctl *tmp = mcastctl;

    while(tmp) {
        logit(LOG_DEBUG, 0, "Dbgmcastctl:channel=%s, tunnel=%s, Niftunnel=%d.",
              tmp->channel, tmp->tunnel, tmp->Niftunnel);
        logit(LOG_DEBUG, 0, "localip32=%s, remoteip32=%s,",
              inet_fmt(tmp->localip32, s1, sizeof(s1)), 
              inet_fmt(tmp->remoteip32, s2, sizeof(s2)));
        logit(LOG_DEBUG, 0, "srcip32=%s, srcmask32=%s, grpip32=%s.",
              inet_fmt(tmp->srcip32, s1, sizeof(s1)), 
              inet_fmt(tmp->srcmask32, s2, sizeof(s2)),
              inet_fmt(tmp->grpip32, s3, sizeof(s3)));
        tmp = tmp->next;
    }
}
    
void init_tree(void)
{
    char line[LINE_LENGTH];
    FILE* frd = NULL;
    char *temp;
    struct _mcastctl *tmcastctl;

    char buf[20][256];
    int i;
    int FoundStart;

    frd = fopen("/etc/config/mcast", "rt");
    if(frd == NULL) {
        logit(LOG_ERR, 0, "Couldn't read %s.", "/etc/config/mcast");
        return; 
    }

    detectrp = 0;
    mcastctl = 0;
    FoundStart = 0;
    memset(buf, 0, sizeof(buf));
    i = 0;
    while(1) {
        if(!FoundStart){
            temp = fgets(line,LINE_LENGTH,frd);
            //printf("1:%s", line);
            if(!temp)break;
        } else if (FoundStart == 1000) break;
        else {
            temp = strstr(line, "config \'mcast");
            if(!temp) {
                temp = fgets(line,LINE_LENGTH,frd);
                //printf("2:%s", line);
                if(!temp)break;
            }
        }

        temp = strchr(line, '\n');
        if(temp) *temp = 0;
        temp = strchr(line, '#');
        if(temp) *temp = 0;
        if(!FoundStart) {
            temp = strstr(line, "mcast");
            if(temp) {
                while(1)
                {
                    temp = fgets(line,LINE_LENGTH,frd);                    
                    if(!temp) {
                        mcastctl = malloc(sizeof(struct _mcastctl));                        
                        if(!mcastctl) {
                            logit(LOG_ERR, 0, "Run out of memory 903");
                            return;
                        }
                        memset(mcastctl, 0, sizeof(struct _mcastctl));
                        UciParser(buf, mcastctl);                        
                        FoundStart = 1000;
                        break;
                    }
                    temp = strchr(line, '\n');
                    if(temp) *temp = 0;
                    temp = strchr(line, '#');
                    if(temp) *temp = 0;
                    //printf("3:%s\n", line);

                    temp = strstr(line, "config");
                    if(temp) 
                    {
                        mcastctl = malloc(sizeof(struct _mcastctl));                        
                        if(!mcastctl) {
                            logit(LOG_ERR, 0, "Run out of memory 923");
                            return;
                        }
                        memset(mcastctl, 0, sizeof(struct _mcastctl));
                        UciParser(buf, mcastctl);
                        memset(buf, 0, sizeof(buf));
                        i = 0;
                        FoundStart++;
                        break;
                    }
                    strcpy(buf[i++], line);   
                }
            }
        } else {
            temp = strstr(line, "mcast");
            if(temp) {
                while(1)
                {
                    temp = fgets(line,LINE_LENGTH,frd);
                    //printf("4:%s", line);
                    if(!temp) {
                        tmcastctl = malloc(sizeof(struct _mcastctl));
                        if(!tmcastctl) {
                            logit(LOG_ERR, 0, "Run out of memory 945");
                            return;
                        }
                        memset(tmcastctl, 0, sizeof(struct _mcastctl));
                        AddNewMcastctl(tmcastctl);
                        UciParser(buf, tmcastctl);
                        FoundStart = 1000;
                        break;
                    }
                    temp = strchr(line, '\n');
                    if(temp) *temp = 0;
                    temp = strchr(line, '#');
                    if(temp) *temp = 0;
                    temp = strstr(line, "config");
                    if(temp) 
                    {
                        tmcastctl = malloc(sizeof(struct _mcastctl));
                        if(!tmcastctl) {
                            logit(LOG_ERR, 0, "Run out of memory 945");
                            return;
                        }
                        memset(tmcastctl, 0, sizeof(struct _mcastctl));
                        AddNewMcastctl(tmcastctl);
                        UciParser(buf, tmcastctl);
                        memset(buf, 0, sizeof(buf));
                        i = 0;
                        FoundStart++;
                        break;
                    }
                    strcpy(buf[i++], line);
                }
            }
        }
    }
    fclose(frd);
}

void McastParse(void)
{
    struct _mcastctl *tmcastctl = mcastctl;
    struct _mcastctl *current;
    vifi_t vifi;
    struct uvif *v;

    if(detectrp) return;
    while(tmcastctl)
    {
        current = tmcastctl;
        tmcastctl = tmcastctl->next;
        if (!current->channel[0] && strcmp(current->tunnel, "yes")) continue;
        if( !current->srcip32 || !current->srcmask32 || !current->localip32 || 
            !current->remoteip32 || !current->grpip32 ) continue;

        for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
            if( (v->uv_lcl_addr & (current->srcmask32)) == ((current->localip32) & (current->srcmask32)) )
                v->uv_flags &= ~VIFF_DISABLED; 
            else if( (v->uv_lcl_addr & (current->srcmask32)) == ((current->srcip32) & (current->srcmask32)) )
                v->uv_flags &= ~VIFF_DISABLED; 
            else v->uv_flags |= VIFF_DISABLED;       
        }
        
        if (current->channel[0]){
            for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
                if ((!(v->uv_flags & VIFF_DISABLED)) && (!(v->uv_flags & VIFF_DOWN))) {                    
                    if (v->uv_flags & VIFF_TUNNEL) {   
                        if(!strcmp(v->uv_name, current->channel)) {                     
                            v->uv_flags &= ~VIFF_DISABLED; 
                        }
                    }
                } 
            }
        } else {
            if (strcmp(current->tunnel, "yes")) continue;
            for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
                if ((!(v->uv_flags & VIFF_DISABLED)) && (!(v->uv_flags & VIFF_DOWN))) {                    
                    if (v->uv_flags & VIFF_TUNNEL) {                        
                        v->uv_flags &= ~VIFF_DISABLED; 
                    }
                } 
            }
        }
    }
}

void config_vifs_from_file(void)
{
    order = 0;
    state = 0;
    numbounds = 0;
    lineno = 0;

    if ((f = fopen(configfilename, "r")) == NULL) {
        if (errno != ENOENT)
            logit(LOG_ERR, errno, "Cannot open %s", configfilename);
        return;
    }

    yyparse();
    McastParse();

    fclose(f);
}

static u_int32 valid_if(char *s)
{
    vifi_t vifi;
    struct uvif *v;

    for (vifi=0, v=uvifs; vifi<numvifs; vifi++, v++) {
        if (!strcmp(v->uv_name, s))
            return v->uv_lcl_addr;
    }

    return 0;
}

static const char *ifconfaddr(u_int32_t a)
{
    static char ifname[IFNAMSIZ];
    struct ifaddrs *ifap, *ifa;

    if (getifaddrs(&ifap) != 0)
	return NULL;

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
	if (ifa->ifa_addr &&
	    ifa->ifa_addr->sa_family == AF_INET &&
	    ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr == a) {
	    strlcpy(ifname, ifa->ifa_name, sizeof(ifname));
	    freeifaddrs(ifap);

	    return ifname;
	}
    }

    freeifaddrs(ifap);

    return NULL;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 4
 * End:
 */
//#line 770 "y.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(void)
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = yyssp - yyss;
    newss = (yyss != 0)
          ? (short *)realloc(yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    yyss  = newss;
    yyssp = newss + i;
    newvs = (yyvs != 0)
          ? (YYSTYPE *)realloc(yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse(void)
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    yyerror("syntax error");

#ifdef lint
    goto yyerrlab;
#endif

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yyvsp[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 5:
//#line 102 "cfparse.y"
{
	    vifi_t vifi;

	    state++;

	    if (order)
		fatal("phyints must appear before tunnels");

	    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
		if (!(v->uv_flags & VIFF_TUNNEL) && yyvsp[0].addr == v->uv_lcl_addr)
		    break;
	    }

	    if (vifi == numvifs)
		fatal("%s is not a configured interface", inet_fmt(yyvsp[0].addr, s1, sizeof(s1)));
	}
break;
case 7:
//#line 120 "cfparse.y"
{
	    const char *ifname;
	    struct ifreq ffr;
	    vifi_t vifi;

	    order++;

	    ifname = ifconfaddr(yyvsp[-1].addr);
	    if (ifname == 0)
		fatal("Tunnel local address %s is not mine", inet_fmt(yyvsp[-1].addr, s1, sizeof(s1)));

	    if (((ntohl(yyvsp[-1].addr) & IN_CLASSA_NET) >> IN_CLASSA_NSHIFT) == IN_LOOPBACKNET)
		fatal("Tunnel local address %s is a loopback address", inet_fmt(yyvsp[-1].addr, s1, sizeof(s1)));

	    if (ifconfaddr(yyvsp[0].addr) != NULL)
		fatal("Tunnel remote address %s is one of mine", inet_fmt(yyvsp[0].addr, s1, sizeof(s1)));

	    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
		if (v->uv_flags & VIFF_TUNNEL) {
		    if (yyvsp[0].addr == v->uv_rmt_addr)
			fatal("Duplicate tunnel to %s",
			      inet_fmt(yyvsp[0].addr, s1, sizeof(s1)));
		} else if (!(v->uv_flags & VIFF_DISABLED)) {
		    if ((yyvsp[0].addr & v->uv_subnetmask) == v->uv_subnet)
			fatal("Unnecessary tunnel to %s, same subnet as vif %d (%s)",
			      inet_fmt(yyvsp[0].addr, s1, sizeof(s1)), vifi, v->uv_name);
		}
	    }

	    if (numvifs == MAXVIFS)
		fatal("too many vifs");

	    strlcpy(ffr.ifr_name, ifname, sizeof(ffr.ifr_name));
	    if (ioctl(udp_socket, SIOCGIFFLAGS, (char *)&ffr)<0)
		fatal("ioctl SIOCGIFFLAGS on %s", ffr.ifr_name);

	    v = &uvifs[numvifs];
	    zero_vif(v, 1);
	    v->uv_flags	= VIFF_TUNNEL | rexmit | noflood;
	    v->uv_flags |= VIFF_OTUNNEL; /*XXX*/
	    v->uv_lcl_addr	= yyvsp[-1].addr;
	    v->uv_rmt_addr	= yyvsp[0].addr;
	    v->uv_dst_addr	= yyvsp[0].addr;
	    strlcpy(v->uv_name, ffr.ifr_name, sizeof(v->uv_name));

	    if (!(ffr.ifr_flags & IFF_UP)) {
		v->uv_flags |= VIFF_DOWN;
		vifs_down = TRUE;
	    }
	}
break;
case 8:
//#line 171 "cfparse.y"
{
	    if (!(v->uv_flags & VIFF_OTUNNEL))
		init_ipip_on_vif(v);

	    logit(LOG_INFO, 0, "installing tunnel from %s to %s as vif #%u - rate=%d",
		  inet_fmt(yyvsp[-3].addr, s1, sizeof(s1)), inet_fmt(yyvsp[-2].addr, s2, sizeof(s2)),
		  numvifs, v->uv_rate_limit);

	    ++numvifs;

	}
break;
case 9:
//#line 183 "cfparse.y"
{
	    if (yyvsp[0].num < MIN_CACHE_LIFETIME)
		warn("cache_lifetime %d must be at least %d", yyvsp[0].num, MIN_CACHE_LIFETIME);
	    else
		cache_lifetime = yyvsp[0].num;
	}
break;
case 10:
//#line 190 "cfparse.y"
{
	    if (yyvsp[0].num < MIN_PRUNE_LIFETIME)
		warn("prune_lifetime %d must be at least %d", yyvsp[0].num, MIN_PRUNE_LIFETIME);
	    else
		prune_lifetime = yyvsp[0].num;
	}
break;
case 11:
//#line 197 "cfparse.y"
{
	    if (yyvsp[0].num != 1)
		warn("Disabling pruning is no longer supported");
	}
break;
case 12:
//#line 202 "cfparse.y"
{
#ifdef ALLOW_BLACK_HOLES
	    allow_black_holes = 1;
#endif
	}
break;
case 13:
//#line 213 "cfparse.y"
{
	    vifi_t vifi;

	    noflood = VIFF_NOFLOOD;
	    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v)
		v->uv_flags |= VIFF_NOFLOOD;
	}
break;
case 14:
//#line 226 "cfparse.y"
{
	    vifi_t vifi;

	    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v)
		v->uv_flags |= VIFF_REXMIT_PRUNES;
	}
break;
case 15:
//#line 238 "cfparse.y"
{
	    if (yyvsp[0].num) {
		vifi_t vifi;

		for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v)
		    v->uv_flags |= VIFF_REXMIT_PRUNES;
	    } else {
		rexmit = 0;
	    }
	}
break;
case 16:
//#line 249 "cfparse.y"
{
	    size_t len = strlen(yyvsp[-1].ptr) + 1;
	    if (numbounds >= MAXBOUNDS) {
		fatal("Too many named boundaries (max %d)", MAXBOUNDS);
	    }

	    boundlist[numbounds].name = malloc(len);
	    strlcpy(boundlist[numbounds].name, yyvsp[-1].ptr, len);
	    boundlist[numbounds++].bound = yyvsp[0].addrmask;
	}
break;
case 17:
//#line 260 "cfparse.y"
{
#ifdef SNMP
	    set_sysName(yyvsp[0].ptr);
#endif /* SNMP */
	}
break;
case 18:
//#line 266 "cfparse.y"
{
#ifdef SNMP
	    set_sysContact(yyvsp[0].ptr);
#endif /* SNMP */
	}
break;
case 19:
//#line 272 "cfparse.y"
{
#ifdef SNMP
	    set_sysVersion(yyvsp[0].ptr);
#endif /* SNMP */
	}
break;
case 20:
//#line 278 "cfparse.y"
{
#ifdef SNMP
	    set_sysLocation(yyvsp[0].ptr);
#endif /* SNMP */
	}
break;
case 24:
//#line 291 "cfparse.y"
{
	    v->uv_flags |= VIFF_OTUNNEL;
	}
break;
case 25:
//#line 295 "cfparse.y"
{
	    if (yyvsp[0].num)
		v->uv_flags |= VIFF_OTUNNEL;
	    else
		v->uv_flags &= ~VIFF_OTUNNEL;
	}
break;
case 26:
//#line 302 "cfparse.y"
{
	    fatal("Source-route tunnels not supported");
	}
break;
case 30:
//#line 312 "cfparse.y"
{ v->uv_flags |= VIFF_DISABLED; }
break;
case 31:
//#line 313 "cfparse.y"
{ v->uv_flags |= VIFF_IGMPV1; }
break;
case 32:
//#line 315 "cfparse.y"
{
	    u_int32 subnet, mask;

	    mask = yyvsp[0].addr;
	    subnet = v->uv_lcl_addr & mask;
	    if (!inet_valid_subnet(subnet, mask))
		fatal("Invalid netmask");
	    v->uv_subnet = subnet;
	    v->uv_subnetmask = mask;
	    v->uv_subnetbcast = subnet | ~mask;
	}
break;
case 33:
//#line 327 "cfparse.y"
{
	    warn("Expected address after netmask keyword, ignored");
	}
break;
case 34:
//#line 331 "cfparse.y"
{
	    struct phaddr *ph;

	    ph = (struct phaddr *)malloc(sizeof(struct phaddr));
	    if (ph == NULL)
		fatal("out of memory");

	    if (yyvsp[0].addrmask.mask) {
		VAL_TO_MASK(ph->pa_subnetmask, yyvsp[0].addrmask.mask);
	    } else {
		ph->pa_subnetmask = v->uv_subnetmask;
	    }

	    ph->pa_subnet = yyvsp[0].addrmask.addr & ph->pa_subnetmask;
	    ph->pa_subnetbcast = ph->pa_subnet | ~ph->pa_subnetmask;

	    if (yyvsp[0].addrmask.addr & ~ph->pa_subnetmask)
		warn("Extra subnet %s/%d has host bits set",
		     inet_fmt(yyvsp[0].addrmask.addr, s1, sizeof(s1)), yyvsp[0].addrmask.mask);

	    ph->pa_next = v->uv_addrs;
	    v->uv_addrs = ph;
	}
break;
case 35:
//#line 355 "cfparse.y"
{
	    warn("Expected address after altnet keyword, ignored");
	}
break;
case 36:
//#line 359 "cfparse.y"
{
	    v->uv_flags |= VIFF_FORCE_LEAF;
	}
break;
case 37:
//#line 363 "cfparse.y"
{
	    if (yyvsp[0].num)
		v->uv_flags |= VIFF_FORCE_LEAF;
	    else
		v->uv_flags &= ~VIFF_FORCE_LEAF;
	}
break;
case 38:
//#line 372 "cfparse.y"
{
	    if (yyvsp[0].num < 1 || yyvsp[0].num > 255)
		fatal("Invalid threshold %d",yyvsp[0].num);
	    v->uv_threshold = yyvsp[0].num;
	}
break;
case 39:
//#line 378 "cfparse.y"
{
	    warn("Expected number after threshold keyword, ignored");
	}
break;
case 40:
//#line 382 "cfparse.y"
{
	    if (yyvsp[0].num < 1 || yyvsp[0].num > UNREACHABLE)
		fatal("Invalid metric %d",yyvsp[0].num);
	    v->uv_metric = yyvsp[0].num;
	}
break;
case 41:
//#line 388 "cfparse.y"
{
	    warn("Expected number after metric keyword, ignored");
	}
break;
case 42:
//#line 392 "cfparse.y"
{
	    if (yyvsp[0].num < 0 || yyvsp[0].num > UNREACHABLE - 1)
		fatal("Invalid advert_metric %d", yyvsp[0].num);
	    v->uv_admetric = yyvsp[0].num;
	}
break;
case 43:
//#line 398 "cfparse.y"
{
	    warn("Expected number after advert_metric keyword, ignored");
	}
break;
case 44:
//#line 402 "cfparse.y"
{
	    if (yyvsp[0].num > MAX_RATE_LIMIT)
		fatal("Invalid rate_limit %d",yyvsp[0].num);
	    v->uv_rate_limit = yyvsp[0].num;
	}
break;
case 45:
//#line 408 "cfparse.y"
{
	    warn("Expected number after rate_limit keyword, ignored");
	}
break;
case 46:
//#line 412 "cfparse.y"
{
	    struct vif_acl *v_acl;

	    v_acl = (struct vif_acl *)malloc(sizeof(struct vif_acl));
	    if (v_acl == NULL)
		fatal("out of memory");

	    VAL_TO_MASK(v_acl->acl_mask, yyvsp[0].addrmask.mask);
	    v_acl->acl_addr = yyvsp[0].addrmask.addr & v_acl->acl_mask;
	    if (yyvsp[0].addrmask.addr & ~v_acl->acl_mask)
		warn("Boundary spec %s/%d has host bits set",
		     inet_fmt(yyvsp[0].addrmask.addr, s1, sizeof(s1)), yyvsp[0].addrmask.mask);
	    v_acl->acl_next = v->uv_acl;
	    v->uv_acl = v_acl;
	}
break;
case 47:
//#line 428 "cfparse.y"
{
	    warn("Expected boundary spec after boundary keyword, ignored");
	}
break;
case 48:
//#line 432 "cfparse.y"
{
	    v->uv_flags |= VIFF_REXMIT_PRUNES;
	}
break;
case 49:
//#line 436 "cfparse.y"
{
	    if (yyvsp[0].num)
		v->uv_flags |= VIFF_REXMIT_PRUNES;
	    else
		v->uv_flags &= ~VIFF_REXMIT_PRUNES;
	}
break;
case 50:
//#line 443 "cfparse.y"
{
	    v->uv_flags |= VIFF_PASSIVE;
	}
break;
case 51:
//#line 447 "cfparse.y"
{
	    v->uv_flags |= VIFF_NOFLOOD;
	}
break;
case 52:
//#line 451 "cfparse.y"
{
	    v->uv_flags |= VIFF_NOTRANSIT;
	}
break;
case 53:
//#line 455 "cfparse.y"
{
	    v->uv_flags |= VIFF_BLASTER;
	    blaster_alloc(v - uvifs);
	}
break;
case 54:
//#line 460 "cfparse.y"
{
		    v->uv_flags |= VIFF_ALLOW_NONPRUNERS;
	}
break;
case 55:
//#line 464 "cfparse.y"
{
	    if (yyvsp[0].num < MIN_PRUNE_LIFETIME)
		warn("prune_lifetime %d must be at least %d", yyvsp[0].num, MIN_PRUNE_LIFETIME);
	    else
		v->uv_prune_lifetime = yyvsp[0].num;
	}
break;
case 56:
//#line 471 "cfparse.y"
{
	    if (v->uv_filter == NULL) {
		struct vif_filter *v_filter;

		v_filter = (struct vif_filter *)malloc(sizeof(struct vif_filter));
		if (v_filter == NULL)
		    fatal("out of memory");

		v_filter->vf_flags = 0;
		v_filter->vf_type = VFT_ACCEPT;
		v_filter->vf_filter = yyvsp[0].filterelem;
		v->uv_filter = v_filter;
	    } else if (v->uv_filter->vf_type != VFT_ACCEPT) {
		fatal("Cannot accept and deny");
	    } else {
		struct vf_element *p;

		p = v->uv_filter->vf_filter;
		while (p->vfe_next)
		    p = p->vfe_next;
		p->vfe_next = yyvsp[0].filterelem;
	    }
	}
break;
case 57:
//#line 495 "cfparse.y"
{
	    warn("Expected filter spec after accept keyword, ignored");
	}
break;
case 58:
//#line 499 "cfparse.y"
{
	    if (v->uv_filter == NULL) {
		struct vif_filter *v_filter;

		v_filter = (struct vif_filter *)malloc(sizeof(struct vif_filter));
		if (v_filter == NULL)
		    fatal("out of memory");

		v_filter->vf_flags = 0;
		v_filter->vf_type = VFT_DENY;
		v_filter->vf_filter = yyvsp[0].filterelem;
		v->uv_filter = v_filter;
	    } else if (v->uv_filter->vf_type != VFT_DENY) {
		fatal("Cannot accept and deny");
	    } else {
		struct vf_element *p;

		p = v->uv_filter->vf_filter;
		while (p->vfe_next)
		    p = p->vfe_next;
		p->vfe_next = yyvsp[0].filterelem;
	    }
	}
break;
case 59:
//#line 523 "cfparse.y"
{
		warn("Expected filter spec after deny keyword, ignored");
	}
break;
case 60:
//#line 527 "cfparse.y"
{
	    if (v->uv_filter == NULL)
		fatal("bidir goes after filters");
	    v->uv_filter->vf_flags |= VFF_BIDIR;
	}
break;
case 61:
//#line 535 "cfparse.y"
{
	    yyval.addr = yyvsp[0].addr;
	}
break;
case 62:
//#line 539 "cfparse.y"
{
	    yyval.addr = valid_if(yyvsp[0].ptr);
	    if (yyval.addr == 0)
		fatal("Invalid interface name %s",yyvsp[0].ptr);
	}
break;
case 63:
//#line 547 "cfparse.y"
{
	    yyval.addr = yyvsp[0].addr;
	}
break;
case 64:
//#line 551 "cfparse.y"
{
	    struct hostent *hp;

	    if ((hp = gethostbyname(yyvsp[0].ptr)) == NULL || hp->h_length != sizeof(yyval.addr))
		fatal("No such host %s", yyvsp[0].ptr);

	    if (hp->h_addr_list[1])
		fatal("Hostname %s does not %s", yyvsp[0].ptr, "map to a unique address");

	    memmove (&yyval.addr, hp->h_addr_list[0], hp->h_length);
	}
break;
case 65:
//#line 564 "cfparse.y"
{
	    yyval.addrmask = yyvsp[0].addrmask;
	}
break;
case 66:
//#line 568 "cfparse.y"
{
	    int i;

	    for (i=0; i < numbounds; i++) {
		if (!strcmp(boundlist[i].name, yyvsp[0].ptr)) {
		    yyval.addrmask = boundlist[i].bound;
		    break;
		}
	    }

	    if (i == numbounds)
		fatal("Invalid boundary name %s",yyvsp[0].ptr);
	}
break;
case 67:
//#line 584 "cfparse.y"
{
#ifdef ALLOW_BLACK_HOLES
	    if (!allow_black_holes)
#endif
		if ((ntohl(yyvsp[0].addrmask.addr) & 0xff000000) != 0xef000000) {
		    fatal("Boundaries must be 239.x.x.x, not %s/%d",
			  inet_fmt(yyvsp[0].addrmask.addr, s1, sizeof(s1)), yyvsp[0].addrmask.mask);
		}
	    yyval.addrmask = yyvsp[0].addrmask;
	}
break;
case 68:
//#line 596 "cfparse.y"
{ yyval.addrmask = yyvsp[0].addrmask; }
break;
case 69:
//#line 597 "cfparse.y"
{ yyval.addrmask.addr = yyvsp[0].addr; yyval.addrmask.mask = 0; }
break;
case 70:
//#line 600 "cfparse.y"
{ yyval.filterelem = yyvsp[0].filterelem; }
break;
case 71:
//#line 601 "cfparse.y"
{ fatal("named filters no implemented yet"); }
break;
case 72:
//#line 604 "cfparse.y"
{ yyval.filterelem = yyvsp[0].filterelem; }
break;
case 73:
//#line 605 "cfparse.y"
{ yyvsp[-1].filterelem->vfe_next = yyvsp[0].filterelem; yyval.filterelem = yyvsp[-1].filterelem; }
break;
case 74:
//#line 608 "cfparse.y"
{ yyval.filterelem = yyvsp[0].filterelem; }
break;
case 75:
//#line 609 "cfparse.y"
{ yyvsp[-1].filterelem->vfe_flags |= VFEF_EXACT; yyval.filterelem = yyvsp[-1].filterelem; }
break;
case 76:
//#line 613 "cfparse.y"
{
	    struct vf_element *vfe;

	    vfe = (struct vf_element *)malloc(sizeof(struct vf_element));
	    if (vfe == NULL)
		fatal("out of memory");

	    vfe->vfe_addr = yyvsp[0].addrmask.addr;
	    VAL_TO_MASK(vfe->vfe_mask, yyvsp[0].addrmask.mask);
	    vfe->vfe_flags = 0;
	    vfe->vfe_next = NULL;

	    yyval.filterelem = vfe;
	}
break;
//#line 1568 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    return (1);

yyaccept:
    return (0);
}
