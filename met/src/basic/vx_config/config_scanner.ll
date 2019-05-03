

%{


////////////////////////////////////////////////////////////////////////


#define YY_NO_UNPUT 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>

#include "vx_log.h"
#include "empty_string.h"
#include "math_constants.h"
#include "util_constants.h"
#include "is_number.h"

#include "dictionary.h"
#include "builtin.h"

#include "scanner_stuff.h" //  must be included before config.tab.h
#include "threshold.h"     //  must be included before config.tab.h

#include "config.tab.h"


////////////////////////////////////////////////////////////////////////


extern "C" {

   int configwrap();

}

extern int LineNumber;

extern int Column;

extern DictionaryStack * dict_stack;

extern IdentifierArray  ida;

extern bool is_lhs;

extern bool is_function_def;

extern void start_string_scan  (const char *);
extern void finish_string_scan ();


////////////////////////////////////////////////////////////////////////


// static bool reading_string = false;

static bool reading_env    = false;

static YY_BUFFER_STATE file_buffer   = (YY_BUFFER_STATE) 0;

static YY_BUFFER_STATE string_buffer = 0;   //  used for string parsing

static YY_BUFFER_STATE env_buffer = 0;   //  used for environment variables outside of strings


////////////////////////////////////////////////////////////////////////


static void do_c_comment();
static void do_cpp_comment();

static  int nextchar();

static  int do_comp();

static  int do_id();

static void do_int();
static void do_float();

static void do_quoted_string();

static void patch_env(char * text);

static void get_env_text(const char * line, char * env_text, int len);

static bool do_eof();

static void do_env();

static int do_fort_thresh();


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


INT_NUMBER  ("-"?){DIGITS}


REAL_NUMBER  (("-"?){DIGITS}{EXP})|(("-"?)"."{DIGITS}{OPT_EXP})|(("-"?){DIGITS}"."{OPT_EXP})|(("-"?){DIGITS}"."{DIGITS}{OPT_EXP})


%%

"<"                                 { return ( do_comp() ); }
">"                                 { return ( do_comp() ); }
"<="                                { return ( do_comp() ); }
">="                                { return ( do_comp() ); }
"=="                                { return ( do_comp() ); }
"!="                                { return ( do_comp() ); }
"NA"                                { return ( do_comp() ); }


"le"({INT_NUMBER}|{REAL_NUMBER})    { return ( do_fort_thresh() ); }
"lt"({INT_NUMBER}|{REAL_NUMBER})    { return ( do_fort_thresh() ); }
"gt"({INT_NUMBER}|{REAL_NUMBER})    { return ( do_fort_thresh() ); }
"ge"({INT_NUMBER}|{REAL_NUMBER})    { return ( do_fort_thresh() ); }
"eq"({INT_NUMBER}|{REAL_NUMBER})    { return ( do_fort_thresh() ); }
"ne"({INT_NUMBER}|{REAL_NUMBER})    { return ( do_fort_thresh() ); }


"&&"                                { Column+=2;   return ( LOGICAL_OP_AND ); }
"||"                                { Column+=2;   return ( LOGICAL_OP_OR  ); }

"!"                                 { Column+=1;   return ( LOGICAL_OP_NOT ); }


"["                                 { ++Column;  is_lhs = false;  dict_stack->push_array();  return ( configtext[0] ); }
"{"                                 { ++Column;  is_lhs = true;   dict_stack->push();        return ( configtext[0] ); }

"]"                                 { ++Column;  return ( configtext[0] ); }
"}"                                 { ++Column;  return ( configtext[0] ); }

"("                                 { ++Column;  return ( configtext[0] ); }
")"                                 { ++Column;  return ( configtext[0] ); }

"+"                                 { ++Column;  return ( configtext[0] ); }
"-"                                 { ++Column;  return ( configtext[0] ); }
"/"                                 { ++Column;  return ( configtext[0] ); }
"*"                                 { ++Column;  return ( configtext[0] ); }
"^"                                 { ++Column;  return ( configtext[0] ); }

"="                                 { ++Column;  return ( configtext[0] ); }
";"                                 { ++Column;  is_lhs = true;  return ( configtext[0] ); }
","                                 { ++Column;  return ( configtext[0] ); }



"\""                                { do_quoted_string();  return ( QUOTED_STRING ); }

"${"{IDENTIFIER}"}"                 { do_env(); }

{IDENTIFIER}                        { return ( do_id() ); }



{INT_NUMBER}                        { do_int();    return ( INTEGER ); }



{REAL_NUMBER}                       { do_float();  return ( FLOAT ); }






"/*"                                { do_c_comment();   }
"//"                                { do_cpp_comment(); }



"\n"                                { ++LineNumber;  Column = 1; }

<<EOF>>                             { if ( do_eof() )  return ( 0 ); }

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

int return_value;

Column += strlen(configtext);

     if ( strcmp(configtext, "<" ) == 0 )  { configlval.cval = thresh_lt;  return_value = COMPARISON; }
else if ( strcmp(configtext, ">" ) == 0 )  { configlval.cval = thresh_gt;  return_value = COMPARISON; }
else if ( strcmp(configtext, "<=") == 0 )  { configlval.cval = thresh_le;  return_value = COMPARISON; }
else if ( strcmp(configtext, ">=") == 0 )  { configlval.cval = thresh_ge;  return_value = COMPARISON; }
else if ( strcmp(configtext, "==") == 0 )  { configlval.cval = thresh_eq;  return_value = COMPARISON; }
else if ( strcmp(configtext, "!=") == 0 )  { configlval.cval = thresh_ne;  return_value = COMPARISON; }

else if ( strcmp(configtext, na_str) == 0 )  { configlval.cval = thresh_na;  return_value = NA_COMPARISON; }

else {

   mlog << Error
        << "\ndo_comp() -> bad comparison operator ... \""
        << configtext << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( return_value );

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

      mlog << Error
           << "\ndo_quoted_string() -> string too long! ... c = \""
           << c << "\"\n\n";

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
   //  print?
   //

if ( strcmp(configtext, "print"  ) == 0 )  { return ( PRINT ); }

   //
   //  boolean?
   //

if ( strcmp(configtext, "true"  ) == 0 )  { configlval.bval = true;   return ( BOOLEAN ); }
if ( strcmp(configtext, "false" ) == 0 )  { configlval.bval = false;  return ( BOOLEAN ); }

if ( strcmp(configtext, "TRUE"  ) == 0 )  { configlval.bval = true;   return ( BOOLEAN ); }
if ( strcmp(configtext, "FALSE" ) == 0 )  { configlval.bval = false;  return ( BOOLEAN ); }

   //
   //  comparison?
   //

if ( strcmp(configtext, "lt" ) == 0 )  { configlval.cval = thresh_lt;  return ( COMPARISON ); }
if ( strcmp(configtext, "le" ) == 0 )  { configlval.cval = thresh_le;  return ( COMPARISON ); }

if ( strcmp(configtext, "gt" ) == 0 )  { configlval.cval = thresh_gt;  return ( COMPARISON ); }
if ( strcmp(configtext, "ge" ) == 0 )  { configlval.cval = thresh_ge;  return ( COMPARISON ); }

if ( strcmp(configtext, "ne" ) == 0 )  { configlval.cval = thresh_ne;  return ( COMPARISON ); }
if ( strcmp(configtext, "eq" ) == 0 )  { configlval.cval = thresh_eq;  return ( COMPARISON ); }

if ( strcmp(configtext, na_str ) == 0 )  { configlval.cval = thresh_na;  return ( NA_COMPARISON ); }

   //
   //  builtin ?
   //

int index;

if ( (! is_lhs) && is_builtin((string)configtext, index) )  { configlval.index = index;  return ( BUILTIN ); }

   //
   //  local variable ?
   //

if ( is_function_def && ida.has(configtext, index) )  { configlval.index = index;  return ( LOCAL_VAR ); }

   //
   //  number?
   //

const DictionaryEntry * e = dict_stack->lookup(configtext);

if ( e && (e->is_number()) && (! is_lhs) )  {

   // cout << "=================== id = \"" << configtext << "\"    is_lhs = " << (is_lhs ? "true" : "false") << "\n";

   // cout << "do_id() -> \n";
   // e->dump(cout);

   if ( e->type() == IntegerType )  {

      set_int(configlval.nval, e->i_value());

      return ( INTEGER );

   } else {

      set_double(configlval.nval, e->d_value());

      return ( FLOAT );

   }

}

   //
   //  user function?
   //

if ( e && (! is_lhs) && (e->type() == UserFunctionType) )  {

   configlval.entry = e;

   return ( USER_FUNCTION ); 

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

         e = get_env(env_text);

         if ( !e )  {

            mlog << Error
                 << "\npatch_env() -> environment variable \""
                 << env_text << "\" not found!\n\n";

            exit ( 1 );

         }

         nn = strlen(e);

         if ( (k + nn) >= max_id_length )  {

            mlog << Error
                 << "\npatch_env() -> "
                 << "replacement text for environment variable \""
                 << env_text << "\" too long!\n\n";

            exit ( 1 );


         }

         strcpy(text + k, e);

         k += nn;

         j += strlen(env_text) + 2;   //  +3 for "${" and "}" and -1 'cuz j will get incremented at the top of the loop

      }

   } else text[k++] = c;

   if ( k >= max_id_length )  {

      mlog << Error
           << "\npatch_env() -> string too long!\n\n";

      exit ( 1 );

   }

}   //  for j

   //
   //  check for any more environment variables remaining in the string
   //

