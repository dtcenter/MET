// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


static const int Nr = 4;
static const int Nc = 3;

static const int precision = 6;


////////////////////////////////////////////////////////////////////////


static const double values [Nr][Nc] = {

   {  0.0000001,    123.4567,  89.75      }, 
   {  0.0000000123, 12340.5,    100       }, 
   { -0.0000000123, 1234.56,   -100       }, 
   {  1, 123.567,   -100.5922  }

};


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_log.h"
#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

int r, c;
AsciiTable table;


   //
   //  fill in a table and write it out
   //

table.set_size(Nr, Nc);

table.set_precision(precision);

table.set_ics(2);


for (r=0; r<Nr; ++r)  {

   for (c=0; c<Nc; ++c)  {

      table.set_entry(r, c, values[r][c]);

   }

}

// table.line_up_decimal_points();

cout << '\n';
cout << table;
cout << '\n';

   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


