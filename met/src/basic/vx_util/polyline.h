// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   polyline.h
//
//   Description:
//
//
//   Mod#   Date       Name           Description
//   ----   ----       ----           -----------
//   000    11-03-06   Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  __DATA2D_UTIL_POLYLINE_H__
#define  __DATA2D_UTIL_POLYLINE_H__

///////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_util.h"
#include "vx_math.h"
// #include "vx_grid.h"

///////////////////////////////////////////////////////////////////////////////

static const int polyline_alloc_jump = 100;

///////////////////////////////////////////////////////////////////////////////

class Polyline {

   protected:

      void assign(const Polyline &);

   public:

      Polyline();
      virtual ~Polyline();
      Polyline(const Polyline &);
      Polyline & operator=(const Polyline &);

      void clear();

      void dump(ostream &, int = 0) const;

         /////////////////////////////////////////////////////////////////////////

      std::string name;  
      
      double * u;    //  allocated
      double * v;    //  allocated

      int n_points;
      int n_alloc;

      void set_name(std::string);
      
      virtual void add_point(double, double);

      void extend_points(int);

         /////////////////////////////////////////////////////////////////////////
         //
         // Single polyline functions
         //
         /////////////////////////////////////////////////////////////////////////

      int is_closed() const;

      void centroid(double &ubar, double &vbar) const;

      void translate(double du, double dv);

      double angle() const;

      void rotate(double deg);

      void rotate(double deg, double ubar, double vbar);

      double uv_signed_area() const;

      virtual int is_inside(double u_test, double v_test) const;

      int is_polyline_point(double u_test, double v_test) const;

      void bounding_box(Box &) const;

      void sum_first_moments(double &, double &) const;

      void sum_second_moments(double, double, double &, double &, double &) const;

};

///////////////////////////////////////////////////////////////////////////////

extern double point_dist(double x1, double y1, double x2, double y2);

extern double polyline_dist(const Polyline &, const Polyline &);

extern double polyline_pw_ls_mean_dist(const Polyline &, const Polyline &);

extern double polyline_pw_mean_sq_dist(const Polyline &, const Polyline &);

extern int intersect_linesegment(double x1, double y1, double x2, double y2,
                                 double x3, double y3, double x4, double y4);

extern double min_dist_linesegment(double px, double py,
                                   double qx, double qy,
                                   double x_test, double y_test);

extern void parse_latlon_poly_str  (const char *, Polyline &);
extern void parse_latlon_poly_file (const char *, Polyline &);
extern void parse_xy_poly_str      (const char *, Polyline &);
extern void parse_xy_poly_file     (const char *, Polyline &);

///////////////////////////////////////////////////////////////////////////////


#endif   //  __DATA2D_UTIL_POLYLINE_H__


///////////////////////////////////////////////////////////////////////////////



