// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

//
// This header file contains class and struct definitions for creating
// and using various particular grid data classes.
// The user needs to know the following:
//
// Grid
//
//   This is the user class for all grids. It instantiates any of the
//   various particular grids by passing in data of the proper type to
//   the Grid constructors:
//
//      Grid(const LambertData &);
//      Grid(const StereographicData &);
//      Grid(const ExpData &);
//      Grid(const PlateCarreeData &);
//      Grid(const MercatorData &);
//
//    where the structs are defined below. The user can then call the
//    methods found in GridInterface:
//
//      void latlon_to_xy(double lat, double lon, double &x, double &y) const;
//      void xy_to_latlon(double x, double y, double &lat, double &lon) const;
//      double calc_area(int x, int y) const;
//      double calc_area_ll(int x, int y) const;
//      int nx() const;
//      int ny() const;
//      const char * name() const;
//      double EarthRadiusKM() const;
//      ProjType proj_type() const;
//      double rot_grid_to_earth(int x, int y) const;
//
//   Detailed descriptions of which are found in that class.
//
//
////////////////////////////////////////////////////////////////////////


#ifndef  __DATA_GRIDS_GRID_BASE_H__
#define  __DATA_GRIDS_GRID_BASE_H__


////////////////////////////////////////////////////////////////////////


//
//  grid classes by Randy Bullock
//
// All latitudes are degrees , North+, South-
// All longitudes are degrees, West+, East-
//
// All x values are in km east from westmost grid location.
// All y values are in km north from southmost grid location.
//
// All structs that do not contain the sphere radius assume an earth
// radius as found in "vx_math/constants.h".
//

////////////////////////////////////////////////////////////////////////

//
// Enumeration for all supported projection types
//
enum ProjType {

    NoProj            = 0,
    LambertProj       = 1,
    StereographicProj = 2,
    ExpProj           = 3,
    PlateCarreeProj   = 4,
    MercatorProj      = 5

};

////////////////////////////////////////////////////////////////////////

//
// Lambert conformal projection struct
// Lambert conformal projection is based on a cone intersecting the
// sphere. The cone will intersect the sphere at two latitudes. The
// two latitudes might be the same.
//
struct LambertData {

   const char *name;   // identifier

   double p1_deg;      // first latitude where cone intersects earth.
   double p2_deg;      // second latitude where cone intersects earth.

   double p0_deg;      // latitude of lower left (southwest most) gridpoint
   double l0_deg;      // longitude of lower left (southwest most) gridpoint

   double lcen_deg;    // longitude that looks vertical on the grid.

   double d_km;        // "resolution" of grid in km.
   double r_km;        // Radius of sphere.

   int nx;             // number of x gridpoints.
   int ny;             // number of y gridpoints.

};


////////////////////////////////////////////////////////////////////////


//
// stereographic grid projection
//
struct StereographicData {

   const char *name; // identifier

   double p1_deg;    // latitude where scale factor is determined.

   double p0_deg;    // latitude of lower left (x,y)=(0,0) gridpoint.
   double l0_deg;    // longitude of lower left (x,y)=(0,0) gridpoint.

   double lcen_deg;  // longitude that looks vertical on the grid.

   double d_km;      // "resolution" of grid in km.

   double r_km;      // Radius of sphere.

   int nx;           // number of x gridpoints.
   int ny;           // number of y gridpoints.

};


////////////////////////////////////////////////////////////////////////

//
// Exponential projection, sometimes known as 'flat' projection.
//
// Gridpoints are: x = x_offset + x_scale*k, k=0,ny-1
//                 y = y_offset + y_scale*k, k=0,ny-1
//
// where x=0,y=0 is the point of tangency.
//
// Grid rotation is handled by the lat_2_deg,lon_2_deg parameter.
// This may be changed in the future into an actual rotation parameter.
//
struct ExpData {

   const char * name;      // identifier

   double lat_origin_deg;  // latitude where tangent plane touches sphere.
   double lon_origin_deg;  // longitude where tangent plane touches sphere.

   double lat_2_deg;       // latitude that defines direction of positive Y
   double lon_2_deg;       // longitude that defines direction of positive Y

   double x_scale;         // x resolution of grid along the tangent plane, km.
   double y_scale;         // y resolution of grid along the tangent plane, km.

   double x_offset;        // offset in number of gridpoints to lower left x.
   double y_offset;        // offset in number of gridpoints to lower left y.

   int nx;                 // number of x gridpoints.
   int ny;                 // number of y gridpoints.

};


