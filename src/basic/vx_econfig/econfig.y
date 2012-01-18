

%{


////////////////////////////////////////////////////////////////////////


#define YYDEBUG 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "is_number.h"

#include "builtin.h"
#include "idstack.h"
#include "icode.h"
#include "symtab.h"
#include "pwl.h"
#include "machine.h"
#include "celltype_to_string.h"


////////////////////////////////////////////////////////////////////////


   //
   //  declarations that have external linkage
   //


extern int            yylex();

extern void           yyerror(const char *);

extern "C" int        econfigwrap();




extern char *         econfigtext;

extern Machine *      bison_machine;

extern const char *   bison_input_filename;


   //
   //  definitions that have external linkage
   //


int econfig_LineNumber                      = 1;

int econfig_column                          = 1;

Machine *     bison_machine         = (Machine *) 0;

const char *  bison_input_filename  = (const char *) 0;


////////////////////////////////////////////////////////////////////////


   //
   //  static objects
   //


static  ICVStack         icvs;

static  IdentifierArray  ida;

static  PiecewiseLinear  pee_ell;


   //
   //  static functions
   //


static void  do_paren_exp          ();

static void  do_negate             ();

static void  do_op                 (char op);

static void  add_point             ();




static void  do_variable_def       (const char *);

static void  do_pwl_def            (const char *);

static void  do_function_def       (const char *);



static void  do_function_call      (const char *);


static void  add_id                (const char *);


static void  do_number             (const Number &);
static void  do_boolean            (int);
static void  do_id                 (const char *);

static void  mark                  (int);

static void  do_array_def_1        (const char *);
static void  do_array_def_2        (const char *);
static void  do_array_exp          (const char *);

static void  do_element_assign     (const char *);

static void  do_print              ();

static void  string_to_expr        (const char *);


////////////////////////////////////////////////////////////////////////


%}



%union {

   char text[1024];

   Number num;

   int ival;

}




%token BOOLEAN ID INTEGER FLOAT QUOTED_STRING
%token PRINT


%type <text> ID QUOTED_STRING
%type <num>  INTEGER FLOAT
%type <ival> BOOLEAN

%type <num>  number

%left '+' '-'
%left '*' '/'
%left '^'
%nonassoc UMINUS


%%


statement_list : statement
               | statement_list statement
               ;


statement : assignment           { }
          | print_statement      { }
          ;



print_statement : PRINT expression ';'     { do_print(); }


assignment : ID                        '=' expression ';'                           { do_variable_def   ($1); }
           | ID                        '=' pwl ';'                                  { do_pwl_def        ($1); }
           | ID '(' id_list ')'        '=' expression ';'                           { do_function_def   ($1); }
           | ID '[' ']' { mark('b'); } '=' '[' expression_list opt_comma ']' ';'    { do_array_def_2    ($1); }
           | ID bracket_exp_list       '=' '[' expression_list ']' ';'              { do_array_def_1    ($1); }
           | ID bracket_exp_list       '=' expression ';'                           { do_element_assign ($1); }
           ;


opt_comma: ','
         |   /*  empty  */
         ;


id_list : ID              { add_id($1); }
        | id_list ',' ID  { add_id($3); }
        ;


expression : number                                     { do_number($1); }
           | BOOLEAN                                    { do_boolean($1); }
           | ID                                         { do_id($1); }
           | expression '+' expression                  { do_op('+'); }
           | expression '-' expression                  { do_op('-'); }
           | expression '*' expression                  { do_op('*'); }
           | expression '/' expression                  { do_op('/'); }
           | expression '^' expression                  { do_op('^'); }
           | '-' expression  %prec UMINUS               { do_negate(); }
           | '(' expression ')'                         { do_paren_exp(); }
           | ID '(' { mark(0); } expression_list ')'    { do_function_call($1); }
           | ID bracket_exp_list                        { do_array_exp($1); }
           | QUOTED_STRING                              { string_to_expr($1); }
           ;


expression_list : expression                         { }
                | expression_list ',' expression     { }
                |     /*  empty */
                ;


bracket_exp : '[' expression ']'   { mark('b'); }
            ;


bracket_exp_list : bracket_exp                    { }
                 | bracket_exp_list bracket_exp   { }
                 ;


number : INTEGER  { $$ = $1; }
       | FLOAT    { $$ = $1; }
       ;



