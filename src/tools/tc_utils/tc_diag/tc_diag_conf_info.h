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

#include "vx_config.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

struct TCDiagNcOutInfo {

   bool do_track;
   bool do_grid_latlon;
   bool do_grid_raw;
   bool do_cyl_latlon;
   bool do_cyl_raw;
   bool do_diag;

      //////////////////////////////////////////////////////////////////

   TCDiagNcOutInfo();
   TCDiagNcOutInfo & operator+=(const TCDiagNcOutInfo &);

   void clear();   //  sets everything to true

   bool all_false() const;

   void set_all_false();
   void set_all_true();
};

////////////////////////////////////////////////////////////////////////

class TCDiagDataOpt {

   private:

      void init_from_scratch();

   public:

      TCDiagDataOpt();
     ~TCDiagDataOpt();

      //////////////////////////////////////////////////////////////////

      // VarInfo pointer (allocated)
      VarInfo *var_info;

      // Range/Azimuth grid parameters
      int    n_range;
      int    n_azimuth;
      double max_range_km;
      double delta_range_km;
      double rmw_scale;

      // Output file options
      TCDiagNcOutInfo nc_info;

      //////////////////////////////////////////////////////////////////

      void clear();

      void process_config(GrdFileType, Dictionary &);
      void parse_nc_info(Dictionary &);
};

////////////////////////////////////////////////////////////////////////

class TCDiagConfInfo {

   private:

      int n_data;
      void init_from_scratch();

   public:

      TCDiagConfInfo();
      ~TCDiagConfInfo();

      //////////////////////////////////////////////////////////////////

      // TCDiag configuration object
      MetConfig conf;

      // Track line filtering criteria
      ConcatString model;
      ConcatString storm_id;
      ConcatString basin;
      ConcatString cyclone;
      unixtime     init_inc;
      unixtime     valid_beg, valid_end;
      TimeArray    valid_inc, valid_exc;
      NumArray     valid_hour;
      NumArray     lead_time;

      // Array of options for each data.field entry [n_data] (allocated)
      TCDiagDataOpt * data_opt;

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

      // Config file version
      ConcatString version;

      // Output file options
      TCDiagNcOutInfo nc_info;

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const char *, const char *);
      void process_config(GrdFileType);

      int get_n_data() const;
};

////////////////////////////////////////////////////////////////////////

inline int TCDiagConfInfo::get_n_data() const { return n_data; }

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
