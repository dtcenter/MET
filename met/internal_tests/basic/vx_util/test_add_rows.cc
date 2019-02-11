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

#include "vx_log.h"
#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

// const int nr = 4;
const int nr = 1;
const int nc = 3;
int r, c;
int n;
AsciiTable table;

   //
   //  fill in a table and write it out
   //

table.set_size(nr, nc);

n = 0;

table.set_column_just(0, RightJust);
table.set_column_just(1, CenterJust);
table.set_column_just(2, LeftJust);

for (r=0; r<nr; ++r)  {

   for (c=0; c<nc; ++c)  {

      table.set_entry(r, c, n*n*n);

      ++n;

   }

}

cout << "\n";
cout << table;
cout << "\n";

   //
   //  add some rows
   //

const int NR = 2;

table.add_rows(NR);


n = 0;

for (r=0; r<NR; ++r)  {

   for (c=0; c<nc; ++c)  {

      table.set_entry(nr + r, c, n*n*n*n);

      ++n;

   }

}

cout << "\n";
cout << table;
cout << "\n";

   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


