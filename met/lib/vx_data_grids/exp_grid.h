
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __EXPONENTIAL_GRID_H__
#define  __EXPONENTIAL_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "vx_data_grids/grid_base.h"


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

};


////////////////////////////////////////////////////////////////////////


#endif   //  __EXPONENTIAL_GRID_H__


////////////////////////////////////////////////////////////////////////



