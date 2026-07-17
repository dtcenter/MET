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

#include <netcdf>

#include "data2d_nc_cf.h"
#include "vx_math.h"
#include "vx_log.h"

using namespace std;
using namespace netCDF;

////////////////////////////////////////////////////////////////////////

static const int error_code_no_error            =   0;
static const int error_code_no_dim              =   1;
static const int error_code_no_matching_value   =   2;  // no matching value
static const int error_code_no_matching_values  =   3;  // no matching values
static const int error_code_no_matching_offsets =   4;  // no matching offsets
static const int error_code_empty               =   5;
static const int error_code_bad_increment       =   6;
static const int error_code_out_of_index        =   7;
static const int error_code_unknown             = 999;

static const int nc_cf_debug_level              =   7;

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
}

////////////////////////////////////////////////////////////////////////

void MetNcCFDataFile::nccf_init_from_scratch() {

   _file = (NcCfFile *) nullptr;
   cur_time_index = -1;
   cur_z_index = -1;

   close();

   return;
}

////////////////////////////////////////////////////////////////////////

NcVarInfo *MetNcCFDataFile::find_first_data_var() {
   NcVarInfo *first_data_var = nullptr;
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

   if(_file) { delete _file; _file = (NcCfFile *) nullptr; }

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

      return false;
   }

   Filename = _filename;

   Raw_Grid = new Grid;

   (*Raw_Grid) = _file->grid;

   Dest_Grid = new Grid;

   (*Dest_Grid) = (*Raw_Grid);

   return true;
}

////////////////////////////////////////////////////////////////////////

void MetNcCFDataFile::dump(ostream & out, int depth) const {

   if(_file) _file->dump(out, depth);

   return;
}

////////////////////////////////////////////////////////////////////////

int MetNcCFDataFile::add_data_planes_by_time(VarInfo &vinfo, const LevelInfo &level,
                                             DataPlaneArray &plane_array, bool do_winds) {
   int n_rec = 0;
   const auto *vinfo_nc = (VarInfoNcCF *)&vinfo;
   const NcVarInfo *data_var = get_data_var(vinfo);
   if (nullptr == data_var) return 0;
   LongArray dimension = vinfo_nc->dimension();
   static const string method_name
         = "MetNcCFDataFile::add_data_planes_by_time() -> ";

   int z_slot = data_var->z_slot;
   if(0 <= z_slot) {
      if (!vinfo_nc->is_offset(z_slot)
          || vx_data2d_dim_by_value == dimension[z_slot]) {
         dimension[z_slot] = convert_z_to_offset(vinfo_nc->dim_value(z_slot),
                                                      data_var);
      }
      if(range_flag == dimension[z_slot]) {
         mlog << Warning << "\n" << method_name << "vertical level at dim["
              << z_slot << "] can not be range\n\n";
      }
   }

   int t_slot = data_var->t_slot;
   if(0 <= t_slot) {
      DataPlane plane;
      auto time_lower = (unixtime)level.lower();
      auto time_upper = (unixtime)level.upper();
      auto time_cnt = _file->ValidTime.n();
      LongArray time_offsets = collect_time_offsets(vinfo);
      for (int idx=0; idx<time_offsets.n_elements(); idx++) {
         auto time_idx = time_offsets[idx];
         if (time_idx < time_cnt) {
            dimension[t_slot] = time_offsets[idx];
            if (read_data_plane(vinfo, plane, dimension, do_winds)) {
               plane_array.add(plane, (double)time_lower, (double)time_upper);
               n_rec++;
               if (mlog.verbosity_level() >= nc_cf_debug_level) {
                  mlog << Debug(nc_cf_debug_level) << method_name << "time: "
                       << unix_to_yyyymmdd_hhmmss(_file->ValidTime[(int)time_idx])
                       << " from index " << time_idx << "\n";
               }
            }
         }
      }
   }
   return n_rec;
}

////////////////////////////////////////////////////////////////////////

