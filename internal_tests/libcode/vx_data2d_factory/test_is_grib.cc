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

#include "vx_util.h"
#include "vx_log.h"

#include "is_grib_file.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc != 2 )  {

   mlog << Error << "\nusage:  " << program_name << " filename\n\n";

   exit ( 1 );

}

bool status1 = false;
bool status2 = false;
const char * filename = argv[1];


status1 = is_grib1_file(filename);
status2 = is_grib2_file(filename);

   //
   //  neither?
   //

if ( !status1 && !status2 )  {

   mlog << Debug(1) << "\n\n  Not a grib file\n\n";

   return ( 0 );

}

   //
   //  both?  (shouldn't happen ...)
   //

if ( status1 && status2 )  {

   mlog << Error << "\nthere must be a problem ... the code says it's BOTH grib1 and grib2 !!!\n\n";

   exit ( 1 );

}

   //
   //  must be grib1 or grib2
   //

if ( status1 )  mlog << Debug(1) << "\n\n  Grib 1\n\n";
else            mlog << Debug(1) << "\n\n  Grib 2\n\n";

   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


