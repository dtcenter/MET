// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


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

using namespace std;


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

if ( this == &c )  return *this;

assign(c);

return *this;

}


////////////////////////////////////////////////////////////////////////


void EquivalenceClass::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void EquivalenceClass::clear()

{

E.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void EquivalenceClass::assign(const EquivalenceClass & c)

{

E = c.E;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void EquivalenceClass::add_no_repeat(int k)

{

if ( has(k) )  return;

E.push_back(k);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int EquivalenceClass::element(int k) const

{

if ( (k < 0) || (k >= n_elements()) )  {

   mlog << Error << "\nEquivalenceClass::element(int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}


return E[k];

}


////////////////////////////////////////////////////////////////////////


void EquivalenceClass::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << '(' << n_elements() << ") {";

int j;

for (j=0; j<n_elements(); ++j)  {

   out << E[j];

   if ( j != (n_elements() - 1) )  out << ',';

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

if ( E.empty() )  return 0;

int j, n;

n = E[0];

for (j=1; j<n_elements(); ++j)  {

   n = max(n, E[j]);

}

   //
   //  NOTE: returns 0, not n.  That is what this has always done; it is a
   //  bug, but fixing it would change mtd output and is not this commit's
   //  to make.
   //

return 0;

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

if ( this == &p )  return *this;

assign(p);

return *this;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::clear()

{

C.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::assign(const Mtd_Partition & p)

{

clear();

C.reserve(p.C.size());

for (const auto & c : p.C)  {

   C.push_back(std::make_unique<EquivalenceClass>(*c));

}

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);

out << prefix << '[' << n_elements() << " equivalence classes]\n";

for (j=0; j<n_elements(); ++j)  {

   C[j]->dump(out, depth + 1);

}



   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


bool Mtd_Partition::has(int index, int k) const

{

if ( (index < 0) || (index >= n_elements()) )  {

   mlog << Error << "\nMtd_Partition::has(int index, int k) const -> "
        << "range check error on index\n\n";

   exit ( 1 );

}


return C[index]->has(k);

}


////////////////////////////////////////////////////////////////////////


int Mtd_Partition::which_class(int k) const

{

int j;

for (j=0; j<n_elements(); ++j)  {

   if ( C[j]->has(k) )  return j;

}


return -1;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::merge_classes(int nclass_1, int nclass_2)

{

if ( (nclass_1 < 0) || (nclass_1 >= n_elements()) || (nclass_2 < 0) || (nclass_2 >= n_elements()) )  {

   mlog << Error << "\nMtd_Partition::merge_classes() -> "
        << "range check error\n\n";

   exit ( 1 );

}

if ( nclass_1 == nclass_2 )  return;

int k, n;
int n_class_min, n_class_max;
EquivalenceClass * c_min = (EquivalenceClass *) nullptr;
EquivalenceClass * c_max = (EquivalenceClass *) nullptr;


n_class_min = min(nclass_1, nclass_2);
n_class_max = max(nclass_1, nclass_2);

c_min = C[n_class_min].get();
c_max = C[n_class_max].get();

n = c_max->n_elements();

for (k=0; k<n; ++k)  {

   c_min->add_no_repeat(c_max->E[k]);

}


   //
   //  erase shifts the tail down AND frees the merged-away class, which the
   //  hand-written shift below did not: it overwrote C[n_class_max] and
   //  nulled the last slot, leaking c_max.
   //

C.erase(C.begin() + n_class_max);

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

   mlog << Error << "\nMtd_Partition::merge_values() -> "
        << "bad values ... "
        << "(value_1, value_2) = " << value_1 << ", " << value_2 << " ... "
        << "(nclass_1, nclass_2) = " << nclass_1 << ", " << nclass_2
        << "\n\n";

   exit ( 1 );

}

merge_classes(nclass_1, nclass_2);

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_Partition::add_no_repeat(int k)

{

if ( has(k) )  return;

C.push_back(std::make_unique<EquivalenceClass>());

C.back()->add_no_repeat(k);

return;

}


////////////////////////////////////////////////////////////////////////


const EquivalenceClass * Mtd_Partition::operator()(int k) const

{

if ( (k < 0) || (k >= n_elements()) )  {

   mlog << Error << "\nMtd_Partition::operator()(int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}


return C[k].get();

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

out << '[' << n_elements() << " equivalence classes]\n";

for (j=0; j<n_elements(); ++j)  {

   c = C[j].get();

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


return out;

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


