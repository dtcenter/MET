// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_SEMILATLON_GRID_H__
#define  __MET_SEMILATLON_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "grid_base.h"


////////////////////////////////////////////////////////////////////////


class SemiLatLonGrid : public GridRep {

      friend class Grid;

   private:

      SemiLatLonGrid();
     ~SemiLatLonGrid();
      SemiLatLonGrid(const SemiLatLonData &);

      void add_dimension(const NumArray &, NumArray &);

         //
         //
         //

      void clear();

      ConcatString Name;

         // Lat and/or Lon are non-empty and Levels or Times are non-empty
         // When Lat and Lon are both specified, they must be the same length

      NumArray Lats;
      NumArray Lons;
      NumArray Levels;
      NumArray Times;

      NumArray * xDim; // not allocated
      NumArray * yDim; // not allocated

      bool IsLatLon;
      int Nx;
      int Ny;

      SemiLatLonData Data;

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


inline double SemiLatLonGrid::scale_km() const { return ( -1.0 ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_SEMILATLON_GRID_H__  */


////////////////////////////////////////////////////////////////////////



