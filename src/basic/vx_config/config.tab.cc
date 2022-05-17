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
#define yyparse configparse
#define yylex   configlex
#define yyerror configerror
#define yylval  configlval
#define yychar  configchar
#define yydebug configdebug
#define yynerrs confignerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     QUOTED_STRING = 259,
     INTEGER = 260,
     FLOAT = 261,
     BOOLEAN = 262,
     COMPARISON = 263,
     NA_COMPARISON = 264,
     LOGICAL_OP_NOT = 265,
     LOGICAL_OP_AND = 266,
     LOGICAL_OP_OR = 267,
     FORTRAN_THRESHOLD = 268,
     BUILTIN = 269,
     LOCAL_VAR = 270,
     SIMPLE_PERC_THRESH = 271,
     USER_FUNCTION = 272,
     PRINT = 273,
     UNARY_MINUS = 274
   };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define QUOTED_STRING 259
#define INTEGER 260
#define FLOAT 261
#define BOOLEAN 262
#define COMPARISON 263
#define NA_COMPARISON 264
#define LOGICAL_OP_NOT 265
#define LOGICAL_OP_AND 266
#define LOGICAL_OP_OR 267
#define FORTRAN_THRESHOLD 268
#define BUILTIN 269
#define LOCAL_VAR 270
#define SIMPLE_PERC_THRESH 271
#define USER_FUNCTION 272
#define PRINT 273
#define UNARY_MINUS 274




/* Copy the first part of user declarations.  */
#line 3 "config.tab.yy"


////////////////////////////////////////////////////////////////////////


#define YYDEBUG 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "math_constants.h"
#include "is_bad_data.h"
#include "scanner_stuff.h"
#include "threshold.h"
#include "is_number.h"
#include "concat_string.h"
#include "pwl.h"
#include "dictionary.h"
#include "icode.h"
#include "idstack.h"
#include "calculator.h"
#include "fix_float.h"

#include "scanner_stuff.h"
#include "threshold.h"


////////////////////////////////////////////////////////////////////////


   //
   //  declarations that have external linkage
   //


extern int            yylex();

extern void           yyerror(const char *);

extern "C" int        configwrap();




   //
   //  definitions that have external linkage
   //

char *         configtext;

FILE *         configin;


int               LineNumber            = 1;

int               Column                = 1;

const char *      bison_input_filename  = (const char *) 0;

DictionaryStack * dict_stack            = (DictionaryStack *) 0;

bool              is_lhs                = true;    //  used by the scanner

ThreshNode *      result                = 0;   //  for testing

bool              test_mode             = false;

char number_string [max_id_length + 1];

IdentifierArray  ida;

bool is_function_def = false;

Calculator hp;


////////////////////////////////////////////////////////////////////////


   //
   //  static definitions
   //

static PiecewiseLinear pwl;

static Dictionary DD;

static SingleThresh STH;

static const char default_print_prefix [] = "config";


static ICVStack         icvs;



static ConcatString function_name;

static const char apm = 'b';   //  assign_prefix mark
static const char fcm = 'f';   //  function def mark



////////////////////////////////////////////////////////////////////////


   //
   //  static declarations
   //

static void do_op(char op);

static Number do_integer_op(char op, const Number & a, const Number & b);

static void do_negate();
static void do_paren_exp();

static void do_builtin_call(int which);


static void do_assign_boolean   (const char * name, bool);
static void do_assign_exp       (const char * name);
static void do_assign_string    (const char * name, const char * text);
static void do_assign_threshold (const char * name);

static void do_assign_id      (const char * LHS, const char * RHS);

static void do_assign_dict    (const char * name);

static void do_assign_exp_array(const char * name);

static void do_dict();

static void do_string(const char *);

static void do_boolean(const bool &);

static void store_exp();

static void do_thresh(ThreshNode *);

static void do_na_thresh();

static void add_point();

static void do_pwl(const char * LHS);

static void do_number(const Number &);

static void do_local_var(int);


static ThreshNode * do_and_thresh    (ThreshNode *, ThreshNode *);
static ThreshNode * do_or_thresh     (ThreshNode *, ThreshNode *);
static ThreshNode * do_not_thresh    (ThreshNode *);
static ThreshNode * do_paren_thresh  (ThreshNode *);
static ThreshNode * do_simple_thresh (ThreshType, const Number &);

static ThreshNode * do_simple_perc_thresh (const ThreshType, const PC_info &);
static ThreshNode * do_compound_perc_thresh (const ThreshType, const PC_info &, const Number &);
static ThreshNode * do_fortran_thresh(const char *);


static void set_number_string();
static void set_number_string(const char *);

static void mark(int);

static void do_user_function_call(const DictionaryEntry *);

static void do_print(const char *);

static void do_user_function_def();


////////////////////////////////////////////////////////////////////////




