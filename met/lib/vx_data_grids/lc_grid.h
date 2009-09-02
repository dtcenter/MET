// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

//
// LambertGrid: Lambert conformal grid class.
// This class is a private class derived from the GridRep class.
//

////////////////////////////////////////////////////////////////////////


#ifndef  __DATA_GRIDS_LAMBERT_GRID_H__
#define  __DATA_GRIDS_LAMBERT_GRID_H__


////////////////////////////////////////////////////////////////////////


   //
   //  grid classes by Randy Bullock
   //


////////////////////////////////////////////////////////////////////////


#include <vx_data_grids/grid_base.h>


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

      LambertData lc_data;

      char *Name;

      double Phi1_radians;
      double Phi2_radians;

      double Phi0_radians;
      double Lon0_radians;

      double Lon_cen_radians;

      double Delta_km;

      double Radius_km;

      double Bx;
      double By;

      double alpha;

      double cone;

      int Nx;
      int Ny;

      double  f(double) const;
      double df(double) const;

         //
         //  grid interface
         //

      void xy_to_uv(double x, double y, double &u, double &v) const;
      void uv_to_xy(double u, double v, double &x, double &y) const;

      double uv_closedpolyline_area(const double *u, const double *v, int n) const;
      double xy_closedpolyline_area(const double *x, const double *y, int n) const;

      void latlon_to_xy(double lat, double lon, double &x, double &y) const;

      void xy_to_latlon(double x, double y, double &lat, double &lon) const;

      double calc_area(int x, int y) const;
      double calc_area_ll(int x, int y) const;

      int nx() const;
      int ny() const;

      const char * name() const;

      double EarthRadiusKM() const;

      ProjType proj_type() const;

      double rot_grid_to_earth(int x, int y) const;

      void grid_data(GridData &) const;
};


////////////////////////////////////////////////////////////////////////


#endif   //  __DATA_GRIDS_LAMBERT_GRID_H__


////////////////////////////////////////////////////////////////////////