////////////////////////////////////////////////////////////////////////

//
// Gridpoints data along the surface in terms of latitude and longitude.
//
// Gridpoints are lat = lat_l1_deg + k*delta_lat_deg, k=0,...,Nlat-1
//                lon = lon_l1_deg + k*delta_lon_deg, k=0,...,Nlat-1
//
struct PlateCarreeData {

   const char *name;      // identifier.

   double lat_ll_deg;     // latitude at edge of grid.
   double lon_ll_deg;     // longitude at edge of grid.

   double delta_lat_deg;  // latitude resolution
   double delta_lon_deg;  // longitude resolution

   int Nlat;              // number of latitude points.
   int Nlon;              // number of longitude points.

};


////////////////////////////////////////////////////////////////////////

//
// Mercator Projection
//

struct MercatorData {

   const char * name;  // identifier.

   double lat_ll_deg;  // latitude of lower left gridpoint.
   double lon_ll_deg;  // longitude of lower left gridpoint.

   double lat_ur_deg;  // latitude of upper right gridpoint.
   double lon_ur_deg;  // longitude of upper right gridpoint.

   int nx;             // number of x gridpoints.
   int ny;             // number of y gridpoints.

};

////////////////////////////////////////////////////////////////////////

union GridData {

   struct PlateCarreeData   pc_data;
   struct MercatorData      mc_data;
   struct ExpData           ex_data; 
   struct LambertData       lc_data;
   struct StereographicData st_data;

};

////////////////////////////////////////////////////////////////////////


class Integrand {

   public:

      virtual ~Integrand();

      virtual double operator()(double) const = 0;

};


////////////////////////////////////////////////////////////////////////


class GridInterface {   //  pure abstract class for grid public interface

   public:

      virtual ~GridInterface();

      //
      // input is a latitude/longitude in degrees. Returned is x, y location
      // on the grid in terms of number of grid points from the lower left
      // grid location.
      virtual void latlon_to_xy(double lat, double lon,
				double &x, double &y) const = 0;


      //
      // input is the x,y location of a gridpoint in terms of number of
      // gridpoints from the lower left corner, returned is the
      // latitude/longitude in degrees of this gridpoint.
      virtual void xy_to_latlon(double x, double y,
				double &lat, double &lon) const = 0;

      // input is the x,y location of a gridpoint in terms of number of
      // gridpoints from the lower left corner, returned is the
      // area of the grid square centered on this point in km squared.
      virtual double calc_area(int x, int y) const = 0;

      // input is the x,y location of a gridpoint in terms of number of
      // gridpoints from the lower left corner, returned is the
      // area of the grid square with this point in the lower left
      // corner in km squared.
      virtual double calc_area_ll(int x, int y) const = 0;

      // return number of gridpoints x and y.
      virtual int nx() const = 0;
      virtual int ny() const = 0;

      // return identiying name of this grid.
      virtual const char * name() const = 0;

      virtual double EarthRadiusKM() const = 0;

      // return the projection type for this grid
      virtual ProjType proj_type() const = 0;

      // return the rotation angle (in degrees) necessary to rotate from
      // grid relative coordinates to earth relative coordinates
      virtual double rot_grid_to_earth(int x, int y) const = 0;

      // retrieve the grid data used to instantiate this grid
      virtual void grid_data(GridData &gdata) const = 0;
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
};


////////////////////////////////////////////////////////////////////////


class Grid : public GridInterface {

   private:

      GridRep *rep;

      void detach();

      void attach(GridRep *);

   public:

      Grid();
      Grid(const LambertData &);
      Grid(const StereographicData &);
      Grid(const ExpData &);
      Grid(const PlateCarreeData &);
      Grid(const MercatorData &);
      virtual ~Grid();
      Grid(const Grid &);
      Grid & operator=(const Grid &);

      void latlon_to_xy(double lat, double lon, double &x, double &y) const;

      void xy_to_latlon(double x, double y, double &lat, double &lon) const;

      double calc_area(int x, int y) const;
      double calc_area_ll(int x, int y) const;

      int nx() const;
      int ny() const;

      const char * name() const;

      double EarthRadiusKM() const;

      ProjType proj_type() const;

      double rot_grid_to_earth(int x, int y) const;

      void grid_data(GridData &) const;
};


////////////////////////////////////////////////////////////////////////


#endif   //  __DATA_GRIDS_GRID_BASE_H__


////////////////////////////////////////////////////////////////////////



