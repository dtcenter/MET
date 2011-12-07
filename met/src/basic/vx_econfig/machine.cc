

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "celltype_to_string.h"
#include "stetype_to_string.h"

#include "builtin.h"
#include "indent.h"
#include "pwl.h"
#include "machine.h"


////////////////////////////////////////////////////////////////////////


extern Machine * bison_machine;

extern int econfigparse();

extern FILE *econfigin;

extern int econfigdebug;

extern const char * bison_input_filename;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Machine
   //


////////////////////////////////////////////////////////////////////////


Machine::Machine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Machine::~Machine()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Machine::Machine(const Machine & m)

{

init_from_scratch();

assign(m);

}


////////////////////////////////////////////////////////////////////////


Machine & Machine::operator=(const Machine & m)

{

if ( this == &m )  return ( * this );

assign(m);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Machine::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::clear()

{

sts.clear();

cstack.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::assign(const Machine & m)

{

clear();

sts = m.sts;

cstack = m.cstack;


return;

}


////////////////////////////////////////////////////////////////////////


IcodeCell Machine::pop()

{

if ( depth() == 0 )  {

   cerr << "\n\n  Machine::pop() -> stack empty!\n\n";

   exit ( 1 );

}

IcodeCell cell = cstack.pop();

return ( cell );

}


////////////////////////////////////////////////////////////////////////


void Machine::push(const IcodeCell & cell)

{

cstack.push(cell);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::run(const IcodeVector & v)

{

int pos;
IcodeCell cell;
SymbolTableEntry * e = (SymbolTableEntry *) 0;
char junk[128];


pos = 0;



while ( pos < v.length() )  {

   cell = v[pos];

   switch ( cell.type )  {


      case integer:
         cstack.push(cell);
         break;

      case boolean:
         cstack.push(cell);
         break;

      case floating_point:
         cstack.push(cell);
         break;


      case op_add:
         do_add();
         break;

      case op_subtract:
         do_subtract();
         break;

      case op_multiply:
         do_multiply();
         break;

      case op_divide:
         do_divide();
         break;

      case op_power:
         do_power();
         break;

      case op_square:
         do_square();
         break;


      case op_negate:
         do_negate();
         break;

      case identifier:
         e = sts.find(cell.name);
         if ( e )   {
            run ( *e );
         } else {
            cerr << "\n\n  Machine::run(const IcodeVector &) -> undefined identifier ... \""
                 << (cell.name) << "\"\n\n";
            exit ( 1 );
         }
         break;

       case builtin_func:
         do_builtin(cell.val);
         break;

      case character_string:
         cstack.push(cell);
         break;


      case op_store:
      case op_recall:
      default:
         celltype_to_string(cell.type, junk);
         cerr << "\n\n  Machine::run(const IcodeVector &) -> bad icode cell type ... \"" << junk << "\"\n\n";
         exit ( 1 );
         break;

   }   //   switch

   ++pos;

}   //  while









   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::run(const SymbolTableEntry & e)

{

char junk[128];
IcodeCell cell;



switch ( e.type )  {

   case ste_integer:
      cell.set_integer(e.i);
      cstack.push(cell);
      break;

   case ste_double:
      cell.set_double(e.d);
      cstack.push(cell);
      break;

   case ste_variable:
      run( *(e.v) );
      break;

   case ste_pwl:
      do_pwl( *(e.pl) );
      break;

   case ste_function:
      do_function_call(e);
      break;

   case ste_array:
      do_array(e);
      break;

   default:
      stetype_to_string(e.type, junk);
      cerr << "\n\n  void Machine::run(const SymbolTableEntry &)  -> bad type ... \""
           << junk << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch



return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_add()

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();

if ( !operand1.is_numeric() || !operand2.is_numeric() )  {

   cerr << "\n\n  void Machine::do_add() -> can't add non-numeric types!\n\n";

   exit ( 1 );

}

   //
   //  are both integers?
   //

if ( (operand1.type == integer) && (operand2.type == integer) )  {

   int i, j;

   i = operand1.val;
   j = operand2.val;

   result.set_integer( i + j );

   cstack.push(result);

   return;

}

   //
   //  at least one is a double, so make them both doubles
   //

double x, y;

if ( operand1.type == integer )   x = (double) (operand1.val);
else                              x = operand1.d;

if ( operand2.type == integer )   y = (double) (operand2.val);
else                              y = operand2.d;


result.set_double(x + y);

cstack.push(result);



   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_subtract()

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();

if ( !operand1.is_numeric() || !operand2.is_numeric() )  {

   cerr << "\n\n  void Machine::do_add() -> can't subtract non-numeric types!\n\n";

   exit ( 1 );

}

   //
   //  are both integers?
   //

if ( (operand1.type == integer) && (operand2.type == integer) )  {

   int i, j;

   i = operand1.val;
   j = operand2.val;

   result.set_integer( i - j );

   cstack.push(result);

   return;

}

   //
   //  at least one is a double, so make them both doubles
   //

double x, y;

if ( operand1.type == integer )   x = (double) (operand1.val);
else                              x = operand1.d;

if ( operand2.type == integer )   y = (double) (operand2.val);
else                              y = operand2.d;


result.set_double(x - y);

cstack.push(result);



   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_multiply()

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();

if ( !operand1.is_numeric() || !operand2.is_numeric() )  {

   cerr << "\n\n  void Machine::do_add() -> can't multiply non-numeric types!\n\n";

   exit ( 1 );

}

   //
   //  are both integers?
   //

if ( (operand1.type == integer) && (operand2.type == integer) )  {

   int i, j;

   i = operand1.val;
   j = operand2.val;

   result.set_integer( i*j );

   cstack.push(result);

   return;

}

   //
   //  at least one is a double, so make them both doubles
   //

double x, y;

if ( operand1.type == integer )   x = (double) (operand1.val);
else                              x = operand1.d;

if ( operand2.type == integer )   y = (double) (operand2.val);
else                              y = operand2.d;


result.set_double(x*y);

cstack.push(result);






   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_divide()

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();

if ( !operand1.is_numeric() || !operand2.is_numeric() )  {

   cerr << "\n\n  void Machine::do_add() -> can't divide non-numeric types!\n\n";

   exit ( 1 );

}

   //
   //  are both integers?
   //

if ( (operand1.type == integer) && (operand2.type == integer) )  {

   int i, j;

   i = operand1.val;
   j = operand2.val;

   if ( j == 0 )  {

      cerr << "\n\n  void Machine::do_divide() -> integer division by zero!\n\n";

      exit ( 1 );

   }

   result.set_integer( i/j );

   cstack.push(result);

   return;

}

   //
   //  at least one is a double, so make them both doubles
   //

double x, y;

if ( operand1.type == integer )   x = (double) (operand1.val);
else                              x = operand1.d;

if ( operand2.type == integer )   y = (double) (operand2.val);
else                              y = operand2.d;

if ( y == 0.0 )  {

   cerr << "\n\n  void Machine::do_divide() -> floating-point division by zero!\n\n";

   exit ( 1 );

}

result.set_double(x/y);

cstack.push(result);






   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_power()

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();

if ( !operand1.is_numeric() || !operand2.is_numeric() )  {

   cerr << "\n\n  void Machine::do_power() -> can't exponentiate non-numeric types!\n\n";

   exit ( 1 );

}

double x, y, z;

x = operand1.as_double();
y = operand2.as_double();

z = pow(x, y);

result.set_double(z);

cstack.push(result);

   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_square()

{

IcodeCell operand;
IcodeCell result;


operand = cstack.pop();

if ( !operand.is_numeric() )  {

   cerr << "\n\n  void Machine::do_power() -> can't square non-numeric types!\n\n";

   exit ( 1 );

}

if ( operand.type == integer )  {

   int i = operand.val;

   result.set_integer( i*i );

   cstack.push(result);

   return;

}




double x;


x = operand.d;


result.set_double( x*x );

cstack.push(result);

   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_negate()

{

IcodeCell operand, result;


operand = cstack.pop();

if ( !operand.is_numeric() )  {

   cerr << "\n\n  void Machine::do_add() -> can't negate non-numeric type!\n\n";

   exit ( 1 );

}

   //
   //  is it an integer?
   //

if ( operand.type == integer )  {

   int j;

   j = operand.val;

   result.set_integer( -j );

   cstack.push(result);

}

   //
   //  nope, is's floating-point
   //

double x;

x = operand.d;

result.set_double( -x );

cstack.push(result);






   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::st_dump(ostream & out, int indent_depth) const

{


sts.dump(out, indent_depth);


return;

}


////////////////////////////////////////////////////////////////////////


void Machine::store(const SymbolTableEntry & e)

{


sts.store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void Machine::dump_cell(ostream & out, int n, int indent) const

{

cstack.dump_cell(out, n, indent);


return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_pwl(const PiecewiseLinear & p)

{

IcodeCell operand, result;


operand = cstack.pop();

if ( !operand.is_numeric() )  {

   cerr << "\n\n  Machine::do_pwl(const PL *) -> can't apply pwl to non-numeric argument!\n\n";

   exit ( 1 );

}


double x, y;


x = operand.as_double();


y = p(x);


result.set_double(y);

cstack.push(result);



   //
   //  done
   //


return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_function_call(const SymbolTableEntry & e)

{

int j, n;
IcodeCell cell;
char junk[128];
SymbolTableEntry ee;
SymbolTableEntry * u = (SymbolTableEntry *) 0;
IdentifierArray & a = *(e.local_vars);



n = a.n_elements();

   //
   //  push the temporary scope onto the symbol table stack
   //

sts.push(e.st);

   //
   //  store the values, starting from the bottom of the stack, 
   //     into the function's local variables
   //

for (j=(n - 1); j>=0; --j)  {   //  reverse order here

   start:

   cell = cstack.pop();

   switch ( cell.type )  {

      case integer:
         sts.store_as_int(a[j].name, cell.val);
         break;

      case floating_point:
         sts.store_as_double(a[j].name, cell.d);
         break;

      case identifier:
         u = sts.find(cell.name);
         if ( !u )  {
            cerr << "\n\n  Machine::do_function_call(const SymbolTableEntry &) -> undefined name on stack ... \""
                 << (cell.name) << "\"\n\n";
            exit ( 1 );
         }
         eval(cell.name);
         goto start;
         break;

      default:
         celltype_to_string(cell.type, junk);
         cerr << "\n\n  Machine::do_function_call(const SymbolTableEntry &) -> bad cell type on stack ... \""
              << junk << "\"\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  for j

   //
   //  run the function program
   //

run( *(e.v) );

   //
   //  remove the temporary scope
   //

sts.pop();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_array(const SymbolTableEntry & e)

{

int j, dim;
int indices[max_array_dim];
const IcodeVector * v = (const IcodeVector *) 0;
IcodeCell cell;
SymbolTableEntry * u = (SymbolTableEntry *) 0;
char junk[128];


memset(indices, 0, sizeof(indices));

   //
   //  get the values, starting from the bottom of the stack,
   //     of the indices
   //

dim = e.ai->dim();

for (j=(dim - 1); j>=0; --j)  {   //  reverse order here

   start:

   cell = cstack.pop();

   switch ( cell.type )  {

      case integer:
         indices[j] = cell.val;
         break;

      case floating_point:
         indices[j] = my_nint(cell.d);
         break;

      case identifier:
         u = sts.find(cell.name);
         if ( !u )  {
            cerr << "\n\n  Machine::do_array(const SymbolTableEntry &) -> undefined name on stack ... \""
                 << (cell.name) << "\"\n\n";
            exit ( 1 );
         }
         eval(cell.name);
         goto start;
         break;

      default:
         celltype_to_string(cell.type, junk);
         cerr << "\n\n  Machine::do_array(const SymbolTableEntry &) -> bad cell type on stack ... \""
              << junk << "\"\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  for j


   //
   //  get the formula for the array entry
   //

v = e.ai->get(indices);

   //
   //  run the formula
   //

run(*v);

   //
   //
   //

return;

}


////////////////////////////////////////////////////////////////////////


double Machine::func(const char * Name, double x)

{

int index;
double y;
IcodeCell operand, result;
SymbolTableEntry * e = (SymbolTableEntry *) 0;


operand.set_double(x);

cstack.push(operand);

   //
   //  is it a builtin function?
   //

if ( is_builtin(Name, index) )  {

   if ( binfo[index].n_vars != 1 )  {

      cerr << "\n\n  Machine::func(const char * Name, double) -> bad signature\n\n";

      exit ( 1 );

   }

   do_builtin(index);

   result = cstack.pop();

} else {

   e = sts.find(Name);

   if ( !e )  {

      cerr << "\n\n  Machine::func(const char * Name, double) -> function \"" << Name << "\" not defined!\n\n";

      exit ( 1 );

   }

   switch ( e->type )  {

      case ste_function:
         do_function_call(*e);
         result = cstack.pop();
         break;

      case ste_pwl:
         do_pwl( *(e->pl) );
         result = cstack.pop();
         break;

      default:
         cerr << "\n\n  Machine::func(const char * Name, double) -> function \"" << Name << "\" not a function!\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  else



if ( !result.is_numeric() )  {

   cerr << "\n\n  Machine::func(const char * Name, double) -> non-numeric result!\n\n";

   exit ( 1 );

}

y = result.as_double();


   //
   //  done
   //

return ( y );

}


////////////////////////////////////////////////////////////////////////


double Machine::func(const char * Name, double x, double y)

{

int index;
double z;
IcodeCell operand, result;
SymbolTableEntry * e = (SymbolTableEntry *) 0;


operand.set_double(x);
cstack.push(operand);

operand.set_double(y);
cstack.push(operand);

   //
   //  is it a builtin function?
   //

if ( is_builtin(Name, index) )  {

   if ( binfo[index].n_vars != 2 )  {

      cerr << "\n\n  Machine::func(const char * Name, double) -> bad signature\n\n";

      exit ( 1 );

   }

   do_builtin(index);

   result = cstack.pop();

} else {

   e = sts.find(Name);

   if ( !e )  {

      cerr << "\n\n  Machine::func(const char * Name, double) -> function \"" << Name << "\" not defined!\n\n";

      exit ( 1 );

   }

   switch ( e->type )  {

      case ste_function:
         do_function_call(*e);
         result = cstack.pop();
         break;

      case ste_pwl:
         do_pwl( *(e->pl) );
         result = cstack.pop();
         break;

      default:
         cerr << "\n\n  Machine::func(const char * Name, double) -> function \"" << Name << "\" not a function!\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  else



if ( !result.is_numeric() )  {

   cerr << "\n\n  Machine::func(const char * Name, double) -> non-numeric result!\n\n";

   exit ( 1 );

}

z = result.as_double();


   //
   //  done
   //

return ( z );

}


////////////////////////////////////////////////////////////////////////


double Machine::func(const char * Name, double * x, int n)

{

int j;
int index;
double z;
IcodeCell operand, result;
SymbolTableEntry * e = (SymbolTableEntry *) 0;


for (j=0; j<n; ++j)  {

   operand.set_double(x[j]);
   cstack.push(operand);

}

   //
   //  is it a builtin function?
   //

if ( is_builtin(Name, index) )  {

   if ( binfo[index].n_vars != n )  {

      cerr << "\n\n  Machine::func(const char * Name, double) -> bad signature\n\n";

      exit ( 1 );

   }

   do_builtin(index);

   result = cstack.pop();

} else {

   e = sts.find(Name);

   if ( !e )  {

      cerr << "\n\n  Machine::func(const char * Name, double) -> function \"" << Name << "\" not defined!\n\n";

      exit ( 1 );

   }

   switch ( e->type )  {

      case ste_function:
         do_function_call(*e);
         result = cstack.pop();
         break;

      case ste_pwl:
         do_pwl( *(e->pl) );
         result = cstack.pop();
         break;

      default:
         cerr << "\n\n  Machine::func(const char * Name, double) -> function \"" << Name << "\" not a function!\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  else



if ( !result.is_numeric() )  {

   cerr << "\n\n  Machine::func(const char * Name, double) -> non-numeric result!\n\n";

   exit ( 1 );

}

z = result.as_double();


   //
   //  done
   //

return ( z );

}


////////////////////////////////////////////////////////////////////////


void Machine::eval(const char * Name)

{

IcodeCell cell;
SymbolTableEntry * u = (SymbolTableEntry *) 0;


u = sts.find(Name);

if ( !u )  {

   cerr << "\n\n  Machine::eval(const char *) -> undefined identifier ... \""
        << Name << "\"\n\n";

   exit ( 1 );

}


run( *u );

u = (SymbolTableEntry *) 0;


return;

}


////////////////////////////////////////////////////////////////////////


void Machine::algebraic_dump(ostream & out) const

{

sts.algebraic_dump(out);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::read(const char * filename)

{

bison_machine = this;

bison_input_filename = filename;

// econfigdebug = 0;

if ( (econfigin = fopen(filename, "r")) == NULL )  {

   cerr << "\n\n  Machine::read() -> unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

}


int parse_status;

parse_status = econfigparse();

if ( parse_status != 0 )  {

   cerr << "\n\n  Machine::read() -> parse status is nonzero! ... = " << parse_status << "\n\n";

   exit ( 1 );

}


   //
   //  done
   //

bison_machine = (Machine *) 0;

bison_input_filename = (const char *) 0;

fclose(econfigin);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_builtin(int k)

{

const BuiltinInfo & info = binfo[k];

   //
   //  nint and sign are treated differently
   //

if ( info.id == builtin_nint )  { do_nint();  return; }

if ( info.id == builtin_sign )  { do_sign();  return; }

   //
   //
   //

     if ( info.n_vars == 1 )  do_builtin_1(info);
else if ( info.n_vars == 2 )  do_builtin_2(info);
else {

   cerr << "\n\n  Machine::do_builtin(int) -> bad signature\n\n";

   exit ( 1 );

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_builtin_1(const BuiltinInfo & info)

{

IcodeCell operand;
IcodeCell result;


operand = cstack.pop();

if ( !(operand.is_numeric()) )  {

   cerr << "\n\n  Machine::do_builtin_1(const BuiltinInfo &) -> non-numeric operand!\n\n";

   exit ( 1 );

}

   //
   //  can we do an integer version of this function?
   //

if ( (info.i1) && (operand.type == integer) )  {

   int k;

   k = info.i1(operand.val);

   result.set_integer(k);

   cstack.push(result);

   return;

}

   //
   //  nope, do a double
   //

double x, y;


x = operand.as_double();

y = info.d1(x);


result.set_double(y);

cstack.push(result);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_builtin_2(const BuiltinInfo & info)

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();


if ( !(operand1.is_numeric()) || !(operand2.is_numeric()) )  {

   cerr << "\n\n  Machine::do_builtin_2(const BuiltinInfo &) -> non-numeric operand!\n\n";

   exit ( 1 );

}

   //
   //  can we do an integer version of this function?
   //

if ( (info.i2) && (operand1.type == integer) && (operand2.type == integer) )  {

   int k;

   k = info.i2(operand1.val, operand2.val);   

   result.set_integer(k);

   cstack.push(result);

   return;

}

   //
   //  nope, do a double
   //

double x, y, z;

x = operand1.as_double();
y = operand2.as_double();

z = info.d2(x, y);

result.set_double(z);

cstack.push(result);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_nint()

{

IcodeCell operand;


operand = cstack.pop();

if ( !(operand.is_numeric()) )  {

   cerr << "\n\n  Machine::do_nint() -> non-numeric operand!\n\n";

   exit ( 1 );

}

   //
   //  Is it an integer?
   //  If so, just return it
   //

if ( operand.type == integer )  {

   cstack.push(operand);

   return;

}

   //
   //  it's a double
   //

int j;
double x;

x = operand.as_double();

j = my_nint(x);

operand.set_integer(j);

cstack.push(operand);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_sign()

{

IcodeCell operand;
IcodeCell result;


operand = cstack.pop();

if ( !(operand.is_numeric()) )  {

   cerr << "\n\n  Machine::do_sign() -> non-numeric operand!\n\n";

   exit ( 1 );

}

int k;

if ( operand.type == integer )  {

   k = my_isign(operand.val);

} else {

   k = my_dsign(operand.d);

}

result.set_integer(k);

cstack.push(result);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::dump_constants(ostream & out)

{

int t, e;
int k, max_len;
int count;
IcodeCell cell;
const SymbolTableEntry * entry = (const SymbolTableEntry *) 0;




max_len = 0;

for (t=0; t<(sts.n_tables()); ++t)  {

   for (e=0; e<(sts.table(t).n_entries()); ++e)  {

      entry = sts.table(t).entry(e);

      switch ( entry->type )  {

         case ste_integer:
         case ste_double:
         case ste_variable:
            k = strlen(entry->name);
            if ( k > max_len )  max_len = k;
         break;

         default:
            break;

      }   //  switch

   }

}

// max_len += 1;


count = 0;

for (t=0; t<(sts.n_tables()); ++t)  {

   for (e=0; e<(sts.table(t).n_entries()); ++e)  {

      entry = sts.table(t).entry(e);

      switch ( entry->type )  {

         case ste_integer:
            out << (entry->name);
            for (k=strlen(entry->name); k<max_len; ++k)  out.put(' ');
            out << " = "
                << (entry->i)
                << "\n" << flush;
            ++count;
            if ( (count%4) == 0 )  out << "\n";
            break;

         case ste_double:
            out << (entry->name);
            for (k=strlen(entry->name); k<max_len; ++k)  out.put(' ');
            out << " = "
                << (entry->d)
                << "\n" << flush;
            ++count;
            if ( (count%4) == 0 )  out << "\n";
            break;

         case ste_variable:
            eval(entry->name);
            cell = pop();
            if ( !(cell.is_numeric()) )  {
               cerr << "\n\n  void Machine::dump_numerical_constants() const -> non-numeric result!\n\n";
               exit ( 1 );
            }
            if ( cell.type == integer )  {
               out << " (i)  " << (entry->name);
               for (k=strlen(entry->name); k<max_len; ++k)  out.put(' ');
               out << " = " << (cell.val);
            }
            if ( cell.type == floating_point )  {
               out << " (d)  " << (entry->name);
               for (k=strlen(entry->name); k<max_len; ++k)  out.put(' ');
               out << " = " << (cell.d);
            }
            out << "\n" << flush;
            ++count;
            if ( (count%4) == 0 )  out << "\n";
            break;




         default:
            break;

      }   //  switch


   }

}


return;

}


////////////////////////////////////////////////////////////////////////


SymbolTableEntry * Machine::find(const char * name) const

{

return ( sts.find(name) );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////