int MetNcCFDataFile::add_data_planes_by_z(VarInfo &vinfo, const LevelInfo &level,
                                          DataPlaneArray &plane_array, bool do_winds) {
   int n_rec = 0;
   const auto *vinfo_nc = (VarInfoNcCF *)&vinfo;
   const NcVarInfo *data_var = get_data_var(vinfo);
   if (nullptr == data_var) return 0;
   LongArray dimension = vinfo_nc->dimension();
   static const string method_name
         = "MetNcCFDataFile::add_data_planes_by_z() -> ";

   int t_slot = data_var->t_slot;
   if(0 <= t_slot) {
      if(!vinfo_nc->is_offset(t_slot)
          || vx_data2d_dim_by_value == vinfo_nc->dimension(t_slot)) {
         dimension[t_slot] = convert_time_to_offset(vinfo_nc->dim_value(t_slot));
      }
      if(range_flag == dimension[t_slot]) {
         mlog << Warning << "\n" << method_name
              << "time at dim[" << t_slot << "] can not be range\n\n";
      }
   }

   int z_slot = data_var->z_slot;
   if(0 <= z_slot) {
      DataPlane plane;
      double z_lower = level.lower();
      double z_upper = level.upper();
      auto z_cnt = data_var->vlevels.n();
      LongArray z_offsets = collect_z_offsets(vinfo);
      for (int idx=0; idx<z_offsets.n_elements(); idx++) {
         auto z_idx = (int)z_offsets[idx];
         if (z_idx < z_cnt) {
            dimension[z_slot] = z_offsets[idx];
            if (read_data_plane(vinfo, plane, dimension, do_winds)) {
               plane_array.add(plane, z_lower, z_upper);
               n_rec++;
            }
            if (mlog.verbosity_level() >= nc_cf_debug_level) {
               mlog << Debug(nc_cf_debug_level) << method_name << "z: "
                    << data_var->vlevels[z_idx]
                    << " from index " << z_idx << "\n";
            }
         }
      }

   }
   return n_rec;
}

////////////////////////////////////////////////////////////////////////

Grid MetNcCFDataFile::build_grid_from_lat_lon_vars(NcVar *lat_var, NcVar *lon_var,
                                                   const long lat_counts,
                                                   const long lon_counts) {
   return (nullptr != _file)
          ? _file->build_grid_from_lat_lon_vars(lat_var, lon_var, lat_counts, lon_counts)
          : grid();
}

////////////////////////////////////////////////////////////////////////

