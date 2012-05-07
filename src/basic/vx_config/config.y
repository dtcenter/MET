

%{

////////////////////////////////////////////////////////////////////////


#define YYDEBUG 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "math_constants.h"
#include "scanner_stuff.h"
#include "dictionary.h"
#include "threshold.h"
#include "pwl.h"


////////////////////////////////////////////////////////////////////////


   //
   //  declarations that have external linkage
   //


extern int            yylex();

extern void           yyerror(const char *);

extern "C" int        configwrap();


extern char *         configtext;

extern FILE *         configin;


   //
   //  definitions that have external linkage
   //


int               LineNumber            = 1;

int               Column                = 1;

const char *      bison_input_filename  = (const char *) 0;

DictionaryStack * dict_stack            = (DictionaryStack *) 0;

bool              is_lhs                = true;


////////////////////////////////////////////////////////////////////////


   //
   //  static definitions
   //

static PiecewiseLinear pwl;

static Dictionary DD;

static SingleThresh ST;


////////////////////////////////////////////////////////////////////////


   //
   //  static declarations
   //

static Number do_op(char op, const Number & a, const Number & b);

static Number do_integer_op(char op, const Number & a, const Number & b);

static Number do_negate(const Number &);


static void do_assign_boolean   (const char * name, bool);
static void do_assign_exp       (const char * name, const Number &);
static void do_assign_string    (const char * name, const char * text);
static void do_assign_threshold (const char * name);

static void do_assign_id      (const char * LHS, const char * RHS);

static void do_assign_dict    (const char * name);

static void do_dict();

static void do_string(const char *);

static void do_number(const Number &);

// static void do_array(const char * LHS);

static void do_thresh(const ThreshType, const Number &);

static void do_na_thresh();

static void add_point(const Number &, const Number &);

static void do_pwl(const char * LHS);


////////////////////////////////////////////////////////////////////////


%}


%union {

   char text[max_id_length];

   Number nval;

   bool bval;

   ThreshType cval;

}


%token IDENTIFIER QUOTED_STRING INTEGER FLOAT BOOLEAN
%token COMPARISON NA_COMPARISON


%type <text> IDENTIFIER QUOTED_STRING assign_prefix array_prefix

%type <nval> INTEGER FLOAT number expression

%type <bval> BOOLEAN

%type <cval> COMPARISON NA_COMPARISON


%left '+' '-'
%left '*' '/'
%left '^'
%nonassoc UNARY_MINUS


%%

assignment_list : assignment                    { is_lhs = true; }
                | assignment_list assignment    { is_lhs = true; }
                ;


assignment : assign_prefix BOOLEAN          ';'        { do_assign_boolean   ($1, $2); }
           | assign_prefix expression       ';'        { do_assign_exp       ($1, $2); }
           | assign_prefix IDENTIFIER       ';'        { do_assign_id        ($1, $2); }
           | assign_prefix piecewise_linear ';'        { do_pwl              ($1); }

           | assign_prefix threshold        ';'        { do_assign_threshold ($1); }
           | assign_prefix QUOTED_STRING    ';'        { do_assign_string    ($1, $2); }
           | assign_prefix dictionary                  { do_assign_dict      ($1); }

           | array_prefix number_list     ']' ';'      { do_assign_dict($1); }
           | array_prefix string_list     ']' ';'      { do_assign_dict($1); }
           | array_prefix threshold_list  ']' ';'      { do_assign_dict($1); }
           | array_prefix dictionary_list ']' ';'      { do_assign_dict($1); }
           | array_prefix ']' ';'                      { do_assign_dict($1); }

           ;


assign_prefix : IDENTIFIER '='     { is_lhs = false;  strcpy($$, $1); }
              ;


array_prefix : assign_prefix '['   { is_lhs = false;  strcpy($$, $1); }
             ;


dictionary : '{' assignment_list '}'  opt_semi  { do_dict(); }
           ;


dictionary_list : dictionary
                | dictionary_list ',' dictionary
                ;


