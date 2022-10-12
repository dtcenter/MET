/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_COLOR_COLOR_PARSER_YACC_H_INCLUDED
# define YY_COLOR_COLOR_PARSER_YACC_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int colordebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ID = 258,
    COLOR_NAME = 259,
    INTEGER = 260,
    QUOTED_STRING = 261,
    FLOAT = 262,
    BLEND = 263,
    HSV = 264,
    GRAYVALUE = 265,
    CMYK = 266
  };
#endif
/* Tokens.  */
#define ID 258
#define COLOR_NAME 259
#define INTEGER 260
#define QUOTED_STRING 261
#define FLOAT 262
#define BLEND 263
#define HSV 264
#define GRAYVALUE 265
#define CMYK 266

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 126 "color_parser_yacc.yy" /* yacc.c:1921  */


   char text[129];

   int ival;

   double dval;

   Dcolor cval;

   ColorNumber nval;


#line 94 "color_parser_yacc.h" /* yacc.c:1921  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE colorlval;

int colorparse (void);

#endif /* !YY_COLOR_COLOR_PARSER_YACC_H_INCLUDED  */
