// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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

#include "vx_log.h"
#include "indent.h"
#include "data_class.h"
#include "data2d_utils.h"
#include "apply_mask.h"

using namespace std;


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

Raw_Grid  = (Grid *) nullptr;
Dest_Grid = (Grid *) nullptr;

ShiftRight = 0;
GridShifted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void Met2dDataFile::mtddf_clear()

{

if ( Raw_Grid  )  { delete Raw_Grid;   Raw_Grid  = (Grid *) nullptr; }
if ( Dest_Grid )  { delete Dest_Grid;  Dest_Grid = (Grid *) nullptr; }

Filename.clear();

ShiftRight = 0;
GridShifted = false;

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
out << prefix << "GridShifted = " << bool_to_string(GridShifted) << '\n';

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

return *Dest_Grid;

}


////////////////////////////////////////////////////////////////////////


const Grid & Met2dDataFile::raw_grid() const

{

if ( ! Raw_Grid )  {

   mlog << Error << "\nMet2dDataFile::raw_grid() -> "
        << "no raw grid defined!\n\n";

   exit ( 1 );

}

return *Raw_Grid;

}


////////////////////////////////////////////////////////////////////////


void Met2dDataFile::set_shift_right(int N)

{

ShiftRight = N;

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

  if ( raw_nx() <= 0 && raw_ny() <= 0 )  {

     mlog << Warning << "\nMet2dDataFile::set_grid() -> "
          << "When resetting the grid definition to \""
          << grid.serialize() << "\", the grid dimensions "
          << "are changed (" << grid.nx() << ", " << grid.ny()
          << ") != (" << raw_nx() << ", " << raw_ny() << ").\n\n";
  }
  else if ( raw_nx() != grid.nx() || raw_ny() != grid.ny() )  {

     mlog << Error << "\nMet2dDataFile::set_grid() -> "
          << "When resetting the grid definition to \""
          << grid.serialize() << "\", the grid dimensions "
          << "cannot change to (" << grid.nx() << ", " << grid.ny()
          << ") from (" << raw_nx() << ", " << raw_ny() << ").\n\n";

     exit ( 1 );

  }

if ( Dest_Grid )  { delete Dest_Grid;  Dest_Grid = nullptr; }

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

if ( Dest_Grid )  { delete Dest_Grid;  Dest_Grid = nullptr; }

Dest_Grid = new Grid;

(*Dest_Grid) = (*Raw_Grid);

return;

}


////////////////////////////////////////////////////////////////////////


int Met2dDataFile::data_planes(vector<VarInfo*> &vi_list,
                               vector<DataPlane> &dp_list)

{

int n_valid = 0;
DataPlane cur_dp;

   //
   // Loop over requested VarInfo objects
   //

for(int i=0; i<vi_list.size(); i++) {

   //
   // Initialize
   //

   cur_dp.clear();

   //
   // Read the current DataPlane
   //

   if(data_plane(*vi_list[i], cur_dp)) n_valid++;
   else                                cur_dp.clear();

   //
   // Store the result
   //

   dp_list.emplace_back(cur_dp);

}

return n_valid;

}


////////////////////////////////////////////////////////////////////////

bool Met2dDataFile::derive_winds(VarInfo *vinfo, DataPlane &dp)

