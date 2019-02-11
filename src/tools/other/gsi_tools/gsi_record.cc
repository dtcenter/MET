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

#include "gsi_record.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GsiRecord
   //


////////////////////////////////////////////////////////////////////////


GsiRecord::GsiRecord()

{

gsi_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


GsiRecord::~GsiRecord()

{

gsi_clear();

}


////////////////////////////////////////////////////////////////////////

/*
GsiRecord::GsiRecord(const GsiRecord & g)

{

gsi_init_from_scratch();

gsi_assign(g);

return;

}
*/

////////////////////////////////////////////////////////////////////////

/*
GsiRecord & GsiRecord::operator=(const GsiRecord & g)

{

if ( this == &g )  return ( * this );

gsi_assign(g);

return;

}
*/

////////////////////////////////////////////////////////////////////////


void GsiRecord::gsi_init_from_scratch()

{

Buf = 0;

gsi_clear();

return;

}


////////////////////////////////////////////////////////////////////////


void GsiRecord::gsi_clear()

{

if ( Buf )  { delete [] Buf;  Buf = 0; }

Nalloc = 0;

Shuffle = true;

RecPadLength = 4;

return;

}


////////////////////////////////////////////////////////////////////////


void GsiRecord::gsi_assign(const GsiRecord & g)

{

gsi_clear();

if ( !(g.Buf) )  return;

extend(g.Nalloc);

memcpy(Buf, g.Buf, Nalloc);

Shuffle = g.Shuffle;

RecPadLength = g.RecPadLength;

return;

}


////////////////////////////////////////////////////////////////////////


void GsiRecord::extend(int n_bytes)

{

if ( n_bytes <= Nalloc )  return;

unsigned char * u = new unsigned char [n_bytes];

if ( Buf )  {

   memcpy(u, Buf, Nalloc);

   delete [] Buf;  Buf = 0;

}

Nalloc = n_bytes;

Buf = u;   u = 0;


return;

}


////////////////////////////////////////////////////////////////////////




