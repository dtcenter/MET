// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include "indent.h"
#include "data_class.h"
#include "data2d_utils.h"
#include "apply_mask.h"

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Met2dData
   //


////////////////////////////////////////////////////////////////////////


Met2dData::Met2dData()

{

mtdd_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Met2dData::~Met2dData()

{


}


////////////////////////////////////////////////////////////////////////


void Met2dData::mtdd_init_from_scratch()

{


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Met2dDataFile
   //


////////////////////////////////////////////////////////////////////////


Met2dDataFile::Met2dDataFile()

{

mtddf_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Met2dDataFile::~Met2dDataFile()

{

mtddf_clear();

}


////////////////////////////////////////////////////////////////////////


void Met2dDataFile::mtddf_init_from_scratch()

{

Raw_Grid  = (Grid *) 0;
Dest_Grid = (Grid *) 0;

ShiftRight = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void Met2dDataFile::mtddf_clear()

{

if ( Raw_Grid  )  { delete Raw_Grid;   Raw_Grid  = (Grid *) 0; }
if ( Dest_Grid )  { delete Dest_Grid;  Dest_Grid = (Grid *) 0; }

Filename.clear();

ShiftRight = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void Met2dDataFile::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "File = ";

if ( Filename.empty() )  out << "(nul)\n";
else                     out << '\"' << Filename << "\"\n";

out << prefix << "Raw Grid = ";

if ( Raw_Grid )  {

   out << '\n';

   Raw_Grid->dump(out, depth + 1);

} else out << "(nul)\n";


out << prefix << "Dest Grid = ";

if ( Dest_Grid )  {

   out << '\n';

   Dest_Grid->dump(out, depth + 1);

} else out << "(nul)\n";

out << prefix << "ShiftRight = " << ShiftRight << '\n';

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


const Grid & Met2dDataFile::grid() const

{

if ( ! Dest_Grid )  {

   mlog << Error << "\nMet2dDataFile::grid() -> "
        << "no grid defined!\n\n";

   exit ( 1 );

}

return ( *Dest_Grid );

}


////////////////////////////////////////////////////////////////////////


const Grid & Met2dDataFile::raw_grid() const

{

if ( ! Raw_Grid )  {

   mlog << Error << "\nMet2dDataFile::raw_grid() -> "
        << "no raw grid defined!\n\n";

   exit ( 1 );

}

return ( *Raw_Grid );

}


////////////////////////////////////////////////////////////////////////


void Met2dDataFile::set_shift_right(int N)

{

ShiftRight = N;

if ( Dest_Grid )  Dest_Grid->shift_right(ShiftRight);

return;

}


////////////////////////////////////////////////////////////////////////


void Met2dDataFile::set_grid(const Grid &grid)

{

mlog << Debug(3) << "Resetting grid definition from \""
     << Dest_Grid->serialize() << "\" to \"" << grid.serialize()
     << "\".\n";

     //
     // Make sure the grid dimensions do not change
     //

  if ( raw_nx() != grid.nx() || raw_ny() != grid.ny() )  {

     mlog << Error << "\nMet2dDataFile::set_grid() -> "
          << "When resetting the grid definition to \""
          << grid.serialize() << "\", the grid dimensions "
          << "cannot change (" << grid.nx() << ", " << grid.ny()
          << ") != (" << raw_nx() << ", " << raw_ny() << ").\n\n";

     exit ( 1 );

  }

if ( Dest_Grid )  { delete Dest_Grid;  Dest_Grid = 0; }

Dest_Grid = new Grid;

(*Dest_Grid) = grid;

return;

}


////////////////////////////////////////////////////////////////////////


void Met2dDataFile::copy_raw_grid_to_dest()

{

if ( ! Raw_Grid )  {

   mlog << Error << "\nMet2dDataFile::copy_raw_grid_to_dest() -> "
        << "no raw grid set!\n\n";

   exit ( 1 );

}

if ( Dest_Grid )  { delete Dest_Grid;  Dest_Grid = 0; }

Dest_Grid = new Grid;

(*Dest_Grid) = (*Raw_Grid);

return;

}


////////////////////////////////////////////////////////////////////////


void Met2dDataFile::process_data_plane(VarInfo *vinfo, DataPlane &dp)

{

if ( ! vinfo )  return;

   //
   // Apply shift to the right logic
   //

if ( ShiftRight != 0 )  dp.shift_right(ShiftRight);

   //
   // Apply conversion logic
   //

dp.convert(vinfo->ConvertFx);

   //
   // Apply censor logic
   //

dp.censor(vinfo->censor_thresh(), vinfo->censor_val());

   //
   // Update the metadata, if requested
   //

set_attrs(vinfo, dp);

   //
   // Update the grid definition, if requested
   //

if ( vinfo->grid_attr().nxy() > 0 )  {

   set_grid(vinfo->grid_attr());

}

return;

}


////////////////////////////////////////////////////////////////////////
