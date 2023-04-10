// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __STEREOGRAPHIC_GRID_H__
#define  __STEREOGRAPHIC_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "st_grid_defs.h"
#include "grid_base.h"


////////////////////////////////////////////////////////////////////////


class StereographicGrid : public GridRep {

      friend class Grid;

   private:

      StereographicGrid();
     ~StereographicGrid();
      StereographicGrid(const StereographicData &);

         //
         //
         //

      void clear();

      void xy_to_uv(double x, double y, double &u, double &v) const;
      void uv_to_xy(double u, double v, double &x, double &y) const;

      double uv_closedpolyline_area(const double * u, const double * v, int n) const;

      double xy_closedpolyline_area(const double * x, const double * y, int n) const;

      double f(double) const;

      double df(double) const;

      bool IsNorthHemisphere;

      double Lon_orient;

      double Bx;
      double By;

      double Alpha;

      int Nx;
      int Ny;

      ConcatString Name;

      StereographicData Data;

      bool is_north() const;
      bool is_south() const;

         //
         //  grid interface
         //

      void latlon_to_xy(double lat, double lon, double & x, double & y) const;

      void xy_to_latlon(double x, double y, double & lat, double & lon) const;

      double calc_area(int x, int y) const;

      int nx() const;
      int ny() const;

      double scale_km() const;

      ConcatString name() const;

      void dump(std::ostream &, int = 0) const;

      ConcatString serialize(const char *sep=" ") const;

      GridInfo info() const;

      double rot_grid_to_earth(int x, int y) const;

      bool wrap_lon() const;

      void shift_right(int);

      GridRep * copy() const;

};


////////////////////////////////////////////////////////////////////////


inline bool StereographicGrid::is_north () const { return (   IsNorthHemisphere ); }
inline bool StereographicGrid::is_south () const { return ( ! IsNorthHemisphere ); }

inline double StereographicGrid::scale_km () const { return ( Data.d_km ); }

////////////////////////////////////////////////////////////////////////


extern double stereographic_alpha(double scale_lat, double r_km, double d_km);

extern double     st_func (double lat, bool is_north_hemisphere, double eccentricity = 0.);
extern double st_der_func (double lat, bool is_north_hemisphere);

extern double st_inv_func (double r, bool is_north_hemisphere);

extern double st_eccentricity_func(double semi_major_axis, double semi_minor_axis,
                                   double inverse_flattening);
extern double st_sf_func(double standard_parallel, double eccentricity, bool is_north_hemisphere);
extern bool st_latlon_to_xy_func(double lat, double lon, double &x_m, double &y_m,
                                 double scale_factor, double scale_lat, double semi_major_axis,
                                 double false_east, double false_north,
                                 double e, bool is_north_hemisphere);
extern bool st_xy_to_latlon_func(double x_m, double y_m, double &lat, double &lon,
                                 double scale_factor, double semi_major_axis,
                                 double proj_vertical_lon, double false_east, double false_north,
                                 double eccentricity, bool is_north_hemisphere);


////////////////////////////////////////////////////////////////////////


extern Grid create_aligned_st(double lat_center,   double lon_center,
                              double lat_previous, double lon_previous,
                              double d_km, double r_km,
                              int nx, int ny);


////////////////////////////////////////////////////////////////////////


#endif   //  __STEREOGRAPHIC_GRID_H__


////////////////////////////////////////////////////////////////////////
