

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_MET_TCRMW_GRID_H__
#define  __VX_MET_TCRMW_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "vx_vector.h"

#include "rot_latlon_grid.h"


////////////////////////////////////////////////////////////////////////


class TcrmwGrid : public RotatedLatLonGrid {

   private:

      void init_from_scratch();

      void assign(const TcrmwGrid &);

      void calc_ijk();   //  calculate rotated basis vectors

      void range_azi_to_basis(const double range_deg, const double azi_deg, Vector & B_range, Vector & B_azi) const;

      TcrmwData TData;


      Vector Ir, Jr, Kr;

      int Range_n, Azimuth_n;   //  # of points in the radial and azimuthal directions

      double Range_max_km;

      double Lat_Center_Deg;
      double Lon_Center_Deg;    //  + west, - east

   public:

      TcrmwGrid();
      virtual ~TcrmwGrid();
      TcrmwGrid(const TcrmwGrid &);
      TcrmwGrid(const TcrmwData &);
      TcrmwGrid & operator=(const TcrmwGrid &);

      void clear();

      void set_from_data(const TcrmwData &);

         //
         //  get stuff
         //

      int range_n   () const;
      int azimuth_n () const;

      double range_max_km      () const;
      double range_delta_km    () const;   //  Range_Max_km/(Range_n - 1)

      double azimuth_delta_deg () const;   //  360.0/(Azimuth_n - 1)

      double lat_center_deg    () const;
      double lon_center_deg    () const;

         //
         //  do stuff
         //

      void range_azi_to_latlon(const double range_km, const double azi_deg, double & lat, double & lon) const;

      void latlon_to_range_azi(const double lat, const double lon, double & range_km, double & azi_deg) const;


      void latlon_to_xy(double true_lat, double true_lon, double & x, double & y) const;

      void xy_to_latlon(double x, double y, double & true_lat, double & true_lon) const;



      void wind_ne_to_ra(const double lat, const double lon, 
                         const double east_component, const double north_component, 
                         double & radial_component,   double & azimuthal_component) const;


         //
         //  possibly toggles the signs of the radial and/or azimuthal components
         //
         //      to align with the conventions used in the TC community
         //

      void wind_ne_to_ra_conventional (const double lat, const double lon, 
                                       const double east_component, const double north_component, 
                                       double & radial_component,   double & azimuthal_component) const;
  
};


////////////////////////////////////////////////////////////////////////


inline int TcrmwGrid::range_n  () const { return ( Range_n ); }
inline int TcrmwGrid::azimuth_n () const { return ( Azimuth_n ); }

inline double TcrmwGrid::range_max_km () const { return ( Range_max_km ); }

inline double TcrmwGrid::range_delta_km () const { return ( Range_max_km/(Range_n - 1.0) ); }

inline double TcrmwGrid::azimuth_delta_deg () const { return ( 360.0/(Azimuth_n - 1.0) ); }

inline double TcrmwGrid::lat_center_deg () const { return ( Lat_Center_Deg ); }
inline double TcrmwGrid::lon_center_deg () const { return ( Lon_Center_Deg ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_MET_TCRMW_GRID_H__  */


////////////////////////////////////////////////////////////////////////


