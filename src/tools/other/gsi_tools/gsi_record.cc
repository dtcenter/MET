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

#include "gsi_record.h"

using namespace std;


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

if ( this == &g )  return *this;

gsi_assign(g);

return;

}
*/

////////////////////////////////////////////////////////////////////////


void GsiRecord::gsi_init_from_scratch()

{

Buf.clear();

gsi_clear();

return;

}


////////////////////////////////////////////////////////////////////////


void GsiRecord::gsi_clear()

{

Buf.clear();

Nalloc = 0;

Shuffle = true;

RecPadLength = 4;

return;

}


////////////////////////////////////////////////////////////////////////


void GsiRecord::gsi_assign(const GsiRecord & g)

{

gsi_clear();

if ( g.Buf.empty() )  return;

if (g.Nalloc > 0) {

   extend(g.Nalloc);

   memcpy(Buf.data(), g.Buf.data(), Nalloc);

}

Shuffle = g.Shuffle;

RecPadLength = g.RecPadLength;

return;

}


////////////////////////////////////////////////////////////////////////


void GsiRecord::extend(int n_bytes)

{

if ( n_bytes <= Nalloc )  return;

   //
   //  resize, not reserve: callers read and write through Buf.data()
   //

Buf.resize(n_bytes);

Nalloc = n_bytes;


return;

}


////////////////////////////////////////////////////////////////////////




