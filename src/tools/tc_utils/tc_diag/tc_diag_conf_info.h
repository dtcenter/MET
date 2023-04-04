// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_DIAG_CONF_INFO_H__
#define  __TC_DIAG_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>
#include <netcdf>

#include "vx_config.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

struct TCDiagNcOutInfo {

   bool do_track;
   bool do_diag;
   bool do_grid_latlon;
   bool do_grid_raw;
   bool do_cyl_latlon;
   bool do_cyl_raw;

      //////////////////////////////////////////////////////////////////

   TCDiagNcOutInfo();
   TCDiagNcOutInfo & operator+=(const TCDiagNcOutInfo &);

   void clear();

   bool all_false() const;

   void set_all_false();
   void set_all_true();
};

////////////////////////////////////////////////////////////////////////

class TCDiagDomainInfo {

   private:

      void init_from_scratch();

   public:

      TCDiagDomainInfo();
      ~TCDiagDomainInfo();

      // TcrmwData structure for creating a TcrmwGrid object
      TcrmwData data;
      double delta_range_km;

      // Domain data files
      StringArray data_files;

      // Vector of VarInfo pointers (not allocated)
      std::vector<VarInfo *> var_info_ptr;

      // Diagnostic scripts to be run
      StringArray diag_script;

      // Flag to write this grid to the output
      bool write_nc;

      // Corresponding NetCDF dimensions
      netCDF::NcDim range_dim;
      netCDF::NcDim azimuth_dim;

      //////////////////////////////////////////////////////////////////

      void clear();

      void parse_domain_info(Dictionary &, ConcatString &);

      int get_n_data() const;
};

////////////////////////////////////////////////////////////////////////

inline int TCDiagDomainInfo::get_n_data() const { return var_info_ptr.size(); }

////////////////////////////////////////////////////////////////////////

class TCDiagConfInfo {

   private:

      void init_from_scratch();

   public:

      TCDiagConfInfo();
      ~TCDiagConfInfo();

      //////////////////////////////////////////////////////////////////

      // TCDiag configuration object
      MetConfig conf;

      // Track line filtering criteria
      StringArray  model;
      ConcatString storm_id;
      ConcatString basin;
      ConcatString cyclone;
      unixtime     init_inc;
      unixtime     valid_beg, valid_end;
      TimeArray    valid_inc, valid_exc;
      NumArray     valid_hour;
      NumArray     lead_time;

      // Vector of VarInfo objects from data.field (allocated)
      std::vector<VarInfo *> var_info;

      // Mapping of domain name to TCDiagDomainInfo
      std::map<std::string,TCDiagDomainInfo> domain_info_map;

      // Wind conversion information
      bool compute_tangential_and_radial_winds;
      ConcatString u_wind_field_name;
      ConcatString v_wind_field_name;
      ConcatString tangential_velocity_field_name;
      ConcatString radial_velocity_field_name;
      ConcatString tangential_velocity_long_field_name;
      ConcatString radial_velocity_long_field_name;

      // Directory for temporary files
      ConcatString tmp_dir;

      // String to customize output file name
      ConcatString output_prefix;

      // Output file options
      TCDiagNcOutInfo nc_diag_info;
      bool cira_diag_flag;

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const char *, const char *);
      void process_config(GrdFileType,
                          std::map<std::string,StringArray>);

      void parse_domain_info_map(std::map<std::string,StringArray>);
      void parse_nc_diag_info();

      int get_n_domain() const;
};

////////////////////////////////////////////////////////////////////////

inline int TCDiagConfInfo::get_n_domain() const { return domain_info_map.size(); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
