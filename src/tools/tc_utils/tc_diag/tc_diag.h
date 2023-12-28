// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_diag.h
//
//   Description:
//
//   Mod#  Date      Name          Description
//   ----  ----      ----          -----------
//   000   09/27/22  Halley Gotway New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __TC_DIAG_H__
#define  __TC_DIAG_H__

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <netcdf>

#include "tc_diag_conf_info.h"

#include "vx_data2d_factory.h"
#include "vx_tc_util.h"
#include "vx_grid.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

// Program name
static const char* program_name = "tc_diag";

// ATCF file suffix
static const char* atcf_suffix = ".dat";

// Default configuration file name
static const char* default_config_filename =
   "MET_BASE/config/TCDiagConfig_default";

// Default output directory
static const char* default_out_dir = ".";

// Output file suffix names
static const char* nc_cyl_grid_suffix = "_cyl_grid_{domain}.nc";
static const char* nc_diag_suffix     = "_diag.nc";
static const char* cira_diag_suffix   = "_diag.dat";

// Default output prefix
static const char* default_out_prefix = "";

// Diagnostics bad data value
static const double diag_bad_data_double = 9999.0;

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input files
static std::map<std::string,DataOptInfo> data_opt_map;
static StringArray    deck_source, deck_model_suffix;
static ConcatString   config_file;
static TCDiagConfInfo conf_info;
static GrdFileType    file_type = FileType_None;

// Optional arguments
static ConcatString out_dir;
static ConcatString out_prefix;

////////////////////////////////////////////////////////////////////////
//
// Variables for Temp Files
//
////////////////////////////////////////////////////////////////////////

class TmpFileInfo {

   private:

      void init_from_scratch();

   public:

      TmpFileInfo();
      ~TmpFileInfo();

      //////////////////////////////////////////////////////////////////

      // Keep the temp file rather than deleting it
      bool keep_file_flag;

      // Track information
      const TrackInfo  *trk_ptr; // not allocated
      const TrackPoint *pnt_ptr; // not allocated

      // Vector of diagnostic names to track insertion order
      std::vector<std::string> diag_storm_keys;
      std::vector<std::string> diag_sounding_keys;
      std::vector<std::string> diag_custom_keys;

      // Mappings of diagnostic names to values
      std::map<std::string,double> diag_storm_map;
      std::map<std::string,double> diag_sounding_map;
      std::map<std::string,double> diag_custom_map;

      // Mappings of diagnostics names to units, long names,
      // and source domains
      std::map<std::string,std::string> diag_units_map;
      std::map<std::string,std::string> diag_long_name_map;
      std::map<std::string,std::string> diag_domain_map;

      // Array of comment lines
      StringArray comment_lines;

      // Set of unique pressure levels
      std::set<double> pressure_levels;

      // Range azimuth grid
      Grid      grid_out;
      TcrmwGrid ra_grid;

      // Domain name
      std::string domain;

      // NetCDF Cylindrical Coordinates output
      ConcatString    tmp_file;
      netCDF::NcFile *tmp_out;

      // NetCDF Dimensions
      netCDF::NcDim trk_dim;
      netCDF::NcDim vld_dim;
      netCDF::NcDim rng_dim;
      netCDF::NcDim azi_dim;
      netCDF::NcDim prs_dim;

      void open(const TrackInfo *, const TrackPoint *,
                const DomainInfo &, const std::set<double> &,
                const bool);
      void close();

      void clear();

      void setup_nc_file(const DomainInfo &,
                         const std::set<double> &);

      void write_nc_data(const VarInfo *, const DataPlane &,
                         const Grid &);
};

static std::map<std::string,TmpFileInfo> tmp_file_map;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

static unixtime init_ut = (unixtime) 0;

class OutFileInfo {

   private:

      void init_from_scratch();
      void add_diag_data(const std::vector<std::string> &,
                         const std::map<std::string,double> &,
                         std::vector<std::string> &,
                         std::map<std::string,NumArray> &,
                         const StringArray &, const std::string &,
                         int);
      void add_diag_meta(const std::map<std::string,std::string> &,
                         std::map<std::string,std::string> &);
      void set_diag_comments(const StringArray &);

   public:

      OutFileInfo();
      ~OutFileInfo();

      //////////////////////////////////////////////////////////////////

      // Track information
      const TrackInfo *trk_ptr; // not allocated

      // Vector of diagnostic names to track insertion order
      std::vector<std::string> diag_storm_keys;
      std::vector<std::string> diag_sounding_keys;
      std::vector<std::string> diag_custom_keys;

      // Mappings of diagnostic names to values
      // for each track point
      std::map<std::string,NumArray> diag_storm_map;
      std::map<std::string,NumArray> diag_sounding_map;
      std::map<std::string,NumArray> diag_custom_map;

      // Mappings of diagnostics names to units, long names,
      // and source domains
      std::map<std::string,std::string> diag_units_map;
      std::map<std::string,std::string> diag_long_name_map;
      std::map<std::string,std::string> diag_domain_map;

      // Array of comment lines
      StringArray comment_lines;

      // NetCDF Diagnostics output
      ConcatString    nc_diag_file;
      netCDF::NcFile *nc_diag_out;

      // NetCDF Dimensions
      netCDF::NcDim vld_dim;
      netCDF::NcDim prs_dim;

      // CIRA Diagnostics output
      ConcatString   cira_diag_file;
      std::ofstream *cira_diag_out;

      void clear();

      netCDF::NcFile *setup_nc_file(const std::string &);
      void add_tmp_file_info(const TmpFileInfo &, const StringArray &, int);
      void write_nc_diag();
      void write_nc_domain_info(const DomainInfo &);
      void write_nc_diag_vals(const std::string &, NumArray &);
      void write_nc_diag_prs_vals(const std::string &, const float *);

      void write_cira_diag();
      void write_cira_diag_section_header(const char *);
      void write_cira_diag_vals(std::vector<std::string> &,
              std::map<std::string,NumArray> &, bool);

      std::string get_diag_units(const std::string &);

      int n_diag() const;
};

////////////////////////////////////////////////////////////////////////

inline int OutFileInfo::n_diag() const { return(diag_storm_keys.size()    +
                                                diag_sounding_keys.size() +
                                                diag_custom_keys.size()); }

static std::map<std::string,OutFileInfo> out_file_map;

////////////////////////////////////////////////////////////////////////

#endif  //  __TC_DIAG_H__

////////////////////////////////////////////////////////////////////////
