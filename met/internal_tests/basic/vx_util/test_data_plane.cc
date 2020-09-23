// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


static const int test_x        = 15;
static const int test_y        = 30;

static const double test_value = 333.33333;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

DataPlane p;
DataPlaneArray a;

p.set_size(100, 200);

a.add(p, 500.0, 1000.0);

p.set(test_value, test_x, test_y);

a.add(p, 1000.0, 2000.0);

cout << "\n\n";

a.dump(cout);

cout << "\n\n";

cout << "Test value = " << a.data(1, test_x, test_y) << "\n\n";

const DataPlane & pp = a[1];

cout << "Test value = " << pp(test_x, test_y) << "\n\n";


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


