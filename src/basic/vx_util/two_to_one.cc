// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <stdlib.h>

#include "two_to_one.h"

////////////////////////////////////////////////////////////////////////

int two_to_one(const int const nx, const int ny, const int x, const int y)

{

int n;

if ( (x < 0) || (x >= nx) || (y < 0) || (y >= ny) ) {

   cerr << "\n\nERROR: two_to_one() -> "
        << "range check error: (nx, ny) = (" << nx << ", " << ny
        << "), (x, y) = (" << x << ", " << y << ")\n\n" << flush;

   exit(1);

}

n = y*nx + x;

return ( n );

}

////////////////////////////////////////////////////////////////////////