string_list : QUOTED_STRING                  { do_string($1); }
            | string_list ',' QUOTED_STRING  { do_string($3); }
            ;


threshold_list : threshold
               | threshold_list ',' threshold
               ;


threshold : COMPARISON number { do_thresh($1, $2); }
          | NA_COMPARISON { do_na_thresh(); }
          ;


number : INTEGER { }
       | FLOAT   { }
       ;


number_list : number                   { do_number($1); }
            | number_list ',' number   { do_number($3); }
            ;


opt_semi : ';'
         |  /*  nothing  */
         ;


expression : number                                     { $$ = $1; }
           | expression '+' expression                  { $$ = do_op('+', $1, $3); }
           | expression '-' expression                  { $$ = do_op('-', $1, $3); }
           | expression '*' expression                  { $$ = do_op('*', $1, $3); }
           | expression '/' expression                  { $$ = do_op('/', $1, $3); }
           | expression '^' expression                  { $$ = do_op('^', $1, $3); }
           | '-' expression  %prec UNARY_MINUS          { $$ = do_negate($2); }
           | '(' expression ')'                         { $$ = $2; }
           ;


piecewise_linear : '[' point_list ']'   { }
                 ;


point_list : point              { }
           | point_list point   { }
           ;


point : '(' number ',' number ')'   { add_point($2, $4); }



%%


////////////////////////////////////////////////////////////////////////


   //
   //  standard yacc stuff
   //


////////////////////////////////////////////////////////////////////////


void yyerror(const char * s)

