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

#include "data2d_nc_met.h"
#include "get_met_grid.h"
#include "vx_math.h"
#include "vx_log.h"

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

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void MetNcMetDataFile::nc_met_init_from_scratch() {

   MetNc  = (MetNcFile *) 0;

   close();

   return;
}

////////////////////////////////////////////////////////////////////////

void MetNcMetDataFile::close() {

   if(MetNc) { delete MetNc; MetNc = (MetNcFile *) 0; }

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

      return(false);
   }

   Filename = _filename;

   Raw_Grid = new Grid;

   *(Raw_Grid) = MetNc->grid;

   Dest_Grid = new Grid;

   (*Dest_Grid) = (*Raw_Grid);

   return(true);
}

////////////////////////////////////////////////////////////////////////

void MetNcMetDataFile::dump(ostream & out, int depth) const {

   if(MetNc) MetNc->dump(out, depth);

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetNcMetDataFile::data_plane(VarInfo &vinfo, DataPlane &plane)

{

   bool status = false;
   ConcatString req_time_str, data_time_str;
   VarInfoNcMet * vinfo_nc = (VarInfoNcMet *) &vinfo;
   NcVarInfo *info = (NcVarInfo *) 0;
   int i;

   // Initialize the data plane
   plane.clear();

   // Check for NA in the requested name
   if( vinfo_nc->req_name() ==  na_str ) {

      // Store the name of the first data variable
      for(i=0; i<MetNc->Nvars; i++) {
         if( MetNc->Var[i].name != nc_met_lat_var_name &&
             MetNc->Var[i].name != nc_met_lon_var_name ) {
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

      process_data_plane(&vinfo, plane);

      // Set the VarInfo object's name, long_name, level, and units strings
      if(info->name_att.length()      > 0) vinfo.set_name(info->name_att);
      else                                 vinfo.set_name(info->name);
      if(info->long_name_att.length() > 0) vinfo.set_long_name(info->long_name_att.c_str());
      if(info->level_att.length()     > 0) vinfo.set_level_name(info->level_att.c_str());
      if(info->units_att.length()     > 0) vinfo.set_units(info->units_att.c_str());
   }

   return(status);
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

   return(n_rec);
}

////////////////////////////////////////////////////////////////////////

int MetNcMetDataFile::index(VarInfo &vinfo){

   if( NULL == MetNc->find_var_name( vinfo.name().c_str() ) ) return -1;

   if( ( vinfo.valid()              && MetNc->ValidTime   != vinfo.valid() ) ||
       ( vinfo.init()               && MetNc->InitTime    != vinfo.init()  ) ||
       ( !is_bad_data(vinfo.lead()) && MetNc->lead_time() != vinfo.lead()  ) )
      return -1;

   return 0;
}
