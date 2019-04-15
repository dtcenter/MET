

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
#include "threshold.h"
#include "is_number.h"
#include "concat_string.h"
#include "pwl.h"
#include "dictionary.h"
#include "icode.h"
#include "idstack.h"
#include "calculator.h"
#include "fix_float.h"

#include "scanner_stuff.h"
#include "threshold.h"


////////////////////////////////////////////////////////////////////////


   //
   //  declarations that have external linkage
   //


extern int            yylex();

extern void           yyerror(const char *);

extern "C" int        configwrap();




   //
   //  definitions that have external linkage
   //

char *         configtext;

FILE *         configin;


int               LineNumber            = 1;

int               Column                = 1;

const char *      bison_input_filename  = (const char *) 0;

DictionaryStack * dict_stack            = (DictionaryStack *) 0;

bool              is_lhs                = true;    //  used by the scanner

ThreshNode *      result                = 0;   //  for testing

bool              test_mode             = false;

char number_string [max_id_length + 1];

IdentifierArray  ida;

bool is_function_def = false;

Calculator hp;


////////////////////////////////////////////////////////////////////////


   //
   //  static definitions
   //

static PiecewiseLinear pwl;

static Dictionary DD;

static SingleThresh STH;

static const char default_print_prefix [] = "config";


static ICVStack         icvs;



static ConcatString function_name;

static const char apm = 'b';   //  assign_prefix mark
static const char fcm = 'f';   //  function def mark



////////////////////////////////////////////////////////////////////////


   //
   //  static declarations
   //

static void do_op(char op);

static Number do_integer_op(char op, const Number & a, const Number & b);

static void do_negate();
static void do_paren_exp();

static void do_builtin_call(int which);


static void do_assign_boolean   (const char * name, bool);
static void do_assign_exp       (const char * name);
static void do_assign_string    (const char * name, const char * text);
static void do_assign_threshold (const char * name);

static void do_assign_id      (const char * LHS, const char * RHS);

static void do_assign_dict    (const char * name);

static void do_assign_exp_array(const char * name);

static void do_dict();

static void do_string(const char *);

static void do_boolean(const bool &);

static void store_exp();

static void do_thresh(ThreshNode *);

static void do_na_thresh();

static void add_point();

static void do_pwl(const char * LHS);

static void do_number(const Number &);

static void do_local_var(int);


static ThreshNode * do_and_thresh    (ThreshNode *, ThreshNode *);
static ThreshNode * do_or_thresh     (ThreshNode *, ThreshNode *);
static ThreshNode * do_not_thresh    (ThreshNode *);
static ThreshNode * do_paren_thresh  (ThreshNode *);
static ThreshNode * do_simple_thresh (ThreshType, const Number &);

static ThreshNode * do_simple_perc_thresh (const ThreshType, const PC_info &);
static ThreshNode * do_compound_perc_thresh (const ThreshType, const PC_info &, const Number &);
static ThreshNode * do_fortran_thresh(const char *);


static void set_number_string();
static void set_number_string(const char *);

static void mark(int);

static void do_user_function_call(const DictionaryEntry *);

static void do_print(const char *);

static void do_user_function_def();


////////////////////////////////////////////////////////////////////////


%}


%union {

   char text[max_id_length + 1];

   Number nval;

   bool bval;

   int index;

   ThreshType cval;

   ThreshNode * node;

   const DictionaryEntry * entry;

   PC_info pc_info;

}


%token IDENTIFIER QUOTED_STRING INTEGER FLOAT BOOLEAN
%token COMPARISON NA_COMPARISON
%token LOGICAL_OP_NOT LOGICAL_OP_AND LOGICAL_OP_OR
%token FORTRAN_THRESHOLD
%token BUILTIN
%token LOCAL_VAR
%token SIMPLE_PERC_THRESH

%token USER_FUNCTION
%token PRINT


%type <text> IDENTIFIER QUOTED_STRING assign_prefix array_prefix FORTRAN_THRESHOLD

