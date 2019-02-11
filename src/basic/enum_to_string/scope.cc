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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "scope.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ScopeStackElement
   //


////////////////////////////////////////////////////////////////////////


ScopeStackElement::ScopeStackElement()

{

Name = (const char *) 0;

clear();

}


////////////////////////////////////////////////////////////////////////


ScopeStackElement::~ScopeStackElement()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ScopeStackElement::ScopeStackElement(const ScopeStackElement & e)

{

Name = (const char *) 0;

assign(e);

}


////////////////////////////////////////////////////////////////////////


ScopeStackElement & ScopeStackElement::operator=(const ScopeStackElement & e)

{

if ( this == &e )  return ( *this );

assign(e);

return ( *this );

}


////////////////////////////////////////////////////////////////////////


void ScopeStackElement::assign(const ScopeStackElement & e)

{


clear();

Level = e.Level;

set_name(e.Name);


return;

}


////////////////////////////////////////////////////////////////////////


void ScopeStackElement::clear()

{

Level = 0;

if ( Name )  { delete [] Name;  Name = (const char *) 0; }


return;

}


////////////////////////////////////////////////////////////////////////


void ScopeStackElement::set_name(const char * text)

{

if ( Name )  { delete [] Name;  Name = (const char *) 0; }

if ( !text )  return;

int k;
char * c = (char *) 0;

k = strlen(text);

c = new char [1 + k];

if ( !c )  {

   cerr << "\n\n  void ScopeStackElement::set_name(const char *) -> memory allocation error\n\n";

   exit ( 1 );

}

strncpy(c, text, k);

c[k] = (char) 0;   //  just to make sure

Name = (const char *) c;   c = (char *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ScopeStackElement::set_level(int d)

{

if ( d < 0 )   {

   cerr << "\n\n  ScopeStackElement::set_depth(int) -> bad value!\n\n";

   exit ( 1 );

}

Level = d;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ScopeStack
   //


////////////////////////////////////////////////////////////////////////


ScopeStack::ScopeStack()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ScopeStack::~ScopeStack()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ScopeStack::ScopeStack(const ScopeStack & ss)

{

assign(ss);

}


////////////////////////////////////////////////////////////////////////


ScopeStack & ScopeStack::operator=(const ScopeStack & ss)

{

if ( this == &ss )  return ( *this );

assign(ss);

return ( *this );

}


////////////////////////////////////////////////////////////////////////


void ScopeStack::clear()

{

int j;
ScopeStackElement e;

for (j=0; j<max_scope_stack_depth; ++j)  {

   s[j].clear();

}

N = 0;

push(e);

return;

}


////////////////////////////////////////////////////////////////////////


void ScopeStack::assign(const ScopeStack & ss)

{

clear();

if ( ss.N == 0 )  return;


N = ss.N;

int j;

for (j=0; j<N; ++j)  {

   s[j] = ss.s[j];

}


return;

}


////////////////////////////////////////////////////////////////////////


const ScopeStackElement & ScopeStack::peek(int pos) const

{

if ( (pos < 0) || (pos >= N) )  {

   cerr << "\n\n  ScopeStack::peek(int) -> range check error\n\n";

   exit ( 1 );

}


return ( s[pos] );

}


////////////////////////////////////////////////////////////////////////


void ScopeStack::pop()

{

if ( N == 0 )  {

   cerr << "\n\n  void ScopeStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

--N;

s[N].clear();


return;

}


////////////////////////////////////////////////////////////////////////


void ScopeStack::push(const ScopeStackElement & e)

{

if ( N >= max_scope_stack_depth )  {

   cerr << "\n\n  ScopeStack::push() -> stack full!\n\n";

   exit ( 1 );

}

s[N] = e;

++N;

return;

}


////////////////////////////////////////////////////////////////////////


void ScopeStack::level_push(const ScopeStackElement & e)

{

clear_to_level(e.level());

push(e);


return;

}


////////////////////////////////////////////////////////////////////////


void ScopeStack::clear_to_level(int k)

{

if ( k < 0 )  {

   cerr << "\n\n  ScopeStack::clear_to_level(int) -> bad level\n\n";

   exit ( 1 );

}

while ( (N > 0) && (s[N - 1].level() >= k) )  pop();


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & s, const ScopeStackElement & sse)

{

if ( sse.name() )  {

   s << "   name  = \"" << (sse.name())  << "\"\n";

} else {

   s << "   name  = (nul)\n";

}


s << "   level = "   << (sse.level()) << "\n";

s.flush();

return ( s );

}


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & s, const ScopeStack & ss)

{

if ( ss.n_elements() == 0 )  {

   s << "   (stack empty)\n";

   return ( s );

}

int j;
int n;
ScopeStackElement e;

n = ss.n_elements();

for (j=0; j<n; ++j)  {

   e = ss.peek(j);

   s << "Scope Element " << j << "\n";

   s << e;

}


return ( s );

}


////////////////////////////////////////////////////////////////////////