pwl : '{' { pee_ell.clear(); }  point_list '}'   { }
    ;


point_list : point              { }
           | point_list point   { }
           ;


point : '(' expression ',' expression ')'   { add_point(); }
      ;



%%


////////////////////////////////////////////////////////////////////////


void do_paren_exp()

{

   //
   //  nothing to do here!
   //

return;

}


////////////////////////////////////////////////////////////////////////


static void do_negate()

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
      if ( (R.length() == 1) && (R[0].type == integer) && (R[0].val == 2) )   cell.type = op_square;
      else                                                                    cell.type = op_power;
      break;


   default:
      cerr << "\n\n  do_op() -> unrecognized op ... \"" << op << "\"\n\n";
      exit ( 1 );
      break; 


}   //  switch



if ( cell.type != op_square )   L.add(R);

L.add(cell);

icvs.push(L);

return;

}


////////////////////////////////////////////////////////////////////////


void add_point()

{


double x, y;
IcodeCell cell;
IcodeVector X, Y;


Y = icvs.pop();
X = icvs.pop();

   //
   //  get x
   //

bison_machine->run( X );

if ( bison_machine->depth() > 1 )  {

   cerr << "\n\n  add_point() -> bad \"x\" expression!\n\n";

   exit ( 1 );

}

cell = bison_machine->pop();

x = cell.as_double();


   //
   //  get y
   //

bison_machine->run( Y );

if ( bison_machine->depth() > 1 )  {

   cerr << "\n\n  add_point() -> bad \"y\" expression!\n\n";

   exit ( 1 );

}

cell = bison_machine->pop();

y = cell.as_double();

   //
   //  add point
   //

pee_ell.add_point(x, y);


// cout << "   added point (" << x << ", " << y << ")\n" << flush;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_pwl_def(const char * Name)

{

SymbolTableEntry e;


pee_ell.set_name(Name);


e.set_pwl(Name, pee_ell);


bison_machine->store(e);


pee_ell.clear();


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_function_call (const char * function_name)

{

int index;
int count;
IcodeVector v;
IcodeCell cell;
ICVStack s;


count = 0;

while ( 1 )  {

   v = icvs.pop();

   if ( v.is_mark() )  break;

   s.push(v);

   ++count;

}

v.clear();

while ( s.depth() > 0 )  {

   v.add(s.pop());

}


if ( is_builtin(function_name, index) )  { 

   cell.set_builtin(index);

} else {

   cell.set_identifier(function_name);

}



v.add(cell);

icvs.push(v);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_variable_def(const char * Name)

{

SymbolTableEntry e;
IcodeVector v;

v = icvs.pop();

e.set_variable(Name, v);

bison_machine->store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void do_function_def(const char * func_name)

{

int index;

if ( is_builtin(func_name, index) )  {

   cerr << "\n\n  do_function_def() -> attempt to redefine built-in function \"" << func_name << "\"\n\n";

   exit ( 1 );

}

SymbolTableEntry e;
IcodeVector v;

v = icvs.pop();


e.set_function(func_name, ida, v);

bison_machine->store(e);

ida.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void add_id(const char * text)

{

Identifier id;

id.set(text);

ida.add(id);


return;

}


////////////////////////////////////////////////////////////////////////


void do_number(const Number & number)

{

IcodeVector v;
IcodeCell cell;

if ( number.is_int )  {

   cell.set_integer(number.i);

} else {

   cell.set_double (number.d);

}

v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_boolean(int i)

{

IcodeVector v;
IcodeCell cell;

if ( i == 0 )  cell.set_boolean(false);
else           cell.set_boolean(true);

v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_id(const char * text)

{

IcodeVector v;
IcodeCell cell;

cell.set_identifier(text);

v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void mark(int k)

{

IcodeVector v;
IcodeCell cell;


cell.type = cell_mark;

cell.val = k;

v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_array_def_1(const char * name)

{

int j, d, k;
ICVStack s;
int sizes[max_array_dim];
IcodeVector v;
IcodeVector zero;
IcodeCell cell;
ArrayInfo * ai = (ArrayInfo *) 0;
SymbolTableEntry e;




memset(sizes, 0, sizeof(sizes));

   //
   //  get all the array elements
   //

while ( 1 )  {

   IcodeVector & vp = *(icvs.peek());

   if ( (vp.is_mark()) && (vp[0].val == 'b') ) break;

   v = icvs.pop();

   s.push(v);

}

// icvs.write_to_screen(0);

   //
   //  get the array sizes and dimension
   //

d = 0;

while ( 1 )  {

   if ( d >= max_array_dim )  {

      cerr << "\n\n  do_array_def_1(const char *) -> array dimension limit exceeded\n\n";

      exit ( 1 );

   }

   if ( icvs.depth() == 0 )  break;

   v = icvs.pop();

   if ( !(v.is_mark()) || !(v[0].val == 'b') ) { icvs.push(v);  break; }

   v = icvs.pop();

   bison_machine->run(v);
   
   cell = bison_machine->pop();

   if ( !(cell.is_numeric()) )  {

      cerr << "\n\n  do_array_def_1(const char *) -> non-numeric array size\n\n";

      exit ( 1 );

   }

   if ( cell.type == integer )  k = cell.val;
   if ( cell.type == floating_point )  k = my_nint(cell.d);

   if ( k <= 0 )  {

      cerr << "\n\n  do_array_def_1(const char *) -> bad array size ... " << k << "\n\n";

      exit ( 1 );

   }

   sizes[max_array_dim - 1 - d] = k;

   ++d;

}


// cout << "d = " << d << "\n";


for (j=0; j<d; ++j)  {

   sizes[j] = sizes[max_array_dim - d + j];

}

ai = new ArrayInfo;

if ( !ai )  {

   cerr << "\n\n  do_array_def_1(const char *) -> memory allocation error\n\n";

   exit ( 1 );

}

ai->set(sizes, d);

e.type = ste_array;

e.ai = ai;

e.set_name(name);

   //
   //  fill the entries of the array
   //

cell.set_integer(0);

zero.add(cell);

for (j=0; j<(ai->n_alloc()); ++j)  {

   if ( s.depth() > 0 )  {

      v = s.pop();

      ai->put(j, v);

   } else {

      ai->put(j, zero);

   }

}


   //
   //  store it
   //

bison_machine->store(e);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_array_def_2(const char * name)

{

int j, d;
ICVStack s;
int sizes[max_array_dim];
IcodeVector v;
IcodeVector zero;
IcodeCell cell;
ArrayInfo * ai = (ArrayInfo *) 0;
SymbolTableEntry e;




   //
   //  get all the array elements
   //

while ( 1 )  {

   IcodeVector & vp = *(icvs.peek());

   if ( (vp.is_mark()) && (vp[0].val == 'b') ) break;

   v = icvs.pop();

   s.push(v);

}


v = icvs.pop();   //  pop off the mark

d = 1;

sizes[0] = s.depth();

ai = new ArrayInfo;

if ( !ai )  {

   cerr << "\n\n  do_array_def_1(const char *) -> memory allocation error\n\n";

   exit ( 1 );

}

ai->set(sizes, d);

e.type = ste_array;

e.ai = ai;

e.set_name(name);

   //
   //  fill the entries of the array
   //

cell.set_integer(0);

zero.add(cell);

for (j=0; j<(ai->n_alloc()); ++j)  {

   if ( s.depth() > 0 )  {

      v = s.pop();

      ai->put(j, v);

   } else {

      ai->put(j, zero);

   }

}

   //
   //  store it
   //

bison_machine->store(e);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_array_exp(const char * name)

{

SymbolTableEntry * e = (SymbolTableEntry *) 0;


e = bison_machine->find(name);

if ( !e )  {

   cerr << "\n\n  void do_array_exp(const char *) -> unidentified name used as array\n\n";

   exit ( 1 );

}

if ( e->type != ste_array )  {

   cerr << "\n\n  void do_array_exp(const char *) -> non-array name \"" << name << "\" used as array\n\n";

   exit ( 1 );

}

int j;
IcodeVector v, vv;
IcodeCell cell;
ICVStack s;



for (j=0; j<(e->ai->dim()); ++j)  {

   v = icvs.pop();   //  should be mark 'b' ... just toss it

   v = icvs.pop();

   s.push(v);

}

v.clear();

while ( s.depth() > 0 )  {

   vv = s.pop();

   v.add(vv);

}


cell.set_identifier(name);


v.add(cell);

icvs.push(v);


return;

}


////////////////////////////////////////////////////////////////////////


void do_element_assign(const char * name)

{

int j, k, dim;
SymbolTableEntry * e = (SymbolTableEntry *) 0;
ICVStack s;
IcodeVector v, RHS;
IcodeCell cell;
int indices[max_array_dim];


e = bison_machine->find(name);

if ( !e )  {

   cerr << "\n\n  do_element_assign(const char *) -> can't find symbol table entry for name \"" << name << "\"\n\n";

   exit ( 1 );

}

if ( e->type != ste_array )  {

   cerr << "\n\n  do_element_assign(const char *) -> name \"" << name << "\" is not an array!\n\n";

   exit ( 1 );

}

dim = e->ai->dim();

   //
   //  pop the right-hand-side expression off the stack
   //

RHS = icvs.pop();

   //
   //  pop the array indices off the stack
   //

for (j=0; j<dim; ++j)  {

   v = icvs.pop();

   if ( (v.is_mark()) && (v[0].val == 'b') )  {

      v = icvs.pop();

      s.push(v);

   } else {

      cerr << "\n\n  do_element_assign(const char *) -> expected mark not found\n\n";

      exit ( 1 );

   }

}

   //
   //  evaluate the indices
   //

for (j=0; j<dim; ++j)  {

   v = s.pop();

   bison_machine->run(v);

   cell = bison_machine->pop();

   if ( !(cell.is_numeric()) )  {

      cerr << "\n\n  do_element_assign(const char *) -> non-numeric value for index " << j << "\n\n";

      exit ( 1 );

   }

   if ( cell.type == integer )  k = cell.val;
   else                         k = my_nint(cell.d);

   indices[j] = k;

}

   //
   //  range-check the indices
   //

for (j=0; j<dim; ++j)  {

   if ( (indices[j] < 0) || (indices[j] >= e->ai->size(j)) )  {

      cerr << "\n\n  do_element_assign(const char *) -> range-check error on value for index " << j << "\n\n";

      exit ( 1 );


   }

}

   //
   //  store
   //

e->ai->put(indices, RHS);

   //
   //  done
   //

// cerr << "\n\n  void do_element_assign() -> not yet implemented!\n\n";
// 
// exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void do_print()

{

IcodeVector v;
IcodeCell cell;


v = icvs.pop();


bison_machine->run(v);

cell = bison_machine->pop();

switch ( cell.type )  {

   case integer:
      cout << (cell.val) << "\n";
      break;

   case boolean:
      cout << ( cell.val ? "true" : "false" ) << "\n";
      break;

   case floating_point:
      cout << (cell.d) << "\n";
      break;

   case identifier:
      cout << (cell.name) << "\n";
      break;

   case character_string:
      cout << "\"" << (cell.text) << "\"\n";
      break;


   default:
      cerr << "\n\n  do_print() -> unrecognized cell type: "
           << celltype_to_string(cell.type) << "\n\n";
      exit ( 1 );
      break;

}   //  switch




   //
   // done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void string_to_expr(const char * text)

{

IcodeVector v;
IcodeCell cell;

cell.set_string(text);

v.add(cell);

icvs.push(v);



return;

}


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


c = (int) (econfig_column - strlen(econfigtext));

cout << "\n\n"
     << "   config() -> syntax error in file \"" << bison_input_filename << "\"\n\n"
     << "      line   = " << econfig_LineNumber << "\n\n"
     << "      econfig_column = " << c << "\n\n"
     << "      text   = \"" << econfigtext << "\"\n\n";

in.open(bison_input_filename);

for (j=1; j<econfig_LineNumber; ++j)  {   //  j starts at one here, not zero

   in.getline(line, sizeof(line));

}

in.getline(line, sizeof(line));

in.close();




cout << "\n\n"
     << line
     << "\n";

line_len = strlen(line);

text_len = strlen(econfigtext);

j1 = c;
j2 = c + text_len - 1;


for (j=1; j<=line_len; ++j)  {   //  j starts a one here, not zero

   if ( (j >= j1) && (j <= j2) )  cout.put('^');
   else                           cout.put('_');

}





cout << "\n\n";

cout.flush();

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


int econfigwrap()

{

return ( 1 );

}


////////////////////////////////////////////////////////////////////////










