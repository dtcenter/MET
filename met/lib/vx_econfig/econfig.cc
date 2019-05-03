/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse econfigparse
#define yylex   econfiglex
#define yyerror econfigerror
#define yylval  econfiglval
#define yychar  econfigchar
#define yydebug econfigdebug
#define yynerrs econfignerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ID = 258,
     INTEGER = 259,
     FLOAT = 260,
     QUOTED_STRING = 261,
     PRINT = 262,
     UMINUS = 263
   };
#endif
/* Tokens.  */
#define ID 258
#define INTEGER 259
#define FLOAT 260
#define QUOTED_STRING 261
#define PRINT 262
#define UMINUS 263




/* Copy the first part of user declarations.  */
#line 3 "econfig.y"



////////////////////////////////////////////////////////////////////////


#define YYDEBUG 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_econfig/scanner_stuff.h"

#include "vx_econfig/builtin.h"
#include "vx_econfig/idstack.h"
#include "vx_econfig/icode.h"
#include "vx_econfig/symtab.h"
#include "vx_econfig/pwl.h"
#include "vx_econfig/machine.h"
#include "vx_econfig/celltype_to_string.h"


////////////////////////////////////////////////////////////////////////



   //
   //  declarations that have external linkage
   //


extern int            yylex();

extern void           yyerror(char *);

extern "C" int        econfigwrap();




extern char *         econfigtext;

extern Machine *      bison_machine;

extern const char *   bison_input_filename;


   //
   //  definitions that have external linkage
   //


int econfig_LineNumber                      = 1;

int econfig_column                          = 1;

Machine *     bison_machine         = (Machine *) 0;

const char *  bison_input_filename  = (const char *) 0;


////////////////////////////////////////////////////////////////////////


   //
   //  static objects
   //


static  ICVStack         icvs;

static  IdentifierArray  ida;

static  PiecewiseLinear  pee_ell;


   //
   //  static functions
   //


static void  do_paren_exp          ();

static void  do_negate             ();

static void  do_op                 (char op);

static void  add_point             ();




static void  do_variable_def       (const char *);

static void  do_pwl_def            (const char *);

static void  do_function_def       (const char *);



static void  do_function_call      (const char *);


static void  add_id                (const char *);


static void  do_number             (const Number &);
static void  do_id                 (const char *);

static void  mark                  (int);

static void  do_array_def_1        (const char *);
static void  do_array_def_2        (const char *);
static void  do_array_exp          (const char *);

static void  do_element_assign     (const char *);

static void  do_print              ();

static void  string_to_expr        (const char *);


////////////////////////////////////////////////////////////////////////




