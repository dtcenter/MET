

////////////////////////////////////////////////////////////////////////


static const bool verbose = true;


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <cstdio>
#include <cmath>


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


extern "C" { int configwrap(); }


////////////////////////////////////////////////////////////////////////

   //
   //  extern declarations
   //

extern int LineNumber;

extern int Column;

extern FILE * configin;

extern char * configtext;


////////////////////////////////////////////////////////////////////////


static unsigned char lexeme[max_id_length + 1];

static int pos = 0;   //  lexeme position

static const int char_class_other  = 1;
static const int char_class_digit  = 2;
static const int char_class_alpha  = 3;   //  includes underscore
static const int char_class_space  = 4;
static const int char_class_sign   = 5;   //  '+' or '-'
static const int char_class_dp     = 6;   //  decimal point '.'
static const int char_class_dollar = 7;   //  dollar sign   '$'

static int char_class[256];

static const int eof  = 0 ;
static const int skip = -2 ;

extern DictionaryStack * dict_stack;

extern IdentifierArray  ida;

extern bool is_lhs;

extern bool is_function_def;

extern void start_string_scan  (const char *);
extern void finish_string_scan ();

static const char L_curly = '{';
static const char R_curly = '}';

static ConcatString env_value;
static int env_index = 0;

static const int max_putback_chars = 10;

static int putback_chars [max_putback_chars];

static int n_putback_chars = 0;


inline bool have_putback() { return ( n_putback_chars > 0 ); }


static const char * fort_thresh_string [] = { "lt", "le", "gt", "ge", "ne", "eq" };

static const int n_fort_thresh_strings = sizeof(fort_thresh_string)/sizeof(*fort_thresh_string);


////////////////////////////////////////////////////////////////////////


static bool reading_env    = false;

static bool reading_comment = false;

static bool need_number     = false;

// static unsigned char * file_buffer = 0;


////////////////////////////////////////////////////////////////////////


static int next_token();

static void init();

static void clear_lexeme();

static bool char_ok(int);

static bool is_int ();
static bool is_id  ();

static bool is_float_v2();
static bool do_float();

static bool is_float_v2(const char *, int len);
static bool is_int     (const char *, int len);
static bool is_number  (const char *, int len);

static int  do_id();
static int  do_int();

static void do_single_char_token(char);

static void do_quoted_string();

static void do_c_comment();
static void do_cpp_comment();

static int nextchar();
static void putback(int);

static int pop_from_putback();

static int  my_put(int);

static int  token(int);

static int do_comp();

static int do_fort_thresh();

static bool replace_env(ConcatString &);

static bool is_fort_thresh_no_spaces();

static int  do_simple_perc_thresh();


////////////////////////////////////////////////////////////////////////


int configlex()

{

int t;
static bool first_call = true;

if ( first_call )  { init();  first_call = false; }

configtext = (char *) lexeme;

while ( 1 )  {

   t = next_token();

   if ( t == 0 )  return ( 0 );

   if ( t < 0 )  continue;

   // cout << "next\n";

   if ( t > 0 )  break;

}   //  while


// cout << "\n\n   my configlex() -> returned token " << t;
// 
// if ( lexeme[0] )  cout << " ... \"" << lexeme << "\"   LineNumber = " << LineNumber;
//  
// cout << "\n\n" << flush;

return ( t );

}


////////////////////////////////////////////////////////////////////////


int next_token()

