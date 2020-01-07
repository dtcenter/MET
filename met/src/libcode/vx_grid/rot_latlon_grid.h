

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

   protected:

      RotatedLatLonGrid();
     ~RotatedLatLonGrid();
      RotatedLatLonGrid(const RotatedLatLonData &);


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

      void dump(ostream &, int = 0) const;

      ConcatString serialize() const;

      GridInfo info() const;

      double rot_grid_to_earth(int x, int y) const;

      bool is_global() const;

      void shift_right(int);

      GridRep * copy() const;

};


////////////////////////////////////////////////////////////////////////


inline double RotatedLatLonGrid::scale_km() const { return ( -1.0 ); }


////////////////////////////////////////////////////////////////////////


#endif   //  __ROTATED_LATLON_GRID_H__


////////////////////////////////////////////////////////////////////////



