

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


#ifndef  __LATLON_GRID_DEFINITIONS_H__
#define  __LATLON_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct LatLonData {

   const char * name;   //  not allocated

   double lat_ll;
   double lon_ll;

   double delta_lat;
   double delta_lon;

   int Nlat;
   int Nlon;

   void dump();

};


////////////////////////////////////////////////////////////////////////


struct RotatedLatLonData {

   const char * name;   //  not allocated

   double rot_lat_ll;
   double rot_lon_ll;

   double delta_rot_lat;
   double delta_rot_lon;

   int Nlat;
   int Nlon;

   double true_lat_south_pole;
   double true_lon_south_pole;

   double aux_rotation;

      //////////

   void dump() const;   //  doesn't work if verbosity level < 4

   void dump(ostream &, int depth) const;

};


////////////////////////////////////////////////////////////////////////


struct TcrmwData {

   const char * name;   //  not allocated

   int range_n;
   int azimuth_n;

   double range_max_km;

   double lat_center;
   double lon_center;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __LATLON_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



