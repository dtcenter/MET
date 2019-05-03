

%{


////////////////////////////////////////////////////////////////////////


#define YY_NO_UNPUT 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>


#include "enum.tab.h"

#include "scope.h"


////////////////////////////////////////////////////////////////////////


extern "C" {

   int yywrap();

}

extern int LineNumber;

extern int column;

extern int enum_mode;

int bracket_level = 0;

extern ScopeStack ss;

extern ScopeStackElement sse;

extern int saw_class_prefix;

static int last_was_class = 0;
static int last_was_enum  = 0;


////////////////////////////////////////////////////////////////////////


static void do_enum();
static void do_class();

static int  do_id();
static int  do_int();

static int  do_left_curly();
static int  do_right_curly();

static int  do_semi();

static int  do_equals();
static int  do_comma();

static void do_c_comment();
static void do_cpp_comment();

static  int nextchar();

static  int token(int);


////////////////////////////////////////////////////////////////////////


%}


DIGIT  [0-9]

LETTER  [A-Z]|[a-z]|"_"

WS      " "|"\t"


%%

enum                             { do_enum();   return ( token(ENUM)  ); }
class                            { do_class();  return ( token(CLASS) ); }

"{"                              { if ( do_left_curly()  )  return ( token(L_CURLY) ); }
"}"                              { if ( do_right_curly() )  return ( token(R_CURLY) ); }
"="                              { if ( do_equals()      )  return ( token (EQ) );     }
";"                              { if ( do_semi()        )  return ( token(';') );     }
","                              { if ( do_comma()       )  return ( token(',') );     }
("-"?)({DIGIT})+                 { if ( do_int()         )  return ( token(INTEGER) ); }
({LETTER})({LETTER}|{DIGIT})*    { if ( do_id()          )  return ( token(ID) );      }



"/*"                             { do_c_comment(); }
"//"                             { do_cpp_comment(); }

"\n"                             { ++LineNumber;  column = 1; }


{WS}                             { ++column; }

.                                { ++column; }


%%


////////////////////////////////////////////////////////////////////////


void do_enum()

{

column += 4;

enum_mode = 1;

return;

}


////////////////////////////////////////////////////////////////////////


void do_class()

{

column += 5;


return;

}


////////////////////////////////////////////////////////////////////////


int do_id()

{

column += strlen(yytext);

if ( enum_mode || last_was_enum || last_was_class )  {

   strncpy(yylval.name, yytext, sizeof(yylval.name));

   return ( 1 );

}



return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int do_int()

{

column += strlen(yytext);

if ( !enum_mode )  return ( 0 );

yylval.ival = atoi(yytext);



return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int do_left_curly()

{

++bracket_level;

++column;

if ( !enum_mode )  return ( 0 );

yylval.ival = bracket_level;


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int do_right_curly()

{

--bracket_level;

++column;

ss.clear_to_level(bracket_level);

if ( !enum_mode )  return ( 0 );

yylval.ival = bracket_level + 1;


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int do_semi()

{

++column;

if ( enum_mode )  return ( 1 );


return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int do_equals()

{

++column;

if ( enum_mode )  return ( 1 );


return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int do_comma()

{

++column;

if ( enum_mode )  return ( 1 );


return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void do_c_comment()

{

int c1, c2;


c1 = nextchar();
c2 = nextchar();


while ( 1 )  {

   if ( (c1 == EOF) || (c2 == EOF) )  break;

   if ( (c1 == '*') && (c2 == '/') )  break;

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

++column;

if ( c == '\n' )  {

   ++LineNumber;

   column = 1;

}


return ( c );

}


////////////////////////////////////////////////////////////////////////


int token(int t)

{

saw_class_prefix = 0;

last_was_class   = 0;
last_was_enum    = 0;


if ( t == CLASS )  last_was_class = 1;
if ( t == ENUM  )  last_was_enum  = 1;


return ( t );

}


////////////////////////////////////////////////////////////////////////




