// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "netcdf.hh"

#include "read_gridded_data.h"
#include "pair_data.h"
#include "read_grib.h"
#include "read_netcdf.h"
#include "grib_classes.h"
#include "vx_gdata.h"

///////////////////////////////////////////////////////////////////////////////

//
// Functions to read a single field from various input files
//
static bool read_field_grib(   const char *, GCInfo &, unixtime, int,
                               WrfData &, Grid &, int);
static bool read_field_pinterp(const char *, GCInfo &, unixtime, int,
                               WrfData &, Grid &, int);
static bool read_field_met(    const char *, GCInfo &, unixtime, int,
                               WrfData &, Grid &, int);
//
// Functions to read a range of fields from various input files
//
static int read_levels_grib(   const char *, GCInfo &, unixtime, int,
                               WrfData *&, NumArray &, Grid &, int);
static int read_levels_pinterp(const char *, GCInfo &, unixtime, int,
                               WrfData *&, NumArray &, Grid &, int);

///////////////////////////////////////////////////////////////////////////////

bool read_field(const char *file_name, GCInfo &gci,
                unixtime valid_ut, int lead_sec,
                WrfData &wd, Grid &gr, int verbosity) {
   FileType ftype = NoFileType;
   bool status = false;

   // Determine the file type
   ftype = get_file_type(file_name);

   // Switch based on the file type
   switch(ftype) {

      // GRIB file type
      case(GbFileType):

         status = read_field_grib(file_name, gci, valid_ut, lead_sec,
                                  wd, gr, verbosity);

         break;

      // NetCDF file type
      case(NcFileType):

         // Check for a pinterp file
         if(is_pinterp_file(file_name)) {
            status = read_field_pinterp(file_name, gci, valid_ut, lead_sec,
                                        wd, gr, verbosity);
         }
         // Assume this is NetCDF output from MET
         else {
            status = read_field_met(file_name, gci, valid_ut, lead_sec,
                                    wd, gr, verbosity);
         }
         break;

      default:
         cerr << "\n\nERROR: read_field() -> "
              << "unsupported file type: "
              << file_name << "\n\n" << flush;
         exit(1);
         break;
   }

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

bool read_field_grib(const char *file_name, GCInfo &gci,
                     unixtime valid_ut, int lead_sec,
                     WrfData &wd, Grid &gr, int verbosity) {
   GribFile gb_file;
   GribRecord rec;
   bool status = false;

   // Open the GRIB file
   if(!(gb_file.open(file_name))) {
      cerr << "\n\nERROR: read_field_grib() -> "
           << "can't open GRIB file: "
           << file_name << "\n\n" << flush;
      exit(1);
   }

   // Read the requested GRIB record
   status = get_grib_record(gb_file, rec, gci, valid_ut, lead_sec,
                            wd, gr, verbosity);

   // Close the file
   gb_file.close();

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

bool read_field_pinterp(const char *file_name, GCInfo &gci,
                        unixtime valid_ut, int lead_sec,
                        WrfData &wd, Grid &gr, int verbosity) {
   PinterpFile nc_file;
   ConcatString lvl_str, units_str;
   bool status = false;
   double pressure;
   char time_str[max_str_len], time2_str[max_str_len];

   // Open the p_interp NetCDF File
   if(!nc_file.open(file_name)) {
      cerr << "\n\nERROR: read_field_pinterp() -> "
           << "can't open p_interp NetCDF file: "
           << file_name << "\n\n" << flush;
      exit(1);
   }

   // Read the data
   status = nc_file.data(gci.abbr_str, gci.dim_la,
                         wd, pressure, units_str);

   if(status) {

      // Check that the valid time matches the request
      if(valid_ut > 0 && valid_ut != wd.get_valid_time()) {

         // Compute time strings
         unix_to_yyyymmdd_hhmmss(wd.get_valid_time(), time_str);
         unix_to_yyyymmdd_hhmmss(valid_ut,            time2_str);

         cerr << "\n\nERROR: read_field_pinterp() -> "
              << "the valid time for the " << gci.info_str
              << " variable in the p_interp NetCDF file "
              << file_name << " does not match the requested valid time: ("
              << time_str << " != " << time2_str << "\n\n" << flush;
         exit(1);
      }

      // Check that the lead time matches the request
      if(lead_sec > 0 && lead_sec != wd.get_lead_time()) {

         // Compute time strings
         sec_to_hhmmss(wd.get_valid_time(), time_str);
         sec_to_hhmmss(valid_ut,            time2_str);

         cerr << "\n\nERROR: read_field_pinterp() -> "
              << "the lead time for the " << gci.info_str
              << " variable in the p_interp NetCDF file "
              << file_name << " does not match the requested lead time: ("
              << time_str << " != " << time2_str << "\n\n" << flush;
         exit(1);
      }

      // Set the GCInfo object's level and units strings
      lvl_str.clear();
      if(!is_bad_data(pressure)) lvl_str << "P" << nint(pressure);
      else                       lvl_str << na_str;
      gci.set_lvl_str(lvl_str);
      if(strlen(units_str) > 0)  gci.set_units_str(units_str);
      else                       gci.set_units_str(na_str);
   }

   // Store the grid definition
   gr = nc_file.grid;

   // Close the file
   nc_file.close();

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

bool read_field_met(const char *file_name, GCInfo &gci,
                    unixtime valid_ut, int lead_sec,
                    WrfData &wd, Grid &gr, int verbosity) {
   MetNcFile nc_file;
   ConcatString lvl_str, units_str;
   bool status = false;
   char time_str[max_str_len], time2_str[max_str_len];

   // Open the MET NetCDF File
   if(!nc_file.open(file_name)) {
      cerr << "\n\nERROR: read_field_met() -> "
           << "can't open MET NetCDF file: "
           << file_name << "\n\n" << flush;
      exit(1);
   }

   // Read the data
   status = nc_file.data(gci.abbr_str, gci.dim_la,
                         wd, lvl_str, units_str);

   if(status) {

      // Check that the valid time matches the request
      if(valid_ut > 0 && valid_ut != wd.get_valid_time()) {

         // Compute time strings
         unix_to_yyyymmdd_hhmmss(wd.get_valid_time(), time_str);
         unix_to_yyyymmdd_hhmmss(valid_ut,            time2_str);

         cerr << "\n\nERROR: read_field_met() -> "
              << "the valid time for the " << gci.info_str
              << " variable in the MET NetCDF file "
              << file_name << " does not match the requested valid time: ("
              << time_str << " != " << time2_str << "\n\n" << flush;
         exit(1);
      }

      // Check that the lead time matches the request
      if(lead_sec > 0 && lead_sec != wd.get_lead_time()) {

         // Compute time strings
         sec_to_hhmmss(wd.get_valid_time(), time_str);
         sec_to_hhmmss(valid_ut,            time2_str);

         cerr << "\n\nERROR: read_field_met() -> "
              << "the lead time for the " << gci.info_str
              << " variable in the MET NetCDF file "
              << file_name << " does not match the requested lead time: ("
              << time_str << " != " << time2_str << "\n\n" << flush;
         exit(1);
      }

      // Set the GCInfo object's level and units string
      if(strlen(lvl_str) > 0)   gci.set_lvl_str(lvl_str);
      else                      gci.set_lvl_str(na_str);
      if(strlen(units_str) > 0) gci.set_units_str(units_str);
      else                      gci.set_units_str(na_str);
   }

   // Store the grid definition
   gr = nc_file.grid;

   // Close the file
   nc_file.close();

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

int read_field_levels(const char *file_name, GCInfo &gci,
                      unixtime valid_ut, int lead_sec,
                      WrfData *&wd, NumArray &lvl_na,
                      Grid &gr, int verbosity) {
   FileType ftype = NoFileType;
   int n_rec;
   bool status = false;

   // Determine the file type
   ftype = get_file_type(file_name);

   // Switch based on the file type
   switch(ftype) {

      // GRIB file type
      case(GbFileType):
         n_rec = read_levels_grib(file_name, gci, valid_ut, lead_sec,
                                  wd, lvl_na, gr, verbosity);
         break;

      // NetCDF file type
      case(NcFileType):

         // Check for a pinterp file
         if(is_pinterp_file(file_name)) {
            n_rec = read_levels_pinterp(file_name, gci, valid_ut, lead_sec,
                                        wd, lvl_na, gr, verbosity);
         }
         else {

            // Can only read data on a single level
            n_rec = 1;

            // Allocate space to store the fields
            wd = new WrfData [n_rec];

            // Read the requested NetCDF data
            status = read_field_met(file_name, gci, valid_ut, lead_sec,
                                    wd[0], gr, verbosity);

            if(!status) {
               cerr << "\n\nERROR: read_levels() -> "
                    << "no record matching "
                    << gci.info_str << " found in NetCDF file: "
                    << file_name << "\n" << flush;
               exit(1);
            }

            // Add a single level value
            lvl_na.add(bad_data_double);
         }

         break;

      default:
         cerr << "\n\nERROR: read_field_levels() -> "
              << "unsupported file type: "
              << file_name << "\n\n" << flush;
         exit(1);
         break;
   }

   return(n_rec);
}

///////////////////////////////////////////////////////////////////////////////

int read_levels_grib(const char *file_name, GCInfo &gci,
                     unixtime valid_ut, int lead_sec,
                     WrfData *&wd, NumArray &lvl_na,
                     Grid &gr, int verbosity) {
   GribFile gb_file;
   GribRecord rec;
   Grid data_grid;
   GCInfo gc_info;
   int i, n_rec;
   bool status;
   NumArray rec_na;

   // Initialize
   lvl_na.clear();

   // Open the GRIB file
   if(!(gb_file.open(file_name))) {
      cerr << "\n\nERROR: read_levels_grib() -> "
           << "can't open GRIB file: "
           << file_name << "\n\n" << flush;
      exit(1);
   }

   // Determine the number of records
   n_rec = find_grib_record_levels(gb_file, gci,
                                   valid_ut, lead_sec,
                                   rec_na, lvl_na);

   if(n_rec == 0) {
      cerr << "\n\nERROR: read_levels_grib() -> "
           << "no GRIB records matching "
           << gci.info_str << " found in GRIB file: "
           << file_name << "\n" << flush;
      exit(1);
   }

   // Allocate space to store the fields
   if(n_rec > 0) wd = new WrfData [n_rec];

   // Loop through and read each GRIB record
   for(i=0; i<n_rec; i++) {

      // Initialize the GCInfo object
      gc_info = gci;

      // Set the current level value for the record to retrieve.
      gc_info.lvl_1 = lvl_na[i];
      gc_info.lvl_2 = lvl_na[i];

      status = get_grib_record(gb_file, rec, gc_info,
                               valid_ut, lead_sec,
                               wd[i], data_grid, verbosity);

      if(!status) {
         cerr << "\n\nERROR: read_levels_grib() -> "
              << "no records matching GRIB code "
              << gc_info.code << " with level indicator of "
              << gc_info.lvl_str << " found in GRIB file: "
              << file_name << "\n" << flush;
         exit(1);
      }

      // Set the grid if unset
      if(gr.nx() == 0 && gr.ny() == 0) {
         gr = data_grid;
      }
      // Otherwise, check to make sure the grid doesn't change
      else {

         if(!(data_grid == gr)) {
            cerr << "\n\nERROR: read_levels_grib() -> "
                 << "The grid has changed for GRIB code "
                 << gc_info.code << ".\n\n"
                 << flush;
            exit(1);
         }
      }

   } // end for i

   // Close the file
   gb_file.close();

   return(n_rec);
}

///////////////////////////////////////////////////////////////////////////////

int read_levels_pinterp(const char *file_name, GCInfo &gci,
                        unixtime valid_ut, int lead_sec,
                        WrfData *&wd, NumArray &lvl_na, Grid &gr,
                        int verbosity) {
   PinterpFile nc_file;
   ConcatString lvl_str, units_str;
   bool found;
   int i_dim, i, n_lvl, lvl_min, lvl_max;
   double pressure;
   LongArray lvl_dim_la;
   bool status = false;
   char time_str[max_str_len], time2_str[max_str_len];

   // Open the p_interp NetCDF File
   if(!nc_file.open(file_name)) {
      cerr << "\n\nERROR: read_field_pinterp() -> "
           << "can't open p_interp NetCDF file: "
           << file_name << "\n\n" << flush;
      exit(1);
   }

   // Find the dimension that has the range flag set
   for(i_dim=0; i_dim<gci.dim_la.n_elements(); i_dim++) {
      if(gci.dim_la[i_dim] == range_flag) {
         found = true;
         break;
      }
   }

   // Compute the number of levels
   n_lvl = gci.lvl_2 - gci.lvl_1 + 1;

   // Allocate space to store the fields
   if(n_lvl > 0) wd = new WrfData [n_lvl];

   // Initialize
   lvl_dim_la = gci.dim_la;
   lvl_na.clear();

   // Loop through each of levels specified in the range
   for(i=0; i<n_lvl; i++) {

      // Set the current level
      if(found) lvl_dim_la[i_dim] = gci.lvl_1 + i;

      // Read the data
      status = nc_file.data(gci.abbr_str, lvl_dim_la,
                            wd[i], pressure, units_str);

      // Check for bad status
      if(!status) {
         cerr << "\n\nERROR: read_levels_pinterp() -> "
              << "error reading " << gci.info_str
              << " from the p_interp NetCDF file " << file_name
              << "\n\n" << flush;
      }

      // Check that the valid time matches the request
      if(valid_ut > 0 && valid_ut != wd[i].get_valid_time()) {

         // Compute time strings
         unix_to_yyyymmdd_hhmmss(wd[i].get_valid_time(), time_str);
         unix_to_yyyymmdd_hhmmss(valid_ut,               time2_str);

         cerr << "\n\nERROR: read_levels_pinterp() -> "
              << "the valid time for the " << gci.info_str
              << " variable in the p_interp NetCDF file "
              << file_name << " does not match the requested valid time: ("
              << time_str << " != " << time2_str << "\n\n" << flush;
         exit(1);
      }

      // Check that the lead time matches the request
      if(lead_sec > 0 && lead_sec != wd[i].get_lead_time()) {

         // Compute time strings
         sec_to_hhmmss(wd[i].get_valid_time(), time_str);
         sec_to_hhmmss(valid_ut,               time2_str);

         cerr << "\n\nERROR: read_levels_pinterp() -> "
              << "the lead time for the " << gci.info_str
              << " variable in the p_interp NetCDF file "
              << file_name << " does not match the requested lead time: ("
              << time_str << " != " << time2_str << "\n\n" << flush;
         exit(1);
      }

      // Store the pressure level
      lvl_na.add(pressure);

   } // end for i

   // Set the GCInfo object's level and units strings
   lvl_str.clear();
   lvl_min = nint(lvl_na.min());
   lvl_max = nint(lvl_na.max());
   if(lvl_min == lvl_max) lvl_str << "P" << lvl_min;
   else                   lvl_str << "P" << lvl_max << "-" << lvl_min;
   gci.set_lvl_str(lvl_str);
   if(strlen(units_str) > 0) gci.set_units_str(units_str);
   else                      gci.set_units_str(na_str);

   // Store the grid definition
   gr = nc_file.grid;

   // Close the file
   nc_file.close();

   return(n_lvl);
}

///////////////////////////////////////////////////////////////////////////////
