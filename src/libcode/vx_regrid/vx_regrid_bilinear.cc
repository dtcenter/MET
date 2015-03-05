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


DataPlane met_regrid_bilinear (const DataPlane & in, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info)

{

DataPlane out;

out.set_size(to_grid.nx(), to_grid.ny());

// Copy timing info
out.set_init(in.init());
out.set_valid(in.valid());
out.set_lead(in.lead());
out.set_accum(in.accum());

   //
   //  done
   //

return ( out );

}


////////////////////////////////////////////////////////////////////////


