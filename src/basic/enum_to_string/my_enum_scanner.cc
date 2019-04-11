


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <cstdio>
#include <cmath>


#include "enum.tab.h"

#include "scope.h"


////////////////////////////////////////////////////////////////////////


extern "C" { int yywrap(); }


////////////////////////////////////////////////////////////////////////

   //
   //  extern declarations
   //

extern int LineNumber;

extern int column;

extern int enum_mode;

extern ScopeStack ss;

extern ScopeStackElement sse;

extern int saw_class_prefix;

extern FILE * yyin;

extern char * yytext;

   //
   //  extern definitions
   //

int bracket_level = 0;

   //
   //  static definitions
   //

static int last_was_class = 0;
static int last_was_enum  = 0;

static const int max_lexeme_size = 128;

static unsigned char lexeme[max_lexeme_size + 1];

static const int char_class_other = 1;
static const int char_class_digit = 2;
static const int char_class_alpha = 3;   //  includes underscore
static const int char_class_space = 4;
static const int char_class_sign  = 5;   //  '+' or '-'

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

static int  nextchar();

static int  token(int);


////////////////////////////////////////////////////////////////////////


int yylex()

{

int t;
static bool first_call = true;

if ( first_call )  { init();  first_call = false; }

yytext = (char *) lexeme;

while ( 1 )  {

   t = next_token();

   if ( t == eof )  return ( 0 );

   if ( t < 0 )  continue;

   // cout << "next\n";

   if ( t == eof   )  break;
   if ( t == CLASS )  break;
   if ( t == ENUM  )  break;
   if ( t == ID    )  break;

   if ( enum_mode )  break;

}   //  while


// cout << "\n\n   my yylex() -> returned token " << t;
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

   // cout << "c = " << c << " (" << (char) c << ") ... LineNumber = " << LineNumber << ", column = " << column << "\n" << flush;

   if ( c == eof )  break;

   if ( char_class[c] != char_class_space )  break;

}   //  while

if ( c == eof )  return ( eof );

clear_lexeme();

switch ( c )  {

      //
      //  single character tokens
      //

   case '{':  { if ( do_left_curly()  )  return ( token(L_CURLY) ); }  break;
   case '}':  { if ( do_right_curly() )  return ( token(R_CURLY) ); }  break;
   case '=':  { if ( do_equals()      )  return ( token (EQ) );     }  break;
   case ';':  { if ( do_semi()        )  return ( token(';') );     }  break;
   case ',':  { if ( do_comma()       )  return ( token(',') );     }  break;

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
   else                    ungetc(c2, yyin);

   return ( skip );

}

   //
   //  multi-character tokens: integers and identifiers
   //

int k = char_class[c];

if ( k == char_class_other )  return ( skip );

   //
   //  from this point on, we're only interested in characters
   //    that are digits, letters, or underscore
   //

clear_lexeme();

lexeme[0] = (unsigned char) c;

int count = 1;

while ( count < max_lexeme_size )  {

   c = nextchar();

   if ( char_ok(c) )  lexeme[count++] = (unsigned char) c;
   else               { ungetc(c, yyin);  break; }

}   //  while

lexeme[max_lexeme_size] = (char) 0;

if ( count == 0 )  return ( skip );

if ( strncmp((char *) lexeme, "enum",  max_lexeme_size) == 0 )  { do_enum();   return ( token(ENUM)  ); }
if ( strncmp((char *) lexeme, "class", max_lexeme_size) == 0 )  { do_class();  return ( token(CLASS) ); }

if ( is_int() )  { if ( do_int() )  return ( token(INTEGER) ); }

if ( is_id() )  { if ( do_id() )  return ( token(ID) ); }




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

// column += strlen(yytext);

if ( enum_mode || last_was_enum || last_was_class )  {

   strncpy(yylval.name, yytext, sizeof(yylval.name));

   return ( 1 );

}



return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int do_int()

{

// column += strlen(yytext);

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

lexeme[0] = '{';


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

lexeme[0] = '}';


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int do_semi()

{

++column;

if ( enum_mode )  {

   lexeme[0] = '}';

   return ( 1 );

}



return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int do_equals()

{

++column;

if ( enum_mode )  {

   lexeme[0] = '=';

   return ( 1 );

}


return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int do_comma()

{

++column;

if ( enum_mode )  {

   lexeme[0] = ',';

   return ( 1 );

}


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

// if ( c == '\n' )  { ++LineNumber;  column = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


int nextchar()

{

int c;


c = fgetc(yyin);

// if ( c < 0 )  c += 256;   //  fgetc returns unsigned char cast to int
                             //  so don't need this

++column;

if ( c == '\n' )  {

   ++LineNumber;

   column = 0;

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




