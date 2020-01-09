

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __LAMBERT_GRID_H__
#define  __LAMBERT_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "grid_base.h"


////////////////////////////////////////////////////////////////////////


class LambertGrid : public GridRep {

      friend class Grid;

   private:

      LambertGrid();
     ~LambertGrid();
      LambertGrid(const LambertData &);

      void clear();

         //
         //
         //

      ConcatString Name;

      bool IsNorthHemisphere;

      double Lat_LL;
      double Lon_LL;

      double Lon_orient;

      double Alpha;

      double Cone;

      double Bx;
      double By;

      int Nx;
      int Ny;

      bool Has_SO2;

      double     SO2_Angle;   //  degrees
      double Cos_SO2_Angle;
      double Sin_SO2_Angle;

      double  f(double) const;
      double df(double) const;

      LambertData Data;

         //

      bool has_so2() const;

      double so2_angle() const;

      void set_so2(double degrees);

      void so2_forward (double & x, double & y) const;
      void so2_reverse (double & x, double & y) const;

         //

      bool is_north() const;
      bool is_south() const;

         //
         //  grid interface
         //

      void xy_to_uv(double x, double y, double & u, double & v) const;
      void uv_to_xy(double u, double v, double & x, double & y) const;

      double uv_closedpolyline_area(const double * u, const double * v, int n) const;
      double xy_closedpolyline_area(const double * x, const double * y, int n) const;

      void latlon_to_xy(double lat, double lon, double & x, double & y) const;

      void xy_to_latlon(double x, double y, double & lat, double & lon) const;

      double calc_area(int x, int y) const;

      int nx() const;
      int ny() const;

      ConcatString name() const;

      void dump(ostream &, int = 0) const;

      ConcatString serialize() const;

      GridInfo info () const;

      double rot_grid_to_earth(int x, int y) const;

      bool is_global() const;

      void shift_right(int);

      GridRep * copy() const;

      double scale_km() const;

};


////////////////////////////////////////////////////////////////////////


inline bool LambertGrid::is_north() const { return (   IsNorthHemisphere ); }
inline bool LambertGrid::is_south() const { return ( ! IsNorthHemisphere ); }

inline double LambertGrid::scale_km() const { return ( Data.d_km ); }

inline bool LambertGrid::has_so2() const { return ( Has_SO2 ); }

inline double LambertGrid::so2_angle() const { return ( SO2_Angle ); }


////////////////////////////////////////////////////////////////////////


inline void LambertGrid::so2_forward (double & x, double & y) const

{

double u = x;
double v = y;

x = u*Cos_SO2_Angle - v*Sin_SO2_Angle;

y = u*Sin_SO2_Angle + v*Cos_SO2_Angle;

return;

}


////////////////////////////////////////////////////////////////////////


inline void LambertGrid::so2_reverse (double & x, double & y) const

{

double u = x;
double v = y;

x =  u*Cos_SO2_Angle + v*Sin_SO2_Angle;

y = -u*Sin_SO2_Angle + v*Cos_SO2_Angle;

return;

}


////////////////////////////////////////////////////////////////////////


extern Grid create_oriented_lc(bool is_north_projection,
                               double lat_cen,  double lon_cen,
                               double lat_prev, double lon_prev,
                               double d_km, double r_km,
                               int nx, int ny,
                               double bearing);


////////////////////////////////////////////////////////////////////////


#endif   //  __LAMBERT_GRID_H__


////////////////////////////////////////////////////////////////////////



