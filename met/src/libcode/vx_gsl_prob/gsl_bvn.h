// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __MET_GSL_BIVARIATE_NORMAL_H__
#define  __MET_GSL_BIVARIATE_NORMAL_H__

////////////////////////////////////////////////////////////////////////

struct BVN_ellipse_data {

   double a;
   double b;

   double x_center;
   double y_center;

   double angle_degrees;

};

struct BVN_params {

   double xbar;
   double ybar;

   double Suu;   //
   double Suv;   //   second moments about the MEAN
   double Svv;   //

};

extern  BVN_ellipse_data  bvn_get_ellipse  (const BVN_params &, double q);

extern  double            bvn_get_quantile (const BVN_params &, double x, double y);   //  level curve that (x, y) is on

extern  void              bvn_y_to_z       (const BVN_params & params, double y_1, double y_2, double & z_1, double & z_2);


extern double calc_angle_deg(const double & Suu, const double & Svv, const double & Suv);

extern void rotate_moments(double & Sxx, double & Syy, double & Sxy, double angle_degrees);

////////////////////////////////////////////////////////////////////////

#endif   /*  __MET_GSL_BIVARIATE_NORMAL_H__  */

////////////////////////////////////////////////////////////////////////
