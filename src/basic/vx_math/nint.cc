// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "nint.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


int nint(double x)

{

double y;
int a;

y = floor(x + 0.5);

a = (int) y;

if ( fabs(a - y) > 0.3 )  ++a;

return ( a );

}


////////////////////////////////////////////////////////////////////////


int positive_modulo(int i, int n)

{

return ( i % n + n ) % n;

}


////////////////////////////////////////////////////////////////////////