{

if(!vinfo) return false;

bool status = false;

   //
   // Derive wind fields from U and V
   //

if(vinfo->need_uv_wind()) {
   DataPlane uwnd_dp;
   DataPlane vwnd_dp;
   ConcatString uwnd_units;
   ConcatString vwnd_units;
   status = read_wind_data(vinfo, vinfo->wind_info().u_wind,
                           uwnd_dp, uwnd_units) &&
            read_wind_data(vinfo, vinfo->wind_info().v_wind,
                           vwnd_dp, vwnd_units);

   if(status) {

      // Rotate U/V winds, if needed
      if(vinfo->need_rotation()) {
         DataPlane uwnd_dp_orig(uwnd_dp);
         DataPlane vwnd_dp_orig(vwnd_dp);
         rotate_uv_grid_to_earth(uwnd_dp_orig, vwnd_dp_orig,
                                 *Raw_Grid, uwnd_dp, vwnd_dp);
         vinfo->set_earth_relative();
      }

      // Derive wind speed
      if(vinfo->is_wind_speed()) {
         status = derive_wind_speed(uwnd_dp, vwnd_dp, dp);
         vinfo->set_long_name("Wind Speed");
         vinfo->set_units(uwnd_units);
      }
      // Derive wind direction
      else if(vinfo->is_wind_direction()) {
         status = derive_wind_direction(uwnd_dp, vwnd_dp, dp);
         vinfo->set_long_name("Wind Direction");
         vinfo->set_units("deg");
      }
      // Derive kinetic energy
      else if(vinfo->is_kinetic_energy()) {
         status = derive_kinetic_energy(uwnd_dp, vwnd_dp, dp);
         vinfo->set_long_name("Kinetic Energy");
         vinfo->set_units("J/kg");
      }
   }
}

   //
   // Derive U and V from wind speed and direction
   //

else if(vinfo->is_u_wind() || vinfo->is_v_wind()) {
   DataPlane wspd_dp;
   DataPlane wdir_dp;
   ConcatString wspd_units;
   ConcatString wdir_units;
   status = read_wind_data(vinfo, vinfo->wind_info().wind_speed,
                           wspd_dp, wspd_units) &&
            read_wind_data(vinfo, vinfo->wind_info().wind_direction,
                           wdir_dp, wdir_units);

   // Rotate wind direction, if needed
   if(vinfo->need_rotation()) {
      DataPlane wdir_dp_orig(wdir_dp);
      rotate_wind_direction_grid_to_earth(wdir_dp_orig,
                                          *Raw_Grid, wdir_dp);
      vinfo->set_earth_relative();
   }

   if(status) {
      if(vinfo->is_u_wind()) {
         status = derive_u_wind(wspd_dp, wdir_dp, dp);
         vinfo->set_long_name("U-Component of Wind");
         vinfo->set_units(wspd_units);
      }
      else {
         status = derive_v_wind(wspd_dp, wdir_dp, dp);
         vinfo->set_long_name("V-Component of Wind");
         vinfo->set_units(wspd_units);
      }
   }
}

return status;

}


////////////////////////////////////////////////////////////////////////


bool Met2dDataFile::derive_winds(VarInfo *vinfo, DataPlaneArray &dpa)

