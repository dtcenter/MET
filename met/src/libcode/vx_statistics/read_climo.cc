// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
          bool, bool, int,
          const Grid &, const RegridInfo &, DataPlaneArray &dpa);

static DataPlaneArray climo_time_interp(
          const DataPlaneArray &, unixtime, InterpMthd);

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
   bool match_month, match_day;
   int i, time_step;

   // Check for null
   if(!dict) return(dpa);

   // Get the i-th array entry
   Dictionary i_dict = parse_conf_i_vx_dict(dict, i_vx);

   // Climatology mean and standard deviation files
   climo_files = i_dict.lookup_string_array(conf_key_file_name, false);

   // Check for at least one file
   if(climo_files.n_elements() == 0) return(dpa);

   // Regrid info
   regrid_info = parse_conf_regrid(&i_dict);

   // Time interpolation
   time_interp = int_to_interpmthd(i_dict.lookup_int(conf_key_time_interp_method));

   // Match month flag
   match_month = i_dict.lookup_bool(conf_key_match_month);

   // Match day flag
   match_day = i_dict.lookup_bool(conf_key_match_day);

   // Time step in seconds
   time_step = i_dict.lookup_int(conf_key_time_step);

   // Check if file_type was specified
   ctype = parse_conf_file_type(&i_dict);

   // Search the files for the requested records
   for(i=0; i<climo_files.n_elements(); i++) {
      read_climo_file(climo_files[i].c_str(), ctype, &i_dict, vld_ut,
                      match_month, match_day, time_step,
                      vx_grid, regrid_info, dpa);
   }

   // Time interpolation for mean fields
   dpa = climo_time_interp(dpa, vld_ut, time_interp);

   mlog << Debug(3)
        << "Found " << dpa.n_planes() << " climatology fields.\n";

   return(dpa);
}

////////////////////////////////////////////////////////////////////////

void read_climo_file(const char *climo_file, GrdFileType ctype,
                     Dictionary *dict, unixtime vld_ut,
                     bool match_month, bool match_day, int time_step,
                     const Grid &vx_grid, const RegridInfo &regrid_info,
                     DataPlaneArray &dpa) {
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   VarInfoFactory info_factory;
   VarInfo *info = (VarInfo *) 0;

   DataPlaneArray cur_dpa;
   DataPlane dp;

   int n_climo, i;
   int vld_month, vld_day, vld_year;
   int month, day, year, hour, minute, second;
   unixtime cur_ut;

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
   unix_to_mdyhms(vld_ut, vld_month, vld_day, vld_year, hour, minute, second);

   // Loop through matching records
   for(i=0; i<n_climo; i++) {

      // Compute the current month and day
      unix_to_mdyhms(cur_dpa[i].valid(), month, day, year, hour, minute, second);

      // Check for matching month
      if(match_month && month != vld_month) {
         mlog << Debug(3)
              << "Skipping climatology field since \""
              << conf_key_match_month << "\" is TRUE but the forecast ("
              << unix_to_yyyymmdd_hhmmss(vld_ut) << ") and climatology ("
              << unix_to_yyyymmdd_hhmmss(cur_dpa[i].valid())
              << ") months do not match for file: " << climo_file
              << "\n";
         continue;
      }

      // Check for matching day
      if(match_day && day != vld_day) {
         mlog << Debug(3)
              << "Skipping climatology field since \""
              << conf_key_match_day << "\" is TRUE but the forecast ("
              << unix_to_yyyymmdd_hhmmss(vld_ut) << ") and climatology ("
              << unix_to_yyyymmdd_hhmmss(cur_dpa[i].valid())
              << ") days do not match for file: " << climo_file
              << "\n";
         continue;
      }

      // Recompute the unixtime using the valid year, month, and day
      cur_ut = mdyhms_to_unix(vld_month, vld_day, vld_year, hour, minute, second);

      // Check the time step
      if(labs(cur_ut - vld_ut) >= time_step) {
         mlog << Debug(3) << "Skipping climatology field since time offset "
              << "exceeds the specified \"" << conf_key_time_step << "\" ("
              << labs(cur_ut - vld_ut) << " >= " << time_step << ") from file: "
              << climo_file << "\n";
         continue;
      }

      // Regrid, if needed
      if(!(mtddf->grid() == vx_grid)) {
         mlog << Debug(1)
              << "Regridding climatology " << info->magic_str()
              << " to the verification grid.\n";
         dp = met_regrid(cur_dpa[i], mtddf->grid(), vx_grid, regrid_info);
      }
      else {
         dp = cur_dpa[i];
      }

      // Set the valid time as the reference time
      dp.set_valid(cur_ut);

      // Store the match
      dpa.add(dp, cur_dpa.lower(i), cur_dpa.upper(i));

   } // end for i

   // Deallocate memory
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }
   if(info)  { delete info;  info  = (VarInfo       *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

DataPlaneArray climo_time_interp(const DataPlaneArray &dpa,
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
      for(i=0; i<it->second.n_elements(); i++) {
         cs << unix_to_yyyymmdd_hhmmss(dpa[i].valid())
            << (i+1 == it->second.n_elements() ? "" : ", ");
      }
      mlog << Debug(3)
           << "For forecast valid at " << unix_to_yyyymmdd_hhmmss(vld_ut)
           << ", found " << it->second.n_elements()
           << " climatology field(s) valid at " << cs << ".\n";

      // Add a single field
      if(it->second.n_elements() == 1) {
         interp_dpa.add(dpa[it->second[0]],
                        dpa.lower(it->second[0]),
                        dpa.upper(it->second[0]));
      }
      // Do time interpolation
      else if(it->second.n_elements() == 2) {
         mlog << Debug(3)
              << "Interpolating climatology data at "
              << unix_to_yyyymmdd_hhmmss(dpa[it->second[0]].valid())
              << " and "
              << unix_to_yyyymmdd_hhmmss(dpa[it->second[1]].valid())
              << " to "
              << unix_to_yyyymmdd_hhmmss(vld_ut) << " using the "
              << interpmthd_to_string(mthd)
              << " interpolation method.\n";

         interp_dpa.add(valid_time_interp(
                        dpa[it->second[0]],
                        dpa[it->second[1]], vld_ut, mthd),
                        dpa.lower(it->second[0]),
                        dpa.upper(it->second[0]));
      }
      // Error out
      else {
         mlog << Error << "\nclimo_time_interp() -> "
              << "Expecting 1 or 2 climatology fields but found "
              << it->second.n_elements() << "\n\n";
         exit(1);
      }
   } // end for it

   return(interp_dpa);
}

////////////////////////////////////////////////////////////////////////