/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 144 "econfig.y"
{

   char text[1024];

   Number num;

}
/* Line 187 of yacc.c.  */
#line 267 "temp000.cc"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 280 "temp000.cc"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  20
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   139

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  23
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  41
/* YYNRULES -- Number of states.  */
#define YYNSTATES  85

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   263

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      16,    17,    10,     8,    20,     9,     2,    11,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    14,
       2,    15,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    18,     2,    19,    12,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    21,     2,    22,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,    13
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    16,    21,    26,
      34,    35,    45,    53,    59,    61,    65,    67,    69,    73,
      77,    81,    85,    89,    92,    96,    97,   103,   106,   108,
     110,   114,   115,   119,   121,   124,   126,   128,   129,   134,
     136,   139
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      24,     0,    -1,    25,    -1,    24,    25,    -1,    27,    -1,
      26,    -1,     7,    30,    14,    -1,     3,    15,    30,    14,
      -1,     3,    15,    36,    14,    -1,     3,    16,    29,    17,
      15,    30,    14,    -1,    -1,     3,    18,    19,    28,    15,
      18,    32,    19,    14,    -1,     3,    34,    15,    18,    32,
      19,    14,    -1,     3,    34,    15,    30,    14,    -1,     3,
      -1,    29,    20,     3,    -1,    35,    -1,     3,    -1,    30,
       8,    30,    -1,    30,     9,    30,    -1,    30,    10,    30,
      -1,    30,    11,    30,    -1,    30,    12,    30,    -1,     9,
      30,    -1,    16,    30,    17,    -1,    -1,     3,    16,    31,
      32,    17,    -1,     3,    34,    -1,     6,    -1,    30,    -1,
      32,    20,    30,    -1,    -1,    18,    30,    19,    -1,    33,
      -1,    34,    33,    -1,     4,    -1,     5,    -1,    -1,    21,
      37,    38,    22,    -1,    39,    -1,    38,    39,    -1,    16,
      30,    20,    30,    17,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   173,   173,   174,   178,   179,   184,   187,   188,   189,
     190,   190,   191,   192,   196,   197,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   210,   211,   212,   216,
     217,   218,   222,   226,   227,   231,   232,   237,   237,   241,
     242,   246
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ID", "INTEGER", "FLOAT",
  "QUOTED_STRING", "PRINT", "'+'", "'-'", "'*'", "'/'", "'^'", "UMINUS",
  "';'", "'='", "'('", "')'", "'['", "']'", "','", "'{'", "'}'", "$accept",
  "statement_list", "statement", "print_statement", "assignment", "@1",
  "id_list", "expression", "@2", "expression_list", "bracket_exp",
  "bracket_exp_list", "number", "pwl", "@3", "point_list", "point", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,    43,    45,
      42,    47,    94,   263,    59,    61,    40,    41,    91,    93,
      44,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    23,    24,    24,    25,    25,    26,    27,    27,    27,
      28,    27,    27,    27,    29,    29,    30,    30,    30,    30,
      30,    30,    30,    30,    30,    31,    30,    30,    30,    32,
      32,    32,    33,    34,    34,    35,    35,    37,    36,    38,
      38,    39
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     3,     4,     4,     7,
       0,     9,     7,     5,     1,     3,     1,     1,     3,     3,
       3,     3,     3,     2,     3,     0,     5,     2,     1,     1,
       3,     0,     3,     1,     2,     1,     1,     0,     4,     1,
       2,     5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     2,     5,     4,     0,     0,     0,
      33,     0,    17,    35,    36,    28,     0,     0,     0,    16,
       1,     3,    37,     0,     0,    14,     0,    10,     0,     0,
       0,    34,    25,    27,    23,     0,     0,     0,     0,     0,
       0,     6,     0,     7,     8,     0,     0,     0,    32,    31,
       0,    31,    24,    18,    19,    20,    21,    22,     0,     0,
      39,     0,    15,     0,    29,     0,    13,     0,     0,    38,
      40,     0,    31,     0,     0,    26,     0,     9,     0,    12,
      30,     0,     0,    41,    11
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    47,    26,    64,    51,    65,
      10,    11,    19,    24,    42,    59,    60
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -49
static const yytype_int8 yypact[] =
{
      -1,    28,    60,    51,   -49,   -49,   -49,    36,     1,    13,
     -49,    15,    55,   -49,   -49,   -49,    60,    60,    94,   -49,
     -49,   -49,   -49,   101,     7,   -49,    50,   -49,    69,    44,
      60,   -49,   -49,    -5,   -49,    74,    60,    60,    60,    60,
      60,   -49,    10,   -49,   -49,    40,    58,    53,   -49,    60,
     108,    60,   -49,    87,    87,    63,    63,   -49,    60,     9,
     -49,    60,   -49,    71,   122,   116,   -49,    70,     0,   -49,
     -49,   115,    60,    86,    60,   -49,    60,   -49,   118,   -49,
     122,    84,    93,   -49,   -49
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -49,   -49,   111,   -49,   -49,   -49,   -49,    -2,   -49,   -48,
     -10,   109,   -49,   -49,   -49,   -49,    80
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      18,    31,     1,    67,    25,    23,     2,    28,    36,    37,
      38,    39,    40,    30,    34,    35,    12,    13,    14,    15,
      76,    44,    16,    31,    78,    58,    58,    50,    28,    17,
      29,    69,    27,    30,    53,    54,    55,    56,    57,    12,
      13,    14,    15,     7,     8,    16,     9,    12,    13,    14,
      15,    20,    17,    16,     1,    61,    68,    22,     2,    71,
      17,    62,    49,    12,    13,    14,    15,    45,    63,    16,
      46,    32,    80,    30,    81,    40,    17,    36,    37,    38,
      39,    40,    36,    37,    38,    39,    40,    75,    48,    72,
      74,    52,    36,    37,    38,    39,    40,    38,    39,    40,
      79,    83,    36,    37,    38,    39,    40,    84,    41,    36,
      37,    38,    39,    40,    21,    43,    36,    37,    38,    39,
      40,    33,    66,    36,    37,    38,    39,    40,     0,    77,
      36,    37,    38,    39,    40,    73,    74,    82,    74,    70
};