{

static const char *method_name = "Met2dDataFile::derive_winds() -> ";

if(!vinfo) return false;

bool status = false;

   //
   // Derive wind fields from U and V
   //

if(vinfo->need_uv_wind()) {
   DataPlaneArray uwnd_dpa;
   DataPlaneArray vwnd_dpa;
   ConcatString uwnd_units;
   ConcatString vwnd_units;
   status = read_wind_data(vinfo, vinfo->wind_info().u_wind,
                           uwnd_dpa, uwnd_units) &&
            read_wind_data(vinfo, vinfo->wind_info().v_wind,
                           vwnd_dpa, vwnd_units);

   if(!status) return status;

   // Rotate U/V winds, if needed
   if(vinfo->need_rotation()) {
      for(int i=0; i<uwnd_dpa.n_planes(); i++) {
         DataPlane uwnd_dp_orig(uwnd_dpa[i]);
         DataPlane vwnd_dp_orig(vwnd_dpa[i]);
         rotate_uv_grid_to_earth(uwnd_dp_orig, vwnd_dp_orig,
                                 *Raw_Grid,
                                 uwnd_dpa.at(i), vwnd_dpa.at(i));
      }
      vinfo->set_earth_relative();
   }

   // Store the long name and units
   if(vinfo->is_wind_speed()) {
      vinfo->set_long_name("Wind Speed");
      vinfo->set_units(uwnd_units);
   }
   else if(vinfo->is_wind_direction()) {
      vinfo->set_long_name("Wind Direction");
      vinfo->set_units("deg");
   }
   else if(vinfo->is_kinetic_energy()) {
      vinfo->set_long_name("Kinetic Energy");
      vinfo->set_units("J/kg");
   }

   // Check for matching dimensions
   if(uwnd_dpa.n_planes() != vwnd_dpa.n_planes()) {
      mlog << Warning << "\n" << method_name
           << "when deriving winds, the number of U-wind records ("
           << uwnd_dpa.n_planes()
           << ") does not match the number of V-wind records ("
           << vwnd_dpa.n_planes()
           << ") for file '" << filename() << "'\n\n";
      return false;
   }

   //
   // Loop through each of the data planes
   //
   for(int i=0; i<uwnd_dpa.n_planes(); i++) {

      // Levels must match
      if(!is_eq(uwnd_dpa.lower(i), vwnd_dpa.lower(i)) ||
         !is_eq(uwnd_dpa.upper(i), vwnd_dpa.upper(i)) ){
         mlog << Warning << "\n" << method_name
              << "when deriving winds for level " << i+1
              << ", the U-wind levels (" << uwnd_dpa.lower(i)
              << ", " << uwnd_dpa.upper(i)
              << ") do not match the V-wind levels ("
              << vwnd_dpa.lower(i) << ", " << vwnd_dpa.upper(i)
              << ") in file '" << filename() << "'\n\n";
         return false;
      }

      // Do the derivation
      DataPlane dp;

      // Derive wind speed
      if(vinfo->is_wind_speed()) {
         status = derive_wind_speed(uwnd_dpa[i], vwnd_dpa[i], dp);
      }
      // Derive wind direction
      else if(vinfo->is_wind_direction()) {
         status = derive_wind_direction(uwnd_dpa[i], vwnd_dpa[i], dp);
      }
      // Derive kinetic energy
      else if(vinfo->is_kinetic_energy()) {
         status = derive_kinetic_energy(uwnd_dpa[i], vwnd_dpa[i], dp);
      }

      // Store the result
      dpa.add(dp, uwnd_dpa.lower(i), uwnd_dpa.upper(i));
   }
}

   //
   // Derive U and V from wind speed and direction
   //

else if(vinfo->is_u_wind() || vinfo->is_v_wind()) {
   DataPlaneArray wspd_dpa;
   DataPlaneArray wdir_dpa;
   ConcatString wspd_units;
   ConcatString wdir_units;
   status = read_wind_data(vinfo, vinfo->wind_info().wind_speed,
                           wspd_dpa, wspd_units) &&
            read_wind_data(vinfo, vinfo->wind_info().wind_direction,
                           wdir_dpa, wdir_units);

   if(!status) return status;

   // Rotate wind direction, if needed
   if(vinfo->need_rotation()) {
      for(int i=0; i<wdir_dpa.n_planes(); i++) {
         DataPlane wdir_dp_orig(wdir_dpa[i]);
         rotate_wind_direction_grid_to_earth(wdir_dp_orig,
                                             *Raw_Grid, wdir_dpa.at(i));
         vinfo->set_earth_relative();
      }
   }

   // Store the long name and units
   if(vinfo->is_u_wind()) {
      vinfo->set_long_name("U-Component of Wind");
      vinfo->set_units(wspd_units);
   }
   else {
      vinfo->set_long_name("V-Component of Wind");
      vinfo->set_units(wspd_units);
   }

   if(wspd_dpa.n_planes() != wdir_dpa.n_planes()) {
      mlog << Warning << "\n" << method_name
           << "when deriving winds, the number of wind speed records ("
           << wspd_dpa.n_planes()
           << ") does not match the number of wind direction records ("
           << wdir_dpa.n_planes()
           << ") for file '" << filename() << "'\n\n";
      return false;
   }

   //
   // Loop through each of the data planes
   //
   for(int i=0; i<wspd_dpa.n_planes(); i++) {

      // Levels must match
      if(!is_eq(wspd_dpa.lower(i), wdir_dpa.lower(i)) ||
         !is_eq(wspd_dpa.upper(i), wdir_dpa.upper(i)) ){
         mlog << Warning << "\n" << method_name
              << "when deriving winds for level " << i+1
              << ", the wind speed levels (" << wspd_dpa.lower(i)
              << ", " << wspd_dpa.upper(i)
              << ") do not match the wind direction levels ("
              << wdir_dpa.lower(i) << ", " << wdir_dpa.upper(i)
              << ") in file '" << filename() << "'\n\n";
         return false;
      }

      // Do the derivation
      DataPlane dp;
      if(vinfo->is_u_wind()) {
         status = derive_u_wind(wspd_dpa[i], wdir_dpa[i], dp);
      }
      else {
         status = derive_v_wind(wspd_dpa[i], wdir_dpa[i], dp);
      }

      // Store the result
      dpa.add(dp, wspd_dpa.lower(i), wspd_dpa.upper(i));
   }
}

return status;

}


////////////////////////////////////////////////////////////////////////


bool Met2dDataFile::rotate_winds(VarInfo *vinfo, DataPlane &dp)

