// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include "vx_regrid.h"

#include "interp_mthd.h"


////////////////////////////////////////////////////////////////////////


DataPlane met_regrid_budget (const DataPlane & from_data, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info)

{

DataPlane to_data;

to_data.set_size(to_grid.nx(), to_grid.ny());

   //
   // Copy timing info
   //

to_data.set_init  (from_data.init());
to_data.set_valid (from_data.valid());
to_data.set_lead  (from_data.lead());
to_data.set_accum (from_data.accum());




   //
   //  done
   //

return ( to_data );

}


////////////////////////////////////////////////////////////////////////


