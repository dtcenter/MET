

////////////////////////////////////////////////////////////////////////


#ifndef  __EXPONENTIAL_GRID_H__
#define  __EXPONENTIAL_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "grid_base.h"


////////////////////////////////////////////////////////////////////////


class ExpGrid : public GridRep {

      friend class Grid;

   private:

      ExpGrid();
     ~ExpGrid();
      ExpGrid(const ExpData &);

      double e1x, e1y, e1z;
      double e2x, e2y, e2z;
      double e3x, e3y, e3z;

      double lat_origin_deg;
      double lon_origin_deg;

      double x_scale;    //  grid-units per fake-kilometer
      double y_scale;

      double x_offset;   //  grid-units
      double y_offset;

      int Nx;
      int Ny;

      ExpData Data;

      ConcatString Name;

      void clear();

      void old_to_new(double lat_old_deg, double lon_old_deg, double & lat_new_deg, double & lon_new_deg) const;
      void new_to_old(double lat_new_deg, double lon_new_deg, double & lat_old_deg, double & lon_old_deg) const;

      void xy_to_uv(double x, double y, double & u, double & v) const;
      void uv_to_xy(double u, double v, double & x, double & y) const;

         //
         //  grid interface
         //

      virtual void latlon_to_xy(double lat, double lon, double & x, double & y) const;

      virtual void xy_to_latlon(double x, double y, double & lat, double & lon) const;

      virtual double calc_area(int x, int y) const;

      virtual int nx() const;
      virtual int ny() const;

      virtual ConcatString name() const;

      void dump(ostream &, int = 0) const;

      ConcatString serialize() const;

      GridInfo info() const;

      double rot_grid_to_earth(int x, int y) const;

};


////////////////////////////////////////////////////////////////////////


#endif   //  __EXPONENTIAL_GRID_H__


////////////////////////////////////////////////////////////////////////