static const yytype_int8 yycheck[] =
{
       2,    11,     3,    51,     3,     7,     7,     9,     8,     9,
      10,    11,    12,    18,    16,    17,     3,     4,     5,     6,
      20,    14,     9,    33,    72,    16,    16,    29,    30,    16,
      15,    22,    19,    18,    36,    37,    38,    39,    40,     3,
       4,     5,     6,    15,    16,     9,    18,     3,     4,     5,
       6,     0,    16,     9,     3,    15,    58,    21,     7,    61,
      16,     3,    18,     3,     4,     5,     6,    17,    15,     9,
      20,    16,    74,    18,    76,    12,    16,     8,     9,    10,
      11,    12,     8,     9,    10,    11,    12,    17,    19,    18,
      20,    17,     8,     9,    10,    11,    12,    10,    11,    12,
      14,    17,     8,     9,    10,    11,    12,    14,    14,     8,
       9,    10,    11,    12,     3,    14,     8,     9,    10,    11,
      12,    12,    14,     8,     9,    10,    11,    12,    -1,    14,
       8,     9,    10,    11,    12,    19,    20,    19,    20,    59
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     7,    24,    25,    26,    27,    15,    16,    18,
      33,    34,     3,     4,     5,     6,     9,    16,    30,    35,
       0,    25,    21,    30,    36,     3,    29,    19,    30,    15,
      18,    33,    16,    34,    30,    30,     8,     9,    10,    11,
      12,    14,    37,    14,    14,    17,    20,    28,    19,    18,
      30,    31,    17,    30,    30,    30,    30,    30,    16,    38,
      39,    15,     3,    15,    30,    32,    14,    32,    30,    22,
      39,    30,    18,    19,    20,    17,    20,    14,    32,    14,
      30,    30,    19,    17,    14
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 178 "econfig.y"
    { }
    break;

  case 5:
#line 179 "econfig.y"
    { }
    break;

  case 6:
#line 184 "econfig.y"
    { do_print(); }
    break;

  case 7:
#line 187 "econfig.y"
    { do_variable_def ((yyvsp[(1) - (4)].text));     }
    break;

  case 8:
#line 188 "econfig.y"
    { do_pwl_def      ((yyvsp[(1) - (4)].text));     }
    break;

  case 9:
#line 189 "econfig.y"
    { do_function_def ((yyvsp[(1) - (7)].text));     }
    break;

  case 10:
#line 190 "econfig.y"
    { mark('b'); }
    break;

  case 11:
#line 190 "econfig.y"
    { do_array_def_2  ((yyvsp[(1) - (9)].text));     }
    break;

  case 12:
#line 191 "econfig.y"
    { do_array_def_1  ((yyvsp[(1) - (7)].text));     }
    break;

  case 13:
#line 192 "econfig.y"
    { do_element_assign((yyvsp[(1) - (5)].text));    }
    break;

  case 14:
#line 196 "econfig.y"
    { add_id((yyvsp[(1) - (1)].text)); }
    break;

  case 15:
#line 197 "econfig.y"
    { add_id((yyvsp[(3) - (3)].text)); }
    break;

  case 16:
#line 201 "econfig.y"
    { do_number((yyvsp[(1) - (1)].num)); }
    break;

  case 17:
#line 202 "econfig.y"
    { do_id((yyvsp[(1) - (1)].text)); }
    break;

  case 18:
#line 203 "econfig.y"
    { do_op('+'); }
    break;

  case 19:
#line 204 "econfig.y"
    { do_op('-'); }
    break;

  case 20:
#line 205 "econfig.y"
    { do_op('*'); }
    break;

  case 21:
#line 206 "econfig.y"
    { do_op('/'); }
    break;

  case 22:
#line 207 "econfig.y"
    { do_op('^'); }
    break;

  case 23:
#line 208 "econfig.y"
    { do_negate(); }
    break;

  case 24:
#line 209 "econfig.y"
    { do_paren_exp(); }
    break;

  case 25:
#line 210 "econfig.y"
    { mark(0); }
    break;

  case 26:
#line 210 "econfig.y"
    { do_function_call((yyvsp[(1) - (5)].text)); }
    break;

  case 27:
#line 211 "econfig.y"
    { do_array_exp((yyvsp[(1) - (2)].text)); }
    break;

  case 28:
#line 212 "econfig.y"
    { string_to_expr((yyvsp[(1) - (1)].text)); }
    break;

  case 29:
#line 216 "econfig.y"
    { }
    break;

  case 30:
#line 217 "econfig.y"
    { }
    break;

  case 31:
#line 218 "econfig.y"
    { }
    break;

  case 32:
#line 222 "econfig.y"
    { mark('b'); }
    break;

  case 33:
#line 226 "econfig.y"
    { }
    break;

  case 34:
#line 227 "econfig.y"
    { }
    break;

  case 35:
#line 231 "econfig.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 36:
#line 232 "econfig.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 37:
#line 237 "econfig.y"
    { pee_ell.clear(); }
    break;

  case 38:
#line 237 "econfig.y"
    { }
    break;

  case 39:
#line 241 "econfig.y"
    { }
    break;

  case 40:
#line 242 "econfig.y"
    { }
    break;

  case 41:
#line 246 "econfig.y"
    { add_point(); }
    break;


/* Line 1267 of yacc.c.  */
#line 1738 "temp000.cc"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 251 "econfig.y"



////////////////////////////////////////////////////////////////////////


void do_paren_exp()

{

   //
   //  nothing to do here!
   //

return;

}


////////////////////////////////////////////////////////////////////////


static void do_negate()

{

IcodeVector v;
IcodeCell cell;

cell.type = op_negate;

v = icvs.pop();

v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_op(char op)

{

IcodeCell cell;
IcodeVector L, R;


R = icvs.pop();
L = icvs.pop();


switch ( op )  {


   case '+':  cell.type = op_add;       break;
   case '-':  cell.type = op_subtract;  break;


   case '*':  cell.type = op_multiply;  break;
   case '/':  cell.type = op_divide;    break;


   case '^':  
      if ( (R.length() == 1) && (R[0].type == integer) && (R[0].val == 2) )   cell.type = op_square;
      else                                                                    cell.type = op_power;
      break;


   default:
      cerr << "\n\n  do_op() -> unrecognized op ... \"" << op << "\"\n\n";
      exit ( 1 );
      break; 


}   //  switch



if ( cell.type != op_square )   L.add(R);

L.add(cell);

icvs.push(L);

return;

}


////////////////////////////////////////////////////////////////////////


void add_point()

{


double x, y;
IcodeCell cell;
IcodeVector X, Y;


Y = icvs.pop();
X = icvs.pop();

   //
   //  get x
   //

bison_machine->run( X );

if ( bison_machine->depth() > 1 )  {

   cerr << "\n\n  add_point() -> bad \"x\" expression!\n\n";

   exit ( 1 );

}

cell = bison_machine->pop();

x = cell.as_double();


   //
   //  get y
   //

bison_machine->run( Y );

if ( bison_machine->depth() > 1 )  {

   cerr << "\n\n  add_point() -> bad \"y\" expression!\n\n";

   exit ( 1 );

}

cell = bison_machine->pop();

y = cell.as_double();

   //
   //  add point
   //

pee_ell.add_point(x, y);


// cout << "   added point (" << x << ", " << y << ")\n" << flush;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_pwl_def(const char * Name)

{

SymbolTableEntry e;


pee_ell.set_name(Name);


e.set_pwl(Name, pee_ell);


bison_machine->store(e);


pee_ell.clear();


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_function_call (const char * function_name)

{

int index;
int count;
IcodeVector v;
IcodeCell cell;
ICVStack s;


count = 0;

while ( 1 )  {

   v = icvs.pop();

   if ( v.is_mark() )  break;

   s.push(v);

   ++count;

}

v.clear();

while ( s.depth() > 0 )  {

   v.add(s.pop());

}


if ( is_builtin(function_name, index) )  { 

   cell.set_builtin(index);

} else {

   cell.set_identifier(function_name);

}



v.add(cell);

icvs.push(v);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_variable_def(const char * Name)

{

SymbolTableEntry e;
IcodeVector v;

v = icvs.pop();

e.set_variable(Name, v);

bison_machine->store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void do_function_def(const char * func_name)

{

int index;

if ( is_builtin(func_name, index) )  {

   cerr << "\n\n  do_function_def() -> attempt to redefine built-in function \"" << func_name << "\"\n\n";

   exit ( 1 );

}

SymbolTableEntry e;
IcodeVector v;

v = icvs.pop();


e.set_function(func_name, ida, v);

bison_machine->store(e);

ida.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void add_id(const char * text)

{

Identifier id;

id.set(text);

ida.add(id);


return;

}


////////////////////////////////////////////////////////////////////////


void do_number(const Number & number)

{

IcodeVector v;
IcodeCell cell;

if ( number.is_int )  {

   cell.set_integer(number.i);

} else {

   cell.set_double (number.d);

}

v.add(cell);


icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_id(const char * text)

{

IcodeVector v;
IcodeCell cell;

cell.set_identifier(text);

v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void mark(int k)

{

IcodeVector v;
IcodeCell cell;


cell.type = cell_mark;

cell.val = k;

v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_array_def_1(const char * name)

{

int j, d, k;
ICVStack s;
int sizes[max_array_dim];
IcodeVector v;
IcodeVector zero;
IcodeCell cell;
ArrayInfo * ai = (ArrayInfo *) 0;
SymbolTableEntry e;




memset(sizes, 0, sizeof(sizes));

   //
   //  get all the array elements
   //

while ( 1 )  {

   IcodeVector & vp = *(icvs.peek());

   if ( (vp.is_mark()) && (vp[0].val == 'b') ) break;

   v = icvs.pop();

   s.push(v);

}

// icvs.write_to_screen(0);

   //
   //  get the array sizes and dimension
   //

d = 0;

while ( 1 )  {

   if ( d >= max_array_dim )  {

      cerr << "\n\n  do_array_def_1(const char *) -> array dimension limit exceeded\n\n";

      exit ( 1 );

   }

   if ( icvs.depth() == 0 )  break;

   v = icvs.pop();

   if ( !(v.is_mark()) || !(v[0].val == 'b') ) { icvs.push(v);  break; }

   v = icvs.pop();

   bison_machine->run(v);
   
   cell = bison_machine->pop();

   if ( !(cell.is_numeric()) )  {

      cerr << "\n\n  do_array_def_1(const char *) -> non-numeric array size\n\n";

      exit ( 1 );

   }

   if ( cell.type == integer )  k = cell.val;
   if ( cell.type == floating_point )  k = my_nint(cell.d);

   if ( k <= 0 )  {

      cerr << "\n\n  do_array_def_1(const char *) -> bad array size ... " << k << "\n\n";

      exit ( 1 );

   }

   sizes[max_array_dim - 1 - d] = k;

   ++d;

}


// cout << "d = " << d << "\n";


for (j=0; j<d; ++j)  {

   sizes[j] = sizes[max_array_dim - d + j];

}

ai = new ArrayInfo;

if ( !ai )  {

   cerr << "\n\n  do_array_def_1(const char *) -> memory allocation error\n\n";

   exit ( 1 );

}

ai->set(sizes, d);

e.type = ste_array;

e.ai = ai;

e.set_name(name);

   //
   //  fill the entries of the array
   //

cell.set_integer(0);

zero.add(cell);

for (j=0; j<(ai->n_alloc()); ++j)  {

   if ( s.depth() > 0 )  {

      v = s.pop();

      ai->put(j, v);

   } else {

      ai->put(j, zero);

   }

}


   //
   //  store it
   //

bison_machine->store(e);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_array_def_2(const char * name)

{

int j, d;
ICVStack s;
int sizes[max_array_dim];
IcodeVector v;
IcodeVector zero;
IcodeCell cell;
ArrayInfo * ai = (ArrayInfo *) 0;
SymbolTableEntry e;




   //
   //  get all the array elements
   //

while ( 1 )  {

   IcodeVector & vp = *(icvs.peek());

   if ( (vp.is_mark()) && (vp[0].val == 'b') ) break;

   v = icvs.pop();

   s.push(v);

}


v = icvs.pop();   //  pop off the mark

d = 1;

sizes[0] = s.depth();

ai = new ArrayInfo;

if ( !ai )  {

   cerr << "\n\n  do_array_def_1(const char *) -> memory allocation error\n\n";

   exit ( 1 );

}

ai->set(sizes, d);

e.type = ste_array;

e.ai = ai;

e.set_name(name);

   //
   //  fill the entries of the array
   //

cell.set_integer(0);

zero.add(cell);

for (j=0; j<(ai->n_alloc()); ++j)  {

   if ( s.depth() > 0 )  {

      v = s.pop();

      ai->put(j, v);

   } else {

      ai->put(j, zero);

   }

}

   //
   //  store it
   //

bison_machine->store(e);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_array_exp(const char * name)

{

SymbolTableEntry * e = (SymbolTableEntry *) 0;


e = bison_machine->find(name);

if ( !e )  {

   cerr << "\n\n  void do_array_exp(const char *) -> unidentified name used as array\n\n";

   exit ( 1 );

}

if ( e->type != ste_array )  {

   cerr << "\n\n  void do_array_exp(const char *) -> non-array name \"" << name << "\" used as array\n\n";

   exit ( 1 );

}

int j;
IcodeVector v, vv;
IcodeCell cell;
ICVStack s;



for (j=0; j<(e->ai->dim()); ++j)  {

   v = icvs.pop();   //  should be mark 'b' ... just toss it

   v = icvs.pop();

   s.push(v);

}

v.clear();

while ( s.depth() > 0 )  {

   vv = s.pop();

   v.add(vv);

}


cell.set_identifier(name);


v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_element_assign(const char * name)

{

int j, k, dim;
SymbolTableEntry * e = (SymbolTableEntry *) 0;
ICVStack s;
IcodeVector v, RHS;
IcodeCell cell;
int indices[max_array_dim];


e = bison_machine->find(name);

if ( !e )  {

   cerr << "\n\n  do_element_assign(const char *) -> can't find symbol table entry for name \"" << name << "\"\n\n";

   exit ( 1 );

}

if ( e->type != ste_array )  {

   cerr << "\n\n  do_element_assign(const char *) -> name \"" << name << "\" is not an array!\n\n";

   exit ( 1 );

}

dim = e->ai->dim();

   //
   //  pop the right-hand-side expression off the stack
   //

RHS = icvs.pop();

   //
   //  pop the array indices off the stack
   //

for (j=0; j<dim; ++j)  {

   v = icvs.pop();

   if ( (v.is_mark()) && (v[0].val == 'b') )  {

      v = icvs.pop();

      s.push(v);

   } else {

      cerr << "\n\n  do_element_assign(const char *) -> expected mark not found\n\n";

      exit ( 1 );

   }

}

   //
   //  evaluate the indices
   //

for (j=0; j<dim; ++j)  {

   v = s.pop();

   bison_machine->run(v);

   cell = bison_machine->pop();

   if ( !(cell.is_numeric()) )  {

      cerr << "\n\n  do_element_assign(const char *) -> non-numeric value for index " << j << "\n\n";

      exit ( 1 );

   }

   if ( cell.type == integer )  k = cell.val;
   else                         k = my_nint(cell.d);

   indices[j] = k;

}

   //
   //  range-check the indices
   //

for (j=0; j<dim; ++j)  {

   if ( (indices[j] < 0) || (indices[j] >= e->ai->size(j)) )  {

      cerr << "\n\n  do_element_assign(const char *) -> range-check error on value for index " << j << "\n\n";

      exit ( 1 );


   }

}

   //
   //  store
   //

e->ai->put(indices, RHS);

   //
   //  done
   //

// cerr << "\n\n  void do_element_assign() -> not yet implemented!\n\n";
// 
// exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void do_print()

{

IcodeVector v;
IcodeCell cell;
char junk[1024];


v = icvs.pop();


bison_machine->run(v);

cell = bison_machine->pop();

switch ( cell.type )  {

   case integer:
      cout << (cell.val) << "\n";
      break;

   case floating_point:
      cout << (cell.d) << "\n";
      break;

   case identifier:
      cout << (cell.name) << "\n";
      break;

   case character_string:
      cout << "\"" << (cell.text) << "\"\n";
      break;


   default:
      cerr << "\n\n  do_print() -> unrecognized cell type: ";
      celltype_to_string(cell.type, junk);
      cerr << junk << "\n\n";
      exit ( 1 );
      break;

}   //  switch




   //
   // done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void string_to_expr(const char * text)

{

IcodeVector v;
IcodeCell cell;

cell.set_string(text);

v.add(cell);

icvs.push(v);



return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  standard yacc stuff
   //


////////////////////////////////////////////////////////////////////////


void yyerror(char * s)

{

int j, j1, j2;
int line_len, text_len;
int c;
char line[512];
ifstream in;


c = (int) (econfig_column - strlen(econfigtext));

cout << "\n\n"
     << "   config() -> syntax error in file \"" << bison_input_filename << "\"\n\n"
     << "      line   = " << econfig_LineNumber << "\n\n"
     << "      econfig_column = " << c << "\n\n"
     << "      text   = \"" << econfigtext << "\"\n\n";

in.open(bison_input_filename);

for (j=1; j<econfig_LineNumber; ++j)  {   //  j starts at one here, not zero

   in.getline(line, sizeof(line));

}

in.getline(line, sizeof(line));

in.close();




cout << "\n\n"
     << line
     << "\n";

line_len = strlen(line);

text_len = strlen(econfigtext);

j1 = c;
j2 = c + text_len - 1;


for (j=1; j<=line_len; ++j)  {   //  j starts a one here, not zero

   if ( (j >= j1) && (j <= j2) )  cout.put('^');
   else                           cout.put('_');

}





cout << "\n\n";

cout.flush();

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


int econfigwrap()

{

return ( 1 );

}


////////////////////////////////////////////////////////////////////////











