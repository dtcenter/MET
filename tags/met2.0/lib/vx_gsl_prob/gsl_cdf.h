// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __VX_GSL_CDF_H__
#define  __VX_GSL_CDF_H__

////////////////////////////////////////////////////////////////////////

#include "gsl/gsl_cdf.h"

////////////////////////////////////////////////////////////////////////

   //
   //  standard normal
   //

extern double znorm  (double x);

extern double zinv   (double y);

////////////////////////////////////////////////////////////////////////

   //
   //  general normal
   //

extern double normal_cdf     (double x, double mu, double sigma);

extern double normal_cdf_inv (double y, double mu, double sigma);

////////////////////////////////////////////////////////////////////////

   //
   //  chi squared
   //

extern double chi2_cdf     (double x, double deg_freedom);

extern double chi2_cdf_inv (double y, double deg_freedom);

////////////////////////////////////////////////////////////////////////

   //
   //  Student's t
   //

extern double students_t_cdf     (double x, double def_freedom);

extern double students_t_cdf_inv (double y, double def_freedom);

////////////////////////////////////////////////////////////////////////

   //
   //  F
   //

extern double F_cdf      (double x, double deg_freedom_1, double deg_freedom_2);

extern double F_cdf_inv  (double y, double deg_freedom_1, double deg_freedom_2);

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_GSL_CDF_H__  */

////////////////////////////////////////////////////////////////////////