{

int c, c2;

   //
   //  skip whitespace
   //

while ( 1 )  {

   c = nextchar();

   if ( c < 0 )  c += 256;

   // cout << "c = " << c << " (" << (char) c << ") ... LineNumber = " << LineNumber << ", Column = " << Column << "\n" << flush;

   if ( c == eof )  break;

   if ( char_class[c] != char_class_space )  break;

}   //  while

if ( c == eof )  return ( 0 );

clear_lexeme();

my_put(c);

switch ( c )  {

      //
      //  single character tokens
      //

   case '[':  { do_single_char_token(lexeme[0]);  is_lhs = false;  dict_stack->push_array();  return ( token(lexeme[0]) ); }  break;
   case '{':  { do_single_char_token(lexeme[0]);  is_lhs = true;   dict_stack->push();        return ( token(lexeme[0]) ); }  break;

   case ']':  { do_single_char_token(lexeme[0]);  return ( token(lexeme[0]) ); }  break;
   case '}':  { do_single_char_token(lexeme[0]);  return ( token(lexeme[0]) ); }  break;

   case '(':  { do_single_char_token(lexeme[0]);  return ( token(lexeme[0]) ); }  break;
   case ')':  { do_single_char_token(lexeme[0]);  return ( token(lexeme[0]) ); }  break;

   case '+':  { do_single_char_token(lexeme[0]);  return ( token(lexeme[0]) ); }  break;

   case '-':  { if ( ! need_number )  { do_single_char_token(lexeme[0]);  return ( token(lexeme[0]) ); } }  break;

   case '*':  { do_single_char_token(lexeme[0]);  return ( token(lexeme[0]) ); }  break;
   case '^':  { do_single_char_token(lexeme[0]);  return ( token(lexeme[0]) ); }  break;

   // case '=':  { do_single_char_token(lexeme[0]);  return ( token(lexeme[0]) ); }  break;


   case ';':  { do_single_char_token(lexeme[0]);  is_lhs = true;  return ( token( ';' ) ); }  break;
   case ',':  { do_single_char_token(lexeme[0]);  return ( token(lexeme[0]) ); }  break;

   case '\"': { do_quoted_string();   return ( token ( QUOTED_STRING ) ); }

   case '\n': { ++LineNumber;  Column = 1;   return ( -1 ); }

   default:
      break;

}   //  switch

   //
   //  two character tokens
   //

if ( c == '/' )  {

   c2 = nextchar();

   if ( c2 == eof )  return ( eof );

        if ( c2 == '*' )   { do_c_comment();     return ( skip ); }
   else if ( c2 == '/' )   { do_cpp_comment();   return ( skip ); }
   else                    putback(c2);

   return ( token('/') );

}

c2 = nextchar();

     if ( strncmp(configtext, "<=", 2) == 0 )  { need_number = true;  return ( do_comp() ); }
else if ( strncmp(configtext, ">=", 2) == 0 )  { need_number = true;  return ( do_comp() ); }
else if ( strncmp(configtext, "==", 2) == 0 )  { need_number = true;  return ( do_comp() ); }

else if ( strncmp(configtext, "!=", 2) == 0 )  { need_number = true;  return ( do_comp() ); }
else if ( strncmp(configtext, "NA", 2) == 0 )  return ( do_comp() );

else if ( strncmp(configtext, "&&", 2) == 0 )  { Column += 2;  return ( LOGICAL_OP_AND ); }
else if ( strncmp(configtext, "||", 2) == 0 )  { Column += 2;  return ( LOGICAL_OP_OR  ); }

else {

  putback(c2);

}


if ( strncmp(configtext, "<" , 1) == 0 )  { need_number = true;  return ( do_comp() ); }
if ( strncmp(configtext, ">" , 1) == 0 )  { need_number = true;  return ( do_comp() ); }
if ( strncmp(configtext, "=" , 1) == 0 )  return ( token( '=' ) );
if ( strncmp(configtext, "!" , 1) == 0 )  return ( token( LOGICAL_OP_NOT ) );


// putback(configtext[0]);

   //
   //  multi-character tokens: integers and identifiers
   //

int k = char_class[c];

if ( k == char_class_other )  return ( skip );

   //
   //  quote?
   //


   //
   //  from this point on, we're only interested in characters
   //    that are digits, decimal_points, letters, or underscore
   //

clear_lexeme();

my_put(c);

while ( pos < max_id_length )  {

   c = nextchar();

   if ( ! (char_ok(c)) )   { putback(c);  break; }

}   //  while

lexeme[max_id_length] = (char) 0;

if ( pos == 0 )  return ( skip );

// if ( strncmp((char *) lexeme, "enum",  max_id_length) == 0 )  { do_enum();   return ( token(ENUM)  ); }
// if ( strncmp((char *) lexeme, "class", max_id_length) == 0 )  { do_class();  return ( token(CLASS) ); }

if ( is_int() )  { if ( do_int() )  return ( token(INTEGER) ); }

if ( is_float_v2() )  { if ( do_float() )  return ( token(FLOAT) ); }

if ( is_fort_thresh_no_spaces() )  { return ( do_fort_thresh() ); }

int t;

if ( is_id() )  { t = do_id();  return ( token(t) ); }




return ( skip );

}


////////////////////////////////////////////////////////////////////////


void init()