{

int j, j1, j2;
int line_len, text_len;
int c;
char line[512];
ifstream in;
ConcatString msg;


c = (int) (Column - strlen(configtext));

mlog << Error
     << "\n"
     << "yyerror() -> syntax error in file \"" << bison_input_filename << "\"\n\n"
     << "   line   = " << LineNumber << "\n\n"
     << "   column = " << c << "\n\n"
     << "   text   = \"" << configtext << "\"\n\n";

in.open(bison_input_filename);

for (j=1; j<LineNumber; ++j)  {   //  j starts at one here, not zero

   in.getline(line, sizeof(line));

}

in.getline(line, sizeof(line));

in.close();




mlog << Error
     << "\n" << line << "\n";

line_len = strlen(line);

text_len = strlen(configtext);

j1 = c;
j2 = c + text_len - 1;

msg.erase();
for (j=1; j<=line_len; ++j)  {   //  j starts at one here, not zero

   if ( (j >= j1) && (j <= j2) )  msg << '^';
   else                           msg << '_';

}

mlog << Error
     << msg << "\n\n";


exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


int configwrap()

{

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


Number do_op(char op, const Number & a, const Number & b)

{

Number c;
double A, B, C;

   //
   //  treat ^ as a special case
   //

if ( op == '^' )  {

   A = as_double(a);   
   B = as_double(b);   

   C = pow(A, B);

   set_double(c, C);

   return ( c );

}

   //
   //  for all other operators, see if we're doing integer arithmetic
   //

if ( a.is_int && b.is_int )  {

   c = do_integer_op(op, a, b);

   return ( c );

}

   //
   //  do floating-point arithmetic
   //

A = as_double(a);   
B = as_double(b);   

switch ( op )  {

   case '+':  C = A + B;  break;
   case '-':  C = A - B;  break;
   case '*':  C = A * B;  break;
   case '/':
      if ( B == 0.0 )  {
         mlog << Error
              << "\ndo_op() -> "
              << "division by zero!\n\n";
         exit ( 1 );
      }
      C = A / B;
      break;

   default:
      mlog << Error
           << "\ndo_op() -> "
           << "bad operator ... \"" << op << "\"\n\n";
      exit ( 1 );
      break;

}

set_double(c, C);

   //
   //  done
   //

return ( c );

}


////////////////////////////////////////////////////////////////////////


Number do_integer_op(char op, const Number & a, const Number & b)

{

Number c;
int A, B, C;

A = a.i;
B = b.i;

switch ( op )  {

   case '+':  C = A + B;  break;
   case '-':  C = A - B;  break;
   case '*':  C = A * B;  break;
   case '/':
      if ( B == 0 )  {
         mlog << Error
              << "\ndo_integer_op() -> "
              << "division by zero!\n\n";
         exit ( 1 );
      }
      C = A / B;
      break;

   default:
      mlog << Error
           << "\ndo_integer_op() -> "
           << "bad operator ... \"" << op << "\"\n\n";
      exit ( 1 );
      break;

}

set_int(c, C);

   //
   //  done
   //

return ( c );

}


////////////////////////////////////////////////////////////////////////


Number do_negate(const Number & a)

{

Number b;

if ( a.is_int )  set_int    (b, -(a.i));
else             set_double (b, -(a.d));

return ( b );

}


////////////////////////////////////////////////////////////////////////


void do_assign_boolean(const char * name, bool tf)

{

DictionaryEntry entry;

entry.set_boolean(name, tf);

dict_stack->store(entry);

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_threshold(const char * name)

{

DictionaryEntry e;

e.set_threshold(name, ST);

dict_stack->store(e);

// dict_stack->pop_element(name);

ST.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_exp(const char * name, const Number & n)

{

DictionaryEntry entry;

if ( n.is_int )  entry.set_int    (name, n.i);
else             entry.set_double (name, n.d);

dict_stack->store(entry);


return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_string(const char * name, const char * text)

{

DictionaryEntry entry;

entry.set_string(name, text);

dict_stack->store(entry);

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_id(const char * LHS, const char * RHS)

{

const DictionaryEntry * e = dict_stack->lookup(RHS);

if ( !e )  {

   mlog << Error
        << "\ndo_assign_id() -> "
        << "identifier \"" << RHS
        << "\" not defined in this scope!\n\n";

   exit ( 1 );

}

DictionaryEntry ee = *e;

ee.set_name(LHS);

dict_stack->store(ee);

return;

}


////////////////////////////////////////////////////////////////////////


void do_dict()

{

// cout << "   in do_dict()\n\n\n";

DD = *(dict_stack->top());

dict_stack->erase_top();

if ( ! dict_stack->top_is_array() )  return;

DictionaryEntry e;

e.set_dict(0, DD);

dict_stack->store(e);

DD.clear();

// dict_stack->pop_dict(0);

// dict_stack->dump(cout);
// cout << "   in do_dict()\n\n\n";
// cout.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_dict (const char * name)

{

// const bool is_array = dict_stack->top_is_array();

// cout << "\n\n  in do_assign_dict() ... DD.n_entries() = " << DD.n_entries() << "\n\n" << flush;

if ( DD.n_entries() > 0 )  {

   DictionaryEntry e;

   e.set_dict(name, DD);

   dict_stack->store(e);

   DD.clear();

} else {

   dict_stack->pop_dict(name);

}

return;

}


////////////////////////////////////////////////////////////////////////


void do_string(const char * text)

{

DictionaryEntry e;

e.set_string(0, text);

dict_stack->store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void do_number(const Number & number)

{

DictionaryEntry e;

if ( number.is_int )  e.set_int    (0, number.i);
else                  e.set_double (0, number.d);

dict_stack->store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void do_thresh(const ThreshType t, const Number & n)

{

if ( ! dict_stack->top_is_array() )  {

   ST.set(as_double(n), t);

   return;

}

DictionaryEntry e;
SingleThresh T;

T.set(as_double(n), t);

e.set_threshold(0, T);

dict_stack->store(e);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_na_thresh()

{

if ( !dict_stack->top_is_array() )  return;

DictionaryEntry e;
SingleThresh T;

T.set(na_str);

e.set_threshold(0, T);

dict_stack->store(e);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void add_point(const Number & x, const Number & y)

{

pwl.add_point(as_double(x), as_double(y));

return;

}


////////////////////////////////////////////////////////////////////////


void do_pwl(const char * LHS)

{

DictionaryEntry e;

e.set_pwl(LHS, pwl);

dict_stack->store(e);


   //
   //  done
   //

pwl.clear();

return;

}


////////////////////////////////////////////////////////////////////////





