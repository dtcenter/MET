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

#include "color_list.h"

#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ClistEntry
   //


////////////////////////////////////////////////////////////////////////


ClistEntry::ClistEntry()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ClistEntry::~ClistEntry()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ClistEntry::ClistEntry(const ClistEntry & e)

{

init_from_scratch();

assign(e);

}


////////////////////////////////////////////////////////////////////////


ClistEntry & ClistEntry::operator=(const ClistEntry & e)

{

if ( this == &e )  return ( * this );

assign(e);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ClistEntry::init_from_scratch()

{

Name = (string)"";

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ClistEntry::clear()

{

  if ( Name != "" )  { Name.clear(); }

D.r = D.g = D.b = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void ClistEntry::assign(const ClistEntry & e)

{

clear();

if ( e.Name != "" )  set_name(e.Name);

set_color(e.D);

return;

}


////////////////////////////////////////////////////////////////////////


void ClistEntry::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Name = \"" << Name << "\"\n";

out << prefix << "r    = " << (D.r) << "\n";
out << prefix << "g    = " << (D.g) << "\n";
out << prefix << "b    = " << (D.b) << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void ClistEntry::set_name(const std::string text)

{

  if ( Name != "" )  { Name.clear(); }

  Name = text;

  return;

}


////////////////////////////////////////////////////////////////////////


void ClistEntry::set_color(const Dcolor & d)

{

D = d;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ColorList
   //


////////////////////////////////////////////////////////////////////////


ColorList::ColorList()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ColorList::~ColorList()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ColorList::ColorList(const ColorList & c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


ColorList & ColorList::operator=(const ColorList & c)

{

if ( this == &c )  return ( * this );

assign(c);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ColorList::init_from_scratch()

{

e = (ClistEntry *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ColorList::clear()

{

if ( e )  { delete [] e;   e = (ClistEntry *) 0; }

Nelements = Nalloc = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ColorList::assign(const ColorList & c)

{

clear();

if ( c.e )  {

   extend(c.Nelements);

   int j;

   for (j=0; j<(c.Nelements); ++j)  {

      add( c.e[j] );

   }

}


return;

}


////////////////////////////////////////////////////////////////////////


void ColorList::extend(int n)

{

if ( Nalloc >= n )  return;

int k;
ClistEntry * u = (ClistEntry *) 0;


k = n/colorlist_alloc_inc;

if ( n%colorlist_alloc_inc )  ++k;

n = k*colorlist_alloc_inc;

u = new ClistEntry [n];

if ( !u )  {

   mlog << Error << "\nColorList::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

if ( e )  {

   for (k=0; k<Nelements; ++k)  u[k] = e[k];

   delete [] e;   e = (ClistEntry *) 0;

}

e = u;   u = (ClistEntry *) 0;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void ColorList::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";

int j;

for (j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " ...\n";

   e[j].dump(out, depth + 1);

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


int ColorList::has_name(const std::string text, int & index)

{

int j;

index = -1;

for (j=0; j<Nelements; ++j)  {

   if ( e[j].name() == text )  {

      index = j;

      return ( 1 );

   }

}




return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void ColorList::add(const ClistEntry & ce)

{

   //
   //  first check if a color with that name is
   //    already in the list
   //

int index;


if ( has_name(ce.name(), index) )  {

   e[index] = ce;

} else {

   extend(Nelements + 1);

   e[Nelements++] = ce;

}




return;

}


////////////////////////////////////////////////////////////////////////


ClistEntry ColorList::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   mlog << Error << "\nClist::operator[](int) const -> range check error\n\n";

   exit ( 1 );

}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////





