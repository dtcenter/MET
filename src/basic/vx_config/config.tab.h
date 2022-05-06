/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
/* Line 1529 of yacc.c.  */
#line 107 "config.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE configlval;

