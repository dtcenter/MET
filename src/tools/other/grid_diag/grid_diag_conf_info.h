// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __GRID_DIAG_CONF_INFO_H__
#define  __GRID_DIAG_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_gsl_prob.h"
#include "vx_statistics.h"
#include "vx_stat_out.h"

////////////////////////////////////////////////////////////////////////

struct GridDiagNcOutInfo {

   bool do_hist1d;
   bool do_hist2d;
   bool do_info_theory;

   GridDiagNcOutInfo();

   void clear();   // sets everything to true

   bool all_false() const;

   void set_all_false();
   void set_all_true();
};

////////////////////////////////////////////////////////////////////////

class GridDiagConfInfo {

   private:

      void init_from_scratch();

   public:

      // Grid Diagnostics configuration object
      MetConfig conf;

      ConcatString version;        // Config file version
      ConcatString desc;           // Data description

      std::vector<VarInfo *> data_info; // VarInfo pointer array [n_data]

      // Masking region names and MaskPlanes
      StringArray mask_name;
      std::vector<MaskPlane> mask_mp;

      GridDiagNcOutInfo nc_info;   // Output NetCDF file contents

      GridDiagConfInfo();
      ~GridDiagConfInfo();

      void clear();

      void read_config(const char *, const char *);
      void set_n_data();
      void process_config(std::vector<GrdFileType>);
      void parse_output_flag();
      void process_masks(const Grid &);

      int get_n_mask() const;
      int get_n_data() const;
      int get_compression_level();
};

////////////////////////////////////////////////////////////////////////

inline int GridDiagConfInfo::get_n_mask() const { return mask_name.n(); }
inline int GridDiagConfInfo::get_n_data() const { return (int) data_info.size(); }
inline int GridDiagConfInfo::get_compression_level() { return conf.nc_compression(); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __GRID_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
