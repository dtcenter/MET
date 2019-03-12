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

#include "data2d_nccf.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static const int error_code_no_error          = 0;
static const int error_code_no_time_dim       = 1;
static const int error_code_no_matching_times = 2;
static const int error_code_empty_times       = 3;

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

NcVarInfo *MetNcCFDataFile::find_first_data_var() {
   NcVarInfo *first_data_var = NULL;
   // Store the name of the first data variable
   for (int i = 0; i < _file->Nvars; ++i) {
      if (is_nc_unit_time(_file->Var[i].units_att) || 
          is_nc_unit_longitude(_file->Var[i].units_att) || 
          is_nc_unit_latitude(_file->Var[i].units_att) ||
          _file->get_time_var_info() == &_file->Var[i]
         ) continue;
     
      if (strcmp(_file->Var[i].name, nccf_lat_var_name) != 0 &&
          strcmp(_file->Var[i].name, nccf_lon_var_name) != 0) {
         first_data_var = &(_file->Var[i]);
         break;
      }
   }
   return first_data_var;
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

bool MetNcCFDataFile::data_plane(VarInfo &vinfo, DataPlane &plane, int offset)
{
  // Not sure why we do this

  VarInfoNcCF *vinfo_nc = (VarInfoNcCF *)&vinfo;
  static const string method_name
      = "MetNcCFDataFile::data_plane(VarInfo &, DataPlane &, int) -> ";

  // Initialize the data plane

  plane.clear();

  // Check for NA in the requested name
  if (strcmp(vinfo_nc->req_name(), na_str) == 0)
  {
    // Store the name of the first data variable
    NcVarInfo *data_var = find_first_data_var();
    if (NULL != data_var) vinfo_nc->set_req_name(data_var->name);
  }

  LongArray dimension = vinfo_nc->dimension();
  if (offset >= 0) {
    NcVarInfo *data_var = _file->find_var_name(vinfo_nc->req_name());
    if (NULL != data_var) {
      int time_dim_slot = data_var->t_slot;
      if (0 <= time_dim_slot) {
        int dim_offset = dimension[time_dim_slot];
        int dim_size = _file->ValidTime.n_elements();
        if ((dim_offset == range_flag) || (offset < dim_size))
           dimension[time_dim_slot] = offset;
      }
    }
  }

  // Read the data
  NcVarInfo *info = (NcVarInfo *) 0;

  bool status = _file->getData(vinfo_nc->req_name(),
                               dimension,
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

      mlog << Warning << "\n" << method_name
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

      mlog << Warning << "\n" << method_name
           << "for \"" << vinfo.req_name() << "\" variable, the lead "
           << "time does not match the requested lead time: ("
           << data_time_str << " != " << req_time_str << ")\n\n";
      status = false;
    }

    process_data_plane(&vinfo, plane);

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
   int n_rec = 0;
   bool status = false;
   int error_code = error_code_no_error;
   double time_lower = bad_data_double;
   double time_upper = bad_data_double;
   VarInfoNcCF *vinfo_nc = (VarInfoNcCF *)&vinfo;
   static const string method_name_short
         = "MetNcCFDataFile::data_plane_array() -> ";
   static const string method_name
         = "MetNcCFDataFile::data_plane_array(VarInfo &, DataPlaneArray &) -> ";

   // Initialize
   plane_array.clear();

   if (strcmp(vinfo_nc->req_name(), na_str) == 0) {
      // Store the name of the first data variable
      NcVarInfo *data_var = find_first_data_var();
      if (NULL != data_var) vinfo_nc->set_req_name(data_var->name);
   }

   DataPlane plane;
   LevelInfo level = vinfo.level();
   bool is_time_levels = (level.type() == LevelType_Time);
   LongArray dimension = vinfo_nc->dimension();
   NcVarInfo *info = _file->find_var_name(vinfo_nc->req_name());
   if (NULL == info) return(n_rec);
   
   int time_dim_slot = info->t_slot;
   int time_dim_size = _file->ValidTime.n_elements();
   if ((0 >= time_dim_size) || (0 > time_dim_slot)){
      if (is_time_levels) {
         error_code = error_code_no_time_dim;
      }
      else if(data_plane(vinfo, plane)) {
         plane_array.add(plane, bad_data_double, bad_data_double);
         n_rec = 1;
      }
   }
   else {
      int idx;
      LongArray time_offsets;
      TimeArray missing_times;
      int dim_offset = dimension[time_dim_slot];
      bool include_all_times = (dim_offset == vx_data2d_star);
      
      if (include_all_times) {
         for (idx=0; idx<time_dim_size; idx++) {
            time_offsets.add(idx);
         }
      }
      else if (is_time_levels) {
         int start_offset, end_offset;
         double time_inc = level.increment();
         
         start_offset = -1;
         time_lower = level.lower();
         time_upper = level.upper();
         for (idx=0; idx<time_dim_size; idx++) {
            if (_file->ValidTime[idx] < time_lower) continue;
            if (_file->ValidTime[idx] > time_lower) break;
            start_offset = idx;
            time_offsets.add(start_offset);
            break;
         }
         if (start_offset < 0) missing_times.add(time_lower);
             
         if (0 < time_inc) {
            long next_time;
            next_time = time_lower + time_inc;
            for (idx=(start_offset+1); idx<time_dim_size; idx++) {
               bool no_upper_time = false;
               if (_file->ValidTime[idx] > time_upper) break;
               else if (_file->ValidTime[idx] < next_time) continue;
               else if (_file->ValidTime[idx] == next_time) {
                  time_offsets.add(idx);
                  next_time += time_inc;
               }
               else {   // _file->ValidTime[idx] > next_time
                  while (next_time < _file->ValidTime[idx]) {
                     missing_times.add(next_time);
                     next_time += time_inc;
                  }
                  if (_file->ValidTime[idx] == next_time) {
                     time_offsets.add(idx);
                     next_time += time_inc;
                  }
               }
               if (next_time > time_upper) break;
            }
         }
         else {
            for (idx=(start_offset+1); idx<time_dim_size; idx++) {
               if (_file->ValidTime[idx] > time_upper) break;
               time_offsets.add(idx);
            }
         }
      }
      else if (dim_offset >= time_dim_size) {
         for (int idx=0; idx<_file->ValidTime.n_elements(); idx++) {
            if (_file->ValidTime[idx] == dim_offset) {
               time_offsets.add(idx);
               break;
            }
         }
      }
      
      int missing_count = missing_times.n_elements();
      if (0 < missing_count) {
         for (idx = 0; idx<missing_count; idx++) {
            mlog << Warning << method_name_short
                 << "The time \"" << unix_to_yyyymmdd_hhmmss(missing_times[idx])
                 << "\" does not exist.\n";
         }
      }
      
      if (0 < time_offsets.n_elements()) {
         for (idx=0; idx<time_offsets.n_elements(); idx++) {
            if (data_plane(vinfo, plane, time_offsets[idx])) {
               plane_array.add(plane, time_lower, time_upper);
               n_rec++;
            }
         }
      }
      else if (is_time_levels) error_code = error_code_no_matching_times;
      else if (include_all_times) error_code = error_code_empty_times;
   }
   
   // Handling error code
   if (error_code > error_code_no_error) {
      ConcatString log_msg;
      log_msg << "variable \"" << vinfo_nc->req_name() << "\" does not ";
      if (error_code == error_code_no_time_dim) {
         log_msg << "support the range of times because the time dimension is "
                 << time_dim_size;
      }
      else if (error_code == error_code_no_matching_times) {
         log_msg << "have the matching time ranges between "
                 << unix_to_yyyymmdd_hhmmss(time_lower) << " and "
                 << unix_to_yyyymmdd_hhmmss(time_upper);
         if (0 < _file->ValidTime.n_elements()) {
            log_msg << " from ["
                    << unix_to_yyyymmdd_hhmmss(_file->ValidTime.min()) << " and "
                    << unix_to_yyyymmdd_hhmmss(_file->ValidTime.max()) << "]";
         }
      }
      else if (error_code == error_code_empty_times) {
         log_msg << "have the time values.\n\n";
      }
      mlog << Error << "\n" << method_name << log_msg << ".\n\n";
      exit(1);
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