{

static const char *method_name = "Met2dDataFile::rotate_winds(DataPlane) -> ";

if(!vinfo) return false;

if(!vinfo->is_wind_rotation()) return true;

if(vinfo->is_wind_rotation() && !vinfo->is_grid_relative()) {
   mlog << Debug(3) << "Wind field \"" << vinfo->magic_str()
        << "\" is already defined as earth-relative.\n";
   return true;
}

   //
   // Apply conversion logic

bool status = false;

DataPlane uwnd_dp;
DataPlane vwnd_dp;
DataPlane tmp_dp;
ConcatString units;

// Rotate U-Wind
if(vinfo->is_u_wind()) {
   uwnd_dp = dp;
   status = read_wind_data(vinfo, vinfo->wind_info().v_wind,
                           vwnd_dp, units);
   if(status) status = rotate_uv_grid_to_earth(uwnd_dp, vwnd_dp,
                                               *Raw_Grid,
                                               dp, tmp_dp);
}
// Rotate V-Wind
else if(vinfo->is_v_wind()) {
   vwnd_dp = dp;
   status = read_wind_data(vinfo, vinfo->wind_info().u_wind,
                           uwnd_dp, units);
   if(status) status = rotate_uv_grid_to_earth(uwnd_dp, vwnd_dp,
                                               *Raw_Grid,
                                               tmp_dp, dp);
}
// Rotate Wind Direction
else if(vinfo->is_wind_direction()) {
   tmp_dp = dp;
   rotate_wind_direction_grid_to_earth(tmp_dp, *Raw_Grid, dp);
   status = true;
}

if(!status) {
   mlog << Warning << "\n" << method_name
        << "Trouble rotating wind field (" << vinfo->magic_str()
        << ") from grid to earth relative.\n\n";
}
// Update flag for a successful rotation
else {
   vinfo->set_earth_relative();
}

return status;

}


////////////////////////////////////////////////////////////////////////


bool Met2dDataFile::rotate_winds(VarInfo *vinfo, DataPlaneArray &dpa)

{

static const char *method_name = "Met2dDataFile::rotate_winds(DataPlaneArray) -> ";

if(!vinfo) return false;

if(!vinfo->is_wind_rotation()) return true;

if(vinfo->is_wind_rotation() && !vinfo->is_grid_relative()) {
   mlog << Debug(3) << "Wind field \"" << vinfo->magic_str()
        << "\" is already defined as earth-relative.\n";
   return true;
}

bool status = false;

DataPlaneArray uwnd_dpa;
DataPlaneArray vwnd_dpa;
DataPlaneArray tmp_dpa(dpa);
ConcatString units;
DataPlaneArray *uwnd_out;
DataPlaneArray *vwnd_out;

// Rotate U-Wind and V-Wind
if(vinfo->is_u_wind() || vinfo->is_v_wind()) {

   if(vinfo->is_u_wind()) {
      uwnd_dpa = dpa;
      uwnd_out = &dpa;
      vwnd_out = &tmp_dpa;
      status = read_wind_data(vinfo, vinfo->wind_info().v_wind,
                              vwnd_dpa, units);
   }
   else {
      vwnd_dpa = dpa;
      uwnd_out = &tmp_dpa;
      vwnd_out = &dpa;
      status = read_wind_data(vinfo, vinfo->wind_info().u_wind,
                              uwnd_dpa, units);
   }

   // Check for matching levels
   if(!uwnd_dpa.levels_match(vwnd_dpa)) {
      mlog << Warning << "\n" << method_name
           << "The U-wind and V-wind levels do not match.\n\n";
      status = false;
   }

   // Rotate each plane
   if(status) {
      for(int i=0; i<uwnd_dpa.n_planes(); i++) {
         status = rotate_uv_grid_to_earth(
                     uwnd_dpa[i], vwnd_dpa[i],
                     *Raw_Grid, uwnd_out->at(i), vwnd_out->at(i));
         if(!status) break;
      }
   }
}
// Rotate Wind Direction
else if(vinfo->is_wind_direction()) {
   tmp_dpa = dpa;

   // Rotate each plane
   for(int i=0; i<dpa.n_planes(); i++) {
      rotate_wind_direction_grid_to_earth(
         tmp_dpa[i], *Raw_Grid, dpa.at(i));
   }
   status = true;
}

if(!status) {
   mlog << Warning << "\n" << method_name
        << "Trouble rotating wind fields (" << vinfo->magic_str()
        << ") from grid to earth relative.\n\n";
}
// Update flag for a successful rotation
else {
   vinfo->set_earth_relative();
}

return status;

}


