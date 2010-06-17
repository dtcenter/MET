
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __GRID_BASE_H__
#define  __GRID_BASE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


#include "vx_util/vx_util.h"

#include "vx_data_grids/st_grid_defs.h"
#include "vx_data_grids/lc_grid_defs.h"
#include "vx_data_grids/exp_grid_defs.h"
#include "vx_data_grids/latlon_grid_defs.h"
#include "vx_data_grids/merc_grid_defs.h"


////////////////////////////////////////////////////////////////////////


class Integrand {

   public:

      Integrand();
      virtual ~Integrand();

      virtual double operator()(double) const = 0;

};


////////////////////////////////////////////////////////////////////////


class GridInterface {   //  pure abstract class for grid public interface

   public:

      GridInterface();
      virtual ~GridInterface();


      virtual void latlon_to_xy(double lat, double lon, double &x, double &y) const = 0;

      virtual void xy_to_latlon(double x, double y, double &lat, double &lon) const = 0;

      virtual double calc_area(int x, int y) const = 0;

      virtual int nx() const = 0;
      virtual int ny() const = 0;

      virtual ConcatString name() const = 0;

      virtual void dump(ostream &, int = 0) const = 0;

};


////////////////////////////////////////////////////////////////////////


class GridRep : public GridInterface {

      friend class Grid;

   private:

      int refCount;

      GridRep(const GridRep &);
      GridRep & operator=(const GridRep &);

   public:

      GridRep();
      virtual ~GridRep();

      virtual void dump(ostream &, int = 0) const = 0;

      virtual ConcatString serialize() const = 0;

};


////////////////////////////////////////////////////////////////////////


class Grid : public GridInterface {

   friend bool operator==(const Grid &, const Grid &);

   protected:

      void init_from_scratch();

      void assign(const Grid &);

      GridRep * rep;

      void detach();

      void attach(GridRep *);

   public:

      Grid();
      Grid(const LambertData &);
      Grid(const StereographicData &);
      Grid(const StereoType2Data &);
      Grid(const StereoType3Data &);
      Grid(const ExpData &);
      Grid(const LatLonData &);
      Grid(const MercatorData &);
      virtual ~Grid();
      Grid(const Grid &);
      Grid & operator=(const Grid &);

      void clear();

      void dump(ostream &, int = 0) const;

      void set (const LambertData &);
      void set (const StereographicData &);
      void set (const StereoType2Data &);
      void set (const StereoType3Data &);
      void set (const ExpData &);
      void set (const LatLonData &);
      void set (const MercatorData &);

      void latlon_to_xy(double lat, double lon, double & x, double & y) const;

      void xy_to_latlon(double x, double y, double & lat, double & lon) const;

      double calc_area(int x, int y) const;

      int nx() const;
      int ny() const;

      ConcatString name() const;

      ConcatString serialize() const;

};


////////////////////////////////////////////////////////////////////////


extern bool operator==(const Grid &, const Grid &);


////////////////////////////////////////////////////////////////////////


#endif   //  __GRID_BASE_H__


////////////////////////////////////////////////////////////////////////



