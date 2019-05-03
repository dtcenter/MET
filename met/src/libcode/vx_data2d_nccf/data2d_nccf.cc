// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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

#include "data2d_nccf.h"
#include "get_nccf_grid.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class MetNcCFDataFile
//
////////////////////////////////////////////////////////////////////////

MetNcCFDataFile::MetNcCFDataFile() {

   nccf_init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

MetNcCFDataFile::~MetNcCFDataFile() {

   close();
}

////////////////////////////////////////////////////////////////////////

MetNcCFDataFile::MetNcCFDataFile(const MetNcCFDataFile &) {

   mlog << Error << "\nMetNcCFDataFile::MetNcCFDataFile(const MetNcCFDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

MetNcCFDataFile & MetNcCFDataFile::operator=(const MetNcCFDataFile &) {

   mlog << Error << "\nMetNcCFDataFile::operator=(const MetNcCFDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void MetNcCFDataFile::nccf_init_from_scratch() {

   _file  = (NcCfFile *) 0;

   close();

   return;
}

////////////////////////////////////////////////////////////////////////

void MetNcCFDataFile::close() {

   if(_file) { delete _file; _file = (NcCfFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetNcCFDataFile::open(const char * _filename) {

   close();

   _file = new NcCfFile;

   if(!_file->open(_filename)) {
      mlog << Error << "\nMetNcCFDataFile::open(const char *) -> "
           << "unable to open NetCDF file \"" << _filename << "\"\n\n";
      close();

      return(false);
   }

   Filename = _filename;

   Raw_Grid = new Grid;

   (*Raw_Grid) = _file->grid;

   Dest_Grid = new Grid;

   (*Dest_Grid) = (*Raw_Grid);

   return(true);
}

////////////////////////////////////////////////////////////////////////

void MetNcCFDataFile::dump(ostream & out, int depth) const {

   if(_file) _file->dump(out, depth);

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetNcCFDataFile::data_plane(VarInfo &vinfo, DataPlane &plane)
{
  // Not sure why we do this

  VarInfoNcCF *vinfo_nc = (VarInfoNcCF *)&vinfo;

  // Initialize the data plane

  plane.clear();

  // Check for NA in the requested name

  if (strcmp(vinfo_nc->req_name(), na_str) == 0)
  {
    // Store the name of the first data variable

    for (int i = 0; i < _file->Nvars; ++i)
    {
      if (strcmp(_file->Var[i].name, nccf_lat_var_name) != 0 &&
          strcmp(_file->Var[i].name, nccf_lon_var_name) != 0)
      {
        vinfo_nc->set_req_name(_file->Var[i].name);
        break;
      }
    }
  }

  // Read the data

  NcVarInfo *info = (NcVarInfo *) 0;

  bool status = _file->getData(vinfo_nc->req_name(),
                               vinfo_nc->dimension(),
                               plane, info);

  // Check that the times match those requested

  if (status)
  {
    // Check that the valid time matches the request

    if (vinfo.valid() > 0 && vinfo.valid() != plane.valid())
    {
      // Compute time strings

      ConcatString req_time_str  = unix_to_yyyymmdd_hhmmss(vinfo.valid());
      ConcatString data_time_str = unix_to_yyyymmdd_hhmmss(plane.valid());

      mlog << Warning << "\nMetNcCFDataFile::data_plane() -> "
           << "for \"" << vinfo.req_name() << "\" variable, the valid "
           << "time does not match the requested valid time: ("
           << data_time_str << " != " << req_time_str << ")\n\n";
      status = false;
    }

    // Check that the lead time matches the request

    if (vinfo.lead() > 0 && vinfo.lead() != plane.lead())
    {
      // Compute time strings

      ConcatString req_time_str  = sec_to_hhmmss(vinfo.lead());
      ConcatString data_time_str = sec_to_hhmmss(plane.lead());

      mlog << Warning << "\nMetNcCFDataFile::data_plane() -> "
           << "for \"" << vinfo.req_name() << "\" variable, the lead "
           << "time does not match the requested lead time: ("
           << data_time_str << " != " << req_time_str << ")\n\n";
      status = false;
    }

    // Set the VarInfo object's name, long_name, level, and units strings

    if (info->name_att.length() > 0)
      vinfo.set_name(info->name_att);
    else
      vinfo.set_name(info->name);

    if (info->long_name_att.length() > 0)
      vinfo.set_long_name(info->long_name_att);

    if (info->level_att.length() > 0)
      vinfo.set_level_name(info->level_att);

    if (info->units_att.length() > 0)
      vinfo.set_units(info->units_att);

    //  print a report
    double plane_min, plane_max;
    plane.data_range(plane_min, plane_max);
    mlog << Debug(4) << "\n"
         << "Data plane information:\n"
         << "      plane min: " << plane_min << "\n"
         << "      plane max: " << plane_max << "\n"
         << "     valid time: " << unix_to_yyyymmdd_hhmmss(plane.valid()) << "\n"
         << "      lead time: " << sec_to_hhmmss(plane.lead()) << "\n"
         << "      init time: " << unix_to_yyyymmdd_hhmmss(plane.init()) << "\n"
         << "     accum time: " << sec_to_hhmmss(plane.accum()) << "\n";
  }



  return status;
}

////////////////////////////////////////////////////////////////////////

int MetNcCFDataFile::data_plane_array(VarInfo &vinfo,
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

int MetNcCFDataFile::index(VarInfo &vinfo){

   if( NULL == _file->find_var_name( vinfo.name() ) ) return -1;

   if( ( vinfo.valid() != 0         && _file->ValidTime[0] != vinfo.valid() ) ||
       ( vinfo.init()  != 0         && _file->InitTime     != vinfo.init()  ) ||
       ( !is_bad_data(vinfo.lead()) && _file->lead_time()  != vinfo.lead()  ) )
      return -1;

   return 0;
}
