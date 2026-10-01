// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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

         //
         //  Key is private, so only Grid and GoesImagerGrid itself can create
         //  one.  It lets the constructors below be public - which is
         //  what std::make_unique needs - without opening grid
         //  construction up to the rest of the code.
         //

      struct Key { explicit Key() = default; };

   public:

     ~GoesImagerGrid();
      GoesImagerGrid(const GoesImagerData &, Key);

   private:

      GoesImagerGrid();

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

      void dump(std::ostream &, int = 0) const;

      ConcatString serialize(const char *sep=" ") const;

      GridInfo info () const;

      double rot_grid_to_earth(int x, int y) const;
      
      bool wrap_lon() const;

      void shift_right(int);

      std::unique_ptr<GridRep> copy() const;

      double scale_km() const;
};


////////////////////////////////////////////////////////////////////////

inline double GoesImagerGrid::scale_km() const { return 1.0; }

////////////////////////////////////////////////////////////////////////


#endif   //  __GOES_IMAGER_GRID_H__


////////////////////////////////////////////////////////////////////////
