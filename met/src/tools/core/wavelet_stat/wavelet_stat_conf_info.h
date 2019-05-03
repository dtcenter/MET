// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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

#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_gsl_prob.h"
#include "vx_statistics.h"
#include "result.h"

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
      VarInfo     **fcst_info;  // Array of pointers for fcst VarInfo [n_vx]
      VarInfo     **obs_info;   // Array of pointers for obs VarInfo [n_vx]
      ThreshArray *fcst_ta;     // Array for fcst thresholds [n_vx]
      ThreshArray *obs_ta;      // Array for obs thresholds [n_vx]

      NumArray     tile_xll;    // Array of lower-left x coordinates
      NumArray     tile_yll;    // Array of lower-left y coordinates
      Box          pad_bb;      // Pad bouding box

      WaveletStatConfInfo();
     ~WaveletStatConfInfo();

      void clear();

      void read_config   (const char *, const char *,
                          GrdFileType, unixtime, int,
                          GrdFileType, unixtime, int);
      void process_config(GrdFileType, unixtime, int,
                          GrdFileType, unixtime, int);
      void process_tiles (const Grid &);
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
