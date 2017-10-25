

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
#include "is_bad_data.h"
#include "scanner_stuff.h"
#include "dictionary.h"
#include "threshold.h"
#include "is_number.h"
#include "concat_string.h"
#include "pwl.h"
#include "icode.h"
#include "idstack.h"

#include "scanner_stuff.h"
#include "threshold.h"


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

bool              is_lhs                = true;    //  used by the scanner

ThreshNode *      result                = 0;   //  for testing

bool              test_mode             = false;

// ConcatString   number_string;

char number_string [max_id_length];


////////////////////////////////////////////////////////////////////////


   //
   //  static definitions
   //

static PiecewiseLinear pwl;

static Dictionary DD;

static SingleThresh STH;

static const char print_prefix [] = "config: ";

static bool is_function_def = false;

static  ICVStack         icvs;

static  IdentifierArray  ida;


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

static void do_boolean(const bool &);

static void do_number(const Number &);

static void do_thresh(ThreshNode *);

static void do_na_thresh();

static void add_point(const Number &, const Number &);

static void do_pwl(const char * LHS);


static ThreshNode * do_and_thresh    (ThreshNode *, ThreshNode *);
static ThreshNode * do_or_thresh     (ThreshNode *, ThreshNode *);
static ThreshNode * do_not_thresh    (ThreshNode *);
static ThreshNode * do_paren_thresh  (ThreshNode *);
static ThreshNode * do_simple_thresh (ThreshType, const Number &);

static ThreshNode * do_fortran_thresh(const char *);


static void set_number_string();
static void set_number_string(const char *);

static void do_print(const Number &);
static void do_print(const char *, const Number &);

static void do_function_def();


////////////////////////////////////////////////////////////////////////


%}


%union {

   char text[max_id_length];

   Number nval;

   bool bval;

   int index;

   ThreshType cval;

   ThreshNode * node;

}


%token IDENTIFIER QUOTED_STRING INTEGER FLOAT BOOLEAN
%token COMPARISON NA_COMPARISON
%token LOGICAL_OP_NOT LOGICAL_OP_AND LOGICAL_OP_OR
%token FORTRAN_THRESHOLD
%token BUILTIN

%token USER_FUNCTION
%token PRINT


%type <text> IDENTIFIER QUOTED_STRING assign_prefix array_prefix FORTRAN_THRESHOLD

%type <nval> INTEGER FLOAT number expression

%type <index> BUILTIN

%type <bval> BOOLEAN

%type <cval> COMPARISON NA_COMPARISON

%type <node> simple_thresh thresh_node


%left '+' '-'
%left '*' '/'
%left '^'

%left LOGICAL_OP_AND LOGICAL_OP_OR

%nonassoc UNARY_MINUS
%nonassoc LOGICAL_OP_NOT


%%


statement_list : statement                { is_lhs = true; }
               | statement_list statement { is_lhs = true; }


statement : assign_stmt   { is_lhs = true; }
          | print_stmt    { is_lhs = true; }
          | threshold     { }
          ;


print_stmt : PRINT expression                         ';' { do_print($2); }
           | PRINT QUOTED_STRING opt_comma expression ';' { do_print($2, $4); }
           ;



assign_stmt : assign_prefix BOOLEAN            ';'      { do_assign_boolean   ($1, $2); }
            | assign_prefix expression         ';'      { do_assign_exp       ($1, $2); }
            | assign_prefix IDENTIFIER         ';'      { do_assign_id        ($1, $2); }
            | assign_prefix piecewise_linear   ';'      { do_pwl              ($1); }

            | assign_prefix threshold          ';'      { do_assign_threshold ($1); }
            | assign_prefix QUOTED_STRING      ';'      { do_assign_string    ($1, $2); }
            | assign_prefix dictionary                  { do_assign_dict      ($1); }

            | array_prefix boolean_list    ']' ';'      { do_assign_dict($1); }
            | array_prefix expression_list ']' ';'      { do_assign_dict($1); }
            | array_prefix string_list     ']' ';'      { do_assign_dict($1); }
            | array_prefix threshold_list  ']' ';'      { do_assign_dict($1); }
            | array_prefix dictionary_list ']' ';'      { do_assign_dict($1); }
            | array_prefix                 ']' ';'      { do_assign_dict($1); }

            | function_prefix expression       ';'      { do_function_def();  }

            ;



id_list : IDENTIFIER             {}
        | id_list ',' IDENTIFIER {}
        ;


function_prefix : IDENTIFIER '(' id_list ')' '='    { is_function_def = true; }
                ;


assign_prefix : IDENTIFIER '='     { is_lhs = false;  strcpy($$, $1); }
              ;


array_prefix : assign_prefix '['   { is_lhs = false;  strcpy($$, $1); }
             ;


dictionary : '{' statement_list '}'  opt_semi  { do_dict(); }
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

threshold : thresh_node            { do_thresh    ($1); }
          | NA_COMPARISON          { do_na_thresh (); }
          ;

thresh_node : simple_thresh                          { $$ = $1; }
            | thresh_node LOGICAL_OP_AND thresh_node { $$ = do_and_thresh   ($1, $3); }
            | thresh_node LOGICAL_OP_OR  thresh_node { $$ = do_or_thresh    ($1, $3); }
            | LOGICAL_OP_NOT             thresh_node { $$ = do_not_thresh   ($2);     }
            | '(' thresh_node ')'                    { $$ = do_paren_thresh ($2);     }
            ;


