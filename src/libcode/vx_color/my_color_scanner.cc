


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <cstdio>
#include <cmath>


#include "color_parser.h"   //  this must be included before color_parser_yacc.h
#include "color.h"

#include "color_parser_yacc.h"



////////////////////////////////////////////////////////////////////////


extern "C" { int colorwrap(); }


////////////////////////////////////////////////////////////////////////

   //
   //  extern declarations
   //

extern int color_file_line_number;

extern int color_file_column;

extern ColorList clist;

extern FILE * colorin;

extern char * colortext;


////////////////////////////////////////////////////////////////////////


static const int max_lexeme_size = 128;

static unsigned char lexeme[max_lexeme_size + 1];

static const int char_class_other = 1;
static const int char_class_digit = 2;
static const int char_class_alpha = 3;   //  includes underscore
static const int char_class_space = 4;
static const int char_class_sign  = 5;   //  '+' or '-'
static const int char_class_dp    = 6;   //  decimal point '.'

static int char_class[256];

static const int eof  = EOF ;
static const int skip = -2 ;


////////////////////////////////////////////////////////////////////////


static int next_token();

static void init();

static void clear_lexeme();

static bool char_ok(int);

static bool is_int ();
static bool is_id  ();

static bool is_float_v2();
static bool do_float();

static int  do_id();
static int  do_int();

static int  do_left_curly();
static int  do_right_curly();

static int  do_left_paren();
static int  do_right_paren();

static int  do_comma();
static int  do_equals();

static void do_quoted_string();

static void do_c_comment();
static void do_cpp_comment();

static int  nextchar();

static int  token(int);


////////////////////////////////////////////////////////////////////////


int colorlex()

{

int t;
static bool first_call = true;

if ( first_call )  { init();  first_call = false; }

colortext = (char *) lexeme;

while ( 1 )  {

   t = next_token();

   if ( t == eof )  return ( 0 );

   if ( t < 0 )  continue;

   // cout << "next\n";

   if ( t > 0 )  break;

   // if ( t == eof           )  break;
   // if ( t == COLOR_NAME    )  break;
   // if ( t == BLEND         )  break;
   // if ( t == HSV           )  break;
   // if ( t == CMYK          )  break;
   // if ( t == GRAYVALUE     )  break;
   // if ( t == QUOTED_STRING )  break;
   // if ( t == ID            )  break;

}   //  while


// cout << "\n\n   my colorlex() -> returned token " << t;
// 
// if ( lexeme[0] )  cout << " ... \"" << lexeme << "\"   color_file_line_number = " << color_file_line_number;
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

   // cout << "c = " << c << " (" << (char) c << ") ... color_file_line_number = " << color_file_line_number << ", color_file_column = " << color_file_column << "\n" << flush;

   if ( c == eof )  break;

   if ( char_class[c] != char_class_space )  break;

}   //  while

if ( c == eof )  return ( eof );

clear_lexeme();

switch ( c )  {

      //
      //  single character tokens
      //

   case '{':  { if ( do_left_curly()  )  return ( token('{') ); }  break;
   case '}':  { if ( do_right_curly() )  return ( token('}') ); }  break;
   case '(':  { if ( do_left_paren()  )  return ( token('(') ); }  break;
   case ')':  { if ( do_right_paren() )  return ( token(')') ); }  break;
   case ',':  { if ( do_comma()       )  return ( token(',') ); }  break;
   case '=':  { if ( do_equals()      )  return ( token('=') ); }  break;

   default:
      break;

}   //  switch

   //
   //  two character tokens
   //

if ( c == '/' )  {

   c2 = nextchar();

   if ( c2 == EOF )  return ( EOF );

        if ( c2 == '*' )   do_c_comment();
   else if ( c2 == '/' )   do_cpp_comment();
   else                    ungetc(c2, colorin);

   return ( skip );

}

   //
   //  multi-character tokens: integers and identifiers
   //

int k = char_class[c];

if ( k == char_class_other )  return ( skip );

   //
   //  quote?
   //

if ( c == '\"' )  { do_quoted_string();   return ( token ( QUOTED_STRING ) ); }

   //
   //  from this point on, we're only interested in characters
   //    that are digits, decimal_points, letters, or underscore
   //

clear_lexeme();

lexeme[0] = (unsigned char) c;

int count = 1;

