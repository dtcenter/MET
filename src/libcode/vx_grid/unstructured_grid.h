// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_UNSTRUCTURED_GRID_H__
#define  __MET_UNSTRUCTURED_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "grid_base.h"


////////////////////////////////////////////////////////////////////////


class UnstructuredGrid : public GridRep {

      friend class Grid;

   private:

      UnstructuredGrid();
     ~UnstructuredGrid();
      UnstructuredGrid(const UnstructuredData &);

      void add_dimension(const NumArray &, NumArray &);

         //
         //
         //

      void clear();

      ConcatString Name;

         // Exactly 2 have non-zero length

      NumArray Lats;
      NumArray Lons;
      NumArray Levels;
      NumArray Times;

      NumArray * xDim; // not allocated
      NumArray * yDim; // not allocated

         // TRUE:  for lat or lon vs level or time with unique lat/lon values
         // FALSE: for a 1D list of potentially non-unique lat/lon values

      bool Is2Dim;
      int Nx;
      int Ny;

      UnstructuredData Data;

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

      void dump(ostream &, int = 0) const;

      ConcatString serialize(const char *sep=" ") const;

      GridInfo info() const;

      double rot_grid_to_earth(int x, int y) const;

      bool wrap_lon() const;

      void shift_right(int);

      GridRep * copy() const;

};


////////////////////////////////////////////////////////////////////////


inline double UnstructuredGrid::scale_km() const { return ( -1.0 ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_UNSTRUCTURED_GRID_H__  */


////////////////////////////////////////////////////////////////////////