%type <nval> INTEGER FLOAT number

%type <index> BUILTIN
%type <index> LOCAL_VAR

%type <entry> USER_FUNCTION

%type <bval> BOOLEAN

%type <cval> COMPARISON NA_COMPARISON

%type <node> simple_thresh thresh_node

%type <pc_info> SIMPLE_PERC_THRESH


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


print_stmt : print_prefix expression                         ';' { do_print( 0); }
           | print_prefix QUOTED_STRING opt_comma expression ';' { do_print($2); }
           ;


print_prefix : PRINT   { is_lhs = false; }
             ;



assign_stmt : assign_prefix BOOLEAN            ';'      { do_assign_boolean   ($1, $2); }
            | assign_prefix expression         ';'      { do_assign_exp       ($1); }
            | assign_prefix IDENTIFIER         ';'      { do_assign_id        ($1, $2); }
            | assign_prefix piecewise_linear   ';'      { do_pwl              ($1); }

            | assign_prefix threshold          ';'      { do_assign_threshold ($1); }
            | assign_prefix QUOTED_STRING      ';'      { do_assign_string    ($1, $2); }
            | assign_prefix dictionary                  { do_assign_dict      ($1); }

            | array_prefix boolean_list    ']' ';'      { do_assign_dict($1); }
            | array_prefix expression_list ']' ';'      { do_assign_exp_array($1); }
            | array_prefix string_list     ']' ';'      { do_assign_dict($1); }
            | array_prefix threshold_list  ']' ';'      { do_assign_dict($1); }
            | array_prefix dictionary_list ']' ';'      { do_assign_dict($1); }
            | array_prefix                 ']' ';'      { do_assign_dict($1); }

            | function_prefix expression       ';'      { do_user_function_def(); }

            ;



id_list : IDENTIFIER             { ida.add($1); }
        | id_list ',' IDENTIFIER { ida.add($3); }
        ;


function_prefix : IDENTIFIER '(' id_list ')' '='    { is_lhs = false;  function_name = $1;  is_function_def = true; }
                ;


assign_prefix : IDENTIFIER '='     { is_lhs = false;  strcpy($$, $1); }
              ;


array_prefix : assign_prefix { mark(apm); } '['   { is_lhs = false;  strcpy($$, $1); }
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


simple_thresh : COMPARISON number                                  { $$ = do_simple_thresh($1, $2);     }
              | COMPARISON SIMPLE_PERC_THRESH                      { $$ = do_simple_perc_thresh($1, $2); }
              | COMPARISON SIMPLE_PERC_THRESH '(' number ')'       { $$ = do_compound_perc_thresh($1, $2, $4); }
              | FORTRAN_THRESHOLD                                  { $$ = do_fortran_thresh($1);        }
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


expression : number                                                { do_number($1); }
           | LOCAL_VAR                                             { do_local_var($1); }
           | expression '+' expression                             { do_op('+'); }
           | expression '-' expression                             { do_op('-'); }
           | expression '*' expression                             { do_op('*'); }
           | expression '/' expression                             { do_op('/'); }
           | expression '^' expression                             { do_op('^'); }
           | '-' expression  %prec UNARY_MINUS                     { do_negate(); }
           | '(' expression ')'                                    { do_paren_exp(); }
           | BUILTIN       '(' { mark(fcm); } expression_list ')'  { do_builtin_call($1);  }
           | USER_FUNCTION '(' { mark(fcm); } expression_list ')'  { do_user_function_call($1); }
           ;



expression_list : expression                     { store_exp(); }
                | expression_list ',' expression { store_exp(); }
                ;


piecewise_linear : '(' point_list ')'   { }
                 ;


point_list : point              { }
           | point_list point   { }
           ;


point : '(' expression ',' expression ')'   { add_point(); }



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

met_open(in, bison_input_filename);

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


void do_op(char op)

