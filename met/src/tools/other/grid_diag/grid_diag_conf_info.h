// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

      int n_data; // Number of data fields

   public:

      // Grid Diagnostics configuration object
      MetConfig conf;

      ConcatString version;        // Config file version
      ConcatString desc;           // Data description

      VarInfo ** data_info;        // Pointer array for data VarInfo [n_data]

      ConcatString mask_grid_file; // Path for masking grid area
      ConcatString mask_grid_name; // Name of masking grid area
      ConcatString mask_poly_file; // Path for masking poly area
      ConcatString mask_poly_name; // Name of masking poly area
      MaskPlane    mask_area;

      GridDiagConfInfo();
      ~GridDiagConfInfo();

      void clear();

      void read_config(const char *, const char *);
      void set_n_data();
      void process_config(vector<GrdFileType>);
      void process_masks(const Grid &);

      int get_n_data() const;
      int get_compression_level();
};

////////////////////////////////////////////////////////////////////////

inline int GridDiagConfInfo::get_n_data() const { return(n_data); }
inline int GridDiagConfInfo::get_compression_level() { return conf.nc_compression(); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __GRID_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
