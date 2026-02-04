////////////////////////////////////////////////////////////////////////
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "idstack.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static const int id_array_jump = 30;

////////////////////////////////////////////////////////////////////////
//
//  Code for class IdentifierQueue
//
////////////////////////////////////////////////////////////////////////

IdentifierQueue::IdentifierQueue()

{

init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

IdentifierQueue::~IdentifierQueue()

{

clear();

}

////////////////////////////////////////////////////////////////////////

IdentifierQueue::IdentifierQueue(const IdentifierQueue & iq)

{

init_from_scratch();

assign(iq);

}

////////////////////////////////////////////////////////////////////////

IdentifierQueue & IdentifierQueue::operator=(const IdentifierQueue & iq)

{

if ( this == &iq )  return *this;

assign(iq);

return *this;

}

////////////////////////////////////////////////////////////////////////

void IdentifierQueue::init_from_scratch()

{

return;

}

////////////////////////////////////////////////////////////////////////

void IdentifierQueue::clear()

{

i.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void IdentifierQueue::assign(const IdentifierQueue & iq)

{

i = iq.i;

return;

}


////////////////////////////////////////////////////////////////////////


void IdentifierQueue::push(const string & id)

{

i.emplace_back(id);

return;

}


////////////////////////////////////////////////////////////////////////


string IdentifierQueue::pop()

{

if ( i.empty() )  {

   cerr << "\nIdentifierQueue::pop() -> "
        << "queue empty!\n\n";

   exit ( 1 );

}

string s = i[0];

i.erase(i.begin());

return s;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class IdentifierArray
   //


////////////////////////////////////////////////////////////////////////


IdentifierArray::IdentifierArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


IdentifierArray::~IdentifierArray()

{

clear();

}

////////////////////////////////////////////////////////////////////////

IdentifierArray::IdentifierArray(const IdentifierArray & a)

{

clear();

assign(a);

}

////////////////////////////////////////////////////////////////////////

IdentifierArray & IdentifierArray::operator=(const IdentifierArray & a)

{

if ( this == &a )  return *this;

assign(a);

return *this;

}

////////////////////////////////////////////////////////////////////////

void IdentifierArray::init_from_scratch()

{

extend(1);

return;

}

////////////////////////////////////////////////////////////////////////

void IdentifierArray::clear()

{

i.clear();

return;

}

////////////////////////////////////////////////////////////////////////

void IdentifierArray::assign(const IdentifierArray & a)

{

clear();

i = a.i;

return;

}

////////////////////////////////////////////////////////////////////////

void IdentifierArray::extend(int n)

{

int k = n/id_array_jump;

if ( n%id_array_jump )  ++k;

k *= id_array_jump;

i.reserve(k);

return;

}

////////////////////////////////////////////////////////////////////////

const string & IdentifierArray::operator[](int k) const

{

if ( (k < 0) || (k >= i.size()) )  {

   cerr << "\nIdentifierArray::operator[](int) -> "
        << "range check error!\n\n";

   exit ( 1 );

}

return i[k];

}

////////////////////////////////////////////////////////////////////////

void IdentifierArray::add(const string & s)

{

extend((int) i.size() + 1);

i.emplace_back(s);

return;

}

////////////////////////////////////////////////////////////////////////

void IdentifierArray::dump(ostream & out, int indent_depth) const

{

Indent prefix(indent_depth);

Indent prefix2(indent_depth + 1);

out << prefix << "Nelements = " << i.size() << "\n";

for (int j=0; j<i.size(); ++j)  {

   out << "Element # " << j << " ...\n";

   out << prefix2 << "\"" << i[j] << "\"\n";

}

   //
   //  done
   //

out.flush();

return;

}

////////////////////////////////////////////////////////////////////////

bool IdentifierArray::has(const char * text) const

{

for (const auto &j : i) {

   if ( j.compare(text) == 0 )  return true;

}

return false;

}

////////////////////////////////////////////////////////////////////////

bool IdentifierArray::has(const char * text, int & index) const

{

index = -1;

for (int j=0; j<i.size(); ++j)  {

   if ( i[j].compare(text) == 0 )  { index = j; return true; }

}

return false;

}

////////////////////////////////////////////////////////////////////////

void IdentifierArray::add(const char * text)

{

   //
   //  make sure it's not already there
   //

if ( has(text) )  {

   cerr << "\nIdentifierArray::add(const char *) -> "
        << "identifier \"" << text
        << "\" is already in the array\n\n";

   exit ( 1 );

}

extend((int) i.size() + 1);

i.emplace_back(text);

   //
   //  done
   //

return;

}

////////////////////////////////////////////////////////////////////////