simple_thresh : COMPARISON number { $$ = do_simple_thresh($1, $2); }
              | FORTRAN_THRESHOLD { $$ = do_fortran_thresh($1); }
              ;


number : INTEGER { set_number_string(); }
       | FLOAT   { set_number_string(); }
       ;
    

boolean_list : BOOLEAN                   { do_boolean($1); }
            | boolean_list ',' BOOLEAN   { do_boolean($3); }
            ;


opt_semi : ';'
         |  /*  nothing  */
         ;


opt_comma : ','
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
           | BUILTIN       '(' expression ')'           {  }
           | USER_FUNCTION '(' expression ')'           {  }
           ;



expression_list : expression                     { do_number($1); }
                | expression_list ',' expression { do_number($3); }
                ;


piecewise_linear : '(' point_list ')'   { }
                 ;


point_list : point              { }
           | point_list point   { }
           ;


point : '(' expression ',' expression ')'   { add_point($2, $4); }



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
char line[max_id_length + 1];
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
      if ( is_eq(B, 0.0) )  {
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

e.set_threshold(name, STH);

dict_stack->store(e);

STH.clear();

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

DD = *(dict_stack->top());

dict_stack->erase_top();

if ( ! dict_stack->top_is_array() )  return;

DictionaryEntry e;

e.set_dict(0, DD);

dict_stack->store(e);

DD.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_dict (const char * name)

{

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


void do_boolean(const bool & boolean)

{

DictionaryEntry e;

e.set_boolean(0, boolean);

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


void do_thresh(ThreshNode * node)

{

if ( test_mode )  {

   result = node;

} 

else {

   if ( dict_stack->top_is_array() )  {

      DictionaryEntry e;
      SingleThresh T;

      T.set(node);

      e.set_threshold(0, T);

      dict_stack->store(e);

   }  else STH.set(node);

}

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


ThreshNode * do_and_thresh    (ThreshNode * a, ThreshNode * b)

{

And_Node * n = new And_Node;

n->left_child  = a;
n->right_child = b;

n->s << a->s << "&&" << b->s;

n->abbr_s << a->abbr_s << ".and." << b->abbr_s;

return ( n );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_or_thresh (ThreshNode * a, ThreshNode * b)

{

Or_Node * n = new Or_Node;

n->left_child  = a;
n->right_child = b;

n->s << a->s << "||" << b->s;

n->abbr_s << a->abbr_s << ".or." << b->abbr_s;

return ( n );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_not_thresh    (ThreshNode * n)

{

Not_Node * nn = new Not_Node;

nn->child = n;

nn->s << '!' << n->s;

nn->abbr_s << ".not." << n->abbr_s;

return ( nn );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_paren_thresh  (ThreshNode * n)

{

ConcatString b;

b.erase();

b << '(' << n->s << ')';

n->s = b;

b.erase();

b << '(' << n->abbr_s << ')';

n->abbr_s = b;

return ( n );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_simple_thresh (ThreshType op, const Number & n)

{

Simple_Node * s = new Simple_Node;

s->op = op;

s->T = as_double(n);

if ( op >= 0 )  {

   s->s      << thresh_type_str[op] << number_string;
   s->abbr_s << thresh_abbr_str[op] << number_string;

}

return ( s );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_fortran_thresh(const char * text)

{

ThreshType op  = no_thresh_type;
const char * p = text + 2;         //  we know that all the prefixes
                                   //  (like "le" or "gt") are two  
                                   //  characters long

     if ( strncmp(text, "le", 2) == 0 )  op = thresh_le;
else if ( strncmp(text, "lt", 2) == 0 )  op = thresh_lt;

else if ( strncmp(text, "gt", 2) == 0 )  op = thresh_gt;
else if ( strncmp(text, "ge", 2) == 0 )  op = thresh_ge;

else if ( strncmp(text, "eq", 2) == 0 )  op = thresh_eq;
else if ( strncmp(text, "ne", 2) == 0 )  op = thresh_ne;

else {

   mlog << Error
        << "do_fortran_thresh(const char *) -> can't parse threshold text \""
        << text << "\"\n\n";

   exit ( 1 );

}

Number n;
const double value = atof(p);

n.is_int = 0;

n.d = value;

set_number_string(p);

return ( do_simple_thresh (op, n) );

}


////////////////////////////////////////////////////////////////////////


void set_number_string()

{

set_number_string(configtext);

return;

}


////////////////////////////////////////////////////////////////////////


void set_number_string(const char * text)

{

const int k = (int) (sizeof(number_string));

strncpy(number_string, text, k);

number_string[k - 1] = (char) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void do_print(const Number & n)

{

cout << print_prefix << n << '\n' << flush;

return;

}


////////////////////////////////////////////////////////////////////////


void do_print(const char * s, const Number & n)

{

cout << print_prefix << s << n << '\n' << flush;

return;

}


////////////////////////////////////////////////////////////////////////


void do_function_def()

{



   //
   //  done
   //

is_function_def = false;

return;

}


////////////////////////////////////////////////////////////////////////





