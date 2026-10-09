// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __ROTATED_LATLON_GRID_H__
#define  __ROTATED_LATLON_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "grid_base.h"
#include "latlon_grid.h"

#include "earth_rotation.h"


////////////////////////////////////////////////////////////////////////


class RotatedLatLonGrid : public LatLonGrid {

      friend class Grid;

         //
         //  Key is private, so only Grid and RotatedLatLonGrid itself can create
         //  one.  It lets the constructors below be public - which is
         //  what std::make_unique needs - without opening grid
         //  construction up to the rest of the code.
         //

      struct Key { explicit Key() = default; };

   public:

     ~RotatedLatLonGrid();
      RotatedLatLonGrid(const RotatedLatLonData &, Key);

   protected:

      RotatedLatLonGrid();

      void clear();

      void set_from_rdata(const RotatedLatLonData &);

      EarthRotation er;

      RotatedLatLonData RData;

         //
         //  grid interface
         //

      virtual void latlon_to_xy(double true_lat, double true_lon, double & x, double & y) const;

      virtual void xy_to_latlon(double x, double y, double & true_lat, double & true_lon) const;

      virtual double calc_area(int x, int y) const;

      virtual int nx() const;
      virtual int ny() const;

      double scale_km() const;

      virtual ConcatString name() const;

      void dump(std::ostream &, int = 0) const;

      ConcatString serialize(const char *sep=" ") const;

      GridInfo info() const;

      double rot_grid_to_earth(int x, int y) const;

      bool wrap_lon() const;

      void shift_right(int);

      std::unique_ptr<GridRep> copy() const;

};


////////////////////////////////////////////////////////////////////////


inline double RotatedLatLonGrid::scale_km() const { return ( -1.0 );    }
inline bool   RotatedLatLonGrid::wrap_lon() const { return ( wrapLon ); }


////////////////////////////////////////////////////////////////////////


#endif   //  __ROTATED_LATLON_GRID_H__


////////////////////////////////////////////////////////////////////////
