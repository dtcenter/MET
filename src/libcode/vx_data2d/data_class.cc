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


static StringArray swap_uv_name(VarInfo *,
                                const ConcatString &,
                                const StringArray &);


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

   // Create local copies
   VarInfo * vinfo_uwnd = vinfo->clone();
   VarInfo * vinfo_vwnd = vinfo->clone();

   DataPlane uwnd_dp;
   DataPlane vwnd_dp;
   status = read_wind_data(vinfo_uwnd,
                           conf_key_u_wind_field_name,
                           vinfo->wind_info().u_wind,
                           uwnd_dp) &&
            read_wind_data(vinfo_vwnd,
                           conf_key_v_wind_field_name,
                           vinfo->wind_info().v_wind,
                           vwnd_dp);

   if(status) {

      // Rotate U/V winds, if needed
      if(vinfo->is_wind_rotation() &&
         vinfo_uwnd->need_rotation() &&
         vinfo_vwnd->need_rotation()) {
         DataPlane uwnd_dp_orig(uwnd_dp);
         DataPlane vwnd_dp_orig(vwnd_dp);
         mlog << Debug(3) << "Rotating U and V wind fields "
              << "from grid-relative to earth-relative.\n";
         rotate_uv_grid_to_earth(uwnd_dp_orig, vwnd_dp_orig,
                                 *Raw_Grid, uwnd_dp, vwnd_dp);
      }

      // Derive wind speed
      if(vinfo->is_wind_speed()) {
         status = derive_wind_speed(uwnd_dp, vwnd_dp, dp);
         vinfo->set_long_name("Wind Speed");
         vinfo->set_units(vinfo_uwnd->units());
      }
      // Derive wind direction
      else if(vinfo->is_wind_direction()) {
         status = derive_wind_direction(uwnd_dp, vwnd_dp, dp);
         vinfo->set_earth_relative();
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

   // Cleanup
   if(vinfo_uwnd) { delete vinfo_uwnd; vinfo_uwnd = nullptr; }
   if(vinfo_vwnd) { delete vinfo_vwnd; vinfo_vwnd = nullptr; }
}

   //
   // Derive U and V from wind speed and direction
   //

else if(vinfo->is_u_wind() || vinfo->is_v_wind()) {

   // Create local copies
   VarInfo * vinfo_wspd = vinfo->clone();
   VarInfo * vinfo_wdir = vinfo->clone();

   DataPlane wspd_dp;
   DataPlane wdir_dp;
   status = read_wind_data(vinfo_wspd,
                           conf_key_wind_speed_field_name,
                           vinfo->wind_info().wind_speed,
                           wspd_dp) &&
            read_wind_data(vinfo_wdir,
                           conf_key_wind_direction_field_name,
                           vinfo->wind_info().wind_direction,
                           wdir_dp);

   // Rotate wind direction, if needed
   if(vinfo_wdir->need_rotation()) {
      DataPlane wdir_dp_orig(wdir_dp);
      mlog << Debug(3) << "Rotating wind direction field "
           << "from grid-relative to earth-relative.\n";
      rotate_wind_direction_grid_to_earth(wdir_dp_orig,
                                          *Raw_Grid, wdir_dp);
      vinfo->set_earth_relative();
   }

   if(status) {
      if(vinfo->is_u_wind()) {
         status = derive_u_wind(wspd_dp, wdir_dp, dp);
         vinfo->set_long_name("U-Component of Wind");
         vinfo->set_units(vinfo_wspd->units());
      }
      else {
         status = derive_v_wind(wspd_dp, wdir_dp, dp);
         vinfo->set_long_name("V-Component of Wind");
         vinfo->set_units(vinfo_wspd->units());
      }
   }

   // Cleanup
   if(vinfo_wspd) { delete vinfo_wspd; vinfo_wspd = nullptr; }
   if(vinfo_wdir) { delete vinfo_wdir; vinfo_wdir = nullptr; }
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

   // Create local copies
   VarInfo * vinfo_uwnd = vinfo->clone();
   VarInfo * vinfo_vwnd = vinfo->clone();

   DataPlaneArray uwnd_dpa;
   DataPlaneArray vwnd_dpa;
   status = read_wind_data(vinfo_uwnd,
                           conf_key_u_wind_field_name,
                           vinfo->wind_info().u_wind,
                           uwnd_dpa) &&
            read_wind_data(vinfo_vwnd,
                           conf_key_v_wind_field_name,
                           vinfo->wind_info().v_wind,
                           vwnd_dpa);

   if(!status) return status;

   // Rotate U/V winds, if needed
   if(vinfo->is_wind_rotation() &&
      vinfo_uwnd->need_rotation() &&
      vinfo_vwnd->need_rotation()) {
      mlog << Debug(3) << "Rotating " << uwnd_dpa.n_planes()
           << " U and V wind fields from grid-relative "
           << "to earth-relative.\n";
      for(int i=0; i<uwnd_dpa.n_planes(); i++) {
         DataPlane uwnd_dp_orig(uwnd_dpa[i]);
         DataPlane vwnd_dp_orig(vwnd_dpa[i]);
         rotate_uv_grid_to_earth(uwnd_dp_orig, vwnd_dp_orig,
                                 *Raw_Grid,
                                 uwnd_dpa.at(i), vwnd_dpa.at(i));
      }
   }

   // Store the long name and units
   if(vinfo->is_wind_speed()) {
      vinfo->set_long_name("Wind Speed");
      vinfo->set_units(vinfo_uwnd->units());
   }
   else if(vinfo->is_wind_direction()) {
      vinfo->set_earth_relative();
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

   // Cleanup
   if(vinfo_uwnd) { delete vinfo_uwnd; vinfo_uwnd = nullptr; }
   if(vinfo_vwnd) { delete vinfo_vwnd; vinfo_vwnd = nullptr; }
}

   //
   // Derive U and V from wind speed and direction
   //

else if(vinfo->is_u_wind() || vinfo->is_v_wind()) {

   // Create local copies
   VarInfo * vinfo_wspd = vinfo->clone();
   VarInfo * vinfo_wdir = vinfo->clone();

   DataPlaneArray wspd_dpa;
   DataPlaneArray wdir_dpa;
   status = read_wind_data(vinfo_wspd,
                           conf_key_wind_speed_field_name,
                           vinfo->wind_info().wind_speed,
                           wspd_dpa) &&
            read_wind_data(vinfo_wdir,
                           conf_key_wind_direction_field_name,
                           vinfo->wind_info().wind_direction,
                           wdir_dpa);

   if(!status) return status;

   // Rotate wind direction, if needed
   if(vinfo_wdir->need_rotation()) {
      mlog << Debug(3) << "Rotating " << wdir_dpa.n_planes()
           << " wind direction field(s) from grid-relative "
           << "to earth-relative.\n";
      for(int i=0; i<wdir_dpa.n_planes(); i++) {
         DataPlane wdir_dp_orig(wdir_dpa[i]);
         rotate_wind_direction_grid_to_earth(wdir_dp_orig,
                                             *Raw_Grid, wdir_dpa.at(i));
      }
      vinfo->set_earth_relative();
   }

   // Store the long name and units
   if(vinfo->is_u_wind()) {
      vinfo->set_long_name("U-Component of Wind");
      vinfo->set_units(vinfo_wspd->units());
   }
   else {
      vinfo->set_long_name("V-Component of Wind");
      vinfo->set_units(vinfo_wspd->units());
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

   // Cleanup
   if(vinfo_wspd) { delete vinfo_wspd; vinfo_wspd = nullptr; }
   if(vinfo_wdir) { delete vinfo_wdir; vinfo_wdir = nullptr; }
}

return status;

}


////////////////////////////////////////////////////////////////////////


bool Met2dDataFile::rotate_winds(VarInfo *vinfo, DataPlane &dp)

{

static const char *method_name = "Met2dDataFile::rotate_winds(DataPlane) -> ";

if(!vinfo) return false;

if(!vinfo->is_wind_rotation()) return true;

if(vinfo->is_grid_relative()) {
   mlog << Debug(3) << "Rotating wind field \""
        << vinfo->magic_str()
        << "\" from grid-relative to earth-relative.\n";
}
else {
   mlog << Debug(3) << "Identified wind field \""
        << vinfo->magic_str()
        << "\" as being earth-relative.\n";
   return true;
}

bool status = false;

// Create local copy
VarInfo * vinfo_wind = vinfo->clone();

DataPlane uwnd_dp;
DataPlane vwnd_dp;
DataPlane tmp_dp;

// Rotate U-Wind
if(vinfo->is_u_wind()) {
   uwnd_dp = dp;
   status = read_wind_data(vinfo_wind,
                           conf_key_v_wind_field_name,
                           vinfo->wind_info().v_wind,
                           vwnd_dp);
   if(status) status = rotate_uv_grid_to_earth(uwnd_dp, vwnd_dp,
                                               *Raw_Grid,
                                               dp, tmp_dp);
}
// Rotate V-Wind
else if(vinfo->is_v_wind()) {
   vwnd_dp = dp;
   status = read_wind_data(vinfo_wind,
                           conf_key_u_wind_field_name,
                           vinfo->wind_info().u_wind,
                           uwnd_dp);
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

// Cleanup
if(vinfo_wind) { delete vinfo_wind; vinfo_wind = nullptr; }

return status;

}


////////////////////////////////////////////////////////////////////////


bool Met2dDataFile::rotate_winds(VarInfo *vinfo, DataPlaneArray &dpa)

{

static const char *method_name = "Met2dDataFile::rotate_winds(DataPlaneArray) -> ";

if(!vinfo) return false;

if(!vinfo->is_wind_rotation() || dpa.n_planes() == 0) return true;

if(vinfo->is_grid_relative()) {
   mlog << Debug(3) << "Rotating " << dpa.n_planes()
        << " wind field(s) for \"" << vinfo->magic_str()
        << "\" from grid-relative to earth-relative.\n";
}
else {
   mlog << Debug(3) << "Identified " << dpa.n_planes()
        << " wind field(s) for \"" << vinfo->magic_str()
        << "\" as being earth-relative.\n";
   return true;
}

bool status = false;

// Create local copy
VarInfo * vinfo_wind = vinfo->clone();

DataPlaneArray uwnd_dpa;
DataPlaneArray vwnd_dpa;
DataPlaneArray tmp_dpa(dpa);
DataPlaneArray *uwnd_out;
DataPlaneArray *vwnd_out;

// Rotate U-Wind and V-Wind
if(vinfo->is_u_wind() || vinfo->is_v_wind()) {

   if(vinfo->is_u_wind()) {
      uwnd_dpa = dpa;
      uwnd_out = &dpa;
      vwnd_out = &tmp_dpa;
      status = read_wind_data(vinfo_wind,
                              conf_key_v_wind_field_name,
                              vinfo->wind_info().v_wind,
                              vwnd_dpa);
   }
   else {
      vwnd_dpa = dpa;
      uwnd_out = &tmp_dpa;
      vwnd_out = &dpa;
      status = read_wind_data(vinfo_wind,
                              conf_key_u_wind_field_name,
                              vinfo->wind_info().u_wind,
                              uwnd_dpa);
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

// Cleanup
if(vinfo_wind) { delete vinfo_wind; vinfo_wind = nullptr; }

return status;

}


////////////////////////////////////////////////////////////////////////


bool Met2dDataFile::read_wind_data(VarInfo *vinfo,
                                   const char *conf_key_name,
                                   const StringArray &names,
                                   DataPlane &dp)

{

static const char *method_name = "Met2dDataFile::read_wind_data(DataPlane) -> ";

if(!vinfo) return false;

bool status = false;

    // Update search names, if needed

StringArray search_names(swap_uv_name(vinfo, conf_key_name, names));

    // Try each of the possible names

for(int i=0; i<search_names.n(); i++) {

   // Find matching data with no more wind processing
   if(vinfo->reset_dict_with_name(search_names[i].c_str()) &&
      data_plane(*vinfo, dp, false)) {
      status = true;
      mlog << "Found matching wind field \""
           << vinfo->magic_str() << "\".\n";
      break;
   }
}

if(!status) {
   mlog << Warning << "\n" << method_name
        << "No matching wind field found for name(s) \""
        << write_css(search_names) << "\".\n"
        << "Set \"" << conf_key_name
        << "\" to specify the matching variable name.\n\n";
}

return status;

}


////////////////////////////////////////////////////////////////////////


bool Met2dDataFile::read_wind_data(VarInfo *vinfo,
                                   const char *conf_key_name,
                                   const StringArray &names,
                                   DataPlaneArray &dpa)

{

static const char *method_name = "Met2dDataFile::read_wind_data(DataPlaneArray) -> ";

if(!vinfo) return false;

bool status = false;

    // Update search names, if needed

StringArray search_names(swap_uv_name(vinfo, conf_key_name, names));

    // Try each of the possible names

for(int i=0; i<search_names.n(); i++) {

   // Find matching data with no more wind processing
   if(vinfo->reset_dict_with_name(search_names[i].c_str()) &&
      data_plane_array(*vinfo, dpa, false)) {
      status = true;
      mlog << "Found matching wind field(s) \""
           << vinfo->magic_str() << "\".\n";
      break;
   }
}

if(!status) {
   mlog << Warning << "\n" << method_name
        << "No matching wind field(s) found for name(s) \""
        << write_css(search_names) << "\".\n"
        << "Set \"" << conf_key_name
        << "\" to specify the matching variable name.\n\n";
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


static StringArray swap_uv_name(VarInfo *vinfo,
                                const ConcatString &conf_key_name,
                                const StringArray &names)
{

   // If converting between U-wind and V-wind, try swapping U and V
   // Only add the search string if the substituion works
   ConcatString cs(vinfo->name());
   StringArray sa(names);

   // Replace U with V
   if(vinfo->is_u_wind() &&
      conf_key_name == conf_key_v_wind_field_name) {
      cs.replace("U", "V", false);
      cs.replace("u", "v", false);
      if(cs != vinfo->name()) sa.insert(0, cs.c_str());
   }
   // Replace V with U
   else if(vinfo->is_v_wind() &&
           conf_key_name == conf_key_u_wind_field_name) {
      cs.replace("V", "U", false);
      cs.replace("v", "u", false);
      if(cs != vinfo->name()) sa.insert(0, cs.c_str());
   }

   return sa;

}


////////////////////////////////////////////////////////////////////////
