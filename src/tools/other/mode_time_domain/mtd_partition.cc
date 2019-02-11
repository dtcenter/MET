// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <time.h>

#include "vx_log.h"
#include "nint.h"
#include "indent.h"
#include "concat_string.h"
#include "string_array.h"

#include "mtd_partition.h"


////////////////////////////////////////////////////////////////////////


int n_eq_max = 0;   //  needs external linkage


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class EquivalenceClass
   //


////////////////////////////////////////////////////////////////////////


EquivalenceClass::EquivalenceClass()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


EquivalenceClass::~EquivalenceClass() 

{

n_eq_max = max(n_eq_max, n_max());

clear();

}


////////////////////////////////////////////////////////////////////////


EquivalenceClass::EquivalenceClass(const EquivalenceClass & c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


EquivalenceClass & EquivalenceClass::operator=(const EquivalenceClass &c)

{

if ( this == &c )  return ( *this );

assign(c);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void EquivalenceClass::init_from_scratch()

{

E = (int *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void EquivalenceClass::clear()

{

if ( E )  { delete [] E;  E = (int *) 0; }

Nelements = Nalloc = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void EquivalenceClass::assign(const EquivalenceClass & c)

{

clear();

if ( !(c.E) )  return;

extend(c.Nelements);

int j;

for (j=0; j<(c.Nelements); ++j)  E[j] = c.E[j];

Nelements = c.Nelements;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void EquivalenceClass::extend(int n)

{

if ( Nalloc >= n )  return;

n = eq_alloc_inc*((n + eq_alloc_inc - 1)/eq_alloc_inc);

int * u = new int [n];

memset(u, 0, n*sizeof(int));

if ( E )  {

   memcpy(u, E, Nelements*sizeof(int));

   delete [] E;  E = (int *) 0;

}

E = u;  u = (int *) 0;

Nalloc = n;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

/*
bool EquivalenceClass::has(int k) const

{

int j;
int * e = E;

for (j=0; j<Nelements; ++j, ++e)  {

   if ( *e == k )  return ( true );

}


return ( false );

}
*/

////////////////////////////////////////////////////////////////////////


void EquivalenceClass::add_no_repeat(int k)

{

if ( has(k) )  return;

extend(Nelements + 1);

E[Nelements++] = k;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int EquivalenceClass::element(int k) const

{

if ( (k < 0) || (k >= Nelements) )  {

   mlog << Error << "\n\n  EquivalenceClass::element(int) const -> range check error\n\n";

   exit ( 1 );

}


return ( E[k] );

}


////////////////////////////////////////////////////////////////////////


void EquivalenceClass::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << '(' << Nelements << ") {";

int j;

for (j=0; j<Nelements; ++j)  {

   out << E[j];

   if ( j != (Nelements - 1) )  out << ',';

}

out << "}\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


int EquivalenceClass::n_max() const

{

if ( Nelements == 0 )  return ( 0 );

int j, n;

n = E[0];

for (j=1; j<Nelements; ++j)  {

   n = max(n, E[j]);

}

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Mtd_Partition
   //


////////////////////////////////////////////////////////////////////////


Mtd_Partition::Mtd_Partition()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Mtd_Partition::~Mtd_Partition() 

{

clear();

}


////////////////////////////////////////////////////////////////////////


Mtd_Partition::Mtd_Partition(const Mtd_Partition &p)

{

init_from_scratch();

assign(p);

}


////////////////////////////////////////////////////////////////////////


Mtd_Partition & Mtd_Partition::operator=(const Mtd_Partition &p)

{

if ( this == &p )  return ( * this );

assign(p);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::init_from_scratch()

{

C = (EquivalenceClass **) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::clear()

{

int j;

if ( C )  {

   for (j=0; j<Nalloc; ++j)  {

      if ( C[j] )  { delete C[j];  C[j] = (EquivalenceClass *) 0; }

   }

   delete [] C;   C = (EquivalenceClass **) 0;

}

Nelements = Nalloc = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::assign(const Mtd_Partition & p)

{

clear();

if ( !(p.C) )  return;

extend(p.Nelements);

int j;

for (j=0; j<(p.Nelements); ++j)  {

   C[j] = new EquivalenceClass;

   *(C[j]) = *(p.C[j]);

}

Nelements = p.Nelements;

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::extend(int n)

{

if ( n <= Nalloc )  return;

EquivalenceClass ** u = (EquivalenceClass **) 0;

n = mtd_partition_alloc_inc*((n + mtd_partition_alloc_inc - 1)/mtd_partition_alloc_inc);

u = new EquivalenceClass * [n];

memset(u, 0, n*(sizeof(EquivalenceClass *)));

if ( C )  {

   memcpy(u, C, Nelements*(sizeof(EquivalenceClass *)));

   delete [] C;  C = (EquivalenceClass **) 0;

}

C = u;  u = (EquivalenceClass **) 0;

Nalloc = n;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);

out << prefix << '[' << Nelements << " equivalence classes]\n";

for (j=0; j<Nelements; ++j)  {

   C[j]->dump(out, depth + 1);

}



   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////

/*
bool Mtd_Partition::has(int k) const

{

int j;
EquivalenceClass ** c = C;

for (j=0; j<Nelements; ++j, ++c)  {

   if ( (*c)->has(k) )  return ( true );

}


return ( false );

}
*/

////////////////////////////////////////////////////////////////////////


bool Mtd_Partition::has(int index, int k) const

{

if ( (index < 0) || (index >= Nelements) )  {

   mlog << Error << "\n\n  Mtd_Partition::has(int index, int k) const -> range check error on index\n\n";

   exit ( 1 );

}


return ( C[index]->has(k) );

}


////////////////////////////////////////////////////////////////////////


int Mtd_Partition::which_class(int k) const

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( C[j]->has(k) )  return ( j );

}


return ( -1 );

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::merge_classes(int nclass_1, int nclass_2)

{

if ( (nclass_1 < 0) || (nclass_1 >= Nelements) || (nclass_2 < 0) || (nclass_2 >= Nelements) )  {

   mlog << Error << "\n\n  Mtd_Partition::merge_classes() -> range check error\n\n";

   exit ( 1 );

}

if ( nclass_1 == nclass_2 )  return;

int k, n;
int n_class_min, n_class_max;
EquivalenceClass * c_min = (EquivalenceClass *) 0;
EquivalenceClass * c_max = (EquivalenceClass *) 0;


n_class_min = min(nclass_1, nclass_2);
n_class_max = max(nclass_1, nclass_2);

c_min = C[n_class_min];
c_max = C[n_class_max];

n = c_max->Nelements;

for (k=0; k<n; ++k)  {

   c_min->add_no_repeat(c_max->E[k]);

}


for (k=n_class_max; k<(Nelements - 1); ++k)  {

   C[k] = C[k + 1];

}

C[Nelements - 1] = (EquivalenceClass *) 0;

--Nelements;

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::merge_values(int value_1, int value_2)

{

if ( value_1 == value_2 )  return;

int nclass_1, nclass_2;

nclass_1 = which_class(value_1);
nclass_2 = which_class(value_2);

if ( (nclass_1 < 0) || (nclass_2 < 0) )  {

   mlog << Error << "\n\n  Mtd_Partition::merge_values() -> bad values ... "
        << "(value_1, value_2) = " << value_1 << ", " << value_2 << " ... "
        << "(nclass_1, nclass_2) = " << nclass_1 << ", " << nclass_2
        << "\n\n";

   exit ( 1 );

   return;

}

merge_classes(nclass_1, nclass_2);

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::add_no_repeat(int k)

{

if ( has(k) )  return;

extend(Nelements + 1);

C[Nelements] = new EquivalenceClass;

C[Nelements]->add_no_repeat(k);

++Nelements;

return;

}


////////////////////////////////////////////////////////////////////////


const EquivalenceClass * Mtd_Partition::operator()(int k) const

{

if ( (k < 0) || (k >= Nelements) )  {

   mlog << Error << "\n\n  Mtd_Partition::operator()(int) const -> range check error\n\n";

   exit ( 1 );

}


return ( C[k] );

}


////////////////////////////////////////////////////////////////////////


ConcatString Mtd_Partition::specialized_dump_string(const int Nf, const int No) const

{

int j, k, n;
int id;
static const char tab [] = "   ";
const EquivalenceClass * c = 0;
ConcatString s;
ConcatString out;
StringArray a;

out << '[' << Nelements << " equivalence classes]\n";

for (j=0; j<Nelements; ++j)  {

   c = C[j];

   a.clear();

   out << tab << '(' << (c->n_elements()) << ") { ";

   for (k=0; k<(c->n_elements()); ++k)  {

      s.erase();

      n = c->element(k);

      if ( n < Nf )  { s << 'F';  id = n; }
      else           { s << 'O';  id = n - Nf; }

      s << '_' << id;

      a.add(s);

   }

   a.sort();

   for (k=0; k<(a.n_elements()); ++k)  {

      out << a[k];

      if ( k != (a.n_elements() - 1) )  out << ' ';

   }

   out << " }\n";

}   //  for j




return ( out );

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::specialized_dump(ostream & out, const int Nf, const int No) const

{

ConcatString s = specialized_dump_string(Nf, No);

out << s;


return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::specialized_dump(int verbosity, const int Nf, const int No) const

{

ConcatString s = specialized_dump_string(Nf, No);

mlog << Debug(verbosity) << s;

return;

}


////////////////////////////////////////////////////////////////////////




