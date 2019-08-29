

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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


#include "vx_util.h"

#include "st_grid_defs.h"
#include "lc_grid_defs.h"
#include "latlon_grid_defs.h"
#include "merc_grid_defs.h"
#include "gaussian_grid_defs.h"
#include "goes_grid_defs.h"


////////////////////////////////////////////////////////////////////////


class Grid;   //  forward reference


////////////////////////////////////////////////////////////////////////


class GridInfo {

   friend bool operator==(const GridInfo &, const GridInfo &);

   private:

      void init_from_scratch();

      void assign(const GridInfo &);

   public:

      GridInfo();
     ~GridInfo();
      GridInfo(const GridInfo &);
      GridInfo & operator=(const GridInfo &);

      void clear();

      bool ok() const;

      void set(const LambertData        &);
      void set(const StereographicData  &);
      void set(const LatLonData         &);
      void set(const RotatedLatLonData  &);
      void set(const MercatorData       &);
      void set(const GaussianData       &);
      void set(const GoesImagerData     &);

      void create_grid(Grid &) const;

         //
         //  at most ONE of these should be nonzero
         //

      const LambertData       * lc;   //  allocated
      const StereographicData * st;   //  allocated
      const LatLonData        * ll;   //  allocated
      const RotatedLatLonData * rll;  //  allocated
      const MercatorData      * m;    //  allocated
      const GaussianData      * g;    //  allocated
      const GoesImagerData    * gi;   //  allocated

};


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

      virtual double scale_km() const = 0;   //  returns -1.0 if scale is unknown

      virtual ConcatString name() const = 0;

      virtual void dump(ostream &, int = 0) const = 0;

      virtual GridInfo info() const = 0;

      virtual double rot_grid_to_earth(int x, int y) const = 0;

      virtual bool is_global() const = 0;

      virtual void shift_right(int) = 0;

};


////////////////////////////////////////////////////////////////////////


class GridRep : public GridInterface {

      friend class Grid;

   private:

      GridRep(const GridRep &);
      GridRep & operator=(const GridRep &);

   public:

      GridRep();
      virtual ~GridRep();

      virtual void dump(ostream &, int = 0) const = 0;

      virtual ConcatString serialize() const = 0;

      virtual GridInfo info() const = 0;

      virtual double rot_grid_to_earth(int x, int y) const = 0;

      virtual bool is_global() const = 0;

      virtual void shift_right(int) = 0;

      virtual GridRep * copy() const = 0;

};


////////////////////////////////////////////////////////////////////////


class Grid : public GridInterface {

   friend bool operator==(const Grid &, const Grid &);

   protected:

      void init_from_scratch();

      void assign(const Grid &);

      GridRep * rep;
      bool swap_to_north;       // The raw latitude data is north to south

   public:

      Grid();
      Grid(const char *);   //  lookup by name
      Grid(const LambertData       &);
      Grid(const StereographicData &);
      Grid(const LatLonData        &);
      Grid(const RotatedLatLonData &);
      Grid(const MercatorData      &);
      Grid(const GaussianData      &);
      Grid(const GoesImagerData    &);
      virtual ~Grid();
      Grid(const Grid &);
      Grid & operator=(const Grid &);

      void clear();

      void dump(ostream &, int = 0) const;

      void set (const char *);   //  lookup by name
      void set (const LambertData       &);
      void set (const StereographicData &);
      void set (const LatLonData        &);
      void set (const RotatedLatLonData &);
      void set (const MercatorData      &);
      void set (const GaussianData      &);
      void set (const GoesImagerData    &);

      void set_swap_to_north(bool swap_to_north);
      bool get_swap_to_north() const;

      void latlon_to_xy(double lat, double lon, double & x, double & y) const;

      void xy_to_latlon(double x, double y, double & lat, double & lon) const;

      double calc_area(int x, int y) const;

      int nx() const;
      int ny() const;

      double scale_km() const;   //  returns -1.0 if scale is unknown or inapplicable

      ConcatString name() const;

      ConcatString serialize() const;

      GridInfo info() const;

      double rot_grid_to_earth(int x, int y) const;

      bool is_global() const;

      void shift_right(int);

         //
         //  subsetting the grid
         //

      Grid subset_ll(int x_ll, int y_ll, int nx_new, int ny_new) const;   //  input: lower left corner, and size

      Grid subset_center(double lat_center, double lon_center, int nx_new, int ny_new) const;

};


////////////////////////////////////////////////////////////////////////


extern bool operator==(const Grid &, const Grid &);
extern bool operator!=(const Grid &, const Grid &);
extern bool operator==(const GridInfo &, const GridInfo &);

extern bool is_eq(const LambertData *,       const LambertData *);
extern bool is_eq(const StereographicData *, const StereographicData *);
extern bool is_eq(const LatLonData *,        const LatLonData *);
extern bool is_eq(const RotatedLatLonData *, const RotatedLatLonData *);
extern bool is_eq(const MercatorData *,      const MercatorData *);
extern bool is_eq(const GoesImagerData *,    const GoesImagerData *);


////////////////////////////////////////////////////////////////////////


#endif   //  __GRID_BASE_H__


////////////////////////////////////////////////////////////////////////



