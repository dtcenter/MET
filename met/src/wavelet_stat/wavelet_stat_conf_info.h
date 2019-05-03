// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __WAVELET_STAT_CONF_INFO_H__
#define  __WAVELET_STAT_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "wavelet_stat_Conf.h"

#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_met_util/vx_met_util.h"
#include "vx_data_grids/grid.h"
#include "vx_util/vx_util.h"
#include "vx_cal/vx_cal.h"
#include "vx_math/vx_math.h"
#include "vx_contable/vx_contable.h"
#include "vx_gsl_prob/vx_gsl_prob.h"
#include "vx_econfig/result.h"

////////////////////////////////////////////////////////////////////////

// Indices for the output flag types in the configuration file
static const int i_isc    = 0;
static const int i_nc     = 1;
static const int i_ps     = 2;
static const int n_out    = 3;

// Enumeration to store possible output flag values
enum OutputFlag {
   flag_no_out   = 0,
   flag_stat_out = 1,
   flag_txt_out  = 2
};

////////////////////////////////////////////////////////////////////////

class WaveletStatConfInfo {

   private:

      void init_from_scratch();

      // Counts based on the contents of the config file
      int n_vx;              // Number of fields to be verified
      int max_n_thresh;      // Maximum number of thresholds
      int n_tile;            // Number of tiles to apply
      int tile_dim;          // Tile dimension
      int n_scale;           // Number of scales based on tile_dim

   public:

      // Wavelet-Stat configuration object
      wavelet_stat_Conf conf;

      // Pointer for the wavelet and wavelet workspace to be used
      gsl_wavelet           *wvlt_ptr;
      gsl_wavelet_workspace *wvlt_work_ptr;

      // Various objects to store the data that's parsed from the
      // Wavelet-Stat configuration object
      GCInfo      *gci;         // Array for verification fields [n_vx]
      ThreshArray *ta;          // Array for thresholds [n_vx]

      NumArray     tile_xll;    // Array of lower-left x coordinates
      NumArray     tile_yll;    // Array of lower-left y coordinates
      BoundingBox  pad_bb;      // Pad bouding box

      WaveletStatConfInfo();
     ~WaveletStatConfInfo();

      void clear();

      void read_config   (const char *);
      void process_config();
      void process_tiles (const Grid &, int);
      void center_tiles  (int, int);
      void pad_tiles     (int, int);

      // Dump out the counts
      int get_n_vx        () const;
      int get_max_n_thresh() const;
      int get_n_tile      () const;
      int get_tile_dim    () const;
      int get_n_scale     () const;

      // Compute the maximum number of output lines possible based
      // on the contents of the configuration file
      int n_isc_row ();
      int n_stat_row();
};

////////////////////////////////////////////////////////////////////////

inline int WaveletStatConfInfo::get_n_vx()         const { return(n_vx);          }
inline int WaveletStatConfInfo::get_max_n_thresh() const { return(max_n_thresh);  }
inline int WaveletStatConfInfo::get_n_tile()       const { return(n_tile);        }
inline int WaveletStatConfInfo::get_tile_dim()     const { return(tile_dim);      }
inline int WaveletStatConfInfo::get_n_scale()      const { return(n_scale);      }

////////////////////////////////////////////////////////////////////////

extern int get_pow2(double);

////////////////////////////////////////////////////////////////////////

#endif   /*  __WAVELET_STAT_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
