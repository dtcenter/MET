// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __RNG_AZI_GRID_H__
#define  __RNG_AZI_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "vx_vector.h"

#include "rot_latlon_grid.h"


////////////////////////////////////////////////////////////////////////


class RngAziGrid : public RotatedLatLonGrid {

      friend class Grid;

   private:

      void assign(const RngAziGrid &);

   public:

      RngAziGrid();
     ~RngAziGrid();
      RngAziGrid(const RngAziData &);
      RngAziGrid & operator=(const RngAziGrid &);

      void calc_ijk();   //  calculate rotated basis vectors

      Vector Ir, Jr, Kr;

      int Range_n, Azimuth_n;   //  # of points in the radial and azimuthal directions

      double Range_max_km;

      double Lat_Center_Deg;
      double Lon_Center_Deg;    //  + west, - east

      RngAziData TData;

      void clear();

      void set_from_data(const RngAziData &);

         //
         //  get stuff
         //

      int range_n   () const;
      int azimuth_n () const;

      double range_max_km      () const;
      double range_delta_km    () const;   //  Range_max_km/(Range_n - 1)

      double azimuth_delta_deg () const;   //  360.0/Azimuth_n

      double lat_center_deg    () const;
      double lon_center_deg    () const;

         //
         //  do stuff
         //

      void range_azi_to_latlon(const double range_km, const double azi_deg, double & lat, double & lon) const;

      void latlon_to_range_azi(const double lat, const double lon, double & range_km, double & azi_deg) const;


      void latlon_to_xy(double true_lat, double true_lon, double & x, double & y) const;

      void xy_to_latlon(double x, double y, double & true_lat, double & true_lon) const;

      void wind_ne_to_rt(const double azi_deg,
                         const double u_wind, const double v_wind, 
                         double & radial_wind, double & tangential_wind) const;

      void wind_ne_to_rt(const double lat, const double lon, 
                         const double u_wind, const double v_wind, 
                         double & radial_wind, double & tangential_wind) const;

      void dump(std::ostream &, int = 0) const;

      ConcatString serialize(const char *sep=" ") const;

      GridInfo info() const;

      GridRep * copy() const;

};


////////////////////////////////////////////////////////////////////////


inline int RngAziGrid::range_n  () const { return Range_n; }
inline int RngAziGrid::azimuth_n () const { return Azimuth_n; }

inline double RngAziGrid::range_max_km () const { return Range_max_km; }

inline double RngAziGrid::range_delta_km () const { return Range_max_km/(Range_n - 1); }

inline double RngAziGrid::azimuth_delta_deg () const { return 360.0/Azimuth_n; }

inline double RngAziGrid::lat_center_deg () const { return Lat_Center_Deg; }
inline double RngAziGrid::lon_center_deg () const { return Lon_Center_Deg; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __RNG_AZI_GRID_H__  */


////////////////////////////////////////////////////////////////////////
