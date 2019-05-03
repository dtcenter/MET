

%{


////////////////////////////////////////////////////////////////////////


#define YY_NO_UNPUT 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <stdlib.h>

#include "scanner_stuff.h"   //  must be included before econfig.tab.h
#include "idstack.h"         //  must be included before econfig.tab.h
#include "icode.h"           //  must be included before econfig.tab.h
#include "symtab.h"
#include "machine.h"

#include "econfig.tab.h"


////////////////////////////////////////////////////////////////////////


extern "C" {

   int econfigwrap();

}


extern int econfig_LineNumber;

extern int econfig_column;

extern Machine machine;


////////////////////////////////////////////////////////////////////////


static int comment_depth = 0;

static YY_BUFFER_STATE file_buffer = (YY_BUFFER_STATE) 0;

static int reading_string = 0;

static int max_token_length = (int) sizeof(YYSTYPE);


////////////////////////////////////////////////////////////////////////


static int  do_id();

static void do_int();
static void do_float();

static void do_c_comment();
static void do_cpp_comment();

static  int nextchar();

static void do_quoted_string();

static void do_env();

static int do_eof();

static void patch_env(char * text);

static void get_env_text(const char * line, char * env_text, int len);


////////////////////////////////////////////////////////////////////////


%}


DIGIT     [0-9]

DIGITS    [0-9]+

LETTER    [A-Z]|[a-z]|"_"

WS        " "|"\t"

EXP       (e|E)(-?){DIGITS}

OPT_EXP   {EXP}?

IDENTIFIER ({LETTER})({LETTER}|{DIGIT})*


%%


";"                                 { ++econfig_column;    return ( ';' ); }

"{"                                 { ++econfig_column;    return ( '{' ); }
"}"                                 { ++econfig_column;    return ( '}' ); }
"["                                 { ++econfig_column;    return ( '[' ); }
"]"                                 { ++econfig_column;    return ( ']' ); }
"="                                 { ++econfig_column;    return ( '=' ); }
","                                 { ++econfig_column;    return ( ',' ); }
"("                                 { ++econfig_column;    return ( '(' ); }
")"                                 { ++econfig_column;    return ( ')' ); }


"+"                                 { ++econfig_column;    return ( '+' ); }
"-"                                 { ++econfig_column;    return ( '-' ); }
"*"                                 { ++econfig_column;    return ( '*' ); }
"/"                                 { ++econfig_column;    return ( '/' ); }

"^"                                 { ++econfig_column;    return ( '^' ); }


"\""                                { do_quoted_string();  return ( QUOTED_STRING ); }

"${"{IDENTIFIER}"}"                 { do_env(); }

{IDENTIFIER}                        { return ( do_id() ); }



("-"?){DIGITS}                      { do_int();    return ( INTEGER ); }



("-"?){DIGITS}{EXP}                 { do_float();  return ( FLOAT ); }

("-"?)"."{DIGITS}{OPT_EXP}          { do_float();  return ( FLOAT ); }

("-"?){DIGITS}"."{OPT_EXP}          { do_float();  return ( FLOAT ); }

("-"?){DIGITS}"."{DIGITS}{OPT_EXP}  { do_float();  return ( FLOAT ); }




"/*"                                { do_c_comment();   }
"//"                                { do_cpp_comment(); }



"\n"                                { ++econfig_LineNumber;  econfig_column = 1; }



{WS}                                { ++econfig_column; }

<<EOF>>                             { if ( do_eof() )  return ( 0 ); }

.                                   { ++econfig_column; }


%%


////////////////////////////////////////////////////////////////////////


int do_id()

{

econfig_column += strlen(econfigtext);

if ( strcmp(econfigtext, "print") == 0 )  return ( PRINT );

if ( strcmp(econfigtext, "true"  ) == 0 )  { econfiglval.ival = 1;  return ( BOOLEAN ); }
if ( strcmp(econfigtext, "false" ) == 0 )  { econfiglval.ival = 0;  return ( BOOLEAN ); }


strncpy(econfiglval.text, econfigtext, sizeof(econfiglval.text) - 1);

return ( ID );

}


////////////////////////////////////////////////////////////////////////


void do_int()