/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
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
#line 195 "config.tab.yy"
{

   char text[max_id_length + 1];

   Number nval;

   bool bval;

   int index;

   ThreshType cval;

   ThreshNode * node;

   const DictionaryEntry * entry;

   PC_info pc_info;

}
/* Line 193 of yacc.c.  */
#line 353 "config.tab.cc"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 366 "config.tab.cc"

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
# if defined YYENABLE_NLS && YYENABLE_NLS
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
#define YYFINAL  27
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   251

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  34
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  29
/* YYNRULES -- Number of rules.  */
#define YYNRULES  74
/* YYNRULES -- Number of states.  */
#define YYNSTATES  144

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   274

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      28,    29,    21,    19,    27,    20,     2,    22,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    25,
       2,    30,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    31,     2,    26,    23,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    32,     2,    33,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    24
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    14,    18,    24,
      26,    30,    34,    38,    42,    46,    50,    53,    58,    63,
      68,    73,    78,    82,    86,    88,    92,    98,   101,   102,
     106,   111,   113,   117,   119,   123,   125,   129,   131,   133,
     135,   139,   143,   146,   150,   153,   156,   162,   164,   166,
     168,   170,   174,   176,   177,   179,   180,   182,   184,   188,
     192,   196,   200,   204,   207,   211,   212,   218,   219,   225,
     227,   231,   235,   237,   240
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      35,     0,    -1,    36,    -1,    35,    36,    -1,    39,    -1,
      37,    -1,    49,    -1,    38,    56,    25,    -1,    38,     4,
      55,    56,    25,    -1,    18,    -1,    42,     7,    25,    -1,
      42,    56,    25,    -1,    42,     3,    25,    -1,    42,    60,
      25,    -1,    42,    49,    25,    -1,    42,     4,    25,    -1,
      42,    45,    -1,    43,    53,    26,    25,    -1,    43,    59,
      26,    25,    -1,    43,    47,    26,    25,    -1,    43,    48,
      26,    25,    -1,    43,    46,    26,    25,    -1,    43,    26,
      25,    -1,    41,    56,    25,    -1,     3,    -1,    40,    27,
       3,    -1,     3,    28,    40,    29,    30,    -1,     3,    30,
      -1,    -1,    42,    44,    31,    -1,    32,    35,    33,    54,
      -1,    45,    -1,    46,    27,    45,    -1,     4,    -1,    47,
      27,     4,    -1,    49,    -1,    48,    27,    49,    -1,    50,
      -1,     9,    -1,    51,    -1,    50,    11,    50,    -1,    50,
      12,    50,    -1,    10,    50,    -1,    28,    50,    29,    -1,
       8,    52,    -1,     8,    16,    -1,     8,    16,    28,    52,
      29,    -1,    13,    -1,     5,    -1,     6,    -1,     7,    -1,
      53,    27,     7,    -1,    25,    -1,    -1,    27,    -1,    -1,
      52,    -1,    15,    -1,    56,    19,    56,    -1,    56,    20,
      56,    -1,    56,    21,    56,    -1,    56,    22,    56,    -1,
      56,    23,    56,    -1,    20,    56,    -1,    28,    56,    29,
      -1,    -1,    14,    28,    57,    59,    29,    -1,    -1,    17,
      28,    58,    59,    29,    -1,    56,    -1,    59,    27,    56,
      -1,    28,    61,    29,    -1,    62,    -1,    61,    62,    -1,
      28,    56,    27,    56,    29,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   259,   259,   260,   263,   264,   265,   269,   270,   274,
     279,   280,   281,   282,   284,   285,   286,   288,   289,   290,
     291,   292,   293,   295,   301,   302,   306,   310,   314,   314,
     318,   322,   323,   327,   328,   332,   333,   336,   337,   341,
     342,   343,   344,   345,   349,   350,   351,   352,   356,   357,
     361,   362,   366,   367,   371,   372,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   385,   386,   386,   391,
     392,   396,   400,   401,   405
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "QUOTED_STRING", "INTEGER",
  "FLOAT", "BOOLEAN", "COMPARISON", "NA_COMPARISON", "LOGICAL_OP_NOT",
  "LOGICAL_OP_AND", "LOGICAL_OP_OR", "FORTRAN_THRESHOLD", "BUILTIN",
  "LOCAL_VAR", "SIMPLE_PERC_THRESH", "USER_FUNCTION", "PRINT", "'+'",
  "'-'", "'*'", "'/'", "'^'", "UNARY_MINUS", "';'", "']'", "','", "'('",
  "')'", "'='", "'['", "'{'", "'}'", "$accept", "statement_list",
  "statement", "print_stmt", "print_prefix", "assign_stmt", "id_list",
  "function_prefix", "assign_prefix", "array_prefix", "@1", "dictionary",
  "dictionary_list", "string_list", "threshold_list", "threshold",
  "thresh_node", "simple_thresh", "number", "boolean_list", "opt_semi",
  "opt_comma", "expression", "@2", "@3", "expression_list",
  "piecewise_linear", "point_list", "point", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,    43,
      45,    42,    47,    94,   274,    59,    93,    44,    40,    41,
      61,    91,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    34,    35,    35,    36,    36,    36,    37,    37,    38,
      39,    39,    39,    39,    39,    39,    39,    39,    39,    39,
      39,    39,    39,    39,    40,    40,    41,    42,    44,    43,
      45,    46,    46,    47,    47,    48,    48,    49,    49,    50,
      50,    50,    50,    50,    51,    51,    51,    51,    52,    52,
      53,    53,    54,    54,    55,    55,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    57,    56,    58,    56,    59,
      59,    60,    61,    61,    62
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     3,     5,     1,
       3,     3,     3,     3,     3,     3,     2,     4,     4,     4,
       4,     4,     3,     3,     1,     3,     5,     2,     0,     3,
       4,     1,     3,     1,     3,     1,     3,     1,     1,     1,
       3,     3,     2,     3,     2,     2,     5,     1,     1,     1,
       1,     3,     1,     0,     1,     0,     1,     1,     3,     3,
       3,     3,     3,     2,     3,     0,     5,     0,     5,     1,
       3,     3,     1,     2,     5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,    38,     0,    47,     9,     0,     0,     2,
       5,     0,     4,     0,    28,     0,     6,    37,    39,     0,
      27,    48,    49,    45,    44,    42,     0,     1,     3,    55,
       0,    57,     0,     0,     0,    56,     0,     0,     0,     0,
       0,     0,     0,     0,    16,     0,     0,     0,    33,    50,
       0,     0,    31,     0,     0,     0,    35,     0,    69,     0,
       0,     0,    24,     0,     0,    43,    54,     0,    65,    67,
      63,     0,     0,     0,     0,     0,     0,     7,    23,    12,
      15,    10,     0,     0,    72,     0,    29,    14,    11,    13,
      22,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    40,    41,     0,     0,     0,     0,     0,     0,    64,
      58,    59,    60,    61,    62,     0,     0,    71,    73,    53,
      21,    32,    19,    34,    20,    36,    17,    51,    18,    70,
      25,    26,    46,     8,     0,     0,     0,     0,    52,    30,
      66,    68,     0,    74
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     8,     9,    10,    11,    12,    63,    13,    14,    15,
      43,    44,    53,    54,    55,    16,    17,    18,    35,    57,
     139,    67,    58,   107,   108,    59,    47,    83,    84
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -17
static const yytype_int16 yypact[] =
{
      40,   -16,     3,   -17,   110,   -17,   -17,   110,    24,   -17,
     -17,   141,   -17,   189,    87,   122,   -17,    27,   -17,    12,
     -17,   -17,   -17,    -8,   -17,   -17,    17,   -17,   -17,    -2,
       7,   -17,    23,   189,   189,   -17,   177,   207,    45,    52,
      73,   157,    40,     0,   -17,    78,   214,    81,   -17,   -17,
      92,   173,   -17,    18,    28,    33,   -17,   107,   228,   133,
     110,   110,   -17,    58,   108,   -17,   -17,   189,   -17,   -17,
     -17,    53,   189,   189,   189,   189,   189,   -17,   -17,   -17,
     -17,   -17,   173,   147,   -17,     8,   -17,   -17,   -17,   -17,
     -17,    96,   112,   115,   145,   132,    71,   139,   159,   143,
     189,   -17,   -17,   170,   150,   153,   221,   189,   189,   -17,
     130,   130,   161,   161,   -17,   191,   189,   -17,   -17,   164,
     -17,   -17,   -17,   -17,   -17,   -17,   -17,   -17,   -17,   228,
     -17,   -17,   -17,   -17,    95,   114,   189,    89,   -17,   -17,
     -17,   -17,   202,   -17
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -17,   163,    -7,   -17,   -17,   -17,   -17,   -17,   -17,   -17,
     -17,    -9,   -17,   -17,   -17,   -10,     6,   -17,     5,   -17,
     -17,   -17,   -11,   -17,   -17,    84,   -17,   -17,   124
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      36,    28,    37,    46,    45,    56,    52,    24,    21,    22,
      25,     1,    19,    26,    20,    62,     2,     3,     4,    23,
      64,     5,    70,    71,    27,    66,     6,     1,    60,    61,
      71,    86,     2,     3,     4,    68,     7,     5,    60,    61,
      71,   119,     6,     1,    91,    92,    65,    26,     2,     3,
       4,    69,     7,     5,    93,    94,   106,    26,     6,    95,
      96,   110,   111,   112,   113,   114,   101,   102,     7,   105,
      79,   115,    72,    73,    74,    75,    76,    80,    28,     2,
       3,     4,   109,   121,     5,   103,   125,   104,    26,   129,
      38,    39,    21,    22,    40,     2,     3,     4,    81,     7,
       5,    30,    31,    87,    32,   137,    89,    33,    72,    73,
      74,    75,    76,    21,    22,    41,   136,    90,     2,    42,
       4,   120,   100,     5,   140,   142,    48,    21,    22,    49,
       2,     3,     4,    97,    98,     5,    30,    31,     7,    32,
     122,   100,    33,   141,    42,    29,    21,    22,    50,   123,
      51,    74,    75,    76,    42,    30,    31,   124,    32,    99,
     100,    33,    21,    22,   126,     2,   127,     4,   128,    34,
       5,    30,    31,   130,    32,   116,   117,    33,    21,    22,
     131,     2,   132,     4,    76,    82,     5,    30,    31,   138,
      32,   134,   135,    33,    21,    22,    72,    73,    74,    75,
      76,    51,    77,    30,    31,    85,    32,   118,     0,    33,
      72,    73,    74,    75,    76,     0,     0,    34,   136,     0,
     109,    72,    73,    74,    75,    76,    72,    73,    74,    75,
      76,   143,    78,    72,    73,    74,    75,    76,     0,    88,
      72,    73,    74,    75,    76,     0,   133,    72,    73,    74,
      75,    76
};

