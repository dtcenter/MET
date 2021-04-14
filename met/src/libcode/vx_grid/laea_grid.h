

////////////////////////////////////////////////////////////////////////


#ifndef  __LAMBERT_AZIMUTHAL_EQUAL_AREA_GRID_H__
#define  __LAMBERT_AZIMUTHAL_EQUAL_AREA_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "affine.h"
#include "vx_geodesy.h"

#include "laea_grid_defs.h"
#include "grid_base.h"


////////////////////////////////////////////////////////////////////////


class LaeaGrid : public GridRep {

      friend class Grid;

   protected:

      LaeaGrid();
     ~LaeaGrid();
      LaeaGrid(const LaeaData &);

      Spheroid geoid;

      LaeaData Data;

      double snyder_m_func(double lat) const;

      void snyder_latlon_to_xy(double lat, double lon, double & x_snyder, double & y_snyder) const;

         //
         //
         //

      void clear();


      void calc_aff();

      // void latlon_old_to_rot(double lat_old, double lon_old, double & lat_rot, double & lon_rot) const;

      // void latlon_rot_to_old(double lat_rot, double lon_rot, double & lat_old, double & lon_old) const;

      // void latlon_to_rt(double lat, double lon, double & r, double & theta) const;
      // void rt_to_latlon(double r, double theta, double & lat, double & lon) const;

      // void rt_to_uv(double r, double theta, double & u, double & v) const;
      // void uv_to_rt(double u, double v, double & r, double & theta) const;

      void xy_to_uv(double x, double y, double & u, double & v) const;
      void uv_to_xy(double u, double v, double & x, double & y) const;

      double uv_closedpolyline_area(const double * u, const double * v, int n) const;

      double xy_closedpolyline_area(const double * x, const double * y, int n) const;

      Affine aff;   //  takes (u,v) to (x, y)

      int Nx;
      int Ny;

      ConcatString Name;

         //
         //  grid interface
         //

      void latlon_to_xy(double lat, double lon, double & x, double & y) const;

      void xy_to_latlon(double x, double y, double & lat, double & lon) const;

      double calc_area(int x, int y) const;

      bool is_north() const;
      bool is_south() const;

      int nx() const;
      int ny() const;

      void set_so2(double);

      ConcatString name() const;

      const char * projection_name() const;

      void dump(ostream &, int = 0) const;

      ConcatString serialize(int version = 1) const;

      void deserialize(const StringArray &);

      GridInfo info() const;

      double rot_grid_to_earth(int x, int y) const;

      GridRep * copy() const;



      double scale_km() const;

      bool is_global() const;

      void shift_right(int);

      ConcatString serialize() const;

};


////////////////////////////////////////////////////////////////////////


inline bool LaeaGrid::is_north () const { return ( true ); }
inline bool LaeaGrid::is_south () const { return ( false ); }

inline void LaeaGrid::set_so2(double) { return; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __LAMBERT_AZIMUTHAL_EQUAL_AREA_GRID_H__  */


////////////////////////////////////////////////////////////////////////



