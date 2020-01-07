

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "builtin.h"
#include "indent.h"
#include "icode.h"
#include "dictionary.h"

#include "celltype_to_string.h"


////////////////////////////////////////////////////////////////////////


static const int icodevector_alloc_inc = 50;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class IcodeCell
   //


////////////////////////////////////////////////////////////////////////


IcodeCell::IcodeCell()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


IcodeCell::~IcodeCell()

{

clear();

}


////////////////////////////////////////////////////////////////////////


IcodeCell::IcodeCell(const IcodeCell & icc)

{

init_from_scratch();

assign(icc);

}


////////////////////////////////////////////////////////////////////////


IcodeCell & IcodeCell::operator=(const IcodeCell & icc)

{

if ( this == &icc )  return ( * this );


assign(icc);


return ( * this );

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::init_from_scratch()

{

i = 0;

d = 0.0;

type = no_cell_type;

name = (char *) 0;

text = (char *) 0;

e = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::clear()

{

i = 0;

d = 0.0;

if ( name )  { delete [] name;   name = (char *) 0; }

if ( text )  { delete [] text;   text = (char *) 0; }

e = 0;


type = no_cell_type;



return;

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::assign(const IcodeCell & icc)

{

i = icc.i;

d = icc.d;

type = icc.type;

if ( type == identifier )  {

   name = new char [1 + strlen(icc.name)];

   strcpy(name, icc.name);

}

if ( type == character_string )  {

   text = new char [1 + strlen(icc.text)];

   strcpy(text, icc.text);

}

e = icc.e;

return;

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::set_integer(int k)

{

clear();

type = integer;

i = k;


return;

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::set_boolean(bool tf)

{

type = boolean;

i = ( tf ? 1 : 0 );

return;

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::set_double(double x)

{

clear();

type = floating_point;

d = x;

return;

}


////////////////////////////////////////////////////////////////////////


double IcodeCell::as_double() const

{

double x = 0.0;


switch ( type )  {

   case integer:         x = (double) i;  break;
   case floating_point:  x = d;           break;

   default:
      cerr << "\n\n  IcodeCell::as_double() const -> bad type ... \"" << celltype_to_string(type) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch


return ( x );

}


////////////////////////////////////////////////////////////////////////


int IcodeCell::as_int() const

{

int k = 0;

switch ( type )  {

   case integer:         k = i;            break;
   case floating_point:  k = my_nint(d);   break;

   default:
      cerr << "\n\n  IcodeCell::as_int() const -> bad type ... \"" << celltype_to_string(type) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch

return ( k );

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::set_user_function(const DictionaryEntry * _e)

{

clear();

type = user_func;

e = _e;

return;

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::dump(ostream & out, int indent_depth) const

{

// char junk[128];
Indent prefix;


prefix.depth = indent_depth;


out << prefix << "Type   = " << celltype_to_string(type) << "\n";

switch ( type )  {

   case integer:
      out << prefix << "value  = " << i << "\n";
      break;

   case boolean:
      out << prefix << "value  = " << (i ? "true" : "false" ) << "\n";
      break;

   case floating_point:
      out.precision(10);
      out << prefix << "value  = " << d << "\n";
      break;

   case op_add:
   case op_subtract:
   case op_multiply:
   case op_divide:
   case op_negate:
   case op_power:
   case op_square:
      break;

   case op_store:
   case op_recall:
      break;


   case cell_mark:
      out << prefix << "value  = " << i;
      if ( (i >= 33) && (i <= 126) )  out << "  " << "'" << ((char) i) << "'";
      out << "\n";
      break;


   case identifier:
      out << prefix << "name   = " << name << "\n";
      break;

   case character_string:
      out << prefix << "text   = \"" << text << "\"\n";
      break;

   case builtin_func:
      out << prefix << "name   = " << (binfo[i].name)   << "\n";
      out << prefix << "# args = " << (binfo[i].n_args) << "\n";
      break;

   case user_func:
      e->dump(out, indent_depth + 1);
      break;

   case local_var:
      out << prefix << "n      = " << i << "\n";
      break;

   default:
      cerr << "\n\n  IcodeCell::dump() -> unrecognized type ... \"" << celltype_to_string(type) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch



   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::set_identifier(const char * Text)

{

clear();

type = identifier;

name = new char [1 + strlen(Text)];

strcpy(name, Text);


return;

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::set_string(const char * Text)

{

clear();

const int n = strlen(Text);

type = character_string;

text = new char [1 + n];

strcpy(text, Text);

text[n] = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::set_mark(int k)

{

clear();

type = cell_mark;

i = k;

return;

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::set_local_var(int k)

{

clear();

type = local_var;

i = k;

return;

}


////////////////////////////////////////////////////////////////////////


void IcodeCell::set_builtin(int index)

{

clear();

type = builtin_func;

i = index;


return;

}


////////////////////////////////////////////////////////////////////////


bool IcodeCell::is_mark() const

{

return ( type == cell_mark );

}


////////////////////////////////////////////////////////////////////////


bool IcodeCell::is_mark(int k) const

{

return ( (type == cell_mark) && (i == k) );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class IcodeVector
   //


////////////////////////////////////////////////////////////////////////


IcodeVector::IcodeVector()

{

init_from_scratch();

return;

}


////////////////////////////////////////////////////////////////////////


IcodeVector::~IcodeVector()

{

clear();

}


////////////////////////////////////////////////////////////////////////


IcodeVector::IcodeVector(const IcodeVector & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


IcodeVector & IcodeVector::operator=(const IcodeVector & i)

{

if ( this == &i )  return ( *this );


assign(i);


return ( *this );

}


////////////////////////////////////////////////////////////////////////


void IcodeVector::init_from_scratch()

{

Cell = (IcodeCell *) 0;

Ncells = Nalloc = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void IcodeVector::assign(const IcodeVector & i)

{

clear();

if ( i.Nalloc == 0 )  return;

extend(i.Ncells);

Ncells = i.Ncells;

int j;

for (j=0; j<Ncells; ++j)  {

   Cell[j] = i.Cell[j];

}


return;

}


////////////////////////////////////////////////////////////////////////


void IcodeVector::clear()

{

if ( Cell )  { delete [] Cell;  Cell = (IcodeCell *) 0; }


Nalloc = Ncells = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void IcodeVector::extend(int n)

{

if ( n <= Nalloc )  return;

int k;
IcodeCell * u = (IcodeCell *) 0;


k = n/icodevector_alloc_inc;

if ( n%icodevector_alloc_inc ) ++k;

n = k*icodevector_alloc_inc;

u = new IcodeCell [n];

if ( !u )  {

   cerr << "\n\n  void IcodeVector::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

if ( Cell )  {

   for (k=0; k<Ncells; ++k)  {

      u[k] = Cell[k];

   }

   delete [] Cell;   Cell = (IcodeCell *) 0;

}

Cell = u;

u = (IcodeCell *) 0;

Nalloc = n;



return;

}


////////////////////////////////////////////////////////////////////////


const IcodeCell & IcodeVector::operator[](int n) const

{

if ( (n < 0) || (n >= Ncells) )  {

   cerr << "\n\n  IcodeVector::operator[](int) -> range check error!\n\n";

   exit ( 1 );

}

return ( Cell[n] );

}


////////////////////////////////////////////////////////////////////////


const IcodeCell & IcodeVector::cell(int n) const

{

if ( (n < 0) || (n >= Ncells) )  {

   cerr << "\n\n  IcodeVector::cell(int) -> range check error!\n\n";

   exit ( 1 );

}

return ( Cell[n] );

}


////////////////////////////////////////////////////////////////////////


void IcodeVector::add(const IcodeCell & c)

{

extend(Ncells + 1);

Cell[Ncells++] = c;


return;

}


////////////////////////////////////////////////////////////////////////


void IcodeVector::add(const IcodeVector & v)

{

if ( v.Ncells == 0 )  return;

extend(Ncells + v.Ncells);

int j;

for (j=0; j<(v.Ncells); ++j)  {

   Cell[Ncells + j] = v.Cell[j];

}

Ncells += v.Ncells;

return;

}


////////////////////////////////////////////////////////////////////////


void IcodeVector::add_front(const IcodeVector & v)

{

if ( Ncells == 0 )  {

   assign(v);

   return;

}

IcodeVector vv = *this;

assign(v);

add(vv);


return;

}


////////////////////////////////////////////////////////////////////////


void IcodeVector::dump(ostream & out, int indent_depth) const

{

int j;
// const char * sep = "/////////////////////////////////\n";
   const char * sep = "---------------------------------\n";
Indent prefix;


prefix.depth = indent_depth;


out << prefix << "N cells = " << Ncells << "\n";



for (j=0; j<Ncells; ++j)  {

   out << prefix << sep;

   out << prefix << "Cell # " << j << "\n";

   Cell[j].dump(out, indent_depth + 1);

}

out << prefix << sep;

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


bool IcodeVector::is_mark() const

{

if ( Ncells != 1 )  return ( false );

return ( Cell[0].is_mark() );

}


////////////////////////////////////////////////////////////////////////


bool IcodeVector::is_mark(int k) const

{

if ( Ncells != 1 )  return ( false );

return ( Cell[0].is_mark(k) );

}


////////////////////////////////////////////////////////////////////////


bool IcodeVector::is_numeric() const

{

if ( Ncells != 1 )  return ( false );

return ( Cell[0].is_numeric() );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class CellStack
   //


////////////////////////////////////////////////////////////////////////


CellStack::CellStack()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CellStack::~CellStack()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CellStack::CellStack(const CellStack & c)

{

assign(c);

}


////////////////////////////////////////////////////////////////////////


CellStack & CellStack::operator=(const CellStack & c)

{

if ( this == &c )  return ( *this );


assign(c);


return ( *this );

}


////////////////////////////////////////////////////////////////////////


void CellStack::clear()

{

Depth = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void CellStack::assign(const CellStack & s)

{

clear();

Depth = s.Depth;

int j;

for (j=0; j<Depth; ++j)  {

   cell[j] = s.cell[j];

}


return;

}


////////////////////////////////////////////////////////////////////////


void CellStack::push(const IcodeCell & c)

{

if ( Depth >= cell_stack_size )  {

   cerr << "\n\n  void CellStack::push(const CellStack &) -> stack full!\n\n";

   exit ( 1 );

}

cell[Depth++] = c;

return;

}


////////////////////////////////////////////////////////////////////////


IcodeCell CellStack::pop()

{

if ( Depth <= 0 )  {

   cerr << "\n\n  CellStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

return ( cell[--Depth] );

}


////////////////////////////////////////////////////////////////////////


const IcodeCell & CellStack::peek() const

{

if ( Depth <= 0 )  {

   cerr << "\n\n  CellStack::peek() -> stack empty!\n\n";

   exit ( 1 );

}


return ( cell[Depth - 1] );

}


////////////////////////////////////////////////////////////////////////


CellType CellStack::peek_cell_type() const

{

if ( Depth <= 0 )  {

   cerr << "\n\n  CellStack::peek_cell_type() -> stack empty!\n\n";

   exit ( 1 );

}


return ( cell[Depth - 1].type );

}


////////////////////////////////////////////////////////////////////////


void CellStack::dump_cell(ostream & out, int n, int indent_depth) const

{

if ( n >= Depth )  {

   cerr << "\n\n  CellStack::write_cell_to_screen(int, int) const -> specified level greater than stack depth\n\n";

   exit ( 1 );

}


cell[n].dump(out, indent_depth);


return;

}


////////////////////////////////////////////////////////////////////////


void CellStack::dump(ostream & out, int indent_depth) const

{

int j;
Indent prefix(indent_depth);

out << prefix << "Depth = " << Depth << '\n';

for (j=(Depth - 1); j>=0; --j)  {

   out << prefix << "Level = " << j << '\n';

   dump_cell(out, j, indent_depth + 1);

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ICVStack
   //


////////////////////////////////////////////////////////////////////////


ICVStack::ICVStack()

{

int j;

for (j=0; j<icv_stack_size; ++j)  v[j] = (IcodeVector *) 0;

clear();

}


////////////////////////////////////////////////////////////////////////


ICVStack::~ICVStack()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ICVStack::ICVStack(const ICVStack & s)

{

int j;

for (j=0; j<icv_stack_size; ++j)  v[j] = (IcodeVector *) 0;

Depth = 0;

assign(s);

}

////////////////////////////////////////////////////////////////////////


ICVStack & ICVStack::operator=(const ICVStack & s)

{

if ( this == &s )  return ( *this );

assign(s);


return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ICVStack::clear()

{

int j;

for (j=0; j<icv_stack_size; ++j)  {

   if ( v[j] )  { delete v[j];  v[j] = (IcodeVector *) 0; }

}


Depth = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void ICVStack::assign(const ICVStack & s)

{

clear();

if ( s.Depth == 0 )  return;

int j;

Depth = s.Depth;

for (j=0; j<Depth; ++j)  {

   v[j] = new IcodeVector;

   if ( !(v[j]) )  {

      cerr << "\n\n  ICVStack::assign(const ICVStack &) -> memory allocation error\n\n";

      exit ( 1 );

   }

   *(v[j]) = *(s.v[j]);

}


return;

}


////////////////////////////////////////////////////////////////////////


void ICVStack::push(const IcodeVector & V)

{

if ( Depth >= icv_stack_size )  {

   cerr << "\n\n  ICVStack::push(const IcodeVector &) -> stack full!\n\n";

   dump(cout);

   exit ( 1 );

}

v[Depth] = new IcodeVector;

if ( !(v[Depth]) )  {

   cerr << "\n\n  ICVStack::push(const IcodeVector &) -> memory allocation error\n\n";

   exit ( 1 );

}

*(v[Depth++]) = V;


return;

}


////////////////////////////////////////////////////////////////////////


IcodeVector ICVStack::pop()

{

if ( Depth <= 0 )  {

   cerr << "\n\n  ICVStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

IcodeVector V;


V = *(v[Depth - 1]);

delete v[Depth - 1];   v[Depth - 1] = (IcodeVector *) 0;

--Depth;




return ( V );

}


////////////////////////////////////////////////////////////////////////


void ICVStack::toss()

{

if ( Depth <= 0 )  return;   //  no error if stack is empty

IcodeVector V = pop();

return;

}


////////////////////////////////////////////////////////////////////////


IcodeVector * ICVStack::peek()

{

if ( Depth <= 0 )  {

   cerr << "\n\n  ICVStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}


return ( v[Depth - 1] );

}


////////////////////////////////////////////////////////////////////////


bool ICVStack::top_is_mark(int k) const

{

if ( Depth <= 0 )  return ( false );

const IcodeVector & top = *(v[Depth - 1]);


return ( top.is_mark(k) );

}


////////////////////////////////////////////////////////////////////////


void ICVStack::dump(ostream & out, int indent_depth) const

{

int j;
Indent prefix;


prefix.depth = indent_depth;

out << prefix << "ICVStack dump ... \n";
out << prefix << "Depth = " << Depth << "\n";


for (j=0; j<Depth; ++j)  {

   out << prefix << "Level " << j << "\n";

   v[j]->dump(out, indent_depth + 1);

}




out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ICVQueue
   //


////////////////////////////////////////////////////////////////////////


ICVQueue::ICVQueue()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ICVQueue::~ICVQueue()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ICVQueue::ICVQueue(const ICVQueue & q)

{

init_from_scratch();

assign(q);

}


////////////////////////////////////////////////////////////////////////


ICVQueue & ICVQueue::operator=(const ICVQueue & q)

{

if ( this == &q )  return ( * this );

assign(q);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ICVQueue::init_from_scratch()

{

memset(v, 0, sizeof(v));

Nelements = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ICVQueue::clear()

{

int j;


for (j=0; j<icv_stack_size; ++j)  {

   if ( v[j] )  { delete v[j];  v[j] = (IcodeVector *) 0; }

}

Nelements = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ICVQueue::assign(const ICVQueue & q)

{

clear();

if ( q.Nelements == 0 )  return;

Nelements = q.Nelements;

int j;

for (j=0; j<Nelements; ++j)  {

   *(v[j]) = *(q.v[j]);

}


return;

}


////////////////////////////////////////////////////////////////////////


void ICVQueue::push(const IcodeVector & icv)

{

if ( Nelements >= icv_stack_size )  {

   cerr << "\n\n  ICVQueue::push(const IcodeVector &) -> queue is full!\n\n";

   exit ( 1 );

}

v[Nelements] = new IcodeVector;

*(v[Nelements]) = icv;

++Nelements;

return;

}


////////////////////////////////////////////////////////////////////////


IcodeVector ICVQueue::pop()

{

if ( Nelements == 0 )  {

   cerr << "\n\n  ICVQueue::pop() -> queue empty!\n\n";

   exit ( 1 );

}

IcodeVector icv;

icv = *(v[0]);

int j;

for (j=1; j<Nelements; ++j)  {

   v[j - 1] = v[j];

}

v[Nelements - 1] = (IcodeVector *) 0;

--Nelements;

return ( icv );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ICVArray
   //


////////////////////////////////////////////////////////////////////////


ICVArray::ICVArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ICVArray::~ICVArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ICVArray::ICVArray(const ICVArray & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


ICVArray & ICVArray::operator=(const ICVArray & i)

{

if ( this == &i )  return ( * this );

assign(i);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ICVArray::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ICVArray::clear()

{

int j;

for (j=0; j<icv_array_size; ++j)  v[j].clear();

Nelements = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ICVArray::assign(const ICVArray & i)

{

clear();

int j;

Nelements = i.Nelements;

for (j=0; j<Nelements; ++j)  v[j] = i.v[j];


return;

}


////////////////////////////////////////////////////////////////////////


IcodeVector & ICVArray::operator[](int k)

{

if ( (k < 0) || (k >= Nelements) )  {

   cerr << "\n\n  IcodeVector & ICVArray::operator[](int) -> range check error!\n\n";

   exit ( 1 );

}


return ( v[k] );

}


////////////////////////////////////////////////////////////////////////


void ICVArray::add(const IcodeVector & icv)

{

if ( Nelements >= icv_array_size )  {

   cerr << "\n\n  ICVArray::add(const IcodeVector &) -> array full!\n\n";

   exit ( 1 );

}

v[Nelements++] = icv;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////








