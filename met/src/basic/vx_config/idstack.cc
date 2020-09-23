

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

#include "idstack.h"


////////////////////////////////////////////////////////////////////////


static const int id_array_jump = 30;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Identifier
   //


////////////////////////////////////////////////////////////////////////


Identifier::Identifier()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Identifier::~Identifier()

{

}


////////////////////////////////////////////////////////////////////////


Identifier::Identifier(const Identifier & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


Identifier & Identifier::operator=(const Identifier & i)

{

if ( this == &i )  return ( * this );

assign(i);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Identifier::init_from_scratch()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void Identifier::assign(const Identifier & i)

{

clear();

name = i.name;

return;

}


////////////////////////////////////////////////////////////////////////


void Identifier::clear()

{

name.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Identifier::set(const char * text)

{

clear();

name.assign(text);

return;

}


////////////////////////////////////////////////////////////////////////


void Identifier::dump(ostream & out, int indent_depth) const

{

Indent prefix(indent_depth);

out << prefix << "\"" << name << "\"\n";

return;

}


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

if ( this == &iq )  return ( * this );

assign(iq);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void IdentifierQueue::init_from_scratch()

{

memset(i, 0, sizeof(i));

Nelements = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void IdentifierQueue::clear()

{

int j;

for (j=0; j<max_id_queue_size; ++j)  {

   if ( i[j] )  {  delete i[j];  i[j] = (Identifier *) 0; }

}

Nelements = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void IdentifierQueue::assign(const IdentifierQueue & iq)

{

clear();

if ( iq.Nelements == 0 )  return;

int j;

Nelements = iq.Nelements;

for (j=0; j<Nelements; ++j)  {

   i[j] = new Identifier;

   *(i[j]) = *(iq.i[j]);

}



return;

}


////////////////////////////////////////////////////////////////////////


void IdentifierQueue::push(const Identifier & id)

{

if ( Nelements >= max_id_queue_size )  {

   cerr << "\n\n  void IdentifierQueue::push(const Identifier &) -> queue full!\n\n";

   exit ( 1 );

}

i[Nelements] = new Identifier;

*(i[Nelements]) = id;

++Nelements;

return;

}


////////////////////////////////////////////////////////////////////////


Identifier IdentifierQueue::pop()

{

if ( Nelements == 0 )  {

   cerr << "\n\n  IdentifierQueue::pop() -> queue empty!\n\n";

   exit ( 1 );

}

Identifier id;

id = *(i[0]);

int j;

for (j=1; j<Nelements; ++j)  {

   i[j - 1] = i[j];

}

i[Nelements - 1] = (Identifier *) 0;

--Nelements;

return ( id );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class IdentifierArray
   //


////////////////////////////////////////////////////////////////////////


IdentifierArray::IdentifierArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


IdentifierArray::~IdentifierArray()

{

clear();

if (i) delete [] i;

}


////////////////////////////////////////////////////////////////////////


IdentifierArray::IdentifierArray(const IdentifierArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


IdentifierArray & IdentifierArray::operator=(const IdentifierArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void IdentifierArray::init_from_scratch()

{

Nelements = Nalloc = 0;

i = (Identifier *) 0;

extend(1);

return;

}


////////////////////////////////////////////////////////////////////////


void IdentifierArray::clear()

{

int j;

for (j=0; j<Nalloc; ++j)  {

   i[j].clear();

}

Nelements = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void IdentifierArray::assign(const IdentifierArray & a)

{

clear();

int j;

if (a.Nelements > Nalloc) extend(a.Nelements);

Nelements = a.Nelements;

for (j=0; j<Nelements; ++j)  i[j] = a.i[j];


return;

}


////////////////////////////////////////////////////////////////////////


void IdentifierArray::extend(int n)

{

if ( Nalloc > n )  return;

int j, k;
Identifier * inew = (Identifier *) 0;


k = n/id_array_jump;

if ( n%id_array_jump )  ++k;

k *= id_array_jump;

inew = new Identifier [k];

if ( !inew )  {

   cerr << "\n\n  void IdentifierArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for (j=0; j<Nelements; ++j)  inew[j] = i[j];

if ( i )  { delete [] i;  i = (Identifier *) 0; }

i = inew;  inew = (Identifier *) 0;

Nalloc = k;

return;

}


////////////////////////////////////////////////////////////////////////


const Identifier & IdentifierArray::operator[](int k) const

{

if ( (k < 0) || (k >= Nelements) )  {

   cerr << "\n\n  IdentifierArray::operator[](int) -> range check error!\n\n";

   exit ( 1 );

}


return ( i[k] );

}


////////////////////////////////////////////////////////////////////////


void IdentifierArray::add(const Identifier & id)

{

extend(Nelements + 1);

i[Nelements++] = id;

return;

}


////////////////////////////////////////////////////////////////////////


void IdentifierArray::dump(ostream & out, int indent_depth) const

{

int j;
Indent prefix(indent_depth);


out << prefix << "Nelements = " << Nelements << "\n";

for (j=0; j<Nelements; ++j)  {

   out << "Element # " << j << " ...\n";

   i[j].dump(out, indent_depth + 1);

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

int j;

for (j=0; j<Nelements; ++j)  {

   if ( i[j].name.compare(text) == 0 )  return ( true );

}



return ( false );

}


////////////////////////////////////////////////////////////////////////


bool IdentifierArray::has(const char * text, int & index) const

{

int j;

index = -1;

for (j=0; j<Nelements; ++j)  {

   if ( i[j].name.compare(text) == 0 )  { index = j;   return ( true ); }

}



return ( false );

}


////////////////////////////////////////////////////////////////////////


void IdentifierArray::add(const char * text)

{

   //
   //  make sure it's not already there
   //

if ( has(text) )  {

   cerr << "\n\n  IdentifierArray::add(const char *) -> identifier \""
        << text << "\" is already in the array\n\n";

   exit ( 1 );

}

extend(Nelements + 1);

i[Nelements++].set(text);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////








