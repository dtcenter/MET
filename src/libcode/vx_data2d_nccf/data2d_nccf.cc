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

static const int error_code_no_error                = 0;
static const int error_code_no_time_dim             = 1;
static const int error_code_missing_time_value      = 2;
static const int error_code_missing_time_values     = 3;
static const int error_code_missing_time_offsets    = 4;
static const int error_code_empty_times             = 5;
static const int error_code_bad_increment           = 6;
static const int error_code_bad_offset              = 7;
static const int error_code_unknown                 = 999;

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
   _time_dim_offset = -1;

   close();

   return;
}

////////////////////////////////////////////////////////////////////////

NcVarInfo *MetNcCFDataFile::find_first_data_var() {
   NcVarInfo *first_data_var = NULL;
   // Store the name of the first data variable
   for (int i = 0; i < _file->Nvars; ++i) {
      if (is_nc_unit_time(_file->Var[i].units_att.c_str()) || 
          is_nc_unit_longitude(_file->Var[i].units_att.c_str()) || 
          is_nc_unit_latitude(_file->Var[i].units_att.c_str()) ||
          _file->get_time_var_info() == &_file->Var[i]
         ) continue;
     
      if (strcmp(_file->Var[i].name.c_str(), nccf_lat_var_name) != 0 &&
          strcmp(_file->Var[i].name.c_str(), nccf_lon_var_name) != 0) {
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

bool MetNcCFDataFile::data_plane(VarInfo &vinfo, DataPlane &plane)
{
  // Not sure why we do this

  NcVarInfo *data_var = (NcVarInfo *)NULL;
  VarInfoNcCF *vinfo_nc = (VarInfoNcCF *)&vinfo;
  static const string method_name
      = "MetNcCFDataFile::data_plane(VarInfo &, DataPlane &) -> ";

  // Initialize the data plane

  plane.clear();

  // Check for NA in the requested name
  if ( vinfo_nc->req_name() == na_str )
  {
    // Store the name of the first data variable
    data_var = find_first_data_var();
    if (NULL != data_var) vinfo_nc->set_req_name(data_var->name.c_str());
  }

  LongArray dimension = vinfo_nc->dimension();

  long time_cnt = (long)_file->ValidTime.n_elements();
  data_var = _file->find_var_name(vinfo_nc->req_name().c_str());
  if (NULL != data_var) {
    int time_dim_slot = data_var->t_slot;
    if (0 <= time_dim_slot) {
      long time_offset = dimension[time_dim_slot];
      bool time_as_value = !vinfo_nc->level().is_as_offset();
      if (time_as_value || (time_offset == range_flag) || (time_offset == vx_data2d_star)) {
        time_offset = _time_dim_offset;
        if (time_as_value && time_as_value >= time_cnt)
          time_offset = get_time_offset(_time_dim_offset);
        if ((0 <= time_offset) && (time_offset < time_cnt))
          dimension[time_dim_slot] = time_offset;
        else {
          mlog << Warning << "\n" << method_name << "the requested time "
               << unix_to_yyyymmdd_hhmmss(dimension[time_dim_slot])
               << " for \"" << vinfo.req_name()
               << "\" variable does not exist.\n\n";
          return false;
        }
      }
      else if (time_offset >= time_cnt) {
        mlog << Warning << "\n" << method_name << "the requested time offset "
             << time_offset << " for \"" << vinfo.req_name()
             << "\" variable is out of range (should be less than " << time_cnt << ").\n\n";
        return false;
      }
    }
  }

  // Read the data
  NcVarInfo *info = (NcVarInfo *) 0;

  bool status = _file->getData(vinfo_nc->req_name().c_str(),
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
      vinfo.set_long_name(info->long_name_att.c_str());

    if (info->level_att.length() > 0)
      vinfo.set_level_name(info->level_att.c_str());

    if (info->units_att.length() > 0)
      vinfo.set_units(info->units_att.c_str());

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
   VarInfoNcCF *vinfo_nc = (VarInfoNcCF *)&vinfo;
   static const string method_name_short
         = "MetNcCFDataFile::data_plane_array() -> ";
   static const string method_name
         = "MetNcCFDataFile::data_plane_array(VarInfo &, DataPlaneArray &) -> ";

   // Initialize
   plane_array.clear();

   if ( vinfo_nc->req_name() == na_str ) {
      // Store the name of the first data variable
      NcVarInfo *data_var = find_first_data_var();
      if (NULL != data_var) vinfo_nc->set_req_name(data_var->name.c_str());
   }

   NcVarInfo *info = _file->find_var_name(vinfo_nc->req_name().c_str());
   if (NULL == info) return(n_rec);
   
   DataPlane plane;
   bool not_found = true;
   double time_lower = bad_data_double;
   double time_upper = bad_data_double;
   int error_code = error_code_no_error;
   int time_dim_slot = info->t_slot;
   int time_dim_size = _file->ValidTime.n_elements();
   LevelInfo level = vinfo.level();
   LongArray dimension = vinfo_nc->dimension();
   bool is_time_range = (level.type() == LevelType_Time);
   bool time_as_value = !level.is_as_offset();
   
   long dim_offset = (time_dim_slot >= 0) ? dimension[time_dim_slot] : -1;
   long time_value = (time_as_value ? dim_offset : -1);
   bool include_all_times = (dim_offset == vx_data2d_star);

   if ((0 >= time_dim_size) || (0 > time_dim_slot)
       || (!is_time_range && !include_all_times)) {
      // Does not have time dimension OR range was not given
      if ((0 < time_dim_size) && (0 <= time_dim_slot)) {
         _time_dim_offset = -1; //reset _time_dim_offset
         if (time_as_value) {
            // Contains single yyyymmdd instead of time offset.
            error_code = error_code_missing_time_value;
            for (int idx=0; idx<time_dim_size; idx++) {
               if (_file->ValidTime[idx] == time_value) {
                  _time_dim_offset = idx;
                  error_code = error_code_no_error;
                  break;
               }
            }
         }
         else if (dim_offset >= time_dim_size || dim_offset < 0) {       
            error_code = error_code_bad_offset;
         }
      }
      if (error_code == error_code_no_error) {
         if (data_plane(vinfo, plane)) {
            //error_code = error_code_no_error;
            plane_array.add(plane, bad_data_double, bad_data_double);
            n_rec = 1;
         }
      }
   }
   else {   // has time dim and requested range of times
      int idx;
      LongArray time_offsets;
      TimeArray missing_times;
      if (include_all_times) {
         for (idx=0; idx<time_dim_size; idx++) {
            time_offsets.add(idx);
         }
      }
      else if (is_time_range) {
         time_lower = level.lower();
         time_upper = level.upper();
         int next_offset = -1;
         double time_inc = level.increment();
         if (time_as_value) {
            // Find the start offset.
            for (idx=0; idx<time_dim_size; idx++) {
               if (_file->ValidTime[idx] < time_lower) continue;
               next_offset = idx;
               if (_file->ValidTime[idx] == time_lower) {
                  time_offsets.add(idx);
                  next_offset++;
                  //not_found = false;
               }
               break;
            }
            //if (not_found) missing_times.add(time_lower);
            
            if (0 > next_offset) error_code = error_code_missing_time_values;
            else if (0 > time_inc) error_code = error_code_bad_increment;
            else if (0 == time_inc) {   // no increment configuration
               for (idx=next_offset; idx<time_dim_size; idx++) {
                  if (_file->ValidTime[idx] > time_upper) break;
                  time_offsets.add(idx);
               }
            }
            else {
               long next_time;
               next_time = time_lower + time_inc;
               for (idx=next_offset; idx<time_dim_size; idx++) {
                  if (_file->ValidTime[idx] > time_upper) break;
                  else if (_file->ValidTime[idx] < next_time) continue;
                  else if (_file->ValidTime[idx] == next_time) {
                     time_offsets.add(idx);
                     next_time += time_inc;
                  }
                  else { // next_time < _file->ValidTime[idx]
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
         }
         else if (time_lower < time_dim_size) {
            int inc_offset = (time_inc <= 0) ? 1 : time_inc;
            int max_time_offset = time_upper;
            if (max_time_offset == time_dim_size) missing_times.add(time_dim_size);
            else if (max_time_offset > time_dim_size) {
               for (idx=time_dim_size; idx<=max_time_offset; idx++)
                  missing_times.add(idx);
            }
            for (idx=time_lower; idx<max_time_offset; idx+=inc_offset) {
               time_offsets.add(idx);
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
      }
      //else if (dim_offset >= time_dim_size || (dim_offset < 0)) {
      //   mlog << Warning << method_name_short
      //        << "The time offset is out of range (0 <= offset < "
      //        << time_dim_size << ").\n";
      //}
      else error_code = error_code_unknown;

      if (0 < time_offsets.n_elements()) {
         for (idx=0; idx<time_offsets.n_elements(); idx++) {
            _time_dim_offset = time_offsets[idx];
            if (data_plane(vinfo, plane)) {
               plane_array.add(plane, time_lower, time_upper);
               n_rec++;
            }
         }
      }
      else if (is_time_range) {
          error_code = time_as_value ? error_code_missing_time_values
                                     : error_code_missing_time_offsets;
      }
      else if (include_all_times) error_code = error_code_empty_times;
   }
   
   // Handling error code
   if (error_code > error_code_no_error) {
      ConcatString log_msg;
      log_msg << "variable \"" << vinfo_nc->req_name() << "\" ";
      if (error_code == error_code_no_time_dim) {
         log_msg << "does not support the range of times because the time dimension is "
                 << time_dim_size;
      }
      else if (error_code == error_code_missing_time_values) {
         log_msg << "does not have the matching time ranges between "
                 << unix_to_yyyymmdd_hhmmss(time_lower) << " and "
                 << unix_to_yyyymmdd_hhmmss(time_upper);
         if (0 < _file->ValidTime.n_elements()) {
            log_msg << " from ["
                    << unix_to_yyyymmdd_hhmmss(_file->ValidTime.min()) << " and "
                    << unix_to_yyyymmdd_hhmmss(_file->ValidTime.max()) << "]";
         }
      }
      else if (error_code == error_code_missing_time_offsets) {
         log_msg << "does not have the matching time offsets between "
                 << nint(time_lower) << " and " << nint(time_upper);
         if (0 < _file->ValidTime.n_elements()) {
            log_msg << " [0 <= offset < " << _file->ValidTime.n_elements() << "]";
         }
      }
      else if (error_code == error_code_empty_times) {
         log_msg << "does not have the time values";
      }
      else if (error_code == error_code_bad_increment) {
         log_msg << "was configured with bad increment";
      }
      else if (error_code == error_code_bad_offset) {
         log_msg << "was configured with the bad time offset"
                 << " (0 <= offset < " << time_dim_size << ")";
      }
      else if (error_code == error_code_missing_time_value) {
         log_msg << "does not have the matching time "
                 << unix_to_yyyymmdd_hhmmss(time_value) << " ["
                 << unix_to_yyyymmdd_hhmmss(_file->ValidTime.min()) << " and "
                 << unix_to_yyyymmdd_hhmmss(_file->ValidTime.max()) << "]";
      }
      else {
         log_msg.clear();
         log_msg << "variable \"" << vinfo_nc->req_name()
                 << "\" has unknown error";
      }
      mlog << Error << "\n" << method_name << log_msg << ".\n\n";
      exit(1);
   }

   return(n_rec);
}

////////////////////////////////////////////////////////////////////////

int MetNcCFDataFile::index(VarInfo &vinfo){

   if( NULL == _file->find_var_name( vinfo.name().c_str() ) ) return -1;

   if( ( vinfo.valid() != 0         && _file->ValidTime[0] != vinfo.valid() ) ||
       ( vinfo.init()  != 0         && _file->InitTime     != vinfo.init()  ) ||
       ( !is_bad_data(vinfo.lead()) && _file->lead_time()  != vinfo.lead()  ) )
      return -1;

   return 0;
}

////////////////////////////////////////////////////////////////////////

long MetNcCFDataFile::get_time_offset(long time_dim_value) {
   long cur_time_offset = time_dim_value;
   int dim_size = _file->ValidTime.n_elements();
   if (time_dim_value >= dim_size) {
      for (int idx=0; idx<dim_size; idx++) {
         if (_file->ValidTime[idx] == time_dim_value) {
            cur_time_offset = idx;
            break;
         }
      }
   }
   return cur_time_offset;
}

////////////////////////////////////////////////////////////////////////