{

econfig_column += strlen(econfigtext);

econfiglval.num.is_int = 1;

econfiglval.num.i = atoi(econfigtext);

return;

}


////////////////////////////////////////////////////////////////////////


void do_float()

{

econfig_column += strlen(econfigtext);

econfiglval.num.is_int = 0;

econfiglval.num.d = atof(econfigtext);

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

++econfig_column;

if ( c == '\n' )  {

   ++econfig_LineNumber;

   econfig_column = 1;

}


return ( c );

}


////////////////////////////////////////////////////////////////////////


void do_quoted_string()

{

int n;
char c;
char line[max_token_length];


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


strncpy(econfiglval.text, line, sizeof(econfiglval.text));

econfiglval.text[ sizeof(econfiglval.text) - 1 ] = (char) 0;

patch_env(econfiglval.text);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_env()

{

int n;
char junk[256];
char * value = (char *) 0;



memset(junk, 0, sizeof(junk));

n = strlen(yytext);

if ( n >= (int) (sizeof(junk) - 1) )  {

   cerr << "\n\n  do_env() -> environment variable name too long\n\n";

   exit ( 1 );

}

strncpy(junk, yytext + 2, sizeof(junk) - 1);

junk[n - 3] = (char) 0;

// cout << "environment variable \"" << junk << "\"\n" << flush;

value = getenv(junk);

if ( !value )  {

   cerr << "\n\n  do_env() -> can't get value of environment variable \"" << junk << "\"\n\n";

   exit ( 1 );

}

// static const int max_buf_stack_size = 2;
// 
// static YY_BUFFER_STATE buf_stack[max_buf_stack_size];
// 
// static int buf_stack_depth = 0;

file_buffer = YY_CURRENT_BUFFER;

reading_string = 1;

econfig_scan_string(value);


   //
   //  done
   //

econfig_column += 3;

return;

}


////////////////////////////////////////////////////////////////////////


int do_eof()

{

if ( !reading_string )  return ( 1 );

econfig_delete_buffer(YY_CURRENT_BUFFER);

econfig_switch_to_buffer(file_buffer);

reading_string = 0;

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void patch_env(char * text)

{

int j, k, n, nn;
char c, cc;
char line[max_token_length + 1];
char env_text[max_token_length + 1];
const char * e = (const char *) 0;


memset(line, 0, sizeof(line));

strncpy(line, text, max_token_length);

memset(text, 0, max_token_length);


n = strlen(line);

k = 0;

for (j=0; j<n; ++j)  {

   c = line[j];

   if ( c == '$' )  {

      cc = line[j + 1];

      if ( cc != '{' ) text[k++] = c;
      else {

         get_env_text(line + j, env_text, sizeof(env_text));

         e = getenv(env_text);

         if ( !e )  {

            cerr << "\n\n  patch_env() -> environment variable \"" << env_text << "\" not found!\n\n";

            exit ( 1 );

         }

         nn = strlen(e);

         if ( (k + nn) >= max_token_length )  {

            cerr << "\n\n  patch_env() -> replacement text for environment variable \"" << env_text << "\" too long!\n\n";

            exit ( 1 );


         }

         strcpy(text + k, e);

         k += nn;

         j += strlen(env_text) + 2;   //  +3 for "${" and "}" and -1 'cuz j will get incremented at the top of the loop

      }

   } else text[k++] = c;





   if ( k >= max_token_length )  {

      cerr << "\n\n  patch_env() -> string too long!\n\n";

      exit ( 1 );

   }

}   //  for j








return;

}


////////////////////////////////////////////////////////////////////////


void get_env_text(const char * line, char * env_text, int len)

{

   //
   //  add 2 to skip the initial "${"
   //

const char * c = strchr(line + 2, '}');

if ( !c )  {

   cerr << "\n\n  get_env_text() -> closing bracket not found in string \"" << line << "\"\n\n";

   exit ( 1 );

}

int n = (int) (c - (line + 2));

if ( n >= len )  {

   cerr << "\n\n  get_env_text() -> environment variable text too long in \"" << line << "\"\n\n";

   exit ( 1 );

}

memset(env_text, 0, len);

strncpy(env_text, line + 2, n);


return;

}


////////////////////////////////////////////////////////////////////////






