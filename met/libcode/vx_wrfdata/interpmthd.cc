// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

#include "vx_wrfdata/interpmthd.h"

///////////////////////////////////////////////////////////////////////////////

void interpmthd_to_string(const InterpMthd m, char *out) {

   switch(m) {
      case(im_na):      strcpy(out, im_na_str);      break;
      case(im_min):     strcpy(out, im_min_str);     break;
      case(im_max):     strcpy(out, im_max_str);     break;
      case(im_median):  strcpy(out, im_median_str);  break;
      case(im_uw_mean): strcpy(out, im_uw_mean_str); break;
      case(im_dw_mean): strcpy(out, im_dw_mean_str); break;
      case(im_ls_fit):  strcpy(out, im_ls_fit_str);  break;
      case(im_nbrhd):   strcpy(out, im_nbrhd_str);   break;

      default:
         strcpy(out, im_na_str);
         break;
   }   //  switch

   return;
}

///////////////////////////////////////////////////////////////////////////////

InterpMthd string_to_interpmthd(const char *mthd_str) {
   InterpMthd m;

        if(strcmp(mthd_str, im_min_str)    == 0)  m = im_min;
   else if(strcmp(mthd_str, im_max_str)     == 0) m = im_max;
   else if(strcmp(mthd_str, im_median_str)  == 0) m = im_median;
   else if(strcmp(mthd_str, im_uw_mean_str) == 0) m = im_uw_mean;
   else if(strcmp(mthd_str, im_dw_mean_str) == 0) m = im_dw_mean;
   else if(strcmp(mthd_str, im_ls_fit_str)  == 0) m = im_ls_fit;
   else if(strcmp(mthd_str, im_nbrhd_str)   == 0) m = im_nbrhd;
   else                                           m = im_na;

   return(m);
}

////////////////////////////////////////////////////////////////////////