{

IcodeCell cell;
IcodeVector L, R;


R = icvs.pop();
L = icvs.pop();


switch ( op )  {


   case '+':  cell.type = op_add;       break;
   case '-':  cell.type = op_subtract;  break;


   case '*':  cell.type = op_multiply;  break;
   case '/':  cell.type = op_divide;    break;


   case '^':
      if ( (R.length() == 1) && (R[0].type == integer) && (R[0].i == 2) )   cell.type = op_square;
      else                                                                  cell.type = op_power;
      break;


   default:
      cerr << "\n\n  do_op() -> unrecognized op ... \"" << op << "\"\n\n";
      exit ( 1 );
      break;


}   //  switch

if ( cell.type != op_square )   L.add(R);

L.add(cell);

icvs.push(L);

   //
   //  done
   //

return;

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
         mlog << Error << "\ndo_integer_op() -> "
              << "division by zero!\n\n";
         exit ( 1 );
      }
      C = A / B;
      break;

   default:
      mlog << Error << "\ndo_integer_op() -> "
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


void do_negate()

{

IcodeVector v;
IcodeCell cell;

cell.type = op_negate;

v = icvs.pop();

v.add(cell);

icvs.push(v);

return;

}


////////////////////////////////////////////////////////////////////////


void do_paren_exp()