while ( count < max_lexeme_size )  {

   c = nextchar();

   if ( char_ok((unsigned char) c) )  lexeme[count++] = (unsigned char) c;
   else                               { ungetc(c, colorin);  break; }

}   //  while

lexeme[max_lexeme_size] = (char) 0;

if ( count == 0 )  return ( skip );

// if ( strncmp((char *) lexeme, "enum",  max_lexeme_size) == 0 )  { do_enum();   return ( token(ENUM)  ); }
// if ( strncmp((char *) lexeme, "class", max_lexeme_size) == 0 )  { do_class();  return ( token(CLASS) ); }

if ( is_int() )  { if ( do_int() )  return ( token(INTEGER) ); }

if ( is_float_v2() )  { if ( do_float() )  return ( token(FLOAT) ); }

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


return;

}


////////////////////////////////////////////////////////////////////////


void clear_lexeme()

{

memset(lexeme, 0, sizeof(lexeme));

return;

}


////////////////////////////////////////////////////////////////////////


bool char_ok(int c)

{

const int k = char_class[c];

if ( k == char_class_digit )  return ( true );

if ( k == char_class_alpha )  return ( true );

if ( c == '.' )  return ( true );


return ( false );

}


////////////////////////////////////////////////////////////////////////


bool is_int()

{

int j, k;
int j_start = 0;
int digit_count = 0;

k = char_class[lexeme[0]];

if ( k == char_class_sign )  j_start = 1;

for (j=j_start; j<max_lexeme_size; ++j)  {

   if ( lexeme[j] == 0 )  break;

   k = char_class[lexeme[j]];

   if ( k != char_class_digit )  return ( false );

   ++digit_count;

}   //  for j



return ( digit_count > 0 );

}


////////////////////////////////////////////////////////////////////////


bool is_float_v2 ()

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

for (j=0; j<max_lexeme_size; ++j)  {

   c = (int) (lexeme[j]);

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


bool is_id()

{

if ( char_class[lexeme[0]] != char_class_alpha )  return ( false );

int j, k;

for (j=0; j<max_lexeme_size; ++j)  {

   if ( lexeme[j] == 0 )  break;

   k = char_class[lexeme[j]];

   if ( (k != char_class_digit) && (k != char_class_alpha) )  return ( false );

}   //  for j

return ( true );

}


////////////////////////////////////////////////////////////////////////


int do_id()

{

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


int do_int()

{

// color_file_column += strlen(colortext);

colorlval.ival = atoi(colortext);



return ( 1 );

}


////////////////////////////////////////////////////////////////////////


bool do_float()

{

// color_file_column += strlen(colortext);

colorlval.dval = atof(colortext);



return ( true );

}


////////////////////////////////////////////////////////////////////////


int do_left_curly()

{

++color_file_column;

colorlval.ival = 0;

lexeme[0] = '{';


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int do_right_curly()

{

++color_file_column;

colorlval.ival = 0;

lexeme[0] = '}';


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int do_left_paren()

{

++color_file_column;

colorlval.ival = 0;

lexeme[0] = '(';

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int do_right_paren()

{

++color_file_column;

colorlval.ival = 0;

lexeme[0] = ')';

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int do_comma()

{

++color_file_column;

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int do_equals()

{

++color_file_column;

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


void do_quoted_string()

{

int n;
char c;
char line[max_lexeme_size + 1];

memset(line, 0, sizeof(line));

n = 0;

while ( n < max_lexeme_size )  {

   c = nextchar();

   if ( c == '\"' )  break;

   if ( c == '\\' )  {

      c = nextchar();

      switch ( c )  {

         case 'n':   line[n++] = '\n';  break;
         case 't':   line[n++] = '\t';  break;
         case 'b':   line[n++] = '\b';  break;
         case '\"':  line[n++] = '\"';  break;
         case '\\':  line[n++] = '\\';  break;

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

// if ( c == '\n' )  { ++color_file_line_number;  color_file_column = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


int nextchar()

{

int c;


c = fgetc(colorin);

// if ( c < 0 )  c += 256;   //  fgetc returns unsigned char cast to int
                             //  so don't need this

++color_file_column;

if ( c == '\n' )  {

   ++color_file_line_number;

   color_file_column = 0;

}


return ( c );

}


////////////////////////////////////////////////////////////////////////


int token(int t)

{

return ( t );

}


////////////////////////////////////////////////////////////////////////




