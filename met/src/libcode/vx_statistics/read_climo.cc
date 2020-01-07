// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <map>

#include "read_climo.h"

#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_regrid.h"

////////////////////////////////////////////////////////////////////////

static void read_climo_file(
          const char *, GrdFileType, Dictionary *, unixtime,
          int, int, const Grid &, const RegridInfo &,
          DataPlaneArray &dpa);

static DataPlaneArray climo_time_interp(
          const DataPlaneArray &, int, unixtime, InterpMthd);

static DataPlane climo_hms_interp(
          const DataPlaneArray &, const IntArray&, unixtime, InterpMthd);

////////////////////////////////////////////////////////////////////////

DataPlane read_climo_data_plane(Dictionary *dict, int i_vx,
                                unixtime vld_ut, const Grid &vx_grid) {
   DataPlane dp;
   DataPlaneArray dpa;

   // Check for null
   if(!dict) return(dp);

   // Read array of climatology fields
   dpa = read_climo_data_plane_array(dict, i_vx, vld_ut, vx_grid);

   // Check for multiple matches
   if(dpa.n_planes() > 1) {
      mlog << Warning << "\nread_climo_data_plane() -> "
           << "Found " << dpa.n_planes() << " matching climatology "
           << "fields.  Using the first match found.\n\n";
   }

   // Store the first match found
   if(dpa.n_planes() > 0) dp = dpa[0];

   return(dp);
}

////////////////////////////////////////////////////////////////////////

DataPlaneArray read_climo_data_plane_array(Dictionary *dict, int i_vx,
                                           unixtime vld_ut,
                                           const Grid &vx_grid) {
   DataPlaneArray dpa;
   StringArray climo_files;
   RegridInfo regrid_info;
   InterpMthd time_interp;
   GrdFileType ctype;
   double day_interval, hour_interval;
   int i, day_ts, hour_ts;

   // Check for null
   if(!dict) return(dpa);

   // Get the i-th array entry
   Dictionary i_dict = parse_conf_i_vx_dict(dict, i_vx);

   // Climatology mean and standard deviation files
   climo_files = i_dict.lookup_string_array(conf_key_file_name, false);

   // Check for at least one file
   if(climo_files.n() == 0) return(dpa);

   // Regrid info
   regrid_info = parse_conf_regrid(&i_dict);

   // Time interpolation
   time_interp = int_to_interpmthd(i_dict.lookup_int(conf_key_time_interp_method));

   // Day interval
   day_interval = i_dict.lookup_double(conf_key_day_interval);

   // Range check day_interval
   if(!is_bad_data(day_interval) && day_interval < 1) {
      mlog << Error << "\nread_climo_data_plane_array() -> "
           << "The \"" << conf_key_day_interval << "\" entry ("
           << day_interval << ") can be set to " << na_str
           << " or a value of at least 1.\n\n";
      exit(1);
   }

   // Hour interval
   hour_interval = i_dict.lookup_double(conf_key_hour_interval);

   // Range check hour_interval
   if(!is_bad_data(hour_interval) &&
      (hour_interval <= 0 || hour_interval > 24)) {
      mlog << Error << "\nread_climo_data_plane_array() -> "
           << "The \"" << conf_key_hour_interval << "\" entry ("
           << hour_interval << ") can be set to " << na_str
           << " or a value between 0 and 24.\n\n";
      exit(1);
   }

   // Check if file_type was specified
   ctype = parse_conf_file_type(&i_dict);

   // Store the time steps in seconds
   day_ts  = (is_bad_data(day_interval) ? bad_data_int :
              nint(day_interval * 24.0 * sec_per_hour));
   hour_ts = (is_bad_data(hour_interval) ? bad_data_int :
              nint(hour_interval * sec_per_hour));
   
   // Search the files for the requested records
   for(i=0; i<climo_files.n(); i++) {
      read_climo_file(climo_files[i].c_str(), ctype, &i_dict, vld_ut,
                      day_ts, hour_ts, vx_grid, regrid_info, dpa);
   }
   
   // Time interpolation for climo fields
   dpa = climo_time_interp(dpa, day_ts, vld_ut, time_interp);

   mlog << Debug(3)
        << "Found " << dpa.n_planes() << " climatology fields.\n";

   return(dpa);
}

