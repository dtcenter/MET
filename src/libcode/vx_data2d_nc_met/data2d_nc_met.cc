// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
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
#include <netcdf>
#include <vector>

#include "data2d_nc_met.h"
#include "get_met_grid.h"
#include "nc_utils.h"
#include "vx_math.h"
#include "vx_log.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////
//
// Code for class MetNcMetDataFile
//
////////////////////////////////////////////////////////////////////////

MetNcMetDataFile::MetNcMetDataFile() {

   nc_met_init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

MetNcMetDataFile::~MetNcMetDataFile() {

   close();
}

////////////////////////////////////////////////////////////////////////

MetNcMetDataFile::MetNcMetDataFile(const MetNcMetDataFile &) {

   mlog << Error << "\nMetNcMetDataFile::MetNcMetDataFile(const MetNcMetDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

MetNcMetDataFile & MetNcMetDataFile::operator=(const MetNcMetDataFile &) {

   mlog << Error << "\nMetNcMetDataFile::operator=(const MetNcMetDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

void MetNcMetDataFile::nc_met_init_from_scratch() {

   MetNc  = (MetNcFile *) nullptr;

   close();

   return;
}

////////////////////////////////////////////////////////////////////////

void MetNcMetDataFile::close() {

   if(MetNc) { delete MetNc; MetNc = (MetNcFile *) nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetNcMetDataFile::open(const char * _filename) {

   close();

   MetNc = new MetNcFile;

   if(!MetNc->open(_filename)) {
      mlog << Error << "\nMetNcMetDataFile::open(const char *) -> "
           << "unable to open NetCDF file \"" << _filename << "\"\n\n";
      close();

      return false;
   }

   Filename = _filename;

   Raw_Grid = new Grid;

   *(Raw_Grid) = MetNc->grid;

   Dest_Grid = new Grid;

   (*Dest_Grid) = (*Raw_Grid);

   return true;
}

////////////////////////////////////////////////////////////////////////

void MetNcMetDataFile::dump(ostream & out, int depth) const {

   if(MetNc) MetNc->dump(out, depth);

   return;
}

////////////////////////////////////////////////////////////////////////

void MetNcMetDataFile::set_range_azimuth_grid_center(int i_track_point) {

   if(!MetNc->is_range_azimuth() || i_track_point < 0) return;

   // Get current RngAziData object
   RngAziData d = *(MetNc->grid.info().ra);
   d.lat_center = bad_data_double;
   d.lon_center = bad_data_double;

   vector<size_t> start(1, i_track_point);
   vector<size_t> count(1, 1);

   // FullTrackLat and FullTrackLon variables
   if(has_var(MetNc->Nc, "FullTrackLat") &&
      has_var(MetNc->Nc, "FullTrackLon")) {
      NcVar var_lat = get_nc_var(MetNc->Nc, "FullTrackLat");
      var_lat.getVar(start, count, &d.lat_center);
      NcVar var_lon = get_nc_var(MetNc->Nc, "FullTrackLon");
      var_lon.getVar(start, count, &d.lon_center);
      d.lon_center *= -1.0;
   }
   // TrackLat and TrackLon variables
   else if(has_var(MetNc->Nc, "TrackLat") &&
           has_var(MetNc->Nc, "TrackLon")) {
      NcVar var_lat = get_nc_var(MetNc->Nc, "TrackLat");
      var_lat.getVar(start, count, &d.lat_center);
      NcVar var_lon = get_nc_var(MetNc->Nc, "TrackLon");
      var_lon.getVar(start, count, &d.lon_center);
      d.lon_center *= -1.0;
   }

   // Reset the range/azimuth grid
   if(!is_bad_data(d.lat_center) &&
      !is_bad_data(d.lon_center)) {
      MetNc->grid.set(d);
      set_grid(MetNc->grid);
   }
}

////////////////////////////////////////////////////////////////////////

void MetNcMetDataFile::set_range_azimuth_times(int i_track_point, DataPlane &plane) {

   if(!MetNc->is_range_azimuth() || i_track_point < 0) return;

   string ymd_hms_str("19700101_000000");
   vector<size_t> start(1, i_track_point);
   vector<size_t> count(1, 1);

   // Initialization time
   NcVar var_init = get_nc_var(MetNc->Nc, "init_time");
   var_init.getVar(&ymd_hms_str);
   plane.set_init(timestring_to_unix(ymd_hms_str.c_str()));

   // Valid time
   NcVar var_valid = get_nc_var(MetNc->Nc, "valid_time");
   var_valid.getVar(start, count, &ymd_hms_str);
   plane.set_valid(timestring_to_unix(ymd_hms_str.c_str()));

   // Lead time
   int lead_sec;
   NcVar var_lead = get_nc_var(MetNc->Nc, "lead_time_sec");
   var_lead.getVar(start, count, &lead_sec);
   plane.set_lead(lead_sec);
}

////////////////////////////////////////////////////////////////////////

bool MetNcMetDataFile::data_plane(VarInfo &vinfo, DataPlane &plane) {
   bool status = false;
   ConcatString req_time_str, data_time_str;
   VarInfoNcMet * vinfo_nc = (VarInfoNcMet *) &vinfo;
   NcVarInfo *info = (NcVarInfo *) nullptr;
   int i;

   // Initialize the data plane
   plane.clear();

   // Check for NA in the requested name
   if(vinfo_nc->req_name() == na_str) {

      // Store the name of the first data variable
      for(i=0; i<MetNc->Nvars; i++) {
         if( MetNc->Var[i].name != nc_met_lat_name &&
             MetNc->Var[i].name != nc_met_lon_name ) {
            vinfo_nc->set_req_name(MetNc->Var[i].name.c_str());
            break;
         }
      }
   }

   // Read the data
   status = MetNc->data(vinfo_nc->req_name().c_str(),
                        vinfo_nc->dimension(),
                        plane, info);

   // Check that the times match those requested
   if(status) {

      // Update the range/azimuth times and grid location, if possible 
      if(MetNc->is_range_azimuth() && info->t_slot >= 0) {
         auto i_track_point = (int) vinfo_nc->dimension()[info->t_slot];
         set_range_azimuth_grid_center(i_track_point);
         set_range_azimuth_times(i_track_point, plane);
      } 

      // Check that the valid time matches the request
      if(vinfo.valid() > 0 && vinfo.valid() != plane.valid()) {

         // Compute time strings
         req_time_str  = unix_to_yyyymmdd_hhmmss(vinfo.valid());
         data_time_str = unix_to_yyyymmdd_hhmmss(plane.valid());

         mlog << Warning << "\nMetNcMetDataFile::data_plane() -> "
              << "for \"" << vinfo.req_name() << "\" variable, the valid "
              << "time does not match the requested valid time: ("
              << data_time_str << " != " << req_time_str << ")\n\n";
         status = false;
      }

      // Check that the lead time matches the request
      if(vinfo.lead() > 0 && vinfo.lead() != plane.lead()) {

         // Compute time strings
         req_time_str  = sec_to_hhmmss(vinfo.lead());
         data_time_str = sec_to_hhmmss(plane.lead());

         mlog << Warning << "\nMetNcMetDataFile::data_plane() -> "
              << "for \"" << vinfo.req_name() << "\" variable, the lead "
              << "time does not match the requested lead time: ("
              << data_time_str << " != " << req_time_str << ")\n\n";
         status = false;
      }

      status = process_data_plane(&vinfo, plane);

      // Set the VarInfo object's name, long_name, level, and units strings
      if(info->name_att.length()      > 0) vinfo.set_name(info->name_att);
      else                                 vinfo.set_name(info->name);
      if(info->long_name_att.length() > 0) vinfo.set_long_name(info->long_name_att.c_str());
      if(info->level_att.length()     > 0) vinfo.set_level_name(info->level_att.c_str());
      if(info->units_att.length()     > 0) vinfo.set_units(info->units_att.c_str());
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

int MetNcMetDataFile::data_plane_array(VarInfo &vinfo,
                                       DataPlaneArray &plane_array) {
   bool status = false;
   int n_rec = 0;
   DataPlane plane;

   // Initialize
   plane_array.clear();

   // Can only read a single 2D data plane from a MET NetCDF file
   status = data_plane(vinfo, plane);

   // Add the data plane to the DataPlaneArray with no level values
   if(status) {
      plane_array.add(plane, bad_data_double, bad_data_double);
      n_rec = 1;
   }

   return n_rec;
}

////////////////////////////////////////////////////////////////////////

int MetNcMetDataFile::index(VarInfo &vinfo) {

   NcVarInfo *ncinfo = MetNc->find_var_name( vinfo.name().c_str() );

   if( !ncinfo ) return -1;

   if( ( vinfo.valid() != 0         && ncinfo->ValidTime   != vinfo.valid() ) ||
       ( vinfo.init() != 0          && ncinfo->InitTime    != vinfo.init()  ) ||
       ( !is_bad_data(vinfo.lead()) && ncinfo->lead_time() != vinfo.lead()  ) )
      return -1;

   return 0;
}

////////////////////////////////////////////////////////////////////////
