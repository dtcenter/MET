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

      // NetCDF dimensions
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
      bool nc_cyl_coord_flag;
      bool nc_diag_flag;
      bool cira_diag_flag;

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const char *, const char *);
      void process_config(GrdFileType,
                          std::map<std::string,StringArray>);

      void parse_domain_info_map(std::map<std::string,StringArray>);

      int get_n_domain() const;
};

////////////////////////////////////////////////////////////////////////

inline int TCDiagConfInfo::get_n_domain() const { return domain_info_map.size(); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////