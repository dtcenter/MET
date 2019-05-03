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

#include "data2d_nc_pinterp.h"
#include "get_pinterp_grid.h"
#include "vx_nc_util.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class MetNcPinterpDataFile
//
////////////////////////////////////////////////////////////////////////

MetNcPinterpDataFile::MetNcPinterpDataFile() {

   nc_pinterp_init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

MetNcPinterpDataFile::~MetNcPinterpDataFile() {

   close();
}

////////////////////////////////////////////////////////////////////////

MetNcPinterpDataFile::MetNcPinterpDataFile(const MetNcPinterpDataFile &) {

   mlog << Error << "\nMetNcPinterpDataFile::MetNcPinterpDataFile(const MetNcPinterpDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

MetNcPinterpDataFile & MetNcPinterpDataFile::operator=(const MetNcPinterpDataFile &) {

   mlog << Error << "\nMetNcPinterpDataFile::operator=(const MetNcPinterpDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void MetNcPinterpDataFile::nc_pinterp_init_from_scratch() {

   PinterpNc  = (PinterpFile *) 0;

   close();

   return;
}

////////////////////////////////////////////////////////////////////////

void MetNcPinterpDataFile::close() {

   if(PinterpNc) { delete PinterpNc; PinterpNc = (PinterpFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetNcPinterpDataFile::open(const char * _filename) {

   close();

   PinterpNc = new PinterpFile;

   if(!PinterpNc->open(_filename)) {
      mlog << Error << "\nMetNcPinterpDataFile::open(const char *) -> "
           << "unable to open NetCDF file \"" << _filename << "\"\n\n";
      close();

      return(false);
   }

   Filename = _filename;

   Raw_Grid = new Grid;

   (*Raw_Grid) = PinterpNc->grid;

   Dest_Grid = new Grid;

   (*Dest_Grid) = (*Raw_Grid);

   return(true);
}

////////////////////////////////////////////////////////////////////////

void MetNcPinterpDataFile::dump(ostream & out, int depth) const {

   if(PinterpNc) PinterpNc->dump(out, depth);

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetNcPinterpDataFile::data_plane(VarInfo &vinfo, DataPlane &plane) {
   bool status = false;
   double pressure;
   ConcatString level_str;
   VarInfoNcPinterp * vinfo_nc = (VarInfoNcPinterp *) &vinfo;
   NcVarInfo *info = (NcVarInfo *) 0;

   // Initialize the data plane
   plane.clear();

   // Read the data
   status = PinterpNc->data(vinfo_nc->req_name().c_str(),
                            vinfo_nc->dimension(),
                            plane, pressure, info);

   // Check that the times match those requested
   if(status) {

      // Check that the valid time matches the request
      if(vinfo.valid() > 0 && vinfo.valid() != plane.valid()) {

         mlog << Warning << "\nMetNcPinterpDataFile::data_plane() -> "
              << "for \"" << vinfo.req_name() << "\" variable, the valid "
              << "time does not match the requested valid time: ("
              << unix_to_yyyymmdd_hhmmss(plane.valid()) << " != "
              << unix_to_yyyymmdd_hhmmss(vinfo.valid()) << ")\n\n";
         status = false;
      }

      // Check that the lead time matches the request
      if(vinfo.lead() > 0 && vinfo.lead() != plane.lead()) {

         mlog << Warning << "\nMetNcPinterpDataFile::data_plane() -> "
              << "for \"" << vinfo.req_name() << "\" variable, the lead "
              << "time does not match the requested lead time: ("
              << sec_to_hhmmss(plane.lead()) << " != "
              << sec_to_hhmmss(vinfo.lead()) << ")\n\n";
         status = false;
      }

      process_data_plane(&vinfo, plane);

      // Set the VarInfo object's name, long_name, and units strings
      if(info->name_att.length()      > 0) vinfo.set_name(info->name_att);
      else                                 vinfo.set_name(info->name);
      if(info->long_name_att.length() > 0) vinfo.set_long_name(info->long_name_att.c_str());
      if(info->units_att.length()     > 0) vinfo.set_units(info->units_att.c_str());

      // Set the VarInfo object's level string for pressure levels
      if(!is_bad_data(pressure)) {
         level_str << cs_erase << "P" << nint(pressure);
         vinfo.set_level_name(level_str.c_str());
      }
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

int MetNcPinterpDataFile::data_plane_array(VarInfo &vinfo,
                                           DataPlaneArray &plane_array) {
   int i, i_dim, n_level, status, lower, upper;
   ConcatString level_str;
   double pressure, min_level, max_level;
   bool found = false;
   VarInfoNcPinterp *vinfo_nc = (VarInfoNcPinterp *) &vinfo;
   LongArray dim = vinfo_nc->dimension();
   NcVarInfo *info = (NcVarInfo *) 0;

   LongArray cur_dim;
   DataPlane cur_plane;

   // Initialize the data plane array
   plane_array.clear();

   // Find the dimension that has the range flag set
   for(i_dim=0; i_dim<dim.n_elements(); i_dim++) {
      if(dim[i_dim] == range_flag) {
         found = true;
         break;
      }
   }

   // If the range flag was not found, call data_plane() and return
   if( !found ){
      DataPlane plane;
      if( !data_plane(vinfo, plane) ) return 0;
      plane_array.add(plane, bad_data_float, bad_data_float);
      return 1;
   }

   // Compute the number of levels
   lower   = nint(vinfo.level().lower());
   upper   = nint(vinfo.level().upper());
   n_level = upper - lower + 1;

   // Loop through each of levels specified in the range
   cur_dim = dim;
   for(i=0; i<n_level; i++) {

      // Set the dimension for the current level
      cur_dim[i_dim] = lower + i;

      // Read data for the current level
      status = PinterpNc->data(vinfo_nc->req_name().c_str(),
                               cur_dim, cur_plane, pressure, info);

      // Check that the times match those requested
      if(status) {

         // Check that the valid time matches the request
         if(vinfo.valid() > 0 && vinfo.valid() != cur_plane.valid()) {

            mlog << Warning << "\nMetNcPinterpDataFile::data_plane_array() -> "
                 << "for \"" << vinfo.req_name() << "\" variable, the valid "
                 << "time does not match the requested valid time: ("
                 << unix_to_yyyymmdd_hhmmss(cur_plane.valid()) << " != "
                 << unix_to_yyyymmdd_hhmmss(vinfo.valid()) << ")\n\n";
            status = false;
         }

         // Check that the lead time matches the request
         if(vinfo.lead() > 0 && vinfo.lead() != cur_plane.lead()) {

            mlog << Warning << "\nMetNcPinterpDataFile::data_plane_array() -> "
                 << "for \"" << vinfo.req_name() << "\" variable, the lead "
                 << "time does not match the requested lead time: ("
                 << sec_to_hhmmss(cur_plane.lead()) << " != "
                 << sec_to_hhmmss(vinfo.lead()) << ")\n\n";
            status = false;
         }

         process_data_plane(&vinfo, cur_plane);

         // Add current plane to the data plane array
         plane_array.add(cur_plane, pressure, pressure);
      }

      // Check for bad status
      if(!status) return(0);

      // Add current plane to the data plane array
      plane_array.add(cur_plane, pressure, pressure);

      // Set the VarInfo object's name, long_name, and units strings
      if(i==0) {
         if(info->name_att.length()      > 0) vinfo.set_name(info->name_att);
         else                                 vinfo.set_name(info->name);
         if(info->long_name_att.length() > 0) vinfo.set_long_name(info->long_name_att.c_str());
         if(info->units_att.length()     > 0) vinfo.set_units(info->units_att.c_str());
      }

   } // end for i

   // Set the VarInfo object's level string for pressure levels
   // Check for a missing value, a single pressure level, or a range
   // of pressure levels
   plane_array.level_range(min_level, max_level);
   if(is_bad_data(min_level) ||
      is_bad_data(max_level))           level_str << cs_erase << na_str;
   else if(is_eq(min_level, max_level)) level_str << cs_erase << "P" << nint(min_level);
   else                                 level_str << cs_erase << "P" << nint(max_level)
                                                  << "-" << nint(min_level);
   vinfo.set_level_name(level_str.c_str());

   return(n_level);
}

////////////////////////////////////////////////////////////////////////

int MetNcPinterpDataFile::index(VarInfo &vinfo){
   return -1;
}
