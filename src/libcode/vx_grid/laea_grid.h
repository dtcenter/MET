// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


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
      LaeaGrid(const LaeaCornerData &);

      Spheroid geoid;

      LaeaData       Data;
      LaeaCornerData CornerData;

      double snyder_m_func(double lat) const;

      void snyder_latlon_to_xy(double lat, double lon, double & x_snyder, double & y_snyder) const;

         //
         //
         //

      void clear();

      void calc_aff();

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

      void dump(std::ostream &, int = 0) const;

      ConcatString serialize(const char *sep=" ") const;

      void deserialize(const StringArray &);

      GridInfo info() const;

      double rot_grid_to_earth(int x, int y) const;

      bool wrap_lon() const;

      GridRep * copy() const;

      double scale_km() const;

      bool is_global() const;

      void shift_right(int);

};


////////////////////////////////////////////////////////////////////////


inline bool LaeaGrid::is_north () const { return ( true ); }
inline bool LaeaGrid::is_south () const { return ( false ); }

inline void LaeaGrid::set_so2(double) { return; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __LAMBERT_AZIMUTHAL_EQUAL_AREA_GRID_H__  */


////////////////////////////////////////////////////////////////////////
