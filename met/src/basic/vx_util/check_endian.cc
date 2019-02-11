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
#include <cmath>

#include "check_endian.h"


////////////////////////////////////////////////////////////////////////


void shuffle_2(void * p)

{

unsigned char * u = (unsigned char *) p;


swap_uc(u[0], u[1]);


return;

}


////////////////////////////////////////////////////////////////////////


void shuffle_4(void * p)

{

unsigned char * u = (unsigned char *) p;


swap_uc(u[0], u[3]);

swap_uc(u[1], u[2]);


return;

}


////////////////////////////////////////////////////////////////////////


void shuffle_8(void * p)

{

unsigned char * u = (unsigned char *) p;


swap_uc(u[0], u[7]);

swap_uc(u[1], u[6]);

swap_uc(u[2], u[5]);

swap_uc(u[3], u[4]);


return;

}


////////////////////////////////////////////////////////////////////////





