// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   map_region.h
//
//   Description:
//
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-12-27  Bullock
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VX_MAP_REGION_H_
#define _VX_MAP_REGION_H_

////////////////////////////////////////////////////////////////////////

static const int max_region_points = 150;

////////////////////////////////////////////////////////////////////////////////

class MapRegion {

   private:

      void init_from_scratch();

      void assign(const MapRegion &);

   public:

      MapRegion();
     ~MapRegion();
      MapRegion(const MapRegion &);
      MapRegion & operator=(const MapRegion &);

      void clear();

      int number;
      int n_points;

      double lat_min;
      double lat_max;

      double lon_min;
      double lon_max;

      double lat[max_region_points];
      double lon[max_region_points];

};

////////////////////////////////////////////////////////////////////////


extern bool operator>>(istream &, MapRegion &);


////////////////////////////////////////////////////////////////////////

#endif  //  _VX_MAP_REGION_H_

////////////////////////////////////////////////////////////////////////////////


