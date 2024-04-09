// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "interp_mthd.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

ConcatString interpmthd_to_string(const InterpMthd m) {
   ConcatString out;

   switch(m) {
      case InterpMthd::Min:         out = interpmthd_min_str;         break;
      case InterpMthd::Max:         out = interpmthd_max_str;         break;
      case InterpMthd::Median:      out = interpmthd_median_str;      break;
      case InterpMthd::UW_Mean:     out = interpmthd_uw_mean_str;     break;
      case InterpMthd::DW_Mean:     out = interpmthd_dw_mean_str;     break;
      case InterpMthd::AW_Mean:     out = interpmthd_aw_mean_str;     break;
      case InterpMthd::LS_Fit:      out = interpmthd_ls_fit_str;      break;
      case InterpMthd::Nbrhd:       out = interpmthd_nbrhd_str;       break;
      case InterpMthd::Bilin:       out = interpmthd_bilin_str;       break;
      case InterpMthd::Nearest:     out = interpmthd_nearest_str;     break;
      case InterpMthd::Budget:      out = interpmthd_budget_str;      break;
      case InterpMthd::Force:       out = interpmthd_force_str;       break;
      case InterpMthd::Best:        out = interpmthd_best_str;        break;
      case InterpMthd::Upper_Left:  out = interpmthd_upper_left_str;  break;
      case InterpMthd::Upper_Right: out = interpmthd_upper_right_str; break;
      case InterpMthd::Lower_Right: out = interpmthd_lower_right_str; break;
      case InterpMthd::Lower_Left:  out = interpmthd_lower_left_str;  break;
      case InterpMthd::Gaussian:    out = interpmthd_gaussian_str;    break;
      case InterpMthd::MaxGauss:    out = interpmthd_maxgauss_str;    break;
      case InterpMthd::Geog_Match:  out = interpmthd_geog_match_str;  break;
      case InterpMthd::HiRA:        out = interpmthd_hira_str;        break;

      case InterpMthd::None:
      default:                      out = interpmthd_none_str;        break;
   }   //  switch

   return out;
}

///////////////////////////////////////////////////////////////////////////////

InterpMthd string_to_interpmthd(const char *mthd_str) {
   InterpMthd m;

        if(strcmp(mthd_str, interpmthd_min_str)         == 0) m = InterpMthd::Min;
   else if(strcmp(mthd_str, interpmthd_max_str)         == 0) m = InterpMthd::Max;
   else if(strcmp(mthd_str, interpmthd_median_str)      == 0) m = InterpMthd::Median;
   else if(strcmp(mthd_str, interpmthd_uw_mean_str)     == 0) m = InterpMthd::UW_Mean;
   else if(strcmp(mthd_str, interpmthd_dw_mean_str)     == 0) m = InterpMthd::DW_Mean;
   else if(strcmp(mthd_str, interpmthd_aw_mean_str)     == 0) m = InterpMthd::AW_Mean;
   else if(strcmp(mthd_str, interpmthd_ls_fit_str)      == 0) m = InterpMthd::LS_Fit;
   else if(strcmp(mthd_str, interpmthd_nbrhd_str)       == 0) m = InterpMthd::Nbrhd;
   else if(strcmp(mthd_str, interpmthd_bilin_str)       == 0) m = InterpMthd::Bilin;
   else if(strcmp(mthd_str, interpmthd_nearest_str)     == 0) m = InterpMthd::Nearest;
   else if(strcmp(mthd_str, interpmthd_budget_str)      == 0) m = InterpMthd::Budget;
   else if(strcmp(mthd_str, interpmthd_force_str)       == 0) m = InterpMthd::Force;
   else if(strcmp(mthd_str, interpmthd_best_str)        == 0) m = InterpMthd::Best;
   else if(strcmp(mthd_str, interpmthd_upper_left_str)  == 0) m = InterpMthd::Upper_Left;
   else if(strcmp(mthd_str, interpmthd_upper_right_str) == 0) m = InterpMthd::Upper_Right;
   else if(strcmp(mthd_str, interpmthd_lower_right_str) == 0) m = InterpMthd::Lower_Right;
   else if(strcmp(mthd_str, interpmthd_lower_left_str)  == 0) m = InterpMthd::Lower_Left;
   else if(strcmp(mthd_str, interpmthd_gaussian_str  )  == 0) m = InterpMthd::Gaussian;
   else if(strcmp(mthd_str, interpmthd_maxgauss_str  )  == 0) m = InterpMthd::MaxGauss;
   else if(strcmp(mthd_str, interpmthd_geog_match_str)  == 0) m = InterpMthd::Geog_Match;
   else if(strcmp(mthd_str, interpmthd_hira_str)        == 0) m = InterpMthd::HiRA;
   else                                                       m = InterpMthd::None;

   return m;
}

///////////////////////////////////////////////////////////////////////////////
