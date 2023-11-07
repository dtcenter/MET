// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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

#include "data2d_ugrid.h"
#include "vx_math.h"
#include "vx_log.h"

using namespace std;
using namespace netCDF;

////////////////////////////////////////////////////////////////////////

static const int error_code_no_error                = 0;
static const int error_code_no_time_dim             = 1;
static const int error_code_missing_time_value      = 2;
static const int error_code_missing_time_values     = 3;
static const int error_code_missing_time_offsets    = 4;
static const int error_code_empty_times             = 5;
static const int error_code_bad_increment           = 6;
static const int error_code_bad_offset              = 7;
static const int error_code_no_vert_dim             = 11;
static const int error_code_missing_vert_value      = 12;
static const int error_code_missing_vert_values     = 13;
static const int error_code_missing_vert_offsets    = 14;
static const int error_code_empty_verticals         = 15;
static const int error_code_unknown                 = 999;

////////////////////////////////////////////////////////////////////////
//
// Code for class MetUGridDataFile
//
////////////////////////////////////////////////////////////////////////

MetUGridDataFile::MetUGridDataFile() {

   ugrid_init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

MetUGridDataFile::~MetUGridDataFile() {

   close();
}

////////////////////////////////////////////////////////////////////////

MetUGridDataFile::MetUGridDataFile(const MetUGridDataFile &) {

   mlog << Error << "\nMetUGridDataFile::MetUGridDataFile(const MetUGridDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

MetUGridDataFile & MetUGridDataFile::operator=(const MetUGridDataFile &) {

   mlog << Error << "\nMetUGridDataFile::operator=(const MetUGridDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

void MetUGridDataFile::ugrid_init_from_scratch() {

   _file = (UGridFile *) nullptr;
   _cur_time_index = -1;
   _cur_vert_index = -1;

   close();

   return;
}

////////////////////////////////////////////////////////////////////////

void MetUGridDataFile::close() {

   if(_file) { delete _file; _file = (UGridFile *) nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetUGridDataFile::open(const char * _filename) {

   close();

   _file = new UGridFile;

   if(!_file->open(_filename)) {
      mlog << Error << "\nMetUGridDataFile::open(const char *) -> "
           << "unable to open NetCDF file \"" << _filename << "\"\n\n";
      close();

      return false;
   }

   Filename = _filename;

   return true;
}

////////////////////////////////////////////////////////////////////////

bool MetUGridDataFile::open_metadata(const char * _filename) {
   const char *method_name = "MetUGridDataFile::open_metadata(const char *) -> ";

   if (! _file) {
      mlog << Error << "\n" << method_name
           << "UGridFile is not initiated to handle \"" << _filename << "\"\n\n";
      close();
      return false;
   }

   if (!_file->open_metadata(_filename)) {
      mlog << Error << "\n" << method_name
           << "unable to open NetCDF file \"" << _filename << "\"\n\n";
      close();
      return false ;
   }

   meta_filename = _filename;

   Raw_Grid = new Grid;

   (*Raw_Grid) = _file->grid;
   //Raw_Grid->set(*_file->grid.info().us);

   Dest_Grid = new Grid;

   (*Dest_Grid) = (*Raw_Grid);
   //Dest_Grid->set(*_file->grid.info().us);

   return(true);
}

////////////////////////////////////////////////////////////////////////

void MetUGridDataFile::dump(ostream & out, int depth) const {

   if(_file) _file->dump(out, depth);

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetUGridDataFile::data_plane(VarInfo &vinfo, DataPlane &plane)
{
   bool status = false;
   NcVarInfo *data_var = (NcVarInfo *)nullptr;
   VarInfoUGrid *vinfo_nc = (VarInfoUGrid *)&vinfo;
   const long time_cnt = (long)_file->ValidTime.n();
   static const string method_name
      = "MetUGridDataFile::data_plane(VarInfo &, DataPlane &) -> ";

   // Initialize the data plane

   plane.clear();

   data_var = _file->find_by_name(vinfo_nc->req_name().c_str());
   if (nullptr != data_var) {
      // Read the data
      status = data_plane(vinfo, plane, data_var);
   }
   else {
      LevelInfo level = vinfo.level();
      long lvl_lower = level.lower();
      long lvl_upper = level.upper();
      vector<NcVarInfo *> vinfo_list;
      if (_file->find_nc_vinfo_list(vinfo_nc->req_name().c_str(), vinfo_list)) {
         for (int idx=0; idx<vinfo_list.size(); idx++) {
            int vlevel = extract_vlevels(vinfo_nc->req_name(), vinfo_list[idx]->name.c_str());
            if (vlevel >= lvl_lower && vlevel <= lvl_upper) {
               vinfo.set_req_name(vinfo_list[idx]->name.c_str());
               status = data_plane(vinfo, plane, vinfo_list[idx]);
               if (status) {
                  mlog << Debug(5) << method_name
                       << "Found range match for VarInfo \"" << vinfo_nc->req_name()
                       << " (" << vinfo_list[idx]->name << ")\"\n";
                  break;
               }
            }
         }
      }
   }

   return status;
}


////////////////////////////////////////////////////////////////////////
// Find the actual variable from the requested variuable name.
// The requested variable name is the prefix of the actual variable name.
//

bool MetUGridDataFile::data_plane(VarInfo &vinfo, DataPlane &plane, NcVarInfo *data_var)
{
   // Not sure why we do this
   bool status = false;
   const long time_cnt = (long)_file->ValidTime.n();
   VarInfoUGrid *vinfo_nc = (VarInfoUGrid *)&vinfo;
   static const string method_name
         = "MetUGridDataFile::data_plane(VarInfo &, DataPlane &, NcVarInfo *) -> ";

   // Initialize the data plane

   plane.clear();

   int zdim_slot = bad_data_int;
   int time_dim_slot = bad_data_int;
   NumArray dim_value = vinfo_nc->dim_value();
   LongArray dimension(vinfo_nc->dimension());
   BoolArray is_offset(vinfo_nc->is_offset());

   if (nullptr != data_var) {
      zdim_slot = data_var->z_slot;
      time_dim_slot = data_var->t_slot;

      // Adjust dimension and is_offset
      int dim_size = dimension.n_elements();
      if (0 == dimension.n_elements()) {
         for (int idx=0; idx<data_var->Ndims; idx++) {
            long offset = 0;
            if (zdim_slot == idx &&_cur_vert_index >= 0) offset = _cur_vert_index;
            dimension.add(offset);
            is_offset.add(true);
         }
      }
      for (int idx=0; idx<is_offset.n(); idx++) {
         long dim_offset = dimension[idx];
         if (dim_offset == vx_data2d_star) {
            if (idx == time_dim_slot || idx == zdim_slot) dimension[idx] = 0;  // force offset to 0
            continue;
         }
         if (idx == time_dim_slot) {
            long time_offset = dim_offset;
            if (time_offset == range_flag) time_offset = _cur_time_index;   // from data_plane_array()
            else if (!is_offset[idx]) {
               time_offset = get_time_offset(dim_value[idx], time_cnt,
                                             vinfo.req_name().c_str(), method_name);
            }
            else if ((0 > time_offset) || (time_offset >= time_cnt)) {
               time_offset = convert_time_to_offset(time_offset);
            }
            if ((0 > time_offset) || (time_offset >= time_cnt)) {
               mlog << Error << "\n" << method_name << "the requested time offset "
                    << time_offset << " for \"" << vinfo.req_name() << "\" variable "
                    << "is out of range (between 0 and " << (time_cnt-1) << ").\n\n";
               return false;
            }
            dimension[time_dim_slot] = time_offset;
         }
         else if (idx == zdim_slot) {
            long z_cnt = (long)get_dim_size(_file->get_vert_dim());
            long z_offset = dim_offset;
            if (z_offset == range_flag) z_offset = _cur_vert_index;   // from data_plane_array()
            //else if (!is_offset[idx]) {
            //  // convert the value to index for slicing
            //  z_offset = convert_value_to_offset(dim_value[idx], z_dim_name);
            //}
            if ((z_offset >= 0) && (z_offset < z_cnt)) {
              dimension[idx] = long(z_offset);
            }
         }
      }

      // Read the data
      status = read_data_plane(data_var->name, vinfo, plane, dimension);
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

int MetUGridDataFile::data_plane_array(VarInfo &vinfo,
                                       DataPlaneArray &plane_array) {
   int n_rec = 0;
   DataPlane plane;
   bool status = false;
   static const string method_name
         = "MetUGridDataFile::data_plane_array(VarInfo &, DataPlaneArray &) -> ";

   // Initialize
   plane_array.clear();

   LevelInfo level = vinfo.level();
   long lvl_lower = level.lower();
   long lvl_upper = level.upper();
   const int debug_level = 7;
   ConcatString req_name = vinfo.req_name();
   NcVarInfo *data_vinfo  = _file->find_by_name(req_name.c_str());
   if (level.type() == LevelType_Time) {
      mlog << Error << "\n" << method_name
           << "LevelType_Time for unstructured grid is not enabled\n\n";
      exit(1);
      /* Not enabled
      LongArray time_offsets = collect_time_offsets(vinfo);
      if (0 < time_offsets.n_elements()) {
         for (int idx=0; idx<time_offsets.n_elements(); idx++) {
            _cur_time_index = time_offsets[idx];
            if (data_plane(vinfo, plane)) {
               plane_array.add(plane, lvl_lower, lvl_upper);
               n_rec++;
            }
         }

         if (mlog.verbosity_level() >= debug_level) {
            for (int idx=0; idx< time_offsets.n_elements(); idx++ ) {
               mlog << Debug(debug_level) << method_name << "time: "
                    << unix_to_yyyymmdd_hhmmss(_file->ValidTime[time_offsets[idx]])
                    << " from index " << time_offsets[idx] << "\n";
            }
         }
      }
      */
   }
   else if (level.type() == LevelType_Pres) {
      if (nullptr == data_vinfo) {
         vector<NcVarInfo *> vinfo_list;
         if (_file->find_nc_vinfo_list(req_name.c_str(), vinfo_list)) {
            for (int idx=0; idx<vinfo_list.size(); idx++) {
               int vlevel = extract_vlevels(req_name, vinfo_list[idx]->name.c_str());
               if (vlevel >= lvl_lower && vlevel <= lvl_upper) {
                  vinfo.set_req_name(vinfo_list[idx]->name.c_str());
                  if (data_plane(vinfo, plane, vinfo_list[idx])) {
                     plane_array.add(plane, lvl_lower, lvl_upper);
                     n_rec++;
                  }
                  mlog << Debug(5) << method_name
                       << "Found range match for VarInfo \"" << req_name
                       << " (" << vinfo_list[idx]->name << ")\"\n";
               }
            }
         }
         else {
            mlog << Error << "\n" << method_name
                 << "Can not find variables which begin with \"" << req_name
                 << "\"\n\n";
            exit(1);
         }
      }
      else {
          // NOT SUPPORTED???
      }
   }
   else {   // if (level.type() == LevelType_None) {
      if (lvl_lower != lvl_upper && nullptr != data_vinfo) {
         int tmp_lower = lvl_lower;
         int tmp_upper = lvl_upper;
         int z_offset = bad_data_int;
         if (data_vinfo->z_slot >= 0) {
            int zdim_size = get_dim_size(data_vinfo->var, data_vinfo->z_slot);
            if (tmp_lower >= zdim_size) tmp_lower = zdim_size - 1;
            if (tmp_upper >= zdim_size) tmp_upper = zdim_size - 1;
         }
         for (int idx=tmp_lower; idx<=tmp_upper; idx++) {
            _cur_vert_index = idx;
            if (data_plane(vinfo, plane, data_vinfo)) {
               plane_array.add(plane, lvl_lower, lvl_upper);
               n_rec++;
            }
         }
      }
      else if (data_plane(vinfo, plane)) {
         plane_array.add(plane, lvl_lower, lvl_upper);
         n_rec++;
      }
   }

   return n_rec;
}

////////////////////////////////////////////////////////////////////////

LongArray MetUGridDataFile::collect_time_offsets(VarInfo &vinfo) {
   int n_rec = 0;
   bool status = false;
   VarInfoUGrid *vinfo_nc = (VarInfoUGrid *)&vinfo;
   static const string method_name
         = "MetUGridDataFile::collect_time_offsets(VarInfo &) -> ";

   LongArray time_offsets;
   NcVarInfo *info = _file->find_by_name(vinfo_nc->req_name().c_str());

   // Check for variable not found
   if(!info) {
      mlog << Warning << "\n" << method_name
           << "can't find NetCDF variable \"" << vinfo_nc->req_name()
           << "\" in file \"" << Filename << "\".\n\n";
      return(time_offsets);
   }

   int time_dim_slot = info->t_slot;
   int time_dim_size = _file->ValidTime.n_elements();
   if (0 < time_dim_size && time_dim_slot < 0) {
      // The time dimension does not exist at the variable and the time
      // variable exists. Stop time slicing and set the time offset to 0.
      time_offsets.add(0);
      return(time_offsets);
   }

   double time_lower = bad_data_double;
   double time_upper = bad_data_double;
   int error_code = error_code_no_error;
   LevelInfo level = vinfo.level();
   LongArray dimension = vinfo_nc->dimension();
   bool is_time_range = (level.type() == LevelType_Time);
   bool time_as_value = !level.is_offset();

   long dim_offset = (time_dim_slot >= 0) ? dimension[time_dim_slot] : -1;
   bool include_all_times = (dim_offset == vx_data2d_star);

   int idx;
   TimeArray missing_times;
   if (include_all_times) {
      for (idx=0; idx<time_dim_size; idx++) {
         time_offsets.add(idx);
      }
   }
   else if (is_time_range) {
      double time_inc = level.increment();

      time_lower = level.lower();
      time_upper = level.upper();
      if (time_as_value) {
         int next_offset = -1;
         // Skip times lower than time_lower
         for (idx=0; idx<time_dim_size; idx++) {
            if (_file->ValidTime[idx] < time_lower) continue;
            next_offset = idx;
            if (_file->ValidTime[idx] != time_lower)
               missing_times.add(time_lower);
            else {
               mlog << Debug(9) << method_name << " found the lower time "
                    << unix_to_yyyymmdd_hhmmss(time_lower) << "\n";
               time_offsets.add(idx);
               next_offset++;
            }
            break;
         }

         if (0 > next_offset) error_code = error_code_missing_time_values;
         else if (0 > time_inc) error_code = error_code_bad_increment;
         else if (0 == time_inc) {   // no increment configuration
            for (idx=next_offset; idx<time_dim_size; idx++) {
               if (_file->ValidTime[idx] > time_upper) break;
               time_offsets.add(idx);
               mlog << Debug(9) << method_name << " found the time "
                    << unix_to_yyyymmdd_hhmmss(_file->ValidTime[idx]) << "\n";
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
                  mlog << Debug(9) << method_name << " found the time "
                       << unix_to_yyyymmdd_hhmmss(_file->ValidTime[idx]) << "\n";
                  next_time += time_inc;
               }
               else { // next_time < _file->ValidTime[idx]
                  while (next_time < _file->ValidTime[idx]) {
                     missing_times.add(next_time);
                     next_time += time_inc;
                  }
                  if (_file->ValidTime[idx] == next_time) {
                     time_offsets.add(idx);
                     mlog << Debug(9) << method_name << " found the time "
                          << unix_to_yyyymmdd_hhmmss(_file->ValidTime[idx]) << "\n";
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
         if (max_time_offset == time_dim_size)
            missing_times.add(time_dim_size);
         else if (max_time_offset > time_dim_size) {
            for (idx=time_dim_size; idx<=max_time_offset; idx++)
               missing_times.add(idx);
         }
         for (idx=time_lower; idx<max_time_offset; idx+=inc_offset) {
            time_offsets.add(idx);
            mlog << Debug(9) << method_name << " added index " << idx << "\n";
         }
      }
      
      int missing_count = missing_times.n_elements();
      if (0 < missing_count) {
         for (idx = 0; idx<missing_count; idx++) {
            mlog << Warning << method_name << "Not exist time \""
                 << unix_to_yyyymmdd_hhmmss(missing_times[idx]) << "\".\n";
         }
      }
   }
   else {
      if (time_as_value) dim_offset = convert_time_to_offset(dim_offset);
      if (0 <= time_dim_slot && dim_offset < time_dim_size)
         time_offsets.add(dim_offset);
      else error_code = error_code_unknown;
   }

   int time_count = time_offsets.n_elements();
   if (0 < time_count)
      mlog << Debug(7) << method_name << " Found " << time_count
           << (time_count==1 ? " time" : " times") << " between "
           << unix_to_yyyymmdd_hhmmss(_file->ValidTime[0]) << " and "
           << unix_to_yyyymmdd_hhmmss(_file->ValidTime[time_dim_size-1]) << "\n";
   else {
      mlog << Warning << method_name << "Not found time out of "
           << time_dim_size << ".\n";
      if (include_all_times) error_code = error_code_empty_times;
      else if (is_time_range) {
         error_code = time_as_value ? error_code_missing_time_values
                                    : error_code_missing_time_offsets;
      }
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
         if (0 < time_dim_size) {
            log_msg << " from ["
                    << unix_to_yyyymmdd_hhmmss(_file->ValidTime.min()) << " and "
                    << unix_to_yyyymmdd_hhmmss(_file->ValidTime.max()) << "]";
         }
      }
      else if (error_code == error_code_missing_time_offsets) {
         log_msg << "does not have the matching time offsets between "
                 << nint(time_lower) << " and " << nint(time_upper);
         if (0 < time_dim_size) {
            log_msg << " [0 <= offset < " << time_dim_size << "]";
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
         long time_value = (time_as_value ? dim_offset : -1);
         log_msg << "does not have the matching time "
                 << unix_to_yyyymmdd_hhmmss(time_value) << " ["
                 << unix_to_yyyymmdd_hhmmss(_file->ValidTime.min()) << " and "
                 << unix_to_yyyymmdd_hhmmss(_file->ValidTime.max()) << "]";
      }
      else {
         log_msg.clear();
         log_msg << "variable \"" << vinfo_nc->req_name()
                 << "\" has unknown error (" << error_code << ")";
      }
      mlog << Error << "\n" << method_name << log_msg << ".\n\n";
      exit(1);
   }

   return(time_offsets);
}


////////////////////////////////////////////////////////////////////////

/*
LongArray MetUGridDataFile::collect_vertical_offsets(VarInfo &vinfo) {
   int n_rec = 0;
   bool status = false;
   VarInfoUGrid *vinfo_nc = (VarInfoUGrid *)&vinfo;
   static const string method_name
         = "MetUGridDataFile::collect_vertical_offsets(VarInfo &) -> ";

   LongArray vertical_offsets;
   NcVarInfo *info = _file->find_by_name(vinfo_nc->req_name().c_str());

   // Check for variable not found
   if(!info) {
      mlog << Warning << "\n" << method_name
           << "can't find NetCDF variable \"" << vinfo_nc->req_name()
           << "\" in file \"" << Filename << "\".\n\n";
      return(vertical_offsets);
   }

   int z_dim_slot = info->z_slot;
   int z_dim_size = get_dim_size(_file->get_vert_dim());
   if (0 < z_dim_size && z_dim_slot < 0) {
      // The vertical dimension does not exist at the variable and the vertical
      // variable exists. Stop vertical slicing and set the vertical offset to 0.
      vertical_offsets.add(0);
      return(vertical_offsets);
   }

   double z_lower = bad_data_double;
   double z_upper = bad_data_double;
   int error_code = error_code_no_error;
   LevelInfo level = vinfo.level();
   LongArray dimension = vinfo_nc->dimension();
   bool is_vert_range = (level.type() == LevelType_Pres);
   bool z_as_value = !level.is_offset();

   long dim_offset = (z_dim_slot >= 0) ? dimension[z_dim_slot] : -1;
   bool include_all_verticals = (dim_offset == vx_data2d_star);

   z_as_value = false;  // this is not supported yet.
   if (include_all_verticals) {
      for (int idx=0; idx<z_dim_size; idx++) {
         vertical_offsets.add(idx);
      }
   }
   else if (is_vert_range) {
      int idx;
      LongArray missing_verticals;

      double z_inc = level.increment();

      z_lower = level.lower();
      z_upper = level.upper();
      if (z_as_value) {
         int next_offset = -1;
         // Skip vertical lower than vertical_lower
         for (idx=0; idx<z_dim_size; idx++) {
            if (_file->vlevels[idx] < z_lower) continue;
            next_offset = idx;
            if (_file->vlevels[idx] != z_lower)
               missing_verticals.add(z_lower);
            else {
               mlog << Debug(9) << method_name << " found the lower vertical "
                    << z_lower << "\n";
               vertical_offsets.add(idx);
               next_offset++;
            }
            break;
         }

         if (0 > next_offset) error_code = error_code_missing_vert_values;
         else if (0 > z_inc) error_code = error_code_bad_increment;
         else if (0 == z_inc) {   // no increment configuration
            for (idx=next_offset; idx<z_dim_size; idx++) {
               if (_file->vlevels[idx] > z_upper) break;
               vertical_offsets.add(idx);
               mlog << Debug(9) << method_name << " found the vertical "
                    << _file->vlevels[idx] << "\n";
            }
         }
         else {
            long next_z;
            next_z = z_lower + z_inc;
            for (idx=next_offset; idx<z_dim_size; idx++) {
               if (_file->vlevels[idx] > z_upper) break;
               else if (_file->vlevels[idx] < next_z) continue;
               else if (_file->vlevels[idx] == next_z) {
                  vertical_offsets.add(idx);
                  mlog << Debug(9) << method_name << " found the vertical "
                       << _file->vlevels[idx] << "\n";
                  next_z += z_inc;
               }
               else { // next_z < _file->vlevels[idx]
                  while (next_z < _file->vlevels[idx]) {
                     missing_verticals.add(next_z);
                     next_z += z_inc;
                  }
                  if (_file->vlevels[idx] == next_z) {
                     vertical_offsets.add(idx);
                     mlog << Debug(9) << method_name << " found the vertical "
                          << _file->vlevels[idx] << "\n";
                     next_z += z_inc;
                  }
               }
               if (next_z > z_upper) break;
            }
         }
      }
      else if (z_lower < z_dim_size) {
         int inc_offset = (z_inc <= 0) ? 1 : z_inc;
         int max_vert_offset = z_upper;
         if (max_vert_offset == z_dim_size)
            missing_verticals.add(z_dim_size);
         else if (max_vert_offset > z_dim_size) {
            for (idx=z_dim_size; idx<=z_upper; idx++)
               missing_verticals.add(idx);
            max_vert_offset = z_dim_size - 1;
         }
         for (idx=z_lower; idx<=max_vert_offset; idx+=inc_offset) {
            vertical_offsets.add(idx);
            mlog << Debug(9) << method_name << " added index " << idx << "\n";
         }
      }

      int missing_count = missing_verticals.n_elements();
      if (0 < missing_count) {
         for (idx = 0; idx<missing_count; idx++) {
            mlog << Warning << method_name << "Not exist vertical \""
                 << missing_verticals[idx] << "\".\n";
         }
      }
   }
   else {
      //if (z_as_value) dim_offset = convert_vert_to_offset(dim_offset);
      if (0 <= z_dim_slot && dim_offset < z_dim_size)
         vertical_offsets.add(dim_offset);
      else error_code = error_code_unknown;
   }

//   int z_count = vertical_offsets.n_elements();
//   if (0 < z_count)
//      mlog << Debug(7) << method_name << " Found " << z_count
//           << (z_count==1 ? " vertical" : " verticals") << " between "
//           << _file->vlevels[0] << " and "
//           << _file->vlevels[z_dim_size-1] << "\n";
//   else {
//      mlog << Warning << method_name << "Not found vertical out of "
//           << z_dim_size << ".\n";
//      if (include_all_verticals) error_code = error_code_empty_verticals;
//      else if (is_vert_range) {
//         error_code = z_as_value ? error_code_missing_vert_values
//                                 : error_code_missing_vert_offsets;
//      }
//   }

   // Handling error code
   if (error_code > error_code_no_error) {
      ConcatString log_msg;
      log_msg << "variable \"" << vinfo_nc->req_name() << "\" ";
      if (error_code == error_code_no_vert_dim) {
         log_msg << "does not support the range of verticals because the vertical dimension is "
                 << z_dim_size;
      }
      else if (error_code == error_code_missing_vert_values) {
         log_msg << "does not have the matching vertical ranges between "
                 << z_lower << " and " << z_upper;
         if (0 < z_dim_size) {
            log_msg << " from ["
                    << std::to_string(_file->vlevels.min()) << " and "
                    << std::to_string(_file->vlevels.max()) << "]";
         }
      }
      else if (error_code == error_code_missing_vert_offsets) {
         log_msg << "does not have the matching vertical offsets between "
                 << nint(z_lower) << " and " << nint(z_upper);
         if (0 < z_dim_size) {
            log_msg << " [0 <= offset < " << z_dim_size << "]";
         }
      }
      else if (error_code == error_code_empty_verticals) {
         log_msg << "does not have the vertical values";
      }
      else if (error_code == error_code_bad_increment) {
         log_msg << "was configured with bad increment";
      }
      else if (error_code == error_code_bad_offset) {
         log_msg << "was configured with the bad vertical offset"
                 << " (0 <= offset < " << z_dim_size << ")";
      }
      else if (error_code == error_code_missing_vert_value) {
         long z_value = (z_as_value ? dim_offset : -1);
         log_msg << "does not have the matching vertical "
                 << std::to_string(z_value) << " ["
                 << std::to_string(_file->vlevels.min()) << " and "
                 << std::to_string(_file->vlevels.max()) << "]";
      }
      else {
         log_msg.clear();
         log_msg << "variable \"" << vinfo_nc->req_name()
                 << "\" has unknown error (" << std::to_string(error_code) << ")";
      }
      mlog << Error << "\n" << method_name << log_msg << ".\n\n";
      exit(1);
   }

   return(vertical_offsets);
}
*/

////////////////////////////////////////////////////////////////////////

long MetUGridDataFile::convert_time_to_offset(long time_value) {
   bool found = false;
   bool found_value = false;
   long time_offset = bad_data_int;
   int dim_size = _file->ValidTime.n();
   static const string method_name
         = "MetUGridDataFile::convert_time_to_offset() -> ";

   for (int idx=0; idx<dim_size; idx++) {
      if (_file->ValidTime[idx] == time_value) {
         time_offset = idx;
         found = true;
         break;
      }
   }

   if (!found) {
      dim_size = _file->raw_times.n();
      for (int idx=0; idx<dim_size; idx++) {
         if (_file->raw_times[idx] == time_value) {
            time_offset = idx;
            found_value = true;
            break;
         }
      }
   }

   if (found)
      mlog << Debug(7) << method_name << " Found "
           << unix_to_yyyymmdd_hhmmss(time_value)
           << " at index " << time_offset << " from time value\n";
   else if (found_value)
      mlog << Debug(7) << method_name << " Found " << time_value
           << " at index " << time_offset << " from time value\n";
   else
      mlog << Warning << "\n" << method_name << time_value
           << " does not exist at time variable\n\n";

   return time_offset;
}

////////////////////////////////////////////////////////////////////////

/*
long MetUGridDataFile::convert_value_to_offset(double z_value, string z_dim_name) {
   bool found = false;
   long z_offset = bad_data_int;
   int dim_size = _file->vlevels.n();
   static const string method_name
         = "MetUGridDataFile::convert_value_to_offset() -> ";

   for (int idx=0; idx<dim_size; idx++) {
      if (is_eq(_file->vlevels[idx], z_value)) {
         found = true;
         z_offset = idx;
         break;
      }
   }

   if (!found && 0 < z_dim_name.length()) {
      NcVarInfo *var_info = find_var_info_by_dim_name(_file->Var, z_dim_name, _file->Nvars);
      if (var_info) {
         long new_offset = get_index_at_nc_data(var_info->var, z_value, z_dim_name);
         if (new_offset != bad_data_int) z_offset = new_offset;
      }
   }

   return z_offset;
}
*/

////////////////////////////////////////////////////////////////////////

int MetUGridDataFile::extract_vlevels(ConcatString var_name_base, const char *var_name) {
   int num_mat = 0;
   char** mat = nullptr;
   int vlevel = bad_data_int;
   ConcatString name_pattern;

   name_pattern.erase();
   name_pattern << "(" << var_name_base << "[ _])([0-9]+)(.*)";
   num_mat = regex_apply(name_pattern.c_str(), 4, var_name, mat);
   if (2 < num_mat) vlevel = atoi(mat[2]);

   return vlevel;
}

////////////////////////////////////////////////////////////////////////


long MetUGridDataFile::get_time_offset(long time_value, const long time_cnt,
                                       const char *var_name, const string caller) {
   const long time_threshold_cnt = 10000000;
   long time_offset = convert_time_to_offset(time_value);
   if ((0 > time_offset) || (time_offset >= time_cnt)) {
      if (time_value > time_threshold_cnt)  // from time string (yyyymmdd_hh)
         mlog << Warning << "\n" << caller << "the requested time "
              << unix_to_yyyymmdd_hhmmss(time_value) << " for \""
              << var_name << "\" variable does not exist ("
              << unix_to_yyyymmdd_hhmmss(_file->ValidTime[0]) << " and "
              << unix_to_yyyymmdd_hhmmss(_file->ValidTime[time_cnt-1]) << ").\n\n";
      else
         mlog << Warning << "\n" << caller << "the requested time value "
              << time_value << " for \"" << var_name << "\" variable "
              << "is out of range (between 0 and " << (time_cnt-1) << ").\n\n";
      time_offset = -1;
   }
   return time_offset;
}

////////////////////////////////////////////////////////////////////////

int MetUGridDataFile::index(VarInfo &vinfo){

   if( nullptr == _file->find_by_name( vinfo.name().c_str() ) ) return -1;

   if( ( vinfo.valid() != 0         && _file->ValidTime[0] != vinfo.valid() ) ||
       ( vinfo.init()  != 0         && _file->InitTime     != vinfo.init()  ) ||
       ( !is_bad_data(vinfo.lead()) && _file->lead_time()  != vinfo.lead()  ) )
      return -1;

   return 0;
}

////////////////////////////////////////////////////////////////////////

bool MetUGridDataFile::read_data_plane(ConcatString var_name, VarInfo &vinfo,
                                       DataPlane &plane, LongArray &dimension) {
  static const string method_name
      = "MetUGridDataFile::read_data_plane() -> ";

  // Read the data
  NcVarInfo *info = (NcVarInfo *) nullptr;

  bool status = _file->getData(var_name.c_str(),
                               dimension,
                               plane, info);

  //if (org_time_offset != bad_data_int && 0 <= time_dim_slot)
  //  dimension[time_dim_slot] = org_time_offset;
  //if (org_z_offset != bad_data_int && 0 <= zdim_slot)
  //  dimension[zdim_slot] = org_z_offset;

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

    status = process_data_plane(&vinfo, plane);

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
  }

  return status;
}

////////////////////////////////////////////////////////////////////////

void MetUGridDataFile::set_dataset(ConcatString dataset_name) {
   _file->set_dataset(dataset_name);
}

////////////////////////////////////////////////////////////////////////

void MetUGridDataFile::set_max_distance_km(double max_distance) {
   _file->set_max_distance_km(max_distance);
}

////////////////////////////////////////////////////////////////////////

void MetUGridDataFile::set_map_config_file(ConcatString filename) {
   _file->set_map_config_file(filename);
}


////////////////////////////////////////////////////////////////////////