////////////////////////////////////////////////////////////////////////


bool Met2dDataFile::read_wind_data(VarInfo *vinfo,
                                   const StringArray &names,
                                   DataPlane &dp,
                                   ConcatString &units)

{

static const char *method_name = "Met2dDataFile::read_wind_data(DataPlane) -> ";

if(!vinfo) return false;

bool status = false;

    // Copy input VarInfo

VarInfo * vinfo_cur = vinfo->clone();

    // Try each of the possible names

for(int i=0; i<names.n(); i++) {

   // Check for a match
   if(vinfo_cur->reset_dict_with_name(names[i].c_str()) &&
      data_plane(*vinfo_cur, dp, false)) {
      status = true;
      units = vinfo_cur->units();
      vinfo->set_grid_relative_flag(vinfo_cur->is_grid_relative());
      mlog << "Found matching wind field \""
           << vinfo_cur->magic_str() << "\".\n";
      break;
   }
}

   // Cleanup

if(vinfo_cur) { delete vinfo_cur; vinfo_cur = nullptr; }

if(!status) {
   mlog << Warning << "\n" << method_name
        << "No matching wind data found for names \""
        << write_css(names) << "\".\n\n";
}

return status;

}


////////////////////////////////////////////////////////////////////////


bool Met2dDataFile::read_wind_data(VarInfo *vinfo,
                                   const StringArray &names,
                                   DataPlaneArray &dpa,
                                   ConcatString &units)

{

static const char *method_name = "Met2dDataFile::read_wind_data(DataPlaneArray) -> ";

if(!vinfo) return false;

bool status = false;

    // Copy input VarInfo

VarInfo * vinfo_cur = vinfo->clone();

    // Try each of the possible names

for(int i=0; i<names.n(); i++) {

   // Check for a match
   if(vinfo_cur->reset_dict_with_name(names[i].c_str()) &&
      data_plane_array(*vinfo_cur, dpa, false)) {
      status = true;
      units = vinfo_cur->units();
      vinfo->set_grid_relative_flag(vinfo_cur->is_grid_relative());
      mlog << "Found matching wind field(s) \""
           << vinfo_cur->magic_str() << "\".\n";
      break;
   }
}

   // Cleanup

if(vinfo_cur) { delete vinfo_cur; vinfo_cur = nullptr; }

if(!status) {
   mlog << Warning << "\n" << method_name
        << "No matching wind data found for names \""
        << write_css(names) << "\".\n\n";
}

return status;

}


////////////////////////////////////////////////////////////////////////


bool Met2dDataFile::process_data_plane(VarInfo *vinfo, DataPlane &dp)

{

if ( ! vinfo )  return false;

   //
   // Apply conversion logic
   //

dp.convert(vinfo->ConvertFx);

   //
   // Apply censor logic
   //

dp.censor(vinfo->censor_thresh(), vinfo->censor_val());

   //
   // Check for no valid input data
   //

if ( dp.is_all_bad_data() )  {

   mlog << Warning << "\nThe field \"" << vinfo->magic_str()
        << "\" contains no valid data!\n\n";

}

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

   //
   // Apply shift to the right logic
   //

if ( ShiftRight != 0 )  {

   // Shift the grid, but only once
   if ( Dest_Grid && !GridShifted )  {

      Dest_Grid->shift_right(ShiftRight);
      GridShifted = true;

   }

   // Shift the data
   dp.shift_right(ShiftRight);

}

   //
   // Print the grid information and data summary
   //

if ( mlog.verbosity_level() >= 4 ) {

   mlog << Debug(4) << "\n"
        << "Grid information:\n   "
        << (Dest_Grid ? Dest_Grid->serialize("\n   ") : "(nul)") << "\n";

   double min_v;
   double max_v;
   dp.data_range(min_v, max_v);
   mlog << Debug(4) << "\n"
        << "Data plane information:\n"
        << "   plane min: " << min_v << "\n"
        << "   plane max: " << max_v << "\n"
        << "   valid time: " << unix_to_yyyymmdd_hhmmss(dp.valid()) << "\n"
        << "   lead time: " << sec_to_hhmmss(dp.lead()) << "\n"
        << "   init time: " << unix_to_yyyymmdd_hhmmss(dp.init()) << "\n"
        << "   accum time: " << sec_to_hhmmss(dp.accum()) << "\n\n";

}

return true;

}


////////////////////////////////////////////////////////////////////////
