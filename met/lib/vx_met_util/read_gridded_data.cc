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

#include "vx_met_util/read_gridded_data.h"
#include "vx_met_util/pair_data.h"
#include "vx_met_util/read_grib.h"
#include "vx_met_util/read_netcdf.h"
#include "vx_grib_classes/grib_classes.h"
#include "vx_gdata/vx_gdata.h"

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

int read_field(const char *file_name, GCInfo &gci,
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
   Section1_Header *pds_ptr;
   char lvl_str[max_str_len];
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

   // Get the level name
   pds_ptr = (Section1_Header *) rec.pds;
   get_grib_level_str(pds_ptr->type, pds_ptr->level_info, lvl_str);

   // Set the GCInfo object's level string
   gci.set_lvl_str(lvl_str);

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

bool read_field_pinterp(const char *file_name, GCInfo &gci,
                        unixtime valid_ut, int lead_sec,
                        WrfData &wd, Grid &gr, int verbosity) {
   PinterpFile p_file;
   char lvl_str[max_str_len];
   double prs;
   bool status = false;
   Pgm image;
// JHG, PPMImage needs to change to WrfData

   // Open the p_interp NetCDF File
   if(!p_file.open(file_name)) {
      cerr << "\n\nERROR: read_field_pinterp() -> "
           << "can't open p_interp NetCDF file: "
           << file_name << "\n\n" << flush;
      exit(1);
   }

   // Read the data
   status = p_file.data(gci.abbr_str.text(), gci.dim_la, image, prs);

   // Set the GCInfo object's level string
   gci.set_lvl_str(lvl_str);

   // Close the file
   p_file.close();

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

// JHG, this function will change with changes from Randy

bool read_field_met(const char *file_name, GCInfo &gci,
                    unixtime valid_ut, int lead_sec,
                    WrfData &wd, Grid &gr, int verbosity) {
   NcFile *nc_file = (NcFile *) 0;
   char lvl_str[max_str_len];
   bool status = false;

   // Open the NetCDF File
   nc_file = new NcFile(file_name);

   if(!nc_file->is_valid()) {
      cerr << "\n\nERROR: read_field_met() -> "
           << "can't open NetCDF file: "
           << file_name << "\n\n" << flush;
      exit(1);
   }

   // Read the requested NetCDF data
   status = (read_netcdf_status(nc_file, gci.abbr_str.text(),
                                lvl_str, wd, gr, verbosity) == 0);

   // Set the GCInfo object's level string
   gci.set_lvl_str(lvl_str);

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
                                    wd[1], gr, verbosity);
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
   int i, n_rec, status;
   NumArray rec_na;

   // Initialize
   lvl_na.clear();

   // JHG work here and read multiple records

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

      if(status != 0) {
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

   return(n_rec);
}

///////////////////////////////////////////////////////////////////////////////

int read_levels_pinterp(const char *file_name, GCInfo &gci,
                        unixtime valid_ut, int lead_sec,
                        WrfData *&wd, NumArray &lvl_na,
                        Grid &gr, int verbosity) {

// JHG, work here

   cerr << "\n\nERROR: read_levels_pinterp() -> "
        << "SHOULD NOT BE CALLED YET!\n\n" << flush;
   exit(1);

   GribFile gb_file;
   GribRecord rec;
   Grid data_grid;
   GCInfo gc_info;
   int i, n_rec, status;
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

      if(status != 0) {
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

   return(n_rec);
}

///////////////////////////////////////////////////////////////////////////////
