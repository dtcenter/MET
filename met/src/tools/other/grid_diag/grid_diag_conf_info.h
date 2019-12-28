// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

class GridDiagConfInfo {

   private:

      void init_from_scratch();

      // Counts based on the contents of the config file
      int n_fcst;                          // Number of forecast fields
      int n_data;                          // Number of data fields

   public:

      // Series-Analysis configuration object
      MetConfig conf;

      // Store data parsed from the Series-Analysis configuration object
      ConcatString     model;              // Model name

      VarInfo **       fcst_info;          // Array of pointers for fcst VarInfo [n_fcst]
      VarInfo **       data_info;          // Array of pointers for data VarInfo [n_data]

      ConcatString     mask_grid_file;     // Path for masking grid area
      ConcatString     mask_grid_name;     // Name of masking grid area
      ConcatString     mask_poly_file;     // Path for masking poly area
      ConcatString     mask_poly_name;     // Name of masking poly area
      MaskPlane        mask_area;

      ConcatString     version;            // Config file version

      GridDiagConfInfo();
     ~GridDiagConfInfo();

      void clear();

      void read_config   (const char *, const char *);
      void process_config(GrdFileType, GrdFileType);
      void process_masks (const Grid &);
      int get_compression_level();

      // Dump out the counts
      int get_n_fcst() const;
};

////////////////////////////////////////////////////////////////////////

inline int GridDiagConfInfo::get_n_fcst() const { return(n_fcst); }
inline int GridDiagConfInfo::get_compression_level()  { return conf.nc_compression(); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __GRID_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
