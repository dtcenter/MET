// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __STAT_HDR_INFO_H__
#define  __STAT_HDR_INFO_H__

////////////////////////////////////////////////////////////////////////

#include "vx_config.h"
#include "vx_util.h"
#include "vx_analysis_util.h"

#include "stat_hdr_columns.h"

////////////////////////////////////////////////////////////////////////

struct StatHdrInfo {
   StringArray model, desc;
   StringArray fcst_var, fcst_units, fcst_lev;
   StringArray obs_var, obs_units, obs_lev;
   StringArray obtype, vx_mask, interp_mthd;
   StringArray fcst_thresh, obs_thresh, cov_thresh;
   NumArray fcst_lead, obs_lead, interp_pnts, alpha;
   unixtime fcst_valid_beg, fcst_valid_end;
   unixtime obs_valid_beg, obs_valid_end;

   StatHdrInfo();

   void clear();
   void add(const STATLine &line);
   void check_shc(const ConcatString &cur_case);
   StatHdrColumns   get_shc(const ConcatString &cur_case,
                            const StringArray  &case_cols,
                            const StringArray  &hdr_cols,
                            const StringArray  &hdr_vals,
                            const STATLineType lt);
   ConcatString get_col_css(const ConcatString &cur_case,
                            const char         *col_name,
                            const StringArray  &col_vals,
                            bool               warning) const;
};

////////////////////////////////////////////////////////////////////////

#endif   //  __STAT_HDR_INFO_H__

////////////////////////////////////////////////////////////////////////
