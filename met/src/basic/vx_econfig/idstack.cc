

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

memcpy(name, i.name, sizeof(name));

return;

}


////////////////////////////////////////////////////////////////////////


void Identifier::clear()

{

memset(name, 0, sizeof(name));

return;

}


////////////////////////////////////////////////////////////////////////


void Identifier::set(const char * text)

{

clear();

int n;

n = strlen(text);


if ( n >= (max_id_size - 1) )  {

   cerr << "\n\n  void Identifier::set(const char *) string too long!\n\n";

   exit ( 1 );

}


memcpy(name, text, n);


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
int do_sep;

do_sep = 0;

for (j=0; j<Nelements; ++j)  {

   if ( do_sep )   out << ", ";

   out << (i[j].name);

   do_sep = 1;

}

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


int IdentifierArray::has(const char * text) const

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( strcmp(text, i[j].name) == 0 )  return ( 1 );

}



return ( 0 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////