{

int j;

for (j=0; j<256; ++j)     char_class[j] = char_class_other;

for (j='0'; j<='9'; ++j)  char_class[j] = char_class_digit;

for (j='A'; j<='Z'; ++j)  char_class[j] = char_class_alpha;
for (j='a'; j<='z'; ++j)  char_class[j] = char_class_alpha;

char_class['_']  = char_class_alpha;

char_class[' ']  = char_class_space;
char_class['\t'] = char_class_space;
char_class['\n'] = char_class_space;

char_class['+']  = char_class_sign;
char_class['-']  = char_class_sign;

char_class['.']  = char_class_dp;

char_class['$']  = char_class_dollar;


return;

}


////////////////////////////////////////////////////////////////////////


void clear_lexeme()

{

memset(lexeme, 0, sizeof(lexeme));

pos = 0;

int j;

for (j=n_putback_chars; j<max_putback_chars; ++j)  putback_chars[j] = 0;

return;

}


////////////////////////////////////////////////////////////////////////


bool char_ok(int c)

{

const int k = char_class[c];

if ( k == char_class_digit )  return ( true );

if ( k == char_class_alpha )  return ( true );

if ( k == char_class_sign  )  return ( true );

if ( c == '.' )  return ( true );


return ( false );

}


////////////////////////////////////////////////////////////////////////


bool is_int()

