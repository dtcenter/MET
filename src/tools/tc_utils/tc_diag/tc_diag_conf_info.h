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
#include <vector>

#include "vx_config.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

struct TCDiagNcOutInfo {

   bool do_latlon;
   bool do_fcst_genesis;
   bool do_fcst_tracks;
   bool do_fcst_fy_oy;
   bool do_fcst_fy_on;
   bool do_best_genesis;
   bool do_best_tracks;
   bool do_best_fy_oy;
   bool do_best_fn_oy;

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

      void process_config(GrdFileType, Dictionary &,
                          GrdFileType, Dictionary &);
      void parse_nc_info(Dictionary &);
};

////////////////////////////////////////////////////////////////////////

class TCDiagConfInfo {

   private:

      void init_from_scratch();

   public:

      TCDiagConfInfo();
      ~TCDiagConfInfo();

      //////////////////////////////////////////////////////////////////

      // TCDiag configuration object
      MetConfig Conf;

      // Track line filtering criteria
      ConcatString Model;
      ConcatString StormId;
      ConcatString Basin;
      ConcatString Cyclone;
      unixtime     InitInc;
      unixtime     ValidBeg, ValidEnd;
      TimeArray    ValidInc, ValidExc;
      NumArray     ValidHour;
      NumArray     LeadTime;

      // Vector of input data
      vector<TCDiagDataOpt> DataOpt;

      // Wind conversion information
      bool compute_tangential_and_radial_winds;
      ConcatString u_wind_field_name;
      ConcatString v_wind_field_name;
      ConcatString tangential_velocity_field_name;
      ConcatString radial_velocity_field_name;
      ConcatString tangential_velocity_long_field_name;
      ConcatString radial_velocity_long_field_name;

      // Variable information
      VarInfo** data_info;

      ConcatString   tmp_dir;               // Directory for temporary files
      ConcatString   output_prefix;         // String to customize output file name
      ConcatString   version;               // Config file version

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const char *, const char *);
      void process_config(GrdFileType);

      int get_n_data() const;
};

////////////////////////////////////////////////////////////////////////

inline int TCDiagConfInfo::get_n_data() const { return DataOpt.size(); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
