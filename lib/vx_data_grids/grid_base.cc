// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <string.h>
#include <cmath>

#include <vx_data_grids/grid_base.h>
#include <vx_math/is_bad_data.h>

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GridInterface
   //


////////////////////////////////////////////////////////////////////////


GridInterface::~GridInterface()

{

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GridRep
   //


////////////////////////////////////////////////////////////////////////


GridRep::GridRep()

{

refCount = 0;

}


////////////////////////////////////////////////////////////////////////


GridRep::~GridRep() { }


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Grid
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid()

{

rep = (GridRep *) 0;

}


////////////////////////////////////////////////////////////////////////


Grid::~Grid()

{

detach();

}


////////////////////////////////////////////////////////////////////////


Grid::Grid(const Grid &g)

{

rep = (GridRep *) 0;

attach(g.rep);

}


////////////////////////////////////////////////////////////////////////


Grid & Grid::operator=(const Grid &g)

{

if ( this == &g )  return ( *this );

attach(g.rep);

return ( *this );

}


////////////////////////////////////////////////////////////////////////


bool Grid::operator==(const Grid &g)

{

GridData d1, d2;
bool match = false;

if ( this->rep->proj_type() == g.rep->proj_type() ) {

   // Retrieve the grid data
   this->rep->grid_data(d1);
   g.rep->grid_data(d2);

   // Switch on the projection type
   switch ( this->rep->proj_type() ) {

      case LambertProj:
         if ( is_eq(d1.lc_data.p1_deg,   d2.lc_data.p1_deg   ) &&
              is_eq(d1.lc_data.p2_deg,   d2.lc_data.p2_deg   ) &&
              is_eq(d1.lc_data.p0_deg,   d2.lc_data.p0_deg   ) &&
              is_eq(d1.lc_data.l0_deg,   d2.lc_data.l0_deg   ) &&
              is_eq(d1.lc_data.lcen_deg, d2.lc_data.lcen_deg ) &&
              is_eq(d1.lc_data.d_km,     d2.lc_data.d_km     ) &&
              is_eq(d1.lc_data.r_km,     d2.lc_data.r_km     ) &&
              d1.lc_data.nx           == d2.lc_data.nx         &&
              d1.lc_data.ny           == d2.lc_data.ny )  match = true;
         break;

      case StereographicProj:
         if ( is_eq(d1.st_data.p1_deg,   d2.st_data.p1_deg   ) &&
              is_eq(d1.st_data.p0_deg,   d2.st_data.p0_deg   ) &&
              is_eq(d1.st_data.l0_deg,   d2.st_data.l0_deg   ) &&
              is_eq(d1.st_data.lcen_deg, d2.st_data.lcen_deg ) &&
              is_eq(d1.st_data.d_km,     d2.st_data.d_km     ) &&
              is_eq(d1.st_data.r_km,     d2.st_data.r_km     ) &&
              d1.st_data.nx           == d2.st_data.nx         &&
              d1.st_data.ny           == d2.st_data.ny )  match = true;
         break;

      case ExpProj:
         if ( is_eq(d1.ex_data.lat_origin_deg, d2.ex_data.lat_origin_deg ) &&
              is_eq(d1.ex_data.lon_origin_deg, d2.ex_data.lon_origin_deg ) &&
              is_eq(d1.ex_data.lat_2_deg,      d2.ex_data.lat_2_deg      ) &&
              is_eq(d1.ex_data.lon_2_deg,      d2.ex_data.lon_2_deg      ) &&
              is_eq(d1.ex_data.x_scale,        d2.ex_data.x_scale        ) &&
              is_eq(d1.ex_data.y_scale,        d2.ex_data.y_scale        ) &&
              is_eq(d1.ex_data.x_offset,       d2.ex_data.x_offset       ) &&
              is_eq(d1.ex_data.y_offset,       d2.ex_data.y_offset       ) &&
              d1.ex_data.nx                 == d2.ex_data.nx               &&
              d1.ex_data.ny                 == d2.ex_data.ny )  match = true;
         break;

      case PlateCarreeProj:
         if ( is_eq(d1.pc_data.lat_ll_deg,    d2.pc_data.lat_ll_deg    ) &&
              is_eq(d1.pc_data.lon_ll_deg,    d2.pc_data.lon_ll_deg    ) &&
              is_eq(d1.pc_data.delta_lat_deg, d2.pc_data.delta_lat_deg ) &&
              is_eq(d1.pc_data.delta_lon_deg, d2.pc_data.delta_lon_deg ) &&
              d1.pc_data.Nlat              == d2.pc_data.Nlat            &&
              d1.pc_data.Nlon              == d2.pc_data.Nlon )  match = true;
         break;

      case MercatorProj:
         if ( is_eq(d1.mc_data.lat_ll_deg, d2.mc_data.lat_ll_deg ) &&
              is_eq(d1.mc_data.lon_ll_deg, d2.mc_data.lon_ll_deg ) &&
              is_eq(d1.mc_data.lat_ur_deg, d2.mc_data.lat_ur_deg ) &&
              is_eq(d1.mc_data.lon_ur_deg, d2.mc_data.lon_ur_deg ) &&
              d1.mc_data.nx             == d2.mc_data.nx           &&
              d1.mc_data.ny             == d2.mc_data.ny )  match = true;
         break;

      case NoProj:
      default:
         match = false;
         break;
   }

}

return ( match );

}


////////////////////////////////////////////////////////////////////////


bool Grid::operator!=(const Grid &g)

{

return ( !(*this == g) );

}

////////////////////////////////////////////////////////////////////////


void Grid::detach()

{

if ( !rep )  return;

if ( --(rep->refCount) <= 0 )  { delete rep;  rep = (GridRep *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::attach(GridRep *r)

{

detach();

rep = r;

if ( rep )  ++(rep->refCount);

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::latlon_to_xy(double lat, double lon, double &x, double &y) const

{

if ( !rep )  { x = y = 0.0;  return; }

rep->latlon_to_xy(lat, lon, x, y);

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::xy_to_latlon(double x, double y, double &lat, double &lon) const

{

if ( !rep )  { lat = lon = 0.0;  return; }

rep->xy_to_latlon(x, y, lat, lon);

return;

}


////////////////////////////////////////////////////////////////////////


double Grid::calc_area(int x, int y) const

{

if ( !rep )  return ( 0.0 );

return ( rep->calc_area(x, y) );

}


////////////////////////////////////////////////////////////////////////


double Grid::calc_area_ll(int x, int y) const

{

if ( !rep )  return ( 0.0 );

return ( rep->calc_area_ll(x, y) );

}


////////////////////////////////////////////////////////////////////////


int Grid::nx() const

{

if ( !rep )  return ( 0 );

return ( rep->nx() );

}


////////////////////////////////////////////////////////////////////////


int Grid::ny() const

{

if ( !rep )  return ( 0 );

return ( rep->ny() );

}


////////////////////////////////////////////////////////////////////////


double Grid::EarthRadiusKM() const

{

if ( !rep )  return ( 0.0 );

return ( rep->EarthRadiusKM() );

}


////////////////////////////////////////////////////////////////////////


const char * Grid::name() const

{

if ( !rep )  return ( "(no name)" );

return ( rep->name() );

}


////////////////////////////////////////////////////////////////////////


ProjType Grid::proj_type() const

{

if ( !rep )  return ( NoProj );

return ( rep->proj_type() );

}

////////////////////////////////////////////////////////////////////////


double Grid::rot_grid_to_earth(int x, int y) const

{

if ( !rep )  return ( 0.0 );

return ( rep->rot_grid_to_earth(x, y) );

}


////////////////////////////////////////////////////////////////////////


void Grid::grid_data(GridData &gdata) const
{

if ( !rep )  return;

return ( rep->grid_data(gdata) );

}

////////////////////////////////////////////////////////////////////////