bool MetNcCFDataFile::data_plane(VarInfo &vinfo, DataPlane &plane,
                                 bool do_winds) {
   auto vinfo_nc = (VarInfoNcCF *) &vinfo;
   static const string method_name
      = "MetNcCFDataFile::data_plane() -> ";

   LongArray dimension = vinfo_nc->dimension();
   NcVarInfo *data_var = get_data_var(vinfo);
   if (nullptr == data_var) {
      mlog << Error << "\n" << method_name
           << "\"" << vinfo.req_name() << "\" variable does not exist\n\n";
      return false;
   }

   int time_dim_slot = data_var->t_slot;
   int zdim_slot = data_var->z_slot;

   // set vlevels if needed
   _file->set_vlevels(data_var);

   for (int idx=0; idx<dimension.n_elements(); idx++) {
      long dim_offset = dimension[idx];
      if (dim_offset == vx_data2d_star) continue;
      if (idx == time_dim_slot) {
        dimension[time_dim_slot] = find_time_offset(vinfo, data_var);
      }
      else if (idx == zdim_slot) {
        dimension[idx] = long(find_z_offset(vinfo, data_var));
      }
      else {
         mlog << Debug(7) << method_name << "parsing generic dimension " << idx
              << " for \"" << vinfo.req_name() << "\" variable.\n\n";
          dimension[idx] = long(find_generic_offset(vinfo, data_var, idx));
      }
   }

   // Read the data
   bool status = read_data_plane(vinfo, plane, dimension, do_winds);

   // Assume that CF-compliant NetCDF winds are earth-relative
   vinfo_nc->set_grid_relative_flag(false);

   // Attempt to derive the data
   if(!status && do_winds) {
      status = derive_winds(vinfo_nc, plane);
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

bool MetNcCFDataFile::read_data_plane(VarInfo &vinfo, DataPlane &plane,
                                      const LongArray &dimension, bool do_winds)
{
  auto vinfo_nc = (VarInfoNcCF *)&vinfo;
  static const string method_name
      = "MetNcCFDataFile::read_data_plane() -> ";

  Grid grid_attr = vinfo.grid_attr();
  _file->update_grid(grid_attr);

  // Initialize the data plane

  plane.clear();

  // Read the data
  auto info = (NcVarInfo *)nullptr;

  bool status = _file->getData(vinfo_nc->req_name().c_str(),
                               dimension,
                               plane, info);

  // Check that the times match those requested

  if(status) {
    if(vinfo.valid() > 0) {
      // Check that the valid time and matches the request
      if (vinfo.valid() != plane.valid()) {
        // Compute time strings

        ConcatString req_time_str  = unix_to_yyyymmdd_hhmmss(vinfo.valid());
        ConcatString data_time_str = unix_to_yyyymmdd_hhmmss(plane.valid());

        mlog << Warning << "\n" << method_name
             << "for \"" << vinfo.req_name() << "\" variable, the valid "
             << "time does not match the requested valid time: ("
             << data_time_str << " != " << req_time_str << ")\n\n";
        // set status false;
      }

      // Check that the lead time matches the request
      if (vinfo.lead() != plane.lead()) {
        // Compute time strings

        ConcatString req_time_str  = sec_to_hhmmss(vinfo.lead());
        ConcatString data_time_str = sec_to_hhmmss(plane.lead());

        mlog << Warning << "\n" << method_name
             << "for \"" << vinfo.req_name() << "\" variable, the lead "
             << "time does not match the requested lead time: ("
             << data_time_str << " != " << req_time_str << ")\n\n";
        // set status false;
      }
    }

    // Handle wind rotation
    if(status && do_winds) status = rotate_winds(&vinfo, plane);

    if(status) status = process_data_plane(&vinfo, plane);

    // Set the VarInfo object's name, long_name, level, and units strings

    if (!info->name_att.empty())
      vinfo.set_name(info->name_att);
    else
      vinfo.set_name(info->name);

    if (!info->long_name_att.empty())
      vinfo.set_long_name(info->long_name_att.c_str());

    if (!info->level_att.empty())
      vinfo.set_level_name(info->level_att.c_str());

    if (!info->units_att.empty())
      vinfo.set_units(info->units_att.c_str());
  }

  return status;
}

////////////////////////////////////////////////////////////////////////

int MetNcCFDataFile::data_plane_array(VarInfo &vinfo,
                                      DataPlaneArray &plane_array,
                                      bool do_winds) {
   int n_rec = 0;
   DataPlane plane;
   static const string method_name
         = "MetNcCFDataFile::data_plane_array(VarInfo &, DataPlaneArray &) -> ";

   Grid grid_attr = vinfo.grid_attr();
   _file->update_grid(grid_attr);

   // Initialize
   plane_array.clear();

   auto vinfo_nc = (VarInfoNcCF *)&vinfo;
   NcVarInfo *data_var = get_data_var(vinfo);
   if (nullptr == data_var) return 0;
   int t_dim_slot = data_var->t_slot;
   int z_dim_slot = data_var->z_slot;
   LongArray dimension = vinfo_nc->dimension();

   // set vlevels if needed
   _file->set_vlevels(data_var);

   LevelInfo level = vinfo.level();
   cur_time_index = cur_z_index = 0;

   if (0 <= t_dim_slot && range_flag == dimension[t_dim_slot] && level.type() == LevelType_Time) {
      n_rec = add_data_planes_by_time(vinfo, level, plane_array, do_winds);
   }
   else if (0 <= z_dim_slot && range_flag == dimension[z_dim_slot] && level.type() == LevelType_Pres) {
      n_rec = add_data_planes_by_z(vinfo, level, plane_array, do_winds);
   }
   else if (data_plane(vinfo, plane, do_winds)) {
      plane_array.add(plane, bad_data_int, bad_data_int);
      n_rec++;
   }

   // Assume that CF-compliant NetCDF winds are earth-relative
   vinfo_nc->set_grid_relative_flag(true);

   // Attempt to derive the data
   if(n_rec == 0 && do_winds) {
      derive_winds(vinfo_nc, plane_array);
   }

   return n_rec;
}

////////////////////////////////////////////////////////////////////////

LongArray MetNcCFDataFile::collect_time_offsets(VarInfo &vinfo) {
   auto vinfo_nc = (VarInfoNcCF *)&vinfo;
   static const string method_name
         = "MetNcCFDataFile::collect_time_offsets(VarInfo &) -> ";

   LongArray time_offsets;
   const NcVarInfo *info = _file->find_var_name(vinfo_nc->req_name().c_str());

   // Check for variable not found
   if(!info) {
      mlog << Warning << "\n" << method_name
           << "can't find NetCDF variable \"" << vinfo_nc->req_name()
           << "\" in file \"" << Filename << "\".\n\n";
      return time_offsets;
   }

   int time_dim_slot = info->t_slot;
   int time_dim_size = _file->ValidTime.n_elements();
   if (0 < time_dim_size && time_dim_slot < 0) {
      // The time dimension does not exist at the variable and the time
      // variable exists. Stop time slicing and set the time offset to 0.
      time_offsets.add(0);
      return time_offsets;
   }

   int tmp_time_dim_slot = time_dim_slot;
   if (time_dim_slot < 0) tmp_time_dim_slot = 0;    // to avoid the out of range error

   int error_code = error_code_no_error;
   LevelInfo level = vinfo.level();
   LongArray dimension = vinfo_nc->dimension();
   bool is_time_range = (LevelType_Time == level.type());
   bool time_as_value = !vinfo_nc->is_offset(tmp_time_dim_slot);

   long dim_offset = (time_dim_slot >= 0) ? dimension[time_dim_slot] : -1;
   bool include_all_times = (dim_offset == vx_data2d_star);

   auto time_lower = (unixtime)level.lower();
   auto time_upper = (unixtime)level.upper();
   TimeArray missing_times;
   if (include_all_times) {
      for (int idx=0; idx<time_dim_size; idx++) {
         time_offsets.add(idx);
      }
   }
   else if (is_time_range) {
      auto time_inc = (unixtime)level.increment();
      if (0 > time_inc) error_code = error_code_bad_increment;
      else {
         if (time_as_value) {
            bool found_lower = false;
            int first_idx = -1;
            int next_offset = -1;
            // Skip times lower than time_lower
            for (int idx=0; idx<time_dim_size; idx++) {
               if(_file->ValidTime[idx] < time_lower) continue;
               if(_file->ValidTime[idx] == time_lower) found_lower = true;
               if(_file->ValidTime[idx] <= time_upper) {
                  mlog << Debug(9) << method_name << " found the first time "
                       << unix_to_yyyymmdd_hhmmss(_file->ValidTime[idx]) << " level lower: ["
                       << unix_to_yyyymmdd_hhmmss(time_lower) << "]\n";
                  first_idx = idx;
                  next_offset = idx + 1;
                  break;
               }
            }

            if(0 > first_idx) error_code = error_code_no_matching_values;
            else if(0 == time_inc) {   // no increment configuration
               time_offsets.add(first_idx);
               for (int idx=next_offset; idx<time_dim_size; idx++) {
                  if(_file->ValidTime[idx] < time_lower     // in case of not sorted
                     || _file->ValidTime[idx] > time_upper) continue;
                  time_offsets.add(idx);
                  mlog << Debug(9) << method_name << " found the time "
                       << unix_to_yyyymmdd_hhmmss(_file->ValidTime[idx]) << "\n";
               }
            }
            else {
               auto next_time = time_lower + time_inc;
               if (found_lower) time_offsets.add(first_idx);
               bool should_break = false;
               for (int idx=next_offset; idx<time_dim_size && !should_break; idx++) {
                  if (_file->ValidTime[idx] > time_upper) {
                    should_break = true;
                    continue;
                  }
                  if (_file->ValidTime[idx] < next_time) continue;
                  if (_file->ValidTime[idx] == next_time) {
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
                  if (next_time > time_upper) {
                    should_break = true;
                  }
               }
            }
         }
         else if (time_lower < time_dim_size) {      // index, not values
            int inc_offset = (time_inc <= 0) ? 1 : (int)time_inc;
            auto max_time_offset = (int)time_upper;
            if (max_time_offset >= time_dim_size) {
               max_time_offset = time_dim_size - 1;
               mlog << Debug(7) << method_name
                    << "Ignored index above dimension size (" << time_dim_size << ")\n";
            }
            for (auto idx=(int)time_lower; idx<=max_time_offset; idx+=inc_offset) {
               time_offsets.add(idx);
               mlog << Debug(9) << method_name << " added index " << idx << "\n";
            }
         }

         int missing_count = missing_times.n_elements();
         for (int idx = 0; idx<missing_count; idx++) {
            mlog << Warning << method_name << "Not exist time \""
                 << unix_to_yyyymmdd_hhmmss(missing_times[idx]) << "\".\n";
         }
      }
   }
   else {    // a single match
      if (time_as_value
          || vx_data2d_dim_by_value == dim_offset) {
         dim_offset = convert_time_to_offset(vinfo_nc->dim_value(tmp_time_dim_slot));
      }
      if (dim_offset >= time_dim_size) error_code = error_code_out_of_index;
      else if (0 <= time_dim_slot) time_offsets.add(dim_offset);
      else error_code = error_code_unknown;
   }

   int time_count = time_offsets.n_elements();
   if (0 < time_count)
      mlog << Debug(7) << method_name << "Found " << time_count
           << (time_count==1 ? " time" : " times") << " between "
           << unix_to_yyyymmdd_hhmmss(_file->ValidTime[0]) << " and "
           << unix_to_yyyymmdd_hhmmss(_file->ValidTime[time_dim_size-1]) << "\n";
   else {
      mlog << Warning << method_name << "Not found time out of "
           << time_dim_size << ".\n";
      if (include_all_times) error_code = error_code_empty;
      else if (is_time_range) {
         error_code = time_as_value ? error_code_no_matching_values
                                    : error_code_no_matching_offsets;
      }
   }

   // Handling error code
   if (error_code > error_code_no_error) {
      long time_value = (time_as_value ? dim_offset : -1);
      error_message(true, error_code, (double)time_lower, (double)time_upper,
                    time_value, vinfo_nc->req_name(), method_name);
      exit(1);
   }

   return time_offsets;
}


////////////////////////////////////////////////////////////////////////

LongArray MetNcCFDataFile::collect_z_offsets(VarInfo &vinfo) {
   auto vinfo_nc = (VarInfoNcCF *)&vinfo;
   static const string method_name
         = "MetNcCFDataFile::collect_z_offsets(VarInfo &) -> ";

   LongArray z_offsets;
   const NcVarInfo *info = _file->find_var_name(vinfo_nc->req_name().c_str());

   // Check for variable not found
   if(!info) {
      mlog << Warning << "\n" << method_name
           << "can't find NetCDF variable \"" << vinfo_nc->req_name()
           << "\" in file \"" << Filename << "\".\n\n";
      return z_offsets;
   }

   int z_slot = info->z_slot;
   int z_dim_size = info->vlevels.n_elements();
   if (0 < z_dim_size && z_slot < 0) {
      // The z dimension does not exist at the variable and the z
      // variable exists. Stop z slicing and set the z offset to 0.
      z_offsets.add(0);
      return z_offsets;
   }

   int tmp_z_slot = z_slot;
   if (z_slot < 0) tmp_z_slot = 0;    // to avoid the out of range error

   int error_code = error_code_no_error;
   LevelInfo level = vinfo.level();
   long dim_offset = vinfo_nc->dimension(tmp_z_slot);
   bool is_z_range = (dim_offset == range_flag);    // && (level.type() == LevelType_Pres)
   bool z_as_value = !vinfo_nc->is_offset(tmp_z_slot);

   bool include_all_z = (dim_offset == vx_data2d_star);

   TimeArray missing_z;
   double z_lower = level.lower();
   double z_upper = level.upper();
   if (include_all_z) {
      for (int idx=0; idx<z_dim_size; idx++) {
         z_offsets.add(idx);
      }
   }
   else if (is_z_range) {
      double z_inc = level.increment();
      if (0 > z_inc) error_code = error_code_bad_increment;
      else {
         if (z_as_value) {
            bool found_lower = false;
            int first_idx = -1;
            int next_offset = -1;
            // Skip z lower than z_lower
            for (int idx=0; idx<z_dim_size; idx++) {
               if(info->vlevels[idx] < z_lower) continue;
               if(info->vlevels[idx] == z_lower) found_lower = true;
               if(info->vlevels[idx] <= z_upper) {
                  mlog << Debug(9) << method_name << " found the first z "
                       << info->vlevels[idx] << " lower: ["
                       << z_lower << "]\n";
                  first_idx = idx;
                  next_offset = idx + 1;
                  break;
               }
            }

            if(0 > next_offset) error_code = error_code_no_matching_values;
            else if(0 == z_inc) {   // no increment configuration
               z_offsets.add(first_idx);
               for (int idx=next_offset; idx<z_dim_size; idx++) {
                  if(info->vlevels[idx] < z_lower     // in case of not sorted
                     || info->vlevels[idx] > z_upper) continue;
                  z_offsets.add(idx);
                  mlog << Debug(9) << method_name << " found the z "
                       << info->vlevels[idx] << "\n";
               }
            }
            else {
               double next_z = z_lower + z_inc;
               if (found_lower) z_offsets.add(first_idx);
               for (int idx=next_offset; idx<z_dim_size; idx++) {
                  if (info->vlevels[idx] > z_upper) break;
                  if (info->vlevels[idx] < next_z) continue;
                  if (info->vlevels[idx] == next_z) {
                     z_offsets.add(idx);
                     mlog << Debug(9) << method_name << " found the z "
                          << info->vlevels[idx] << " index=" << idx << "\n";
                     next_z += z_inc;
                  }
                  else { // next_z < info->vlevels[idx]
                     while (next_z < info->vlevels[idx]) {
                        missing_z.add((unixtime)next_z);
                        next_z += z_inc;
                     }
                     if (info->vlevels[idx] == next_z) {
                        z_offsets.add(idx);
                        mlog << Debug(9) << method_name << " found the z "
                             << info->vlevels[idx] << " index=" << idx << "\n";
                        next_z += z_inc;
                     }
                  }
                  if (next_z > z_upper) break;
               }
            }
         }
         else if (z_lower < z_dim_size) {
            int inc_offset = (z_inc <= 0) ? 1 : (int)z_inc;
            auto max_z_offset = (int)z_upper;
            if (max_z_offset >= z_dim_size) {
               max_z_offset = z_dim_size;
               mlog << Debug(7) << method_name
                    << "Ignored index above dimension size (" << z_dim_size << ")\n";
            }
            for (auto idx=(int)z_lower; idx<=max_z_offset; idx+=inc_offset) {
               z_offsets.add(idx);
               mlog << Debug(9) << method_name << " added index " << idx << "\n";
            }
         }

         int missing_count = missing_z.n_elements();
         for (int idx = 0; idx<missing_count; idx++) {
            mlog << Warning << method_name << "Not exist z \""
                 << missing_z[idx] << "\".\n";
         }
      }
   }
   else {    // a single match
      const NcVarInfo *data_var = _file->find_var_name(vinfo_nc->req_name().c_str());
      if (z_as_value) dim_offset = convert_z_to_offset(vinfo_nc->dim_value(tmp_z_slot),
                                                       data_var);

      if (0 <= z_slot && dim_offset < z_dim_size)
         z_offsets.add(dim_offset);
      else error_code = error_code_unknown;
   }

   int z_count = z_offsets.n_elements();
   if (0 < z_count)
      mlog << Debug(7) << method_name << "Found " << z_count
           << (z_count==1 ? " vlevel" : " vlevels") << " between "
           << info->vlevels[0] << " and "
           << info->vlevels[z_dim_size-1] << "\n";
   else {
      mlog << Warning << method_name << "Not found vlevel out of "
           << z_dim_size << ".\n";
      if (include_all_z) error_code = error_code_empty;
      else if (is_z_range) {
         error_code = z_as_value ? error_code_no_matching_values
                                      : error_code_no_matching_offsets;
      }
   }

   // Handling error code
   if (error_code > error_code_no_error) {
      long z_value = (z_as_value ? dim_offset : -1);
      error_message(false, error_code, z_lower, z_upper,
                    z_value, vinfo_nc->req_name(), method_name);
      exit(1);
   }

   return z_offsets;
}


////////////////////////////////////////////////////////////////////////

long MetNcCFDataFile::convert_time_to_offset(double time_value) const {
   bool found = false;
   bool found_value = false;
   long time_offset = bad_data_int;
   int dim_size = _file->ValidTime.n();
   static const string method_name
         = "MetNcCFDataFile::convert_time_to_offset() -> ";

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
      mlog << Debug(7) << method_name << "Found "
           << unix_to_yyyymmdd_hhmmss(time_value)
           << " at index " << time_offset << " from time value\n";
   else if (found_value)
      mlog << Debug(7) << method_name << "Found " << time_value
           << " at index " << time_offset << " from time value\n";
   else
      mlog << Warning << "\n" << method_name << time_value
           << " does not exist at time variable\n\n";

   return time_offset;
}

////////////////////////////////////////////////////////////////////////

long MetNcCFDataFile::convert_generic_to_offset(double value, const string &dim_name, vector<double> values) {
   static const string method_name = "MetNcCFDataFile::convert_generic_to_offset() -> ";
   long offset = bad_data_int;

   // search for the value in the array of values
   auto dim_size = static_cast<int>(values.size());
   for (int idx=0; idx<dim_size; idx++) {
      if (is_eq(values[idx], value)) {
         offset = idx;
         break;
      }
   }

   if (offset == (long) bad_data_int && !dim_name.empty()) {
      NcVarInfo *var_info = find_var_info_by_dim_name(_file->Var, dim_name, _file->Nvars);
      if (var_info) {
         long new_offset = get_index_at_nc_data(var_info->var, value, dim_name);
         if (new_offset != bad_data_int) offset = new_offset;
      }
   }

   // Log the dimension value to index conversion
   if(offset != (long) bad_data_int) {
      mlog << Debug(7) << method_name << "Found \""
           << dim_name << "\" dimension value of \"" << value
           << "\" at dimension index " << offset << ".\n";
   }

  return offset;
}

////////////////////////////////////////////////////////////////////////

long MetNcCFDataFile::convert_z_to_offset(double z_value, const NcVarInfo* data_var) {
  return convert_generic_to_offset(z_value, get_z_dim_name(data_var), data_var->vlevels.vals());
}

////////////////////////////////////////////////////////////////////////

void MetNcCFDataFile::error_message(const bool is_dim_time, const int error_code,
                                    const double _lower, const double _upper,
                                    const long _value, const ConcatString &var_name,
                                    const string &method_name) const {
   // Handling error code
   ConcatString log_msg;
   const NcVarInfo* info = _file->find_var_name(var_name.c_str());
   const int dim_size = is_dim_time ? _file->ValidTime.n() : info->vlevels.n();
   const char *dim_name = is_dim_time ? "time" : "vlevel";

   log_msg << "variable \"" << var_name << "\" ";
   if (error_code == error_code_no_dim) {
      log_msg << "does not support the range of " << dim_name << " because the "
              << dim_name << " dimension does not exist";
   }
   else if (error_code == error_code_no_matching_values) {
      if(is_dim_time) {
         log_msg << "does not have the matching " << dim_name << " ranges between "
                 << unix_to_yyyymmdd_hhmmss((unixtime)_lower) << " and "
                 << unix_to_yyyymmdd_hhmmss((unixtime)_upper)
                 << " from ["
                 << unix_to_yyyymmdd_hhmmss(_file->ValidTime.min()) << " and "
                 << unix_to_yyyymmdd_hhmmss(_file->ValidTime.max()) << "]";
      }
      else {
         log_msg << "does not have the matching " << dim_name << " ranges between "
                 << _lower << " and " << _upper
                 << " from [" << info->vlevels.min() << " and "
                 << info->vlevels.max() << "]";
      }
   }
   else if (error_code == error_code_no_matching_offsets) {
      log_msg << "does not have the matching " << dim_name << " offsets between "
              << nint(_lower) << " and " << nint(_upper)
              << " [0 <= offset < " << dim_size << "]";
   }
   else if (error_code == error_code_empty) {
      log_msg << "does not have the " << dim_name << " values";
   }
   else if (error_code == error_code_bad_increment) {
      log_msg << "was configured with bad increment";
   }
   else if (error_code == error_code_out_of_index) {
      log_msg << "was configured with the bad " << dim_name << " offset"
              << " (0 <= offset < " << dim_size << ")";
   }
   else if (error_code == error_code_no_matching_value) {
      if(is_dim_time) {
         log_msg << "does not have the matching " << dim_name
                 << unix_to_yyyymmdd_hhmmss(_value) << " ["
                 << unix_to_yyyymmdd_hhmmss(_file->ValidTime.min()) << " and "
                 << unix_to_yyyymmdd_hhmmss(_file->ValidTime.max()) << "]";
      }
      else {
         log_msg << "does not have the matching " << dim_name
                 << (int)_value << " [" << info->vlevels.min() << " and "
                 << info->vlevels.max() << "]";
      }
   }
   else {
      log_msg << " has unknown error (" << error_code << ")";
   }
   mlog << Error << "\n" << method_name << log_msg << ".\n\n";
}

////////////////////////////////////////////////////////////////////////

long MetNcCFDataFile::find_time_offset(VarInfo &vinfo, const NcVarInfo *data_var) {
   static const string method_name
         = "MetNcCFDataFile::find_time_offset() -> ";

   long time_offset = -1;
   int t_slot = data_var->t_slot;
   if(0 > t_slot) return time_offset;

   auto vinfo_nc = (VarInfoNcCF *)&vinfo;
   time_offset = vinfo_nc->dimension(t_slot);
   auto time_value = time_offset;

   int time_cnt = _file->ValidTime.n();
   const long time_threshold_cnt = 10000000;
   if (time_offset == range_flag) {
      if (cur_time_index < 0) { // cur_time_index is not initialized
         LevelInfo level = vinfo.level();
         double t_lower = level.lower();
         double t_upper = level.upper();
         if (vinfo_nc->is_offset(t_slot)) {
            time_offset = (long)t_lower;
         }
         else {
            time_value = (long)t_lower;
            time_offset = convert_time_to_offset(t_lower);
            if (time_offset < 0) {
               LongArray time_offsets = collect_time_offsets(vinfo);
               if(0 < time_offsets.n_elements()) time_offset = time_offsets[0];
            }
         }
         if (time_offset >= 0) {
            if (vinfo_nc->is_offset(t_slot)) {
               mlog << Debug(1) << method_name << "the time ["
                    << time_offset << "] was selected between " << t_lower
                    << " and " << t_upper << "\n";
            }
            else {
               mlog << Debug(1) << method_name << "the time ["
                    << time_offset << "] was selected between "
                    << unix_to_yyyymmdd_hhmmss((unixtime)t_lower)
                    << " and " << unix_to_yyyymmdd_hhmmss((unixtime)t_upper) << "\n";
            }
         }
      }
      else time_offset = cur_time_index;  // from data_plane_array()
   }
   else if (!vinfo_nc->is_offset(t_slot)) {
      time_value = (unixtime)vinfo_nc->dim_value(t_slot);
      time_offset = convert_time_to_offset((double)time_value);
   }
   else if (time_offset >= time_threshold_cnt) {
      time_offset = convert_time_to_offset((double)time_offset);
   }

   if ((0 > time_offset) || (time_offset >= time_cnt)) {
      if (time_value > time_threshold_cnt)  // from time string (yyyymmdd_hh)
         mlog << Error << "\n" << method_name << "the requested time "
              << unix_to_yyyymmdd_hhmmss(time_value) << " for \""
              << vinfo.req_name() << "\" variable does not exist ("
              << unix_to_yyyymmdd_hhmmss(_file->ValidTime[0]) << " and "
              << unix_to_yyyymmdd_hhmmss(_file->ValidTime[time_cnt-1]) << ").\n\n";
      else
         mlog << Error << "\n" << method_name << "the requested time value "
              << time_value << " for \"" << vinfo.req_name() << "\" variable "
              << "is out of range (between 0 and " << (time_cnt-1) << ").\n\n";
      exit(1);
   }

   return time_offset;
}


////////////////////////////////////////////////////////////////////////

long MetNcCFDataFile::find_z_offset(VarInfo &vinfo, const NcVarInfo *data_var) {
   static const string method_name
         = "MetNcCFDataFile::find_z_offset() -> ";

   long z_offset = -1;
   int z_slot = data_var->z_slot;

   if(0 > z_slot) return z_offset;

   auto vinfo_nc = (VarInfoNcCF *)&vinfo;
   z_offset = vinfo_nc->dimension(z_slot);

   int z_cnt = data_var->vlevels.n();
   if (z_offset == range_flag) {
      if (cur_z_index < 0) { // cur_z_index is not initialized
         LevelInfo level = vinfo.level();
         double z_lower = level.lower();
         double z_upper = level.upper();
         if (vinfo_nc->is_offset(z_slot)) {
            z_offset = (long)z_lower;
         }
         else {
            z_offset = convert_z_to_offset(z_lower, data_var);
            if (z_offset < 0) {
               LongArray z_offsets = collect_z_offsets(vinfo);
               if(0 <z_offsets.n_elements()) z_offset = z_offsets[0];
            }
         }
         if (z_offset >= 0) {
            mlog << Debug(1) << method_name << "the lowest level ["
                 << z_offset << "] was selected between " << z_lower
                 << " and " << z_upper << "\n";
         }
      }
      else z_offset = (int)cur_z_index;   // from data_plane_array()
   }
   else if (!vinfo_nc->is_offset(z_slot)) {
      double z_value = vinfo_nc->dim_value(z_slot);
      z_offset = convert_z_to_offset(z_value, data_var);
      if ((0 > z_offset) || (z_offset >= z_cnt)) {
         mlog << Error << "\n" << method_name << "the requested vlevel "
              << z_value << " for \""
              << vinfo.req_name() << "\" variable does not exist ("
              << data_var->vlevels[0] << " and "
              << data_var->vlevels[z_cnt-1] << ").\n\n";
         exit(1);
      }
   }

   if ((0 > z_offset) || (z_offset >= z_cnt)) {
      mlog << Error << "\n" << method_name << "the requested vlevel offset "
           << z_offset << " for \"" << vinfo.req_name() << "\" variable "
           << "is out of range (between 0 and " << (z_cnt-1) << ").\n\n";
      exit(1);
   }

   return z_offset;
}

////////////////////////////////////////////////////////////////////////

long MetNcCFDataFile::find_generic_offset(VarInfo &vinfo, const NcVarInfo *data_var, int index) {
   static const string method_name = "MetNcCFDataFile::find_generic_offset() -> ";

   auto vinfo_nc = (VarInfoNcCF *)&vinfo;
   long offset = vinfo_nc->dimension(index);
   string dim_name = get_dim_name(data_var, index);
   NcVarInfo* dim_var_info = _file->find_var_by_dim_name(dim_name.c_str());

   int dim_size = get_data_size(dim_var_info->var);
   vector<double> values(dim_size);

   if( !get_nc_data(dim_var_info->var, values.data()) ) {
      mlog << Error << "\n" << method_name << "failed to get data from " << dim_name << "\n\n";
      exit(1);
   }

   if (offset == range_flag) {
      mlog << Warning << "\n" << method_name << "range is not supported for "
           << offset << " dimension for generic \"" << vinfo.req_name() << "\" variable\n\n";
   }
   else if (!vinfo_nc->is_offset(index)) {
      double value = vinfo_nc->dim_value(index);
      offset = convert_generic_to_offset(value, dim_name, values);
      if (offset < 0) {
         mlog << Error << "\n" << method_name << "the requested value "
              << value << " for \""
              << vinfo.req_name() << "\" variable does not exist\n\n";
         exit(1);
      }
   }

   if (offset < 0 || offset >= dim_size) {
      mlog << Error << "\n" << method_name << "the requested offset "
           << offset << " for \"" << vinfo.req_name() << "\" variable "
           << "is out of range (between 0 and " << (dim_size-1) << ").\n\n";
      exit(1);
   }

   return offset;
}

/////////////////////////////////////////////////////////////////////////

NcVarInfo *MetNcCFDataFile::get_data_var(VarInfo &vinfo) {
   auto data_var = (NcVarInfo *)nullptr;

   // Check for NA in the requested name
   if(vinfo.req_name() == na_str) {
      // Store the name of the first data variable
      data_var = find_first_data_var();
      if (nullptr != data_var) {
         vinfo.set_req_name(data_var->name.c_str());
      }
   }
   else data_var = _file->find_var_name(vinfo.req_name().c_str());

   return data_var;
}

/////////////////////////////////////////////////////////////////////////

string MetNcCFDataFile::get_dim_name(const NcVarInfo *data_var, int index) const {
   string dim_name;
   if (index >= 0) {
      NcDim dim = get_nc_dim(data_var->var, index);
      if (IS_VALID_NC(dim)) dim_name = GET_NC_NAME(dim);
   }
   return dim_name;
}

/////////////////////////////////////////////////////////////////////////

string MetNcCFDataFile::get_z_dim_name(const NcVarInfo *data_var) const {
   return get_dim_name(data_var, data_var->z_slot);
}

/////////////////////////////////////////////////////////////////////////

int MetNcCFDataFile::index(VarInfo &vinfo){

   if( nullptr == _file->find_var_name( vinfo.name().c_str() ) ) return -1;

   if( ( vinfo.valid() != 0         && _file->ValidTime[0] != vinfo.valid() ) ||
       ( vinfo.init()  != 0         && _file->InitTime     != vinfo.init()  ) ||
       ( !is_bad_data(vinfo.lead()) && _file->lead_time()  != vinfo.lead()  ) )
      return -1;

   return 0;
}

/////////////////////////////////////////////////////////////////////////