////////////////////////////////////////////////////////////////////////

void read_climo_file(const char *climo_file, GrdFileType ctype,
                     Dictionary *dict, unixtime vld_ut,
                     int day_ts, int hour_ts, const Grid &vx_grid,
                     const RegridInfo &regrid_info,
                     DataPlaneArray &dpa) {
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   VarInfoFactory info_factory;
   VarInfo *info = (VarInfo *) 0;

   DataPlaneArray cur_dpa;
   DataPlane dp;

   int n_climo, i;
   int vld_mon, vld_day, vld_yr, vld_hr, vld_min, vld_sec;
   int clm_mon, clm_day, clm_yr, clm_hr, clm_min, clm_sec;
   unixtime ut;

   ConcatString cur_ut_cs;

   // Allocate memory for data file
   if(!(mtddf = mtddf_factory.new_met_2d_data_file(climo_file, ctype))) {
      mlog << Warning << "\nread_climo_file() -> "
           << "Trouble reading climatology file \""
           << climo_file << "\"\n\n";
      return;
   }

   // Parse the variable name and level
   info = info_factory.new_var_info(mtddf->file_type());
   info->set_dict(*dict);

   // Read data planes
   n_climo = mtddf->data_plane_array(*info, cur_dpa);

   // Compute the valid month and day
   unix_to_mdyhms(vld_ut, vld_mon, vld_day, vld_yr,
                  vld_hr, vld_min, vld_sec);

   // Loop through matching records
   for(i=0; i<n_climo; i++) {

      // Store current valid time string
      cur_ut_cs = unix_to_yyyymmdd_hhmmss(cur_dpa[i].valid());

      // Compute the current month and day
      unix_to_mdyhms(cur_dpa[i].valid(), clm_mon, clm_day, clm_yr,
                     clm_hr, clm_min, clm_sec);

      // Recompute the unixtime to check the hour of the day.
      // Use the valid YYYYMMDD and climo HHMMSS.
      ut = mdyhms_to_unix(vld_mon, vld_day, vld_yr,
                          clm_hr, clm_min, clm_sec);

      // Check the hour time step.
      if(!is_bad_data(hour_ts) && labs(ut - vld_ut) >= hour_ts) {
         mlog << Debug(3) << "Skipping the " << cur_ut_cs << " \""
              << info->magic_str()
              << "\" climatology field since the time offset ("
              << labs(ut - vld_ut)
              << " seconds) >= the \"" << conf_key_hour_interval
              << "\" entry (" << hour_ts << " seconds) from file: "
              << climo_file << "\n";
         continue;
      }

      // Recompute the unixtime to check the day of the year.
      // Use the valid YYYY and climo MMDD_HHMMSS.
      ut = mdyhms_to_unix(clm_mon, clm_day, vld_yr,
                          clm_hr, clm_min, clm_sec);

      // Check the day time step.
      if(!is_bad_data(day_ts)) {

         // For daily climatology, check the hour timestep. 
         if(day_ts <= 3600*24 && labs(ut - vld_ut) >= hour_ts) {
            mlog << Debug(3) << "Skipping the " << cur_ut_cs << " \""
                 << info->magic_str()
                 << "\" climatology field since the time offset ("
                 << labs(ut - vld_ut) << " seconds) >= the \""
                 << conf_key_hour_interval << "\" entry (" << hour_ts
                 << " seconds) from daily climatology file: "
                 << climo_file << "\n";
            continue;
         }
         // For non-daily climatology, check the day timestep. 
         else if(labs(ut - vld_ut) >= day_ts) {
            mlog << Debug(3) << "Skipping the " << cur_ut_cs << " \""
                 << info->magic_str()
                 << "\" climatology field since the time offset ("
                 << labs(ut - vld_ut) << " seconds) >= the \""
                 << conf_key_day_interval << "\" entry (" << day_ts
                 << " seconds) from file: " << climo_file << "\n";
            continue;
         }
      }

      // Regrid, if needed
      if(!(mtddf->grid() == vx_grid)) {
         mlog << Debug(1)
              << "Regridding the " << cur_ut_cs << " \""
              << info->magic_str()
              << "\" climatology field to the verification grid.\n";
         dp = met_regrid(cur_dpa[i], mtddf->grid(), vx_grid,
                         regrid_info);
      }
      else {
         dp = cur_dpa[i];
      }

      // Set the climo time as valid YYYY and climo MMDD_HHMMSS.
      dp.set_valid(ut);

      // Store the match
      dpa.add(dp, cur_dpa.lower(i), cur_dpa.upper(i));

   } // end for i

   // Deallocate memory
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }
   if(info)  { delete info;  info  = (VarInfo       *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

DataPlaneArray climo_time_interp(const DataPlaneArray &dpa, int day_ts,
                                 unixtime vld_ut, InterpMthd mthd) {
   DataPlaneArray interp_dpa;
   ConcatString cs;
   int i;

   // Map and iterator
   map<ConcatString,IntArray> level_map;
   map<ConcatString,IntArray>::const_iterator it;

   // Construct map of unique level combinations to indices
   for(i=0; i<dpa.n_planes(); i++) {
      cs << cs_erase << dpa.lower(i) << ":" << dpa.upper(i);
      level_map[cs].add(i);
   }

   // Process each map entry
   for(it = level_map.begin(); it != level_map.end(); it++) {
      cs << cs_erase;
      for(i=0; i<it->second.n(); i++) {
         cs << unix_to_yyyymmdd_hhmmss(dpa[i].valid())
            << (i+1 == it->second.n() ? "" : ", ");
      }
      mlog << Debug(3)
           << "For forecast valid at "
           << unix_to_yyyymmdd_hhmmss(vld_ut) << ", found "
           << it->second.n()
           << " climatology field(s) with valid time(s): " << cs
           << "\n";

      // Check if day_ts is set to bad data.
      if(is_bad_data(day_ts)) {
         if(it->second.n() > 1) {
            mlog << Warning << "\nclimo_time_interp() -> "
                 << "Found multiple matches (" << it->second.n()
                 << ") for climatology field at "
                 << unix_to_yyyymmdd_hhmmss(vld_ut)
                 << ". Using the first match found.\n\n";
         }
         interp_dpa.add(dpa[it->second[0]],
                        dpa.lower(it->second[0]),
                        dpa.upper(it->second[0]));
      }
      // For a single field, just add it.
      else if(it->second.n() == 1) {
         interp_dpa.add(dpa[it->second[0]],
                        dpa.lower(it->second[0]),
                        dpa.upper(it->second[0]));
      }
      // For exactly 2 fields, do a simple time interpolation.
      else if(it->second.n() == 2) {
         mlog << Debug(3)
              << "Interpolating climatology fields at "
              << unix_to_yyyymmdd_hhmmss(dpa[it->second[0]].valid())
              << " and "
              << unix_to_yyyymmdd_hhmmss(dpa[it->second[1]].valid())
              << " to "
              << unix_to_yyyymmdd_hhmmss(vld_ut) << " using the "
              << interpmthd_to_string(mthd) << " interpolation method.\n";

         interp_dpa.add(valid_time_interp(
                        dpa[it->second[0]],
                        dpa[it->second[1]], vld_ut, mthd),
                        dpa.lower(it->second[0]),
                        dpa.upper(it->second[0]));
      }
      // For more than 2 fields interpolate as follows:
      // (1) Find closest hours before/after the valid time.
      // (2) Interpolate to the valid day for those hours before/after.
      // (3) Interpolate from hours before/after to the valid hour.
      else {

         // This should only occur when day_interval > 1.
         if(day_ts <= 3600*24) {
            mlog << Error << "\nclimo_time_interp() -> "
                 << "Expecting 1 or 2 climatology fields when \""
                 << conf_key_day_interval << "\" <= 1 but found "
                 << it->second.n() << "\n\n";
            exit(1);
         }

         // Get the number of HHMMSS seconds for the valid time.
         unixtime ut;
         int hms, prv_hms, nxt_hms, vld_hms;
         vld_hms = vld_ut % sec_per_day;

         // Find the nearest HHMMSS before/after the valid time.
         prv_hms = 0;
         nxt_hms = sec_per_day;
         for(i=0; i<it->second.n(); i++) {
            hms = dpa[it->second[i]].valid() % sec_per_day;
            if(hms <= vld_hms && hms > prv_hms) prv_hms = hms;
            if(hms >= vld_hms && hms < nxt_hms) nxt_hms = hms;
         }

         // For equality, do a single time interpolation.
         if(prv_hms == nxt_hms) {
             ut = (vld_ut / sec_per_day) + prv_hms;
             interp_dpa.add(climo_hms_interp(
                               dpa, it->second, ut, mthd),
                               dpa.lower(it->second[0]),
                               dpa.upper(it->second[0]));
         }
         // Otherwise, do multiple time interpolations.
         else {
            DataPlane prv_dp, nxt_dp;

            // Interpolate to the day for the previous hour.
            ut     = (vld_ut/sec_per_day)*sec_per_day + prv_hms;
            prv_dp = climo_hms_interp(dpa, it->second, ut, mthd);
         
            // Interpolate to the day for the next hour.
            ut     = (vld_ut/sec_per_day)*sec_per_day + nxt_hms;
            nxt_dp = climo_hms_interp(dpa, it->second, ut, mthd);

            mlog << Debug(3)
                 << "Interpolating climatology fields at "
                 << unix_to_yyyymmdd_hhmmss(prv_dp.valid()) << " and "
                 << unix_to_yyyymmdd_hhmmss(nxt_dp.valid()) << " to "
                 << unix_to_yyyymmdd_hhmmss(vld_ut) << " using the "
                 << interpmthd_to_string(mthd)
                 << " interpolation method.\n";

            // Interpolate to the valid hour.
            interp_dpa.add(valid_time_interp(
                              prv_dp, nxt_dp, vld_ut, mthd),
                              dpa.lower(it->second[0]),
                              dpa.upper(it->second[0]));
         } // end else
      } // end else
   } // end for it

   return(interp_dpa);
}

////////////////////////////////////////////////////////////////////////

DataPlane climo_hms_interp(const DataPlaneArray &dpa,
                           const IntArray &idx, unixtime vld_ut,
                           InterpMthd mthd) {
   int i, i_prv, i_nxt, dt, dt_prv, dt_nxt;
   DataPlane dp;
   int vld_hms = vld_ut % sec_per_day;

   // Find the closest times before/after the valid time.
   // Only consider dates matching the valid hms.
   dt_prv =  365*sec_per_day;
   dt_nxt = -365*sec_per_day;
   i_prv = i_nxt = bad_data_int;
   for(i=0; i<idx.n(); i++) {
      if(dpa[idx[i]].valid() % sec_per_day != vld_hms) continue;
      dt = vld_ut - dpa[idx[i]].valid();
      if(dt >= 0 && dt < dt_prv) { dt_prv = dt; i_prv = idx[i]; }
      if(dt <= 0 && dt > dt_nxt) { dt_nxt = dt; i_nxt = idx[i]; }
   }

   // Range check
   if(is_bad_data(i_prv) || is_bad_data(i_nxt)) {
      mlog << Error << "\nclimo_hms_interp() -> "
           << "Can't find climatology data before/after date "
           << unix_to_yyyymmdd_hhmmss(vld_ut) << ".\n\n";
      exit(1);
   }

   mlog << Debug(3)
        << "Interpolating climatology fields at "
        << unix_to_yyyymmdd_hhmmss(dpa[i_prv].valid()) << " and "
        << unix_to_yyyymmdd_hhmmss(dpa[i_nxt].valid()) << " to "
        << unix_to_yyyymmdd_hhmmss(vld_ut) << " using the "
        << interpmthd_to_string(mthd) << " interpolation method.\n";

   return(valid_time_interp(dpa[i_prv], dpa[i_nxt], vld_ut, mthd));
}

////////////////////////////////////////////////////////////////////////
