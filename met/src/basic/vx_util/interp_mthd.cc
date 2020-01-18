// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdlib>
#include <iostream>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "interp_mthd.h"

///////////////////////////////////////////////////////////////////////////////

ConcatString interpmthd_to_string(const InterpMthd m) {
   ConcatString out;

   switch(m) {
      case(InterpMthd_Min):         out = interpmthd_min_str;         break;
      case(InterpMthd_Max):         out = interpmthd_max_str;         break;
      case(InterpMthd_Median):      out = interpmthd_median_str;      break;
      case(InterpMthd_UW_Mean):     out = interpmthd_uw_mean_str;     break;
      case(InterpMthd_DW_Mean):     out = interpmthd_dw_mean_str;     break;
      case(InterpMthd_AW_Mean):     out = interpmthd_aw_mean_str;     break;
      case(InterpMthd_LS_Fit):      out = interpmthd_ls_fit_str;      break;
      case(InterpMthd_Nbrhd):       out = interpmthd_nbrhd_str;       break;
      case(InterpMthd_Bilin):       out = interpmthd_bilin_str;       break;
      case(InterpMthd_Nearest):     out = interpmthd_nearest_str;     break;
      case(InterpMthd_Budget):      out = interpmthd_budget_str;      break;
      case(InterpMthd_Force):       out = interpmthd_force_str;       break;
      case(InterpMthd_Best):        out = interpmthd_best_str;        break;
      case(InterpMthd_Upper_Left):  out = interpmthd_upper_left_str;  break;
      case(InterpMthd_Upper_Right): out = interpmthd_upper_right_str; break;
      case(InterpMthd_Lower_Right): out = interpmthd_lower_right_str; break;
      case(InterpMthd_Lower_Left):  out = interpmthd_lower_left_str;  break;
      case(InterpMthd_Gaussian):    out = interpmthd_gaussian_str;    break;
      case(InterpMthd_MaxGauss):    out = interpmthd_maxgauss_str;    break;
      case(InterpMthd_Geog_Match):  out = interpmthd_geog_match_str;  break;

      case(InterpMthd_None):
      default:                      out = interpmthd_none_str;        break;
   }   //  switch

   return(out);
}

///////////////////////////////////////////////////////////////////////////////

InterpMthd string_to_interpmthd(const char *mthd_str) {
   InterpMthd m;

        if(strcmp(mthd_str, interpmthd_min_str)         == 0) m = InterpMthd_Min;
   else if(strcmp(mthd_str, interpmthd_max_str)         == 0) m = InterpMthd_Max;
   else if(strcmp(mthd_str, interpmthd_median_str)      == 0) m = InterpMthd_Median;
   else if(strcmp(mthd_str, interpmthd_uw_mean_str)     == 0) m = InterpMthd_UW_Mean;
   else if(strcmp(mthd_str, interpmthd_dw_mean_str)     == 0) m = InterpMthd_DW_Mean;
   else if(strcmp(mthd_str, interpmthd_aw_mean_str)     == 0) m = InterpMthd_AW_Mean;
   else if(strcmp(mthd_str, interpmthd_ls_fit_str)      == 0) m = InterpMthd_LS_Fit;
   else if(strcmp(mthd_str, interpmthd_nbrhd_str)       == 0) m = InterpMthd_Nbrhd;
   else if(strcmp(mthd_str, interpmthd_bilin_str)       == 0) m = InterpMthd_Bilin;
   else if(strcmp(mthd_str, interpmthd_nearest_str)     == 0) m = InterpMthd_Nearest;
   else if(strcmp(mthd_str, interpmthd_budget_str)      == 0) m = InterpMthd_Budget;
   else if(strcmp(mthd_str, interpmthd_force_str)       == 0) m = InterpMthd_Force;
   else if(strcmp(mthd_str, interpmthd_best_str)        == 0) m = InterpMthd_Best;
   else if(strcmp(mthd_str, interpmthd_upper_left_str)  == 0) m = InterpMthd_Upper_Left;
   else if(strcmp(mthd_str, interpmthd_upper_right_str) == 0) m = InterpMthd_Upper_Right;
   else if(strcmp(mthd_str, interpmthd_lower_right_str) == 0) m = InterpMthd_Lower_Right;
   else if(strcmp(mthd_str, interpmthd_lower_left_str)  == 0) m = InterpMthd_Lower_Left;
   else if(strcmp(mthd_str, interpmthd_gaussian_str  )  == 0) m = InterpMthd_Gaussian;
   else if(strcmp(mthd_str, interpmthd_maxgauss_str  )  == 0) m = InterpMthd_MaxGauss;
   else if(strcmp(mthd_str, interpmthd_geog_match_str)  == 0) m = InterpMthd_Geog_Match;
   else                                                       m = InterpMthd_None;

   return(m);
}

///////////////////////////////////////////////////////////////////////////////