static const yytype_int16 yycheck[] =
{
      11,     8,    13,    14,    14,    15,    15,     2,     5,     6,
       4,     3,    28,     7,    30,     3,     8,     9,    10,    16,
      28,    13,    33,    34,     0,    27,    18,     3,    11,    12,
      41,    31,     8,     9,    10,    28,    28,    13,    11,    12,
      51,    33,    18,     3,    26,    27,    29,    41,     8,     9,
      10,    28,    28,    13,    26,    27,    67,    51,    18,    26,
      27,    72,    73,    74,    75,    76,    60,    61,    28,    64,
      25,    82,    19,    20,    21,    22,    23,    25,    85,     8,
       9,    10,    29,    92,    13,    27,    96,    29,    82,   100,
       3,     4,     5,     6,     7,     8,     9,    10,    25,    28,
      13,    14,    15,    25,    17,   116,    25,    20,    19,    20,
      21,    22,    23,     5,     6,    28,    27,    25,     8,    32,
      10,    25,    27,    13,    29,   136,     4,     5,     6,     7,
       8,     9,    10,    26,    27,    13,    14,    15,    28,    17,
      25,    27,    20,    29,    32,     4,     5,     6,    26,     4,
      28,    21,    22,    23,    32,    14,    15,    25,    17,    26,
      27,    20,     5,     6,    25,     8,     7,    10,    25,    28,
      13,    14,    15,     3,    17,    28,    29,    20,     5,     6,
      30,     8,    29,    10,    23,    28,    13,    14,    15,    25,
      17,   107,   108,    20,     5,     6,    19,    20,    21,    22,
      23,    28,    25,    14,    15,    42,    17,    83,    -1,    20,
      19,    20,    21,    22,    23,    -1,    -1,    28,    27,    -1,
      29,    19,    20,    21,    22,    23,    19,    20,    21,    22,
      23,    29,    25,    19,    20,    21,    22,    23,    -1,    25,
      19,    20,    21,    22,    23,    -1,    25,    19,    20,    21,
      22,    23
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     8,     9,    10,    13,    18,    28,    35,    36,
      37,    38,    39,    41,    42,    43,    49,    50,    51,    28,
      30,     5,     6,    16,    52,    50,    50,     0,    36,     4,
      14,    15,    17,    20,    28,    52,    56,    56,     3,     4,
       7,    28,    32,    44,    45,    49,    56,    60,     4,     7,
      26,    28,    45,    46,    47,    48,    49,    53,    56,    59,
      11,    12,     3,    40,    28,    29,    27,    55,    28,    28,
      56,    56,    19,    20,    21,    22,    23,    25,    25,    25,
      25,    25,    28,    61,    62,    35,    31,    25,    25,    25,
      25,    26,    27,    26,    27,    26,    27,    26,    27,    26,
      27,    50,    50,    27,    29,    52,    56,    57,    58,    29,
      56,    56,    56,    56,    56,    56,    28,    29,    62,    33,
      25,    45,    25,     4,    25,    49,    25,     7,    25,    56,
       3,    30,    29,    25,    59,    59,    27,    56,    25,    54,
      29,    29,    56,    29
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
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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
        case 2:
#line 259 "config.tab.yy"
    { is_lhs = true; }
    break;

  case 3:
#line 260 "config.tab.yy"
    { is_lhs = true; }
    break;

  case 4:
#line 263 "config.tab.yy"
    { is_lhs = true; }
    break;

  case 5:
#line 264 "config.tab.yy"
    { is_lhs = true; }
    break;

  case 6:
#line 265 "config.tab.yy"
    { }
    break;

  case 7:
#line 269 "config.tab.yy"
    { do_print( 0); }
    break;

  case 8:
#line 270 "config.tab.yy"
    { do_print((yyvsp[(2) - (5)].text)); }
    break;

  case 9:
#line 274 "config.tab.yy"
    { is_lhs = false; }
    break;

  case 10:
#line 279 "config.tab.yy"
    { do_assign_boolean   ((yyvsp[(1) - (3)].text), (yyvsp[(2) - (3)].bval)); }
    break;

  case 11:
#line 280 "config.tab.yy"
    { do_assign_exp       ((yyvsp[(1) - (3)].text)); }
    break;

  case 12:
#line 281 "config.tab.yy"
    { do_assign_id        ((yyvsp[(1) - (3)].text), (yyvsp[(2) - (3)].text)); }
    break;

  case 13:
#line 282 "config.tab.yy"
    { do_pwl              ((yyvsp[(1) - (3)].text)); }
    break;

  case 14:
#line 284 "config.tab.yy"
    { do_assign_threshold ((yyvsp[(1) - (3)].text)); }
    break;

  case 15:
#line 285 "config.tab.yy"
    { do_assign_string    ((yyvsp[(1) - (3)].text), (yyvsp[(2) - (3)].text)); }
    break;

  case 16:
#line 286 "config.tab.yy"
    { do_assign_dict      ((yyvsp[(1) - (2)].text)); }
    break;

  case 17:
#line 288 "config.tab.yy"
    { do_assign_dict((yyvsp[(1) - (4)].text)); }
    break;

  case 18:
#line 289 "config.tab.yy"
    { do_assign_exp_array((yyvsp[(1) - (4)].text)); }
    break;

  case 19:
#line 290 "config.tab.yy"
    { do_assign_dict((yyvsp[(1) - (4)].text)); }
    break;

  case 20:
#line 291 "config.tab.yy"
    { do_assign_dict((yyvsp[(1) - (4)].text)); }
    break;

  case 21:
#line 292 "config.tab.yy"
    { do_assign_dict((yyvsp[(1) - (4)].text)); }
    break;

  case 22:
#line 293 "config.tab.yy"
    { do_assign_dict((yyvsp[(1) - (3)].text)); }
    break;

  case 23:
#line 295 "config.tab.yy"
    { do_user_function_def(); }
    break;

  case 24:
#line 301 "config.tab.yy"
    { ida.add((yyvsp[(1) - (1)].text)); }
    break;

  case 25:
#line 302 "config.tab.yy"
    { ida.add((yyvsp[(3) - (3)].text)); }
    break;

  case 26:
#line 306 "config.tab.yy"
    { is_lhs = false;  function_name = (yyvsp[(1) - (5)].text);  is_function_def = true; }
    break;

  case 27:
#line 310 "config.tab.yy"
    { is_lhs = false;  strcpy((yyval.text), (yyvsp[(1) - (2)].text)); }
    break;

  case 28:
#line 314 "config.tab.yy"
    { mark(apm); }
    break;

  case 29:
#line 314 "config.tab.yy"
    { is_lhs = false;  strcpy((yyval.text), (yyvsp[(1) - (3)].text)); }
    break;

  case 30:
#line 318 "config.tab.yy"
    { do_dict(); }
    break;

  case 33:
#line 327 "config.tab.yy"
    { do_string((yyvsp[(1) - (1)].text)); }
    break;

  case 34:
#line 328 "config.tab.yy"
    { do_string((yyvsp[(3) - (3)].text)); }
    break;

  case 37:
#line 336 "config.tab.yy"
    { do_thresh    ((yyvsp[(1) - (1)].node)); }
    break;

  case 38:
#line 337 "config.tab.yy"
    { do_na_thresh (); }
    break;

  case 39:
#line 341 "config.tab.yy"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 40:
#line 342 "config.tab.yy"
    { (yyval.node) = do_and_thresh   ((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 41:
#line 343 "config.tab.yy"
    { (yyval.node) = do_or_thresh    ((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 42:
#line 344 "config.tab.yy"
    { (yyval.node) = do_not_thresh   ((yyvsp[(2) - (2)].node));     }
    break;

  case 43:
#line 345 "config.tab.yy"
    { (yyval.node) = do_paren_thresh ((yyvsp[(2) - (3)].node));     }
    break;

  case 44:
#line 349 "config.tab.yy"
    { (yyval.node) = do_simple_thresh((yyvsp[(1) - (2)].cval), (yyvsp[(2) - (2)].nval));     }
    break;

  case 45:
#line 350 "config.tab.yy"
    { (yyval.node) = do_simple_perc_thresh((yyvsp[(1) - (2)].cval), (yyvsp[(2) - (2)].pc_info)); }
    break;

  case 46:
#line 351 "config.tab.yy"
    { (yyval.node) = do_compound_perc_thresh((yyvsp[(1) - (5)].cval), (yyvsp[(2) - (5)].pc_info), (yyvsp[(4) - (5)].nval)); }
    break;

  case 47:
#line 352 "config.tab.yy"
    { (yyval.node) = do_fortran_thresh((yyvsp[(1) - (1)].text));        }
    break;

  case 48:
#line 356 "config.tab.yy"
    { set_number_string(); }
    break;

  case 49:
#line 357 "config.tab.yy"
    { set_number_string(); }
    break;

  case 50:
#line 361 "config.tab.yy"
    { do_boolean((yyvsp[(1) - (1)].bval)); }
    break;

  case 51:
#line 362 "config.tab.yy"
    { do_boolean((yyvsp[(3) - (3)].bval)); }
    break;

  case 56:
#line 376 "config.tab.yy"
    { do_number((yyvsp[(1) - (1)].nval)); }
    break;

  case 57:
#line 377 "config.tab.yy"
    { do_local_var((yyvsp[(1) - (1)].index)); }
    break;

  case 58:
#line 378 "config.tab.yy"
    { do_op('+'); }
    break;

  case 59:
#line 379 "config.tab.yy"
    { do_op('-'); }
    break;

  case 60:
#line 380 "config.tab.yy"
    { do_op('*'); }
    break;

  case 61:
#line 381 "config.tab.yy"
    { do_op('/'); }
    break;

  case 62:
#line 382 "config.tab.yy"
    { do_op('^'); }
    break;

  case 63:
#line 383 "config.tab.yy"
    { do_negate(); }
    break;

  case 64:
#line 384 "config.tab.yy"
    { do_paren_exp(); }
    break;

  case 65:
#line 385 "config.tab.yy"
    { mark(fcm); }
    break;

  case 66:
#line 385 "config.tab.yy"
    { do_builtin_call((yyvsp[(1) - (5)].index));  }
    break;

  case 67:
#line 386 "config.tab.yy"
    { mark(fcm); }
    break;

  case 68:
#line 386 "config.tab.yy"
    { do_user_function_call((yyvsp[(1) - (5)].entry)); }
    break;

  case 69:
#line 391 "config.tab.yy"
    { store_exp(); }
    break;

  case 70:
#line 392 "config.tab.yy"
    { store_exp(); }
    break;

  case 71:
#line 396 "config.tab.yy"
    { }
    break;

  case 72:
#line 400 "config.tab.yy"
    { }
    break;

  case 73:
#line 401 "config.tab.yy"
    { }
    break;

  case 74:
#line 405 "config.tab.yy"
    { add_point(); }
    break;


/* Line 1267 of yacc.c.  */
#line 2033 "config.tab.cc"
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


#line 409 "config.tab.yy"



////////////////////////////////////////////////////////////////////////


   //
   //  standard yacc stuff
   //


////////////////////////////////////////////////////////////////////////


void yyerror(const char * s)

{

int j, j1, j2;
int line_len, text_len;
int c;
char line[max_id_length + 1];
ifstream in;
ConcatString msg;


c = (int) (Column - strlen(configtext));

mlog << Error
     << "\n"
     << "yyerror() -> syntax error in file \"" << bison_input_filename << "\"\n\n"
     << "   line   = " << LineNumber << "\n\n"
     << "   column = " << c << "\n\n"
     << "   text   = \"" << configtext << "\"\n\n";

met_open(in, bison_input_filename);

for (j=1; j<LineNumber; ++j)  {   //  j starts at one here, not zero

   in.getline(line, sizeof(line));

}

in.getline(line, sizeof(line));

in.close();




mlog << Error
     << "\n" << line << "\n";

line_len = strlen(line);

text_len = strlen(configtext);

j1 = c;
j2 = c + text_len - 1;

msg.erase();
for (j=1; j<=line_len; ++j)  {   //  j starts at one here, not zero

   if ( (j >= j1) && (j <= j2) )  msg << '^';
   else                           msg << '_';

}

mlog << Error
     << msg << "\n\n";


exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


int configwrap()

{

return ( 1 );

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
      if ( (R.length() == 1) && (R[0].type == integer) && (R[0].i == 2) )   cell.type = op_square;
      else                                                                  cell.type = op_power;
      break;


   default:
      cerr << "\n\n  do_op() -> unrecognized op ... \"" << op << "\"\n\n";
      exit ( 1 );
      break;


}   //  switch

if ( cell.type != op_square )   L.add(R);

L.add(cell);

icvs.push(L);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Number do_integer_op(char op, const Number & a, const Number & b)

{

Number c;
int A, B, C;

A = a.i;
B = b.i;

switch ( op )  {

   case '+':  C = A + B;  break;
   case '-':  C = A - B;  break;
   case '*':  C = A * B;  break;
   case '/':
      if ( B == 0 )  {
         mlog << Error << "\ndo_integer_op() -> "
              << "division by zero!\n\n";
         exit ( 1 );
      }
      C = A / B;
      break;

   default:
      mlog << Error << "\ndo_integer_op() -> "
           << "bad operator ... \"" << op << "\"\n\n";
      exit ( 1 );
      break;

}

set_int(c, C);

   //
   //  done
   //

return ( c );

}


////////////////////////////////////////////////////////////////////////


void do_negate()

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


void do_paren_exp()

{

   //
   //  nothing to do here!
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_boolean(const char * name, bool tf)

{

DictionaryEntry entry;

entry.set_boolean(name, tf);

dict_stack->store(entry);

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_threshold(const char * name)

{

DictionaryEntry e;

e.set_threshold(name, STH);

dict_stack->store(e);

STH.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_exp(const char * name)

{

DictionaryEntry entry;
IcodeVector v;
Number n;

v = icvs.pop();

hp.run(v);

n = hp.pop();

if ( n.is_int)  entry.set_int    (name, n.i);
else            entry.set_double (name, n.d);

dict_stack->store(entry);


return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_string(const char * name, const char * text)

{

DictionaryEntry entry;

entry.set_string(name, text);

dict_stack->store(entry);

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_id(const char * LHS, const char * RHS)

{

const DictionaryEntry * e = dict_stack->lookup(RHS);

if ( !e )  {

   mlog << Error << "\ndo_assign_id() -> "
        << "identifier \"" << RHS
        << "\" not defined in this scope!\n\n";

   exit ( 1 );

}

DictionaryEntry ee = *e;

ee.set_name(LHS);

dict_stack->store(ee);

return;

}


////////////////////////////////////////////////////////////////////////


void do_dict()

{

DD = *(dict_stack->top());

dict_stack->erase_top();

if ( ! dict_stack->top_is_array() )  return;

DictionaryEntry e;

e.set_dict("", DD);

dict_stack->store(e);

DD.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_dict (const char * name)

{

if ( DD.n_entries() > 0 )  {

   DictionaryEntry e;

   e.set_dict(name, DD);

   dict_stack->store(e);

   DD.clear();

} else {

   dict_stack->pop_dict(name);

}

if ( icvs.top_is_mark(apm) )  icvs.toss();

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_exp_array(const char * name)

{

int j, count;
IcodeVector v;
Number n;
NumberStack ns;
DictionaryEntry e;



count = 0;

while ( 1 )  {

   v = icvs.pop();

   if ( v.is_mark() )  break;

   hp.run(v);

   n = hp.pop();

   ns.push(n);

   ++count;

}   //  while


for (j=0; j<count; ++j)  {

   e.clear();

   n = ns.pop();

   if ( n.is_int )  e.set_int    ("", n.i);
   else             e.set_double ("", n.d);

   dict_stack->store(e);

}   //  for j

dict_stack->set_top_is_array(true);

dict_stack->pop_dict(name);


DD.clear();

if ( icvs.top_is_mark(apm) )  icvs.toss();

return;

}


////////////////////////////////////////////////////////////////////////


void do_string(const char * text)

{

DictionaryEntry e;

e.set_string("", text);

dict_stack->store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void do_boolean(const bool & boolean)

{

DictionaryEntry e;

e.set_boolean("", boolean);

dict_stack->store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void store_exp()

{


// DictionaryEntry e;
// IcodeVector v;
// Number n;
//
// v = icvs.pop();
//
// hp.run(v);
//
// n = hp.pop();
//
// if ( n.is_int )  e.set_int    (0, n.i);
// else             e.set_double (0, n.d);
//
// dict_stack->store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void do_thresh(ThreshNode * node)

{

if ( test_mode )  {

   result = node;

}

else {

   if ( dict_stack->top_is_array() )  {

      DictionaryEntry e;
      SingleThresh T;

      T.set(node);

      e.set_threshold("", T);

      dict_stack->store(e);

   }  else STH.set(node);

}

return;

}


////////////////////////////////////////////////////////////////////////


void do_na_thresh()

{

if ( !dict_stack->top_is_array() )  return;

DictionaryEntry e;
SingleThresh T;

T.set(na_str);

e.set_threshold("", T);

dict_stack->store(e);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void add_point()

{

double x, y;
IcodeVector xv, yv;
Number n;

yv = icvs.pop();
xv = icvs.pop();

hp.run(xv);

n = hp.pop();

x = as_double(n);

hp.run(yv);

n = hp.pop();

y = as_double(n);

pwl.add_point(x, y);

return;

}


////////////////////////////////////////////////////////////////////////


void do_pwl(const char * LHS)

{

DictionaryEntry e;

e.set_pwl(LHS, pwl);

dict_stack->store(e);


   //
   //  done
   //

pwl.clear();

return;

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_and_thresh    (ThreshNode * a, ThreshNode * b)

{

And_Node * n = new And_Node;

n->left_child  = a;
n->right_child = b;

n->s << a->s << "&&" << b->s;

n->abbr_s << a->abbr_s << ".and." << b->abbr_s;

return ( n );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_or_thresh (ThreshNode * a, ThreshNode * b)

{

Or_Node * n = new Or_Node;

n->left_child  = a;
n->right_child = b;

n->s << a->s << "||" << b->s;

n->abbr_s << a->abbr_s << ".or." << b->abbr_s;

return ( n );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_not_thresh    (ThreshNode * n)

{

Not_Node * nn = new Not_Node;

nn->child = n;

nn->s << '!' << n->s;

nn->abbr_s << ".not." << n->abbr_s;

return ( nn );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_paren_thresh  (ThreshNode * n)

{

ConcatString b;

b.erase();

b << '(' << n->s << ')';

n->s = b;

b.erase();

b << '(' << n->abbr_s << ')';

n->abbr_s = b;

return ( n );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_simple_thresh (ThreshType op, const Number & n)

{

Simple_Node * s = new Simple_Node;

s->op = op;

s->T = as_double(n);

if ( op >= 0 )  {

   s->s      << thresh_type_str[op] << number_string;
   s->abbr_s << thresh_abbr_str[op] << number_string;

}

return ( s );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_fortran_thresh(const char * text)

{

ThreshType op  = no_thresh_type;
const char * p = text + 2;         //  we know that all the prefixes
                                   //  (like "le" or "gt") are two
                                   //  characters long

  //  foo


     if ( strncmp(text, "le", 2) == 0 )  op = thresh_le;
else if ( strncmp(text, "lt", 2) == 0 )  op = thresh_lt;

else if ( strncmp(text, "gt", 2) == 0 )  op = thresh_gt;
else if ( strncmp(text, "ge", 2) == 0 )  op = thresh_ge;

else if ( strncmp(text, "eq", 2) == 0 )  op = thresh_eq;
else if ( strncmp(text, "ne", 2) == 0 )  op = thresh_ne;

else {

   mlog << Error << "do_fortran_thresh(const char *) -> "
        << "can't parse threshold text \""
        << text << "\"\n\n";

   exit ( 1 );

}

Number n;
const double value = atof(p);

n.is_int = 0;

n.d = value;

set_number_string(p);

return ( do_simple_thresh (op, n) );

}


////////////////////////////////////////////////////////////////////////


void set_number_string()

{

set_number_string(configtext);

return;

}


////////////////////////////////////////////////////////////////////////


void set_number_string(const char * text)

{

const int k = (int) (sizeof(number_string));

strncpy(number_string, text, k);

number_string[k - 1] = (char) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void mark(int k)

{

IcodeVector v;
IcodeCell cell;


cell.type = cell_mark;

cell.i = k;

v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_number(const Number & n)

{

IcodeVector v;
IcodeCell cell;

if ( n.is_int )  {

   cell.set_integer(n.i);

} else {

   cell.set_double (n.d);

}

v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_local_var(int n)

{

IcodeVector v;
IcodeCell cell;

cell.set_local_var(n);

v.add(cell);

icvs.push(v);



return;

}


////////////////////////////////////////////////////////////////////////


void do_print(const char * s)

{

IcodeVector v;
Number n;


if ( bison_input_filename )  cout << bison_input_filename;
else                         cout << default_print_prefix;

cout << ": ";

if ( s )  cout << s;

v = icvs.pop();

hp.run(v);

n = hp.pop();

if ( n.is_int )  cout << (n.i) << "\n";
else             cout << (n.d) << "\n";



cout.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void do_builtin_call(int which)

{

int j;
IcodeVector v;
IcodeCell cell;
const BuiltinInfo & info = binfo[which];


if ( is_function_def )  {

   IcodeVector vv;

      //  pop the args (in reverse order) from the icodevector stack

   for (j=0; j<(info.n_args); ++j)  {

      vv = icvs.pop();

      v.add_front(vv);

   }

   if ( icvs.top_is_mark(fcm) )  icvs.toss();

      //

   cell.set_builtin(which);

   v.add(cell);

      //

   icvs.push(v);

   return;

}   //  if is function def

   ///////////////////////////////////////

Number n[max_builtin_args];
Number cur_result;

   //
   //  pop the args (in reverse order) from the icodevector stack
   //

for (j=0; j<(info.n_args); ++j)  {

   v = icvs.pop();

   if ( v.is_mark() )  {

      cerr << "\n\n  do_builtin_call(int) -> too few arguments to builtin function \""
           << info.name << "\"\n\n";

      exit ( 1 );

   }

   hp.run(v);

   n[info.n_args - 1 - j] = hp.pop();

}

   //
   //  next one should be a mark
   //

v = icvs.pop();

if ( ! (v.is_mark()) )  {

   cerr << "\n\n  do_builtin_call(int) -> too many arguments to builtin function \""
        << info.name << "\"\n\n";

   exit ( 1 );

}

   //
   //  call the function
   //

hp.do_builtin(which, n);

cur_result = hp.pop();

if ( cur_result.is_int )  cell.set_integer (cur_result.i);
else                      cell.set_double  (cur_result.d);

v.clear();

v.add(cell);

icvs.push(v);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_user_function_call(const DictionaryEntry * e)

{

int j;
IcodeVector v;
IcodeCell cell;
const int Nargs = e->n_args();


if ( is_function_def )  {

   IcodeVector vv;

      //  pop the args (in reverse order) from the icodevector stack

   for (j=0; j<Nargs; ++j)  {

      vv = icvs.pop();

      v.add_front(vv);

   }

   if ( icvs.top_is_mark(fcm) )  icvs.toss();

      //

   cell.set_user_function(e);

   v.add(cell);

      //

   icvs.push(v);

   return;

}   //  if is function def


   //////////////////////////////////////

Number n[max_user_function_args];
Number cur_result;

   //
   //  pop the args (in reverse order) from the icodevector stack
   //

for (j=0; j<Nargs; ++j)  {

   v = icvs.pop();

   if ( v.is_mark() )  {

      cerr << "\n\n  do_user_function_call(int) -> too few arguments to user function \""
           << (e->name()) << "\"\n\n";

      exit ( 1 );

   }

   hp.run(v);

   n[Nargs - 1 - j] = hp.pop();

}


if ( icvs.top_is_mark(fcm) )  icvs.toss();


   //
   //  call the function
   //

hp.run(*(e->icv()), n);

cur_result = hp.pop();

if ( cur_result.is_int )  cell.set_integer (cur_result.i);
else                      cell.set_double  (cur_result.d);

v.clear();

v.add(cell);

icvs.push(v);






   //
   //  done
   //

// if ( icvs.top_is_mark(fcm) )  icvs.toss();

return;

}


////////////////////////////////////////////////////////////////////////


void do_user_function_def()

{

DictionaryEntry e;

if ( ida.n_elements() > max_user_function_args )  {

   cerr << "\n\n  do_user_function_def() -> too many arguments to function \""
        << function_name << "\" definition\n\n";

   exit ( 1 );

}

e.set_user_function(function_name.c_str(), icvs.pop(), ida.n_elements());

dict_stack->store(e);

   //
   //  done
   //

is_function_def = false;

ida.clear();

function_name.erase();

return;

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_simple_perc_thresh (const ThreshType op, const PC_info & info)

{

Simple_Node * s = new Simple_Node;

s->op = op;

if ( (info.perc_index < 0) || (info.perc_index >= n_perc_thresh_infos) )  {

   mlog << Error
        << "\ndo_simple_perc_thresh() -> bad perc_index ... "
        << (info.perc_index) << "\n\n";

   exit ( 1 );

}

s->T     = bad_data_double;

s->PT    = info.value;

s->Ptype = perc_thresh_info[info.perc_index].type;

   //
   //  sanity check
   //

if ( s->PT < 0 || s->PT > 100 )  {

   mlog << Error << "\ndo_simple_perc_thresh() -> "
        << "the percentile (" << s->PT << ") must be between 0 and 100!\n\n";

   exit ( 1 );

}

if ( s->Ptype == perc_thresh_freq_bias && s->PT <= 0 )  {

   mlog << Error << "\ndo_simple_perc_thresh() -> "
        << "unsupported frequency bias percentile threshold!\n\n";

   exit ( 1 );

}

   //
   //  update the strings
   //

if ( op >= 0 )  {

   ConcatString cs;
   cs << perc_thresh_info[info.perc_index].short_name;
   cs << info.value;
   fix_float(cs);

   s->s      << thresh_type_str[op] << cs;
   s->abbr_s << thresh_abbr_str[op] << cs;

}

   //
   //  done
   //

return ( s );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_compound_perc_thresh (const ThreshType op, const PC_info & info, const Number & num)

{

Simple_Node * s = new Simple_Node;

s->op = op;

if ( (info.perc_index < 0) || (info.perc_index >= n_perc_thresh_infos) )  {

   mlog << Error
        << "\ndo_compound_perc_thresh() -> bad perc_index ... "
        << (info.perc_index) << "\n\n";

   exit ( 1 );

}

if ( num.is_int )  s->T     = (double) (num.i);
else               s->T     = num.d;

s->PT    = info.value;

s->Ptype = perc_thresh_info[info.perc_index].type;

   //
   //  sanity check
   //

if ( s->PT < 0 || s->PT > 100 )  {

   mlog << Error << "\ndo_compound_perc_thresh() -> "
        << "the percentile (" << s->PT << ") must be between 0 and 100!\n\n";

   exit ( 1 );

}

if ( s->Ptype == perc_thresh_freq_bias && !is_eq(s->PT, 1.0) )  {

   mlog << Error << "\ndo_compound_perc_thresh() -> "
        << "unsupported frequency bias percentile threshold!\n\n";

   exit ( 1 );

}

   //
   //  update the strings
   //

if ( op >= 0 )  {

   ConcatString cs;
   cs << perc_thresh_info[info.perc_index].short_name;
   cs << info.value;
   fix_float(cs);
   cs << "(" << number_string << ")";

   s->s      << thresh_type_str[op] << cs;
   s->abbr_s << thresh_abbr_str[op] << cs;

}

   //
   //  done
   //

return ( s );

}


////////////////////////////////////////////////////////////////////////


