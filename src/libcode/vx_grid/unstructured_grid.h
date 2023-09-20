// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __UNSTRUCTURTED_GRID_H__
#define  __UNSTRUCTURTED_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "grid_base.h"


////////////////////////////////////////////////////////////////////////


class UnstructuredGrid : public GridRep {

      friend class Grid;

   protected:

      UnstructuredGrid();
     ~UnstructuredGrid();
      UnstructuredGrid(const UnstructuredData &);


      int Nx;
      //double distance;  //in meters. Set the negative distance to disbale disathce capability


      bool wrapLon;

      ConcatString Name;

      UnstructuredData Data;

      void clear();

      void set_from_data(const UnstructuredData &);
      //void set_max_distance_deg(double max_distance);
      void set_max_distance_km(double max_distance);

         //
         //  grid interface
         //

      virtual void latlon_to_xy(double lat, double lon, double & x, double & y) const;

      virtual void xy_to_latlon(double x, double y, double & lat, double & lon) const;

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

      GridRep * copy() const;

};


////////////////////////////////////////////////////////////////////////


inline double UnstructuredGrid::scale_km() const { return ( -1.0 );    }
inline bool   UnstructuredGrid::wrap_lon() const { return ( wrapLon ); }


////////////////////////////////////////////////////////////////////////


#endif   //  __UNSTRUCTURTED_GRID_H__


////////////////////////////////////////////////////////////////////////
