// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


   //
   //   Tests the calculation of solar right ascension and declination
   //
   //     used by the vx_solar library.
   //
   //
   //
   //  Test case using Example 24.a (Page 153) of 
   //
   //  "Astronomical Algorithms", 1st Ed.  by Jean Meeus
   //
   //
   //   Note: it's Example 25.a (Page 165) in the 2nd Ed.
   //


static const double correct_ra  = 15.0*( 13.0 + 13.0/60.0 + 30.749/3600.0 );   //  "correct" according to Meeus
static const double correct_dec =     -(  7.0 + 47.0/60.0 +  1.740/3600.0 );   //


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "solar.h"


////////////////////////////////////////////////////////////////////////


int main()

{

double Ra, Dec;
double ra_error, dec_error;
double max_error;
unixtime gmt;
char junk[256];
const char * format = "%10.5f";


gmt = mdyhms_to_unix(10, 13, 1992, 0, 0, 0);

solar_radec(gmt, Ra, Dec);

ra_error  = fabs(Ra  - correct_ra);
dec_error = fabs(Dec - correct_dec);


max_error = (ra_error > dec_error) ? ra_error : dec_error;

cout << "\n\n";

snprintf(junk, sizeof(junk), format, Ra);
cout << "Right Ascension          = " << junk << " degrees\n\n";

snprintf(junk, sizeof(junk), format, Dec);
cout << "Declination              = " << junk << " degrees\n\n";

cout << "\n\n";

cout << "Error in Right Ascension = " <<  ra_error << " degrees\n\n";

cout << "Error in Declination     = " << dec_error << " degrees\n\n";

cout << "\n\n";

cout << "Maximum Error            = " << max_error << " degrees\n\n";

cout << "\n\n";

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


