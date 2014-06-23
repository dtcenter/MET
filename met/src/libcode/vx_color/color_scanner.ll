

%{


////////////////////////////////////////////////////////////////////////


#define YY_NO_UNPUT 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>

#include "color_parser.h"   //  this must be included before color_parser_yacc.h
#include "color.h"

#include "color_parser_yacc.h"


////////////////////////////////////////////////////////////////////////


extern "C" {

   int colorwrap();

}

extern int color_file_line_number;

extern int color_file_column;

extern ColorList clist;


////////////////////////////////////////////////////////////////////////


static int comment_depth = 0;


////////////////////////////////////////////////////////////////////////


static int  do_id();

static void do_int();
static void do_float();

static void do_c_comment();
static void do_cpp_comment();

static  int nextchar();

static void do_quoted_string();


////////////////////////////////////////////////////////////////////////


%}


DIGIT     [0-9]

DIGITS    [0-9]+

LETTER    [A-Z]|[a-z]|"_"

WS        " "|"\t"

EXP       (e|E)(-?){DIGITS}

OPT_EXP   {EXP}?


%%


","                                 { ++color_file_column;    return ( ',' ); }


"{"                                 { ++color_file_column;    return ( '{' ); }
"}"                                 { ++color_file_column;    return ( '}' ); }

"("                                 { ++color_file_column;    return ( '(' ); }
")"                                 { ++color_file_column;    return ( ')' ); }

"="                                 { ++color_file_column;    return ( '=' ); }




"\""                                { do_quoted_string();  return ( QUOTED_STRING ); }


({LETTER})({LETTER}|{DIGIT})*       { return ( do_id() ); }



("-"?){DIGITS}                      { do_int();    return ( INTEGER ); }



("-"?){DIGITS}{EXP}                 { do_float();  return ( FLOAT ); }

("-"?)"."{DIGITS}{OPT_EXP}          { do_float();  return ( FLOAT ); }

("-"?){DIGITS}"."{OPT_EXP}          { do_float();  return ( FLOAT ); }

("-"?){DIGITS}"."{DIGITS}{OPT_EXP}  { do_float();  return ( FLOAT ); }




"/*"                                { do_c_comment();   }
"//"                                { do_cpp_comment(); }



"\n"                                { ++color_file_line_number;  color_file_column = 1; }



{WS}                                { ++color_file_column; }

.                                   { ++color_file_column; }


%%


////////////////////////////////////////////////////////////////////////


int do_id()

{

color_file_column += strlen(colortext);


if ( strcmp(colortext, "blend"    ) == 0 )  return ( BLEND     );
if ( strcmp(colortext, "hsv"      ) == 0 )  return ( HSV       );
if ( strcmp(colortext, "cmyk"     ) == 0 )  return ( CMYK      );
if ( strcmp(colortext, "grayvalue") == 0 )  return ( GRAYVALUE );


int index;

if ( clist.has_name(colortext, index) )  {

   colorlval.ival = index;

   return ( COLOR_NAME );

}

strncpy(colorlval.text, colortext, sizeof(colorlval.text) - 1);

return ( ID );

}


////////////////////////////////////////////////////////////////////////


void do_int()

{

color_file_column += strlen(colortext);

colorlval.ival = atoi(colortext);

return;

}


////////////////////////////////////////////////////////////////////////


void do_float()

{

color_file_column += strlen(colortext);

colorlval.dval = atof(colortext);

return;

}

////////////////////////////////////////////////////////////////////////


void do_c_comment()

{

int c1, c2;


c1 = nextchar();
c2 = nextchar();

comment_depth = 1;


while ( 1 )  {

   if ( (c1 == EOF) || (c2 == EOF) )  break;

   if ( (c1 == '/') && (c2 == '*') )  ++comment_depth;

   if ( (c1 == '*') && (c2 == '/') )  {

      --comment_depth;

      if ( comment_depth == 0 )  break;

   }

   c1 = c2;

   c2 = nextchar();

}



return;

}


////////////////////////////////////////////////////////////////////////


void do_cpp_comment()

{

int c;


while ( 1 )  {

   c = nextchar();

   if ( (c == EOF) || (c == '\n') )  break;

}


return;

}


////////////////////////////////////////////////////////////////////////


int nextchar()

{

int c;


c = yyinput();

++color_file_column;

if ( c == '\n' )  {

   ++color_file_line_number;

   color_file_column = 1;

}


return ( c );

}


////////////////////////////////////////////////////////////////////////


void do_quoted_string()

{

int n;
char c;
char line[128];


n = 0;

while ( 1 )  {

   c = nextchar();

   if ( c == '\"' )  break;

   if ( c == '\\' )  {

      c = nextchar();

      switch ( c )  {

         case 'n':
            line[n++] = '\n';
            break;

         case 't':
            line[n++] = '\t';
            break;

         case 'b':
            line[n++] = '\b';
            break;

         case '\"':
            line[n++] = '\"';
            break;

         case '\\':
            line[n++] = '\\';
            break;

         default:
            line[n++] = c;
            break;

      }   //  switch

   } else {

      line[n++] = c;

   }

   if ( (n + 1) >= (int) sizeof(line) )  {

      cerr << "\n\n  do_quoted_string() -> string too long! ... c = \"" << c << "\"\n\n";

      exit ( 1 );

   }

}   //  while

line[n] = (char) 0;

strncpy(colorlval.text, line, sizeof(colorlval.text));

colorlval.text[ sizeof(colorlval.text) - 1 ] = (char) 0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////






