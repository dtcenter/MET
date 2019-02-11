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

#include "vx_util.h"

#include "color_stack.h"


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class ColorStack
   //


////////////////////////////////////////////////////////////////////////


ColorStack::ColorStack()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ColorStack::~ColorStack()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ColorStack::ColorStack(const ColorStack & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


ColorStack & ColorStack::operator=(const ColorStack & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ColorStack::init_from_scratch()

{

e = (Color **) 0;

AllocInc = 10;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ColorStack::clear()

{

if ( e )  {

   int j;

   for (j=0; j<Nalloc; ++j)  {

      if ( e[j] )  { delete e[j];  e[j] = (Color *) 0; }

   }

   delete [] e;  e = (Color **) 0;

}   //  if e


Nelements = 0;

Nalloc = 0;

// AllocInc = 10;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


void ColorStack::assign(const ColorStack & _a)

{

clear();

if ( _a.depth() == 0 )  return;

extend(_a.depth());

int j;

for (j=0; j<(_a.depth()); ++j)  {

   *(e[j]) = *(_a.e[j]);

}

Nelements = _a.Nelements;

return;

}


////////////////////////////////////////////////////////////////////////


void ColorStack::extend(int n)

{

if ( n <= Nalloc )  return;

n = AllocInc*( (n + AllocInc - 1)/AllocInc );

int j;
Color ** u = new Color * [n];

if ( !u )  {

   mlog << Error << "ColorStack::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, n*sizeof(Color *));

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (Color **) 0; }

e = u;

u = (Color **) 0;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void ColorStack::dump(ostream & out, int _depth_) const

{

Indent prefix(_depth_);

out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "AllocInc  = " << AllocInc  << "\n";

int j;

for(j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " ... \n";
   e[j]->dump(out, _depth_ + 1);


}

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void ColorStack::set_alloc_inc(int n)

{

if ( n < 0 )  {

   mlog << Error << "ColorStack::set_alloc_int(int) -> bad value ... " << n << "\n\n";

   exit ( 1 );

}

if ( n == 0 )  AllocInc = 10;   //  default value
else           AllocInc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void ColorStack::push(const Color & a)

{

extend(Nelements + 1);

e[Nelements] = new Color;

if ( !(e[Nelements]) )  {

   mlog << Error << "ColorStack::add(const Color &) -> memory allocation error\n\n";

   exit ( 1 );

}

*(e[Nelements++]) = a;

return;

}


////////////////////////////////////////////////////////////////////////


Color ColorStack::pop()

{

if ( Nelements <= 0 )  {

   mlog << Error << "ColorStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

Color _t = *(e[Nelements - 1]);

--Nelements;

return ( _t );

}


////////////////////////////////////////////////////////////////////////


Color ColorStack::peek() const

{

if ( Nelements <= 0 )  {

   mlog << Error << "ColorStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

Color _t = *(e[Nelements - 1]);

return ( _t );

}


////////////////////////////////////////////////////////////////////////


