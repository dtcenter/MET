

%{


////////////////////////////////////////////////////////////////////////


#define YY_NO_UNPUT 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>

#include "scanner_stuff.h"   //  must be included before config.tab.h

#include "config.tab.h"

#include "dictionary.h"


////////////////////////////////////////////////////////////////////////


extern "C" {

   int configwrap();

}

extern int LineNumber;

extern int Column;

extern DictionaryStack * dict_stack;

extern bool is_lhs;


////////////////////////////////////////////////////////////////////////


static bool reading_string = false;

static YY_BUFFER_STATE file_buffer = (YY_BUFFER_STATE) 0;


////////////////////////////////////////////////////////////////////////


static void do_c_comment();
static void do_cpp_comment();

static  int nextchar();

static int  do_comp();

static int  do_id();

static void do_int();
static void do_float();

static void do_quoted_string();

static void patch_env(char * text);

static void get_env_text(const char * line, char * env_text, int len);


////////////////////////////////////////////////////////////////////////


%}


DIGIT       [0-9]

DIGITS      {DIGIT}+

LETTER      [A-Z]|[a-z]|"_"

LETTERS     {LETTER}+

IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*

EXP         (e|E)(-?){DIGITS}

OPT_EXP     {EXP}?

WS          [ \n\t]*


%%

"<"                                 { return ( do_comp() ); }
">"                                 { return ( do_comp() ); }
"<="                                { return ( do_comp() ); }
">="                                { return ( do_comp() ); }


"["                                 { ++Column;  return ( configtext[0] ); }
"]"                                 { ++Column;  return ( configtext[0] ); }
"{"                                 { ++Column;  is_lhs = true;  dict_stack->push(); return ( configtext[0] ); }
"}"                                 { ++Column;  return ( configtext[0] ); }
"("                                 { ++Column;  return ( configtext[0] ); }
")"                                 { ++Column;  return ( configtext[0] ); }

"+"                                 { ++Column;  return ( configtext[0] ); }
"-"                                 { ++Column;  return ( configtext[0] ); }
"/"                                 { ++Column;  return ( configtext[0] ); }
"*"                                 { ++Column;  return ( configtext[0] ); }
"^"                                 { ++Column;  return ( configtext[0] ); }

"="                                 { ++Column;  return ( configtext[0] ); }
";"                                 { ++Column;  return ( configtext[0] ); }
","                                 { ++Column;  return ( configtext[0] ); }



"\""                                { do_quoted_string();  return ( QUOTED_STRING ); }

{IDENTIFIER}                        { return ( do_id() ); }



("-"?){DIGITS}                      { do_int();    return ( INTEGER ); }



("-"?){DIGITS}{EXP}                 { do_float();  return ( FLOAT ); }

("-"?)"."{DIGITS}{OPT_EXP}          { do_float();  return ( FLOAT ); }

("-"?){DIGITS}"."{OPT_EXP}          { do_float();  return ( FLOAT ); }

("-"?){DIGITS}"."{DIGITS}{OPT_EXP}  { do_float();  return ( FLOAT ); }





"/*"                                { do_c_comment();   }
"//"                                { do_cpp_comment(); }



"\n"                                { ++LineNumber;  Column = 1; }


.                                   { ++Column; }


%%


////////////////////////////////////////////////////////////////////////


int nextchar()

{

int c;


c = yyinput();

++Column;

if ( c == '\n' )  {

   ++LineNumber;

   Column = 1;

}


return ( c );

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


void do_c_comment()

{

int c1, c2;
int comment_depth;


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

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int do_comp()

{

Column += strlen(configtext);

     if ( strcmp(configtext, "<" ) == 0 )  configlval.cval = comparison_lt;
else if ( strcmp(configtext, ">" ) == 0 )  configlval.cval = comparison_gt;
else if ( strcmp(configtext, "<=") == 0 )  configlval.cval = comparison_le;
else if ( strcmp(configtext, ">=") == 0 )  configlval.cval = comparison_ge;
else {

   cerr << "\n\n  do_comp() -> bad comparison operator ... \"" << configtext << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( COMPARISON );

}


////////////////////////////////////////////////////////////////////////


void do_int()

{

Column += strlen(configtext);

set_int(configlval.nval, atoi(configtext));

return;

}


////////////////////////////////////////////////////////////////////////


void do_float()

{

Column += strlen(configtext);

set_double(configlval.nval, atof(configtext));

return;

}


////////////////////////////////////////////////////////////////////////


void do_quoted_string()

{

int n;
char c;
char line[max_id_length];


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


strncpy(configlval.text, line, sizeof(configlval.text));

configlval.text[ sizeof(configlval.text) - 1 ] = (char) 0;

patch_env(configlval.text);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int do_id()

{

Column += strlen(configtext);

   //
   //  boolean?
   //

if ( strcmp(configtext, "true"  ) == 0 )  { configlval.bval = true;   return ( BOOLEAN ); }
if ( strcmp(configtext, "false" ) == 0 )  { configlval.bval = false;  return ( BOOLEAN ); }

   //
   //  number?
   //

const DictionaryEntry * e = dict_stack->lookup(configtext);

if ( e && (e->is_number()) && (! is_lhs) )  {

   if ( e->type() == IntegerType )  {

      set_int(configlval.nval, e->i_value());

      return ( INTEGER ); 

   } else {

      set_double(configlval.nval, e->d_value());

      return ( FLOAT ); 

   }

}

   //
   //  nope
   //

strncpy(configlval.text, configtext, sizeof(configlval.text) - 1);

return ( IDENTIFIER );

}


////////////////////////////////////////////////////////////////////////


void patch_env(char * text)

{

int j, k, n, nn;
char c, cc;
char line[max_id_length + 1];
char env_text[max_id_length + 1];
const char * e = (const char *) 0;

memset(line, 0, sizeof(line));

strncpy(line, text, max_id_length);

memset(text, 0, max_id_length);


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

         if ( (k + nn) >= max_id_length )  {

            cerr << "\n\n  patch_env() -> replacement text for environment variable \"" << env_text << "\" too long!\n\n";

            exit ( 1 );


         }

         strcpy(text + k, e);

         k += nn;

         j += strlen(env_text) + 2;   //  +3 for "${" and "}" and -1 'cuz j will get incremented at the top of the loop

      }

   } else text[k++] = c;

   if ( k >= max_id_length )  {

      cerr << "\n\n  patch_env() -> string too long!\n\n";

      exit ( 1 );

   }

}   //  for j

   //
   //  done
   //

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







