/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_CONFIG_CONFIG_TAB_H_INCLUDED
# define YY_CONFIG_CONFIG_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int configdebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IDENTIFIER = 258,              /* IDENTIFIER  */
    QUOTED_STRING = 259,           /* QUOTED_STRING  */
    INTEGER = 260,                 /* INTEGER  */
    FLOAT = 261,                   /* FLOAT  */
    BOOLEAN = 262,                 /* BOOLEAN  */
    COMPARISON = 263,              /* COMPARISON  */
    NA_COMPARISON = 264,           /* NA_COMPARISON  */
    LOGICAL_OP_NOT = 265,          /* LOGICAL_OP_NOT  */
    LOGICAL_OP_AND = 266,          /* LOGICAL_OP_AND  */
    LOGICAL_OP_OR = 267,           /* LOGICAL_OP_OR  */
    FORTRAN_THRESHOLD = 268,       /* FORTRAN_THRESHOLD  */
    BUILTIN = 269,                 /* BUILTIN  */
    LOCAL_VAR = 270,               /* LOCAL_VAR  */
    SIMPLE_PERC_THRESH = 271,      /* SIMPLE_PERC_THRESH  */
    USER_FUNCTION = 272,           /* USER_FUNCTION  */
    PRINT = 273,                   /* PRINT  */
    UNARY_MINUS = 274              /* UNARY_MINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
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

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 191 "config.tab.yy"


   char text[max_id_length + 1];

   Number nval;

   bool bval;

   int index;

   ThreshType cval;

   ThreshNode * node;

   const DictionaryEntry * entry;

   PC_info pc_info;


#line 125 "config.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE configlval;


int configparse (void);


#endif /* !YY_CONFIG_CONFIG_TAB_H_INCLUDED  */