if ( strstr(text, "${") )  patch_env(text);

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

   mlog << Error
        << "\nget_env_text() -> closing bracket not found in string \""
        << line << "\"\n\n";

   exit ( 1 );

}

int n = (int) (c - (line + 2));

if ( n >= len )  {

   mlog << Error
        << "\nget_env_text() -> environment variable text too long in \""
        << line << "\"\n\n";

   exit ( 1 );

}

memset(env_text, 0, len);

strncpy(env_text, line + 2, n);


return;

}


////////////////////////////////////////////////////////////////////////


void start_string_scan(const char * s)

{

if ( empty(s) )  {

   mlog << Error
        << "\nvoid start_string_scan(const char *) -> empty string!\n\n";

   exit ( 1 );

}


string_buffer = config_scan_string(s);

config_switch_to_buffer(string_buffer);


return;

}


////////////////////////////////////////////////////////////////////////


void finish_string_scan()

{

config_delete_buffer(string_buffer);

string_buffer = 0;

return;

}


////////////////////////////////////////////////////////////////////////


bool do_eof()

{

if ( ! reading_env )  return ( true );

config_delete_buffer(YY_CURRENT_BUFFER);

config_switch_to_buffer(file_buffer);

reading_env = false;

Column += 3;

return ( false );

}


////////////////////////////////////////////////////////////////////////


void do_env()

{

int n;
char junk[max_id_length];
char * value = (char *) 0;



memset(junk, 0, sizeof(junk));

n = strlen(yytext);

if ( n >= (int) (sizeof(junk) - 1) )  {

   mlog << Error
        << "\ndo_env() -> environment variable name too long\n\n";

   exit ( 1 );

}

strncpy(junk, yytext + 2, sizeof(junk) - 1);

junk[n - 3] = (char) 0;

value = get_env(junk);

if ( !value )  {

   mlog << Error
        << "\ndo_env() -> can't get value of environment variable \""
        << junk << "\"\n\n";

   exit ( 1 );

}

file_buffer = YY_CURRENT_BUFFER;

reading_env = true;

env_buffer = config_scan_string(value);

config_switch_to_buffer(env_buffer);



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int do_fort_thresh()

{

strncpy(configlval.text, configtext, sizeof(configlval.text));

return ( FORTRAN_THRESHOLD );

}


////////////////////////////////////////////////////////////////////////









