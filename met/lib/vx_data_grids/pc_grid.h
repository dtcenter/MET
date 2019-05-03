// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

//
// PlateCarreeGrid: latitude/longitude grid class.
//  This class is a private class derived from the GridRep class.
//



////////////////////////////////////////////////////////////////////////


#ifndef  __DATA_GRIDS_PLATE_CARREE_GRID_H__
#define  __DATA_GRIDS_PLATE_CARREE_GRID_H__


////////////////////////////////////////////////////////////////////////


   //
   //  grid classes by Randy Bullock
   //


////////////////////////////////////////////////////////////////////////


#include <vx_data_grids/grid_base.h>


////////////////////////////////////////////////////////////////////////


class PlateCarreeGrid : public GridRep {

      friend class Grid;

   private:

      PlateCarreeGrid();
     ~PlateCarreeGrid();
      PlateCarreeGrid(const PlateCarreeData &);

      PlateCarreeData pc_data;

      double lat_ll_deg;
      double lon_ll_deg;

      double delta_lat_deg;
      double delta_lon_deg;

      int Nx;
      int Ny;

      char *Name;

      void clear();

         //
         //  grid interface
         //

      virtual void latlon_to_xy(double lat, double lon, double &x, double &y) const;

      virtual void xy_to_latlon(double x, double y, double &lat, double &lon) const;

      virtual double calc_area(int x, int y) const;
      virtual double calc_area_ll(int x, int y) const;

      virtual int nx() const;
      virtual int ny() const;

      virtual const char * name() const;

      virtual double EarthRadiusKM() const;

      virtual ProjType proj_type() const;

      virtual double rot_grid_to_earth(int x, int y) const;

      virtual void grid_data(GridData &) const;
};


////////////////////////////////////////////////////////////////////////


#endif   //  __DATA_GRIDS_PLATE_CARREE_GRID_H__


////////////////////////////////////////////////////////////////////////