{

bool status = is_int((char *) lexeme, max_id_length);

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool is_float_v2 ()

{

bool status = is_float_v2((char *) lexeme, max_id_length);

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool is_id()

{

if ( char_class[lexeme[0]] != char_class_alpha )  return ( false );

int j, k;

for (j=0; j<max_id_length; ++j)  {

   if ( lexeme[j] == 0 )  break;

   k = char_class[lexeme[j]];

   if ( (k != char_class_digit) && (k != char_class_alpha) )  return ( false );

}   //  for j

return ( true );

}


////////////////////////////////////////////////////////////////////////


int do_id()

{

int j, k;

Column += strlen(configtext);

if ( is_lhs )  { strncpy(configlval.text, configtext, max_id_length);  return ( IDENTIFIER );  }

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

for (j=0; j<n_fort_thresh_strings; ++j)  {

   if ( strcmp(configtext, fort_thresh_string[j] ) == 0 )  { configlval.cval = thresh_lt;  return ( COMPARISON ); }

}





   //
   //  builtin ?
   //

int index;

 if ( (! is_lhs) && is_builtin((string)configtext, index) )  { configlval.index = index;  return ( BUILTIN ); }

   //
   //  local variable ?   //  ie, in argument list
   //

if ( is_function_def && ida.has(configtext, index) )  { configlval.index = index;  return ( LOCAL_VAR ); }

   //
   //  seen_before?
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


    ///////////////////////////////////////////////////////////////////////





   //
   //  fortran threshold without spaces?  (example: "le150")
   //

if ( (strncmp(configtext, "lt", 2) == 0) && is_number(configtext + 2, max_id_length - 2) )  { return ( do_fort_thresh() ); }

for (j=0; j<n_fort_thresh_strings; ++j)  {

   if (    (strncmp(configtext, fort_thresh_string[j], 2) == 0)
        && (is_number(configtext + 2, max_id_length - 2))
        )  { configlval.cval = thresh_lt;  return ( do_fort_thresh() ); }

}


   //
   //  simple percentile threshold?  (example: "SOP50")
   //

for (j=0; j<n_perc_thresh_infos; ++j)  {

   k = perc_thresh_info[j].short_name_length;

   if (    (strncmp(configtext, perc_thresh_info[j].short_name, k) == 0)
        && (is_number(configtext + k, max_id_length - k))  )
           { return ( do_simple_perc_thresh() ); }

}



    ///////////////////////////////////////////////////////////////////////


   //
   //  nope
   //

strncpy(configlval.text, configtext, sizeof(configlval.text) - 1);

need_number = false;

return ( IDENTIFIER );

}


////////////////////////////////////////////////////////////////////////


int do_int()

{

// Column += strlen(configtext);

configlval.nval.i = atoi(configtext);

configlval.nval.is_int = true;

need_number = false;


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


bool do_float()

{

// Column += strlen(configtext);

configlval.nval.d = atof(configtext);

configlval.nval.is_int = false;

need_number = false;


return ( true );

}


////////////////////////////////////////////////////////////////////////


void do_single_char_token(char c)

{

++Column;

// configlval.ival = 0;

configtext[0] = c;


return;

}


////////////////////////////////////////////////////////////////////////


void do_quoted_string()

{

clear_lexeme();

int n;
int c;
char * line = (char *) lexeme;

clear_lexeme();

n = 0;

while ( n < max_id_length )  {

   c = fgetc(configin);

   if ( c == EOF )  break;

   if ( c == '\"' )  break;

   if ( c == '\\' )  {

     c = fgetc(configin);

     if ( c == EOF )  break;

      switch ( c )  {

         case 'n':   line[n++] = '\n';  break;
         case 't':   line[n++] = '\t';  break;
         case 'b':   line[n++] = '\b';  break;
         case '\"':  line[n++] = '\"';  break;
         case '\\':  line[n++] = '\\';  break;

        default:
            line[n++] = (char) c;
            break;

      }   //  switch

   } else {

      line[n++] = c;

   }

   if ( (n + 1) >= max_id_length )  {

      mlog << Error << "\ndo_quoted_string() -> "
           << "string too long! ... c = \"" << c << "\"\n\n";

      exit ( 1 );

   }

}   //  while

if ( (pos > 0) && (lexeme[pos - 1] == '\"') )  { lexeme[pos - 1] = (char) 0;  --pos; }


 ConcatString s = (string)(char*)lexeme;

while ( replace_env(s) )  {

   // cout << "s = \"" << s << "\"\n\n" << flush;

}

if ( s.length() >= max_id_length )  {

   mlog << Error << "\ndo_quoted_string() -> "
        << "string \"" << s << "\" too long!\n\n";

   exit ( 1 );

}

clear_lexeme();

if ( s.nonempty() )  {

   strncpy((char *) lexeme, s.c_str(), max_id_length);

   strncpy(configlval.text, line, max_id_length);

} else {

   lexeme[0] = 0;

   configlval.text[0] = (char) 0;

}

lexeme[max_id_length] = 0;

configlval.text[ sizeof(configlval.text) - 1 ] = (char) 0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_c_comment()

{

reading_comment = true;

int c1, c2;


c1 = nextchar();
c2 = nextchar();


while ( 1 )  {

   if ( (c1 == eof) || (c2 == eof) )  break;

   if ( (c1 == '*') && (c2 == '/') )  break;

   c1 = c2;

   // c2 = nextchar();
   c2 = fgetc(configin);

}

reading_comment = false;


return;

}


////////////////////////////////////////////////////////////////////////


void do_cpp_comment()

{

int c = 0;

reading_comment = true;

while ( 1 )  {

   if ( feof (configin) )  break;

   // c = nextchar();
   c = fgetc(configin);

   if ( (c == eof) || (c == '\n') )  break;

}

if ( c == '\n' )  { ++LineNumber;  Column = 0; }

reading_comment = false;

return;

}


////////////////////////////////////////////////////////////////////////


void putback(int c)

{

if ( n_putback_chars >= max_putback_chars )  {

   mlog << Error << "\nbackchar(int) -> "
        << "can't have more than " << max_putback_chars
        << " putback chars!\n\n";

   exit ( 1 );

}


putback_chars[n_putback_chars++] = c;


if ( pos <= 0 )  {

   mlog << Error << "\nputback(int) -> "
        << "can't putback char before beginning of lexeme!\n\n";

   exit ( 1 );

}

lexeme[pos - 1] = 0;

--pos;



return;

}


////////////////////////////////////////////////////////////////////////


int pop_from_putback()

{

if ( n_putback_chars <= 0 )  {

   mlog << Error << "\npop_from_putback() -> "
        << "no putback chars!\n\n";

   exit ( 1 );

}

int k = putback_chars[n_putback_chars - 1];

putback_chars[n_putback_chars - 1] = 0;

--n_putback_chars;

return ( k );

}


////////////////////////////////////////////////////////////////////////


int nextchar()

{

int c, cc;

if ( have_putback() )  {

   c = pop_from_putback();

   return ( my_put(c) );

}



if ( reading_env )   {

   if ( env_index < (env_value.length()) )  return ( my_put((int) (env_value[env_index++])) );
   else {

      env_value.clear();

      env_index = 0;

      reading_env = false;

   }

   return ( nextchar() );

}   //  if reading env

   ////////////////////////////////////////////
   //
   //  not reading env
   //

if ( feof(configin) )  return ( eof );

c = fgetc(configin);

if ( c == '\n' )   { ++LineNumber;  Column = 0; }

if ( c == '$' )  {

   cc = fgetc(configin);

   ++Column;

   if ( cc != L_curly )  {

      mlog << Error << "\nnextchar() -> "
           << "unexpected \"$\" in line " << LineNumber
           << " of config file\n\n";

      exit ( 1 );

   }

   char env_name[max_id_length + 1];
   int env_pos = 0;
   char * e;

   memset(env_name, 0, sizeof(env_name));

   while ( (env_pos < max_id_length) && ((c = fgetc(configin)) != R_curly) )  env_name[env_pos++] = (char) c;

   e = getenv ( env_name );

   if ( !e )  {

      mlog << Error << "\nnextchar() -> "
           << "can't get value of environment variable \""
           << env_name << "\"\n\n";

      exit ( 1 );

   }

   env_value = e;

   while ( 1 )  {

      if ( ! replace_env(env_value) )  break;

   }

   reading_env = true;

   env_index = 0;

   return ( nextchar() );

}    //  if c == '$'


return ( my_put(c) );

}


////////////////////////////////////////////////////////////////////////


int my_put(int c)

{

if ( pos >= max_id_length )  {

   mlog << Error << "\nmy_put() -> "
        << "lexeme too long! ... partial lexeme = \"" << configtext
        << "\"\n\n";

   exit ( 1 );

}

configtext[pos++] = (char) c;

return ( c );

}


////////////////////////////////////////////////////////////////////////


int token(int t)

{

// cout << "token " << t << " ...  text = ";
// if ( (configtext != 0) && (configtext[0] != 0) )   cout << '\"' << configtext << "\"\n";
// else                                               cout << "(nul)\n";
// cout.flush();

return ( t );

}


////////////////////////////////////////////////////////////////////////


int do_comp()

{

// if ( verbose )  cout << "\n\n   ... in do_comp()\n\n" << flush;

int return_value = 0;

Column += strlen(configtext);

     if ( strcmp(configtext, "<" ) == 0 )  { configlval.cval = thresh_lt;  return_value = COMPARISON; }
else if ( strcmp(configtext, ">" ) == 0 )  { configlval.cval = thresh_gt;  return_value = COMPARISON; }
else if ( strcmp(configtext, "<=") == 0 )  { configlval.cval = thresh_le;  return_value = COMPARISON; }
else if ( strcmp(configtext, ">=") == 0 )  { configlval.cval = thresh_ge;  return_value = COMPARISON; }
else if ( strcmp(configtext, "==") == 0 )  { configlval.cval = thresh_eq;  return_value = COMPARISON; }
else if ( strcmp(configtext, "!=") == 0 )  { configlval.cval = thresh_ne;  return_value = COMPARISON; }

else if ( strcmp(configtext, na_str) == 0 )  { configlval.cval = thresh_na;  return_value = NA_COMPARISON; }

else {

   mlog << Error << "\ndo_comp() -> "
        << "bad comparison operator ... \"" << configtext << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( return_value );

}


////////////////////////////////////////////////////////////////////////


int do_fort_thresh()

{

strncpy(configlval.text, configtext, sizeof(configlval.text));

return ( FORTRAN_THRESHOLD );

}


////////////////////////////////////////////////////////////////////////


bool is_float_v2(const char * text, int len)

{

int j;
int c;
int state;
const int Error = -1;
bool has_exp = false;
int e_dig = 0;   //  exponent digit count
int m_dig = 0;   //  mantissa digit count
int m_dot = 0;   //  mantissa dot count


state = 0;

for (j=0; j<len; ++j)  {

   c = (int) (text[j]);

   if ( c == 0 )  break;

   if ( c < 0 )  c += 256;   //  I hate signed characters ...


   switch ( state )  {

      case 0:

              if ( (c == '+') || (c == '-') )  state = 1;
         else if ( c == '.' )                { ++m_dot;  state = 1; }
         else if ( isdigit(c) )              { ++m_dig;  state = 1; }
         else state = Error;

         break;


      case 1:

              if ( isdigit(c) )                { ++m_dig;  state = 1; }
         else if ( c == '.' )                  { ++m_dot;  state = ((m_dot > 1) ? Error : 1); }
         else if ( (c == 'e') || (c == 'E') )  { has_exp = true;  state = 2; }
         else state = Error;

         break;


      case 2:

              if ( (c == '+') || (c == '-') )  state = 3;
         else if ( isdigit(c) )              { ++e_dig;  state = 3; }
         else state = Error;

         break;


      case 3:

         if ( isdigit(c) )              { ++e_dig;  state = 3; }
         else state = Error;

         break;

      case Error:
         break;

      default:
         state = Error;
         break;

   }   //  switch

   if ( state == Error )  return ( false );

}   //  for j


   ////////////////////////////////////


   //
   //    do some tests
   //

      //
      //  gotta have at least one mantissa digit
      //

if ( m_dig < 1 )  return ( false );

      //
      //  if there is an exponent, then there
      //   had better be some exponent digits
      //

if ( has_exp && (e_dig < 1) )  return ( false );

      //
      //  If there is no exponent, then there has to be a
      //    single decimal point in the mantissa, otherwise
      //    it's an integer, not a float
      //
      //  Note: we could comment this out if we wanted
      //        the function to accept integers as well
      //        as floats
      //

if ( !has_exp && (m_dot != 1) )  return ( false );


   ///////////////////////////



   //
   //  grudgingly accept
   //

return ( true );

}

////////////////////////////////////////////////////////////////////////


bool is_int (const char * text, int len)

{

int j, k, m;
int j_start = 0;
int digit_count = 0;

m = text[0];

if ( m < 0 )  m += 256;

k = char_class[m];

if ( k == char_class_sign )  j_start = 1;

for (j=j_start; j<len; ++j)  {

   if ( text[j] == 0 )  break;

   m = text[j];

   if ( m < 0 )  m += 256;

   k = char_class[m];

   if ( k != char_class_digit )  return ( false );

   ++digit_count;

}   //  for j

return ( digit_count > 0 );

}


////////////////////////////////////////////////////////////////////////


bool is_number  (const char * s, int len)

{

if ( is_int(s, len) )  return  ( true );

if ( is_float_v2(s, len) )  return ( true );

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool replace_env(ConcatString & cs)

{

size_t pos1, pos2;
const std::string & s = cs.string();


if ( (pos1 = s.find("${", 0, 2)) == string::npos )  return ( false );

   //
   //  replace the environment variable
   //

if ( (pos2 = s.find('}', pos)) == string::npos )  {

   mlog << Error << "\nreplace_env() -> "
        << "can't closing brackent in string \"" << cs << "\"\n\n";

   exit ( 1 );

}

std::string out;
std::string env;
char * env_value = 0;

env = s.substr(pos1 + 2, pos2 - pos1 - 2);

env_value = getenv(env.c_str());

if ( ! env_value )  {

   mlog << Error << "\nreplace_env() -> "
        << "unable to get value for environment variable \""
        << (env.c_str()) << "\"\n\n";

   exit ( 1 );

}

out = s.substr(0, pos1);

out += env_value;

out += s.substr(pos2 + 1);


// cout << "\n\n   out = \"" << (out.c_str()) << "\n\n" << flush;



   //
   //  done
   //

cs.clear();

cs = out;

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool is_fort_thresh_no_spaces()

{

int j;

for (j=0; j<n_fort_thresh_strings; ++j)  {

   if ( (strncmp(configtext, fort_thresh_string[j], 2) == 0) && (is_number(configtext + 2, max_id_length - 2)) )    return ( true );

}


return ( false );

}


////////////////////////////////////////////////////////////////////////


int do_simple_perc_thresh()

{

int j, k;
int index = -1;
double value = bad_data_double;

for (j=0; j<n_perc_thresh_infos; ++j)  {

   k = perc_thresh_info[j].short_name_length;

   if ( strncmp(configtext, perc_thresh_info[j].short_name, k) == 0 )  {

      index = j;

      value = atof(configtext + k);

      break;

   }

}


if ( index < 0 )   {

   mlog << Error << "\ndo_simple_perc_thresh() -> "
        << "unable to parse string \"" << configtext << "\"\n\n";

   exit ( 1 );

}


configlval.pc_info.perc_index = index;
// configlval.pc_info.is_simple  = true;
configlval.pc_info.value     = value;
// configlval.pc_info.value2     = bad_data_double;;


return ( SIMPLE_PERC_THRESH );

}


////////////////////////////////////////////////////////////////////////






