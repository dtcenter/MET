

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __GOES_IMAGER_GRID_H__
#define  __GOES_IMAGER_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "grid_base.h"


////////////////////////////////////////////////////////////////////////

class GoesImagerGrid : public GridRep {

      friend class Grid;

   private:

      GoesImagerGrid();
     ~GoesImagerGrid();
      GoesImagerGrid(const GoesImagerData &);

      void clear();

         //
         //
         //

      ConcatString Name;

      int Nx;
      int Ny;

      GoesImagerData Data;

         //

         //
         //  grid interface
         //

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

inline double GoesImagerGrid::scale_km() const { return ( 1.0 ); }

////////////////////////////////////////////////////////////////////////


#endif   //  __GOES_IMAGER_GRID_H__


////////////////////////////////////////////////////////////////////////



