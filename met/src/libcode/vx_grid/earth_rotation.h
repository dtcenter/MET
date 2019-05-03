

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __EARTH_ROTATION_H__
#define  __EARTH_ROTATION_H__


////////////////////////////////////////////////////////////////////////


#include "so3.h"


////////////////////////////////////////////////////////////////////////


// static const double fake_lon_left = 90.0;
static const double fake_lon_left = 0.0;


////////////////////////////////////////////////////////////////////////


class EarthRotation : public SO3 {

   public:

      EarthRotation();
     ~EarthRotation();

      EarthRotation & operator=(const EarthRotation &);

         //
         //  set stuff
         //

    void set_true_np (double true_lat_north_pole, double true_lon_north_pole, double aux_rotation);

    void set_rot_sp  (double rot_lat_south_pole, double rot_lon_south_pole, double aux_rotation, 
                      double rot_lat_ll, double rot_lon_ll);

         //
         //  get stuff
         //


         //
         //  do stuff
         //


      void latlon_rot_to_true(double lat_rot, double lon_rot, double & lat_true, double & lon_true) const;

      void latlon_true_to_rot(double lat_true, double lon_true, double & lat_rot, double & lon_rot) const;

};


////////////////////////////////////////////////////////////////////////


inline EarthRotation & EarthRotation::operator=(const EarthRotation & r)

{

if ( this == &r )  return ( * this );

SO3::assign(r);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


#endif   //  __EARTH_ROTATION_H__


////////////////////////////////////////////////////////////////////////



