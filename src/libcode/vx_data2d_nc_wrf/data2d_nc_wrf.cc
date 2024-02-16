// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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

#include "data2d_nc_wrf.h"
#include "get_wrf_grid.h"
#include "vx_nc_util.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class MetNcWrfDataFile
//
////////////////////////////////////////////////////////////////////////

MetNcWrfDataFile::MetNcWrfDataFile() {

    nc_wrf_init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

MetNcWrfDataFile::~MetNcWrfDataFile() {

   close();
}

////////////////////////////////////////////////////////////////////////

MetNcWrfDataFile::MetNcWrfDataFile(const MetNcWrfDataFile &) {

   mlog << Error << "\nMetNcWrfDataFile::MetNcWrfDataFile(const MetNcWrfDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

MetNcWrfDataFile & MetNcWrfDataFile::operator=(const MetNcWrfDataFile &) {

   mlog << Error << "\nMetNcWrfDataFile::operator=(const MetNcWrfDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

void MetNcWrfDataFile::nc_wrf_init_from_scratch() {

   WrfNc  = (WrfFile *) 0;

   close();

   return;
}

////////////////////////////////////////////////////////////////////////

void MetNcWrfDataFile::close() {

   if(WrfNc) { delete WrfNc; WrfNc = (WrfFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetNcWrfDataFile::open(const char * _filename) {

   close();

    WrfNc = new WrfFile;

   if(!WrfNc->open(_filename)) {
      mlog << Error << "\nMetNcWrfDataFile::open(const char *) -> "
           << "unable to open NetCDF file \"" << _filename << "\"\n\n";
      close();

      return(false);
   }

   Filename = _filename;

   Raw_Grid = new Grid;

   (*Raw_Grid) = WrfNc->grid;

   Dest_Grid = new Grid;

   (*Dest_Grid) = (*Raw_Grid);

   return(true);
}

////////////////////////////////////////////////////////////////////////

void MetNcWrfDataFile::dump(ostream & out, int depth) const {

   if(WrfNc) WrfNc->dump(out, depth);

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetNcWrfDataFile::data_plane(VarInfo &vinfo, DataPlane &plane) {
   bool status = false;
   double pressure;
   ConcatString level_str;
   VarInfoNcWrf * vinfo_nc = (VarInfoNcWrf *) &vinfo;
   NcVarInfo *info = (NcVarInfo *) 0;

   // Initialize the data plane
   plane.clear();

   // Read the data
   WrfNc->get_nc_var_info(vinfo_nc->req_name().c_str(), info);
   LongArray dimension = vinfo_nc->dimension();
   int dim_count = dimension.n_elements();
   for (int k=0; k<dim_count; k++) {
      if (dimension[k] == vx_data2d_dim_by_value) {
         string dim_name = GET_NC_NAME(get_nc_dim(info->var, k));
         NcVarInfo *var_info = find_var_info_by_dim_name(WrfNc->Var, dim_name,
                                                         WrfNc->Nvars);
         if (var_info) {
            long new_offset = get_index_at_nc_data(var_info->var,
                                                   vinfo_nc->dim_value(k),
                                                   dim_name, (k == info->t_slot));
            if (new_offset != bad_data_int) dimension[k] = new_offset;
         }
      }
   }

   status = WrfNc->data(vinfo_nc->req_name().c_str(),
                        dimension, plane, pressure, info);

   // Check that the times match those requested
   if(status) {

      // Check that the valid time matches the request
      if(vinfo.valid() > 0 && vinfo.valid() != plane.valid()) {

         mlog << Warning << "\nMetNcWrfDataFile::data_plane() -> "
              << "for \"" << vinfo.req_name() << "\" variable, the valid "
              << "time does not match the requested valid time: ("
              << unix_to_yyyymmdd_hhmmss(plane.valid()) << " != "
              << unix_to_yyyymmdd_hhmmss(vinfo.valid()) << ")\n\n";
         status = false;
      }

      // Check that the lead time matches the request
      if(vinfo.lead() > 0 && vinfo.lead() != plane.lead()) {

         mlog << Warning << "\nMetNcWrfDataFile::data_plane() -> "
              << "for \"" << vinfo.req_name() << "\" variable, the lead "
              << "time does not match the requested lead time: ("
              << sec_to_hhmmss(plane.lead()) << " != "
              << sec_to_hhmmss(vinfo.lead()) << ")\n\n";
         status = false;
      }

      status = process_data_plane(&vinfo, plane);

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

int MetNcWrfDataFile::data_plane_array(VarInfo &vinfo,
                                           DataPlaneArray &plane_array) {
   int i, i_dim, n_level, status, lower, upper;
   ConcatString level_str;
   double pressure, min_level, max_level;
   bool found = false;
   VarInfoNcWrf *vinfo_nc = (VarInfoNcWrf *) &vinfo;
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
      status = WrfNc->data(vinfo_nc->req_name().c_str(),
                           cur_dim, cur_plane, pressure, info);

      // Check that the times match those requested
      if(status) {

         // Check that the valid time matches the request
         if(vinfo.valid() > 0 && vinfo.valid() != cur_plane.valid()) {

            mlog << Warning << "\nMetNcWrfDataFile::data_plane_array() -> "
                 << "for \"" << vinfo.req_name() << "\" variable, the valid "
                 << "time does not match the requested valid time: ("
                 << unix_to_yyyymmdd_hhmmss(cur_plane.valid()) << " != "
                 << unix_to_yyyymmdd_hhmmss(vinfo.valid()) << ")\n\n";
            status = false;
         }

         // Check that the lead time matches the request
         if(vinfo.lead() > 0 && vinfo.lead() != cur_plane.lead()) {

            mlog << Warning << "\nMetNcWrfDataFile::data_plane_array() -> "
                 << "for \"" << vinfo.req_name() << "\" variable, the lead "
                 << "time does not match the requested lead time: ("
                 << sec_to_hhmmss(cur_plane.lead()) << " != "
                 << sec_to_hhmmss(vinfo.lead()) << ")\n\n";
            status = false;
         }

         status = process_data_plane(&vinfo, cur_plane);

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

int MetNcWrfDataFile::index(VarInfo &vinfo){
   return -1;
}

////////////////////////////////////////////////////////////////////////