{

   //
   //  nothing to do here!
   //

return;

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


void do_assign_exp(const char * name)

{

DictionaryEntry entry;
IcodeVector v;
Number n;

v = icvs.pop();

hp.run(v);

n = hp.pop();

if ( n.is_int)  entry.set_int    (name, n.i);
else            entry.set_double (name, n.d);

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

   mlog << Error << "\ndo_assign_id() -> "
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

e.set_dict("", DD);

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

if ( icvs.top_is_mark(apm) )  icvs.toss();

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_exp_array(const char * name)

{

int j, count;
IcodeVector v;
Number n;
NumberStack ns;
DictionaryEntry e;



count = 0;

while ( 1 )  {

   v = icvs.pop();

   if ( v.is_mark() )  break;

   hp.run(v);

   n = hp.pop();

   ns.push(n);

   ++count;

}   //  while


for (j=0; j<count; ++j)  {

   e.clear();

   n = ns.pop();

   if ( n.is_int )  e.set_int    ("", n.i);
   else             e.set_double ("", n.d);

   dict_stack->store(e);

}   //  for j

dict_stack->set_top_is_array(true);

dict_stack->pop_dict(name);


DD.clear();

if ( icvs.top_is_mark(apm) )  icvs.toss();

return;

}


////////////////////////////////////////////////////////////////////////


void do_string(const char * text)

{

DictionaryEntry e;

e.set_string("", text);

dict_stack->store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void do_boolean(const bool & boolean)

{

DictionaryEntry e;

e.set_boolean("", boolean);

dict_stack->store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void store_exp()

{


// DictionaryEntry e;
// IcodeVector v;
// Number n;
//
// v = icvs.pop();
//
// hp.run(v);
//
// n = hp.pop();
//
// if ( n.is_int )  e.set_int    (0, n.i);
// else             e.set_double (0, n.d);
//
// dict_stack->store(e);


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

      e.set_threshold("", T);

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

e.set_threshold("", T);

dict_stack->store(e);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void add_point()

{

double x, y;
IcodeVector xv, yv;
Number n;

yv = icvs.pop();
xv = icvs.pop();

hp.run(xv);

n = hp.pop();

x = as_double(n);

hp.run(yv);

n = hp.pop();

y = as_double(n);

pwl.add_point(x, y);

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

  //  foo


     if ( strncmp(text, "le", 2) == 0 )  op = thresh_le;
else if ( strncmp(text, "lt", 2) == 0 )  op = thresh_lt;

else if ( strncmp(text, "gt", 2) == 0 )  op = thresh_gt;
else if ( strncmp(text, "ge", 2) == 0 )  op = thresh_ge;

else if ( strncmp(text, "eq", 2) == 0 )  op = thresh_eq;
else if ( strncmp(text, "ne", 2) == 0 )  op = thresh_ne;

else {

   mlog << Error << "do_fortran_thresh(const char *) -> "
        << "can't parse threshold text \""
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


void mark(int k)

{

IcodeVector v;
IcodeCell cell;


cell.type = cell_mark;

cell.i = k;

v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_number(const Number & n)

{

IcodeVector v;
IcodeCell cell;

if ( n.is_int )  {

   cell.set_integer(n.i);

} else {

   cell.set_double (n.d);

}

v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_local_var(int n)

{

IcodeVector v;
IcodeCell cell;

cell.set_local_var(n);

v.add(cell);

icvs.push(v);



return;

}


////////////////////////////////////////////////////////////////////////


void do_print(const char * s)

{

IcodeVector v;
Number n;


if ( bison_input_filename )  cout << bison_input_filename;
else                         cout << default_print_prefix;

cout << ": ";

if ( s )  cout << s;

v = icvs.pop();

hp.run(v);

n = hp.pop();

if ( n.is_int )  cout << (n.i) << "\n";
else             cout << (n.d) << "\n";



cout.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void do_builtin_call(int which)

{

int j;
IcodeVector v;
IcodeCell cell;
const BuiltinInfo & info = binfo[which];


if ( is_function_def )  {

   IcodeVector vv;

      //  pop the args (in reverse order) from the icodevector stack

   for (j=0; j<(info.n_args); ++j)  {

      vv = icvs.pop();

      v.add_front(vv);

   }

   if ( icvs.top_is_mark(fcm) )  icvs.toss();

      //

   cell.set_builtin(which);

   v.add(cell);

      //

   icvs.push(v);

   return;

}   //  if is function def

   ///////////////////////////////////////

Number n[max_builtin_args];
Number cur_result;

   //
   //  pop the args (in reverse order) from the icodevector stack
   //

for (j=0; j<(info.n_args); ++j)  {

   v = icvs.pop();

   if ( v.is_mark() )  {

      cerr << "\n\n  do_builtin_call(int) -> too few arguments to builtin function \""
           << info.name << "\"\n\n";

      exit ( 1 );

   }

   hp.run(v);

   n[info.n_args - 1 - j] = hp.pop();

}

   //
   //  next one should be a mark
   //

v = icvs.pop();

if ( ! (v.is_mark()) )  {

   cerr << "\n\n  do_builtin_call(int) -> too many arguments to builtin function \""
        << info.name << "\"\n\n";

   exit ( 1 );

}

   //
   //  call the function
   //

hp.do_builtin(which, n);

cur_result = hp.pop();

if ( cur_result.is_int )  cell.set_integer (cur_result.i);
else                      cell.set_double  (cur_result.d);

v.clear();

v.add(cell);

icvs.push(v);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_user_function_call(const DictionaryEntry * e)

{

int j;
IcodeVector v;
IcodeCell cell;
const int Nargs = e->n_args();


if ( is_function_def )  {

   IcodeVector vv;

      //  pop the args (in reverse order) from the icodevector stack

   for (j=0; j<Nargs; ++j)  {

      vv = icvs.pop();

      v.add_front(vv);

   }

   if ( icvs.top_is_mark(fcm) )  icvs.toss();

      //

   cell.set_user_function(e);

   v.add(cell);

      //

   icvs.push(v);

   return;

}   //  if is function def


   //////////////////////////////////////

Number n[max_user_function_args];
Number cur_result;

   //
   //  pop the args (in reverse order) from the icodevector stack
   //

for (j=0; j<Nargs; ++j)  {

   v = icvs.pop();

   if ( v.is_mark() )  {

      cerr << "\n\n  do_user_function_call(int) -> too few arguments to user function \""
           << (e->name()) << "\"\n\n";

      exit ( 1 );

   }

   hp.run(v);

   n[Nargs - 1 - j] = hp.pop();

}


if ( icvs.top_is_mark(fcm) )  icvs.toss();


   //
   //  call the function
   //

hp.run(*(e->icv()), n);

cur_result = hp.pop();

if ( cur_result.is_int )  cell.set_integer (cur_result.i);
else                      cell.set_double  (cur_result.d);

v.clear();

v.add(cell);

icvs.push(v);






   //
   //  done
   //

// if ( icvs.top_is_mark(fcm) )  icvs.toss();

return;

}


////////////////////////////////////////////////////////////////////////


void do_user_function_def()

{

DictionaryEntry e;

if ( ida.n_elements() > max_user_function_args )  {

   cerr << "\n\n  do_user_function_def() -> too many arguments to function \""
        << function_name << "\" definition\n\n";

   exit ( 1 );

}

e.set_user_function(function_name.c_str(), icvs.pop(), ida.n_elements());

dict_stack->store(e);

   //
   //  done
   //

is_function_def = false;

ida.clear();

function_name.erase();

return;

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_simple_perc_thresh (const ThreshType op, const PC_info & info)

{

Simple_Node * s = new Simple_Node;

s->op = op;

if ( (info.perc_index < 0) || (info.perc_index >= n_perc_thresh_infos) )  {

   mlog << Error
        << "\ndo_simple_perc_thresh() -> bad perc_index ... "
        << (info.perc_index) << "\n\n";

   exit ( 1 );

}

s->T     = bad_data_double;

s->PT    = info.value;

s->Ptype = perc_thresh_info[info.perc_index].type;

   //
   //  sanity check
   //

if ( s->PT < 0 || s->PT > 100 )  {

   mlog << Error << "\ndo_simple_perc_thresh() -> "
        << "the percentile (" << s->PT << ") must be between 0 and 100!\n\n";

   exit ( 1 );

}

if ( s->Ptype == perc_thresh_freq_bias && !is_eq(s->PT, 1.0) )  {

   mlog << Error << "\ndo_simple_perc_thresh() -> "
        << "unsupported frequency bias percentile threshold!\n\n";

   exit ( 1 );

}

   //
   //  update the strings
   //

if ( op >= 0 )  {

   ConcatString cs;
   cs << perc_thresh_info[info.perc_index].short_name;
   cs << info.value;
   fix_float(cs);

   s->s      << thresh_type_str[op] << cs;
   s->abbr_s << thresh_abbr_str[op] << cs;

}

   //
   //  done
   //

return ( s );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * do_compound_perc_thresh (const ThreshType op, const PC_info & info, const Number & num)

{

Simple_Node * s = new Simple_Node;

s->op = op;

if ( (info.perc_index < 0) || (info.perc_index >= n_perc_thresh_infos) )  {

   mlog << Error
        << "\ndo_compound_perc_thresh() -> bad perc_index ... "
        << (info.perc_index) << "\n\n";

   exit ( 1 );

}

if ( num.is_int )  s->T     = (double) (num.i);
else               s->T     = num.d;

s->PT    = info.value;

s->Ptype = perc_thresh_info[info.perc_index].type;

   //
   //  sanity check
   //

if ( s->PT < 0 || s->PT > 100 )  {

   mlog << Error << "\ndo_compound_perc_thresh() -> "
        << "the percentile (" << s->PT << ") must be between 0 and 100!\n\n";

   exit ( 1 );

}

if ( s->Ptype == perc_thresh_freq_bias && !is_eq(s->PT, 1.0) )  {

   mlog << Error << "\ndo_compound_perc_thresh() -> "
        << "unsupported frequency bias percentile threshold!\n\n";

   exit ( 1 );

}

   //
   //  update the strings
   //

if ( op >= 0 )  {

   ConcatString cs;
   cs << perc_thresh_info[info.perc_index].short_name;
   cs << info.value;
   fix_float(cs);
   cs << "(" << number_string << ")";

   s->s      << thresh_type_str[op] << cs;
   s->abbr_s << thresh_abbr_str[op] << cs;

}

   //
   //  done
   //

return ( s );

}


////////////////////////////////////////////////////////////////////////

